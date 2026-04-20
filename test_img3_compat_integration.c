#include <stdio.h>
#include <string.h>

#include "image_expand_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_same_stride_wrapper(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char dst[2] = {0x00, 0x00};
    F0689_IMG_ExpandGraphicToBitmap_Compat(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x00;
}

static int test_padded_wrapper(void) {
    unsigned char src[9] = {0x03, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE1};
    unsigned char dst[4] = {0x00, 0x00, 0x00, 0x00};
    F0689_IMG_ExpandGraphicToBitmap_Compat(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x20 && dst[2] == 0x22 && dst[3] == 0x20;
}

int main(void) {
    if (!test_same_stride_wrapper()) {
        fprintf(stderr, "test_same_stride_wrapper failed\n");
        return 1;
    }
    if (!test_padded_wrapper()) {
        fprintf(stderr, "test_padded_wrapper failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
