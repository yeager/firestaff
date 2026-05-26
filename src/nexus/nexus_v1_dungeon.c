
#include "nexus_v1_dungeon.h"
#include <string.h>
#include <stdio.h>

static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static uint16_t rb16(const uint8_t *p) { return ((uint16_t)p[0]<<8)|p[1]; }

int nexus_v1_level_load(Nexus_V1_Level *level, const uint8_t *data, int size, int level_index) {
    if (!level || !data || size < 64) return -1;
    memset(level, 0, sizeof(*level));

    /*
     * DGN file structure (Nexus Saturn LEV00-LEV15, 147-321 KB each):
     *   16-byte LEV header block may contain width/height + metadata.
     *   Square grid follows (32x32 uint16_t, little-endian, 2048 bytes).
     *   Remaining bytes: 3D wall/floor geometry + thing list (provenance-locked).
     *
     * Decodes both possible header layouts:
     *   - Layout A (LE): bytes 0-1 = width, 2-3 = height (big-endian read)
     *   - Layout B (DM1-style): raw 32x32 grid at offset 0
     * Source: DM1 DUNGEON.C F0001 grid parsing; Saturn SH-2 big-endian
     * note from nexus_v1_dmdf_model.c; docs/nexus_squares.md.
     */

    /* --- Layout A: width/height header at byte 0-3 (big-endian u16) --- */
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
            printf("Nexus level %d: %dx%d, geometry=%d bytes [Layout A: w/h header at 0]\n",
                   level_index, level->width, level->height, level->geometry_size);
            return 0;
        }
    }

    /* --- Layout B: raw 32x32 grid at offset 0 (DM1-style fallback) --- */
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
            printf("Nexus level %d: 32x32, geometry=%d bytes [Layout B: raw grid at 0]\n",
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

