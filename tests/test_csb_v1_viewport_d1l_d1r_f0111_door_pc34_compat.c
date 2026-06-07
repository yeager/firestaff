#include "csb_v1_viewport_d1l_d1r_f0111_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_D1L =
    "ReDMCSB DUNVIEW.C:7492-7508,7520-7536 F0122_DUNGEONVIEW_DrawSquareD1L";
static const char *A_D1R =
    "ReDMCSB DUNVIEW.C:7660-7676,7688-7704 F0123_DUNGEONVIEW_DrawSquareD1R";
static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2599-2601,2791,4258-4260,2078-2088";
static const char *A_COORD =
    "ReDMCSB COORD.C:780-877,1548-1567";
static const char *A_LINEAGE =
    "ReDMCSB DUNVIEW.C:7494/7662; CSB-lineage Viewport.cpp:1892-1900,1919-1927";

static int g_assertions = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    const int got = haystack && needle && strstr(haystack, needle) != NULL;
    return expect_int(label, got, 1, anchor);
}

static int test_identity_and_scope(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *d1l =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(4);
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *d1r =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(5);

    ok &= expect_int("route.count",
                     (int)csb_v1_viewport_d1l_d1r_f0111_door_pc34_count(), 2,
                     A_DEFS);
    ok &= expect_int("d1l.present", d1l != NULL, 1, A_D1L);
    ok &= expect_int("d1r.present", d1r != NULL, 1, A_D1R);
    ok &= expect_int("unknown.square",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(9) == NULL,
                     1, A_DEFS);
    ok &= expect_int("index0.matches.d1l",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_at(0) == d1l,
                     1, A_D1L);
    ok &= expect_int("index2.null",
                     csb_v1_viewport_d1l_d1r_f0111_door_pc34_at(2) == NULL,
                     1, A_DEFS);
    ok &= expect_int("contract.only", d1l ? d1l->source_locked_contract_only : 0,
                     1, A_F0111);
    ok &= expect_int("no.real.asset.parity",
                     d1l ? d1l->no_real_asset_bitmap_parity : 0, 1, A_F0111);
    ok &= expect_int("no.game.data.load", d1l ? d1l->no_game_data_load : 0,
                     1, A_F0111);
    ok &= expect_contains("d1l.source", d1l ? d1l->source_lines : NULL,
                          "7492-7508", A_D1L);
    ok &= expect_contains("d1r.source", d1r ? d1r->source_lines : NULL,
                          "7660-7676", A_D1R);

    return ok;
}

static int test_d1l_d1r_route_contracts(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *d1l =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(4);
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *d1r =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(5);

    ok &= expect_int("d1l.view_square", d1l ? d1l->view_square : -1, 4, A_DEFS);
    ok &= expect_int("d1r.view_square", d1r ? d1r->view_square : -1, 5, A_DEFS);
    ok &= expect_int("d1l.depth", d1l ? d1l->view_depth : -1, 1,
                     "ReDMCSB DUNVIEW.C:372 G2027[4]");
    ok &= expect_int("d1r.depth", d1r ? d1r->view_depth : -1, 1,
                     "ReDMCSB DUNVIEW.C:372 G2027[5]");
    ok &= expect_int("d1l.lane", d1l ? d1l->view_lane : 99, -1,
                     "ReDMCSB DUNVIEW.C:371 G2026[4]");
    ok &= expect_int("d1r.lane", d1r ? d1r->view_lane : 99, 1,
                     "ReDMCSB DUNVIEW.C:371 G2026[5]");
    ok &= expect_int("d1l.f0111.present", d1l ? d1l->f0111_route_present : 0,
                     1, A_D1L);
    ok &= expect_int("d1r.f0111.present", d1r ? d1r->f0111_route_present : 0,
                     1, A_D1R);
    ok &= expect_int("d1l.pass1", d1l ? d1l->door_front_rear_f0115_order : -1,
                     0x0028, A_D1L);
    ok &= expect_int("d1l.pass2", d1l ? d1l->door_front_front_f0115_order : -1,
                     0x0039, A_D1L);
    ok &= expect_int("d1r.pass1", d1r ? d1r->door_front_rear_f0115_order : -1,
                     0x0018, A_D1R);
    ok &= expect_int("d1r.pass2", d1r ? d1r->door_front_front_f0115_order : -1,
                     0x0049, A_D1R);
    ok &= expect_int("d1l.floor_ornament",
                     d1l ? d1l->floor_ornament_before_rear_pass : 0, 1, A_D1L);
    ok &= expect_int("d1r.floor_ornament",
                     d1r ? d1r->floor_ornament_before_rear_pass : 0, 1, A_D1R);

    return ok;
}

static int test_frame_door_and_field_constants(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *d1l =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(4);
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *d1r =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(5);

    ok &= expect_int("d1l.top_track_zone", d1l ? d1l->door_frame_top_zone : -1,
                     732, A_D1L);
    ok &= expect_int("d1r.top_track_zone", d1r ? d1r->door_frame_top_zone : -1,
                     734, A_D1R);
    ok &= expect_int("d1l.door_zone", d1l ? d1l->door_zone_base : -1, 3780,
                     A_DEFS);
    ok &= expect_int("d1r.door_zone", d1r ? d1r->door_zone_base : -1, 3800,
                     A_DEFS);
    ok &= expect_int("d1l.ornament_index",
                     d1l ? d1l->view_door_ornament_index : -1, 2, A_DEFS);
    ok &= expect_int("d1r.ornament_index",
                     d1r ? d1r->view_door_ornament_index : -1, 2, A_DEFS);
    ok &= expect_int("d1l.door_graphics_f1",
                     d1l ? d1l->door_graphic_depth_index : -1, 0, A_LINEAGE);
    ok &= expect_int("d1r.door_graphics_f1",
                     d1r ? d1r->door_graphic_depth_index : -1, 0, A_LINEAGE);
    ok &= expect_int("d1l.field_zone", d1l ? d1l->field_zone : -1, 713,
                     A_DEFS);
    ok &= expect_int("d1r.field_zone", d1r ? d1r->field_zone : -1, 714,
                     A_DEFS);

    return ok;
}

static int test_f0111_state_zone_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *d1l =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(4);
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *d1r =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(5);

    ok &= expect_int("open.skips.flag", d1l ? d1l->open_state_skips_f0111 : 0,
                     1, A_F0111);
    ok &= expect_int("d1l.open.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(d1l, 0),
                     -1, A_F0111);
    ok &= expect_int("d1l.state1.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(d1l, 1),
                     3781, A_F0111);
    ok &= expect_int("d1l.state3.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(d1l, 3),
                     3783, A_F0111);
    ok &= expect_int("d1l.closed.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(d1l, 4),
                     3780, A_F0111);
    ok &= expect_int("d1l.destroyed.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(d1l, 5),
                     3780, A_F0111);
    ok &= expect_int("d1r.state2.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(d1r, 2),
                     3802, A_F0111);
    ok &= expect_int("null.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(NULL, 2),
                     -1, A_F0111);
    ok &= expect_int("bad.state.zone",
                     csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(d1l, 6),
                     -1, A_F0111);
    ok &= expect_int("destroyed.mask",
                     d1l ? d1l->destroyed_state_applies_c15_mask : -1, 15,
                     "ReDMCSB DUNVIEW.C:4301-4304; DEFS.H:2466");

    return ok;
}

static int test_horizontal_half_and_c10_synthetic_blit(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *d1l =
        csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(4);
    uint8_t source[8] = { 10, 1, 2, 10, 3, 4, 10, 5 };
    uint8_t destination[8] = { 77, 77, 77, 77, 77, 77, 77, 77 };

    ok &= expect_int("horizontal.mask", d1l ? d1l->horizontal_second_half_mask : -1,
                     0x4000, "ReDMCSB DEFS.H:3516 MASK0x4000");
    ok &= expect_int("horizontal.left_half",
                     csb_v1_viewport_d1l_d1r_f0111_door_horizontal_half_zone_pc34(
                         d1l, 2, 0),
                     3788, "ReDMCSB DUNVIEW.C:4322 C6_UNKNOWN");
    ok &= expect_int("horizontal.right_half",
                     csb_v1_viewport_d1l_d1r_f0111_door_horizontal_half_zone_pc34(
                         d1l, 2, 1),
                     20169, "ReDMCSB DUNVIEW.C:4325 MASK0x4000 shift");
    ok &= expect_int("horizontal.closed.reject",
                     csb_v1_viewport_d1l_d1r_f0111_door_horizontal_half_zone_pc34(
                         d1l, 4, 1),
                     -1, A_F0111);
    ok &= expect_int("transparent.color", d1l ? d1l->transparent_color : -1,
                     10, A_DEFS);
    ok &= expect_int("blit.copied",
                     csb_v1_viewport_d1l_d1r_f0111_door_apply_c10_blit_pc34(
                         d1l, source, 4, destination, 4, 4, 2),
                     5, A_F0111);
    ok &= expect_int("blit.transparent0", destination[0], 77, A_DEFS);
    ok &= expect_int("blit.pixel1", destination[1], 1, A_DEFS);
    ok &= expect_int("blit.pixel2", destination[2], 2, A_DEFS);
    ok &= expect_int("blit.transparent3", destination[3], 77, A_DEFS);
    ok &= expect_int("blit.pixel7", destination[7], 5, A_DEFS);
    ok &= expect_int("blit.reject_null",
                     csb_v1_viewport_d1l_d1r_f0111_door_apply_c10_blit_pc34(
                         NULL, source, 4, destination, 4, 4, 2),
                     -1, A_F0111);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d1l_d1r_f0111_door_source_evidence_pc34();
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatEvidence *ev =
        csb_v1_viewport_d1l_d1r_f0111_door_evidence_pc34();

    ok &= expect_contains("evidence.contract", e, "no real-asset bitmap parity",
                          A_F0111);
    ok &= expect_contains("evidence.d1l", e, "DUNVIEW.C:7492-7508", A_D1L);
    ok &= expect_contains("evidence.d1r", e, "DUNVIEW.C:7660-7676", A_D1R);
    ok &= expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4337", A_F0111);
    ok &= expect_contains("evidence.defs", e, "DEFS.H:4258-4260", A_DEFS);
    ok &= expect_contains("evidence.coord", e, "COORD.C:780-877", A_COORD);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1892-1900",
                          A_LINEAGE);
    ok &= expect_contains("struct.lineage_open", ev ? ev->lineage_open : NULL,
                          "DUNVIEW.C:7520-7536", A_D1L);
    ok &= expect_contains("struct.lineage_teleporter",
                          ev ? ev->lineage_teleporter : NULL, "DUNVIEW.C:7542-7555",
                          A_D1L);
    ok &= expect_contains("struct.frame_binding", ev ? ev->frame_binding : NULL,
                          "frame-blt/frame-rect", A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d1l_d1r_f0111_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d1l_d1r_f0111_door_source_evidence_pc34());

    ok &= test_identity_and_scope();
    ok &= test_d1l_d1r_route_contracts();
    ok &= test_frame_door_and_field_constants();
    ok &= test_f0111_state_zone_contract();
    ok &= test_horizontal_half_and_c10_synthetic_blit();
    ok &= test_evidence_strings();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_25", g_assertions >= 25, 1,
                     A_F0111);

    if (ok) {
        printf("PASS csb_v1_viewport_d1l_d1r_f0111_door_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
