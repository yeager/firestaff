#ifndef FIRESTAFF_REDMCSB_F0472_CACHE_ADD_UNUSED_BLOCK_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0472_CACHE_ADD_UNUSED_BLOCK_PC34_COMPAT_H

#include <stddef.h>

/* ReDMCSB MEMORY.C F0472_CACHE_AddUnusedBlock (lines 486-551). */
typedef struct ReDMCSBF0472CacheUnusedBlockPc34Compat {
    long block_size;
    struct ReDMCSBF0472CacheUnusedBlockPc34Compat *previous_unused_block;
    struct ReDMCSBF0472CacheUnusedBlockPc34Compat *next_unused_block;
} ReDMCSBF0472CacheUnusedBlockPc34Compat;

void F0472_CACHE_AddUnusedBlock_PC34(
    ReDMCSBF0472CacheUnusedBlockPc34Compat *block,
    ReDMCSBF0472CacheUnusedBlockPc34Compat **first_unused_block);

#endif
