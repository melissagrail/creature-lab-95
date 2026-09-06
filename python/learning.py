"""Shared learner/runtime contract. No privileged simulator state enters the actor."""
from pathlib import Path
import ctypes as C
import json
import math
import struct
import numpy as np
import torch
from torch.distributions import Normal, Categorical
from creature import Batch, OBS_SIZE, SLICES, I, P, library
from policy import CreaturePolicy, PRESETS, selected_motion
from legacy_policy import LegacyPolicy

FORMAT = 3
HIDDEN = 96
MAGIC = b'TINIBRN1'
PARAMETERS = 89959
PARAMETER_COUNTS = {2:86755, 3:89959}

def identity(format=FORMAT):
    lib = library()
    stamp = dict(format=format, rules=lib.cr_version(), observations=lib.cr_observation_version(),
                content=lib.cr_content_hash(), observation_size=OBS_SIZE, hidden=HIDDEN,
                action_codec='opponent-local-tanh-v1' if format==2 else 'opponent-local-conditional-tanh-v2')
    if format==3:stamp['personality']=['aggression','reserve','territory']
    return stamp

def load_checkpoint(path):
    # Tensor/basic-type checkpoints only; do not execute arbitrary pickled Python objects.
    data = torch.load(path, map_location='cpu', weights_only=True)
    format=data.get('identity',{}).get('format')
    if format not in PARAMETER_COUNTS or data.get('identity') != identity(format):
        raise ValueError('Incompatible brain format, rules, observations, content or action codec')
    policy = CreaturePolicy() if format==3 else LegacyPolicy()
    policy.load_state_dict(data['policy'], strict=True)
    if not all(torch.isfinite(p).all() for p in policy.parameters()):
        raise ValueError('Nonfinite brain weights')
    return policy, data

def save_checkpoint(path, policy, **extra):
    path=Path(path); path.parent.mkdir(parents=True, exist_ok=True)
    temp=path.with_suffix(path.suffix+'.tmp')
    torch.save(dict(identity=identity(policy.brain_format), policy=policy.state_dict(), **extra), temp)
    temp.replace(path)

def fnv(data):
    value=2166136261
    for byte in data: value=((value ^ byte)*16777619)&0xffffffff
    return value

def export_brain(policy, path, metadata=None):
    path=Path(path); path.parent.mkdir(parents=True,exist_ok=True)
    values=np.concatenate([p.detach().cpu().numpy().reshape(-1) for p in policy.parameters()]).astype('<f4')
    format=policy.brain_format;parameters=PARAMETER_COUNTS[format]
    if values.size!=parameters or not np.isfinite(values).all() or np.abs(values).max()>1000:
        raise ValueError('Invalid dimensions, nonfinite or excessive weights for native export')
    stamp=identity(format)
    header=MAGIC+struct.pack('<7I', format,stamp['rules'],stamp['observations'],stamp['content'],OBS_SIZE,HIDDEN,parameters)
    payload=header+values.tobytes()
    checksum=fnv(payload)
    temp=path.with_suffix('.tmp');temp.write_bytes(payload+struct.pack('<I',checksum));temp.replace(path)
    manifest=dict(identity=stamp,parameters=parameters,bytes=path.stat().st_size,checksum=f'{checksum:08x}',
                  parameter_order=[dict(name=n,shape=list(p.shape)) for n,p in policy.named_parameters()],
                  metadata=metadata or {})
    path.with_suffix('.json').write_text(json.dumps(manifest,indent=2)+'\n')
    return manifest

def action_basis(obs):
    facing=obs[:,4:6]
    enemy=obs[:,SLICES['entities'].start+1:SLICES['entities'].start+3]
    basis=np.column_stack((enemy[:,0]*facing[:,0]-enemy[:,1]*facing[:,1],
                           enemy[:,0]*facing[:,1]+enemy[:,1]*facing[:,0]))
    length=np.sqrt((basis*basis).sum(-1,keepdims=True))
    return np.where(length>1e-8,basis/np.maximum(length,1e-8),facing)

def world_actions(local, ability, obs):
    """Rotate opponent-local control to world axes, then ties-away quantize once."""
    a=np.asarray(local,dtype=np.float32).reshape(-1,2,2)
    facing=action_basis(obs)
    out=np.empty_like(a)
    out[:,:,0]=a[:,:,0]*facing[:,0:1]-a[:,:,1]*facing[:,1:2]
    out[:,:,1]=a[:,:,0]*facing[:,1:2]+a[:,:,1]*facing[:,0:1]
    scaled=np.clip(out.reshape(-1,4),-1,1)*1024
    quantized=(np.sign(scaled)*np.floor(np.abs(scaled)+.5)).astype(np.int32)
    return np.column_stack((quantized,np.asarray(ability,dtype=np.int32)))

def local_teacher(actions,obs):
    a=actions[:,:4].astype(np.float32).reshape(-1,2,2)/1024
    facing=action_basis(obs)
    out=np.empty_like(a)
    out[:,:,0]=a[:,:,0]*facing[:,0:1]+a[:,:,1]*facing[:,1:2]
    out[:,:,1]=-a[:,:,0]*facing[:,1:2]+a[:,:,1]*facing[:,0:1]
    return np.clip(out.reshape(-1,4),-1,1)

def distribution(policy, obs, memory, raw=None, ability=None, deterministic=False, personality=None):
    mean,logits,value,hidden=policy(obs,memory,personality)
    categorical=Categorical(logits=logits)
    if ability is None: ability=logits.argmax(-1) if deterministic else categorical.sample()
    mean=selected_motion(mean,ability)
    normal=Normal(mean,policy.log_std.clamp(-3,0).exp())
    if raw is None: raw=mean if deterministic else normal.sample()
    motion=raw.tanh()
    # Stable log(tanh Jacobian). Keep latent samples: do not invert quantized actions.
    correction=2*(math.log(2)-raw-torch.nn.functional.softplus(-2*raw))
    logp=(normal.log_prob(raw)-correction).sum(-1)+categorical.log_prob(ability)
    entropy=normal.entropy().sum(-1)+categorical.entropy()
    return motion,ability,raw,logp,value,hidden,entropy

def potential(obs):
    own=obs[:,0]; enemy=obs[:,SLICES['entities'].start+11]
    g=obs[:,SLICES['global_']]
    center=np.sqrt((obs[:,13]-.5)**2+((obs[:,14]-.5)*.75)**2)
    return 2*(own-enemy)+(g[:,8]-g[:,9])-.4*center

def rewards(before, after, done, winners, seats, gamma):
    # Potential-based shaping telescopes. Terminal potential MUST be zero (also at 90s verdict).
    phi_next=np.where(done,0,potential(after))
    outcome=np.where(winners<0,0,np.where(winners==seats,3.,-3.))
    return (gamma*phi_next-potential(before)+np.where(done,outcome,0)).astype(np.float32)

def gae(reward, value, done, bootstrap, gamma=.997, lam=.95):
    advantage=np.zeros_like(reward); carry=np.zeros(reward.shape[1],np.float32)
    for t in range(len(reward)-1,-1,-1):
        following=bootstrap if t==len(reward)-1 else value[t+1]
        live=1-done[t]
        delta=reward[t]+gamma*following*live-value[t]
        carry=delta+gamma*lam*live*carry
        advantage[t]=carry
    return advantage,advantage+value

class ArenaBatch:
    def __init__(self,count,seed=7,scenarios=None,personality_rate=0.,fixed_personality=None,target_species=None):
        self.count=count; self.rng=np.random.default_rng(seed);self.scenarios=scenarios
        self.personality_rate=personality_rate;self.fixed_personality=fixed_personality;self.target_species=target_species
        self.personalities=np.zeros((count,2,3),np.float32)
        self.next_case=0;self.batch=Batch(count);self.indices=np.arange(count)
        self.seats=np.zeros(count,np.int64); self.styles=np.zeros((count,2),np.int32)
        self.episode=np.zeros(count,np.int64); self.case=np.zeros(count,np.int64)
        self.obs=np.zeros((count,2,OBS_SIZE),np.float32)
        self.reset_before=np.ones(count,np.float32)
        self.actions=np.zeros((count,2,5),np.int32)
        self.teacher_fn=self.batch.lib.cr_batch_scripted
        self.teacher_fn.argtypes=[C.POINTER(P),C.c_size_t,C.POINTER(I),C.POINTER(I)]
        self.teacher_fn.restype=I
        for i in range(count):self.reset(i)
    def reset(self,i):
        if self.scenarios is None:
            a,b=self.rng.integers(0,40,2); arena=int(self.rng.integers(6)); weather=int(self.rng.integers(3))
            seed=int(self.rng.integers(1,100_000_000));seat=int(self.rng.integers(2));style=int(self.rng.integers(4))
        else:
            case=self.next_case%len(self.scenarios);self.next_case+=1;self.case[i]=case
            a,b,arena,weather,seed,seat,style=self.scenarios[case]
        if self.target_species is not None:a=self.target_species
        species=(int(a),int(b)) if seat==0 else (int(b),int(a))
        self.batch.reset(i,seed,weather,species,arena)
        self.styles[i]=self.rng.integers(4,size=2) if self.scenarios is None else [style,style]
        self.styles[i,1-seat]=style;self.seats[i]=seat;self.episode[i]=0
        self.reset_before[i]=1
        self.personalities[i]=0
        if self.fixed_personality is not None:
            self.personalities[i,seat]=self.fixed_personality
        elif self.rng.random()<self.personality_rate:
            for p in range(2):
                if self.rng.random()<.85:
                    self.personalities[i,p]=list(PRESETS.values())[int(self.rng.integers(1,len(PRESETS)))]
                else:self.personalities[i,p]=self.rng.uniform(-1,1,3)
        for p in range(2):self.obs[i,p]=np.asarray(self.batch.observe(i,p))
    def learner_obs(self):return self.obs[self.indices,self.seats].copy()
    def learner_personality(self):return self.personalities[self.indices,self.seats].copy()
    def teacher(self):
        result=self.teacher_fn(self.batch.handles,self.count,self.styles.ctypes.data_as(C.POINTER(I)),self.actions.ctypes.data_as(C.POINTER(I)))
        if result:raise RuntimeError('Native batched scripted policy failed')
        return self.actions.copy()
    def step(self, actions):
        o,f,s=self.batch.step(actions.reshape(-1).tolist())
        self.obs[:]=np.ctypeslib.as_array(o).reshape(self.count,2,OBS_SIZE)
        features=np.ctypeslib.as_array(f).reshape(self.count,2,10).copy()
        status=np.ctypeslib.as_array(s).reshape(self.count,4).copy()
        self.episode+=1;self.reset_before[:]=0
        return features,status
    def close(self):self.batch.close()

def cases(count,seed=900_000_000):
    return [(i%40,(i%40+17+7*(i//40))%40,(i//40+i)%6,(i+i//40)%3,
             seed+i,(i+i//40)%2,(i//6)%4) for i in range(count)]

@torch.no_grad()
def evaluate(policy,count=240,seed=900_000_000,opponent='scripted',personality=(0.,0.,0.),species=None):
    scenarios=cases(count,seed)
    if species is not None:scenarios=[(species,*row[1:]) for row in scenarios]
    env=ArenaBatch(min(32,count),seed,scenarios,fixed_personality=personality)
    memory=torch.zeros(env.count,HIDDEN); results={}; steps=0
    try:
        while len(results)<count:
            obs=env.learner_obs(); memory*=torch.from_numpy(1-env.reset_before[:,None])
            motion,ability,_,_,_,memory,_=distribution(policy,torch.from_numpy(obs),memory,deterministic=True,personality=torch.from_numpy(env.learner_personality()))
            acts=env.teacher()
            if opponent=='idle':acts[env.indices,1-env.seats]=0
            acts[env.indices,env.seats]=world_actions(motion.numpy(),ability.numpy(),obs)
            _,status=env.step(acts);steps+=env.count
            for i in np.flatnonzero(status[:,0]|status[:,1]):
                case=int(env.case[i]);win=int(status[i,2]);seat=int(env.seats[i])
                if case not in results:
                    results[case]=dict(case=case,species=scenarios[case][0],opponent=scenarios[case][1],arena=scenarios[case][2],
                        seed=scenarios[case][4],seat=seat,style=scenarios[case][6],score=.5 if win<0 else float(win==seat),
                        seconds=float(env.obs[i,seat,SLICES['global_'].start]*90),reason=int(round(env.obs[i,seat,SLICES['global_'].start+12]*3)))
                env.reset(int(i))
    finally:env.close()
    rows=[results[i] for i in range(count)]
    return dict(matches=count,score=sum(r['score'] for r in rows)/count,
                mean_seconds=sum(r['seconds'] for r in rows)/count,seed_base=seed,rows=rows)


def personality_reward(obs, personality, scale=.003):
    """A bounded, deliberately non-potential style preference, separate from match reward."""
    enemy=obs[:,SLICES['entities']]
    distance=np.sqrt((enemy[:,1:3]**2).sum(-1))*24
    center=np.sqrt(((obs[:,13]-.5)*24)**2+((obs[:,14]-.5)*18)**2)
    features=np.column_stack((1-2*np.clip(distance/10,0,1),
                              2*np.clip(obs[:,1],0,1)-1,1-2*np.clip(center/10,0,1)))
    return (scale*(features*personality).sum(-1)).astype(np.float32)
