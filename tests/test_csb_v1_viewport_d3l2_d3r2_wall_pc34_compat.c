#include "csb_v1_viewport_d3l2_d3r2_wall_pc34_compat.h"

#include "csb_v1_viewport_d3l2_wall_pc34_compat.h"

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

static int build_sources(uint8_t *left_source, uint8_t *right_source, int stride)
{
    if (!left_source || !right_source || stride < 8) return -1;
    memset(left_source, 10, (size_t)stride * 49);
    memset(right_source, 10, (size_t)stride * 49);
    left_source[0 * stride + 0] = 0x21;
    left_source[0 * stride + 1] = 0x22;
    left_source[48 * stride + 7] = 0x23;
    right_source[0 * stride + 7] = 0x31;
    right_source[0 * stride + 6] = 0x32;
    right_source[48 * stride + 0] = 0x33;
    return 0;
}

static int test_relative_positions(void)
{
    int ok = 1;
    CSB_V1_ViewportD3L2D3R2WallPartyPositionPc34 party = { 10, 10, 0 };
    CSB_V1_ViewportD3L2D3R2WallPositionPc34 left;
    CSB_V1_ViewportD3L2D3R2WallPositionPc34 right;

    ok &= expect_int("route.count",
                     (int)csb_v1_viewport_d3l2_d3r2_wall_route_spec_count_pc34(),
                     2,
                     "ReDMCSB DUNGEON.C:1423-1504 F0151/F0152/F0153; DUNVIEW.C:8482-8487");
    ok &= expect_int("resolve.left",
                     csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
                         &party, 3, -2, &left),
                     0,
                     "ReDMCSB DUNGEON.C:1481-1488 F0152; DUNVIEW.C:8482-8483 F0676");
    ok &= expect_int("resolve.right",
                     csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
                         &party, 3, 2, &right),
                     0,
                     "ReDMCSB DUNGEON.C:1481-1488 F0152; DUNVIEW.C:8485-8487 F0677");
    ok &= expect_int("left.map_x", left.map_x, 8,
                     "ReDMCSB DUNGEON.C:1481-1488 F0152 depth 3 lateral -2");
    ok &= expect_int("left.map_y", left.map_y, 7,
                     "ReDMCSB DUNGEON.C:1481-1488 F0152 depth 3 lateral -2");
    ok &= expect_int("right.map_x", right.map_x, 12,
                     "ReDMCSB DUNGEON.C:1481-1488 F0152 depth 3 lateral +2");
    ok &= expect_int("right.map_y", right.map_y, 7,
                     "ReDMCSB DUNGEON.C:1481-1488 F0152 depth 3 lateral +2");
    ok &= expect_int("left.view_square", left.view_square, 14,
                     "ReDMCSB DEFS.H:2610 C14_VIEW_SQUARE_D3L2");
    ok &= expect_int("right.view_square", right.view_square, 15,
                     "ReDMCSB DEFS.H:2611 C15_VIEW_SQUARE_D3R2");
    ok &= expect_int("reject.depth2",
                     csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
                         &party, 2, -2, &left),
                     -1,
                     "ReDMCSB DUNVIEW.C:8482-8487 only depth 3 for this gate");

    return ok;
}

static int test_render_pair_and_pixels(void)
{
    int ok = 1;
    CSB_V1_ViewportD3L2D3R2WallPartyPositionPc34 party = { 10, 10, 0 };
    CSB_V1_ViewportD3L2D3R2WallPositionPc34 left;
    CSB_V1_ViewportD3L2D3R2WallPositionPc34 right;
    CSB_V1_ViewportD3L2D3R2WallRenderResultPc34 result;
    uint8_t left_source[8 * 49];
    uint8_t right_source[8 * 49];
    uint8_t destination[224 * 80];

    memset(destination, 0xee, sizeof(destination));
    ok &= expect_int("build.sources", build_sources(left_source, right_source, 8), 0,
                     "ReDMCSB DUNVIEW.C:579-580 G0711/G0712 8x49 depth-3 frames");
    ok &= expect_int("resolve.left.for.render",
                     csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
                         &party, 3, -2, &left),
                     0,
                     "ReDMCSB DUNGEON.C:1481-1504 F0152/F0153; DUNVIEW.C:8482");
    ok &= expect_int("resolve.right.for.render",
                     csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
                         &party, 3, 2, &right),
                     0,
                     "ReDMCSB DUNGEON.C:1481-1504 F0152/F0153; DUNVIEW.C:8486");
    ok &= expect_int("render.return",
                     csb_v1_viewport_d3l2_d3r2_wall_render_square_pc34(
                         &party, &left, &right, left_source, right_source, 8,
                         destination, 224, 80, &result),
                     0,
                     "ReDMCSB DUNVIEW.C:8482-8487 left then right D3 ring");
    ok &= expect_int("render.ok", result.ok, 1,
                     "ReDMCSB DUNVIEW.C:6253-6264/6320-6331 wall routes");
    ok &= expect_int("render.left_drawn", result.left_drawn, 1,
                     "ReDMCSB DUNVIEW.C:6259 F0104 C702 wall blit");
    ok &= expect_int("render.right_drawn", result.right_drawn, 1,
                     "ReDMCSB DUNVIEW.C:6323/6326 F0105/F0104 C703 wall blit");
    ok &= expect_int("render.order", result.draw_order_left_then_right, 1,
                     "ReDMCSB DUNVIEW.C:8482-8487 F0676 before F0677");
    ok &= expect_int("render.relative_gate", result.relative_square_gate_ok, 1,
                     "ReDMCSB DUNGEON.C:1423-1504 F0151/F0152/F0153");
    ok &= expect_int("render.left_pixels", result.left_copied_pixels, 3,
                     "ReDMCSB DUNVIEW.C:3113-3129 F0104 C10 transparency");
    ok &= expect_int("render.right_pixels", result.right_copied_pixels, 3,
                     "ReDMCSB DUNVIEW.C:3185-3204 F0105 C10 transparency");
    ok &= expect_int("pixel.left.top", destination[25 * 224 + 0], 0x21,
                     "ReDMCSB DUNVIEW.C:579 G0711 top edge");
    ok &= expect_int("pixel.left.bottom", destination[73 * 224 + 7], 0x23,
                     "ReDMCSB DUNVIEW.C:579 G0711 bottom edge");
    ok &= expect_int("pixel.right.top_flipped", destination[25 * 224 + 208], 0x31,
                     "ReDMCSB DUNVIEW.C:580 G0712; DUNVIEW.C:3185-3204 F0105");
    ok &= expect_int("pixel.right.bottom_flipped", destination[73 * 224 + 215], 0x33,
                     "ReDMCSB DUNVIEW.C:580 G0712 bottom edge");
    ok &= expect_int("pixel.left.transparent", destination[25 * 224 + 2], 0xee,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("pixel.right.transparent", destination[25 * 224 + 210], 0xee,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");

    return ok;
}

static int test_depth3_attenuation_and_routes(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2WallRouteSpec *helper =
        csb_v1_viewport_d3l2_wall_route_spec_pc34();
    const char *e = csb_v1_viewport_d3l2_d3r2_wall_source_evidence_pc34();

    ok &= expect_int("helper.present", helper != NULL, 1,
                     "ReDMCSB DUNVIEW.C:6253-6264/6320-6331 helper route");
    ok &= expect_int("d3.width", helper ? helper->d3l2.byte_width : -1, 8,
                     "ReDMCSB DUNVIEW.C:579 G0711 width attenuated at depth 3");
    ok &= expect_int("d3.height", helper ? helper->d3l2.height : -1, 49,
                     "ReDMCSB DUNVIEW.C:579 G0711 height attenuated at depth 3");
    ok &= expect_int("d3.smaller_than_d2_width",
                     helper ? helper->d3l2.byte_width < 24 : 0, 1,
                     "ReDMCSB DUNVIEW.C:579-580 depth-3 vs D2 C707/C708 route");
    ok &= expect_int("d3.smaller_than_d2_height",
                     helper ? helper->d3l2.height < 65 : 0, 1,
                     "ReDMCSB DUNVIEW.C:579-580 depth-3 attenuation");
    ok &= expect_int("d3.left_zone", helper ? helper->d3l2.wall_zone : -1, 702,
                     "ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2");
    ok &= expect_int("d3.right_zone", helper ? helper->d3r2.wall_zone : -1, 703,
                     "ReDMCSB DEFS.H:4043 C703_ZONE_WALL_D3R2");
    ok &= expect_int("d3.same_top_edge",
                     helper ? helper->d3l2.frame_y1 == helper->d3r2.frame_y1 : 0,
                     1,
                     "ReDMCSB DUNVIEW.C:579-580 G0711/G0712 same top edge");
    ok &= expect_int("d3.same_bottom_edge",
                     helper ? helper->d3l2.frame_y2 == helper->d3r2.frame_y2 : 0,
                     1,
                     "ReDMCSB DUNVIEW.C:579-580 G0711/G0712 same bottom edge");
    ok &= expect_int("d3.ornament.right",
                     helper ? helper->d3l2.native_wall_index == 11 : 0, 1,
                     "ReDMCSB DUNVIEW.C:6263 F0107 C00_VIEW_WALL_D3L2_RIGHT");
    ok &= expect_int("d3.ornament.left",
                     helper ? helper->d3r2.native_wall_index == 10 : 0, 1,
                     "ReDMCSB DUNVIEW.C:6330 F0107 C01_VIEW_WALL_D3R2_LEFT");
    ok &= expect_contains("evidence.f0107", e, "F0107",
                          "ReDMCSB DUNVIEW.C:3502-3939 wall ornament/lighting");
    ok &= expect_contains("evidence.depth3", e, "attenuated depth-3 8x49",
                          "ReDMCSB DUNVIEW.C:579-580 G0711/G0712");
    ok &= expect_contains("evidence.f0151", e, "F0151_DUNGEON_GetSquare",
                          "ReDMCSB DUNGEON.C:1423-1478 F0151");
    ok &= expect_contains("evidence.f0152", e, "F0152_DUNGEON_GetRelativeSquare",
                          "ReDMCSB DUNGEON.C:1481-1488 F0152");
    ok &= expect_contains("evidence.f0153", e, "F0153_DUNGEON_GetRelativeSquareType",
                          "ReDMCSB DUNGEON.C:1495-1504 F0153");

    return ok;
}

static int test_no_floor_ceiling_door_or_thing_leak(void)
{
    int ok = 1;
    CSB_V1_ViewportD3L2D3R2WallPartyPositionPc34 party = { 3, 4, 1 };
    CSB_V1_ViewportD3L2D3R2WallPositionPc34 left;
    CSB_V1_ViewportD3L2D3R2WallPositionPc34 right;
    CSB_V1_ViewportD3L2D3R2WallRenderResultPc34 result;
    uint8_t left_source[8 * 49];
    uint8_t right_source[8 * 49];
    uint8_t destination[224 * 80];

    memset(destination, 0xcc, sizeof(destination));
    ok &= expect_int("leak.sources", build_sources(left_source, right_source, 8), 0,
                     "ReDMCSB DUNVIEW.C:579-580 G0711/G0712 wall-only source");
    ok &= expect_int("leak.resolve.left",
                     csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
                         &party, 3, -2, &left),
                     0,
                     "ReDMCSB DUNGEON.C:1481-1504 F0152/F0153");
    ok &= expect_int("leak.resolve.right",
                     csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
                         &party, 3, 2, &right),
                     0,
                     "ReDMCSB DUNGEON.C:1481-1504 F0152/F0153");
    ok &= expect_int("leak.render",
                     csb_v1_viewport_d3l2_d3r2_wall_render_square_pc34(
                         &party, &left, &right, left_source, right_source, 8,
                         destination, 224, 80, &result),
                     0,
                     "ReDMCSB DUNVIEW.C:6253-6264/6320-6331 wall return before door/thing");
    ok &= expect_int("leak.before_top", destination[24 * 224 + 0], 0xcc,
                     "ReDMCSB DUNVIEW.C:579-580 wall band starts at y=25");
    ok &= expect_int("leak.after_bottom", destination[74 * 224 + 215], 0xcc,
                     "ReDMCSB DUNVIEW.C:579-580 wall band ends at y=73");
    ok &= expect_int("leak.center_ceiling_floor", destination[10 * 224 + 112], 0xcc,
                     "ReDMCSB DUNVIEW.C:6253-6264 wall route has no floor/ceiling blit");
    ok &= expect_int("leak.center_floor", destination[79 * 224 + 112], 0xcc,
                     "ReDMCSB DUNVIEW.C:6320-6331 wall route has no floor leak");
    ok &= expect_int("leak.door_suppressed", result.door_route_suppressed_for_wall_ok, 1,
                     "ReDMCSB DUNVIEW.C:6253-6264/6320-6331 return before F0111");
    ok &= expect_int("leak.thing_suppressed", result.thing_pass_suppressed_for_wall_ok, 1,
                     "ReDMCSB DUNVIEW.C:6253-6264/6320-6331 return before F0115");
    ok &= expect_int("leak.bad_square_rejected",
                     (left.square_type = 17,
                      csb_v1_viewport_d3l2_d3r2_wall_render_square_pc34(
                          &party, &left, &right, left_source, right_source, 8,
                          destination, 224, 80, &result)),
                     1,
                     "ReDMCSB DUNGEON.C:1495-1504 F0153 rejects non-wall for wall slice");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d3l2_d3r2_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d3l2_d3r2_wall_source_evidence_pc34());

    ok &= test_relative_positions();
    ok &= test_render_pair_and_pixels();
    ok &= test_depth3_attenuation_and_routes();
    ok &= test_no_floor_ceiling_door_or_thing_leak();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_50", g_assertions >= 50, 1,
                     "ReDMCSB DUNGEON.C:1423-1504 and DUNVIEW.C:6253-6331 source-lock gate");

    if (ok) {
        printf("PASS csb_v1_viewport_d3l2_d3r2_wall_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
