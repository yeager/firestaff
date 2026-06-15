#include "csb_v1_viewport_d3l2_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_int(const char *label, int got, int want, const char *anchor)
{
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

static int expect_absent(const char *label, const char *haystack,
                         const char *needle, const char *anchor)
{
    const int got = !haystack || !needle || strstr(haystack, needle) == NULL;
    return expect_int(label, got, 1, anchor);
}

static int test_route_spec(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2WallRouteSpec *spec =
        csb_v1_viewport_d3l2_wall_route_spec_pc34();

    /* ReDMCSB: DUNVIEW.C:6253-6264 routes the D3L2 WALL case through
     * C11_WALL_D3L2 and C702_ZONE_WALL_D3L2 before the F0107 return;
     * DEFS.H:2610 defines C14_VIEW_SQUARE_D3L2 and DEFS.H:4042 defines
     * C702_ZONE_WALL_D3L2. */
    ok &= expect_int("route.present", spec != NULL, 1,
                     "ReDMCSB DUNVIEW.C:6253-6264");
    ok &= expect_int("route.contract_only",
                     spec ? spec->source_locked_contract_only : -1, 1,
                     "ReDMCSB DUNVIEW.C:6253-6264 source-locked gate only");
    ok &= expect_int("route.view_square",
                     spec ? spec->d3l2.view_square : -1, 14,
                     "ReDMCSB DEFS.H:2610 C14_VIEW_SQUARE_D3L2");
    ok &= expect_int("route.wall_zone",
                     spec ? spec->d3l2.wall_zone : -1, 702,
                     "ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2");
    ok &= expect_int("route.native_wall",
                     spec ? spec->d3l2.native_wall_index : -1, 11,
                     "ReDMCSB DUNVIEW.C:6259; DEFS.H:3433-3434 C11_WALL_D3L2");

    /* ReDMCSB: DUNVIEW.C:579 fixes G0711 to {0,15,25,73,8,49,0,0};
     * CSB lineage: Viewport.cpp:659/2267 binds StdWallBitmapF3L2 to
     * pWallBitmaps[5], Viewport.cpp:465/2304 binds StdWallRectangleF3L2
     * to wallRectangles[13], and Viewport.cpp:954-957 uses StdBltShape. */
    ok &= expect_int("route.frame_x1",
                     spec ? spec->d3l2.frame_x1 : -1, 0,
                     "ReDMCSB DUNVIEW.C:579 G0711");
    ok &= expect_int("route.frame_x2",
                     spec ? spec->d3l2.frame_x2 : -1, 15,
                     "ReDMCSB DUNVIEW.C:579 G0711");
    ok &= expect_int("route.frame_y1",
                     spec ? spec->d3l2.frame_y1 : -1, 25,
                     "ReDMCSB DUNVIEW.C:579 G0711");
    ok &= expect_int("route.frame_y2",
                     spec ? spec->d3l2.frame_y2 : -1, 73,
                     "ReDMCSB DUNVIEW.C:579 G0711");
    ok &= expect_int("route.frame_width",
                     spec ? spec->d3l2.byte_width : -1, 8,
                     "ReDMCSB DUNVIEW.C:579 G0711");
    ok &= expect_int("route.frame_height",
                     spec ? spec->d3l2.height : -1, 49,
                     "ReDMCSB DUNVIEW.C:579 G0711");
    ok &= expect_int("route.wall_bitmap_command",
                     spec ? spec->d3l2.bitmap_command : -1, 60258,
                     "CSB Viewport.cpp:659 StdWallBitmapF3L2");
    ok &= expect_int("route.wall_bitmap_index",
                     spec ? spec->d3l2.bitmap_index : -1, 5,
                     "CSB Viewport.cpp:2267 pWallBitmaps[5]");
    ok &= expect_int("route.wall_rect_command",
                     spec ? spec->d3l2.rect_command : -1, 60082,
                     "CSB Viewport.cpp:465 StdWallRectangleF3L2");
    ok &= expect_int("route.wall_rect_index",
                     spec ? spec->d3l2.rect_index : -1, 13,
                     "CSB Viewport.cpp:2304 wallRectangles[13]");
    ok &= expect_int("route.wall_blit_command",
                     spec ? spec->d3l2.blit_command : -1, 60001,
                     "CSB Viewport.cpp:374/956-957/2959-2962 StdBltShape");

    /* ReDMCSB: DEFS.H:2088 defines C10_COLOR_FLESH; DUNVIEW.C:3126-3129
     * sends the D3L2 wall bitmap through F0132 with C10 transparency. */
    ok &= expect_int("route.transparent",
                     spec ? spec->transparent_color : -1, 10,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("route.preserves_transparent",
                     spec ? spec->preserves_c10_transparency : -1, 1,
                     "ReDMCSB DUNVIEW.C:3113-3129 F0104 C10 blit");

    return ok;
}

static int test_c03_zone_math(void)
{
    int ok = 1;
    int x = -1;
    int y = -1;
    int width = -1;
    int height = -1;
    const CSB_V1_ViewportD3L2WallRouteSpec *spec =
        csb_v1_viewport_d3l2_wall_route_spec_pc34();

    /* ReDMCSB: this is the CSB C03_GAME wall contract, but the D3L2 wall
     * zone is not the D3L2 door's G3018_s_LayoutData03/C3700 route. The
     * wall uses COORD.C:1491-1498 G3019_s_LayoutData04 records 702/703,
     * parent record 4, and the viewport clip parent chain at COORD.C:1656-1658. */
    ok &= expect_int("c03.csb_exetype",
                     spec ? spec->d3l2.csb_exetype_c03 : -1, 3,
                     "ReDMCSB DEFS.H C03_GAME");
    ok &= expect_int("c03.wall_layout_range",
                     spec ? spec->d3l2.coord_layout_range : -1, 4,
                     "ReDMCSB COORD.C:1491-1494 G3019_s_LayoutData04");
    ok &= expect_int("c03.zone_record_type",
                     spec ? spec->d3l2.zone_record_type : -1, 11,
                     "ReDMCSB COORD.C:1495-1498 record 702 type 11");
    ok &= expect_int("c03.zone_parent_record",
                     spec ? spec->d3l2.zone_parent_record : -1, 4,
                     "ReDMCSB COORD.C:1495-1498 record 702 parent 4");
    ok &= expect_int("c03.viewport_clip_record",
                     spec ? spec->d3l2.viewport_clip_record : -1, 3,
                     "ReDMCSB COORD.C:1656-1658 record 3/4 viewport clip chain");

    /* ReDMCSB: COORD.C:1495-1498 sets records 702/703 at y=25, while
     * DUNVIEW.C:579-580 G0711/G0712 supply the exact 8x49 wall frame that
     * the C10 clip path consumes. */
    ok &= expect_int("c03.resolve_d3l2",
                     csb_v1_viewport_d3l2_wall_resolve_zone_pc34(
                         spec, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3L2,
                         &x, &y, &width, &height),
                     0,
                     "ReDMCSB COORD.C:1495-1498; DUNVIEW.C:579");
    ok &= expect_int("c03.d3l2.x", x, 0,
                     "ReDMCSB DUNVIEW.C:579 G0711 x1");
    ok &= expect_int("c03.d3l2.y", y, 25,
                     "ReDMCSB COORD.C:1497 record 702 y=25; DUNVIEW.C:579");
    ok &= expect_int("c03.d3l2.width", width, 8,
                     "ReDMCSB DUNVIEW.C:579 G0711 byte width");
    ok &= expect_int("c03.d3l2.height", height, 49,
                     "ReDMCSB DUNVIEW.C:579 G0711 height");

    /* ReDMCSB: DUNVIEW.C:580 G0712 mirrors the wall frame on the D3R2 side,
     * and COORD.C:1498 assigns record 703 to the same parent/clip chain. */
    ok &= expect_int("c03.resolve_d3r2",
                     csb_v1_viewport_d3l2_wall_resolve_zone_pc34(
                         spec, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3R2,
                         &x, &y, &width, &height),
                     0,
                     "ReDMCSB COORD.C:1498; DUNVIEW.C:580");
    ok &= expect_int("c03.d3r2.x", x, 208,
                     "ReDMCSB DUNVIEW.C:580 G0712 x1");
    ok &= expect_int("c03.d3r2.y", y, 25,
                     "ReDMCSB COORD.C:1498 record 703 y=25; DUNVIEW.C:580");
    ok &= expect_int("c03.null.reject",
                     csb_v1_viewport_d3l2_wall_resolve_zone_pc34(
                         NULL, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3L2,
                         &x, &y, &width, &height),
                     -1,
                     "ReDMCSB COORD.C:2390-2409 rejects unresolved clips");

    return ok;
}

static int test_c10_frame_clip(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2WallRouteSpec *spec =
        csb_v1_viewport_d3l2_wall_route_spec_pc34();
    uint8_t source[8 * 49];
    uint8_t destination[224 * 80];
    uint8_t small_destination[4 * 30];
    uint8_t right_clip_destination[210 * 80];

    memset(source, 10, sizeof(source));
    memset(destination, 0xee, sizeof(destination));
    source[0 * 8 + 0] = 10;
    source[0 * 8 + 1] = 0x42;
    source[0 * 8 + 7] = 0x7e;
    source[48 * 8 + 7] = 0x55;

    /* ReDMCSB: DUNVIEW.C:3126-3129 F0104 keeps color 10 transparent and
     * blits the G0711 8x49 D3L2 frame from DUNVIEW.C:579; IMAGE3.C:866-889
     * is the CSB C03_GAME pixel loop that skips only transparent pixels. */
    ok &= expect_int("clip.full.copied",
                     csb_v1_viewport_d3l2_wall_apply_c10_frame_clip_pc34(
                         spec, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3L2,
                         source, 8, destination, 224, 80, 0),
                     3,
                     "ReDMCSB DUNVIEW.C:3126-3129; IMAGE3.C:866-889");
    ok &= expect_int("clip.pixel.transparent",
                     destination[25 * 224 + 0], 0xee,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("clip.pixel.full",
                     destination[25 * 224 + 1], 0x42,
                     "ReDMCSB DUNVIEW.C:579/3126-3129 full D3L2 pixel");
    ok &= expect_int("clip.pixel.right_edge",
                     destination[25 * 224 + 7], 0x7e,
                     "ReDMCSB DUNVIEW.C:579 G0711 right edge");
    ok &= expect_int("clip.pixel.bottom_edge",
                     destination[73 * 224 + 7], 0x55,
                     "ReDMCSB DUNVIEW.C:579 G0711 bottom edge");
    ok &= expect_int("clip.pixel.after_frame",
                     destination[74 * 224 + 7], 0xee,
                     "ReDMCSB COORD.C:2390-2409 clips beyond frame height");

    memset(source, 10, sizeof(source));
    memset(small_destination, 0xee, sizeof(small_destination));
    source[0 * 8 + 0] = 0x11;
    source[0 * 8 + 3] = 0x22;
    source[0 * 8 + 7] = 0x77;
    source[30 * 8 + 0] = 0x33;

    /* ReDMCSB: COORD.C:2390-2409 clips the wall frame to the viewport; with
     * a small synthetic viewport only columns 0..3 and rows through y=29
     * survive, while the source-locked G0711 dimensions remain 8x49. */
    ok &= expect_int("clip.partial.copied",
                     csb_v1_viewport_d3l2_wall_apply_c10_frame_clip_pc34(
                         spec, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3L2,
                         source, 8, small_destination, 4, 30, 0),
                     2,
                     "ReDMCSB COORD.C:2390-2409 clipped intersection");
    ok &= expect_int("clip.partial.pixel_0",
                     small_destination[25 * 4 + 0], 0x11,
                     "ReDMCSB DUNVIEW.C:579/3126-3129 clipped D3L2 pixel");
    ok &= expect_int("clip.partial.pixel_3",
                     small_destination[25 * 4 + 3], 0x22,
                     "ReDMCSB DUNVIEW.C:579/3126-3129 clipped D3L2 pixel");

    memset(source, 10, sizeof(source));
    memset(right_clip_destination, 0xee, sizeof(right_clip_destination));
    source[0 * 8 + 0] = 0x31;
    source[0 * 8 + 1] = 0x32;
    source[0 * 8 + 2] = 0x33;

    /* ReDMCSB: DUNVIEW.C:580 G0712 starts D3R2 at x=208; COORD.C:2390-2409
     * clips an over-tight right edge so only x=208 and x=209 are written. */
    ok &= expect_int("clip.over_right.copied",
                     csb_v1_viewport_d3l2_wall_apply_c10_frame_clip_pc34(
                         spec, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3R2,
                         source, 8, right_clip_destination, 210, 80, 0),
                     2,
                     "ReDMCSB DUNVIEW.C:580; COORD.C:2390-2409");
    ok &= expect_int("clip.over_right.pixel_208",
                     right_clip_destination[25 * 210 + 208], 0x31,
                     "ReDMCSB DUNVIEW.C:580 G0712 left clipped pixel");
    ok &= expect_int("clip.over_right.pixel_209",
                     right_clip_destination[25 * 210 + 209], 0x32,
                     "ReDMCSB DUNVIEW.C:580 G0712 clipped edge write");
    ok &= expect_int("clip.reject_null",
                     csb_v1_viewport_d3l2_wall_apply_c10_frame_clip_pc34(
                         NULL, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3L2,
                         source, 8, destination, 224, 80, 0),
                     -1,
                     "ReDMCSB COORD.C:2390-2409 unresolved clip reject");

    return ok;
}

static int test_parity_d3r2_wall_flip(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2WallRouteSpec *spec =
        csb_v1_viewport_d3l2_wall_route_spec_pc34();
    uint8_t source[8 * 49];
    uint8_t destination[224 * 80];
    uint8_t clipped_destination[210 * 80];

    memset(source, 10, sizeof(source));
    memset(destination, 0xee, sizeof(destination));
    source[0 * 8 + 7] = 10;
    source[0 * 8 + 6] = 0x73;
    source[0 * 8 + 0] = 0x7a;

    /* ReDMCSB: DUNVIEW.C:6320-6327 makes D3R2 the parity side; the optional
     * F0105 route at DUNVIEW.C:6321-6324 uses C11_WALL_D3L2 as the opposite
     * native bitmap, flips it through the F0105 scratch path at 3185-3204,
     * preserves C10, and writes the C703/G0712 frame. */
    ok &= expect_int("parity.d3r2.native_wall",
                     spec ? spec->d3r2.native_wall_index : -1, 10,
                     "ReDMCSB DUNVIEW.C:6326; DEFS.H:3433 C10_WALL_D3R2");
    ok &= expect_int("parity.d3r2.opposite_wall",
                     spec ? spec->d3r2.opposite_wall_index : -1, 11,
                     "ReDMCSB DUNVIEW.C:6323; DEFS.H:3434 C11_WALL_D3L2");
    ok &= expect_int("parity.d3r2.uses_f0105",
                     spec ? spec->d3r2.uses_f0105_scratch_flip : -1, 1,
                     "ReDMCSB DUNVIEW.C:3185-3204 F0105 scratch flip");
    ok &= expect_int("parity.d3r2.copied",
                     csb_v1_viewport_d3l2_wall_apply_c10_frame_clip_pc34(
                         spec, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3R2,
                         source, 8, destination, 224, 80, 1),
                     2,
                     "ReDMCSB DUNVIEW.C:3185-3204 F0105 C10 flip");
    ok &= expect_int("parity.d3r2.transparent_preserved",
                     destination[25 * 224 + 208], 0xee,
                     "ReDMCSB DEFS.H:2088; DUNVIEW.C:3201 C10 transparent");
    ok &= expect_int("parity.d3r2.flipped_next",
                     destination[25 * 224 + 209], 0x73,
                     "ReDMCSB DUNVIEW.C:6323/3185-3204 flipped D3L2 source");
    ok &= expect_int("parity.d3r2.flipped_right_edge",
                     destination[25 * 224 + 215], 0x7a,
                     "ReDMCSB DUNVIEW.C:580/6323 G0712 flipped edge");
    ok &= expect_int("parity.d3r2.d3l2_zone_untouched",
                     destination[25 * 224 + 0], 0xee,
                     "ReDMCSB DUNVIEW.C:6323 writes C703, not C702");

    memset(source, 10, sizeof(source));
    memset(clipped_destination, 0xee, sizeof(clipped_destination));
    source[0 * 8 + 7] = 0x61;
    source[0 * 8 + 6] = 0x62;
    source[0 * 8 + 5] = 0x63;

    /* ReDMCSB: COORD.C:2390-2409 still clips the F0105 parity blit against
     * the right edge, so G0712 writes only x=208 and x=209 in this gate. */
    ok &= expect_int("parity.d3r2.clipped_edge_copied",
                     csb_v1_viewport_d3l2_wall_apply_c10_frame_clip_pc34(
                         spec, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3R2,
                         source, 8, clipped_destination, 210, 80, 1),
                     2,
                     "ReDMCSB DUNVIEW.C:3185-3204; COORD.C:2390-2409");
    ok &= expect_int("parity.d3r2.clipped_pixel_208",
                     clipped_destination[25 * 210 + 208], 0x61,
                     "ReDMCSB DUNVIEW.C:580 G0712 parity clipped write");
    ok &= expect_int("parity.d3r2.clipped_pixel_209",
                     clipped_destination[25 * 210 + 209], 0x62,
                     "ReDMCSB DUNVIEW.C:580 G0712 parity clipped write");

    return ok;
}

static int test_source_evidence(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d3l2_wall_source_evidence_pc34();

    /* ReDMCSB: evidence must prove this is the wall route, not the just-merged
     * F0111 door route: F0676/F0677 wall cases, G0711/G0712 metadata, C03_GAME
     * coordinate lineage, C10 transparency, and F0105 parity are all required. */
    ok &= expect_contains("evidence.f0676", e, "F0676",
                          "ReDMCSB DUNVIEW.C:6253-6264 F0676 wall path");
    ok &= expect_contains("evidence.f0677", e, "F0677",
                          "ReDMCSB DUNVIEW.C:6320-6331 F0677 wall path");
    ok &= expect_contains("evidence.g0711", e, "G0711",
                          "ReDMCSB DUNVIEW.C:579 G0711 wall frame");
    ok &= expect_contains("evidence.g0712", e, "G0712",
                          "ReDMCSB DUNVIEW.C:580 G0712 wall frame");
    ok &= expect_contains("evidence.c03", e, "C03",
                          "ReDMCSB DEFS.H C03_GAME / COORD.C:1491-1498");
    ok &= expect_contains("evidence.c10", e, "C10",
                          "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_contains("evidence.parity", e, "F0105",
                          "ReDMCSB DUNVIEW.C:3185-3204 parity scratch flip");
    ok &= expect_contains("evidence.csbroute", e, "StdDrawF3L2Stone",
                          "CSB Viewport.cpp:954-963 wall route");
    ok &= expect_absent("evidence.not_f0111", e, "F0111_DUNGEONVIEW_DrawDoor",
                        "ReDMCSB DUNVIEW.C:6253-6264 wall route is not F0111");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d3l2_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n", csb_v1_viewport_d3l2_wall_source_evidence_pc34());

    ok &= test_route_spec();
    ok &= test_c03_zone_math();
    ok &= test_c10_frame_clip();
    ok &= test_parity_d3r2_wall_flip();
    ok &= test_source_evidence();

    if (ok) {
        printf("PASS csb_v1_viewport_d3l2_wall_pc34_compat\n");
    }
    return ok ? 0 : 1;
}
