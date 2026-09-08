#!/usr/bin/env python3
"""Register authored walking frames by the head, not the changing cloak silhouette.

Only crops, transparent speck cleanup, uniform nearest-neighbor scaling and translation.
No drawing or synthesized poses. Runtime gait uses the first three columns.
"""
from pathlib import Path
import numpy as np
from PIL import Image
from scipy.ndimage import label
from compile_animations import decode, raw

art = Path(__file__).resolve().parents[1] / 'assets/tinikami'
im = decode(art / 'wayfarer-source.png')
frames = []
for row in range(4):
    for col in range(4):
        part = np.array(im.crop((col*im.width//4, row*im.height//4,
                                (col+1)*im.width//4, (row+1)*im.height//4)))
        components, _ = label(part[:, :, 3] > 32)
        sizes = np.bincount(components.ravel())
        keep = sizes >= 40
        keep[0] = False
        part[:, :, 3][~keep[components]] = 0
        f = Image.fromarray(part)
        frames.append(f.crop(f.getbbox()))

scale = min(80/max(f.height for f in frames), 80/max(f.width for f in frames))
atlas = Image.new('RGBA', (384, 384))
for i, frame in enumerate(frames):
    f = frame.resize((round(frame.width*scale), round(frame.height*scale)), Image.Resampling.NEAREST)
    a = np.array(f)
    # Hair and face are stable; staff, satchel, scarf and alternating feet are not.
    ys, xs = np.where(a[:max(1, f.height//4), :, 3] > 64)
    centre = int(round(float(np.median(xs))))
    x, y = 48-centre, 8
    assert x >= 2 and x+f.width <= 94 and y+f.height <= 90
    atlas.alpha_composite(f, ((i%4)*96+x, (i//4)*96+y))
atlas.save(art / 'wayfarer-atlas.png')
raw(atlas, art / 'wayfarer-atlas.rgba')
print('Registered 16 wayfarer frames; runtime uses 12 poses / 4 orientations.')
