#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/launcher-smoke}
DATA_DIR=${FIRESTAFF_DATA:-${2:-$HOME/.firestaff/data}}
mkdir -p "$OUT_DIR"

# Prefer the CMake-built launcher when ctest invokes this smoke after a build.
# Falling back to the legacy source-tree binary preserves direct script use.
# Only build via the Phase A probe when neither binary exists; that probe is
# intentionally heavier and should not dominate launcher-click smoke runtime.
FIRESTAFF_BIN=${FIRESTAFF_BIN:-}
if [ -z "$FIRESTAFF_BIN" ]; then
    if [ -x "$HERE/build/firestaff" ]; then
        FIRESTAFF_BIN="$HERE/build/firestaff"
    elif [ -x "$HERE/firestaff" ]; then
        FIRESTAFF_BIN="$HERE/firestaff"
    else
        "$HERE/run_firestaff_m11_phase_a_probe.sh" "$OUT_DIR/build" >/dev/null
        FIRESTAFF_BIN="$HERE/firestaff"
    fi
fi

LOG="$OUT_DIR/launcher_smoke.log"
SDL_AUDIODRIVER=${SDL_AUDIODRIVER:-dummy} \
SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-dummy} \
    "$FIRESTAFF_BIN" \
    --data-dir "$DATA_DIR" \
    --script enter,sl,sr,tab,space,esc \
    --duration 80 >"$LOG" 2>&1

echo "M11 launcher smoke: PASS"
echo "Log: $LOG"
