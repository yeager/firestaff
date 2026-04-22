#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/game-view}
DATA_DIR=${FIRESTAFF_DATA:-${2:-$HOME/.firestaff/data}}
mkdir -p "$OUT_DIR"

PROBE_BIN="$HERE/firestaff_m11_game_view_probe_bin"
LOG="$OUT_DIR/game_view_probe.log"
PROBE_SRC_LOCAL="$HERE/firestaff_m11_game_view_probe.c"
PROBE_SRC_NESTED="$HERE/probes/m11/firestaff_m11_game_view_probe.c"
if [ -f "$PROBE_SRC_LOCAL" ]; then
    PROBE_SRC="$PROBE_SRC_LOCAL"
else
    PROBE_SRC="$PROBE_SRC_NESTED"
fi

CFLAGS_COMMON="-std=c99 -Wall -Wextra -O2 -I $HERE"
CFLAGS_M10="$CFLAGS_COMMON -DCOMPILE_H -DSTATICFUNCTION=static -DSEPARATOR=, -DFINAL_SEPARATOR=)"

# Compile M10 GRAPHICS.DAT pipeline objects (need COMPILE_H bypass)
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
    "$PROBE_SRC" \
    "$HERE/m11_game_view.c" \
    "$HERE/audio_sdl_m11.c" \
    "$HERE/asset_loader_m11.c" \
    "$HERE/font_m11.c" \
    "$HERE/fs_portable_compat.c" \
    "$HERE/config_m12.c" \
    "$HERE/asset_status_m12.c" \
    "$HERE/branding_logo_m12.c" \
    "$HERE/card_art_m12.c" \
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
    "$HERE/memory_champion_state_pc34_compat.c" \
    "$HERE/memory_dungeon_dat_pc34_compat.c" \
    $GFX_OBJS

FIRESTAFF_DATA="$DATA_DIR" "$PROBE_BIN" "$DATA_DIR" | tee "$LOG"

SUMMARY=$(grep '^# summary: ' "$LOG" | tail -n 1)
PASSED=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f1)
TOTAL=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f2)

echo "M11 game-view probe: $SUMMARY"
if [ "$PASSED" != "$TOTAL" ]; then
    exit 1
fi
