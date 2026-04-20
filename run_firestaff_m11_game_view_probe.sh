#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/game-view}
DATA_DIR=${FIRESTAFF_DATA:-${2:-$HOME/.firestaff/data}}
mkdir -p "$OUT_DIR"

PROBE_BIN="$HERE/firestaff_m11_game_view_probe_bin"
LOG="$OUT_DIR/game_view_probe.log"

CFLAGS_COMMON="-std=c99 -Wall -Wextra -O2 -I $HERE"

cc $CFLAGS_COMMON \
    -o "$PROBE_BIN" \
    "$HERE/firestaff_m11_game_view_probe.c" \
    "$HERE/m11_game_view.c" \
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
    "$HERE/memory_dungeon_dat_pc34_compat.c"

FIRESTAFF_DATA="$DATA_DIR" "$PROBE_BIN" "$DATA_DIR" | tee "$LOG"

SUMMARY=$(grep '^# summary: ' "$LOG" | tail -n 1)
PASSED=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f1)
TOTAL=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f2)

echo "M11 game-view probe: $SUMMARY"
if [ "$PASSED" != "$TOTAL" ]; then
    exit 1
fi
