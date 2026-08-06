/* Product-boundary regression: a retired Firestaff word-square fixture is
 * not DM2 media and must never be accepted by the shipped loader. */
#include "dm2_v1_dungeon_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void put16le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)(v >> 8);
}

int main(void)
{
    enum { header_size = 44, descriptor_bytes = 28 * 16,
           tile_start = header_size + descriptor_bytes };
    uint8_t bytes[tile_start + 12];
    DM2_V1_DungeonData dungeon;

    memset(bytes, 0, sizeof(bytes));
    /* This is deliberately the former fixture layout: no native G1 magic,
     * a 3x2 word-square map and six local tile words. */
    bytes[6] = 1u;
    put16le(bytes + header_size + 4, (uint16_t)((2u << 5) | 1u));
    put16le(bytes + header_size + 12, 3u);
    put16le(bytes + header_size + 14, 2u);
    put16le(bytes + tile_start, 0x0021u);

    if (dm2_v1_dungeon_load(&dungeon, bytes, (int)sizeof(bytes)) != -1 ||
        dungeon.raw_data != NULL) {
        fprintf(stderr, "FAIL: product loader accepted word-square fixture\n");
        dm2_v1_dungeon_free(&dungeon);
        return 1;
    }
    puts("PASS: product loader rejects retired word-square fixture");
    return 0;
}
