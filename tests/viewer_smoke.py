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
    for arena in range(6):
        for a,b in [(a,b) for a in ['scripted','learned','baseline'] for b in ['scripted','learned','baseline']]:
            hashes=[]
            for skin in ('debug','tinikami'):
                capture=Path(tmp)/f'pilots-{arena}-{a}-{b}-{skin}'
                result=subprocess.run([str(exe),'--arena',str(arena),'--pilot-a',a,'--pilot-b',b,
                    '--skin',skin,'--frames','1','--preview-ticks','300','--captures',str(capture)],
                    env=dict(os.environ,SDL_VIDEODRIVER='dummy'),text=True,capture_output=True,check=True,cwd=root)
                hashes.append(re.search(r'Render state ([0-9a-f]+)',result.stdout).group(1))
            assert hashes[0]==hashes[1], 'Pilot configuration depends on skin'
    for personality in ['steady','aggressive','skittish','patient','territorial','0.25,0.8,-0.2']:
        hashes=[]
        for skin in ['debug','tinikami']:
            result=subprocess.run([str(exe),'--personality-a',personality,'--personality-b','skittish',
                '--pilot-b','learned','--skin',skin,'--frames','1','--preview-ticks','300','--captures',str(Path(tmp)/'personality')],
                env=dict(os.environ,SDL_VIDEODRIVER='dummy'),text=True,capture_output=True,check=True,cwd=root)
            hashes.append(re.search(r'Render state ([0-9a-f]+)',result.stdout).group(1))
        assert hashes[0]==hashes[1], 'Temperament depends on skin'
    # Seeded identity must match the explicit vector without importing Torch.
    reference='.8779296875,.509765625,-.3759765625'
    hashes=[]
    for args in [['--personality-seed-a','42'],['--traits-a',reference]]:
        result=subprocess.run([str(exe),*args,'--frames','1','--preview-ticks','300','--captures',str(Path(tmp)/'seed')],
            env=dict(os.environ,SDL_VIDEODRIVER='dummy'),text=True,capture_output=True,check=True,cwd=root)
        hashes.append(re.search(r'Render state ([0-9a-f]+)',result.stdout).group(1))
    assert hashes[0]==hashes[1], 'Seeded individual differs from its explicit trait vector'
    for value in ['2,0,0','0,0,nan','0,0','unrecognized']:
        result=subprocess.run([str(exe),'--personality-a',value,'--frames','1'],
            env=dict(os.environ,SDL_VIDEODRIVER='dummy'),text=True,capture_output=True,cwd=root)
        assert result.returncode==2 and 'Personality must be' in result.stderr
    invalid=subprocess.run([str(exe),'--brain',str(Path(tmp)/'missing.tbrain'),'--pilot-a','learned','--frames','1'],
        env=dict(os.environ,SDL_VIDEODRIVER='dummy'),text=True,capture_output=True,cwd=root)
    assert invalid.returncode==2 and 'Cannot enable learned pilot' in invalid.stderr
print('Viewer: six arenas, both skins, every pilot configuration, all temperament presets and custom traits, geometry, reproducible tiles and 40 sprites preserve state; missing explicit brain fails closed')
