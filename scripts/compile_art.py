#!/usr/bin/env python3
"""Decode source PNGs to lossless RGBA textures; no runtime image library required.
Requires Pillow only to regenerate assets. No resizing, alpha removal, or repainting.
"""
import struct
from pathlib import Path
from PIL import Image
root = Path(__file__).resolve().parents[1] / 'assets/tinikami'
for name in ('spirits', 'environment', 'environment-v2', 'effects'):
    im = Image.open(root / (name + '.png')).convert('RGBA')
    (root / (name + '.rgba')).write_bytes(b'TINI' + struct.pack('<II', *im.size) + im.tobytes())
    print(name, im.size)

# Record source rectangles from connected alpha islands. This keeps neighboring art
# out of a sprite without editing any source pixels. Tiny detached wisps join the
# nearest body. The generator's nominal grid is a layout hint, not a crop guarantee.
im = Image.open(root / 'spirits.png').convert('RGBA')
w, h = im.size
alpha = im.getchannel('A').tobytes()
seen = bytearray(w * h)
parts = []
for i, value in enumerate(alpha):
    if value < 128 or seen[i]:
        continue
    stack = [i]; seen[i] = 1
    n = 0; left = w; right = 0; top = h; bottom = 0
    while stack:
        q = stack.pop(); y, x = divmod(q, w)
        n += 1; left = min(left, x); right = max(right, x)
        top = min(top, y); bottom = max(bottom, y)
        for p in (q - 1 if x else q, q + 1 if x + 1 < w else q,
                  q - w if y else q, q + w if y + 1 < h else q):
            if not seen[p] and alpha[p] >= 128:
                seen[p] = 1; stack.append(p)
    if n >= 5:
        parts.append((n, [left, top, right + 1, bottom + 1]))
main = sorted((b for n, b in parts if n > 10000),
              key=lambda b: (int((b[1] + b[3]) / 2 / h * 5), b[0]))
assert len(main) == 40
bounds = [b.copy() for b in main]
centers = [((b[0] + b[2]) / 2, (b[1] + b[3]) / 2) for b in main]
for n, b in parts:
    x, y = (b[0] + b[2]) / 2, (b[1] + b[3]) / 2
    index = min(range(40), key=lambda i: (centers[i][0] - x)**2 + (centers[i][1] - y)**2)
    dest = bounds[index]
    dest[0] = min(dest[0], b[0]); dest[1] = min(dest[1], b[1])
    dest[2] = max(dest[2], b[2]); dest[3] = max(dest[3], b[3])
lines = ['// Generated source rectangles. See scripts/compile_art.py.',
         'static constexpr SDL_Rect SpiritRects[40] = {']
for b in bounds:
    lines.append('    {' + ', '.join(map(str, (b[0], b[1], b[2]-b[0], b[3]-b[1]))) + '},')
lines.append('};')
(root.parent.parent / 'client/spirit_rects.hpp').write_text('\n'.join(lines) + '\n')
