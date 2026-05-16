#include <stdio.h>
#include <string.h>

#include "memory_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_memory_expand_sets_bitmap_and_dimensions_same_stride(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char storage[6] = {0};
    unsigned char* bitmap = storage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {2, 1};
    F0488_MEMORY_ExpandGraphicToBitmap_Compat(src, bitmap, &sizeInfo);
    return storage[0] == 0x02 && storage[1] == 0x00 && storage[2] == 0x01 && storage[3] == 0x00 && bitmap[0] == 0x22 && bitmap[1] == 0x00;
}

static int test_memory_expand_sets_bitmap_and_dimensions_padded(void) {
    unsigned char src[9] = {0x03, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE1};
    unsigned char storage[8] = {0};
    unsigned char* bitmap = storage + 4;
    struct GraphicWidthHeight_Compat sizeInfo = {3, 2};
    F0488_MEMORY_ExpandGraphicToBitmap_Compat(src, bitmap, &sizeInfo);
    return storage[0] == 0x03 && storage[1] == 0x00 && storage[2] == 0x02 && storage[3] == 0x00 && bitmap[0] == 0x22 && bitmap[1] == 0x20 && bitmap[2] == 0x22 && bitmap[3] == 0x20;
}

static int test_memory_expand_null_sizeinfo_is_safe(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char storage[6] = {0};
    unsigned char* bitmap = storage + 4;
    F0488_MEMORY_ExpandGraphicToBitmap_Compat(src, bitmap, NULL);
    return bitmap[0] == 0x22 && bitmap[1] == 0x00;
}

int main(void) {
    if (!test_memory_expand_sets_bitmap_and_dimensions_same_stride()) {
        fprintf(stderr, "test_memory_expand_sets_bitmap_and_dimensions_same_stride failed\n");
        return 1;
    }
    if (!test_memory_expand_sets_bitmap_and_dimensions_padded()) {
        fprintf(stderr, "test_memory_expand_sets_bitmap_and_dimensions_padded failed\n");
        return 1;
    }
    if (!test_memory_expand_null_sizeinfo_is_safe()) {
        fprintf(stderr, "test_memory_expand_null_sizeinfo_is_safe failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
