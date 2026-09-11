#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

DUNGEON_DAT=${1:-$FIRESTAFF_DATA/DUNGEON.DAT}
ROOT="$(cd -- "$HERE/../.." >/dev/null 2>&1 && pwd)"
OUT_DIR=${2:-$ROOT/.codex-scratch/champion-lifecycle}

mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_m10_champion_lifecycle_probe_bin"

cc -Wall -Wextra -O1 \
    -I "$ROOT/include" \
    -o "$PROBE_BIN" \
    "$ROOT/probes/firestaff_m10_champion_lifecycle_probe.c" \
    "$ROOT/src/memory/memory_champion_lifecycle_pc34_compat.c" \
    "$ROOT/src/memory/memory_magic_pc34_compat.c" \
    "$ROOT/src/memory/memory_combat_pc34_compat.c" \
    "$ROOT/src/memory/memory_timeline_pc34_compat.c" \
    "$ROOT/src/memory/memory_dungeon_dat_pc34_compat.c" \
    "$ROOT/src/memory/memory_champion_state_pc34_compat.c" \
    "$ROOT/src/shared/dungeon_decompressor_ftl.c" \
    "$ROOT/src/dm1/dm1_v1_fmtowns_dungeon_dat.c" \
    "$ROOT/src/csb/csb_v1_reincarnation_penalty_pc34_compat.c"

"$PROBE_BIN" "$DUNGEON_DAT" "$OUT_DIR"
echo "Champion lifecycle probe complete. Output: $OUT_DIR"
