#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
ROOT="$(cd -- "$HERE/../.." >/dev/null 2>&1 && pwd)"
BUILD_DIR="${FIRESTAFF_PROBE_BUILD_DIR:-$ROOT/.codex-scratch/probes}"
mkdir -p "$BUILD_DIR"
if [ -n "${FIRESTAFF_PASS57_DUMP_DIR:-}" ]; then
  mkdir -p "$FIRESTAFF_PASS57_DUMP_DIR"
fi

cc -std=c99 -D_DEFAULT_SOURCE -Wall -Wextra -pedantic -O2 -I"$ROOT/include" \
  "$ROOT/probes/v1/firestaff_v1_pass57_title_render_probe.c" \
  "$ROOT/src/frontend/title_dat_loader_v1.c" \
  "$ROOT/src/shared/asset_find_by_hash.c" \
  "$ROOT/src/shared/firestaff_zip_extract.c" \
  -lz -o "$BUILD_DIR/firestaff_v1_pass57_title_render_probe"

"$BUILD_DIR/firestaff_v1_pass57_title_render_probe" "$@"
