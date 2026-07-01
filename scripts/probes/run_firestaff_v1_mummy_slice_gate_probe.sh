#!/bin/sh
# Pass 1090 — DM1 V1 Mummy slice gate probe.
#
# Usage: ./run_firestaff_v1_mummy_slice_gate_probe.sh [OUT_DIR]
#
# Builds and runs a focused Mummy (C010_CREATURE_MUMMY) source-locked
# probe.  No GRAPHICS.DAT is required.  No SDL3 runtime is required —
# this probe exercises the M10 source-locked creature render module
# directly and prints deterministic output to stdout.
#
# Output is mirrored to verification-m11/v1-mummy-slice-gate-YYYYMMDD-HHMMSS/
# for inclusion in pass notes.

set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
REPO_ROOT="$(cd -- "$HERE/../.." >/dev/null 2>&1 && pwd)"
TS="$(date +%Y%m%d-%H%M%S)"
OUT_DIR=${1:-"$REPO_ROOT/verification-m11/v1-mummy-slice-gate-$TS"}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_v1_mummy_slice_gate_probe_bin"

cc -std=c99 -Wall -Wextra -Wno-shift-negative-value -O2 \
    -I "$REPO_ROOT/include" \
    -o "$PROBE_BIN" \
    "$REPO_ROOT/probes/v1/firestaff_v1_mummy_slice_gate_probe.c" \
    "$REPO_ROOT/src/dm1/dm1_v1_creature_render_pc34_compat.c"

"$PROBE_BIN" | tee "$OUT_DIR/v1_mummy_slice_gate_probe.log"
