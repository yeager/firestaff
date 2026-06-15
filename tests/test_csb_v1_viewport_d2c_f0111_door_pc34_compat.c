#include "csb_v1_viewport_d2c_f0111_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor";
static const char *A_F0121 =
    "ReDMCSB DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C";
static const char *A_D2C_DOOR =
    "ReDMCSB DUNVIEW.C:7313-7341 D2C C17_ELEMENT_DOOR_FRONT";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2159,2790,2796,3508,3516,4256";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C:8508-8533 F0128_DUNGEONVIEW_Draw_CPSF";
static const char *A_DUNGEON =
    "ReDMCSB DUNGEON.C:F0163/F0164:1769-1840";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1903-1915 requested; local 1865-1879";

static int g_assertions = 0;
static int g_failures = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_int(label, haystack && needle &&
                         strstr(haystack, needle) != NULL, 1, anchor);
}

static int test_identity_and_scope(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *c =
        csb_v1_viewport_d2c_f0111_door_pc34_contract();

    ok &= expect_int("contract.non_null", c != NULL, 1, A_F0121);
    ok &= expect_int("contract.only", c ? c->source_locked_contract_only : 0,
                     1, A_F0121);
    ok &= expect_int("no.real.asset.parity",
                     c ? c->no_real_asset_bitmap_parity : 0, 1, A_F0121);
    ok &= expect_int("no.game.data.load", c ? c->no_game_data_load : 0,
                     1, A_F0121);
    ok &= expect_int("view_square.d2c", c ? c->view_square_d2c : -1, 6,
                     A_F0121);
    ok &= expect_int("view_depth.d2", c ? c->view_depth : -1, 2,
                     "ReDMCSB DUNVIEW.C:372 G2027[6]");
    ok &= expect_int("view_lane.center", c ? c->view_lane : -9, 0,
                     "ReDMCSB DUNVIEW.C:371 G2026[6]");
    ok &= expect_int("element.door_front", c ? c->element_door_front : -1,
                     17, A_D2C_DOOR);
    ok &= expect_contains("f0121.anchor", c ? c->redmcsb_f0121_anchor : NULL,
                          "7313-7341", A_F0121);
    ok &= expect_contains("f0111.anchor", c ? c->redmcsb_f0111_anchor : NULL,
                          "4218-4337", A_F0111);

    return ok;
}

static int test_d2c_f0111_call_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *c =
        csb_v1_viewport_d2c_f0111_door_pc34_contract();

    ok &= expect_contains("bitmap.index.symbol", c ? c->door_bitmap_index_symbol : NULL,
                          "G0694_ai_DoorNativeBitmapIndex_Front_D2LCR",
                          A_D2C_DOOR);
    ok &= expect_int("native.width", c ? c->door_native_width : -1, 64,
                     A_D2C_DOOR);
    ok &= expect_int("native.height", c ? c->door_native_height : -1, 61,
                     A_D2C_DOOR);
    ok &= expect_int("byte_count.helper",
                     csb_v1_viewport_d2c_f0111_door_byte_count_pc34(64, 61),
                     1952, A_DEFS);
    ok &= expect_int("native.byte_count", c ? c->door_native_byte_count : -1,
                     1952, A_D2C_DOOR);
    ok &= expect_contains("byte.count.macro", c ? c->door_byte_count_macro : NULL,
                          "M075_BITMAP_BYTE_COUNT(64, 61)", A_D2C_DOOR);
    ok &= expect_int("rejects.d1.96x88",
                     c ? c->rejects_d1_96x88_byte_count : 0, 1, A_D2C_DOOR);
    ok &= expect_int("d1.96x88.helper",
                     csb_v1_viewport_d2c_f0111_door_byte_count_pc34(96, 88),
                     4224, "ReDMCSB DUNVIEW.C:7905 D1C guard");
    ok &= expect_int("d2c.not.d1.size",
                     c ? c->door_native_byte_count !=
                         csb_v1_viewport_d2c_f0111_door_byte_count_pc34(96, 88) : 0,
                     1, A_D2C_DOOR);
    ok &= expect_int("view.ornament.d2lcr",
                     c ? c->view_door_ornament_d2lcr : -1, 1, A_DEFS);
    ok &= expect_contains("view.ornament.symbol", c ? c->door_view_symbol : NULL,
                          "C1_VIEW_DOOR_ORNAMENT_D2LCR", A_D2C_DOOR);
    ok &= expect_contains("frame.symbol", c ? c->door_frame_symbol : NULL,
                          "G0183_s_Graphic558_Frames_Door_D2C", A_D2C_DOOR);
    ok &= expect_int("door.zone.d2c", c ? c->door_zone_d2c : -1, 3760,
                     A_DEFS);
    ok &= expect_contains("door.zone.symbol", c ? c->door_zone_symbol : NULL,
                          "M628_ZONE_DOOR_D2C", A_DEFS);

    return ok;
}

static int test_frame_button_and_pass_order(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *c =
        csb_v1_viewport_d2c_f0111_door_pc34_contract();

    ok &= expect_int("pass1.order", c ? c->doorpass1_order : -1, 0x0218,
                     A_D2C_DOOR);
    ok &= expect_int("pass2.order", c ? c->doorpass2_order : -1, 0x0349,
                     A_D2C_DOOR);
    ok &= expect_int("floor.before.pass1",
                     c ? c->floor_ornament_before_rear_pass : 0, 1,
                     A_D2C_DOOR);
    ok &= expect_int("rear.before.frames", c ? c->rear_pass_before_frames : 0,
                     1, A_D2C_DOOR);
    ok &= expect_int("top.before.side.frames",
                     c ? c->top_track_before_side_frames : 0, 1, A_D2C_DOOR);
    ok &= expect_int("side.before.button",
                     c ? c->side_frames_before_button : 0, 1, A_D2C_DOOR);
    ok &= expect_int("button.before.f0111", c ? c->button_before_f0111 : 0,
                     1, A_D2C_DOOR);
    ok &= expect_int("f0111.before.front.pass",
                     c ? c->f0111_before_front_pass : 0, 1, A_D2C_DOOR);
    ok &= expect_int("terminal.front.pass.ordered",
                     c ? c->terminal_front_pass_ordered : 0, 1, A_D2C_DOOR);
    ok &= expect_int("frame.top.zone", c ? c->door_frame_top_zone : -1, 730,
                     A_D2C_DOOR);
    ok &= expect_int("frame.left.zone", c ? c->door_frame_left_zone : -1, 724,
                     A_D2C_DOOR);
    ok &= expect_int("frame.right.zone", c ? c->door_frame_right_zone : -1, 725,
                     A_D2C_DOOR);
    ok &= expect_int("door.button.view", c ? c->door_button_view_d2c : -1, 2,
                     A_DEFS);
    ok &= expect_int("lineage.door.graphics.f2",
                     c ? c->door_graphic_depth_index : -1, 1, A_LINEAGE);

    return ok;
}

static int test_f0111_state_zone_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *c =
        csb_v1_viewport_d2c_f0111_door_pc34_contract();

    ok &= expect_int("open.skips.flag", c ? c->open_state_skips_f0111 : 0,
                     1, A_F0111);
    ok &= expect_int("open.zone",
                     csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(c, 0),
                     -1, A_F0111);
    ok &= expect_int("state1.zone",
                     csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(c, 1),
                     3761, A_F0111);
    ok &= expect_int("state2.zone",
                     csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(c, 2),
                     3762, A_F0111);
    ok &= expect_int("state3.zone",
                     csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(c, 3),
                     3763, A_F0111);
    ok &= expect_int("closed.zone",
                     csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(c, 4),
                     3760, A_F0111);
    ok &= expect_int("destroyed.zone",
                     csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(c, 5),
                     3760, A_F0111);
    ok &= expect_int("null.zone",
                     csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(NULL, 2),
                     -1, A_F0111);
    ok &= expect_int("bad.negative.zone",
                     csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(c, -1),
                     -1, A_F0111);
    ok &= expect_int("bad.high.zone",
                     csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(c, 6),
                     -1, A_F0111);
    ok &= expect_int("closed.uses.base",
                     c ? c->closed_state_uses_base_zone : 0, 1, A_F0111);
    ok &= expect_int("destroyed.uses.base",
                     c ? c->destroyed_state_uses_base_zone : 0, 1, A_F0111);
    ok &= expect_int("destroyed.mask",
                     c ? c->destroyed_state_applies_c15_mask : -1, 15,
                     A_F0111);

    return ok;
}

static int test_horizontal_half_and_c10_blit(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *c =
        csb_v1_viewport_d2c_f0111_door_pc34_contract();
    uint8_t source[12] = { 10, 1, 2, 10, 3, 4, 10, 5, 6, 7, 10, 8 };
    uint8_t destination[12] = { 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77 };

    ok &= expect_int("partial.shifts.flag", c ? c->partial_state_shifts_zone : 0,
                     1, A_F0111);
    ok &= expect_int("horizontal.mask",
                     c ? c->horizontal_second_half_mask : -1, 0x4000, A_DEFS);
    ok &= expect_int("horizontal.left_half.state2",
                     csb_v1_viewport_d2c_f0111_door_horizontal_half_zone_pc34(
                         c, 2, 0),
                     3768, A_F0111);
    ok &= expect_int("horizontal.right_half.state2",
                     csb_v1_viewport_d2c_f0111_door_horizontal_half_zone_pc34(
                         c, 2, 1),
                     20149, A_F0111);
    ok &= expect_int("horizontal.closed.reject",
                     csb_v1_viewport_d2c_f0111_door_horizontal_half_zone_pc34(
                         c, 4, 1),
                     -1, A_F0111);
    ok &= expect_int("transparent.macro",
                     CSB_V1_D2C_F0111_DOOR_PC34_TRANSPARENT_COLOR, 10,
                     A_DEFS);
    ok &= expect_int("transparent.color", c ? c->transparent_color : -1, 10,
                     A_DEFS);
    ok &= expect_int("blit.copied",
                     csb_v1_viewport_d2c_f0111_door_apply_c10_blit_pc34(
                         c, source, 4, destination, 4, 4, 3),
                     8, A_F0111);
    ok &= expect_int("blit.transparent0", destination[0], 77, A_DEFS);
    ok &= expect_int("blit.pixel1", destination[1], 1, A_DEFS);
    ok &= expect_int("blit.pixel2", destination[2], 2, A_DEFS);
    ok &= expect_int("blit.transparent3", destination[3], 77, A_DEFS);
    ok &= expect_int("blit.pixel4", destination[4], 3, A_DEFS);
    ok &= expect_int("blit.pixel7", destination[7], 5, A_DEFS);
    ok &= expect_int("blit.transparent10", destination[10], 77, A_DEFS);
    ok &= expect_int("blit.pixel11", destination[11], 8, A_DEFS);
    ok &= expect_int("blit.reject.null.contract",
                     csb_v1_viewport_d2c_f0111_door_apply_c10_blit_pc34(
                         NULL, source, 4, destination, 4, 4, 3),
                     -1, A_F0111);
    ok &= expect_int("blit.reject.bad.stride",
                     csb_v1_viewport_d2c_f0111_door_apply_c10_blit_pc34(
                         c, source, 3, destination, 4, 4, 3),
                     -1, A_F0111);

    return ok;
}

static int test_f0128_and_noninterference(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *c =
        csb_v1_viewport_d2c_f0111_door_pc34_contract();

    ok &= expect_int("f0128.after.d2l.d2r",
                     c ? c->f0128_dispatch_after_d2l_d2r : 0, 1, A_F0128);
    ok &= expect_int("f0128.dispatches.d2c",
                     c ? c->f0128_dispatches_d2c : 0, 1, A_F0128);
    ok &= expect_int("f0128.before.d1",
                     c ? c->f0128_dispatch_before_d1l_d1r_d1c : 0, 1,
                     A_F0128);
    ok &= expect_int("not.f0119.d2l", c ? c->uses_f0119_d2l : 1, 0,
                     A_F0128);
    ok &= expect_int("not.f0120.d2r", c ? c->uses_f0120_d2r : 1, 0,
                     A_F0128);
    ok &= expect_int("not.f0124.d1c", c ? c->uses_f0124_d1c : 1, 0,
                     A_F0128);
    ok &= expect_int("dungeon.f0163.noninterference",
                     c ? c->dungeon_f0163_link_noninterference : 0, 1,
                     A_DUNGEON);
    ok &= expect_int("dungeon.f0164.noninterference",
                     c ? c->dungeon_f0164_unlink_noninterference : 0, 1,
                     A_DUNGEON);
    ok &= expect_contains("f0128.anchor", c ? c->redmcsb_f0128_anchor : NULL,
                          "8508-8533", A_F0128);
    ok &= expect_contains("dungeon.anchor", c ? c->redmcsb_dungeon_anchor : NULL,
                          "F0163/F0164", A_DUNGEON);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *c =
        csb_v1_viewport_d2c_f0111_door_pc34_contract();
    const char *e = csb_v1_viewport_d2c_f0111_door_source_evidence_pc34();

    ok &= expect_contains("defs.anchor.byte_count", c ? c->redmcsb_defs_anchor : NULL,
                          "2159", A_DEFS);
    ok &= expect_contains("defs.anchor.ornament", c ? c->redmcsb_defs_anchor : NULL,
                          "2790", A_DEFS);
    ok &= expect_contains("defs.anchor.zone", c ? c->redmcsb_defs_anchor : NULL,
                          "4256", A_DEFS);
    ok &= expect_contains("lineage.requested",
                          c ? c->csb_lineage_viewport_anchor : NULL,
                          "1903-1915", A_LINEAGE);
    ok &= expect_contains("lineage.local",
                          c ? c->csb_lineage_viewport_anchor : NULL,
                          "1865-1879", A_LINEAGE);
    ok &= expect_contains("evidence.contract", e, "Source-locked contract gate only",
                          A_F0121);
    ok &= expect_contains("evidence.f0121", e, "DUNVIEW.C:7244-7389",
                          A_F0121);
    ok &= expect_contains("evidence.branch", e, "DUNVIEW.C:7313-7341",
                          A_D2C_DOOR);
    ok &= expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4337",
                          A_F0111);
    ok &= expect_contains("evidence.bitmap", e,
                          "G0694_ai_DoorNativeBitmapIndex_Front_D2LCR",
                          A_D2C_DOOR);
    ok &= expect_contains("evidence.byte_count", e,
                          "M075_BITMAP_BYTE_COUNT(64, 61)", A_D2C_DOOR);
    ok &= expect_contains("evidence.ornament", e,
                          "C1_VIEW_DOOR_ORNAMENT_D2LCR", A_DEFS);
    ok &= expect_contains("evidence.frame", e,
                          "G0183_s_Graphic558_Frames_Door_D2C", A_D2C_DOOR);
    ok &= expect_contains("evidence.zone", e, "M628_ZONE_DOOR_D2C", A_DEFS);
    ok &= expect_contains("evidence.f0128", e, "DUNVIEW.C:8508-8533",
                          A_F0128);
    ok &= expect_contains("evidence.no_f0119", e, "does not use F0119",
                          A_F0128);
    ok &= expect_contains("evidence.no_f0120", e, "F0120", A_F0128);
    ok &= expect_contains("evidence.no_f0124", e, "F0124", A_F0128);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1903-1915",
                          A_LINEAGE);
    ok &= expect_contains("evidence.dungeon", e, "DUNGEON.C:F0163/F0164",
                          A_DUNGEON);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2c_f0111_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2c_f0111_door_source_evidence_pc34());

    ok &= test_identity_and_scope();
    ok &= test_d2c_f0111_call_contract();
    ok &= test_frame_button_and_pass_order();
    ok &= test_f0111_state_zone_contract();
    ok &= test_horizontal_half_and_c10_blit();
    ok &= test_f0128_and_noninterference();
    ok &= test_evidence_strings();
    ok &= expect_int("assertion_count_at_least_40", g_assertions >= 40, 1,
                     A_D2C_DOOR);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_d2c_f0111_door_pc34_compat assertions=%d failures=0\n",
               g_assertions);
    }

    return (ok && g_failures == 0) ? 0 : 1;
}
