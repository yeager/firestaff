#include "csb_v1_viewport_d2l2_d2r2_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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

static int test_run_entry_point(void)
{
    int ok = 1;
    CSB_V1_ViewportD2L2D2R2WallRunResult result;

    ok &= expect_int("run.return",
                     csb_v1_viewport_d2l2_d2r2_wall_pc34_compat_run(&result), 0,
                     "source-locked D2L2/D2R2 wall runner");
    ok &= expect_int("run.ok", result.ok, 1,
                     "source-locked D2L2/D2R2 wall runner");
    ok &= expect_int("run.route_count", result.route_count, 2,
                     "ReDMCSB DUNVIEW.C:6837-6896");
    ok &= expect_int("run.wall_zone_draw_order_ok", result.wall_zone_draw_order_ok, 1,
                     "ReDMCSB DUNVIEW.C:8503-8508");
    ok &= expect_int("run.palette_indices_ok", result.palette_indices_ok, 1,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("run.lineage_frame_bindings_ok", result.lineage_frame_bindings_ok, 1,
                     "CSB-lineage Viewport.cpp:1813-1820");
    ok &= expect_int("run.symmetry_ok", result.symmetry_ok, 1,
                     "ReDMCSB DUNVIEW.C:6837-6896 mirrored pair");
    ok &= expect_int("run.d2l2_copied_pixels", result.d2l2_copied_pixels, 6,
                     "ReDMCSB DUNVIEW.C:3113-3129 F0104 C10 blit");
    ok &= expect_int("run.d2r2_copied_pixels", result.d2r2_copied_pixels, 6,
                     "ReDMCSB DUNVIEW.C:3185-3204 F0105 C10 blit");

    return ok;
}

static int test_route_identity_and_order(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(10);

    /* ReDMCSB: DUNVIEW.C:6837-6865 F0678_DrawD2L2 and 6868-6896
     * F0679_DrawD2R2 are reached by F0128 at 8503-8508 in left-first order. */
    ok &= expect_int("route.count",
                     (int)csb_v1_viewport_d2l2_d2r2_wall_route_spec_count_pc34(), 2,
                     "ReDMCSB DUNVIEW.C:6837-6896");
    ok &= expect_int("d2l2.present", d2l2 != NULL, 1,
                     "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_int("d2r2.present", d2r2 != NULL, 1,
                     "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    ok &= expect_int("index0.d2l2",
                     csb_v1_viewport_d2l2_d2r2_wall_route_spec_at_pc34(0) == d2l2,
                     1, "ReDMCSB DUNVIEW.C:8503-8504");
    ok &= expect_int("index1.d2r2",
                     csb_v1_viewport_d2l2_d2r2_wall_route_spec_at_pc34(1) == d2r2,
                     1, "ReDMCSB DUNVIEW.C:8507-8508");
    ok &= expect_int("index2.null",
                     csb_v1_viewport_d2l2_d2r2_wall_route_spec_at_pc34(2) == NULL,
                     1, "D2L2/D2R2-only contract");
    ok &= expect_int("unknown.square",
                     csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(11) == NULL,
                     1, "D2L2/D2R2-only contract");
    ok &= expect_int("d2l2.order", d2l2 ? d2l2->f0128_draw_order_index : -1, 8,
                     "ReDMCSB DUNVIEW.C:8503-8504");
    ok &= expect_int("d2r2.order", d2r2 ? d2r2->f0128_draw_order_index : -1, 9,
                     "ReDMCSB DUNVIEW.C:8507-8508");
    ok &= expect_int("d2l2.depth", d2l2 ? d2l2->f0128_relative_depth : -1, 2,
                     "ReDMCSB DUNVIEW.C:8503");
    ok &= expect_int("d2r2.depth", d2r2 ? d2r2->f0128_relative_depth : -1, 2,
                     "ReDMCSB DUNVIEW.C:8507");
    ok &= expect_int("d2l2.lateral", d2l2 ? d2l2->f0128_relative_lateral : 0, -2,
                     "ReDMCSB DUNVIEW.C:8503");
    ok &= expect_int("d2r2.lateral", d2r2 ? d2r2->f0128_relative_lateral : 0, 2,
                     "ReDMCSB DUNVIEW.C:8507");

    return ok;
}

static int test_wall_routes_and_palette_indices(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(10);
    uint8_t source[8] = { 1, 10, 2, 3, 4, 10, 5, 6 };
    uint8_t destination[8] = { 77, 77, 77, 77, 77, 77, 77, 77 };

    ok &= expect_int("d2l2.wall_zone", d2l2 ? d2l2->wall_zone : -1, 707,
                     "ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2");
    ok &= expect_int("d2r2.wall_zone", d2r2 ? d2r2->wall_zone : -1, 708,
                     "ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2");
    ok &= expect_int("wall.element", d2l2 ? d2l2->wall_element : -1, 0,
                     "ReDMCSB DEFS.H:1007 C00_ELEMENT_WALL");
    ok &= expect_int("teleporter.element", d2l2 ? d2l2->teleporter_element : -1, 5,
                     "ReDMCSB DEFS.H:1012 C05_ELEMENT_TELEPORTER");
    ok &= expect_int("d2l2.native_base", d2l2 ? d2l2->native_wall_index_base : -1, 6,
                     "ReDMCSB DEFS.H:3429 C06_WALL_D2L2");
    ok &= expect_int("d2r2.native_base", d2r2 ? d2r2->native_wall_index_base : -1, 5,
                     "ReDMCSB DEFS.H:3428 C05_WALL_D2R2");
    ok &= expect_int("d2l2.pc34_effective",
                     d2l2 ? d2l2->native_wall_index_pc34_effective : -1, 8,
                     "ReDMCSB DUNVIEW.C:6854-6858 C06_WALL_D2L2 + 2");
    ok &= expect_int("d2r2.pc34_effective",
                     d2r2 ? d2r2->native_wall_index_pc34_effective : -1, 7,
                     "ReDMCSB DUNVIEW.C:6885-6889 C05_WALL_D2R2 + 2");
    ok &= expect_int("d2l2.media709_flipped",
                     d2l2 ? d2l2->media709_flipped_wall_index : -1, 5,
                     "ReDMCSB DUNVIEW.C:6849-6851");
    ok &= expect_int("d2r2.media709_flipped",
                     d2r2 ? d2r2->media709_flipped_wall_index : -1, 6,
                     "ReDMCSB DUNVIEW.C:6880-6882");
    ok &= expect_int("d2l2.f0104", d2l2 ? d2l2->f0104_wall_route : -1, 1,
                     "ReDMCSB DUNVIEW.C:6854-6858 F0104");
    ok &= expect_int("d2r2.f0104", d2r2 ? d2r2->f0104_wall_route : -1, 1,
                     "ReDMCSB DUNVIEW.C:6885-6889 F0104");
    ok &= expect_int("d2l2.f0105", d2l2 ? d2l2->f0105_media709_flipped_route : -1, 1,
                     "ReDMCSB DUNVIEW.C:6849-6851 F0105");
    ok &= expect_int("d2r2.f0105", d2r2 ? d2r2->f0105_media709_flipped_route : -1, 1,
                     "ReDMCSB DUNVIEW.C:6880-6882 F0105");
    ok &= expect_int("transparent.color", d2l2 ? d2l2->transparent_color : -1, 10,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("transparent.preserved",
                     d2l2 ? d2l2->preserves_c10_transparency : -1, 1,
                     "ReDMCSB DUNVIEW.C:3113-3129/3185-3204");
    ok &= expect_int("blit.copied",
                     csb_v1_viewport_d2l2_d2r2_wall_apply_c10_blit_pc34(
                         d2l2, source, 4, destination, 4, 4, 2, 0),
                     6, "ReDMCSB DUNVIEW.C:3113-3129 F0104 C10 blit");
    ok &= expect_int("blit.pixel0", destination[0], 1,
                     "synthetic nontransparent palette index copy");
    ok &= expect_int("blit.transparent1", destination[1], 77,
                     "ReDMCSB DEFS.H:2088 C10 transparent");
    ok &= expect_int("blit.pixel7", destination[7], 6,
                     "synthetic nontransparent palette index copy");
    ok &= expect_int("blit.reject_null",
                     csb_v1_viewport_d2l2_d2r2_wall_apply_c10_blit_pc34(
                         NULL, source, 4, destination, 4, 4, 2, 0),
                     -1, "route helper rejects unresolved spec");

    return ok;
}

static int test_lineage_bindings_and_symmetry(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(10);

    ok &= expect_int("d2l2.frame_blit_command_60200",
                     d2l2 ? d2l2->frame_blit_command_60200 : -1, 60200,
                     "CSB-lineage Viewport.cpp:592/1817");
    ok &= expect_int("d2r2.frame_blit_command_60200",
                     d2r2 ? d2r2->frame_blit_command_60200 : -1, 60200,
                     "CSB-lineage Viewport.cpp:592/1817");
    ok &= expect_int("d2l2.frame_rect_command_60250",
                     d2l2 ? d2l2->frame_rect_command_60250 : -1, 60250,
                     "CSB-lineage Viewport.cpp:650/1817");
    ok &= expect_int("d2r2.frame_rect_command_60250",
                     d2r2 ? d2r2->frame_rect_command_60250 : -1, 60250,
                     "CSB-lineage Viewport.cpp:650/1817");
    ok &= expect_int("d2l2.pwallbitmap",
                     d2l2 ? d2l2->csb_viewport_wall_bitmap_index : -1, 5,
                     "CSB-lineage Viewport.cpp:2267/3379 pWallBitmaps[5]");
    ok &= expect_int("d2r2.pwallbitmap",
                     d2r2 ? d2r2->csb_viewport_wall_bitmap_index : -1, 6,
                     "CSB-lineage Viewport.cpp:2271/3390 pWallBitmaps[6]");
    ok &= expect_int("d2l2.wallrect",
                     d2l2 ? d2l2->csb_viewport_wall_rectangle_index : -1, 13,
                     "CSB-lineage Viewport.cpp:3379 wallRectangles[13]");
    ok &= expect_int("d2r2.wallrect",
                     d2r2 ? d2r2->csb_viewport_wall_rectangle_index : -1, 12,
                     "CSB-lineage Viewport.cpp:3390 wallRectangles[12]");
    ok &= expect_int("symmetry.view_square",
                     d2l2 && d2r2 ? d2l2->view_square + 1 == d2r2->view_square : 0,
                     1, "ReDMCSB DEFS.H:2605-2606");
    ok &= expect_int("symmetry.lateral",
                     d2l2 && d2r2 ?
                         d2l2->f0128_relative_lateral == -d2r2->f0128_relative_lateral : 0,
                     1, "ReDMCSB DUNVIEW.C:8503-8508");
    ok &= expect_int("symmetry.wall_zone",
                     d2l2 && d2r2 ? d2l2->wall_zone + 1 == d2r2->wall_zone : 0,
                     1, "ReDMCSB DEFS.H:4047-4048");
    ok &= expect_int("symmetry.media709_cross_pair",
                     d2l2 && d2r2 ?
                         d2l2->native_wall_index_base == d2r2->media709_flipped_wall_index &&
                         d2r2->native_wall_index_base == d2l2->media709_flipped_wall_index : 0,
                     1, "ReDMCSB DUNVIEW.C:6849-6889");
    ok &= expect_int("symmetry.field_aspect",
                     d2l2 && d2r2 ?
                         d2l2->teleporter_field_aspect_index + 1 ==
                             d2r2->teleporter_field_aspect_index : 0,
                     1, "ReDMCSB DUNVIEW.C:377");

    return ok;
}

static int test_teleporter_no_door_and_evidence(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(10);
    const char *e = csb_v1_viewport_d2l2_d2r2_wall_source_evidence_pc34();

    ok &= expect_int("d2l2.f0113", d2l2 ? d2l2->f0113_teleporter_route : -1, 1,
                     "ReDMCSB DUNVIEW.C:6863-6865");
    ok &= expect_int("d2r2.f0113", d2r2 ? d2r2->f0113_teleporter_route : -1, 1,
                     "ReDMCSB DUNVIEW.C:6894-6895");
    ok &= expect_int("d2l2.field_aspect",
                     d2l2 ? d2l2->teleporter_field_aspect_index : -1, 5,
                     "ReDMCSB DUNVIEW.C:377 G2035[9]");
    ok &= expect_int("d2r2.field_aspect",
                     d2r2 ? d2r2->teleporter_field_aspect_index : -1, 6,
                     "ReDMCSB DUNVIEW.C:377 G2035[10]");
    ok &= expect_int("d2l2.no_f0111", d2l2 ? d2l2->f0111_door_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6837-6865 no F0111");
    ok &= expect_int("d2r2.no_f0111", d2r2 ? d2r2->f0111_door_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6868-6896 no F0111");
    ok &= expect_int("d2l2.no_f0115", d2l2 ? d2l2->f0115_thing_pass_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6862 return before thing pass");
    ok &= expect_int("d2r2.no_f0115", d2r2 ? d2r2->f0115_thing_pass_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6893 return before thing pass");
    ok &= expect_contains("evidence.f0678", e, "F0678_DrawD2L2",
                          "ReDMCSB DUNVIEW.C:6837-6865");
    ok &= expect_contains("evidence.f0679", e, "F0679_DrawD2R2",
                          "ReDMCSB DUNVIEW.C:6868-6896");
    ok &= expect_contains("evidence.f0128", e, "DUNVIEW.C:8503-8508",
                          "ReDMCSB DUNVIEW.C:8503-8508");
    ok &= expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3129",
                          "ReDMCSB DUNVIEW.C:3113-3129");
    ok &= expect_contains("evidence.f0105", e, "3185-3204",
                          "ReDMCSB DUNVIEW.C:3185-3204");
    ok &= expect_contains("evidence.c10", e, "C10_COLOR_FLESH",
                          "ReDMCSB DEFS.H:2088");
    ok &= expect_contains("evidence.lineage_commands", e, "command 60200",
                          "CSB-lineage Viewport.cpp:1813-1820");
    ok &= expect_contains("evidence.lineage_rect", e, "60250",
                          "CSB-lineage Viewport.cpp:1813-1820");
    ok &= expect_contains("evidence.pwall", e, "pWallBitmaps 5/6",
                          "CSB-lineage Viewport.cpp:2267/2271");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2l2_d2r2_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2l2_d2r2_wall_source_evidence_pc34());

    ok &= test_run_entry_point();
    ok &= test_route_identity_and_order();
    ok &= test_wall_routes_and_palette_indices();
    ok &= test_lineage_bindings_and_symmetry();
    ok &= test_teleporter_no_door_and_evidence();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_60", g_assertions >= 60, 1,
                     "assigned D2L2/D2R2 wall contract gate");

    if (ok) {
        printf("PASS csb_v1_viewport_d2l2_d2r2_wall_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
