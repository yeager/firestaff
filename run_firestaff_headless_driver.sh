#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

DUNGEON_DAT=${1:-$FIRESTAFF_DATA/DUNGEON.DAT}
ROOT=$HERE
BIN="$ROOT/firestaff_headless"

cc -Wall -Wextra -O1 \
    -I "$ROOT" \
    -o "$BIN" \
    "$ROOT/firestaff_headless_driver.c" \
    "$ROOT/memory_tick_orchestrator_pc34_compat.c" \
    "$ROOT/memory_champion_lifecycle_pc34_compat.c" \
    "$ROOT/memory_runtime_dynamics_pc34_compat.c" \
    "$ROOT/memory_projectile_pc34_compat.c" \
    "$ROOT/memory_creature_ai_pc34_compat.c" \
    "$ROOT/memory_savegame_pc34_compat.c" \
    "$ROOT/memory_magic_pc34_compat.c" \
    "$ROOT/memory_combat_pc34_compat.c" \
    "$ROOT/memory_timeline_pc34_compat.c" \
    "$ROOT/memory_sensor_execution_pc34_compat.c" \
    "$ROOT/memory_movement_pc34_compat.c" \
    "$ROOT/memory_champion_state_pc34_compat.c" \
    "$ROOT/memory_dungeon_dat_pc34_compat.c" \
    "$ROOT/memory_door_action_pc34_compat.c"

"$BIN" --dungeon "$DUNGEON_DAT" --seed 1234 --ticks 100
