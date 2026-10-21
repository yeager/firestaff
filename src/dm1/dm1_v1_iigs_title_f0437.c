#include "dm1_v1_iigs_title_f0437.h"

#include <string.h>

int dm1_v1_iigs_title_f0437_compose(const uint8_t title[320u*200u],
    unsigned frame, uint8_t screen[320u*200u], uint16_t palette_rgb4[16]) {
    unsigned width,height,x,y,left,top;
    if (!title || !screen || !palette_rgb4 || frame>=DM1_V1_IIGS_TITLE_ZOOM_FRAMES)
        return 0;

    /* ReDMCSB TITLE.C F0437, MEDIA351 (G20E/G21E): palette before the
     * 18-step C001 title zoom. RGB4 values are the native IIGS registers
     * in DEFS.H; the GS F0129 sampling algorithm remains a known gap. */
    for (x=0; x<16u; ++x) palette_rgb4[x]=0x0009u;
    palette_rgb4[3]=0x0a82u; palette_rgb4[4]=0x0842u;
    palette_rgb4[5]=0x0ca2u; palette_rgb4[6]=0x0a62u;
    palette_rgb4[8]=0x0ff4u; palette_rgb4[15]=0x0f00u;
    width=48u+16u*frame; height=12u+4u*frame;
    left=(320u-width)/2u; top=(160u-height)/2u;
    memset(screen,0,320u*200u); /* ReDMCSB clears the screen to palette index 0. */
    for (y=0; y<height; ++y) for (x=0; x<width; ++x) {
        unsigned sx=(x*320u)/width, sy=(y*80u)/height;
        screen[(top+y)*320u+left+x]=title[sy*320u+sx];
    }
    return 1;
}
