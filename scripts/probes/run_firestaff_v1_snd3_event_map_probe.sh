#!/bin/sh
# Pass 52 — V1 DM sound-event -> GRAPHICS.DAT SND3 mapping probe.
#
# Usage: ./run_firestaff_v1_snd3_event_map_probe.sh [OUT_DIR]
#   GRAPHICS_DAT_PATH=<file> optional override for original GRAPHICS.DAT
#
# The mapping invariants run without original assets.  If GRAPHICS.DAT is
# visible, the probe additionally validates that every mapped item resolves to
# populated SND3 bank metadata.  No original asset is checked in.
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/audio}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_v1_snd3_event_map_probe_bin"

cc -std=c99 -Wall -Wextra -O2 -I "$HERE" \
    -o "$PROBE_BIN" \
    "$HERE/probes/v1/firestaff_v1_snd3_event_map_probe.c" \
    "$HERE/sound_event_snd3_map_v1.c" \
    "$HERE/graphics_dat_snd3_loader_v1.c"

"$PROBE_BIN" | tee "$OUT_DIR/v1_snd3_event_map_probe.log"
