#!/bin/sh
set -eu
cd "$(dirname "$0")"
make viewer
exec ./build/creature_lab --campaign --captures "$PWD/captures"
