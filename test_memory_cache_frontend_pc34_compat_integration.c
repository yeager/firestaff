#include <stdio.h>
#include <string.h>

#include "memory_cache_frontend_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_build_native_bitmap_same_stride_returns_bitmap(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char storage[6] = {0};
    unsigned char* bitmap = storage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    return F0489_MEMORY_BuildNativeBitmapInPlace_Compat(src, bitmap, &sizeInfo) == bitmap &&
           storage[0] == 0x02 && storage[1] == 0x00 && storage[2] == 0x01 && storage[3] == 0x00 &&
           bitmap[0] == 0x22 && bitmap[1] == 0x00;
}

static int test_build_native_bitmap_padded_returns_bitmap(void) {
    unsigned char src[9] = {0x03, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE1};
    unsigned char storage[8] = {0};
    unsigned char* bitmap = storage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {3, 2};
    return F0489_MEMORY_BuildNativeBitmapInPlace_Compat(src, bitmap, &sizeInfo) == bitmap &&
           storage[0] == 0x03 && storage[1] == 0x00 && storage[2] == 0x02 && storage[3] == 0x00 &&
           bitmap[0] == 0x22 && bitmap[1] == 0x20 && bitmap[2] == 0x22 && bitmap[3] == 0x20;
}

int main(void) {
    if (!test_build_native_bitmap_same_stride_returns_bitmap()) {
        fprintf(stderr, "test_build_native_bitmap_same_stride_returns_bitmap failed\n");
        return 1;
    }
    if (!test_build_native_bitmap_padded_returns_bitmap()) {
        fprintf(stderr, "test_build_native_bitmap_padded_returns_bitmap failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
