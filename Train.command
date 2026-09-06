#!/bin/sh
set -eu
cd "$(dirname "$0")"
make core brain
if [ ! -x .venv/bin/python ]; then
  python3 -m venv .venv
fi
if ! .venv/bin/python -c 'import torch, numpy' >/dev/null 2>&1; then
  .venv/bin/python -m pip install -r python/requirements-ml.txt
fi
training_run="runs/train-$(date +%Y%m%d-%H%M%S)"
if [ -f models/apprentice.pt ]; then
  .venv/bin/python python/train.py --resume models/apprentice.pt --out "$training_run" --threads 1
else
  .venv/bin/python python/train.py --out "$training_run" --threads 1
fi
.venv/bin/python python/export_brain.py "$training_run/best.pt" models/apprentice.tbrain --copy-checkpoint
if [ "$(uname -s)" = Darwin ]; then
  sh scripts/package-macos.sh
fi
printf '\nTraining complete. Press F6 in the game to reload the brain, or reopen Launch.command.\n'
