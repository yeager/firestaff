#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "firestaff_graphics_dat_reader.h"

/* LZW stream for CLEAR, literal 0, END.  It decodes to one packed byte,
 * which is intentionally shorter than a 4x4 source bitmap's eight bytes. */
static const uint8_t kShortLzw[] = { 0x00, 0x01, 0x04, 0x04, 0x00 };

int main(void)
{
    FS_GraphicsDat gfx;
    uint8_t pixels[16];
    int width = 0;
    int height = 0;
    int result;

    memset(&gfx, 0, sizeof(gfx));
    gfx.raw_data = kShortLzw;
    gfx.raw_size = (int)sizeof(kShortLzw);
    gfx.graphic_count = 1;
    gfx.loaded = 1;
    gfx.entries[0].offset = 0;
    gfx.entries[0].compressed_size = (int)sizeof(kShortLzw);
    gfx.entries[0].width = 4;
    gfx.entries[0].height = 4;

    result = fs_gfx_extract_bitmap(&gfx, 0, pixels, (int)sizeof(pixels),
                                   &width, &height);
    if (result != -1 || width != 4 || height != 4) {
        fprintf(stderr, "FAIL: short source surface was admitted (%d, %dx%d)\n",
                result, width, height);
        return 1;
    }

    result = fs_gfx_extract_bitmap(&gfx, 0, NULL, 0, &width, &height);
    if (result != -1) {
        fprintf(stderr, "FAIL: size-only query admitted short source surface (%d)\n",
                result);
        return 1;
    }

    puts("PASS dm1 graphics reader rejects incomplete source surfaces");
    return 0;
}
