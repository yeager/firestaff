#include <stdio.h>

#include "memory_native_or_graphic_pc34_compat.h"
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

static int test_not_expanded_returns_graphic_directly_without_touching_cache(void) {
    unsigned long lastReset = 7;
    unsigned char graphic0[8] = {1};
    unsigned char graphic1[8] = {2};
    const unsigned char* graphics[2] = {graphic0, graphic1};
    unsigned short indices[2] = {MEMORY_CACHE_INDEX_NONE, MEMORY_CACHE_INDEX_NONE};
    struct NativeBitmapBlock_Compat* blocks[2] = {0};
    struct MemoryCacheUsageState_Compat usageState = {0};
    return F0489_MEMORY_GetNativeBitmapOrGraphicTimeGated_Compat(8, &lastReset, MEMORY_NATIVE_OR_GRAPHIC_FLAG_NOT_EXPANDED | 1, graphics, indices, blocks, 2, NULL, &usageState, NULL) == graphic1 &&
           lastReset == 7 && usageState.firstUsedBlock == 0 && blocks[0] == 0;
}

static int test_expanded_path_builds_bitmap_via_gated_access(void) {
    unsigned long lastReset = 10;
    unsigned char graphic0[8] = {0x02,0x00,0x01,0x00,0x12,0x34,0x56,0x11};
    const unsigned char* graphics[1] = {graphic0};
    unsigned short indices[1] = {MEMORY_CACHE_INDEX_NONE};
    struct NativeBitmapBlock_Compat* blocks[1] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct GraphicWidthHeight_Compat sizeInfo = {2,1};
    unsigned char* result = F0489_MEMORY_GetNativeBitmapOrGraphicTimeGated_Compat(11, &lastReset, 0, graphics, indices, blocks, 1, block, &usageState, &sizeInfo);
    return result == block->bitmap && lastReset == 11 && block->usageCount == 1 && usageState.firstUsedBlock == block && result[0] == 0x22 && result[1] == 0x00;
}

static int test_expanded_hit_touches_existing_bitmap(void) {
    unsigned long lastReset = 20;
    unsigned char graphic0[8] = {0xFF};
    const unsigned char* graphics[1] = {graphic0};
    unsigned short indices[1] = {0};
    struct NativeBitmapBlock_Compat* blocks[1] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    struct MemoryCacheUsageState_Compat usageState = {0};
    blocks[0] = block;
    block->usageCount = 4;
    block->previousIndex = MEMORY_CACHE_INDEX_NONE;
    block->nextIndex = MEMORY_CACHE_INDEX_NONE;
    usageState.firstUsedBlock = block;
    usageState.lastUsedBlock = block;
    usageState.firstReferencedUsedBlock = block;
    return F0489_MEMORY_GetNativeBitmapOrGraphicTimeGated_Compat(21, &lastReset, 0, graphics, indices, blocks, 1, NULL, &usageState, NULL) == block->bitmap &&
           lastReset == 21 && block->usageCount == 1;
}

int main(void) {
    if (!test_not_expanded_returns_graphic_directly_without_touching_cache()) {
        fprintf(stderr, "test_not_expanded_returns_graphic_directly_without_touching_cache failed\n");
        return 1;
    }
    if (!test_expanded_path_builds_bitmap_via_gated_access()) {
        fprintf(stderr, "test_expanded_path_builds_bitmap_via_gated_access failed\n");
        return 1;
    }
    if (!test_expanded_hit_touches_existing_bitmap()) {
        fprintf(stderr, "test_expanded_hit_touches_existing_bitmap failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
