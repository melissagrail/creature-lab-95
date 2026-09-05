#!/usr/bin/env python3
"""Produce reproducible baseline reports. Standard library; optional matplotlib figure."""
import csv,json,math,sys,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
source=Path(sys.argv[1]);prefix=Path(sys.argv[2]);prefix.parent.mkdir(parents=True,exist_ok=True)
data=json.loads((ROOT/'content/roster.json').read_text());names=[s['name'] for s in data['species']]
wins=[0.]*40;games=[0]*40;pair_w=[[0.]*40 for _ in range(40)];pair_n=[[0]*40 for _ in range(40)];reasons={};total=0;ticks=[];seat0=0;overflow=0;weather=[[0.,0] for _ in range(3)];arena=[[0.,0] for _ in range(3)]
for r in csv.DictReader(source.open()):
 a,b,w=int(r['a']),int(r['b']),int(r['winner']);v=.5 if w<0 else float(w==0);total+=1;seat0+=v;ticks.append(int(r['ticks']));overflow+=int(r['overflow']);reason=int(r['reason']);reasons[reason]=reasons.get(reason,0)+1
 for i,j,score in [(a,b,v),(b,a,1-v)]:wins[i]+=score;games[i]+=1;pair_w[i][j]+=score;pair_n[i][j]+=1
 for table,key in [(weather,'weather'),(arena,'arena')]:table[int(r[key])][0]+=v;table[int(r[key])][1]+=1
rates=[wins[i]/games[i] for i in range(40)];matrix=[[pair_w[i][j]/pair_n[i][j] if pair_n[i][j] else .5 for j in range(40)]for i in range(40)]
extremes=sorted([(abs(matrix[i][j]-.5),i,j,matrix[i][j],pair_n[i][j])for i in range(40) for j in range(i+1,40)],reverse=True)
meta_path=Path(str(source)+'.meta.json');meta=json.loads(meta_path.read_text()) if meta_path.exists() else {}
fingerprint=2166136261
for b in (ROOT/'content/roster.json').read_bytes():fingerprint=((fingerprint^b)*16777619)&0xffffffff
if meta and meta['content_hash']!=fingerprint:raise SystemExit('Report content does not match canonical roster')
summary=dict(matches=total,per_species=games[0],min_rate=min(rates),max_rate=max(rates),seat_a_win_rate=seat0/total,mean_seconds=sum(ticks)/len(ticks)/30,median_seconds=sorted(ticks)[len(ticks)//2]/30,reasons=reasons,overflow=overflow,metadata=meta,source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),engine_sha256=hashlib.sha256((ROOT/'src/sim.cpp').read_bytes()).hexdigest(),species=[dict(id=i,name=names[i],wins=wins[i],games=games[i],rate=rates[i]) for i in range(40)])
prefix.with_suffix('.json').write_text(json.dumps(summary,indent=2)+'\n')
with prefix.with_name(prefix.name+'-matrix.csv').open('w') as f:
 out=csv.writer(f);out.writerow(['species']+names)
 for name,row in zip(names,matrix):out.writerow([name]+[f'{v:.4f}'for v in row])
lines=['# Scripted balance evidence','',f'**{total:,} matches; {games[0]:,} games per species; {pair_n[0][1]} games per unordered matchup.** Every unordered pairing is run in both seats. The 36-seed protocol covers 3 weather states × 3 arenas × 4 style pairings. The species share a kit-aware heuristic; these are not independently trained or optimized policies.','',f'Aggregate species win rates: **{min(rates):.1%}–{max(rates):.1%}**. Side A wins: **{seat0/total:.2%}**. Mean fight: **{summary["mean_seconds"]:.1f}s**; median **{summary["median_seconds"]:.1f}s**. Pool overflows: **{overflow}**.','',f'Endings: KO {reasons.get(1,0):,}, control {reasons.get(2,0):,}, time limit {reasons.get(3,0):,}.','', 'These aggregate rates are an alpha smoke-test gate, not a claim of competitive balance. Scenario outcomes are correlated, so a binomial confidence interval would overstate the strength of this evidence. Fresh-seed holdout varies starting jitter; it does not constitute an independent policy-population holdout. The worst pairings below remain explicit human/RL playtest targets.','', '| Species | Win rate | Games |','|---|---:|---:|']
for i in sorted(range(40),key=lambda i:-rates[i]):lines.append(f'| {names[i]} | {rates[i]:.1%} | {games[i]:,} |')
lines+=['','## Largest matchup asymmetries','','| Species A | Species B | A win rate | Games |','|---|---|---:|---:|']
for _,i,j,v,n in extremes[:20]:lines.append(f'| {names[i]} | {names[j]} | {v:.1%} | {n} |')
lines+=['','The full matrix is in the companion CSV. A 100% result in this deterministic scripted population is a counterexample to assuming every matchup is healthy; it may reflect genuine mechanics, pilot blind spots, or both. Test alternative policies and targeted scenarios before changing a kit solely to flatten this matrix.','', '## Reproduce','',f'`./build/tournament {meta.get("seeds",36)} {source} {meta.get("seed_offset",1000)}`','',f'Content fingerprint: `{fingerprint:08x}`. Rules/observations: v2. The JSON report records source and engine hashes.']
prefix.with_suffix('.md').write_text('\n'.join(lines)+'\n')
try:
 import matplotlib
 matplotlib.use('Agg')
 import matplotlib.pyplot as plt
 import numpy as np
 fig,ax=plt.subplots(figsize=(15,13),dpi=130);im=ax.imshow(np.array(matrix),vmin=0,vmax=1,cmap='RdYlBu');ax.set_xticks(range(40),names,rotation=90,fontsize=7);ax.set_yticks(range(40),names,fontsize=7);ax.set_title(f'Creature Lab alpha — scripted matchup win rate\n{total:,} matches · rows versus columns · not trained-policy balance',pad=16);fig.colorbar(im,ax=ax,label='Row species win rate',fraction=.035,pad=.02);fig.tight_layout();fig.savefig(prefix.with_suffix('.png'));plt.close(fig)
except ImportError:pass
print(json.dumps({k:summary[k]for k in ['matches','min_rate','max_rate','seat_a_win_rate','mean_seconds','overflow']}))
