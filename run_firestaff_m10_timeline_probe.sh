#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

OUT_DIR=${1:-$HERE/verification-m10/timeline}
ROOT=$HERE

mkdir -p "$OUT_DIR"

PROBE_BIN="$ROOT/firestaff_m10_timeline_probe_bin"

cc -Wall -Wextra -O1 \
    -I "$ROOT" \
    -o "$PROBE_BIN" \
    "$ROOT/firestaff_m10_timeline_probe.c" \
    "$ROOT/memory_timeline_pc34_compat.c"

"$PROBE_BIN" "$OUT_DIR"
echo "Timeline probe complete. Output: $OUT_DIR"
