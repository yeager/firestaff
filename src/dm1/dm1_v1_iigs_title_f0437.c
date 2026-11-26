#include "dm1_v1_iigs_title_f0437.h"

#include <string.h>

/* ReDMCSB BLTSHRNK.C:F0129 MEDIA351 (G20E/G21E): the Apple IIGS path
 * computes 16.16 source steps from a 10-bit fixed-point quotient, starts each
 * axis at zero, and advances the source accumulator once per destination
 * pixel. Retaining that quotient truncation matters for narrow zoom frames. */
static unsigned dm1_v1_iigs_f0129_sample(unsigned destination_index,
                                         unsigned source_extent,
                                         unsigned destination_extent) {
    unsigned step;
    if (destination_extent == 0u) return 0u;
    step = ((source_extent << 10u) / destination_extent) << 6u;
    return (destination_index * step) >> 16u;
}

int dm1_v1_iigs_title_f0437_compose(const uint8_t title[320u*200u],
    unsigned frame, uint8_t screen[320u*200u], uint16_t palette_rgb4[16]) {
    unsigned width,height,x,y,left,top;
    if (!title || !screen || !palette_rgb4 || frame>=DM1_V1_IIGS_TITLE_ZOOM_FRAMES)
        return 0;

    /* ReDMCSB TITLE.C:F0437, MEDIA351 (G20E/G21E): palette before the
     * 18-step C001 title zoom. RGB4 values are the native IIGS registers
     * in DEFS.H. TITLE.C prepares each 320x80 zoom bitmap through F0129. */
    for (x=0; x<16u; ++x) palette_rgb4[x]=0x0009u;
    palette_rgb4[3]=0x0a82u; palette_rgb4[4]=0x0842u;
    palette_rgb4[5]=0x0ca2u; palette_rgb4[6]=0x0a62u;
    palette_rgb4[8]=0x0ff4u; palette_rgb4[15]=0x0f00u;
    width=48u+16u*frame; height=12u+4u*frame;
    left=(320u-width)/2u; top=(160u-height)/2u;
    memset(screen,0,320u*200u); /* ReDMCSB clears the screen to palette index 0. */
    for (y=0; y<height; ++y) for (x=0; x<width; ++x) {
        unsigned sx = dm1_v1_iigs_f0129_sample(x, 320u, width);
        unsigned sy = dm1_v1_iigs_f0129_sample(y, 80u, height);
        screen[(top+y)*320u+left+x]=title[sy*320u+sx];
    }
    return 1;
}
