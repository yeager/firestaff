#include "csb_v1_viewport_d3l_d3r_sidewall_backdrops_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;

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

static int expect_uint(const char *label, unsigned int got, unsigned int want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%x want=0x%x anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("ok %s=0x%x anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    const int got = haystack && needle && strstr(haystack, needle) != 0;
    return expect_int(label, got, 1, anchor);
}

static int test_specs_and_dispatch_order(void)
{
    int ok = 1;
    const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *d3l =
        csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_for_side_pc34(
            CSB_V1_D3L_D3R_SIDEWALL_SIDE_D3L_PC34);
    const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *d3r =
        csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_for_side_pc34(
            CSB_V1_D3L_D3R_SIDEWALL_SIDE_D3R_PC34);

    /* ReDMCSB: DUNVIEW.C F0128 lines 8478-8500 dispatch D3L2/D3R2 before
     * F0116 D3L and F0117 D3R; DEFS.H lines 2608-2609 bind C12/C13. */
    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_count_pc34(),
                     2, "ReDMCSB DUNVIEW.C:8478-8500 F0128");
    ok &= expect_int("spec.index0.d3l",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_at_pc34(0) == d3l,
                     1, "ReDMCSB DUNVIEW.C:8490-8491 F0116 D3L");
    ok &= expect_int("spec.index1.d3r",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_at_pc34(1) == d3r,
                     1, "ReDMCSB DUNVIEW.C:8494-8495 F0117 D3R");
    ok &= expect_int("spec.index2.null",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_at_pc34(2) == 0,
                     1, "D3L/D3R-only side-wall table");
    ok &= expect_int("spec.unknown.null",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_for_side_pc34(9) == 0,
                     1, "D3L/D3R-only side ids");
    ok &= expect_int("d3l.view_square", d3l ? d3l->view_square : -1, 12,
                     "ReDMCSB DEFS.H:2608 C12_VIEW_SQUARE_D3L");
    ok &= expect_int("d3r.view_square", d3r ? d3r->view_square : -1, 13,
                     "ReDMCSB DEFS.H:2609 C13_VIEW_SQUARE_D3R");
    ok &= expect_int("d3l.depth", d3l ? d3l->relative_depth : -1, 3,
                     "ReDMCSB DUNVIEW.C:8490 relative depth 3");
    ok &= expect_int("d3r.depth", d3r ? d3r->relative_depth : -1, 3,
                     "ReDMCSB DUNVIEW.C:8494 relative depth 3");
    ok &= expect_int("d3l.lateral", d3l ? d3l->relative_lateral : 0, -1,
                     "ReDMCSB DUNVIEW.C:8490 relative lateral -1");
    ok &= expect_int("d3r.lateral", d3r ? d3r->relative_lateral : 0, 1,
                     "ReDMCSB DUNVIEW.C:8494 relative lateral +1");
    ok &= expect_int("d3l.after.backdrops", d3l ? d3l->preceding_backdrop_count : -1,
                     2, "ReDMCSB DUNVIEW.C:8481-8486 D3L2/D3R2 first");
    ok &= expect_int("d3r.after.backdrops", d3r ? d3r->preceding_backdrop_count : -1,
                     2, "ReDMCSB DUNVIEW.C:8481-8486 D3L2/D3R2 first");
    ok &= expect_int("d3l.order.index", d3l ? d3l->f0128_order_after_d3l2_d3r2 : -1,
                     2, "ReDMCSB DUNVIEW.C:8490-8491");
    ok &= expect_int("d3r.order.index", d3r ? d3r->f0128_order_after_d3l2_d3r2 : -1,
                     3, "ReDMCSB DUNVIEW.C:8494-8495");

    return ok;
}

static int test_wall_backdrop_and_ornament_route(void)
{
    int ok = 1;
    const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *d3l =
        csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_at_pc34(0);
    const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *d3r =
        csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_at_pc34(1);
    CSB_V1_D3LD3RSidewallBackdropTracePc34 trace;

    /* ReDMCSB: DUNVIEW.C F0116 lines 6406-6437 and F0117 lines 6545-6573
     * blit wall backdrops, call F0107 twice, then return unless front alcove. */
    ok &= expect_int("d3l.wall_zone", d3l ? d3l->wall_zone : -1, 705,
                     "ReDMCSB DEFS.H:4045 C705_ZONE_WALL_D3L");
    ok &= expect_int("d3r.wall_zone", d3r ? d3r->wall_zone : -1, 706,
                     "ReDMCSB DEFS.H:4046 C706_ZONE_WALL_D3R");
    ok &= expect_int("d3l.native_wall", d3l ? d3l->native_wall_index : -1, 13,
                     "ReDMCSB DUNVIEW.C:6427 G2107_WallSet[C13_WALL_D3L]");
    ok &= expect_int("d3r.native_wall", d3r ? d3r->native_wall_index : -1, 12,
                     "ReDMCSB DUNVIEW.C:6563 G2107_WallSet[C12_WALL_D3R]");
    ok &= expect_int("d3l.flipped_wall", d3l ? d3l->flipped_wall_index : -1, 12,
                     "ReDMCSB DUNVIEW.C:6423 F0105 C12_WALL_D3R");
    ok &= expect_int("d3r.flipped_wall", d3r ? d3r->flipped_wall_index : -1, 13,
                     "ReDMCSB DUNVIEW.C:6555 F0105 C13_WALL_D3L");
    ok &= expect_int("d3l.side_ornament", d3l ? d3l->side_wall_ornament_view : -1, 2,
                     "ReDMCSB DEFS.H:2698 M575_VIEW_WALL_D3L_RIGHT");
    ok &= expect_int("d3l.front_ornament", d3l ? d3l->front_wall_ornament_view : -1, 4,
                     "ReDMCSB DEFS.H:2700 M577_VIEW_WALL_D3L_FRONT");
    ok &= expect_int("d3r.side_ornament", d3r ? d3r->side_wall_ornament_view : -1, 3,
                     "ReDMCSB DEFS.H:2699 M576_VIEW_WALL_D3R_LEFT");
    ok &= expect_int("d3r.front_ornament", d3r ? d3r->front_wall_ornament_view : -1, 6,
                     "ReDMCSB DEFS.H:2702 M579_VIEW_WALL_D3R_FRONT");
    ok &= expect_int("trace.d3l.wall.no_alcove",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_trace_pc34(
                         d3l, CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_WALL_PC34, 0, &trace),
                     0, "ReDMCSB DUNVIEW.C:6406-6437 F0116 WALL");
    ok &= expect_int("trace.wall.blit", trace.wall_blit_calls, 1,
                     "ReDMCSB DUNVIEW.C:6411-6427 wall blit");
    ok &= expect_int("trace.wall.f0107", trace.f0107_calls, 2,
                     "ReDMCSB DUNVIEW.C:6432-6433 F0107 side/front");
    ok &= expect_int("trace.wall.no_f0108", trace.f0108_calls, 0,
                     "ReDMCSB DUNVIEW.C:6437 returns before F0108");
    ok &= expect_int("trace.wall.no_f0115", trace.f0115_calls, 0,
                     "ReDMCSB DUNVIEW.C:6437 returns before F0115");
    ok &= expect_int("trace.wall.return", trace.wall_returns_without_front_alcove, 1,
                     "ReDMCSB DUNVIEW.C:6433-6437 front non-alcove return");
    ok &= expect_int("trace.d3r.wall.alcove",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_trace_pc34(
                         d3r, CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_WALL_PC34, 1, &trace),
                     0, "ReDMCSB DUNVIEW.C:6568-6572 F0117 alcove");
    ok &= expect_int("trace.alcove.f0115", trace.f0115_calls, 1,
                     "ReDMCSB DUNVIEW.C:6570-6572 and 6622 F0115");
    ok &= expect_uint("trace.alcove.order", trace.first_f0115_order, 0,
                      "ReDMCSB DUNVIEW.C:6570 C0x0000_CELL_ORDER_ALCOVE");
    ok &= expect_int("trace.alcove.flag", trace.front_alcove_uses_f0115_zero_order, 1,
                     "ReDMCSB DUNVIEW.C:6570-6572");

    return ok;
}

static int test_open_door_and_thing_pass_orders(void)
{
    int ok = 1;
    const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *d3l =
        csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_at_pc34(0);
    const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *d3r =
        csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_at_pc34(1);
    CSB_V1_D3LD3RSidewallBackdropTracePc34 trace;

    /* ReDMCSB: DUNVIEW.C F0116 lines 6442-6480 and F0117 lines 6578-6622
     * bracket door fronts with F0108, rear F0115, F0111, then front F0115. */
    ok &= expect_uint("d3l.open.order", d3l ? d3l->open_order : 0, 0x3421u,
                      "ReDMCSB DEFS.H:2676 C0x3421; DUNVIEW.C:6476-6480");
    ok &= expect_uint("d3r.open.order", d3r ? d3r->open_order : 0, 0x4312u,
                      "ReDMCSB DEFS.H:2677 C0x4312; DUNVIEW.C:6618-6622");
    ok &= expect_uint("d3l.door_side.order", d3l ? d3l->door_side_order : 0, 0x0321u,
                      "ReDMCSB DEFS.H:2670 C0x0321; DUNVIEW.C:6438-6441");
    ok &= expect_uint("d3r.door_side.order", d3r ? d3r->door_side_order : 0, 0x0412u,
                      "ReDMCSB DEFS.H:2673 C0x0412; DUNVIEW.C:6574-6577");
    ok &= expect_uint("d3l.door_rear.order", d3l ? d3l->door_rear_order : 0, 0x0218u,
                      "ReDMCSB DEFS.H:2669 C0x0218; DUNVIEW.C:6443-6444");
    ok &= expect_uint("d3r.door_rear.order", d3r ? d3r->door_rear_order : 0, 0x0128u,
                      "ReDMCSB DEFS.H:2668 C0x0128; DUNVIEW.C:6579-6580");
    ok &= expect_uint("d3l.door_front.order", d3l ? d3l->door_front_order : 0, 0x0349u,
                      "ReDMCSB DEFS.H:2672 C0x0349; DUNVIEW.C:6459-6480");
    ok &= expect_uint("d3r.door_front.order", d3r ? d3r->door_front_order : 0, 0x0439u,
                      "ReDMCSB DEFS.H:2675 C0x0439; DUNVIEW.C:6601-6622");
    ok &= expect_int("trace.d3l.open",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_trace_pc34(
                         d3l, CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_CORRIDOR_PC34, 0, &trace),
                     0, "ReDMCSB DUNVIEW.C:6473-6480 corridor route");
    ok &= expect_int("trace.open.f0108", trace.f0108_calls, 1,
                     "ReDMCSB DUNVIEW.C:6478 F0108");
    ok &= expect_int("trace.open.f0115", trace.f0115_calls, 1,
                     "ReDMCSB DUNVIEW.C:6480 F0115");
    ok &= expect_uint("trace.open.order", trace.first_f0115_order, 0x3421u,
                      "ReDMCSB DUNVIEW.C:6476 C0x3421");
    ok &= expect_int("trace.d3r.door_side",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_trace_pc34(
                         d3r, CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_DOOR_SIDE_PC34, 0, &trace),
                     0, "ReDMCSB DUNVIEW.C:6574-6577 door-side route");
    ok &= expect_int("trace.door_side.f0108", trace.f0108_calls, 1,
                     "ReDMCSB DUNVIEW.C:6620 F0108");
    ok &= expect_int("trace.door_side.f0115", trace.f0115_calls, 1,
                     "ReDMCSB DUNVIEW.C:6622 F0115");
    ok &= expect_uint("trace.door_side.order", trace.first_f0115_order, 0x0412u,
                      "ReDMCSB DUNVIEW.C:6576 C0x0412");
    ok &= expect_int("trace.d3l.door_front",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_trace_pc34(
                         d3l, CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_DOOR_FRONT_PC34, 0, &trace),
                     0, "ReDMCSB DUNVIEW.C:6442-6460 door-front route");
    ok &= expect_int("trace.door_front.f0108", trace.f0108_calls, 1,
                     "ReDMCSB DUNVIEW.C:6443 F0108");
    ok &= expect_int("trace.door_front.f0111", trace.f0111_calls, 1,
                     "ReDMCSB DUNVIEW.C:6447/6457 F0111");
    ok &= expect_int("trace.door_front.f0115", trace.f0115_calls, 2,
                     "ReDMCSB DUNVIEW.C:6444 and 6480 F0115 passes");
    ok &= expect_uint("trace.door_front.rear", trace.first_f0115_order, 0x0218u,
                      "ReDMCSB DUNVIEW.C:6444 rear F0115");
    ok &= expect_uint("trace.door_front.front", trace.second_f0115_order, 0x0349u,
                      "ReDMCSB DUNVIEW.C:6459/6480 front F0115");

    return ok;
}

static int test_transparency_and_evidence(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d3l_d3r_sidewall_backdrops_source_evidence_pc34();
    const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *d3l =
        csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_at_pc34(0);
    CSB_V1_D3LD3RSidewallBackdropTracePc34 trace;

    /* ReDMCSB: DUNVIEW.C F0104 lines 3113-3156 and F0105 lines 3185-3247
     * use DEFS.H line 2088 C10_COLOR_FLESH transparency for wall blits. */
    ok &= expect_int("blend.c10.preserves",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_blend_c10_pc34(
                         0x44u, 10u),
                     0x44, "ReDMCSB DUNVIEW.C:3113-3156/3185-3247");
    ok &= expect_int("blend.non_c10.copies",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_blend_c10_pc34(
                         0x44u, 0x55u),
                     0x55, "ReDMCSB DUNVIEW.C:3113-3156/3185-3247");
    ok &= expect_int("trace.c10",
                     csb_v1_viewport_d3l_d3r_sidewall_backdrops_trace_pc34(
                         d3l, CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_PIT_PC34, 0, &trace),
                     0, "ReDMCSB DUNVIEW.C:6461-6480 pit route");
    ok &= expect_int("trace.c10.flag", trace.c10_transparency_preserved, 1,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_contains("evidence.f0128", e, "F0128 draws D3L2 and D3R2 first",
                          "ReDMCSB DUNVIEW.C:8478-8500");
    ok &= expect_contains("evidence.f0116", e, "F0116",
                          "ReDMCSB DUNVIEW.C:6361-6480");
    ok &= expect_contains("evidence.f0117", e, "F0117",
                          "ReDMCSB DUNVIEW.C:6500-6622");
    ok &= expect_contains("evidence.f0107", e, "F0107",
                          "ReDMCSB DUNVIEW.C:3502-3938");
    ok &= expect_contains("evidence.f0108", e, "F0108",
                          "ReDMCSB DUNVIEW.C:3940-4009");
    ok &= expect_contains("evidence.f0115", e, "F0115",
                          "ReDMCSB DUNVIEW.C:4547-4581");
    ok &= expect_contains("evidence.lineage.open", e, "Viewport.cpp:1192-1209",
                          "CSB-lineage Viewport.cpp:1192-1209");
    ok &= expect_contains("evidence.lineage.door", e, "Viewport.cpp:1903-1915",
                          "CSB-lineage Viewport.cpp:1903-1915");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d3l_d3r_sidewall_backdrops_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d3l_d3r_sidewall_backdrops_source_evidence_pc34());

    ok &= test_specs_and_dispatch_order();
    ok &= test_wall_backdrop_and_ornament_route();
    ok &= test_open_door_and_thing_pass_orders();
    ok &= test_transparency_and_evidence();

    if (!ok) {
        printf("FAIL csb_v1_viewport_d3l_d3r_sidewall_backdrops_pc34_compat assertions=%d\n",
               g_assertions);
        return 1;
    }
    printf("PASS csb_v1_viewport_d3l_d3r_sidewall_backdrops_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
