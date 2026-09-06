"""Rebuild the shipped RL report from frozen native match CSVs and the training log."""
import csv,json,hashlib
from pathlib import Path
root=Path(__file__).resolve().parents[1];report=root/'reports'
labels={'random':'Untrained network','imitation':'Imitation only','ppo':'Imitation + PPO','scripted':'Scripted style 0 reference'}
summary={}
for name,label in labels.items():
 path=report/f'rl-v8-{name}.csv';rows=list(csv.DictReader(path.open()))
 summary[name]=dict(label=label,matches=len(rows),score=sum(float(r['score']) for r in rows)/len(rows),
   wins=sum(float(r['score'])==1 for r in rows),draws=sum(float(r['score'])==.5 for r in rows),
   mean_seconds=sum(int(r['ticks']) for r in rows)/len(rows)/30,overflow=sum(int(r['overflow']) for r in rows),
   casts_per_minute=sum(int(r['casts']) for r in rows)*1800/sum(int(r['ticks']) for r in rows),
   source_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
   species={str(s):sum(float(r['score']) for r in rows if int(r['species'])==s)/sum(int(r['species'])==s for r in rows) for s in range(40)})
 for case in range(240):
  pair=[r for r in rows if int(r['case'])==case]
  assert len(pair)==2 and {int(r['seat']) for r in pair}=={0,1}
 assert {int(r['arena']) for r in rows}==set(range(6)) and not summary[name]['overflow']
manifest=json.loads((root/'models/baselines/apprentice-v8.json').read_text())
log=[json.loads(line) for line in (report/'rl-v8-learning.jsonl').read_text().splitlines()]
summary['run']=dict(selected_steps=manifest['metadata']['steps'],total_steps=max(r.get('steps',0) for r in log),
   demonstration_transitions=next(r['transitions'] for r in log if r['stage']=='demonstrations'),
   parameters=manifest['parameters'],native_model_bytes=manifest['bytes'],model_checksum=manifest['checksum'],
   model_sha256=hashlib.sha256((root/'models/baselines/apprentice-v8.tbrain').read_bytes()).hexdigest(),
   native_source_sha256=hashlib.sha256((root/'agents/brain.cpp').read_bytes()).hexdigest(),
   simulation_source_sha256=hashlib.sha256((root/'src/sim.cpp').read_bytes()).hexdigest())
(report/'rl-v8-summary.json').write_text(json.dumps(summary,indent=2)+'\n')
lines=['# First trained Tinikami pilot','',
 'The same 480 native matches compare a deterministic untrained network, an imitation warm start, the selected PPO checkpoint, and a scripted reference. Each of 240 scenarios is played from both seats. All 40 species, six maps, three weather states and four opponent spacing styles are represented. These are scripted-opponent tests, not human or learned-population evidence.','',
 '| Controller | Match score | Wins | Draws | Mean duration | Casts / minute |','|---|---:|---:|---:|---:|---:|']
for name in labels:
 r=summary[name];lines.append(f'| {r["label"]} | {r["score"]:.2%} | {r["wins"]} | {r["draws"]} | {r["mean_seconds"]:.2f}s | {r["casts_per_minute"]:.2f} |')
r=summary['run']
lines+=['','Score is 1 for a win, 0.5 for a draw and 0 for a loss. All four runs had zero pool overflows. Paired scenarios are correlated; these percentages are descriptive, not precise population estimates.','',
 f'Training collected {r["demonstration_transitions"]:,} demonstration transitions and then {r["total_steps"]:,} on-policy decisions. The shipped model was selected at {r["selected_steps"]:,} PPO decisions by its score on the fixed 80-match validation set. That selection set covers all 40 species but only arena IDs 0–3; the separate native test covers all six. The training environment samples all six maps. The current trainer defaults to a larger 240-match validation set.','',
 'The chosen validation score was 42.5%; the different held-out native scenario set scores lower. No further tuning or checkpoint selection used the held-out native results. The bundled Python holdout is another diagnostic of this same selected model; do not add its games to the native set as independent evidence.','',
 'Some species still score zero in these twelve-game-per-species probes. Aim timing, route choice, range management and special-kit execution remain visibly weak. The roster is mechanically imbalanced before learning; a shared short training run does not remedy that. The network has separate recurrent memory per actor, but no per-individual persistent training, live praise-based updates or self-play league is implemented.','',
 '## Reproduce','', '```sh','make core brain build/brain_eval',
 './build/brain_eval models/baselines/random.tbrain reports/rl-v8-random.csv 240 1900000000',
 './build/brain_eval models/baselines/imitation.tbrain reports/rl-v8-imitation.csv 240 1900000000',
 './build/brain_eval models/baselines/apprentice-v8.tbrain reports/rl-v8-ppo.csv 240 1900000000',
 './build/brain_eval scripted reports/rl-v8-scripted.csv 240 1900000000',
 'python3 scripts/analyze_learning.py','```','',
 'Raw CSVs, the selected checkpoint, both baseline checkpoints, training configuration and the complete accepted-run log are included. The JSON report records hashes for the model, native controller, simulation and each input CSV. The native controller is the one used by the SDL game.','',
 f'Brain format 2; rules/observations 7; content `223a8716`; {r["parameters"]:,} parameters; {r["native_model_bytes"]:,} bytes; model checksum `{r["model_checksum"]}`.']
(report/'rl-v8-summary.md').write_text('\n'.join(lines)+'\n')
try:
 import matplotlib
 matplotlib.use('Agg')
 import matplotlib.pyplot as plt
 fig,axes=plt.subplots(1,2,figsize=(11,4.1),dpi=150)
 val=[(0,next(x['score'] for x in log if x['stage']=='warmstart-validation'))]+[(x['steps'],x['score']) for x in log if x['stage']=='validation']
 axes[0].plot([x[0]/1e6 for x in val],[100*x[1] for x in val],color='#317b76',marker='o',lw=1.5)
 axes[0].axvline(r['selected_steps']/1e6,color='#aa714b',ls='--',lw=1,label='Selected checkpoint')
 axes[0].set(xlabel='PPO decisions (millions)',ylabel='Match score (%)',title='Validation during training',ylim=(0,60));axes[0].legend(frameon=False)
 axes[1].bar(['Untrained','Imitation','PPO','Scripted'],[100*summary[k]['score'] for k in labels],color=['#a9afa9','#8baa9a','#317b76','#bca57f'])
 axes[1].set(ylabel='Match score (%)',title='Held-out native evaluation · 480 games each',ylim=(0,60))
 for ax in axes:
  ax.spines[['top','right']].set_visible(False);ax.grid(axis='y',alpha=.16);ax.set_axisbelow(True)
 fig.suptitle('Tinikami: first trained generalist',fontweight='bold');fig.tight_layout();fig.savefig(report/'rl-v8-learning.png');plt.close(fig)
except ImportError:pass
print(json.dumps({k:dict(score=summary[k]['score'],matches=summary[k]['matches']) for k in labels}))
