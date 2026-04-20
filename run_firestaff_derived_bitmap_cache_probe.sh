#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

cc -std=c99 -Wall -Wextra -pedantic -DCOMPILE_H '-DSTATICFUNCTION=static' '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)'   -I$HERE   $HERE/test_derived_bitmap_cache_pc34_compat_integration.c   $HERE/derived_bitmap_cache_pc34_compat.c   $HERE/memory_cache_block_pc34_compat.c   -o $HERE/test_derived_bitmap_cache_pc34_compat_integration
$HERE/test_derived_bitmap_cache_pc34_compat_integration
