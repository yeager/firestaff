
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

    /* DGN file structure (to be reverse-engineered):
     * The files are 147-321 KB — much larger than DM1's ~20 KB per level.
     * This suggests embedded 3D geometry (wall polygons, floor/ceiling meshes).
     *
     * Hypothesis based on DM1 structure:
     * - First section: map grid (32x32 uint16 = 2048 bytes)
     * - Second section: thing list
     * - Third section: 3D geometry data (bulk of the file)
     *
     * Try reading first 2 bytes as width/height: */
    {
        uint16_t w = rb16(data);
        uint16_t h = rb16(data + 2);
        if (w > 0 && w <= 32 && h > 0 && h <= 32) {
            level->width = w;
            level->height = h;
            /* Read square grid */
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
        } else {
            /* Alternative: grid might be elsewhere in the file */
            level->width = 32;
            level->height = 32;
            level->has_3d_geometry = 1;
            level->geometry_size = size;
        }
    }

    /* ENHANCED: try multiple DGN header interpretations */
    /* Try DM1-style: first 4 bytes = header, then 16-bit square entries */
    if (size >= 2048 + 4) {
        /* Alternate: fixed 32x32 grid at offset 0 */
        int grid_bytes = 32 * 32 * 2;
        if (size > grid_bytes) {
            for (int gy = 0; gy < 32; gy++)
                for (int gx = 0; gx < 32; gx++) {
                    int off = (gy * 32 + gx) * 2;
                    level->squares[gy][gx] = rb16(data + off) & 0x1F;
                }
            level->width = 32;
            level->height = 32;
            level->geometry_offset = grid_bytes;
            level->geometry_size = size - grid_bytes;
        }
    }

    printf("Nexus level %d: %dx%d, geometry=%d bytes\n",
        level_index, level->width, level->height, level->geometry_size);
    return 0;
}

int nexus_v1_level_get_square(const Nexus_V1_Level *level, int x, int y) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return 0; /* wall */
    return level->squares[y][x];
}

