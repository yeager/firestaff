#include "dm1_v1_viewport_d2l_d2r_wall_pc34_compat.h"

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

static void test_d2l_spec_source_locked_metadata(void)
{
    const DM1_V1_D2LD2RWallSpecPc34 *spec =
        dm1_v1_viewport_d2l_d2r_wall_spec_pc34(DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    int source_x = -1;
    int source_y = -1;

    /* ReDMCSB: DUNVIEW.C:6900 F0119; wall path lines 6947-6971. */
    expect_int("d2l.spec.present", spec != NULL ? 1 : 0, 1, "DUNVIEW.C:6900 F0119");
    expect_int("d2l.contract_only", spec->contract_only ? 1 : 0, 1,
               "contract-only synthetic regression");
    expect_int("d2l.no_asset_parity", spec->real_asset_bitmap_parity ? 1 : 0, 0,
               "no real-asset bitmap parity claim");
    expect_int("d2l.depth", spec->depth, 2, "DUNVIEW.C:8511-8513 relative 2,-1");
    expect_int("d2l.lateral", spec->lateral, -1, "DUNVIEW.C:8511-8513 relative 2,-1");
    expect_int("d2l.view_square", spec->view_square_index, 4,
               "DEFS.H:2582 M604_VIEW_SQUARE_D2L");
    expect_int("d2l.native_wall", spec->native_wall_index_pc34, 8,
               "DEFS.H:3431 C08_WALL_D2L");
    expect_int("d2l.flipped_wall", spec->flipped_wall_index_pc34, 7,
               "DEFS.H:3430 C07_WALL_D2R");
    expect_int("d2l.zone", spec->wall_zone_pc34, 710,
               "DEFS.H:4050 C710_ZONE_WALL_D2L");
    expect_int("d2l.order", spec->draw_order_index, 10,
               "DUNVIEW.C:8508 center field then D2L");
    expect_int("d2l.center_before", spec->center_field_order_index < spec->draw_order_index,
               1, "DUNVIEW.C:8508-8513 F0128 draw order");
    expect_int("d2l.raw_frame_x0", spec->raw_frame_viewport_x_first, 0,
               "DUNVIEW.C:585 G0163 D2L");
    expect_int("d2l.raw_frame_x1", spec->raw_frame_viewport_x_last, 74,
               "DUNVIEW.C:585 G0163 D2L");
    expect_int("d2l.raw_frame_y0", spec->raw_frame_viewport_y_first, 20,
               "DUNVIEW.C:585 G0163 D2L");
    expect_int("d2l.raw_frame_y1", spec->raw_frame_viewport_y_last, 90,
               "DUNVIEW.C:585 G0163 D2L");
    expect_int("d2l.raw_byte_width", spec->raw_frame_byte_width, 72,
               "DUNVIEW.C:585 G0163 D2L");
    expect_int("d2l.raw_height", spec->raw_frame_height, 71,
               "DUNVIEW.C:585 G0163 D2L");
    expect_int("d2l.raw_source_x", spec->raw_frame_source_x, 61,
               "DUNVIEW.C:585 G0163 D2L");
    expect_int("d2l.slice_viewport_x0", spec->viewport_x_first, 0,
               "G0163 D2L side slice viewport X 0..10");
    expect_int("d2l.slice_viewport_x1", spec->viewport_x_last, 10,
               "G0163 D2L side slice viewport X 0..10");
    expect_int("d2l.slice_source_x0", spec->source_x_first, 61,
               "G0163 D2L side slice source X 61..71");
    expect_int("d2l.slice_source_x1", spec->source_x_last, 71,
               "G0163 D2L side slice source X 61..71");
    expect_int("d2l.visible_width", spec->visible_width, 11,
               "D2L side-wall pixel slice width");
    expect_int("d2l.uses_f0100", spec->uses_f0100_frame_blit ? 1 : 0, 1,
               "DUNVIEW.C:3048-3058 F0100");
    expect_int("d2l.uses_f0105", spec->uses_f0105_party_side_flip ? 1 : 0, 1,
               "DUNVIEW.C:3185-3204 F0105 / F0128 flip pointer");
    expect_int("d2l.uses_g0699", spec->uses_g0699_wall_d2lcr_pointer ? 1 : 0, 1,
               "DUNVIEW.C:6947 G0699_puc_Bitmap_WallSet_Wall_D2LCR");
    expect_int("d2l.c10", spec->preserves_c10_transparency ? 1 : 0, 1,
               "DUNVIEW.C:3055 C10_COLOR_FLESH");
    expect_int("d2l.wall_returns", spec->wall_case_returns ? 1 : 0, 1,
               "F0122 wall-return evidence style; D2L returns after F0107");
    expect_int("d2l.f0107_probe", spec->calls_f0107_side_ornament_probe ? 1 : 0, 1,
               "DUNVIEW.C:6968-6969 F0107");
    expect_int("d2l.no_f0108", spec->calls_f0108_floor_ornament ? 1 : 0, 0,
               "F0119 wall case returns before corridor floor route");
    expect_int("d2l.no_f0111", spec->calls_f0111_door ? 1 : 0, 0,
               "F0119 door-front route is separate");
    expect_int("d2l.no_f0115", spec->calls_f0115_thing_pass ? 1 : 0, 0,
               "F0119 wall case returns before thing pass");
    expect_int("d2l.map.left",
               dm1_v1_viewport_d2l_d2r_wall_map_pixel_pc34(
                   spec, 20, 0, &source_x, &source_y) ? 1 : 0,
               1, "D2L left/top side slice");
    expect_int("d2l.map.left_source_x", source_x, 61, "G0163 D2L source X=61");
    expect_int("d2l.map.left_source_y", source_y, 0, "G0163 D2L source Y=0");
    expect_int("d2l.map.right",
               dm1_v1_viewport_d2l_d2r_wall_map_pixel_pc34(
                   spec, 90, 10, &source_x, &source_y) ? 1 : 0,
               1, "D2L right/bottom side slice");
    expect_int("d2l.map.right_source_x", source_x, 71, "G0163 D2L source X 61..71");
    expect_int("d2l.map.bottom_source_y", source_y, 70, "G0163 D2L height=71");
}

static void test_d2r_spec_source_locked_metadata(void)
{
    const DM1_V1_D2LD2RWallSpecPc34 *spec =
        dm1_v1_viewport_d2l_d2r_wall_spec_pc34(DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34);
    int source_x = -1;
    int source_y = -1;

    /* ReDMCSB: DUNVIEW.C:7051 F0120; wall path lines 7098-7122. */
    expect_int("d2r.spec.present", spec != NULL ? 1 : 0, 1, "DUNVIEW.C:7051 F0120");
    expect_int("d2r.contract_only", spec->contract_only ? 1 : 0, 1,
               "contract-only synthetic regression");
    expect_int("d2r.no_asset_parity", spec->real_asset_bitmap_parity ? 1 : 0, 0,
               "no real-asset bitmap parity claim");
    expect_int("d2r.depth", spec->depth, 2, "DUNVIEW.C:8515-8517 relative 2,1");
    expect_int("d2r.lateral", spec->lateral, 1, "DUNVIEW.C:8515-8517 relative 2,1");
    expect_int("d2r.view_square", spec->view_square_index, 5,
               "DEFS.H:2583 M605_VIEW_SQUARE_D2R");
    expect_int("d2r.native_wall", spec->native_wall_index_pc34, 7,
               "DEFS.H:3430 C07_WALL_D2R");
    expect_int("d2r.flipped_wall", spec->flipped_wall_index_pc34, 8,
               "DEFS.H:3431 C08_WALL_D2L");
    expect_int("d2r.zone", spec->wall_zone_pc34, 711,
               "DEFS.H:4051 C711_ZONE_WALL_D2R");
    expect_int("d2r.order", spec->draw_order_index, 11,
               "DUNVIEW.C:8517 F0128 D2R after D2L");
    expect_int("d2r.center_before", spec->center_field_order_index < spec->draw_order_index,
               1, "DUNVIEW.C:8508-8517 F0128 draw order");
    expect_int("d2r.raw_frame_x0", spec->raw_frame_viewport_x_first, 149,
               "DUNVIEW.C:586 G0163 D2R");
    expect_int("d2r.raw_frame_x1", spec->raw_frame_viewport_x_last, 223,
               "DUNVIEW.C:586 G0163 D2R");
    expect_int("d2r.raw_source_x", spec->raw_frame_source_x, 0,
               "DUNVIEW.C:586 G0163 D2R");
    expect_int("d2r.slice_viewport_x0", spec->viewport_x_first, 224,
               "G0163 D2R side slice viewport X 224..233");
    expect_int("d2r.slice_viewport_x1", spec->viewport_x_last, 233,
               "G0163 D2R side slice viewport X 224..233");
    expect_int("d2r.slice_source_x0", spec->source_x_first, 0,
               "G0163 D2R side slice source X 0..9");
    expect_int("d2r.slice_source_x1", spec->source_x_last, 9,
               "G0163 D2R side slice source X 0..9");
    expect_int("d2r.visible_width", spec->visible_width, 10,
               "D2R side-wall pixel slice width");
    expect_int("d2r.uses_f0100", spec->uses_f0100_frame_blit ? 1 : 0, 1,
               "DUNVIEW.C:3048-3058 F0100");
    expect_int("d2r.uses_f0105", spec->uses_f0105_party_side_flip ? 1 : 0, 1,
               "DUNVIEW.C:3185-3204 F0105 / F0128 flip pointer");
    expect_int("d2r.uses_g0699", spec->uses_g0699_wall_d2lcr_pointer ? 1 : 0, 1,
               "DUNVIEW.C:7098 G0699_puc_Bitmap_WallSet_Wall_D2LCR");
    expect_int("d2r.c10", spec->preserves_c10_transparency ? 1 : 0, 1,
               "DUNVIEW.C:3201 C10_COLOR_FLESH");
    expect_int("d2r.wall_returns", spec->wall_case_returns ? 1 : 0, 1,
               "F0122 wall-return evidence style; D2R returns after F0107");
    expect_int("d2r.f0107_probe", spec->calls_f0107_side_ornament_probe ? 1 : 0, 1,
               "DUNVIEW.C:7119-7120 F0107");
    expect_int("d2r.no_f0108", spec->calls_f0108_floor_ornament ? 1 : 0, 0,
               "F0120 wall case returns before corridor floor route");
    expect_int("d2r.no_f0111", spec->calls_f0111_door ? 1 : 0, 0,
               "F0120 door-front route is separate");
    expect_int("d2r.no_f0115", spec->calls_f0115_thing_pass ? 1 : 0, 0,
               "F0120 wall case returns before thing pass");
    expect_int("d2r.map.left",
               dm1_v1_viewport_d2l_d2r_wall_map_pixel_pc34(
                   spec, 20, 224, &source_x, &source_y) ? 1 : 0,
               1, "D2R left/top side slice");
    expect_int("d2r.map.left_source_x", source_x, 0, "G0163 D2R source X=0");
    expect_int("d2r.map.left_source_y", source_y, 0, "G0163 D2R source Y=0");
    expect_int("d2r.map.right",
               dm1_v1_viewport_d2l_d2r_wall_map_pixel_pc34(
                   spec, 90, 233, &source_x, &source_y) ? 1 : 0,
               1, "D2R right/bottom side slice");
    expect_int("d2r.map.right_source_x", source_x, 9, "G0163 D2R source X 0..9");
    expect_int("d2r.map.bottom_source_y", source_y, 70, "G0163 D2R height=71");
}

static void test_pixels_c10_and_no_write_edges(void)
{
    uint8_t source[DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D2L_D2R_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D2LD2RWallPixelResultPc34 out;
    DM1_V1_D2LD2RWallPixelInputPc34 d2l = {
        DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34,
        20,
        0,
        DM1_V1_D2L_D2R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D2LD2RWallPixelInputPc34 d2r = {
        DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34,
        20,
        224,
        DM1_V1_D2L_D2R_WALL_C10_COLOR_FLESH_PC34
    };

    memset(source, 10, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[0 * DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 + 61] = 0x21;
    source[0 * DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 + 62] = 10;
    source[0 * DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 + 71] = 0x31;
    source[70 * DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 + 61] = 0x71;
    source[0 * DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 + 0] = 0x41;
    source[0 * DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 + 9] = 0x49;
    source[70 * DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 + 9] = 0x69;

    expect_int("pixel.d2l.left.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2l, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:3048-3058 F0100");
    expect_int("pixel.d2l.left.in_clip", out.in_clip ? 1 : 0, 1,
               "G0163 D2L side slice");
    expect_int("pixel.d2l.left.source_x", out.source_x, 61,
               "G0163 D2L source X=61");
    expect_int("pixel.d2l.left.value", out.pixel_after, 0x21,
               "deterministic synthetic pixel");

    d2l.viewport_x = 1;
    expect_int("pixel.d2l.c10.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2l, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("pixel.d2l.c10.skip", out.transparent_skip ? 1 : 0, 1,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("pixel.d2l.c10.no_write", out.writes_pixel ? 1 : 0, 0,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("pixel.d2l.c10.preserved", out.pixel_after, 0xee,
               "DUNVIEW.C:3055 C10 transparent blit");

    d2l.viewport_x = 10;
    expect_int("pixel.d2l.right.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2l, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D2L side slice right edge");
    expect_int("pixel.d2l.right.source_x", out.source_x, 71,
               "G0163 D2L source X 61..71");
    expect_int("pixel.d2l.right.value", out.pixel_after, 0x31,
               "deterministic synthetic pixel");

    expect_int("pixel.d2r.left.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2r, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:7098 F0100 D2R wall");
    expect_int("pixel.d2r.left.source_x", out.source_x, 0,
               "G0163 D2R source X=0");
    expect_int("pixel.d2r.left.value", out.pixel_after, 0x41,
               "deterministic synthetic pixel");

    d2r.viewport_x = 233;
    expect_int("pixel.d2r.right.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2r, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D2R side slice right edge");
    expect_int("pixel.d2r.right.source_x", out.source_x, 9,
               "G0163 D2R source X 0..9");
    expect_int("pixel.d2r.right.value", out.pixel_after, 0x49,
               "deterministic synthetic pixel");

    d2r.row = 90;
    d2r.viewport_x = 233;
    expect_int("pixel.d2r.bottom_right.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2r, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D2R side slice bottom/right");
    expect_int("pixel.d2r.bottom_right.source_y", out.source_y, 70,
               "G0163 D2R height=71");
    expect_int("pixel.d2r.bottom_right.value", out.pixel_after, 0x69,
               "deterministic synthetic pixel");

    d2l.row = 90;
    d2l.viewport_x = 0;
    expect_int("pixel.d2l.bottom_left.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2l, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:587 G0163 D2L bottom-row frame");
    expect_int("pixel.d2l.bottom_left.source_y", out.source_y, 70,
               "DUNVIEW.C:587 G0163 D2L height=71");
    expect_int("pixel.d2l.bottom_left.source_x", out.source_x, 61,
               "DUNVIEW.C:587 G0163 D2L source X=61");
    expect_int("pixel.d2l.bottom_left.value", out.pixel_after, 0x71,
               "deterministic synthetic pixel");

    d2l.row = 19;
    d2l.viewport_x = 0;
    expect_int("pixel.before_top.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2l, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "G0163 D2L no-write above frame");
    expect_int("pixel.before_top.no_write", out.no_write_metadata ? 1 : 0, 1,
               "no-write metadata");
    expect_int("pixel.before_top.in_clip", out.in_clip ? 1 : 0, 0,
               "no-write metadata");

    d2l.row = 91;
    d2l.viewport_x = 0;
    expect_int("pixel.after_d2l_bottom.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2l, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:587 G0163 D2L no-write below frame");
    expect_int("pixel.after_d2l_bottom.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:587 G0163 D2L Y 20..90");
    expect_int("pixel.after_d2l_bottom.viewport_untouched",
               viewport[91 * DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 + 0], 0xee,
               "DUNVIEW.C:3055 F0100 frame-height bound");

    d2r.row = 20;
    d2r.viewport_x = 223;
    expect_int("pixel.before_d2r.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2r, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "G0163 D2R no-write before side slice");
    expect_int("pixel.before_d2r.no_write", out.no_write_metadata ? 1 : 0, 1,
               "no-write metadata");
    expect_int("pixel.before_d2r.viewport_untouched",
               viewport[20 * DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 + 223], 0xee,
               "no-write metadata");

    d2r.row = 91;
    d2r.viewport_x = 233;
    expect_int("pixel.after_d2r_bottom.apply",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &d2r, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:588 G0163 D2R no-write below frame");
    expect_int("pixel.after_d2r_bottom.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:588 G0163 D2R Y 20..90");
    expect_int("pixel.after_d2r_bottom.viewport_untouched",
               viewport[91 * DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 + 233], 0xee,
               "DUNVIEW.C:3055 F0100 frame-height bound");
}

static void test_party_side_flip_pixel_columns(void)
{
    const DM1_V1_D2LD2RWallSpecPc34 *d2l =
        dm1_v1_viewport_d2l_d2r_wall_spec_pc34(DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    const DM1_V1_D2LD2RWallSpecPc34 *d2r =
        dm1_v1_viewport_d2l_d2r_wall_spec_pc34(DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34);
    int source_x = -1;
    int source_y = -1;

    expect_int("flip.d2l.left.native",
               dm1_v1_viewport_d2l_d2r_wall_map_party_side_pixel_pc34(
                   d2l, 20, 0, false, &source_x, &source_y) ? 1 : 0,
               1, "DUNVIEW.C:581-591 G0163 D2L");
    expect_int("flip.d2l.left.native_source_x", source_x, 61,
               "G0163 D2L source X=61");
    expect_int("flip.d2l.left.flipped",
               dm1_v1_viewport_d2l_d2r_wall_map_party_side_pixel_pc34(
                   d2l, 20, 0, true, &source_x, &source_y) ? 1 : 0,
               1, "DUNVIEW.C:3185-3204 F0105 row flip");
    expect_int("flip.d2l.left.flipped_source_x", source_x, 10,
               "72 - 1 - 61");
    expect_int("flip.d2l.right.flipped",
               dm1_v1_viewport_d2l_d2r_wall_map_party_side_pixel_pc34(
                   d2l, 90, 10, true, &source_x, &source_y) ? 1 : 0,
               1, "DUNVIEW.C:8390-8555 F0128 flipped D2L route");
    expect_int("flip.d2l.right.flipped_source_x", source_x, 0,
               "72 - 1 - 71");
    expect_int("flip.d2l.right.flipped_source_y", source_y, 70,
               "G0163 D2L height=71");

    expect_int("flip.d2r.left.native",
               dm1_v1_viewport_d2l_d2r_wall_map_party_side_pixel_pc34(
                   d2r, 20, 224, false, &source_x, &source_y) ? 1 : 0,
               1, "DUNVIEW.C:581-591 G0163 D2R");
    expect_int("flip.d2r.left.native_source_x", source_x, 0,
               "G0163 D2R source X=0");
    expect_int("flip.d2r.left.flipped",
               dm1_v1_viewport_d2l_d2r_wall_map_party_side_pixel_pc34(
                   d2r, 20, 224, true, &source_x, &source_y) ? 1 : 0,
               1, "DUNVIEW.C:3185-3204 F0105 row flip");
    expect_int("flip.d2r.left.flipped_source_x", source_x, 71,
               "72 - 1 - 0");
    expect_int("flip.d2r.right.flipped",
               dm1_v1_viewport_d2l_d2r_wall_map_party_side_pixel_pc34(
                   d2r, 90, 233, true, &source_x, &source_y) ? 1 : 0,
               1, "DUNVIEW.C:8390-8555 F0128 flipped D2R route");
    expect_int("flip.d2r.right.flipped_source_x", source_x, 62,
               "72 - 1 - 9");
    expect_int("flip.d2r.right.flipped_source_y", source_y, 70,
               "G0163 D2R height=71");

    expect_int("flip.outside_clip",
               dm1_v1_viewport_d2l_d2r_wall_map_party_side_pixel_pc34(
                   d2r, 20, 223, true, &source_x, &source_y) ? 1 : 0,
               0, "D2R side slice begins at viewport X=224");
    expect_int("flip.null_source_x",
               dm1_v1_viewport_d2l_d2r_wall_map_party_side_pixel_pc34(
                   d2l, 20, 0, true, NULL, &source_y) ? 1 : 0,
               0, "flip mapper requires output coordinates");
}

static void test_invalid_inputs_and_blend(void)
{
    DM1_V1_D2LD2RWallPixelInputPc34 input = {
        DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34,
        20,
        0,
        DM1_V1_D2L_D2R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D2LD2RWallPixelResultPc34 out;
    int source_x = -1;
    int source_y = -1;

    expect_int("invalid.bad_side",
               dm1_v1_viewport_d2l_d2r_wall_spec_pc34(
                   (DM1_V1_D2LD2RWallSidePc34)99) != NULL ? 1 : 0,
               0, "route enum guard");
    expect_int("invalid.null_out",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &input, NULL, 0, NULL, 0, NULL) ? 1 : 0,
               0, "contract rejects null output");
    expect_int("invalid.null_input",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   NULL, NULL, 0, NULL, 0, &out) ? 1 : 0,
               0, "contract rejects null input");
    expect_int("invalid.map_null_spec",
               dm1_v1_viewport_d2l_d2r_wall_map_pixel_pc34(
                   NULL, 20, 0, &source_x, &source_y) ? 1 : 0,
               0, "map rejects null spec");
    expect_int("invalid.map_null_source_x",
               dm1_v1_viewport_d2l_d2r_wall_map_pixel_pc34(
                   &out.spec, 20, 0, NULL, &source_y) ? 1 : 0,
               0, "map rejects null source_x");
    expect_int("blend.c10",
               dm1_v1_viewport_d2l_d2r_wall_blend_pixel_pc34(0x44, 10, 10),
               0x44, "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("blend.opaque",
               dm1_v1_viewport_d2l_d2r_wall_blend_pixel_pc34(0x44, 0x52, 10),
               0x52, "DUNVIEW.C:3055 F0100 opaque pixel writes");
    expect_int("invalid.null_source_in_clip",
               dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
                   &input, NULL, 0, NULL, 0, &out) ? 1 : 0,
               0, "in-clip pixel needs buffers");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const DM1_V1_D2LD2RWallSpecPc34 *spec =
        dm1_v1_viewport_d2l_d2r_wall_spec_pc34(DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    const char *e = dm1_v1_viewport_d2l_d2r_wall_source_evidence_pc34();

    expect_int("evidence.pointer", spec->source_lines == e ? 1 : 0, 1,
               "source evidence pointer");
    expect_contains("evidence.contract", e, "contract_only=1", "source evidence");
    expect_contains("evidence.no_asset", e, "no_asset_parity=1", "source evidence");
    expect_contains("evidence.f0119", e, "DUNVIEW.C:6900 F0119",
                    "DUNVIEW.C F0119 D2L wall");
    expect_contains("evidence.f0120", e, "DUNVIEW.C:7051 F0120",
                    "DUNVIEW.C F0120 D2R wall");
    expect_contains("evidence.f0100", e, "DUNVIEW.C:3048-3058 F0100",
                    "DUNVIEW.C F0100 wall bitmap blit");
    expect_contains("evidence.f0105", e, "DUNVIEW.C:3185-3204 F0105",
                    "DUNVIEW.C F0105 row flip parity");
    expect_contains("evidence.f0128", e, "DUNVIEW.C:8318-8555 F0128",
                    "DUNVIEW.C F0128 draw order");
    expect_contains("evidence.m604", e, "M604_VIEW_SQUARE_D2L",
                    "G0163 frame ordinal");
    expect_contains("evidence.m605", e, "M605_VIEW_SQUARE_D2R",
                    "G0163 frame ordinal");
    expect_contains("evidence.g0699", e, "G0699_puc_Bitmap_WallSet_Wall_D2LCR",
                    "DUNVIEW.C:6947/7098");
    expect_contains("evidence.d2l_clip", e, "D2L viewport X 0..10/source X 61..71",
                    "G0163 D2L clip math");
    expect_contains("evidence.d2r_clip", e, "D2R viewport X 224..233/source X 0..9",
                    "G0163 D2R clip math");
    expect_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                    "DEFS.H C10");
    expect_contains("evidence.wall_indices", e, "DEFS.H:3430-3431",
                    "DEFS.H C07/C08");
    expect_contains("evidence.zones", e, "DEFS.H:4050-4051",
                    "DEFS.H C710/C711");
    expect_contains("evidence.return_style", e, "F0122-style wall-return evidence",
                    "do not over-claim");
}

int main(void)
{
    test_d2l_spec_source_locked_metadata();
    test_d2r_spec_source_locked_metadata();
    test_pixels_c10_and_no_write_edges();
    test_party_side_flip_pixel_columns();
    test_invalid_inputs_and_blend();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d2l_d2r_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2l_d2r_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
