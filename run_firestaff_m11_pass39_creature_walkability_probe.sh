#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/pass39-creature-walkability}
mkdir -p "$OUT_DIR"

PROBE_BIN="$HERE/firestaff_m11_pass39_creature_walkability_probe_bin"
LOG="$OUT_DIR/pass39_creature_walkability_probe.log"

cc -std=c99 -Wall -Wextra -O1 \
    -I "$HERE" \
    -o "$PROBE_BIN" \
    "$HERE/firestaff_m11_pass39_creature_walkability_probe.c" \
    "$HERE/memory_movement_pc34_compat.c" \
    "$HERE/memory_dungeon_dat_pc34_compat.c" \
    "$HERE/memory_champion_state_pc34_compat.c"

"$PROBE_BIN" | tee "$LOG"

SUMMARY=$(grep '^# summary: ' "$LOG" | tail -n 1)
PASSED=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f1)
TOTAL=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f2)
echo "Pass 39 creature walkability probe: $SUMMARY"
if [ -z "$PASSED" ] || [ "$PASSED" != "$TOTAL" ]; then
    exit 1
fi
