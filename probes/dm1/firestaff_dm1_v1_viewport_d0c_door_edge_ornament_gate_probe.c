/*
 * Focused DM1 V1 D0C door-edge ornament source-lock probe.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - DUNVIEW.C:603 G0172_auc_Graphic558_Frame_DoorFrame_D0C =
 *   {96,127,0,122,16,123,0,0}.
 * - DUNVIEW.C:2162/2181/2196 loads G2116_DoorFrameFrontD0C for
 *   modern PC wall-set media; legacy media use G0709 at 2156/2159.
 * - DUNVIEW.C F0127:8185-8236 C16_ELEMENT_DOOR_SIDE draws the D0C
 *   door-frame edge and optional thieves-eye hole, not the F0111
 *   door-panel body.
 * - DUNVIEW.C:8542 dispatches F0127 for the D0C square from F0128.
 *
 * Asset-free, contract-only, no original DOS pixel parity claim.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "firestaff/dm1/v1/viewport/d0c_door_edge_ornament_pc34_compat.h"

enum {
    D0C_DOOR_EDGE_TARGET_LEGACY = 0,
    D0C_DOOR_EDGE_TARGET_F20E = 1,
    D0C_DOOR_EDGE_TARGET_I34E = 2
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
    const DM1_V1_D0CDoorEdgeOrnamentSelfTestResultPc34 *result =
        dm1_v1_viewport_d0c_door_edge_ornament_last_self_test_result_pc34();
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
        (int)DM1_V1_D0C_DOOR_EDGE_ORNAMENT_HASH_PC34);
    ok &= expect_int("no_thieves_eye_legacy_count",
        result->no_thieves_eye_legacy_branch, 1);
    ok &= expect_int("no_thieves_eye_f20e_count",
        result->no_thieves_eye_f20e_branch, 1);
    ok &= expect_int("no_thieves_eye_i34e_count",
        result->no_thieves_eye_i34e_branch, 1);
    ok &= expect_int("thieves_eye_legacy_count",
        result->thieves_eye_legacy_branch, 1);
    ok &= expect_int("thieves_eye_f20e_count",
        result->thieves_eye_f20e_branch, 1);
    ok &= expect_int("thieves_eye_i34e_count",
        result->thieves_eye_i34e_branch, 1);
    ok &= expect_int("invalid_branch_count", result->invalid_branch, 1);
    ok &= expect_int("g0172_stride_checks", result->g0172_stride_checks, 6);
    ok &= expect_int("g2116_zone_checks", result->g2116_zone_checks, 6);
    ok &= expect_int("thieves_eye_zone_checks",
        result->thieves_eye_zone_checks, 6);
    ok &= expect_int("transparency_color_checks",
        result->transparency_color_checks, 6);
    ok &= expect_int("post_frame_f0112_checks",
        result->post_frame_f0112_checks, 6);
    ok &= expect_int("post_frame_f0115_checks",
        result->post_frame_f0115_checks, 6);
    ok &= expect_int("post_frame_f0113_checks",
        result->post_frame_f0113_checks, 6);
    ok &= expect_int("non_overlap_checks", result->non_overlap_checks, 16);
    ok &= expect_int("bitmap_strip_byte_width_checks",
        result->bitmap_strip_byte_width_checks, 6);

    printf("resultInvariants assertions=%d hash=0x%08x "
           "g0172=%d g2116=%d thievesEye=%d nonOverlap=%d\n",
           result->assertions,
           (unsigned)result->deterministic_hash,
           result->g0172_stride_checks,
           result->g2116_zone_checks,
           result->thieves_eye_zone_checks,
           result->non_overlap_checks);
    return ok;
}

static int run_trace_case(
    const char *label,
    int has_thieves_eye,
    int target_media,
    int expected_branch,
    int expected_frame_zone,
    int expected_thieves_eye_zone,
    int expected_hole_bitmap,
    int expected_g2116,
    int expected_g0709)
{
    DM1_V1_D0CDoorEdgeOrnamentTracePc34 trace;
    int ok = 1;

    ok &= expect_int("trace_ok",
        dm1_v1_viewport_d0c_door_edge_ornament_trace_pc34(
            has_thieves_eye, target_media, &trace), 1);
    ok &= expect_int("trace_branch", trace.branch, expected_branch);
    ok &= expect_int("trace_has_thieves_eye",
        trace.has_thieves_eye, has_thieves_eye ? 1 : 0);
    ok &= expect_int("trace_target_media", trace.target_media, target_media);

    ok &= expect_int("framebuffer_width", trace.framebuffer_width, 320);
    ok &= expect_int("framebuffer_height", trace.framebuffer_height, 200);
    ok &= expect_int("viewport_width", trace.viewport_width, 224);
    ok &= expect_int("viewport_height", trace.viewport_height, 136);

    ok &= expect_int("g0172_left", trace.g0172_left_x, 96);
    ok &= expect_int("g0172_right", trace.g0172_right_x, 127);
    ok &= expect_int("g0172_top", trace.g0172_top_y, 0);
    ok &= expect_int("g0172_bottom", trace.g0172_bottom_y, 122);
    ok &= expect_int("g0172_byte_width", trace.g0172_byte_width, 16);
    ok &= expect_int("g0172_height", trace.g0172_height, 123);
    ok &= expect_int("g0172_strides_are_16x123",
        trace.g0172_strides_are_16x123, 1);

    ok &= expect_int("door_frame_zone",
        trace.g2116_door_frame_zone, expected_frame_zone);
    ok &= expect_int("thieves_eye_zone",
        trace.thieves_eye_zone, expected_thieves_eye_zone);
    ok &= expect_int("thieves_eye_hole_native_bitmap",
        trace.thieves_eye_hole_native_bitmap, expected_hole_bitmap);
    ok &= expect_int("g2116_used_for_modern_i34e",
        trace.g2116_used_for_modern_i34e, expected_g2116);
    ok &= expect_int("g0709_used_for_legacy",
        trace.g0709_used_for_legacy, expected_g0709);

    ok &= expect_int("c09_gold_hole_blit", trace.c09_gold_hole_blit, 1);
    ok &= expect_int("thieves_eye_color", trace.thieves_eye_color, 9);
    ok &= expect_int("c10_transparent_blit", trace.c10_transparent_blit, 1);
    ok &= expect_int("frame_transparency_color",
        trace.frame_transparency_color, 10);

    ok &= expect_int("f0127_line_start", trace.f0127_line_start, 8164);
    ok &= expect_int("f0127_line_end", trace.f0127_line_end, 8311);
    ok &= expect_int("f0127_door_side_start",
        trace.f0127_door_side_branch_start, 8185);
    ok &= expect_int("f0127_door_side_end",
        trace.f0127_door_side_branch_end, 8236);
    ok &= expect_int("f0128_d0c_call_line",
        trace.f0128_d0c_call_line, 8542);
    ok &= expect_int("f0128_d0c_view_square",
        trace.f0128_d0c_view_square, 9);
    ok &= expect_int("f0128_d0c_cell_order",
        trace.f0128_d0c_cell_order, 0x0021);
    ok &= expect_int("f0127_dispatches_d0c_door_side",
        trace.f0127_dispatches_d0c_door_side, 1);

    ok &= expect_int("half_clip_first_byte_width",
        trace.half_clip_first_byte_width, 48);
    ok &= expect_int("half_clip_second_byte_width",
        trace.half_clip_second_byte_width, 16);
    ok &= expect_int("half_clip_first_height",
        trace.half_clip_first_height, 95);
    ok &= expect_int("half_clip_second_height",
        trace.half_clip_second_height, 123);
    ok &= expect_int("frame_buffer_strip_byte_width",
        trace.frame_buffer_strip_byte_width, 16);
    ok &= expect_int("frame_buffer_strip_destination_x",
        trace.frame_buffer_strip_destination_x, 96);
    ok &= expect_int("frame_buffer_strip_destination_y",
        trace.frame_buffer_strip_destination_y, 0);

    printf("traceCase=%s branch=%d frameZone=%d thievesEyeZone=%d "
           "holeBitmap=%d g2116=%d g0709=%d\n",
           label,
           trace.branch,
           trace.g2116_door_frame_zone,
           trace.thieves_eye_zone,
           trace.thieves_eye_hole_native_bitmap,
           trace.g2116_used_for_modern_i34e,
           trace.g0709_used_for_legacy);
    return ok;
}

static int run_trace_invariants(void)
{
    int ok = 1;

    ok &= run_trace_case(
        "noThievesEyeLegacy",
        0,
        D0C_DOOR_EDGE_TARGET_LEGACY,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_LEGACY_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C724_ZONE_DOOR_FRAME_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C732_ZONE_THIEVES_EYE_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C041_GRAPHIC_HOLE_PC34,
        0,
        1);
    ok &= run_trace_case(
        "noThievesEyeF20E",
        0,
        D0C_DOOR_EDGE_TARGET_F20E,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_F20E_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C724_ZONE_DOOR_FRAME_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C732_ZONE_THIEVES_EYE_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M711_NEGGRAPHIC_HOLE_PC34,
        0,
        0);
    ok &= run_trace_case(
        "noThievesEyeI34E",
        0,
        D0C_DOOR_EDGE_TARGET_I34E,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_I34E_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C728_ZONE_DOOR_FRAME_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C736_ZONE_THIEVES_EYE_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M711_NEGGRAPHIC_HOLE_PC34,
        1,
        0);
    ok &= run_trace_case(
        "thievesEyeLegacy",
        1,
        D0C_DOOR_EDGE_TARGET_LEGACY,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_LEGACY_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C724_ZONE_DOOR_FRAME_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C732_ZONE_THIEVES_EYE_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C041_GRAPHIC_HOLE_PC34,
        0,
        1);
    ok &= run_trace_case(
        "thievesEyeF20E",
        1,
        D0C_DOOR_EDGE_TARGET_F20E,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_F20E_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C724_ZONE_DOOR_FRAME_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C732_ZONE_THIEVES_EYE_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M711_NEGGRAPHIC_HOLE_PC34,
        0,
        0);
    ok &= run_trace_case(
        "thievesEyeI34E",
        1,
        D0C_DOOR_EDGE_TARGET_I34E,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_I34E_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C728_ZONE_DOOR_FRAME_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C736_ZONE_THIEVES_EYE_PC34,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M711_NEGGRAPHIC_HOLE_PC34,
        1,
        0);

    return ok;
}

static int run_invalid_trace_invariant(void)
{
    DM1_V1_D0CDoorEdgeOrnamentTracePc34 trace;
    int ok = 1;

    ok &= expect_int("invalid_trace_ok",
        dm1_v1_viewport_d0c_door_edge_ornament_trace_pc34(1, 99, &trace),
        1);
    ok &= expect_int("invalid_branch",
        trace.branch,
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_BRANCH_INVALID_PC34);
    ok &= expect_int("invalid_door_frame_zone",
        trace.g2116_door_frame_zone, -1);
    ok &= expect_int("invalid_thieves_eye_zone", trace.thieves_eye_zone, -1);
    ok &= expect_int("invalid_hole_bitmap",
        trace.thieves_eye_hole_native_bitmap, 0);
    printf("invalidTrace branch=%d frameZone=%d thievesEyeZone=%d\n",
           trace.branch,
           trace.g2116_door_frame_zone,
           trace.thieves_eye_zone);
    return ok;
}

static int run_source_evidence_invariants(void)
{
    const char *evidence =
        dm1_v1_viewport_d0c_door_edge_ornament_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("src_f0127", evidence, "DUNVIEW.C:8164-8311");
    ok &= expect_contains("src_door_side", evidence, "8185-8236");
    ok &= expect_contains("src_g0172", evidence,
        "G0172_auc_Graphic558_Frame_DoorFrame_D0C");
    ok &= expect_contains("src_g0172_values", evidence,
        "96, 127, 0, 122, 16, 123, 0, 0");
    ok &= expect_contains("src_g2116", evidence, "G2116_DoorFrameFrontD0C");
    ok &= expect_contains("src_g0709", evidence,
        "G0709_puc_Bitmap_WallSet_DoorFrameFront");
    ok &= expect_contains("src_thieves_eye_hole", evidence,
        "C041_GRAPHIC_HOLE_IN_WALL");
    ok &= expect_contains("src_m711", evidence,
        "M711_NEGGRAPHIC_HOLE_IN_WALL");
    ok &= expect_contains("src_c09", evidence, "C09_COLOR_GOLD");
    ok &= expect_contains("src_c10", evidence, "C10_COLOR_FLESH");
    ok &= expect_contains("src_f0112", evidence, "F0112");
    ok &= expect_contains("src_f0115", evidence, "F0115");
    ok &= expect_contains("src_f0113", evidence, "F0113");
    ok &= expect_contains("src_non_overlap_marker", evidence,
        "pass792-d0c-door-edge-ornament-source-lock");
    ok &= expect_contains("src_no_original_parity", evidence,
        "no real-asset or original-DOS pixel parity claim");
    ok &= expect_contains("src_door_frame_not_panel", evidence,
        "door-edge-ornament is the door frame border");

    return ok;
}

int main(void)
{
    int ok = 1;
    int lib_rc;
    uint32_t hash_before = 0;
    uint32_t hash_after = 0;
    const DM1_V1_D0CDoorEdgeOrnamentSelfTestResultPc34 *result;

    printf("probe=firestaff_dm1_v1_viewport_d0c_door_edge_ornament_gate_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("sourceEvidence="
           "DUNVIEW.C:603,2162,2181,2196,8185-8236,8542\n");

    lib_rc = run_dm1_v1_viewport_d0c_door_edge_ornament_self_test();
    ok &= expect_int("library_self_test_rc", lib_rc, 0);
    result = dm1_v1_viewport_d0c_door_edge_ornament_last_self_test_result_pc34();
    if (result) hash_before = result->deterministic_hash;

    (void)run_dm1_v1_viewport_d0c_door_edge_ornament_self_test();
    result = dm1_v1_viewport_d0c_door_edge_ornament_last_self_test_result_pc34();
    if (result) hash_after = result->deterministic_hash;

    ok &= expect_int("deterministic_hash_stable",
        (int)(hash_before == hash_after &&
              hash_after == DM1_V1_D0C_DOOR_EDGE_ORNAMENT_HASH_PC34), 1);
    ok &= run_result_invariants();
    ok &= run_trace_invariants();
    ok &= run_invalid_trace_invariant();
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
