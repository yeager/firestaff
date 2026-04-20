#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

cc -std=c99 -Wall -Wextra -pedantic -DCOMPILE_H '-DSTATICFUNCTION=static' '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)'   -I$HERE   $HERE/firestaff_graphics_dat_entry_classify_probe.c   $HERE/graphics_dat_entry_classify_pc34_compat.c   $HERE/late_special_dispatch_pc34_compat.c   $HERE/memory_graphics_dat_state_pc34_compat.c   $HERE/memory_graphics_dat_header_pc34_compat.c   $HERE/memory_graphics_dat_metadata_pc34_compat.c   $HERE/memory_graphics_dat_transaction_pc34_compat.c   $HERE/memory_graphics_dat_pc34_compat.c   $HERE/memory_cache_frontend_pc34_compat.c   $HERE/memory_frontend_pc34_compat.c   $HERE/memory_load_expand_pc34_compat.c   $HERE/expand_frontend_pc34_compat.c   $HERE/bitmap_call_pc34_compat.c   $HERE/image_expand_pc34_compat.c   $HERE/image_backend_pc34_compat.c   $HERE/bitmap_copy_pc34_compat.c   -o $HERE/firestaff_graphics_dat_entry_classify_probe
if [ $# -eq 0 ]; then
  echo "built: $HERE/firestaff_graphics_dat_entry_classify_probe"
  exit 0
fi
$HERE/firestaff_graphics_dat_entry_classify_probe "$@"
