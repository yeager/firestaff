#include "dm1_v1_viewport_d3c_wall_pc34_compat.h"

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
        printf("FAIL %s missing \"%s\" at %s\n", id,
               needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void test_frame_resolution_and_wall_route(void)
{
    const DM1_V1_D3CWallSpecPc34 *spec =
        dm1_v1_viewport_d3c_wall_spec_pc34();

    expect_int("spec.contract_only", spec->contract_only ? 1 : 0, 1,
               "DUNVIEW.C:6642-6720 F0118, DUNVIEW.C:6699 F0100");
    expect_int("spec.no_real_asset_pixel_parity",
               spec->real_asset_pixel_parity ? 1 : 0, 0,
               "DUNVIEW.C:6642-6720 F0118 source-lock gate only");
    expect_int("spec.view_square_m600", spec->view_square_index, 0,
               "DUNVIEW.C:6699 F0118 uses M600; DEFS.H:2578 M600=0");
    expect_int("spec.frame_index_m600", spec->frame_index, 0,
               "DUNVIEW.C:581-583 G0163[M600]; DEFS.H:2578");
    expect_int("spec.frame_x1", spec->frame_x1, 74,
               "DUNVIEW.C:583 G0163 D3C X1; F0118 line 6699");
    expect_int("spec.frame_x2", spec->frame_x2, 149,
               "DUNVIEW.C:583 G0163 D3C X2; F0118 line 6699");
    expect_int("spec.frame_y1", spec->frame_y1, 25,
               "DUNVIEW.C:583 G0163 D3C Y1; F0118 line 6699");
    expect_int("spec.frame_y2", spec->frame_y2, 75,
               "DUNVIEW.C:583 G0163 D3C Y2; F0118 line 6699");
    expect_int("spec.frame_table_byte_width", spec->frame_table_byte_width, 64,
               "DUNVIEW.C:583 G0163 D3C C4; F0100 line 3055");
    expect_int("spec.frame_height", spec->frame_height, 51,
               "DUNVIEW.C:583 G0163 D3C C5; F0100 line 3058");
    expect_int("spec.frame_source_x", spec->frame_source_x, 18,
               "DUNVIEW.C:583 G0163 D3C C6; F0100 line 3055");
    expect_int("spec.frame_source_y", spec->frame_source_y, 0,
               "DUNVIEW.C:583 G0163 D3C C7; F0100 line 3055");
    expect_int("spec.uses_f0100", spec->uses_f0100_wallset_bitmap ? 1 : 0, 1,
               "DUNVIEW.C:6699 F0118 C00_ELEMENT_WALL calls F0100");
    expect_int("spec.uses_g0698", spec->uses_g0698_wall_d3lcr ? 1 : 0, 1,
               "DUNVIEW.C:6699 G0698_puc_Bitmap_WallSet_Wall_D3LCR");
    expect_int("spec.uses_m600_frame", spec->uses_g0163_m600_frame ? 1 : 0, 1,
               "DUNVIEW.C:6699 G0163[M600]; DEFS.H:2578");
    expect_int("spec.wall_case_returns", spec->wall_case_returns ? 1 : 0, 1,
               "DUNVIEW.C:6716-6720 F0107 false path returns");
}

static void test_blit_path_c10_and_center_columns(void)
{
    const DM1_V1_D3CWallSpecPc34 *spec =
        dm1_v1_viewport_d3c_wall_spec_pc34();

    expect_int("blit.f0100_source_byte_width", spec->f0100_source_byte_width, 64,
               "DUNVIEW.C:3055 F0100 uses frame C4; DUNVIEW.C:583 C4=64");
    expect_int("blit.c112_viewport_byte_width", spec->f0100_viewport_byte_width, 112,
               "DUNVIEW.C:3055 F0100 C112; DEFS.H:2478 C112=112");
    expect_int("blit.height_matches_frame", spec->frame_height, 51,
               "DUNVIEW.C:3058 F0100 frame C5; DUNVIEW.C:583 C5=51");
    expect_int("blit.transparent_c10", spec->transparent_color, 10,
               "DUNVIEW.C:3055 F0100 C10; DEFS.H:2088 C10=10");
    expect_int("blit.uses_c10", spec->uses_c10_transparency ? 1 : 0, 1,
               "DUNVIEW.C:3055 F0100 C10_COLOR_FLESH");
    expect_int("blit.preserves_c112", spec->frame_clip_preserves_c112_byte_width ? 1 : 0, 1,
               "DUNVIEW.C:3055 F0100 preserves C112; DEFS.H:2478");
    expect_int("center.viewport_center_x", spec->viewport_center_x, 112,
               "DUNVIEW.C:583 frame 74..149 spans center; F0118 line 6699");
    expect_int("center.frame_contains_center",
               (spec->frame_x1 <= spec->viewport_center_x &&
                spec->viewport_center_x <= spec->frame_x2) ? 1 : 0,
               1, "DUNVIEW.C:583 D3C is central D3 column; DEFS.H:2578");
    expect_int("center.resolves_center_column",
               spec->frame_resolves_center_column ? 1 : 0, 1,
               "DUNVIEW.C:583 G0163 D3C X1/X2; F0118 line 6699");
}

static void test_wall_pixel_set_and_transparency(void)
{
    uint8_t source[DM1_V1_D3C_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D3C_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D3C_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D3CWallPixelResultPc34 out;
    DM1_V1_D3CWallPixelInputPc34 input = {
        DM1_V1_D3C_ELEMENT_WALL_PC34,
        25,
        112,
        DM1_V1_D3C_WALL_C10_COLOR_FLESH_PC34,
        false
    };

    memset(source, 10, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[56] = 0x42;
    source[57] = 10;
    source[93] = 0x7a;

    expect_int("pixel.center.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:6699 F0100 wall blit; F0100 line 3055");
    expect_int("pixel.center.draws_wall", out.draws_d3c_wall_pixels ? 1 : 0, 1,
               "DUNVIEW.C:6697-6720 F0118 C00_ELEMENT_WALL path");
    expect_int("pixel.center.in_clip", out.in_clip ? 1 : 0, 1,
               "DUNVIEW.C:583 G0163 D3C clip; F0118 line 6699");
    expect_int("pixel.center.source_x", out.source_x, 56,
               "DUNVIEW.C:583 C6=18 plus central column offset 38");
    expect_int("pixel.center.source_y", out.source_y, 0,
               "DUNVIEW.C:583 C7=0 and viewport Y1=25");
    expect_int("pixel.center.writes", out.writes_pixel ? 1 : 0, 1,
               "DUNVIEW.C:3055 F0100 writes non-C10 pixels");
    expect_int("pixel.center.value", out.pixel_after, 0x42,
               "DUNVIEW.C:6699 F0100 deterministic gate, no asset parity");
    expect_contains("pixel.evidence.wall_path", out.source_lock_evidence,
                    "DUNVIEW.C F0118 C00_ELEMENT_WALL path",
                    "DUNVIEW.C:6697-6720 F0118 source evidence");
    expect_contains("pixel.evidence.alcove_return", out.source_lock_evidence,
                    "F0107 C00_ELEMENT_WALL alcove return",
                    "DUNVIEW.C:6716-6718 F0107 return evidence");

    input.viewport_x = 113;
    expect_int("pixel.c10.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:3055 F0100 C10 transparent blit");
    expect_int("pixel.c10.source_x", out.source_x, 57,
               "DUNVIEW.C:583 G0163 source X increments without skew");
    expect_int("pixel.c10.skip", out.transparent_skip ? 1 : 0, 1,
               "DUNVIEW.C:3055 C10_COLOR_FLESH; DEFS.H:2088");
    expect_int("pixel.c10.no_write", out.writes_pixel ? 1 : 0, 0,
               "DUNVIEW.C:3055 transparent pixels preserve destination");
    expect_int("pixel.c10.preserved", out.pixel_after, 0xee,
               "DUNVIEW.C:3055 F0100 C10 transparency");

    input.viewport_x = 149;
    expect_int("pixel.right_edge.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:583 G0163 D3C right edge; F0100 line 3055");
    expect_int("pixel.right_edge.source_x", out.source_x, 93,
               "DUNVIEW.C:583 source X 18 plus frame offset 75");
    expect_int("pixel.right_edge.value", out.pixel_after, 0x7a,
               "DUNVIEW.C:6699 F0100 deterministic edge gate");
}

static void test_f0107_alcove_return_path(void)
{
    uint8_t source[DM1_V1_D3C_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D3C_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D3C_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D3CWallPixelResultPc34 out;
    DM1_V1_D3CWallPixelInputPc34 input = {
        DM1_V1_D3C_ELEMENT_WALL_PC34,
        25,
        112,
        DM1_V1_D3C_WALL_C10_COLOR_FLESH_PC34,
        true
    };

    memset(source, 0x33, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));

    expect_int("f0107.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:6716 F0118 calls F0107");
    expect_int("f0107.called", out.calls_f0107 ? 1 : 0, 1,
               "DUNVIEW.C:6716 F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF");
    expect_int("f0107.ornament_arg", out.f0107_ornament_ordinal_arg, 3,
               "DUNVIEW.C:6716 uses M552; DEFS.H:2538 M552=3");
    expect_int("f0107.view_wall_arg", out.f0107_view_wall_arg, 3,
               "DUNVIEW.C:6716 uses M578; DEFS.H:2684 M578=3");
    expect_int("f0107.result_true", out.f0107_alcove_result ? 1 : 0, 1,
               "DUNVIEW.C:6716-6718 F0107 true drives alcove return path");
    expect_int("f0107.alcove_path", out.f0107_alcove_return_path ? 1 : 0, 1,
               "DUNVIEW.C:6717-6718 C0x0000_CELL_ORDER_ALCOVE goto");
    expect_int("f0107.cell_order", out.alcove_cell_order, 0,
               "DUNVIEW.C:6717 C0x0000_CELL_ORDER_ALCOVE; DEFS.H:2658");
    expect_int("f0107.no_plain_return", out.returns_after_wall_blit ? 1 : 0, 0,
               "DUNVIEW.C:6718 skips line 6720 return when alcove is true");

    input.f0107_alcove_result = false;
    expect_int("f0107.false.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:6716-6720 F0107 false path");
    expect_int("f0107.false_plain_return", out.returns_after_wall_blit ? 1 : 0, 1,
               "DUNVIEW.C:6720 F0118 C00_ELEMENT_WALL returns");
    expect_int("f0107.false_no_alcove_path", out.f0107_alcove_return_path ? 1 : 0, 0,
               "DUNVIEW.C:6716-6720 F0107 false path");
}

static void test_non_wall_no_write_contracts(void)
{
    uint8_t source[DM1_V1_D3C_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D3C_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D3C_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D3CWallPixelResultPc34 out;
    DM1_V1_D3CWallPixelInputPc34 input = {
        DM1_V1_D3C_ELEMENT_DOOR_FRONT_PC34,
        25,
        112,
        DM1_V1_D3C_WALL_C10_COLOR_FLESH_PC34,
        false
    };
    size_t center_offset = 25u * DM1_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 + 112u;

    memset(source, 0x44, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));

    expect_int("nowrap.spec.door_no_wall_pixels",
               dm1_v1_viewport_d3c_wall_spec_pc34()->door_front_draws_d3c_wall_pixels ? 1 : 0,
               0, "DUNVIEW.C:6721 F0118 C17 separate door arm; DEFS.H:1015");
    expect_int("nowrap.spec.stairs_front_no_wall_pixels",
               dm1_v1_viewport_d3c_wall_spec_pc34()->stairs_front_draws_d3c_wall_pixels ? 1 : 0,
               0, "DUNVIEW.C:6666 F0118 C19 separate stairs arm; DEFS.H:1017");
    expect_int("nowrap.spec.stairs_side_no_wall_pixels",
               dm1_v1_viewport_d3c_wall_spec_pc34()->stairs_side_draws_d3c_wall_pixels ? 1 : 0,
               0, "DUNVIEW.C:6642-6720 no C18 wall blit arm; DEFS.H:1016");
    expect_int("nowrap.spec.pit_no_wall_pixels",
               dm1_v1_viewport_d3c_wall_spec_pc34()->pit_draws_d3c_wall_pixels ? 1 : 0,
               0, "DUNVIEW.C:6763 C02 pit arm, no G0698 wall blit; DEFS.H:1009");

    expect_int("nowrap.door.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:6721 C17 door arm excludes F0100 line 6699");
    expect_int("nowrap.door.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:6721 C17 door arm no D3C wall pixels");
    expect_int("nowrap.door.untouched", viewport[center_offset], 0xee,
               "DUNVIEW.C:6721 C17 door arm no G0698 F0100 wall blit");

    input.element = DM1_V1_D3C_ELEMENT_STAIRS_FRONT_PC34;
    expect_int("nowrap.stairs_front.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:6666 C19 stairs arm excludes F0100 line 6699");
    expect_int("nowrap.stairs_front.no_write", out.draws_d3c_wall_pixels ? 1 : 0, 0,
               "DUNVIEW.C:6666-6696 C19 routes stairs bitmap, not wall pixels");

    input.element = DM1_V1_D3C_ELEMENT_STAIRS_SIDE_PC34;
    expect_int("nowrap.stairs_side.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:6642-6720 F0118 lacks C18 wall case; DEFS.H:1016");
    expect_int("nowrap.stairs_side.no_write", out.draws_d3c_wall_pixels ? 1 : 0, 0,
               "DUNVIEW.C:6642-6720 only C00 reaches D3C wall F0100");

    input.element = DM1_V1_D3C_ELEMENT_PIT_PC34;
    expect_int("nowrap.pit.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:6763 C02 pit arm excludes F0100 line 6699");
    expect_int("nowrap.pit.no_write", out.draws_d3c_wall_pixels ? 1 : 0, 0,
               "DUNVIEW.C:6763 C02 pit arm no D3C wall pixels");
}

static void test_frame_clip_and_invalid_inputs(void)
{
    uint8_t source[DM1_V1_D3C_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D3C_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D3C_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D3CWallPixelResultPc34 out;
    DM1_V1_D3CWallPixelInputPc34 input = {
        DM1_V1_D3C_ELEMENT_WALL_PC34,
        25,
        73,
        0,
        false
    };

    memset(source, 0x55, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));

    expect_int("clip.before_x.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:583 G0163 X1=74; F0100 line 3055");
    expect_int("clip.before_x.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:583 frame clip excludes X before 74");

    input.viewport_x = 150;
    expect_int("clip.after_x.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:583 G0163 X2=149; F0100 line 3055");
    expect_int("clip.after_x.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:583 frame clip excludes X after 149");

    input.row = 76;
    input.viewport_x = 112;
    expect_int("clip.after_y.apply",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               1, "DUNVIEW.C:583 G0163 Y2=75; F0100 line 3058");
    expect_int("clip.after_y.no_write", out.no_write_metadata ? 1 : 0, 1,
               "DUNVIEW.C:583 frame clip excludes Y after 75");
    expect_int("clip.blend_c10",
               dm1_v1_viewport_d3c_wall_blend_pixel_pc34(0x44, 10, 10), 0x44,
               "DUNVIEW.C:3055 F0100 C10; DEFS.H:2088");
    expect_int("clip.blend_opaque",
               dm1_v1_viewport_d3c_wall_blend_pixel_pc34(0x44, 0x51, 10), 0x51,
               "DUNVIEW.C:3055 F0100 non-C10 writes");
    expect_int("invalid.null_out",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   &input, source, sizeof(source), viewport, sizeof(viewport), NULL) ? 1 : 0,
               0, "DUNVIEW.C:6642-6720 F0118 gate rejects null result");
    expect_int("invalid.null_input",
               dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
                   NULL, source, sizeof(source), viewport, sizeof(viewport), &out) ? 1 : 0,
               0, "DUNVIEW.C:6642-6720 F0118 gate rejects null input");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const DM1_V1_D3CWallSpecPc34 *spec =
        dm1_v1_viewport_d3c_wall_spec_pc34();
    const char *e = dm1_v1_viewport_d3c_wall_source_evidence_pc34();

    expect_int("evidence.pointer", spec->source_lock_evidence == e ? 1 : 0, 1,
               "DUNVIEW.C:6642-6720 F0118 evidence pointer");
    expect_contains("evidence.no_asset_parity", e, "no real-asset pixel parity",
                    "DUNVIEW.C:6642-6720 source-lock gate only");
    expect_contains("evidence.f0118_wall_path", e,
                    "DUNVIEW.C F0118 C00_ELEMENT_WALL path",
                    "DUNVIEW.C:6697-6720 F0118 C00 wall path");
    expect_contains("evidence.f0100_call", e,
                    "line 6699 calls F0100_DUNGEONVIEW_DrawWallSetBitmap",
                    "DUNVIEW.C:6699 F0100 wallset call");
    expect_contains("evidence.g0698", e,
                    "G0698_puc_Bitmap_WallSet_Wall_D3LCR",
                    "DUNVIEW.C:6699 wallset bitmap");
    expect_contains("evidence.m600_frame", e,
                    "G0163_aauc_Graphic558_Frame_Walls[M600_VIEW_SQUARE_D3C]",
                    "DUNVIEW.C:6699 frame pointer; DEFS.H:2578");
    expect_contains("evidence.f0107_return", e,
                    "F0107 C00_ELEMENT_WALL alcove return",
                    "DUNVIEW.C:6716-6718 F0107 alcove return");
    expect_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                    "DUNVIEW.C:3055 F0100 C10");
    expect_contains("evidence.c112", e, "DEFS.H:2478 C112_BYTE_WIDTH_VIEWPORT",
                    "DUNVIEW.C:3055 F0100 C112");
    expect_contains("evidence.m552", e, "DEFS.H:2538 M552_FRONT_WALL_ORNAMENT_ORDINAL",
                    "DUNVIEW.C:6716 F0107 M552 argument");
    expect_contains("evidence.m600", e, "DEFS.H:2578 M600_VIEW_SQUARE_D3C=0",
                    "DUNVIEW.C:6699 M600 frame index");
    expect_contains("evidence.m578", e, "DEFS.H:2684 M578_VIEW_WALL_D3C_FRONT=3",
                    "DUNVIEW.C:6716 F0107 M578 argument");
}

int main(void)
{
    test_frame_resolution_and_wall_route();
    test_blit_path_c10_and_center_columns();
    test_wall_pixel_set_and_transparency();
    test_f0107_alcove_return_path();
    test_non_wall_no_write_contracts();
    test_frame_clip_and_invalid_inputs();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d3c_wall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d3c_wall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
