#!/usr/bin/env python3
"""Bounded balance intervention from a measured baseline; never run automatically in a build.
Records every changed numeric value. Pilot strength is a confound: these are alpha seeds,
not assertions of learned-policy fairness. Canonical JSON is the editable content source.
"""
import csv,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
source=Path(sys.argv[1]); label=sys.argv[2]
games=[0]*40;wins=[0.]*40
for r in csv.DictReader(source.open()):
 a,b,w=int(r['a']),int(r['b']),int(r['winner']);games[a]+=1;games[b]+=1
 if w<0:wins[a]+=.5;wins[b]+=.5
 else:wins[a if w==0 else b]+=1
p=ROOT/'content/roster.json';data=json.loads(p.read_text());changes=[]
def setv(d,key,value,path):
 if d[key]!=value:changes.append({'field':path+'.'+key,'before':d[key],'after':value});d[key]=value
for s in data['species']:
 rate=wins[s['id']]/games[s['id']]
 if .40<=rate<=.60:continue
 factor=max(.88,min(1.15,1+(.5-rate)*.40))
 setv(s,'hp',max(85,min(175,round(s['hp']*factor))),s['name'])
 for m in s['moves']:
  # Never tune shared dodge. Preserve utility timings, range, and unique interactions.
  if m['damage']:setv(m,'damage',max(2,round(m['damage']*factor)),s['name']+'.'+m['name'])
  if rate>.65:
   for key in ['shield','heal']:
    if m[key]:setv(m,key,max(1,round(m[key]*.86)),s['name']+'.'+m['name'])
p.write_text(json.dumps(data,indent=2)+'\n')
(ROOT/'reports'/f'{label}-changes.json').write_text(json.dumps({'source':str(source),'changes':changes},indent=2)+'\n')
print(f'{len(changes)} bounded changes recorded in reports/{label}-changes.json')
