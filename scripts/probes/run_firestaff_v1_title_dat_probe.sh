#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
ROOT="$(cd -- "$HERE/../.." >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-${TMPDIR:-/tmp}/firestaff-v1-title-probe}
mkdir -p "$OUT_DIR"
PROBE_BIN="$OUT_DIR/firestaff_v1_pass56_title_dat_probe"

cc -std=c99 -Wall -Wextra -pedantic -O2 -I"$ROOT/include" \
  "$ROOT/probes/v1/firestaff_v1_pass56_title_dat_probe.c" \
  "$ROOT/src/frontend/title_dat_loader_v1.c" \
  -o "$PROBE_BIN"

"$PROBE_BIN"
