#!/bin/sh
set -eu
cd "$(dirname "$0")"
make viewer
exec ./build/creature_lab --workbench --captures "$PWD/captures"
