#include "csb_v1_viewport_d3l2_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_int(const char *label, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("ok %s=%d\n", label, got);
    return 1;
}

static int expect_contains(const char *label, const char *haystack, const char *needle)
{
    const int got = haystack && needle && strstr(haystack, needle) != NULL;
    return expect_int(label, got, 1);
}

static int test_route_spec(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2DoorRouteSpec *spec =
        csb_v1_viewport_d3l2_door_route_spec_pc34();

    /* ReDMCSB: DUNVIEW.C F0676 lines 6271-6273 routes C14_VIEW_SQUARE_D3L2
     * through F0115 order 0x0218, F0111 C3700_ZONE_DOOR_D3L2, then F0115
     * order 0x0349; F0111 starts at DUNVIEW.C line 4218. */
    ok &= expect_int("route.present", spec != NULL, 1);
    ok &= expect_int("route.view_square", spec ? spec->view_square : -1, 14);
    ok &= expect_int("route.pass1", spec ? spec->rear_f0115_order : -1, 0x0218);
    ok &= expect_int("route.pass2", spec ? spec->front_f0115_order : -1, 0x0349);
    ok &= expect_int("route.zone", spec ? spec->door_zone_base : -1, 3700);

    /* CSB lineage: Viewport.cpp StdDrawF3L1DoorFacing lines 1813-1820 adds
     * StdDoorFacingFrameLeftBitmapF3L1/RectF3L1/StdBltShapeToViewport before
     * StdDrawDoor; Viewport.cpp lines 592, 650, 2281, 2386, and 2856-2859
     * bind that command to pDoorBitmaps[5], DoorFrameRect[7], and a normal
     * BltShapeToViewport call. */
    ok &= expect_int("route.frame_bitmap_command",
                     spec ? spec->frame_bitmap_command : -1, 60200);
    ok &= expect_int("route.frame_bitmap_index",
                     spec ? spec->frame_bitmap_index : -1, 5);
    ok &= expect_int("route.frame_rect_command",
                     spec ? spec->frame_rect_command : -1, 60250);
    ok &= expect_int("route.frame_rect_index",
                     spec ? spec->frame_rect_index : -1, 7);
    ok &= expect_int("route.frame_blit_command",
                     spec ? spec->frame_blit_command : -1, 60010);
    ok &= expect_int("route.frame_not_mirrored",
                     spec ? spec->frame_blit_is_mirrored : -1, 0);

    /* CSB lineage: Viewport.cpp lines 1818-1819 passes StdDoorGraphicsF3 into
     * StdDrawDoor; lines 2568 and 2596-2616 resolve that to d.DoorGraphic[2],
     * graphicSize 984, and nearness 0 before reaching DrawDoor. */
    ok &= expect_int("route.door_graphic_command",
                     spec ? spec->door_graphic_command : -1, 60223);
    ok &= expect_int("route.door_graphic_index",
                     spec ? spec->door_graphic_index : -1, 2);
    ok &= expect_int("route.door_graphic_size",
                     spec ? spec->door_graphic_size : -1, 984);
    ok &= expect_int("route.door_nearness",
                     spec ? spec->door_nearness : -1, 0);

    /* ReDMCSB: DEFS.H line 2088 defines C10_COLOR_FLESH; DUNVIEW.C F0111
     * line 4334 passes C10 to F0791. CSBCode.cpp BltShapeToViewport lines
     * 2912-2929 uses the same C10 transparency for the CSB frame detour. */
    ok &= expect_int("route.transparent",
                     spec ? spec->transparent_color : -1, 10);
    ok &= expect_int("route.preserves_transparent",
                     spec ? spec->preserves_c10_transparency : -1, 1);

    return ok;
}

static int test_c03_zone_math(void)
{
    int ok = 1;
    int x = -1;
    int y = -1;
    const CSB_V1_ViewportD3L2DoorRouteSpec *spec =
        csb_v1_viewport_d3l2_door_route_spec_pc34();

    /* ReDMCSB: COORD.C G3018_s_LayoutData03 lines 1546-1560 defines record
     * 126 as the 48x40 C03 clip and record 129 at x=24,y=28; COORD.C lines
     * 788-797 make C3700_ZONE_DOOR_D3L2 inherit record 129 offsets. */
    ok &= expect_int("c03.layout_range", spec ? spec->c03_layout_range : -1, 3);
    ok &= expect_int("c03.clip_record", spec ? spec->door_panel_clip_record : -1, 126);
    ok &= expect_int("c03.parent_record", spec ? spec->door_panel_parent_record : -1, 129);
    ok &= expect_int("c03.clipped_width", spec ? spec->clipped_width : -1, 48);
    ok &= expect_int("c03.clipped_height", spec ? spec->clipped_height : -1, 40);
    ok &= expect_int("c03.frame_x", spec ? spec->frame_x : -1, 24);
    ok &= expect_int("c03.frame_y", spec ? spec->frame_y : -1, 28);

    /* ReDMCSB: DUNVIEW.C F0111 starts at line 4218 and F0676 line 6272 passes
     * C3700_ZONE_DOOR_D3L2; COORD.C lines 788-790 plus line 1559 resolve the
     * closed and vertical-offset zones as frame_x + zone_x, frame_y + zone_y. */
    ok &= expect_int("zone.closed.resolve",
                     csb_v1_viewport_d3l2_door_resolve_zone_pc34(spec, 0, 0, &x, &y), 0);
    ok &= expect_int("zone.closed.x", x, 24);
    ok &= expect_int("zone.closed.y", y, 28);
    ok &= expect_int("zone.vertical_half.resolve",
                     csb_v1_viewport_d3l2_door_resolve_zone_pc34(spec, 0, 20, &x, &y), 0);
    ok &= expect_int("zone.vertical_half.x", x, 24);
    ok &= expect_int("zone.vertical_half.y", y, 48);
    ok &= expect_int("zone.null.reject",
                     csb_v1_viewport_d3l2_door_resolve_zone_pc34(NULL, 0, 0, &x, &y), -1);

    return ok;
}

static int test_c10_frame_clip(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2DoorRouteSpec *spec =
        csb_v1_viewport_d3l2_door_route_spec_pc34();
    uint8_t source[12];
    uint8_t destination[12];

    for (size_t i = 0; i < sizeof(source); ++i) source[i] = 10;
    for (size_t i = 0; i < sizeof(destination); ++i) destination[i] = 77;
    source[0] = 1;
    source[3] = 2;
    source[8] = 3;
    source[11] = 4;

    /* CSBCode.cpp BltShapeToViewport lines 2912-2929 copies through the frame
     * rectangle only when width is nonzero and uses transparent color 10;
     * ReDMCSB DUNVIEW.C F0111 line 4334 preserves the same C10 contract for
     * the following D3L2 door panel blit. */
    ok &= expect_int("clip.copied",
                     csb_v1_viewport_d3l2_door_apply_c03_frame_clip_pc34(
                         spec, source, 4, destination, 4, 4, 3),
                     4);
    ok &= expect_int("clip.pixel_0_0", destination[0], 1);
    ok &= expect_int("clip.transparent_1_0", destination[1], 77);
    ok &= expect_int("clip.pixel_3_0", destination[3], 2);
    ok &= expect_int("clip.pixel_0_2", destination[8], 3);
    ok &= expect_int("clip.pixel_3_2", destination[11], 4);

    /* ReDMCSB: COORD.C lines 1556 and 1559 constrain the C03 door clip to
     * 48x40 before F0111 line 4334; oversized synthetic frame clips are
     * rejected so the helper cannot drift past the source rectangle. */
    ok &= expect_int("clip.reject_width",
                     csb_v1_viewport_d3l2_door_apply_c03_frame_clip_pc34(
                         spec, source, 4, destination, 4, 49, 1),
                     -1);
    ok &= expect_int("clip.reject_null",
                     csb_v1_viewport_d3l2_door_apply_c03_frame_clip_pc34(
                         NULL, source, 4, destination, 4, 4, 3),
                     -1);

    return ok;
}

static int test_source_evidence(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d3l2_door_source_evidence_pc34();

    /* ReDMCSB: DUNVIEW.C line 4218 names F0111, lines 6271-6273 name the
     * D3L2 door route, and CSB Viewport.cpp lines 1813-1820 name the CSB
     * frame-before-door detour that this non-duplicative gate covers. */
    ok &= expect_contains("evidence.f0111", e, "F0111_DUNGEONVIEW_DrawDoor");
    ok &= expect_contains("evidence.d3l2", e, "F0676 D3L2");
    ok &= expect_contains("evidence.csb_route", e, "StdDrawF3L1DoorFacing");
    ok &= expect_contains("evidence.c03", e, "C03 layout");
    ok &= expect_contains("evidence.c10", e, "C10");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d3l2_door_pc34_compat\n");
    printf("sourceEvidence=%s\n", csb_v1_viewport_d3l2_door_source_evidence_pc34());

    ok &= test_route_spec();
    ok &= test_c03_zone_math();
    ok &= test_c10_frame_clip();
    ok &= test_source_evidence();

    return ok ? 0 : 1;
}
