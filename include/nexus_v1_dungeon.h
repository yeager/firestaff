
#ifndef NEXUS_V1_DUNGEON_H
#define NEXUS_V1_DUNGEON_H
#include <stdint.h>

/* DM Nexus dungeon level format (.DGN files).
 * 16 levels (LEV00-LEV15), 147-321 KB each.
 * Much larger than DM1 (33 KB total) due to 3D geometry data.
 * Likely contains: square grid + 3D wall/floor geometry + thing list. */

#define NEXUS_MAX_MAP_SIZE 32

typedef struct {
    int width, height;
    uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    int thing_count;
    int creature_count;
    int has_3d_geometry;
    int geometry_offset;
    int geometry_size;
} Nexus_V1_Level;

int nexus_v1_level_load(Nexus_V1_Level *level, const uint8_t *data, int size, int level_index);
int nexus_v1_level_get_square(const Nexus_V1_Level *level, int x, int y);

#endif

