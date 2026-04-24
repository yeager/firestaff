#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/pass46-vga-palette}
mkdir -p "$OUT_DIR"

PROBE_BIN="$HERE/firestaff_m11_pass46_vga_palette_probe_bin"
LOG="$OUT_DIR/pass46_vga_palette_probe.log"

cc -std=c99 -Wall -Wextra -pedantic \
  -DCOMPILE_H '-DSTATICFUNCTION=static' '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)' \
  -I"$HERE" \
  -o "$PROBE_BIN" \
  "$HERE/firestaff_m11_pass46_vga_palette_probe.c" \
  "$HERE/vga_palette_pc34_compat.c"

"$PROBE_BIN" | tee "$LOG"
SUMMARY=$(grep '^# summary: ' "$LOG" | tail -n 1 || true)
PASSED=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f1)
TOTAL=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f2)
printf 'Pass 46 VGA palette probe: %s\n' "$SUMMARY"
if [ -z "$PASSED" ] || [ "$PASSED" != "$TOTAL" ]; then
  exit 1
fi
