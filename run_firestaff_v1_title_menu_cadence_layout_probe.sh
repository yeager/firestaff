#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
GRAPHICS_DAT=${1:-"$HOME/.firestaff/data/GRAPHICS.DAT"}
TITLE_DAT=${2:-"$HOME/.openclaw/data/redmcsb-original/TITLE"}

cc -std=c99 -Wall -Wextra -pedantic \
  -DCOMPILE_H '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)' -DSTATICFUNCTION=static -DHUGE= -Dhuge= \
  -I"$HERE" \
  "$HERE/probes/v1/firestaff_v1_title_menu_cadence_layout_probe.c" \
  "$HERE/title_frontend_v1.c" \
  "$HERE/title_dat_loader_v1.c" \
  "$HERE/memory_graphics_dat_header_pc34_compat.c" \
  "$HERE/memory_graphics_dat_metadata_pc34_compat.c" \
  "$HERE/memory_graphics_dat_pc34_compat.c" \
  -o "$HERE/firestaff_v1_title_menu_cadence_layout_probe"

if [ -f "$TITLE_DAT" ]; then
  "$HERE/firestaff_v1_title_menu_cadence_layout_probe" "$GRAPHICS_DAT" "$TITLE_DAT"
else
  "$HERE/firestaff_v1_title_menu_cadence_layout_probe" "$GRAPHICS_DAT"
fi
