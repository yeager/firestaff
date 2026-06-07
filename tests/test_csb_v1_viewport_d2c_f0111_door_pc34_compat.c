#include "csb_v1_viewport_d2c_f0111_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_F0121 =
    "ReDMCSB DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C";
static const char *A_WALL_RETURN =
    "ReDMCSB DUNVIEW.C:7289-7312 wall-case early-return";
static const char *A_C09 =
    "ReDMCSB DEFS.H:3432 C09_WALL_D2C";
static const char *A_C707 =
    "ReDMCSB DEFS.H:4030 C707_ZONE_WALL_D2C";
static const char *A_C709 =
    "ReDMCSB DEFS.H:4049 C709_ZONE_WALL_D2C";
static const char *A_C3700 =
    "ReDMCSB DEFS.H:4250 C3700 door zone";
static const char *A_C10 =
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH transparency";
static const char *A_COORD =
    "ReDMCSB COORD.C:1556-1559 door record path";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1151-1156,1414-1420 frame-blt/frame-rect bindings";

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

static int test_f0121_d2c_identity(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec =
        csb_v1_viewport_d2c_f0111_door_non_route_spec_pc34();

    ok &= expect_int("spec.present", spec != NULL, 1, A_F0121);
    ok &= expect_int("contract_only", spec ? spec->source_locked_contract_only : 0, 1,
                     A_F0121);
    ok &= expect_int("f0121.route_present", spec ? spec->f0121_center_route_present : 0, 1,
                     A_F0121);
    ok &= expect_int("view_square.d2c", spec ? spec->view_square : -1, 6, A_F0121);
    ok &= expect_int("f0128.depth", spec ? spec->f0128_relative_depth : -1, 2,
                     A_F0121);
    ok &= expect_int("f0128.lateral", spec ? spec->f0128_relative_lateral : -9, 0,
                     A_F0121);
    ok &= expect_contains("f0121.source", spec ? spec->f0121_source_lines : NULL,
                          "7244-7389", A_F0121);
    ok &= expect_contains("wall.source", spec ? spec->wall_case_source_lines : NULL,
                          "7289-7312", A_WALL_RETURN);
    ok &= expect_contains("no_wall.source", spec ? spec->no_wall_source_lines : NULL,
                          "7353-7388", A_F0121);
    ok &= expect_contains("lineage.source", spec ? spec->lineage_source_lines : NULL,
                          "1151-1156", A_LINEAGE);
    ok &= expect_contains("lineage.source.teleporter",
                          spec ? spec->lineage_source_lines : NULL, "1414-1420",
                          A_LINEAGE);
    ok &= expect_int("wall.zone.helper",
                     csb_v1_viewport_d2c_f0111_door_zone_from_wall_spec_pc34(spec),
                     709, A_C709);
    ok &= expect_int("wall.zone.helper.null",
                     csb_v1_viewport_d2c_f0111_door_zone_from_wall_spec_pc34(NULL),
                     -1, A_F0121);

    return ok;
}

static int test_d2c_wall_case_rejects_f0111_c3700_panel(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec =
        csb_v1_viewport_d2c_f0111_door_non_route_spec_pc34();

    ok &= expect_int("wall.ordinal.c09", spec ? spec->wall_ordinal_c09_d2c : -1,
                     9, A_C09);
    ok &= expect_int("wall.zone.media508", spec ? spec->media508_wall_zone_c707 : -1,
                     707, A_C707);
    ok &= expect_int("wall.zone.media720", spec ? spec->media720_wall_zone_c709 : -1,
                     709, A_C709);
    ok &= expect_int("wall.returns.before_f0111",
                     spec ? spec->wall_case_returns_before_f0111 : 0, 1,
                     A_WALL_RETURN);
    ok &= expect_int("wall.calls.f0100", spec ? spec->wall_case_calls_f0100 : -1,
                     1, A_WALL_RETURN);
    ok &= expect_int("wall.calls.f0105", spec ? spec->wall_case_calls_f0105 : -1,
                     0, A_WALL_RETURN);
    ok &= expect_int("wall.calls.f0107", spec ? spec->wall_case_calls_f0107 : -1,
                     1, A_WALL_RETURN);
    ok &= expect_int("wall.calls.f0111", spec ? spec->wall_case_calls_f0111 : -1,
                     0, A_WALL_RETURN);
    ok &= expect_int("wall.calls.c3700_panel",
                     spec ? spec->wall_case_calls_c3700_door_panel : -1, 0,
                     A_C3700);
    ok &= expect_int("wall.c3700.zone.constant", spec ? spec->c3700_door_zone : -1,
                     3700, A_C3700);
    ok &= expect_int("wall.c3700.is_d3l2",
                     spec ? spec->c3700_is_d3l2_door_zone : -1, 1, A_C3700);
    ok &= expect_int("wall.d2c.uses_c3700",
                     spec ? spec->d2c_uses_c3700_door_zone : -1, 0, A_C3700);
    ok &= expect_int("wall.d2c.rejects_c3700",
                     spec ? spec->d2c_c3700_panel_path_rejected : -1, 1,
                     A_C3700);

    return ok;
}

static int test_c01_c05_no_wall_path_excludes_door_panel(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec =
        csb_v1_viewport_d2c_f0111_door_non_route_spec_pc34();

    ok &= expect_int("corridor.enters_no_wall",
                     spec ? spec->c01_corridor_enters_no_wall_path : 0, 1,
                     A_F0121);
    ok &= expect_int("teleporter.enters_no_wall",
                     spec ? spec->c05_teleporter_enters_no_wall_path : 0, 1,
                     A_F0121);
    ok &= expect_int("no_wall.calls.f0100", spec ? spec->no_wall_path_calls_f0100 : -1,
                     0, A_F0121);
    ok &= expect_int("no_wall.calls.f0105", spec ? spec->no_wall_path_calls_f0105 : -1,
                     0, A_F0121);
    ok &= expect_int("no_wall.calls.f0107", spec ? spec->no_wall_path_calls_f0107 : -1,
                     0, A_F0121);
    ok &= expect_int("no_wall.calls.f0111", spec ? spec->no_wall_path_calls_f0111 : -1,
                     0, A_F0121);
    ok &= expect_int("no_wall.calls.f0113", spec ? spec->no_wall_path_calls_f0113 : -1,
                     1, A_F0121);
    ok &= expect_int("no_wall.floor_ornament",
                     spec ? spec->no_wall_path_draws_floor_ornament : 0, 1,
                     A_F0121);
    ok &= expect_int("no_wall.ceiling_pit",
                     spec ? spec->no_wall_path_draws_ceiling_pit : 0, 1,
                     A_F0121);
    ok &= expect_int("no_wall.f0115.before_field",
                     spec ? spec->no_wall_path_draws_f0115_before_field : 0, 1,
                     A_F0121);
    ok &= expect_int("no_wall.cell_order",
                     spec ? spec->no_wall_path_cell_order : -1, 0x3421,
                     A_F0121);
    ok &= expect_int("no_wall.field_zone",
                     spec ? spec->no_wall_path_field_zone : -1, 709, A_C709);
    ok &= expect_int("no_wall.field_zone_matches_wall",
                     spec ? spec->no_wall_path_field_zone == spec->media720_wall_zone_c709 : 0,
                     1, A_C709);
    ok &= expect_int("no_wall.preserves_c10",
                     spec ? spec->no_wall_path_preserves_c10_transparency : 0, 1,
                     A_C10);
    ok &= expect_int("no_wall.not_c3700",
                     spec ? spec->no_wall_path_field_zone != spec->c3700_door_zone : 0,
                     1, A_C3700);
    ok &= expect_int("door_panel.no_wall.no_c3700",
                     spec ? spec->no_wall_path_calls_f0111 ||
                                spec->d2c_uses_c3700_door_zone : -1,
                     0, A_C3700);

    return ok;
}

static int test_c3700_coord_path_is_metadata_only_for_d2c(void)
{
    int ok = 1;
    int x = 1234;
    int y = 5678;
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec =
        csb_v1_viewport_d2c_f0111_door_non_route_spec_pc34();

    ok &= expect_int("coord.clip_record", spec ? spec->coord_clip_record : -1,
                     126, A_COORD);
    ok &= expect_int("coord.parent_record", spec ? spec->coord_parent_record : -1,
                     129, A_COORD);
    ok &= expect_int("coord.clip_width", spec ? spec->coord_clip_width : -1,
                     48, A_COORD);
    ok &= expect_int("coord.clip_height", spec ? spec->coord_clip_height : -1,
                     40, A_COORD);
    ok &= expect_int("coord.frame_x", spec ? spec->coord_frame_x : -1, 24,
                     A_COORD);
    ok &= expect_int("coord.frame_y", spec ? spec->coord_frame_y : -1, 28,
                     A_COORD);
    ok &= expect_int("d2c.uses_coord_record_path",
                     spec ? spec->d2c_uses_coord_door_record_path : -1, 0,
                     A_COORD);
    ok &= expect_int("reject_c3700.panel",
                     csb_v1_viewport_d2c_f0111_door_reject_c3700_panel_path_pc34(
                         spec, 0, 0, &x, &y),
                     -2, A_C3700);
    ok &= expect_int("reject_c3700.keeps_x", x, 1234, A_COORD);
    ok &= expect_int("reject_c3700.keeps_y", y, 5678, A_COORD);
    ok &= expect_int("reject_c3700.null_spec",
                     csb_v1_viewport_d2c_f0111_door_reject_c3700_panel_path_pc34(
                         NULL, 0, 0, &x, &y),
                     -1, A_C3700);
    ok &= expect_int("reject_c3700.null_out_x",
                     csb_v1_viewport_d2c_f0111_door_reject_c3700_panel_path_pc34(
                         spec, 0, 0, NULL, &y),
                     -1, A_COORD);
    ok &= expect_int("reject_c3700.null_out_y",
                     csb_v1_viewport_d2c_f0111_door_reject_c3700_panel_path_pc34(
                         spec, 0, 0, &x, NULL),
                     -1, A_COORD);
    ok &= expect_int("c3700.not_d2c_wall_zone",
                     spec ? spec->c3700_door_zone != spec->media720_wall_zone_c709 : 0,
                     1, A_C3700);
    ok &= expect_int("c3700.not_d2c_media508_zone",
                     spec ? spec->c3700_door_zone != spec->media508_wall_zone_c707 : 0,
                     1, A_C3700);

    return ok;
}

static int test_c10_transparency_for_no_wall_field_path(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec =
        csb_v1_viewport_d2c_f0111_door_non_route_spec_pc34();
    uint8_t source[12] = { 10, 1, 10, 2, 3, 10, 4, 10, 5, 6, 10, 7 };
    uint8_t destination[12] = { 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77 };

    ok &= expect_int("transparent.color.macro",
                     CSB_V1_D2C_F0111_DOOR_PC34_C10_COLOR_FLESH, 10, A_C10);
    ok &= expect_int("transparent.color", spec ? spec->transparent_color : -1,
                     10, A_C10);
    ok &= expect_int("transparent.color.matches_macro",
                     spec ? spec->transparent_color ==
                                CSB_V1_D2C_F0111_DOOR_PC34_C10_COLOR_FLESH : 0,
                     1, A_C10);
    ok &= expect_int("field.blit.copied",
                     csb_v1_viewport_d2c_f0111_door_apply_c10_field_blit_pc34(
                         spec, source, 4, destination, 4, 4, 3),
                     7, A_C10);
    ok &= expect_int("field.blit.transparent0", destination[0], 77, A_C10);
    ok &= expect_int("field.blit.pixel1", destination[1], 1, A_C10);
    ok &= expect_int("field.blit.transparent2", destination[2], 77, A_C10);
    ok &= expect_int("field.blit.pixel3", destination[3], 2, A_C10);
    ok &= expect_int("field.blit.pixel4", destination[4], 3, A_C10);
    ok &= expect_int("field.blit.transparent5", destination[5], 77, A_C10);
    ok &= expect_int("field.blit.pixel6", destination[6], 4, A_C10);
    ok &= expect_int("field.blit.transparent7", destination[7], 77, A_C10);
    ok &= expect_int("field.blit.pixel8", destination[8], 5, A_C10);
    ok &= expect_int("field.blit.pixel9", destination[9], 6, A_C10);
    ok &= expect_int("field.blit.transparent10", destination[10], 77, A_C10);
    ok &= expect_int("field.blit.pixel11", destination[11], 7, A_C10);
    ok &= expect_int("field.blit.reject_null_spec",
                     csb_v1_viewport_d2c_f0111_door_apply_c10_field_blit_pc34(
                         NULL, source, 4, destination, 4, 4, 3),
                     -1, A_C10);
    ok &= expect_int("field.blit.reject_bad_stride",
                     csb_v1_viewport_d2c_f0111_door_apply_c10_field_blit_pc34(
                         spec, source, 3, destination, 4, 4, 3),
                     -1, A_C10);

    return ok;
}

static int test_lineage_and_evidence_anchors(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d2c_f0111_door_source_evidence_pc34();
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec =
        csb_v1_viewport_d2c_f0111_door_non_route_spec_pc34();

    ok &= expect_int("lineage.open.binding",
                     spec ? spec->lineage_open_binding_present : 0, 1,
                     A_LINEAGE);
    ok &= expect_int("lineage.teleporter.binding",
                     spec ? spec->lineage_teleporter_binding_present : 0, 1,
                     A_LINEAGE);
    ok &= expect_int("lineage.frame_blt.binding",
                     spec ? spec->lineage_frame_blt_binding_present : 0, 1,
                     A_LINEAGE);
    ok &= expect_int("lineage.frame_rect.binding",
                     spec ? spec->lineage_frame_rect_binding_present : 0, 1,
                     A_LINEAGE);
    ok &= expect_contains("evidence.contract", e, "Source-locked contract gate only",
                          A_F0121);
    ok &= expect_contains("evidence.f0121", e, "DUNVIEW.C:7244-7389",
                          A_F0121);
    ok &= expect_contains("evidence.wall_return", e, "DUNVIEW.C:7289-7312",
                          A_WALL_RETURN);
    ok &= expect_contains("evidence.c01_c05", e, "C05/C01 no-wall", A_F0121);
    ok &= expect_contains("evidence.no_f0100", e, "excludes F0100", A_F0121);
    ok &= expect_contains("evidence.no_f0105", e, "F0105", A_F0121);
    ok &= expect_contains("evidence.no_f0107", e, "F0107", A_F0121);
    ok &= expect_contains("evidence.no_f0111", e, "F0111", A_WALL_RETURN);
    ok &= expect_contains("evidence.c09", e, "C09_WALL_D2C", A_C09);
    ok &= expect_contains("evidence.c707", e, "C707_ZONE_WALL_D2C", A_C707);
    ok &= expect_contains("evidence.c709", e, "C709_ZONE_WALL_D2C", A_C709);
    ok &= expect_contains("evidence.c3700", e, "C3700_ZONE_DOOR_D3L2",
                          A_C3700);
    ok &= expect_contains("evidence.c10", e, "C10_COLOR_FLESH", A_C10);
    ok &= expect_contains("evidence.coord", e, "COORD.C:1556-1559", A_COORD);
    ok &= expect_contains("evidence.lineage_open", e, "Viewport.cpp:1151-1156",
                          A_LINEAGE);
    ok &= expect_contains("evidence.lineage_teleporter", e,
                          "Viewport.cpp:1414-1420", A_LINEAGE);
    ok &= expect_contains("evidence.frame_binding", e, "frame-blt/frame-rect",
                          A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2c_f0111_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2c_f0111_door_source_evidence_pc34());

    ok &= test_f0121_d2c_identity();
    ok &= test_d2c_wall_case_rejects_f0111_c3700_panel();
    ok &= test_c01_c05_no_wall_path_excludes_door_panel();
    ok &= test_c3700_coord_path_is_metadata_only_for_d2c();
    ok &= test_c10_transparency_for_no_wall_field_path();
    ok &= test_lineage_and_evidence_anchors();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_60", g_assertions >= 60, 1,
                     A_F0121);

    if (ok) {
        printf("PASS csb_v1_viewport_d2c_f0111_door_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
