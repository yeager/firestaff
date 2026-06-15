#include "dm1_v1_viewport_d2c_f0115_front_rear_overlap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static const char *A_F0121_REAR =
    "ReDMCSB DUNVIEW.C:7313-7315 F0121_DUNGEONVIEW_DrawSquareD2C door rear";
static const char *A_F0121_FRONT =
    "ReDMCSB DUNVIEW.C:7341-7342 F0121_DUNGEONVIEW_DrawSquareD2C door front";
static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C:4547-4582/4795-4800 F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF";
static const char *A_DEFS_C0218 =
    "ReDMCSB DEFS.H:2669 C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT";
static const char *A_DEFS_C0349 =
    "ReDMCSB DEFS.H:2672 C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT";
static const char *A_DEFS_C10 =
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH";
static const char *A_DEFS_M603 =
    "ReDMCSB DEFS.H:2602 M603_VIEW_SQUARE_D2C=6";
static const char *A_DEFS_C709 =
    "ReDMCSB DEFS.H:4049 C709_ZONE_WALL_D2C";
static const char *A_DEFS_DOOR =
    "ReDMCSB DEFS.H:4256 M628_ZONE_DOOR_D2C=3760";
static const char *A_F0115_FLUX =
    "ReDMCSB DUNVIEW.C:6006-6015/6199-6219 fluxcage pass-1 only";

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void test_spec_count_and_accessors(void)
{
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *rear =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(1);
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *front =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(2);
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *at0 =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_at_pc34(0);
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *at1 =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_at_pc34(1);

    dm1_v1_viewport_d2c_f0115_front_rear_overlap_init_pc34();

    expect_int("count",
               (int)dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_count_pc34(),
               2, "two F0115 door-front passes: rear and front");
    expect_int("spec_for_pass.rear", rear != NULL, 1, A_F0121_REAR);
    expect_int("spec_for_pass.front", front != NULL, 1, A_F0121_FRONT);
    expect_int("spec_for_pass.unknown",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(0) == NULL,
               1, "non-door pass is out of scope");
    expect_int("spec_for_pass.invalid",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(3) == NULL,
               1, "only pass 1 and pass 2 are valid");
    expect_int("at0", at0 != NULL, 1, "fixture at index 0");
    expect_int("at1", at1 != NULL, 1, "fixture at index 1");
    expect_int("at0.equals.rear", at0 == rear, 1,
               "fixture at index 0 is the rear pass");
    expect_int("at1.equals.front", at1 == front, 1,
               "fixture at index 1 is the front pass");
    expect_int("past.end.null",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_at_pc34(2) == NULL,
               1, "contract-only accessor bounds");
}

static void test_pass_role_and_cell_order(void)
{
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *rear =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(1);
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *front =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(2);

    expect_int("rear.pass", rear ? rear->pass : -1, 1, A_F0121_REAR);
    expect_int("front.pass", front ? front->pass : -1, 2, A_F0121_FRONT);
    expect_int("rear.order", rear ? (int)rear->cell_order : -1, 0x0218,
               A_DEFS_C0218);
    expect_int("front.order", front ? (int)front->cell_order : -1, 0x0349,
               A_DEFS_C0349);
    expect_int("rear.door_marker",
               rear ? (int)rear->door_marker_nibble : -1, 0x8, A_F0115);
    expect_int("front.door_marker",
               front ? (int)front->door_marker_nibble : -1, 0x9, A_F0115);
    expect_int("rear.cells_nibble",
               rear ? (int)rear->cells_nibble : -1, 0x021, A_F0115);
    expect_int("front.cells_nibble",
               front ? (int)front->cells_nibble : -1, 0x034, A_F0115);
    expect_int("rear.door_marker_set",
               rear ? rear->door_marker_set : 0, 1, A_F0115);
    expect_int("front.door_marker_set",
               front ? front->door_marker_set : 0, 1, A_F0115);
    expect_int("rear.door_pass_bit",
               rear ? rear->door_pass_bit : -1, 0, A_F0115);
    expect_int("front.door_pass_bit",
               front ? front->door_pass_bit : -1, 1, A_F0115);
    expect_int("rear.cell_count",
               rear ? rear->cell_count : -1, 2, A_DEFS_C0218);
    expect_int("front.cell_count",
               front ? front->cell_count : -1, 2, A_DEFS_C0349);
    expect_int("rear.cell0",
               rear ? rear->cell_ordinals[0] : -1, 1,
               "ReDMCSB DEFS.H:2669 BACKLEFT=1 (low nibble of 0x021)");
    expect_int("rear.cell1",
               rear ? rear->cell_ordinals[1] : -1, 2,
               "ReDMCSB DEFS.H:2669 BACKRIGHT=2 (mid nibble of 0x021)");
    expect_int("front.cell0",
               front ? front->cell_ordinals[0] : -1, 4,
               "ReDMCSB DEFS.H:2672 FRONTRIGHT=4 (low nibble of 0x034)");
    expect_int("front.cell1",
               front ? front->cell_ordinals[1] : -1, 3,
               "ReDMCSB DEFS.H:2672 FRONTLEFT=3 (mid nibble of 0x034)");
    expect_int("rear.role_match",
               rear && rear->pass_role &&
               strcmp(rear->pass_role, "rear_pass1") == 0, 1, A_F0121_REAR);
    expect_int("front.role_match",
               front && front->pass_role &&
               strcmp(front->pass_role, "front_pass2") == 0, 1, A_F0121_FRONT);
}

static void test_view_square_lane_metadata(void)
{
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *rear =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(1);
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *front =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(2);

    expect_int("rear.view_square",
               rear ? rear->view_square_index : -1, 6, A_DEFS_M603);
    expect_int("front.view_square",
               front ? front->view_square_index : -1, 6, A_DEFS_M603);
    expect_int("rear.view_depth",
               rear ? rear->view_depth : -1, 2,
               "ReDMCSB DUNVIEW.C:370-377 G2027[6]=2");
    expect_int("front.view_depth",
               front ? front->view_depth : -1, 2,
               "ReDMCSB DUNVIEW.C:370-377 G2027[6]=2");
    expect_int("rear.view_lane",
               rear ? rear->view_lane : -9, 0,
               "ReDMCSB DUNVIEW.C:370-377 G2026[6]=0");
    expect_int("front.view_lane",
               front ? front->view_lane : -9, 0,
               "ReDMCSB DUNVIEW.C:370-377 G2026[6]=0");
    expect_int("rear.wall_zone", rear ? rear->wall_zone : -1, 709, A_DEFS_C709);
    expect_int("front.wall_zone", front ? front->wall_zone : -1, 709, A_DEFS_C709);
    expect_int("rear.door_zone", rear ? rear->door_zone : -1, 3760, A_DEFS_DOOR);
    expect_int("front.door_zone", front ? front->door_zone : -1, 3760, A_DEFS_DOOR);
    expect_int("rear.transparent", rear ? rear->transparent_color : -1, 10,
               A_DEFS_C10);
    expect_int("front.transparent", front ? front->transparent_color : -1, 10,
               A_DEFS_C10);
    expect_int("rear.contract_only", rear ? rear->contract_only : 0, 1, A_F0115);
    expect_int("front.contract_only", front ? front->contract_only : 0, 1, A_F0115);
    expect_int("rear.no_real_asset_parity",
               rear ? rear->no_real_asset_bitmap_parity : 0, 1, A_F0115);
    expect_int("front.no_real_asset_parity",
               front ? front->no_real_asset_bitmap_parity : 0, 1, A_F0115);
    expect_int("rear.no_game_data_load",
               rear ? rear->no_game_data_load : 0, 1, A_F0115);
    expect_int("front.no_game_data_load",
               front ? front->no_game_data_load : 0, 1, A_F0115);
    expect_int("rear.anchor_set",
               rear ? rear->f0115_internal_dispatch_anchor_set : 0, 1, A_F0115);
    expect_int("front.anchor_set",
               front ? front->f0115_internal_dispatch_anchor_set : 0, 1, A_F0115);
}

static void test_fluxcage_per_pass_invariant(void)
{
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *rear =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(1);
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *front =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(2);

    expect_int("rear.fluxcage_recorded",
               rear ? rear->fluxcage_recorded_this_pass : -1, 1, A_F0115_FLUX);
    expect_int("front.fluxcage_recorded",
               front ? front->fluxcage_recorded_this_pass : 1, 0, A_F0115_FLUX);
}

static void test_zone_table_reuse_invariants(void)
{
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *rear =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(1);
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *front =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(2);

    expect_int("rear.creature_zone_table_reused",
               rear ? rear->creature_zone_table_reused : 0, 1,
               "ReDMCSB DUNVIEW.C:5295/5615-5617 creature zone reuse");
    expect_int("front.creature_zone_table_reused",
               front ? front->creature_zone_table_reused : 0, 1,
               "ReDMCSB DUNVIEW.C:5295/5615-5617 creature zone reuse");
    expect_int("rear.projectile_zone_table_reused",
               rear ? rear->projectile_zone_table_reused : 0, 1,
               "ReDMCSB DUNVIEW.C:5668-5671 projectile zone reuse");
    expect_int("front.projectile_zone_table_reused",
               front ? front->projectile_zone_table_reused : 0, 1,
               "ReDMCSB DUNVIEW.C:5668-5671 projectile zone reuse");
    expect_int("rear.item_zone_table_reused",
               rear ? rear->item_zone_table_reused : 0, 1,
               "ReDMCSB DUNVIEW.C:4923 item zone reuse");
    expect_int("front.item_zone_table_reused",
               front ? front->item_zone_table_reused : 0, 1,
               "ReDMCSB DUNVIEW.C:4923 item zone reuse");
    expect_int("rear.explosion_zone_table_reused",
               rear ? rear->explosion_zone_table_reused : 0, 1,
               "ReDMCSB DUNVIEW.C:6107/6122 explosion zone reuse");
    expect_int("front.explosion_zone_table_reused",
               front ? front->explosion_zone_table_reused : 0, 1,
               "ReDMCSB DUNVIEW.C:6107/6122 explosion zone reuse");
    expect_int("rear.field_aspect_lookup_reused",
               rear ? rear->field_aspect_lookup_reused : 0, 1,
               "ReDMCSB DUNVIEW.C:6219 G2035[viewSquare]");
    expect_int("front.field_aspect_lookup_reused",
               front ? front->field_aspect_lookup_reused : 0, 1,
               "ReDMCSB DUNVIEW.C:6219 G2035[viewSquare]");
    expect_int("rear.first_thing_pointer_passed_through",
               rear ? rear->first_thing_pointer_passed_through : 0, 1,
               A_F0115);
    expect_int("front.first_thing_pointer_passed_through",
               front ? front->first_thing_pointer_passed_through : 0, 1,
               A_F0115);
    expect_int("rear.same_thing_list_walked_twice",
               rear ? rear->same_thing_list_walked_twice : 0, 1,
               "ReDMCSB DUNVIEW.C:7315/7341-7342 F0121 issues two F0115 calls");
    expect_int("front.same_thing_list_walked_twice",
               front ? front->same_thing_list_walked_twice : 0, 1,
               "ReDMCSB DUNVIEW.C:7315/7341-7342 F0121 issues two F0115 calls");
}

static void test_door_marker_decode_pc34(void)
{
    int marker_set = -1;
    int door_pass_bit = -1;
    unsigned int remaining = 0xDEADBEEFU;

    expect_int("decode.rear",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_marker_pc34(
                   0x0218U, &marker_set, &door_pass_bit, &remaining),
               0, A_F0115);
    expect_int("decode.rear.marker", marker_set, 1, A_F0115);
    expect_int("decode.rear.pass_bit", door_pass_bit, 0, A_F0115);
    expect_int("decode.rear.remaining", (int)remaining, 0x021, A_F0115);

    expect_int("decode.front",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_marker_pc34(
                   0x0349U, &marker_set, &door_pass_bit, &remaining),
               0, A_F0115);
    expect_int("decode.front.marker", marker_set, 1, A_F0115);
    expect_int("decode.front.pass_bit", door_pass_bit, 1, A_F0115);
    expect_int("decode.front.remaining", (int)remaining, 0x034, A_F0115);

    /* Open-square cell order has no door marker (low nibble 0x0). */
    expect_int("decode.open",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_marker_pc34(
                   0x3421U, &marker_set, &door_pass_bit, &remaining),
               0, A_F0115);
    expect_int("decode.open.marker", marker_set, 0, A_F0115);
    expect_int("decode.open.pass_bit", door_pass_bit, 1,
               "low bit of 0x1 is 1 but marker is not set");
    expect_int("decode.open.remaining", (int)remaining, 0x342, A_F0115);

    expect_int("decode.alcove",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_marker_pc34(
                   0x0000U, &marker_set, &door_pass_bit, &remaining),
               0, A_F0115);
    expect_int("decode.alcove.marker", marker_set, 0, A_F0115);
    expect_int("decode.alcove.remaining", (int)remaining, 0x0, A_F0115);

    expect_int("decode.null",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_marker_pc34(
                   0x0218U, NULL, &door_pass_bit, &remaining),
               -1, "invalid input guard");
    expect_int("decode.null2",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_marker_pc34(
                   0x0218U, &marker_set, NULL, &remaining),
               -1, "invalid input guard");
    expect_int("decode.null3",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_marker_pc34(
                   0x0218U, &marker_set, &door_pass_bit, NULL),
               -1, "invalid input guard");
}

static void test_cell_order_decode_pc34(void)
{
    int cells[4] = {-1, -1, -1, -1};

    /* ReDMCSB DUNVIEW.C:4796 `P0146_ui_OrderedViewCellOrdinals >>= 4`
     * strips the door marker; the post-strip value 0x021 is the input
     * to the F0115 cell nibble loop. The function walks nibbles from
     * low to high, so for 0x021 the cells are [BACKLEFT=1, BACKRIGHT=2]. */
    expect_int("cells.rear",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_cells_pc34(
                   0x0218U >> 4, cells, 4),
               2, A_F0115);
    expect_int("cells.rear.0", cells[0], 1, A_DEFS_C0218);
    expect_int("cells.rear.1", cells[1], 2, A_DEFS_C0218);

    cells[0] = cells[1] = cells[2] = cells[3] = -1;
    /* For 0x0349 the post-strip value 0x034 has nibbles [4, 3, 0, 0]
     * from low to high, so the F0115 walks [FRONTRIGHT=4, FRONTLEFT=3]. */
    expect_int("cells.front",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_cells_pc34(
                   0x0349U >> 4, cells, 4),
               2, A_F0115);
    expect_int("cells.front.0", cells[0], 4, A_DEFS_C0349);
    expect_int("cells.front.1", cells[1], 3, A_DEFS_C0349);

    cells[0] = cells[1] = cells[2] = cells[3] = -1;
    /* Open-square 0x3421 has no door marker; nibbles are [1, 2, 3, 4]
     * from low to high. The four cells are walked in F0115 nibble order. */
    expect_int("cells.open",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_cells_pc34(
                   0x3421U, cells, 4),
               4, A_F0115);
    expect_int("cells.open.0", cells[0], 1, "ReDMCSB DEFS.H:2676 BACKLEFT");
    expect_int("cells.open.1", cells[1], 2, "ReDMCSB DEFS.H:2676 BACKRIGHT");
    expect_int("cells.open.2", cells[2], 4,
               "ReDMCSB DUNVIEW.C:4561-4564 F0115 nibble order low-to-high");
    expect_int("cells.open.3", cells[3], 3,
               "ReDMCSB DUNVIEW.C:4561-4564 F0115 nibble order low-to-high");

    expect_int("cells.null",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_cells_pc34(
                   0x0218U >> 4, NULL, 4),
               -1, "invalid input guard");
    expect_int("cells.too_small",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_cells_pc34(
                   0x0218U >> 4, cells, 1),
               -1, "output buffer too small guard");
    expect_int("cells.terminator_only",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_cells_pc34(
                   0x0U, cells, 4),
               -1, "no cells to draw");
    expect_int("cells.bad_cell",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_cells_pc34(
                   0x0DU, cells, 4),
               -1, "0xD is not a valid cell ordinal");
}

static void test_door_pass_trace_pc34(void)
{
    DM1_V1_D2CF0115FrontRearOverlapTracePc34 rear = {0};
    DM1_V1_D2CF0115FrontRearOverlapTracePc34 front = {0};

    expect_int("trace.rear",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
                   0x0218U, &rear),
               0, A_F0121_REAR);
    expect_int("trace.rear.ok", rear.ok, 1, A_F0121_REAR);
    expect_int("trace.rear.decoded_pass", rear.decoded_pass, 1, A_F0115);
    expect_int("trace.rear.marker_set", rear.door_marker_set, 1, A_F0115);
    expect_int("trace.rear.pass_bit", rear.door_pass_bit, 0, A_F0115);
    expect_int("trace.rear.remaining", (int)rear.remaining_ordinals, 0x021,
               A_F0115);
    expect_int("trace.rear.cell_count", rear.cell_count, 2, A_F0115);
    expect_int("trace.rear.cell0", rear.cell_ordinals[0], 1, A_DEFS_C0218);
    expect_int("trace.rear.cell1", rear.cell_ordinals[1], 2, A_DEFS_C0218);
    expect_int("trace.rear.fluxcage", rear.fluxcage_recorded_this_pass, 1,
               A_F0115_FLUX);

    expect_int("trace.front",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
                   0x0349U, &front),
               0, A_F0121_FRONT);
    expect_int("trace.front.ok", front.ok, 1, A_F0121_FRONT);
    expect_int("trace.front.decoded_pass", front.decoded_pass, 2, A_F0115);
    expect_int("trace.front.marker_set", front.door_marker_set, 1, A_F0115);
    expect_int("trace.front.pass_bit", front.door_pass_bit, 1, A_F0115);
    expect_int("trace.front.remaining", (int)front.remaining_ordinals, 0x034,
               A_F0115);
    expect_int("trace.front.cell_count", front.cell_count, 2, A_F0115);
    expect_int("trace.front.cell0", front.cell_ordinals[0], 4, A_DEFS_C0349);
    expect_int("trace.front.cell1", front.cell_ordinals[1], 3, A_DEFS_C0349);
    expect_int("trace.front.fluxcage", front.fluxcage_recorded_this_pass, 0,
               A_F0115_FLUX);

    expect_int("trace.open",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
                   0x3421U, &rear),
               0, A_F0115);
    expect_int("trace.open.decoded_pass", rear.decoded_pass, 0,
               "non-door cell order has no door pass");
    expect_int("trace.open.fluxcage", rear.fluxcage_recorded_this_pass, 0,
               "non-door cell order has no fluxcage pass");
    expect_int("trace.bad",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
                   0x0218U, NULL),
               -1, "invalid input guard");
    expect_int("trace.bad.cell",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
                   0x0D00U, &rear),
               -1, "bad cell ordinal guard");
}

static void test_disjoint_cells_pc34(void)
{
    expect_int("disjoint.rear.front",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_cells_disjoint_pc34(
                   0x0218U, 0x0349U),
               1, A_F0115);
    expect_int("disjoint.front.rear",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_cells_disjoint_pc34(
                   0x0349U, 0x0218U),
               1, A_F0115);
    /* Same order is not disjoint. */
    expect_int("disjoint.same",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_cells_disjoint_pc34(
                   0x0218U, 0x0218U),
               0, "F0115 must never be called with the same cell order twice");
    expect_int("disjoint.bad.rear",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_cells_disjoint_pc34(
                   0x0D00U, 0x0349U),
               -1, "bad rear cell order guard");
    expect_int("disjoint.bad.front",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_cells_disjoint_pc34(
                   0x0218U, 0x0D00U),
               -1, "bad front cell order guard");
}

static void test_compose_pixel_pc34(void)
{
    DM1_V1_D2CF0115FrontRearOverlapTracePc34 rear = {0};
    DM1_V1_D2CF0115FrontRearOverlapTracePc34 front = {0};
    uint8_t after_rear = 0;
    uint8_t after_door = 0;
    uint8_t after_front = 0;

    expect_int("compose.basic",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_compose_pixel_pc34(
                   0x11, 0x31, 0x41, 0x51, 10,
                   &rear, &front, &after_rear, &after_door, &after_front),
               0, A_F0115);
    expect_int("compose.rear.pass", rear.decoded_pass, 1, A_F0115);
    expect_int("compose.front.pass", front.decoded_pass, 2, A_F0115);
    expect_int("compose.after_rear", after_rear, 0x31, A_F0115);
    expect_int("compose.after_door", after_door, 0x41, A_F0115);
    expect_int("compose.after_front", after_front, 0x51, A_F0115);

    /* C10 transparency is preserved on all three layers. */
    expect_int("compose.c10.floor",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_compose_pixel_pc34(
                   0x66, 10, 10, 10, 10,
                   &rear, &front, &after_rear, &after_door, &after_front),
               0, A_DEFS_C10);
    expect_int("compose.c10.after_rear", after_rear, 0x66, A_DEFS_C10);
    expect_int("compose.c10.after_door", after_door, 0x66, A_DEFS_C10);
    expect_int("compose.c10.after_front", after_front, 0x66, A_DEFS_C10);

    /* Door body opaque overrides prior rear F0115. */
    expect_int("compose.door.opaque",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_compose_pixel_pc34(
                   0x11, 0x31, 0x99, 0x51, 10,
                   &rear, &front, &after_rear, &after_door, &after_front),
               0, A_F0115);
    expect_int("compose.door.opaque.after_door", after_door, 0x99, A_F0115);
    expect_int("compose.door.opaque.after_front", after_front, 0x51, A_F0115);

    /* Front F0115 transparent keeps door body. */
    expect_int("compose.front.transparent",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_compose_pixel_pc34(
                   0x11, 0x31, 0x41, 10, 10,
                   &rear, &front, &after_rear, &after_door, &after_front),
               0, A_DEFS_C10);
    expect_int("compose.front.transparent.after_front", after_front, 0x41,
               A_DEFS_C10);

    expect_int("compose.null",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_compose_pixel_pc34(
                   0, 0, 0, 0, 10, NULL, &front,
                   &after_rear, &after_door, &after_front),
               -1, "invalid input guard");
    expect_int("compose.null.out_after_rear",
               dm1_v1_viewport_d2c_f0115_front_rear_overlap_compose_pixel_pc34(
                   0, 0, 0, 0, 10, &rear, &front,
                   NULL, &after_door, &after_front),
               -1, "invalid input guard");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_source_evidence_pc34();
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *rear =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(1);
    const DM1_V1_D2CF0115FrontRearOverlapPc34 *front =
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(2);

    expect_contains("evidence.contract_only", e, "contract_only=1",
                    "source evidence");
    expect_contains("evidence.no_real_asset_parity", e,
                    "no_real_asset_bitmap_parity=1", "source evidence");
    expect_contains("evidence.f0115", e, "F0115_DUNGEONVIEW_DrawObjects",
                    "source evidence");
    expect_contains("evidence.f0121", e, "F0121_DUNGEONVIEW_DrawSquareD2C",
                    "source evidence");
    expect_contains("evidence.door_marker", e, "MASK0x0008_DOOR_FRONT",
                    "DUNVIEW.C:4795-4799 door marker anchor");
    expect_contains("evidence.pass_calc", e, "+ 1",
                    "DUNVIEW.C:4799 door pass calculation");
    expect_contains("evidence.c0218", e, "C0x0218",
                    "DEFS.H:2669 door pass 1 cell order");
    expect_contains("evidence.c0349", e, "C0x0349",
                    "DEFS.H:2672 door pass 2 cell order");
    expect_contains("evidence.c10", e, "C10_COLOR_FLESH",
                    "DEFS.H:2088 C10 transparency");
    expect_contains("evidence.m603", e, "M603_VIEW_SQUARE_D2C=6",
                    "DEFS.H:2602 view square index");
    expect_contains("evidence.c709", e, "C709_ZONE_WALL_D2C",
                    "DEFS.H:4049 D2C wall zone");
    expect_contains("evidence.door_zone", e, "M628_ZONE_DOOR_D2C=3760",
                    "DEFS.H:4256 D2C door blit zone");
    expect_contains("evidence.fluxcage", e, "fluxcage",
                    "DUNVIEW.C:6006-6015/6199-6219 fluxcage anchor");
    expect_contains("evidence.cell_table", e, "C3200",
                    "DUNVIEW.C:5615-5617 creature zone table");
    expect_contains("evidence.f0128", e, "F0128",
                    "DUNVIEW.C:8520-8521 F0128 dispatch");
    expect_contains("evidence.l0175", e, "L0175_i_DoorFrontViewDrawingPass",
                    "DUNVIEW.C:4799 door pass local variable");
    expect_contains("evidence.first_thing", e, "M550_FIRST_THING",
                    "DUNVIEW.C:7315/7341 first thing argument");
    expect_contains("evidence.contract_marker", e, "DUNVIEW.C:7313-7342",
                    "DUNVIEW.C door-front route");
    expect_contains("rear.redmcsb_f0121",
                    rear ? rear->redmcsb_f0121_anchor : NULL,
                    "F0121_DUNGEONVIEW_DrawSquareD2C", "fixture F0121 anchor");
    expect_contains("front.redmcsb_f0121",
                    front ? front->redmcsb_f0121_anchor : NULL,
                    "F0121_DUNGEONVIEW_DrawSquareD2C", "fixture F0121 anchor");
    expect_contains("rear.redmcsb_f0115",
                    rear ? rear->redmcsb_f0115_anchor : NULL,
                    "F0115_DUNGEONVIEW", "fixture F0115 anchor");
    expect_contains("front.redmcsb_defs",
                    front ? front->redmcsb_defs_anchor : NULL,
                    "DEFS.H:2672", "fixture DEFS.H anchor");
    expect_contains("rear.source_lines",
                    rear ? rear->source_lines : NULL,
                    "DUNVIEW.C:7313-7342", "fixture source anchor");
    expect_contains("front.source_lines",
                    front ? front->source_lines : NULL,
                    "DUNVIEW.C:7313-7342", "fixture source anchor");
}

int main(void)
{
    test_spec_count_and_accessors();
    test_pass_role_and_cell_order();
    test_view_square_lane_metadata();
    test_fluxcage_per_pass_invariant();
    test_zone_table_reuse_invariants();
    test_door_marker_decode_pc34();
    test_cell_order_decode_pc34();
    test_door_pass_trace_pc34();
    test_disjoint_cells_pc34();
    test_compose_pixel_pc34();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d2c_f0115_front_rear_overlap_pc34_compat "
               "failures=%d assertions=%d\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2c_f0115_front_rear_overlap_pc34_compat "
           "%d/%d assertions\n", g_assertions, g_assertions);
    return 0;
}
