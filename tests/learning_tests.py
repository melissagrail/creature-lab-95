"""Learning math, recurrent sequence alignment, native export parity and corrupt-file rejection."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'python'))
import tempfile
import struct
import numpy as np
import torch
from creature import Batch, SLICES
from learning import (CreaturePolicy,HIDDEN,export_brain,distribution,world_actions,local_teacher,
    rewards,potential,gae,fnv,ArenaBatch,save_checkpoint,load_checkpoint,personality_reward)
from native_brain import NativeBrain
from policy import selected_motion, PRESETS, upgrade, personality_from_seed
from train import rollout,chunks

def run():
    torch.set_num_threads(1);torch.manual_seed(9)
    assert personality_from_seed(42)==(.8779296875,.509765625,-.3759765625)
    advantage,ret=gae(np.array([[1.],[2.],[3.]],np.float32),np.array([[.2],[.4],[.8]],np.float32),
                      np.array([[0.],[1.],[0.]],np.float32),np.array([99.],np.float32),gamma=.9,lam=1.)
    np.testing.assert_allclose(ret[:,0],[2.8,2,92.1],rtol=1e-5)
    # Exact discounted telescoping, including the native time-limit verdict as a terminal.
    obs=np.zeros((3,3628),np.float32);obs[:,0]=[1,.8,.7];obs[:,66+11]=[1,.9,.2]
    gamma=.9
    r0=rewards(obs[:1],obs[1:2],np.array([False]),np.array([-1]),np.array([0]),gamma)
    r1=rewards(obs[1:2],obs[2:3],np.array([True]),np.array([0]),np.array([0]),gamma)
    np.testing.assert_allclose(r0+gamma*r1,-potential(obs[:1])+gamma*3,rtol=1e-6)
    # Personality creates explicit bounded preference differences; neutral preserves task reward.
    traits=np.array([[1,-.25,0],[-1,.4,0],[0,1,.25]],np.float32)
    assert np.all(np.abs(personality_reward(obs,traits)) <= .003*np.abs(traits).sum(-1)+1e-8)
    assert np.all(personality_reward(obs,np.zeros_like(traits)) == 0)
    # Reserve preference compares with the current developmental cap, not an unreachable 100.
    reserve_obs=obs[:2].copy();reserve_obs[:,1]=[.3,.5]
    reserve_obs[:,SLICES['global_'].start+34]=[.6,1.]
    np.testing.assert_allclose(personality_reward(reserve_obs,np.array([[0,1,0],[0,1,0]],np.float32)),[0,0],atol=1e-8)
    from legacy_policy import LegacyPolicy
    legacy=LegacyPolicy();modern=upgrade(legacy)
    with Batch(1) as upgrade_env, torch.no_grad():
        public=torch.tensor([list(upgrade_env.observe())]);memory=torch.zeros(1,HIDDEN)
        old=legacy(public,memory);new=modern(public,memory)
        for x,y in zip((old[0],*old[1:]),(new[0][:,0],*new[1:])):
            torch.testing.assert_close(x,y,rtol=3e-6,atol=3e-6)
    with_species=ArenaBatch(4,17,target_species=2,fixed_personality=personality_from_seed(42))
    try:
        np.testing.assert_allclose(with_species.learner_obs()[:,22],2/39)
        np.testing.assert_allclose(with_species.learner_personality(),np.tile(personality_from_seed(42),(4,1)))
        with_species.reset(0)
        np.testing.assert_allclose(with_species.learner_obs()[0,22],2/39)
    finally:with_species.close()
    curriculum=ArenaBatch(32,51,development_rate=1.)
    try:
        development=curriculum.obs[:,:,SLICES['global_'].start+32:SLICES['global_'].start+40]
        assert np.any(development[:,:,1]<1.) and np.all(development[:,:,2]>=.6)
        with torch.no_grad():
            p=CreaturePolicy();o=torch.from_numpy(curriculum.learner_obs())
            _, logits, _, _=p(o,torch.zeros(32,HIDDEN))
            assert torch.all(logits[o[:,SLICES['mask']]<.5]<-1e8)
        try:curriculum.batch.development(0,0,capacity=499);raise AssertionError('Bad capacity accepted')
        except ValueError:pass
        curriculum.step(curriculum.teacher())
        try:curriculum.batch.development(0,0);raise AssertionError('Midmatch change accepted')
        except ValueError:pass
    finally:curriculum.close()
    policy=load_checkpoint(sys.argv[1])[0] if len(sys.argv)>1 else CreaturePolicy();env=ArenaBatch(4,77,personality_rate=1. if policy.brain_format==3 else 0.)
    try:
        env.styles[:]=0
        np.testing.assert_array_equal(env.teacher().reshape(-1),list(env.batch.scripted_actions()))
        old_hashes=[env.batch.hash(i) for i in range(4)]
        env.styles[0,0]=4
        try:env.teacher();raise AssertionError('Invalid teacher style accepted')
        except RuntimeError:pass
        assert old_hashes==[env.batch.hash(i) for i in range(4)]
        env.styles[0,0]=0
        data,_,_=rollout(policy,env,torch.zeros(4,HIDDEN),960,.997)
        assert data['reset'][1:].any(), 'No episode boundary exercised'
        assert (data['memory'][data['reset']>.5]==0).all(), 'Memory leaked between episodes'
        selected=set(range(8))
        for t,i in np.argwhere(data['reset']>.5):selected.add(int(t//16*4+i))
        b=chunks(data,np.array(sorted(selected)),16);h=b['memory'][0]
        with torch.no_grad():
            for t in range(16):
                h=h*(1-b['reset'][t,:,None])
                _,_,_,lp,v,h,_=distribution(policy,b['obs'][t],h,b['raw'][t],b['ability'][t].long(),personality=b['personality'][t])
                torch.testing.assert_close(lp,b['logp'][t],atol=2e-5,rtol=2e-5)
                torch.testing.assert_close(v,b['value'][t],atol=2e-5,rtol=2e-5)
    finally:env.close()
    with tempfile.TemporaryDirectory() as temp:
        path=Path(temp)/'parity.tbrain';manifest=export_brain(policy,path)
        save_checkpoint(Path(temp)/'test.pt',policy,stage='test')
        restored,_=load_checkpoint(Path(temp)/'test.pt')
        for p,q in zip(policy.parameters(),restored.parameters()):torch.testing.assert_close(p,q,atol=0,rtol=0)
        worst=0.;action_delta=0;decisions=0
        with NativeBrain(path) as native,Batch(1) as batch:
            for species in range(40):
                player=species%2
                pair=(species,(species+17)%40) if player==0 else ((species+17)%40,species)
                batch.reset(0,100+species,species%3,pair,species%6)
                if species%3==0:
                    batch.development(0,player,1,65,600,60)
                    batch.development(0,1-player,19,75,700,70)
                h=torch.zeros(1,HIDDEN)
                for t in range(12):
                    o=np.asarray(batch.observe(player=player),np.float32)
                    traits=list(PRESETS.values())[(species+t)%len(PRESETS)] if policy.brain_format==3 else (0.,0.,0.)
                    with torch.no_grad():
                        mean,logits,value,next_h=policy(torch.from_numpy(o)[None],h,torch.tensor([traits]))
                        mean=selected_motion(mean,logits.argmax(-1))
                    expected=torch.cat((mean,logits,value[:,None],next_h),-1).numpy()[0]
                    actual=np.array(native.forward(o,h.numpy()[0],traits),np.float32)
                    np.testing.assert_allclose(actual,expected,rtol=3e-5,atol=3e-5)
                    worst=max(worst,float(np.max(np.abs(actual-expected))))
                    actions,nh=native.action(o,h.numpy()[0],traits)
                    target=world_actions(mean.tanh().numpy(),logits.argmax(-1).numpy(),o[None])[0]
                    np.testing.assert_allclose(actions[:4],target[:4],atol=1,rtol=0)
                    assert actions[4]==target[4] and o[SLICES['mask']][actions[4]]==1
                    action_delta=max(action_delta,int(np.max(np.abs(np.array(actions)-target))))
                    teacher=list(batch.scripted_actions());teacher[5*player:5*player+5]=actions;batch.step(teacher)
                    h=next_h;decisions+=1
            for traits in [(0,0,float('nan')), (0,0,1.01)]:
                try:native.forward(o,np.zeros(96),traits);raise AssertionError('Invalid personality accepted')
                except ValueError:pass
            bad=o.copy();bad[0]=np.nan
            try:native.forward(bad,np.zeros(96));raise AssertionError('NaN observation accepted')
            except ValueError:pass
            try:native.forward(o,np.full(96,2.));raise AssertionError('Unbounded recurrent state accepted')
            except ValueError:pass
        original=path.read_bytes()
        variants=[original[:-1],original+b'x']
        for offset in [0,8,12,16,20,24,28,32,36,len(original)-1]:
            bad=bytearray(original);bad[offset]^=1;variants.append(bytes(bad))
        # Recompute checksum so invalid NaN payload exercises weight validation independently.
        bad=bytearray(original);bad[36:40]=struct.pack('<f',float('nan'));bad[-4:]=struct.pack('<I',fnv(bad[:-4]));variants.append(bytes(bad))
        for bad in variants:
            path.write_bytes(bad)
            try:NativeBrain(path);raise AssertionError('Corrupt/incompatible model accepted')
            except ValueError:pass
        print(f'Learning: GAE terminals/bootstrap, potential telescoping, recurrent PPO likelihoods, checkpoint roundtrip; native parity {decisions} decisions / 40 species; max float error {worst:.2g}, max quantized delta {action_delta}; malformed models rejected')

if __name__=='__main__':run()
