"""Freeze controller exports using validation only, before running the release evaluation."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'python'))
from learning import load_checkpoint,export_brain
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('personality_run',type=Path);p.add_argument('winning_run',type=Path)
a=p.parse_args()
personality,data=load_checkpoint(a.personality_run/'best.pt')
winning,win_data=load_checkpoint(a.winning_run/'best.pt')
assert data['validation']['seed_base']==win_data['validation']['seed_base']==900000000
assert data['validation']['matches']==win_data['validation']['matches']==240
chosen='v9' if data['validation']['score']>=win_data['validation']['score'] else 'winning-only'
models={}
for name,policy,checkpoint,path,metadata in [
    ('v9',personality,a.personality_run/'best.pt','models/apprentice.tbrain',data),
    ('winning-only',winning,a.winning_run/'best.pt','models/baselines/winning-only-v9.tbrain',win_data),
    ('baseline',personality if chosen=='v9' else winning,
        (a.personality_run if chosen=='v9' else a.winning_run)/'best.pt','models/champion.tbrain',data if chosen=='v9' else win_data)]:
    manifest=export_brain(policy,root/path,{k:metadata[k] for k in ['stage','steps','config']})
    shutil.copyfile(checkpoint,(root/path).with_suffix('.pt'))
    models[name]=dict(path=path,checksum=manifest['checksum'],sha256=hashlib.sha256((root/path).read_bytes()).hexdigest(),
        validation_score=metadata['validation']['score'],new_ppo_decisions=metadata['steps'])
path='models/baselines/apprentice-v8.tbrain'
models['v8']=dict(path=path,checksum='c79f418b',sha256=hashlib.sha256((root/path).read_bytes()).hexdigest())
selection=dict(method='Highest score on the fixed 240-game validation cohort; ties prefer the temperament model.',
    baseline_source=chosen,models=models,evaluation=dict(seed=2100000000,scenarios=720,seats=2,grid='crossed',selection_uses_test=False))
(root/'reports/rl-v9-selection.json').write_text(json.dumps(selection,indent=2)+'\n')
print(json.dumps(selection,indent=2))
