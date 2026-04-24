#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
TMPDIR="${TMPDIR:-/tmp}/firestaff-pass57-title-render-$$"
mkdir -p "$TMPDIR"
trap 'rm -rf "$TMPDIR"' EXIT INT TERM

cc -std=c99 -Wall -Wextra -pedantic -O2 -I"$HERE" \
  "$HERE/probes/v1/firestaff_v1_pass57_title_render_probe.c" \
  "$HERE/title_dat_loader_v1.c" \
  -o "$HERE/firestaff_v1_pass57_title_render_probe"

FIRESTAFF_PASS57_DUMP_DIR="$TMPDIR" "$HERE/firestaff_v1_pass57_title_render_probe"
python3 "$HERE/probes/v1/firestaff_v1_pass57_title_png_compare_probe.py" "$TMPDIR" "$@"
