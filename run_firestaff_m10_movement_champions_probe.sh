#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

DUNGEON_DAT=${1:-$FIRESTAFF_DATA/DUNGEON.DAT}
OUT_DIR=${2:-$HERE/verification-m10/movement-champions}
ROOT=$HERE

mkdir -p "$OUT_DIR"

PROBE_BIN="$ROOT/firestaff_m10_movement_champions_probe_bin"

cc -Wall -Wextra -O1 \
    -o "$PROBE_BIN" \
    "$ROOT/firestaff_m10_movement_champions_probe.c" \
    "$ROOT/memory_champion_state_pc34_compat.c" \
    "$ROOT/memory_movement_pc34_compat.c" \
    "$ROOT/memory_dungeon_dat_pc34_compat.c"

"$PROBE_BIN" "$DUNGEON_DAT" "$OUT_DIR"
echo "Movement + Champions probe complete. Output: $OUT_DIR"
