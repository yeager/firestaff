#include <stdio.h>
#include <string.h>

#include "bitmap_copy_pc34_compat.h"

static int test_copy_dimensions_only(void) {
    unsigned char srcStorage[8] = {0x03, 0x00, 0x02, 0x00, 0xAA, 0xBB, 0xCC, 0xDD};
    unsigned char dstStorage[8] = {0};
    unsigned char* srcBitmap = srcStorage + 4;
    unsigned char* dstBitmap = dstStorage + 4;
    dstBitmap[0] = 0x11;
    dstBitmap[1] = 0x22;
    dstBitmap[2] = 0x33;
    dstBitmap[3] = 0x44;
    F0615_CopyBitmapDimensions_Compat(srcBitmap, dstBitmap);
    return dstStorage[0] == 0x03 && dstStorage[1] == 0x00 && dstStorage[2] == 0x02 && dstStorage[3] == 0x00 &&
           dstBitmap[0] == 0x11 && dstBitmap[1] == 0x22 && dstBitmap[2] == 0x33 && dstBitmap[3] == 0x44;
}

static int test_copy_bitmap_same_stride(void) {
    unsigned char srcStorage[6] = {0x02, 0x00, 0x01, 0x00, 0x22, 0x00};
    unsigned char dstStorage[6] = {0};
    unsigned char* srcBitmap = srcStorage + 4;
    unsigned char* dstBitmap = dstStorage + 4;
    F0616_CopyBitmap_Compat(srcBitmap, dstBitmap);
    return memcmp(srcStorage, dstStorage, sizeof(srcStorage)) == 0;
}

static int test_copy_bitmap_padded(void) {
    unsigned char srcStorage[8] = {0x03, 0x00, 0x02, 0x00, 0x22, 0x20, 0x22, 0x20};
    unsigned char dstStorage[8] = {0};
    unsigned char* srcBitmap = srcStorage + 4;
    unsigned char* dstBitmap = dstStorage + 4;
    F0616_CopyBitmap_Compat(srcBitmap, dstBitmap);
    return memcmp(srcStorage, dstStorage, sizeof(srcStorage)) == 0;
}

int main(void) {
    if (!test_copy_dimensions_only()) {
        fprintf(stderr, "test_copy_dimensions_only failed\n");
        return 1;
    }
    if (!test_copy_bitmap_same_stride()) {
        fprintf(stderr, "test_copy_bitmap_same_stride failed\n");
        return 1;
    }
    if (!test_copy_bitmap_padded()) {
        fprintf(stderr, "test_copy_bitmap_padded failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
