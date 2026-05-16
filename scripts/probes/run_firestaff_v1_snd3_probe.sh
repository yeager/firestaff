#!/bin/sh
# Pass 51 — V1 GRAPHICS.DAT SND3 loader/decoder probe.
#
# Usage: ./run_firestaff_v1_snd3_probe.sh [OUT_DIR]
#   GRAPHICS_DAT_PATH=<file> optional override for original GRAPHICS.DAT
#
# When no original GRAPHICS.DAT is visible on disk, the probe exits with
# a SKIP line and success status.  No original asset is checked in.
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/audio}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_v1_graphics_dat_snd3_probe_bin"

cc -std=c99 -Wall -Wextra -O2 -I "$HERE" \
    -o "$PROBE_BIN" \
    "$HERE/probes/v1/firestaff_v1_graphics_dat_snd3_probe.c" \
    "$HERE/graphics_dat_snd3_loader_v1.c"

"$PROBE_BIN" | tee "$OUT_DIR/v1_graphics_dat_snd3_probe.log"
