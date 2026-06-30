/*
 * Focused DM1 V1 D1L/D1R door-frame-top edge source-lock probe.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - DUNVIEW.C:607-609 G0176_auc_Graphic558_Frame_DoorFrameTop_D1L =
 *   { 0, 31, 14, 17, 64, 4, 16, 0 } and G0178_auc_Graphic558_Frame_
 *   DoorFrameTop_D1R = { 192, 223, 14, 17, 64, 4, 16, 0 }.
 * - DUNVIEW.C:7496 F0122_DUNGEONVIEW_DrawSquareD1L C17_ELEMENT_DOOR_FRONT
 *   MEDIA009 (legacy) calls F0100_DUNGEONVIEW_DrawWallSetBitmap(G0704,
 *   G0176).
 * - DUNVIEW.C:7500 F0122 D1L MEDIA508 (F20E) calls F0104(G2111,
 *   C728_ZONE_DOOR_FRAME_TOP_D1L).
 * - DUNVIEW.C:7503 F0122 D1L MEDIA720 (I34E) calls F0104(G2111,
 *   C732_ZONE_DOOR_FRAME_TOP_D1L).
 * - DUNVIEW.C:7664 F0123_DUNGEONVIEW_DrawSquareD1R MEDIA009 calls
 *   F0100(G0704, G0178).
 * - DUNVIEW.C:7668 F0123 D1R MEDIA508 (F20E) calls F0104(G2110,
 *   C730_ZONE_DOOR_FRAME_TOP_D1R).
 * - DUNVIEW.C:7671 F0123 D1R MEDIA720 (I34E) calls F0104(G2110,
 *   C734_ZONE_DOOR_FRAME_TOP_D1R).
 * - DUNVIEW.C:7391 / 7559 F0122 / F0123 dispatch start lines;
 *   DUNVIEW.C:8525 / 8529 F0128 caller sites.
 *
 * Asset-free, contract-only, no original DOS pixel parity claim.
 * The optional trailing --data-dir argument is accepted for parity with
 * other probes but is intentionally ignored; this gate never opens
 * game data files.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "firestaff/dm1/v1/viewport/d1l_d1r_door_frame_top_edge_pc34_compat.h"

enum {
    D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY = 0,
    D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E = 1,
    D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E = 2
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
    const DM1_V1_D1L_D1RDoorFrameTopEdgeSelfTestResultPc34 *result =
        dm1_v1_viewport_d1l_d1r_door_frame_top_edge_last_self_test_result_pc34();
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
        (int)DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_HASH_PC34);
    ok &= expect_int("d1l_legacy_count", result->d1l_legacy_zone_count, 1);
    ok &= expect_int("d1l_f20e_count", result->d1l_f20e_zone_count, 1);
    ok &= expect_int("d1l_i34e_count", result->d1l_i34e_zone_count, 1);
    ok &= expect_int("d1r_legacy_count", result->d1r_legacy_zone_count, 1);
    ok &= expect_int("d1r_f20e_count", result->d1r_f20e_zone_count, 1);
    ok &= expect_int("d1r_i34e_count", result->d1r_i34e_zone_count, 1);
    ok &= expect_int("invalid_target_count", result->invalid_target_count, 2);
    ok &= expect_int("stride_g0176_checks", result->stride_g0176_checks, 6);
    ok &= expect_int("stride_g0178_checks", result->stride_g0178_checks, 6);
    ok &= expect_int("band_strip_checks", result->band_strip_checks, 6);
    ok &= expect_int("zone_id_family_checks",
        result->zone_id_family_checks, 6);
    ok &= expect_int("door_panel_post_band_checks",
        result->door_panel_post_band_checks, 6);
    ok &= expect_int("view_square_anchor_checks",
        result->view_square_anchor_checks, 6);
    ok &= expect_int("non_overlap_checks", result->non_overlap_checks, 30);
    ok &= expect_int("bitmap_route_checks", result->bitmap_route_checks, 6);
    ok &= expect_int("c10_transparency_checks",
        result->c10_transparency_checks, 6);

    printf("resultInvariants assertions=%d hash=0x%08x "
           "stride_g0176=%d stride_g0178=%d band_strip=%d zone=%d "
           "nonOverlap=%d\n",
           result->assertions,
           (unsigned)result->deterministic_hash,
           result->stride_g0176_checks,
           result->stride_g0178_checks,
           result->band_strip_checks,
           result->zone_id_family_checks,
           result->non_overlap_checks);
    return ok;
}

static int run_trace_case(
    const char *label,
    int side,
    int target_media,
    int expected_d1l_zone,
    int expected_d1r_zone)
{
    DM1_V1_D1L_D1RDoorFrameTopEdgeTracePc34 trace;
    int ok = 1;

    ok &= expect_int("trace_ok",
        dm1_v1_viewport_d1l_d1r_door_frame_top_edge_trace_pc34(
            side, target_media, &trace), 1);
    ok &= expect_int("trace_side", trace.side, side);
    ok &= expect_int("trace_target_media", trace.target_media, target_media);

    ok &= expect_int("framebuffer_width", trace.framebuffer_width, 320);
    ok &= expect_int("framebuffer_height", trace.framebuffer_height, 200);
    ok &= expect_int("viewport_width", trace.viewport_width, 224);
    ok &= expect_int("viewport_height", trace.viewport_height, 136);

    ok &= expect_int("d1l_stride_left", trace.d1l_stride_left_x, 0);
    ok &= expect_int("d1l_stride_right", trace.d1l_stride_right_x, 31);
    ok &= expect_int("d1l_stride_top", trace.d1l_stride_top_y, 14);
    ok &= expect_int("d1l_stride_bottom", trace.d1l_stride_bottom_y, 17);
    ok &= expect_int("d1l_stride_byte_width", trace.d1l_stride_byte_width, 64);
    ok &= expect_int("d1l_stride_height", trace.d1l_stride_height, 4);
    ok &= expect_int("d1l_stride_x_offset", trace.d1l_stride_x_offset, 16);
    ok &= expect_int("d1l_stride_y_offset", trace.d1l_stride_y_offset, 0);

    ok &= expect_int("d1r_stride_left", trace.d1r_stride_left_x, 192);
    ok &= expect_int("d1r_stride_right", trace.d1r_stride_right_x, 223);
    ok &= expect_int("d1r_stride_top", trace.d1r_stride_top_y, 14);
    ok &= expect_int("d1r_stride_bottom", trace.d1r_stride_bottom_y, 17);
    ok &= expect_int("d1r_stride_byte_width", trace.d1r_stride_byte_width, 64);
    ok &= expect_int("d1r_stride_height", trace.d1r_stride_height, 4);
    ok &= expect_int("d1r_stride_x_offset", trace.d1r_stride_x_offset, 16);
    ok &= expect_int("d1r_stride_y_offset", trace.d1r_stride_y_offset, 0);

    ok &= expect_int("band_top_y", trace.band_top_y, 14);
    ok &= expect_int("band_bottom_y", trace.band_bottom_y, 17);
    ok &= expect_int("band_height", trace.band_height, 4);
    ok &= expect_int("band_byte_width", trace.band_byte_width, 64);

    ok &= expect_int("m607_view_square_d1l", trace.m607_view_square_d1l, 7);
    ok &= expect_int("m608_view_square_d1r", trace.m608_view_square_d1r, 8);
    ok &= expect_int("d1l_pass1_cell_order", trace.d1l_pass1_cell_order,
        0x0028);
    ok &= expect_int("d1r_pass1_cell_order", trace.d1r_pass1_cell_order,
        0x0018);

    ok &= expect_int("door_panel_top_y", trace.door_panel_top_y, 17);
    ok &= expect_int("door_panel_bottom_y", trace.door_panel_bottom_y, 102);
    ok &= expect_int("door_panel_height", trace.door_panel_height, 88);
    ok &= expect_int("door_panel_byte_width", trace.door_panel_byte_width, 48);
    ok &= expect_int("bitmap_byte_count_d1lcr",
        trace.bitmap_byte_count_d1lcr, 4224);

    ok &= expect_int("door_frame_top_bitmap_id",
        trace.door_frame_top_bitmap_id, 704);
    ok &= expect_int("door_frame_top_stride_d1l_id",
        trace.door_frame_top_stride_d1l_id, 176);
    ok &= expect_int("door_frame_top_stride_d1r_id",
        trace.door_frame_top_stride_d1r_id, 178);
    ok &= expect_int("door_frame_top_native_bitmap_d1l",
        trace.door_frame_top_native_bitmap_d1l, 2111);
    ok &= expect_int("door_frame_top_native_bitmap_d1r",
        trace.door_frame_top_native_bitmap_d1r, 2110);
    ok &= expect_int("door_frame_top_native_bitmap_d1lcr",
        trace.door_frame_top_native_bitmap_d1lcr, 2112);

    ok &= expect_int("f0100_blit_transparency_color",
        trace.f0100_blit_transparency_color, 10);
    ok &= expect_int("c10_transparent_blit", trace.c10_transparent_blit, 1);

    ok &= expect_int("f0122_d1l_door_front_line",
        trace.f0122_d1l_door_front_line, 7494);
    ok &= expect_int("f0122_d1l_door_frame_top_legacy_line",
        trace.f0122_d1l_door_frame_top_legacy_line, 7496);
    ok &= expect_int("f0122_d1l_door_frame_top_f20e_line",
        trace.f0122_d1l_door_frame_top_f20e_line, 7500);
    ok &= expect_int("f0122_d1l_door_frame_top_i34e_line",
        trace.f0122_d1l_door_frame_top_i34e_line, 7503);
    ok &= expect_int("f0123_d1r_door_front_line",
        trace.f0123_d1r_door_front_line, 7662);
    ok &= expect_int("f0123_d1r_door_frame_top_legacy_line",
        trace.f0123_d1r_door_frame_top_legacy_line, 7664);
    ok &= expect_int("f0123_d1r_door_frame_top_f20e_line",
        trace.f0123_d1r_door_frame_top_f20e_line, 7668);
    ok &= expect_int("f0123_d1r_door_frame_top_i34e_line",
        trace.f0123_d1r_door_frame_top_i34e_line, 7671);
    ok &= expect_int("f0128_dispatch_line", trace.f0128_dispatch_line, 8525);

    ok &= expect_int("d1l_door_frame_top_zone",
        trace.d1l_door_frame_top_zone, expected_d1l_zone);
    ok &= expect_int("d1r_door_frame_top_zone",
        trace.d1r_door_frame_top_zone, expected_d1r_zone);

    ok &= expect_int("g0695_door_panel_bitmap",
        trace.g0695_door_panel_bitmap, 695);
    ok &= expect_int("g0185_door_frames_d1l",
        trace.g0185_door_frames_d1l, 185);
    ok &= expect_int("g0187_door_frames_d1r",
        trace.g0187_door_frames_d1r, 187);
    ok &= expect_int("c2_view_door_ornament_d1lcr",
        trace.c2_view_door_ornament_d1lcr, 2);

    ok &= expect_int("d1l_band_in_viewport", trace.d1l_band_in_viewport, 1);
    ok &= expect_int("d1r_band_in_viewport", trace.d1r_band_in_viewport, 1);
    ok &= expect_int("d1l_band_above_door_panel",
        trace.d1l_band_inside_door_panel_band, 1);
    ok &= expect_int("d1r_band_above_door_panel",
        trace.d1r_band_inside_door_panel_band, 1);

    ok &= expect_int("band_strip_destination_x_d1l",
        trace.band_strip_destination_x_d1l, 16);
    ok &= expect_int("band_strip_destination_x_d1r",
        trace.band_strip_destination_x_d1r, 208);
    ok &= expect_int("band_strip_destination_y",
        trace.band_strip_destination_y, 14);
    ok &= expect_int("band_strip_byte_width_d1l",
        trace.band_strip_byte_width_d1l, 64);
    ok &= expect_int("band_strip_byte_width_d1r",
        trace.band_strip_byte_width_d1r, 64);

    printf("traceCase=%s side=%d target=%d d1lZone=%d d1rZone=%d "
           "g2111=%d g2110=%d g0704=%d\n",
           label,
           side,
           target_media,
           trace.d1l_door_frame_top_zone,
           trace.d1r_door_frame_top_zone,
           trace.d1l_i34e_route_uses_g2111,
           trace.d1r_i34e_route_uses_g2110,
           trace.d1l_legacy_route_uses_g0704);
    return ok;
}

static int run_trace_invariants(void)
{
    int ok = 1;

    ok &= run_trace_case(
        "d1lLegacy",
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
        D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34);
    ok &= run_trace_case(
        "d1lF20E",
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
        D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C728_ZONE_D1L_F20E_PC34,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C730_ZONE_D1R_F20E_PC34);
    ok &= run_trace_case(
        "d1lI34E",
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
        D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C732_ZONE_D1L_I34E_PC34,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C734_ZONE_D1R_I34E_PC34);
    ok &= run_trace_case(
        "d1rLegacy",
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
        D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34);
    ok &= run_trace_case(
        "d1rF20E",
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
        D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C728_ZONE_D1L_F20E_PC34,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C730_ZONE_D1R_F20E_PC34);
    ok &= run_trace_case(
        "d1rI34E",
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34,
        D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C732_ZONE_D1L_I34E_PC34,
        DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C734_ZONE_D1R_I34E_PC34);

    return ok;
}

static int run_invalid_trace_invariants(void)
{
    int ok = 1;

    /* Invalid target_media. */
    {
        DM1_V1_D1L_D1RDoorFrameTopEdgeTracePc34 trace;
        ok &= expect_int("invalid_target_rc",
            dm1_v1_viewport_d1l_d1r_door_frame_top_edge_trace_pc34(
                DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34, 99, &trace),
            0);
    }

    /* Invalid side. */
    {
        DM1_V1_D1L_D1RDoorFrameTopEdgeTracePc34 trace;
        ok &= expect_int("invalid_side_rc",
            dm1_v1_viewport_d1l_d1r_door_frame_top_edge_trace_pc34(
                99, D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY, &trace),
            0);
    }

    /* Null output. */
    ok &= expect_int("null_output_rc",
        dm1_v1_viewport_d1l_d1r_door_frame_top_edge_trace_pc34(
            DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34,
            D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY, NULL),
        0);

    return ok;
}

static int run_source_evidence_invariants(void)
{
    const char *evidence =
        dm1_v1_viewport_d1l_d1r_door_frame_top_edge_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("src_g0176", evidence,
        "G0176_auc_Graphic558_Frame_DoorFrameTop_D1L");
    ok &= expect_contains("src_g0178", evidence,
        "G0178_auc_Graphic558_Frame_DoorFrameTop_D1R");
    ok &= expect_contains("src_d1l_stride_values", evidence,
        "{ 0, 31, 14, 17, 64, 4, 16, 0 }");
    ok &= expect_contains("src_d1r_stride_values", evidence,
        "{ 192, 223, 14, 17, 64, 4, 16, 0 }");
    ok &= expect_contains("src_f0122_d1l_legacy", evidence, "DUNVIEW.C:7496");
    ok &= expect_contains("src_f0122_d1l_f20e", evidence, "DUNVIEW.C:7500");
    ok &= expect_contains("src_f0122_d1l_i34e", evidence, "DUNVIEW.C:7503");
    ok &= expect_contains("src_f0123_d1r_legacy", evidence, "DUNVIEW.C:7664");
    ok &= expect_contains("src_f0123_d1r_f20e", evidence, "DUNVIEW.C:7668");
    ok &= expect_contains("src_f0123_d1r_i34e", evidence, "DUNVIEW.C:7671");
    ok &= expect_contains("src_f0122_start", evidence, "DUNVIEW.C:7391");
    ok &= expect_contains("src_f0123_start", evidence, "DUNVIEW.C:7559");
    ok &= expect_contains("src_f0128_dispatch", evidence, "DUNVIEW.C:8525");
    ok &= expect_contains("src_g0704", evidence, "G0704_puc_Bitmap_WallSet_DoorFrameTop_D1LCR");
    ok &= expect_contains("src_g2111", evidence, "G2111_DoorFrameTopD1L");
    ok &= expect_contains("src_g2110", evidence, "G2110_DoorFrameTopD1R");
    ok &= expect_contains("src_c728", evidence,
        "C728_ZONE_DOOR_FRAME_TOP_D1L = 728");
    ok &= expect_contains("src_c730", evidence,
        "C730_ZONE_DOOR_FRAME_TOP_D1R = 730");
    ok &= expect_contains("src_c732", evidence,
        "C732_ZONE_DOOR_FRAME_TOP_D1L = 732");
    ok &= expect_contains("src_c734", evidence,
        "C734_ZONE_DOOR_FRAME_TOP_D1R = 734");
    ok &= expect_contains("src_m607_m608", evidence, "M607_VIEW_SQUARE_D1L");
    ok &= expect_contains("src_c10", evidence, "C10_COLOR_FLESH");
    ok &= expect_contains("src_non_overlap_marker", evidence,
        "pass794-d1l-d1r-door-frame-top-edge-source-lock");
    ok &= expect_contains("src_no_original_parity", evidence,
        "no real-asset or original-DOS pixel parity claim");
    ok &= expect_contains("src_bridge", evidence,
        "door-frame-top edge stride, zone, and dispatch");
    ok &= expect_contains("src_m075_byte_count", evidence,
        "M075_BITMAP_BYTE_COUNT(96, 88)");
    ok &= expect_contains("src_g2112_d1lcr", evidence,
        "G2112_DoorFrameTopD1LCR");
    ok &= expect_contains("src_c2_view_door_ornament", evidence,
        "C2_VIEW_DOOR_ORNAMENT_D1LCR");
    ok &= expect_contains("src_f0122_door_front_body", evidence,
        "DUNVIEW.C:7494");
    ok &= expect_contains("src_f0123_door_front_body", evidence,
        "DUNVIEW.C:7662");
    ok &= expect_contains("src_f0128_d1r_caller", evidence,
        "DUNVIEW.C:8529");

    return ok;
}

int main(int argc, char **argv)
{
    int ok = 1;
    int lib_rc;
    uint32_t hash_before = 0;
    uint32_t hash_after = 0;
    const DM1_V1_D1L_D1RDoorFrameTopEdgeSelfTestResultPc34 *result;

    /* Optional --data-dir argument is accepted for parity with other
     * probes but intentionally ignored — this gate is asset-free. */
    if (argc > 1 && argv[1] && argv[1][0] != '\0') {
        printf("dataDirIgnored=%s\n", argv[1]);
    }

    printf("probe=firestaff_dm1_v1_viewport_d1l_d1r_door_frame_top_edge_gate_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("sourceEvidence="
           "DUNVIEW.C:607-609,7391,7494,7496,7500,7503,7559,7662,7664,"
           "7668,7671,8525,8529\n");

    lib_rc = run_dm1_v1_viewport_d1l_d1r_door_frame_top_edge_self_test();
    ok &= expect_int("library_self_test_rc", lib_rc, 0);
    result = dm1_v1_viewport_d1l_d1r_door_frame_top_edge_last_self_test_result_pc34();
    if (result) hash_before = result->deterministic_hash;

    (void)run_dm1_v1_viewport_d1l_d1r_door_frame_top_edge_self_test();
    result = dm1_v1_viewport_d1l_d1r_door_frame_top_edge_last_self_test_result_pc34();
    if (result) hash_after = result->deterministic_hash;

    ok &= expect_int("deterministic_hash_stable",
        (int)(hash_before == hash_after &&
              hash_after == DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_HASH_PC34), 1);
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
