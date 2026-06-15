#include "csb_v1_viewport_d0l_d0r_f0111_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Chosen slice: D0L/D0R F0111 door boundary. The label stays in the F0111
 * lane because it locks the D0 lateral ring as a source-proven absence:
 * ReDMCSB DUNVIEW.C:7976-8060 and 8080-8159 dispatch D0L/D0R corridor,
 * door-side, and teleporter elements through F0115 only; DUNVIEW.C:3502-3938
 * F0107 and 4218-4337 F0111 are not called there. CSBWin Viewport.cpp:
 * 1903-1915 is the positive center-door StdDrawDoor contrast, while
 * CSB-lineage Viewport.cpp:1930-1944 returns for F0L1/F0R1 and lines
 * 6503-6551 keep CustomBackgrounds/ApplyDecoration out of this route.
 */

static const char *A_D0L =
    "ReDMCSB DUNVIEW.C:7976-8060 F0125_DUNGEONVIEW_DrawSquareD0L";
static const char *A_D0R =
    "ReDMCSB DUNVIEW.C:8080-8159 F0126_DUNGEONVIEW_DrawSquareD0R";
static const char *A_F0107 =
    "ReDMCSB DUNVIEW.C:3502-3938 F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF";
static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2596-2601,2659-2660,4056-4057,4217-4219,4250-4260";
static const char *A_CSBWIN_CENTER =
    "CSBWin Viewport.cpp:1903-1915 StdDrawF1DoorFacing center-door dispatch";
static const char *A_CSB_LINEAGE =
    "CSB-lineage Viewport.cpp:1930-1944 D0 side returns; 6503-6551 CustomBackgrounds";

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
    return expect_int(label, haystack && needle &&
                         strstr(haystack, needle) != NULL, 1, anchor);
}

static int test_identity_and_scope(void)
{
    int ok = 1;
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *d0l =
        csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(1);
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *d0r =
        csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(2);

    ok &= expect_int("route.count",
                     (int)csb_v1_viewport_d0l_d0r_f0111_door_pc34_count(), 2,
                     A_DEFS);
    ok &= expect_int("d0l.present", d0l != NULL, 1, A_D0L);
    ok &= expect_int("d0r.present", d0r != NULL, 1, A_D0R);
    ok &= expect_int("unknown.square",
                     csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(9) == NULL,
                     1, A_DEFS);
    ok &= expect_int("index0.matches.d0l",
                     csb_v1_viewport_d0l_d0r_f0111_door_pc34_at(0) == d0l,
                     1, A_D0L);
    ok &= expect_int("index2.null",
                     csb_v1_viewport_d0l_d0r_f0111_door_pc34_at(2) == NULL,
                     1, A_DEFS);
    ok &= expect_int("contract.only", d0l ? d0l->source_locked_contract_only : 0,
                     1, A_F0111);
    ok &= expect_int("no.real.asset.parity",
                     d0l ? d0l->no_real_asset_bitmap_parity : 0, 1, A_F0111);
    ok &= expect_int("no.game.data.load", d0l ? d0l->no_game_data_load : 0,
                     1, A_F0111);

    return ok;
}

static int test_d0l_d0r_geometry_and_orders(void)
{
    int ok = 1;
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *d0l =
        csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(1);
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *d0r =
        csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(2);

    ok &= expect_int("d0l.view_square", d0l ? d0l->view_square : -1, 1, A_DEFS);
    ok &= expect_int("d0r.view_square", d0r ? d0r->view_square : -1, 2, A_DEFS);
    ok &= expect_int("d0l.depth", d0l ? d0l->view_depth : -1, 0,
                     "ReDMCSB DUNVIEW.C:372 G2027[1]");
    ok &= expect_int("d0r.depth", d0r ? d0r->view_depth : -1, 0,
                     "ReDMCSB DUNVIEW.C:372 G2027[2]");
    ok &= expect_int("d0l.lane", d0l ? d0l->view_lane : 99, -1,
                     "ReDMCSB DUNVIEW.C:371 G2026[1]");
    ok &= expect_int("d0r.lane", d0r ? d0r->view_lane : 99, 1,
                     "ReDMCSB DUNVIEW.C:371 G2026[2]");
    ok &= expect_int("d0l.f0115.order", d0l ? d0l->f0115_cell_order : -1,
                     0x0002, A_D0L);
    ok &= expect_int("d0r.f0115.order", d0r ? d0r->f0115_cell_order : -1,
                     0x0001, A_D0R);
    ok &= expect_int("d0l.wall.zone", d0l ? d0l->wall_zone : -1, 716, A_DEFS);
    ok &= expect_int("d0r.wall.zone", d0r ? d0r->wall_zone : -1, 717, A_DEFS);
    ok &= expect_int("d0l.field.zone", d0l ? d0l->field_zone : -1, 716, A_D0L);
    ok &= expect_int("d0r.field.zone", d0r ? d0r->field_zone : -1, 717, A_D0R);
    ok &= expect_int("d0l.ceiling.zone", d0l ? d0l->ceiling_pit_zone : -1,
                     870, A_D0L);
    ok &= expect_int("d0r.ceiling.zone", d0r ? d0r->ceiling_pit_zone : -1,
                     872, A_D0R);

    return ok;
}

static int test_f0111_f0107_negative_boundary(void)
{
    int ok = 1;
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *d0l =
        csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(1);
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *d0r =
        csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(2);

    ok &= expect_int("d0l.no.f0111", d0l ? d0l->f0111_route_present : 1, 0,
                     A_F0111);
    ok &= expect_int("d0r.no.f0111", d0r ? d0r->f0111_route_present : 1, 0,
                     A_F0111);
    ok &= expect_int("d0l.no.f0107", d0l ? d0l->f0107_route_present : 1, 0,
                     A_F0107);
    ok &= expect_int("d0r.no.f0107", d0r ? d0r->f0107_route_present : 1, 0,
                     A_F0107);
    ok &= expect_int("d0l.no.door.zone", d0l ? d0l->door_zone_present : 1, 0,
                     A_DEFS);
    ok &= expect_int("d0r.no.door.zone", d0r ? d0r->door_zone_present : 1, 0,
                     A_DEFS);
    ok &= expect_int("d0l.no.frame.zone",
                     d0l ? d0l->door_frame_zone_present : 1, 0, A_D0L);
    ok &= expect_int("d0r.no.frame.zone",
                     d0r ? d0r->door_frame_zone_present : 1, 0, A_D0R);
    ok &= expect_int("d0l.corridor.rejects.f0111",
                     csb_v1_viewport_d0l_d0r_f0111_door_allows_f0111_pc34(d0l, 1),
                     0, A_D0L);
    ok &= expect_int("d0l.door_side.rejects.f0111",
                     csb_v1_viewport_d0l_d0r_f0111_door_allows_f0111_pc34(d0l, 16),
                     0, A_D0L);
    ok &= expect_int("d0r.teleporter.rejects.f0111",
                     csb_v1_viewport_d0l_d0r_f0111_door_allows_f0111_pc34(d0r, 5),
                     0, A_D0R);
    ok &= expect_int("wall.rejects.f0111",
                     csb_v1_viewport_d0l_d0r_f0111_door_allows_f0111_pc34(d0r, 0),
                     0, A_F0111);
    ok &= expect_int("null.rejects.f0111",
                     csb_v1_viewport_d0l_d0r_f0111_door_allows_f0111_pc34(NULL, 16),
                     0, A_F0111);

    return ok;
}

static int test_cross_source_lineage_boundaries(void)
{
    int ok = 1;
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *d0l =
        csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(1);
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *d0r =
        csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(2);

    ok &= expect_int("d0l.f0115.corridor", d0l ? d0l->corridor_routes_f0115 : 0,
                     1, A_D0L);
    ok &= expect_int("d0r.f0115.corridor", d0r ? d0r->corridor_routes_f0115 : 0,
                     1, A_D0R);
    ok &= expect_int("d0l.f0115.door_side", d0l ? d0l->door_side_routes_f0115 : 0,
                     1, A_D0L);
    ok &= expect_int("d0r.f0115.door_side", d0r ? d0r->door_side_routes_f0115 : 0,
                     1, A_D0R);
    ok &= expect_int("d0l.csbwin.return", d0l ? d0l->csbwin_d0_side_returns : 0,
                     1, A_CSB_LINEAGE);
    ok &= expect_int("d0r.csbwin.return", d0r ? d0r->csbwin_d0_side_returns : 0,
                     1, A_CSB_LINEAGE);
    ok &= expect_int("center.door.contrast",
                     d0l ? d0l->csbwin_center_door_is_contrast : 0, 1,
                     A_CSBWIN_CENTER);
    ok &= expect_int("custom.backgrounds.separate",
                     d0r ? d0r->custom_backgrounds_are_separate : 0, 1,
                     A_CSB_LINEAGE);
    ok &= expect_contains("route.label.d0l",
                          csb_v1_viewport_d0l_d0r_f0111_door_route_label_pc34(d0l),
                          "D0L F0111 absence", A_D0L);
    ok &= expect_int("route.label.null",
                     csb_v1_viewport_d0l_d0r_f0111_door_route_label_pc34(NULL) == NULL,
                     1, A_D0R);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d0l_d0r_f0111_door_source_evidence_pc34();
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatEvidence *ev =
        csb_v1_viewport_d0l_d0r_f0111_door_evidence_pc34();

    ok &= expect_contains("evidence.choice", e, "Chosen slice: D0L/D0R", A_D0L);
    ok &= expect_contains("evidence.d0l", e, "DUNVIEW.C:7976-8060", A_D0L);
    ok &= expect_contains("evidence.d0r", e, "DUNVIEW.C:8080-8159", A_D0R);
    ok &= expect_contains("evidence.f0107", e, "DUNVIEW.C:3502-3938", A_F0107);
    ok &= expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4337", A_F0111);
    ok &= expect_contains("evidence.center", e, "Viewport.cpp:1903-1915",
                          A_CSBWIN_CENTER);
    ok &= expect_contains("evidence.d0.return", e, "Viewport.cpp:1930-1944",
                          A_CSB_LINEAGE);
    ok &= expect_contains("evidence.custom", e, "Viewport.cpp:6503-6551",
                          A_CSB_LINEAGE);
    ok &= expect_contains("struct.f0111", ev ? ev->f0111_lines : NULL,
                          "F0111_DUNGEONVIEW_DrawDoor", A_F0111);
    ok &= expect_contains("struct.custom",
                          ev ? ev->csb_lineage_custom_backgrounds_lines : NULL,
                          "6503-6551", A_CSB_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d0l_d0r_f0111_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d0l_d0r_f0111_door_source_evidence_pc34());

    ok &= test_identity_and_scope();
    ok &= test_d0l_d0r_geometry_and_orders();
    ok &= test_f0111_f0107_negative_boundary();
    ok &= test_cross_source_lineage_boundaries();
    ok &= test_evidence_strings();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_45", g_assertions >= 45, 1,
                     A_F0111);

    if (ok) {
        printf("PASS csb_v1_viewport_d0l_d0r_f0111_door_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
