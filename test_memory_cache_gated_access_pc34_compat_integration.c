#include <stdio.h>

#include "memory_cache_gated_access_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

struct NativeBitmapBlockStorage_Compat {
    unsigned long align;
    unsigned char bytes[128];
};

static struct NativeBitmapBlock_Compat* as_block(struct NativeBitmapBlockStorage_Compat* storage) {
    return (struct NativeBitmapBlock_Compat*)storage->bytes;
}

static int test_native_timegated_hit_resets_then_touches(void) {
    unsigned long lastReset = 10;
    unsigned short indices[4] = {MEMORY_CACHE_INDEX_NONE, 0, MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    struct MemoryCacheUsageState_Compat usageState = {0};
    blocks[0] = block;
    block->usageCount = 3;
    block->previousIndex = MEMORY_CACHE_INDEX_NONE;
    block->nextIndex = MEMORY_CACHE_INDEX_NONE;
    usageState.firstUsedBlock = block;
    usageState.lastUsedBlock = block;
    usageState.firstReferencedUsedBlock = block;
    return F0489_MEMORY_GetNativeBitmapByIndexTimeGated_Compat(11, &lastReset, 1, NULL, indices, blocks, 4, NULL, &usageState, NULL) == block->bitmap &&
           lastReset == 11 && block->usageCount == 1 && usageState.firstReferencedUsedBlock == block;
}

static int test_derived_timegated_hit_resets_then_touches(void) {
    unsigned long lastReset = 20;
    unsigned short indices[4] = {MEMORY_CACHE_INDEX_NONE, 0, MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    struct MemoryCacheUsageState_Compat usageState = {0};
    blocks[0] = block;
    block->usageCount = 2;
    block->previousIndex = MEMORY_CACHE_INDEX_NONE;
    block->nextIndex = MEMORY_CACHE_INDEX_NONE;
    usageState.firstUsedBlock = block;
    usageState.lastUsedBlock = block;
    usageState.firstReferencedUsedBlock = block;
    return F0491_CACHE_IsDerivedBitmapInCacheTimeGated_Compat(21, &lastReset, 1, indices, blocks, 4, NULL, &usageState) == 1 &&
           lastReset == 21 && block->usageCount == 1 && usageState.firstReferencedUsedBlock == block;
}

static int test_timegated_native_miss_builds_and_sets_last_reset(void) {
    unsigned long lastReset = 30;
    unsigned short indices[4] = {MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    struct MemoryCacheUsageState_Compat usageState = {0};
    unsigned char src[8] = {0x02,0x00,0x01,0x00,0x12,0x34,0x56,0x11};
    struct GraphicWidthHeight_Compat sizeInfo = {2,1};
    return F0489_MEMORY_GetNativeBitmapByIndexTimeGated_Compat(31, &lastReset, 2, src, indices, blocks, 4, block, &usageState, &sizeInfo) == block->bitmap &&
           lastReset == 31 && block->usageCount == 1 && usageState.firstUsedBlock == block;
}

int main(void) {
    if (!test_native_timegated_hit_resets_then_touches()) {
        fprintf(stderr, "test_native_timegated_hit_resets_then_touches failed\n");
        return 1;
    }
    if (!test_derived_timegated_hit_resets_then_touches()) {
        fprintf(stderr, "test_derived_timegated_hit_resets_then_touches failed\n");
        return 1;
    }
    if (!test_timegated_native_miss_builds_and_sets_last_reset()) {
        fprintf(stderr, "test_timegated_native_miss_builds_and_sets_last_reset failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
