#include <stdio.h>
#include <string.h>

#include "memory_cache_usage_pc34_compat.h"

struct NativeBitmapBlockStorage_Compat {
    unsigned long align;
    unsigned char bytes[128];
};

static struct NativeBitmapBlock_Compat* as_block(struct NativeBitmapBlockStorage_Compat* storage) {
    return (struct NativeBitmapBlock_Compat*)storage->bytes;
}

static int test_add_block_to_empty_list(void) {
    struct MemoryCacheUsageState_Compat state = {0};
    struct NativeBitmapBlock_Compat* blocks[3] = {0};
    struct NativeBitmapBlockStorage_Compat storage0 = {0};
    struct NativeBitmapBlock_Compat* block0 = as_block(&storage0);
    blocks[0] = block0;
    F0486_MEMORY_AddBlockToUsedList_Compat(0, &state, blocks, 3);
    return state.firstUsedBlock == block0 && state.lastUsedBlock == block0 && state.firstReferencedUsedBlock == block0 &&
           block0->usageCount == 1 && block0->previousIndex == MEMORY_CACHE_INDEX_NONE && block0->nextIndex == MEMORY_CACHE_INDEX_NONE;
}

static int test_reset_usage_counts_clears_referenced_tail_chain(void) {
    struct MemoryCacheUsageState_Compat state = {0};
    struct NativeBitmapBlock_Compat* blocks[3] = {0};
    struct NativeBitmapBlockStorage_Compat storage0 = {0};
    struct NativeBitmapBlockStorage_Compat storage1 = {0};
    struct NativeBitmapBlock_Compat* block0 = as_block(&storage0);
    struct NativeBitmapBlock_Compat* block1 = as_block(&storage1);
    blocks[0] = block0;
    blocks[1] = block1;
    F0486_MEMORY_AddBlockToUsedList_Compat(0, &state, blocks, 3);
    F0486_MEMORY_AddBlockToUsedList_Compat(1, &state, blocks, 3);
    F0485_CACHE_ResetUsageCounts_Compat(&state, blocks);
    return state.firstReferencedUsedBlock == 0 && block0->usageCount == 0 && block1->usageCount == 0;
}

static int test_increment_usage_reorders_block_after_higher_usage_neighbors(void) {
    struct MemoryCacheUsageState_Compat state = {0};
    struct NativeBitmapBlock_Compat* blocks[3] = {0};
    struct NativeBitmapBlockStorage_Compat storage0 = {0};
    struct NativeBitmapBlockStorage_Compat storage1 = {0};
    struct NativeBitmapBlockStorage_Compat storage2 = {0};
    struct NativeBitmapBlock_Compat* block0 = as_block(&storage0);
    struct NativeBitmapBlock_Compat* block1 = as_block(&storage1);
    struct NativeBitmapBlock_Compat* block2 = as_block(&storage2);
    blocks[0] = block0;
    blocks[1] = block1;
    blocks[2] = block2;
    F0486_MEMORY_AddBlockToUsedList_Compat(0, &state, blocks, 3);
    F0486_MEMORY_AddBlockToUsedList_Compat(1, &state, blocks, 3);
    F0486_MEMORY_AddBlockToUsedList_Compat(2, &state, blocks, 3);
    F0485_CACHE_ResetUsageCounts_Compat(&state, blocks);
    F0487_CACHE_GetBlockAndIncrementUsageCount_Compat(0, &state, blocks, 3);
    F0487_CACHE_GetBlockAndIncrementUsageCount_Compat(1, &state, blocks, 3);
    F0487_CACHE_GetBlockAndIncrementUsageCount_Compat(2, &state, blocks, 3);
    F0487_CACHE_GetBlockAndIncrementUsageCount_Compat(0, &state, blocks, 3);
    return state.firstUsedBlock == block2 && state.lastUsedBlock == block0 && state.firstReferencedUsedBlock == block2 &&
           block2->nextIndex == 1 && block1->nextIndex == 0 && block0->previousIndex == 1 && block0->usageCount == 2;
}

int main(void) {
    if (!test_add_block_to_empty_list()) {
        fprintf(stderr, "test_add_block_to_empty_list failed\n");
        return 1;
    }
    if (!test_reset_usage_counts_clears_referenced_tail_chain()) {
        fprintf(stderr, "test_reset_usage_counts_clears_referenced_tail_chain failed\n");
        return 1;
    }
    if (!test_increment_usage_reorders_block_after_higher_usage_neighbors()) {
        fprintf(stderr, "test_increment_usage_reorders_block_after_higher_usage_neighbors failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
