#include "dm1_v1_viewport_d1l_d1r_wall_pc34_compat.h"

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

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void test_d1l_source_locked_spec(void)
{
    DM1_V1_D1LD1RWallInputPc34 input = {
        DM1_V1_D1L_D1R_WALL_ROUTE_D1L_NATIVE_PC34,
        9,
        0,
        0
    };
    DM1_V1_D1LD1RWallSpecPc34 spec;
    int source_x = -1;
    int source_y = -1;
    int scratch_x = -1;

    /* ReDMCSB: DUNVIEW.C:7436-7460 F0122 D1L WALL selects C03/C713 and returns. */
    expect_int("d1l.resolve", M11_GameView_D1LD1RWallResolvePc34(&input, &spec) ? 1 : 0,
               1, "DUNVIEW.C:7436-7460");
    expect_int("d1l.contract_only", spec.contract_only ? 1 : 0, 1,
               "source-lock gate only");
    expect_int("d1l.no_asset_parity", spec.real_asset_bitmap_parity ? 1 : 0, 0,
               "no real-asset claim");
    expect_int("d1l.depth", spec.depth, 1, "DUNVIEW.C:8524-8525");
    expect_int("d1l.lateral", spec.lateral, -1, "DUNVIEW.C:8524-8525");
    expect_int("d1l.view_square", spec.view_square_index, 4, "DEFS.H:2600");
    expect_int("d1l.selected_c03", spec.selected_wall_bitmap_index, 3,
               "DUNVIEW.C:7454 / DEFS.H:3426");
    expect_int("d1l.partner_c02", spec.parity_partner_wall_bitmap_index, 2,
               "DUNVIEW.C:2438-2439 / DEFS.H:3425");
    expect_int("d1l.zone_c713", spec.wall_zone_index, 713,
               "DUNVIEW.C:7454 / DEFS.H:4053");
    expect_int("d1l.zone_family_first", spec.wall_zone_family_first, 702,
               "DEFS.H:4042");
    expect_int("d1l.zone_family_last", spec.wall_zone_family_last, 717,
               "DEFS.H:4057");
    expect_int("d1l.frame_index", spec.frame_index, 4, "DUNVIEW.C:7438");
    expect_int("d1l.field_aspect", spec.field_aspect_index, 7, "DUNVIEW.C:727");
    expect_int("d1l.field_mask", spec.field_mask, 0x82, "DUNVIEW.C:727");
    expect_int("d1l.viewport_x0", spec.frame_viewport_x_first, 0, "DUNVIEW.C:590");
    expect_int("d1l.viewport_x1", spec.frame_viewport_x_last, 63, "DUNVIEW.C:590");
    expect_int("d1l.viewport_y0", spec.frame_viewport_y_first, 9, "DUNVIEW.C:590");
    expect_int("d1l.viewport_y1", spec.frame_viewport_y_last, 119, "DUNVIEW.C:590");
    expect_int("d1l.frame_source_x", spec.frame_source_x, 192, "DUNVIEW.C:590");
    expect_int("d1l.frame_byte_width", spec.frame_byte_width, 128, "DUNVIEW.C:590");
    expect_int("d1l.frame_height", spec.frame_height, 111, "DUNVIEW.C:590");
    expect_int("d1l.source_x_first", spec.source_x_first, 192, "DUNVIEW.C:590");
    expect_int("d1l.source_x_last", spec.source_x_last, 255, "DUNVIEW.C:590");
    expect_int("d1l.source_height", spec.source_height, 111, "DUNVIEW.C:590");
    expect_int("d1l.f0100", spec.uses_f0100_frame_blit ? 1 : 0, 1,
               "DUNVIEW.C:3048-3058");
    expect_int("d1l.f0104", spec.uses_f0104_native_blit ? 1 : 0, 1,
               "DUNVIEW.C:7454");
    expect_int("d1l.no_f0105", spec.uses_f0105_parity_scratch_flip ? 1 : 0, 0,
               "DUNVIEW.C:7454 native route");
    expect_int("d1l.c10", spec.transparent_color, 10, "DEFS.H:2088");
    expect_int("d1l.wall_returns", spec.wall_case_returns ? 1 : 0, 1,
               "DUNVIEW.C:7460");
    expect_int("d1l.f0107_probe", spec.calls_f0107_side_ornament_probe ? 1 : 0,
               1, "DUNVIEW.C:7459");
    expect_int("d1l.no_f0108", spec.calls_f0108_floor_ornament ? 1 : 0, 0,
               "DUNVIEW.C:7525 separate corridor route");
    expect_int("d1l.no_f0111", spec.calls_f0111_door ? 1 : 0, 0,
               "DUNVIEW.C:7496-7506 separate door route");
    expect_int("d1l.no_f0115", spec.calls_f0115_thing_pass ? 1 : 0, 0,
               "DUNVIEW.C:7460 returns before thing pass");

    expect_int("d1l.map.left",
               M11_GameView_D1LD1RWallMapViewportToSourcePc34(
                   &spec, 9, 0, &source_x, &source_y, &scratch_x) ? 1 : 0,
               1, "DUNVIEW.C:590 left edge");
    expect_int("d1l.map.left_source_x", source_x, 192, "DUNVIEW.C:590 X=192");
    expect_int("d1l.map.left_source_y", source_y, 0, "DUNVIEW.C:590 Y=0");
    expect_int("d1l.map.left_scratch", scratch_x, 192, "native route no scratch flip");
    expect_int("d1l.map.right",
               M11_GameView_D1LD1RWallMapViewportToSourcePc34(
                   &spec, 119, 63, &source_x, &source_y, &scratch_x) ? 1 : 0,
               1, "DUNVIEW.C:590 right/bottom edge");
    expect_int("d1l.map.right_source_x", source_x, 255, "DUNVIEW.C:590 source span");
    expect_int("d1l.map.bottom_source_y", source_y, 110, "DUNVIEW.C:590 height=111");
}

static void test_d1r_source_locked_spec_and_mirror(void)
{
    DM1_V1_D1LD1RWallInputPc34 input = {
        DM1_V1_D1L_D1R_WALL_ROUTE_D1R_PARITY_PC34,
        9,
        160,
        DM1_V1_D1L_D1R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D1LD1RWallSpecPc34 spec;
    int source_x = -1;
    int source_y = -1;
    int scratch_x = -1;

    /* ReDMCSB: DUNVIEW.C:7604-7628 F0123 D1R WALL exposes C03 flipped into C714. */
    expect_int("d1r.resolve", M11_GameView_D1LD1RWallResolvePc34(&input, &spec) ? 1 : 0,
               1, "DUNVIEW.C:7604-7628");
    expect_int("d1r.depth", spec.depth, 1, "DUNVIEW.C:8528-8529");
    expect_int("d1r.lateral", spec.lateral, 1, "DUNVIEW.C:8528-8529");
    expect_int("d1r.view_square", spec.view_square_index, 5, "DEFS.H:2601");
    expect_int("d1r.selected_c03_flipped", spec.selected_wall_bitmap_index, 3,
               "DUNVIEW.C:7614");
    expect_int("d1r.partner_c02", spec.parity_partner_wall_bitmap_index, 2,
               "DUNVIEW.C:7622");
    expect_int("d1r.zone_c714", spec.wall_zone_index, 714,
               "DUNVIEW.C:7614 / DEFS.H:4054");
    expect_int("d1r.frame_index", spec.frame_index, 5, "DUNVIEW.C:7606");
    expect_int("d1r.field_aspect", spec.field_aspect_index, 8, "DUNVIEW.C:728");
    expect_int("d1r.field_mask", spec.field_mask, 0x02, "DUNVIEW.C:728");
    expect_int("d1r.viewport_x0", spec.frame_viewport_x_first, 160, "DUNVIEW.C:591");
    expect_int("d1r.viewport_x1", spec.frame_viewport_x_last, 223, "DUNVIEW.C:591");
    expect_int("d1r.viewport_y0", spec.frame_viewport_y_first, 9, "DUNVIEW.C:591");
    expect_int("d1r.viewport_y1", spec.frame_viewport_y_last, 119, "DUNVIEW.C:591");
    expect_int("d1r.f0100", spec.uses_f0100_frame_blit ? 1 : 0, 1,
               "DUNVIEW.C:3048-3058");
    expect_int("d1r.no_f0104", spec.uses_f0104_native_blit ? 1 : 0, 0,
               "parity route uses F0105");
    expect_int("d1r.f0105", spec.uses_f0105_parity_scratch_flip ? 1 : 0, 1,
               "DUNVIEW.C:7614 / 3185-3204");
    expect_int("d1r.c10", spec.uses_c10_transparency ? 1 : 0, 1,
               "DUNVIEW.C:3201");
    expect_int("d1r.wall_returns", spec.wall_case_returns ? 1 : 0, 1,
               "DUNVIEW.C:7628");
    expect_int("d1r.f0107_probe", spec.calls_f0107_side_ornament_probe ? 1 : 0,
               1, "DUNVIEW.C:7627");

    expect_int("d1r.map.left",
               M11_GameView_D1LD1RWallMapViewportToSourcePc34(
                   &spec, 9, 160, &source_x, &source_y, &scratch_x) ? 1 : 0,
               1, "DUNVIEW.C:591 + F0105 mirror");
    expect_int("d1r.map.left_source_x", source_x, 255,
               "DUNVIEW.C:3185-3204 mirror from D1L source");
    expect_int("d1r.map.left_scratch", scratch_x, 192,
               "DUNVIEW.C:3199 temporary flipped copy");
    expect_int("d1r.map.left_source_y", source_y, 0, "DUNVIEW.C:591");
    expect_int("d1r.map.right",
               M11_GameView_D1LD1RWallMapViewportToSourcePc34(
                   &spec, 119, 223, &source_x, &source_y, &scratch_x) ? 1 : 0,
               1, "DUNVIEW.C:591 + F0105 mirror");
    expect_int("d1r.map.right_source_x", source_x, 192,
               "DUNVIEW.C:3185-3204 mirrored right edge");
    expect_int("d1r.map.right_scratch", scratch_x, 255,
               "DUNVIEW.C:3199 temporary flipped copy");
    expect_int("d1r.map.bottom_source_y", source_y, 110, "DUNVIEW.C:591");
}

static void test_pixels_c10_and_no_write_edges(void)
{
    uint8_t source[DM1_V1_D1L_D1R_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D1L_D1R_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D1L_D1R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D1L_D1R_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D1LD1RWallPixelPc34 pixel;
    DM1_V1_D1LD1RWallInputPc34 d1l = {
        DM1_V1_D1L_D1R_WALL_ROUTE_D1L_NATIVE_PC34,
        9,
        0,
        DM1_V1_D1L_D1R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D1LD1RWallInputPc34 d1r = {
        DM1_V1_D1L_D1R_WALL_ROUTE_D1R_PARITY_PC34,
        9,
        160,
        DM1_V1_D1L_D1R_WALL_C10_COLOR_FLESH_PC34
    };

    memset(source, 10, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[0 * DM1_V1_D1L_D1R_WALL_SOURCE_WIDTH_PC34 + 192] = 0x31;
    source[0 * DM1_V1_D1L_D1R_WALL_SOURCE_WIDTH_PC34 + 193] = 10;
    source[0 * DM1_V1_D1L_D1R_WALL_SOURCE_WIDTH_PC34 + 253] = 10;
    source[0 * DM1_V1_D1L_D1R_WALL_SOURCE_WIDTH_PC34 + 254] = 0x62;
    source[0 * DM1_V1_D1L_D1R_WALL_SOURCE_WIDTH_PC34 + 255] = 0x7d;
    source[110 * DM1_V1_D1L_D1R_WALL_SOURCE_WIDTH_PC34 + 192] = 0x55;

    expect_int("pixel.d1l.left.apply",
               M11_GameView_D1LD1RWallApplyPixelPc34(
                   &d1l, source, sizeof(source), viewport, sizeof(viewport), &pixel) ? 1 : 0,
               1, "DUNVIEW.C:3048-3058 F0100");
    expect_int("pixel.d1l.left.in_clip", pixel.in_clip ? 1 : 0, 1,
               "DUNVIEW.C:590 frame");
    expect_int("pixel.d1l.left.source_x", pixel.source_x, 192, "DUNVIEW.C:590");
    expect_int("pixel.d1l.left.writes", pixel.writes_pixel ? 1 : 0, 1,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("pixel.d1l.left.value", pixel.pixel_after, 0x31,
               "deterministic source pixel");

    d1l.viewport_x = 1;
    expect_int("pixel.d1l.c10.apply",
               M11_GameView_D1LD1RWallApplyPixelPc34(
                   &d1l, source, sizeof(source), viewport, sizeof(viewport), &pixel) ? 1 : 0,
               1, "DEFS.H:2088 C10");
    expect_int("pixel.d1l.c10.skip", pixel.transparent_skip ? 1 : 0, 1,
               "DUNVIEW.C:3055");
    expect_int("pixel.d1l.c10.no_write", pixel.writes_pixel ? 1 : 0, 0,
               "DUNVIEW.C:3055");
    expect_int("pixel.d1l.c10.preserved", pixel.pixel_after, 0xee,
               "DUNVIEW.C:3055");

    d1r.viewport_x = 160;
    expect_int("pixel.d1r.left.apply",
               M11_GameView_D1LD1RWallApplyPixelPc34(
                   &d1r, source, sizeof(source), viewport, sizeof(viewport), &pixel) ? 1 : 0,
               1, "DUNVIEW.C:7614 F0105");
    expect_int("pixel.d1r.left.source_x", pixel.source_x, 255,
               "DUNVIEW.C:3185-3204 mirror");
    expect_int("pixel.d1r.left.scratch_x", pixel.scratch_x, 192,
               "DUNVIEW.C:3199 temporary flip");
    expect_int("pixel.d1r.left.value", pixel.pixel_after, 0x7d,
               "deterministic mirrored source pixel");

    d1r.viewport_x = 161;
    expect_int("pixel.d1r.next.apply",
               M11_GameView_D1LD1RWallApplyPixelPc34(
                   &d1r, source, sizeof(source), viewport, sizeof(viewport), &pixel) ? 1 : 0,
               1, "DUNVIEW.C:7614 F0105");
    expect_int("pixel.d1r.next.source_x", pixel.source_x, 254,
               "DUNVIEW.C:3185-3204 mirror");
    expect_int("pixel.d1r.next.value", pixel.pixel_after, 0x62,
               "deterministic mirrored neighbor pixel");

    d1r.viewport_x = 162;
    expect_int("pixel.d1r.c10.apply",
               M11_GameView_D1LD1RWallApplyPixelPc34(
                   &d1r, source, sizeof(source), viewport, sizeof(viewport), &pixel) ? 1 : 0,
               1, "DUNVIEW.C:3185-3204 F0105 mirrored C10 skip");
    expect_int("pixel.d1r.c10.source_x", pixel.source_x, 253,
               "DUNVIEW.C:3185-3204 mirrored D1R source walk");
    expect_int("pixel.d1r.c10.scratch_x", pixel.scratch_x, 194,
               "DUNVIEW.C:3199 temporary flipped copy");
    expect_int("pixel.d1r.c10.skip", pixel.transparent_skip ? 1 : 0, 1,
               "DUNVIEW.C:3201 C10_COLOR_FLESH skip");
    expect_int("pixel.d1r.c10.no_write", pixel.writes_pixel ? 1 : 0, 0,
               "DUNVIEW.C:3201 transparent pixel preserves destination");
    expect_int("pixel.d1r.c10.preserved", pixel.pixel_after, 0xee,
               "DUNVIEW.C:3185-3204 F0105 C10 transparency");

    d1r.row = 119;
    d1r.viewport_x = 223;
    expect_int("pixel.d1r.bottom_right.apply",
               M11_GameView_D1LD1RWallApplyPixelPc34(
                   &d1r, source, sizeof(source), viewport, sizeof(viewport), &pixel) ? 1 : 0,
               1, "DUNVIEW.C:591 bottom/right");
    expect_int("pixel.d1r.bottom_right.source_x", pixel.source_x, 192,
               "DUNVIEW.C:3185-3204 mirror");
    expect_int("pixel.d1r.bottom_right.source_y", pixel.source_y, 110,
               "DUNVIEW.C:591 height=111");
    expect_int("pixel.d1r.bottom_right.value", pixel.pixel_after, 0x55,
               "deterministic bottom mirrored pixel");

    d1l.row = 8;
    d1l.viewport_x = 0;
    expect_int("pixel.before_top.apply",
               M11_GameView_D1LD1RWallApplyPixelPc34(
                   &d1l, source, sizeof(source), viewport, sizeof(viewport), &pixel) ? 1 : 0,
               1, "DUNVIEW.C:590 no-write above frame");
    expect_int("pixel.before_top.no_write", pixel.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:590");
    expect_int("pixel.before_top.in_clip", pixel.in_clip ? 1 : 0, 0,
               "DUNVIEW.C:590");

    d1r.row = 9;
    d1r.viewport_x = 159;
    expect_int("pixel.before_d1r.apply",
               M11_GameView_D1LD1RWallApplyPixelPc34(
                   &d1r, source, sizeof(source), viewport, sizeof(viewport), &pixel) ? 1 : 0,
               1, "DUNVIEW.C:591 no-write before D1R frame");
    expect_int("pixel.before_d1r.no_write", pixel.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:591");
    expect_int("pixel.before_d1r.viewport_untouched",
               viewport[9 * DM1_V1_D1L_D1R_WALL_VIEWPORT_WIDTH_PC34 + 159], 0xee,
               "DUNVIEW.C:591");
}

static void test_invalid_inputs_and_blend(void)
{
    DM1_V1_D1LD1RWallInputPc34 input = {
        DM1_V1_D1L_D1R_WALL_ROUTE_D1L_NATIVE_PC34,
        9,
        0,
        DM1_V1_D1L_D1R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D1LD1RWallInputPc34 bad_route = {
        (DM1_V1_D1LD1RWallRoutePc34)99,
        9,
        0,
        DM1_V1_D1L_D1R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D1LD1RWallSpecPc34 spec;
    DM1_V1_D1LD1RWallPixelPc34 pixel;
    int source_x = -1;
    int source_y = -1;
    int scratch_x = -1;

    expect_int("invalid.null_input", M11_GameView_D1LD1RWallResolvePc34(NULL, &spec) ? 1 : 0,
               0, "contract rejects null input");
    expect_int("invalid.null_output", M11_GameView_D1LD1RWallResolvePc34(&input, NULL) ? 1 : 0,
               0, "contract rejects null output");
    expect_int("invalid.bad_route", M11_GameView_D1LD1RWallResolvePc34(&bad_route, &spec) ? 1 : 0,
               0, "route enum guard");
    expect_int("invalid.null_pixel_out",
               M11_GameView_D1LD1RWallApplyPixelPc34(&input, NULL, 0, NULL, 0, NULL) ? 1 : 0,
               0, "contract rejects null pixel output");
    expect_int("invalid.map_null_spec",
               M11_GameView_D1LD1RWallMapViewportToSourcePc34(
                   NULL, 9, 0, &source_x, &source_y, &scratch_x) ? 1 : 0,
               0, "map rejects null spec");
    expect_int("invalid.map_null_x",
               M11_GameView_D1LD1RWallMapViewportToSourcePc34(
                   &spec, 9, 0, NULL, &source_y, &scratch_x) ? 1 : 0,
               0, "map rejects null source_x");
    expect_int("blend.c10", M11_GameView_D1LD1RWallBlendPixelPc34(0x44, 10, 10),
               0x44, "DEFS.H:2088");
    expect_int("blend.opaque", M11_GameView_D1LD1RWallBlendPixelPc34(0x44, 0x52, 10),
               0x52, "DUNVIEW.C:3055 / 3201");
    expect_int("invalid.null_source_in_clip",
               M11_GameView_D1LD1RWallApplyPixelPc34(&input, NULL, 0, NULL, 0, &pixel) ? 1 : 0,
               0, "in-clip pixel needs buffers");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = M11_GameView_D1LD1RWallSourceLockPc34();

    expect_contains("evidence.contract", e, "contract_only=1", "source evidence");
    expect_contains("evidence.no_asset", e, "no_asset_parity=1", "source evidence");
    expect_contains("evidence.f0122", e, "DUNVIEW.C:7436-7460 F0122",
                    "DUNVIEW.C D1L wall");
    expect_contains("evidence.f0123", e, "DUNVIEW.C:7604-7628 F0123",
                    "DUNVIEW.C D1R wall");
    expect_contains("evidence.dispatch", e, "DUNVIEW.C:8524-8529",
                    "DUNVIEW.C draw order depth/lateral");
    expect_contains("evidence.f0100", e, "DUNVIEW.C:3048-3058 F0100",
                    "DUNVIEW.C wallset blit");
    expect_contains("evidence.f0105", e, "DUNVIEW.C:3185-3204 F0105",
                    "DUNVIEW.C parity flip");
    expect_contains("evidence.frames", e, "DUNVIEW.C:581-591",
                    "DUNVIEW.C frame metadata");
    expect_contains("evidence.fields", e, "DUNVIEW.C:718-731 G0188",
                    "DUNVIEW.C field rows");
    expect_contains("evidence.flip_partner", e, "DUNVIEW.C:2438-2439",
                    "DUNVIEW.C C02/C03 flip partner");
    expect_contains("evidence.drawviewport", e, "DRAWVIEW.C:709-722 F0097",
                    "DRAWVIEW.C viewport presentation");
    expect_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                    "DEFS.H C10");
    expect_contains("evidence.squares", e, "DEFS.H:2600-2601",
                    "DEFS.H view square constants");
    expect_contains("evidence.wall_indices", e, "DEFS.H:3425-3426",
                    "DEFS.H wall indices");
    expect_contains("evidence.zones", e, "DEFS.H:4053-4054",
                    "DEFS.H C713/C714 zones");
    expect_contains("evidence.panel_note", e, "PANEL.C contains no symbolic C713/C714",
                    "PANEL.C source grep result");
}

int main(void)
{
    test_d1l_source_locked_spec();
    test_d1r_source_locked_spec_and_mirror();
    test_pixels_c10_and_no_write_edges();
    test_invalid_inputs_and_blend();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d1l_d1r_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d1l_d1r_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
