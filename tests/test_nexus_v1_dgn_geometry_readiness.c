#include "nexus_v1_dungeon.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        g_fail++; \
    } \
} while (0)

static void wb16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffU);
}

static void wb32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xffU);
    p[2] = (uint8_t)((v >> 8) & 0xffU);
    p[3] = (uint8_t)(v & 0xffU);
}

static uint8_t *cell_at(uint8_t *structure1, int structure1b_rel, int x, int y) {
    return structure1 + structure1b_rel +
           ((y * NEXUS_MAX_MAP_SIZE + x) *
            NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
}

static int build_dmweb_dgn(uint8_t *buf,
                           int buf_size,
                           int structure1_blocks,
                           int structure1b_rel,
                           int geometry_bytes) {
    uint8_t *structure1;
    int useful;

    if (!buf || buf_size <= 0) {
        return -1;
    }
    memset(buf, 0, (size_t)buf_size);
    useful = structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + geometry_bytes;
    if (useful > structure1_blocks * NEXUS_DGN_BLOCK_SIZE) {
        return -1;
    }

    wb16(buf + 0x0C, 1U);
    wb16(buf + 0x0E, (uint16_t)structure1_blocks);
    wb32(buf + 0x10, (uint32_t)useful);

    structure1 = buf + NEXUS_DGN_BLOCK_SIZE;
    structure1[0] = 0x00;
    structure1[1] = 0x50;
    structure1[2] = 0x40;
    structure1[3] = 0x40;
    wb32(structure1 + 0x10, 0x38U);
    wb32(structure1 + 0x14, (uint32_t)structure1b_rel);
    wb32(structure1 + 0x18,
         (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES));
    if (geometry_bytes >= 168) {
        int records_end = 152 + ((geometry_bytes - 152) / 16) * 16;
        int record_count = (records_end - 152) / 16;
        int record;
        if (record_count >
            NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK + 1) {
            record_count = NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK + 1;
            records_end = 152 + record_count * 16;
        }
        /* Bounded Structure1C, reserved 0x24 span, then opaque 0x30 rows. */
        wb32(structure1 + 0x24,
             (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24));
        wb32(structure1 + 0x2c,
             (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 152));
        wb32(structure1 + 0x30,
             (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 152));
        wb32(structure1 + 0x34,
             (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES +
                        records_end));
        structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES] = 6;
        for (record = 0; record < record_count - 1; ++record) {
            structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES +
                       152 + record * 16 +
                       NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE] =
                (uint8_t)record;
        }
    }
    return 0;
}

static void set_collision_ref(uint8_t *structure1,
                              int structure1b_rel,
                              int x,
                              int y,
                              int ref) {
    uint8_t *cell = cell_at(structure1, structure1b_rel, x, y);
    cell[6] = (uint8_t)((ref >> 8) & 0x0f);
    cell[7] = (uint8_t)(ref & 0xff);
}

static void set_post_grid_0x30_ref(uint8_t *structure1,
                                   int structure1b_rel,
                                   int x,
                                   int y,
                                   int ref) {
    uint8_t *cell = cell_at(structure1, structure1b_rel, x, y);
    cell[5] = (uint8_t)((ref >> 4) & 0xff);
    cell[6] = (uint8_t)((cell[6] & 0x0fU) | ((ref & 0x0f) << 4));
}

static void set_structure1a_owner_ref(uint8_t *structure1,
                                       int structure1b_rel,
                                       int x, int y, int ref) {
    uint8_t *cell = cell_at(structure1, structure1b_rel, x, y);
    cell[4] = (uint8_t)(cell[4] | 0x80U);
    cell[5] = (uint8_t)((ref >> 4) & 0xff);
    cell[6] = (uint8_t)((cell[6] & 0x0fU) | ((ref & 0x0f) << 4));
}

static void set_floor_flags(uint8_t *structure1, int structure1b_rel,
                            int x, int y, uint16_t flags) {
    uint8_t *cell = cell_at(structure1, structure1b_rel, x, y);
    cell[0] = (uint8_t)(flags >> 8);
    cell[1] = (uint8_t)(flags & 0xffU);
}

static void build_structure1f_fixture(uint8_t *structure1,
                                      int structure1b_rel) {
    uint8_t *structure1f = structure1 + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 312;
    static const uint8_t tags[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        {0x10U, 0x11U, 0x12U, 0x20U, 0x21U, 0x22U};
    static const int sizes[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        {8, 12, 16, 12, 12, 16};
    static const int counts[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        {2, 2, 2, 2, 2, 4};
    int family;
    int cursor = NEXUS_DGN_STRUCTURE1F_HEADER_BYTES;

    wb32(structure1 + 0x34, (uint32_t)(structure1b_rel +
                                        NEXUS_DGN_STRUCTURE1B_BYTES + 312));
    wb16(structure1f, 0x0034U);
    wb16(structure1f + 2, 0x0012U);
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        int record;
        wb16(structure1f + 4 + family * 2, (uint16_t)counts[family]);
        for (record = 0; record < counts[family]; ++record) {
            uint8_t *entry = structure1f + cursor + record * sizes[family];
            entry[0] = tags[family];
            if (family <= NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
                entry[1] = (uint8_t)(10 + record);
                entry[2] = (uint8_t)(20 + record);
            }
        }
        cursor += counts[family] * sizes[family];
    }
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 3] = 4;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 4] = 0x8eU;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 5] = 7;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 7] = 15;
    /* Floor-decoration and sensor fields retain their source-backed slots. */
    structure1f[32 + 3] = (uint8_t)-30;
    structure1f[32 + 4] = 30;
    structure1f[32 + 5] = 0x28U;
    structure1f[32 + 7] = 3U;
    structure1f[32 + 8] = 14U;
    structure1f[32 + 9] = 15U;
    structure1f[56 + 5] = 0x27U;
    structure1f[56 + 6] = 0x28U;
    structure1f[56 + 10] = 80;
    structure1f[56 + 11] = 40;
    structure1f[56 + 12] = 3;
    structure1f[56 + 13] = 30;
    structure1f[56 + 14] = 31;
    structure1f[56 + 15] = 2;
    /* Alcove/wall records keep their original Structure1A indexes as raw
     * provenance.  Their target table remains deliberately undecoded. */
    wb16(structure1f + 88 + 2, 3U);
    wb16(structure1f + 100 + 2, 3U);
    wb16(structure1f + 112 + 2, 0U);
    wb16(structure1f + 124 + 2, 4U);
    wb16(structure1f + 136 + 2, 5U);
    wb16(structure1f + 152 + 2, 6U);
    wb16(structure1f + 168 + 2, 7U);
    wb16(structure1f + 184 + 2, 8U);
    structure1f[88 + 1] = 1U;
    structure1f[100 + 1] = 1U;
    structure1f[112 + 1] = 2U;
    structure1f[124 + 1] = 3U;
    structure1f[136 + 1] = 3U;
    structure1f[152 + 1] = 4U;
    structure1f[168 + 1] = 4U;
    structure1f[184 + 1] = 4U;
    structure1f[88 + 4] = 1U;
    structure1f[100 + 4] = 1U;
    structure1f[112 + 4] = 2U;
    structure1f[124 + 4] = 3U;
    structure1f[136 + 4] = 3U;
    structure1f[152 + 4] = 4U;
    structure1f[168 + 4] = 4U;
    structure1f[184 + 4] = 4U;
    structure1f[88 + 5] = (uint8_t)-1;
    structure1f[100 + 5] = (uint8_t)-1;
    structure1f[112 + 5] = 1U;
    structure1f[112 + 6] = 2U;
    structure1f[124 + 5] = 1U;
    structure1f[124 + 6] = 2U;
    structure1f[136 + 5] = 3U;
    structure1f[136 + 6] = (uint8_t)-4;
    structure1f[152 + 5] = 3U;
    structure1f[152 + 6] = (uint8_t)-4;
    structure1f[168 + 5] = 127U;
    structure1f[168 + 6] = (uint8_t)-128;
    structure1f[184 + 5] = 127U;
    structure1f[184 + 6] = (uint8_t)-128;
    structure1f[112 + 7] = 5U;
    structure1f[124 + 7] = 5U;
    structure1f[136 + 7] = 7U;
    structure1f[152 + 7] = 9U;
    structure1f[168 + 7] = 9U;
    structure1f[184 + 7] = 9U;
    structure1f[136 + 13] = 20U;
    structure1f[136 + 14] = 21U;
    structure1f[152 + 13] = 20U;
    structure1f[152 + 14] = 21U;
    structure1f[168 + 13] = 30U;
    structure1f[168 + 14] = 31U;
    structure1f[168 + 15] = 2U;
    structure1f[184 + 13] = 30U;
    structure1f[184 + 14] = 31U;
    structure1f[184 + 15] = 2U;
    structure1f[136 + 12] = 1U;
    structure1f[152 + 12] = 1U;
    structure1f[168 + 12] = 2U;
    structure1f[184 + 12] = 2U;
    structure1f[88 + 7] = 6U;
    structure1f[100 + 7] = 6U;
}

static void build_direct_structure1f_fixture(uint8_t *structure1,
                                             int structure1b_rel,
                                             int x,
                                             int y) {
    uint8_t *structure1f = structure1 + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 312;

    /* One documented direct-coordinate item record. No Structure1A-bound
     * record is present, so the plan reaches its explicit no-draw gate. */
    memset(structure1f, 0, 64U);
    wb32(structure1 + 0x34, (uint32_t)(structure1b_rel +
                                        NEXUS_DGN_STRUCTURE1B_BYTES + 312));
    wb32(structure1 - NEXUS_DGN_BLOCK_SIZE + 0x10,
         (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES +
                    312 + NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 8));
    wb16(structure1f, 0x0034U);
    wb16(structure1f + 2, 0x0012U);
    wb16(structure1f + 4, 1U);
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES] = 0x10U;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 1] = (uint8_t)x;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 2] = (uint8_t)y;
}

static void build_structure1g_fixture(uint8_t *structure1,
                                      int structure1b_rel) {
    const int post_grid = structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES;
    uint8_t *structure1g = structure1 + post_grid + 24;
    uint8_t *structure1f = structure1 + post_grid + 212;

    /* C (24 bytes), G (28 bytes), D (128 bytes), E (32-byte opaque tail),
     * F (empty counted header). This is the exact source-backed G framing. */
    wb32(structure1 + 0x1c, (uint32_t)(post_grid + 24));
    wb32(structure1 + 0x24, (uint32_t)(post_grid + 52));
    wb32(structure1 + 0x30, (uint32_t)(post_grid + 180));
    wb32(structure1 + 0x34, (uint32_t)(post_grid + 212));
    wb32(structure1 - NEXUS_DGN_BLOCK_SIZE + 0x10,
         (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 228));
    wb16(structure1g, 2U);
    wb16(structure1g + 2, 20U);
    structure1g[4] = 7U;
    wb16(structure1g + 6, 1U);
    wb16(structure1g + 8, 0x0156U);
    wb16(structure1g + 10, 0U);
    structure1g[12] = 0xffU;
    wb16(structure1g + 20, 0x0156U);
    wb16(structure1g + 24, 0xffffU);
    (void)structure1f;
}

static void build_structure2_fixture(uint8_t *dgn) {
    uint8_t *structure2 = dgn + NEXUS_DGN_BLOCK_SIZE * 20;
    int descriptor;

    /* DMWeb DGN Structure2: local IDs 0..10 and FFFF. Structure1G's
     * global 0x156 image ID therefore resolves to local descriptor 10. */
    wb16(dgn + 0x14, 20U);
    wb16(dgn + 0x16, 1U);
    wb32(dgn + 0x18, 240U);
    for (descriptor = 0; descriptor <= 10; ++descriptor) {
        uint8_t *entry = structure2 +
            descriptor * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES;
        wb16(entry, (uint16_t)descriptor);
        wb16(entry + 2, 0x0008U);
        wb16(entry + 6, 16U);
        wb16(entry + 8, 16U);
    }
    wb16(structure2 + 11 * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES, 0xffffU);
}

static void test_variable_grid_and_mesh_ready(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 25];
    const int structure1b_rel = 0x1a90;
    const int geometry_bytes = 256;
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_DgnStructure1Layout layout;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_Level level;
    uint8_t *structure1;
    uint8_t *geometry;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 24,
                          structure1b_rel, geometry_bytes) == 0,
          "synthetic DMWeb DGN buffer builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;

    set_collision_ref(structure1, structure1b_rel, 2, 2, 1);
    set_collision_ref(structure1, structure1b_rel, 3, 2, 5);
    set_collision_ref(structure1, structure1b_rel, 4, 2, 5);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 2, 2, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 2, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 4, 2, 4);
    set_collision_ref(structure1, structure1b_rel, 0, 0, 0x0fff);
    cell_at(structure1, structure1b_rel, 5, 5)[1] = 0x01; /* door flag */
    geometry = dgn + NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
               NEXUS_DGN_STRUCTURE1B_BYTES;
    geometry[152 + 16] = 1;
    geometry[152 + 16 * 5 + 3] = 2;
    geometry[152 + 16 + NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE] = 0x81U;

    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          layout.valid && layout.structure1b_relative_offset == structure1b_rel &&
          layout.post_grid_offset == structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES &&
          layout.post_grid_size == geometry_bytes,
          "Structure1 layout bounds grid and post-grid span without naming unknown sections");

    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0,
          "geometry info parses DMWeb block header");
    CHECK(info.dmweb_container == 1, "geometry info marks DMWeb container");
    CHECK(info.structure1_offset == NEXUS_DGN_BLOCK_SIZE,
          "Structure1 offset is block-based");
    CHECK(info.structure1b_offset == NEXUS_DGN_BLOCK_SIZE + structure1b_rel,
          "Structure1B absolute offset follows header rel pointer");
    CHECK(info.structure1b_size == NEXUS_DGN_STRUCTURE1B_BYTES,
          "Structure1B size is fixed 0x8000");
    CHECK(info.geometry_offset ==
          NEXUS_DGN_BLOCK_SIZE + structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES,
          "geometry span starts immediately after Structure1B");
    CHECK(info.geometry_size == geometry_bytes,
          "geometry span is bounded by useful Structure1 size");
    CHECK(info.collision_ref_count == 3,
          "collision ref count includes duplicate cell references");
    CHECK(info.collision_ref_unique_count == 2,
          "collision ref unique count deduplicates Structure1C refs");
    CHECK(info.max_collision_ref == 5,
          "max collision descriptor ref is captured");
    CHECK(info.post_grid_0x30_ref_count == 3 &&
          info.post_grid_0x30_ref_unique_count == 2 &&
          info.max_post_grid_0x30_ref == 4 &&
          info.post_grid_0x30_ref_value_count == 3,
          "packed-high post-grid values are counted without claiming row indexing");
    CHECK(info.post_grid_0x30_references_valid &&
          info.post_grid_0x30_invalid_ref_count == 0,
          "all visible packed Structure1F references stay within the typed prefix");
    CHECK(info.mesh_ready == 1,
          "bounded collision and mesh descriptor budgets mark fixture mesh-ready");
    CHECK(info.post_grid_0x30_row_ordinal_flagged_prefix_record_count == 1 &&
          info.post_grid_0x30_first_row_ordinal_flagged_prefix_record == 1 &&
          info.post_grid_0x30_last_row_ordinal_flagged_prefix_record == 1,
          "0x30 ordinal flag remains row-local evidence rather than a cell reference");

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0,
          "level loader consumes same geometry info");
    CHECK(level.width == 64 && level.height == 64,
          "level loader keeps 64x64 Structure1B shape");
    CHECK(level.squares[0][0] == 0, "0x0fff collision decodes as wall");
    CHECK(level.geometry_info.mesh_ready == 1,
          "level carries mesh readiness info");
    CHECK(level.geometry_info.geometry_size == geometry_bytes,
          "level carries bounded Structure1 geometry span");
    CHECK(nexus_v1_level_get_collision_ref(&level, 3, 2) == 5,
          "level keeps Structure1B collision refs for renderer route");
    CHECK(level.geometry_size ==
          ((int)sizeof(dgn) - level.geometry_offset),
          "legacy level.geometry_size remains full file tail");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0,
          "DGN renderer handoff receipt builds for mesh-ready level");
    CHECK(handoff.status == NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH,
          "DGN renderer handoff marks ready mesh");
    CHECK(handoff.can_render_dgn_mesh == 1 &&
          handoff.blocks_real_dgn_mesh_render == 0 &&
          handoff.fallback_visuals_permitted == 0,
          "DGN renderer handoff routes real mesh without fallback visuals");
    CHECK(handoff.max_collision_ref == 5 &&
          handoff.max_post_grid_0x30_ref == 4 &&
          handoff.post_grid_0x30_ref_unique_count == 2 &&
          handoff.post_grid_0x30_references_valid &&
          handoff.post_grid_0x30_invalid_ref_count == 0 &&
          handoff.post_grid_0x30_row_ordinal_prefix_valid &&
          handoff.post_grid_0x30_row_ordinal_flagged_prefix_record_count == 1 &&
          handoff.post_grid_0x30_first_row_ordinal_flagged_prefix_record == 1,
          "DGN renderer handoff carries the verified typed 0x30 row prefix");
    CHECK(strcmp(nexus_v1_dgn_renderer_handoff_status_name(handoff.status),
                 "ready-mesh") == 0,
          "DGN renderer handoff has stable ready route name");
}

static void test_real_dgn_structure1_layout_corpus(void) {
    static const int expected_flagged_prefix_records[16] =
        {0, 3, 11, 20, 9, 15, 4, 15, 21, 0, 1, 7, 11, 4, 6, 4};
    static const int expected_first_flagged_prefix_records[16] =
        {-1, 1, 0, 1, 0, 0, 10, 2, 0, -1, 13, 9, 2, 4, 3, 0};
    static const int expected_last_flagged_prefix_records[16] =
        {-1, 3, 22, 48, 20, 29, 14, 43, 28, -1, 13, 19, 58, 7, 9, 15};
    static const int expected_structure1g_entries[16] =
        {0, 1, 5, 10, 2, 4, 2, 1, 6, 3, 4, 2, 1, 2, 0, 8};
    static const int expected_structure1g_animated_floors[16] =
        {0, 0, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 0};
    static const int expected_structure2_textures[16] =
        {82, 122, 100, 126, 97, 98, 85, 104,
         122, 113, 114, 118, 102, 106, 95, 94};
    /* SN_WALL.MNS has exactly 15 TEXT descriptors (IDs 0..14).  These are
     * raw Structure1B byte-3/byte-4 counts above that range, measured from
     * the hash-verified retail LEV corpus. They prove those bytes cannot be
     * promoted as direct wall-material IDs. */
    static const int expected_byte3_above_wall_bank[16] =
        {0, 86, 228, 976, 1255, 1280, 1451, 1494,
         1005, 98, 1969, 2113, 409, 647, 820, 998};
    static const int expected_byte4_above_wall_bank[16] =
        {49, 455, 1267, 1341, 1558, 1331, 596, 1707,
         1306, 786, 1318, 1428, 1474, 987, 552, 1045};
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    int structure2_descriptor_total = 0;
    int structure2_nonzero_target_total = 0;
    int structure2_in_span_target_total = 0;
    int structure2_word_bounded_target_total = 0;
    int structure2_unaligned_target_total = 0;
    int structure3_declared_level_count = 0;
    int structure3_valid_level_count = 0;
    int structure3_byte_total = 0;
    int structure3_nonzero_byte_total = 0;
    int structure3_transition_total = 0;
    int structure3_nonzero_byte_run_total = 0;
    int structure3_longest_nonzero_byte_run = 0;
    int structure3_complete_block_total = 0;
    int structure3_zero_block_total = 0;
    int structure3_nonzero_block_total = 0;
    int structure3_nonzero_block_run_total = 0;
    int structure3_longest_nonzero_block_run = 0;
    int level;
    int checked = 0;
    if (!data_dir || !data_dir[0]) return;
    for (level = 0; level <= 15; ++level) {
        char path[1024];
        FILE *file;
        long size;
        uint8_t *data;
        Nexus_V1_DgnStructure1Layout layout;
        Nexus_V1_DgnGeometryInfo info;
        Nexus_V1_Level loaded_level;
        Nexus_V1_DgnRendererHandoffReceipt handoff;
        Nexus_V1_DgnStructure3OrdinalCorrelationReceipt correlation;
        int byte3_above_wall_bank = 0;
        int byte4_above_wall_bank = 0;
        int cell;
        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level);
        file = fopen(path, "rb");
        CHECK(file != NULL, "real DGN corpus file opens");
        if (!file) continue;
        CHECK(fseek(file, 0, SEEK_END) == 0, "real DGN corpus file seeks");
        size = ftell(file);
        CHECK(size > 0 && fseek(file, 0, SEEK_SET) == 0,
              "real DGN corpus file has bounded size");
        if (size <= 0) { fclose(file); continue; }
        data = (uint8_t *)malloc((size_t)size);
        CHECK(data != NULL, "real DGN corpus buffer allocates");
        if (!data) { fclose(file); continue; }
        CHECK(fread(data, 1, (size_t)size, file) == (size_t)size,
              "real DGN corpus file reads completely");
        fclose(file);
        CHECK(nexus_v1_dgn_structure1_layout(&layout, data, (int)size) == 0 &&
              layout.valid && layout.post_grid_offset ==
                  layout.structure1b_end_relative_offset &&
              layout.post_grid[0].header_offset == 0x18 &&
              layout.post_grid[1].header_offset == 0x1c &&
              layout.post_grid[2].header_offset == 0x24 &&
              layout.post_grid[3].header_offset == 0x2c &&
              layout.post_grid[4].header_offset == 0x30 &&
              layout.post_grid[5].header_offset == 0x34,
              "real DGN layout retains bounded observed post-grid header pointers");
        CHECK(layout.structure1c.valid &&
              layout.structure1c.relative_offset == layout.post_grid_offset &&
              layout.structure1c.record_size == 4 &&
              layout.structure1c.record_count > 0 &&
              layout.structure1c.indexed_record_count ==
                  layout.structure1c.record_count - 1,
              "real DGN corpus validates the counted Structure1C four-byte record form");
        CHECK(nexus_v1_dgn_geometry_info(&info, data, (int)size) == 0 &&
              info.collision_records_valid &&
              info.structure1c_record_count == layout.structure1c.record_count &&
              info.max_collision_ref < info.structure1c_record_count,
              "all real Structure1B collision indexes fit the validated Structure1C table");
        CHECK(layout.post_grid_0x24_zero_span.valid &&
              layout.post_grid_0x24_zero_span.size ==
                  NEXUS_DGN_POST_GRID_0X24_ZERO_BYTES,
              "real DGN corpus proves the 0x24 post-grid span is 128 zero bytes");
        CHECK(layout.post_grid_0x30_records.valid &&
              layout.post_grid_0x30_records.record_size ==
                  NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES &&
              layout.post_grid_0x30_records.record_count > 0 &&
              layout.post_grid_0x30_records.typed_prefix_record_count ==
                  layout.post_grid_0x30_records.record_count - 1 &&
              layout.post_grid_0x30_records.opaque_tail_record_count == 1 &&
              layout.post_grid_0x30_records.row_ordinal_prefix_valid &&
              info.post_grid_0x30_record_table_valid &&
              info.post_grid_0x30_record_count ==
                  layout.post_grid_0x30_records.record_count &&
              info.post_grid_0x30_typed_prefix_record_count ==
                  layout.post_grid_0x30_records.typed_prefix_record_count &&
              info.post_grid_0x30_row_ordinal_prefix_valid &&
              layout.post_grid_0x30_records
                      .row_ordinal_flagged_prefix_record_count ==
                  expected_flagged_prefix_records[level] &&
              layout.post_grid_0x30_records
                      .first_row_ordinal_flagged_prefix_record ==
                  expected_first_flagged_prefix_records[level] &&
              layout.post_grid_0x30_records
                      .last_row_ordinal_flagged_prefix_record ==
                  expected_last_flagged_prefix_records[level] &&
              info.post_grid_0x30_row_ordinal_flagged_prefix_record_count ==
                  expected_flagged_prefix_records[level],
              "real DGN corpus proves the bounded typed 0x30 row prefix and opaque tail");
        CHECK(layout.structure1f.valid && info.structure1f_valid &&
              layout.structure1f.total_entry_count == info.structure1f_total_entry_count &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_ITEMS] >= 0 &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS] >= 0 &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS] >= 0 &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_ALCOVES] >= 0 &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS] >= 0 &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS] >= 0,
              "real DGN corpus validates the complete counted Structure1F family layout");
        CHECK(nexus_v1_level_load(&loaded_level, data, (int)size, level) == 0 &&
              loaded_level.structure1f_entry_count ==
                  layout.structure1f.total_entry_count &&
              nexus_v1_level_dgn_renderer_handoff_receipt(&loaded_level,
                                                          &handoff) == 0 &&
              handoff.structure1f_valid &&
              handoff.structure1f_total_entry_count ==
                  layout.structure1f.total_entry_count &&
              handoff.structure1f_typed_entry_count ==
                  loaded_level.structure1f_entry_count &&
              memcmp(handoff.structure1f_family_count,
                     layout.structure1f.family_count,
                     sizeof(handoff.structure1f_family_count)) == 0 &&
              handoff.structure1g_present == layout.post_grid[1].present &&
              handoff.structure1g_valid == layout.structure1g.valid &&
              handoff.structure1g_animated_texture_count ==
                  loaded_level.structure1g_entry_count &&
              handoff.structure1g_sequence_count ==
                  layout.structure1g.sequence_count &&
              layout.structure1g.valid == (expected_structure1g_entries[level] > 0) &&
              loaded_level.structure1g_entry_count ==
                  expected_structure1g_entries[level] &&
              loaded_level.structure1g_floor_animation_cell_count ==
                  expected_structure1g_animated_floors[level] &&
              loaded_level.structure1g_floor_animation_bound_count ==
                  expected_structure1g_animated_floors[level] &&
              loaded_level.structure2_texture_table_valid &&
              loaded_level.structure2_texture_count ==
                  expected_structure2_textures[level],
              "real Structure1F and optional Structure1G typed records survive level load and reach host handoff");
        CHECK(handoff.structure3_payload.declared ==
                  loaded_level.structure3_payload.declared &&
              handoff.structure3_payload.valid ==
                  loaded_level.structure3_payload.valid &&
              handoff.structure3_payload.raw_payload_hash ==
                  loaded_level.structure3_payload.raw_payload_hash &&
              handoff.structure1f_face_selectors.structure1a_relation_complete ==
                  handoff.structure3_model_references.complete &&
              handoff.structure1f_face_selectors.resolved_face_selector_count ==
                  handoff.structure3_model_references.resolved_model_reference_count &&
              !handoff.structure1f_face_selectors.face_semantics_proven &&
              handoff.structure3_model_references.structure1f_bound_entry_count ==
                  loaded_level.structure1f_entry_count -
                      handoff.structure1f_spatial.direct_coordinate_entry_count,
              "real Structure3 payload, owner-model, and raw face-selector provenance reach the host unchanged");
        CHECK(nexus_v1_level_structure3_ordinal_correlation_receipt(
                  &loaded_level, &correlation) == 0 &&
              correlation.structure1a_relation_complete ==
                  handoff.structure3_model_references.complete &&
              correlation.structure3_payload_valid ==
                  loaded_level.structure3_payload.valid &&
              correlation.structure3_block_count ==
                  loaded_level.structure3_payload.block_count &&
              correlation.structure3_nonzero_byte_run_count ==
                  loaded_level.structure3_payload.nonzero_byte_run_count &&
              correlation.structure3_nonzero_block_run_count ==
                  loaded_level.structure3_payload.nonzero_block_run_count &&
              correlation.direct_block_ordinal_mapping_disproven ==
                  (correlation.zero_based_block_ordinal_mapping_disproven &&
                   correlation.one_based_block_ordinal_mapping_disproven) &&
              correlation.direct_byte_run_ordinal_mapping_disproven ==
                  (correlation.zero_based_byte_run_ordinal_mapping_disproven &&
                   correlation.one_based_byte_run_ordinal_mapping_disproven) &&
              correlation.direct_run_ordinal_mapping_disproven ==
                  (correlation.zero_based_run_ordinal_mapping_disproven &&
                   correlation.one_based_run_ordinal_mapping_disproven) &&
              !correlation.face_semantics_proven,
              "retail Structure3 correlation rules out only disproven direct ordinals");
        if (loaded_level.structure3_payload.declared) {
            CHECK(loaded_level.structure3_payload.valid &&
                  loaded_level.structure3_payload.zero_byte_count +
                          loaded_level.structure3_payload.nonzero_byte_count ==
                      loaded_level.structure3_payload.byte_size &&
                  ((loaded_level.structure3_payload.nonzero_byte_count == 0 &&
                    loaded_level.structure3_payload.nonzero_byte_run_count == 0 &&
                    loaded_level.structure3_payload.longest_nonzero_byte_run == 0 &&
                    loaded_level.structure3_payload.first_nonzero_byte_run_offset == -1 &&
                    loaded_level.structure3_payload.first_nonzero_byte_run_byte_count == 0 &&
                    loaded_level.structure3_payload.last_nonzero_byte_run_offset == -1 &&
                    loaded_level.structure3_payload.last_nonzero_byte_run_byte_count == 0) ||
                   (loaded_level.structure3_payload.nonzero_byte_count > 0 &&
                    loaded_level.structure3_payload.nonzero_byte_run_count > 0 &&
                    loaded_level.structure3_payload.longest_nonzero_byte_run > 0 &&
                    loaded_level.structure3_payload.longest_nonzero_byte_run <=
                        loaded_level.structure3_payload.nonzero_byte_count &&
                    loaded_level.structure3_payload.first_nonzero_byte_run_offset >= 0 &&
                    loaded_level.structure3_payload.first_nonzero_byte_run_byte_count > 0 &&
                    loaded_level.structure3_payload.first_nonzero_byte_run_offset +
                        loaded_level.structure3_payload.first_nonzero_byte_run_byte_count <=
                        loaded_level.structure3_payload.byte_size &&
                    loaded_level.structure3_payload.last_nonzero_byte_run_offset >= 0 &&
                    loaded_level.structure3_payload.last_nonzero_byte_run_byte_count > 0 &&
                    loaded_level.structure3_payload.last_nonzero_byte_run_offset +
                        loaded_level.structure3_payload.last_nonzero_byte_run_byte_count <=
                        loaded_level.structure3_payload.byte_size)) &&
                  loaded_level.structure3_payload.complete_block_count ==
                      loaded_level.structure3_payload.block_count &&
                  loaded_level.structure3_payload.zero_block_count +
                          loaded_level.structure3_payload.nonzero_block_count ==
                      loaded_level.structure3_payload.complete_block_count &&
                  ((loaded_level.structure3_payload.nonzero_block_count == 0 &&
                    loaded_level.structure3_payload.first_nonzero_block_index == -1 &&
                    loaded_level.structure3_payload.last_nonzero_block_index == -1 &&
                    loaded_level.structure3_payload.nonzero_block_run_count == 0 &&
                    loaded_level.structure3_payload.longest_nonzero_block_run == 0 &&
                    loaded_level.structure3_payload
                        .first_nonzero_block_run_start_block_index == -1 &&
                    loaded_level.structure3_payload
                        .first_nonzero_block_run_block_count == 0 &&
                    loaded_level.structure3_payload
                        .last_nonzero_block_run_start_block_index == -1 &&
                    loaded_level.structure3_payload
                        .last_nonzero_block_run_block_count == 0) ||
                   (loaded_level.structure3_payload.nonzero_block_count > 0 &&
                    loaded_level.structure3_payload.first_nonzero_block_index >= 0 &&
                    loaded_level.structure3_payload.last_nonzero_block_index >=
                        loaded_level.structure3_payload.first_nonzero_block_index &&
                    loaded_level.structure3_payload.nonzero_block_run_count > 0 &&
                    loaded_level.structure3_payload.longest_nonzero_block_run > 0 &&
                    loaded_level.structure3_payload.longest_nonzero_block_run <=
                        loaded_level.structure3_payload.nonzero_block_count &&
                    loaded_level.structure3_payload
                        .first_nonzero_block_run_start_block_index >= 0 &&
                    loaded_level.structure3_payload
                        .first_nonzero_block_run_block_count > 0 &&
                    loaded_level.structure3_payload
                        .first_nonzero_block_run_start_block_index +
                            loaded_level.structure3_payload
                                .first_nonzero_block_run_block_count <=
                        loaded_level.structure3_payload.complete_block_count &&
                    loaded_level.structure3_payload
                        .last_nonzero_block_run_start_block_index >= 0 &&
                    loaded_level.structure3_payload
                        .last_nonzero_block_run_block_count > 0 &&
                    loaded_level.structure3_payload
                        .last_nonzero_block_run_start_block_index +
                            loaded_level.structure3_payload
                                .last_nonzero_block_run_block_count <=
                        loaded_level.structure3_payload.complete_block_count)) &&
                  loaded_level.structure3_payload.distinct_byte_value_count > 0 &&
                  loaded_level.structure3_payload.first_nonzero_byte_offset >= -1 &&
                  loaded_level.structure3_payload.last_nonzero_byte_offset >=
                      loaded_level.structure3_payload.first_nonzero_byte_offset &&
                  loaded_level.structure3_payload.raw_payload_hash != 0U &&
                  !loaded_level.structure3_payload.face_semantics_proven,
                  "retail Structure3 spans retain raw bounded observations without face semantics");
            ++structure3_declared_level_count;
        }
        if (loaded_level.structure3_payload.valid) {
            ++structure3_valid_level_count;
            structure3_byte_total += loaded_level.structure3_payload.byte_size;
            structure3_nonzero_byte_total +=
                loaded_level.structure3_payload.nonzero_byte_count;
            structure3_transition_total +=
                loaded_level.structure3_payload.byte_transition_count;
            structure3_nonzero_byte_run_total +=
                loaded_level.structure3_payload.nonzero_byte_run_count;
            if (loaded_level.structure3_payload.longest_nonzero_byte_run >
                structure3_longest_nonzero_byte_run) {
                structure3_longest_nonzero_byte_run =
                    loaded_level.structure3_payload.longest_nonzero_byte_run;
            }
            structure3_complete_block_total +=
                loaded_level.structure3_payload.complete_block_count;
            structure3_zero_block_total +=
                loaded_level.structure3_payload.zero_block_count;
            structure3_nonzero_block_total +=
                loaded_level.structure3_payload.nonzero_block_count;
            structure3_nonzero_block_run_total +=
                loaded_level.structure3_payload.nonzero_block_run_count;
            if (loaded_level.structure3_payload.longest_nonzero_block_run >
                structure3_longest_nonzero_block_run) {
                structure3_longest_nonzero_block_run =
                    loaded_level.structure3_payload.longest_nonzero_block_run;
            }
        }
        CHECK(loaded_level.structure2_payload.valid &&
              loaded_level.structure2_payload.opaque_payload_zero_byte_count +
                      loaded_level.structure2_payload.opaque_payload_nonzero_byte_count ==
                  loaded_level.structure2_payload.opaque_payload_size &&
              loaded_level.structure2_payload.opaque_payload_complete_pair_count ==
                  loaded_level.structure2_payload.opaque_payload_size / 2 &&
              loaded_level.structure2_payload.opaque_payload_trailing_byte_count ==
                  loaded_level.structure2_payload.opaque_payload_size % 2 &&
              loaded_level.structure2_payload.opaque_payload_zero_pair_count +
                      loaded_level.structure2_payload.opaque_payload_nonzero_pair_count ==
                  loaded_level.structure2_payload.opaque_payload_complete_pair_count &&
              loaded_level.structure2_payload.local_payload_offset_pattern_observed &&
              loaded_level.structure2_payload
                  .local_payload_word_aligned_offset_pattern_observed &&
              loaded_level.structure2_payload
                  .local_payload_word_bounded_offset_pattern_observed &&
              loaded_level.structure2_payload
                  .nonzero_descriptor_offsets_unaligned_count == 0 &&
              loaded_level.structure2_payload
                  .nonzero_descriptor_offsets_in_opaque_payload_count ==
                  loaded_level.structure2_payload.nonzero_descriptor_offset_count &&
              loaded_level.structure2_payload
                  .nonzero_descriptor_offsets_word_bounded_count ==
                  loaded_level.structure2_payload.nonzero_descriptor_offset_count &&
              loaded_level.structure2_payload.descriptor_offset_envelope_valid &&
              nexus_v1_level_structure2_source_envelope_valid(&loaded_level) &&
              !loaded_level.structure2_payload.material_or_image_data_proven,
              "real Structure2 descriptors retain only their complete aligned opaque target windows");
        structure2_descriptor_total += loaded_level.structure2_texture_count;
        structure2_nonzero_target_total +=
            loaded_level.structure2_payload.nonzero_descriptor_offset_count;
        structure2_in_span_target_total +=
            loaded_level.structure2_payload
                .nonzero_descriptor_offsets_in_opaque_payload_count;
        structure2_word_bounded_target_total +=
            loaded_level.structure2_payload
                .nonzero_descriptor_offsets_word_bounded_count;
        structure2_unaligned_target_total +=
            loaded_level.structure2_payload
                .nonzero_descriptor_offsets_unaligned_count;
        for (cell = 0; cell < NEXUS_MAX_MAP_SIZE * NEXUS_MAX_MAP_SIZE;
             ++cell) {
            const uint8_t *raw = data + info.structure1b_offset + cell * 8;
            if (raw[3] > 14U) ++byte3_above_wall_bank;
            if (raw[4] > 14U) ++byte4_above_wall_bank;
        }
        CHECK(byte3_above_wall_bank == expected_byte3_above_wall_bank[level] &&
              byte4_above_wall_bank == expected_byte4_above_wall_bank[level] &&
              byte3_above_wall_bank + byte4_above_wall_bank > 0,
              "retail Structure1B bytes 3/4 exceed the SN_WALL descriptor bank and stay unbound");
        for (int entry = 0; entry < loaded_level.structure1g_entry_count;
             ++entry) {
            CHECK(loaded_level.structure1g_entries[entry].first_structure2_image_valid,
                  "every canonical Structure1G first image binds Structure2");
        }
        checked++;
        free(data);
    }
    CHECK(checked == 16, "all LEV00 through LEV15 files were checked");
    CHECK(structure2_descriptor_total == 1678 &&
          structure2_nonzero_target_total == 2944 &&
          structure2_in_span_target_total == 2944 &&
          structure2_word_bounded_target_total == 2944 &&
          structure2_unaligned_target_total == 0,
          "retail Structure2 corpus retains the known 20-byte descriptor target envelope only");
    CHECK(structure3_valid_level_count == structure3_declared_level_count &&
          structure3_byte_total >= structure3_nonzero_byte_total &&
          structure3_zero_block_total + structure3_nonzero_block_total ==
              structure3_complete_block_total &&
          structure3_nonzero_block_run_total <= structure3_nonzero_block_total &&
          structure3_longest_nonzero_block_run <= structure3_nonzero_block_total &&
          structure3_nonzero_byte_run_total <= structure3_nonzero_byte_total &&
          structure3_longest_nonzero_byte_run <= structure3_nonzero_byte_total &&
          structure3_transition_total >= 0,
          "retail Structure3 corpus retains only raw zero-separated byte and block spans");
}

static void test_structure1c_record_table_bounds(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    int post_grid;
    uint8_t *structure1;
    Nexus_V1_DgnStructure1Layout layout;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 32) == 0,
          "Structure1C record-table fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    post_grid = structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES;
    wb32(structure1 + 0x24, (uint32_t)(post_grid + 8));
    structure1[post_grid] = 2;
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          layout.structure1c.valid &&
          layout.structure1c.size == 8 &&
          layout.structure1c.record_count == 2 &&
          layout.structure1c.indexed_record_count == 1,
          "counted four-byte Structure1C span becomes consumable only when bounded");
    structure1[post_grid] = 3;
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          !layout.structure1c.valid,
          "mismatched Structure1C count leaves the post-grid span untyped");
}

static void test_structure1f_semantics_and_bounds(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 24];
    const int structure1b_rel = 0x200;
    uint8_t *structure1;
    Nexus_V1_DgnStructure1Layout layout;
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_Level level;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    Nexus_V1_DgnStructure1ABoundaryReceipt structure1a_boundary;
    Nexus_V1_DgnStructure1ARelationReceipt structure1a_relation;
    Nexus_V1_DgnStructure1AKindReceipt structure1a_kinds;
    Nexus_V1_DgnStructure3ModelReferenceReceipt structure3_model_references;
    Nexus_V1_DgnStructure1ATransformSelectorReceipt transform_selectors;
    Nexus_V1_DgnStructure1FFaceSelectorReceipt face_selectors;
    Nexus_V1_DgnStructure1FRotationSelectorReceipt rotation_selectors;
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt face_rotation_pairs;
    Nexus_V1_DgnStructure1FOffsetPairReceipt offset_pairs;
    Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt wall_payload_selectors;
    Nexus_V1_DgnStructure1FWallSensorDestinationReceipt wall_sensor_destinations;
    Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt wall_sensor_controls;
    Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt
        wall_sensor_control_destination_tuples;
    Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt
        wall_sensor_model_rotation_pairs;
    Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt
        wall_decoration_model_rotation_pairs;
    Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt alcove_payload_selectors;
    Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt
        alcove_payload_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt floor_sensor_controls;
    Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt floor_sensor_destinations;
    Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt
        floor_sensor_model_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt floor_sensor_extent_pairs;
    Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt floor_decoration_payloads;
    Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt floor_decoration_rotations;
    Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt
        floor_decoration_model_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt
        floor_decoration_control_extents;
    Nexus_V1_DgnStructure1FItemAttributePairReceipt item_attribute_pairs;
    Nexus_V1_DgnStructure1FItemLocationPairReceipt item_location_pairs;
    Nexus_V1_DgnStructure1FItemCoordinatePairReceipt item_coordinate_pairs;
    Nexus_V1_DgnStructure3PayloadReceipt structure3_payload;
    Nexus_V1_DgnStructure3OrdinalCorrelationReceipt structure3_correlation;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt render_plan;
    Nexus_V1_DgnStructure1FStructure1ACommandSource structure1a_sources[8];
    Nexus_V1_DgnStructure1FStructure1ACommandSourceReceipt
        structure1a_source_receipt;
    Nexus_V1_DgnStructure1AStructure3TopologyCandidate
        structure3_candidates[8];
    Nexus_V1_DgnStructure1AStructure3TopologyCandidateReceipt
        structure3_candidate_receipt;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 512) == 0,
          "Structure1F fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    /* DMWeb DGN container Structure3 block envelope: opaque payload only. */
    wb16(dgn + 0x1c, 20U);
    wb16(dgn + 0x1e, 4U);
    dgn[NEXUS_DGN_BLOCK_SIZE * 20 + 1] = 0x7fU;
    dgn[NEXUS_DGN_BLOCK_SIZE * 20 + 2] = 0x7fU;
    dgn[NEXUS_DGN_BLOCK_SIZE * 21] = 0x33U;
    dgn[NEXUS_DGN_BLOCK_SIZE * 23 + 7] = 0x42U;
    wb32(structure1 + 0x0c, 9U);
    for (int index = 0; index < 9; ++index) {
        structure1[0x38 + index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES] =
            (uint8_t)(index % 3);
        structure1[0x38 + index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES + 1] =
            (uint8_t)(0x20 + index);
        structure1[0x38 + index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES + 2] =
            (uint8_t)(index % 4);
    }
    set_structure1a_owner_ref(structure1, structure1b_rel, 10, 10, 0);
    set_structure1a_owner_ref(structure1, structure1b_rel, 11, 10, 3);
    set_structure1a_owner_ref(structure1, structure1b_rel, 12, 10, 4);
    set_structure1a_owner_ref(structure1, structure1b_rel, 13, 10, 5);
    set_structure1a_owner_ref(structure1, structure1b_rel, 14, 10, 6);
    set_structure1a_owner_ref(structure1, structure1b_rel, 15, 10, 7);
    set_structure1a_owner_ref(structure1, structure1b_rel, 16, 10, 8);
    build_structure1f_fixture(structure1, structure1b_rel);
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          layout.structure1f.valid && layout.structure1f.relative_offset ==
              structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 312 &&
          layout.structure1f.size == 200 &&
          layout.structure1f.wall_sensor_first_texture_index == 0x34 &&
          layout.structure1f.wall_sensor_first_model_index == 0x12 &&
          layout.structure1f.total_entry_count == 14 &&
          layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_ITEMS] == 2 &&
          layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS] == 4,
          "Structure1F accepts only an exact counted six-family span");
    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0 &&
          info.structure1f_valid && info.structure1f_total_entry_count == 14 &&
          info.structure1f_family_count[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS] == 2,
          "Structure1F coverage reaches DGN geometry provenance");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure1f_entry_count == 14 &&
          level.structure1f_entries[0].family == NEXUS_V1_DGN_STRUCTURE1F_ITEMS &&
          level.structure1f_entries[0].location == 4 &&
          level.structure1f_entries[0].item_id == 0x8eU &&
          level.structure1f_entries[0].attribute1 == 7 &&
          level.structure1f_entries[0].attribute2 == 15 &&
          level.structure1f_entries[2].offset_x == -30 &&
          level.structure1f_entries[2].type_or_control == 3U &&
          level.structure1f_entries[2].width == 14U &&
          level.structure1f_entries[2].height == 15U &&
          level.structure1f_entries[4].model_or_aspect == 0x27U &&
          level.structure1f_entries[4].destination_orientation == 2,
          "Structure1F retains documented item, decoration, and sensor fields");
    CHECK(nexus_v1_level_structure1f_spatial_receipt(&level, &spatial) == 0 &&
          spatial.valid && spatial.typed_entry_count == 14 &&
          spatial.direct_coordinate_entry_count == 6 &&
          spatial.item_entry_count == 2 &&
          spatial.floor_decoration_entry_count == 2 &&
          spatial.floor_sensor_entry_count == 2 &&
          spatial.structure1a_bound_entry_count == 8,
          "Structure1F separates direct cell records from unresolved Structure1A records");
    CHECK(nexus_v1_level_structure1f_item_attribute_pair_receipt(
              &level, &item_attribute_pairs) == 0 &&
          item_attribute_pairs.spatial_valid &&
          item_attribute_pairs.item_count == 2 &&
          item_attribute_pairs.resolved_pair_count == 2 &&
          item_attribute_pairs.unique_pair_count == 2 &&
          item_attribute_pairs.duplicate_pair_count == 0 &&
          item_attribute_pairs.complete &&
          !item_attribute_pairs.semantics_proven,
          "Structure1F item attributes remain raw no-draw provenance");
    CHECK(nexus_v1_level_structure1f_item_location_pair_receipt(
              &level, &item_location_pairs) == 0 &&
          item_location_pairs.spatial_valid &&
          item_location_pairs.item_count == 2 &&
          item_location_pairs.resolved_pair_count == 2 &&
          item_location_pairs.unique_pair_count == 2 &&
          item_location_pairs.duplicate_pair_count == 0 &&
          item_location_pairs.complete &&
          !item_location_pairs.semantics_proven,
          "Structure1F item locations remain raw no-draw provenance");
    CHECK(nexus_v1_level_structure1f_item_coordinate_pair_receipt(
              &level, &item_coordinate_pairs) == 0 &&
          item_coordinate_pairs.spatial_valid &&
          item_coordinate_pairs.item_count == 2 &&
          item_coordinate_pairs.resolved_pair_count == 2 &&
          item_coordinate_pairs.unique_pair_count == 2 &&
          item_coordinate_pairs.duplicate_pair_count == 0 &&
          item_coordinate_pairs.zero_pair_count == 0 &&
          item_coordinate_pairs.nonzero_pair_count == 2 &&
          item_coordinate_pairs.highest_pair == 0x0b15U &&
          item_coordinate_pairs.complete &&
          !item_coordinate_pairs.semantics_proven,
          "Structure1F item coordinate bytes remain raw no-draw provenance");
    CHECK(nexus_v1_level_structure1a_boundary_receipt(&level,
                                                       &structure1a_boundary) == 0 &&
          structure1a_boundary.valid && structure1a_boundary.entry_count == 8 &&
          structure1a_boundary.alcove_entry_count == 2 &&
          structure1a_boundary.wall_decoration_entry_count == 2 &&
          structure1a_boundary.wall_sensor_entry_count == 4 &&
          structure1a_boundary.zero_index_count == 1 &&
          structure1a_boundary.nonzero_index_count == 7 &&
          structure1a_boundary.unique_index_count == 7 &&
          structure1a_boundary.duplicate_index_count == 1 &&
          structure1a_boundary.highest_index == 8U,
          "Structure1A indexes reach the host boundary as raw provenance only");
    CHECK(nexus_v1_level_structure1a_relation_receipt(&level,
                                                       &structure1a_relation) == 0 &&
          structure1a_relation.table_valid &&
          structure1a_relation.table_entry_count == 9 &&
          structure1a_relation.structure1f_bound_entry_count == 8 &&
          structure1a_relation.resolved_entry_count == 8 &&
          structure1a_relation.complete &&
          level.structure1f_entries[7].structure1a_relation_valid &&
          level.structure1f_entries[7].structure1a_owner_x == 11 &&
          level.structure1f_entries[7].structure1a_owner_y == 10 &&
          level.structure1f_entries[7].structure1a_structure3_model_index == 0x23 &&
          level.structure1f_entries[7].structure1a_z_rotation == 3U,
          "Structure1F references resolve only through unique Structure1B owners");
    memset(commands, 0, sizeof(commands));
    commands[0].kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
    commands[0].x = 11;
    commands[0].y = 10;
    commands[1].kind = NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT;
    commands[1].x = 11;
    commands[1].y = 10;
    memset(structure1a_sources, 0, sizeof(structure1a_sources));
    memset(&structure1a_source_receipt, 0, sizeof(structure1a_source_receipt));
    CHECK(nexus_v1_dgn_bind_structure1a_owned_cell_sources(
              &level, commands, 2, structure1a_sources, 8,
              &structure1a_source_receipt) == 0 &&
          structure1a_source_receipt.complete &&
          structure1a_source_receipt.visible_owned_entry_count == 2 &&
          structure1a_source_receipt.floor_command_source_count == 2 &&
          structure1a_source_receipt.alcove_floor_command_source_count == 2 &&
          structure1a_source_receipt.wall_decoration_floor_command_source_count == 0 &&
          structure1a_source_receipt.wall_sensor_floor_command_source_count == 0 &&
          structure1a_sources[0].command_index == 0 &&
          structure1a_sources[0].owner_x == 11 &&
          structure1a_sources[0].owner_y == 10 &&
          structure1a_sources[0].entry.family ==
              NEXUS_V1_DGN_STRUCTURE1F_ALCOVES &&
          !structure1a_sources[0].draw_authorized &&
          !structure1a_source_receipt.fallback_visuals_permitted,
          "Structure1A-owned rows reach only their verified visible cell anchor without a draw claim");
    memset(structure3_candidates, 0, sizeof(structure3_candidates));
    memset(&structure3_candidate_receipt, 0,
           sizeof(structure3_candidate_receipt));
    CHECK(nexus_v1_dgn_bind_structure1a_structure3_topology_candidates(
              &level, structure1a_sources,
              structure1a_source_receipt.floor_command_source_count,
              structure3_candidates, 8, &structure3_candidate_receipt) == 0 &&
          structure3_candidate_receipt.complete &&
          structure3_candidate_receipt.owner_cell_source_count == 2 &&
          structure3_candidate_receipt.topology_candidate_count == 2 &&
          structure3_candidate_receipt.direct_ordinal_mapping_disproven_count == 2 &&
          structure3_candidates[0].command_index == 0 &&
          structure3_candidates[0].owner_x == 11 &&
          structure3_candidates[0].owner_y == 10 &&
          structure3_candidates[0].structure3_model_index == 0x23U &&
          structure3_candidates[0].structure3_block_offset == 20 &&
          structure3_candidates[0].structure3_block_count == 4 &&
          structure3_candidates[0].structure3_byte_size ==
              NEXUS_DGN_BLOCK_SIZE * 4 &&
          structure3_candidates[0].structure3_raw_payload_hash != 0U &&
          structure3_candidates[0].model_index_exceeds_block_count &&
          structure3_candidates[0].model_index_exceeds_nonzero_byte_run_count &&
          structure3_candidates[0].model_index_exceeds_nonzero_block_run_count &&
          structure3_candidates[0].direct_ordinal_mapping_disproven &&
          !structure3_candidates[0].model_ordinal_proven &&
          !structure3_candidates[0].face_semantics_proven &&
          !structure3_candidates[0].draw_authorized &&
          !structure3_candidate_receipt.fallback_visuals_permitted,
          "Structure1A owner cells retain only bounded Structure3 topology candidates without ordinal or draw claims");
    CHECK(nexus_v1_level_structure1a_kind_receipt(&level, &structure1a_kinds) == 0 &&
          structure1a_kinds.structure1a_relation_complete &&
          structure1a_kinds.structure1f_bound_entry_count == 8 &&
          structure1a_kinds.resolved_kind_count == 8 &&
          structure1a_kinds.unique_kind_count == 3 &&
          structure1a_kinds.duplicate_kind_count == 5 &&
          structure1a_kinds.zero_kind_count == 4 &&
          structure1a_kinds.nonzero_kind_count == 4 &&
          structure1a_kinds.highest_kind == 2U &&
          structure1a_kinds.complete &&
          !structure1a_kinds.kind_semantics_proven,
          "resolved Structure1A kind bytes remain raw no-draw provenance");
    CHECK(nexus_v1_level_structure1a_transform_selector_receipt(
              &level, &transform_selectors) == 0 &&
          transform_selectors.structure1a_relation_complete &&
          transform_selectors.structure1f_bound_entry_count == 8 &&
          transform_selectors.resolved_selector_count == 8 &&
          transform_selectors.unique_selector_count == 4 &&
          transform_selectors.duplicate_selector_count == 4 &&
          transform_selectors.zero_selector_count == 3 &&
          transform_selectors.nonzero_selector_count == 5 &&
          transform_selectors.highest_selector == 3U &&
          transform_selectors.complete &&
          !transform_selectors.transform_semantics_proven,
          "resolved Structure1A transform selectors remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_face_selector_receipt(
              &level, &face_selectors) == 0 &&
          face_selectors.structure1a_relation_complete &&
          face_selectors.structure1f_bound_entry_count == 8 &&
          face_selectors.resolved_face_selector_count == 8 &&
          face_selectors.unique_face_selector_count == 4 &&
          face_selectors.duplicate_face_selector_count == 4 &&
          face_selectors.zero_face_selector_count == 0 &&
          face_selectors.nonzero_face_selector_count == 8 &&
          face_selectors.highest_face_selector == 4U &&
          face_selectors.complete && !face_selectors.face_semantics_proven,
          "resolved Structure1F face selectors remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_rotation_selector_receipt(
              &level, &rotation_selectors) == 0 &&
          rotation_selectors.structure1a_relation_complete &&
          rotation_selectors.structure1f_bound_entry_count == 8 &&
          rotation_selectors.resolved_rotation_selector_count == 8 &&
          rotation_selectors.unique_rotation_selector_count == 4 &&
          rotation_selectors.duplicate_rotation_selector_count == 4 &&
          rotation_selectors.zero_rotation_selector_count == 0 &&
          rotation_selectors.nonzero_rotation_selector_count == 8 &&
          rotation_selectors.highest_rotation_selector == 4U &&
          rotation_selectors.complete &&
          !rotation_selectors.rotation_semantics_proven,
          "resolved Structure1F rotation selectors remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_face_rotation_pair_receipt(
              &level, &face_rotation_pairs) == 0 &&
          face_rotation_pairs.structure1a_relation_complete &&
          face_rotation_pairs.structure1f_bound_entry_count == 8 &&
          face_rotation_pairs.resolved_pair_count == 8 &&
          face_rotation_pairs.unique_pair_count == 4 &&
          face_rotation_pairs.duplicate_pair_count == 4 &&
          face_rotation_pairs.zero_pair_count == 0 &&
          face_rotation_pairs.nonzero_pair_count == 8 &&
          face_rotation_pairs.highest_pair == 0x0404U &&
          face_rotation_pairs.complete &&
          !face_rotation_pairs.pair_semantics_proven,
          "Structure1F face-rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_offset_pair_receipt(
              &level, &offset_pairs) == 0 &&
          offset_pairs.structure1a_relation_complete &&
          offset_pairs.structure1f_bound_entry_count == 8 &&
          offset_pairs.resolved_offset_pair_count == 8 &&
          offset_pairs.unique_offset_pair_count == 4 &&
          offset_pairs.duplicate_offset_pair_count == 4 &&
          offset_pairs.zero_offset_pair_count == 0 &&
          offset_pairs.nonzero_offset_pair_count == 8 &&
          offset_pairs.minimum_offset_x == -1 &&
          offset_pairs.maximum_offset_x == 127 &&
          offset_pairs.minimum_offset_y == -128 &&
          offset_pairs.maximum_offset_y == 2 &&
          offset_pairs.complete && !offset_pairs.offset_semantics_proven,
          "Structure1F signed offset pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_payload_selector_receipt(
              &level, &wall_payload_selectors) == 0 &&
          wall_payload_selectors.structure1a_relation_complete &&
          wall_payload_selectors.wall_payload_entry_count == 6 &&
          wall_payload_selectors.resolved_payload_selector_count == 6 &&
          wall_payload_selectors.wall_decoration_selector_count == 2 &&
          wall_payload_selectors.wall_sensor_selector_count == 4 &&
          wall_payload_selectors.unique_payload_selector_count == 3 &&
          wall_payload_selectors.duplicate_payload_selector_count == 3 &&
          wall_payload_selectors.zero_payload_selector_count == 0 &&
          wall_payload_selectors.nonzero_payload_selector_count == 6 &&
          wall_payload_selectors.highest_payload_selector == 9U &&
          wall_payload_selectors.complete &&
          !wall_payload_selectors.payload_semantics_proven,
          "Structure1F wall payload selectors remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_sensor_destination_receipt(
              &level, &wall_sensor_destinations) == 0 &&
          wall_sensor_destinations.structure1a_relation_complete &&
          wall_sensor_destinations.wall_sensor_entry_count == 4 &&
          wall_sensor_destinations.resolved_destination_count == 4 &&
          wall_sensor_destinations.unique_destination_count == 2 &&
          wall_sensor_destinations.duplicate_destination_count == 2 &&
          wall_sensor_destinations.zero_destination_count == 0 &&
          wall_sensor_destinations.nonzero_destination_count == 4 &&
          wall_sensor_destinations.highest_destination_x == 30U &&
          wall_sensor_destinations.highest_destination_y == 31U &&
          wall_sensor_destinations.highest_destination_orientation == 2U &&
          wall_sensor_destinations.complete &&
          !wall_sensor_destinations.destination_semantics_proven,
          "Structure1F wall-sensor destination tuples remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_sensor_control_selector_receipt(
              &level, &wall_sensor_controls) == 0 &&
          wall_sensor_controls.structure1a_relation_complete &&
          wall_sensor_controls.wall_sensor_entry_count == 4 &&
          wall_sensor_controls.resolved_control_selector_count == 4 &&
          wall_sensor_controls.unique_control_selector_count == 2 &&
          wall_sensor_controls.duplicate_control_selector_count == 2 &&
          wall_sensor_controls.zero_control_selector_count == 0 &&
          wall_sensor_controls.nonzero_control_selector_count == 4 &&
          wall_sensor_controls.highest_control_selector == 2U &&
          wall_sensor_controls.complete &&
          !wall_sensor_controls.control_semantics_proven,
          "Structure1F wall-sensor controls remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_sensor_control_destination_tuple_receipt(
              &level, &wall_sensor_control_destination_tuples) == 0 &&
          wall_sensor_control_destination_tuples.structure1a_relation_complete &&
          wall_sensor_control_destination_tuples.wall_sensor_entry_count == 4 &&
          wall_sensor_control_destination_tuples.resolved_tuple_count == 4 &&
          wall_sensor_control_destination_tuples.unique_tuple_count == 2 &&
          wall_sensor_control_destination_tuples.duplicate_tuple_count == 2 &&
          wall_sensor_control_destination_tuples.zero_tuple_count == 0 &&
          wall_sensor_control_destination_tuples.nonzero_tuple_count == 4 &&
          wall_sensor_control_destination_tuples.highest_tuple == 0x021e1f02U &&
          wall_sensor_control_destination_tuples.complete &&
          !wall_sensor_control_destination_tuples.tuple_semantics_proven,
          "Structure1F wall-sensor control/destination tuples remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_sensor_model_rotation_pair_receipt(
              &level, &wall_sensor_model_rotation_pairs) == 0 &&
          wall_sensor_model_rotation_pairs.structure1a_relation_complete &&
          wall_sensor_model_rotation_pairs.wall_sensor_entry_count == 4 &&
          wall_sensor_model_rotation_pairs.resolved_pair_count == 4 &&
          wall_sensor_model_rotation_pairs.unique_pair_count == 2 &&
          wall_sensor_model_rotation_pairs.duplicate_pair_count == 2 &&
          wall_sensor_model_rotation_pairs.zero_pair_count == 0 &&
          wall_sensor_model_rotation_pairs.nonzero_pair_count == 4 &&
          wall_sensor_model_rotation_pairs.highest_pair == 0x0904U &&
          wall_sensor_model_rotation_pairs.complete &&
          !wall_sensor_model_rotation_pairs.pair_semantics_proven,
          "Structure1F wall-sensor model/rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_decoration_model_rotation_pair_receipt(
              &level, &wall_decoration_model_rotation_pairs) == 0 &&
          wall_decoration_model_rotation_pairs.structure1a_relation_complete &&
          wall_decoration_model_rotation_pairs.wall_decoration_entry_count == 2 &&
          wall_decoration_model_rotation_pairs.resolved_pair_count == 2 &&
          wall_decoration_model_rotation_pairs.unique_pair_count == 2 &&
          wall_decoration_model_rotation_pairs.duplicate_pair_count == 0 &&
          wall_decoration_model_rotation_pairs.zero_pair_count == 0 &&
          wall_decoration_model_rotation_pairs.nonzero_pair_count == 2 &&
          wall_decoration_model_rotation_pairs.highest_pair == 0x0503U &&
          wall_decoration_model_rotation_pairs.complete &&
          !wall_decoration_model_rotation_pairs.pair_semantics_proven,
          "Structure1F wall-decoration model/rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_alcove_payload_selector_receipt(
              &level, &alcove_payload_selectors) == 0 &&
          alcove_payload_selectors.structure1a_relation_complete &&
          alcove_payload_selectors.alcove_entry_count == 2 &&
          alcove_payload_selectors.resolved_payload_selector_count == 2 &&
          alcove_payload_selectors.unique_payload_selector_count == 1 &&
          alcove_payload_selectors.duplicate_payload_selector_count == 1 &&
          alcove_payload_selectors.zero_payload_selector_count == 0 &&
          alcove_payload_selectors.nonzero_payload_selector_count == 2 &&
          alcove_payload_selectors.highest_payload_selector == 6U &&
          alcove_payload_selectors.complete &&
          !alcove_payload_selectors.payload_semantics_proven,
          "Structure1F alcove payload selectors remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_alcove_payload_rotation_pair_receipt(
              &level, &alcove_payload_rotation_pairs) == 0 &&
          alcove_payload_rotation_pairs.structure1a_relation_complete &&
          alcove_payload_rotation_pairs.alcove_entry_count == 2 &&
          alcove_payload_rotation_pairs.resolved_pair_count == 2 &&
          alcove_payload_rotation_pairs.unique_pair_count == 1 &&
          alcove_payload_rotation_pairs.duplicate_pair_count == 1 &&
          alcove_payload_rotation_pairs.zero_pair_count == 0 &&
          alcove_payload_rotation_pairs.nonzero_pair_count == 2 &&
          alcove_payload_rotation_pairs.highest_pair == 0x0601U &&
          alcove_payload_rotation_pairs.complete &&
          !alcove_payload_rotation_pairs.pair_semantics_proven,
          "Structure1F alcove payload/rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_sensor_control_selector_receipt(
              &level, &floor_sensor_controls) == 0 &&
          floor_sensor_controls.structure1f_spatial_valid &&
          floor_sensor_controls.floor_sensor_entry_count == 2 &&
          floor_sensor_controls.resolved_control_selector_count == 2 &&
          floor_sensor_controls.unique_control_selector_count == 2 &&
          floor_sensor_controls.duplicate_control_selector_count == 0 &&
          floor_sensor_controls.zero_control_selector_count == 1 &&
          floor_sensor_controls.nonzero_control_selector_count == 1 &&
          floor_sensor_controls.highest_control_selector == 3U &&
          floor_sensor_controls.complete &&
          !floor_sensor_controls.control_semantics_proven,
          "Structure1F floor-sensor controls remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_sensor_destination_receipt(
              &level, &floor_sensor_destinations) == 0 &&
          floor_sensor_destinations.structure1f_spatial_valid &&
          floor_sensor_destinations.floor_sensor_entry_count == 2 &&
          floor_sensor_destinations.resolved_destination_count == 2 &&
          floor_sensor_destinations.unique_destination_count == 2 &&
          floor_sensor_destinations.duplicate_destination_count == 0 &&
          floor_sensor_destinations.zero_destination_count == 1 &&
          floor_sensor_destinations.nonzero_destination_count == 1 &&
          floor_sensor_destinations.highest_destination_x == 30U &&
          floor_sensor_destinations.highest_destination_y == 31U &&
          floor_sensor_destinations.highest_destination_orientation == 2U &&
          floor_sensor_destinations.complete &&
          !floor_sensor_destinations.destination_semantics_proven,
          "Structure1F floor-sensor destinations remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_sensor_model_rotation_pair_receipt(
              &level, &floor_sensor_model_rotation_pairs) == 0 &&
          floor_sensor_model_rotation_pairs.structure1f_spatial_valid &&
          floor_sensor_model_rotation_pairs.floor_sensor_entry_count == 2 &&
          floor_sensor_model_rotation_pairs.resolved_pair_count == 2 &&
          floor_sensor_model_rotation_pairs.unique_pair_count == 2 &&
          floor_sensor_model_rotation_pairs.duplicate_pair_count == 0 &&
          floor_sensor_model_rotation_pairs.zero_pair_count == 1 &&
          floor_sensor_model_rotation_pairs.nonzero_pair_count == 1 &&
          floor_sensor_model_rotation_pairs.highest_pair == 0x2728U &&
          floor_sensor_model_rotation_pairs.complete &&
          !floor_sensor_model_rotation_pairs.pair_semantics_proven,
          "Structure1F floor-sensor model/rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_sensor_extent_pair_receipt(
              &level, &floor_sensor_extent_pairs) == 0 &&
          floor_sensor_extent_pairs.structure1f_spatial_valid &&
          floor_sensor_extent_pairs.floor_sensor_entry_count == 2 &&
          floor_sensor_extent_pairs.resolved_pair_count == 2 &&
          floor_sensor_extent_pairs.unique_pair_count == 2 &&
          floor_sensor_extent_pairs.duplicate_pair_count == 0 &&
          floor_sensor_extent_pairs.zero_pair_count +
              floor_sensor_extent_pairs.nonzero_pair_count == 2 &&
          floor_sensor_extent_pairs.highest_pair == 0x5028U &&
          floor_sensor_extent_pairs.complete &&
          !floor_sensor_extent_pairs.pair_semantics_proven,
          "Structure1F floor-sensor extent pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_decoration_payload_selector_receipt(
              &level, &floor_decoration_payloads) == 0 &&
          floor_decoration_payloads.structure1f_spatial_valid &&
          floor_decoration_payloads.floor_decoration_entry_count == 2 &&
          floor_decoration_payloads.resolved_payload_selector_count == 2 &&
          floor_decoration_payloads.unique_payload_selector_count == 2 &&
          floor_decoration_payloads.duplicate_payload_selector_count == 0 &&
          floor_decoration_payloads.zero_payload_selector_count == 1 &&
          floor_decoration_payloads.nonzero_payload_selector_count == 1 &&
          floor_decoration_payloads.highest_payload_selector == 0x28U &&
          floor_decoration_payloads.complete &&
          !floor_decoration_payloads.payload_semantics_proven,
          "Structure1F floor-decoration payloads remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_decoration_rotation_selector_receipt(
              &level, &floor_decoration_rotations) == 0 &&
          floor_decoration_rotations.structure1f_spatial_valid &&
          floor_decoration_rotations.floor_decoration_entry_count == 2 &&
          floor_decoration_rotations.resolved_rotation_selector_count == 2 &&
          floor_decoration_rotations.unique_rotation_selector_count == 1 &&
          floor_decoration_rotations.duplicate_rotation_selector_count == 1 &&
          floor_decoration_rotations.zero_rotation_selector_count == 2 &&
          floor_decoration_rotations.nonzero_rotation_selector_count == 0 &&
          floor_decoration_rotations.complete &&
          !floor_decoration_rotations.rotation_semantics_proven,
          "Structure1F floor-decoration rotations remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_decoration_model_rotation_pair_receipt(
              &level, &floor_decoration_model_rotation_pairs) == 0 &&
          floor_decoration_model_rotation_pairs.structure1f_spatial_valid &&
          floor_decoration_model_rotation_pairs.floor_decoration_entry_count == 2 &&
          floor_decoration_model_rotation_pairs.resolved_pair_count == 2 &&
          floor_decoration_model_rotation_pairs.unique_pair_count == 2 &&
          floor_decoration_model_rotation_pairs.duplicate_pair_count == 0 &&
          floor_decoration_model_rotation_pairs.zero_pair_count == 1 &&
          floor_decoration_model_rotation_pairs.nonzero_pair_count == 1 &&
          floor_decoration_model_rotation_pairs.highest_pair == 0x2800U &&
          floor_decoration_model_rotation_pairs.complete &&
          !floor_decoration_model_rotation_pairs.pair_semantics_proven,
          "Structure1F floor-decoration model/rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_decoration_control_extent_receipt(
              &level, &floor_decoration_control_extents) == 0 &&
          floor_decoration_control_extents.structure1f_spatial_valid &&
          floor_decoration_control_extents.floor_decoration_entry_count == 2 &&
          floor_decoration_control_extents.resolved_tuple_count == 2 &&
          floor_decoration_control_extents.unique_tuple_count == 2 &&
          floor_decoration_control_extents.duplicate_tuple_count == 0 &&
          floor_decoration_control_extents.zero_tuple_count +
              floor_decoration_control_extents.nonzero_tuple_count == 2 &&
          floor_decoration_control_extents.highest_type_or_control >= 3U &&
          floor_decoration_control_extents.highest_width >= 14U &&
          floor_decoration_control_extents.highest_height >= 15U &&
          floor_decoration_control_extents.complete &&
          !floor_decoration_control_extents.tuple_semantics_proven,
          "Structure1F floor-decoration control/extents remain no-draw provenance");
    CHECK(nexus_v1_level_structure3_model_reference_receipt(
              &level, &structure3_model_references) == 0 &&
          structure3_model_references.structure1a_relation_complete &&
          structure3_model_references.structure1f_bound_entry_count == 8 &&
          structure3_model_references.resolved_model_reference_count == 8 &&
          structure3_model_references.unique_model_index_count == 7 &&
          structure3_model_references.duplicate_model_index_count == 1 &&
          structure3_model_references.zero_model_index_count == 0 &&
          structure3_model_references.nonzero_model_index_count == 8 &&
          structure3_model_references.complete,
          "resolved Structure1F owners retain Structure3 indexes as no-draw provenance");
    CHECK(nexus_v1_level_structure3_payload_receipt(
              &level, &structure3_payload) == 0 &&
          structure3_payload.declared && structure3_payload.valid &&
          structure3_payload.block_offset == 20 &&
          structure3_payload.block_count == 4 &&
          structure3_payload.byte_offset == NEXUS_DGN_BLOCK_SIZE * 20 &&
          structure3_payload.byte_size == NEXUS_DGN_BLOCK_SIZE * 4 &&
          structure3_payload.zero_byte_count == NEXUS_DGN_BLOCK_SIZE * 4 - 4 &&
          structure3_payload.nonzero_byte_count == 4 &&
          structure3_payload.distinct_byte_value_count == 4 &&
          structure3_payload.byte_transition_count == 6 &&
          structure3_payload.first_nonzero_byte_offset == 1 &&
          structure3_payload.last_nonzero_byte_offset ==
              NEXUS_DGN_BLOCK_SIZE * 3 + 7 &&
          structure3_payload.nonzero_byte_run_count == 3 &&
          structure3_payload.longest_nonzero_byte_run == 2 &&
          structure3_payload.first_nonzero_byte_run_offset == 1 &&
          structure3_payload.first_nonzero_byte_run_byte_count == 2 &&
          structure3_payload.last_nonzero_byte_run_offset ==
              NEXUS_DGN_BLOCK_SIZE * 3 + 7 &&
          structure3_payload.last_nonzero_byte_run_byte_count == 1 &&
          structure3_payload.complete_block_count == 4 &&
          structure3_payload.zero_block_count == 1 &&
          structure3_payload.nonzero_block_count == 3 &&
          structure3_payload.first_nonzero_block_index == 0 &&
          structure3_payload.last_nonzero_block_index == 3 &&
          structure3_payload.nonzero_block_run_count == 2 &&
          structure3_payload.longest_nonzero_block_run == 2 &&
          structure3_payload.first_nonzero_block_run_start_block_index == 0 &&
          structure3_payload.first_nonzero_block_run_block_count == 2 &&
          structure3_payload.last_nonzero_block_run_start_block_index == 3 &&
          structure3_payload.last_nonzero_block_run_block_count == 1 &&
          structure3_payload.raw_payload_hash != 0U &&
          !structure3_payload.face_semantics_proven,
          "Structure3 payload retains documented block boundaries without face semantics");
    CHECK(nexus_v1_level_structure3_ordinal_correlation_receipt(
              &level, &structure3_correlation) == 0 &&
          structure3_correlation.valid &&
          structure3_correlation.structure1a_relation_complete &&
          structure3_correlation.structure3_payload_valid &&
          structure3_correlation.resolved_model_reference_count == 8 &&
          structure3_correlation.highest_model_index == 0x28 &&
          structure3_correlation.structure3_block_count == 4 &&
          structure3_correlation.structure3_nonzero_byte_run_count == 3 &&
          structure3_correlation.structure3_nonzero_block_run_count == 2 &&
          structure3_correlation.model_index_exceeds_block_count == 8 &&
          structure3_correlation.model_index_exceeds_nonzero_byte_run_count == 8 &&
          structure3_correlation.model_index_exceeds_nonzero_block_run_count == 8 &&
          structure3_correlation.zero_based_block_ordinal_mapping_disproven &&
          structure3_correlation.one_based_block_ordinal_mapping_disproven &&
          structure3_correlation.zero_based_byte_run_ordinal_mapping_disproven &&
          structure3_correlation.one_based_byte_run_ordinal_mapping_disproven &&
          structure3_correlation.zero_based_run_ordinal_mapping_disproven &&
          structure3_correlation.one_based_run_ordinal_mapping_disproven &&
          structure3_correlation.direct_block_ordinal_mapping_disproven &&
          structure3_correlation.direct_byte_run_ordinal_mapping_disproven &&
          structure3_correlation.direct_run_ordinal_mapping_disproven &&
          !structure3_correlation.face_semantics_proven,
          "Structure1A model indexes cannot become direct Structure3 byte, block, or run ordinals");
    for (int entry = 0; entry < level.structure1f_entry_count; ++entry) {
        if (level.structure1f_entries[entry].family >=
            NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) {
            level.structure1f_entries[entry].structure1a_structure3_model_index = 3U;
        }
    }
    CHECK(nexus_v1_level_structure3_ordinal_correlation_receipt(
              &level, &structure3_correlation) == 0 &&
          structure3_correlation.zero_based_byte_run_ordinal_mapping_disproven &&
          !structure3_correlation.one_based_byte_run_ordinal_mapping_disproven &&
          !structure3_correlation.direct_byte_run_ordinal_mapping_disproven &&
          !structure3_correlation.zero_based_block_ordinal_mapping_disproven &&
          !structure3_correlation.one_based_block_ordinal_mapping_disproven &&
          structure3_correlation.zero_based_run_ordinal_mapping_disproven &&
          structure3_correlation.one_based_run_ordinal_mapping_disproven &&
          structure3_correlation.direct_run_ordinal_mapping_disproven,
          "Structure3 ordinal receipts keep zero- and one-based exclusions separate");
    for (int entry = 0; entry < level.structure1f_entry_count; ++entry) {
        if (level.structure1f_entries[entry].family >=
            NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) {
            level.structure1f_entries[entry].structure1a_structure3_model_index =
                (uint8_t)(0x20 + level.structure1f_entries[entry].structure1a_index);
        }
    }
    level.structure1f_entries[7].structure1a_index = 9U;
    level.structure1f_entries[7].structure1a_relation_valid = 0;
    CHECK(nexus_v1_level_structure1a_relation_receipt(&level,
                                                       &structure1a_relation) == 0 &&
          !structure1a_relation.complete &&
          structure1a_relation.out_of_range_index_count == 1,
          "out-of-range Structure1F references remain fail-closed");
    level.structure1f_entries[7].structure1a_index = 3U;
    level.structure1f_entries[7].structure1a_relation_valid = 1;
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.structure1f_valid && handoff.structure1f_total_entry_count == 14 &&
          handoff.structure1f_typed_entry_count == level.structure1f_entry_count &&
          handoff.structure1f_spatial.valid &&
          handoff.structure1f_spatial.direct_coordinate_entry_count == 6 &&
          handoff.structure1f_spatial.structure1a_bound_entry_count == 8 &&
          handoff.structure1a_boundary.valid &&
          handoff.structure1a_boundary.entry_count == 8 &&
          handoff.structure1a_boundary.duplicate_index_count == 1 &&
          handoff.structure1a_relation.complete &&
          handoff.structure1a_relation.resolved_entry_count == 8 &&
          handoff.structure1a_kinds.complete &&
          handoff.structure1a_kinds.unique_kind_count == 3 &&
          !handoff.structure1a_kinds.kind_semantics_proven &&
          handoff.structure3_model_references.complete &&
          handoff.structure3_model_references.unique_model_index_count == 7 &&
          handoff.structure1a_transform_selectors.complete &&
          handoff.structure1a_transform_selectors.unique_selector_count == 4 &&
          !handoff.structure1a_transform_selectors.transform_semantics_proven &&
          handoff.structure1f_face_selectors.complete &&
          handoff.structure1f_face_selectors.unique_face_selector_count == 4 &&
          !handoff.structure1f_face_selectors.face_semantics_proven &&
          handoff.structure1f_rotation_selectors.complete &&
          handoff.structure1f_rotation_selectors.unique_rotation_selector_count == 4 &&
          !handoff.structure1f_rotation_selectors.rotation_semantics_proven &&
          handoff.structure1f_face_rotation_pairs.complete &&
          handoff.structure1f_face_rotation_pairs.unique_pair_count == 4 &&
          !handoff.structure1f_face_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_offset_pairs.complete &&
          handoff.structure1f_offset_pairs.unique_offset_pair_count == 4 &&
          !handoff.structure1f_offset_pairs.offset_semantics_proven &&
          handoff.structure1f_wall_payload_selectors.complete &&
          handoff.structure1f_wall_payload_selectors.unique_payload_selector_count == 3 &&
          !handoff.structure1f_wall_payload_selectors.payload_semantics_proven &&
          handoff.structure1f_wall_sensor_destinations.complete &&
          handoff.structure1f_wall_sensor_destinations.unique_destination_count == 2 &&
          !handoff.structure1f_wall_sensor_destinations.destination_semantics_proven &&
          handoff.structure1f_wall_sensor_control_selectors.complete &&
          handoff.structure1f_wall_sensor_control_selectors.unique_control_selector_count == 2 &&
          !handoff.structure1f_wall_sensor_control_selectors.control_semantics_proven &&
          handoff.structure1f_wall_sensor_control_destination_tuples.complete &&
          handoff.structure1f_wall_sensor_control_destination_tuples.unique_tuple_count == 2 &&
          !handoff.structure1f_wall_sensor_control_destination_tuples.tuple_semantics_proven &&
          handoff.structure1f_wall_sensor_model_rotation_pairs.complete &&
          handoff.structure1f_wall_sensor_model_rotation_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_wall_sensor_model_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_wall_decoration_model_rotation_pairs.complete &&
          handoff.structure1f_wall_decoration_model_rotation_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_wall_decoration_model_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_alcove_payload_selectors.complete &&
          handoff.structure1f_alcove_payload_selectors.unique_payload_selector_count == 1 &&
          !handoff.structure1f_alcove_payload_selectors.payload_semantics_proven &&
          handoff.structure1f_alcove_payload_rotation_pairs.complete &&
          handoff.structure1f_alcove_payload_rotation_pairs.unique_pair_count == 1 &&
          !handoff.structure1f_alcove_payload_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_floor_sensor_control_selectors.complete &&
          handoff.structure1f_floor_sensor_control_selectors.unique_control_selector_count == 2 &&
          !handoff.structure1f_floor_sensor_control_selectors.control_semantics_proven &&
          handoff.structure1f_floor_sensor_destinations.complete &&
          handoff.structure1f_floor_sensor_destinations.unique_destination_count == 2 &&
          !handoff.structure1f_floor_sensor_destinations.destination_semantics_proven &&
          handoff.structure1f_floor_sensor_model_rotation_pairs.complete &&
          handoff.structure1f_floor_sensor_model_rotation_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_floor_sensor_model_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_floor_sensor_extent_pairs.complete &&
          handoff.structure1f_floor_sensor_extent_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_floor_sensor_extent_pairs.pair_semantics_proven &&
          handoff.structure1f_floor_decoration_payload_selectors.complete &&
          handoff.structure1f_floor_decoration_payload_selectors.unique_payload_selector_count == 2 &&
          !handoff.structure1f_floor_decoration_payload_selectors.payload_semantics_proven &&
          handoff.structure1f_floor_decoration_rotation_selectors.complete &&
          !handoff.structure1f_floor_decoration_rotation_selectors.rotation_semantics_proven &&
          handoff.structure1f_floor_decoration_model_rotation_pairs.complete &&
          handoff.structure1f_floor_decoration_model_rotation_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_floor_decoration_model_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_floor_decoration_control_extents.complete &&
          handoff.structure1f_floor_decoration_control_extents.unique_tuple_count == 2 &&
          !handoff.structure1f_floor_decoration_control_extents.tuple_semantics_proven &&
          handoff.structure1f_floor_decoration_offset_pairs.complete &&
          handoff.structure1f_floor_decoration_offset_pairs.unique_pair_count == 2 &&
          handoff.structure1f_floor_decoration_offset_pairs.nonzero_pair_count == 1 &&
          !handoff.structure1f_floor_decoration_offset_pairs.offset_semantics_proven &&
          handoff.structure1f_item_attribute_pairs.complete &&
          handoff.structure1f_item_attribute_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_item_attribute_pairs.semantics_proven &&
          handoff.structure1f_item_location_pairs.complete &&
          handoff.structure1f_item_location_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_item_location_pairs.semantics_proven &&
          handoff.structure1f_item_coordinate_pairs.complete &&
          handoff.structure1f_item_coordinate_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_item_coordinate_pairs.semantics_proven &&
          handoff.structure1f_family_count[NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS] == 4,
          "Structure1F typed records are consumed by the no-fallback host handoff");
    CHECK(handoff.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS &&
          handoff.blocks_real_dgn_mesh_render && !handoff.fallback_visuals_permitted &&
          strcmp(nexus_v1_dgn_renderer_handoff_status_name(handoff.status),
                 "blocked-structure3-face-semantics") == 0,
          "bounded Structure3 payload blocks until original face semantics are proven");
    memset(commands, 0x5a, sizeof(commands));
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
          &level, 10, 10, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &render_plan) == 0 &&
          render_plan.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS &&
          render_plan.command_count > 0 &&
          !render_plan.plan_ready &&
          render_plan.blocks_real_dgn_mesh_render &&
          !render_plan.fallback_visuals_permitted &&
          render_plan.structure1a_kinds.complete &&
          !render_plan.structure1a_kinds.kind_semantics_proven &&
          render_plan.structure3_model_references.complete &&
          render_plan.structure1a_transform_selectors.complete &&
          !render_plan.structure1a_transform_selectors.transform_semantics_proven &&
          render_plan.structure1f_face_selectors.complete &&
          !render_plan.structure1f_face_selectors.face_semantics_proven &&
          render_plan.structure1f_rotation_selectors.complete &&
          !render_plan.structure1f_rotation_selectors.rotation_semantics_proven &&
          render_plan.structure1f_face_rotation_pairs.complete &&
          !render_plan.structure1f_face_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_offset_pairs.complete &&
          !render_plan.structure1f_offset_pairs.offset_semantics_proven &&
          render_plan.structure1f_wall_payload_selectors.complete &&
          !render_plan.structure1f_wall_payload_selectors.payload_semantics_proven &&
          render_plan.structure1f_wall_sensor_destinations.complete &&
          !render_plan.structure1f_wall_sensor_destinations.destination_semantics_proven &&
          render_plan.structure1f_wall_sensor_control_selectors.complete &&
          !render_plan.structure1f_wall_sensor_control_selectors.control_semantics_proven &&
          render_plan.structure1f_wall_sensor_control_destination_tuples.complete &&
          !render_plan.structure1f_wall_sensor_control_destination_tuples.tuple_semantics_proven &&
          render_plan.structure1f_wall_sensor_model_rotation_pairs.complete &&
          !render_plan.structure1f_wall_sensor_model_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_wall_decoration_model_rotation_pairs.complete &&
          !render_plan.structure1f_wall_decoration_model_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_alcove_payload_selectors.complete &&
          !render_plan.structure1f_alcove_payload_selectors.payload_semantics_proven &&
          render_plan.structure1f_alcove_payload_rotation_pairs.complete &&
          !render_plan.structure1f_alcove_payload_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_floor_sensor_control_selectors.complete &&
          !render_plan.structure1f_floor_sensor_control_selectors.control_semantics_proven &&
          render_plan.structure1f_floor_sensor_destinations.complete &&
          !render_plan.structure1f_floor_sensor_destinations.destination_semantics_proven &&
          render_plan.structure1f_floor_sensor_model_rotation_pairs.complete &&
          !render_plan.structure1f_floor_sensor_model_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_floor_sensor_extent_pairs.complete &&
          !render_plan.structure1f_floor_sensor_extent_pairs.pair_semantics_proven &&
          render_plan.structure1f_floor_decoration_payload_selectors.complete &&
          !render_plan.structure1f_floor_decoration_payload_selectors.payload_semantics_proven &&
          render_plan.structure1f_floor_decoration_rotation_selectors.complete &&
          !render_plan.structure1f_floor_decoration_rotation_selectors.rotation_semantics_proven &&
          render_plan.structure1f_floor_decoration_model_rotation_pairs.complete &&
          !render_plan.structure1f_floor_decoration_model_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_floor_decoration_control_extents.complete &&
          render_plan.structure1f_floor_decoration_control_extents.unique_tuple_count == 2 &&
          !render_plan.structure1f_floor_decoration_control_extents.tuple_semantics_proven &&
          render_plan.structure1f_floor_decoration_offset_pairs.complete &&
          render_plan.structure1f_floor_decoration_offset_pairs.unique_pair_count == 2 &&
          render_plan.structure1f_floor_decoration_offset_pairs.nonzero_pair_count == 1 &&
          !render_plan.structure1f_floor_decoration_offset_pairs.offset_semantics_proven &&
          render_plan.structure1f_item_attribute_pairs.complete &&
          render_plan.structure1f_item_attribute_pairs.unique_pair_count == 2 &&
          !render_plan.structure1f_item_attribute_pairs.semantics_proven &&
          render_plan.structure1f_item_location_pairs.complete &&
          render_plan.structure1f_item_location_pairs.unique_pair_count == 2 &&
          !render_plan.structure1f_item_location_pairs.semantics_proven &&
          render_plan.structure1f_item_coordinate_pairs.complete &&
          render_plan.structure1f_item_coordinate_pairs.unique_pair_count == 2 &&
          !render_plan.structure1f_item_coordinate_pairs.semantics_proven &&
          render_plan.structure3_payload.valid &&
          render_plan.command_count > 0 && commands[0].kind != 0 &&
          render_plan.blocks_real_dgn_mesh_render && !render_plan.plan_ready &&
          !render_plan.fallback_visuals_permitted,
          "DGN render planning retains bounded Structure3 source commands without a mesh draw");
    wb16(dgn + 0x1c, 24U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) != 0,
          "out-of-file Structure3 payload envelopes fail closed during DGN load");
    wb16(dgn + 0x1c, 20U);
    wb16(dgn + 0x1e, 0U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) != 0,
          "partial Structure3 payload headers fail closed during DGN load");
    wb16(dgn + 0x1e, 4U);
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 312 +
               NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 1] = 64U;
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.structure1f_declared && !handoff.structure1f_valid &&
          handoff.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_LAYOUT &&
          handoff.blocks_real_dgn_mesh_render &&
          !handoff.fallback_visuals_permitted &&
          strcmp(nexus_v1_dgn_renderer_handoff_status_name(handoff.status),
                 "blocked-structure1f-layout") == 0,
          "declared malformed Structure1F coordinates block host handoff");
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 312 +
               NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 1] = 10U;
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 312 + 16] = 0x13U;
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          !layout.structure1f.valid,
          "Structure1F rejects a family tag that is not documented by the original format");
}

static void test_visible_structure1f_semantics_block_render_plan(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    const int geometry_bytes = 512;
    uint8_t *structure1;
    uint8_t *structure1f;
    Nexus_V1_Level level;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, geometry_bytes) == 0,
          "visible Structure1F fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    structure1[9] = 22;
    set_floor_flags(structure1, structure1b_rel, 3, 4,
                    (uint16_t)((1U << 14) | (21U << 7) | (2U << 4) | 2U));
    set_collision_ref(structure1, structure1b_rel, 3, 4, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 4, 1);
    set_collision_ref(structure1, structure1b_rel, 2, 4, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 4, 4, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 3, 3, 0x0fff);

    build_structure1f_fixture(structure1, structure1b_rel);
    structure1f = structure1 + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 312;
    /* First floor-decoration record: its documented source cell is visible,
     * but no original Saturn draw semantics are currently evidenced. */
    structure1f[32 + 1] = 3;
    structure1f[32 + 2] = 4;

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0,
          "visible Structure1F level loads");
    memset(commands, 0, sizeof(commands));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level, 3, 4, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &receipt) == 0,
          "visible Structure1F render plan evaluates");
    CHECK(receipt.structure1f_plan_direct_entry_count == 0 &&
          receipt.command_count == 0 &&
          receipt.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS &&
          receipt.blocks_real_dgn_mesh_render && !receipt.plan_ready &&
          !receipt.fallback_visuals_permitted,
          "visible unproven Structure1F decoration blocks DGN rendering without fallback");

    /* Item coordinates use the same direct 64x64 source-cell binding. The
     * original Saturn object draw ABI is likewise not established, so an
     * otherwise renderable view must not silently drop a visible item. */
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 1] = 3;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 2] = 4;
    structure1f[32 + 1] = 10;
    structure1f[32 + 2] = 20;
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0,
          "visible Structure1F item level reloads");
    memset(commands, 0, sizeof(commands));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level, 3, 4, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &receipt) == 0,
          "visible Structure1F item render plan evaluates");
    CHECK(receipt.structure1f_plan_direct_entry_count == 0 &&
          receipt.command_count == 0 &&
          receipt.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS &&
          receipt.blocks_real_dgn_mesh_render && !receipt.plan_ready &&
          !receipt.fallback_visuals_permitted,
          "visible unproven Structure1F item blocks DGN rendering without fallback");
}

static void test_direct_structure1f_mesh_command_provenance(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    uint8_t *structure1;
    Nexus_V1_Level level;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;
    Nexus_V1_DgnStructure1FDirectFloorCommandSource direct_sources[2];
    Nexus_V1_DgnStructure1FDirectFloorCommandSourceReceipt direct_receipt;
    int command_index;
    int matched_commands = 0;
    int matched_floor_commands = 0;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 2048) == 0,
          "direct Structure1F mesh-provenance fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    structure1[9] = 22;
    set_floor_flags(structure1, structure1b_rel, 3, 4,
                    (uint16_t)((1U << 14) | (21U << 7) | (2U << 4) | 2U));
    set_collision_ref(structure1, structure1b_rel, 3, 4, 1);
    cell_at(structure1, structure1b_rel, 3, 4)[3] = 4;
    cell_at(structure1, structure1b_rel, 3, 4)[4] = 41;
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 4, 1);
    cell_at(structure1, structure1b_rel, 4, 4)[3] = 12;
    set_collision_ref(structure1, structure1b_rel, 4, 4, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 4, 4, 2);
    set_collision_ref(structure1, structure1b_rel, 3, 3, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 3, 3);
    set_collision_ref(structure1, structure1b_rel, 2, 4, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 2, 3, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 4, 3, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 3, 2, 0x0fff);
    build_direct_structure1f_fixture(structure1, structure1b_rel, 3, 4);

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0,
          "direct Structure1F mesh-provenance level loads");
    memset(commands, 0, sizeof(commands));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level, 3, 4, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &receipt) == 0,
          "direct Structure1F mesh-provenance plan evaluates");
    for (command_index = 0; command_index < receipt.command_count;
         ++command_index) {
        if (commands[command_index].x == 3 && commands[command_index].y == 4) {
            ++matched_commands;
            if (commands[command_index].kind ==
                NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
                ++matched_floor_commands;
                CHECK(commands[command_index].structure1f_direct_entry_count == 1 &&
                      commands[command_index].structure1f_direct_family_mask ==
                          NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_ITEM,
                      "the source-cell floor command retains exact direct item provenance");
            } else {
                CHECK(commands[command_index].structure1f_direct_entry_count == 0 &&
                      commands[command_index].structure1f_direct_family_mask == 0,
                      "direct floor-cell provenance never invents a wall or ceiling relation");
            }
        }
    }
    CHECK(receipt.status == NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS &&
          receipt.blocks_real_dgn_mesh_render && !receipt.plan_ready &&
          !receipt.fallback_visuals_permitted &&
          receipt.structure1f_plan_direct_entry_count == 1 &&
          receipt.structure1f_plan_item_entry_count == 1 &&
          receipt.structure1f_plan_direct_command_count == matched_floor_commands &&
          receipt.structure1f_plan_direct_command_entry_count == matched_floor_commands &&
          receipt.structure1f_plan_direct_floor_command_count == matched_floor_commands &&
          receipt.structure1f_plan_direct_floor_command_entry_count == matched_floor_commands &&
          receipt.structure1f_plan_item_floor_command_count == matched_floor_commands &&
          receipt.structure1f_plan_item_floor_command_entry_count == matched_floor_commands &&
          matched_commands > matched_floor_commands && matched_floor_commands > 0,
          "direct Structure1F source cells bind only to exact floor material commands then remain no-draw");
    memset(direct_sources, 0, sizeof(direct_sources));
    memset(&direct_receipt, 0, sizeof(direct_receipt));
    CHECK(nexus_v1_dgn_bind_direct_structure1f_floor_sources(
              &level, commands, receipt.command_count, direct_sources, 2,
              &direct_receipt) == 0 && direct_receipt.complete &&
          direct_receipt.visible_direct_entry_count == 1 &&
          direct_receipt.floor_command_source_count == 1 &&
          direct_receipt.item_floor_command_source_count == 1 &&
          direct_receipt.floor_decoration_command_source_count == 0 &&
          direct_receipt.floor_sensor_command_source_count == 0 &&
          direct_sources[0].command_index >= 0 &&
          commands[direct_sources[0].command_index].kind ==
              NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
          direct_sources[0].entry.family == NEXUS_V1_DGN_STRUCTURE1F_ITEMS &&
          direct_sources[0].entry.x == 3 && direct_sources[0].entry.y == 4 &&
          !direct_sources[0].draw_authorized &&
          !direct_receipt.fallback_visuals_permitted,
          "direct Structure1F record reaches only its exact runtime floor command without a draw claim");
}

/* Retail-only companion to the mesh-command fixture above. It checks the
 * source relation used by the command binder against each supplied Track 1
 * DGN, without promoting any raw item/decor/sensor bytes to Saturn visuals. */
static void test_real_structure1f_direct_cell_corpus(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    int level_index;
    int checked = 0;

    if (!data_dir || !data_dir[0]) return;
    for (level_index = 0; level_index <= 15; ++level_index) {
        char path[1024];
        FILE *file;
        long size;
        uint8_t *data;
        Nexus_V1_Level level;
        Nexus_V1_DgnStructure1FSpatialReceipt spatial;
        int entry_index;
        int direct_count = 0;
        int item_count = 0;
        int floor_decoration_count = 0;
        int floor_sensor_count = 0;

        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level_index);
        file = fopen(path, "rb");
        CHECK(file != NULL, "real Structure1F direct-cell corpus file opens");
        if (!file) continue;
        CHECK(fseek(file, 0, SEEK_END) == 0,
              "real Structure1F direct-cell corpus file seeks");
        size = ftell(file);
        CHECK(size > 0 && fseek(file, 0, SEEK_SET) == 0,
              "real Structure1F direct-cell corpus file has data");
        if (size <= 0 || fseek(file, 0, SEEK_SET) != 0) {
            fclose(file);
            continue;
        }
        data = (uint8_t *)malloc((size_t)size);
        CHECK(data != NULL, "real Structure1F direct-cell corpus allocates");
        if (!data) {
            fclose(file);
            continue;
        }
        CHECK(fread(data, 1, (size_t)size, file) == (size_t)size,
              "real Structure1F direct-cell corpus reads");
        fclose(file);
        if (nexus_v1_level_load(&level, data, (int)size, level_index) == 0 &&
            nexus_v1_level_structure1f_spatial_receipt(&level, &spatial) == 0) {
            for (entry_index = 0; entry_index < level.structure1f_entry_count;
                 ++entry_index) {
                const Nexus_V1_DgnStructure1FEntry *entry =
                    &level.structure1f_entries[entry_index];
                switch (entry->family) {
                case NEXUS_V1_DGN_STRUCTURE1F_ITEMS:
                    ++item_count;
                    ++direct_count;
                    break;
                case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
                    ++floor_decoration_count;
                    ++direct_count;
                    break;
                case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
                    ++floor_sensor_count;
                    ++direct_count;
                    break;
                default:
                    break;
                }
            }
            CHECK(spatial.valid &&
                  spatial.direct_coordinate_entry_count == direct_count &&
                  spatial.item_entry_count == item_count &&
                  spatial.floor_decoration_entry_count == floor_decoration_count &&
                  spatial.floor_sensor_entry_count == floor_sensor_count,
                  "real Structure1F direct-cell records retain the mesh-command source relation");
            ++checked;
        } else {
            CHECK(0, "real Structure1F direct-cell corpus level loads");
        }
        free(data);
    }
    CHECK(checked == 16,
          "real Structure1F direct-cell corpus covers every supplied retail DGN");
}

static void test_structure1g_semantics_and_bounds(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    uint8_t *structure1;
    Nexus_V1_DgnStructure1Layout layout;
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_Level level;
    Nexus_V1_DgnRendererHandoffReceipt handoff;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 512) == 0,
          "Structure1G fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    build_structure1g_fixture(structure1, structure1b_rel);
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          layout.structure1g.valid && layout.structure1g.descriptor_count == 2 &&
          layout.structure1g.animated_texture_count == 1 &&
          layout.structure1g.sequence_count == 1,
          "Structure1G accepts a bounded descriptor and terminated instruction stream");
    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0 &&
          info.structure1g_present && info.structure1g_valid &&
          info.structure1g_animated_texture_count == 1,
          "Structure1G provenance reaches geometry information");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure1g_entry_count == 1 &&
          level.structure1g_entries[0].animation_id == 7U &&
          level.structure1g_entries[0].first_image_index == 0x0156U &&
          level.structure1g_entries[0].sequence_instruction_count == 2 &&
          level.structure1g_entries[0].image_instruction_count == 1 &&
          nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.structure1g_present && handoff.structure1g_valid &&
          handoff.structure1g_animated_texture_count == level.structure1g_entry_count,
          "Structure1G typed animation declarations are consumed by host handoff");
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24 + 24] = 0U;
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          !layout.structure1g.valid,
          "Structure1G rejects an unterminated animation instruction stream");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.status == NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE_SEMANTICS &&
          handoff.blocks_real_dgn_mesh_render && !handoff.fallback_visuals_permitted,
          "invalid declared Structure1G blocks host promotion without fallback visuals");
}

static void test_structure1g_animated_floor_material_handoff(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 21];
    const int structure1b_rel = 0x40;
    uint8_t *structure1;
    uint8_t *cell;
    Nexus_V1_Level level;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderCommand source_commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;
    Nexus_V1_DgnStructure2FloorCommandSource sources[2];
    Nexus_V1_DgnStructure2FloorCommandSourceReceipt source_receipt;
    int source_command_count;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 512) == 0,
          "Structure1G animated-floor fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    build_structure1g_fixture(structure1, structure1b_rel);
    build_structure2_fixture(dgn);
    cell = cell_at(structure1, structure1b_rel, 1, 1);
    cell[0] = 0x03U;
    cell[1] = 0x80U;
    cell[4] = 3U;
    set_collision_ref(structure1, structure1b_rel, 1, 0, 0x0fff);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure1g_floor_animation_cell_count == 1 &&
          level.structure1g_floor_animation_bound_count == 1,
          "Structure1B animated-floor flag binds only a declared Structure1G id");
    CHECK(level.structure2_texture_table_valid && level.structure2_texture_count == 11 &&
          level.structure1g_entries[0].first_structure2_image_valid &&
          level.structure1g_entries[0].first_structure2_image_id == 10U &&
          level.structure1g_entries[0]
              .structure2_image_instruction_bound_count == 1 &&
          level.structure1g_entries[0]
              .structure2_image_instruction_unbound_count == 0,
          "Structure1G global image instructions bind only through local Structure2 descriptors");
    CHECK(level.structure1g_structure2_bindings_complete,
          "complete Structure1G image declarations are required before host handoff");
    CHECK(level.structure2_payload.valid &&
          level.structure2_payload.descriptor_bytes == 220 &&
          level.structure2_payload.terminator_offset == 220 &&
          level.structure2_payload.opaque_payload_offset == 222 &&
          level.structure2_payload.opaque_payload_size == 18 &&
          level.structure2_payload.opaque_payload_zero_byte_count == 18 &&
          level.structure2_payload.opaque_payload_nonzero_byte_count == 0 &&
          level.structure2_payload.opaque_payload_complete_pair_count == 9 &&
          level.structure2_payload.opaque_payload_trailing_byte_count == 0 &&
          level.structure2_payload.opaque_payload_zero_pair_count == 9 &&
          level.structure2_payload.opaque_payload_nonzero_pair_count == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_count == 0 &&
          level.structure2_payload.nonzero_descriptor_offsets_unaligned_count == 0 &&
          level.structure2_payload.nonzero_descriptor_offsets_word_bounded_count == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_unique_count == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_reused_count == 0 &&
          level.structure2_payload.descriptor_offset_envelope_valid &&
          !level.structure2_payload.local_payload_offset_pattern_observed &&
          !level.structure2_payload
              .local_payload_word_aligned_offset_pattern_observed &&
          !level.structure2_payload
              .local_payload_word_bounded_offset_pattern_observed &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 retains only a bounded opaque payload after its FFFF terminator");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 222U);
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 16, 224U);
    dgn[NEXUS_DGN_BLOCK_SIZE * 20 + 222] = 0xa5U;
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload.opaque_payload_zero_byte_count == 17 &&
          level.structure2_payload.opaque_payload_nonzero_byte_count == 1 &&
          level.structure2_payload.opaque_payload_complete_pair_count == 9 &&
          level.structure2_payload.opaque_payload_trailing_byte_count == 0 &&
          level.structure2_payload.opaque_payload_zero_pair_count == 8 &&
          level.structure2_payload.opaque_payload_nonzero_pair_count == 1 &&
          level.structure2_payload.nonzero_descriptor_offset_count == 2 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_in_opaque_payload_count == 2 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_outside_opaque_payload_count == 0 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_word_bounded_count == 2 &&
          level.structure2_payload.nonzero_descriptor_offset_unique_count == 2 &&
          level.structure2_payload.nonzero_descriptor_offset_reused_count == 0 &&
          level.structure2_payload.local_payload_offset_pattern_observed &&
          level.structure2_payload
              .local_payload_word_aligned_offset_pattern_observed &&
          level.structure2_payload
              .local_payload_word_bounded_offset_pattern_observed &&
          level.structure2_payload.descriptor_offset_envelope_valid &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 records bounded descriptor-offset correlation without decoding payload bytes");
    dgn[NEXUS_DGN_BLOCK_SIZE * 20 + 222] = 0U;
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 16, 222U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_count == 2 &&
          level.structure2_payload.nonzero_descriptor_offset_unique_count == 1 &&
          level.structure2_payload.nonzero_descriptor_offset_reused_count == 1 &&
          level.structure2_payload.local_payload_word_aligned_offset_pattern_observed &&
          level.structure2_payload.local_payload_word_bounded_offset_pattern_observed &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 retains a repeated in-span target as opaque layout provenance");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 16, 224U);
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 223U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_count == 2 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_unaligned_count == 1 &&
          level.structure2_payload.local_payload_offset_pattern_observed &&
          !level.structure2_payload
              .local_payload_word_aligned_offset_pattern_observed &&
          level.structure2_payload
              .local_payload_word_bounded_offset_pattern_observed &&
          !level.structure2_payload.descriptor_offset_envelope_valid &&
          !nexus_v1_level_structure2_source_envelope_valid(&level) &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 records an unaligned in-span target without promoting it");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_ENVELOPE &&
          !handoff.structure2_descriptor_offset_envelope_valid &&
          handoff.blocks_real_dgn_mesh_render &&
          !handoff.fallback_visuals_permitted,
          "unaligned Structure2 descriptor targets fail closed at the host handoff");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 240U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_count == 2 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_in_opaque_payload_count == 1 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_outside_opaque_payload_count == 1 &&
          !level.structure2_payload.local_payload_offset_pattern_observed &&
          !level.structure2_payload
              .local_payload_word_bounded_offset_pattern_observed &&
          !level.structure2_payload.descriptor_offset_envelope_valid &&
          !nexus_v1_level_structure2_source_envelope_valid(&level) &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 leaves out-of-span descriptor offsets non-promoting");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 238U);
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 16, 239U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_in_opaque_payload_count == 2 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_word_bounded_count == 1 &&
          !level.structure2_payload
              .local_payload_word_bounded_offset_pattern_observed &&
          !level.structure2_payload.descriptor_offset_envelope_valid &&
          !nexus_v1_level_structure2_source_envelope_valid(&level) &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 keeps a trailing opaque-byte target non-promoting");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 0U);
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 16, 0U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload.descriptor_offset_envelope_valid &&
          nexus_v1_level_structure2_source_envelope_valid(&level) &&
          level.geometry_info.mesh_ready,
          "animated-floor fixture retains a mesh-ready DGN handoff");
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level, 1, 1, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &receipt) == 0,
          "animated-floor render plan builds");
    CHECK(receipt.animated_material_command_count == 1,
          "animated floor declaration reaches one host material command");
    CHECK(commands[0].animated_texture_structure2_image_valid &&
          commands[0].animated_texture_structure1g_entry_index == 0 &&
          commands[0].animated_texture_structure1g_sequence_word_offset == 0U &&
          commands[0].animated_texture_structure2_image_id == 10U &&
          commands[0].animated_texture_host_route ==
              NEXUS_V1_DGN_ANIMATED_MATERIAL_ROUTE_STRUCTURE2_FLOOR,
          "animated floor host route retains its typed Structure2 image identifier");
    source_command_count = receipt.command_count;
    memcpy(source_commands, commands, sizeof(source_commands));
    memset(sources, 0, sizeof(sources));
    memset(&source_receipt, 0, sizeof(source_receipt));
    CHECK(nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, commands, receipt.command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.animated_floor_command_count == 1 &&
          source_receipt.structure1g_provenance_count == 1 &&
          source_receipt.global_image_index_binding_count == 1 &&
          source_receipt.complete_sequence_provenance_count == 1 &&
          source_receipt.descriptor_offset_envelope_count == 1 &&
          source_receipt.source_command_count == 1 && source_receipt.complete &&
          !source_receipt.fallback_visuals_permitted &&
          sources[0].command_index == 0 &&
          sources[0].structure1g_entry_index == 0 &&
          sources[0].structure1g_sequence_word_offset == 0U &&
          sources[0].structure1g_global_image_index == 0x0156U &&
          sources[0].structure1g_sequence_instruction_count == 2 &&
          sources[0].structure1g_sequence_image_instruction_count == 1 &&
          sources[0].structure1g_sequence_goto_instruction_count == 0 &&
          sources[0].structure1g_sequence_bound_image_count == 1 &&
          sources[0].structure1g_sequence_unbound_image_count == 0 &&
          sources[0].image_offset_word_bounded &&
          sources[0].palette_offset_word_bounded &&
          sources[0].image_id == 10U &&
          sources[0].encoding == 8U && sources[0].palette_id == 0U &&
          sources[0].width == 16U && sources[0].height == 16U &&
          sources[0].structure2_source_envelope_valid &&
          !sources[0].payload_decoder_proven && !sources[0].draw_authorized,
          "Structure1G/Structure2 binds raw original descriptor provenance to its exact floor command only");
    commands[0].animated_texture_structure1g_entry_index = 1;
    CHECK(nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, commands, receipt.command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.source_command_count == 0 &&
          source_receipt.blocked_structure1g_provenance_count == 1 &&
          !source_receipt.complete && !source_receipt.fallback_visuals_permitted,
          "a Structure2 descriptor cannot be detached from its original Structure1G entry");
    memcpy(commands, source_commands, sizeof(commands));
    level.structure1g_entries[0].first_image_index = 0x0157U;
    commands[0].animated_texture_first_image_index = 0x0157U;
    CHECK(nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, commands, receipt.command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.source_command_count == 0 &&
          source_receipt.blocked_global_image_index_count == 1 &&
          !source_receipt.complete && !source_receipt.fallback_visuals_permitted,
          "a Structure1G global image index must resolve to the declared local Structure2 id");
    level.structure1g_entries[0].first_image_index = 0x0156U;
    memcpy(commands, source_commands, sizeof(commands));
    level.structure1g_entries[0].structure2_image_instruction_unbound_count = 1;
    CHECK(nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, commands, receipt.command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.source_command_count == 0 &&
          source_receipt.blocked_sequence_provenance_count == 1 &&
          !source_receipt.complete && !source_receipt.fallback_visuals_permitted,
          "an incompletely bound Structure1G sequence cannot reach a DGN floor command");
    level.structure1g_entries[0].structure2_image_instruction_unbound_count = 0;
    level.structure2_textures[10].image_relative_offset = 240U;
    CHECK(nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, commands, receipt.command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.source_command_count == 0 &&
          source_receipt.blocked_descriptor_offset_envelope_count == 1 &&
          !source_receipt.complete && !source_receipt.fallback_visuals_permitted,
          "a Structure2 descriptor target outside its raw opaque span cannot reach a floor command");
    level.structure2_textures[10].image_relative_offset = 0U;
    CHECK(receipt.unresolved_animated_material_count == 1,
          "animated floor declaration remains unresolved without Structure2 handoff");
    CHECK(receipt.blocks_real_dgn_mesh_render && !receipt.fallback_visuals_permitted,
          "animated floor declaration blocks until a proven Structure2 material handoff exists");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 223U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, source_commands, source_command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.source_command_count == 0 &&
          source_receipt.blocked_source_envelope_count == 1 &&
          !source_receipt.complete && !source_receipt.fallback_visuals_permitted,
          "unaligned Structure2 source envelopes cannot reach a floor command");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 0U);
    /* A bounded sequence that names an absent local descriptor is not a
     * partially valid animation. It must block at handoff, even before a
     * particular viewport happens to look at that floor. */
    wb16(structure1 + structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24 + 8,
         0x0157U);
    wb16(structure1 + structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24 + 20,
         0x0157U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0,
          "unbound Structure1G fixture reloads");
    CHECK(!level.structure1g_structure2_bindings_complete,
          "unbound Structure1G image instruction invalidates the complete binding");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0,
          "unbound Structure1G handoff receipt builds");
    CHECK(handoff.status == NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE,
          "unbound Structure1G image instruction has the Structure2 handoff status");
    CHECK(!handoff.structure1g_structure2_bindings_complete,
          "unbound Structure1G image instruction reaches the host receipt");
    CHECK(handoff.blocks_real_dgn_mesh_render && !handoff.fallback_visuals_permitted,
          "unbound Structure1G image instruction blocks the complete DGN host route");
    wb16(structure1 + structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24 + 8,
         0x0156U);
    wb16(structure1 + structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24 + 20,
         0x0156U);
    wb32(dgn + 0x18, 221U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          !level.structure2_texture_table_valid && !level.structure2_payload.valid,
          "Structure2 rejects a terminator that is not fully bounded by useful bytes");
    wb32(dgn + 0x18, 240U);
    wb16(dgn + NEXUS_DGN_BLOCK_SIZE * 20 +
             10 * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES, 11U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          !level.structure2_texture_table_valid &&
          !level.structure1g_entries[0].first_structure2_image_valid,
          "noncanonical Structure2 descriptor IDs cannot fabricate an animated-material route");
}

static void test_dgn_view_render_plan_from_structure1b(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    const int geometry_bytes = 2048;
    Nexus_V1_Level level;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;
    Nexus_V1_DgnCellGeometry cell;
    uint8_t *structure1;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, geometry_bytes) == 0,
          "render-plan DGN fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    structure1[9] = 22;
    set_floor_flags(structure1, structure1b_rel, 3, 4,
                    (uint16_t)((1U << 14) | (21U << 7) | (2U << 4) | 2U));
    set_collision_ref(structure1, structure1b_rel, 3, 4, 1);
    cell_at(structure1, structure1b_rel, 3, 4)[3] = 4;
    cell_at(structure1, structure1b_rel, 3, 4)[4] = 41;
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 4, 1);
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 152 +
               NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES +
               NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE] = 0x81U;
    cell_at(structure1, structure1b_rel, 4, 4)[3] = 12;
    set_collision_ref(structure1, structure1b_rel, 4, 4, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 4, 4, 2);
    set_collision_ref(structure1, structure1b_rel, 3, 3, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 3, 3);
    set_collision_ref(structure1, structure1b_rel, 2, 4, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 2, 3, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 4, 3, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 3, 2, 0x0fff);
    dgn[NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 4] = (uint8_t)-20;
    dgn[NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 5] = (uint8_t)-10;
    dgn[NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 6] = 20;
    dgn[NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 7] = 10;

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 4) == 0,
          "render-plan level loads from DMWeb DGN");
    CHECK(nexus_v1_level_get_cell_geometry(&level, 3, 4, &cell) == 0 &&
          cell.post_grid_0x30_ref == level.post_grid_0x30_refs[4][3] &&
          cell.post_grid_0x30_row_prefix_valid &&
          cell.floor_material_ref == 21 && cell.floor_height[1] == 12 &&
          !cell.collision_sector.valid,
          "movement and viewport retain a bounded opaque Structure1C reference");
    memset(commands, 0, sizeof(commands));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level,
              3,
              4,
              0,
              commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
              &receipt) == 0,
          "DGN view render plan builds");
    CHECK(receipt.status == NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH &&
          receipt.plan_ready == 1 &&
          receipt.blocks_real_dgn_mesh_render == 0 &&
          receipt.fallback_visuals_permitted == 0,
          "DGN view render plan is ready without fallback visuals");
    CHECK(receipt.command_count == 10 &&
          receipt.floor_count == 3 &&
          receipt.ceiling_count == 3 &&
          receipt.wall_count == 4,
          "DGN view render plan emits bounded floor/ceiling/wall commands");
    CHECK(receipt.material_semantics_complete == 1 &&
          receipt.floor_material_command_count == receipt.floor_count &&
          receipt.ceiling_material_command_count == receipt.ceiling_count &&
          receipt.wall_material_command_count == receipt.wall_count,
          "DGN render plan proves Structure1B surface semantics per command family");
    CHECK(receipt.post_grid_0x30_reference_command_count > 0 &&
          receipt.post_grid_0x30_valid_reference_command_count ==
              receipt.post_grid_0x30_reference_command_count &&
          receipt.first_post_grid_0x30_ref == 1 &&
          receipt.max_post_grid_0x30_ref == 3,
          "DGN view render plan consumes only bounded opaque-record references");
    CHECK(receipt.post_grid_0x30_row_ordinal_flagged_prefix_record_count == 1 &&
          receipt.post_grid_0x30_first_row_ordinal_flagged_prefix_record == 1 &&
          receipt.post_grid_0x30_last_row_ordinal_flagged_prefix_record == 1,
          "DGN render plan carries row-order flag provenance without resolving references as ordinals");
    CHECK(commands[0].kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
          commands[0].x == 3 &&
          commands[0].y == 4 &&
          commands[0].collision_ref == 1 &&
          commands[0].post_grid_0x30_ref == 1 &&
          commands[0].post_grid_0x30_row_prefix_valid &&
          commands[0].material_id == 21 && commands[0].floor_rotation == 1 &&
          commands[0].material_source_kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
          commands[0].floor_slope == 2 &&
          commands[0].floor_height[0] == 4 &&
          commands[0].floor_height[1] == 12 &&
          commands[0].quad_y[0] == 1008 &&
          commands[0].quad_y[1] == 688 &&
          commands[0].quad_y[2] == 496 &&
          commands[0].quad_y[3] == 603 &&
          !commands[0].collision_sector.valid &&
          commands[0].palette_index == 21 &&
          commands[0].quad_y[0] > commands[0].quad_y[2],
          "DGN plan projects Structure1B floor heights instead of a flat host quad");
    CHECK(commands[1].kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING &&
          commands[1].material_id == 22 &&
          commands[1].material_source_kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING &&
          commands[1].floor_height[2] == 12 &&
          commands[1].ceiling_height[2] == 44 &&
          commands[1].quad_y[0] == -272 &&
          commands[1].quad_y[1] == 176 &&
          commands[1].quad_y[2] == 70 &&
          commands[1].quad_y[3] == -592 &&
          commands[1].palette_index == 22 &&
          commands[1].quad_y[0] < commands[1].quad_y[1],
          "DGN plan projects matching Structure1B ceiling heights");
    CHECK(commands[4].kind == NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT &&
          commands[4].wall_dir == 3 &&
          commands[4].x == 3 &&
          commands[4].y == 4 &&
          commands[4].material_id == 41 &&
          commands[4].material_source_kind == NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT &&
          commands[4].quad_x[0] < commands[4].quad_x[1],
          "DGN render plan projects the left wall from Structure1B visibility");
    CHECK(receipt.first_blocking_depth == 2 &&
          receipt.first_blocking_x == 3 &&
          receipt.first_blocking_y == 2 &&
          commands[9].kind == NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT &&
          commands[9].quad_y[0] > commands[9].quad_y[2],
          "DGN render plan stops at first front wall");
}

static void test_structure1f_out_of_prefix_ref_blocks_mesh(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_Level level;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;
    uint8_t *structure1;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 256) == 0,
          "Structure1F prefix-overrun fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    set_collision_ref(structure1, structure1b_rel, 3, 3, 1);
    /* 256 bytes gives six rows: records 0..4 are the observed prefix;
     * record 5 is the opaque tail and cannot be selected by a cell. */
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 3, 5);

    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0 &&
          !info.post_grid_0x30_references_valid &&
          info.post_grid_0x30_invalid_ref_count == 1 &&
          info.first_invalid_post_grid_0x30_ref == 5 &&
          !info.mesh_ready,
          "an opaque Structure1F tail reference blocks mesh promotion");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_REFERENCE &&
          !handoff.post_grid_0x30_references_valid &&
          handoff.first_invalid_post_grid_0x30_ref == 5 &&
          handoff.blocks_real_dgn_mesh_render &&
          !handoff.fallback_visuals_permitted,
          "DGN handoff identifies and fails closed for an unproven Structure1F reference");
    CHECK(strcmp(nexus_v1_dgn_renderer_handoff_status_name(handoff.status),
                 "blocked-structure1f-reference") == 0,
          "unproven Structure1F reference keeps a stable no-fallback receipt name");
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level, 3, 3, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &receipt) == 0 &&
          receipt.blocks_real_dgn_mesh_render && receipt.command_count == 0,
          "blocked Structure1F reference emits no synthetic render plan");
}

static void test_descriptor_budget_blocks_mesh_ready(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 21];
    const int structure1b_rel = 0x40;
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_Level level;
    uint8_t *structure1;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 20,
                          structure1b_rel, 8) == 0,
          "small-geometry DGN buffer builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    set_collision_ref(structure1, structure1b_rel, 8, 8, 3);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 8, 8, 10);

    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0,
          "small-geometry DGN still parses");
    CHECK(info.max_collision_ref == 3,
          "small-geometry max ref captured");
    CHECK(info.max_post_grid_0x30_ref == 10,
          "small-geometry max opaque-record ref captured");
    CHECK(info.geometry_size == 8,
          "small-geometry span size captured");
    CHECK(info.mesh_ready == 0,
          "descriptor budget overflow does not promote mesh readiness");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 2) == 0,
          "small-geometry DGN still loads as a level");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0,
          "DGN renderer handoff receipt builds for budget-blocked level");
    CHECK(handoff.status ==
          NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY,
          "DGN renderer handoff blocks an untyped 0x30 span");
    CHECK(handoff.blocks_real_dgn_mesh_render == 1 &&
          handoff.fallback_visuals_permitted == 0,
          "DGN untyped-span handoff forbids fallback visuals");
}

static void test_post_grid_0x30_row_prefix_rejection(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    uint8_t *structure1;
    Nexus_V1_DgnStructure1Layout layout;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 256) == 0,
          "row-prefix rejection fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 152 +
               NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE] = 0x40U;
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          !layout.post_grid_0x30_records.valid &&
          !layout.post_grid_0x30_records.row_ordinal_prefix_valid,
          "0x30 row prefix rejects an unobserved ordinal bit");
}

static void test_structure1c_bytes_do_not_invent_collision_geometry(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    Nexus_V1_Level level;
    uint8_t *structure1;
    uint8_t *sector;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 2048) == 0,
          "Structure1C movement fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    set_collision_ref(structure1, structure1b_rel, 8, 8, 1);
    set_collision_ref(structure1, structure1b_rel, 8, 9, 1);
    wb32(structure1 + 0x24,
         (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 8));
    sector = dgn + NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
             NEXUS_DGN_STRUCTURE1B_BYTES;
    sector[0] = 2; /* one descriptor follows */
    sector[4] = (uint8_t)-100;
    sector[5] = 40;
    sector[6] = 100;
    sector[7] = 40;

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 5) == 0,
          "Structure1C movement level loads");
    CHECK(nexus_v1_level_move_allowed(&level, 8, 9, 8, 8),
          "opaque Structure1C bytes do not invent a blocking segment");
    CHECK(nexus_v1_level_move_allowed(&level, 7, 8, 8, 8),
          "documented Structure1B passability remains usable");
}

static void test_bounds_and_legacy_non_promotion(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 21];
    uint8_t legacy[64];
    Nexus_V1_DgnGeometryInfo info;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 20, 0x40, 16) == 0,
          "bounds fixture builds");
    wb32(dgn + 0x10, (uint32_t)(0x40 + NEXUS_DGN_STRUCTURE1B_BYTES - 1));
    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) != 0,
          "truncated useful Structure1 size is rejected");

    memset(legacy, 0, sizeof(legacy));
    wb16(legacy + 0, 4U);
    wb16(legacy + 2, 4U);
    CHECK(nexus_v1_dgn_geometry_info(&info, legacy, (int)sizeof(legacy)) != 0,
          "legacy synthetic layout does not parse as DMWeb geometry info");
    CHECK(nexus_v1_level_load(NULL, legacy, (int)sizeof(legacy), 99) != 0,
          "Nexus DGN loader rejects unsupported input before mutating runtime state");
    {
        Nexus_V1_Level level;
        CHECK(nexus_v1_level_load(&level, legacy, (int)sizeof(legacy), 99) != 0,
              "legacy raw-grid fixture cannot become a Nexus runtime level");
    }
}

static void test_determinism(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 21];
    Nexus_V1_DgnGeometryInfo a;
    Nexus_V1_DgnGeometryInfo b;
    uint8_t *structure1;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 20, 0x308, 128) == 0,
          "determinism fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    set_collision_ref(structure1, 0x308, 10, 10, 7);
    set_collision_ref(structure1, 0x308, 11, 10, 2);

    CHECK(nexus_v1_dgn_geometry_info(&a, dgn, (int)sizeof(dgn)) == 0,
          "first deterministic parse succeeds");
    CHECK(nexus_v1_dgn_geometry_info(&b, dgn, (int)sizeof(dgn)) == 0,
          "second deterministic parse succeeds");
    CHECK(memcmp(&a, &b, sizeof(a)) == 0,
          "geometry info parse is byte-stable across runs");
}

static void test_structure1f_item_ibs_material_binding(void) {
    uint8_t *ibs = (uint8_t *)calloc(1, NEXUS_V1_ITEM_IBS_BYTES);
    Nexus_V1_ItemIbsBank bank;
    Nexus_V1_Level level;
    Nexus_V1_DgnRenderCommand commands[2];
    Nexus_V1_DgnStructure1FItemMaterialBinding bindings[2];
    Nexus_V1_DgnStructure1FItemMaterialReceipt receipt;
    Nexus_V1_DgnCommandPacked4BppMaterial materials[2];
    Nexus_V1_DgnCommandPacked4BppMaterialReceipt material_receipt;
    Nexus_V1_DgnStructure1FItemIbsCoverageReceipt coverage;
    Nexus_V1_ItemIbs0008Vdp1Provenance provenance;
    Nexus_V1_ItemIbs0008CodecReceipt codec_receipt;
    uint8_t decoded[NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_TEXELS];
    int i;

    CHECK(ibs != NULL, "ITEM.IBS material fixture allocates");
    if (!ibs) return;
    /* DMWeb ITEM.IBS: 243 x 40-byte declarations at 0x0800, regular
     * palettes at 0x3000, association table at 0x3100 and 16x16 4bpp
     * images at 0x3300.  The fixture proves the documented FFFF route only. */
    for (i = 0; i < NEXUS_V1_ITEM_IBS_DECLARATION_COUNT; ++i) {
        uint8_t *decl = ibs + 0x0800 + i * 40;
        decl[0] = (uint8_t)i;
        wb16(decl + 0x14, 3U);
        wb16(decl + 0x16, 0xffffU);
    }
    wb16(ibs + 0x3000 + 1 * 16 * 2 + 2 * 2, 0x7c00U);
    ibs[0x3100 + 3 * 2] = 1;
    ibs[0x3100 + 3 * 2 + 1] = 7;
    ibs[0x3300 + 7 * 128] = 0x2fU;
    wb16(ibs + 0xa800, 43U); /* 223 regular images + ordinal 43 = 266 */
    wb16(ibs + 0xa802, 8U);
    wb16(ibs + 0xa806, 16U);
    wb16(ibs + 0xa808, 16U);
    wb32(ibs + 0xa80c, 0x1000U);
    wb32(ibs + 0xa810, 0x0900U);
    wb16(ibs + 0xa800 + 20, 0xffffU);
    wb32(ibs + 0xa800 + 20 + 12, 0x1100U);
    wb16(ibs + 0xa800 + 0x0900, 0x03e0U);
    ibs[0xa800 + 0x1000] = 0x6cU;

    CHECK(nexus_v1_item_ibs_parse_verified(ibs, NEXUS_V1_ITEM_IBS_BYTES,
                                             0, &bank) != 0,
          "unverified ITEM.IBS never becomes a material bank");
    CHECK(nexus_v1_item_ibs_parse_verified(ibs, NEXUS_V1_ITEM_IBS_BYTES,
                                             1, &bank) == 0,
          "hash-verified ITEM.IBS regular-icon table parses");
    memset(&level, 0, sizeof(level));
    level.structure1f_entry_count = 2;
    level.structure1f_entries[0].family = NEXUS_V1_DGN_STRUCTURE1F_ITEMS;
    level.structure1f_entries[0].x = 4;
    level.structure1f_entries[0].y = 5;
    level.structure1f_entries[0].item_id = 5;
    level.structure1f_entries[1].family = NEXUS_V1_DGN_STRUCTURE1F_ITEMS;
    level.structure1f_entries[1].x = 6;
    level.structure1f_entries[1].y = 7;
    level.structure1f_entries[1].item_id = 6;
    bank.floor_image[6] = 266U; /* documented separate floor-image range */
    memset(commands, 0, sizeof(commands));
    commands[0].kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
    commands[0].x = 4;
    commands[0].y = 5;
    commands[1].kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
    commands[1].x = 6;
    commands[1].y = 7;
    CHECK(nexus_v1_dgn_bind_structure1f_item_materials(
              &level, &bank, commands, 2, bindings, 2, &receipt) == 0,
          "Structure1Fa items bind to matching mesh-command cells");
    CHECK(nexus_v1_dgn_structure1f_item_ibs_coverage(&level, &bank, &coverage) == 0 &&
          coverage.complete && coverage.dgn_item_entry_count == 2 &&
          coverage.inventory_inherited_item_count == 1 &&
          coverage.special_floor_reference_count == 1 &&
          coverage.special_floor_0008_count == 1 &&
          !coverage.fallback_visuals_permitted,
          "Structure1Fa coverage binds each source item to ITEM.IBS without a substitute");
    CHECK(receipt.bound_regular_inventory_count == 1 &&
          receipt.bound_special_floor_palette_count == 1 &&
          !receipt.fallback_visuals_permitted,
          "special floor descriptor carries its local palette but no pixel fallback");
    CHECK(bindings[0].command_index == 0 && bindings[0].item_id == 5 &&
          bindings[0].palette_index == 1 && bindings[0].image_index == 7 &&
          bindings[0].palette_bgr555[2] == 0x7c00U &&
          bindings[0].packed_4bpp_texels[0] == 0x2fU,
          "binding carries the exact ITEM.IBS palette and packed texels");
    CHECK(bindings[1].special_floor_image != NULL &&
          bindings[1].special_floor_image->encoding == 8U &&
          bindings[1].special_floor_image->palette_bgr555[0] == 0x03e0U &&
          bindings[1].special_floor_image->image_id == 266U &&
          bindings[1].special_floor_image->packed_4bpp_bytes == 128U &&
          bindings[1].special_floor_image->packed_4bpp_valid &&
          bindings[1].packed_4bpp_texels == NULL,
          "special floor binding preserves bounded original payload and palette only");
    memset(decoded, 0xa5, sizeof(decoded));
    CHECK(nexus_v1_item_ibs_decode_0008_vdp1_4bpp(
              bindings[1].special_floor_image, NULL, decoded,
              (int)sizeof(decoded), &codec_receipt) == 0 &&
          codec_receipt.descriptor_0008_verified &&
          codec_receipt.packed_span_verified && codec_receipt.palette_bound &&
          codec_receipt.blocked_missing_vdp1_command_provenance &&
          !codec_receipt.decode_authorized && decoded[0] == 0xa5U &&
          !codec_receipt.fallback_visuals_permitted,
          "ITEM.IBS alone cannot authorize a VDP1 nibble-order decode");
    memset(&provenance, 0, sizeof(provenance));
    provenance.item_ibs_hash_verified = 1;
    provenance.original_vdp1_command_stream_verified = 1;
    provenance.vdp1_16_colour_mode_verified = 1;
    provenance.vdp1_high_nibble_first_verified = 1;
    CHECK(nexus_v1_item_ibs_decode_0008_vdp1_4bpp(
              bindings[1].special_floor_image, &provenance, decoded,
              (int)sizeof(decoded), &codec_receipt) == 0 &&
          codec_receipt.source_hash_verified && codec_receipt.decode_authorized &&
          codec_receipt.decoded_texel_count == 256 && decoded[0] == 6U &&
          decoded[1] == 12U && !codec_receipt.fallback_visuals_permitted,
          "source-proven VDP1 mode expands descriptor-0008 high nibble first");
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              bindings, 2, commands, 2, materials, 2, &material_receipt) == 0 &&
          material_receipt.complete &&
          material_receipt.special_floor_binding_count == 1 &&
          material_receipt.source_cell_match_count == 1 &&
          material_receipt.command_material_count == 1 &&
          !material_receipt.fallback_visuals_permitted,
          "descriptor-0008 is consumed by its matching DGN floor command only");
    CHECK(materials[0].command_index == 1 && materials[0].source_entry_index == 1 &&
          materials[0].source_x == 6 && materials[0].source_y == 7 &&
          materials[0].item_id == 6U && materials[0].image_id == 266U &&
          materials[0].encoding == 8U && materials[0].width == 16U &&
          materials[0].height == 16U && materials[0].packed_4bpp_bytes == 128U &&
          materials[0].palette_bgr555[0] == 0x03e0U &&
          materials[0].packed_4bpp_texels[0] == 0x6cU &&
          materials[0].source_hash_verified && materials[0].packed_4bpp_valid &&
          !materials[0].texel_order_proven && !materials[0].draw_authorized,
          "command material keeps exact authenticated 4bpp bytes no-draw");
    bindings[1].source_x = 4;
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              bindings, 2, commands, 2, materials, 2, &material_receipt) == 0 &&
          !material_receipt.complete &&
          material_receipt.blocked_source_cell_mismatch_count == 1 &&
          material_receipt.command_material_count == 0 &&
          !material_receipt.fallback_visuals_permitted,
          "a descriptor-0008 source cell cannot be rebound to another DGN floor");
    bindings[1].source_x = 6;
    bindings[1].command_index = 2;
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              bindings, 2, commands, 2, materials, 2, &material_receipt) == 0 &&
          !material_receipt.complete &&
          material_receipt.blocked_invalid_command_count == 1 &&
          material_receipt.command_material_count == 0 &&
          !material_receipt.fallback_visuals_permitted,
          "out-of-range DGN command cannot promote a descriptor-0008 surface");
    level.structure1f_entries[1].item_id = 250U;
    CHECK(nexus_v1_dgn_structure1f_item_ibs_coverage(&level, &bank, &coverage) == 0 &&
          !coverage.complete && coverage.blocked_invalid_item_count == 1 &&
          !coverage.fallback_visuals_permitted,
          "an unsupported Structure1Fa item reference remains blocked");
    free(ibs);
}

static void test_real_item_ibs_special_floor_corpus(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[1024];
    uint8_t *data;
    FILE *file;
    Nexus_V1_ItemIbsBank bank;
    Nexus_V1_DgnStructure1FItemMaterialBinding binding;
    Nexus_V1_DgnRenderCommand command;
    Nexus_V1_DgnCommandPacked4BppMaterial material;
    Nexus_V1_DgnCommandPacked4BppMaterialReceipt material_receipt;
    Nexus_V1_ItemIbs0008CodecReceipt codec_receipt;
    uint8_t decoded[NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_TEXELS];
    int level_index;
    int checked_levels = 0;
    int total_dgn_items = 0;
    int total_special_floor_references = 0;
    int total_blocked_references = 0;

    if (!data_dir || !data_dir[0]) return;
    snprintf(path, sizeof(path), "%s/ITEM.IBS", data_dir);
    file = fopen(path, "rb");
    if (!file) return;
    data = (uint8_t *)malloc(NEXUS_V1_ITEM_IBS_BYTES);
    CHECK(data != NULL, "real ITEM.IBS corpus buffer allocates");
    if (!data) {
        fclose(file);
        return;
    }
    CHECK(fread(data, 1, NEXUS_V1_ITEM_IBS_BYTES, file) ==
              NEXUS_V1_ITEM_IBS_BYTES,
          "real ITEM.IBS corpus reads completely");
    fclose(file);
    CHECK(nexus_v1_item_ibs_parse_verified(data, NEXUS_V1_ITEM_IBS_BYTES,
                                             1, &bank) == 0,
          "real ITEM.IBS proves the special-floor packed-4bpp layout");
    CHECK(bank.floor_image_count == NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT &&
          bank.floor_images[43].image_id == 266U &&
          bank.floor_images[43].packed_4bpp_valid &&
          bank.floor_images[43].packed_4bpp_bytes ==
              (uint32_t)bank.floor_images[43].width *
              (uint32_t)bank.floor_images[43].height / 2U,
          "real ITEM.IBS combines regular and floor-image indices without a fallback");
    memset(&binding, 0, sizeof(binding));
    memset(&command, 0, sizeof(command));
    binding.command_index = 0;
    binding.special_floor_image = &bank.floor_images[43];
    command.kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              &binding, 1, &command, 1, &material, 1, &material_receipt) == 0 &&
          material_receipt.complete && material.source_hash_verified &&
          material.image_id == bank.floor_images[43].image_id &&
          material.width == bank.floor_images[43].width &&
          material.height == bank.floor_images[43].height &&
          material.packed_4bpp_texels == bank.floor_images[43].packed_4bpp_texels &&
          material.palette_bgr555 == bank.floor_images[43].palette_bgr555 &&
          !material.draw_authorized,
          "real Saturn descriptor-0008 payload reaches a DGN command no-draw");
    CHECK(nexus_v1_item_ibs_decode_0008_vdp1_4bpp(
              &bank.floor_images[43], NULL, decoded, (int)sizeof(decoded),
              &codec_receipt) == 0 &&
          codec_receipt.descriptor_0008_verified &&
          codec_receipt.packed_span_verified && codec_receipt.palette_bound &&
          codec_receipt.blocked_missing_vdp1_command_provenance &&
          !codec_receipt.decode_authorized &&
          !codec_receipt.fallback_visuals_permitted,
          "real Saturn ITEM.IBS stays blocked without an original VDP1 command receipt");
    for (level_index = 0; level_index <= 15; ++level_index) {
        Nexus_V1_Level level;
        Nexus_V1_DgnStructure1FItemIbsCoverageReceipt coverage;
        uint8_t *level_data;
        FILE *level_file;
        long level_size;

        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level_index);
        level_file = fopen(path, "rb");
        CHECK(level_file != NULL, "real ITEM.IBS coverage DGN opens");
        if (!level_file) continue;
        CHECK(fseek(level_file, 0, SEEK_END) == 0,
              "real ITEM.IBS coverage DGN seeks");
        level_size = ftell(level_file);
        CHECK(level_size > 0 && fseek(level_file, 0, SEEK_SET) == 0,
              "real ITEM.IBS coverage DGN has data");
        if (level_size <= 0 || fseek(level_file, 0, SEEK_SET) != 0) {
            fclose(level_file);
            continue;
        }
        level_data = (uint8_t *)malloc((size_t)level_size);
        CHECK(level_data != NULL, "real ITEM.IBS coverage DGN allocates");
        if (!level_data) {
            fclose(level_file);
            continue;
        }
        CHECK(fread(level_data, 1, (size_t)level_size, level_file) ==
                  (size_t)level_size,
              "real ITEM.IBS coverage DGN reads");
        fclose(level_file);
        CHECK(nexus_v1_level_load(&level, level_data, (int)level_size,
                                  level_index) == 0 &&
                  nexus_v1_dgn_structure1f_item_ibs_coverage(
                      &level, &bank, &coverage) == 0 &&
                  !coverage.fallback_visuals_permitted,
              "retail DGN Structure1Fa coverage refuses unproved ITEM.IBS routes");
        if (nexus_v1_level_load(&level, level_data, (int)level_size,
                                level_index) == 0 &&
            nexus_v1_dgn_structure1f_item_ibs_coverage(
                &level, &bank, &coverage) == 0) {
            total_dgn_items += coverage.dgn_item_entry_count;
            total_special_floor_references += coverage.special_floor_reference_count;
            total_blocked_references += coverage.blocked_invalid_item_count +
                coverage.blocked_missing_floor_image_count +
                coverage.blocked_unsupported_encoding_count;
            ++checked_levels;
        }
        free(level_data);
    }
    CHECK(checked_levels == 16 && total_dgn_items > 0 &&
          total_special_floor_references > 0 && total_blocked_references == 0,
          "retail DGN direct items reach verified descriptor-0008 sources without fallback");
    free(data);
}

int main(void) {
    test_variable_grid_and_mesh_ready();
    test_dgn_view_render_plan_from_structure1b();
    test_structure1f_out_of_prefix_ref_blocks_mesh();
    test_descriptor_budget_blocks_mesh_ready();
    test_post_grid_0x30_row_prefix_rejection();
    test_structure1c_bytes_do_not_invent_collision_geometry();
    test_bounds_and_legacy_non_promotion();
    test_determinism();
    test_structure1f_item_ibs_material_binding();
    test_real_item_ibs_special_floor_corpus();
    test_structure1c_record_table_bounds();
    test_structure1f_semantics_and_bounds();
    test_visible_structure1f_semantics_block_render_plan();
    test_direct_structure1f_mesh_command_provenance();
    test_structure1g_semantics_and_bounds();
    test_structure1g_animated_floor_material_handoff();
    test_real_dgn_structure1_layout_corpus();
    test_real_structure1f_direct_cell_corpus();

    if (g_fail != 0) {
        printf("Nexus V1 DGN geometry readiness gate: %d failure(s)\n", g_fail);
        return 1;
    }

    printf("Nexus V1 DGN geometry readiness gate passed\n");
    return 0;
}
