"""Optional Gymnasium adapter for a hybrid-action learner against a scripted opponent.

    pip install gymnasium numpy
    python python/gym_env.py

Use Batch directly for self-play/vectorized training. This wrapper does not implement PPO.
"""
import numpy as np
import gymnasium as gym
from gymnasium import spaces
from creature import Batch, SLICES, quantize, ARENA_COUNT

SHAPES = dict(self=(66,), entities=(69,44), moves=(5,48), announced=(48,),
              history=(24,8), global_=(32,), mask=(6,))

class CreatureEnv(gym.Env):
    metadata = {'render_modes': []}

    def __init__(self, species=0, opponent=1, weather=0, arena=0):
        super().__init__()
        self.species,self.opponent,self.weather,self.arena=species,opponent,weather,arena
        self.batch=Batch(1,species=(species,opponent),weather=weather,arena=arena)
        self.action_space=spaces.Dict({'motion':spaces.Box(-1.,1.,(4,),np.float32),
                                      'ability':spaces.Discrete(6)})
        self.observation_space=spaces.Dict({key:spaces.Box(-100.,100.,shape,np.float32)
                                           for key,shape in SHAPES.items()})
        self.ended=True

    def _observe(self):
        flat=np.asarray(self.batch.observe(),dtype=np.float32)
        return {k:flat[SLICES[k]].reshape(shape).copy() for k,shape in SHAPES.items()}

    def reset(self, *, seed=None, options=None):
        super().reset(seed=seed)
        options=options or {}
        match_seed=int(self.np_random.integers(1,2**32,dtype=np.uint32))
        self.batch.reset(0,match_seed,options.get('weather',self.weather),
                         (options.get('species',self.species),options.get('opponent',self.opponent)),
                         options.get('arena',self.arena))
        self.ended=False
        return self._observe(),{'match_seed':match_seed}

    def step(self, action):
        if self.ended:raise RuntimeError('reset is required before stepping an ended episode')
        if not self.action_space.contains(action):raise ValueError('Invalid hybrid action')
        actions=list(self.batch.scripted_actions())
        motion=action['motion']
        actions[:5]=quantize(motion[:2],motion[2:],int(action['ability']))
        _,features,status=self.batch.step(actions)
        self.ended=bool(status[0] or status[1])
        reward=0. if not self.ended or status[2]<0 else 1. if status[2]==0 else -1.
        # A scored 90s verdict is part of this finite game, so it is terminal for the
        # learner. Native status retains its separate clock-expiration flag for UI.
        return self._observe(),reward,self.ended,False,{
            'features':np.asarray(features,dtype=np.int32).reshape(2,10).copy(),
            'winner':int(status[2]),'native_time_limit':bool(status[1]),'ticks':int(status[3])}

    def close(self):
        self.batch.close()

if __name__=='__main__':
    from gymnasium.utils.env_checker import check_env
    env=CreatureEnv(species=21,opponent=8)
    check_env(env,skip_render_check=True)
    for species in range(40):
        obs,_=env.reset(seed=species,options={'species':species,'opponent':(species+17)%40,'arena':species%ARENA_COUNT})
        assert env.observation_space.contains(obs)
        for _ in range(8):
            obs,reward,terminated,truncated,info=env.step(env.action_space.sample())
            assert env.observation_space.contains(obs)
            if terminated or truncated:break
    env.close()
    print('Gymnasium checker and 40-species hybrid-action smoke passed')
