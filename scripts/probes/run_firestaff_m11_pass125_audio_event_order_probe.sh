#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/pass125-audio-event-order}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_m11_pass125_audio_event_order_probe_bin"

SDL3_CFLAGS=$(pkg-config --cflags sdl3 2>/dev/null || echo "-I/opt/homebrew/include")
SDL3_LIBS=$(pkg-config --libs sdl3 2>/dev/null || echo "-L/opt/homebrew/lib -lSDL3")

cc -std=c99 -Wall -Wextra -O2 -I "$HERE" \
    $SDL3_CFLAGS \
    -o "$PROBE_BIN" \
    "$HERE/probes/m11/firestaff_m11_pass125_audio_event_order_probe.c" \
    "$HERE/audio_sdl_m11.c" \
    "$HERE/graphics_dat_snd3_loader_v1.c" \
    "$HERE/song_dat_loader_v1.c" \
    "$HERE/sound_event_snd3_map_v1.c" \
    $SDL3_LIBS -lm

"$PROBE_BIN" | tee "$OUT_DIR/pass125_audio_event_order_probe.log"
