#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

if [ $# -lt 2 ]; then
  echo "usage: $0 /path/to/GRAPHICS.DAT /path/to/output_prefix" >&2
  exit 2
fi

GRAPHICS_DAT="$1"
OUTPUT_PREFIX="$2"
WORKDIR="$HERE"

printf '== STATEFUL BOOT TO HELD MENU ==\n'
"$WORKDIR/run_firestaff_original_stateful_boot_plan_reachability_probe.sh" \
  "$GRAPHICS_DAT" \
  "$OUTPUT_PREFIX" \
  m7_reachability_b \
  1 1 24 16 16 8 8

printf '\n== MENU STATE ADVANCE ==\n'
"$WORKDIR/run_firestaff_memory_graphics_dat_original_menu_state_probe.sh" \
  "$GRAPHICS_DAT"

printf '\n== MENU ACTIVATE ==\n'
"$WORKDIR/run_firestaff_memory_graphics_dat_original_menu_activate_probe.sh" \
  "$GRAPHICS_DAT"

printf '\n== MENU ACTIVATE CONSEQUENCE ==\n'
"$WORKDIR/run_firestaff_memory_graphics_dat_original_menu_activate_consequence_probe.sh" \
  "$GRAPHICS_DAT"
