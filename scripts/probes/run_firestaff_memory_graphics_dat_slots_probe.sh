#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

cc -std=c99 -Wall -Wextra -pedantic -DCOMPILE_H '-DSTATICFUNCTION=static' '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)'   -I$HERE   $HERE/test_memory_graphics_dat_slots_pc34_compat_integration.c   $HERE/memory_graphics_dat_slots_pc34_compat.c   $HERE/memory_graphics_dat_state_pc34_compat.c   $HERE/memory_graphics_dat_select_pc34_compat.c   $HERE/memory_graphics_dat_header_pc34_compat.c   $HERE/memory_graphics_dat_metadata_pc34_compat.c   $HERE/memory_graphics_dat_pc34_compat.c   -o $HERE/test_memory_graphics_dat_slots_pc34_compat_integration
$HERE/test_memory_graphics_dat_slots_pc34_compat_integration
