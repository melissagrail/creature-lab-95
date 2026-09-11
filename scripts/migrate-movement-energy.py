#!/usr/bin/env python3
"""Explicit rules-9 -> rules-10 warm start; unchanged tensors, no training claimed."""
from pathlib import Path
import sys
root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / 'python'))
import torch
from policy import CreaturePolicy
from legacy_policy import LegacyPolicy
from learning import identity, save_checkpoint, export_brain
source, target = Path(sys.argv[1]), Path(sys.argv[2])
data = torch.load(source, map_location='cpu', weights_only=True)
old, new = data['identity'], identity(data['identity']['format'])
assert old['rules'] == 9 and new['rules'] == 10
assert old['content'] == 0x55a82aa5 and new['content'] == 0x8a2d462d
assert {k:v for k,v in old.items() if k not in ('rules','content')} == {k:v for k,v in new.items() if k not in ('rules','content')}
policy = CreaturePolicy() if old['format'] == 3 else LegacyPolicy()
policy.load_state_dict(data['policy'], strict=True)
provenance = dict(source=str(source), source_identity=old,
    migration='Rules 10 movement/ability resource separation. Weights unchanged; explicit warm start, not retrained.')
save_checkpoint(target.with_suffix('.pt'), policy, metadata=provenance)
export_brain(policy, target.with_suffix('.tbrain'), provenance)
print('Movement warm start:', target)
