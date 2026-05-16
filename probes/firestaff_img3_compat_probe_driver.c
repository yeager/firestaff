
#include <stdio.h>
#include "image_backend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static void F0689_IMG_ExpandGraphicToBitmap_Compat(const unsigned char* src, unsigned char* dst) {
    IMG3_Compat_ExpandFromSource(src, dst);
}

int main(void) {
    unsigned char src1[8] = {0x02, 0x00, 0x01, 0x00, 0x12, 0x34, 0x56, 0x11};
    unsigned char dst1[2] = {0x00, 0x00};
    unsigned char src2[9] = {0x03, 0x00, 0x02, 0x00, 0x12, 0x34, 0x56, 0x91, 0xE1};
    unsigned char dst2[4] = {0x00, 0x00, 0x00, 0x00};

    F0689_IMG_ExpandGraphicToBitmap_Compat(src1, dst1);
    F0689_IMG_ExpandGraphicToBitmap_Compat(src2, dst2);

    if (!(dst1[0] == 0x22 && dst1[1] == 0x00)) return 1;
    if (!(dst2[0] == 0x22 && dst2[1] == 0x20 && dst2[2] == 0x22 && dst2[3] == 0x20)) return 2;
    puts("ok");
    return 0;
}
