#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR="$HERE/verification-screens"
DATA_DIR=${FIRESTAFF_DATA:-${1:-$HOME/.firestaff/data}}
BUILD_DIR="$OUT_DIR"
mkdir -p "$OUT_DIR"

CAPTURE_BIN="$OUT_DIR/capture_bin"
CAPTURE_SRC="$OUT_DIR/capture_firestaff_ingame_series.c"

# SDL3 flags
SDL3_CFLAGS=""
SDL3_LIBS=""
if pkg-config --exists sdl3 2>/dev/null; then
    SDL3_CFLAGS=$(pkg-config --cflags sdl3)
    SDL3_LIBS="$(pkg-config --libs sdl3) -lm"
else
    SDL3_CFLAGS="-DFIRESTAFF_NO_SDL_AUDIO"
    SDL3_LIBS="-lm"
fi

CFLAGS_COMMON="-std=c99 -Wall -Wextra -O2 -I $HERE $SDL3_CFLAGS"
CFLAGS_M10="$CFLAGS_COMMON -DCOMPILE_H -DSTATICFUNCTION=static -DSEPARATOR=, -DFINAL_SEPARATOR=)"

# Compile M10 GRAPHICS.DAT pipeline objects
GFX_OBJS=""
for src in \
    memory_graphics_dat_pc34_compat \
    memory_graphics_dat_state_pc34_compat \
    memory_graphics_dat_header_pc34_compat \
    memory_graphics_dat_select_pc34_compat \
    memory_graphics_dat_metadata_pc34_compat \
    memory_frontend_pc34_compat \
    memory_cache_frontend_pc34_compat \
    expand_frontend_pc34_compat \
    bitmap_call_pc34_compat \
    image_expand_pc34_compat \
    image_backend_pc34_compat \
    graphics_dat_entry_classify_pc34_compat \
    late_special_dispatch_pc34_compat \
    memory_graphics_dat_transaction_pc34_compat \
    memory_load_expand_pc34_compat \
    bitmap_copy_pc34_compat; do
    OBJ="$BUILD_DIR/${src}.o"
    cc $CFLAGS_M10 -c "$HERE/${src}.c" -o "$OBJ"
    GFX_OBJS="$GFX_OBJS $OBJ"
done

cc $CFLAGS_COMMON \
    -o "$CAPTURE_BIN" \
    "$CAPTURE_SRC" \
    "$HERE/m11_game_view.c" \
    "$HERE/audio_sdl_m11.c" \
    "$HERE/graphics_dat_snd3_loader_v1.c" \
    "$HERE/song_dat_loader_v1.c" \
    "$HERE/sound_event_snd3_map_v1.c" \
    "$HERE/asset_loader_m11.c" \
    "$HERE/font_m11.c" \
    "$HERE/fs_portable_compat.c" \
    "$HERE/config_m12.c" \
    "$HERE/asset_status_m12.c" \
    "$HERE/branding_logo_m12.c" \
    "$HERE/card_art_m12.c" \
    "$HERE/creature_art_m12.c" \
    "$HERE/menu_startup_m12.c" \
    "$HERE/memory_tick_orchestrator_pc34_compat.c" \
    "$HERE/memory_champion_lifecycle_pc34_compat.c" \
    "$HERE/memory_runtime_dynamics_pc34_compat.c" \
    "$HERE/memory_projectile_pc34_compat.c" \
    "$HERE/memory_creature_ai_pc34_compat.c" \
    "$HERE/memory_savegame_pc34_compat.c" \
    "$HERE/memory_magic_pc34_compat.c" \
    "$HERE/memory_combat_pc34_compat.c" \
    "$HERE/memory_timeline_pc34_compat.c" \
    "$HERE/memory_sensor_execution_pc34_compat.c" \
    "$HERE/memory_movement_pc34_compat.c" \
    "$HERE/memory_door_action_pc34_compat.c" \
    "$HERE/memory_champion_state_pc34_compat.c" \
    "$HERE/memory_dungeon_dat_pc34_compat.c" \
    $GFX_OBJS \
    $SDL3_LIBS

FIRESTAFF_DATA="$DATA_DIR" "$CAPTURE_BIN" "$OUT_DIR" "$DATA_DIR"

# Convert PPM to PNG if convert/magick available
for ppm in "$OUT_DIR"/*_latest.ppm "$OUT_DIR"/*_viewport_224x136.ppm; do
    [ -f "$ppm" ] || continue
    png="${ppm%.ppm}.png"
    if command -v magick >/dev/null 2>&1; then
        magick "$ppm" "$png" 2>/dev/null || true
    elif command -v convert >/dev/null 2>&1; then
        convert "$ppm" "$png" 2>/dev/null || true
    fi
done

echo "Screenshots captured in $OUT_DIR"
