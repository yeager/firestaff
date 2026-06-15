#include "csb_v1_viewport_d2l2_d2r2_f0111_partly_open_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C:8503-8508 F0128 D2L2/D2R2 dispatch";
static const char *A_D2L2 =
    "ReDMCSB DUNVIEW.C:6837-6865 F0678_DrawD2L2";
static const char *A_D2R2 =
    "ReDMCSB DUNVIEW.C:6868-6896 F0679_DrawD2R2";
static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor";
static const char *A_PARTLY =
    "ReDMCSB DUNVIEW.C:4317-4325 F0111 partly-open horizontal door math";
static const char *A_COORD_RANGE =
    "ReDMCSB COORD.C:788-797 C3700 per-state door records";
static const char *A_COORD_CLIP =
    "ReDMCSB COORD.C:1556-1559 C03 record 126/129 door-panel clip";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088,2605-2606,3508,3516,4047-4048,4250";
static const char *A_C10 =
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH and DUNVIEW.C:4334 F0111";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1813-1820 StdDrawF3L1DoorFacing binding";

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

static int test_identity_and_d2_bindings(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(10);

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d2l2_d2r2_f0111_partly_open_count_pc34(),
                     2, A_F0128);
    ok &= expect_int("d2l2.present", d2l2 != NULL, 1,
                     "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_int("d2r2.present", d2r2 != NULL, 1,
                     "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    ok &= expect_int("unknown.square",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(14) == NULL,
                     1, A_F0128);
    ok &= expect_int("index0.d2l2",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_at_pc34(0) == d2l2,
                     1, A_D2L2);
    ok &= expect_int("index2.null",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_at_pc34(2) == NULL,
                     1, A_F0128);
    ok &= expect_int("d2l2.contract_only",
                     d2l2 ? d2l2->source_locked_contract_only : 0, 1, A_F0111);
    ok &= expect_int("d2r2.contract_only",
                     d2r2 ? d2r2->source_locked_contract_only : 0, 1, A_F0111);
    ok &= expect_int("d2l2.no_real_asset_pixel_parity",
                     d2l2 ? d2l2->no_real_asset_pixel_parity : 0, 1, A_F0111);
    ok &= expect_int("d2r2.no_real_asset_pixel_parity",
                     d2r2 ? d2r2->no_real_asset_pixel_parity : 0, 1, A_F0111);
    ok &= expect_int("d2l2.view_square", d2l2 ? d2l2->view_square : -1, 9,
                     "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_int("d2r2.view_square", d2r2 ? d2r2->view_square : -1, 10,
                     "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    ok &= expect_int("d2l2.draw_order", d2l2 ? d2l2->f0128_draw_order_index : -1,
                     8, A_F0128);
    ok &= expect_int("d2r2.draw_order", d2r2 ? d2r2->f0128_draw_order_index : -1,
                     9, A_F0128);
    ok &= expect_int("d2l2.depth", d2l2 ? d2l2->f0128_relative_depth : -1, 2,
                     A_F0128);
    ok &= expect_int("d2r2.depth", d2r2 ? d2r2->f0128_relative_depth : -1, 2,
                     A_F0128);
    ok &= expect_int("d2l2.lateral", d2l2 ? d2l2->f0128_relative_lateral : 0,
                     -2, A_F0128);
    ok &= expect_int("d2r2.lateral", d2r2 ? d2r2->f0128_relative_lateral : 0,
                     2, A_F0128);
    ok &= expect_int("d2l2.wall_zone", d2l2 ? d2l2->wall_zone_binding : -1,
                     707, "ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2");
    ok &= expect_int("d2r2.wall_zone", d2r2 ? d2r2->wall_zone_binding : -1,
                     708, "ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2");

    return ok;
}

static int test_non_routes_and_lineage_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(10);

    ok &= expect_int("d2l2.direct_f0111_absent",
                     d2l2 ? d2l2->direct_f0111_route_present : -1, 0, A_D2L2);
    ok &= expect_int("d2r2.direct_f0111_absent",
                     d2r2 ? d2r2->direct_f0111_route_present : -1, 0, A_D2R2);
    ok &= expect_int("d2l2.direct_f0115_rear_front_absent",
                     d2l2 ? d2l2->direct_f0115_rear_front_route_present : -1, 0,
                     A_D2L2);
    ok &= expect_int("d2r2.direct_f0115_rear_front_absent",
                     d2r2 ? d2r2->direct_f0115_rear_front_route_present : -1, 0,
                     A_D2R2);
    ok &= expect_int("d2l2.no_backwall_f0107",
                     d2l2 ? d2l2->back_wall_ornament_f0107_route_present : -1, 0,
                     A_D2L2);
    ok &= expect_int("d2r2.no_backwall_f0107",
                     d2r2 ? d2r2->back_wall_ornament_f0107_route_present : -1, 0,
                     A_D2R2);
    ok &= expect_int("d2l2.wall_returns",
                     d2l2 ? d2l2->wall_case_returns_before_f0111 : -1, 1,
                     "ReDMCSB DUNVIEW.C:6862 D2L2 wall return");
    ok &= expect_int("d2r2.wall_returns",
                     d2r2 ? d2r2->wall_case_returns_before_f0111 : -1, 1,
                     "ReDMCSB DUNVIEW.C:6893 D2R2 wall return");
    ok &= expect_int("d2l2.excludes_c3700_dispatch",
                     d2l2 ? d2l2->c3700_d3_door_zone_metadata_excluded : -1, 1,
                     "ReDMCSB DEFS.H:4250 C3700 excluded by D2 dispatchers");
    ok &= expect_int("d2r2.excludes_c3700_dispatch",
                     d2r2 ? d2r2->c3700_d3_door_zone_metadata_excluded : -1, 1,
                     "ReDMCSB DEFS.H:4250 C3700 excluded by D2 dispatchers");
    ok &= expect_int("lineage.binding_present",
                     d2l2 ? d2l2->lineage_f3l1_binding_present : -1, 1, A_LINEAGE);
    ok &= expect_int("lineage.frame_before_door",
                     d2l2 ? d2l2->lineage_frame_before_door : -1, 1, A_LINEAGE);
    ok &= expect_int("lineage.rear_order",
                     d2l2 ? d2l2->lineage_draw_order_rear : -1, 0x0218,
                     A_LINEAGE);
    ok &= expect_int("lineage.front_order",
                     d2l2 ? d2l2->lineage_draw_order_front : -1, 0x0349,
                     A_LINEAGE);

    return ok;
}

static int test_partly_open_zone_math(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(10);

    ok &= expect_int("d2l2.open.first_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_first_half_zone_pc34(
                         d2l2, 0, 1),
                     -1, "ReDMCSB DUNVIEW.C:4248 F0111 skips C0 open");
    ok &= expect_int("d2l2.open.final_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_final_zone_pc34(
                         d2l2, 0, 1),
                     -1, "ReDMCSB DUNVIEW.C:4248 F0111 skips C0 open");
    ok &= expect_int("d2l2.state1.first_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_first_half_zone_pc34(
                         d2l2, 1, 1),
                     3707, A_PARTLY);
    ok &= expect_int("d2l2.state1.final_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_final_zone_pc34(
                         d2l2, 1, 1),
                     20088, A_PARTLY);
    ok &= expect_int("d2l2.state1.vertical_final_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_final_zone_pc34(
                         d2l2, 1, 0),
                     3701, "ReDMCSB DUNVIEW.C:4317-4319 vertical state shift");
    ok &= expect_int("d2l2.state2.first_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_first_half_zone_pc34(
                         d2l2, 2, 1),
                     3708, A_PARTLY);
    ok &= expect_int("d2l2.state2.final_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_final_zone_pc34(
                         d2l2, 2, 1),
                     20089, A_PARTLY);
    ok &= expect_int("d2l2.closed.first_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_first_half_zone_pc34(
                         d2l2, 4, 1),
                     -1, "ReDMCSB DUNVIEW.C:4297-4299 closed door base draw");
    ok &= expect_int("d2l2.closed.final_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_final_zone_pc34(
                         d2l2, 4, 1),
                     3700, "ReDMCSB DUNVIEW.C:4297-4299 closed door base draw");
    ok &= expect_int("d2r2.state1.first_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_first_half_zone_pc34(
                         d2r2, 1, 1),
                     3707, A_PARTLY);
    ok &= expect_int("d2r2.state2.final_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_final_zone_pc34(
                         d2r2, 2, 1),
                     20089, A_PARTLY);
    ok &= expect_int("d2r2.closed.final_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_final_zone_pc34(
                         d2r2, 4, 0),
                     3700, "ReDMCSB DUNVIEW.C:4297-4299 closed door base draw");
    ok &= expect_int("null.first_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_first_half_zone_pc34(
                         NULL, 2, 1),
                     -1, A_F0111);
    ok &= expect_int("bad_state.final_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_final_zone_pc34(
                         d2l2, -1, 1),
                     -1, A_F0111);

    return ok;
}

static int test_coord_clip_path(void)
{
    int ok = 1;
    int record_type = -1;
    int x = -1;
    int y = -1;
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(9);
    const CSB_V1_ViewportDoorPanelBlitSpec *context =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_context_panel_pc34();

    ok &= expect_int("coord.closed_record_type",
                     spec ? spec->coord_closed_record_type : -1, 1, A_COORD_RANGE);
    ok &= expect_int("coord.parent_record", spec ? spec->coord_parent_record : -1,
                     129, A_COORD_CLIP);
    ok &= expect_int("coord.clip_record", spec ? spec->coord_clip_record : -1,
                     126, A_COORD_CLIP);
    ok &= expect_int("coord.frame_x", spec ? spec->coord_frame_x : -1, 24,
                     A_COORD_CLIP);
    ok &= expect_int("coord.frame_y", spec ? spec->coord_frame_y : -1, 28,
                     A_COORD_CLIP);
    ok &= expect_int("coord.native_width", spec ? spec->native_bitmap_width : -1,
                     48, "ReDMCSB COORD.C:1550 native door bitmap width");
    ok &= expect_int("coord.native_height", spec ? spec->native_bitmap_height : -1,
                     41, "ReDMCSB COORD.C:1550 native door bitmap height");
    ok &= expect_int("coord.clipped_width", spec ? spec->clipped_width : -1,
                     48, A_COORD_CLIP);
    ok &= expect_int("coord.clipped_height", spec ? spec->clipped_height : -1,
                     40, A_COORD_CLIP);
    ok &= expect_int("context.panel.present", context != NULL, 1,
                     "existing CSB viewport context: ReDMCSB DUNVIEW.C:4218-4337");
    ok &= expect_int("context.panel.zone", context ? context->door_zone_base : -1,
                     3700, "existing CSB viewport context: DEFS.H:4250");
    ok &= expect_int("context.panel.clip_height", context ? context->clipped_height : -1,
                     40, "existing CSB viewport context: COORD.C:1556");

    ok &= expect_int("coord.closed.resolve",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_coord_for_zone_pc34(
                         spec, 3700, &record_type, &x, &y),
                     0, A_COORD_RANGE);
    ok &= expect_int("coord.closed.type", record_type, 1, A_COORD_RANGE);
    ok &= expect_int("coord.closed.x", x, 0, A_COORD_RANGE);
    ok &= expect_int("coord.closed.y", y, 0, A_COORD_RANGE);
    ok &= expect_int("coord.state1.first.resolve",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_coord_for_zone_pc34(
                         spec, 3707, &record_type, &x, &y),
                     0, A_COORD_RANGE);
    ok &= expect_int("coord.state1.first.type", record_type, 1, A_COORD_RANGE);
    ok &= expect_int("coord.state1.first.x", x, 42, "ReDMCSB COORD.C:795");
    ok &= expect_int("coord.state1.first.abs_x",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_resolve_clip_pc34(
                         spec, 3707, &x, &y) == 0 ? x : -1,
                     66, "ReDMCSB COORD.C:795/1559");
    ok &= expect_int("coord.state1.first.abs_y", y, 28,
                     "ReDMCSB COORD.C:795/1559");
    ok &= expect_int("coord.state2.first.resolve",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_coord_for_zone_pc34(
                         spec, 3708, &record_type, &x, &y),
                     0, A_COORD_RANGE);
    ok &= expect_int("coord.state2.first.x", x, 36, "ReDMCSB COORD.C:796");
    ok &= expect_int("coord.state1.final.resolve",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_coord_for_zone_pc34(
                         spec, 20088, &record_type, &x, &y),
                     0, "ReDMCSB COORD.C:792 with DEFS.H:3516 mask cleared");
    ok &= expect_int("coord.state1.final.type", record_type, 2,
                     "ReDMCSB COORD.C:792");
    ok &= expect_int("coord.state1.final.x", x, 6, "ReDMCSB COORD.C:792");
    ok &= expect_int("coord.state1.final.abs_x",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_resolve_clip_pc34(
                         spec, 20088, &x, &y) == 0 ? x : -1,
                     30, "ReDMCSB COORD.C:792/1559");
    ok &= expect_int("coord.state2.final.resolve",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_coord_for_zone_pc34(
                         spec, 20089, &record_type, &x, &y),
                     0, "ReDMCSB COORD.C:793 with DEFS.H:3516 mask cleared");
    ok &= expect_int("coord.state2.final.x", x, 12, "ReDMCSB COORD.C:793");
    ok &= expect_int("coord.bad_zone",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_coord_for_zone_pc34(
                         spec, 3710, &record_type, &x, &y),
                     -1, "ReDMCSB COORD.C:788-797 C3700..3709 range");
    ok &= expect_int("coord.null_spec",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_resolve_clip_pc34(
                         NULL, 3700, &x, &y),
                     -1, A_COORD_CLIP);

    return ok;
}

static int test_c10_transparent_synthetic_blit(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(9);
    uint8_t source[12] = { 10, 1, 10, 2, 3, 10, 4, 10, 5, 6, 10, 7 };
    uint8_t destination[12] = { 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77 };
    uint8_t open_destination[4] = { 88, 88, 88, 88 };

    ok &= expect_int("transparent.macro",
                     CSB_V1_D2L2_D2R2_F0111_PARTLY_OPEN_C10_COLOR_FLESH,
                     10, A_C10);
    ok &= expect_int("transparent.mask_macro",
                     CSB_V1_D2L2_D2R2_F0111_PARTLY_OPEN_MASK0x4000,
                     0x4000, "ReDMCSB DEFS.H:3516 MASK0x4000");
    ok &= expect_int("transparent.spec_color", spec ? spec->transparent_color : -1,
                     10, A_C10);
    ok &= expect_int("transparent.c10_skip", spec ? spec->c10_skip_enabled : -1,
                     1, A_C10);
    ok &= expect_int("blit.uses_clip",
                     spec ? spec->synthetic_blit_uses_d2_panel_clip : -1, 1,
                     A_COORD_CLIP);
    ok &= expect_int("blit.copied",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_apply_c10_blit_pc34(
                         spec, 2, source, 4, destination, 4, 4, 3),
                     7, A_C10);
    ok &= expect_int("blit.transparent0", destination[0], 77, A_C10);
    ok &= expect_int("blit.pixel1", destination[1], 1, A_C10);
    ok &= expect_int("blit.transparent2", destination[2], 77, A_C10);
    ok &= expect_int("blit.pixel3", destination[3], 2, A_C10);
    ok &= expect_int("blit.pixel4", destination[4], 3, A_C10);
    ok &= expect_int("blit.transparent5", destination[5], 77, A_C10);
    ok &= expect_int("blit.pixel11", destination[11], 7, A_C10);
    ok &= expect_int("blit.open_skips",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_apply_c10_blit_pc34(
                         spec, 0, source, 2, open_destination, 2, 2, 2),
                     0, "ReDMCSB DUNVIEW.C:4248 F0111 skips C0 open");
    ok &= expect_int("blit.open_preserves", open_destination[0], 88,
                     "ReDMCSB DUNVIEW.C:4248 F0111 skips C0 open");
    ok &= expect_int("blit.reject_oversize",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_apply_c10_blit_pc34(
                         spec, 2, source, 49, destination, 49, 49, 1),
                     -1, A_COORD_CLIP);
    ok &= expect_int("blit.reject_bad_stride",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_apply_c10_blit_pc34(
                         spec, 2, source, 3, destination, 4, 4, 3),
                     -1, A_COORD_CLIP);
    ok &= expect_int("blit.reject_null",
                     csb_v1_viewport_d2l2_d2r2_f0111_partly_open_apply_c10_blit_pc34(
                         NULL, 2, source, 4, destination, 4, 4, 3),
                     -1, A_F0111);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const char *e =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_source_evidence_pc34();
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(10);

    ok &= expect_contains("evidence.scope", e, "no real-asset pixel parity",
                          A_F0111);
    ok &= expect_contains("evidence.f0128", e, "DUNVIEW.C:8503-8508",
                          A_F0128);
    ok &= expect_contains("evidence.d2_dispatch", e, "DUNVIEW.C:6837-6896",
                          "ReDMCSB DUNVIEW.C:6837-6896");
    ok &= expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4337",
                          A_F0111);
    ok &= expect_contains("evidence.partly", e, "3 | MASK0x4000", A_PARTLY);
    ok &= expect_contains("evidence.coord_range", e, "COORD.C:788-797",
                          A_COORD_RANGE);
    ok &= expect_contains("evidence.coord_clip", e, "COORD.C:1556-1559",
                          A_COORD_CLIP);
    ok &= expect_contains("evidence.defs", e, "DEFS.H:2088", A_DEFS);
    ok &= expect_contains("evidence.c707_c708", e, "DEFS.H:4047-4048",
                          A_DEFS);
    ok &= expect_contains("evidence.c3700_excluded", e, "explicitly exclude",
                          "ReDMCSB DEFS.H:4250");
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1813-1820",
                          A_LINEAGE);
    ok &= expect_contains("d2l2.source", d2l2 ? d2l2->redmcsb_dispatcher_lines : NULL,
                          "6837-6865", A_D2L2);
    ok &= expect_contains("d2r2.source", d2r2 ? d2r2->redmcsb_dispatcher_lines : NULL,
                          "6868-6896", A_D2R2);
    ok &= expect_contains("d2l2.f0111_source", d2l2 ? d2l2->f0111_source_lines : NULL,
                          "4218-4337", A_F0111);
    ok &= expect_contains("d2l2.coord_source", d2l2 ? d2l2->coord_source_lines : NULL,
                          "1556-1559", A_COORD_CLIP);
    ok &= expect_contains("d2r2.defs_source", d2r2 ? d2r2->defs_source_lines : NULL,
                          "4048", A_DEFS);
    ok &= expect_contains("d2l2.lineage_source", d2l2 ? d2l2->lineage_source_lines : NULL,
                          "1813-1820", A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2l2_d2r2_f0111_partly_open_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2l2_d2r2_f0111_partly_open_source_evidence_pc34());

    ok &= test_identity_and_d2_bindings();
    ok &= test_non_routes_and_lineage_contract();
    ok &= test_partly_open_zone_math();
    ok &= test_coord_clip_path();
    ok &= test_c10_transparent_synthetic_blit();
    ok &= test_evidence_strings();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_40", g_assertions >= 40, 1,
                     "assigned D2L2/D2R2 F0111 partly-open source-lock gate");

    if (ok) {
        printf("PASS csb_v1_viewport_d2l2_d2r2_f0111_partly_open_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
