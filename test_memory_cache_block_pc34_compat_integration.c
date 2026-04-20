#include <stdio.h>
#include <string.h>

#include "memory_cache_block_pc34_compat.h"
#include "memory_cache_reuse_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_prepare_block_sets_metadata(void) {
    unsigned char storage[32] = {0};
    struct NativeBitmapBlock_Compat* block = (struct NativeBitmapBlock_Compat*)storage;
    struct GraphicWidthHeight_Compat sizeInfo = {3, 2};
    unsigned char* bitmap = F0489_MEMORY_PrepareNativeBitmapBlock_Compat(block, 42, &sizeInfo);
    return block->bitmapIndex == 42 && block->width == 3 && block->height == 2 && bitmap == block->bitmap;
}

static int test_prepare_and_build_same_stride(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char storage[32] = {0};
    struct NativeBitmapBlock_Compat* block = (struct NativeBitmapBlock_Compat*)storage;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    unsigned char* bitmap = F0489_MEMORY_PrepareNativeBitmapBlock_Compat(block, 7, &sizeInfo);
    unsigned char* result = F0489_MEMORY_GetNativeBitmapOrGraphic_Compat(src, NULL, bitmap, &sizeInfo);
    return result == bitmap && block->bitmapIndex == 7 && block->width == 2 && block->height == 1 && bitmap[0] == 0x22 && bitmap[1] == 0x00;
}

static int test_prepare_and_build_padded(void) {
    unsigned char src[9] = {0x03, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE1};
    unsigned char storage[40] = {0};
    struct NativeBitmapBlock_Compat* block = (struct NativeBitmapBlock_Compat*)storage;
    struct GraphicWidthHeight_Compat sizeInfo = {3, 2};
    unsigned char* bitmap = F0489_MEMORY_PrepareNativeBitmapBlock_Compat(block, 9, &sizeInfo);
    unsigned char* result = F0489_MEMORY_GetNativeBitmapOrGraphic_Compat(src, NULL, bitmap, &sizeInfo);
    return result == bitmap && block->bitmapIndex == 9 && block->width == 3 && block->height == 2 && bitmap[0] == 0x22 && bitmap[1] == 0x20 && bitmap[2] == 0x22 && bitmap[3] == 0x20;
}

int main(void) {
    if (!test_prepare_block_sets_metadata()) {
        fprintf(stderr, "test_prepare_block_sets_metadata failed\n");
        return 1;
    }
    if (!test_prepare_and_build_same_stride()) {
        fprintf(stderr, "test_prepare_and_build_same_stride failed\n");
        return 1;
    }
    if (!test_prepare_and_build_padded()) {
        fprintf(stderr, "test_prepare_and_build_padded failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
