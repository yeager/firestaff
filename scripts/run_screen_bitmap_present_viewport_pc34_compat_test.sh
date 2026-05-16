#!/bin/sh
set -eu
HERE=$(cd -- "$(dirname -- "$0")" && pwd)
TMPDIR=${TMPDIR:-/tmp}
BIN="$TMPDIR/firestaff_screen_bitmap_present_viewport_pc34_compat_test.$$"
trap 'rm -f "$BIN"' EXIT INT HUP TERM
cc -std=c99 -Wall -Wextra -pedantic -DCOMPILE_H "-DSEPARATOR=," "-DFINAL_SEPARATOR=)" \
  -I"$HERE" \
  "$HERE/test_screen_bitmap_present_viewport_pc34_compat.c" \
  "$HERE/screen_bitmap_present_pc34_compat.c" \
  "$HERE/bitmap_copy_pc34_compat.c" \
  -o "$BIN"
"$BIN"
