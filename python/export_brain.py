"""Export a compatible PyTorch checkpoint to the dependency-free native controller."""
import argparse
import shutil
from pathlib import Path
from learning import load_checkpoint,export_brain
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('checkpoint',type=Path);p.add_argument('output',type=Path)
p.add_argument('--copy-checkpoint',action='store_true',help='Also retain .pt for continued training')
a=p.parse_args();policy,data=load_checkpoint(a.checkpoint)
meta=export_brain(policy,a.output,{k:data[k] for k in ('stage','steps','config') if k in data})
if a.copy_checkpoint and a.checkpoint.resolve()!=a.output.with_suffix('.pt').resolve():
 shutil.copyfile(a.checkpoint,a.output.with_suffix('.pt'))
print(f'Exported {a.output}: {meta["parameters"]:,} parameters / {meta["bytes"]:,} bytes / {meta["checksum"]}')
