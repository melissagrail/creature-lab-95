#!/usr/bin/env python3
"""Explicit rules8/obs7 -> rules9/obs8 expansion; eight new inputs start at zero weight.
Full-development duels preserve the learned function; campaign restrictions need evaluation.
"""
import sys
from pathlib import Path
root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'python'))
import torch
from policy import CreaturePolicy
from legacy_policy import LegacyPolicy
from learning import identity,save_checkpoint,export_brain
source=Path(sys.argv[1]).resolve(); target=Path(sys.argv[2])
data=torch.load(source,map_location='cpu',weights_only=True)
old=data['identity']; new=identity(old['format'])
assert old['rules']==8 and old['observations']==7 and old['observation_size']==3620
assert new['rules']==9 and new['observations']==8 and new['observation_size']==3628
assert {k:v for k,v in old.items() if k not in ('rules','observations','observation_size')} == {k:v for k,v in new.items() if k not in ('rules','observations','observation_size')}
p=CreaturePolicy() if old['format']==3 else LegacyPolicy()
weights=data['policy']; expanded=p.state_dict()['encoder.0.weight'].zero_()
# Encoder concatenation: self66 + opponent43 + entity32 + moves24 + history24 + global32.
cut=221
expanded[:,:cut]=weights['encoder.0.weight'][:,:cut]
expanded[:,cut+8:]=weights['encoder.0.weight'][:,cut:]
weights['encoder.0.weight']=expanded
p.load_state_dict(weights,strict=True)
provenance=dict(source=str(source.relative_to(root)),source_identity=old,
 migration='Eight zero-initialized development inputs; old weights retained. Campaign curriculum not retrained.')
save_checkpoint(target.with_suffix('.pt'),p,metadata=provenance)
export_brain(p,target.with_suffix('.tbrain'),provenance)
print('Development warm start exported:',target)
