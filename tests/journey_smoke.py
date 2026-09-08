"""Campaign controller, presentation fixtures and committed animation contract."""
from pathlib import Path
import json,os,struct,subprocess,tempfile
root=Path(__file__).resolve().parents[1];exe=root/'build/creature_lab'
env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
manifest=json.loads((root/'assets/tinikami/animations/manifest.json').read_text())
assert len(manifest['species'])==40
for species in range(40):
    data=(root/f'assets/tinikami/animations/{species}.rgba').read_bytes()
    assert data[:4]==b'TINI' and struct.unpack('<II',data[4:12])==(768,384)
    assert len(data)==12+768*384*4
    for row in range(4):
        frames=[]
        for col in range(8):
            frame=b''.join(data[12+((row*96+y)*768+col*96)*4:12+((row*96+y)*768+col*96+96)*4] for y in range(96))
            assert any(frame[3::4]),f'Empty pose {species}/{row}/{col}'
            assert not any(frame[3:96*4:4]),f'No top padding {species}/{row}/{col}'
            frames.append(frame)
        assert len(set(frames))>=4,f'Insufficient distinct poses {species}/{row}'
with tempfile.TemporaryDirectory(prefix='tinikami-journey-test-') as tmp:
    save=Path(tmp)/'never-written.tini'
    screens=[0,2,5,4,7,11,1,12,13,14,3,8,9,10,6,16,15,8,8,17]
    for scene in range(20):
        capture=Path(tmp)/str(scene)
        result=subprocess.run([str(exe),'--campaign','--journey-scene',str(scene),'--journey-region','4',
            '--frames','2','--save-path',str(save),'--captures',str(capture)],cwd=root,env=env,text=True,capture_output=True,check=True)
        assert 'Campaign render' in result.stdout,result.stderr
        assert f'/ screen {screens[scene]}' in result.stdout,(scene,result.stdout)
        assert (capture/'journey.bmp').stat().st_size>1000
        assert not save.exists(),'Preview modified persistent state'
    result=subprocess.run([str(exe),'--journey-test'],cwd=root,env=env,text=True,capture_output=True,check=True)
    assert 'campaign controller checks passed' in result.stdout,result.stderr
    print(result.stdout.strip())
print('Campaign: 20 rendered fixtures, isolated save behavior, all 1,280 animation cells and native controller flow passed.')
