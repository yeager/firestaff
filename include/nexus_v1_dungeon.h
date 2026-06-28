
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
#define NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES 4

typedef struct {
    int dmweb_container;
    int structure1_offset;
    int structure1_size;
    int structure1_useful_size;
    int structure1b_offset;
    int structure1b_size;
    int geometry_offset;
    int geometry_size;
    int collision_ref_count;
    int collision_ref_unique_count;
    int max_collision_ref;
    int mesh_ready;
} Nexus_V1_DgnGeometryInfo;

typedef struct {
    int width, height;
    uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    int thing_count;
    int creature_count;
    int has_3d_geometry;
    int geometry_offset;
    int geometry_size;
    Nexus_V1_DgnGeometryInfo geometry_info;
} Nexus_V1_Level;

int nexus_v1_level_load(Nexus_V1_Level *level, const uint8_t *data, int size, int level_index);
int nexus_v1_level_get_square(const Nexus_V1_Level *level, int x, int y);
int nexus_v1_dgn_geometry_info(Nexus_V1_DgnGeometryInfo *out_info,
                               const uint8_t *data,
                               int size);

#endif
