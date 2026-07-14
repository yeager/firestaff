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

static void write_vector(uint8_t *p, int32_t x, int32_t y, int32_t z)
{
    wb32(p, (uint32_t)x);
    wb32(p + 4, (uint32_t)y);
    wb32(p + 8, (uint32_t)z);
}

int main(void)
{
    enum { total_blocks = 20, structure1_blocks = 17, structure3_block = 19 };
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * total_blocks];
    uint8_t *structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    uint8_t *structure3 = dgn + NEXUS_DGN_BLOCK_SIZE * structure3_block;
    uint8_t *entry = structure3 + 8;
    uint8_t *vertices = entry + NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES;
    uint8_t *faces = vertices + 4 * 12;
    uint8_t *normals = faces + 3 * 12;
    Nexus_V1_Level level;
    Nexus_V1_DgnStructure3EdgeReceipt receipt;

    memset(dgn, 0, sizeof(dgn));
    wb16(dgn + 0x0c, 1U);
    wb16(dgn + 0x0e, structure1_blocks);
    wb32(dgn + 0x10, 0x40U + NEXUS_DGN_STRUCTURE1B_BYTES);
    wb16(dgn + 0x1c, structure3_block);
    wb16(dgn + 0x1e, 1U);
    structure1[2] = NEXUS_MAX_MAP_SIZE;
    structure1[3] = NEXUS_MAX_MAP_SIZE;
    wb32(structure1 + 0x14, 0x40U);

    wb32(structure3, 1U);
    wb32(structure3 + 4, 8U);
    wb32(entry, 0x100U);
    wb16(entry + 4, 4U);
    wb16(entry + 6, 3U);
    wb32(entry + 8, 48U);
    wb32(entry + 16, 96U);
    wb32(entry + 20, 132U);

    write_vector(vertices, 0, 0, 0);
    write_vector(vertices + 12, 65536, 0, 0);
    write_vector(vertices + 24, 65536, 65536, 0);
    write_vector(vertices + 36, 0, 65536, 0);

    /* Two triangles share edge 0-2; the third has a raw degenerate 0-0 edge. */
    wb16(faces, 0U); wb16(faces + 2, 1U); wb16(faces + 4, 2U); wb16(faces + 6, 2U);
    wb16(faces + 12, 0U); wb16(faces + 14, 2U); wb16(faces + 16, 3U); wb16(faces + 18, 3U);
    wb16(faces + 24, 0U); wb16(faces + 26, 0U); wb16(faces + 28, 1U); wb16(faces + 30, 1U);
    for (int normal = 0; normal < 3; ++normal) {
        write_vector(normals + normal * 12, 0, 0, 65536);
    }

    if (nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 0) != 0 ||
        nexus_v1_level_structure3_edge_receipt(&level, &receipt) != 0 ||
        !receipt.face_receipt_valid || !receipt.topology_measurement_complete ||
        !receipt.valid || receipt.edge_count != 9 ||
        receipt.unique_edge_count != 6 || receipt.single_use_edge_count != 4 ||
        receipt.shared_edge_count != 1 || receipt.nonmanifold_edge_count != 1 ||
        receipt.degenerate_edge_count != 1 || receipt.maximum_edge_use_count != 3 ||
        receipt.topology_semantics_proven) {
        fprintf(stderr,
                "Structure3 edge receipt regression failed: valid=%d edges=%d unique=%d single=%d shared=%d nonmanifold=%d degenerate=%d max=%d\n",
                receipt.valid, receipt.edge_count, receipt.unique_edge_count,
                receipt.single_use_edge_count, receipt.shared_edge_count,
                receipt.nonmanifold_edge_count, receipt.degenerate_edge_count,
                receipt.maximum_edge_use_count);
        return 1;
    }

    puts("Structure3 edge receipt regression passed");
    return 0;
}
