#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/pass45-font-bank}
GRAPHICS_DAT=${2:-$HOME/.firestaff/data/GRAPHICS.DAT}
mkdir -p "$OUT_DIR"

PROBE_BIN="$HERE/firestaff_m11_pass45_font_bank_probe_bin"
LOG="$OUT_DIR/pass45_font_bank_probe.log"

SDL3_CFLAGS="-DFIRESTAFF_NO_SDL_AUDIO"
SDL3_LIBS="-lm"

CFLAGS_COMMON="-std=c99 -Wall -Wextra -O2 -I $HERE $SDL3_CFLAGS"
CFLAGS_M10="$CFLAGS_COMMON -DCOMPILE_H -DSTATICFUNCTION=static -DSEPARATOR=, -DFINAL_SEPARATOR=)"

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

cc $CFLAGS_COMMON \
    -o "$PROBE_BIN" \
    "$HERE/firestaff_m11_pass45_font_bank_probe.c" \
    "$HERE/m11_game_view.c" \
    "$HERE/audio_sdl_m11.c" \
    "$HERE/graphics_dat_snd3_loader_v1.c" \
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

set +e
( cd "$HERE" && "$PROBE_BIN" "$OUT_DIR" "$GRAPHICS_DAT" ) | tee "$LOG"
PROBE_STATUS=$?
set -e

SUMMARY=$(grep '^# summary: ' "$LOG" | tail -n 1 || true)
if [ $PROBE_STATUS -ne 0 ] || [ -z "$SUMMARY" ]; then
    : > "$LOG"
    lldb --batch -o run -- "$PROBE_BIN" "$OUT_DIR" "$GRAPHICS_DAT" | tee "$LOG"
    SUMMARY=$(grep '^# summary: ' "$LOG" | tail -n 1 || true)
fi

PASSED=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f1)
TOTAL=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f2)
echo "Pass 45 font-bank probe: $SUMMARY"
if [ -z "$PASSED" ] || [ "$PASSED" != "$TOTAL" ]; then
    exit 1
fi
