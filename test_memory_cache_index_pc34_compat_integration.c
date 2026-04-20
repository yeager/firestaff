#include <stdio.h>
#include <string.h>

#include "memory_cache_index_pc34_compat.h"
#include "memory_cache_block_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

struct NativeBitmapBlockStorage_Compat {
    unsigned long align;
    unsigned char bytes[128];
};

static int test_cache_index_hit_returns_existing_bitmap(void) {
    unsigned short indices[4] = {0xFFFF, 1, 0xFFFF, 0xFFFF};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = (struct NativeBitmapBlock_Compat*)storage.bytes;
    block->bitmapIndex = 1;
    block->width = 2;
    block->height = 1;
    block->bitmap[0] = 0xAB;
    block->bitmap[1] = 0xCD;
    blocks[1] = block;
    {
        unsigned char src[8] = {0x02,0x00,0x01,0x00,0x12,0x34,0x56,0x11};
        struct GraphicWidthHeight_Compat sizeInfo = {2,1};
        unsigned char* result = F0489_MEMORY_GetNativeBitmapByIndex_Compat(1, src, indices, blocks, 4, NULL, &sizeInfo);
        return result == block->bitmap && result[0] == 0xAB && result[1] == 0xCD;
    }
}

static int test_cache_index_miss_builds_and_registers_block(void) {
    unsigned short indices[4] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = (struct NativeBitmapBlock_Compat*)storage.bytes;
    unsigned char src[8] = {0x02,0x00,0x01,0x00,0x12,0x34,0x56,0x11};
    struct GraphicWidthHeight_Compat sizeInfo = {2,1};
    unsigned char* result = F0489_MEMORY_GetNativeBitmapByIndex_Compat(2, src, indices, blocks, 4, block, &sizeInfo);
    return indices[2] == 0 && blocks[0] == block && result == block->bitmap && block->bitmapIndex == 2 && block->width == 2 && block->height == 1 && result[0] == 0x22 && result[1] == 0x00;
}

static int test_cache_index_miss_padded_builds_and_registers_block(void) {
    unsigned short indices[4] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = (struct NativeBitmapBlock_Compat*)storage.bytes;
    unsigned char src[9] = {0x03,0x00,0x02,0x00,0x12,0x34,0x56,0x91,0xE1};
    struct GraphicWidthHeight_Compat sizeInfo = {3,2};
    unsigned char* result = F0489_MEMORY_GetNativeBitmapByIndex_Compat(3, src, indices, blocks, 4, block, &sizeInfo);
    return indices[3] == 0 && blocks[0] == block && result == block->bitmap && block->bitmapIndex == 3 && block->width == 3 && block->height == 2 && result[0] == 0x22 && result[1] == 0x20 && result[2] == 0x22 && result[3] == 0x20;
}

int main(void) {
    if (!test_cache_index_hit_returns_existing_bitmap()) {
        fprintf(stderr, "test_cache_index_hit_returns_existing_bitmap failed\n");
        return 1;
    }
    if (!test_cache_index_miss_builds_and_registers_block()) {
        fprintf(stderr, "test_cache_index_miss_builds_and_registers_block failed\n");
        return 1;
    }
    if (!test_cache_index_miss_padded_builds_and_registers_block()) {
        fprintf(stderr, "test_cache_index_miss_padded_builds_and_registers_block failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
