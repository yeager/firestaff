#include <stdio.h>
#include <string.h>

#include "bitmap_call_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_expand_if_present_same_stride(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char dst[2] = {0x00, 0x00};
    IMG_Compat_ExpandToBitmapIfPresent(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x00;
}

static int test_expand_if_present_null_is_safe(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    IMG_Compat_ExpandToBitmapIfPresent(src, NULL);
    return 1;
}

static int test_expand_required_padded(void) {
    unsigned char src[9] = {0x03, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE1};
    unsigned char dst[4] = {0x00, 0x00, 0x00, 0x00};
    IMG_Compat_ExpandToBitmapRequired(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x20 && dst[2] == 0x22 && dst[3] == 0x20;
}

int main(void) {
    if (!test_expand_if_present_same_stride()) {
        fprintf(stderr, "test_expand_if_present_same_stride failed\n");
        return 1;
    }
    if (!test_expand_if_present_null_is_safe()) {
        fprintf(stderr, "test_expand_if_present_null_is_safe failed\n");
        return 1;
    }
    if (!test_expand_required_padded()) {
        fprintf(stderr, "test_expand_required_padded failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
