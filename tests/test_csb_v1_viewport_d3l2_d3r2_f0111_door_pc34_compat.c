#include "csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_BOTH =
    "ReDMCSB DUNVIEW.C:6268-6274 F0676 and 6336-6341 F0677; "
    "DEFS.H:2088,2610-2611,2668-2675,2750-2751,2789,4250-4251,5456; "
    "CSB Viewport.cpp:1813-1820,2267/2271,2281,2386/2387,2568,2596-2616";
static const char *A_D3L2 =
    "ReDMCSB DUNVIEW.C:6268-6274 F0676; "
    "DEFS.H:2088,2610,2669,2672,2750,2789,4250,5456; "
    "CSB Viewport.cpp:1813-1820,2267,2281,2386,2568,2596-2616";
static const char *A_D3R2 =
    "ReDMCSB DUNVIEW.C:6336-6341 F0677; "
    "DEFS.H:2088,2611,2668,2675,2751,2789,4251,5456; "
    "CSB Viewport.cpp:1813-1820,2271,2281,2387,2568,2596-2616";
static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4334 F0111 plus F0676/F0677 door-front calls; "
    "DEFS.H:2088,2610-2611,2668-2675,2789,4250-4251,5456; "
    "CSB Viewport.cpp:1813-1820,2281,2386/2387,2568,2596-2616";

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

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    const int got = haystack && needle && strstr(haystack, needle) != NULL;
    return expect_int(label, got, 1, anchor);
}

static int test_specs_and_f0108_routes(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34);

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d3l2_d3r2_f0111_door_count_pc34(),
                     2, A_BOTH);
    ok &= expect_int("spec.index0.d3l2",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_at_pc34(0) == d3l2,
                     1, A_D3L2);
    ok &= expect_int("spec.index2.null",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_at_pc34(2) == NULL,
                     1, A_BOTH);
    ok &= expect_int("d3l2.present", d3l2 != NULL, 1, A_D3L2);
    ok &= expect_int("d3r2.present", d3r2 != NULL, 1, A_D3R2);
    ok &= expect_int("d3l2.element", d3l2 ? d3l2->element_door_front : -1,
                     17, A_D3L2);
    ok &= expect_int("d3r2.element", d3r2 ? d3r2->element_door_front : -1,
                     17, A_D3R2);
    ok &= expect_int("d3l2.f0108.floor_view",
                     d3l2 ? d3l2->floor_view : -1, 0, A_D3L2);
    ok &= expect_contains("d3l2.f0108.ordinal",
                          d3l2 ? d3l2->floor_ornament_ordinal_slot : NULL,
                          "M552_FRONT_WALL_ORNAMENT_ORDINAL", A_D3L2);
    ok &= expect_int("d3r2.f0108.floor_view",
                     d3r2 ? d3r2->floor_view : -1, 1, A_D3R2);
    ok &= expect_contains("d3r2.f0108.ordinal",
                          d3r2 ? d3r2->floor_ornament_ordinal_slot : NULL,
                          "M558_FLOOR_ORNAMENT_ORDINAL", A_D3R2);

    return ok;
}

static int test_f0115_and_f0111_dispatch(void)
{
    int ok = 1;
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 left_result;
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 right_result;
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34);

    ok &= expect_int("trace.d3l2",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
                         d3l2, 2, &left_result),
                     0, A_D3L2);
    ok &= expect_int("trace.d3r2",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
                         d3r2, 2, &right_result),
                     0, A_D3R2);
    ok &= expect_int("d3l2.pass1.called", left_result.f0115_pass1_called,
                     1, A_D3L2);
    ok &= expect_int("d3l2.pass1.square", left_result.f0115_pass1_view_square,
                     14, A_D3L2);
    ok &= expect_int("d3l2.pass1.order", left_result.f0115_pass1_order,
                     0x0218, A_D3L2);
    ok &= expect_int("d3r2.pass1.called", right_result.f0115_pass1_called,
                     1, A_D3R2);
    ok &= expect_int("d3r2.pass1.square", right_result.f0115_pass1_view_square,
                     15, A_D3R2);
    ok &= expect_int("d3r2.pass1.order", right_result.f0115_pass1_order,
                     0x0128, A_D3R2);
    ok &= expect_int("d3l2.f0111.called", left_result.f0111_called, 1,
                     A_D3L2);
    ok &= expect_int("d3r2.f0111.called", right_result.f0111_called, 1,
                     A_D3R2);
    ok &= expect_int("d3l2.f0111.native_index_family",
                     left_result.f0111_native_bitmap_index_family, 693, A_F0111);
    ok &= expect_int("d3r2.f0111.native_index_family",
                     right_result.f0111_native_bitmap_index_family, 693, A_F0111);
    ok &= expect_int("d3l2.f0111.ornament",
                     left_result.f0111_door_ornament_view, 0, A_F0111);
    ok &= expect_int("d3r2.f0111.ornament",
                     right_result.f0111_door_ornament_view, 0, A_F0111);
    ok &= expect_int("d3l2.f0111.zone", left_result.f0111_zone, 3700,
                     A_D3L2);
    ok &= expect_int("d3r2.f0111.zone", right_result.f0111_zone, 3710,
                     A_D3R2);

    return ok;
}

static int test_pass2_c10_frame_and_no_f0107(void)
{
    int ok = 1;
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 left_result;
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 right_result;
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34);

    ok &= expect_int("trace.d3l2.again",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
                         d3l2, 1, &left_result),
                     0, A_D3L2);
    ok &= expect_int("trace.d3r2.again",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
                         d3r2, 1, &right_result),
                     0, A_D3R2);
    ok &= expect_int("d3l2.pass2.before_dispatch",
                     left_result.f0115_pass2_order_set_before_dispatch, 1,
                     A_D3L2);
    ok &= expect_int("d3l2.pass2.order", left_result.f0115_pass2_order,
                     0x0349, A_D3L2);
    ok &= expect_int("d3r2.pass2.before_dispatch",
                     right_result.f0115_pass2_order_set_before_dispatch, 1,
                     A_D3R2);
    ok &= expect_int("d3r2.pass2.order", right_result.f0115_pass2_order,
                     0x0439, A_D3R2);
    ok &= expect_int("d3l2.f0128.reached",
                     left_result.f0128_dispatch_reached_after_pass2, 1,
                     A_D3L2);
    ok &= expect_int("d3r2.f0128.reached",
                     right_result.f0128_dispatch_reached_after_pass2, 1,
                     A_D3R2);
    ok &= expect_int("d3l2.c10", left_result.f0111_transparent_color, 10,
                     A_F0111);
    ok &= expect_int("d3r2.c10", right_result.f0111_transparent_color, 10,
                     A_F0111);
    ok &= expect_int("d3l2.native.fetch.f0489",
                     left_result.native_bitmap_fetches_via_f0489, 1, A_F0111);
    ok &= expect_int("d3r2.native.fetch.f0489",
                     right_result.native_bitmap_fetches_via_f0489, 1, A_F0111);
    ok &= expect_int("d3l2.resolved.native.index",
                     left_result.resolved_native_bitmap_index, 694, A_F0111);
    ok &= expect_int("d3r2.resolved.native.index",
                     right_result.resolved_native_bitmap_index, 694, A_F0111);
    ok &= expect_int("d3l2.frame.byte_width",
                     left_result.preserved_frame_byte_width, 48, A_F0111);
    ok &= expect_int("d3r2.frame.byte_width",
                     right_result.preserved_frame_byte_width, 48, A_F0111);
    ok &= expect_int("d3l2.no_f0107", left_result.f0107_called, 0, A_D3L2);
    ok &= expect_int("d3r2.no_f0107", right_result.f0107_called, 0, A_D3R2);

    return ok;
}

static int test_lineage_pixel_and_evidence(void)
{
    int ok = 1;
    uint8_t source[48 * 2];
    uint8_t destination[48 * 2];
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 result;
    const char *e =
        csb_v1_viewport_d3l2_d3r2_f0111_door_source_evidence_pc34();
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2 =
        csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
            CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);

    memset(source, 10, sizeof(source));
    memset(destination, 0xee, sizeof(destination));
    source[0] = 1;
    source[47] = 2;
    source[48] = 3;
    source[95] = 4;

    ok &= expect_int("lineage.trace",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
                         d3l2, 0, &result),
                     0, A_D3L2);
    ok &= expect_int("lineage.pwall.parity",
                     result.l0201_pwall_parity_preserved, 1, A_BOTH);
    ok &= expect_int("lineage.pwall.left",
                     result.lineage_pwall_left_index, 5, A_BOTH);
    ok &= expect_int("lineage.pwall.right",
                     result.lineage_pwall_right_index, 6, A_BOTH);
    ok &= expect_int("pixel.anchor.ready", result.pixel_anchor_ready, 1,
                     A_F0111);
    ok &= expect_int("pixel.blit.copied",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_apply_c10_blit_pc34(
                         d3l2, source, 48, destination, 48, 48, 2),
                     4, A_F0111);
    ok &= expect_int("pixel.blit.first", destination[0], 1, A_F0111);
    ok &= expect_int("pixel.blit.transparent", destination[1], 0xee, A_F0111);
    ok &= expect_int("pixel.blit.last", destination[95], 4, A_F0111);
    ok &= expect_int("pixel.blit.reject.width",
                     csb_v1_viewport_d3l2_d3r2_f0111_door_apply_c10_blit_pc34(
                         d3l2, source, 48, destination, 48, 49, 1),
                     -1, A_F0111);
    ok &= expect_contains("evidence.path", result.source_lock_evidence,
                          "DUNVIEW.C F0676/F0677 C17_ELEMENT_DOOR_FRONT path",
                          A_BOTH);
    ok &= expect_contains("evidence.f0111", result.source_lock_evidence,
                          "F0111_DUNGEONVIEW_DrawDoor", A_F0111);
    ok &= expect_contains("evidence.global.path", e,
                          "F0111_DUNGEONVIEW_DrawDoor", A_F0111);
    ok &= expect_contains("evidence.no_f0107_contract_source", e,
                          "M552_FRONT_WALL_ORNAMENT_ORDINAL", A_D3L2);
    ok &= expect_contains("evidence.d3r2_floor_ordinal", e,
                          "M558_FLOOR_ORNAMENT_ORDINAL", A_D3R2);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d3l2_d3r2_f0111_door_source_evidence_pc34());

    ok &= test_specs_and_f0108_routes();
    ok &= test_f0115_and_f0111_dispatch();
    ok &= test_pass2_c10_frame_and_no_f0107();
    ok &= test_lineage_pixel_and_evidence();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_35", g_assertions >= 35, 1,
                     A_BOTH);
    if (ok) {
        printf("PASS csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
