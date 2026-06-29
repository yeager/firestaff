/*
 * Focused DM1 V1 D2L/D2R door-frame-top edge source-lock probe.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - DUNVIEW.C:604-606 G0173_auc_Graphic558_Frame_DoorFrameTop_D2L =
 *   { 0, 59, 22, 24, 48, 3, 16, 0 } and G0175_auc_Graphic558_Frame_
 *   DoorFrameTop_D2R = { 164, 223, 22, 24, 48, 3, 16, 0 }.
 * - DUNVIEW.C:6991 F0119_DUNGEONVIEW_DrawSquareD2L C17_ELEMENT_DOOR_FRONT
 *   MEDIA009 (legacy) calls F0100_DUNGEONVIEW_DrawWallSetBitmap(G0703,
 *   G0173).
 * - DUNVIEW.C:6994 F0119 D2L MEDIA508 (F20E) calls F0104(G2114,
 *   C725_ZONE_DOOR_FRAME_TOP_D2L).
 * - DUNVIEW.C:6997 F0119 D2L MEDIA720 (I34E) calls F0104(G2114,
 *   C729_ZONE_DOOR_FRAME_TOP_D2L).
 * - DUNVIEW.C:7184 F0120_DUNGEONVIEW_DrawSquareD2R_CPSF MEDIA009 calls
 *   F0100(G0703, G0175).
 * - DUNVIEW.C:7187 F0120 D2R MEDIA508 (F20E) calls F0104(G2113,
 *   C727_ZONE_DOOR_FRAME_TOP_D2R).
 * - DUNVIEW.C:7190 F0120 D2R MEDIA720 (I34E) calls F0104(G2113,
 *   C731_ZONE_DOOR_FRAME_TOP_D2R).
 * - DUNVIEW.C:6900 / 7051 F0119 / F0120 dispatch start lines;
 *   DUNVIEW.C:8513 F0128 caller site.
 *
 * Asset-free, contract-only, no original DOS pixel parity claim.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "firestaff/dm1/v1/viewport/d2l_d2r_door_frame_top_edge_pc34_compat.h"

enum {
    D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY = 0,
    D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E = 1,
    D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E = 2
};

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

static int run_result_invariants(void)
{
    const DM1_V1_D2L_D2RDoorFrameTopEdgeSelfTestResultPc34 *result =
        dm1_v1_viewport_d2l_d2r_door_frame_top_edge_last_self_test_result_pc34();
    int ok = 1;

    if (!result) {
        fprintf(stderr, "FAIL result_invariants no_result_pointer\n");
        return 0;
    }

    ok &= expect_int("self_test_failures", result->failures, 0);
    ok &= expect_int("self_test_assertions_nonzero",
        result->assertions > 0 ? 1 : 0, 1);
    ok &= expect_int("self_test_hash",
        (int)result->deterministic_hash,
        (int)DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_HASH_PC34);
    ok &= expect_int("d2l_legacy_count", result->d2l_legacy_zone_count, 1);
    ok &= expect_int("d2l_f20e_count", result->d2l_f20e_zone_count, 1);
    ok &= expect_int("d2l_i34e_count", result->d2l_i34e_zone_count, 1);
    ok &= expect_int("d2r_legacy_count", result->d2r_legacy_zone_count, 1);
    ok &= expect_int("d2r_f20e_count", result->d2r_f20e_zone_count, 1);
    ok &= expect_int("d2r_i34e_count", result->d2r_i34e_zone_count, 1);
    ok &= expect_int("invalid_target_count", result->invalid_target_count, 2);
    ok &= expect_int("stride_g0173_checks", result->stride_g0173_checks, 6);
    ok &= expect_int("stride_g0175_checks", result->stride_g0175_checks, 6);
    ok &= expect_int("band_strip_checks", result->band_strip_checks, 6);
    ok &= expect_int("zone_id_family_checks",
        result->zone_id_family_checks, 6);
    ok &= expect_int("door_panel_post_band_checks",
        result->door_panel_post_band_checks, 6);
    ok &= expect_int("view_square_anchor_checks",
        result->view_square_anchor_checks, 6);
    ok &= expect_int("non_overlap_checks", result->non_overlap_checks, 22);
    ok &= expect_int("bitmap_route_checks", result->bitmap_route_checks, 6);
    ok &= expect_int("c10_transparency_checks",
        result->c10_transparency_checks, 6);

    printf("resultInvariants assertions=%d hash=0x%08x "
           "stride_g0173=%d stride_g0175=%d band_strip=%d zone=%d "
           "nonOverlap=%d\n",
           result->assertions,
           (unsigned)result->deterministic_hash,
           result->stride_g0173_checks,
           result->stride_g0175_checks,
           result->band_strip_checks,
           result->zone_id_family_checks,
           result->non_overlap_checks);
    return ok;
}

static int run_trace_case(
    const char *label,
    int side,
    int target_media,
    int expected_d2l_zone,
    int expected_d2r_zone)
{
    DM1_V1_D2L_D2RDoorFrameTopEdgeTracePc34 trace;
    int ok = 1;

    ok &= expect_int("trace_ok",
        dm1_v1_viewport_d2l_d2r_door_frame_top_edge_trace_pc34(
            side, target_media, &trace), 1);
    ok &= expect_int("trace_side", trace.side, side);
    ok &= expect_int("trace_target_media", trace.target_media, target_media);

    ok &= expect_int("framebuffer_width", trace.framebuffer_width, 320);
    ok &= expect_int("framebuffer_height", trace.framebuffer_height, 200);
    ok &= expect_int("viewport_width", trace.viewport_width, 224);
    ok &= expect_int("viewport_height", trace.viewport_height, 136);

    ok &= expect_int("d2l_stride_left", trace.d2l_stride_left_x, 0);
    ok &= expect_int("d2l_stride_right", trace.d2l_stride_right_x, 59);
    ok &= expect_int("d2l_stride_top", trace.d2l_stride_top_y, 22);
    ok &= expect_int("d2l_stride_bottom", trace.d2l_stride_bottom_y, 24);
    ok &= expect_int("d2l_stride_byte_width", trace.d2l_stride_byte_width, 48);
    ok &= expect_int("d2l_stride_height", trace.d2l_stride_height, 3);
    ok &= expect_int("d2l_stride_x_offset", trace.d2l_stride_x_offset, 16);
    ok &= expect_int("d2l_stride_y_offset", trace.d2l_stride_y_offset, 0);

    ok &= expect_int("d2r_stride_left", trace.d2r_stride_left_x, 164);
    ok &= expect_int("d2r_stride_right", trace.d2r_stride_right_x, 223);
    ok &= expect_int("d2r_stride_top", trace.d2r_stride_top_y, 22);
    ok &= expect_int("d2r_stride_bottom", trace.d2r_stride_bottom_y, 24);
    ok &= expect_int("d2r_stride_byte_width", trace.d2r_stride_byte_width, 48);
    ok &= expect_int("d2r_stride_height", trace.d2r_stride_height, 3);
    ok &= expect_int("d2r_stride_x_offset", trace.d2r_stride_x_offset, 16);
    ok &= expect_int("d2r_stride_y_offset", trace.d2r_stride_y_offset, 0);

    ok &= expect_int("band_top_y", trace.band_top_y, 22);
    ok &= expect_int("band_bottom_y", trace.band_bottom_y, 24);
    ok &= expect_int("band_height", trace.band_height, 3);
    ok &= expect_int("band_byte_width", trace.band_byte_width, 48);

    ok &= expect_int("m604_view_square_d2l", trace.m604_view_square_d2l, 7);
    ok &= expect_int("m605_view_square_d2r", trace.m605_view_square_d2r, 8);
    ok &= expect_int("d2l_pass1_cell_order", trace.d2l_pass1_cell_order,
        0x0218);
    ok &= expect_int("d2r_pass1_cell_order", trace.d2r_pass1_cell_order,
        0x0128);

    ok &= expect_int("door_panel_top_y", trace.door_panel_top_y, 24);
    ok &= expect_int("door_panel_bottom_y", trace.door_panel_bottom_y, 82);
    ok &= expect_int("door_panel_height", trace.door_panel_height, 61);
    ok &= expect_int("door_panel_byte_width", trace.door_panel_byte_width, 32);
    ok &= expect_int("bitmap_byte_count_d2lcr",
        trace.bitmap_byte_count_d2lcr, 3904);

    ok &= expect_int("door_frame_top_bitmap_id",
        trace.door_frame_top_bitmap_id, 703);
    ok &= expect_int("door_frame_top_stride_d2l_id",
        trace.door_frame_top_stride_d2l_id, 173);
    ok &= expect_int("door_frame_top_stride_d2r_id",
        trace.door_frame_top_stride_d2r_id, 175);
    ok &= expect_int("door_frame_top_native_bitmap_d2l",
        trace.door_frame_top_native_bitmap_d2l, 2114);
    ok &= expect_int("door_frame_top_native_bitmap_d2r",
        trace.door_frame_top_native_bitmap_d2r, 2113);
    ok &= expect_int("door_frame_top_native_bitmap_d2lcr",
        trace.door_frame_top_native_bitmap_d2lcr, 2115);

    ok &= expect_int("f0100_blit_transparency_color",
        trace.f0100_blit_transparency_color, 10);
    ok &= expect_int("c10_transparent_blit", trace.c10_transparent_blit, 1);

    ok &= expect_int("f0119_d2l_door_front_line",
        trace.f0119_d2l_door_front_line, 6987);
    ok &= expect_int("f0119_d2l_door_frame_top_legacy_line",
        trace.f0119_d2l_door_frame_top_legacy_line, 6991);
    ok &= expect_int("f0119_d2l_door_frame_top_f20e_line",
        trace.f0119_d2l_door_frame_top_f20e_line, 6994);
    ok &= expect_int("f0119_d2l_door_frame_top_i34e_line",
        trace.f0119_d2l_door_frame_top_i34e_line, 6997);
    ok &= expect_int("f0120_d2r_door_front_line",
        trace.f0120_d2r_door_front_line, 7180);
    ok &= expect_int("f0120_d2r_door_frame_top_legacy_line",
        trace.f0120_d2r_door_frame_top_legacy_line, 7184);
    ok &= expect_int("f0120_d2r_door_frame_top_f20e_line",
        trace.f0120_d2r_door_frame_top_f20e_line, 7187);
    ok &= expect_int("f0120_d2r_door_frame_top_i34e_line",
        trace.f0120_d2r_door_frame_top_i34e_line, 7190);
    ok &= expect_int("f0128_dispatch_line", trace.f0128_dispatch_line, 8513);

    ok &= expect_int("d2l_door_frame_top_zone",
        trace.d2l_door_frame_top_zone, expected_d2l_zone);
    ok &= expect_int("d2r_door_frame_top_zone",
        trace.d2r_door_frame_top_zone, expected_d2r_zone);

    ok &= expect_int("g0694_door_panel_bitmap",
        trace.g0694_door_panel_bitmap, 694);
    ok &= expect_int("g0182_door_frames_d2l",
        trace.g0182_door_frames_d2l, 182);
    ok &= expect_int("g0184_door_frames_d2r",
        trace.g0184_door_frames_d2r, 184);
    ok &= expect_int("c1_view_door_ornament_d2lcr",
        trace.c1_view_door_ornament_d2lcr, 1);

    ok &= expect_int("d2l_band_in_viewport", trace.d2l_band_in_viewport, 1);
    ok &= expect_int("d2r_band_in_viewport", trace.d2r_band_in_viewport, 1);
    ok &= expect_int("d2l_band_above_door_panel",
        trace.d2l_band_inside_door_panel_band, 1);
    ok &= expect_int("d2r_band_above_door_panel",
        trace.d2r_band_inside_door_panel_band, 1);

    ok &= expect_int("band_strip_destination_x_d2l",
        trace.band_strip_destination_x_d2l, 16);
    ok &= expect_int("band_strip_destination_x_d2r",
        trace.band_strip_destination_x_d2r, 180);
    ok &= expect_int("band_strip_destination_y",
        trace.band_strip_destination_y, 22);
    ok &= expect_int("band_strip_byte_width_d2l",
        trace.band_strip_byte_width_d2l, 48);
    ok &= expect_int("band_strip_byte_width_d2r",
        trace.band_strip_byte_width_d2r, 48);

    printf("traceCase=%s side=%d target=%d d2lZone=%d d2rZone=%d "
           "g2114=%d g2113=%d g0703=%d\n",
           label,
           side,
           target_media,
           trace.d2l_door_frame_top_zone,
           trace.d2r_door_frame_top_zone,
           trace.d2l_i34e_route_uses_g2114,
           trace.d2r_i34e_route_uses_g2113,
           trace.d2l_legacy_route_uses_g0703);
    return ok;
}

static int run_trace_invariants(void)
{
    int ok = 1;

    ok &= run_trace_case(
        "d2lLegacy",
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
        D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34);
    ok &= run_trace_case(
        "d2lF20E",
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
        D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C725_ZONE_D2L_F20E_PC34,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C727_ZONE_D2R_F20E_PC34);
    ok &= run_trace_case(
        "d2lI34E",
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
        D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C729_ZONE_D2L_I34E_PC34,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C731_ZONE_D2R_I34E_PC34);
    ok &= run_trace_case(
        "d2rLegacy",
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
        D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34);
    ok &= run_trace_case(
        "d2rF20E",
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
        D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C725_ZONE_D2L_F20E_PC34,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C727_ZONE_D2R_F20E_PC34);
    ok &= run_trace_case(
        "d2rI34E",
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
        D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C729_ZONE_D2L_I34E_PC34,
        DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C731_ZONE_D2R_I34E_PC34);

    return ok;
}

static int run_invalid_trace_invariants(void)
{
    int ok = 1;

    /* Invalid target_media. */
    {
        DM1_V1_D2L_D2RDoorFrameTopEdgeTracePc34 trace;
        ok &= expect_int("invalid_target_rc",
            dm1_v1_viewport_d2l_d2r_door_frame_top_edge_trace_pc34(
                DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34, 99, &trace),
            0);
    }

    /* Invalid side. */
    {
        DM1_V1_D2L_D2RDoorFrameTopEdgeTracePc34 trace;
        ok &= expect_int("invalid_side_rc",
            dm1_v1_viewport_d2l_d2r_door_frame_top_edge_trace_pc34(
                99, D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY, &trace),
            0);
    }

    /* Null output. */
    ok &= expect_int("null_output_rc",
        dm1_v1_viewport_d2l_d2r_door_frame_top_edge_trace_pc34(
            DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
            D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY, NULL),
        0);

    return ok;
}

static int run_source_evidence_invariants(void)
{
    const char *evidence =
        dm1_v1_viewport_d2l_d2r_door_frame_top_edge_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("src_g0173", evidence,
        "G0173_auc_Graphic558_Frame_DoorFrameTop_D2L");
    ok &= expect_contains("src_g0175", evidence,
        "G0175_auc_Graphic558_Frame_DoorFrameTop_D2R");
    ok &= expect_contains("src_d2l_stride_values", evidence,
        "{ 0, 59, 22, 24, 48, 3, 16, 0 }");
    ok &= expect_contains("src_d2r_stride_values", evidence,
        "{ 164, 223, 22, 24, 48, 3, 16, 0 }");
    ok &= expect_contains("src_f0119_d2l_legacy", evidence, "DUNVIEW.C:6991");
    ok &= expect_contains("src_f0119_d2l_f20e", evidence, "DUNVIEW.C:6994");
    ok &= expect_contains("src_f0119_d2l_i34e", evidence, "DUNVIEW.C:6997");
    ok &= expect_contains("src_f0120_d2r_legacy", evidence, "DUNVIEW.C:7184");
    ok &= expect_contains("src_f0120_d2r_f20e", evidence, "DUNVIEW.C:7187");
    ok &= expect_contains("src_f0120_d2r_i34e", evidence, "DUNVIEW.C:7190");
    ok &= expect_contains("src_f0119_start", evidence, "DUNVIEW.C:6900");
    ok &= expect_contains("src_f0120_start", evidence, "DUNVIEW.C:7051");
    ok &= expect_contains("src_f0128_dispatch", evidence, "DUNVIEW.C:8513");
    ok &= expect_contains("src_g0703", evidence, "G0703_puc_Bitmap_WallSet_DoorFrameTop_D2LCR");
    ok &= expect_contains("src_g2114", evidence, "G2114_DoorFrameTopD2L");
    ok &= expect_contains("src_g2113", evidence, "G2113_DoorFrameTopD2R");
    ok &= expect_contains("src_c725", evidence,
        "C725_ZONE_DOOR_FRAME_TOP_D2L = 725");
    ok &= expect_contains("src_c727", evidence,
        "C727_ZONE_DOOR_FRAME_TOP_D2R = 727");
    ok &= expect_contains("src_c729", evidence,
        "C729_ZONE_DOOR_FRAME_TOP_D2L = 729");
    ok &= expect_contains("src_c731", evidence,
        "C731_ZONE_DOOR_FRAME_TOP_D2R = 731");
    ok &= expect_contains("src_m604_m605", evidence, "M604_VIEW_SQUARE_D2L");
    ok &= expect_contains("src_c10", evidence, "C10_COLOR_FLESH");
    ok &= expect_contains("src_non_overlap_marker", evidence,
        "pass794-d2l-d2r-door-frame-top-edge-source-lock");
    ok &= expect_contains("src_no_original_parity", evidence,
        "no real-asset or original-DOS pixel parity claim");
    ok &= expect_contains("src_bridge", evidence,
        "door-frame-top edge stride, zone, and dispatch");

    return ok;
}

int main(void)
{
    int ok = 1;
    int lib_rc;
    uint32_t hash_before = 0;
    uint32_t hash_after = 0;
    const DM1_V1_D2L_D2RDoorFrameTopEdgeSelfTestResultPc34 *result;

    printf("probe=firestaff_dm1_v1_viewport_d2l_d2r_door_frame_top_edge_gate_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("sourceEvidence="
           "DUNVIEW.C:604-606,6900,6987,6991,6994,6997,7051,7180,7184,"
           "7187,7190,8513\n");

    lib_rc = run_dm1_v1_viewport_d2l_d2r_door_frame_top_edge_self_test();
    ok &= expect_int("library_self_test_rc", lib_rc, 0);
    result = dm1_v1_viewport_d2l_d2r_door_frame_top_edge_last_self_test_result_pc34();
    if (result) hash_before = result->deterministic_hash;

    (void)run_dm1_v1_viewport_d2l_d2r_door_frame_top_edge_self_test();
    result = dm1_v1_viewport_d2l_d2r_door_frame_top_edge_last_self_test_result_pc34();
    if (result) hash_after = result->deterministic_hash;

    ok &= expect_int("deterministic_hash_stable",
        (int)(hash_before == hash_after &&
              hash_after == DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_HASH_PC34), 1);
    ok &= run_result_invariants();
    ok &= run_trace_invariants();
    ok &= run_invalid_trace_invariants();
    ok &= run_source_evidence_invariants();

    if (!ok) {
        fprintf(stderr, "probe failed hash=0x%08x\n",
                (unsigned)(result ? result->deterministic_hash : 0U));
        return 1;
    }

    printf("result=pass assertions=%d failures=0 hash=0x%08x\n",
           result ? result->assertions : 0,
           (unsigned)(result ? result->deterministic_hash : 0U));
    return 0;
}
