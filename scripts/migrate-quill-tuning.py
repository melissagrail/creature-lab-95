#!/usr/bin/env python3
"""Explicit content-only warm start for the slower Quillrat burst; no new training."""
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
old = data['identity']
new = identity(old['format'])
assert old['content'] == 0x223a8716 and new['content'] == 0x55a82aa5
assert {k:v for k,v in old.items() if k != 'content'} == {k:v for k,v in new.items() if k != 'content'}
policy = CreaturePolicy() if old['format'] == 3 else LegacyPolicy()
policy.load_state_dict(data['policy'], strict=True)
provenance = dict(source=str(source), source_identity=old,
                 migration='Weights unchanged. Explicit warm start after Quillrat burst tuning; no training on keeper calls.')
save_checkpoint(target.with_suffix('.pt'), policy, metadata=provenance)
export_brain(policy, target.with_suffix('.tbrain'), provenance)
print('Content warm start:', target)
