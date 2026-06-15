#include "dm1_v1_viewport_d1c_wall_pc34_compat.h"

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

static void test_spec_source_locked_metadata(void)
{
    const DM1_V1_D1CWallSpecPc34 *spec =
        dm1_v1_viewport_d1c_wall_spec_pc34();

    expect_int("spec.contract_only", spec->contract_only ? 1 : 0, 1,
               "DUNVIEW.C:7727-7843 F0124 D1C wall route");
    expect_int("spec.no_real_asset_bitmap_parity",
               spec->real_asset_bitmap_parity ? 1 : 0, 0,
               "contract gate only, no real-asset bitmap parity claim");
    expect_int("spec.view_square_m606", spec->view_square_index, 6,
               "DEFS.H:2599 M606_VIEW_SQUARE_D1C=6 in MEDIA720");
    expect_int("spec.wall_index_c04", spec->wall_index_pc34, 4,
               "DEFS.H:3427 C04_WALL_D1C=4");
    expect_int("spec.zone_c712", spec->wall_zone_pc34, 712,
               "DEFS.H:4052 C712_ZONE_WALL_D1C=712 in MEDIA720");
    expect_int("spec.native_wall_13", spec->native_wall_bitmap_index, 13,
               "DUNVIEW.C:142 G3013_i_WallSet_Wall_D1C=-13");
    expect_int("spec.flipped_wall_24", spec->flipped_wall_bitmap_index, 24,
               "DUNVIEW.C:165 G3055_i_WallSetFlipped_Wall_D1C=-24");
    expect_int("spec.view_wall_m587_14", spec->view_wall_d1c_front_index, 14,
               "DEFS.H:2710 M587_VIEW_WALL_D1C_FRONT=14 in MEDIA720");
    expect_int("spec.frame_index", spec->frame_index, 6,
               "DUNVIEW.C:589 G0163[M606_VIEW_SQUARE_D1C]");
    expect_int("spec.frame_table_byte_width", spec->frame_table_byte_width, 128,
               "DUNVIEW.C:589 G0163 D1C raw frame byte width");
    expect_int("spec.byte_width", spec->byte_width, 80,
               "D1C center wall pixel slice gate byte_width=80");
    expect_int("spec.height", spec->height, 111,
               "DUNVIEW.C:589 G0163 D1C height=111");
    expect_int("spec.transparent_c10", spec->transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("spec.source_x_first", spec->source_x_first, 48,
               "DUNVIEW.C:589 G0163 D1C source X=48");
    expect_int("spec.source_x_last", spec->source_x_last, 127,
               "D1C clipped source X 48..127");
    expect_int("spec.viewport_x_first", spec->viewport_x_first, 32,
               "DUNVIEW.C:589 G0163 D1C viewport X1=32");
    expect_int("spec.viewport_x_last", spec->viewport_x_last, 111,
               "D1C clipped viewport X 32..111");
    expect_int("spec.viewport_y_first", spec->viewport_y_first, 9,
               "DUNVIEW.C:589 G0163 D1C viewport Y1=9");
    expect_int("spec.viewport_y_last", spec->viewport_y_last, 119,
               "DUNVIEW.C:589 G0163 D1C viewport Y2=119");
    expect_int("spec.source_width", spec->source_width, 128,
               "D1C source X 48..127 fits inside source width 128");
    expect_int("spec.source_height", spec->source_height, 111,
               "DUNVIEW.C:589 G0163 D1C height=111");
    expect_int("spec.visible_width", spec->visible_width, 80,
               "D1C visible width = byte_width - source X");
    expect_int("spec.visible_height", spec->visible_height, 111,
               "D1C visible height = frame height");
    expect_int("spec.uses_f0100", spec->uses_f0100_wallset_c10_transparent_blit ? 1 : 0, 1,
               "DUNVIEW.C:3048-3058 F0100 C10 transparent frame blit");
    expect_int("spec.uses_f0076_flip", spec->uses_f0076_flipped_wall_footprints_bitmap ? 1 : 0, 1,
               "DUNVIEW.C:2417 F1000_/G0076 flipped wall/footprints bitmap");
    expect_int("spec.uses_f0104", spec->uses_f0104_pc34_native_zone_route ? 1 : 0, 1,
               "DUNVIEW.C:3113-3129 F0104 PC34 native bitmap route");
    expect_int("spec.uses_f0765", spec->uses_f0765_pc34_opaque_center_path ? 1 : 0, 1,
               "DUNVIEW.C:7802-7807 F0765 PC34 opaque center-wall path");
    expect_int("spec.uses_f0792", spec->uses_f0792_pc34_third_party_path ? 1 : 0, 1,
               "DUNVIEW.C:7792-7801 F0792 PC34 wall zone blit");
    expect_int("spec.preserves_c10", spec->preserves_c10_transparency ? 1 : 0, 1,
               "DUNVIEW.C:3055 F0100 C10 transparent blit");
    expect_int("spec.wall_returns", spec->wall_case_returns ? 1 : 0, 1,
               "DUNVIEW.C:7843 F0124 wall case returns");
    expect_int("spec.f0107_probe", spec->calls_f0107_front_alcove_probe ? 1 : 0, 1,
               "DUNVIEW.C:7810 F0107 front wall ornament alcove probe");
    expect_int("spec.no_f0108", spec->calls_f0108_floor_ornament ? 1 : 0, 0,
               "DUNVIEW.C:7815-7872 F0108 floor route is in pit/floor case");
    expect_int("spec.no_f0111", spec->calls_f0111_door ? 1 : 0, 0,
               "DUNVIEW.C:7817+ F0111 is in door-front case");
    expect_int("spec.no_f0113", spec->calls_f0113_center_field ? 1 : 0, 0,
               "D1C center field integration is out of scope");
    expect_int("spec.f0115_alcove", spec->calls_f0115_thing_pass ? 1 : 0, 1,
               "DUNVIEW.C:7813-7843 F0115 only on alcove probe true");
    expect_int("spec.flipped_in_alcove", spec->flipped_route_draws_in_alcove_thing_pass ? 1 : 0, 1,
               "DUNVIEW.C:7813 F0115_CELL_ORDER_ALCOVE includes F0115 entry");
}

static void test_deterministic_capture_pixels_and_edges(void)
{
    uint8_t source[DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D1C_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D1C_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D1C_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D1CWallPixelResultPc34 out;
    DM1_V1_D1CWallPixelInputPc34 input = {
        30,
        32,
        false,
        false,
        DM1_V1_D1C_WALL_C10_COLOR_FLESH_PC34
    };

    memset(source, 10, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[21 * DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34 + 48] = 0x24;
    source[21 * DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34 + 49] = 10;
    source[21 * DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34 + 126] = 0x62;
    source[21 * DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34 + 127] = 0x7f;

    expect_int("pixel.left.apply",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:3048-3058 F0100 left clipped edge");
    expect_int("pixel.left.in_clip", out.in_clip ? 1 : 0, 1,
               "DUNVIEW.C:589 G0163 D1C clip");
    expect_int("pixel.left.source_x", out.source_x, 48,
               "DUNVIEW.C:589 G0163 D1C source X=48");
    expect_int("pixel.left.source_y", out.source_y, 21,
               "DUNVIEW.C:589 G0163 D1C height clip");
    expect_int("pixel.left.writes", out.writes_pixel ? 1 : 0, 1,
               "DUNVIEW.C:3055 F0100 C10 transparent blit");
    expect_int("pixel.left.value", out.pixel_after, 0x24,
               "deterministic in-test capture, not real asset runtime");

    input.viewport_x = 33;
    expect_int("pixel.c10.apply",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("pixel.c10.skip", out.transparent_skip ? 1 : 0, 1,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("pixel.c10.no_write", out.writes_pixel ? 1 : 0, 0,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("pixel.c10.preserved", out.pixel_after, 0xee,
               "DUNVIEW.C:3055 C10 transparent blit");

    input.use_no_transparency = true;
    expect_int("pixel.opaque_c10.apply",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:3288-3301 F0792 CM1_COLOR_NO_TRANSPARENCY");
    expect_int("pixel.opaque_c10.route", out.no_transparency_route ? 1 : 0, 1,
               "F0792/F0765 PC34 zone routes use CM1_COLOR_NO_TRANSPARENCY");
    expect_int("pixel.opaque_c10.no_skip", out.transparent_skip ? 1 : 0, 0,
               "CM1_COLOR_NO_TRANSPARENCY writes C10 as an ordinary pixel");
    expect_int("pixel.opaque_c10.writes", out.writes_pixel ? 1 : 0, 1,
               "CM1_COLOR_NO_TRANSPARENCY writes C10 as an ordinary pixel");
    expect_int("pixel.opaque_c10.value", out.pixel_after, 10,
               "DUNVIEW.C:3159-3175 F0765/F0792 opaque zone blit");
    input.use_no_transparency = false;

    input.viewport_x = 110;
    expect_int("pixel.neighbor.apply",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D1C neighboring pixel before clipped edge");
    expect_int("pixel.neighbor.source_x", out.source_x, 126,
               "D1C clipped source X 48..127");
    expect_int("pixel.neighbor.value", out.pixel_after, 0x62,
               "deterministic in-test capture, not real asset runtime");

    input.viewport_x = 111;
    expect_int("pixel.right.apply",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D1C clipped right edge");
    expect_int("pixel.right.source_x", out.source_x, 127,
               "D1C clipped source X 48..127");
    expect_int("pixel.right.value", out.pixel_after, 0x7f,
               "deterministic in-test capture, not real asset runtime");

    input.viewport_x = 112;
    expect_int("pixel.after_edge.apply",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D1C no-write after clipped edge");
    expect_int("pixel.after_edge.no_write", out.no_write_metadata ? 1 : 0, 1,
               "no-write metadata");
    expect_int("pixel.after_edge.in_clip", out.in_clip ? 1 : 0, 0,
               "no-write metadata");
    expect_int("pixel.after_edge.viewport_untouched",
               viewport[30 * DM1_V1_D1C_WALL_VIEWPORT_WIDTH_PC34 + 112], 0xee,
               "no-write metadata");

    input.row = 120;
    input.viewport_x = 32;
    expect_int("pixel.after_bottom.apply",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D1C no-write after bottom edge");
    expect_int("pixel.after_bottom.no_write", out.no_write_metadata ? 1 : 0, 1,
               "no-write metadata");

    input.row = 30;
    input.viewport_x = 32;
    input.use_flipped_wall_bitmap = true;
    input.use_no_transparency = false;
    source[21 * DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34 + 79] = 10;
    source[21 * DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34 + 78] = 0x53;
    source[21 * DM1_V1_D1C_WALL_SOURCE_WIDTH_PC34 + 0] = 0x68;
    expect_int("pixel.flipped.apply",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:2417 G3055 flipped route");
    expect_int("pixel.flipped.route", out.route_is_flipped ? 1 : 0, 1,
               "DUNVIEW.C:2417 G3055 flipped route");
    expect_int("pixel.flipped.selected_x", out.selected_source_x, 79,
               "G3055 flipped source X = 127 - source_x");
    expect_int("pixel.flipped.skip", out.transparent_skip ? 1 : 0, 1,
               "flipped C10 skip maps to D1C left pixel");
    input.viewport_x = 33;
    expect_int("pixel.flipped.next.apply",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:2417 G3055 flipped route");
    expect_int("pixel.flipped.next.value", out.pixel_after, 0x53,
               "flipped D1C source x=78 maps to D1C next pixel");
    input.viewport_x = 111;
    expect_int("pixel.flipped.edge.apply",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:2417 G3055 flipped route");
    expect_int("pixel.flipped.edge.value", out.pixel_after, 0x68,
               "flipped D1C source x=0 maps to D1C right edge");
}

static void test_invalid_inputs_and_blend(void)
{
    DM1_V1_D1CWallPixelResultPc34 out;
    DM1_V1_D1CWallPixelInputPc34 input = {
        9,
        32,
        false,
        false,
        DM1_V1_D1C_WALL_C10_COLOR_FLESH_PC34
    };

    expect_int("invalid.null_out",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(&input, NULL, 0, NULL, 0, NULL) ? 1 : 0,
               0, "contract rejects null output");
    expect_int("invalid.null_input",
               dm1_v1_viewport_d1c_wall_apply_pixel_pc34(NULL, NULL, 0, NULL, 0, &out) ? 1 : 0,
               0, "contract rejects null input");
    expect_int("blend.c10",
               dm1_v1_viewport_d1c_wall_blend_pixel_pc34(0x44, 10, 10), 0x44,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("blend.opaque",
               dm1_v1_viewport_d1c_wall_blend_pixel_pc34(0x44, 0x55, 10), 0x55,
               "DUNVIEW.C:3055 F0100 opaque pixel writes");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const DM1_V1_D1CWallSpecPc34 *spec =
        dm1_v1_viewport_d1c_wall_spec_pc34();
    const char *e = dm1_v1_viewport_d1c_wall_source_evidence_pc34();

    expect_int("evidence.pointer", spec->source_lines == e ? 1 : 0, 1,
               "source evidence pointer");
    expect_contains("evidence.contract_only", e, "contract_only=1",
                    "source-locked evidence block");
    expect_contains("evidence.no_asset_parity", e, "no_asset_parity=1",
                    "source-locked evidence block");
    expect_contains("evidence.f0100", e, "DUNVIEW.C:3048-3058 F0100",
                    "DUNVIEW.C C10 frame blit");
    expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3129 F0104",
                    "DUNVIEW.C PC34 native route");
    expect_contains("evidence.g0163", e, "DUNVIEW.C:581-594 G0163",
                    "DUNVIEW.C frame table");
    expect_contains("evidence.f0124", e, "DUNVIEW.C:7727-7843 F0124",
                    "DUNVIEW.C D1C route");
    expect_contains("evidence.f0765", e, "DUNVIEW.C:7802-7807 F0765",
                    "DUNVIEW.C PC34 opaque center wall path");
    expect_contains("evidence.f0792", e, "DUNVIEW.C:7792-7801 F0792",
                    "DUNVIEW.C PC34 wall zone blit");
    expect_contains("evidence.f0792_opaque", e, "DUNVIEW.C:3288-3301 F0792 passes",
                    "DUNVIEW.C F0792 no-transparency route");
    expect_contains("evidence.g0076_flip", e, "G0076_B_UseFlippedWallAndFootprintsBitmaps",
                    "DUNVIEW.C flipped wall/footprints bit");
    expect_contains("evidence.g3055", e, "G3055_i_WallSetFlipped_Wall_D1C=-24",
                    "DUNVIEW.C flipped wall ordinal");
    expect_contains("evidence.frame_ref", e,
                    "G0163_aauc_Graphic558_Frame_Walls[M606_VIEW_SQUARE_D1C]",
                    "DUNVIEW.C D1C frame reference");
    expect_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                    "DEFS.H color constant");
    expect_contains("evidence.m606", e, "DEFS.H:2599 M606_VIEW_SQUARE_D1C=6",
                    "DEFS.H D1C view square constant");
    expect_contains("evidence.m587", e, "M587_VIEW_WALL_D1C_FRONT=14",
                    "DEFS.H D1C front wall view index");
    expect_contains("evidence.c04", e, "DEFS.H:3427 C04_WALL_D1C=4",
                    "DEFS.H D1C wall ordinal");
    expect_contains("evidence.c712", e, "DEFS.H:4052 C712_ZONE_WALL_D1C=712",
                    "DEFS.H D1C zone constant");
    expect_contains("evidence.c710_legacy", e, "C710_ZONE_WALL_D1C",
                    "DEFS.H legacy D1C zone");
    expect_contains("evidence.clip_rect", e,
                    "byte_width=80, height=111, source X 48..127, viewport X 32..111",
                    "required D1C pixel slice clip rectangle");
    expect_contains("evidence.f0107_alcove", e,
                    "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
                    "DUNVIEW.C front wall ornament alcove probe");
    expect_contains("evidence.f0115_alcove", e,
                    "F0115 only follows the explicit alcove path",
                    "alcove thing pass exception");
    expect_contains("evidence.c10_opaque_zone", e,
                    "C10 writes on these zone routes",
                    "F0792/F0765 no-transparency pixel gate");
    expect_contains("evidence.no_f0108", e, "F0108 floor ornaments",
                    "non-overlap with floor ornament gate");
    expect_contains("evidence.no_field", e, "F0113 center-field integration",
                    "non-overlap with broad integration");
}

int main(void)
{
    test_spec_source_locked_metadata();
    test_deterministic_capture_pixels_and_edges();
    test_invalid_inputs_and_blend();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d1c_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d1c_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
