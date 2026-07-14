#include "redmcsb_f0472_cache_add_unused_block_pc34_compat.h"

void F0472_CACHE_AddUnusedBlock_PC34(
    ReDMCSBF0472CacheUnusedBlockPc34Compat *block,
    ReDMCSBF0472CacheUnusedBlockPc34Compat **first_unused_block)
{
    ReDMCSBF0472CacheUnusedBlockPc34Compat *unused_block;
    ReDMCSBF0472CacheUnusedBlockPc34Compat *current_block;
    long block_size;
    int block_added_to_unused_list;

    unused_block = block;
    if (*first_unused_block == NULL) {
        *first_unused_block = unused_block;
        unused_block->previous_unused_block = unused_block->next_unused_block = NULL;
    } else {
        current_block = *first_unused_block;
        block_size = unused_block->block_size;
        if (block_size >= current_block->block_size) {
            *first_unused_block = unused_block;
            unused_block->previous_unused_block = NULL;
            unused_block->next_unused_block = current_block;
            current_block->previous_unused_block = unused_block;
        } else {
            block_added_to_unused_list = 0;
            while ((unused_block = current_block->next_unused_block) != NULL) {
                if (block_size >= unused_block->block_size) {
                    current_block->next_unused_block =
                        unused_block->previous_unused_block = block;
                    block->previous_unused_block = current_block;
                    block->next_unused_block = unused_block;
                    block_added_to_unused_list = 1;
                    break;
                }
                current_block = unused_block;
            }
            if (!block_added_to_unused_list) {
                current_block->next_unused_block = block;
                block->previous_unused_block = current_block;
                block->next_unused_block = NULL;
            }
        }
    }
}
