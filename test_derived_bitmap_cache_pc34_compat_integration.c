#include <stdio.h>
#include <string.h>

#include "derived_bitmap_cache_pc34_compat.h"
#include "memory_cache_block_pc34_compat.h"

static int test_derived_cache_hit_returns_true(void) {
    unsigned short indices[4] = {0xFFFF, 1, 0xFFFF, 0xFFFF};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    unsigned char storage[64] = {0};
    struct NativeBitmapBlock_Compat* block = (struct NativeBitmapBlock_Compat*)storage;
    block->bitmapIndex = (unsigned short)(1 | 0x8000);
    block->bitmap[0] = 0xAA;
    block->bitmap[1] = 0xBB;
    blocks[1] = block;
    return F0491_CACHE_IsDerivedBitmapInCache_Compat(1, indices, blocks, 4, NULL) == 1;
}

static int test_derived_cache_miss_registers_block_and_returns_false(void) {
    unsigned short indices[4] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    unsigned char storage[64] = {0};
    struct NativeBitmapBlock_Compat* block = (struct NativeBitmapBlock_Compat*)storage;
    return F0491_CACHE_IsDerivedBitmapInCache_Compat(2, indices, blocks, 4, block) == 0 &&
           indices[2] == 0 && blocks[0] == block && block->bitmapIndex == (unsigned short)(2 | 0x8000);
}

static int test_get_derived_bitmap_returns_bitmap_pointer(void) {
    unsigned short indices[4] = {0xFFFF, 0, 0xFFFF, 0xFFFF};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    unsigned char storage[64] = {0};
    struct NativeBitmapBlock_Compat* block = (struct NativeBitmapBlock_Compat*)storage;
    block->bitmap[0] = 0x12;
    block->bitmap[1] = 0x34;
    blocks[0] = block;
    return F0492_CACHE_GetDerivedBitmap_Compat(1, indices, blocks) == block->bitmap && block->bitmap[0] == 0x12 && block->bitmap[1] == 0x34;
}

int main(void) {
    if (!test_derived_cache_hit_returns_true()) {
        fprintf(stderr, "test_derived_cache_hit_returns_true failed\n");
        return 1;
    }
    if (!test_derived_cache_miss_registers_block_and_returns_false()) {
        fprintf(stderr, "test_derived_cache_miss_registers_block_and_returns_false failed\n");
        return 1;
    }
    if (!test_get_derived_bitmap_returns_bitmap_pointer()) {
        fprintf(stderr, "test_get_derived_bitmap_returns_bitmap_pointer failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
