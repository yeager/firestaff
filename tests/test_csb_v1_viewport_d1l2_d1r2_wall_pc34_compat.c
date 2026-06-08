#include "csb/csb_v1_viewport_d1l2_d1r2_wall_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    VIEWPORT_WIDTH = 224,
    VIEWPORT_HEIGHT = 136,
    SOURCE_WIDTH = 256,
    SOURCE_HEIGHT = 111,
    TRANSPARENT = 10
};

static int g_assertions = 0;
static int g_failures = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        ++g_failures;
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_u16(const char *label, uint16_t got, uint16_t want,
                      const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%04x want=0x%04x anchor=%s\n",
               label, (unsigned int)got, (unsigned int)want, anchor);
        ++g_failures;
        return 0;
    }
    printf("PASS %s=0x%04x anchor=%s\n", label, (unsigned int)got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    const int got = haystack && needle && strstr(haystack, needle) != 0;
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
    CSB_V1_D1L2D1R2WallRunResultPc34 result;

    ok &= expect_int("run.return",
                     csb_v1_viewport_d1l2_d1r2_wall_pc34_compat_run(&result),
                     0,
                     "ReDMCSB DUNVIEW.C:8524-8529 F0128 D1 side pair");
    ok &= expect_int("run.ok", result.ok, 1,
                     "contract-only D1L2/D1R2 wall gate");
    ok &= expect_int("run.route_count", result.route_count, 2,
                     "ReDMCSB DUNVIEW.C:7391-7725 F0122/F0123");
    ok &= expect_int("run.dispatch_order_ok", result.dispatch_order_ok, 1,
                     "ReDMCSB DUNVIEW.C:8524-8529 F0128");
    ok &= expect_int("run.wall_variant_binding_ok", result.wall_variant_binding_ok, 1,
                     "ReDMCSB DUNVIEW.C:7446/7454 and 7614/7622");
    ok &= expect_int("run.c10_transparency_ok", result.c10_transparency_ok, 1,
                     "ReDMCSB DUNVIEW.C:3113-3156/3185-3218");
    ok &= expect_int("run.clipped_edge_write_ok", result.clipped_edge_write_ok, 1,
                     "ReDMCSB DUNVIEW.C:590-591 G0163 D1 side clips");
    ok &= expect_int("run.f0128_followup_ok", result.f0128_followup_ok, 1,
                     "ReDMCSB DUNVIEW.C:8530-8533 D1C follow-up");
    ok &= expect_int("run.f0115_keepout_ok", result.f0115_keepout_ok, 1,
                     "ReDMCSB DUNVIEW.C:4547-4581 F0115 keep-out");
    ok &= expect_int("run.dungeon_metadata_binding_ok",
                     result.dungeon_metadata_binding_ok, 1,
                     "ReDMCSB DUNGEON.C:1769-1905/2466-2523");
    ok &= expect_int("run.d1l2_copied_pixels", result.d1l2_copied_pixels, 3,
                     "synthetic D1L2 native C10 wall blit");
    ok &= expect_int("run.d1r2_copied_pixels", result.d1r2_copied_pixels, 3,
                     "synthetic D1R2 native C10 wall blit");
    ok &= expect_u16("run.first_thing_preserved",
                     result.first_thing_after, result.first_thing_before,
                     "ReDMCSB DUNGEON.C:1769-1905 not reached by wall");

    return ok;
}

static int test_route_identity_and_order(void)
{
    int ok = 1;
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *d1l2 =
        csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(
            CSB_V1_D1L2_D1R2_WALL_SIDE_D1L2_PC34);
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *d1r2 =
        csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(
            CSB_V1_D1L2_D1R2_WALL_SIDE_D1R2_PC34);

    ok &= expect_int("route.count",
                     (int)csb_v1_viewport_d1l2_d1r2_wall_route_spec_count_pc34(),
                     2, "ReDMCSB DUNVIEW.C:8524-8529");
    ok &= expect_int("route.index0.d1l2",
                     csb_v1_viewport_d1l2_d1r2_wall_route_spec_at_pc34(0) == d1l2,
                     1, "ReDMCSB DUNVIEW.C:8524-8525 F0128 D1L first");
    ok &= expect_int("route.index1.d1r2",
                     csb_v1_viewport_d1l2_d1r2_wall_route_spec_at_pc34(1) == d1r2,
                     1, "ReDMCSB DUNVIEW.C:8528-8529 F0128 D1R second");
    ok &= expect_int("route.index2.null",
                     csb_v1_viewport_d1l2_d1r2_wall_route_spec_at_pc34(2) == 0,
                     1, "D1L2/D1R2 wall-only route table");
    ok &= expect_int("route.unknown.null",
                     csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(99) == 0,
                     1, "D1L2/D1R2 wall-only side ids");
    ok &= expect_int("d1l2.source_locked",
                     d1l2 ? d1l2->source_locked_contract_only : 0, 1,
                     "contract-only source lock");
    ok &= expect_int("d1r2.source_locked",
                     d1r2 ? d1r2->source_locked_contract_only : 0, 1,
                     "contract-only source lock");
    ok &= expect_int("d1l2.no_real_asset",
                     d1l2 ? d1l2->no_real_asset_bitmap_parity : 0, 1,
                     "no real-asset bitmap parity");
    ok &= expect_int("d1r2.no_game_data",
                     d1r2 ? d1r2->no_game_data_load : 0, 1,
                     "no CSB game-data load");
    ok &= expect_int("d1l2.function", d1l2 ? d1l2->redmcsb_function_number : 0,
                     122, "ReDMCSB DUNVIEW.C:7391 F0122");
    ok &= expect_int("d1r2.function", d1r2 ? d1r2->redmcsb_function_number : 0,
                     123, "ReDMCSB DUNVIEW.C:7559 F0123");
    ok &= expect_int("d1l2.order", d1l2 ? d1l2->f0128_draw_order_index : -1,
                     0, "ReDMCSB DUNVIEW.C:8524-8525");
    ok &= expect_int("d1r2.order", d1r2 ? d1r2->f0128_draw_order_index : -1,
                     1, "ReDMCSB DUNVIEW.C:8528-8529");
    ok &= expect_int("d1l2.depth", d1l2 ? d1l2->f0128_relative_depth : -1,
                     1, "ReDMCSB DUNVIEW.C:8524");
    ok &= expect_int("d1r2.depth", d1r2 ? d1r2->f0128_relative_depth : -1,
                     1, "ReDMCSB DUNVIEW.C:8528");
    ok &= expect_int("d1l2.lateral", d1l2 ? d1l2->f0128_relative_lateral : 0,
                     -1, "ReDMCSB DUNVIEW.C:8524");
    ok &= expect_int("d1r2.lateral", d1r2 ? d1r2->f0128_relative_lateral : 0,
                     1, "ReDMCSB DUNVIEW.C:8528");
    ok &= expect_int("d1l2.view_square", d1l2 ? d1l2->view_square_index : -1,
                     4, "ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L");
    ok &= expect_int("d1r2.view_square", d1r2 ? d1r2->view_square_index : -1,
                     5, "ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R");

    return ok;
}

static int test_wall_routes_zone_math_and_frames(void)
{
    int ok = 1;
    int source_x = -1;
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *d1l2 =
        csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(1);
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *d1r2 =
        csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(2);

    ok &= expect_int("d1l2.wall_element", d1l2 ? d1l2->wall_element : -1, 0,
                     "ReDMCSB DUNVIEW.C:7436 C00_ELEMENT_WALL");
    ok &= expect_int("d1r2.wall_element", d1r2 ? d1r2->wall_element : -1, 0,
                     "ReDMCSB DUNVIEW.C:7604 C00_ELEMENT_WALL");
    ok &= expect_int("d1l2.teleporter_element",
                     d1l2 ? d1l2->teleporter_element : -1, 5,
                     "ReDMCSB DUNVIEW.C:7538-7555");
    ok &= expect_int("d1r2.teleporter_element",
                     d1r2 ? d1r2->teleporter_element : -1, 5,
                     "ReDMCSB DUNVIEW.C:7706-7723");
    ok &= expect_int("d1l2.wall_zone", d1l2 ? d1l2->wall_zone : -1, 713,
                     "ReDMCSB DEFS.H:4053 C713_ZONE_WALL_D1L");
    ok &= expect_int("d1r2.wall_zone", d1r2 ? d1r2->wall_zone : -1, 714,
                     "ReDMCSB DEFS.H:4054 C714_ZONE_WALL_D1R");
    ok &= expect_int("d1l2.neighbor_d1c_zone",
                     d1l2 ? d1l2->neighboring_d1c_zone : -1, 712,
                     "ReDMCSB DEFS.H:4052 C712_ZONE_WALL_D1C");
    ok &= expect_int("zone.d1l2.math",
                     csb_v1_viewport_d1l2_d1r2_wall_expected_zone_for_view_square_pc34(4),
                     713, "ReDMCSB DEFS.H:2599-2601/4052-4054");
    ok &= expect_int("zone.d1r2.math",
                     csb_v1_viewport_d1l2_d1r2_wall_expected_zone_for_view_square_pc34(5),
                     714, "ReDMCSB DEFS.H:2599-2601/4052-4054");
    ok &= expect_int("zone.d1c.rejected",
                     csb_v1_viewport_d1l2_d1r2_wall_expected_zone_for_view_square_pc34(3),
                     -1, "D1L2/D1R2-only wall zone helper");
    ok &= expect_int("d1l2.native_wall", d1l2 ? d1l2->native_wall_index : -1,
                     3, "ReDMCSB DUNVIEW.C:7454 C03_WALL_D1L");
    ok &= expect_int("d1l2.flipped_wall", d1l2 ? d1l2->flipped_wall_index : -1,
                     2, "ReDMCSB DUNVIEW.C:7446 C02_WALL_D1R");
    ok &= expect_int("d1r2.native_wall", d1r2 ? d1r2->native_wall_index : -1,
                     2, "ReDMCSB DUNVIEW.C:7622 C02_WALL_D1R");
    ok &= expect_int("d1r2.flipped_wall", d1r2 ? d1r2->flipped_wall_index : -1,
                     3, "ReDMCSB DUNVIEW.C:7614 C03_WALL_D1L");
    ok &= expect_int("d1l2.f0100", d1l2 ? d1l2->f0100_st_wall_route : -1,
                     1, "ReDMCSB DUNVIEW.C:7438 ST wall route");
    ok &= expect_int("d1r2.f0100", d1r2 ? d1r2->f0100_st_wall_route : -1,
                     1, "ReDMCSB DUNVIEW.C:7606 ST wall route");
    ok &= expect_int("d1l2.f0104", d1l2 ? d1l2->f0104_native_route : -1,
                     1, "ReDMCSB DUNVIEW.C:7454 F0104");
    ok &= expect_int("d1r2.f0104", d1r2 ? d1r2->f0104_native_route : -1,
                     1, "ReDMCSB DUNVIEW.C:7622 F0104");
    ok &= expect_int("d1l2.f0105", d1l2 ? d1l2->f0105_flipped_route : -1,
                     1, "ReDMCSB DUNVIEW.C:7446 F0105");
    ok &= expect_int("d1r2.f0105", d1r2 ? d1r2->f0105_flipped_route : -1,
                     1, "ReDMCSB DUNVIEW.C:7614 F0105");
    ok &= expect_int("d1l2.frame_row", d1l2 ? d1l2->wall_frame_row : -1,
                     7, "ReDMCSB DUNVIEW.C:590 G0163 D1L row");
    ok &= expect_int("d1r2.frame_row", d1r2 ? d1r2->wall_frame_row : -1,
                     8, "ReDMCSB DUNVIEW.C:591 G0163 D1R row");
    ok &= expect_int("d1l2.frame.x1", d1l2 ? d1l2->wall_frame_x1 : -1,
                     0, "ReDMCSB DUNVIEW.C:590");
    ok &= expect_int("d1l2.frame.x2", d1l2 ? d1l2->wall_frame_x2 : -1,
                     63, "ReDMCSB DUNVIEW.C:590");
    ok &= expect_int("d1r2.frame.x1", d1r2 ? d1r2->wall_frame_x1 : -1,
                     160, "ReDMCSB DUNVIEW.C:591");
    ok &= expect_int("d1r2.frame.x2", d1r2 ? d1r2->wall_frame_x2 : -1,
                     223, "ReDMCSB DUNVIEW.C:591");
    ok &= expect_int("d1l2.frame.y1", d1l2 ? d1l2->wall_frame_y1 : -1,
                     9, "ReDMCSB DUNVIEW.C:590");
    ok &= expect_int("d1r2.frame.y2", d1r2 ? d1r2->wall_frame_y2 : -1,
                     119, "ReDMCSB DUNVIEW.C:591");
    ok &= expect_int("d1l2.byte_width", d1l2 ? d1l2->wall_frame_byte_width : -1,
                     128, "ReDMCSB DUNVIEW.C:590 byte width");
    ok &= expect_int("d1r2.height", d1r2 ? d1r2->wall_frame_height : -1,
                     111, "ReDMCSB DUNVIEW.C:591 height");
    ok &= expect_int("map.left.native.x0",
                     csb_v1_viewport_d1l2_d1r2_wall_map_viewport_x_to_source_pc34(
                         d1l2, 0, 0, &source_x),
                     0, "ReDMCSB DUNVIEW.C:7454 F0104 native D1L");
    ok &= expect_int("map.left.native.x0.source", source_x, 192,
                     "ReDMCSB DUNVIEW.C:590 source X");
    ok &= expect_int("map.right.flipped.x160",
                     csb_v1_viewport_d1l2_d1r2_wall_map_viewport_x_to_source_pc34(
                         d1r2, 160, 1, &source_x),
                     0, "ReDMCSB DUNVIEW.C:7614 F0105 flipped D1R");
    ok &= expect_int("map.right.flipped.x160.source", source_x, 63,
                     "F0105 horizontal flip maps viewport left to source right");
    ok &= expect_int("map.right.outside",
                     csb_v1_viewport_d1l2_d1r2_wall_map_viewport_x_to_source_pc34(
                         d1r2, 159, 0, &source_x),
                     1, "D1R wall must not write outside C714 clip");

    return ok;
}

static int test_c10_native_parity_and_clipped_edges(void)
{
    int ok = 1;
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *d1l2 =
        csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(1);
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *d1r2 =
        csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(2);
    uint8_t source[SOURCE_WIDTH * SOURCE_HEIGHT];
    uint8_t viewport[VIEWPORT_WIDTH * VIEWPORT_HEIGHT];
    CSB_V1_D1L2D1R2WallBlitStatsPc34 stats;

    memset(source, TRANSPARENT, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[source_offset(0, 192)] = TRANSPARENT;
    source[source_offset(0, 193)] = 0x31u;
    source[source_offset(0, 255)] = 0x7au;
    source[source_offset(110, 192)] = 0x55u;

    ok &= expect_int("transparent.color", d1l2 ? d1l2->transparent_color : -1,
                     10, "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("d1l2.native.copied",
                     csb_v1_viewport_d1l2_d1r2_wall_apply_frame_clip_pc34(
                         d1l2, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     3, "ReDMCSB DUNVIEW.C:3113-3156 F0104 C10 blit");
    ok &= expect_int("d1l2.native.transparent",
                     stats.transparent_pixels, (64 * 111) - 3,
                     "ReDMCSB DEFS.H:2088 C10 transparent");
    ok &= expect_int("d1l2.native.left_edge_transparent_no_write",
                     viewport[viewport_offset(9, 0)], 0xee,
                     "ReDMCSB DUNVIEW.C:590 x1 C10 skip");
    ok &= expect_int("d1l2.native.next_pixel",
                     viewport[viewport_offset(9, 1)], 0x31,
                     "synthetic non-C10 D1L source[193]");
    ok &= expect_int("d1l2.native.right_edge",
                     viewport[viewport_offset(9, 63)], 0x7a,
                     "ReDMCSB DUNVIEW.C:590 x2 clipped edge");
    ok &= expect_int("d1l2.native.right_neighbor_no_write",
                     viewport[viewport_offset(9, 64)], 0xee,
                     "neighbor outside D1L clip");
    ok &= expect_int("d1l2.native.top_neighbor_no_write",
                     viewport[viewport_offset(8, 1)], 0xee,
                     "neighbor above D1 side clip");
    ok &= expect_int("d1l2.native.bottom_edge",
                     viewport[viewport_offset(119, 0)], 0x55,
                     "ReDMCSB DUNVIEW.C:590 y2 clipped edge");
    ok &= expect_int("d1l2.native.bottom_neighbor_no_write",
                     viewport[viewport_offset(120, 0)], 0xee,
                     "neighbor below D1 side clip");

    memset(viewport, 0xee, sizeof(viewport));
    ok &= expect_int("d1l2.flip.copied",
                     csb_v1_viewport_d1l2_d1r2_wall_apply_frame_clip_pc34(
                         d1l2, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 1, &stats),
                     3, "ReDMCSB DUNVIEW.C:3185-3218 F0105 C10 flip");
    ok &= expect_int("d1l2.flip.left_edge",
                     viewport[viewport_offset(9, 0)], 0x7a,
                     "flipped source[255]");
    ok &= expect_int("d1l2.flip.right_neighbor_no_write",
                     viewport[viewport_offset(9, 64)], 0xee,
                     "F0105 keeps D1L clip bounds");

    memset(source, TRANSPARENT, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[source_offset(0, 0)] = TRANSPARENT;
    source[source_offset(0, 1)] = 0x61u;
    source[source_offset(0, 63)] = 0x62u;
    source[source_offset(110, 62)] = 0x63u;

    ok &= expect_int("d1r2.native.copied",
                     csb_v1_viewport_d1l2_d1r2_wall_apply_frame_clip_pc34(
                         d1r2, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     3, "ReDMCSB DUNVIEW.C:7622 F0104 C714 wall");
    ok &= expect_int("d1r2.native.left_neighbor_no_write",
                     viewport[viewport_offset(9, 159)], 0xee,
                     "neighbor outside D1R clip");
    ok &= expect_int("d1r2.native.left_edge_transparent_no_write",
                     viewport[viewport_offset(9, 160)], 0xee,
                     "ReDMCSB DEFS.H:2088 C10 skip");
    ok &= expect_int("d1r2.native.next_pixel",
                     viewport[viewport_offset(9, 161)], 0x61,
                     "synthetic non-C10 D1R source[1]");
    ok &= expect_int("d1r2.native.right_edge",
                     viewport[viewport_offset(9, 223)], 0x62,
                     "ReDMCSB DUNVIEW.C:591 x2 clipped edge");

    memset(viewport, 0xee, sizeof(viewport));
    ok &= expect_int("d1r2.flip.copied",
                     csb_v1_viewport_d1l2_d1r2_wall_apply_frame_clip_pc34(
                         d1r2, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 1, &stats),
                     3, "ReDMCSB DUNVIEW.C:7614 F0105 C714 wall");
    ok &= expect_int("d1r2.flip.left_edge",
                     viewport[viewport_offset(9, 160)], 0x62,
                     "flipped source[63]");
    ok &= expect_int("d1r2.flip.right_edge_transparent_no_write",
                     viewport[viewport_offset(9, 223)], 0xee,
                     "flipped source[0] C10 skip");

    memset(viewport, 0xee, sizeof(viewport));
    ok &= expect_int("d1l2.small_viewport.copied",
                     csb_v1_viewport_d1l2_d1r2_wall_apply_frame_clip_pc34(
                         d1l2, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         4, 20, 0, &stats),
                     0, "small viewport clips all D1L non-C10 sentinels here");
    ok &= expect_int("d1l2.small_viewport.clipped",
                     stats.clipped_pixels, (64 * 111) - (4 * 11),
                     "deterministic no-write metadata for clipped pixels");
    ok &= expect_int("d1r2.reject_short_source",
                     csb_v1_viewport_d1l2_d1r2_wall_apply_frame_clip_pc34(
                         d1r2, source, 63, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     -1, "helper rejects unresolved source clip");
    ok &= expect_int("d1r2.reject_short_source_flag", stats.rejected, 1,
                     "rejected source leaves no write contract");
    ok &= expect_int("reject.null_spec",
                     csb_v1_viewport_d1l2_d1r2_wall_apply_frame_clip_pc34(
                         0, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     -1, "helper rejects missing source-locked route");

    return ok;
}

static int test_keepouts_followup_metadata_and_evidence(void)
{
    int ok = 1;
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *d1l2 =
        csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(1);
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *d1r2 =
        csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(2);
    const char *e = csb_v1_viewport_d1l2_d1r2_wall_source_evidence_pc34();

    ok &= expect_int("d1l2.f0107", d1l2 ? d1l2->f0107_wall_ornament_route : -1,
                     1, "ReDMCSB DUNVIEW.C:7459 F0107 before return");
    ok &= expect_int("d1r2.f0107", d1r2 ? d1r2->f0107_wall_ornament_route : -1,
                     1, "ReDMCSB DUNVIEW.C:7627 F0107 before return");
    ok &= expect_int("d1l2.no_f0111", d1l2 ? d1l2->f0111_door_route : -1,
                     0, "ReDMCSB DUNVIEW.C:7436-7460 wall returns before doors");
    ok &= expect_int("d1r2.no_f0111", d1r2 ? d1r2->f0111_door_route : -1,
                     0, "ReDMCSB DUNVIEW.C:7604-7628 wall returns before doors");
    ok &= expect_int("d1l2.no_f0115", d1l2 ? d1l2->f0115_thing_pass_route : -1,
                     0, "ReDMCSB DUNVIEW.C:7436-7460 wall returns before F0115");
    ok &= expect_int("d1r2.no_f0115", d1r2 ? d1r2->f0115_thing_pass_route : -1,
                     0, "ReDMCSB DUNVIEW.C:7604-7628 wall returns before F0115");
    ok &= expect_int("d1l2.f0115_keepout",
                     d1l2 ? d1l2->f0115_thing_pass_keepout : -1, 1,
                     "ReDMCSB DUNVIEW.C:4547-4581/5668-5671");
    ok &= expect_int("d1r2.f0115_keepout",
                     d1r2 ? d1r2->f0115_thing_pass_keepout : -1, 1,
                     "ReDMCSB DUNVIEW.C:4547-4581/5668-5671");
    ok &= expect_int("d1l2.f0113", d1l2 ? d1l2->f0113_teleporter_route : -1,
                     1, "ReDMCSB DUNVIEW.C:7538-7555 teleporter field");
    ok &= expect_int("d1r2.f0113", d1r2 ? d1r2->f0113_teleporter_route : -1,
                     1, "ReDMCSB DUNVIEW.C:7706-7723 teleporter field");
    ok &= expect_int("d1l2.f0128.reset",
                     d1l2 ? d1l2->f0128_post_d1_followup_resets_map_coordinates : -1,
                     1, "ReDMCSB DUNVIEW.C:8526-8527");
    ok &= expect_int("d1r2.f0128.reset",
                     d1r2 ? d1r2->f0128_post_d1_followup_resets_map_coordinates : -1,
                     1, "ReDMCSB DUNVIEW.C:8530-8531");
    ok &= expect_int("d1l2.f0128.draws_d1c",
                     d1l2 ? d1l2->f0128_post_d1_followup_draws_d1c : -1,
                     1, "ReDMCSB DUNVIEW.C:8532-8533");
    ok &= expect_int("d1r2.f0128.draws_d1c",
                     d1r2 ? d1r2->f0128_post_d1_followup_draws_d1c : -1,
                     1, "ReDMCSB DUNVIEW.C:8532-8533");
    ok &= expect_int("d1l2.f0127.boundary",
                     d1l2 ? d1l2->f0127_wall_return_boundary : -1, 1,
                     "ReDMCSB DUNVIEW.C:8294 F0127 F0115 boundary");
    ok &= expect_int("d1l2.f0172.read",
                     d1l2 ? d1l2->f0172_square_aspect_metadata_read : -1, 1,
                     "ReDMCSB DUNGEON.C:2466-2523 F0172");
    ok &= expect_int("d1r2.f0172.read",
                     d1r2 ? d1r2->f0172_square_aspect_metadata_read : -1, 1,
                     "ReDMCSB DUNGEON.C:2466-2523 F0172");
    ok &= expect_int("d1l2.no_f0163",
                     d1l2 ? d1l2->f0163_link_thing_keepout : -1, 0,
                     "ReDMCSB DUNGEON.C:1769-1840 not reached");
    ok &= expect_int("d1r2.no_f0164",
                     d1r2 ? d1r2->f0164_unlink_thing_keepout : -1, 0,
                     "ReDMCSB DUNGEON.C:1840-1905 not reached");
    ok &= expect_u16("preserve.d1r2.first_thing",
                     csb_v1_viewport_d1l2_d1r2_wall_preserve_first_thing_pc34(
                         d1r2, 0x4567u),
                     0x4567u, "wall path does not mutate first thing metadata");
    ok &= expect_u16("preserve.null",
                     csb_v1_viewport_d1l2_d1r2_wall_preserve_first_thing_pc34(
                         0, 0x4567u),
                     0xffffu, "missing spec rejected");
    ok &= expect_contains("symbol.d1l2.bitmap",
                          d1l2 ? d1l2->bitmap_symbol : 0,
                          "G2107_WallSet[C03_WALL_D1L]",
                          "ReDMCSB DUNVIEW.C:7454");
    ok &= expect_contains("symbol.d1r2.bitmap",
                          d1r2 ? d1r2->bitmap_symbol : 0,
                          "G2107_WallSet[C02_WALL_D1R]",
                          "ReDMCSB DUNVIEW.C:7622");
    ok &= expect_contains("symbol.d1l2.frame",
                          d1l2 ? d1l2->frame_symbol : 0,
                          "{0,63,9,119,128,111,192,0}",
                          "ReDMCSB DUNVIEW.C:590");
    ok &= expect_contains("symbol.d1r2.frame",
                          d1r2 ? d1r2->frame_symbol : 0,
                          "{160,223,9,119,128,111,0,0}",
                          "ReDMCSB DUNVIEW.C:591");
    ok &= expect_contains("evidence.f0122", e, "F0122_DUNGEONVIEW_DrawSquareD1L",
                          "ReDMCSB DUNVIEW.C:7391");
    ok &= expect_contains("evidence.f0123", e, "F0123_DUNGEONVIEW_DrawSquareD1R",
                          "ReDMCSB DUNVIEW.C:7559");
    ok &= expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3156",
                          "ReDMCSB DUNVIEW.C:3113-3156");
    ok &= expect_contains("evidence.f0105", e, "3185-3218",
                          "ReDMCSB DUNVIEW.C:3185-3218");
    ok &= expect_contains("evidence.f0115", e, "4547-4581",
                          "ReDMCSB DUNVIEW.C:4547-4581");
    ok &= expect_contains("evidence.f0128", e, "DUNVIEW.C:8524-8529",
                          "ReDMCSB DUNVIEW.C:8524-8529");
    ok &= expect_contains("evidence.f0127", e, "DUNVIEW.C:8294",
                          "ReDMCSB DUNVIEW.C:8294");
    ok &= expect_contains("evidence.f0163", e, "DUNGEON.C:1769-1840",
                          "ReDMCSB DUNGEON.C:1769-1840");
    ok &= expect_contains("evidence.f0164", e, "1840-1905",
                          "ReDMCSB DUNGEON.C:1840-1905");
    ok &= expect_contains("evidence.f0172", e, "2466-2523",
                          "ReDMCSB DUNGEON.C:2466-2523");
    ok &= expect_contains("evidence.c10", e, "C10_COLOR_FLESH",
                          "ReDMCSB DEFS.H:2088");
    ok &= expect_contains("evidence.zones", e, "C712/C713/C714",
                          "ReDMCSB DEFS.H:4052-4054");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d1l2_d1r2_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d1l2_d1r2_wall_source_evidence_pc34());

    ok &= test_run_entry_point();
    ok &= test_route_identity_and_order();
    ok &= test_wall_routes_zone_math_and_frames();
    ok &= test_c10_native_parity_and_clipped_edges();
    ok &= test_keepouts_followup_metadata_and_evidence();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_90", g_assertions >= 90, 1,
                     "assigned D1L2/D1R2 wall source-lock gate");

    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_d1l2_d1r2_wall_pc34_compat assertions=%d\n",
               g_assertions);
        return 0;
    }
    printf("FAIL csb_v1_viewport_d1l2_d1r2_wall_pc34_compat assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return 1;
}
