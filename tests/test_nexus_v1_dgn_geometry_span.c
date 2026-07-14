#include "nexus_v1_dungeon.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void wb16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)value;
}

static void wb32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}

int main(void)
{
    enum { geometry_bytes = 64, structure1_blocks = 17, total_blocks = 20 };
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * total_blocks];
    uint8_t *structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    Nexus_V1_Level level;
    int expected_geometry_offset = NEXUS_DGN_BLOCK_SIZE + 0x40 +
        NEXUS_DGN_STRUCTURE1B_BYTES;

    memset(dgn, 0, sizeof(dgn));
    wb16(dgn + 0x0c, 1U);
    wb16(dgn + 0x0e, structure1_blocks);
    wb32(dgn + 0x10, 0x40U + NEXUS_DGN_STRUCTURE1B_BYTES + geometry_bytes);
    wb16(dgn + 0x1c, 19U);
    wb16(dgn + 0x1e, 1U);
    structure1[2] = NEXUS_MAX_MAP_SIZE;
    structure1[3] = NEXUS_MAX_MAP_SIZE;
    wb32(structure1 + 0x14, 0x40U);

    if (nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 0) != 0 ||
        level.geometry_offset != expected_geometry_offset ||
        level.geometry_size != geometry_bytes ||
        level.geometry_size != level.geometry_info.geometry_size ||
        !level.structure3_payload.valid ||
        level.structure3_payload.byte_size != NEXUS_DGN_BLOCK_SIZE) {
        fprintf(stderr, "DGN geometry span regression failed\n");
        return 1;
    }

    puts("DGN geometry span regression passed");
    return 0;
}
