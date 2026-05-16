#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/pass44-party-group-collision}
mkdir -p "$OUT_DIR"

PROBE_BIN="$HERE/firestaff_m11_pass44_party_group_collision_probe_bin"
LOG="$OUT_DIR/pass44_party_group_collision_probe.log"

cc -std=c99 -Wall -Wextra -O1 \
    -I "$HERE" \
    -o "$PROBE_BIN" \
    "$HERE/firestaff_m11_pass44_party_group_collision_probe.c" \
    "$HERE/memory_movement_pc34_compat.c" \
    "$HERE/memory_dungeon_dat_pc34_compat.c" \
    "$HERE/memory_champion_state_pc34_compat.c"

"$PROBE_BIN" | tee "$LOG"
SUMMARY=$(grep '^# summary: ' "$LOG" | tail -n 1)
case "$SUMMARY" in
  *"4/4 invariants passed"*) exit 0 ;;
  *) echo "unexpected summary: $SUMMARY" >&2; exit 1 ;;
esac
