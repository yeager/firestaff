#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m12/settings-smoke}
mkdir -p "$OUT_DIR"

BIN="$OUT_DIR/firestaff_m12_settings_smoke_bin"
VGA_OBJ="$OUT_DIR/vga_palette_pc34_compat.o"
CFLAGS_COMMON="-std=c99 -Wall -Wextra -O2 -I $HERE"
CFLAGS_M10="$CFLAGS_COMMON -DCOMPILE_H -DSTATICFUNCTION=static -DSEPARATOR=, -DFINAL_SEPARATOR=)"

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
    OBJ="$OUT_DIR/${src}.o"
    cc $CFLAGS_M10 -c "$HERE/${src}.c" -o "$OBJ"
    GFX_OBJS="$GFX_OBJS $OBJ"
done

cc $CFLAGS_COMMON $SDL_FLAG $SDL_CFLAGS \
    -o "$BIN" \
    "$HERE/probes/m12/firestaff_m12_settings_smoke.c" \
    "$HERE/main_loop_m11.c" \
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
    "$HERE/menu_startup_render_modern_m12.c" \
    "$HERE/menu_hit_m12.c" \
    "$HERE/render_sdl_m11.c" \
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
    "$VGA_OBJ" \
    $GFX_OBJS \
    $SDL_LIBS

export SDL_VIDEODRIVER=dummy
"$BIN" | tee "$OUT_DIR/settings_smoke.log"
