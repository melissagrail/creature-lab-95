"""Both skins and all forty atlas entries must render the same seeded world."""
import os
import re
import subprocess
import tempfile
from pathlib import Path
root = Path(__file__).resolve().parents[1]
exe = root / 'build/creature_lab'
with tempfile.TemporaryDirectory(prefix='viewer-check-', dir=root/'build') as tmp:
    hashes = []
    variants = [['--skin', 'debug'], ['--skin', 'tinikami'], ['--skin', 'tinikami', '--hitboxes']]
    variants += [['--catalog', '--catalog-page', str(page)] for page in range(4)]
    for i, args in enumerate(variants):
        capture = Path(tmp) / str(i)
        run = subprocess.run([str(exe), '--frames', '1', '--preview-ticks', '300',
                              '--species-a', '9', '--species-b', '6', '--arena', '0',
                              '--captures', str(capture)] + args,
                             env=dict(os.environ, SDL_VIDEODRIVER='dummy'), text=True,
                             capture_output=True, check=True, cwd=root)
        assert 'ART NOT FOUND' not in run.stderr, run.stderr
        hashes.append(re.search(r'Render state ([0-9a-f]+)', run.stdout).group(1))
        assert (capture/'viewer.bmp').stat().st_size > 1000
    assert len(set(hashes)) == 1, hashes
print('Viewer: both skins, geometry overlay and all 40 sprites preserve state', hashes[0])
