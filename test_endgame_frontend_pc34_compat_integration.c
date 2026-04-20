#include <stdio.h>
#include <string.h>

#include "endgame_frontend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_endgame_credits_same_stride(void) {
    unsigned char src[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char dst[2] = {0x00, 0x00};
    ENDGAME_Compat_ExpandCreditsToScreenBitmap(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x00;
}

static int test_endgame_credits_padded(void) {
    unsigned char src[9] = {0x03, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE1};
    unsigned char dst[4] = {0x00, 0x00, 0x00, 0x00};
    ENDGAME_Compat_ExpandCreditsToScreenBitmap(src, dst);
    return dst[0] == 0x22 && dst[1] == 0x20 && dst[2] == 0x22 && dst[3] == 0x20;
}

int main(void) {
    if (!test_endgame_credits_same_stride()) {
        fprintf(stderr, "test_endgame_credits_same_stride failed\n");
        return 1;
    }
    if (!test_endgame_credits_padded()) {
        fprintf(stderr, "test_endgame_credits_padded failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
