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

static void set_floor_flags(uint8_t *structure1, int structure1b_rel,
                            int x, int y, uint16_t flags) {
    uint8_t *cell = cell_at(structure1, structure1b_rel, x, y);
    cell[0] = (uint8_t)(flags >> 8);
    cell[1] = (uint8_t)(flags & 0xffU);
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
    set_post_grid_0x30_ref(structure1, structure1b_rel, 4, 2, 5);
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
          info.max_post_grid_0x30_ref == 5 &&
          info.post_grid_0x30_ref_value_count == 3,
          "packed-high post-grid values are counted without claiming row indexing");
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
          handoff.max_post_grid_0x30_ref == 5 &&
          handoff.post_grid_0x30_ref_unique_count == 2 &&
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
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
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
              layout.post_grid[1].header_offset == 0x24 &&
              layout.post_grid[2].header_offset == 0x2c &&
              layout.post_grid[3].header_offset == 0x30 &&
              layout.post_grid[4].header_offset == 0x34,
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
        checked++;
        free(data);
    }
    CHECK(checked == 16, "all LEV00 through LEV15 files were checked");
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
          cell.collision_sector.valid == 1 && cell.collision_sector.x1 == -20,
          "movement and viewport share one decoded DGN cell and bounded opaque record reference");
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
          commands[0].collision_sector.valid == 1 &&
          commands[0].collision_sector.x1 == -20 &&
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

static void test_collision_sector_blocks_entry(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    Nexus_V1_Level level;
    uint8_t *structure1;
    uint8_t *sector;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 2048) == 0,
          "collision movement fixture builds");
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
          "collision movement level loads");
    CHECK(!nexus_v1_level_move_allowed(&level, 8, 9, 8, 8),
          "DGN collision sector blocks a party entering through its segment");
    CHECK(nexus_v1_level_move_allowed(&level, 7, 8, 8, 8),
          "same DGN sector permits entry that does not cross its segment");
}

static void test_bounds_and_legacy_non_promotion(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 21];
    uint8_t legacy[64];
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_Level level;

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
    CHECK(nexus_v1_level_load(&level, legacy, (int)sizeof(legacy), 99) == 0,
          "legacy synthetic fallback still loads");
    CHECK(level.geometry_info.dmweb_container == 0,
          "legacy fallback does not promote DMWeb geometry info");
    CHECK(level.geometry_info.mesh_ready == 0,
          "legacy fallback cannot be mesh-ready");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0,
          "legacy level still emits renderer handoff receipt");
    CHECK(handoff.status ==
          NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_LEGACY_FALLBACK,
          "legacy synthetic level blocks real DGN mesh handoff");
    CHECK(handoff.blocks_real_dgn_mesh_render == 1 &&
          handoff.fallback_visuals_permitted == 0,
          "legacy handoff forbids fallback visuals for real DGN route");
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

int main(void) {
    test_variable_grid_and_mesh_ready();
    test_dgn_view_render_plan_from_structure1b();
    test_descriptor_budget_blocks_mesh_ready();
    test_post_grid_0x30_row_prefix_rejection();
    test_collision_sector_blocks_entry();
    test_bounds_and_legacy_non_promotion();
    test_determinism();
    test_structure1c_record_table_bounds();
    test_real_dgn_structure1_layout_corpus();

    if (g_fail != 0) {
        printf("Nexus V1 DGN geometry readiness gate: %d failure(s)\n", g_fail);
        return 1;
    }

    printf("Nexus V1 DGN geometry readiness gate passed\n");
    return 0;
}
