#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m12/settings-smoke}
mkdir -p "$OUT_DIR"

BIN="$OUT_DIR/firestaff_m12_settings_smoke_bin"
VGA_OBJ="$OUT_DIR/vga_palette_pc34_compat.o"

SDL_FLAG=""
SDL_CFLAGS=""
SDL_LIBS=""
if pkg-config --exists sdl3 2>/dev/null; then
    SDL_FLAG="-DFS_USE_SDL3=1"
    SDL_CFLAGS=$(pkg-config --cflags sdl3)
    SDL_LIBS=$(pkg-config --libs sdl3)
elif pkg-config --exists sdl2 2>/dev/null; then
    SDL_FLAG="-DFS_USE_SDL2=1"
    SDL_CFLAGS=$(pkg-config --cflags sdl2)
    SDL_LIBS=$(pkg-config --libs sdl2)
else
    echo "ERROR: neither sdl3 nor sdl2 found via pkg-config" >&2
    exit 2
fi

cc -std=c99 -O2 \
    -DCOMPILE_H -DSTATICFUNCTION=static '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)' \
    -I "$HERE" \
    -c "$HERE/vga_palette_pc34_compat.c" \
    -o "$VGA_OBJ"

cc -std=c99 -Wall -Wextra -O2 -I "$HERE" $SDL_FLAG $SDL_CFLAGS \
    -o "$BIN" \
    "$HERE/firestaff_m12_settings_smoke.c" \
    "$HERE/main_loop_m11.c" \
    "$HERE/config_m12.c" \
    "$HERE/asset_status_m12.c" \
    "$HERE/card_art_m12.c" \
    "$HERE/menu_startup_m12.c" \
    "$HERE/render_sdl_m11.c" \
    "$VGA_OBJ" \
    $SDL_LIBS

export SDL_VIDEODRIVER=dummy
"$BIN" | tee "$OUT_DIR/settings_smoke.log"
