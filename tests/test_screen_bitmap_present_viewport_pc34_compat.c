#include <stdio.h>
#include <string.h>

#include "screen_bitmap_present_pc34_compat.h"

static void write_u16_le(unsigned char* p, unsigned short value) {
    p[0] = (unsigned char)(value & 0xFF);
    p[1] = (unsigned char)((value >> 8) & 0xFF);
}

static unsigned char get_nibble(const unsigned char* bitmap, unsigned short width, unsigned short x, unsigned short y) {
    const unsigned short bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
    const unsigned char packed = bitmap[((unsigned long)y * bytesPerRow) + (x >> 1)];
    return (x & 1) ? (unsigned char)(packed & 0x0F) : (unsigned char)(packed >> 4);
}

static void set_nibble(unsigned char* bitmap, unsigned short width, unsigned short x, unsigned short y, unsigned char color) {
    const unsigned short bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
    unsigned char* byte = &bitmap[((unsigned long)y * bytesPerRow) + (x >> 1)];
    color &= 0x0F;
    if (x & 1) {
        *byte = (unsigned char)((*byte & 0xF0) | color);
    } else {
        *byte = (unsigned char)((*byte & 0x0F) | (unsigned char)(color << 4));
    }
}

int main(void) {
    unsigned char viewportStorage[4 + (224U * 136U / 2U)];
    unsigned char screenStorage[4 + (320U * 200U / 2U)];
    unsigned char* viewportBitmap = viewportStorage + 4;
    unsigned char* screenBitmap = screenStorage + 4;
    struct ScreenBitmapPresentResult_Compat result;

    memset(viewportStorage, 0, sizeof(viewportStorage));
    memset(screenStorage, 0, sizeof(screenStorage));
    memset(&result, 0, sizeof(result));
    write_u16_le(viewportBitmap - 4, 224);
    write_u16_le(viewportBitmap - 2, 136);
    write_u16_le(screenBitmap - 4, 320);
    write_u16_le(screenBitmap - 2, 200);
    set_nibble(viewportBitmap, 224, 0, 0, 3);
    set_nibble(viewportBitmap, 224, 223, 135, 9);
    if (!F9006_SCREEN_OverlayViewportBitmapOnScreen_Compat(viewportBitmap, screenBitmap, 0, &result)) {
        fprintf(stderr, "viewport overlay failed\n");
        return 1;
    }
    if (result.composedX != 0 || result.composedY != 33 || result.sourceWidth != 224 || result.sourceHeight != 136) {
        fprintf(stderr, "bad viewport compose result x=%u y=%u w=%u h=%u\n",
                (unsigned int)result.composedX,
                (unsigned int)result.composedY,
                (unsigned int)result.sourceWidth,
                (unsigned int)result.sourceHeight);
        return 1;
    }
    if (get_nibble(screenBitmap, 320, 0, 33) != 3 || get_nibble(screenBitmap, 320, 223, 168) != 9) {
        fprintf(stderr, "viewport pixels did not land at DM1 screen rect\n");
        return 1;
    }
    if (get_nibble(screenBitmap, 320, 48, 32) != 0) {
        fprintf(stderr, "old centred compose position was written unexpectedly\n");
        return 1;
    }
    printf("probe=screen_bitmap_present_viewport_pc34_compat\n");
    printf("sourceEvidence=COORD.C:1693-1698 fixes PC viewport origin x=0 y=33; DEFS.H:2478-2484 fixes viewport byte-width 112 (=224 px) and height 136; DUNVIEW.C:2998-3000 stamps the viewport bitmap dimensions before draw completion.\n");
    printf("viewportComposedX=%u\n", (unsigned int)result.composedX);
    printf("viewportComposedY=%u\n", (unsigned int)result.composedY);
    printf("viewportSourceWidth=%u\n", (unsigned int)result.sourceWidth);
    printf("viewportSourceHeight=%u\n", (unsigned int)result.sourceHeight);
    return 0;
}
