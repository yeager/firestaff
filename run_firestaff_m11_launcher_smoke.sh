#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/launcher-smoke}
DATA_DIR=${FIRESTAFF_DATA:-${2:-$HOME/.firestaff/data}}
mkdir -p "$OUT_DIR"

"$HERE/run_firestaff_m11_phase_a_probe.sh" "$OUT_DIR/build" >/dev/null

LOG="$OUT_DIR/launcher_smoke.log"
SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-dummy} \
    "$HERE/firestaff" \
    --data-dir "$DATA_DIR" \
    --script enter,right,enter,esc \
    --duration 80 >"$LOG" 2>&1

echo "M11 launcher smoke: PASS"
echo "Log: $LOG"
