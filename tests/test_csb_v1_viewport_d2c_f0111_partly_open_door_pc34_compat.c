#include "csb/csb_v1_viewport_d2c_f0111_partly_open_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C F0111 lines 4218-4337 partly-open D2C horizontal path";
static const char *A_F0111_FRAME =
    "ReDMCSB DUNVIEW.C F0111 lines 4311-4313 D2C LeftHorizontal/RightHorizontal";
static const char *A_F0111_FIRST =
    "ReDMCSB DUNVIEW.C F0111 lines 4317-4324 P2084+state+C6_UNKNOWN first half";
static const char *A_F0111_SECOND =
    "ReDMCSB DUNVIEW.C F0111 lines 4325-4334 state+3|MASK0x4000 then C10 blit";
static const char *A_F0121 =
    "ReDMCSB DUNVIEW.C F0121 lines 7244-7389 D2C body";
static const char *A_F0121_DOOR =
    "ReDMCSB DUNVIEW.C F0121 lines 7313-7341 C17_ELEMENT_DOOR_FRONT branch";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C F0128 lines 8508-8533 D2C dispatch";
static const char *A_F0678_F0679 =
    "ReDMCSB DUNVIEW.C F0678 lines 6837-6865 and F0679 lines 6868-6896 wall anchors";
static const char *A_DEFS =
    "ReDMCSB DEFS.H lines 2088, 2602, 2605-2606, 3508, 3516, 4029-4031, "
    "4047-4049, 4228-4230, 4250-4256";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp lines 1903-1915";
static const char *A_NO_PIXEL =
    "NO-CLAIM real-asset pixel parity marker";

static int g_assertions = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_true(const char *label, int got, const char *anchor)
{
    return expect_int(label, got ? 1 : 0, 1, anchor);
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_true(label, haystack && needle && strstr(haystack, needle),
                       anchor);
}

static int test_spec_identity(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *d2c =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(6);

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d2c_f0111_partly_open_door_spec_count_pc34(),
                     1, A_F0128);
    ok &= expect_true("spec.d2c.present", d2c != 0,
                      "ReDMCSB DEFS.H line 2602 M603_VIEW_SQUARE_D2C");
    ok &= expect_true("spec.unknown.absent",
                      csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(5) == 0,
                      A_F0128);
    ok &= expect_true("spec.unknown.d1c.absent",
                      csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(3) == 0,
                      A_F0128);
    ok &= expect_true("spec.unknown.d2l2.absent",
                      csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(9) == 0,
                      "non-duplicative with existing D2L2 F0111 partly-open gate");
    ok &= expect_true("spec.unknown.d2r2.absent",
                      csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(10) == 0,
                      "non-duplicative with existing D2R2 F0111 partly-open gate");
    ok &= expect_true("spec.at0.d2c",
                      csb_v1_viewport_d2c_f0111_partly_open_door_spec_at_pc34(0) == d2c,
                      A_F0121);
    ok &= expect_true("spec.at1.null",
                      csb_v1_viewport_d2c_f0111_partly_open_door_spec_at_pc34(1) == 0,
                      A_F0128);
    ok &= expect_int("d2c.contract_only",
                     d2c ? d2c->source_locked_contract_only : -1, 1, A_F0111);
    ok &= expect_int("d2c.no_game_data",
                     d2c ? d2c->no_game_data_load : -1, 1,
                     "contract-only no CSB game-data load");
    ok &= expect_int("d2c.no_real_asset_pixel_parity",
                     d2c ? d2c->no_real_asset_pixel_parity : -1, 1, A_NO_PIXEL);

    return ok;
}

static int test_f0121_d2c_body_and_f0128_dispatch(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *d2c =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(6);

    ok &= expect_int("d2c.view_square", d2c ? d2c->view_square_d2c : -1, 6,
                     "ReDMCSB DEFS.H line 2602 M603_VIEW_SQUARE_D2C");
    ok &= expect_int("d2c.f0121_function",
                     d2c ? d2c->f0121_function_number : -1, 121, A_F0121);
    ok &= expect_int("d2c.f0128_dispatch_order",
                     d2c ? d2c->f0128_dispatch_order : -1, 10, A_F0128);
    ok &= expect_int("d2c.f0128_depth",
                     d2c ? d2c->f0128_relative_depth : -1, 2, A_F0128);
    ok &= expect_int("d2c.f0128_lane",
                     d2c ? d2c->f0128_relative_lane : -9, 0, A_F0128);
    ok &= expect_int("d2c.excludes_d0l_d0r_fallback",
                     d2c ? d2c->excludes_d0l_d0r_f0100_f0105_f0107_fallback : -1,
                     1, A_F0128);
    ok &= expect_int("d2c.door_native_width",
                     d2c ? d2c->door_native_width : -1, 64, A_F0121_DOOR);
    ok &= expect_int("d2c.door_native_height",
                     d2c ? d2c->door_native_height : -1, 61, A_F0121_DOOR);
    ok &= expect_int("d2c.door_native_byte_count",
                     d2c ? d2c->door_native_byte_count : -1, 1952, A_F0121_DOOR);
    ok &= expect_int("d2c.doorpass1_order",
                     d2c ? d2c->doorpass1_order : -1, 0x0218, A_F0121_DOOR);
    ok &= expect_int("d2c.doorpass2_order",
                     d2c ? d2c->doorpass2_order : -1, 0x0349, A_F0121_DOOR);

    return ok;
}

static int test_d2_wall_anchors_and_zones(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *d2c =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(6);

    ok &= expect_int("d2c.wall_zone_d2c",
                     d2c ? d2c->wall_zone_d2c_binding : -1, 709,
                     "ReDMCSB DEFS.H line 4049 C709_ZONE_WALL_D2C");
    ok &= expect_int("d2c.wall_zone_d2l2",
                     d2c ? d2c->wall_zone_d2l2_binding : -1, 707,
                     "ReDMCSB DEFS.H line 4047 C707_ZONE_WALL_D2L2");
    ok &= expect_int("d2c.wall_zone_d2r2",
                     d2c ? d2c->wall_zone_d2r2_binding : -1, 708,
                     "ReDMCSB DEFS.H line 4048 C708_ZONE_WALL_D2R2");
    ok &= expect_int("d2c.no_f0678_f0679_direct",
                     d2c ? d2c->f0678_f0679_d2l2_d2r2_direct_f0111_route_present : -1,
                     0, A_F0678_F0679);
    ok &= expect_int("d2c.wall_returns",
                     d2c ? d2c->wall_case_returns_before_f0111 : -1, 1,
                     A_F0678_F0679);
    ok &= expect_int("d2c.door_zone_d2c",
                     d2c ? d2c->door_zone_d2c : -1, 3760,
                     "ReDMCSB DEFS.H line 4256 M628_ZONE_DOOR_D2C");
    ok &= expect_int("d2c.door_zone_d2l2",
                     d2c ? d2c->door_zone_d2l2 : -1, 3700,
                     "ReDMCSB DEFS.H line 4250/4242 C3700_ZONE_DOOR_D2L2");
    ok &= expect_int("d2c.door_zone_d2r2",
                     d2c ? d2c->door_zone_d2r2 : -1, 3710,
                     "ReDMCSB DEFS.H line 4252/4244 C3710_ZONE_DOOR_D2R2");

    return ok;
}

static int test_nonduplication_markers(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *d2c =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(6);

    ok &= expect_int("not.front_clipped_gate",
                     d2c ? d2c->excludes_existing_front_clipped_gate : -1, 1,
                     "existing CSB D2L2/D2R2 F0111 front-clipped gate");
    ok &= expect_int("not.d2l2_d2r2_partly_open_gate",
                     d2c ? d2c->excludes_existing_d2l2_d2r2_partly_open_gate : -1,
                     1,
                     "existing csb_v1_viewport_d2l2_d2r2_f0111_partly_open_pc34_compat gate");
    ok &= expect_int("not.d2l2_d2r2_wall_gate",
                     d2c ? d2c->excludes_existing_d2l2_d2r2_wall_gate : -1, 1,
                     "existing CSB D2L2/D2R2 wall gate");
    ok &= expect_int("not.d1l2_d1r2_partly_open_gate",
                     d2c ? d2c->excludes_existing_d1l2_d1r2_partly_open_gate : -1,
                     1,
                     "existing CSB D1L2/D1R2 F0111 partly-open gate");
    ok &= expect_int("not.closed_d2c_gate",
                     d2c ? d2c->excludes_existing_closed_d2c_gate : -1, 1,
                     "existing csb_v1_viewport_d2c_f0111_door_pc34_compat closed gate");

    return ok;
}

static int test_f0111_state_and_zone_math(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *d2c =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(6);

    ok &= expect_int("decrements_state.flag",
                     d2c ? d2c->decrements_state_before_frame_select : -1, 1,
                     "ReDMCSB DUNVIEW.C F0111 line 4307 P0125_ui_DoorState--");
    ok &= expect_int("branch.open",
                     csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(
                         d2c, 0),
                     CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_OPEN_PC34,
                     "ReDMCSB DUNVIEW.C F0111 line 4248 open skip");
    ok &= expect_int("branch.partly1",
                     csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(d2c, 1),
                     CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_PARTLY_OPEN_PC34,
                     A_F0111);
    ok &= expect_int("branch.partly2",
                     csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(d2c, 2),
                     CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_PARTLY_OPEN_PC34,
                     A_F0111);
    ok &= expect_int("branch.partly3",
                     csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(d2c, 3),
                     CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_PARTLY_OPEN_PC34,
                     A_F0111);
    ok &= expect_int("branch.closed",
                     csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(d2c, 4),
                     CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_CLOSED_PC34,
                     "ReDMCSB DUNVIEW.C F0111 line 4297 closed branch");
    ok &= expect_int("branch.destroyed",
                     csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(d2c, 5),
                     CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_DESTROYED_PC34,
                     "ReDMCSB DUNVIEW.C F0111 line 4301 destroyed branch");
    ok &= expect_int("branch.invalid",
                     csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(d2c, 6),
                     CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_INVALID_PC34,
                     A_F0111);
    ok &= expect_int("branch.null",
                     csb_v1_viewport_d2c_f0111_partly_open_door_branch_pc34(0, 2),
                     CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_BRANCH_INVALID_PC34, A_F0111);

    return ok;
}

static int test_f0111_frame_bitmap_selection(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *d2c =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(6);

    ok &= expect_contains("frame.left.d2c",
                          csb_v1_viewport_d2c_f0111_partly_open_door_frame_bitmap_pc34(
                              d2c, 2, 0),
                          "D2C.LeftHorizontal", A_F0111_FRAME);
    ok &= expect_contains("frame.right.d2c",
                          csb_v1_viewport_d2c_f0111_partly_open_door_frame_bitmap_pc34(
                              d2c, 2, 1),
                          "D2C.RightHorizontal", A_F0111_FRAME);
    ok &= expect_true("frame.open.null",
                      csb_v1_viewport_d2c_f0111_partly_open_door_frame_bitmap_pc34(
                          d2c, 0, 0) == 0,
                      "ReDMCSB DUNVIEW.C F0111 line 4248 open skip");
    ok &= expect_true("frame.closed.null",
                      csb_v1_viewport_d2c_f0111_partly_open_door_frame_bitmap_pc34(
                          d2c, 4, 0) == 0,
                      "closed branch does not select LeftHorizontal/RightHorizontal");
    ok &= expect_true("frame.destroyed.null",
                      csb_v1_viewport_d2c_f0111_partly_open_door_frame_bitmap_pc34(
                          d2c, 5, 1) == 0,
                      "destroyed branch does not select LeftHorizontal/RightHorizontal");
    ok &= expect_true("frame.invalid.null",
                      csb_v1_viewport_d2c_f0111_partly_open_door_frame_bitmap_pc34(
                          d2c, 6, 0) == 0,
                      "invalid state does not select LeftHorizontal/RightHorizontal");

    return ok;
}

static int test_f0111_first_and_second_half_zone_math(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *d2c =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(6);

    ok &= expect_int("first.state1",
                     csb_v1_viewport_d2c_f0111_partly_open_door_first_half_zone_pc34(
                         d2c, 1, 1),
                     3767, A_F0111_FIRST);
    ok &= expect_int("first.state2",
                     csb_v1_viewport_d2c_f0111_partly_open_door_first_half_zone_pc34(
                         d2c, 2, 1),
                     3768, A_F0111_FIRST);
    ok &= expect_int("first.state3",
                     csb_v1_viewport_d2c_f0111_partly_open_door_first_half_zone_pc34(
                         d2c, 3, 1),
                     3769, A_F0111_FIRST);
    ok &= expect_int("first.vertical.rejected",
                     csb_v1_viewport_d2c_f0111_partly_open_door_first_half_zone_pc34(
                         d2c, 2, 0),
                     -1, A_F0111_FIRST);
    ok &= expect_int("first.open.rejected",
                     csb_v1_viewport_d2c_f0111_partly_open_door_first_half_zone_pc34(
                         d2c, 0, 1),
                     -1, "ReDMCSB DUNVIEW.C F0111 line 4248 open skip");

    ok &= expect_int("second.state1.horizontal",
                     csb_v1_viewport_d2c_f0111_partly_open_door_second_half_zone_pc34(
                         d2c, 1, 1),
                     20148, A_F0111_SECOND);
    ok &= expect_int("second.state2.horizontal",
                     csb_v1_viewport_d2c_f0111_partly_open_door_second_half_zone_pc34(
                         d2c, 2, 1),
                     20149, A_F0111_SECOND);
    ok &= expect_int("second.state3.horizontal",
                     csb_v1_viewport_d2c_f0111_partly_open_door_second_half_zone_pc34(
                         d2c, 3, 1),
                     20150, A_F0111_SECOND);
    ok &= expect_int("second.state2.vertical",
                     csb_v1_viewport_d2c_f0111_partly_open_door_second_half_zone_pc34(
                         d2c, 2, 0),
                     3762, "vertical D2C partly-open uses base zone + state");
    ok &= expect_int("second.closed.base",
                     csb_v1_viewport_d2c_f0111_partly_open_door_second_half_zone_pc34(
                         d2c, 4, 1),
                     3760, "ReDMCSB DUNVIEW.C F0111 line 4297 closed branch");
    ok &= expect_int("second.destroyed.base",
                     csb_v1_viewport_d2c_f0111_partly_open_door_second_half_zone_pc34(
                         d2c, 5, 1),
                     3760, "ReDMCSB DUNVIEW.C F0111 line 4301 destroyed branch");
    ok &= expect_int("second.open.rejected",
                     csb_v1_viewport_d2c_f0111_partly_open_door_second_half_zone_pc34(
                         d2c, 0, 1),
                     -1, "ReDMCSB DUNVIEW.C F0111 line 4248 open skip");

    return ok;
}

static int test_defs_and_offset_contract(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *d2c =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(6);

    ok &= expect_int("defs.c10.macro",
                     CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_C10_COLOR_FLESH_PC34,
                     10, A_DEFS);
    ok &= expect_int("defs.mask.macro",
                     CSB_V1_D2C_F0111_PARTLY_OPEN_DOOR_MASK0X4000_PC34,
                     0x4000, A_DEFS);
    ok &= expect_int("defs.c10.first_half",
                     d2c ? d2c->first_half_transparent_color : -1, 10, A_DEFS);
    ok &= expect_int("defs.c10.second_half",
                     d2c ? d2c->second_half_transparent_color : -1, 10, A_DEFS);
    ok &= expect_int("defs.first_half_zone_offset",
                     d2c ? d2c->first_half_zone_offset : -1, 6,
                     "ReDMCSB DEFS.H line 3508 C6_UNKNOWN");
    ok &= expect_int("defs.first_half_uses_f0635",
                     d2c ? d2c->first_half_uses_f0635_zone_clip : -1, 1,
                     "ReDMCSB DUNVIEW.C F0111 line 4321 F0635 zone clip");
    ok &= expect_int("defs.first_half_uses_f0654",
                     d2c ? d2c->first_half_uses_f0654_blit : -1, 1,
                     "ReDMCSB DUNVIEW.C F0111 line 4322 F0654 blit");
    ok &= expect_int("defs.first_half_shift_x",
                     d2c ? d2c->first_half_zone_shift_x_is_half_bitmap_width : -1,
                     1, "ReDMCSB DUNVIEW.C F0111 line 4319 ZoneShiftX = w/2");
    ok &= expect_int("defs.second_half_zone_offset",
                     d2c ? d2c->second_half_zone_offset : -1, 3,
                     "ReDMCSB DUNVIEW.C F0111 line 4326 +3");
    ok &= expect_int("defs.second_half_zone_mask",
                     d2c ? d2c->second_half_zone_mask : -1, 0x4000,
                     "ReDMCSB DEFS.H line 3516 MASK0x4000");
    ok &= expect_int("defs.second_half_uses_f0791",
                     d2c ? d2c->second_half_uses_f0791_drawbitmapxx : -1, 1,
                     "ReDMCSB DUNVIEW.C F0111 line 4334 F0791 draw");
    ok &= expect_int("c2600.literal_absent",
                     d2c ? d2c->c2600_literal_symbol_present : -1, 0,
                     "C2600_DOOR_PARTLY_OPEN_BITMAP absent in ReDMCSB Common/Source");
    ok &= expect_contains("c2600.anchor",
                          d2c ? d2c->c2600_anchor : 0,
                          "DUNVIEW.C:4311-4313",
                          "ReDMCSB DUNVIEW.C:4311-4313 actual bitmap-selection anchor");
    ok &= expect_contains("lineage.marker",
                          d2c && d2c->lineage_anchor ? d2c->lineage_anchor : 0,
                          "1903-1915", A_LINEAGE);

    return ok;
}

static int test_synthetic_c10_blit(void)
{
    int ok = 1;
    uint8_t source[6] = { 10, 1, 2, 10, 3, 4 };
    uint8_t destination[6] = { 0, 0, 0, 0, 0, 0 };
    int skipped = -1;
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *d2c =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(6);

    ok &= expect_int("blit.copied",
                     csb_v1_viewport_d2c_f0111_partly_open_door_synthetic_blit_pc34(
                         d2c, 2, source, 3, 2, 3, destination, 3, 2, 3, &skipped),
                     4, A_F0111_SECOND);
    ok &= expect_int("blit.skipped", skipped, 2,
                     "ReDMCSB DEFS.H line 2088 C10_COLOR_FLESH transparency");
    ok &= expect_int("blit.pixel1", destination[1], 1, A_F0111_SECOND);
    ok &= expect_int("blit.pixel2", destination[2], 2, A_F0111_SECOND);
    ok &= expect_int("blit.pixel4", destination[4], 3, A_F0111_SECOND);
    ok &= expect_int("blit.pixel5", destination[5], 4, A_F0111_SECOND);
    ok &= expect_int("blit.transparent0", destination[0], 0,
                     "C10 transparent source preserved");
    ok &= expect_int("blit.open.skip",
                     csb_v1_viewport_d2c_f0111_partly_open_door_synthetic_blit_pc34(
                         d2c, 0, source, 3, 2, 3, destination, 3, 2, 3, 0),
                     0, "ReDMCSB DUNVIEW.C F0111 line 4248");
    ok &= expect_int("blit.reject.bad_stride",
                     csb_v1_viewport_d2c_f0111_partly_open_door_synthetic_blit_pc34(
                         d2c, 2, source, 3, 2, 2, destination, 3, 2, 3, 0),
                     -1, "synthetic source stride guard");
    ok &= expect_int("blit.reject.too_small_dest",
                     csb_v1_viewport_d2c_f0111_partly_open_door_synthetic_blit_pc34(
                         d2c, 2, source, 3, 2, 3, destination, 2, 2, 2, 0),
                     -1, "synthetic destination bounds guard");
    ok &= expect_int("blit.reject.null_contract",
                     csb_v1_viewport_d2c_f0111_partly_open_door_synthetic_blit_pc34(
                         0, 2, source, 3, 2, 3, destination, 3, 2, 3, 0),
                     -1, "null spec guard");

    return ok;
}

static int test_probe_and_evidence(void)
{
    int ok = 1;
    CSB_V1_D2CF0111PartlyOpenDoorProbePc34 probe;
    const char *e =
        csb_v1_viewport_d2c_f0111_partly_open_door_source_evidence_pc34();
    const CSB_V1_D2CF0111PartlyOpenDoorSpecPc34 *d2c =
        csb_v1_viewport_d2c_f0111_partly_open_door_spec_for_square_pc34(6);

    ok &= expect_int("probe.run",
                     csb_v1_viewport_d2c_f0111_partly_open_door_probe_pc34_compat(&probe),
                     0, "synthetic PASS659 probe");
    ok &= expect_int("probe.route_count", probe.route_count, 1, A_F0128);
    ok &= expect_int("probe.dispatch", probe.dispatch_order_ok, 1, A_F0128);
    ok &= expect_int("probe.branch", probe.branch_state_ok, 1, A_F0111);
    ok &= expect_int("probe.frame", probe.frame_selection_ok, 1, A_F0111_FRAME);
    ok &= expect_int("probe.first_half", probe.first_half_zone, 3768, A_F0111_FIRST);
    ok &= expect_int("probe.second_half", probe.second_half_zone, 20149, A_F0111_SECOND);
    ok &= expect_int("probe.horizontal_mask", probe.horizontal_mask_ok, 1, A_F0111_SECOND);
    ok &= expect_int("probe.c10_transparency", probe.c10_transparency_ok, 1, A_F0111_SECOND);
    ok &= expect_int("probe.copied", probe.copied_pixels, 4, A_F0111_SECOND);
    ok &= expect_int("probe.skipped", probe.c10_skipped_pixels, 2, A_DEFS);
    ok &= expect_int("probe.no_pixel", probe.no_real_asset_pixel_parity, 1,
                     A_NO_PIXEL);
    ok &= expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4337", A_F0111);
    ok &= expect_contains("evidence.f0111_partly", e, "4311-4313", A_F0111);
    ok &= expect_contains("evidence.f0121", e, "DUNVIEW.C:7244-7389", A_F0121);
    ok &= expect_contains("evidence.f0121_door", e, "C17_ELEMENT_DOOR_FRONT",
                          A_F0121_DOOR);
    ok &= expect_contains("evidence.f0128", e, "DUNVIEW.C:8508-8533", A_F0128);
    ok &= expect_contains("evidence.f0678_f0679", e, "6837-6865",
                          A_F0678_F0679);
    ok &= expect_contains("evidence.f0679_d2r2", e, "6868-6896",
                          A_F0678_F0679);
    ok &= expect_contains("evidence.defs", e, "DEFS.H:2088", A_DEFS);
    ok &= expect_contains("evidence.defs_d2c_zone", e, "4256", A_DEFS);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1903-1915",
                          A_LINEAGE);
    ok &= expect_contains("evidence.no_pixel", e, "no real-asset pixel parity",
                          A_NO_PIXEL);
    ok &= expect_contains("evidence.c2600_absent", e,
                          "C2600_DOOR_PARTLY_OPEN_BITMAP is absent",
                          "missing literal symbol marker");
    ok &= expect_contains("spec.f0111_anchor", d2c ? d2c->f0111_anchor : 0,
                          "4218-4337", A_F0111);
    ok &= expect_contains("spec.f0121_anchor", d2c ? d2c->f0121_anchor : 0,
                          "7244-7389", A_F0121);
    ok &= expect_contains("spec.f0128_anchor", d2c ? d2c->f0128_anchor : 0,
                          "8508-8533", A_F0128);
    ok &= expect_contains("spec.f0678_f0679_anchor",
                          d2c ? d2c->f0678_f0679_anchor : 0,
                          "6837-6896", A_F0678_F0679);
    ok &= expect_contains("spec.defs_anchor", d2c ? d2c->defs_anchor : 0,
                          "4256", A_DEFS);
    ok &= expect_contains("spec.lineage_anchor", d2c ? d2c->lineage_anchor : 0,
                          "1903-1915", A_LINEAGE);
    ok &= expect_contains("spec.c2600_anchor", d2c ? d2c->c2600_anchor : 0,
                          "4311-4313",
                          "ReDMCSB DUNVIEW.C:4311-4313 actual bitmap-selection anchor");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2c_f0111_partly_open_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2c_f0111_partly_open_door_source_evidence_pc34());

    ok &= test_spec_identity();
    ok &= test_f0121_d2c_body_and_f0128_dispatch();
    ok &= test_d2_wall_anchors_and_zones();
    ok &= test_nonduplication_markers();
    ok &= test_f0111_state_and_zone_math();
    ok &= test_f0111_frame_bitmap_selection();
    ok &= test_f0111_first_and_second_half_zone_math();
    ok &= test_defs_and_offset_contract();
    ok &= test_synthetic_c10_blit();
    ok &= test_probe_and_evidence();
    ok &= expect_true("assertion_count_at_least_40", g_assertions >= 40,
                      "assigned PASS659 CSB V1 D2C F0111 partly-open door gate");

    printf("assertions=%d\n", g_assertions);
    if (ok) {
        printf("PASS csb_v1_viewport_d2c_f0111_partly_open_door_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
