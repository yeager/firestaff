
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

static uint8_t nexus_v1_decode_structure1b_wall_material(
    const uint8_t *cell, int wall_dir) {
    /* DMWeb DGN Structure1B: byte 3 holds north/east surface ids and
     * byte 4 holds south/west ids. The directional bit is the missing link
     * between the level's material references and the mesh command. */
    return cell[(wall_dir & 3) < 2 ? 3 : 4];
}

static uint16_t nexus_v1_decode_structure1b_mesh_ref(const uint8_t *cell) {
    return (uint16_t)((((unsigned)cell[5] << 4) |
                       ((unsigned)cell[6] >> 4)) & 0x0fffU);
}

static int nexus_v1_abs_i8(int value) {
    return value < 0 ? -value : value;
}

static void nexus_v1_decode_structure1f_descriptor(
    const uint8_t *src,
    Nexus_V1_DgnMeshDescriptor *dst) {
    int width;
    int height;

    if (!dst) return;
    memset(dst, 0, sizeof(*dst));
    if (!src) return;

    dst->x1 = (int8_t)src[0];
    dst->y1 = (int8_t)src[1];
    dst->x2 = (int8_t)src[2];
    dst->y2 = (int8_t)src[3];
    width = nexus_v1_abs_i8((int)dst->x2 - (int)dst->x1);
    height = nexus_v1_abs_i8((int)dst->y2 - (int)dst->y1);
    dst->width = width;
    dst->height = height;
    dst->area = width * height;
    dst->solid = dst->area > 0 ? 1 : 0;
    dst->valid = 1;
}

static uint8_t nexus_v1_decode_structure1b_floor_material(const uint8_t *cell) {
    return (uint8_t)((rb16(cell) >> 7) & 0x1fU);
}

static uint8_t nexus_v1_decode_structure1b_ceiling_material(
    const uint8_t *header, const uint8_t *cell) {
    unsigned selection = (rb16(cell) >> 1) & 3U;
    return selection == 0U ? 0U : header[8 + selection];
}

static int nexus_v1_decode_structure1b_cell(const uint8_t *cell) {
    uint16_t flags;
    unsigned collision;
    if (!cell) {
        return 0;
    }
    flags = rb16(cell);
    collision = (unsigned)nexus_v1_decode_structure1b_collision_ref(cell);
    if (collision == 0x0FFFU) {
        return 0; /* wall / cannot enter */
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
    unsigned char seen_mesh_refs[4096];
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
    memset(seen_mesh_refs, 0, sizeof(seen_mesh_refs));
    info.structure1f_descriptor_count =
        geometry_size > 0
            ? geometry_size / NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES
            : 0;
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            int off = structure1b_offset +
                      ((y * NEXUS_MAX_MAP_SIZE + x) *
                       NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
            int ref = nexus_v1_decode_structure1b_collision_ref(data + off);
            int mesh_ref = nexus_v1_decode_structure1b_mesh_ref(data + off);
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
            if (mesh_ref != 0 && mesh_ref != 0x0FFF) {
                info.mesh_ref_count++;
                if (!seen_mesh_refs[mesh_ref]) {
                    Nexus_V1_DgnMeshDescriptor descriptor;
                    seen_mesh_refs[mesh_ref] = 1U;
                    info.mesh_ref_unique_count++;
                    if (info.structure1f_first_ref == 0) {
                        info.structure1f_first_ref = mesh_ref;
                    }
                    if (mesh_ref < info.structure1f_descriptor_count) {
                        nexus_v1_decode_structure1f_descriptor(
                            data + geometry_offset +
                                (mesh_ref *
                                 NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES),
                            &descriptor);
                        if (descriptor.valid) {
                            info.structure1f_valid_descriptor_count++;
                            if (descriptor.solid) {
                                info.structure1f_solid_descriptor_count++;
                            }
                            if (descriptor.area > info.structure1f_max_area) {
                                info.structure1f_max_area = descriptor.area;
                            }
                        }
                    }
                }
                if (mesh_ref > info.max_mesh_ref) {
                    info.max_mesh_ref = mesh_ref;
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
            (geometry_size / NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES) &&
        info.max_mesh_ref <=
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
                    int ref = nexus_v1_decode_structure1b_collision_ref(data + off);
                    level->squares[y][x] = (uint8_t)nexus_v1_decode_structure1b_cell(data + off);
                    level->collision_refs[y][x] = (uint16_t)ref;
                    level->floor_material_refs[y][x] =
                        nexus_v1_decode_structure1b_floor_material(data + off);
                    level->ceiling_material_refs[y][x] =
                        nexus_v1_decode_structure1b_ceiling_material(
                            data + info.structure1_offset, data + off);
                    level->wall_material_refs[y][x][0] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 0);
                    level->wall_material_refs[y][x][1] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 1);
                    level->wall_material_refs[y][x][2] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 2);
                    level->wall_material_refs[y][x][3] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 3);
                    level->floor_heights[y][x] = (int8_t)data[off + 3];
                    level->floor_slopes[y][x] =
                        (uint8_t)((rb16(data + off) >> 4) & 3U);
                    level->floor_rotations[y][x] =
                        (uint8_t)(rb16(data + off) >> 14);
                    level->mesh_refs[y][x] =
                        nexus_v1_decode_structure1b_mesh_ref(data + off);
                }
            }
            {
                const uint8_t *sectors = data + info.geometry_offset;
                int sector_count = sectors[0] > 0 ? sectors[0] - 1 : 0;
                int mesh_descriptor_count =
                    info.geometry_size / NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES;
                if (sector_count > NEXUS_DGN_MAX_COLLISION_SECTORS - 1)
                    sector_count = NEXUS_DGN_MAX_COLLISION_SECTORS - 1;
                for (int sector = 1; sector <= sector_count; ++sector) {
                    const uint8_t *src = sectors + sector * 4;
                    Nexus_V1_DgnCollisionSector *dst =
                        &level->collision_sectors[sector];
                    dst->valid = 1;
                    dst->x1 = (int8_t)src[0]; dst->y1 = (int8_t)src[1];
                    dst->x2 = (int8_t)src[2]; dst->y2 = (int8_t)src[3];
                    dst->circle = src[3] == 0x80U;
                }
                if (mesh_descriptor_count > NEXUS_DGN_MAX_MESH_DESCRIPTORS - 1)
                    mesh_descriptor_count = NEXUS_DGN_MAX_MESH_DESCRIPTORS - 1;
                for (int ref = 1; ref < mesh_descriptor_count; ++ref) {
                    const uint8_t *src = sectors + ref * 4;
                    nexus_v1_decode_structure1f_descriptor(
                        src,
                        &level->mesh_descriptors[ref]);
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

int nexus_v1_level_get_collision_ref(const Nexus_V1_Level *level, int x, int y) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return 0x0fff;
    return (int)level->collision_refs[y][x];
}

int nexus_v1_level_get_material_ref(const Nexus_V1_Level *level, int x, int y,
                                    Nexus_V1_DgnRenderCommandKind kind,
                                    int wall_dir) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return -1;
    if (kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
        kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
        if (kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING)
            return level->ceiling_material_refs[y][x];
        return level->floor_material_refs[y][x];
    }
    return level->wall_material_refs[y][x][wall_dir & 3];
}

int nexus_v1_level_get_cell_geometry(const Nexus_V1_Level *level, int x, int y,
                                     Nexus_V1_DgnCellGeometry *out_cell) {
    Nexus_V1_DgnCellGeometry cell;
    int corner;

    if (!out_cell) return -1;
    memset(out_cell, 0, sizeof(*out_cell));
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return -1;

    memset(&cell, 0, sizeof(cell));
    cell.square_type = level->squares[y][x];
    cell.collision_ref = level->collision_refs[y][x];
    cell.mesh_ref = level->mesh_refs[y][x];
    cell.floor_material_ref = level->floor_material_refs[y][x];
    cell.ceiling_material_ref = level->ceiling_material_refs[y][x];
    memcpy(cell.wall_material_refs, level->wall_material_refs[y][x],
           sizeof(cell.wall_material_refs));
    cell.floor_slope = level->floor_slopes[y][x];
    cell.floor_rotation = level->floor_rotations[y][x];
    for (corner = 0; corner < 4; ++corner)
        cell.floor_height[corner] = level->floor_heights[y][x];
    if (cell.floor_slope == 2U && x + 1 < level->width) {
        cell.floor_height[1] = cell.floor_height[2] =
            level->floor_heights[y][x + 1];
    } else if (cell.floor_slope == 3U && y + 1 < level->height) {
        cell.floor_height[2] = cell.floor_height[3] =
            level->floor_heights[y + 1][x];
    }
    for (corner = 0; corner < 4; ++corner)
        cell.ceiling_height[corner] = (int8_t)(cell.floor_height[corner] + 32);
    if (cell.collision_ref < NEXUS_DGN_MAX_COLLISION_SECTORS)
        cell.collision_sector = level->collision_sectors[cell.collision_ref];
    if (cell.mesh_ref < NEXUS_DGN_MAX_MESH_DESCRIPTORS)
        cell.mesh_descriptor = level->mesh_descriptors[cell.mesh_ref];
    *out_cell = cell;
    return 0;
}

static int nexus_v1_dgn_sign(int value) {
    return (value > 0) - (value < 0);
}

static int nexus_v1_dgn_segments_intersect(int ax, int ay, int bx, int by,
                                           int cx, int cy, int dx, int dy) {
    int ab_c = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    int ab_d = (bx - ax) * (dy - ay) - (by - ay) * (dx - ax);
    int cd_a = (dx - cx) * (ay - cy) - (dy - cy) * (ax - cx);
    int cd_b = (dx - cx) * (by - cy) - (dy - cy) * (bx - cx);
    return nexus_v1_dgn_sign(ab_c) != nexus_v1_dgn_sign(ab_d) &&
           nexus_v1_dgn_sign(cd_a) != nexus_v1_dgn_sign(cd_b);
}

static int nexus_v1_dgn_circle_blocks_step(const Nexus_V1_DgnCollisionSector *sector,
                                           int start_x, int start_y) {
    int radius = sector->x2 < 0 ? -sector->x2 : sector->x2;
    int dx = -start_x;
    int dy = -start_y;
    int length_sq = dx * dx + dy * dy;
    int t_num = (sector->x1 - start_x) * dx + (sector->y1 - start_y) * dy;
    int closest_x;
    int closest_y;
    if (radius == 0) return 0;
    if (t_num <= 0) {
        closest_x = start_x;
        closest_y = start_y;
    } else if (t_num >= length_sq) {
        closest_x = 0;
        closest_y = 0;
    } else {
        closest_x = start_x + (dx * t_num) / length_sq;
        closest_y = start_y + (dy * t_num) / length_sq;
    }
    dx = sector->x1 - closest_x;
    dy = sector->y1 - closest_y;
    return dx * dx + dy * dy <= radius * radius;
}

int nexus_v1_level_move_allowed(const Nexus_V1_Level *level,
                                int from_x, int from_y,
                                int to_x, int to_y) {
    Nexus_V1_DgnCellGeometry cell;
    int start_x;
    int start_y;

    if (nexus_v1_level_get_cell_geometry(level, to_x, to_y, &cell) != 0 ||
        cell.square_type == 0 || cell.collision_ref == 0x0fffU)
        return 0;
    if (!level->geometry_info.dmweb_container || !cell.collision_sector.valid)
        return 1;

    start_x = (from_x - to_x) * 128;
    start_y = (from_y - to_y) * 128;
    if (cell.collision_sector.circle)
        return !nexus_v1_dgn_circle_blocks_step(&cell.collision_sector,
                                                start_x, start_y);
    return !nexus_v1_dgn_segments_intersect(start_x, start_y, 0, 0,
                                             cell.collision_sector.x1,
                                             cell.collision_sector.y1,
                                             cell.collision_sector.x2,
                                             cell.collision_sector.y2);
}

int nexus_v1_level_dgn_renderer_handoff_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnRendererHandoffReceipt *out_receipt) {
    const Nexus_V1_DgnGeometryInfo *info;

    if (!out_receipt) {
        return -1;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    out_receipt->fallback_visuals_permitted = 0;

    if (!level || level->width <= 0 || level->height <= 0) {
        return 0;
    }

    info = &level->geometry_info;
    out_receipt->width = level->width;
    out_receipt->height = level->height;
    out_receipt->dmweb_container = info->dmweb_container;
    out_receipt->mesh_ready = info->mesh_ready;
    out_receipt->geometry_offset = info->geometry_offset;
    out_receipt->geometry_size = info->geometry_size;
    out_receipt->collision_ref_count = info->collision_ref_count;
    out_receipt->collision_ref_unique_count =
        info->collision_ref_unique_count;
    out_receipt->max_collision_ref = info->max_collision_ref;
    out_receipt->mesh_ref_count = info->mesh_ref_count;
    out_receipt->mesh_ref_unique_count = info->mesh_ref_unique_count;
    out_receipt->max_mesh_ref = info->max_mesh_ref;
    out_receipt->structure1f_descriptor_count =
        info->structure1f_descriptor_count;
    out_receipt->structure1f_valid_descriptor_count =
        info->structure1f_valid_descriptor_count;
    out_receipt->structure1f_solid_descriptor_count =
        info->structure1f_solid_descriptor_count;
    out_receipt->structure1f_first_ref = info->structure1f_first_ref;
    out_receipt->structure1f_max_area = info->structure1f_max_area;
    out_receipt->descriptor_capacity =
        info->geometry_size > 0
            ? info->geometry_size / NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES
            : 0;

    if (!info->dmweb_container) {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_LEGACY_FALLBACK;
    } else if (info->mesh_ready) {
        out_receipt->status = NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH;
        out_receipt->can_render_dgn_mesh = 1;
    } else if (info->geometry_size <= 0) {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY;
    } else {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_DESCRIPTOR_BUDGET;
    }

    out_receipt->blocks_real_dgn_mesh_render =
        out_receipt->can_render_dgn_mesh ? 0 : 1;
    return 0;
}

const char *nexus_v1_dgn_renderer_handoff_status_name(
    Nexus_V1_DgnRendererHandoffStatus status) {
    switch (status) {
    case NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING: return "missing";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH: return "ready-mesh";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY:
        return "blocked-no-geometry";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_DESCRIPTOR_BUDGET:
        return "blocked-descriptor-budget";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_LEGACY_FALLBACK:
        return "blocked-legacy-fallback";
    default: return "unknown";
    }
}

static void nexus_v1_dgn_plan_project_quad(Nexus_V1_DgnRenderCommand *command);

static int nexus_v1_dgn_plan_push(
    Nexus_V1_DgnRenderCommand *commands,
    int max_commands,
    Nexus_V1_DgnRenderPlanReceipt *receipt,
    Nexus_V1_DgnRenderCommand command) {
    if (!receipt) {
        return -1;
    }
    if (!commands || receipt->command_count >= max_commands) {
        receipt->blocks_real_dgn_mesh_render = 1;
        receipt->plan_ready = 0;
        return -1;
    }
    nexus_v1_dgn_plan_project_quad(&command);
    commands[receipt->command_count++] = command;
    receipt->source_cell_count++;
    if (command.kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
        receipt->floor_count++;
        if (command.material_source_kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
            receipt->floor_material_command_count++;
        }
    } else if (command.kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
        receipt->ceiling_count++;
        if (command.material_source_kind ==
            NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
            receipt->ceiling_material_command_count++;
        }
    } else {
        receipt->wall_count++;
        if (command.material_source_kind != NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
            command.material_source_kind != NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
            receipt->wall_material_command_count++;
        }
    }
    if (command.mesh_ref != 0U && command.mesh_ref != 0x0FFFU) {
        receipt->mesh_command_count++;
        if (command.mesh_descriptor_projected) {
            receipt->mesh_descriptor_command_count++;
        }
        if (command.mesh_descriptor.valid) {
            receipt->structure1f_command_count++;
            if (command.mesh_descriptor.solid) {
                receipt->structure1f_solid_command_count++;
            }
            if (command.mesh_descriptor.area > receipt->structure1f_max_area) {
                receipt->structure1f_max_area = command.mesh_descriptor.area;
            }
        }
        if (receipt->first_mesh_ref == 0) {
            receipt->first_mesh_ref = command.mesh_ref;
        }
        if ((int)command.mesh_ref > receipt->max_mesh_ref) {
            receipt->max_mesh_ref = command.mesh_ref;
        }
    }
    return 0;
}

static Nexus_V1_DgnRenderCommand nexus_v1_dgn_plan_command(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnRenderCommandKind kind,
    int x,
    int y,
    int depth,
    int lateral,
    int wall_dir) {
    Nexus_V1_DgnRenderCommand command;
    memset(&command, 0, sizeof(command));
    command.kind = kind;
    command.x = x;
    command.y = y;
    command.depth = depth;
    command.lateral = lateral;
    Nexus_V1_DgnCellGeometry cell;
    (void)nexus_v1_level_get_cell_geometry(level, x, y, &cell);
    command.square_type = cell.square_type;
    command.wall_dir = wall_dir & 3;
    command.collision_ref = cell.collision_ref;
    command.mesh_ref = cell.mesh_ref;
    command.collision_sector = cell.collision_sector;
    command.mesh_descriptor = cell.mesh_descriptor;
    command.floor_rotation = cell.floor_rotation;
    command.floor_slope = cell.floor_slope;
    memcpy(command.floor_height, cell.floor_height, sizeof(command.floor_height));
    memcpy(command.ceiling_height, cell.ceiling_height,
           sizeof(command.ceiling_height));
    command.material_id = (uint8_t)nexus_v1_level_get_material_ref(
        level, x, y, kind, wall_dir);
    switch (kind) {
    case NEXUS_V1_DGN_RENDER_COMMAND_FLOOR:
        command.material_source_kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
        command.palette_index = command.material_id;
        command.draw_order = (uint8_t)(32 - depth);
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_CEILING:
        command.material_source_kind = NEXUS_V1_DGN_RENDER_COMMAND_CEILING;
        command.palette_index = command.material_id;
        command.draw_order = (uint8_t)(16 - depth);
        break;
    default:
        command.material_source_kind = kind;
        command.palette_index = command.material_id;
        command.draw_order = (uint8_t)(48 - depth);
        break;
    }
    return command;
}

static int16_t nexus_v1_dgn_view_clamp(int value) {
    if (value < -NEXUS_V1_DGN_VIEWPORT_UNITS) return -NEXUS_V1_DGN_VIEWPORT_UNITS;
    if (value > NEXUS_V1_DGN_VIEWPORT_UNITS * 2) return NEXUS_V1_DGN_VIEWPORT_UNITS * 2;
    return (int16_t)value;
}

static int nexus_v1_dgn_view_x(int lateral_half, int z_half) {
    return (NEXUS_V1_DGN_VIEWPORT_UNITS / 2) +
        (lateral_half * NEXUS_V1_DGN_VIEWPORT_UNITS / 2) / z_half;
}

static int nexus_v1_dgn_view_floor_y(int z_half) {
    return 400 + (768 / z_half);
}

static int nexus_v1_dgn_view_ceiling_y(int z_half) {
    return 400 - (512 / z_half);
}

static void nexus_v1_dgn_plan_set_quad(Nexus_V1_DgnRenderCommand *command,
                                       int x0, int y0, int x1, int y1,
                                       int x2, int y2, int x3, int y3) {
    command->quad_x[0] = nexus_v1_dgn_view_clamp(x0);
    command->quad_y[0] = nexus_v1_dgn_view_clamp(y0);
    command->quad_x[1] = nexus_v1_dgn_view_clamp(x1);
    command->quad_y[1] = nexus_v1_dgn_view_clamp(y1);
    command->quad_x[2] = nexus_v1_dgn_view_clamp(x2);
    command->quad_y[2] = nexus_v1_dgn_view_clamp(y2);
    command->quad_x[3] = nexus_v1_dgn_view_clamp(x3);
    command->quad_y[3] = nexus_v1_dgn_view_clamp(y3);
}

static void nexus_v1_dgn_plan_project_quad(Nexus_V1_DgnRenderCommand *command) {
    int near_z = command->depth * 2 + 1;
    int far_z = near_z + 2;
    int left_half = command->lateral * 2 - 1;
    int right_half = left_half + 2;
    int near_left = nexus_v1_dgn_view_x(left_half, near_z);
    int near_right = nexus_v1_dgn_view_x(right_half, near_z);
    int far_left = nexus_v1_dgn_view_x(left_half, far_z);
    int far_right = nexus_v1_dgn_view_x(right_half, far_z);

    switch (command->kind) {
    case NEXUS_V1_DGN_RENDER_COMMAND_FLOOR:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_floor_y(near_z),
            near_right, nexus_v1_dgn_view_floor_y(near_z),
            far_right, nexus_v1_dgn_view_floor_y(far_z),
            far_left, nexus_v1_dgn_view_floor_y(far_z));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_CEILING:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_ceiling_y(near_z),
            far_left, nexus_v1_dgn_view_ceiling_y(far_z),
            far_right, nexus_v1_dgn_view_ceiling_y(far_z),
            near_right, nexus_v1_dgn_view_ceiling_y(near_z));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_floor_y(near_z),
            near_right, nexus_v1_dgn_view_floor_y(near_z),
            near_right, nexus_v1_dgn_view_ceiling_y(near_z),
            near_left, nexus_v1_dgn_view_ceiling_y(near_z));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_floor_y(near_z),
            far_left, nexus_v1_dgn_view_floor_y(far_z),
            far_left, nexus_v1_dgn_view_ceiling_y(far_z),
            near_left, nexus_v1_dgn_view_ceiling_y(near_z));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT:
        nexus_v1_dgn_plan_set_quad(command,
            far_right, nexus_v1_dgn_view_floor_y(far_z),
            near_right, nexus_v1_dgn_view_floor_y(near_z),
            near_right, nexus_v1_dgn_view_ceiling_y(near_z),
            far_right, nexus_v1_dgn_view_ceiling_y(far_z));
        break;
    default:
        break;
    }
    if (command->mesh_descriptor.valid) {
        int x0 = command->quad_x[0] + (int)command->mesh_descriptor.x1 * 2;
        int y0 = command->quad_y[0] + (int)command->mesh_descriptor.y1 * 2;
        int x1 = command->quad_x[1] + (int)command->mesh_descriptor.x2 * 2;
        int y1 = command->quad_y[1] + (int)command->mesh_descriptor.y1 * 2;
        int x2 = command->quad_x[2] + (int)command->mesh_descriptor.x2 * 2;
        int y2 = command->quad_y[2] + (int)command->mesh_descriptor.y2 * 2;
        int x3 = command->quad_x[3] + (int)command->mesh_descriptor.x1 * 2;
        int y3 = command->quad_y[3] + (int)command->mesh_descriptor.y2 * 2;
        nexus_v1_dgn_plan_set_quad(command, x0, y0, x1, y1, x2, y2, x3, y3);
        command->mesh_descriptor_projected = 1;
    }
}

int nexus_v1_level_build_dgn_view_render_plan(
    const Nexus_V1_Level *level,
    int party_x,
    int party_y,
    int party_dir,
    Nexus_V1_DgnRenderCommand *commands,
    int max_commands,
    Nexus_V1_DgnRenderPlanReceipt *out_receipt) {
    static const int dir_dx[4] = {0, 1, 0, -1};
    static const int dir_dy[4] = {-1, 0, 1, 0};
    static const int left_dx[4] = {-1, 0, 1, 0};
    static const int left_dy[4] = {0, -1, 0, 1};
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_DgnRenderPlanReceipt receipt;
    int pdir;
    int depth;

    if (!out_receipt) {
        return -1;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt.first_blocking_x = -1;
    receipt.first_blocking_y = -1;
    receipt.first_blocking_depth = -1;
    receipt.fallback_visuals_permitted = 0;
    if (commands && max_commands > 0) {
        memset(commands, 0,
               (size_t)max_commands * sizeof(Nexus_V1_DgnRenderCommand));
    }

    if (!level || max_commands < NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS) {
        receipt.blocks_real_dgn_mesh_render = 1;
        *out_receipt = receipt;
        return 0;
    }
    if (nexus_v1_level_dgn_renderer_handoff_receipt(level, &handoff) != 0) {
        receipt.blocks_real_dgn_mesh_render = 1;
        *out_receipt = receipt;
        return 0;
    }
    receipt.status = handoff.status;
    if (handoff.status != NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH ||
        !handoff.can_render_dgn_mesh) {
        receipt.blocks_real_dgn_mesh_render = 1;
        *out_receipt = receipt;
        return 0;
    }

    pdir = party_dir & 3;
    for (depth = 0; depth < NEXUS_V1_DGN_VIEW_DISTANCE; ++depth) {
        int cx = party_x + dir_dx[pdir] * depth;
        int cy = party_y + dir_dy[pdir] * depth;
        int lx = cx + left_dx[pdir];
        int ly = cy + left_dy[pdir];
        int rx = cx - left_dx[pdir];
        int ry = cy - left_dy[pdir];
        int sq = nexus_v1_level_get_square(level, cx, cy);
        int sq_l = nexus_v1_level_get_square(level, lx, ly);
        int sq_r = nexus_v1_level_get_square(level, rx, ry);

        if (sq != 0) {
            if (nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
                        cx, cy, depth, 0, pdir)) != 0) {
                break;
            }
            if (nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
                        cx, cy, depth, 0, pdir)) != 0) {
                break;
            }
            if (sq_l != 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
                        lx, ly, depth, -1, pdir));
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
                        lx, ly, depth, -1, pdir));
            }
            if (sq_r != 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
                        rx, ry, depth, 1, pdir));
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
                        rx, ry, depth, 1, pdir));
            }
            if (sq_l == 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT,
                        cx, cy, depth, -1, (pdir + 3) & 3));
            }
            if (sq_r == 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT,
                        cx, cy, depth, 1, (pdir + 1) & 3));
            }
        } else {
            if (nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT,
                        cx, cy, depth, 0, (pdir + 2) & 3)) == 0) {
                receipt.first_blocking_x = cx;
                receipt.first_blocking_y = cy;
                receipt.first_blocking_depth = depth;
            }
            break;
        }
        if (receipt.blocks_real_dgn_mesh_render) {
            break;
        }
    }

    if (!receipt.blocks_real_dgn_mesh_render) {
        receipt.material_semantics_complete =
            receipt.floor_material_command_count == receipt.floor_count &&
            receipt.ceiling_material_command_count == receipt.ceiling_count &&
            receipt.wall_material_command_count == receipt.wall_count;
        receipt.plan_ready =
            receipt.command_count > 0 && receipt.material_semantics_complete
                ? 1 : 0;
        receipt.blocks_real_dgn_mesh_render =
            receipt.plan_ready ? 0 : 1;
    }
    *out_receipt = receipt;
    return 0;
}
