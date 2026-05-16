#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

cc -std=c99 -Wall -Wextra -pedantic -DCOMPILE_H '-DSTATICFUNCTION=static' '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)'   -I$HERE   $HERE/firestaff_memory_graphics_dat_original_main_loop_command_probe.c   $HERE/memory_graphics_dat_header_pc34_compat.c   $HERE/memory_graphics_dat_main_loop_command_pc34_compat.c   $HERE/memory_graphics_dat_startup_tick_pc34_compat.c   $HERE/memory_graphics_dat_main_loop_entry_pc34_compat.c   $HERE/memory_graphics_dat_startup_dispatch_pc34_compat.c   $HERE/memory_graphics_dat_first_frame_pc34_compat.c   $HERE/memory_graphics_dat_startup_wiring_pc34_compat.c   $HERE/memory_graphics_dat_boot_pc34_compat.c   $HERE/memory_graphics_dat_startup_pc34_compat.c   $HERE/memory_graphics_dat_viewport_path_pc34_compat.c   $HERE/memory_graphics_dat_special_pc34_compat.c   $HERE/memory_graphics_dat_runtime_transaction_pc34_compat.c   $HERE/memory_graphics_dat_state_pc34_compat.c   $HERE/memory_graphics_dat_composed_transaction_pc34_compat.c   $HERE/memory_graphics_dat_select_pc34_compat.c   $HERE/memory_graphics_dat_metadata_pc34_compat.c   $HERE/memory_graphics_dat_transaction_pc34_compat.c   $HERE/memory_graphics_dat_pc34_compat.c   -o $HERE/firestaff_memory_graphics_dat_original_main_loop_command_probe
if [ $# -eq 0 ]; then
  echo "built: $HERE/firestaff_memory_graphics_dat_original_main_loop_command_probe"
  exit 0
fi
$HERE/firestaff_memory_graphics_dat_original_main_loop_command_probe "$@"
