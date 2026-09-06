#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
training_mode="${1:-baseline}"
case "$training_mode" in
 baseline) model_name=champion; personality_rate=0 ;;
 spirit) model_name=apprentice; personality_rate=0.55 ;;
 *) printf '%s\n' 'Choose baseline or spirit'; exit 2 ;;
esac
make core brain
if [ ! -x .venv/bin/python ]; then python3 -m venv .venv; fi
if ! .venv/bin/python -c 'import torch, numpy' >/dev/null 2>&1; then
 .venv/bin/python -m pip install -r python/requirements-ml.txt
fi
training_run="runs/$training_mode-$(date +%Y%m%d-%H%M%S)"
.venv/bin/python python/train.py --resume "models/$model_name.pt" --out "$training_run" --threads 1 \
 --personality-rate "$personality_rate" --opponent models/baselines/apprentice-v8.pt --opponent models/champion.pt
.venv/bin/python python/export_brain.py "$training_run/best.pt" "models/$model_name.tbrain" --copy-checkpoint
if [ "$(uname -s)" = Darwin ]; then sh scripts/package-macos.sh; fi
printf '\nTraining complete. F6 reloads both brains. B/V select a pilot; J/K activate a temperament.\n'
