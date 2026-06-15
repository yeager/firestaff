#include "dm1_v1_viewport_floor_ornament_d3l2_d3r2_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

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

static void test_d3l2_corridor_floor_ornament_pixel_slice(void)
{
    DM1_V1_FloorOrnamentD3L2D3R2InputPc34 in = {
        DM1_V1_D3L2_D3R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        DM1_V1_D3L2_D3R2_FLOOR_VIEW_D3L2_PC34,
        5,
        2,
        240
    };
    DM1_V1_FloorOrnamentD3L2D3R2ResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0676 lines 8478-8482 dispatch D3L2 at depth 3 lane -2. */
    expect_int("d3l2.resolve",
               dm1_v1_viewport_floor_ornament_d3l2_d3r2_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: Firestaff's MEDIA720-only D3L2 metadata uses DM1_VIEW_SQUARE_D3L2 = -101. */
    expect_int("d3l2.view_square_macro", (int)DM1_VIEW_SQUARE_D3L2, -101);
    /* ReDMCSB: DUNVIEW.C F0676 line 8481 supplies the depth-3/-2 relative movement. */
    expect_int("d3l2.view_square", out.view_square_index, -101);
    expect_int("d3l2.depth", out.depth, 3);
    expect_int("d3l2.lane", out.lane, -2);
    /* ReDMCSB: DEFS.H lines 4042-4043 define C702/C703; D3L2 uses C702. */
    expect_int("d3l2.wall_zone", out.wall_zone_index, 702);
    /* ReDMCSB: DEFS.H lines 4056-4057 define D0L/D0R zones, not this D3L2 route. */
    expect_int("d3l2.not_d0_zone", out.d0_wall_zone_reused ? 1 : 0, 0);
    /* ReDMCSB: DUNVIEW.C F0676 line 6284 calls F0108 with M558. */
    expect_int("d3l2.reads_floor_ornament_flag", out.reads_floor_ornament_flag ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 lines 3959-3966 perform the ordinal gate. */
    expect_int("d3l2.calls_f0108", out.calls_f0108 ? 1 : 0, 1);
    expect_int("d3l2.draws", out.draws_floor_ornament ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3965 decrements the ordinal to a zero-based index. */
    expect_int("d3l2.index", out.floor_ornament_index, 4);
    /* ReDMCSB: DUNVIEW.C line 820 table gives C00_VIEW_FLOOR_D3L2 native increment 0. */
    expect_int("d3l2.native_bitmap", out.native_bitmap_index, 240);
    /* ReDMCSB: DEFS.H lines 2750-2751 define C00_VIEW_FLOOR_D3L2 as 0. */
    expect_int("d3l2.view_floor", out.view_floor_index, 0);
    /* ReDMCSB: DUNVIEW.C line 3998 uses C1500 + coordinateSet * 11 + viewFloor. */
    expect_int("d3l2.floor_zone", out.floor_ornament_zone_index, 1500 + 2 * 11 + 0);
    /* ReDMCSB: DUNVIEW.C line 3980 flips D3R2 only, not D3L2. */
    expect_int("d3l2.flip", out.flip_horizontal ? 1 : 0, 0);
    /* ReDMCSB: DEFS.H line 2088 defines C10_COLOR_FLESH transparency. */
    expect_int("d3l2.transparent", out.transparent_color, 10);
    /* ReDMCSB: this F0108 pixel-slice contract explicitly excludes F0111/F0115. */
    expect_int("d3l2.no_f0111", out.calls_f0111_in_pixel_slice ? 1 : 0, 0);
    expect_int("d3l2.no_f0115", out.calls_f0115_in_pixel_slice ? 1 : 0, 0);
    expect_int("d3l2.not_open_pit_bug64_path", out.open_pit_bug64_path ? 1 : 0, 0);
}

static void test_d3r2_corridor_floor_ornament_pixel_slice(void)
{
    DM1_V1_FloorOrnamentD3L2D3R2InputPc34 in = {
        DM1_V1_D3L2_D3R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        DM1_V1_D3L2_D3R2_FLOOR_VIEW_D3R2_PC34,
        2,
        1,
        300
    };
    DM1_V1_FloorOrnamentD3L2D3R2ResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0677 lines 8483-8486 dispatch D3R2 at depth 3 lane +2. */
    expect_int("d3r2.resolve",
               dm1_v1_viewport_floor_ornament_d3l2_d3r2_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    /* ReDMCSB: Firestaff's MEDIA720-only D3R2 metadata uses DM1_VIEW_SQUARE_D3R2 = -102. */
    expect_int("d3r2.view_square_macro", (int)DM1_VIEW_SQUARE_D3R2, -102);
    expect_int("d3r2.view_square", out.view_square_index, -102);
    expect_int("d3r2.depth", out.depth, 3);
    expect_int("d3r2.lane", out.lane, 2);
    /* ReDMCSB: DEFS.H lines 4042-4043 define C702/C703; D3R2 uses C703. */
    expect_int("d3r2.wall_zone", out.wall_zone_index, 703);
    /* ReDMCSB: DEFS.H lines 4056-4057 define D0L/D0R zones, not this D3R2 route. */
    expect_int("d3r2.not_d0_zone", out.d0_wall_zone_reused ? 1 : 0, 0);
    /* ReDMCSB: DEFS.H lines 2750-2751 define C01_VIEW_FLOOR_D3R2 as 1. */
    expect_int("d3r2.view_floor", out.view_floor_index, 1);
    /* ReDMCSB: DUNVIEW.C line 3998 uses C1500 + coordinateSet * 11 + viewFloor. */
    expect_int("d3r2.floor_zone", out.floor_ornament_zone_index, 1500 + 1 * 11 + 1);
    /* ReDMCSB: DUNVIEW.C line 3980 flips C01_VIEW_FLOOR_D3R2. */
    expect_int("d3r2.flip", out.flip_horizontal ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3965 keeps native increment 0 for C01. */
    expect_int("d3r2.native_bitmap", out.native_bitmap_index, 300);
    expect_int("d3r2.index", out.floor_ornament_index, 1);
    /* ReDMCSB: this F0108 pixel-slice contract explicitly excludes F0111/F0115. */
    expect_int("d3r2.no_f0111", out.calls_f0111_in_pixel_slice ? 1 : 0, 0);
    expect_int("d3r2.no_f0115", out.calls_f0115_in_pixel_slice ? 1 : 0, 0);
}

static void test_pit_bug64_and_transparent_pixel_contract(void)
{
    DM1_V1_FloorOrnamentD3L2D3R2InputPc34 in = {
        DM1_V1_D3L2_D3R2_SQUARE_PIT_WITH_FLOOR_ORNAMENT_BUG64_PC34,
        DM1_V1_D3L2_D3R2_FLOOR_VIEW_D3R2_PC34,
        1,
        0,
        42
    };
    DM1_V1_FloorOrnamentD3L2D3R2ResultPc34 out;
    const unsigned char side_pixel = 0x55;
    const unsigned char ornament_pixel = 0x23;

    /* ReDMCSB: DUNVIEW.C F0677 lines 6342-6351 let pit fall through to F0108 (BUG0_64). */
    expect_int("pit.resolve",
               dm1_v1_viewport_floor_ornament_d3l2_d3r2_resolve_f0108_pc34(&in, &out) ? 1 : 0, 1);
    expect_int("pit.open_pit_bug64_path", out.open_pit_bug64_path ? 1 : 0, 1);
    /* ReDMCSB: DUNVIEW.C F0108 line 3980 still flips C01_VIEW_FLOOR_D3R2. */
    expect_int("pit.flip", out.flip_horizontal ? 1 : 0, 1);
    /* ReDMCSB: DEFS.H line 2088 and DUNVIEW.C line 3998 use C10 as transparent color. */
    expect_int("pit.transparent_pixel",
               dm1_v1_viewport_floor_ornament_d3l2_d3r2_blit_pixel_pc34(
                   &out, side_pixel,
                   DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_TRANSPARENT_COLOR),
               side_pixel);
    /* ReDMCSB: DUNVIEW.C F0108 line 3998 writes non-C10 source pixels. */
    expect_int("pit.ornament_pixel",
               dm1_v1_viewport_floor_ornament_d3l2_d3r2_blit_pixel_pc34(
                   &out, side_pixel, ornament_pixel),
               ornament_pixel);
    /* ReDMCSB: the pixel-slice does not enter the later thing or door renderers. */
    expect_int("pit.no_f0111", out.calls_f0111_in_pixel_slice ? 1 : 0, 0);
    expect_int("pit.no_f0115", out.calls_f0115_in_pixel_slice ? 1 : 0, 0);
}

static void test_wall_and_zero_ordinal_contracts(void)
{
    DM1_V1_FloorOrnamentD3L2D3R2InputPc34 wall = {
        DM1_V1_D3L2_D3R2_SQUARE_WALL_WITH_SIDE_ORNAMENT_PC34,
        DM1_V1_D3L2_D3R2_FLOOR_VIEW_D3L2_PC34,
        7,
        0,
        400
    };
    DM1_V1_FloorOrnamentD3L2D3R2InputPc34 zero = {
        DM1_V1_D3L2_D3R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34,
        DM1_V1_D3L2_D3R2_FLOOR_VIEW_D3L2_PC34,
        0,
        0,
        250
    };
    DM1_V1_FloorOrnamentD3L2D3R2ResultPc34 out;

    /* ReDMCSB: DUNVIEW.C F0676 lines 6253-6264 wall path returns before F0108. */
    expect_int("wall.resolve",
               dm1_v1_viewport_floor_ornament_d3l2_d3r2_resolve_f0108_pc34(&wall, &out) ? 1 : 0, 1);
    expect_int("wall.no_floor_flag_read", out.reads_floor_ornament_flag ? 1 : 0, 0);
    expect_int("wall.no_f0108", out.calls_f0108 ? 1 : 0, 0);
    expect_int("wall.zone", out.wall_zone_index, 702);
    /* ReDMCSB: DEFS.H lines 4056-4057 D0 zones are explicitly not reused. */
    expect_int("wall.not_d0_zone", out.d0_wall_zone_reused ? 1 : 0, 0);

    /* ReDMCSB: DUNVIEW.C F0108 lines 3959-3966 calls the gate but draws nothing for ordinal 0. */
    expect_int("zero.resolve",
               dm1_v1_viewport_floor_ornament_d3l2_d3r2_resolve_f0108_pc34(&zero, &out) ? 1 : 0, 1);
    expect_int("zero.reads_floor_ornament_flag", out.reads_floor_ornament_flag ? 1 : 0, 1);
    expect_int("zero.calls_f0108", out.calls_f0108 ? 1 : 0, 1);
    expect_int("zero.draws", out.draws_floor_ornament ? 1 : 0, 0);
    expect_int("zero.index", out.floor_ornament_index, -1);
}

static void test_source_evidence_mentions_all_required_anchors(void)
{
    const char *e = dm1_v1_viewport_floor_ornament_d3l2_d3r2_source_lock_pc34();

    /* ReDMCSB: DUNVIEW.C F0676 lines 8478-8482. */
    expect_int("evidence.d3l2_dispatcher", strstr(e, "8478-8482") != NULL &&
               strstr(e, "F0676_DrawD3L2") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0677 lines 8483-8486. */
    expect_int("evidence.d3r2_dispatcher", strstr(e, "8483-8486") != NULL &&
               strstr(e, "F0677_DrawD3R2") != NULL, 1);
    /* ReDMCSB: DUNVIEW.C F0108 lines 3959-3966. */
    expect_int("evidence.f0108_gate", strstr(e, "3959-3966") != NULL, 1);
    /* ReDMCSB: DEFS.H lines 4042-4043. */
    expect_int("evidence.d3_zones", strstr(e, "4042-4043") != NULL &&
               strstr(e, "C702_ZONE_WALL_D3L2") != NULL &&
               strstr(e, "C703_ZONE_WALL_D3R2") != NULL, 1);
    /* ReDMCSB: DEFS.H lines 4056-4057 are cited only as the non-D3 zones. */
    expect_int("evidence.not_d0_zones", strstr(e, "4056-4057") != NULL &&
               strstr(e, "not used") != NULL, 1);
    /* ReDMCSB: DEFS.H line 2088 C10_COLOR_FLESH. */
    expect_int("evidence.c10", strstr(e, "2088") != NULL &&
               strstr(e, "C10_COLOR_FLESH") != NULL, 1);
    /* ReDMCSB: explicit non-F0111/non-F0115 floor-ornament pixel-slice contract. */
    expect_int("evidence.non_f0111_non_f0115", strstr(e, "non-F0111/non-F0115") != NULL, 1);
    /* ReDMCSB: source-locked gate warning for this synthetic contract. */
    expect_int("evidence.contract_only", strstr(e, "does not claim full real-asset") != NULL, 1);
}

int main(void)
{
    test_d3l2_corridor_floor_ornament_pixel_slice();
    test_d3r2_corridor_floor_ornament_pixel_slice();
    test_pit_bug64_and_transparent_pixel_contract();
    test_wall_and_zero_ordinal_contracts();
    test_source_evidence_mentions_all_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_floor_ornament_d3l2_d3r2_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_floor_ornament_d3l2_d3r2_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
