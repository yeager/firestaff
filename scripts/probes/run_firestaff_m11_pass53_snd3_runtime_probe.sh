#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
ROOT="$(cd -- "$HERE/../.." >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/pass53-snd3-runtime}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_m11_pass53_snd3_runtime_probe_bin"

SDL3_CFLAGS=$(pkg-config --cflags sdl3 2>/dev/null || echo "-I/opt/homebrew/include")
SDL3_LIBS=$(pkg-config --libs sdl3 2>/dev/null || echo "-L/opt/homebrew/lib -lSDL3")

cc -std=c99 -Wall -Wextra -O2 -I "$ROOT/include" -I "$ROOT/src/shared" \
    $SDL3_CFLAGS \
    -o "$PROBE_BIN" \
    "$ROOT/probes/m11/firestaff_m11_pass53_snd3_runtime_probe.c" \
    "$ROOT/src/shared/audio_sdl_m11.c" \
    "$ROOT/src/shared/graphics_dat_snd3_loader_v1.c" \
    "$ROOT/src/shared/song_dat_loader_v1.c" \
    "$ROOT/src/shared/sound_event_snd3_map_v1.c" \
    "$ROOT/probes/m11/m11_audio_probe_link_stubs.c" \
    $SDL3_LIBS -lm

"$PROBE_BIN" | tee "$OUT_DIR/pass53_snd3_runtime_probe.log"
