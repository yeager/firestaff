#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

DUNGEON_DAT=${1:-$FIRESTAFF_DATA/DUNGEON.DAT}
OUT_DIR=${2:-$HERE/verification-m10/dungeon-monsters-items}
ROOT=$HERE
mkdir -p "$OUT_DIR"

echo "=== Compiling dungeon monsters/items probe ==="
cc -std=c11 -Wall -Wextra -O2 \
    -o "$OUT_DIR/dungeon_monsters_items_probe" \
    "$ROOT/firestaff_m10_monsters_items_probe.c" \
    "$ROOT/memory_dungeon_dat_pc34_compat.c" \
    -I"$ROOT"

echo "=== Running dungeon monsters/items probe ==="
"$OUT_DIR/dungeon_monsters_items_probe" "$DUNGEON_DAT" "$OUT_DIR"

echo "=== Results ==="
cat "$OUT_DIR/dungeon_monsters_items_invariants.md"
echo ""
echo "Full report: $OUT_DIR/dungeon_monsters_items_probe.md"
