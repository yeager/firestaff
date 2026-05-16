#!/bin/sh
# Pass 50 — V1 SONG.DAT loader/decoder probe.
#
# Usage: ./run_firestaff_v1_song_dat_probe.sh [OUT_DIR]
#   SONG_DAT_PATH=<file>  optional override for the original SONG.DAT path
#
# When no original SONG.DAT is visible on disk, the probe exits with a
# SKIP line and success status (the format/decoder are covered by
# DM1_SONG_DAT_FORMAT.md).
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/audio}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_v1_song_dat_probe_bin"

cc -std=c99 -Wall -Wextra -O2 -I "$HERE" \
    -o "$PROBE_BIN" \
    "$HERE/probes/v1/firestaff_v1_song_dat_probe.c" \
    "$HERE/song_dat_loader_v1.c"

"$PROBE_BIN" | tee "$OUT_DIR/v1_song_dat_probe.log"
