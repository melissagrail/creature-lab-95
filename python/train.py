"""A reproducible imitation warm start + recurrent PPO learner for the native engine."""
import argparse
import gc
import json
import time
from pathlib import Path
import numpy as np
import torch
from torch.nn import functional as F
from learning import (ArenaBatch, CreaturePolicy, HIDDEN, OBS_SIZE, SLICES, distribution,
    world_actions, local_teacher, rewards, gae, evaluate, save_checkpoint, load_checkpoint, export_brain, personality_reward)
from policy import upgrade, selected_motion, PRESETS, personality_from_seed

@torch.no_grad()
def collect_demonstrations(count,steps,seed,policy=None,personality_rate=.6):
    env=ArenaBatch(count,seed,personality_rate=personality_rate);actors=count*2
    memory=torch.zeros(actors,HIDDEN)
    data=dict(obs=np.empty((steps,actors,OBS_SIZE),np.float16),motion=np.empty((steps,actors,4),np.float32),
              ability=np.empty((steps,actors),np.int64),reset=np.empty((steps,actors),np.float32),
              personality=np.empty((steps,actors,3),np.float32))
    try:
        for t in range(steps):
            o=env.obs.reshape(actors,OBS_SIZE).copy()
            traits=env.personalities.reshape(actors,3).copy()
            env.styles[:]=np.where(env.personalities[:,:,0]>.4,1,np.where(env.personalities[:,:,0]<-.4,2,
                                      np.where(env.personalities[:,:,2]>.4,3,0)))
            a=env.teacher()
            # Reservation examples are labels only; deployment has no scripted override.
            patient=(env.personalities[:,:,1]>.6)&(env.obs[:,:,1]<.55)
            a[:,:,4][patient]=0
            data['personality'][t]=traits
            data['obs'][t]=o;data['motion'][t]=local_teacher(a.reshape(actors,5),o)
            data['ability'][t]=a.reshape(actors,5)[:,4]
            data['reset'][t]=np.repeat(env.reset_before,2)
            if policy is not None:
                memory*=torch.from_numpy(1-np.repeat(env.reset_before,2)[:,None])
                motion,choice,_,_,_,memory,_=distribution(policy,torch.from_numpy(o),memory,
                    deterministic=True,personality=torch.from_numpy(traits))
                proposal=world_actions(motion.numpy(),choice.numpy(),o).reshape(count,2,5)
                use=env.rng.random((count,2))<.65
                a[use]=proposal[use]
            # Perturb 10% of travel requests to include imperfect trajectories in the demonstration states.
            disturb=env.rng.random((count,2))<.1
            a[:,:,:2][disturb]=env.rng.integers(-1024,1025,size=(int(disturb.sum()),2))
            _,status=env.step(a)
            for i in np.flatnonzero(status[:,0]|status[:,1]):env.reset(int(i))
    finally:env.close()
    return data

def chunks(data,indices,length):
    actors=data['obs'].shape[1]
    starts=(indices//actors)*length
    actor=indices%actors
    return {k:torch.from_numpy(v[starts[:,None]+np.arange(length)[None,:],actor[:,None]].swapaxes(0,1).copy()).float()
            for k,v in data.items()}

def imitate(policy,data,epochs,seq,minibatch,rng,log):
    optimizer=torch.optim.Adam(policy.parameters(),lr=7e-4)
    n=(len(data['obs'])//seq)*data['obs'].shape[1]
    for epoch in range(epochs):
        order=rng.permutation(n);totals=[]
        for start in range(0,n,minibatch):
            b=chunks(data,order[start:start+minibatch],seq)
            h=torch.zeros(b['obs'].shape[1],HIDDEN)
            motion_loss=0;aim_loss=0;choice_loss=0;choices=0
            for t in range(seq):
                h=h*(1-b['reset'][t,:,None])
                mean,logits,_,h=policy(b['obs'][t],h,b['personality'][t])
                pred=selected_motion(mean,b['ability'][t].long()).tanh();target=b['motion'][t]
                motion_loss+=F.mse_loss(pred[:,:2],target[:,:2])
                aim_loss+=F.mse_loss(pred[:,2:],target[:,2:])
                selectable=(b['obs'][t,:,SLICES['mask']].sum(-1)>1).float()
                label=b['ability'][t].long();weight=torch.where(label>0,2.,1.)*selectable
                choice_loss+=(F.cross_entropy(logits,label,reduction='none')*weight).sum()
                choices+=weight.sum()
            loss=5*motion_loss/seq+8*aim_loss/seq+choice_loss/choices.clamp_min(1)
            optimizer.zero_grad();loss.backward();torch.nn.utils.clip_grad_norm_(policy.parameters(),1.);optimizer.step()
            totals.append(float(loss.detach()))
        log(dict(stage='imitation',epoch=epoch+1,loss=float(np.mean(totals))))
    with torch.no_grad():policy.log_std.copy_(torch.tensor([-1.2,-1.2,-2.,-2.]))

@torch.no_grad()
def rollout(policy,env,memory,horizon,gamma,opponents=None,style_scale=.003):
    n=env.count
    data=dict(obs=np.empty((horizon,n,OBS_SIZE),np.float32),memory=np.empty((horizon,n,HIDDEN),np.float32),
              raw=np.empty((horizon,n,4),np.float32),ability=np.empty((horizon,n),np.float32),
              logp=np.empty((horizon,n),np.float32),value=np.empty((horizon,n),np.float32),
              personality=np.empty((horizon,n,3),np.float32),
              reward=np.empty((horizon,n),np.float32),done=np.empty((horizon,n),np.float32),reset=np.empty((horizon,n),np.float32))
    endings=[]
    for t in range(horizon):
        obs=env.learner_obs();memory*=torch.from_numpy(1-env.reset_before[:,None])
        data['obs'][t]=obs;data['memory'][t]=memory.numpy();data['reset'][t]=env.reset_before
        data['personality'][t]=env.learner_personality()
        motion,ability,raw,logp,value,memory,_=distribution(policy,torch.from_numpy(obs),memory,
            personality=torch.from_numpy(data['personality'][t]))
        data['raw'][t]=raw.numpy();data['ability'][t]=ability.numpy();data['logp'][t]=logp.numpy();data['value'][t]=value.numpy()
        actions=env.teacher()
        if opponents is not None:opponents.apply(env,actions)
        actions[env.indices,env.seats]=world_actions(motion.numpy(),ability.numpy(),obs)
        _,status=env.step(actions);done=(status[:,0]|status[:,1]).astype(bool)
        data['reward'][t]=rewards(obs,env.learner_obs(),done,status[:,2],env.seats,gamma);data['reward'][t]+=np.where(done,0,personality_reward(env.learner_obs(),data['personality'][t],style_scale))
        data['done'][t]=done
        for i in np.flatnonzero(done):
            endings.append(.5 if status[i,2]<0 else float(status[i,2]==env.seats[i]))
            env.reset(int(i))
    obs=env.learner_obs();memory*=torch.from_numpy(1-env.reset_before[:,None])
    _,_,bootstrap,_=policy(torch.from_numpy(obs),memory,torch.from_numpy(env.learner_personality()))
    advantage,returns=gae(data['reward'],data['value'],data['done'],bootstrap.numpy(),gamma)
    data['advantage']=(advantage-advantage.mean())/(advantage.std()+1e-8);data['returns']=returns
    return data,memory,endings

def ppo_update(policy,optimizer,data,args,rng):
    length=args.sequence;n=(len(data['obs'])//length)*data['obs'].shape[1]
    metrics=[];stop=False
    for epoch in range(args.epochs):
        order=rng.permutation(n)
        for start in range(0,n,args.minibatch):
            b=chunks(data,order[start:start+args.minibatch],length)
            h=b['memory'][0].detach();logps=[];values=[];entropies=[]
            for t in range(length):
                h=h*(1-b['reset'][t,:,None])
                _,_,_,lp,value,h,entropy=distribution(policy,b['obs'][t],h,b['raw'][t],b['ability'][t].long(),personality=b['personality'][t])
                logps.append(lp);values.append(value);entropies.append(entropy)
            logp=torch.stack(logps);value=torch.stack(values);entropy=torch.stack(entropies)
            logratio=logp-b['logp'];ratio=logratio.exp()
            with torch.no_grad():kl=((ratio-1)-logratio).mean()
            if not torch.isfinite(kl):raise RuntimeError('Nonfinite PPO likelihood ratio')
            if kl > args.target_kl*2:stop=True;break
            actor=-torch.minimum(ratio*b['advantage'],ratio.clamp(1-args.clip,1+args.clip)*b['advantage']).mean()
            value_clipped=b['value']+(value-b['value']).clamp(-args.clip,args.clip)
            critic=.5*torch.maximum((value-b['returns']).square(),(value_clipped-b['returns']).square()).mean()
            loss=actor+.5*critic-args.entropy*entropy.mean()
            optimizer.zero_grad();loss.backward();norm=torch.nn.utils.clip_grad_norm_(policy.parameters(),.5)
            if not torch.isfinite(norm):raise RuntimeError('Nonfinite PPO gradient')
            optimizer.step()
            with torch.no_grad():policy.log_std.clamp_(-3,0)
            metrics.append((float(actor.detach()),float(critic.detach()),float(kl),float(entropy.mean().detach())))
        if stop:break
    if not metrics:raise RuntimeError('PPO update rejected before any optimizer step; likelihood mismatch')
    result=np.mean(metrics,axis=0)
    return dict(policy_loss=float(result[0]),value_loss=float(result[1]),kl=float(result[2]),entropy=float(result[3]),optimizer_steps=len(metrics))

class FrozenOpponents:
    """Episode-stable mixture of frozen learned pilots and the existing scripted styles."""
    def __init__(self,paths,count,rate):
        self.policies=[load_checkpoint(path)[0].eval() for path in paths]
        for model in self.policies:model.requires_grad_(False)
        self.rate=rate;self.choice=np.full(count,-1,np.int64)
        self.memory=torch.zeros(count,HIDDEN)

    @torch.no_grad()
    def apply(self,env,actions):
        fresh=np.flatnonzero(env.reset_before)
        self.memory[fresh]=0
        for i in fresh:
            self.choice[i]=int(env.rng.integers(len(self.policies))) if env.rng.random()<self.rate else -1
        for k,policy in enumerate(self.policies):
            ids=np.flatnonzero(self.choice==k)
            if not len(ids):continue
            obs=env.obs[ids,1-env.seats[ids]].copy()
            motion,ability,_,_,_,h,_=distribution(policy,torch.from_numpy(obs),self.memory[ids],deterministic=True)
            self.memory[ids]=h
            actions[ids,1-env.seats[ids]]=world_actions(motion.numpy(),ability.numpy(),obs)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--out',type=Path,default=Path('runs/apprentice'))
    p.add_argument('--resume',type=Path);p.add_argument('--warmstart',type=Path)
    p.add_argument('--seed',type=int,default=20260906);p.add_argument('--threads',type=int,default=2)
    p.add_argument('--demo-envs',type=int,default=32);p.add_argument('--demo-steps',type=int,default=512)
    p.add_argument('--bc-epochs',type=int,default=12);p.add_argument('--envs',type=int,default=64)
    p.add_argument('--horizon',type=int,default=128);p.add_argument('--updates',type=int,default=160)
    p.add_argument('--sequence',type=int,default=16);p.add_argument('--minibatch',type=int,default=32)
    p.add_argument('--epochs',type=int,default=3);p.add_argument('--lr',type=float,default=1e-4)
    p.add_argument('--gamma',type=float,default=.9995);p.add_argument('--clip',type=float,default=.2)
    p.add_argument('--entropy',type=float,default=.003);p.add_argument('--target-kl',type=float,default=.03)
    p.add_argument('--eval-every',type=int,default=20);p.add_argument('--eval-games',type=int,default=240)
    p.add_argument('--test-games',type=int,default=240)
    p.add_argument('--personality-rate',type=float,default=.6)
    p.add_argument('--style-scale',type=float,default=.003)
    p.add_argument('--refine-imitation',action='store_true')
    p.add_argument('--opponent',type=Path,action='append',default=[])
    p.add_argument('--learned-opponent-rate',type=float,default=.25)
    p.add_argument('--species',type=int,help='Optional specialist: train only this learner species (0-39)')
    p.add_argument('--personality',choices=list(PRESETS),help='Optional fixed temperament for individual fine-tuning')
    p.add_argument('--individual-seed',type=int,help='Repeatable custom temperament; matches the viewer seed option')
    args=p.parse_args()
    if min(args.envs,args.horizon,args.sequence,args.minibatch,args.epochs,args.updates,args.threads,
           args.demo_envs,args.demo_steps,args.eval_every,args.eval_games,args.test_games)<=0:p.error('Counts must be positive')
    if args.horizon%args.sequence or args.demo_steps%args.sequence:p.error('Horizon and demonstration steps must be divisible by sequence length')
    if not all(np.isfinite(x) for x in (args.lr,args.gamma,args.clip,args.entropy,args.target_kl,args.style_scale)):
        p.error('Learning hyperparameters must be finite')
    if args.bc_epochs<0:p.error('Imitation epochs cannot be negative')
    if args.resume and args.warmstart:p.error('Choose either --resume or --warmstart')
    if not (0<args.gamma<=1 and 0<args.clip<1 and args.lr>0 and args.target_kl>0 and args.entropy>=0):
        p.error('Invalid learning hyperparameters')
    if not (0<=args.personality_rate<=1 and 0<=args.learned_opponent_rate<=1 and args.style_scale>=0):p.error('Invalid personality/opponent options')
    if args.species is not None and not 0<=args.species<40:p.error('Species must be 0-39')
    if args.individual_seed is not None and (args.personality or not 0<=args.individual_seed<=0xffffffff):
        p.error('Use a uint32 individual seed or a named personality, not both')
    fixed_personality=(personality_from_seed(args.individual_seed) if args.individual_seed is not None else
                       PRESETS[args.personality] if args.personality else None)
    eval_options=dict(personality=fixed_personality or PRESETS['steady'],species=args.species)
    args.out.mkdir(parents=True,exist_ok=True)
    torch.set_num_threads(args.threads);torch.manual_seed(args.seed);rng=np.random.default_rng(args.seed)
    started=time.monotonic();logpath=args.out/'training.jsonl'
    def log(row):
        row=dict(seconds=round(time.monotonic()-started,2),**row)
        with logpath.open('a') as f:f.write(json.dumps(row)+'\n')
        print(json.dumps(row),flush=True)
    if logpath.exists() and not args.resume:p.error('Output already contains a run; use a new --out directory or --resume')
    config={k:str(v) if isinstance(v,Path) else [str(x) for x in v] if isinstance(v,list) else v for k,v in vars(args).items()}
    (args.out/'config.json').write_text(json.dumps(config,indent=2)+'\n')
    policy=CreaturePolicy();prior={};offset=0
    if args.resume or args.warmstart:
        policy,prior=load_checkpoint(args.resume or args.warmstart)
        if policy.brain_format==2:
            policy=upgrade(policy);prior.pop('optimizer',None)
    if not (args.resume or args.warmstart):
        initial=evaluate(policy,args.eval_games,**eval_options)
        (args.out/'random-validation.json').write_text(json.dumps(initial,indent=2)+'\n')
        save_checkpoint(args.out/'random.pt',policy,stage='random',steps=0)
        log(dict(stage='random-validation',score=initial['score']))
    if not (args.resume or args.warmstart) or args.refine_imitation:
        data=collect_demonstrations(args.demo_envs,args.demo_steps,args.seed,policy if args.refine_imitation else None,args.personality_rate)
        log(dict(stage='demonstrations',transitions=args.demo_envs*2*args.demo_steps))
        imitate(policy,data,args.bc_epochs,args.sequence,args.minibatch,rng,log)
        del data;gc.collect()
        save_checkpoint(args.out/'imitation.pt',policy,stage='imitation',steps=0,config=config)
    validation=evaluate(policy,args.eval_games,**eval_options)
    (args.out/'imitation-validation.json').write_text(json.dumps(validation,indent=2)+'\n')
    log(dict(stage='warmstart-validation',score=validation['score']))
    optimizer=torch.optim.Adam(policy.parameters(),lr=args.lr,eps=1e-5)
    if args.resume:
        offset=prior.get('steps',0)
        if 'optimizer' in prior:optimizer.load_state_dict(prior['optimizer'])
        for group in optimizer.param_groups:group['lr']=args.lr
    # Resume retains weights/optimizer, but deliberately starts fresh episodes and memory.
    env=ArenaBatch(args.envs,args.seed+offset+1,personality_rate=args.personality_rate,fixed_personality=fixed_personality,target_species=args.species);memory=torch.zeros(args.envs,HIDDEN)
    opponents=FrozenOpponents(args.opponent,args.envs,args.learned_opponent_rate) if args.opponent else None
    best=-1.;recent=[];last=offset
    if (args.resume or args.warmstart) and prior.get('stage')=='ppo':
        best=validation['score']
        save_checkpoint(args.out/'best.pt',policy,optimizer=optimizer.state_dict(),stage='ppo',steps=offset,config=config,validation=validation)
        export_brain(policy,args.out/'best.tbrain',dict(stage='ppo',steps=offset,validation_score=best))
        (args.out/'best-validation.json').write_text(json.dumps(validation,indent=2)+'\n')
    try:
        for update in range(1,args.updates+1):
            data,memory,endings=rollout(policy,env,memory,args.horizon,args.gamma,opponents,args.style_scale)
            stats=ppo_update(policy,optimizer,data,args,rng)
            last=offset+update*args.envs*args.horizon;recent=(recent+endings)[-200:]
            log(dict(stage='ppo',update=update,steps=last,episodes=len(endings),training_score=float(np.mean(recent)) if recent else None,**stats))
            if update%args.eval_every==0 or update==args.updates:
                metrics=evaluate(policy,args.eval_games,**eval_options)
                log(dict(stage='validation',steps=last,score=metrics['score']))
                save_checkpoint(args.out/'latest.pt',policy,optimizer=optimizer.state_dict(),stage='ppo',steps=last,config=config)
                if metrics['score']>best:
                    best=metrics['score'];save_checkpoint(args.out/'best.pt',policy,optimizer=optimizer.state_dict(),stage='ppo',steps=last,config=config,validation=metrics)
                    export_brain(policy,args.out/'best.tbrain',dict(stage='ppo',steps=last,validation_score=best))
                    (args.out/'best-validation.json').write_text(json.dumps(metrics,indent=2)+'\n')
    except KeyboardInterrupt:
        save_checkpoint(args.out/'interrupted.pt',policy,optimizer=optimizer.state_dict(),stage='ppo',steps=last,config=config)
        log(dict(stage='interrupted',steps=last));raise SystemExit(130)
    finally:env.close()
    best_policy,best_data=load_checkpoint(args.out/'best.pt')
    heldout=evaluate(best_policy,args.test_games,seed=1_900_000_000,**eval_options)
    (args.out/'heldout.json').write_text(json.dumps(heldout,indent=2)+'\n')
    log(dict(stage='heldout',selected_steps=best_data['steps'],score=heldout['score'],matches=args.test_games))

if __name__=='__main__':main()
