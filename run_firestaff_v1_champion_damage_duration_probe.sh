#!/bin/sh
set -eu
HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
python3 "$HERE/tools/verify_v1_champion_damage_duration_redmcsb_gate.py"
