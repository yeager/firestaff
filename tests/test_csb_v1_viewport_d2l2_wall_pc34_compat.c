#include "csb_v1_viewport_d2l2_wall_pc34_compat.h"

#include <stdint.h>
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

static int apply_d2l2_c10_frame_clip(
    const CSB_V1_ViewportD2L2WallRouteSpec *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int flip_horizontal)
{
    int copied = 0;

    if (!spec || !source || !destination) return -1;
    if (source_stride <= 0 || destination_width <= 0 || destination_height <= 0) return -1;

    for (int y = 0; y < spec->height; ++y) {
        const int dst_y = spec->frame_y1 + y;
        if (dst_y < 0 || dst_y >= destination_height) continue;

        for (int x = 0; x < spec->byte_width; ++x) {
            const int raw_src_x = spec->source_x + x;
            const int src_x = flip_horizontal ? source_stride - 1 - raw_src_x : raw_src_x;
            const int src_y = spec->source_y + y;
            const int dst_x = spec->frame_x1 + x;
            uint8_t pixel;

            if (src_x < 0 || src_x >= source_stride) continue;
            if (dst_x < 0 || dst_x >= destination_width) continue;

            pixel = source[(src_y * source_stride) + src_x];
            if (pixel == (uint8_t)spec->transparent_color) continue;

            destination[(dst_y * destination_width) + dst_x] = pixel;
            ++copied;
        }
    }

    return copied;
}

static int test_route_spec(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2WallRouteSpec *spec =
        csb_v1_viewport_d2l2_wall_route_spec_pc34();

    ok &= expect_int("route.present", spec != NULL, 1,
                     "ReDMCSB DUNVIEW.C:6837 F0678_DrawD2L2");
    ok &= expect_int("route.contract_only",
                     spec ? spec->source_locked_contract_only : -1, 1,
                     "source-locked contract marker");
    ok &= expect_int("route.view_square",
                     spec ? spec->view_square : -1, 9,
                     "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_int("route.element_slot",
                     spec ? spec->square_aspect_element_slot : -1, 0,
                     "ReDMCSB DEFS.H:2534 C0_ELEMENT");
    ok &= expect_int("route.wall_element",
                     spec ? spec->wall_element : -1, 0,
                     "ReDMCSB DEFS.H:1007 C00_ELEMENT_WALL");
    ok &= expect_int("route.teleporter_element",
                     spec ? spec->teleporter_element : -1, 5,
                     "ReDMCSB DEFS.H:1012 C05_ELEMENT_TELEPORTER");
    ok &= expect_int("route.wall_zone",
                     spec ? spec->wall_zone : -1, 707,
                     "ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2");

    return ok;
}

static int test_wall_and_media709_routes(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2WallRouteSpec *spec =
        csb_v1_viewport_d2l2_wall_route_spec_pc34();

    ok &= expect_int("wall.f0104_route",
                     spec ? spec->f0104_wall_route : -1, 1,
                     "ReDMCSB DUNVIEW.C:6853-6858 F0104");
    ok &= expect_int("wall.native_base",
                     spec ? spec->native_wall_index_base : -1, 6,
                     "ReDMCSB DEFS.H:3429 C06_WALL_D2L2");
    ok &= expect_int("wall.pc_fix_delta",
                     spec ? spec->native_wall_index_pc_fix_delta : -1, 2,
                     "ReDMCSB DUNVIEW.C:6854-6856 PC_FIX_CODE_SIZE");
    ok &= expect_int("wall.pc34_effective",
                     spec ? spec->native_wall_index_pc34_effective : -1, 8,
                     "ReDMCSB DUNVIEW.C:6853-6858 C06_WALL_D2L2 + 2");
    ok &= expect_int("wall.pc34_base_plus_delta",
                     spec ? spec->native_wall_index_base +
                         spec->native_wall_index_pc_fix_delta : -1, 8,
                     "ReDMCSB DUNVIEW.C:6853-6858 C06_WALL_D2L2 + 2");
    ok &= expect_int("wall.zone_for_f0104",
                     spec ? spec->wall_zone : -1, 707,
                     "ReDMCSB DUNVIEW.C:6858 C707_ZONE_WALL_D2L2");
    ok &= expect_int("media709.f0105_route",
                     spec ? spec->f0105_media709_flipped_route : -1, 1,
                     "ReDMCSB DUNVIEW.C:6849-6851 F0105");
    ok &= expect_int("media709.flipped_wall",
                     spec ? spec->media709_flipped_wall_index : -1, 5,
                     "ReDMCSB DEFS.H:3428 C05_WALL_D2R2");
    ok &= expect_int("media709.zone_for_f0105",
                     spec ? spec->wall_zone : -1, 707,
                     "ReDMCSB DUNVIEW.C:6850 C707_ZONE_WALL_D2L2");

    return ok;
}

static int test_d2l2_frame_and_c10_blit(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2WallRouteSpec *spec =
        csb_v1_viewport_d2l2_wall_route_spec_pc34();
    uint8_t source[36 * 71];
    uint8_t destination[224 * 100];
    uint8_t small_destination[4 * 30];

    ok &= expect_int("frame.x1", spec ? spec->frame_x1 : -1, 0,
                     "ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference");
    ok &= expect_int("frame.x2", spec ? spec->frame_x2 : -1, 37,
                     "ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference");
    ok &= expect_int("frame.y1", spec ? spec->frame_y1 : -1, 20,
                     "ReDMCSB COORD.C:1498 C707 top y=20");
    ok &= expect_int("frame.y2", spec ? spec->frame_y2 : -1, 90,
                     "ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference");
    ok &= expect_int("frame.byte_width", spec ? spec->byte_width : -1, 36,
                     "ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference");
    ok &= expect_int("frame.height", spec ? spec->height : -1, 71,
                     "ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference");
    ok &= expect_int("frame.source_x", spec ? spec->source_x : -1, 30,
                     "ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference");
    ok &= expect_int("frame.source_y", spec ? spec->source_y : -1, 0,
                     "ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference");
    ok &= expect_int("transparent.color",
                     spec ? spec->transparent_color : -1, 10,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("transparent.preserved",
                     spec ? spec->preserves_c10_transparency : -1, 1,
                     "ReDMCSB DUNVIEW.C:3113-3129 F0104");

    memset(source, 10, sizeof(source));
    memset(destination, 0xee, sizeof(destination));
    source[0 * 36 + 29] = 0x31;
    source[0 * 36 + 30] = 10;
    source[0 * 36 + 31] = 0x42;
    source[0 * 36 + 35] = 0x7e;
    source[70 * 36 + 35] = 0x55;

    ok &= expect_int("clip.native.copied",
                     apply_d2l2_c10_frame_clip(spec, source, 36,
                                               destination, 224, 100, 0),
                     3,
                     "ReDMCSB DUNVIEW.C:3113-3129 F0104 C10 blit");
    ok &= expect_int("clip.native.transparent_skip",
                     destination[20 * 224 + 0], 0xee,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("clip.native.next_pixel",
                     destination[20 * 224 + 1], 0x42,
                     "ReDMCSB DUNVIEW.C:6954-6964/3113-3129 source x=31");
    ok &= expect_int("clip.native.pre_source_not_copied",
                     destination[20 * 224 + 0] != 0x31, 1,
                     "ReDMCSB DUNVIEW.C:6954-6964 source x=30");
    ok &= expect_int("clip.native.right_edge",
                     destination[20 * 224 + 5], 0x7e,
                     "ReDMCSB DUNVIEW.C:6954-6964 source x=35");
    ok &= expect_int("clip.native.after_source_clip",
                     destination[20 * 224 + 6], 0xee,
                     "ReDMCSB DUNVIEW.C:6954-6964 D2L2 source columns clip");
    ok &= expect_int("clip.native.bottom_edge",
                     destination[90 * 224 + 5], 0x55,
                     "ReDMCSB DUNVIEW.C:6954-6964 height 71");
    ok &= expect_int("clip.native.after_bottom",
                     destination[91 * 224 + 5], 0xee,
                     "ReDMCSB DUNVIEW.C:6954-6964 y2=90");

    memset(source, 10, sizeof(source));
    memset(small_destination, 0xee, sizeof(small_destination));
    source[0 * 36 + 30] = 0x21;
    source[0 * 36 + 31] = 0x22;
    source[0 * 36 + 35] = 0x25;

    ok &= expect_int("clip.small.copied",
                     apply_d2l2_c10_frame_clip(spec, source, 36,
                                               small_destination, 4, 30, 0),
                     2,
                     "ReDMCSB DUNVIEW.C:3113-3129 F0104 viewport clip");
    ok &= expect_int("clip.small.pixel0",
                     small_destination[20 * 4 + 0], 0x21,
                     "ReDMCSB DUNVIEW.C:6954-6964 clipped x0");
    ok &= expect_int("clip.small.pixel1",
                     small_destination[20 * 4 + 1], 0x22,
                     "ReDMCSB DUNVIEW.C:6954-6964 clipped x1");
    ok &= expect_int("clip.reject_null",
                     apply_d2l2_c10_frame_clip(NULL, source, 36,
                                               destination, 224, 100, 0),
                     -1,
                     "contract rejects unresolved D2L2 route");

    return ok;
}

static int test_media709_flip_blit(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2WallRouteSpec *spec =
        csb_v1_viewport_d2l2_wall_route_spec_pc34();
    uint8_t source[36 * 71];
    uint8_t destination[224 * 100];

    memset(source, 10, sizeof(source));
    memset(destination, 0xee, sizeof(destination));
    source[0 * 36 + 5] = 10;
    source[0 * 36 + 4] = 0x63;
    source[0 * 36 + 0] = 0x6e;

    ok &= expect_int("flip.copied",
                     apply_d2l2_c10_frame_clip(spec, source, 36,
                                               destination, 224, 100, 1),
                     2,
                     "ReDMCSB DUNVIEW.C:3185-3204 F0105 scratch flip");
    ok &= expect_int("flip.transparent_skip",
                     destination[20 * 224 + 0], 0xee,
                     "ReDMCSB DUNVIEW.C:3201 F0105 C10 transparent");
    ok &= expect_int("flip.next_pixel",
                     destination[20 * 224 + 1], 0x63,
                     "ReDMCSB DUNVIEW.C:6849-6851 G2107[C05_WALL_D2R2]");
    ok &= expect_int("flip.right_edge",
                     destination[20 * 224 + 5], 0x6e,
                     "ReDMCSB DUNVIEW.C:3185-3204 flipped source edge");
    ok &= expect_int("flip.d2r2_zone_untouched",
                     destination[20 * 224 + 186], 0xee,
                     "ReDMCSB DUNVIEW.C:6850 writes C707, not C708");

    return ok;
}

static int test_teleporter_and_no_thing_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2WallRouteSpec *spec =
        csb_v1_viewport_d2l2_wall_route_spec_pc34();

    ok &= expect_int("teleporter.f0113_route",
                     spec ? spec->f0113_teleporter_route : -1, 1,
                     "ReDMCSB DUNVIEW.C:6863-6865 F0113");
    ok &= expect_int("teleporter.field_aspect",
                     spec ? spec->teleporter_field_aspect_index : -1, 5,
                     "ReDMCSB DUNVIEW.C:377 G2035[C09_VIEW_SQUARE_D2L2]");
    ok &= expect_int("teleporter.zone",
                     spec ? spec->wall_zone : -1, 707,
                     "ReDMCSB DUNVIEW.C:6865 C707_ZONE_WALL_D2L2");
    ok &= expect_int("return.no_f0107",
                     spec ? spec->f0107_wall_ornament_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6859-6862 returns before F0107");
    ok &= expect_int("thing.no_f0111",
                     spec ? spec->f0111_door_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6837-6872 no F0111 door route");
    ok &= expect_int("thing.no_f0115",
                     spec ? spec->f0115_thing_pass_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6837-6872 no F0115 thing pass");

    return ok;
}

static int test_f0128_and_csb_viewport_evidence(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2WallRouteSpec *spec =
        csb_v1_viewport_d2l2_wall_route_spec_pc34();
    const char *e = csb_v1_viewport_d2l2_wall_source_evidence_pc34();

    ok &= expect_int("f0128.order",
                     spec ? spec->f0128_draw_order_index : -1, 8,
                     "ReDMCSB DUNVIEW.C:8500-8504 F0128 D2L2 dispatch");
    ok &= expect_int("f0128.relative_depth",
                     spec ? spec->f0128_relative_depth : -1, 2,
                     "ReDMCSB DUNVIEW.C:8501 depth 2");
    ok &= expect_int("f0128.relative_lateral",
                     spec ? spec->f0128_relative_lateral : 1, -2,
                     "ReDMCSB DUNVIEW.C:8501 lateral -2");
    ok &= expect_int("csb_viewport.pwall_left_pair",
                     spec ? spec->csb_viewport_pwallbitmap_left_pair_index : -1, 5,
                     "CSB Viewport.cpp:2267 pWallBitmaps[5]");
    ok &= expect_int("csb_viewport.pwall_right_pair",
                     spec ? spec->csb_viewport_pwallbitmap_right_pair_index : -1, 6,
                     "CSB Viewport.cpp:2271 pWallBitmaps[6]");
    ok &= expect_contains("evidence.f0678", e, "F0678_DrawD2L2",
                          "ReDMCSB DUNVIEW.C:6837-6872");
    ok &= expect_contains("evidence.f0104", e, "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap",
                          "ReDMCSB DUNVIEW.C:3113-3129");
    ok &= expect_contains("evidence.f0105", e, "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally",
                          "ReDMCSB DUNVIEW.C:3185-3204");
    ok &= expect_contains("evidence.f0113", e, "F0113_DUNGEONVIEW_DrawField",
                          "ReDMCSB DUNVIEW.C:6863-6865");
    ok &= expect_contains("evidence.f0128", e, "DUNVIEW.C:5920-5923 F0128",
                          "ReDMCSB DUNVIEW.C:5920-5923");
    ok &= expect_contains("evidence.c06_plus_2", e, "C06_WALL_D2L2 + 2",
                          "ReDMCSB DUNVIEW.C:6853-6858 PC_FIX_CODE_SIZE");
    ok &= expect_contains("evidence.c05", e, "C05_WALL_D2R2=5",
                          "ReDMCSB DEFS.H:3428");
    ok &= expect_contains("evidence.c707", e, "C707_ZONE_WALL_D2L2=707",
                          "ReDMCSB DEFS.H:4047");
    ok &= expect_contains("evidence.pwall", e, "CSB Viewport.cpp:2267/2271 pWallBitmaps",
                          "CSB Viewport.cpp:2267/2271");
    ok &= expect_contains("evidence.no_f0115", e, "no F0115 thing pass",
                          "ReDMCSB DUNVIEW.C:6837-6872");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2l2_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n", csb_v1_viewport_d2l2_wall_source_evidence_pc34());

    ok &= test_route_spec();
    ok &= test_wall_and_media709_routes();
    ok &= test_d2l2_frame_and_c10_blit();
    ok &= test_media709_flip_blit();
    ok &= test_teleporter_and_no_thing_contract();
    ok &= test_f0128_and_csb_viewport_evidence();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_35", g_assertions >= 35, 1,
                     "assigned D2L2 wall route parity gate");

    if (ok) {
        printf("PASS csb_v1_viewport_d2l2_wall_pc34_compat\n");
    }
    return ok ? 0 : 1;
}
