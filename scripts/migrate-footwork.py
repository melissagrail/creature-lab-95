#!/usr/bin/env python3
"""Explicit rules-7 -> rules-8 warm start. Same tensors; changed physics needs new evaluation."""
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
old=data['identity'];new=identity(old['format'])
assert old['rules']==7 and new['rules']==8
assert {k:v for k,v in old.items() if k!='rules'}=={k:v for k,v in new.items() if k!='rules'}
p=CreaturePolicy() if old['format']==3 else LegacyPolicy()
p.load_state_dict(data['policy'],strict=True)
provenance=dict(source=str(source.relative_to(root)),source_identity=old,
                migration='Explicit footwork warm start; unchanged observations and weights. Not retrained yet.')
save_checkpoint(target.with_suffix('.pt'),p,metadata=provenance)
export_brain(p,target.with_suffix('.tbrain'),provenance)
print('Warm start exported:',target)
