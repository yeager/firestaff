#include "nexus_v1_dungeon.h"

#include <stdint.h>
#include <stdio.h>
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
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_Level level;
    uint8_t *structure1;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 24,
                          structure1b_rel, geometry_bytes) == 0,
          "synthetic DMWeb DGN buffer builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;

    set_collision_ref(structure1, structure1b_rel, 2, 2, 1);
    set_collision_ref(structure1, structure1b_rel, 3, 2, 5);
    set_collision_ref(structure1, structure1b_rel, 4, 2, 5);
    set_collision_ref(structure1, structure1b_rel, 0, 0, 0x0fff);
    cell_at(structure1, structure1b_rel, 5, 5)[1] = 0x01; /* door flag */

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
    CHECK(info.mesh_ready == 1,
          "bounded descriptor budget marks fixture mesh-ready");

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
    CHECK(handoff.descriptor_capacity == geometry_bytes / 4 &&
          handoff.max_collision_ref == 5,
          "DGN renderer handoff exposes descriptor budget and max ref");
    CHECK(strcmp(nexus_v1_dgn_renderer_handoff_status_name(handoff.status),
                 "ready-mesh") == 0,
          "DGN renderer handoff has stable ready route name");
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
    cell_at(structure1, structure1b_rel, 4, 4)[3] = 12;
    set_collision_ref(structure1, structure1b_rel, 4, 4, 1);
    set_collision_ref(structure1, structure1b_rel, 3, 3, 1);
    set_collision_ref(structure1, structure1b_rel, 2, 4, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 2, 3, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 4, 3, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 3, 2, 0x0fff);
    dgn[NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES] = 2;
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
          cell.mesh_ref == level.mesh_refs[4][3] &&
          cell.floor_material_ref == 21 && cell.floor_height[1] == 12 &&
          cell.collision_sector.valid == 1 && cell.collision_sector.x1 == -20,
          "movement and viewport share one decoded DGN cell geometry record");
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
          receipt.wall_count == 4,
          "DGN view render plan emits bounded floor/ceiling/wall commands");
    CHECK(commands[0].kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
          commands[0].x == 3 &&
          commands[0].y == 4 &&
          commands[0].collision_ref == 1 &&
          commands[0].material_id == 21 && commands[0].floor_rotation == 1 &&
          commands[0].floor_slope == 2 &&
          commands[0].floor_height[0] == 4 &&
          commands[0].floor_height[1] == 12 &&
          commands[0].collision_sector.valid == 1 &&
          commands[0].collision_sector.x1 == -20 &&
          commands[0].palette_index == 21 &&
          commands[0].quad_y[0] > commands[0].quad_y[2],
          "DGN plan carries real tile height, slope, rotation and sector fields");
    CHECK(commands[1].kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING &&
          commands[1].material_id == 22 && commands[1].floor_height[2] == 12 &&
          commands[1].ceiling_height[2] == 44 && commands[1].palette_index == 22 &&
          commands[1].quad_y[0] < commands[1].quad_y[1],
          "DGN plan selects the header-referenced ceiling texture and height");
    CHECK(commands[4].kind == NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT &&
          commands[4].wall_dir == 3 &&
          commands[4].x == 3 &&
          commands[4].y == 4 &&
          commands[4].material_id == 41 &&
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

    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0,
          "small-geometry DGN still parses");
    CHECK(info.max_collision_ref == 3,
          "small-geometry max ref captured");
    CHECK(info.geometry_size == 8,
          "small-geometry span size captured");
    CHECK(info.mesh_ready == 0,
          "descriptor budget overflow does not promote mesh readiness");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 2) == 0,
          "small-geometry DGN still loads as a level");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0,
          "DGN renderer handoff receipt builds for budget-blocked level");
    CHECK(handoff.status ==
          NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_DESCRIPTOR_BUDGET,
          "DGN renderer handoff blocks on descriptor budget");
    CHECK(handoff.blocks_real_dgn_mesh_render == 1 &&
          handoff.fallback_visuals_permitted == 0,
          "DGN budget-blocked handoff forbids fallback visuals");
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
    test_collision_sector_blocks_entry();
    test_bounds_and_legacy_non_promotion();
    test_determinism();

    if (g_fail != 0) {
        printf("Nexus V1 DGN geometry readiness gate: %d failure(s)\n", g_fail);
        return 1;
    }

    printf("Nexus V1 DGN geometry readiness gate passed\n");
    return 0;
}
