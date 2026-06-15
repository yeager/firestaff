#include "dm1_v1_viewport_d1l2_wall_pc34_compat.h"

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
    const DM1_V1_D1L2WallSpecPc34 *spec =
        dm1_v1_viewport_d1l2_wall_spec_pc34();

    expect_int("spec.contract_only", spec->contract_only ? 1 : 0, 1,
               "DUNVIEW.C:7436-7460 F0122 D1L wall route");
    expect_int("spec.no_real_asset_bitmap_parity",
               spec->real_asset_bitmap_parity ? 1 : 0, 0,
               "DUNVIEW.C:3048-3058 F0100 C10 contract only");
    expect_int("spec.view_square_m607", spec->view_square_index, 4,
               "DEFS.H:2596-2601 M607_VIEW_SQUARE_D1L=4");
    expect_int("spec.wall_index_c03", spec->wall_index_pc34, 3,
               "DEFS.H:3423-3427 C03_WALL_D1L=3");
    expect_int("spec.zone_c713", spec->wall_zone_pc34, 713,
               "DEFS.H:4052-4054 C713_ZONE_WALL_D1L");
    expect_int("spec.frame_index", spec->frame_index, 4,
               "DUNVIEW.C:7436-7460 G0163[M607_VIEW_SQUARE_D1L]");
    expect_int("spec.frame_bitmap_g_index", spec->frame_bitmap_index, 0,
               "DUNVIEW.C:7436-7460 G0700 D1LCR wall bitmap");
    expect_int("spec.transparent_c10", spec->transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("spec.field_mask", spec->field_mask, 0x82,
               "DUNVIEW.C:718-731 G0188 D1L field aspect");
    expect_int("spec.byte_width", spec->byte_width, 32,
               "DUNVIEW.C:727 G0188 D1L byte width");
    expect_int("spec.height", spec->height, 111,
               "DUNVIEW.C:727 G0188 D1L height");
    expect_int("spec.clip_left", spec->viewport_x_first, 0,
               "DUNVIEW.C:727 G0188 D1L X=0");
    expect_int("spec.clip_right", spec->viewport_x_last, 63,
               "DUNVIEW.C:727 G0188 D1L byte_width=32");
    expect_int("spec.clip_top", spec->viewport_y_first, 0,
               "DUNVIEW.C:727 G0188 D1L frame top");
    expect_int("spec.clip_bottom", spec->viewport_y_last, 110,
               "DUNVIEW.C:727 G0188 D1L height=111");
    expect_int("spec.uses_f0100", spec->uses_f0100_frame_blit ? 1 : 0, 1,
               "DUNVIEW.C:3048-3058 F0100 frame bitmap clip path");
    expect_int("spec.uses_f0104", spec->uses_f0104_pc34_zone_blit ? 1 : 0, 1,
               "DUNVIEW.C:3113-3129 F0104 PC34 zone clip path");
    expect_int("spec.no_f0105", spec->uses_f0105_flipped_blit ? 1 : 0, 0,
               "DUNVIEW.C:7436-7460 F0122 D1L native/non-flipped route");
    expect_int("spec.wall_returns", spec->wall_case_returns ? 1 : 0, 1,
               "DUNVIEW.C:7460 F0122 wall case returns");
    expect_int("spec.f0107_probe", spec->calls_f0107_side_ornament_probe ? 1 : 0, 1,
               "DUNVIEW.C:7459 F0107 side ornament probe");
    expect_int("spec.no_f0108", spec->calls_f0108_floor_ornament ? 1 : 0, 0,
               "DUNVIEW.C:7520-7525 separate corridor floor route");
    expect_int("spec.no_f0111", spec->calls_f0111_door ? 1 : 0, 0,
               "DUNVIEW.C:7492-7507 separate door route");
    expect_int("spec.no_f0115", spec->calls_f0115_thing_pass ? 1 : 0, 0,
               "DUNVIEW.C:7436-7460 wall case returns before object pass");
}

static void test_deterministic_capture_pixels_and_edges(void)
{
    uint8_t source[DM1_V1_D1L2_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D1L2_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D1L2_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D1L2_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D1L2WallPixelResultPc34 out;
    DM1_V1_D1L2WallPixelInputPc34 input = {
        7,
        0,
        DM1_V1_D1L2_WALL_C10_COLOR_FLESH_PC34
    };

    memset(source, 10, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[7 * DM1_V1_D1L2_WALL_SOURCE_WIDTH_PC34 + 0] = 0x24;
    source[7 * DM1_V1_D1L2_WALL_SOURCE_WIDTH_PC34 + 1] = 10;
    source[7 * DM1_V1_D1L2_WALL_SOURCE_WIDTH_PC34 + 62] = 0x62;
    source[7 * DM1_V1_D1L2_WALL_SOURCE_WIDTH_PC34 + 63] = 0x7f;

    expect_int("pixel.left.apply",
               dm1_v1_viewport_d1l2_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:3048-3058 F0100 left clipped edge");
    expect_int("pixel.left.in_clip", out.in_clip ? 1 : 0, 1,
               "DUNVIEW.C:727 G0188 D1L clip");
    expect_int("pixel.left.source_x", out.source_x, 0,
               "DUNVIEW.C:727 G0188 D1L X=0");
    expect_int("pixel.left.source_y", out.source_y, 7,
               "DUNVIEW.C:727 G0188 D1L height clip");
    expect_int("pixel.left.writes", out.writes_pixel ? 1 : 0, 1,
               "DUNVIEW.C:3055 F0100 C10 transparent blit");
    expect_int("pixel.left.value", out.pixel_after, 0x24,
               "deterministic in-test capture, not real asset runtime");

    input.viewport_x = 1;
    expect_int("pixel.c10.apply",
               dm1_v1_viewport_d1l2_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("pixel.c10.skip", out.transparent_skip ? 1 : 0, 1,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("pixel.c10.no_write", out.writes_pixel ? 1 : 0, 0,
               "DUNVIEW.C:3055 C10 transparent blit");
    expect_int("pixel.c10.preserved", out.pixel_after, 0xee,
               "DUNVIEW.C:3055 C10 transparent blit");

    input.viewport_x = 62;
    expect_int("pixel.neighbor.apply",
               dm1_v1_viewport_d1l2_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:727 neighboring pixel before clipped edge");
    expect_int("pixel.neighbor.source_x", out.source_x, 62,
               "DUNVIEW.C:727 G0188 D1L byte_width=32");
    expect_int("pixel.neighbor.value", out.pixel_after, 0x62,
               "deterministic in-test capture, not real asset runtime");

    input.viewport_x = 63;
    expect_int("pixel.right.apply",
               dm1_v1_viewport_d1l2_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:727 clipped right edge");
    expect_int("pixel.right.source_x", out.source_x, 63,
               "DUNVIEW.C:727 G0188 D1L byte_width=32");
    expect_int("pixel.right.value", out.pixel_after, 0x7f,
               "deterministic in-test capture, not real asset runtime");

    input.viewport_x = 64;
    expect_int("pixel.after_edge.apply",
               dm1_v1_viewport_d1l2_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:727 no-write after D1L clipped edge");
    expect_int("pixel.after_edge.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:727 no-write metadata");
    expect_int("pixel.after_edge.in_clip", out.in_clip ? 1 : 0, 0,
               "DUNVIEW.C:727 no-write metadata");
    expect_int("pixel.after_edge.viewport_untouched",
               viewport[7 * DM1_V1_D1L2_WALL_VIEWPORT_WIDTH_PC34 + 64], 0xee,
               "DUNVIEW.C:727 no-write metadata");

    input.row = 111;
    input.viewport_x = 0;
    expect_int("pixel.after_bottom.apply",
               dm1_v1_viewport_d1l2_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:727 no-write after D1L bottom edge");
    expect_int("pixel.after_bottom.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:727 no-write metadata");
}

static void test_invalid_inputs_and_blend(void)
{
    DM1_V1_D1L2WallPixelResultPc34 out;
    DM1_V1_D1L2WallPixelInputPc34 input = {
        0,
        0,
        DM1_V1_D1L2_WALL_C10_COLOR_FLESH_PC34
    };

    expect_int("invalid.null_out",
               dm1_v1_viewport_d1l2_wall_apply_pixel_pc34(&input, NULL, 0, NULL, 0, NULL) ? 1 : 0,
               0, "contract rejects null output");
    expect_int("invalid.null_input",
               dm1_v1_viewport_d1l2_wall_apply_pixel_pc34(NULL, NULL, 0, NULL, 0, &out) ? 1 : 0,
               0, "contract rejects null input");
    expect_int("blend.c10",
               dm1_v1_viewport_d1l2_wall_blend_pixel_pc34(0x44, 10, 10), 0x44,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("blend.opaque",
               dm1_v1_viewport_d1l2_wall_blend_pixel_pc34(0x44, 0x55, 10), 0x55,
               "DUNVIEW.C:3055 F0100 opaque pixel writes");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const DM1_V1_D1L2WallSpecPc34 *spec =
        dm1_v1_viewport_d1l2_wall_spec_pc34();
    const char *e = dm1_v1_viewport_d1l2_wall_source_evidence_pc34();

    expect_int("evidence.pointer", spec->source_lines == e ? 1 : 0, 1,
               "source evidence pointer");
    expect_contains("evidence.contract_only", e, "contract_only=1",
                    "source-locked evidence block");
    expect_contains("evidence.no_asset_parity", e, "no_asset_parity=1",
                    "source-locked evidence block");
    expect_contains("evidence.no_literal_d1l2", e, "No literal D1L2 wall entry",
                    "DUNVIEW.C search fallback note");
    expect_contains("evidence.f0122", e, "DUNVIEW.C:7436-7460 F0122",
                    "DUNVIEW.C F0122 wall route");
    expect_contains("evidence.g0188", e, "DUNVIEW.C:718-731 G0188",
                    "DUNVIEW.C field aspect table");
    expect_contains("evidence.c10_constant", e, "DEFS.H:2088 C10_COLOR_FLESH",
                    "DEFS.H color constant");
    expect_contains("evidence.frame_bitmap", e, "G0700_puc_Bitmap_WallSet_Wall_D1LCR",
                    "DUNVIEW.C frame bitmap route");
    expect_contains("evidence.frame_clip", e, "G0163_aauc_Graphic558_Frame_Walls[M607_VIEW_SQUARE_D1L]",
                    "DUNVIEW.C frame clip route");
    expect_contains("evidence.f0100", e, "DUNVIEW.C:3048-3058 F0100",
                    "DUNVIEW.C C10 frame blit");
    expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3129 F0104",
                    "DUNVIEW.C PC34 native bitmap route");
    expect_contains("evidence.g_frame_index", e, "NativeBitmapRelativeIndex=0",
                    "DUNVIEW.C G-frame bitmap index");
    expect_contains("evidence.clip_rect", e, "byte_width=32, height=111",
                    "DUNVIEW.C clip rectangle");
    expect_contains("evidence.zone", e, "C713_ZONE_WALL_D1L",
                    "DEFS.H zone constant");
    expect_contains("evidence.no_f0108", e, "separate F0108 floor-ornament path",
                    "non-overlap with floor ornament gate");
    expect_contains("evidence.non_overlap", e,
                    "non-overlap: D1L2 floor ornament gate covers the D1L2 floor route",
                    "required non-overlap note");
}

int main(void)
{
    test_spec_source_locked_metadata();
    test_deterministic_capture_pixels_and_edges();
    test_invalid_inputs_and_blend();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d1l2_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d1l2_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
