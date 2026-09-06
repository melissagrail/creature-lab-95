"""Small species- and preference-conditioned recurrent policy; training is separate from play."""
import torch
from torch import nn
from creature import SELF_SIZE, ENTITY_SIZE, ENTITY_COUNT, MOVE_SIZE, SLICES
from legacy_policy import LegacyPolicy

TRAITS = 3
PRESETS = {
    'steady': (0., 0., 0.),
    'aggressive': (1., -.25, 0.),
    'skittish': (-1., .4, 0.),
    'patient': (0., 1., .25),
    'territorial': (0., .2, 1.),
}

def personality_from_seed(seed):
    if not isinstance(seed,int) or not 0 <= seed <= 0xffffffff:
        raise ValueError('Individual seed must be a uint32')
    state=seed or 1;result=[]
    for _ in range(3):
        state=(state ^ (state << 13)) & 0xffffffff
        state=state ^ (state >> 17)
        state=(state ^ (state << 5)) & 0xffffffff
        result.append((state % 2049 - 1024) / 1024.)
    return tuple(result)

class CreaturePolicy(LegacyPolicy):
    """Format 3. Species embeddings, persistent temperament and ability-conditioned controls.

    Personality is [aggression, reserve, territory], each in [-1,1], supplied separately
    from the unchanged public simulation observation. Recurrent memory is still 96 floats.
    """
    brain_format = 3

    def __init__(self):
        super().__init__()
        self.encoder = nn.Sequential(nn.Linear(272, 96), nn.Tanh())
        self.species = nn.Embedding(40, 12)
        self.slot_motion = nn.Linear(32, 4)
        nn.init.normal_(self.species.weight, std=.2)
        nn.init.zeros_(self.slot_motion.weight)
        nn.init.zeros_(self.slot_motion.bias)

    def forward(self, obs, memory, personality=None):
        if personality is None:
            personality = obs.new_zeros((len(obs), TRAITS))
        entities = obs[:, SLICES['entities']].reshape(-1, ENTITY_COUNT, ENTITY_SIZE)
        moves = obs[:, SLICES['moves']].reshape(-1, 5, MOVE_SIZE)
        history = obs[:, SLICES['history']].reshape(-1, 24, 8)
        own_id = (obs[:,22]*39).round().long().clamp(0,39)
        enemy_id = (entities[:,0,20]*39).round().long().clamp(0,39)
        move_latent = self.move(moves)
        x = torch.cat((obs[:, SLICES['self']], entities[:,0,:-1],
            self.pool(self.entity(entities[:,:,:-1]), entities[:,:,-1:]), move_latent.mean(1),
            self.pool(self.event(history[:,:,:7]), history[:,:,7:8]),
            obs[:, SLICES['global_']], self.move(obs[:, SLICES['announced']]),
            self.species(own_id), self.species(enemy_id), personality), dim=-1)
        hidden = self.memory(self.encoder(x), memory)
        slots = torch.cat((hidden[:,None,:].expand(-1,5,-1), move_latent), dim=-1)
        latent = self.slot_score[1](self.slot_score[0](slots))
        logits = torch.cat((self.noop(hidden), self.slot_score[2](latent).squeeze(-1)), dim=-1)
        logits = logits.masked_fill(obs[:, SLICES['mask']] < .5, -1e9)
        base = self.motion(hidden)
        motion = torch.cat((base[:,None,:], base[:,None,:] + self.slot_motion(latent)), dim=1)
        return motion, logits, self.value(hidden).squeeze(-1), hidden

    def sample(self, obs, memory, deterministic=False, personality=None):
        from learning import distribution
        motion, ability, _, lp, value, hidden, _ = distribution(
            self, obs, memory, deterministic=deterministic, personality=personality)
        return motion, ability, lp, value, hidden


def upgrade(legacy):
    """Preserve the v2 neutral function before training, up to floating-point summation."""
    if getattr(legacy, 'brain_format', 2) == 3:
        return legacy
    policy = CreaturePolicy()
    state = policy.state_dict()
    for name, tensor in legacy.state_dict().items():
        if name == 'encoder.0.weight':
            state[name].zero_()
            state[name][:,:245] = tensor
        else:
            state[name] = tensor
    policy.load_state_dict(state)
    return policy


def selected_motion(mean, ability):
    return mean if mean.ndim == 2 else mean[torch.arange(len(mean), device=mean.device), ability]

def main():
    from creature import Batch
    torch.manual_seed(7)
    policy = CreaturePolicy()
    with Batch(4) as batch:
        obs = torch.tensor([list(batch.observe(i, p)) for i in range(4) for p in range(2)])
        memory = torch.zeros(8, 96)
        motion, ability, log_prob, value, hidden = policy.sample(obs, memory)
        assert hidden.shape == (8, 96) and motion.shape == (8, 4)
        assert torch.isfinite(log_prob).all()
        from learning import world_actions
        actions = world_actions(motion.detach().numpy(), ability.numpy(), obs.numpy())
        batch.step(actions.reshape(-1).tolist())
        # Exercise gradients using synthetic advantages, without claiming learning.
        loss = -(log_prob * torch.linspace(-1, 1, 8)).mean() + value.square().mean()
        loss.backward()
        assert all(p.grad is None or torch.isfinite(p.grad).all() for p in policy.parameters())
        print(f'{sum(p.numel() for p in policy.parameters()):,} parameters; inference, '
              'action masks, native step and gradient smoke test passed')


if __name__ == '__main__':
    main()
