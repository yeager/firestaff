#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/pass37-sensor-runtime-wiring}
mkdir -p "$OUT_DIR"

PROBE_BIN="$HERE/firestaff_m11_pass37_sensor_runtime_wiring_probe_bin"
LOG="$OUT_DIR/pass37_sensor_runtime_wiring_probe.log"

cc -std=c99 -Wall -Wextra -O1 \
    -I "$HERE" \
    -o "$PROBE_BIN" \
    "$HERE/firestaff_m11_pass37_sensor_runtime_wiring_probe.c" \
    "$HERE/memory_tick_orchestrator_pc34_compat.c" \
    "$HERE/memory_sensor_execution_pc34_compat.c" \
    "$HERE/memory_movement_pc34_compat.c" \
    "$HERE/memory_dungeon_dat_pc34_compat.c" \
    "$HERE/memory_champion_state_pc34_compat.c" \
    "$HERE/memory_champion_lifecycle_pc34_compat.c" \
    "$HERE/memory_runtime_dynamics_pc34_compat.c" \
    "$HERE/memory_projectile_pc34_compat.c" \
    "$HERE/memory_creature_ai_pc34_compat.c" \
    "$HERE/memory_savegame_pc34_compat.c" \
    "$HERE/memory_magic_pc34_compat.c" \
    "$HERE/memory_combat_pc34_compat.c" \
    "$HERE/memory_timeline_pc34_compat.c"

"$PROBE_BIN" | tee "$LOG"

SUMMARY=$(grep '^# summary: ' "$LOG" | tail -n 1)
PASSED=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f1)
TOTAL=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f2)
echo "Pass 37 sensor runtime wiring probe: $SUMMARY"
if [ -z "$PASSED" ] || [ "$PASSED" != "$TOTAL" ]; then
    exit 1
fi
