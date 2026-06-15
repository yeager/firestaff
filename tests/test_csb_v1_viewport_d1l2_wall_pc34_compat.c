#include "csb_v1_viewport_d1l2_wall_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_nonnull(const char *id, const void *got, const char *anchor)
{
    ++g_assertions;
    if (!got) {
        printf("FAIL %s got=NULL at %s\n", id, anchor);
        ++g_failures;
    } else {
        printf("PASS %s nonnull (%s)\n", id, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)",
               anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void test_route_and_limitation_contract(void)
{
    const CSB_V1_ViewportD1L2WallRouteSpecPc34 *spec =
        csb_v1_viewport_d1l2_wall_route_spec_pc34();

    expect_nonnull("d1l2.route.spec", spec, "ReDMCSB DUNVIEW.C:7391 F0122");
    if (!spec) return;

    expect_int("d1l2.contract_only", spec->source_locked_contract_only, 1,
               "source-locked contract gate");
    expect_int("d1l2.no_asset_parity", spec->no_asset_parity, 1,
               "no_asset_parity marker");
    expect_int("d1l2.addressable_by_name", spec->requested_d1l2_addressable, 0,
               "ReDMCSB DEFS.H:2599-2601 has D1C/D1L/D1R, no D1L2");
    expect_int("d1l2.uses_d1l_analogue", spec->uses_d1l_closest_analogue, 1,
               "ReDMCSB DUNVIEW.C:7391-7460 F0122 D1L wall route");
    expect_int("d1l2.view_square", spec->view_square, 4,
               "ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L");
    expect_int("d1l2.relative_depth", spec->relative_depth, 1,
               "ReDMCSB DUNVIEW.C:8524 depth 1");
    expect_int("d1l2.relative_lateral", spec->relative_lateral, -1,
               "ReDMCSB DUNVIEW.C:8524 lateral -1");
}

static void test_frame_bitmap_clip_metadata(void)
{
    int x = -1;
    int y = -1;
    int width = -1;
    int height = -1;
    const CSB_V1_ViewportD1L2WallRouteSpecPc34 *spec =
        csb_v1_viewport_d1l2_wall_route_spec_pc34();

    expect_int("d1l2.wall_element", spec ? spec->wall_element : -1, 0,
               "ReDMCSB DEFS.H:1007 C00_ELEMENT_WALL");
    expect_int("d1l2.teleporter_element", spec ? spec->teleporter_element : -1, 5,
               "ReDMCSB DEFS.H:1012 C05_ELEMENT_TELEPORTER");
    expect_int("d1l2.wall_zone", spec ? spec->wall_zone : -1, 713,
               "ReDMCSB DEFS.H:4053 C713_ZONE_WALL_D1L");
    expect_int("d1l2.neighbor_d1c_zone", spec ? spec->neighboring_d1c_zone : -1, 712,
               "ReDMCSB DEFS.H:4052 C712_ZONE_WALL_D1C");
    expect_int("d1l2.neighbor_d1r_zone", spec ? spec->neighboring_d1r_zone : -1, 714,
               "ReDMCSB DEFS.H:4054 C714_ZONE_WALL_D1R");
    expect_int("d1l2.native_wall_index", spec ? spec->native_wall_index : -1, 3,
               "ReDMCSB DEFS.H:3426 C03_WALL_D1L");
    expect_int("d1l2.flipped_wall_index", spec ? spec->flipped_wall_index : -1, 2,
               "ReDMCSB DEFS.H:3425 C02_WALL_D1R");
    expect_int("d1l2.frame_array_index", spec ? spec->frame_array_index : -1, 4,
               "ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L");
    expect_int("d1l2.frame_x1", spec ? spec->frame_x1 : -1, 0,
               "ReDMCSB DUNVIEW.C:590 G0163 D1L");
    expect_int("d1l2.frame_x2", spec ? spec->frame_x2 : -1, 63,
               "ReDMCSB DUNVIEW.C:590 G0163 D1L");
    expect_int("d1l2.frame_y1", spec ? spec->frame_y1 : -1, 9,
               "ReDMCSB DUNVIEW.C:590 G0163 D1L");
    expect_int("d1l2.frame_y2", spec ? spec->frame_y2 : -1, 119,
               "ReDMCSB DUNVIEW.C:590 G0163 D1L");
    expect_int("d1l2.frame_byte_width", spec ? spec->frame_byte_width : -1, 128,
               "ReDMCSB DUNVIEW.C:590 G0163 D1L");
    expect_int("d1l2.frame_height", spec ? spec->frame_height : -1, 111,
               "ReDMCSB DUNVIEW.C:590 G0163 D1L");
    expect_int("d1l2.frame_source_x", spec ? spec->frame_source_x : -1, 192,
               "ReDMCSB DUNVIEW.C:590 G0163 D1L");
    expect_int("d1l2.frame_source_y", spec ? spec->frame_source_y : -1, 0,
               "ReDMCSB DUNVIEW.C:590 G0163 D1L");
    expect_int("d1l2.clip_width", spec ? spec->clip_width : -1, 64,
               "ReDMCSB DUNVIEW.C:590 X1..X2 inclusive");
    expect_int("d1l2.clip_height", spec ? spec->clip_height : -1, 111,
               "ReDMCSB DUNVIEW.C:590 Y1..Y2 inclusive");
    expect_int("d1l2.resolve_clip",
               csb_v1_viewport_d1l2_wall_resolve_clip_pc34(spec, &x, &y,
                                                            &width, &height),
               0, "clip helper contract");
    expect_int("d1l2.resolved_x", x, 0, "ReDMCSB DUNVIEW.C:590");
    expect_int("d1l2.resolved_y", y, 9, "ReDMCSB DUNVIEW.C:590");
    expect_int("d1l2.resolved_width", width, 64, "ReDMCSB DUNVIEW.C:590");
    expect_int("d1l2.resolved_height", height, 111, "ReDMCSB DUNVIEW.C:590");
}

static void test_c10_clip_and_no_write_pixels(void)
{
    const CSB_V1_ViewportD1L2WallRouteSpecPc34 *spec =
        csb_v1_viewport_d1l2_wall_route_spec_pc34();
    uint8_t source[320 * 111];
    uint8_t destination[224 * 136];
    CSB_V1_ViewportD1L2WallBlitStatsPc34 stats;

    memset(source, 10, sizeof(source));
    memset(destination, 0xee, sizeof(destination));
    source[0 * 320 + 192] = 10;
    source[0 * 320 + 193] = 0x31;
    source[0 * 320 + 255] = 0x7a;
    source[110 * 320 + 192] = 0x55;

    expect_int("d1l2.transparent_color", spec ? spec->transparent_color : -1, 10,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("d1l2.c10_native.copied",
               csb_v1_viewport_d1l2_wall_apply_c10_frame_clip_pc34(
                   spec, source, 320, 111, destination, 224, 136, 0, &stats),
               3, "ReDMCSB DUNVIEW.C:3113-3129 F0104 C10 blit");
    expect_int("d1l2.c10_native.stats_copied", stats.copied_pixels, 3,
               "deterministic capture stats");
    expect_int("d1l2.c10_native.transparent_skips", stats.transparent_pixels,
               (64 * 111) - 3, "ReDMCSB DEFS.H:2088 transparency skip");
    expect_int("d1l2.c10_native.left_edge_transparent_no_write",
               destination[9 * 224 + 0], 0xee,
               "ReDMCSB DUNVIEW.C:590 x1 with C10");
    expect_int("d1l2.c10_native.next_pixel",
               destination[9 * 224 + 1], 0x31,
               "deterministic pixel source[193]");
    expect_int("d1l2.c10_native.right_edge",
               destination[9 * 224 + 63], 0x7a,
               "ReDMCSB DUNVIEW.C:590 x2 clipped edge");
    expect_int("d1l2.c10_native.right_neighbor_no_write",
               destination[9 * 224 + 64], 0xee,
               "neighboring pixel outside D1L clip");
    expect_int("d1l2.c10_native.top_neighbor_no_write",
               destination[8 * 224 + 1], 0xee,
               "neighboring pixel above D1L clip");
    expect_int("d1l2.c10_native.bottom_edge",
               destination[119 * 224 + 0], 0x55,
               "ReDMCSB DUNVIEW.C:590 y2 clipped edge");
    expect_int("d1l2.c10_native.bottom_neighbor_no_write",
               destination[120 * 224 + 0], 0xee,
               "neighboring pixel below D1L clip");

    memset(destination, 0xee, sizeof(destination));
    expect_int("d1l2.viewport_clip.copied",
               csb_v1_viewport_d1l2_wall_apply_c10_frame_clip_pc34(
                   spec, source, 320, 111, destination, 4, 20, 0, &stats),
               1, "ReDMCSB COORD.C:2390-2410 clipped viewport edge");
    expect_int("d1l2.viewport_clip.clipped_pixels", stats.clipped_pixels,
               (60 * 111) + (4 * 100),
               "deterministic no-write metadata for clipped pixels");
    expect_int("d1l2.viewport_clip.pixel1", destination[9 * 4 + 1], 0x31,
               "clipped small viewport keeps in-bounds non-C10");
}

static void test_flipped_route_and_rejections(void)
{
    const CSB_V1_ViewportD1L2WallRouteSpecPc34 *spec =
        csb_v1_viewport_d1l2_wall_route_spec_pc34();
    uint8_t source[320 * 111];
    uint8_t destination[224 * 136];
    CSB_V1_ViewportD1L2WallBlitStatsPc34 stats;

    memset(source, 10, sizeof(source));
    memset(destination, 0xee, sizeof(destination));
    source[0 * 320 + 255] = 10;
    source[0 * 320 + 254] = 0x44;
    source[0 * 320 + 192] = 0x66;

    expect_int("d1l2.f0100_st_route", spec ? spec->f0100_st_wall_route : -1, 1,
               "ReDMCSB DUNVIEW.C:7438 F0100");
    expect_int("d1l2.f0104_i34_route", spec ? spec->f0104_i34_wall_route : -1, 1,
               "ReDMCSB DUNVIEW.C:7454 F0104");
    expect_int("d1l2.f0105_flipped_route",
               spec ? spec->f0105_i34_flipped_route : -1, 1,
               "ReDMCSB DUNVIEW.C:7446 F0105");
    expect_int("d1l2.flip.copied",
               csb_v1_viewport_d1l2_wall_apply_c10_frame_clip_pc34(
                   spec, source, 320, 111, destination, 224, 136, 1, &stats),
               2, "ReDMCSB DUNVIEW.C:3185-3204 F0105 scratch flip");
    expect_int("d1l2.flip.transparent_left_no_write",
               destination[9 * 224 + 0], 0xee,
               "flipped source[255] is C10");
    expect_int("d1l2.flip.next_pixel", destination[9 * 224 + 1], 0x44,
               "flipped source[254]");
    expect_int("d1l2.flip.right_edge", destination[9 * 224 + 63], 0x66,
               "flipped source[192]");
    expect_int("d1l2.reject_short_source",
               csb_v1_viewport_d1l2_wall_apply_c10_frame_clip_pc34(
                   spec, source, 255, 111, destination, 224, 136, 0, &stats),
               -1, "helper rejects unresolved source clip");
    expect_int("d1l2.reject_short_source_flag", stats.rejected, 1,
               "no-write metadata on rejected source");
    expect_int("d1l2.reject_null_spec",
               csb_v1_viewport_d1l2_wall_apply_c10_frame_clip_pc34(
                   NULL, source, 320, 111, destination, 224, 136, 0, &stats),
               -1, "helper rejects missing source-locked route");
}

static void test_route_exclusions_and_evidence(void)
{
    const CSB_V1_ViewportD1L2WallRouteSpecPc34 *spec =
        csb_v1_viewport_d1l2_wall_route_spec_pc34();
    const char *e = csb_v1_viewport_d1l2_wall_source_evidence_pc34();

    expect_int("d1l2.f0107_wall_ornament",
               spec ? spec->f0107_wall_ornament_route : -1, 1,
               "ReDMCSB DUNVIEW.C:7459 F0107 before return");
    expect_int("d1l2.no_f0111_door_on_wall",
               spec ? spec->f0111_door_route : -1, 0,
               "ReDMCSB DUNVIEW.C:7436-7460 wall case returns before doors");
    expect_int("d1l2.teleporter_f0113",
               spec ? spec->f0113_teleporter_route : -1, 1,
               "ReDMCSB DUNVIEW.C:7538-7555 teleporter field");
    expect_int("d1l2.no_f0115_wall_thing_pass",
               spec ? spec->f0115_wall_thing_pass_route : -1, 0,
               "ReDMCSB DUNVIEW.C:7436-7460 wall case returns before things");
    expect_contains("d1l2.bitmap_symbol", spec ? spec->bitmap_symbol : NULL,
                    "G0700_puc_Bitmap_WallSet_Wall_D1LCR",
                    "ReDMCSB DUNVIEW.C:7438");
    expect_contains("d1l2.frame_symbol", spec ? spec->frame_symbol : NULL,
                    "G0163_aauc_Graphic558_Frame_Walls",
                    "ReDMCSB DUNVIEW.C:590");
    expect_contains("d1l2.evidence.contract", e, "Source-locked contract gate only",
                    "contract marker");
    expect_contains("d1l2.evidence.no_asset", e, "no_asset_parity",
                    "no real-asset bitmap parity marker");
    expect_contains("d1l2.evidence.no_named_d1l2", e, "no named D1L2",
                    "ReDMCSB DEFS.H:2599-2601");
    expect_contains("d1l2.evidence.f0122", e, "F0122_DUNGEONVIEW_DrawSquareD1L",
                    "ReDMCSB DUNVIEW.C:7391");
    expect_contains("d1l2.evidence.frame", e, "{0,63,9,119,128,111,192,0}",
                    "ReDMCSB DUNVIEW.C:590");
    expect_contains("d1l2.evidence.c10", e, "C10_COLOR_FLESH=10",
                    "ReDMCSB DEFS.H:2088");
    expect_contains("d1l2.evidence.non_overlap", e,
                    "non-overlap: CSB D1L2 wall route not yet covered; CSB D1L/D1R door and D1C center field already covered; this gate covers D1L2 wall route specifically",
                    "assigned non-overlap note");
    expect_contains("d1l2.evidence.limitation", e, "D1L2 is not addressable",
                    "documented D1L analogue limitation");
}

int main(void)
{
    printf("probe=csb_v1_viewport_d1l2_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d1l2_wall_source_evidence_pc34());

    test_route_and_limitation_contract();
    test_frame_bitmap_clip_metadata();
    test_c10_clip_and_no_write_pixels();
    test_flipped_route_and_rejections();
    test_route_exclusions_and_evidence();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS csb_v1_viewport_d1l2_wall_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
