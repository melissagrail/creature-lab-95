"""Optional lightweight PyTorch policy reference; weights are initially RANDOM.

No torch dependency in the engine, viewer or ctypes bridge. Install separately:
    python -m pip install -r python/requirements-ml.txt
    python python/policy.py

This is an integration/gradient smoke test, NOT a learning benchmark or trained brain.
"""
import torch
from torch import nn
from creature import OBS_SIZE, SELF_SIZE, ENTITY_COUNT, ENTITY_SIZE, MOVE_SIZE, SLICES, Batch, quantize


class CreaturePolicy(nn.Module):
    """Masked entity/move/history encoders + 96-wide recurrent individual brain.

    obs: [batch,3620], memory: [batch,96]. For episodes/branches, manage memory
    explicitly; it does not belong to the deterministic physics snapshot.
    """
    def __init__(self):
        super().__init__()
        self.entity = nn.Sequential(nn.Linear(ENTITY_SIZE-1, 32), nn.Tanh())
        self.move = nn.Sequential(nn.Linear(MOVE_SIZE, 24), nn.Tanh())
        self.event = nn.Sequential(nn.Linear(7, 24), nn.Tanh())
        self.encoder = nn.Sequential(nn.Linear(SELF_SIZE + 32 + 24 + 24 + 32 + 24, 96), nn.Tanh())
        self.memory = nn.GRUCell(96, 96)
        self.motion = nn.Linear(96, 4)
        self.noop = nn.Linear(96, 1)
        self.slot_score = nn.Sequential(nn.Linear(96+24, 32), nn.Tanh(), nn.Linear(32, 1))
        self.value = nn.Linear(96, 1)
        self.log_std = nn.Parameter(torch.full((4,), -.7))

    @staticmethod
    def pool(values, mask):
        return (values * mask).sum(1) / mask.sum(1).clamp_min(1)

    def forward(self, obs, memory):
        entities = obs[:, SLICES["entities"]].reshape(-1, ENTITY_COUNT, ENTITY_SIZE)
        moves = obs[:, SLICES["moves"]].reshape(-1, 5, MOVE_SIZE)
        history = obs[:, SLICES["history"]].reshape(-1, 24, 8)
        x = torch.cat((obs[:, SLICES["self"]],
                       self.pool(self.entity(entities[:, :, :-1]), entities[:, :, -1:]),
                       self.move(moves).mean(1),
                       self.pool(self.event(history[:, :, :7]), history[:, :, 7:8]),
                       obs[:, SLICES["global_"]], self.move(obs[:, SLICES["announced"]])), dim=-1)
        hidden = self.memory(self.encoder(x), memory)
        # Score each move using its actual descriptor, preserving slot identity.
        slot_latent = torch.cat((hidden[:,None,:].expand(-1,5,-1), self.move(moves)), dim=-1)
        logits = torch.cat((self.noop(hidden), self.slot_score(slot_latent).squeeze(-1)), dim=-1)
        logits = logits.masked_fill(obs[:, SLICES["mask"]] < .5, -1e9)
        return self.motion(hidden), logits, self.value(hidden).squeeze(-1), hidden

    def sample(self, obs, memory, deterministic=False):
        mean, logits, value, hidden = self(obs, memory)
        normal = torch.distributions.Normal(mean, self.log_std.exp())
        categorical = torch.distributions.Categorical(logits=logits)
        # Non-reparameterized samples for a score-function policy-gradient estimator.
        raw = mean if deterministic else normal.sample()
        ability = logits.argmax(-1) if deterministic else categorical.sample()
        motion = raw.tanh()
        log_prob = (normal.log_prob(raw) - torch.log(1 - motion.square() + 1e-6)).sum(-1)
        return motion, ability, log_prob + categorical.log_prob(ability), value, hidden


def main():
    torch.manual_seed(7)
    policy = CreaturePolicy()
    with Batch(4) as batch:
        obs = torch.tensor([list(batch.observe(i, p)) for i in range(4) for p in range(2)])
        memory = torch.zeros(8, 96)
        motion, ability, log_prob, value, hidden = policy.sample(obs, memory)
        assert hidden.shape == (8, 96) and motion.shape == (8, 4)
        assert torch.isfinite(log_prob).all()
        actions = []
        for values, skill in zip(motion.detach().tolist(), ability.tolist()):
            actions.extend(quantize(values[:2], values[2:], skill))
        batch.step(actions)
        # Exercise gradients using synthetic advantages, without claiming learning.
        loss = -(log_prob * torch.linspace(-1, 1, 8)).mean() + value.square().mean()
        loss.backward()
        assert all(p.grad is None or torch.isfinite(p.grad).all() for p in policy.parameters())
        print(f'{sum(p.numel() for p in policy.parameters()):,} parameters; inference, '
              'action masks, native step and gradient smoke test passed')


if __name__ == '__main__':
    main()
