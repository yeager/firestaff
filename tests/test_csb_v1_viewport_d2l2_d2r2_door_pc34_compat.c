#include "csb_v1_viewport_d2l2_d2r2_door_pc34_compat.h"

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

static int test_route_rows(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_door_route_spec_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_door_route_spec_for_square_pc34(10);

    /* ReDMCSB: DUNVIEW.C:6837-6872 F0678_DrawD2L2 and DUNVIEW.C:6868-6896
     * F0679_DrawD2R2 are reached by F0128 at lines 8503-8508. */
    ok &= expect_int("route.count",
                     (int)csb_v1_viewport_d2l2_d2r2_door_route_spec_count_pc34(), 2,
                     "ReDMCSB DUNVIEW.C:6837-6896");
    ok &= expect_int("d2l2.present", d2l2 != NULL, 1,
                     "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_int("d2r2.present", d2r2 != NULL, 1,
                     "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    ok &= expect_int("d2l2.contract_only",
                     d2l2 ? d2l2->source_locked_contract_only : -1, 1,
                     "source-locked contract marker");
    ok &= expect_int("d2r2.contract_only",
                     d2r2 ? d2r2->source_locked_contract_only : -1, 1,
                     "source-locked contract marker");
    ok &= expect_int("d2l2.depth", d2l2 ? d2l2->f0128_relative_depth : -1, 2,
                     "ReDMCSB DUNVIEW.C:8503 F0128 depth 2");
    ok &= expect_int("d2r2.depth", d2r2 ? d2r2->f0128_relative_depth : -1, 2,
                     "ReDMCSB DUNVIEW.C:8507 F0128 depth 2");
    ok &= expect_int("d2l2.lateral", d2l2 ? d2l2->f0128_relative_lateral : 0, -2,
                     "ReDMCSB DUNVIEW.C:8503 F0128 lateral -2");
    ok &= expect_int("d2r2.lateral", d2r2 ? d2r2->f0128_relative_lateral : 0, 2,
                     "ReDMCSB DUNVIEW.C:8507 F0128 lateral 2");
    ok &= expect_int("d2l2.wall_zone",
                     d2l2 ? d2l2->wall_precedes_door_zone : -1, 707,
                     "ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2");
    ok &= expect_int("d2r2.wall_zone",
                     d2r2 ? d2r2->wall_precedes_door_zone : -1, 708,
                     "ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2");
    ok &= expect_int("unknown.square",
                     csb_v1_viewport_d2l2_d2r2_door_route_spec_for_square_pc34(14) == NULL, 1,
                     "D2L2/D2R2-only contract");

    return ok;
}

static int test_dispatcher_door_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_door_route_spec_at_pc34(0);
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_door_route_spec_at_pc34(1);

    /* ReDMCSB: DUNVIEW.C:6837-6896 has no C17 door-front case for D2L2/D2R2;
     * wall cases return at lines 6862/6893 after drawing C707/C708. */
    ok &= expect_int("d2l2.no_direct_f0111",
                     d2l2 ? d2l2->f0678_f0679_has_direct_f0111_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6837-6872 no F0111");
    ok &= expect_int("d2r2.no_direct_f0111",
                     d2r2 ? d2r2->f0678_f0679_has_direct_f0111_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6868-6896 no F0111");
    ok &= expect_int("d2l2.wall_returns",
                     d2l2 ? d2l2->wall_case_returns_before_f0111 : -1, 1,
                     "ReDMCSB DUNVIEW.C:6862 return");
    ok &= expect_int("d2r2.wall_returns",
                     d2r2 ? d2r2->wall_case_returns_before_f0111 : -1, 1,
                     "ReDMCSB DUNVIEW.C:6893 return");

    /* The door pixel slice is still locked to F0111's C3700 route and C03
     * record math, matching the D3L2 door gate without asserting bitmap parity. */
    ok &= expect_int("door.zone", d2l2 ? d2l2->door_zone_base : -1, 3700,
                     "ReDMCSB DUNVIEW.C F0111:4218 / DEFS.H:4250");
    ok &= expect_int("door.zone_record_type",
                     d2l2 ? d2l2->door_zone_record_type : -1, 1,
                     "ReDMCSB COORD.C:788 C3700 record type");
    ok &= expect_int("door.parent_record",
                     d2l2 ? d2l2->door_panel_parent_record : -1, 129,
                     "ReDMCSB COORD.C:788/1559 C03 parent 129");
    ok &= expect_int("door.clip_record",
                     d2l2 ? d2l2->door_panel_clip_record : -1, 126,
                     "ReDMCSB COORD.C:1556-1559 C03 clip 126");
    ok &= expect_int("door.same_zone_d2r2",
                     d2r2 ? d2r2->door_zone_base : -1, 3700,
                     "contract anchors this pixel slice to C3700");

    return ok;
}

static int test_c03_zone_math(void)
{
    int ok = 1;
    int x = -1;
    int y = -1;
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *spec =
        csb_v1_viewport_d2l2_d2r2_door_route_spec_for_square_pc34(9);

    /* ReDMCSB: COORD.C:1546-1559 C03 records constrain the F0111 door slice:
     * record 126 is the 48x40 clip, and record 129 places it at x=24,y=28. */
    ok &= expect_int("c03.layout_range", spec ? spec->c03_layout_range : -1, 3,
                     "ReDMCSB COORD.C:1546-1548 G3018_s_LayoutData03");
    ok &= expect_int("c03.width", spec ? spec->clipped_width : -1, 48,
                     "ReDMCSB COORD.C:1556 clip width");
    ok &= expect_int("c03.height", spec ? spec->clipped_height : -1, 40,
                     "ReDMCSB COORD.C:1556 clip height");
    ok &= expect_int("c03.frame_x", spec ? spec->frame_x : -1, 24,
                     "ReDMCSB COORD.C:1559 parent x");
    ok &= expect_int("c03.frame_y", spec ? spec->frame_y : -1, 28,
                     "ReDMCSB COORD.C:1559 parent y");
    ok &= expect_int("zone.closed.resolve",
                     csb_v1_viewport_d2l2_d2r2_door_resolve_zone_pc34(
                         spec, 0, 0, &x, &y), 0,
                     "ReDMCSB COORD.C:788/1559 C3700 closed zone");
    ok &= expect_int("zone.closed.x", x, 24,
                     "ReDMCSB COORD.C:1559");
    ok &= expect_int("zone.closed.y", y, 28,
                     "ReDMCSB COORD.C:1559");
    ok &= expect_int("zone.vertical_half.resolve",
                     csb_v1_viewport_d2l2_d2r2_door_resolve_zone_pc34(
                         spec, 0, 20, &x, &y), 0,
                     "ReDMCSB COORD.C:789-790 vertical offsets");
    ok &= expect_int("zone.vertical_half.x", x, 24,
                     "ReDMCSB COORD.C:789-790");
    ok &= expect_int("zone.vertical_half.y", y, 48,
                     "ReDMCSB COORD.C:789-790");
    ok &= expect_int("zone.horizontal_half.resolve",
                     csb_v1_viewport_d2l2_d2r2_door_resolve_zone_pc34(
                         spec, 12, 0, &x, &y), 0,
                     "ReDMCSB COORD.C:792-794 horizontal offsets");
    ok &= expect_int("zone.horizontal_half.x", x, 36,
                     "ReDMCSB COORD.C:792-794");
    ok &= expect_int("zone.horizontal_half.y", y, 28,
                     "ReDMCSB COORD.C:792-794");
    ok &= expect_int("zone.null.reject",
                     csb_v1_viewport_d2l2_d2r2_door_resolve_zone_pc34(
                         NULL, 0, 0, &x, &y), -1,
                     "route helper rejects unresolved spec");

    return ok;
}

static int test_csb_lineage_frame_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *spec =
        csb_v1_viewport_d2l2_d2r2_door_route_spec_for_square_pc34(9);

    /* CSB-lineage: Viewport.cpp:1813-1820 StdDrawF3L1DoorFacing draws the
     * frame before StdDrawDoor; this gate records the command-level slice. */
    ok &= expect_int("frame.bitmap_command",
                     spec ? spec->frame_bitmap_command : -1, 60200,
                     "CSB-lineage Viewport.cpp:592/1817");
    ok &= expect_int("frame.bitmap_index",
                     spec ? spec->frame_bitmap_index : -1, 5,
                     "CSB-lineage Viewport.cpp:2281 pDoorBitmaps[5]");
    ok &= expect_int("frame.rect_command",
                     spec ? spec->frame_rect_command : -1, 60250,
                     "CSB-lineage Viewport.cpp:650/1817");
    ok &= expect_int("frame.rect_index",
                     spec ? spec->frame_rect_index : -1, 7,
                     "CSB-lineage Viewport.cpp:2386 DoorFrameRect[7]");
    ok &= expect_int("frame.blit_command",
                     spec ? spec->frame_blit_command : -1, 60010,
                     "CSB-lineage Viewport.cpp:385/1817 StdBltShapeToViewport");
    ok &= expect_int("frame.not_mirrored",
                     spec ? spec->frame_blit_is_mirrored : -1, 0,
                     "CSB-lineage Viewport.cpp:1817 StdBltShapeToViewport");
    ok &= expect_int("door.graphic_command",
                     spec ? spec->door_graphic_command : -1, 60223,
                     "CSB-lineage Viewport.cpp:618/1818");
    ok &= expect_int("door.graphic_index",
                     spec ? spec->door_graphic_index : -1, 2,
                     "CSB-lineage Viewport.cpp:2568 StdDoorGraphicsF3");
    ok &= expect_int("door.graphic_size",
                     spec ? spec->door_graphic_size : -1, 984,
                     "CSB-lineage Viewport.cpp:2602-2604");
    ok &= expect_int("door.nearness",
                     spec ? spec->door_nearness : -1, 0,
                     "CSB-lineage Viewport.cpp:2602-2604");

    return ok;
}

static int test_c10_frame_clip(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *spec =
        csb_v1_viewport_d2l2_d2r2_door_route_spec_for_square_pc34(9);
    uint8_t source[12];
    uint8_t destination[12];

    for (size_t i = 0; i < sizeof(source); ++i) source[i] = 10;
    for (size_t i = 0; i < sizeof(destination); ++i) destination[i] = 77;
    source[0] = 1;
    source[3] = 2;
    source[8] = 3;
    source[11] = 4;

    /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH is passed by DUNVIEW.C:4334 F0111
     * to F0791; the synthetic slice helper preserves that transparency key. */
    ok &= expect_int("transparent.color", spec ? spec->transparent_color : -1, 10,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("transparent.preserved",
                     spec ? spec->preserves_c10_transparency : -1, 1,
                     "ReDMCSB DUNVIEW.C:4334 F0111 C10");
    ok &= expect_int("clip.copied",
                     csb_v1_viewport_d2l2_d2r2_door_apply_c03_frame_clip_pc34(
                         spec, source, 4, destination, 4, 4, 3),
                     4,
                     "ReDMCSB DUNVIEW.C:4334 / COORD.C:1556");
    ok &= expect_int("clip.pixel_0_0", destination[0], 1,
                     "synthetic F0111 pixel copy");
    ok &= expect_int("clip.transparent_1_0", destination[1], 77,
                     "ReDMCSB DEFS.H:2088 C10 transparent");
    ok &= expect_int("clip.pixel_3_0", destination[3], 2,
                     "synthetic F0111 pixel copy");
    ok &= expect_int("clip.pixel_0_2", destination[8], 3,
                     "synthetic F0111 pixel copy");
    ok &= expect_int("clip.pixel_3_2", destination[11], 4,
                     "synthetic F0111 pixel copy");
    ok &= expect_int("clip.reject_width",
                     csb_v1_viewport_d2l2_d2r2_door_apply_c03_frame_clip_pc34(
                         spec, source, 4, destination, 4, 49, 1),
                     -1,
                     "ReDMCSB COORD.C:1556 width 48");
    ok &= expect_int("clip.reject_null",
                     csb_v1_viewport_d2l2_d2r2_door_apply_c03_frame_clip_pc34(
                         NULL, source, 4, destination, 4, 4, 3),
                     -1,
                     "route helper rejects unresolved spec");

    return ok;
}

static int test_source_evidence(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d2l2_d2r2_door_source_evidence_pc34();

    ok &= expect_contains("evidence.f0678", e, "DUNVIEW.C:6837-6872",
                          "ReDMCSB DUNVIEW.C:6837-6872");
    ok &= expect_contains("evidence.f0679", e, "DUNVIEW.C:6837-6896",
                          "ReDMCSB DUNVIEW.C:6837-6896");
    ok &= expect_contains("evidence.f0111", e, "F0111:4218",
                          "ReDMCSB DUNVIEW.C F0111:4218");
    ok &= expect_contains("evidence.c3700", e, "C3700_ZONE_DOOR_D3L2",
                          "ReDMCSB DEFS.H:4250");
    ok &= expect_contains("evidence.c03", e, "COORD.C:1556-1559",
                          "ReDMCSB COORD.C:1556-1559");
    ok &= expect_contains("evidence.c707", e, "C707_ZONE_WALL_D2L2=707",
                          "ReDMCSB DEFS.H:4047");
    ok &= expect_contains("evidence.c708", e, "C708_ZONE_WALL_D2R2=708",
                          "ReDMCSB DEFS.H:4048");
    ok &= expect_contains("evidence.c10", e, "C10_COLOR_FLESH=10",
                          "ReDMCSB DEFS.H:2088");
    ok &= expect_contains("evidence.csb_viewport", e, "Viewport.cpp:1813-1820",
                          "CSB-lineage Viewport.cpp:1813-1820");
    ok &= expect_contains("evidence.contract_only", e, "not full real-asset",
                          "assigned source-locked contract scope");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2l2_d2r2_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2l2_d2r2_door_source_evidence_pc34());

    ok &= test_route_rows();
    ok &= test_dispatcher_door_contract();
    ok &= test_c03_zone_math();
    ok &= test_csb_lineage_frame_contract();
    ok &= test_c10_frame_clip();
    ok &= test_source_evidence();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_60", g_assertions >= 60, 1,
                     "assigned D2L2/D2R2 F0111 door contract gate");

    if (ok) {
        printf("PASS csb_v1_viewport_d2l2_d2r2_door_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
