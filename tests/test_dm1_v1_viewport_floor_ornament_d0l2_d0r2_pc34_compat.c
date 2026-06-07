#include "dm1_v1_viewport_floor_ornament_d0l2_d0r2_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
        ++g_failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void test_d0l2_corridor_routes_to_f0115_without_f0108(void)
{
    DM1_V1_FloorOrnamentD0L2D0R2InputPc34 in = {
        DM1_V1_D0L2_D0R2_ROUTE_D0L2_PC34,
        DM1_V1_D0L2_D0R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        7,
        0x23,
        0x2A
    };
    DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0125 line 8005 calls F0115 for D0L. */
    expect_int("d0l2.resolve",
               M11_GameView_FloorOrnamentD0L2D0R2ResolvePc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DEFS.H lines 2587-2589 define M610_VIEW_SQUARE_D0L as 10. */
    expect_int("d0l2.view_square", out.view_square_index, 10);
    /* ReDMCSB: DEFS.H lines 4036-4038 define C714_ZONE_WALL_D0L. */
    expect_int("d0l2.wall_zone", out.wall_zone_index, 714);
    /* ReDMCSB: DUNVIEW.C F0125 line 8005 passes C0x0002_CELL_ORDER_BACKRIGHT. */
    expect_int("d0l2.cell_order", out.f0115_cell_order, 0x0002);
    /* ReDMCSB: DEFS.H line 2644 defines C02_VIEW_CELL_BACK_RIGHT. */
    expect_int("d0l2.boundary_cell", out.documented_boundary_cell, 2);
    /* ReDMCSB: DUNVIEW.C F0125 lines 7999-8005 do not read M558. */
    expect_int("d0l2.no_floor_flag_read", out.reads_floor_ornament_flag ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0125 line 8005 calls F0115, not F0108. */
    expect_int("d0l2.no_f0108", out.calls_f0108 ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0125 line 8005 dispatches objects/creatures/projectiles/explosions. */
    expect_int("d0l2.calls_f0115", out.calls_f0115 ? 1 : 0, 1);
    /* ReDMCSB: DEFS.H lines 2739-2747 have no D0 floor-view slot. */
    expect_int("d0l2.unsupported_view_floor", out.unsupported_view_floor_index, -1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3965 is unreachable from F0125. */
    expect_int("d0l2.native_unchanged", out.native_bitmap_index, -1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3965 ordinal-- is unreachable from F0125. */
    expect_int("d0l2.index_unchanged", out.floor_ornament_index, -1);
    /* ReDMCSB: DEFS.H line 2088 is still the C10 transparency contract used by F0115. */
    expect_int("d0l2.transparent", out.transparent_color, 10);
    /* ReDMCSB: DUNVIEW.C F0125 corridor path does not return as a wall. */
    expect_int("d0l2.no_wall_return", out.wall_case_returns ? 1 : 0, 0);
}

static void test_d0r2_corridor_routes_to_f0115_without_f0108(void)
{
    DM1_V1_FloorOrnamentD0L2D0R2InputPc34 in = {
        DM1_V1_D0L2_D0R2_ROUTE_D0R2_PC34,
        DM1_V1_D0L2_D0R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        5,
        0x23,
        0x31
    };
    DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0126 line 8115 calls F0115 for D0R. */
    expect_int("d0r2.resolve",
               M11_GameView_FloorOrnamentD0L2D0R2ResolvePc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: DEFS.H lines 2587-2589 define M611_VIEW_SQUARE_D0R as 11. */
    expect_int("d0r2.view_square", out.view_square_index, 11);
    /* ReDMCSB: DEFS.H lines 4036-4038 define C715_ZONE_WALL_D0R. */
    expect_int("d0r2.wall_zone", out.wall_zone_index, 715);
    /* ReDMCSB: DUNVIEW.C F0126 line 8115 passes C0x0001_CELL_ORDER_BACKLEFT. */
    expect_int("d0r2.cell_order", out.f0115_cell_order, 0x0001);
    /* ReDMCSB: DEFS.H line 2645 defines C03_VIEW_CELL_BACK_LEFT. */
    expect_int("d0r2.boundary_cell", out.documented_boundary_cell, 3);
    /* ReDMCSB: DUNVIEW.C F0126 lines 8103-8115 do not read M558. */
    expect_int("d0r2.no_floor_flag_read", out.reads_floor_ornament_flag ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0126 line 8115 calls F0115, not F0108. */
    expect_int("d0r2.no_f0108", out.calls_f0108 ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0126 line 8115 dispatches objects/creatures/projectiles/explosions. */
    expect_int("d0r2.calls_f0115", out.calls_f0115 ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3989 is not reached by D0R. */
    expect_int("d0r2.draws_floor", out.draws_floor_ornament ? 1 : 0, 0);
}

static void test_wall_and_stair_returns_block_f0115_and_f0108(void)
{
    DM1_V1_FloorOrnamentD0L2D0R2InputPc34 wall = {
        DM1_V1_D0L2_D0R2_ROUTE_D0L2_PC34,
        DM1_V1_D0L2_D0R2_SQUARE_WALL_PC34,
        4,
        0x23,
        0x44
    };
    DM1_V1_FloorOrnamentD0L2D0R2InputPc34 stair = {
        DM1_V1_D0L2_D0R2_ROUTE_D0R2_PC34,
        DM1_V1_D0L2_D0R2_SQUARE_STAIRS_SIDE_PC34,
        4,
        0x23,
        0x44
    };
    DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0125 lines 8007-8038 wall path returns. */
    expect_int("wall.resolve",
               M11_GameView_FloorOrnamentD0L2D0R2ResolvePc34(&wall, &out) ? 1 : 0, 1);
    expect_int("wall.returns", out.wall_case_returns ? 1 : 0, 1);
    expect_int("wall.no_f0115", out.calls_f0115 ? 1 : 0, 0);
    expect_int("wall.no_f0108", out.calls_f0108 ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0126 lines 8082-8092 stairs-side path returns. */
    expect_int("stair.resolve",
               M11_GameView_FloorOrnamentD0L2D0R2ResolvePc34(&stair, &out) ? 1 : 0, 1);
    expect_int("stair.returns", out.stairs_case_returns ? 1 : 0, 1);
    expect_int("stair.no_f0115", out.calls_f0115 ? 1 : 0, 0);
    expect_int("stair.no_f0108", out.calls_f0108 ? 1 : 0, 0);
}

static void test_pit_fallthrough_and_pixel_slice(void)
{
    uint8_t viewport[DM1_V1_PC34_D0L2_D0R2_VIEWPORT_WIDTH *
                     DM1_V1_PC34_D0L2_D0R2_VIEWPORT_HEIGHT];
    DM1_V1_FloorOrnamentD0L2D0R2InputPc34 in = {
        DM1_V1_D0L2_D0R2_ROUTE_D0L2_PC34,
        DM1_V1_D0L2_D0R2_SQUARE_PIT_WITH_FLOOR_ORNAMENT_PC34,
        9,
        0x77,
        0x35
    };
    DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 out;

    memset(viewport, 0x55, sizeof(viewport));

    /* ReDMCSB: DUNVIEW.C F0125 lines 7989-8005 let PIT fall through to F0115. */
    expect_int("pit.pixel_slice",
               M11_GameView_FloorOrnamentD0L2D0R2ApplyPixelSlicePc34(
                   &in, viewport, sizeof(viewport), 66, 3, &out) ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0125 lines 7989-8005 pit path has no break before F0115. */
    expect_int("pit.fallthrough", out.pit_falls_through_to_f0115 ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 is not called, so the synthetic floor pixel stays unchanged. */
    expect_int("pit.floor_pixel_unchanged", out.pixel_after_floor_slice, 0x55);
    /* ReDMCSB: DUNVIEW.C F0115 line 5181 writes non-C10 object pixels. */
    expect_int("pit.object_pixel_written", out.pixel_after_object_slice, 0x35);
    /* ReDMCSB: DUNVIEW.C F0108 line 3959 is unreachable despite a nonzero ordinal. */
    expect_int("pit.no_floor_draw", out.draws_floor_ornament ? 1 : 0, 0);
}

static void test_c10_transparency_and_invalid_inputs(void)
{
    uint8_t viewport[DM1_V1_PC34_D0L2_D0R2_VIEWPORT_WIDTH *
                     DM1_V1_PC34_D0L2_D0R2_VIEWPORT_HEIGHT];
    DM1_V1_FloorOrnamentD0L2D0R2InputPc34 in = {
        DM1_V1_D0L2_D0R2_ROUTE_D0R2_PC34,
        DM1_V1_D0L2_D0R2_SQUARE_TELEPORTER_WITH_FLOOR_ORNAMENT_PC34,
        3,
        0x65,
        DM1_V1_PC34_D0L2_D0R2_TRANSPARENT_COLOR
    };
    DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 out;

    memset(viewport, 0x6C, sizeof(viewport));

    /* ReDMCSB: DUNVIEW.C F0115 line 5181 preserves destination on C10. */
    expect_int("transparent.pixel_slice",
               M11_GameView_FloorOrnamentD0L2D0R2ApplyPixelSlicePc34(
                   &in, viewport, sizeof(viewport), 135, 223, &out) ? 1 : 0, 1);
    expect_int("transparent.floor_pixel_unchanged", out.pixel_after_floor_slice, 0x6C);
    expect_int("transparent.object_pixel_preserved", out.pixel_after_object_slice, 0x6C);
    expect_int("transparent.blend",
               M11_GameView_FloorOrnamentD0L2D0R2BlendPixelPc34(&out, 0x44, 10), 0x44);
    expect_int("invalid.null_input",
               M11_GameView_FloorOrnamentD0L2D0R2ResolvePc34(NULL, &out) ? 1 : 0, 0);
    expect_int("invalid.null_output",
               M11_GameView_FloorOrnamentD0L2D0R2ResolvePc34(&in, NULL) ? 1 : 0, 0);
    expect_int("invalid.pixel_bounds",
               M11_GameView_FloorOrnamentD0L2D0R2ApplyPixelSlicePc34(
                   &in, viewport, sizeof(viewport), 136, 0, &out) ? 1 : 0, 0);
}

static void test_source_evidence_mentions_all_anchors(void)
{
    const char *e = M11_GameView_FloorOrnamentD0L2D0R2SourceLockPc34();

    /* ReDMCSB: DUNVIEW.C F0108 lines 3940-4008. */
    expect_int("evidence.f0108", strstr(e, "3940-4008") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0115 setup lines 4787-4831. */
    expect_int("evidence.f0115_setup", strstr(e, "4787-4831") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0115 C10 blit lines 5181-5184. */
    expect_int("evidence.f0115_transparency", strstr(e, "5181-5184") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C D0 boundary lines 5295-5296. */
    expect_int("evidence.boundary", strstr(e, "5295-5296") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0125 lines 7976-8005 and 8007-8038. */
    expect_int("evidence.d0l", strstr(e, "7976-8005") != NULL &&
               strstr(e, "8007-8038") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0126 lines 8080-8115 and 8117-8138. */
    expect_int("evidence.d0r", strstr(e, "8080-8115") != NULL &&
               strstr(e, "8117-8138") != NULL, 1);
    /* ReDMCSB: DEFS.H line 2088. */
    expect_int("evidence.c10", strstr(e, "2088") != NULL, 1);
    /* ReDMCSB: DEFS.H lines 2587-2589. */
    expect_int("evidence.view_squares", strstr(e, "2587-2589") != NULL, 1);
    /* ReDMCSB: DEFS.H lines 2739-2747 prove no D0 floor view in PC34. */
    expect_int("evidence.no_d0_floor_views", strstr(e, "2739-2747") != NULL, 1);
}

int main(void)
{
    test_d0l2_corridor_routes_to_f0115_without_f0108();
    test_d0r2_corridor_routes_to_f0115_without_f0108();
    test_wall_and_stair_returns_block_f0115_and_f0108();
    test_pit_fallthrough_and_pixel_slice();
    test_c10_transparency_and_invalid_inputs();
    test_source_evidence_mentions_all_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_floor_ornament_d0l2_d0r2_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_floor_ornament_d0l2_d0r2_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
