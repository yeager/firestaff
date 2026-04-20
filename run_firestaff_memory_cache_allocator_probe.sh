#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

cc -std=c99 -Wall -Wextra -pedantic -DCOMPILE_H '-DSTATICFUNCTION=static' '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)'   -I$HERE   $HERE/test_memory_cache_allocator_pc34_compat_integration.c   $HERE/memory_cache_allocator_pc34_compat.c   $HERE/memory_cache_defrag_branch_pc34_compat.c   $HERE/memory_cache_allocate_or_reuse_pc34_compat.c   $HERE/memory_cache_reuse_split_pc34_compat.c   $HERE/memory_cache_unused_blocks_pc34_compat.c   -o $HERE/test_memory_cache_allocator_pc34_compat_integration
$HERE/test_memory_cache_allocator_pc34_compat_integration
