"""Summarize frozen, paired native evaluations of the v9 controller and temperaments."""
from pathlib import Path
import csv
import hashlib
import json
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
REPORT = ROOT / 'reports'
NAMES = ['STEADY','AGGRESSIVE','SKITTISH','PATIENT','TERRITORIAL']
roster = json.loads((ROOT/'content/roster.json').read_text())['species']

def read(name):
    return list(csv.DictReader((REPORT/f'rl-v9-{name}.csv').open()))

def summarize(rows):
    n = sum(int(r['decisions']) for r in rows)
    ticks = sum(int(r['ticks']) for r in rows)
    result = dict(matches=len(rows), score=sum(float(r['score']) for r in rows)/len(rows),
        wins=sum(float(r['score'])==1 for r in rows), draws=sum(float(r['score'])==.5 for r in rows),
        seconds=ticks/len(rows)/30, overflow=sum(int(r['overflow']) for r in rows),
        casts_per_minute=sum(int(r['casts']) for r in rows)*1800/ticks,
        damage_per_energy=sum(int(r['damage']) for r in rows)/max(1,sum(int(r['energy_spent']) for r in rows)/10))
    for key in ['distance','objective_distance','mean_energy','stationary_fraction','retreat_fraction']:
        result[key] = sum(float(r[key])*int(r['decisions']) for r in rows)/n
    return result

def check_grid(rows):
    assert len(rows)==1440 and not sum(int(r['overflow']) for r in rows)
    for species in range(40):
        selected=[r for r in rows if int(r['species'])==species]
        assert {(int(r['arena']),int(r['weather']),int(r['seat'])) for r in selected} == {
            (a,w,p) for a in range(6) for w in range(3) for p in range(2)}
    return rows

selection=json.loads((REPORT/'rl-v9-selection.json').read_text())
for name, info in selection['models'].items():
    path=ROOT/info['path']
    assert hashlib.sha256(path.read_bytes()).hexdigest()==info['sha256'], 'Model changed since evaluation freeze: '+str(path)
profiles=read('personalities')
by_profile={name:check_grid([r for r in profiles if r['personality']==name]) for name in NAMES}
controllers={name:check_grid(read(name)) for name in ['v8','winning-only','scripted']}
controllers['v9']=by_profile['STEADY']
for name, rows in controllers.items():
    expected=selection['models'][name]['checksum'] if name!='scripted' else '0'
    assert {r['model_checksum'] for r in rows}=={expected.lstrip('0') or '0'}, 'Evaluation/model mismatch: '+name
assert {r['model_checksum'] for r in profiles}=={selection['models']['v9']['checksum'].lstrip('0')}

summary=dict(controllers={name:summarize(rows) for name,rows in controllers.items()},
             personalities={name:summarize(rows) for name,rows in by_profile.items()})
trace=read('trace')
head_to_head=check_grid(read('head-to-head'))
assert {r['model_checksum'] for r in head_to_head}=={selection['models']['baseline']['checksum']}
assert {r['opponent_checksum'] for r in head_to_head}=={selection['models']['v8']['checksum']}
summary['head_to_head_vs_v8']=summarize(head_to_head)
summary['same_observation_probe']={}
for name in NAMES:
    rows=[r for r in trace if r['personality']==name]
    n=sum(int(r['decisions']) for r in rows)
    summary['same_observation_probe'][name]=dict(decisions=n,
        changed_from_steady=sum(int(r['changed_from_steady']) for r in rows)/n,
        approach_request=sum(float(r['approach_request'])*int(r['decisions']) for r in rows)/n,
        casts_when_selectable=sum(int(r['cast_requests']) for r in rows)/sum(int(r['selectable_decisions']) for r in rows))

species_rows=[]
for species in roster:
    i=species['id'];rows=[r for r in by_profile['STEADY'] if int(r['species'])==i]
    row=dict(id=i,name=species['name'],role=species['role'],**summarize(rows))
    row['v8_score']=summarize([r for r in controllers['v8'] if int(r['species'])==i])['score']
    slots=[sum(int(r[f'casts_{j}']) for r in rows) for j in range(1,6)]
    dominant=int(np.argmax(slots[:4]))
    row['favorite_art']=species['moves'][dominant]['name'] if sum(slots[:4]) else 'None'
    for j,count in enumerate(slots):row[f'slot_{j+1}_share']=count/max(1,sum(slots))
    for name in ['AGGRESSIVE','SKITTISH','PATIENT','TERRITORIAL']:
        metrics=summarize([r for r in by_profile[name] if int(r['species'])==i])
        for field in ['score','distance','mean_energy','objective_distance']:
            row[name.lower()+'_'+field]=metrics[field]
    species_rows.append(row)
with (REPORT/'rl-v9-species.csv').open('w') as f:
    writer=csv.DictWriter(f,fieldnames=list(species_rows[0]),lineterminator='\n');writer.writeheader();writer.writerows(species_rows)
summary['species']=species_rows
summary['selection']=selection
summary['provenance']={}
for path in [ROOT/'models/apprentice.tbrain',ROOT/'models/champion.tbrain',ROOT/'models/baselines/apprentice-v8.tbrain',
             ROOT/'models/baselines/winning-only-v9.tbrain',ROOT/'agents/brain.cpp',ROOT/'tests/brain_eval.cpp',
             ROOT/'tests/brain_probe.cpp',ROOT/'src/sim.cpp'] + sorted(REPORT.glob('rl-v9-*.csv')):
    summary['provenance'][str(path.relative_to(ROOT))]=hashlib.sha256(path.read_bytes()).hexdigest()
(REPORT/'rl-v9-summary.json').write_text(json.dumps(summary,indent=2)+'\n')
labels={'v8':'Previous v8 apprentice','winning-only':'Winning-only continuation','v9':'V9 steady temperament','scripted':'Scripted reference'}
lines=['# Tinikami: stronger pilots and temperament','',
    'Each controller gets the same 1,440 native games: 720 scenarios played from both seats, with every species on all six maps in all three weather states. Opponents cover 18 pairings per species and four scripted styles. Score awards draws half credit. The selected weights were frozen before this fresh test.','',
    '| Controller | Match score | Mean fight | Casts / minute |','|---|---:|---:|---:|']
for name in ['v8','winning-only','v9','scripted']:
    r=summary['controllers'][name];lines.append(f'| {labels[name]} | {r["score"]:.2%} | {r["seconds"]:.1f}s | {r["casts_per_minute"]:.1f} |')
lines+=['',f'The default baseline was selected from the temperament trial by validation before this test. Its neutral weights are initially identical to the spirit model. The winning-only comparison scored higher on this fresh test; it was retained as a comparison, without using test scores to reselect the default. Against the frozen v8 learned opponent, the selected baseline scored {summary["head_to_head_vs_v8"]["score"]:.2%} in another 1,440 paired games. That opponent appeared during training, so this is progress against a known controller, not unseen-opponent evidence.']
lines+=['','## Measured temperament' ,'',
    'Preferences change policy decisions; they do not change stats, cooldowns, energy rules or collision. Distance and energy below are weighted by observed decision time. Different policies induce different fights, so the same-observation probe below is also provided.','',
    '| Temperament | Score | Enemy distance | Objective distance | Energy reserve | Casts / minute |',
    '|---|---:|---:|---:|---:|---:|']
for name in NAMES:
    r=summary['personalities'][name];lines.append(f'| {name.title()} | {r["score"]:.2%} | {r["distance"]:.2f} | {r["objective_distance"]:.2f} | {r["mean_energy"]:.1%} | {r["casts_per_minute"]:.1f} |')
lines+=['','Distance is in arena units. Energy reserve is the mean fraction of the shared 100-energy pool. Temperament is a preference tradeoff, not a difficulty setting; preset names describe training intent, and this table records what actually happened.','',
    '## Same observations, different preferences','',
    'All five policies receive the same scripted observation sequences for each species and seat. Each carries its own recurrent memory through that identical sequence. Their actions do not drive the probe world. This isolates response to temperament from changes in the opponent or map trajectory; it is a behavioral diagnostic, not a win-rate evaluation.','',
    '| Temperament | Decisions differing from steady | Toward-enemy request | Cast when selectable |','|---|---:|---:|---:|']
for name in NAMES:
    r=summary['same_observation_probe'][name];lines.append(f'| {name.title()} | {r["changed_from_steady"]:.1%} | {r["approach_request"]:.3f} | {r["casts_when_selectable"]:.1%} |')
lines+=['','## Species differentiation','',
    'Thirty-six paired test games per species are a diagnostic sample, not a balance verdict. Favorite art is the most frequently accepted signature cast, excluding dodge. Full slot distributions and each temperament’s species-level results are in [the CSV](rl-v9-species.csv).','',
    '| Species | V8 score | V9 score | Distance | Stationary | Favorite art |','|---|---:|---:|---:|---:|---|']
for r in species_rows:
    lines.append(f'| {r["name"]} | {r["v8_score"]:.1%} | {r["score"]:.1%} | {r["distance"]:.2f} | {r["stationary_fraction"]:.1%} | {r["favorite_art"]} |')
lines+=['','The two training trials differ in curriculum, seed, training length and preference sampling; their score gap is not a clean causal ablation of personality. The scenarios are paired and correlated. These descriptive scores do not establish human-level play, unseen-opponent robustness or a solved roster. The legacy v8 report used a smaller, differently crossed scenario grid and must not be compared directly to these percentages. Raw evaluation CSVs, training configurations, learning traces, model exports and hashes are preserved.','',
    '![Learning and behavior](rl-v9-overview.png)','', '![Species diagnostic](rl-v9-species.png)']
(REPORT/'rl-v9-summary.md').write_text('\n'.join(lines)+'\n')

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
fig,axes=plt.subplots(1,3,figsize=(14,4),dpi=150)
for run,label,color in [('personality','With temperament','#347e78'),('winning','Winning only','#a88753')]:
    data=[json.loads(x) for x in (REPORT/f'rl-v9-{run}-learning.jsonl').read_text().splitlines()]
    points=[(x.get('steps',0)/1e6,100*x['score']) for x in data if x['stage'] in ['warmstart-validation','validation']]
    axes[0].plot(*zip(*points),label=label,color=color,linewidth=1.5)
axes[0].set(title='Checkpoint selection · validation',xlabel='New PPO decisions (millions)',ylabel='Match score (%)');axes[0].legend(frameon=False,fontsize=8)
axes[1].bar(['V8','Win only','V9 steady','Scripted'],[100*summary['controllers'][k]['score'] for k in ['v8','winning-only','v9','scripted']],color=['#aab2ab','#af986d','#347e78','#6c7790'])
axes[1].set(title='Fresh native test · 1,440 each',ylabel='Match score (%)')
axes[2].bar([n.title() for n in NAMES],[100*summary['personalities'][n]['mean_energy'] for n in NAMES],color=['#aab2ab','#c18a6e','#91afae','#af986d','#347e78'])
axes[2].set(title='Shared energy kept in reserve',ylabel='Mean energy (%)');axes[2].tick_params(axis='x',labelrotation=22)
for ax in axes:ax.spines[['top','right']].set_visible(False);ax.grid(axis='y',alpha=.15);ax.set_axisbelow(True)
fig.tight_layout();fig.savefig(REPORT/'rl-v9-overview.png');plt.close(fig)
fig,axes=plt.subplots(1,3,figsize=(12,12),dpi=150,sharey=True)
y=np.arange(40)
axes[0].barh(y,[100*r['v8_score'] for r in species_rows],color='#ccd4ce',label='V8')
axes[0].scatter([100*r['score'] for r in species_rows],y,s=17,color='#347e78',label='V9');axes[0].set(xlabel='Match score (%)',title='Species outcomes',xlim=(0,100));axes[0].legend(frameon=False)
axes[0].set_yticks(y,[r['name'] for r in species_rows],fontsize=8);axes[0].invert_yaxis()
for idx,(field,title) in enumerate([('distance','Spacing · arena units'),('stationary_fraction','Time nearly stationary')],1):
    axes[idx].barh(y,[r[field]*(100 if idx==2 else 1) for r in species_rows],color='#82aaa1');axes[idx].set(title=title,xlabel='Percent' if idx==2 else 'Units')
for ax in axes:ax.spines[['top','right']].set_visible(False);ax.grid(axis='x',alpha=.15);ax.set_axisbelow(True)
fig.tight_layout();fig.savefig(REPORT/'rl-v9-species.png');plt.close(fig)
print(json.dumps({k:round(v['score'],4) for k,v in summary['controllers'].items()}))
