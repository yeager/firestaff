#include "csb/csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor";
static const char *A_F0111_PARTLY =
    "ReDMCSB DUNVIEW.C:4308-4325 F0111 partly-open door-frame branch";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C:8503-8508 F0128 D2L2/D2R2 dispatch";
static const char *A_D2L2 =
    "ReDMCSB DUNVIEW.C:6837-6865 F0678_DrawD2L2";
static const char *A_D2R2 =
    "ReDMCSB DUNVIEW.C:6868-6896 F0679_DrawD2R2";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088,2605-2606,3508,3516,4047-4048,4228-4230";
static const char *A_COORD =
    "ReDMCSB COORD.C:788-797 and 1556-1559";
static const char *A_LINEAGE =
    "CSB Viewport.cpp:1903-1906 CSB-lineage room-object overlay";
static const char *A_NO_PIXEL =
    "NO-CLAIM real-asset pixel parity marker";

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

static int expect_true(const char *label, int got, const char *anchor)
{
    return expect_int(label, got ? 1 : 0, 1, anchor);
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_true(label, haystack && needle && strstr(haystack, needle),
                       anchor);
}

static int test_spec_identity(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(10);

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_count_pc34(),
                     2, A_F0128);
    ok &= expect_true("spec.d2l2.present", d2l2 != 0,
                      "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_true("spec.d2r2.present", d2r2 != 0,
                      "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    ok &= expect_true("spec.unknown.absent",
                      csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(14) == 0,
                      A_F0128);
    ok &= expect_true("spec.at0.d2l2",
                      csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_at_pc34(0) == d2l2,
                      A_D2L2);
    ok &= expect_true("spec.at1.d2r2",
                      csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_at_pc34(1) == d2r2,
                      A_D2R2);
    ok &= expect_true("spec.at2.null",
                      csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_at_pc34(2) == 0,
                      A_F0128);
    ok &= expect_int("d2l2.contract_only",
                     d2l2 ? d2l2->source_locked_contract_only : -1, 1, A_F0111);
    ok &= expect_int("d2r2.contract_only",
                     d2r2 ? d2r2->source_locked_contract_only : -1, 1, A_F0111);
    ok &= expect_int("d2l2.no_game_data",
                     d2l2 ? d2l2->no_game_data_load : -1, 1, A_NO_PIXEL);
    ok &= expect_int("d2r2.no_real_asset_pixel_parity",
                     d2r2 ? d2r2->no_real_asset_pixel_parity : -1, 1, A_NO_PIXEL);

    return ok;
}

static int test_f0128_d2_bindings(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(10);

    ok &= expect_int("d2l2.view_square", d2l2 ? d2l2->view_square : -1, 9,
                     "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_int("d2r2.view_square", d2r2 ? d2r2->view_square : -1, 10,
                     "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    ok &= expect_int("d2l2.dispatch_order", d2l2 ? d2l2->f0128_dispatch_order : -1,
                     8, A_F0128);
    ok &= expect_int("d2r2.dispatch_order", d2r2 ? d2r2->f0128_dispatch_order : -1,
                     9, A_F0128);
    ok &= expect_int("d2l2.depth", d2l2 ? d2l2->f0128_relative_depth : -1,
                     2, A_F0128);
    ok &= expect_int("d2r2.depth", d2r2 ? d2r2->f0128_relative_depth : -1,
                     2, A_F0128);
    ok &= expect_int("d2l2.lane", d2l2 ? d2l2->f0128_relative_lane : 0,
                     -2, A_F0128);
    ok &= expect_int("d2r2.lane", d2r2 ? d2r2->f0128_relative_lane : 0,
                     2, A_F0128);
    ok &= expect_int("d2l2.wall_zone", d2l2 ? d2l2->wall_zone_binding : -1,
                     707, "ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2");
    ok &= expect_int("d2r2.wall_zone", d2r2 ? d2r2->wall_zone_binding : -1,
                     708, "ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2");
    ok &= expect_int("d2l2.no_direct_f0111",
                     d2l2 ? d2l2->f0678_f0679_direct_f0111_route_present : -1,
                     0, A_D2L2);
    ok &= expect_int("d2r2.no_direct_f0111",
                     d2r2 ? d2r2->f0678_f0679_direct_f0111_route_present : -1,
                     0, A_D2R2);
    ok &= expect_int("d2l2.wall_returns",
                     d2l2 ? d2l2->wall_case_returns_before_f0111 : -1,
                     1, "ReDMCSB DUNVIEW.C:6862 D2L2 wall return");
    ok &= expect_int("d2r2.wall_returns",
                     d2r2 ? d2r2->wall_case_returns_before_f0111 : -1,
                     1, "ReDMCSB DUNVIEW.C:6893 D2R2 wall return");

    return ok;
}

static int test_nonduplication_markers(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(9);

    ok &= expect_int("not.front_clipped_gate",
                     spec ? spec->excludes_existing_front_clipped_gate : -1, 1,
                     "existing CSB D2L2/D2R2 F0111 front-clipped gate");
    ok &= expect_int("not.partly_open_gate",
                     spec ? spec->excludes_existing_partly_open_gate : -1, 1,
                     "existing csb_v1_viewport_d2l2_d2r2_f0111_partly_open_pc34_compat gate");
    ok &= expect_int("not.wall_gate",
                     spec ? spec->excludes_existing_wall_gate : -1, 1,
                     "existing CSB D2L2/D2R2 wall gate");
    ok &= expect_int("excludes.c2500_object_base",
                     spec ? spec->excludes_c2500_object_base : -1, 2500,
                     "ReDMCSB DEFS.H:4228 C2500_ZONE_");
    ok &= expect_int("excludes.c2900_projectile_base",
                     spec ? spec->excludes_c2900_projectile_base : -1, 2900,
                     "ReDMCSB DEFS.H:4230 C2900_ZONE_");
    ok &= expect_int("lineage.marker",
                     spec && spec->lineage_anchor ? strstr(spec->lineage_anchor, "1903-1906") != 0 : 0,
                     1, A_LINEAGE);
    ok &= expect_int("no_claim.pixel_marker",
                     spec ? spec->no_real_asset_pixel_parity : -1, 1, A_NO_PIXEL);

    return ok;
}

static int test_f0111_branch_and_zone_math(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(9);

    ok &= expect_int("branch.open",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(spec, 0),
                     0, "ReDMCSB DUNVIEW.C:4248 F0111 open skip");
    ok &= expect_int("branch.partly1",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(spec, 1),
                     1, A_F0111_PARTLY);
    ok &= expect_int("branch.partly2",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(spec, 2),
                     1, A_F0111_PARTLY);
    ok &= expect_int("branch.partly3",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(spec, 3),
                     1, A_F0111_PARTLY);
    ok &= expect_int("branch.closed",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(spec, 4),
                     2, "ReDMCSB DUNVIEW.C:4297-4299 closed branch");
    ok &= expect_int("branch.destroyed",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(spec, 5),
                     3, "ReDMCSB DUNVIEW.C:4301-4304 destroyed branch");
    ok &= expect_int("branch.invalid",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(spec, 6),
                     -1, A_F0111);
    ok &= expect_int("first.open.skip",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_first_half_zone_pc34(spec, 0, 1),
                     -1, "ReDMCSB DUNVIEW.C:4248 F0111 open skip");
    ok &= expect_int("first.state1",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_first_half_zone_pc34(spec, 1, 1),
                     3707, "ReDMCSB DUNVIEW.C:4322 P2084+state+C6");
    ok &= expect_int("first.state2",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_first_half_zone_pc34(spec, 2, 1),
                     3708, "ReDMCSB DUNVIEW.C:4322 P2084+state+C6");
    ok &= expect_int("first.vertical_absent",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_first_half_zone_pc34(spec, 2, 0),
                     -1, "ReDMCSB DUNVIEW.C:4319 horizontal-only first half");
    ok &= expect_int("final.open.skip",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_final_half_zone_pc34(spec, 0, 1),
                     -1, "ReDMCSB DUNVIEW.C:4248 F0111 open skip");
    ok &= expect_int("final.state1.horizontal",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_final_half_zone_pc34(spec, 1, 1),
                     20088, "ReDMCSB DUNVIEW.C:4325 state+3|MASK0x4000");
    ok &= expect_int("final.state2.horizontal",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_final_half_zone_pc34(spec, 2, 1),
                     20089, "ReDMCSB DUNVIEW.C:4325 state+3|MASK0x4000");
    ok &= expect_int("final.state2.vertical",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_final_half_zone_pc34(spec, 2, 0),
                     3702, "ReDMCSB DUNVIEW.C:4317-4318 vertical state zone");
    ok &= expect_int("final.closed.base",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_final_half_zone_pc34(spec, 4, 1),
                     3700, "ReDMCSB DUNVIEW.C:4297-4299 closed branch");
    ok &= expect_int("final.destroyed.base",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_final_half_zone_pc34(spec, 5, 1),
                     3700, "ReDMCSB DUNVIEW.C:4301-4304 destroyed branch");
    ok &= expect_int("null.branch",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(0, 2),
                     -1, A_F0111);

    return ok;
}

static int test_defs_and_coord_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(9);

    ok &= expect_int("defs.c10.macro",
                     CSB_V1_D2L2_D2R2_F0111_PARTLY_OPEN_DOOR_C10_COLOR_FLESH,
                     10, "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("defs.mask.macro",
                     CSB_V1_D2L2_D2R2_F0111_PARTLY_OPEN_DOOR_MASK0X4000,
                     0x4000, "ReDMCSB DEFS.H:3516 MASK0x4000");
    ok &= expect_int("defs.c10.spec", spec ? spec->transparent_color : -1,
                     10, "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("defs.c6.first_half_offset",
                     spec ? spec->first_half_zone_offset : -1,
                     6, "ReDMCSB DEFS.H:3508 C6_UNKNOWN");
    ok &= expect_int("defs.c6.source_mask_offset",
                     spec ? spec->first_half_source_mask_zone_offset : -1,
                     6, "ReDMCSB DUNVIEW.C:4322 zone + C6_UNKNOWN");
    ok &= expect_int("defs.final_offset",
                     spec ? spec->final_half_zone_offset : -1,
                     3, "ReDMCSB DUNVIEW.C:4325 final half +3");
    ok &= expect_int("defs.final_mask",
                     spec ? spec->final_half_mask : -1,
                     0x4000, "ReDMCSB DEFS.H:3516 MASK0x4000");
    ok &= expect_int("coord.parent_record",
                     spec ? spec->coord_parent_record : -1,
                     129, "ReDMCSB COORD.C:788 and 1559");
    ok &= expect_int("coord.clip_record",
                     spec ? spec->coord_clip_record : -1,
                     126, "ReDMCSB COORD.C:1556 and 1559");
    ok &= expect_int("coord.frame_x",
                     spec ? spec->coord_frame_x : -1,
                     24, "ReDMCSB COORD.C:1559");
    ok &= expect_int("coord.frame_y",
                     spec ? spec->coord_frame_y : -1,
                     28, "ReDMCSB COORD.C:1559");
    ok &= expect_int("coord.clipped_width",
                     spec ? spec->clipped_width : -1,
                     48, "ReDMCSB COORD.C:1556");
    ok &= expect_int("coord.clipped_height",
                     spec ? spec->clipped_height : -1,
                     40, "ReDMCSB COORD.C:1556");
    ok &= expect_int("viewport.width",
                     spec ? spec->viewport_width : -1,
                     224, "synthetic 224x136 viewport contract");
    ok &= expect_int("viewport.height",
                     spec ? spec->viewport_height : -1,
                     136, "synthetic 224x136 viewport contract");
    ok &= expect_int("framebuffer.width",
                     spec ? spec->framebuffer_width : -1,
                     320, "synthetic 320x200 buffer contract");
    ok &= expect_int("framebuffer.height",
                     spec ? spec->framebuffer_height : -1,
                     200, "synthetic 320x200 buffer contract");
    ok &= expect_int("c2600.literal_absent",
                     spec ? spec->c2600_literal_symbol_present : -1,
                     0, "C2600_DOOR_PARTLY_OPEN_BITMAP absent in ReDMCSB Common/Source");
    ok &= expect_contains("c2600.anchor",
                          spec ? spec->c2600_anchor : 0,
                          "DUNVIEW.C:4308-4313",
                          "ReDMCSB DUNVIEW.C:4308-4313 actual bitmap-selection anchor");

    return ok;
}

static int test_synthetic_clipped_c10_blit(void)
{
    int ok = 1;
    uint8_t source[12] = { 10, 1, 2, 10, 3, 4, 10, 5, 6, 10, 7, 8 };
    uint8_t framebuffer[320 * 200];
    int skipped = -1;
    int left_writes = -1;
    int right_writes = -1;
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(9);

    for (int i = 0; i < 320 * 200; ++i) framebuffer[i] = 77;

    ok &= expect_int("blit.copied.left_clip",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_synthetic_blit_pc34(
                         spec, 2, -1, 2, source, 4, 3, 4, framebuffer,
                         320, 200, &skipped, &left_writes, &right_writes),
                     6, "ReDMCSB DUNVIEW.C:4334 C10 clipped blit");
    ok &= expect_int("blit.skipped_c10", skipped, 3,
                     "ReDMCSB DEFS.H:2088 C10 transparency");
    ok &= expect_int("blit.left_edge_writes", left_writes, 2,
                     "clipped edge writes inside 224x136 viewport");
    ok &= expect_int("blit.right_edge_writes", right_writes, 0,
                     "clipped edge writes inside 224x136 viewport");
    ok &= expect_int("blit.left_edge.pixel", framebuffer[(2 * 320) + 0], 1,
                     "synthetic clipped edge write");
    ok &= expect_int("blit.transparent.preserved", framebuffer[(2 * 320) + 2], 77,
                     "ReDMCSB DEFS.H:2088 C10 transparency");
    ok &= expect_int("blit.open.skip",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_synthetic_blit_pc34(
                         spec, 0, 0, 0, source, 4, 3, 4, framebuffer,
                         320, 200, &skipped, &left_writes, &right_writes),
                     0, "ReDMCSB DUNVIEW.C:4248 F0111 skips open door");
    ok &= expect_int("blit.reject_oversize",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_synthetic_blit_pc34(
                         spec, 2, 0, 0, source, 49, 1, 49, framebuffer,
                         320, 200, 0, 0, 0),
                     -1, "ReDMCSB COORD.C:1556 48x40 clip");
    ok &= expect_int("blit.reject_bad_stride",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_synthetic_blit_pc34(
                         spec, 2, 0, 0, source, 4, 3, 3, framebuffer,
                         320, 200, 0, 0, 0),
                     -1, "synthetic source stride guard");
    ok &= expect_int("blit.reject_wrong_framebuffer",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_synthetic_blit_pc34(
                         spec, 2, 0, 0, source, 4, 3, 4, framebuffer,
                         224, 136, 0, 0, 0),
                     -1, "synthetic 320x200 buffer contract");

    return ok;
}

static int test_probe_and_evidence(void)
{
    int ok = 1;
    CSB_V1_ViewportD2L2D2R2F0111PartlyOpenDoorProbePc34 probe;
    const char *e =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_source_evidence_pc34();
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(9);

    ok &= expect_int("probe.run",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_probe_pc34_compat(&probe),
                     0, "synthetic 224x136 / 320x200 probe");
    ok &= expect_int("probe.route_count", probe.route_count, 2, A_F0128);
    ok &= expect_true("probe.copied_positive", probe.copied_pixels > 0,
                      "ReDMCSB DUNVIEW.C:4334 C10 blit");
    ok &= expect_true("probe.c10_skipped_positive", probe.c10_skipped_pixels > 0,
                      "ReDMCSB DEFS.H:2088 C10 transparency");
    ok &= expect_true("probe.left_edge", probe.left_edge_writes > 0,
                      "clipped edge writes");
    ok &= expect_int("probe.right_edge", probe.right_edge_writes, 0,
                     "left-clipped synthetic fixture");
    ok &= expect_int("probe.first_half_zone", probe.first_half_zone, 3708,
                     "ReDMCSB DUNVIEW.C:4322 P2084+state+C6");
    ok &= expect_int("probe.final_half_zone", probe.final_half_zone, 20089,
                     "ReDMCSB DUNVIEW.C:4325 state+3|MASK0x4000");
    ok &= expect_int("probe.inside_224x136", probe.clipped_write_inside_224x136,
                     1, "synthetic 224x136 viewport contract");
    ok &= expect_int("probe.no_real_asset_pixel_parity",
                     probe.no_real_asset_pixel_parity, 1, A_NO_PIXEL);
    ok &= expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4337", A_F0111);
    ok &= expect_contains("evidence.f0128", e, "DUNVIEW.C:8503-8508", A_F0128);
    ok &= expect_contains("evidence.d2", e, "DUNVIEW.C:6837-6896",
                          "ReDMCSB DUNVIEW.C:6837-6896");
    ok &= expect_contains("evidence.defs", e, "DEFS.H:2088", A_DEFS);
    ok &= expect_contains("evidence.coord", e, "COORD.C:788-797", A_COORD);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1903-1906",
                          A_LINEAGE);
    ok &= expect_contains("evidence.no_pixel", e, "no real-asset pixel parity",
                          A_NO_PIXEL);
    ok &= expect_contains("evidence.c2600_absent", e,
                          "C2600_DOOR_PARTLY_OPEN_BITMAP is absent",
                          "missing literal symbol marker");
    ok &= expect_contains("spec.f0111_anchor", spec ? spec->f0111_anchor : 0,
                          "4218-4337", A_F0111);
    ok &= expect_contains("spec.f0128_anchor", spec ? spec->f0128_anchor : 0,
                          "8503-8508", A_F0128);
    ok &= expect_contains("spec.defs_anchor", spec ? spec->defs_anchor : 0,
                          "4228-4230", A_DEFS);
    ok &= expect_contains("spec.coord_anchor", spec ? spec->coord_anchor : 0,
                          "1556-1559", A_COORD);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_source_evidence_pc34());

    ok &= test_spec_identity();
    ok &= test_f0128_d2_bindings();
    ok &= test_nonduplication_markers();
    ok &= test_f0111_branch_and_zone_math();
    ok &= test_defs_and_coord_contract();
    ok &= test_synthetic_clipped_c10_blit();
    ok &= test_probe_and_evidence();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_true("assertion_count_at_least_60", g_assertions >= 60,
                      "assigned CSB D2L2/D2R2 F0111 partly-open door gate");

    if (ok) {
        printf("PASS csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
