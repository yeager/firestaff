#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

DUNGEON_DAT=${1:-$FIRESTAFF_DATA/DUNGEON.DAT}
OUT_DIR=${2:-$HERE/verification-m10/dungeon-header}
ROOT=$HERE

mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/dungeon_header_probe"

echo "=== Compiling dungeon header probe ==="
cc -std=c99 -Wall -Wextra -Werror \
   -I"$ROOT" \
   -o "$PROBE_BIN" \
   "$ROOT/firestaff_m10_dungeon_header_probe.c" \
   "$ROOT/memory_dungeon_dat_pc34_compat.c"

echo "=== Running dungeon header probe ==="
"$PROBE_BIN" "$DUNGEON_DAT" "$OUT_DIR"

echo "=== Results ==="
cat "$OUT_DIR/dungeon_header_invariants.md"
echo ""
echo "Full report: $OUT_DIR/dungeon_header_probe.md"
