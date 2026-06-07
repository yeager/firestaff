#include "csb_v1_viewport_d2l_d2r_f0107_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;

static const char *A_BOTH =
    "ReDMCSB DUNVIEW.C F0120/F0121 C00_ELEMENT_WALL path lines "
    "6945-6973/7096-7123, F0107 lines 3502-3939, F0100 lines 3048-3061; "
    "DEFS.H lines 2537-2539,2582-2583,2686-2690,2742-2744,2478,2088; "
    "CSB Viewport.cpp lines 1003-1013/1027-1035";
static const char *A_D2L =
    "ReDMCSB DUNVIEW.C F0120_DUNGEONVIEW_DrawSquareD2L lineage alias "
    "lines 6945-6973 with F0107 at 6968-6969 and 3502-3939; DEFS.H "
    "M551/M552/M580/M582/C112/C10; CSB Viewport.cpp lines 1003-1013";
static const char *A_D2R =
    "ReDMCSB DUNVIEW.C F0121_DUNGEONVIEW_DrawSquareD2R lineage alias "
    "lines 7096-7123 with F0107 at 7119-7120 and 3502-3939; DEFS.H "
    "M553/M552/M581/M584/C112/C10; CSB Viewport.cpp lines 1027-1035";

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

static int test_route_identity(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *d2l =
        csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_for_side_pc34(
            CSB_V1_VIEWPORT_D2L_D2R_F0107_SIDE_D2L_PC34);
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *d2r =
        csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_for_side_pc34(
            CSB_V1_VIEWPORT_D2L_D2R_F0107_SIDE_D2R_PC34);

    ok &= expect_int("route.count",
                     (int)csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_count_pc34(),
                     2, A_BOTH);
    ok &= expect_int("route.index0.d2l",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(0) == d2l,
                     1, A_D2L);
    ok &= expect_int("route.index1.d2r",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(1) == d2r,
                     1, A_D2R);
    ok &= expect_int("route.index2.null",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(2) == NULL,
                     1, A_BOTH);
    ok &= expect_int("route.invalid.null",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_for_side_pc34(
                         (CSB_V1_ViewportD2LD2RF0107SidePc34)9) == NULL,
                     1, A_BOTH);
    ok &= expect_int("d2l.present", d2l != NULL, 1, A_D2L);
    ok &= expect_int("d2r.present", d2r != NULL, 1, A_D2R);
    ok &= expect_int("d2l.view_square", d2l ? d2l->view_square : -1, 4, A_D2L);
    ok &= expect_int("d2r.view_square", d2r ? d2r->view_square : -1, 5, A_D2R);
    ok &= expect_int("d2l.depth", d2l ? d2l->relative_depth : -1, 2, A_D2L);
    ok &= expect_int("d2r.depth", d2r ? d2r->relative_depth : -1, 2, A_D2R);
    ok &= expect_int("d2l.lateral", d2l ? d2l->relative_lateral : 0, -1, A_D2L);
    ok &= expect_int("d2r.lateral", d2r ? d2r->relative_lateral : 0, 1, A_D2R);
    ok &= expect_int("wall.element", d2l ? d2l->wall_element : -1, 0, A_BOTH);
    ok &= expect_int("door_front.element", d2r ? d2r->door_front_element : -1, 17, A_BOTH);

    return ok;
}

static int test_wall_blit_and_metadata(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *d2l =
        csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(0);
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *d2r =
        csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(1);

    ok &= expect_int("d2l.f0100.wall_blit", d2l ? d2l->f0100_wall_blit : -1, 1, A_D2L);
    ok &= expect_int("d2r.f0100.wall_blit", d2r ? d2r->f0100_wall_blit : -1, 1, A_D2R);
    ok &= expect_int("d2l.wall_bitmap", d2l ? d2l->wall_set_bitmap_index : -1, 8, A_D2L);
    ok &= expect_int("d2r.wall_bitmap", d2r ? d2r->wall_set_bitmap_index : -1, 7, A_D2R);
    ok &= expect_int("d2l.frame_view_square", d2l ? d2l->wall_frame_view_square : -1, 4, A_D2L);
    ok &= expect_int("d2r.frame_view_square", d2r ? d2r->wall_frame_view_square : -1, 5, A_D2R);
    ok &= expect_int("d2l.wall_zone", d2l ? d2l->wall_zone : -1, 708, A_D2L);
    ok &= expect_int("d2r.wall_zone", d2r ? d2r->wall_zone : -1, 709, A_D2R);
    ok &= expect_int("d2l.source_bitmap_resolved",
                     d2l ? d2l->f0100_source_bitmap_resolved : -1, 1, A_D2L);
    ok &= expect_int("d2r.source_bitmap_resolved",
                     d2r ? d2r->f0100_source_bitmap_resolved : -1, 1, A_D2R);
    ok &= expect_int("d2l.frame_resolved", d2l ? d2l->f0100_frame_resolved : -1, 1, A_D2L);
    ok &= expect_int("d2r.frame_resolved", d2r ? d2r->f0100_frame_resolved : -1, 1, A_D2R);
    ok &= expect_int("d2l.c10", d2l ? d2l->f0100_transparent_color : -1, 10, A_D2L);
    ok &= expect_int("d2r.c10", d2r ? d2r->f0100_transparent_color : -1, 10, A_D2R);
    ok &= expect_int("d2l.byte_width_viewport",
                     d2l ? d2l->f0100_destination_byte_width : -1, 112, A_D2L);
    ok &= expect_int("d2r.byte_width_viewport",
                     d2r ? d2r->f0100_destination_byte_width : -1, 112, A_D2R);

    return ok;
}

static int test_f0107_routes_and_conditionals(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *d2l =
        csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(0);
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *d2r =
        csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(1);
    CSB_V1_ViewportD2LD2RF0107RunResultPc34 result;

    ok &= expect_int("d2l.side.ordinal", d2l ? d2l->f0107_side_ordinal : -1, 2, A_D2L);
    ok &= expect_int("d2l.side.view_wall", d2l ? d2l->f0107_side_view_wall : -1, 5, A_D2L);
    ok &= expect_int("d2l.front.ordinal", d2l ? d2l->f0107_front_ordinal : -1, 3, A_D2L);
    ok &= expect_int("d2l.front.view_wall", d2l ? d2l->f0107_front_view_wall : -1, 7, A_D2L);
    ok &= expect_int("d2r.side.ordinal", d2r ? d2r->f0107_side_ordinal : -1, 4, A_D2R);
    ok &= expect_int("d2r.side.view_wall", d2r ? d2r->f0107_side_view_wall : -1, 6, A_D2R);
    ok &= expect_int("d2r.front.ordinal", d2r ? d2r->f0107_front_ordinal : -1, 3, A_D2R);
    ok &= expect_int("d2r.front.view_wall", d2r ? d2r->f0107_front_view_wall : -1, 9, A_D2R);
    ok &= expect_int("d2l.side_before_front", d2l ? d2l->f0107_side_before_front : -1, 1, A_D2L);
    ok &= expect_int("d2r.side_before_front", d2r ? d2r->f0107_side_before_front : -1, 1, A_D2R);
    ok &= expect_int("d2l.front.conditional",
                     d2l ? d2l->f0107_front_conditional_branch : -1, 1, A_D2L);
    ok &= expect_int("d2r.front.conditional",
                     d2r ? d2r->f0107_front_conditional_branch : -1, 1, A_D2R);

    ok &= expect_int("d2l.run.alcove",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
                         d2l,
                         CSB_V1_VIEWPORT_D2L_D2R_F0107_ELEMENT_WALL_PC34,
                         1, 1, &result),
                     0, A_D2L);
    ok &= expect_int("d2l.wall_blit_before_f0107", result.wall_blit_before_f0107, 1, A_D2L);
    ok &= expect_int("d2l.side.calls", result.f0107_side_calls, 1, A_D2L);
    ok &= expect_int("d2l.front.calls", result.f0107_front_calls, 1, A_D2L);
    ok &= expect_int("d2l.side.return", result.f0107_side_return_alcove, 1, A_D2L);
    ok &= expect_int("d2l.front.branch_taken", result.f0107_front_branch_taken, 1, A_D2L);
    ok &= expect_int("d2l.calls.compose", result.f0107_calls_compose, 1, A_D2L);

    ok &= expect_int("d2r.run.alcove",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
                         d2r,
                         CSB_V1_VIEWPORT_D2L_D2R_F0107_ELEMENT_WALL_PC34,
                         1, 1, &result),
                     0, A_D2R);
    ok &= expect_int("d2r.wall_blit_before_f0107", result.wall_blit_before_f0107, 1, A_D2R);
    ok &= expect_int("d2r.side.calls", result.f0107_side_calls, 1, A_D2R);
    ok &= expect_int("d2r.front.calls", result.f0107_front_calls, 1, A_D2R);
    ok &= expect_int("d2r.side.return", result.f0107_side_return_alcove, 1, A_D2R);
    ok &= expect_int("d2r.front.branch_taken", result.f0107_front_branch_taken, 1, A_D2R);
    ok &= expect_int("d2r.calls.compose", result.f0107_calls_compose, 1, A_D2R);

    ok &= expect_int("d2l.run.no_alcove",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
                         d2l,
                         CSB_V1_VIEWPORT_D2L_D2R_F0107_ELEMENT_WALL_PC34,
                         0, 0, &result),
                     0, A_D2L);
    ok &= expect_int("d2l.front.branch_fallthrough",
                     result.f0107_front_branch_fallthrough, 1, A_D2L);
    ok &= expect_int("d2l.front.branch_not_taken", result.f0107_front_branch_taken, 0, A_D2L);
    ok &= expect_int("d2r.run.no_alcove",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
                         d2r,
                         CSB_V1_VIEWPORT_D2L_D2R_F0107_ELEMENT_WALL_PC34,
                         0, 0, &result),
                     0, A_D2R);
    ok &= expect_int("d2r.front.branch_fallthrough",
                     result.f0107_front_branch_fallthrough, 1, A_D2R);
    ok &= expect_int("d2r.front.branch_not_taken", result.f0107_front_branch_taken, 0, A_D2R);

    return ok;
}

static int test_f0108_no_f0111_and_csb_parity(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *d2l =
        csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(0);
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *d2r =
        csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(1);
    CSB_V1_ViewportD2LD2RF0107RunResultPc34 result;

    ok &= expect_int("d2l.no_f0111_wall_case",
                     d2l ? d2l->f0111_wall_case_call_count : -1, 0, A_D2L);
    ok &= expect_int("d2r.no_f0111_wall_case",
                     d2r ? d2r->f0111_wall_case_call_count : -1, 0, A_D2R);
    ok &= expect_int("d2l.f0111.door_only", d2l ? d2l->f0111_door_case_only : -1, 1, A_D2L);
    ok &= expect_int("d2r.f0111.door_only", d2r ? d2r->f0111_door_case_only : -1, 1, A_D2R);
    ok &= expect_int("d2l.door_front.f0108",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
                         d2l,
                         CSB_V1_VIEWPORT_D2L_D2R_F0107_ELEMENT_DOOR_FRONT_PC34,
                         0, 0, &result),
                     0, A_D2L);
    ok &= expect_int("d2l.f0108.calls", result.f0108_floor_ornament_calls, 1, A_D2L);
    ok &= expect_int("d2l.f0108.floor_view", result.f0108_floor_ornament_view, 3, A_D2L);
    ok &= expect_int("d2r.door_front.f0108",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
                         d2r,
                         CSB_V1_VIEWPORT_D2L_D2R_F0107_ELEMENT_DOOR_FRONT_PC34,
                         0, 0, &result),
                     0, A_D2R);
    ok &= expect_int("d2r.f0108.calls", result.f0108_floor_ornament_calls, 1, A_D2R);
    ok &= expect_int("d2r.f0108.floor_view", result.f0108_floor_ornament_view, 5, A_D2R);
    ok &= expect_int("d2l.csb.side_location",
                     d2l ? d2l->csb_viewport_script_side_wall_location : -1, 5, A_D2L);
    ok &= expect_int("d2l.csb.front_location",
                     d2l ? d2l->csb_viewport_script_front_wall_location : -1, 7, A_D2L);
    ok &= expect_int("d2r.csb.side_location",
                     d2r ? d2r->csb_viewport_script_side_wall_location : -1, 6, A_D2R);
    ok &= expect_int("d2r.csb.front_location",
                     d2r ? d2r->csb_viewport_script_front_wall_location : -1, 9, A_D2R);
    ok &= expect_int("d2l.csb.alcove_jump",
                     d2l ? d2l->csb_viewport_script_alcove_jump : -1, 1, A_D2L);
    ok &= expect_int("d2r.csb.alcove_jump",
                     d2r ? d2r->csb_viewport_script_alcove_jump : -1, 1, A_D2R);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *d2l =
        csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(0);
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *d2r =
        csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(1);
    const char *e = csb_v1_viewport_d2l_d2r_f0107_ornament_source_evidence_pc34();
    CSB_V1_ViewportD2LD2RF0107RunResultPc34 result;

    ok &= expect_contains("evidence.pixel_anchor.wall_path", e,
                          "DUNVIEW.C F0120/F0121 C00_ELEMENT_WALL path", A_BOTH);
    ok &= expect_contains("evidence.pixel_anchor.alcove", e,
                          "F0107 alcove return", A_BOTH);
    ok &= expect_contains("evidence.d2l.6968", e, "line 6968", A_D2L);
    ok &= expect_contains("evidence.d2r.7119", e, "line 7119", A_D2R);
    ok &= expect_contains("evidence.c112", e, "C112_BYTE_WIDTH_VIEWPORT", A_BOTH);
    ok &= expect_contains("evidence.c10", e, "C10_COLOR_FLESH", A_BOTH);
    ok &= expect_contains("evidence.viewport.d2l", e, "1003-1013", A_D2L);
    ok &= expect_contains("evidence.viewport.d2r", e, "1027-1035", A_D2R);
    ok &= expect_contains("d2l.redmcsb.function", d2l ? d2l->redmcsb_function : "",
                          "F0120_DUNGEONVIEW_DrawSquareD2L", A_D2L);
    ok &= expect_contains("d2r.redmcsb.function", d2r ? d2r->redmcsb_function : "",
                          "F0121_DUNGEONVIEW_DrawSquareD2R", A_D2R);
    ok &= expect_contains("d2l.lines", d2l ? d2l->redmcsb_lines : "",
                          "6968-6969", A_D2L);
    ok &= expect_contains("d2r.lines", d2r ? d2r->redmcsb_lines : "",
                          "7119-7120", A_D2R);
    ok &= expect_int("run.null.reject",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
                         NULL,
                         CSB_V1_VIEWPORT_D2L_D2R_F0107_ELEMENT_WALL_PC34,
                         0, 0, &result),
                     -1, A_BOTH);
    ok &= expect_int("run.bad.element.reject",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
                         d2l, 99, 0, 0, &result),
                     1, A_BOTH);
    ok &= expect_int("run.result.evidence",
                     csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
                         d2r,
                         CSB_V1_VIEWPORT_D2L_D2R_F0107_ELEMENT_WALL_PC34,
                         1, 0, &result),
                     0, A_D2R);
    ok &= expect_contains("result.evidence.wall_path", result.source_lock_evidence,
                          "DUNVIEW.C F0120/F0121 C00_ELEMENT_WALL path", A_BOTH);
    ok &= expect_contains("result.evidence.alcove", result.source_lock_evidence,
                          "F0107 alcove return", A_BOTH);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2l_d2r_f0107_ornament_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2l_d2r_f0107_ornament_source_evidence_pc34());

    ok &= test_route_identity();
    ok &= test_wall_blit_and_metadata();
    ok &= test_f0107_routes_and_conditionals();
    ok &= test_f0108_no_f0111_and_csb_parity();
    ok &= test_evidence_strings();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_40", g_assertions >= 40, 1, A_BOTH);

    if (ok) {
        printf("PASS csb_v1_viewport_d2l_d2r_f0107_ornament_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
