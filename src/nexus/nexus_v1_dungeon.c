
#include "nexus_v1_dungeon.h"
#include <string.h>
#include <stdio.h>

static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static uint16_t rb16(const uint8_t *p) { return ((uint16_t)p[0]<<8)|p[1]; }

static int nexus_v1_decode_structure1b_cell(const uint8_t *cell) {
    uint16_t flags;
    unsigned collision;
    if (!cell) {
        return 0;
    }
    flags = rb16(cell);
    collision = ((unsigned)(cell[6] & 0x0F) << 8) | (unsigned)cell[7];
    if (collision == 0x0FFFU) {
        return 0; /* wall / cannot enter */
    }
    if ((flags & 0x0001U) != 0) {
        return 8; /* door present */
    }
    return 1; /* free corridor/floor */
}

int nexus_v1_level_load(Nexus_V1_Level *level, const uint8_t *data, int size, int level_index) {
    if (!level || !data || size < 64) return -1;
    memset(level, 0, sizeof(*level));

    /*
     * DMWeb source-lock:
     *   http://dmweb.free.fr/community/documentation/dungeon-master-nexus/dgn-files/
     *   DGN files are 2048-byte block containers. Header offsets at 0x0C,
     *   0x0E and 0x10 locate Structure1; Structure1 offset 0x14 locates
     *   Structure1B, always 0x8000 bytes: 64x64 cells, 8 bytes each.
     *   The old raw-32x32 reader below is a synthetic-fixture fallback only.
     */

    if (size >= NEXUS_DGN_BLOCK_SIZE) {
        uint16_t structure1_block = rb16(data + 0x0C);
        uint16_t structure1_blocks = rb16(data + 0x0E);
        uint32_t structure1_useful = rb32(data + 0x10);
        int structure1_offset = (int)structure1_block * NEXUS_DGN_BLOCK_SIZE;
        int structure1_size = (int)structure1_blocks * NEXUS_DGN_BLOCK_SIZE;

        if (structure1_block > 0 &&
            structure1_blocks > 0 &&
            structure1_offset >= NEXUS_DGN_BLOCK_SIZE &&
            structure1_offset + 0x38 <= size &&
            structure1_size > 0 &&
            structure1_offset + structure1_size <= size &&
            structure1_useful <= (uint32_t)structure1_size) {
            const uint8_t *structure1 = data + structure1_offset;
            uint32_t structure1b_rel = rb32(structure1 + 0x14);
            int structure1b_offset = structure1_offset + (int)structure1b_rel;

            if (structure1[2] == 0x40 && structure1[3] == 0x40 &&
                structure1b_rel <= (uint32_t)structure1_size &&
                structure1b_offset >= structure1_offset &&
                structure1b_offset + NEXUS_DGN_STRUCTURE1B_BYTES <= size) {
                int y, x;
                level->width = NEXUS_MAX_MAP_SIZE;
                level->height = NEXUS_MAX_MAP_SIZE;
                for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
                    for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                        int off = structure1b_offset +
                                  ((y * NEXUS_MAX_MAP_SIZE + x) *
                                   NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
                        level->squares[y][x] = (uint8_t)nexus_v1_decode_structure1b_cell(data + off);
                    }
                }
                level->has_3d_geometry = 1;
                level->geometry_offset = structure1b_offset + NEXUS_DGN_STRUCTURE1B_BYTES;
                level->geometry_size = size - level->geometry_offset;
                printf("Nexus level %d: 64x64 Structure1B, payload=%d bytes [DMWeb DGN]\n",
                       level_index, level->geometry_size);
                return 0;
            }
        }
    }

    /* --- Legacy synthetic fallback: width/height header at byte 0-3 --- */
    {
        uint16_t w = rb16(data);
        uint16_t h = rb16(data + 2);
        if (w > 0 && w <= 32 && h > 0 && h <= 32) {
            level->width = w;
            level->height = h;
            int grid_offset = 4;
            for (int y = 0; y < h && grid_offset + 2 <= size; y++) {
                for (int x = 0; x < w && grid_offset + 2 <= size; x++) {
                    level->squares[y][x] = rb16(data + grid_offset) & 0x1F;
                    grid_offset += 2;
                }
            }
            level->has_3d_geometry = 1;
            level->geometry_offset = grid_offset;
            level->geometry_size = size - grid_offset;
            printf("Nexus level %d: %dx%d, geometry=%d bytes [legacy synthetic w/h]\n",
                   level_index, level->width, level->height, level->geometry_size);
            return 0;
        }
    }

    /* --- Legacy synthetic fallback: raw 32x32 grid at offset 0 --- */
    {
        int grid_bytes = 32 * 32 * 2;
        if (size >= grid_bytes) {
            for (int gy = 0; gy < 32; gy++) {
                for (int gx = 0; gx < 32; gx++) {
                    int off = (gy * 32 + gx) * 2;
                    level->squares[gy][gx] = rb16(data + off) & 0x1F;
                }
            }
            level->width = 32;
            level->height = 32;
            level->has_3d_geometry = 1;
            level->geometry_offset = grid_bytes;
            level->geometry_size = size - grid_bytes;
            printf("Nexus level %d: 32x32, geometry=%d bytes [legacy synthetic raw grid]\n",
                   level_index, level->geometry_size);
            return 0;
        }
    }

    printf("Nexus level %d: could not parse DGN header (size=%d)\n",
           level_index, size);
    return -1;
}

int nexus_v1_level_get_square(const Nexus_V1_Level *level, int x, int y) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return 0; /* wall */
    return level->squares[y][x];
}
