#include <stdio.h>
#include <string.h>

#include "memory_cache_reuse_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_reuse_returns_cached_bitmap_without_rebuild(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char cached[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    unsigned char target[6] = {0};
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    unsigned char* result = F0489_MEMORY_GetNativeBitmapOrGraphic_Compat(src, cached + 4, target + 4, &sizeInfo);
    return result == cached + 4 && cached[4] == 0xEE && cached[5] == 0xFF && target[4] == 0x00 && target[5] == 0x00;
}

static int test_reuse_builds_when_cache_missing_same_stride(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char storage[6] = {0};
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    unsigned char* result = F0489_MEMORY_GetNativeBitmapOrGraphic_Compat(src, NULL, storage + 4, &sizeInfo);
    return result == storage + 4 && storage[0] == 0x02 && storage[1] == 0x00 && storage[2] == 0x01 && storage[3] == 0x00 && storage[4] == 0x22 && storage[5] == 0x00;
}

static int test_reuse_builds_when_cache_missing_padded(void) {
    unsigned char src[9] = {0x03, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE1};
    unsigned char storage[8] = {0};
    struct GraphicWidthHeight_Compat sizeInfo = {3, 2};
    unsigned char* result = F0489_MEMORY_GetNativeBitmapOrGraphic_Compat(src, NULL, storage + 4, &sizeInfo);
    return result == storage + 4 && storage[0] == 0x03 && storage[1] == 0x00 && storage[2] == 0x02 && storage[3] == 0x00 && storage[4] == 0x22 && storage[5] == 0x20 && storage[6] == 0x22 && storage[7] == 0x20;
}

int main(void) {
    if (!test_reuse_returns_cached_bitmap_without_rebuild()) {
        fprintf(stderr, "test_reuse_returns_cached_bitmap_without_rebuild failed\n");
        return 1;
    }
    if (!test_reuse_builds_when_cache_missing_same_stride()) {
        fprintf(stderr, "test_reuse_builds_when_cache_missing_same_stride failed\n");
        return 1;
    }
    if (!test_reuse_builds_when_cache_missing_padded()) {
        fprintf(stderr, "test_reuse_builds_when_cache_missing_padded failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
