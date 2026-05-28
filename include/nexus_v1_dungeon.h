
#ifndef NEXUS_V1_DUNGEON_H
#define NEXUS_V1_DUNGEON_H
#include <stdint.h>

/* DM Nexus dungeon level format (.DGN files).
 * Source-lock: DMWeb "Dungeon Master Nexus DGN files", fetched 2026-05-28.
 * Real LEV*.DGN files are 2048-byte block containers. Structure1 contains
 * Structure1B: a 64x64 grid with 8 bytes per cell (0x8000 bytes total).
 * The old Firestaff "raw 32x32 at offset 0" layout is retained only as a
 * synthetic-fixture fallback while older probes are retired. */

#define NEXUS_MAX_MAP_SIZE 64
#define NEXUS_DGN_BLOCK_SIZE 2048
#define NEXUS_DGN_STRUCTURE1B_BYTES 0x8000
#define NEXUS_DGN_STRUCTURE1B_CELL_BYTES 8

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
