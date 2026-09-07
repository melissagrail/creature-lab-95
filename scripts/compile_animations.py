#!/usr/bin/env python3
"""Import generated sprite sheets: decode chroma matte, slice cells, normalize foot pivots.
Source artwork is retained verbatim. No poses are synthesized or repainted.
Requires Pillow, NumPy and SciPy for offline import only; runtime uses raw RGBA.
"""
import argparse,json,struct
from pathlib import Path
import numpy as np
from PIL import Image
from scipy.ndimage import binary_propagation, label, find_objects
ROOT=Path(__file__).resolve().parents[1]
ART=ROOT/'assets/tinikami';DEST=ART/'animations'
def decode(path):
 im=Image.open(path).convert('RGBA');a=np.array(im)
 if a[:,:,3].min()==255:
  rgb=a[:,:,:3].astype(np.int16)
  magenta=(rgb[:,:,0]>175)&(rgb[:,:,2]>175)&(rgb[:,:,1]<135)&(rgb[:,:,0]-rgb[:,:,1]>70)&(rgb[:,:,2]-rgb[:,:,1]>70)
  neutral=(rgb.min(2)>215)&((rgb.max(2)-rgb.min(2))<20)
  # Generated sources are either actual alpha, a chroma matte, or an opaque checker preview.
  candidate=magenta if magenta[0].mean()>.5 else neutral
  seed=np.zeros(candidate.shape,bool)
  seed[0,:]=candidate[0,:];seed[-1,:]=candidate[-1,:]
  seed[:,0]=candidate[:,0];seed[:,-1]=candidate[:,-1]
  background=candidate if magenta[0].mean()>.5 else binary_propagation(seed,mask=candidate)
  if path.stem=="journey-biomes-clean":background|=(rgb.min(2)>220)&((rgb.max(2)-rgb.min(2))<4)
  a[background,3]=0
 return Image.fromarray(a)
def raw(im,path):
 path.write_bytes(b'TINI'+struct.pack('<II',*im.size)+im.convert('RGBA').tobytes())
def compile_sheet(source,dest,cell=96,pivot=88,individual=False,columns=8,rows=4):
 im=decode(source)
 if individual:
  frames=[]
  for y in range(rows):
   for x in range(columns):
    part=im.crop((x*im.width//columns,y*im.height//rows,(x+1)*im.width//columns,(y+1)*im.height//rows))
    bbox=part.getchannel('A').getbbox();assert bbox
    frames.append(part.crop(bbox))
 else:
  pixels=np.array(im); labels,count=label(pixels[:,:,3]>0)
  objects=find_objects(labels); sizes=np.bincount(labels.ravel())
  # Identify the eight bodies in each row, then collect their detached effects.
  # Generated grids are not pixel-perfect: crop whole alpha islands instead of
  # cutting on nominal cell boundaries, which can bisect tails and feet.
  bodies=[]
  for row in range(rows):
   candidates=[i for i,obj in enumerate(objects,1) if obj is not None and
               row*im.height/rows <= (obj[0].start+obj[0].stop)/2 < (row+1)*im.height/rows]
   main=sorted(candidates,key=lambda i:sizes[i],reverse=True)[:columns]
   assert len(main)==columns and min(sizes[i] for i in main)>1000, f'Missing authored pose: {source} row {row}'
   bodies.extend(sorted(main,key=lambda i:objects[i-1][1].start+objects[i-1][1].stop))
  groups={i:[i] for i in bodies}
  for i,obj in enumerate(objects,1):
   if i in groups or obj is None or sizes[i]<3:continue
   cy=(obj[0].start+obj[0].stop)/2;cx=(obj[1].start+obj[1].stop)/2
   row=min(rows-1,int(cy*rows/im.height))
   def distance(j):
    box=objects[j-1]
    dx=max(box[1].start-cx,0,cx-box[1].stop)
    dy=max(box[0].start-cy,0,cy-box[0].stop)
    return dx*dx+dy*dy
   nearest=min(bodies[row*columns:(row+1)*columns],key=distance)
   if distance(nearest)<(im.width/10)**2:groups[nearest].append(i)
  frames=[]
  for body in bodies:
   ids=groups[body]; boxes=[objects[i-1] for i in ids]
   top=min(b[0].start for b in boxes);bottom=max(b[0].stop for b in boxes)
   left=min(b[1].start for b in boxes);right=max(b[1].stop for b in boxes)
   crop=pixels[top:bottom,left:right].copy()
   crop[:,:,3][~np.isin(labels[top:bottom,left:right],ids)]=0
   frames.append(Image.fromarray(crop))
 uniform=min((cell-8)/max(f.width for f in frames),(pivot-5)/max(f.height for f in frames))
 result=Image.new('RGBA',(cell*columns,cell*rows))
 for i,f in enumerate(frames):
  scale=min((cell-8)/f.width,(pivot-5)/f.height) if individual else uniform
  resized=f.resize((max(1,round(f.width*scale)),max(1,round(f.height*scale))),Image.Resampling.NEAREST)
  result.alpha_composite(resized,((i%columns)*cell+(cell-resized.width)//2,(i//columns)*cell+pivot-resized.height))
 result.save(dest.with_suffix('.png'));raw(result,dest.with_suffix('.rgba'))
 return dict(source=source.name,atlas=dest.with_suffix('.png').name,frames=32,cell=cell,pivot=[cell//2,pivot],alpha_pixels=int((np.array(result)[:,:,3]==0).sum()))
def main():
 p=argparse.ArgumentParser();p.add_argument('--partial',action='store_true');args=p.parse_args()
 roster=json.loads((ROOT/'content/roster.json').read_text())['species'];manifest=[]
 for s in roster:
  source=DEST/f"{s['id']:02d}-{s['name'].lower()}.png"
  if not source.exists():
   if args.partial:continue
   raise FileNotFoundError(source)
  info=compile_sheet(source,DEST/str(s['id']))
  info.update(id=s['id'],name=s['name']);manifest.append(info)
 if (ART/'journey-props.png').exists():
  compile_sheet(ART/'journey-props.png',ART/'journey-props-atlas',128,118,True)
 if (ART/'journey-biomes.png').exists():
  compile_sheet(ART/('journey-biomes-clean.png' if (ART/'journey-biomes-clean.png').exists() else 'journey-biomes.png'),ART/'journey-biomes-atlas',160,150,True,4,8)
 if (ART/'journey-inn.png').exists():
  raw(Image.open(ART/'journey-inn.png').convert('RGBA'),ART/'journey-inn.rgba')
 if (ART/'journey-atlas.png').exists():
  raw(Image.open(ART/'journey-atlas.png').convert('RGBA'),ART/'journey-atlas.rgba')
 (DEST/'manifest.json').write_text(json.dumps(dict(version=1,orientations=['south','east','north','west'],poses=['idle','walk_a','walk_b','windup','attack','hit','fall','fainted'],species=manifest),indent=2)+'\n')
 print(f'Imported {len(manifest)} species / {len(manifest)*32} authored frames, with alpha and shared pivots.')
if __name__=='__main__':main()
