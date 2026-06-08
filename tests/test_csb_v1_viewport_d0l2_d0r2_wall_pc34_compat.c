/*
 * CSB V1 D0L2/D0R2 wall row source-lock gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:7960-8062 F0125 and 8064-8162 F0126 route the D0
 *   side WALL cases through C716/C717 and return before F0115.
 * - DUNVIEW.C:3113-3156 F0104 and 3185-3247 F0105 bind native/flipped
 *   wall variants with C10_COLOR_FLESH transparency.
 * - DUNVIEW.C:4547-4581 F0115 is kept out of WALL rows.
 * - DUNVIEW.C:8478-8508 and 8534-8542 anchor the unified D*-L2/D*-R2
 *   wall-row order and the D0L/D0R post-row follow-up into F0127.
 * - DUNGEON.C:1769-1838 F0163, 1840-1905 F0164, and 2466-2523 F0172
 *   anchor thing-list mutation and square-aspect boundaries.
 */
#include "csb_v1_viewport_d0l2_d0r2_wall_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    VIEWPORT_WIDTH = 224,
    VIEWPORT_HEIGHT = 136,
    SOURCE_WIDTH = 32,
    SOURCE_HEIGHT = 136,
    TRANSPARENT = 10
};

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

static int expect_u16(const char *label, uint16_t got, uint16_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%04x want=0x%04x anchor=%s\n",
               label, (unsigned int)got, (unsigned int)want, anchor);
        return 0;
    }
    printf("ok %s=0x%04x anchor=%s\n", label, (unsigned int)got, anchor);
    return 1;
}

static int expect_contains(
    const char *label,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    const int got = haystack && needle && strstr(haystack, needle) != NULL;
    return expect_int(label, got, 1, anchor);
}

static size_t viewport_offset(int y, int x)
{
    return (size_t)y * VIEWPORT_WIDTH + (size_t)x;
}

static size_t source_offset(int y, int x)
{
    return (size_t)y * SOURCE_WIDTH + (size_t)x;
}

static int test_run_entry_point(void)
{
    int ok = 1;
    CSB_V1_D0L2D0R2WallRunResultPc34 result;

    ok &= expect_int("run.return",
                     csb_v1_viewport_d0l2_d0r2_wall_pc34_compat_run(&result),
                     0,
                     "ReDMCSB DUNVIEW.C:8534-8542 F0128 D0L/D0R row");
    ok &= expect_int("run.ok", result.ok, 1,
                     "contract-only D0L2/D0R2 wall gate");
    ok &= expect_int("run.route_count", result.route_count, 2,
                     "ReDMCSB DUNVIEW.C:7960-8162 F0125/F0126");
    ok &= expect_int("run.dispatch_order_ok", result.dispatch_order_ok, 1,
                     "ReDMCSB DUNVIEW.C:8536-8541 F0128 D0L then D0R");
    ok &= expect_int("run.wall_variant_binding_ok", result.wall_variant_binding_ok, 1,
                     "ReDMCSB DUNVIEW.C:8017/8033 and 8127/8139");
    ok &= expect_int("run.c10_transparency_ok", result.c10_transparency_ok, 1,
                     "ReDMCSB DUNVIEW.C:3113-3156/3185-3247; DEFS.H:2088");
    ok &= expect_int("run.f0115_keepout_ok", result.f0115_keepout_ok, 1,
                     "ReDMCSB DUNVIEW.C:4547-4581 F0115 keep-out");
    ok &= expect_int("run.thing_list_keepout_ok", result.thing_list_keepout_ok, 1,
                     "ReDMCSB DUNGEON.C:1769-1905 F0163/F0164 not reached");
    ok &= expect_int("run.row_followup_ok", result.row_followup_ok, 1,
                     "ReDMCSB DUNVIEW.C:8542 F0127 follows D0 side pair");
    ok &= expect_int("run.lineage_binding_ok", result.lineage_binding_ok, 1,
                     "CSB-lineage Viewport.cpp:1192-1209 and 1903-1915");
    ok &= expect_int("run.copied_pixels", result.copied_pixels, 4,
                     "synthetic C10 wall-blit row copies four non-C10 sentinels");
    ok &= expect_u16("run.first_thing_preserved",
                     result.first_thing_after, result.first_thing_before,
                     "ReDMCSB DUNGEON.C:1769-1905 thing-list keep-out");

    return ok;
}

static int test_route_identity_and_dispatch(void)
{
    int ok = 1;
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *d0l2 =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(
            CSB_V1_D0L2_D0R2_WALL_SIDE_D0L2_PC34);
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *d0r2 =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(
            CSB_V1_D0L2_D0R2_WALL_SIDE_D0R2_PC34);

    ok &= expect_int("route.count",
                     (int)csb_v1_viewport_d0l2_d0r2_wall_route_spec_count_pc34(),
                     2,
                     "ReDMCSB DUNVIEW.C:7960-8162 unified D0 side pair");
    ok &= expect_int("route.index0.d0l2",
                     csb_v1_viewport_d0l2_d0r2_wall_route_spec_at_pc34(0) == d0l2,
                     1,
                     "ReDMCSB DUNVIEW.C:8536-8537 F0128 D0L first");
    ok &= expect_int("route.index1.d0r2",
                     csb_v1_viewport_d0l2_d0r2_wall_route_spec_at_pc34(1) == d0r2,
                     1,
                     "ReDMCSB DUNVIEW.C:8540-8541 F0128 D0R second");
    ok &= expect_int("route.index2.null",
                     csb_v1_viewport_d0l2_d0r2_wall_route_spec_at_pc34(2) == NULL,
                     1,
                     "D0L2/D0R2 wall-only route table");
    ok &= expect_int("route.unknown.null",
                     csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(99) == NULL,
                     1,
                     "D0L2/D0R2 wall-only side ids");
    ok &= expect_int("d0l2.source_locked", d0l2 ? d0l2->source_locked_contract_only : 0, 1,
                     "contract-only source lock");
    ok &= expect_int("d0r2.source_locked", d0r2 ? d0r2->source_locked_contract_only : 0, 1,
                     "contract-only source lock");
    ok &= expect_int("d0l2.no_real_asset", d0l2 ? d0l2->no_real_asset_bitmap_parity : 0, 1,
                     "no real-asset bitmap parity");
    ok &= expect_int("d0r2.no_game_data", d0r2 ? d0r2->no_game_data_load : 0, 1,
                     "no CSB game-data load");
    ok &= expect_int("d0l2.order", d0l2 ? d0l2->f0128_draw_order_index : -1, 0,
                     "ReDMCSB DUNVIEW.C:8536-8537");
    ok &= expect_int("d0r2.order", d0r2 ? d0r2->f0128_draw_order_index : -1, 1,
                     "ReDMCSB DUNVIEW.C:8540-8541");
    ok &= expect_int("d0l2.depth", d0l2 ? d0l2->f0128_relative_depth : -1, 0,
                     "ReDMCSB DUNVIEW.C:8536 relative depth 0");
    ok &= expect_int("d0r2.depth", d0r2 ? d0r2->f0128_relative_depth : -1, 0,
                     "ReDMCSB DUNVIEW.C:8540 relative depth 0");
    ok &= expect_int("d0l2.lateral", d0l2 ? d0l2->f0128_relative_lateral : 0, -1,
                     "ReDMCSB DUNVIEW.C:8536 relative lateral -1");
    ok &= expect_int("d0r2.lateral", d0r2 ? d0r2->f0128_relative_lateral : 0, 1,
                     "ReDMCSB DUNVIEW.C:8540 relative lateral +1");
    ok &= expect_int("d0l2.view_square", d0l2 ? d0l2->view_square_index : -1, 1,
                     "ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L");
    ok &= expect_int("d0r2.view_square", d0r2 ? d0r2->view_square_index : -1, 2,
                     "ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R");

    return ok;
}

static int test_wall_variant_bindings(void)
{
    int ok = 1;
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *d0l2 =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(1);
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *d0r2 =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(2);

    ok &= expect_int("d0l2.wall_element", d0l2 ? d0l2->wall_element : -1, 0,
                     "ReDMCSB DUNVIEW.C:8007 C00_ELEMENT_WALL");
    ok &= expect_int("d0r2.wall_element", d0r2 ? d0r2->wall_element : -1, 0,
                     "ReDMCSB DUNVIEW.C:8117 C00_ELEMENT_WALL");
    ok &= expect_int("d0l2.wall_zone", d0l2 ? d0l2->wall_zone : -1, 716,
                     "ReDMCSB DEFS.H:4056 C716_ZONE_WALL_D0L");
    ok &= expect_int("d0r2.wall_zone", d0r2 ? d0r2->wall_zone : -1, 717,
                     "ReDMCSB DEFS.H:4057 C717_ZONE_WALL_D0R");
    ok &= expect_int("d0l2.native_wall", d0l2 ? d0l2->native_wall_index : -1, 1,
                     "ReDMCSB DUNVIEW.C:8033 G2107_WallSet[C01_WALL_D0L]");
    ok &= expect_int("d0l2.flipped_wall", d0l2 ? d0l2->flipped_wall_index : -1, 0,
                     "ReDMCSB DUNVIEW.C:8017 G2107_WallSet[C00_WALL_D0R]");
    ok &= expect_int("d0r2.native_wall", d0r2 ? d0r2->native_wall_index : -1, 0,
                     "ReDMCSB DUNVIEW.C:8139 G2107_WallSet[C00_WALL_D0R]");
    ok &= expect_int("d0r2.flipped_wall", d0r2 ? d0r2->flipped_wall_index : -1, 1,
                     "ReDMCSB DUNVIEW.C:8127 G2107_WallSet[C01_WALL_D0L]");
    ok &= expect_int("d0l2.f0104", d0l2 ? d0l2->f0104_native_route : 0, 1,
                     "ReDMCSB DUNVIEW.C:3113-3156 F0104");
    ok &= expect_int("d0r2.f0104", d0r2 ? d0r2->f0104_native_route : 0, 1,
                     "ReDMCSB DUNVIEW.C:3113-3156 F0104");
    ok &= expect_int("d0l2.f0105", d0l2 ? d0l2->f0105_flipped_route : 0, 1,
                     "ReDMCSB DUNVIEW.C:3185-3247 F0105");
    ok &= expect_int("d0r2.f0105", d0r2 ? d0r2->f0105_flipped_route : 0, 1,
                     "ReDMCSB DUNVIEW.C:3185-3247 F0105");
    ok &= expect_int("d0l2.f0104.before_followup",
                     d0l2 ? d0l2->f0104_zone_binding_before_f0128_followup : 0, 1,
                     "ReDMCSB DUNVIEW.C:8033 before F0128 8542 follow-up");
    ok &= expect_int("d0r2.f0105.before_followup",
                     d0r2 ? d0r2->f0105_zone_binding_before_f0128_followup : 0, 1,
                     "ReDMCSB DUNVIEW.C:8127 before F0128 8542 follow-up");
    ok &= expect_int("symmetry.native_flipped_cross",
                     d0l2 && d0r2 ?
                         d0l2->native_wall_index == d0r2->flipped_wall_index &&
                         d0r2->native_wall_index == d0l2->flipped_wall_index : 0,
                     1,
                     "ReDMCSB DUNVIEW.C:8017/8127 opposite-wall flipped pair");
    ok &= expect_int("zone.family.next",
                     d0l2 && d0r2 ? d0l2->wall_zone + 1 == d0r2->wall_zone : 0,
                     1,
                     "ReDMCSB DEFS.H:4040-4057 D*-L/D*-R wall-zone family");

    return ok;
}

static int test_frames_pixels_and_c10(void)
{
    int ok = 1;
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *d0l2 =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(1);
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *d0r2 =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(2);
    uint8_t source[SOURCE_WIDTH * SOURCE_HEIGHT];
    uint8_t viewport[VIEWPORT_WIDTH * VIEWPORT_HEIGHT];
    int source_x = -1;

    memset(source, TRANSPARENT, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[source_offset(12, 0)] = 0x41;
    source[source_offset(12, 1)] = TRANSPARENT;
    source[source_offset(12, 30)] = 0x42;
    source[source_offset(12, 31)] = 0x43;

    ok &= expect_int("d0l2.frame_row", d0l2 ? d0l2->wall_frame_row : -1, 10,
                     "ReDMCSB DUNVIEW.C:592-593 G0163 D0L row");
    ok &= expect_int("d0r2.frame_row", d0r2 ? d0r2->wall_frame_row : -1, 11,
                     "ReDMCSB DUNVIEW.C:594 G0163 D0R row");
    ok &= expect_int("d0l2.frame.x1", d0l2 ? d0l2->wall_frame_x1 : -1, 0,
                     "ReDMCSB DUNVIEW.C:593 G0163 D0L x1");
    ok &= expect_int("d0l2.frame.x2", d0l2 ? d0l2->wall_frame_x2 : -1, 31,
                     "ReDMCSB DUNVIEW.C:593 G0163 D0L x2");
    ok &= expect_int("d0r2.frame.x1", d0r2 ? d0r2->wall_frame_x1 : -1, 192,
                     "ReDMCSB DUNVIEW.C:594 G0163 D0R x1");
    ok &= expect_int("d0r2.frame.x2", d0r2 ? d0r2->wall_frame_x2 : -1, 223,
                     "ReDMCSB DUNVIEW.C:594 G0163 D0R x2");
    ok &= expect_int("d0l2.frame.y1", d0l2 ? d0l2->wall_frame_y1 : -1, 0,
                     "ReDMCSB DUNVIEW.C:593 G0163 D0L y1");
    ok &= expect_int("d0r2.frame.y2", d0r2 ? d0r2->wall_frame_y2 : -1, 135,
                     "ReDMCSB DUNVIEW.C:594 G0163 D0R y2");
    ok &= expect_int("d0l2.byte_width", d0l2 ? d0l2->wall_frame_byte_width : -1, 16,
                     "ReDMCSB DUNVIEW.C:593 G0163 D0L byte width");
    ok &= expect_int("d0r2.height", d0r2 ? d0r2->wall_frame_height : -1, 136,
                     "ReDMCSB DUNVIEW.C:594 G0163 D0R height");
    ok &= expect_int("map.left.x0",
                     csb_v1_viewport_d0l2_d0r2_wall_map_viewport_x_to_source_pc34(
                         d0l2, 0, 0, &source_x),
                     0,
                     "ReDMCSB DUNVIEW.C:8033 F0104 native left wall");
    ok &= expect_int("map.left.x0.source", source_x, 0,
                     "native D0L source span starts at 0");
    ok &= expect_int("map.left.x31",
                     csb_v1_viewport_d0l2_d0r2_wall_map_viewport_x_to_source_pc34(
                         d0l2, 31, 0, &source_x),
                     0,
                     "ReDMCSB DUNVIEW.C:8033 F0104 native left wall");
    ok &= expect_int("map.left.x31.source", source_x, 31,
                     "native D0L source span ends at 31");
    ok &= expect_int("map.right.flipped.x192",
                     csb_v1_viewport_d0l2_d0r2_wall_map_viewport_x_to_source_pc34(
                         d0r2, 192, 1, &source_x),
                     0,
                     "ReDMCSB DUNVIEW.C:8127 F0105 flipped right wall");
    ok &= expect_int("map.right.flipped.x192.source", source_x, 31,
                     "F0105 horizontal flip maps right edge to source 31");
    ok &= expect_int("map.right.flipped.x223",
                     csb_v1_viewport_d0l2_d0r2_wall_map_viewport_x_to_source_pc34(
                         d0r2, 223, 1, &source_x),
                     0,
                     "ReDMCSB DUNVIEW.C:8127 F0105 flipped right wall");
    ok &= expect_int("map.right.flipped.x223.source", source_x, 0,
                     "F0105 horizontal flip maps left source to viewport right edge");
    ok &= expect_int("map.outside",
                     csb_v1_viewport_d0l2_d0r2_wall_map_viewport_x_to_source_pc34(
                         d0l2, 32, 0, &source_x),
                     1,
                     "D0L wall must not write outside C716 clipped zone");
    ok &= expect_int("blend.transparent",
                     csb_v1_viewport_d0l2_d0r2_wall_blend_pixel_pc34(0xee, 10, 10),
                     0xee,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH preserve");
    ok &= expect_int("blend.opaque",
                     csb_v1_viewport_d0l2_d0r2_wall_blend_pixel_pc34(0xee, 0x44, 10),
                     0x44,
                     "ReDMCSB DUNVIEW.C:3113-3156/3185-3247 C10 blit");
    ok &= expect_int("apply.left.opaque",
                     csb_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
                         d0l2, source, sizeof(source), viewport, sizeof(viewport),
                         0, 12, 0),
                     0,
                     "ReDMCSB DUNVIEW.C:8033 F0104 C716 native blit");
    ok &= expect_int("pixel.left.opaque", viewport[viewport_offset(12, 0)], 0x41,
                     "synthetic non-C10 D0L wall pixel copied");
    ok &= expect_int("apply.left.transparent",
                     csb_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
                         d0l2, source, sizeof(source), viewport, sizeof(viewport),
                         1, 12, 0),
                     0,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("pixel.left.transparent", viewport[viewport_offset(12, 1)], 0xee,
                     "C10 transparent D0L wall pixel preserves destination");
    ok &= expect_int("apply.right.flipped",
                     csb_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
                         d0r2, source, sizeof(source), viewport, sizeof(viewport),
                         192, 12, 1),
                     0,
                     "ReDMCSB DUNVIEW.C:8127 F0105 C717 flipped blit");
    ok &= expect_int("pixel.right.flipped", viewport[viewport_offset(12, 192)], 0x43,
                     "F0105 flipped D0R wall pixel copied");
    ok &= expect_int("apply.right.outside_y",
                     csb_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
                         d0r2, source, sizeof(source), viewport, sizeof(viewport),
                         192, 136, 1),
                     1,
                     "D0R wall must not write outside viewport height");

    return ok;
}

static int test_f0115_and_thing_list_keepout(void)
{
    int ok = 1;
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *d0l2 =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(1);
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *d0r2 =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(2);
    const uint16_t first_thing = 0x4567u;
    uint16_t after_left;
    uint16_t after_right;

    after_left = csb_v1_viewport_d0l2_d0r2_wall_preserve_first_thing_pc34(
        d0l2, first_thing);
    after_right = csb_v1_viewport_d0l2_d0r2_wall_preserve_first_thing_pc34(
        d0r2, after_left);

    ok &= expect_int("d0l2.no_f0115_calls", d0l2 ? d0l2->f0115_call_count_for_wall : -1, 0,
                     "ReDMCSB DUNVIEW.C:8038 returns before F0115");
    ok &= expect_int("d0r2.no_f0115_calls", d0r2 ? d0r2->f0115_call_count_for_wall : -1, 0,
                     "ReDMCSB DUNVIEW.C:8144 returns before F0115");
    ok &= expect_int("d0l2.keepout", d0l2 ? d0l2->f0115_thing_pass_keepout : 0, 1,
                     "ReDMCSB DUNVIEW.C:4547-4581 F0115 thing pass");
    ok &= expect_int("d0r2.keepout", d0r2 ? d0r2->f0115_thing_pass_keepout : 0, 1,
                     "ReDMCSB DUNVIEW.C:4547-4581 F0115 thing pass");
    ok &= expect_int("d0l2.no_f0111", d0l2 ? d0l2->f0111_door_keepout : 0, 1,
                     "ReDMCSB DUNVIEW.C:8007-8038 wall return excludes door");
    ok &= expect_int("d0r2.no_f0111", d0r2 ? d0r2->f0111_door_keepout : 0, 1,
                     "ReDMCSB DUNVIEW.C:8117-8144 wall return excludes door");
    ok &= expect_int("d0l2.no_f0108", d0l2 ? d0l2->f0108_floor_ornament_keepout : 0, 1,
                     "ReDMCSB DUNVIEW.C:8007-8038 wall return excludes floor ornament");
    ok &= expect_int("d0r2.no_f0108", d0r2 ? d0r2->f0108_floor_ornament_keepout : 0, 1,
                     "ReDMCSB DUNVIEW.C:8117-8144 wall return excludes floor ornament");
    ok &= expect_int("d0l2.no_link", d0l2 ? d0l2->thing_list_link_mutation : -1, 0,
                     "ReDMCSB DUNGEON.C:1769-1838 F0163 not reached");
    ok &= expect_int("d0r2.no_link", d0r2 ? d0r2->thing_list_link_mutation : -1, 0,
                     "ReDMCSB DUNGEON.C:1769-1838 F0163 not reached");
    ok &= expect_int("d0l2.no_unlink", d0l2 ? d0l2->thing_list_unlink_mutation : -1, 0,
                     "ReDMCSB DUNGEON.C:1840-1905 F0164 not reached");
    ok &= expect_int("d0r2.no_unlink", d0r2 ? d0r2->thing_list_unlink_mutation : -1, 0,
                     "ReDMCSB DUNGEON.C:1840-1905 F0164 not reached");
    ok &= expect_int("d0l2.f0172_read_only",
                     d0l2 ? d0l2->f0172_square_aspect_read_only : 0, 1,
                     "ReDMCSB DUNGEON.C:2466-2523 F0172 aspect read");
    ok &= expect_int("d0r2.f0172_read_only",
                     d0r2 ? d0r2->f0172_square_aspect_read_only : 0, 1,
                     "ReDMCSB DUNGEON.C:2466-2523 F0172 aspect read");
    ok &= expect_u16("first_thing.after_left", after_left, first_thing,
                     "wall route keeps F0115 and F0163/F0164 out");
    ok &= expect_u16("first_thing.after_right", after_right, first_thing,
                     "wall route keeps F0115 and F0163/F0164 out");
    ok &= expect_u16("first_thing.null_spec",
                     csb_v1_viewport_d0l2_d0r2_wall_preserve_first_thing_pc34(
                         NULL, first_thing),
                     0xffffu,
                     "invalid contract fixture rejected");

    return ok;
}

static int test_row_followup_and_lineage(void)
{
    int ok = 1;
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *d0l2 =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(1);
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *d0r2 =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(2);

    ok &= expect_int("d0l2.followup", d0l2 ? d0l2->f0127_d0c_followup_after_pair : 0, 1,
                     "ReDMCSB DUNVIEW.C:8542 F0127 after D0L/D0R");
    ok &= expect_int("d0r2.followup", d0r2 ? d0r2->f0127_d0c_followup_after_pair : 0, 1,
                     "ReDMCSB DUNVIEW.C:8542 F0127 after D0L/D0R");
    ok &= expect_int("lineage.d0l2.relative", d0l2 ? d0l2->lineage_relative_cell : -1, 18,
                     "CSB-lineage Viewport.cpp:1192-1196 F0L1 open row");
    ok &= expect_int("lineage.d0r2.relative", d0r2 ? d0r2->lineage_relative_cell : -1, 19,
                     "CSB-lineage Viewport.cpp:1206-1210 F0R1 open row");
    ok &= expect_int("lineage.d0l2.contents",
                     d0l2 ? d0l2->lineage_contents_opcode : -1, 60128,
                     "CSB-lineage Viewport.cpp:1194 F0L1Contents");
    ok &= expect_int("lineage.d0r2.contents",
                     d0r2 ? d0r2->lineage_contents_opcode : -1, 60130,
                     "CSB-lineage Viewport.cpp:1209 F0R1Contents");
    ok &= expect_int("lineage.d0l2.order",
                     d0l2 ? d0l2->lineage_draw_order_opcode : -1, 60288,
                     "CSB-lineage Viewport.cpp:1194 DrawOrder02");
    ok &= expect_int("lineage.d0r2.order",
                     d0r2 ? d0r2->lineage_draw_order_opcode : -1, 60287,
                     "CSB-lineage Viewport.cpp:1209 DrawOrder01");
    ok &= expect_int("lineage.room_objects.left",
                     d0l2 ? d0l2->lineage_room_objects_opcode : -1, 60006,
                     "CSB-lineage Viewport.cpp:1194 StdDrawRoomObjects");
    ok &= expect_int("lineage.room_objects.right",
                     d0r2 ? d0r2->lineage_room_objects_opcode : -1, 60006,
                     "CSB-lineage Viewport.cpp:1209 StdDrawRoomObjects");
    ok &= expect_int("lineage.door.first",
                     d0l2 ? d0l2->lineage_door_facing_first_order_opcode : -1, 60279,
                     "CSB-lineage Viewport.cpp:1903-1915 DrawOrder218");
    ok &= expect_int("lineage.door.second",
                     d0r2 ? d0r2->lineage_door_facing_second_order_opcode : -1, 60280,
                     "CSB-lineage Viewport.cpp:1903-1915 DrawOrder349");

    return ok;
}

static int test_source_evidence_mentions_required_anchors(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d0l2_d0r2_wall_source_evidence_pc34();

    ok &= expect_contains("evidence.f0125", e, "DUNVIEW.C:7960-8062 F0125",
                          "ReDMCSB DUNVIEW.C F0125 7960-8062");
    ok &= expect_contains("evidence.f0126", e, "DUNVIEW.C:8064-8162 F0126",
                          "ReDMCSB DUNVIEW.C F0126 8064-8162");
    ok &= expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3156 F0104",
                          "ReDMCSB DUNVIEW.C F0104 3113-3156");
    ok &= expect_contains("evidence.f0105", e, "3185-3247 F0105",
                          "ReDMCSB DUNVIEW.C F0105 3185-3247");
    ok &= expect_contains("evidence.f0115", e, "DUNVIEW.C:4547-4581 F0115",
                          "ReDMCSB DUNVIEW.C F0115 4547-4581");
    ok &= expect_contains("evidence.f0116", e, "DUNVIEW.C:6361-6480 F0116",
                          "ReDMCSB DUNVIEW.C F0116 6361-6480 anchor only");
    ok &= expect_contains("evidence.f0128.pre", e, "DUNVIEW.C:8478-8508",
                          "ReDMCSB DUNVIEW.C F0128 8478-8508");
    ok &= expect_contains("evidence.f0128.d0", e, "DUNVIEW.C:8534-8542",
                          "ReDMCSB DUNVIEW.C F0128 8534-8542");
    ok &= expect_contains("evidence.f0127", e, "DUNVIEW.C:8294 F0127",
                          "ReDMCSB DUNVIEW.C F0127 8294 anchor only");
    ok &= expect_contains("evidence.f0163", e, "DUNGEON.C:1769-1838 F0163",
                          "ReDMCSB DUNGEON.C F0163 1769-1838");
    ok &= expect_contains("evidence.f0164", e, "DUNGEON.C:1840-1905 F0164",
                          "ReDMCSB DUNGEON.C F0164 1840-1905");
    ok &= expect_contains("evidence.f0172", e, "DUNGEON.C:2466-2523 F0172",
                          "ReDMCSB DUNGEON.C F0172 2466-2523");
    ok &= expect_contains("evidence.defs.c10", e, "DEFS.H:2088",
                          "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_contains("evidence.defs.zones", e, "DEFS.H:4040-4057",
                          "ReDMCSB DEFS.H:4040-4057 D*-L/D*-R zones");
    ok &= expect_contains("evidence.lineage.open", e, "Viewport.cpp:1192-1209",
                          "CSB-lineage Viewport.cpp:1192-1209");
    ok &= expect_contains("evidence.lineage.door", e, "Viewport.cpp:1903-1915",
                          "CSB-lineage Viewport.cpp:1903-1915");
    ok &= expect_contains("evidence.no_real_asset", e, "no real-asset",
                          "contract-only scope");
    ok &= expect_contains("evidence.no_game_data", e, "no CSB game-data load",
                          "contract-only scope");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d0l2_d0r2_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d0l2_d0r2_wall_source_evidence_pc34());

    ok &= test_run_entry_point();
    ok &= test_route_identity_and_dispatch();
    ok &= test_wall_variant_bindings();
    ok &= test_frames_pixels_and_c10();
    ok &= test_f0115_and_thing_list_keepout();
    ok &= test_row_followup_and_lineage();
    ok &= test_source_evidence_mentions_required_anchors();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_100", g_assertions >= 100, 1,
                     "assigned CSB D0L2/D0R2 wall source-lock gate");

    if (ok) {
        printf("PASS csb_v1_viewport_d0l2_d0r2_wall_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
