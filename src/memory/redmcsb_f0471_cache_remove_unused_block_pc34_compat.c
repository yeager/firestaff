#include "redmcsb_f0471_cache_remove_unused_block_pc34_compat.h"

void F0471_CACHE_RemoveUnusedBlock_PC34(
    ReDMCSBF0471CacheUnusedBlockPc34Compat *block,
    ReDMCSBF0471CacheUnusedBlockPc34Compat **first_unused_block) {
    ReDMCSBF0471CacheUnusedBlockPc34Compat *previous_unused_block;
    ReDMCSBF0471CacheUnusedBlockPc34Compat *next_unused_block;

    previous_unused_block = block->previous_unused_block;
    next_unused_block = block->next_unused_block;
    if (previous_unused_block == NULL) {
        *first_unused_block = next_unused_block;
        if (next_unused_block != NULL) {
            next_unused_block->previous_unused_block = NULL;
        }
    } else {
        previous_unused_block->next_unused_block = next_unused_block;
        if (next_unused_block != NULL) {
            next_unused_block->previous_unused_block = previous_unused_block;
        }
    }
    block->previous_unused_block = NULL;
    block->next_unused_block = NULL;
}
