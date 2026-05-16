#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

DUNGEON_DAT=${1:-$FIRESTAFF_DATA/DUNGEON.DAT}
OUT_DIR=${2:-$HERE/verification-m10/dungeon-doors-sensors}
ROOT=$HERE
mkdir -p "$OUT_DIR"

echo "=== Compiling dungeon doors/sensors probe ==="
cc -std=c11 -Wall -Wextra -O2 \
    -o "$OUT_DIR/dungeon_doors_sensors_probe" \
    "$ROOT/firestaff_m10_doors_sensors_probe.c" \
    "$ROOT/memory_dungeon_dat_pc34_compat.c" \
    -I"$ROOT"

echo "=== Running dungeon doors/sensors probe ==="
"$OUT_DIR/dungeon_doors_sensors_probe" "$DUNGEON_DAT" "$OUT_DIR"

echo "=== Results ==="
cat "$OUT_DIR/dungeon_doors_sensors_invariants.md"
echo ""
echo "Full report: $OUT_DIR/dungeon_doors_sensors_probe.md"
