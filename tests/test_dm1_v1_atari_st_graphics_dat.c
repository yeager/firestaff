#include "dm1_v1_atari_st_graphics_dat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    enum { header = 2 + DM1_V1_ATARI_ST_GRAPHICS_COUNT * 4, payload = 3 };
    unsigned char bytes[header + payload * DM1_V1_ATARI_ST_GRAPHICS_COUNT];
    DM1_V1_AtariStGraphicsDat dat;
    unsigned char out[3];
    unsigned int i;

    memset(bytes, 0, sizeof(bytes));
    bytes[0] = (unsigned char)(DM1_V1_ATARI_ST_GRAPHICS_COUNT >> 8);
    bytes[1] = (unsigned char)DM1_V1_ATARI_ST_GRAPHICS_COUNT;
    for (i = 0; i < DM1_V1_ATARI_ST_GRAPHICS_COUNT; ++i) {
        size_t comp = 2u + (size_t)i * 2u;
        size_t expanded = 2u + DM1_V1_ATARI_ST_GRAPHICS_COUNT * 2u +
                          (size_t)i * 2u;
        bytes[comp] = bytes[expanded] = 0;
        bytes[comp + 1u] = bytes[expanded + 1u] = payload;
        bytes[header + i * payload] = (unsigned char)i;
        bytes[header + i * payload + 1u] = 0x5a;
        bytes[header + i * payload + 2u] = 0xa5;
    }
    assert(dm1_v1_atari_st_graphics_open(bytes, sizeof(bytes), &dat));
    assert(dat.records[574].offset == header + 574u * payload);
    assert(dm1_v1_atari_st_graphics_read(&dat, 574, out, sizeof(out)) == 3);
    assert(out[0] == (unsigned char)574 && out[1] == 0x5a && out[2] == 0xa5);
    assert(!dm1_v1_atari_st_graphics_open(bytes, sizeof(bytes) - 1u, &dat));
    puts("PASS dm1_v1_atari_st_graphics_dat");
    return 0;
}
