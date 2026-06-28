
#include "nexus_v1_dungeon.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>

static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static uint16_t rb16(const uint8_t *p) { return ((uint16_t)p[0]<<8)|p[1]; }

static int nexus_v1_decode_structure1b_collision_ref(const uint8_t *cell) {
    if (!cell) {
        return 0;
    }
    return (int)((((unsigned)cell[6] & 0x0FU) << 8) | (unsigned)cell[7]);
}

static int nexus_v1_decode_structure1b_cell(const uint8_t *cell) {
    uint16_t flags;
    unsigned square_type;
    unsigned collision;
    if (!cell) {
        return 0;
    }
    flags = rb16(cell);
    square_type = (unsigned)(cell[6] & 0x1FU);
    collision = (unsigned)nexus_v1_decode_structure1b_collision_ref(cell);
    if (collision == 0x0FFFU) {
        return 0; /* wall / cannot enter */
    }
    if (square_type == 0U || square_type == 1U ||
        (square_type >= 2U && square_type <= 14U) ||
        square_type == 21U || square_type == 22U) {
        return (int)square_type;
    }
    if ((flags & 0x0001U) != 0) {
        return 8; /* door present */
    }
    return 1; /* free corridor/floor */
}

int nexus_v1_dgn_geometry_info(Nexus_V1_DgnGeometryInfo *out_info,
                               const uint8_t *data,
                               int size) {
    Nexus_V1_DgnGeometryInfo info;
    uint16_t structure1_block;
    uint16_t structure1_blocks;
    uint32_t structure1_useful;
    int structure1_offset;
    int structure1_size;
    const uint8_t *structure1;
    uint32_t structure1b_rel;
    int structure1b_offset;
    int geometry_offset;
    int geometry_size;
    unsigned char seen_refs[4096];
    int y;
    int x;

    if (out_info) {
        memset(out_info, 0, sizeof(*out_info));
    }
    if (!out_info || !data || size < NEXUS_DGN_BLOCK_SIZE) {
        return -1;
    }

    memset(&info, 0, sizeof(info));
    structure1_block = rb16(data + 0x0C);
    structure1_blocks = rb16(data + 0x0E);
    structure1_useful = rb32(data + 0x10);
    if (structure1_block == 0 || structure1_blocks == 0 ||
        structure1_blocks > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE) ||
        structure1_block > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE)) {
        return -1;
    }

    structure1_offset = (int)structure1_block * NEXUS_DGN_BLOCK_SIZE;
    structure1_size = (int)structure1_blocks * NEXUS_DGN_BLOCK_SIZE;
    if (structure1_offset < NEXUS_DGN_BLOCK_SIZE ||
        structure1_offset + 0x38 > size ||
        structure1_size <= 0 ||
        structure1_offset + structure1_size > size ||
        structure1_useful > (uint32_t)structure1_size) {
        return -1;
    }

    structure1 = data + structure1_offset;
    structure1b_rel = rb32(structure1 + 0x14);
    if (structure1[2] != 0x40 || structure1[3] != 0x40 ||
        structure1b_rel > (uint32_t)structure1_size ||
        structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES > structure1_useful) {
        return -1;
    }

    structure1b_offset = structure1_offset + (int)structure1b_rel;
    geometry_offset = structure1b_offset + NEXUS_DGN_STRUCTURE1B_BYTES;
    geometry_size = (int)structure1_useful -
                    ((int)structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES);
    if (structure1b_offset < structure1_offset ||
        geometry_offset > size ||
        geometry_size < 0 ||
        geometry_offset + geometry_size > size) {
        return -1;
    }

    /*
     * DMWeb DGN Structure1B source-lock:
     * bytes 5..7 pack two 12-bit values; Firestaff's current renderer needs
     * the low collision descriptor reference to be bounded before a real
     * Structure1C/mesh reader can replace the procedural fallback.
     */
    memset(seen_refs, 0, sizeof(seen_refs));
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            int off = structure1b_offset +
                      ((y * NEXUS_MAX_MAP_SIZE + x) *
                       NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
            int ref = nexus_v1_decode_structure1b_collision_ref(data + off);
            if (ref != 0 && ref != 0x0FFF) {
                info.collision_ref_count++;
                if (!seen_refs[ref]) {
                    seen_refs[ref] = 1U;
                    info.collision_ref_unique_count++;
                }
                if (ref > info.max_collision_ref) {
                    info.max_collision_ref = ref;
                }
            }
        }
    }

    info.dmweb_container = 1;
    info.structure1_offset = structure1_offset;
    info.structure1_size = structure1_size;
    info.structure1_useful_size = (int)structure1_useful;
    info.structure1b_offset = structure1b_offset;
    info.structure1b_size = NEXUS_DGN_STRUCTURE1B_BYTES;
    info.geometry_offset = geometry_offset;
    info.geometry_size = geometry_size;
    if (geometry_size > 0 &&
        info.max_collision_ref <=
            (geometry_size / NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES)) {
        info.mesh_ready = 1;
    }

    *out_info = info;
    return 0;
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
        Nexus_V1_DgnGeometryInfo info;
        if (nexus_v1_dgn_geometry_info(&info, data, size) == 0) {
            int y, x;
            level->width = NEXUS_MAX_MAP_SIZE;
            level->height = NEXUS_MAX_MAP_SIZE;
            for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
                for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                    int off = info.structure1b_offset +
                              ((y * NEXUS_MAX_MAP_SIZE + x) *
                               NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
                    level->squares[y][x] = (uint8_t)nexus_v1_decode_structure1b_cell(data + off);
                }
            }
            level->has_3d_geometry = 1;
            level->geometry_offset = info.geometry_offset;
            level->geometry_size = size - info.geometry_offset;
            level->geometry_info = info;
            printf("Nexus level %d: 64x64 Structure1B, payload=%d bytes, mesh_span=%d bytes, refs=%d/%d [DMWeb DGN]\n",
                   level_index, level->geometry_size, info.geometry_size,
                   info.collision_ref_unique_count, info.collision_ref_count);
            return 0;
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
