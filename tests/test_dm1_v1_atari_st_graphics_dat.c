#include "dm1_v1_atari_st_graphics_dat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    enum { header = 2 + DM1_V1_ATARI_ST_GRAPHICS_COUNT * 4 };
    unsigned char bytes[header + 6 + 3 * (DM1_V1_ATARI_ST_GRAPHICS_COUNT - 1)];
    DM1_V1_AtariStGraphicsDat dat;
    unsigned char out[3];
    unsigned char pixels[4];
    uint16_t width = 0u;
    uint16_t height = 0u;
    size_t cursor = header;
    unsigned int i;

    memset(bytes, 0, sizeof(bytes));
    bytes[0] = (unsigned char)(DM1_V1_ATARI_ST_GRAPHICS_COUNT >> 8);
    bytes[1] = (unsigned char)DM1_V1_ATARI_ST_GRAPHICS_COUNT;
    for (i = 0; i < DM1_V1_ATARI_ST_GRAPHICS_COUNT; ++i) {
        const size_t payload = i == 0u ? 6u : 3u;
        size_t comp = 2u + (size_t)i * 2u;
        size_t expanded = 2u + DM1_V1_ATARI_ST_GRAPHICS_COUNT * 2u +
                          (size_t)i * 2u;
        bytes[comp] = bytes[expanded] = 0;
        bytes[comp + 1u] = bytes[expanded + 1u] = payload;
        if (i == 0u) {
            /* IMG1: 2x2 big-endian dimensions, then two literal runs. */
            bytes[cursor + 0u] = 0;
            bytes[cursor + 1u] = 2;
            bytes[cursor + 2u] = 0;
            bytes[cursor + 3u] = 2;
            bytes[cursor + 4u] = 0x11;
            bytes[cursor + 5u] = 0x12;
        } else {
            bytes[cursor] = (unsigned char)i;
            bytes[cursor + 1u] = 0x5a;
            bytes[cursor + 2u] = 0xa5;
        }
        cursor += payload;
    }
    assert(dm1_v1_atari_st_graphics_open(bytes, sizeof(bytes), &dat));
    assert(dat.records[562].offset == header + 6u + 561u * 3u);
    assert(dm1_v1_atari_st_graphics_read(&dat, 562, out, sizeof(out)) == 3);
    assert(out[0] == (unsigned char)562 && out[1] == 0x5a && out[2] == 0xa5);
    assert(dm1_v1_atari_st_graphics_decode(
               &dat, 0, pixels, sizeof(pixels), &width, &height) == 1);
    assert(width == 2u && height == 2u);
    assert(pixels[0] == 1u && pixels[1] == 1u &&
           pixels[2] == 2u && pixels[3] == 2u);
    assert(!dm1_v1_atari_st_graphics_open(bytes, sizeof(bytes) - 1u, &dat));
    puts("PASS dm1_v1_atari_st_graphics_dat");
    return 0;
}
