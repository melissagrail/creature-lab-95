"""Presentation must preserve simulation state across every arena and all 40 sprites."""
import os
import re
import subprocess
import tempfile
from pathlib import Path
root = Path(__file__).resolve().parents[1]
exe = root / 'build/creature_lab'
with tempfile.TemporaryDirectory(prefix='viewer-check-', dir=root/'build') as tmp:
    states = []
    for arena in range(6):
        hashes = []
        variants = [['--skin', 'debug'], ['--skin', 'tinikami'],
                    ['--skin', 'tinikami', '--hitboxes'], ['--skin', 'tinikami']]
        if arena == 0:
            variants += [['--catalog', '--catalog-page', str(page)] for page in range(4)]
        images = []
        for i, args in enumerate(variants):
            capture = Path(tmp) / f'{arena}-{i}'
            run = subprocess.run([str(exe), '--frames', '1', '--preview-ticks', '300',
                                  '--species-a', '9', '--species-b', '6', '--arena', str(arena),
                                  '--captures', str(capture)] + args,
                                 env=dict(os.environ, SDL_VIDEODRIVER='dummy'), text=True,
                                 capture_output=True, check=True, cwd=root)
            assert 'ART NOT FOUND' not in run.stderr, run.stderr
            hashes.append(re.search(r'Render state ([0-9a-f]+)', run.stdout).group(1))
            images.append((capture/'viewer.bmp').read_bytes())
            assert len(images[-1]) > 1000
        assert len(set(hashes)) == 1, hashes
        assert images[1] == images[3], 'Seeded garden drawing changed between identical runs'
        assert images[0] != images[1] != images[2], 'Skin/overlay did not change presentation'
        states.append(hashes[0])
    assert len(set(states)) == 6, 'Arena selection did not change simulation'
print('Viewer: six arenas, both skins, geometry, reproducible tiles, all 40 sprites preserve state')
