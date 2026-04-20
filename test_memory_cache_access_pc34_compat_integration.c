#include <stdio.h>
#include <string.h>

#include "memory_cache_access_pc34_compat.h"
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

static int test_native_miss_builds_and_adds_to_used_list(void) {
    unsigned short indices[4] = {MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    struct MemoryCacheUsageState_Compat usageState = {0};
    unsigned char src[8] = {0x02,0x00,0x01,0x00,0x12,0x34,0x56,0x11};
    struct GraphicWidthHeight_Compat sizeInfo = {2,1};
    unsigned char* result = F0489_MEMORY_GetNativeBitmapByIndexAndTouch_Compat(1, src, indices, blocks, 4, block, &usageState, &sizeInfo);
    return result == block->bitmap && indices[1] == 0 && usageState.firstUsedBlock == block && usageState.lastUsedBlock == block && usageState.firstReferencedUsedBlock == block && block->usageCount == 1;
}

static int test_native_hit_increments_usage(void) {
    unsigned short indices[4] = {MEMORY_CACHE_INDEX_NONE, 0, MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    struct MemoryCacheUsageState_Compat usageState = {0};
    blocks[0] = block;
    block->usageCount = 1;
    block->previousIndex = MEMORY_CACHE_INDEX_NONE;
    block->nextIndex = MEMORY_CACHE_INDEX_NONE;
    block->bitmap[0] = 0xAB;
    block->bitmap[1] = 0xCD;
    usageState.firstUsedBlock = block;
    usageState.lastUsedBlock = block;
    usageState.firstReferencedUsedBlock = block;
    return F0489_MEMORY_GetNativeBitmapByIndexAndTouch_Compat(1, NULL, indices, blocks, 4, NULL, &usageState, NULL) == block->bitmap && block->usageCount == 2;
}

static int test_derived_hit_touches_usage_and_add_derived_links_in(void) {
    unsigned short indices[4] = {MEMORY_CACHE_INDEX_NONE, 0, MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    struct MemoryCacheUsageState_Compat usageState = {0};
    blocks[0] = block;
    block->usageCount = 0;
    block->previousIndex = MEMORY_CACHE_INDEX_NONE;
    block->nextIndex = MEMORY_CACHE_INDEX_NONE;
    if (!F0491_CACHE_IsDerivedBitmapInCacheAndTouch_Compat(1, indices, blocks, 4, NULL, &usageState)) {
        return 0;
    }
    if (block->usageCount != 1 || usageState.firstReferencedUsedBlock != block) {
        return 0;
    }
    block->usageCount = 0;
    F0493_CACHE_AddDerivedBitmap_Compat(1, indices, blocks, 4, &usageState);
    return block->usageCount == 1 && usageState.firstReferencedUsedBlock == block;
}

static int test_derived_miss_registers_without_touching_usage(void) {
    unsigned short indices[4] = {MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    struct MemoryCacheUsageState_Compat usageState = {0};
    return F0491_CACHE_IsDerivedBitmapInCacheAndTouch_Compat(2, indices, blocks, 4, block, &usageState) == 0 && indices[2] == 0 && blocks[0] == block && block->usageCount == 0 && usageState.firstUsedBlock == 0;
}

int main(void) {
    if (!test_native_miss_builds_and_adds_to_used_list()) {
        fprintf(stderr, "test_native_miss_builds_and_adds_to_used_list failed\n");
        return 1;
    }
    if (!test_native_hit_increments_usage()) {
        fprintf(stderr, "test_native_hit_increments_usage failed\n");
        return 1;
    }
    if (!test_derived_hit_touches_usage_and_add_derived_links_in()) {
        fprintf(stderr, "test_derived_hit_touches_usage_and_add_derived_links_in failed\n");
        return 1;
    }
    if (!test_derived_miss_registers_without_touching_usage()) {
        fprintf(stderr, "test_derived_miss_registers_without_touching_usage failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
