#include "dm1_v1_viewport_d2c_wall_pc34_compat.h"

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
    const DM1_V1_D2CWallSpecPc34 *spec =
        dm1_v1_viewport_d2c_wall_spec_pc34();

    expect_int("spec.contract_only", spec->contract_only ? 1 : 0, 1,
               "DUNVIEW.C:7244-7312 F0121 D2C wall route");
    expect_int("spec.no_real_asset_bitmap_parity",
               spec->real_asset_bitmap_parity ? 1 : 0, 0,
               "contract gate only, no real-asset bitmap parity claim");
    expect_int("spec.view_square_m603", spec->view_square_index, 6,
               "DEFS.H:2602 M603_VIEW_SQUARE_D2C=6");
    expect_int("spec.wall_index_c09", spec->wall_index_pc34, 9,
               "DEFS.H:3432 C09_WALL_D2C=9");
    expect_int("spec.zone_c709", spec->wall_zone_pc34, 709,
               "DEFS.H:4049 C709_ZONE_WALL_D2C");
    expect_int("spec.frame_index", spec->frame_index, 6,
               "DUNVIEW.C:7291 G0163[M603_VIEW_SQUARE_D2C]");
    expect_int("spec.frame_table_byte_width", spec->frame_table_byte_width, 72,
               "DUNVIEW.C:586 G0163 D2C raw frame byte width");
    expect_int("spec.byte_width", spec->byte_width, 32,
               "D2C center wall pixel slice gate byte_width=32");
    expect_int("spec.height", spec->height, 71,
               "DUNVIEW.C:586 G0163 D2C height=71");
    expect_int("spec.transparent_c10", spec->transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("spec.source_x_first", spec->source_x_first, 16,
               "DUNVIEW.C:586 G0163 D2C source X=16");
    expect_int("spec.source_x_last", spec->source_x_last, 71,
               "D2C clipped source X 16..71");
    expect_int("spec.viewport_x_first", spec->viewport_x_first, 60,
               "DUNVIEW.C:586 G0163 D2C viewport X1=60");
    expect_int("spec.viewport_x_last", spec->viewport_x_last, 115,
               "D2C clipped viewport X 60..115");
    expect_int("spec.viewport_y_first", spec->viewport_y_first, 20,
               "DUNVIEW.C:586 G0163 D2C viewport Y1=20");
    expect_int("spec.viewport_y_last", spec->viewport_y_last, 90,
               "DUNVIEW.C:586 G0163 D2C viewport Y2=90");
    expect_int("spec.source_width", spec->source_width, 72,
               "D2C source X 16..71 fits inside source width 72");
    expect_int("spec.source_height", spec->source_height, 71,
               "DUNVIEW.C:586 G0163 D2C height=71");
    expect_int("spec.uses_f0100", spec->uses_f0100_c10_transparent_blit ? 1 : 0, 1,
               "DUNVIEW.C:3048-3058 F0100 C10 transparent frame blit");
    expect_int("spec.uses_f0101", spec->uses_f0101_pc34_opaque_center_path ? 1 : 0, 1,
               "DUNVIEW.C:3064-3076 F0101 PC34 center-wall path");
    expect_int("spec.uses_f0104", spec->uses_f0104_pc34_native_route ? 1 : 0, 1,
               "DUNVIEW.C:3113-3129 F0104 PC34 native bitmap route");
    expect_int("spec.preserves_c10", spec->preserves_c10_transparency ? 1 : 0, 1,
               "DUNVIEW.C:3055 F0100 C10 transparent blit");
    expect_int("spec.wall_returns", spec->wall_case_returns ? 1 : 0, 1,
               "DUNVIEW.C:7312 F0121 wall case returns");
    expect_int("spec.f0107_probe", spec->calls_f0107_front_alcove_probe ? 1 : 0, 1,
               "DUNVIEW.C:7308 front wall ornament alcove probe");
    expect_int("spec.no_f0108", spec->calls_f0108_floor_ornament ? 1 : 0, 0,
               "DUNVIEW.C:7314 separate door-front floor route");
    expect_int("spec.no_f0111", spec->calls_f0111_door ? 1 : 0, 0,
               "DUNVIEW.C:7336-7339 separate door route");
    expect_int("spec.no_f0113", spec->calls_f0113_center_field ? 1 : 0, 0,
               "D2C center field integration is out of scope");
    expect_int("spec.no_f0115", spec->calls_f0115_thing_pass ? 1 : 0, 0,
               "DUNVIEW.C:7312 returns before thing pass for plain wall");
}

static void test_deterministic_capture_pixels_and_edges(void)
{
    uint8_t source[DM1_V1_D2C_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D2C_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D2C_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D2C_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D2CWallPixelResultPc34 out;
    DM1_V1_D2CWallPixelInputPc34 input = {
        27,
        60,
        DM1_V1_D2C_WALL_C10_COLOR_FLESH_PC34
    };

    memset(source, 10, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[7 * DM1_V1_D2C_WALL_SOURCE_WIDTH_PC34 + 16] = 0x24;
    source[7 * DM1_V1_D2C_WALL_SOURCE_WIDTH_PC34 + 17] = 10;
    source[7 * DM1_V1_D2C_WALL_SOURCE_WIDTH_PC34 + 70] = 0x62;
    source[7 * DM1_V1_D2C_WALL_SOURCE_WIDTH_PC34 + 71] = 0x7f;

    expect_int("pixel.left.apply",
               dm1_v1_viewport_d2c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:3048-3058 F0100 left clipped edge");
    expect_int("pixel.left.in_clip", out.in_clip ? 1 : 0, 1,
               "DUNVIEW.C:586 G0163 D2C clip");
    expect_int("pixel.left.source_x", out.source_x, 16,
               "DUNVIEW.C:586 G0163 D2C source X=16");
    expect_int("pixel.left.source_y", out.source_y, 7,
               "DUNVIEW.C:586 G0163 D2C height clip");
    expect_int("pixel.left.writes", out.writes_pixel ? 1 : 0, 1,
               "DUNVIEW.C:3055 F0100 C10 transparent blit");
    expect_int("pixel.left.value", out.pixel_after, 0x24,
               "deterministic in-test capture, not real asset runtime");

    input.viewport_x = 61;
    expect_int("pixel.c10.apply",
               dm1_v1_viewport_d2c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("pixel.c10.skip", out.transparent_skip ? 1 : 0, 1,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("pixel.c10.no_write", out.writes_pixel ? 1 : 0, 0,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("pixel.c10.preserved", out.pixel_after, 0xee,
               "DUNVIEW.C:3055 C10 transparent blit");

    input.viewport_x = 114;
    expect_int("pixel.neighbor.apply",
               dm1_v1_viewport_d2c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D2C neighboring pixel before clipped edge");
    expect_int("pixel.neighbor.source_x", out.source_x, 70,
               "D2C clipped source X 16..71");
    expect_int("pixel.neighbor.value", out.pixel_after, 0x62,
               "deterministic in-test capture, not real asset runtime");

    input.viewport_x = 115;
    expect_int("pixel.right.apply",
               dm1_v1_viewport_d2c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D2C clipped right edge");
    expect_int("pixel.right.source_x", out.source_x, 71,
               "D2C clipped source X 16..71");
    expect_int("pixel.right.value", out.pixel_after, 0x7f,
               "deterministic in-test capture, not real asset runtime");

    input.viewport_x = 116;
    expect_int("pixel.after_edge.apply",
               dm1_v1_viewport_d2c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D2C no-write after clipped edge");
    expect_int("pixel.after_edge.no_write", out.no_write_metadata ? 1 : 0, 1,
               "no-write metadata");
    expect_int("pixel.after_edge.in_clip", out.in_clip ? 1 : 0, 0,
               "no-write metadata");
    expect_int("pixel.after_edge.viewport_untouched",
               viewport[27 * DM1_V1_D2C_WALL_VIEWPORT_WIDTH_PC34 + 116], 0xee,
               "no-write metadata");

    input.row = 91;
    input.viewport_x = 60;
    expect_int("pixel.after_bottom.apply",
               dm1_v1_viewport_d2c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "D2C no-write after bottom edge");
    expect_int("pixel.after_bottom.no_write", out.no_write_metadata ? 1 : 0, 1,
               "no-write metadata");
}

static void test_invalid_inputs_and_blend(void)
{
    DM1_V1_D2CWallPixelResultPc34 out;
    DM1_V1_D2CWallPixelInputPc34 input = {
        20,
        60,
        DM1_V1_D2C_WALL_C10_COLOR_FLESH_PC34
    };

    expect_int("invalid.null_out",
               dm1_v1_viewport_d2c_wall_apply_pixel_pc34(&input, NULL, 0, NULL, 0, NULL) ? 1 : 0,
               0, "contract rejects null output");
    expect_int("invalid.null_input",
               dm1_v1_viewport_d2c_wall_apply_pixel_pc34(NULL, NULL, 0, NULL, 0, &out) ? 1 : 0,
               0, "contract rejects null input");
    expect_int("blend.c10",
               dm1_v1_viewport_d2c_wall_blend_pixel_pc34(0x44, 10, 10), 0x44,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("blend.opaque",
               dm1_v1_viewport_d2c_wall_blend_pixel_pc34(0x44, 0x55, 10), 0x55,
               "DUNVIEW.C:3055 F0100 opaque pixel writes");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const DM1_V1_D2CWallSpecPc34 *spec =
        dm1_v1_viewport_d2c_wall_spec_pc34();
    const char *e = dm1_v1_viewport_d2c_wall_source_evidence_pc34();

    expect_int("evidence.pointer", spec->source_lines == e ? 1 : 0, 1,
               "source evidence pointer");
    expect_contains("evidence.contract_only", e, "contract_only=1",
                    "source-locked evidence block");
    expect_contains("evidence.no_asset_parity", e, "no_asset_parity=1",
                    "source-locked evidence block");
    expect_contains("evidence.f0100", e, "DUNVIEW.C:3048-3058 F0100",
                    "DUNVIEW.C C10 frame blit");
    expect_contains("evidence.f0101", e, "DUNVIEW.C:3064-3076 F0101",
                    "DUNVIEW.C PC34 center wall path");
    expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3129 F0104",
                    "DUNVIEW.C PC34 native route");
    expect_contains("evidence.g0163", e, "DUNVIEW.C:581-594 G0163",
                    "DUNVIEW.C frame table");
    expect_contains("evidence.f0121", e, "DUNVIEW.C:7244-7312 F0121",
                    "DUNVIEW.C D2C route");
    expect_contains("evidence.frame_ref", e,
                    "G0163_aauc_Graphic558_Frame_Walls[M603_VIEW_SQUARE_D2C]",
                    "DUNVIEW.C D2C frame reference");
    expect_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                    "DEFS.H color constant");
    expect_contains("evidence.m603", e, "DEFS.H:2602 M603_VIEW_SQUARE_D2C=6",
                    "DEFS.H D2C constant");
    expect_contains("evidence.m607_correction", e,
                    "M607_VIEW_SQUARE_D1L=4",
                    "DEFS.H correction for requested M607 label");
    expect_contains("evidence.c09", e, "DEFS.H:3432 C09_WALL_D2C",
                    "DEFS.H wall constant");
    expect_contains("evidence.c709", e, "DEFS.H:4049 C709_ZONE_WALL_D2C",
                    "DEFS.H zone constant");
    expect_contains("evidence.coord_load", e, "COORD.C:2542-2569 F0640/F0641",
                    "COORD.C layout load");
    expect_contains("evidence.coord_clip", e, "COORD.C:2390-2409 F0635",
                    "COORD.C layout clip");
    expect_contains("evidence.clip_rect", e,
                    "byte_width=32, height=71, source X 16..71, viewport X 60..115",
                    "required D2C pixel slice clip rectangle");
    expect_contains("evidence.no_f0108", e, "excludes F0108 floor ornaments",
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
        printf("FAIL dm1_v1_viewport_d2c_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2c_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
