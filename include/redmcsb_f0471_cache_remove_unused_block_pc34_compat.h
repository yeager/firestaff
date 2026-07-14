#ifndef FIRESTAFF_REDMCSB_F0471_CACHE_REMOVE_UNUSED_BLOCK_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0471_CACHE_REMOVE_UNUSED_BLOCK_PC34_COMPAT_H

#include <stddef.h>

/* ReDMCSB MEMORY.C F0471_CACHE_RemoveUnusedBlock (lines 454-484). */
typedef struct ReDMCSBF0471CacheUnusedBlockPc34Compat {
    struct ReDMCSBF0471CacheUnusedBlockPc34Compat *previous_unused_block;
    struct ReDMCSBF0471CacheUnusedBlockPc34Compat *next_unused_block;
} ReDMCSBF0471CacheUnusedBlockPc34Compat;

void F0471_CACHE_RemoveUnusedBlock_PC34(
    ReDMCSBF0471CacheUnusedBlockPc34Compat *block,
    ReDMCSBF0471CacheUnusedBlockPc34Compat **first_unused_block);

#endif
