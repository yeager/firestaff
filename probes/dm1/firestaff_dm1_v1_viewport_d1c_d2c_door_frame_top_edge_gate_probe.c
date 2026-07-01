/*
 * Focused DM1 V1 D1C/D2C door-frame-top edge source-lock probe.
 *
 * ReDMCSB source lock:
 * - DUNVIEW.C:605/608 define G0174/G0177 center door-frame-top strides.
 * - DUNVIEW.C:7317/7323/7328 route D2C legacy/F20E/I34E.
 * - DUNVIEW.C:7877/7882/7886 route D1C legacy/F20E/I34E.
 * - DUNVIEW.C:8521/8533 are the F0128 caller sites.
 *
 * Asset-free, contract-only, no original DOS pixel parity claim.
 */

#include "firestaff/dm1/v1/viewport/d1c_d2c_door_frame_top_edge_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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
    int square,
    int target,
    int left_x,
    int right_x,
    int top_y,
    int bottom_y,
    int stride_id,
    int native_id,
    int zone_id)
{
    DM1_V1_D1C_D2CDoorFrameTopEdgeTracePc34 trace;
    int ok = 1;

    ok &= expect_int("trace_ok",
        dm1_v1_viewport_d1c_d2c_door_frame_top_edge_trace_pc34(
            square, target, &trace), 1);
    ok &= expect_int("trace_square", trace.square, square);
    ok &= expect_int("trace_target", trace.target_media, target);
    ok &= expect_int("left_x", trace.left_x, left_x);
    ok &= expect_int("right_x", trace.right_x, right_x);
    ok &= expect_int("top_y", trace.top_y, top_y);
    ok &= expect_int("bottom_y", trace.bottom_y, bottom_y);
    ok &= expect_int("stride_id", trace.legacy_stride_id, stride_id);
    ok &= expect_int("native_id", trace.native_bitmap_id, native_id);
    ok &= expect_int("zone_id", trace.zone_id, zone_id);
    ok &= expect_int("pass1_cell_order", trace.pass1_cell_order, 0x0218);
    ok &= expect_int("inside_viewport", trace.band_inside_viewport, 1);
    ok &= expect_int("no_side_shift", trace.center_has_no_side_shift, 1);

    printf("%s square=%d target=%d rect=%d,%d-%d,%d stride=G%04d "
           "native=G%d zone=%d\n",
           label,
           trace.square,
           trace.target_media,
           trace.left_x,
           trace.top_y,
           trace.right_x,
           trace.bottom_y,
           trace.legacy_stride_id,
           trace.native_bitmap_id,
           trace.zone_id);
    return ok;
}

int main(void)
{
    const DM1_V1_D1C_D2CDoorFrameTopEdgeSelfTestResultPc34 *result;
    const char *evidence;
    int ok = 1;

    ok &= expect_int("self_test",
        run_dm1_v1_viewport_d1c_d2c_door_frame_top_edge_self_test(), 0);
    result =
        dm1_v1_viewport_d1c_d2c_door_frame_top_edge_last_self_test_result_pc34();
    ok &= expect_int("result_present", result != 0, 1);
    if (!result) return 1;

    ok &= expect_int("failures", result->failures, 0);
    ok &= expect_int("d2c_legacy", result->d2c_legacy_count, 1);
    ok &= expect_int("d2c_f20e", result->d2c_f20e_count, 1);
    ok &= expect_int("d2c_i34e", result->d2c_i34e_count, 1);
    ok &= expect_int("d1c_legacy", result->d1c_legacy_count, 1);
    ok &= expect_int("d1c_f20e", result->d1c_f20e_count, 1);
    ok &= expect_int("d1c_i34e", result->d1c_i34e_count, 1);
    ok &= expect_int("invalid_count", result->invalid_count, 3);
    ok &= expect_int("stride_checks", result->stride_checks, 6);
    ok &= expect_int("zone_checks", result->zone_checks, 6);
    ok &= expect_int("dispatch_checks", result->dispatch_checks, 6);
    ok &= expect_int("order_checks", result->order_checks, 6);
    ok &= expect_int("bitmap_route_checks", result->bitmap_route_checks, 6);
    ok &= expect_int("viewport_band_checks", result->viewport_band_checks, 6);
    ok &= expect_int("deterministic_hash",
        (int)result->deterministic_hash,
        (int)DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_HASH_PC34);

    ok &= check_trace("D2C legacy",
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34,
        64, 159, 22, 24, 174, 2115, 10);
    ok &= check_trace("D2C F20E",
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34,
        64, 159, 22, 24, 174, 2115, 726);
    ok &= check_trace("D2C I34E",
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34,
        64, 159, 22, 24, 174, 2115, 730);
    ok &= check_trace("D1C legacy",
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D1C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34,
        48, 175, 14, 17, 177, 2112, 10);
    ok &= check_trace("D1C F20E",
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D1C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34,
        48, 175, 14, 17, 177, 2112, 729);
    ok &= check_trace("D1C I34E",
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D1C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34,
        48, 175, 14, 17, 177, 2112, 733);

    evidence = dm1_v1_viewport_d1c_d2c_door_frame_top_edge_source_evidence_pc34();
    ok &= expect_contains("evidence_d2c", evidence, "G0174");
    ok &= expect_contains("evidence_d1c", evidence, "G0177");
    ok &= expect_contains("evidence_non_claim", evidence, "no real-asset");

    printf("%s firestaff_dm1_v1_viewport_d1c_d2c_door_frame_top_edge_gate_probe "
           "assertions=%d hash=0x%08X\n",
           ok ? "PASS" : "FAIL",
           result->assertions,
           (unsigned)result->deterministic_hash);
    return ok ? 0 : 1;
}
