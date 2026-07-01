/*
 * Focused DM1 V1 D2C door-frame-top edge source-lock probe.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - DUNVIEW.C:605 G0174_auc_Graphic558_Frame_DoorFrameTop_D2C =
 *   { 64, 159, 22, 24, 48, 3, 0, 0 }.
 * - DUNVIEW.C:7317 / 7323 / 7328 draw the legacy / F20E / I34E
 *   D2C top-frame routes.
 * - DUNVIEW.C:7332-7339 keeps the optional button and F0111 door
 *   panel after the top-frame band.
 * - DUNVIEW.C:8521 is the F0128 D2C caller site.
 *
 * Asset-free, contract-only, no original DOS pixel parity claim.
 */

#include <stdio.h>
#include <string.h>

#include "firestaff/dm1/v1/viewport/d2c_door_frame_top_edge_pc34_compat.h"

static int expect_int(const char *label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_contains(const char *label, const char *hay, const char *needle)
{
    if (!hay || !needle || !strstr(hay, needle)) {
        fprintf(stderr, "FAIL %s missing=%s\n", label, needle);
        return 0;
    }
    return 1;
}

static int check_trace(
    const char *label,
    int target,
    int zone,
    int bitmap,
    int uses_f0100,
    int uses_f0104)
{
    DM1_V1_D2CDoorFrameTopEdgeTracePc34 trace;
    int ok = 1;

    ok &= expect_int("trace_ok",
        dm1_v1_viewport_d2c_door_frame_top_edge_trace_pc34(target, &trace), 1);
    ok &= expect_int("target_media", trace.target_media, target);
    ok &= expect_int("stride_left_x", trace.stride_left_x, 64);
    ok &= expect_int("stride_right_x", trace.stride_right_x, 159);
    ok &= expect_int("stride_top_y", trace.stride_top_y, 22);
    ok &= expect_int("stride_bottom_y", trace.stride_bottom_y, 24);
    ok &= expect_int("stride_byte_width", trace.stride_byte_width, 48);
    ok &= expect_int("stride_height", trace.stride_height, 3);
    ok &= expect_int("stride_x_offset", trace.stride_x_offset, 0);
    ok &= expect_int("m603_view_square_d2c", trace.m603_view_square_d2c, 6);
    ok &= expect_int("pass1_cell_order", trace.pass1_cell_order, 0x0218);
    ok &= expect_int("pass2_cell_order", trace.pass2_cell_order, 0x0349);
    ok &= expect_int("selected_zone", trace.selected_zone, zone);
    ok &= expect_int("selected_bitmap", trace.selected_bitmap, bitmap);
    ok &= expect_int("selected_uses_f0100", trace.selected_uses_f0100, uses_f0100);
    ok &= expect_int("selected_uses_f0104", trace.selected_uses_f0104, uses_f0104);
    ok &= expect_int("door_button_view_index", trace.door_button_view_index, 2);
    ok &= expect_int("door_panel_top_y", trace.door_panel_top_y, 24);
    ok &= expect_int("door_panel_bottom_y", trace.door_panel_bottom_y, 82);
    ok &= expect_int("f0121_line_start", trace.f0121_line_start, 7244);
    ok &= expect_int("door_front_line", trace.door_front_line, 7313);
    ok &= expect_int("floor_ornament_line", trace.floor_ornament_line, 7314);
    ok &= expect_int("thing_pass1_line", trace.thing_pass1_line, 7315);
    ok &= expect_int("legacy_line", trace.legacy_line, 7317);
    ok &= expect_int("f20e_line", trace.f20e_line, 7323);
    ok &= expect_int("i34e_line", trace.i34e_line, 7328);
    ok &= expect_int("button_branch_line", trace.button_branch_line, 7332);
    ok &= expect_int("button_draw_line", trace.button_draw_line, 7333);
    ok &= expect_int("door_panel_legacy_line", trace.door_panel_legacy_line, 7336);
    ok &= expect_int("door_panel_modern_line", trace.door_panel_modern_line, 7339);
    ok &= expect_int("f0128_dispatch_line", trace.f0128_dispatch_line, 8521);
    ok &= expect_int("g0174_stride", trace.g0174_stride, 174);
    ok &= expect_int("g2115_native_top", trace.g2115_native_top, 2115);
    ok &= expect_int("g2118_native_left", trace.g2118_native_left, 2118);
    ok &= expect_int("g0183_door_frames_d2c", trace.g0183_door_frames_d2c, 183);
    ok &= expect_int("top_edge_inside_viewport", trace.top_edge_inside_viewport, 1);
    ok &= expect_int("top_edge_above_door_panel", trace.top_edge_above_door_panel, 1);

    printf("trace label=%s zone=%d bitmap=%d p0=%u p1=%u p2=%u\n",
           label,
           trace.selected_zone,
           trace.selected_bitmap,
           (unsigned)trace.first_probe_pixel,
           (unsigned)trace.second_probe_pixel,
           (unsigned)trace.third_probe_pixel);
    return ok;
}

int main(void)
{
    const DM1_V1_D2CDoorFrameTopEdgeSelfTestResultPc34 *result;
    const char *evidence;
    int ok = 1;

    printf("probe=firestaff_dm1_v1_viewport_d2c_door_frame_top_edge_gate_probe\n");

    ok &= expect_int("self_test_rc",
        run_dm1_v1_viewport_d2c_door_frame_top_edge_self_test(), 0);
    result = dm1_v1_viewport_d2c_door_frame_top_edge_last_self_test_result_pc34();
    if (!result) {
        fprintf(stderr, "FAIL no_result\n");
        return 1;
    }
    ok &= expect_int("self_test_failures", result->failures, 0);
    ok &= expect_int("self_test_hash",
        (int)result->deterministic_hash,
        (int)DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_HASH_PC34);
    ok &= expect_int("legacy_count", result->legacy_route_count, 1);
    ok &= expect_int("f20e_count", result->f20e_route_count, 1);
    ok &= expect_int("i34e_count", result->i34e_route_count, 1);
    ok &= expect_int("invalid_count", result->invalid_target_count, 3);

    ok &= check_trace("legacy", 0, 10, 703, 1, 0);
    ok &= check_trace("f20e", 1, 726, 2115, 0, 1);
    ok &= check_trace("i34e", 2, 730, 2115, 0, 1);

    evidence = dm1_v1_viewport_d2c_door_frame_top_edge_source_evidence_pc34();
    ok &= expect_contains("evidence_dunview605", evidence, "DUNVIEW.C:605");
    ok &= expect_contains("evidence_f0121", evidence, "F0121");
    ok &= expect_contains("evidence_7317", evidence, "7317");
    ok &= expect_contains("evidence_7323", evidence, "7323");
    ok &= expect_contains("evidence_7328", evidence, "7328");
    ok &= expect_contains("evidence_7332", evidence, "7332");
    ok &= expect_contains("evidence_8521", evidence, "8521");
    ok &= expect_contains("non_claim", evidence, "no real-asset");

    printf("%s dm1_v1_viewport_d2c_door_frame_top_edge_gate assertions=%d "
           "hash=0x%08x\n",
           ok ? "PASS" : "FAIL",
           result->assertions,
           (unsigned)result->deterministic_hash);
    return ok ? 0 : 1;
}
