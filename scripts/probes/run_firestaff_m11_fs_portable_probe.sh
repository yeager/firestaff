#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/fs_portable}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_m11_fs_portable_probe_bin"

cc -std=c99 -Wall -Wextra -O2 -I "$HERE" \
    -o "$PROBE_BIN" \
    "$HERE/probes/m11/firestaff_m11_fs_portable_probe.c" \
    "$HERE/fs_portable_compat.c"

"$PROBE_BIN" | tee "$OUT_DIR/fs_portable_probe.log"
