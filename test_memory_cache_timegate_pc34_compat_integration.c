#include <stdio.h>

#include "memory_cache_timegate_pc34_compat.h"

struct NativeBitmapBlockStorage_Compat {
    unsigned long align;
    unsigned char bytes[128];
};

static struct NativeBitmapBlock_Compat* as_block(struct NativeBitmapBlockStorage_Compat* storage) {
    return (struct NativeBitmapBlock_Compat*)storage->bytes;
}

static int test_timegate_skips_reset_when_time_matches(void) {
    struct MemoryCacheUsageState_Compat state = {0};
    struct NativeBitmapBlock_Compat* blocks[2] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    unsigned long lastReset = 123;
    blocks[0] = block;
    state.firstReferencedUsedBlock = block;
    state.lastUsedBlock = block;
    block->usageCount = 2;
    return MEMORY_CACHE_CheckResetUsageCounts_Compat(123, &lastReset, &state, blocks) == 0 &&
           lastReset == 123 && state.firstReferencedUsedBlock == block && block->usageCount == 2;
}

static int test_timegate_resets_when_time_changes(void) {
    struct MemoryCacheUsageState_Compat state = {0};
    struct NativeBitmapBlock_Compat* blocks[2] = {0};
    struct NativeBitmapBlockStorage_Compat storage0 = {0};
    struct NativeBitmapBlockStorage_Compat storage1 = {0};
    struct NativeBitmapBlock_Compat* block0 = as_block(&storage0);
    struct NativeBitmapBlock_Compat* block1 = as_block(&storage1);
    unsigned long lastReset = 123;
    blocks[0] = block0;
    blocks[1] = block1;
    state.lastUsedBlock = block1;
    state.firstReferencedUsedBlock = block0;
    block0->usageCount = 1;
    block0->previousIndex = MEMORY_CACHE_INDEX_NONE;
    block1->usageCount = 3;
    block1->previousIndex = 0;
    return MEMORY_CACHE_CheckResetUsageCounts_Compat(124, &lastReset, &state, blocks) == 1 &&
           lastReset == 124 && state.firstReferencedUsedBlock == 0 && block0->usageCount == 0 && block1->usageCount == 0;
}

int main(void) {
    if (!test_timegate_skips_reset_when_time_matches()) {
        fprintf(stderr, "test_timegate_skips_reset_when_time_matches failed\n");
        return 1;
    }
    if (!test_timegate_resets_when_time_changes()) {
        fprintf(stderr, "test_timegate_resets_when_time_changes failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
