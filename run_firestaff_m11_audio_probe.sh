#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/audio}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_m11_audio_probe_bin"

# SDL3 flags via pkg-config
SDL3_CFLAGS=$(pkg-config --cflags sdl3 2>/dev/null || echo "-I/opt/homebrew/include")
SDL3_LIBS=$(pkg-config --libs sdl3 2>/dev/null || echo "-L/opt/homebrew/lib -lSDL3")

cc -std=c99 -Wall -Wextra -O2 -I "$HERE" \
    $SDL3_CFLAGS \
    -o "$PROBE_BIN" \
    "$HERE/probes/m11/firestaff_m11_audio_probe.c" \
    "$HERE/audio_sdl_m11.c" \
    $SDL3_LIBS -lm

"$PROBE_BIN" | tee "$OUT_DIR/audio_probe.log"
