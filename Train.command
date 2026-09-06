#!/bin/sh
set -eu
cd "$(dirname "$0")"
exec sh scripts/train-model.sh baseline
