#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

cc -std=c99 -Wall -Wextra -pedantic -DCOMPILE_H '-DSTATICFUNCTION=static' '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)'   -I$HERE   $HERE/test_memory_cache_block_pc34_compat_integration.c   $HERE/memory_cache_block_pc34_compat.c   $HERE/memory_cache_reuse_pc34_compat.c   $HERE/memory_cache_frontend_pc34_compat.c   $HERE/memory_frontend_pc34_compat.c   $HERE/expand_frontend_pc34_compat.c   $HERE/bitmap_call_pc34_compat.c   $HERE/image_expand_pc34_compat.c   $HERE/image_backend_pc34_compat.c   -o $HERE/test_memory_cache_block_pc34_compat_integration
$HERE/test_memory_cache_block_pc34_compat_integration
