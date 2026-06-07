#include "csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_F0102 =
    "ReDMCSB DUNVIEW.C:3082-3093 F0102_DUNGEONVIEW_DrawDoorBitmap";
static const char *A_F0103 =
    "ReDMCSB DUNVIEW.C:3096-3108 F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally";
static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor";
static const char *A_STATE =
    "ReDMCSB DUNVIEW.C:4248,4297-4304 F0111 C0/C4/C5 door state branches";
static const char *A_HALF =
    "ReDMCSB DUNVIEW.C:4317-4325 F0111 horizontal half-zone shift";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2466 C15, 2791 C2, 3516 C4000";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1853-1862,1881-1888,1895,1899,1922,1926,2614-2620";

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

static int test_identity_and_constants(void)
{
    int ok = 1;
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2L2);
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2R2);

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_count_pc34(),
                     2, A_F0111);
    ok &= expect_int("at0.d2l2",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_at_pc34(0) == d2l2,
                     1, A_F0111);
    ok &= expect_int("at2.null",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_at_pc34(2) == NULL,
                     1, A_F0111);
    ok &= expect_int("d2l2.present", d2l2 != NULL, 1,
                     "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_int("d2r2.present", d2r2 != NULL, 1,
                     "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    ok &= expect_int("macro.c4000",
                     CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_C4000_HALF_ZONE_SHIFT,
                     0x4000, "ReDMCSB DEFS.H:3516");
    ok &= expect_int("macro.c15",
                     CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_C15_DESTROYED_MASK,
                     15, "ReDMCSB DEFS.H:2466");
    ok &= expect_int("macro.c2",
                     CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_C2_DOOR_ORNAMENT_D1LCR,
                     2, "ReDMCSB DEFS.H:2791");
    ok &= expect_int("d2l2.square", d2l2 ? d2l2->view_square : -1, 9,
                     "ReDMCSB DEFS.H:2605");
    ok &= expect_int("d2r2.square", d2r2 ? d2r2->view_square : -1, 10,
                     "ReDMCSB DEFS.H:2606");
    ok &= expect_int("d2l2.depth", d2l2 ? d2l2->draw_depth : -1, 2, A_F0111);
    ok &= expect_int("d2r2.depth", d2r2 ? d2r2->draw_depth : -1, 2, A_F0111);
    ok &= expect_int("d2l2.lateral", d2l2 ? d2l2->draw_lateral : 0, -2,
                     "ReDMCSB DUNVIEW.C:8503");
    ok &= expect_int("d2r2.lateral", d2r2 ? d2r2->draw_lateral : 0, 2,
                     "ReDMCSB DUNVIEW.C:8507");
    ok &= expect_int("d2l2.wall_zone", d2l2 ? d2l2->wall_zone : -1, 707,
                     "ReDMCSB DEFS.H:4047");
    ok &= expect_int("d2r2.wall_zone", d2r2 ? d2r2->wall_zone : -1, 708,
                     "ReDMCSB DEFS.H:4048");

    return ok;
}

static int test_panel_half_metadata(void)
{
    int ok = 1;
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2L2);
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2R2);

    ok &= expect_int("d2l2.front_right_half",
                     d2l2 ? d2l2->front_clipped_half_is_right : -1, 1, A_HALF);
    ok &= expect_int("d2l2.not_left_half",
                     d2l2 ? d2l2->front_clipped_half_is_left : -1, 0, A_HALF);
    ok &= expect_int("d2r2.front_left_half",
                     d2r2 ? d2r2->front_clipped_half_is_left : -1, 1, A_HALF);
    ok &= expect_int("d2r2.not_right_half",
                     d2r2 ? d2r2->front_clipped_half_is_right : -1, 0, A_HALF);
    ok &= expect_int("d2l2.source_half_x", d2l2 ? d2l2->source_half_x : -1,
                     24, A_HALF);
    ok &= expect_int("d2r2.source_half_x", d2r2 ? d2r2->source_half_x : -1,
                     0, A_HALF);
    ok &= expect_int("d2l2.half_width", d2l2 ? d2l2->half_clip_width : -1,
                     24, A_HALF);
    ok &= expect_int("d2r2.half_width", d2r2 ? d2r2->half_clip_width : -1,
                     24, A_HALF);
    ok &= expect_int("frame.native_width", d2l2 ? d2l2->native_bitmap_width : -1,
                     48, "ReDMCSB COORD.C:1550");
    ok &= expect_int("frame.native_height", d2l2 ? d2l2->native_bitmap_height : -1,
                     41, "ReDMCSB COORD.C:1550");
    ok &= expect_int("frame.clip_width", d2l2 ? d2l2->frame_clip_width : -1,
                     48, "ReDMCSB COORD.C:1556");
    ok &= expect_int("frame.clip_height", d2l2 ? d2l2->frame_clip_height : -1,
                     40, "ReDMCSB COORD.C:1556");
    ok &= expect_int("frame.x", d2l2 ? d2l2->frame_x : -1, 24,
                     "ReDMCSB COORD.C:1559");
    ok &= expect_int("frame.y", d2l2 ? d2l2->frame_y : -1, 28,
                     "ReDMCSB COORD.C:1559");
    ok &= expect_int("frame.metadata_present",
                     d2l2 ? d2l2->frame_metadata_present : -1, 1, A_F0102);
    ok &= expect_int("transparent.c10", d2l2 ? d2l2->transparent_color : -1,
                     10, "ReDMCSB DEFS.H:2088");

    return ok;
}

static int test_state_machine_and_zones(void)
{
    int ok = 1;
    CSB_V1_D2L2D2R2F0111FrontClippedTracePc34 trace;
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2L2);
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2R2);

    ok &= expect_int("trace.open.rc",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
                         d2l2, 0, 1, &trace),
                     0, A_STATE);
    ok &= expect_int("trace.open.branch", trace.branch,
                     CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_NONE, A_STATE);
    ok &= expect_int("trace.open.drawn", trace.door_drawn, 0, A_STATE);
    ok &= expect_int("trace.open.pass_count", trace.pass_count, 0, A_STATE);
    ok &= expect_int("trace.closed.rc",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
                         d2l2, 4, 1, &trace),
                     0, A_STATE);
    ok &= expect_int("trace.closed.branch", trace.branch,
                     CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_CLOSED, A_STATE);
    ok &= expect_int("trace.closed.pass_count", trace.pass_count, 1, A_STATE);
    ok &= expect_int("trace.closed.zone", trace.final_zone, 3700, A_STATE);
    ok &= expect_int("trace.closed.frame", trace.uses_closed_or_destroyed_frame,
                     1, A_STATE);
    ok &= expect_int("trace.destroyed.rc",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
                         d2l2, 5, 1, &trace),
                     0, A_STATE);
    ok &= expect_int("trace.destroyed.branch", trace.branch,
                     CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_DESTROYED,
                     A_STATE);
    ok &= expect_int("trace.destroyed.mask", trace.destroyed_mask_applied, 1,
                     "ReDMCSB DUNVIEW.C:4301-4304 and DEFS.H:2466");
    ok &= expect_int("trace.destroyed.zone", trace.final_zone, 3700, A_STATE);
    ok &= expect_int("trace.destroyed.selected_bitmap", trace.selected_bitmap_state,
                     5, A_STATE);
    ok &= expect_int("trace.state1.horizontal.rc",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
                         d2l2, 1, 1, &trace),
                     0, A_HALF);
    ok &= expect_int("trace.state1.branch", trace.branch,
                     CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_PARTLY_HORIZONTAL,
                     A_HALF);
    ok &= expect_int("trace.state1.pass_count", trace.pass_count, 2, A_HALF);
    ok &= expect_int("trace.state1.first_zone", trace.first_half_zone, 3707,
                     "ReDMCSB DUNVIEW.C:4322 C6 first half");
    ok &= expect_int("trace.state1.final_unmasked",
                     trace.final_zone_without_shift_mask, 3704, A_HALF);
    ok &= expect_int("trace.state1.final_zone", trace.final_zone, 20088, A_HALF);
    ok &= expect_int("trace.state1.c4000", trace.c4000_shift_applied, 1,
                     "ReDMCSB DEFS.H:3516");
    ok &= expect_int("trace.state1.shift_x", trace.half_zone_shift_x, 24,
                     "ReDMCSB DUNVIEW.C:4320");
    ok &= expect_int("trace.state2.horizontal.rc",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
                         d2r2, 2, 1, &trace),
                     0, A_HALF);
    ok &= expect_int("trace.state2.first_zone", trace.first_half_zone, 3708,
                     "ReDMCSB DUNVIEW.C:4322 C6 first half");
    ok &= expect_int("trace.state2.final_zone", trace.final_zone, 20089, A_HALF);
    ok &= expect_int("trace.state2.source_x", trace.source_x, 0, A_HALF);
    ok &= expect_int("trace.vertical.rc",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
                         d2l2, 1, 0, &trace),
                     0, A_HALF);
    ok &= expect_int("trace.vertical.branch", trace.branch,
                     CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_PARTLY_VERTICAL,
                     A_HALF);
    ok &= expect_int("trace.vertical.zone", trace.final_zone, 3701, A_HALF);
    ok &= expect_int("trace.vertical.no_c4000", trace.c4000_shift_applied, 0,
                     A_HALF);
    ok &= expect_int("trace.bad_state",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
                         d2l2, -1, 1, &trace),
                     -1, A_F0111);
    ok &= expect_int("trace.null_spec",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
                         NULL, 1, 1, &trace),
                     -1, A_F0111);

    return ok;
}

static int test_pass_constants_and_ornaments(void)
{
    int ok = 1;
    CSB_V1_D2L2D2R2F0111FrontClippedTracePc34 trace;
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2L2);
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2R2);

    ok &= expect_int("d2l2.pass1", d2l2 ? d2l2->pass1_order : -1, 0x28,
                     A_LINEAGE);
    ok &= expect_int("d2l2.pass2", d2l2 ? d2l2->pass2_order : -1, 0x39,
                     A_LINEAGE);
    ok &= expect_int("d2r2.pass1", d2r2 ? d2r2->pass1_order : -1, 0x18,
                     A_LINEAGE);
    ok &= expect_int("d2r2.pass2", d2r2 ? d2r2->pass2_order : -1, 0x49,
                     A_LINEAGE);
    ok &= expect_int("d2l2.ornament_view", d2l2 ? d2l2->door_ornament_view : -1,
                     2, "ReDMCSB DEFS.H:2791");
    ok &= expect_int("d2r2.ornament_view", d2r2 ? d2r2->door_ornament_view : -1,
                     2, "ReDMCSB DEFS.H:2791");
    ok &= expect_int("d2l2.destroyed_mask",
                     d2l2 ? d2l2->destroyed_mask_ornament : -1, 15,
                     "ReDMCSB DEFS.H:2466");
    ok &= expect_int("d2r2.destroyed_mask",
                     d2r2 ? d2r2->destroyed_mask_ornament : -1, 15,
                     "ReDMCSB DEFS.H:2466");
    ok &= expect_int("trace.d2l2.pass_orders.rc",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
                         d2l2, 1, 1, &trace),
                     0, A_LINEAGE);
    ok &= expect_int("trace.d2l2.pass1", trace.pass1_order, 0x28, A_LINEAGE);
    ok &= expect_int("trace.d2l2.pass2", trace.pass2_order, 0x39, A_LINEAGE);
    ok &= expect_int("trace.d2l2.ornament_view", trace.ornament_view, 2, A_DEFS);
    ok &= expect_int("trace.d2r2.pass_orders.rc",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
                         d2r2, 1, 1, &trace),
                     0, A_LINEAGE);
    ok &= expect_int("trace.d2r2.pass1", trace.pass1_order, 0x18, A_LINEAGE);
    ok &= expect_int("trace.d2r2.pass2", trace.pass2_order, 0x49, A_LINEAGE);

    return ok;
}

static int test_front_clipped_blit(void)
{
    int ok = 1;
    uint8_t source[96];
    uint8_t destination[64];
    uint8_t open_destination[32];
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2L2);
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2R2);

    for (int i = 0; i < 96; ++i) source[i] = (uint8_t)(i + 1);
    source[9] = 99;
    source[24] = 10;
    source[72] = 10;
    for (int i = 0; i < 64; ++i) destination[i] = 77;
    for (int i = 0; i < 32; ++i) open_destination[i] = 88;

    ok &= expect_int("blit.d2l2.copied",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_half_blit_pc34(
                         d2l2, 1, source, 48, destination, 32, 8, 2),
                     46, A_F0102);
    ok &= expect_int("blit.d2l2.transparent_first", destination[8], 77,
                     A_F0102);
    ok &= expect_int("blit.d2l2.pixel_after_transparent", destination[9], 26,
                     A_F0102);
    ok &= expect_int("blit.d2l2.row1_transparent", destination[40], 77,
                     A_F0102);
    ok &= expect_int("blit.d2l2.row1_pixel", destination[41], 74, A_F0102);
    ok &= expect_int("blit.d2r2.copied",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_half_blit_pc34(
                         d2r2, 1, source, 48, destination, 32, 0, 1),
                     24, A_F0102);
    ok &= expect_int("blit.d2r2.left_pixel", destination[0], 1, A_F0102);
    ok &= expect_int("blit.open_skip",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_half_blit_pc34(
                         d2l2, 0, source, 48, open_destination, 32, 0, 1),
                     0, A_STATE);
    ok &= expect_int("blit.open_preserve", open_destination[0], 88, A_STATE);
    ok &= expect_int("blit.bad_height",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_half_blit_pc34(
                         d2l2, 1, source, 48, destination, 32, 0, 41),
                     -1, "ReDMCSB COORD.C:1556");
    ok &= expect_int("blit.bad_source_stride",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_half_blit_pc34(
                         d2l2, 1, source, 47, destination, 32, 0, 1),
                     -1, "ReDMCSB COORD.C:1550");
    ok &= expect_int("blit.bad_destination_stride",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_half_blit_pc34(
                         d2l2, 1, source, 48, destination, 23, 0, 1),
                     -1, "front-clipped half width");
    ok &= expect_int("blit.null_spec",
                     csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_half_blit_pc34(
                         NULL, 1, source, 48, destination, 32, 0, 1),
                     -1, A_F0102);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const char *e =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_source_evidence_pc34();
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2L2);
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2R2);

    ok &= expect_contains("evidence.f0102", e, "DUNVIEW.C:3082-3093", A_F0102);
    ok &= expect_contains("evidence.f0103", e, "DUNVIEW.C:3096-3108", A_F0103);
    ok &= expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4337", A_F0111);
    ok &= expect_contains("evidence.destroyed", e, "C15 destroyed ornament",
                          A_DEFS);
    ok &= expect_contains("evidence.c2", e, "C2_VIEW_DOOR_ORNAMENT_D1LCR",
                          A_DEFS);
    ok &= expect_contains("evidence.c4000", e, "MASK0x4000", A_DEFS);
    ok &= expect_contains("evidence.d2l2_half", e, "right half of D2L2",
                          A_HALF);
    ok &= expect_contains("evidence.d2r2_half", e, "left half of D2R2",
                          A_HALF);
    ok &= expect_contains("evidence.lineage_route", e, "Viewport.cpp:1853-1862",
                          A_LINEAGE);
    ok &= expect_contains("evidence.pass_constants", e, "0x28/0x39",
                          A_LINEAGE);
    ok &= expect_contains("d2l2.f0102", d2l2 ? d2l2->redmcsb_f0102_lines : NULL,
                          "3082-3093", A_F0102);
    ok &= expect_contains("d2l2.f0103", d2l2 ? d2l2->redmcsb_f0103_lines : NULL,
                          "3096-3108", A_F0103);
    ok &= expect_contains("d2r2.f0111", d2r2 ? d2r2->redmcsb_f0111_lines : NULL,
                          "4218-4337", A_F0111);
    ok &= expect_contains("d2l2.defs", d2l2 ? d2l2->redmcsb_defs_lines : NULL,
                          "2466,2791,3516", A_DEFS);
    ok &= expect_contains("d2r2.lineage", d2r2 ? d2r2->lineage_source_lines : NULL,
                          "1922,1926", A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_source_evidence_pc34());

    ok &= test_identity_and_constants();
    ok &= test_panel_half_metadata();
    ok &= test_state_machine_and_zones();
    ok &= test_pass_constants_and_ornaments();
    ok &= test_front_clipped_blit();
    ok &= test_evidence_strings();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_50", g_assertions >= 50, 1,
                     "assigned D2L2/D2R2 F0111 front-clipped source-lock gate");

    if (ok) {
        printf("PASS csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
