#include "csb_v1_viewport_d3c_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_F0118 =
    "ReDMCSB DUNVIEW.C:6642-6720 F0118_DUNGEONVIEW_DrawSquareD3C_CPSF";
static const char *A_F0100 =
    "ReDMCSB DUNVIEW.C:3048-3058 F0100_DUNGEONVIEW_DrawWallSetBitmap";
static const char *A_F0101 =
    "ReDMCSB DUNVIEW.C:3065-3078 F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency";
static const char *A_G0163 =
    "ReDMCSB DUNVIEW.C:581-583 G0163_aauc_Graphic558_Frame_Walls";
static const char *A_G0698 =
    "ReDMCSB G0698_puc_Bitmap_WallSet_Wall_D3LCR";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1903-1915";

static int g_assertions = 0;
static int g_failures = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_int(label, haystack && needle &&
                         strstr(haystack, needle) != NULL, 1, anchor);
}

static int test_identity_and_source_lock_scope(void)
{
    int ok = 1;
    const CSB_V1_D3CWallSpecPc34 *spec =
        csb_v1_viewport_d3c_wall_spec_pc34();

    ok &= expect_int("contract.non_null", spec != NULL, 1, A_F0118);
    ok &= expect_int("contract.only", spec ? spec->contract_only : 0, 1,
                     A_F0118);
    ok &= expect_int("no.real_asset_pixel_parity",
                     spec ? spec->real_asset_pixel_parity : 1, 0, A_F0118);
    ok &= expect_int("wall.branch.only", spec ? spec->wall_branch_only : 0,
                     1, A_F0118);
    ok &= expect_int("view_square.m600", spec ? spec->view_square_m600 : -1,
                     11, "ReDMCSB DEFS.H:2607 M600_VIEW_SQUARE_D3C");
    ok &= expect_int("frame.ordinal.m600",
                     spec ? spec->frame_ordinal_m600 : -1, 0, A_G0163);
    ok &= expect_int("view.depth", spec ? spec->view_depth : -1, 3,
                     "ReDMCSB DUNVIEW.C:371-377 G2027");
    ok &= expect_int("view.lane", spec ? spec->view_lane : -1, 0,
                     "ReDMCSB DUNVIEW.C:371-377 G2026");
    ok &= expect_int("uses.f0118", spec ? spec->uses_f0118_d3c_wall_route : 0,
                     1, A_F0118);
    ok &= expect_int("rejects.f0121.d2c",
                     spec ? spec->rejects_f0121_d2c_path : 0, 1, A_F0118);
    ok &= expect_contains("anchor.f0118",
                          spec ? spec->redmcsb_f0118_anchor : NULL,
                          "6642-6720", A_F0118);
    ok &= expect_contains("anchor.lineage",
                          spec ? spec->csb_lineage_anchor : NULL,
                          "1903-1915", A_LINEAGE);
    return ok;
}

static int test_g0163_frame_and_effective_clip(void)
{
    int ok = 1;
    const CSB_V1_D3CWallSpecPc34 *spec =
        csb_v1_viewport_d3c_wall_spec_pc34();

    ok &= expect_int("frame.x1", spec ? spec->frame.x1 : -1, 74, A_G0163);
    ok &= expect_int("frame.x2", spec ? spec->frame.x2 : -1, 149, A_G0163);
    ok &= expect_int("frame.y1", spec ? spec->frame.y1 : -1, 25, A_G0163);
    ok &= expect_int("frame.y2", spec ? spec->frame.y2 : -1, 75, A_G0163);
    ok &= expect_int("frame.byte_width",
                     spec ? spec->frame.byte_width : -1, 64, A_G0163);
    ok &= expect_int("frame.height", spec ? spec->frame.height : -1, 51,
                     A_G0163);
    ok &= expect_int("frame.source_x", spec ? spec->frame.source_x : -1,
                     18, A_G0163);
    ok &= expect_int("frame.source_y", spec ? spec->frame.source_y : -1,
                     0, A_G0163);
    ok &= expect_int("effective.source_x1",
                     spec ? spec->effective_source_x1 : -1, 18, A_G0163);
    ok &= expect_int("effective.source_x2",
                     spec ? spec->effective_source_x2 : -1, 63, A_G0163);
    ok &= expect_int("effective.viewport_x1",
                     spec ? spec->effective_viewport_x1 : -1, 74, A_G0163);
    ok &= expect_int("effective.viewport_x2",
                     spec ? spec->effective_viewport_x2 : -1, 119, A_G0163);
    ok &= expect_int("effective.visible_width",
                     spec ? spec->effective_visible_width : -1, 46, A_G0163);
    ok &= expect_int("effective.visible_height",
                     spec ? spec->effective_visible_height : -1, 51, A_G0163);
    ok &= expect_int("viewport.byte_width",
                     spec ? spec->viewport_byte_width : -1, 112, A_F0100);
    ok &= expect_contains("anchor.g0163",
                          spec ? spec->redmcsb_g0163_anchor : NULL,
                          "G0163_aauc_Graphic558_Frame_Walls", A_G0163);
    return ok;
}

static int test_wall_pixel_slice_and_f0101_no_transparency(void)
{
    int ok = 1;
    uint8_t viewport[CSB_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 *
                     CSB_V1_D3C_WALL_VIEWPORT_HEIGHT_PC34];
    uint8_t source[CSB_V1_D3C_WALL_SOURCE_WIDTH_PC34 *
                   CSB_V1_D3C_WALL_SOURCE_HEIGHT_PC34];
    CSB_V1_D3CWallPixelResultPc34 out;
    CSB_V1_D3CWallPixelInputPc34 in = {
        CSB_V1_D3C_WALL_ELEMENT_WALL_PC34,
        25,
        74
    };

    memset(viewport, 0xee, sizeof(viewport));
    memset(source, 0x21, sizeof(source));
    source[18] = 10;
    source[19] = 0x42;
    source[(1 * CSB_V1_D3C_WALL_SOURCE_WIDTH_PC34) + 18] = 0x44;
    source[63] = 0x7e;
    source[(50 * CSB_V1_D3C_WALL_SOURCE_WIDTH_PC34) + 63] = 0x55;

    ok &= expect_int("pixel.left.apply",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     1, A_F0118);
    ok &= expect_int("pixel.left.in_clip", out.in_clip ? 1 : 0, 1,
                     A_G0163);
    ok &= expect_int("pixel.left.source_x", out.source_x, 18, A_G0163);
    ok &= expect_int("pixel.left.source_y", out.source_y, 0, A_G0163);
    ok &= expect_int("pixel.left.f0101_write",
                     out.f0101_no_transparency_write ? 1 : 0, 1, A_F0101);
    ok &= expect_int("pixel.left.f0100_reference_skip",
                     out.f0100_transparent_reference_skip ? 1 : 0, 1, A_F0100);
    ok &= expect_int("pixel.left.c10_written", out.pixel_after, 10,
                     A_F0101);
    ok &= expect_int("pixel.left.viewport_value",
                     viewport[25 * CSB_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 + 74],
                     10, A_F0101);

    in.viewport_x = 75;
    ok &= expect_int("pixel.next.apply",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     1, A_G0163);
    ok &= expect_int("pixel.next.source_x", out.source_x, 19, A_G0163);
    ok &= expect_int("pixel.next.value", out.pixel_after, 0x42, A_F0101);

    in.row = 26;
    in.viewport_x = 74;
    ok &= expect_int("pixel.next_row.apply",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     1, A_G0163);
    ok &= expect_int("pixel.next_row.source_y", out.source_y, 1, A_G0163);
    ok &= expect_int("pixel.next_row.value", out.pixel_after, 0x44, A_F0101);

    in.row = 25;
    in.viewport_x = 119;
    ok &= expect_int("pixel.right.apply",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     1, A_G0163);
    ok &= expect_int("pixel.right.source_x", out.source_x, 63, A_G0163);
    ok &= expect_int("pixel.right.value", out.pixel_after, 0x7e, A_F0101);

    in.row = 75;
    in.viewport_x = 119;
    ok &= expect_int("pixel.bottom.apply",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     1, A_G0163);
    ok &= expect_int("pixel.bottom.source_y", out.source_y, 50, A_G0163);
    ok &= expect_int("pixel.bottom.value", out.pixel_after, 0x55, A_F0101);
    return ok;
}

static int test_frame_clip_rejects_outside_slice(void)
{
    int ok = 1;
    uint8_t viewport[CSB_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 *
                     CSB_V1_D3C_WALL_VIEWPORT_HEIGHT_PC34];
    uint8_t source[CSB_V1_D3C_WALL_SOURCE_WIDTH_PC34 *
                   CSB_V1_D3C_WALL_SOURCE_HEIGHT_PC34];
    CSB_V1_D3CWallPixelResultPc34 out;
    CSB_V1_D3CWallPixelInputPc34 in = {
        CSB_V1_D3C_WALL_ELEMENT_WALL_PC34,
        25,
        73
    };

    memset(viewport, 0xee, sizeof(viewport));
    memset(source, 0x33, sizeof(source));

    ok &= expect_int("clip.before_x.apply",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     1, A_G0163);
    ok &= expect_int("clip.before_x.no_write", out.no_write_metadata ? 1 : 0,
                     1, A_G0163);
    ok &= expect_int("clip.before_x.untouched",
                     viewport[25 * CSB_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 + 73],
                     0xee, A_G0163);

    in.viewport_x = 120;
    ok &= expect_int("clip.after_effective_x.apply",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     1, A_G0163);
    ok &= expect_int("clip.after_effective_x.no_write",
                     out.no_write_metadata ? 1 : 0, 1, A_G0163);
    ok &= expect_int("clip.after_effective_x.untouched",
                     viewport[25 * CSB_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 + 120],
                     0xee, A_G0163);

    in.row = 24;
    in.viewport_x = 74;
    ok &= expect_int("clip.before_y.apply",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     1, A_G0163);
    ok &= expect_int("clip.before_y.no_write", out.no_write_metadata ? 1 : 0,
                     1, A_G0163);

    in.row = 76;
    ok &= expect_int("clip.after_y.apply",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     1, A_G0163);
    ok &= expect_int("clip.after_y.no_write", out.no_write_metadata ? 1 : 0,
                     1, A_G0163);
    ok &= expect_int("invalid.null_out",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), NULL) ? 1 : 0,
                     0, A_F0118);
    ok &= expect_int("invalid.null_input",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         NULL, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     0, A_F0118);
    return ok;
}

static int test_branch_is_wall_only(void)
{
    int ok = 1;
    uint8_t viewport[CSB_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 *
                     CSB_V1_D3C_WALL_VIEWPORT_HEIGHT_PC34];
    uint8_t source[CSB_V1_D3C_WALL_SOURCE_WIDTH_PC34 *
                   CSB_V1_D3C_WALL_SOURCE_HEIGHT_PC34];
    CSB_V1_D3CWallPixelResultPc34 out;
    CSB_V1_D3CWallPixelInputPc34 in = {
        CSB_V1_D3C_WALL_ELEMENT_DOOR_FRONT_PC34,
        25,
        74
    };
    const size_t offset = 25u * CSB_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 + 74u;
    const CSB_V1_D3CWallSpecPc34 *spec =
        csb_v1_viewport_d3c_wall_spec_pc34();

    memset(viewport, 0xee, sizeof(viewport));
    memset(source, 0x66, sizeof(source));

    ok &= expect_int("spec.door.no_wall",
                     spec ? spec->door_front_draws_d3c_wall_pixels : 1, 0,
                     A_F0118);
    ok &= expect_int("spec.stairs_front.no_wall",
                     spec ? spec->stairs_front_draws_d3c_wall_pixels : 1, 0,
                     A_F0118);
    ok &= expect_int("spec.stairs_side.no_wall",
                     spec ? spec->stairs_side_draws_d3c_wall_pixels : 1, 0,
                     A_F0118);
    ok &= expect_int("spec.pit.no_wall",
                     spec ? spec->pit_draws_d3c_wall_pixels : 1, 0, A_F0118);

    ok &= expect_int("door.apply",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) ? 1 : 0,
                     1, A_F0118);
    ok &= expect_int("door.no_write", out.no_write_metadata ? 1 : 0, 1,
                     A_F0118);
    ok &= expect_int("door.untouched", viewport[offset], 0xee, A_F0118);

    in.element = CSB_V1_D3C_WALL_ELEMENT_STAIRS_FRONT_PC34;
    ok &= expect_int("stairs_front.no_write",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) &&
                         !out.draws_d3c_wall_pixels, 1, A_F0118);

    in.element = CSB_V1_D3C_WALL_ELEMENT_PIT_PC34;
    ok &= expect_int("pit.no_write",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) &&
                         !out.draws_d3c_wall_pixels, 1, A_F0118);

    in.element = CSB_V1_D3C_WALL_ELEMENT_WALL_PC34;
    ok &= expect_int("wall.write",
                     csb_v1_viewport_d3c_wall_apply_pixel_pc34(
                         &in, source, sizeof(source), viewport,
                         sizeof(viewport), &out) &&
                         out.draws_d3c_wall_pixels, 1, A_F0118);
    return ok;
}

static int test_f0100_c10_reference_and_f0101_contrast(void)
{
    int ok = 1;
    const CSB_V1_D3CWallSpecPc34 *spec =
        csb_v1_viewport_d3c_wall_spec_pc34();

    ok &= expect_int("transparent.macro",
                     CSB_V1_D3C_WALL_C10_COLOR_FLESH_PC34, 10, A_F0100);
    ok &= expect_int("no_transparency.macro",
                     CSB_V1_D3C_WALL_NO_TRANSPARENCY_PC34, -1, A_F0101);
    ok &= expect_int("spec.transparent_color",
                     spec ? spec->transparent_color : -1, 10, A_F0100);
    ok &= expect_int("spec.no_transparency_color",
                     spec ? spec->no_transparency_color : 0, -1, A_F0101);
    ok &= expect_int("spec.f0101.route",
                     spec ? spec->uses_f0101_no_transparency : 0, 1,
                     A_F0101);
    ok &= expect_int("spec.f0100.reference",
                     spec ? spec->preserves_f0100_c10_reference : 0, 1,
                     A_F0100);
    ok &= expect_int("blend.f0100.c10_preserves",
                     csb_v1_viewport_d3c_wall_blend_f0100_transparent_pc34(
                         0x44, 10, 10),
                     0x44, A_F0100);
    ok &= expect_int("blend.f0100.opaque_writes",
                     csb_v1_viewport_d3c_wall_blend_f0100_transparent_pc34(
                         0x44, 0x51, 10),
                     0x51, A_F0100);
    ok &= expect_int("blend.f0101.c10_writes",
                     csb_v1_viewport_d3c_wall_blend_f0101_no_transparency_pc34(
                         0x44, 10),
                     10, A_F0101);
    ok &= expect_int("blend.f0101.opaque_writes",
                     csb_v1_viewport_d3c_wall_blend_f0101_no_transparency_pc34(
                         0x44, 0x51),
                     0x51, A_F0101);
    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const CSB_V1_D3CWallSpecPc34 *spec =
        csb_v1_viewport_d3c_wall_spec_pc34();
    const char *e = csb_v1_viewport_d3c_wall_source_evidence_pc34();

    ok &= expect_int("evidence.pointer",
                     spec && spec->source_evidence == e, 1, A_F0118);
    ok &= expect_contains("evidence.contract", e,
                          "Source-locked contract gate only", A_F0118);
    ok &= expect_contains("evidence.f0118", e,
                          "DUNVIEW.C:6642-6720", A_F0118);
    ok &= expect_contains("evidence.f0100", e,
                          "F0100_DUNGEONVIEW_DrawWallSetBitmap", A_F0100);
    ok &= expect_contains("evidence.f0101", e,
                          "F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency",
                          A_F0101);
    ok &= expect_contains("evidence.c10", e,
                          "C10_COLOR_FLESH", A_F0100);
    ok &= expect_contains("evidence.no_transparency", e,
                          "CM1_COLOR_NO_TRANSPARENCY", A_F0101);
    ok &= expect_contains("evidence.g0163", e,
                          "G0163_aauc_Graphic558_Frame_Walls", A_G0163);
    ok &= expect_contains("evidence.g0698", e,
                          "G0698_puc_Bitmap_WallSet_Wall_D3LCR", A_G0698);
    ok &= expect_contains("evidence.clip", e,
                          "source X 18..63 maps to viewport X 74..119",
                          A_G0163);
    ok &= expect_contains("evidence.f0107", e,
                          "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
                          A_F0118);
    ok &= expect_contains("evidence.wall_only", e,
                          "wall branch only", A_F0118);
    ok &= expect_contains("evidence.reject_center_field", e,
                          "D3C no-wall center-field", A_F0118);
    ok &= expect_contains("evidence.reject_d2c", e,
                          "D2C F0121/F0101", A_F0118);
    ok &= expect_contains("evidence.lineage_requested", e,
                          "Viewport.cpp:1903-1915", A_LINEAGE);
    ok &= expect_contains("evidence.lineage_f3", e,
                          "Viewport.cpp:1824-1835", A_LINEAGE);
    ok &= expect_contains("anchor.f0100",
                          spec ? spec->redmcsb_f0100_anchor : NULL,
                          "3048-3058", A_F0100);
    ok &= expect_contains("anchor.f0101",
                          spec ? spec->redmcsb_f0101_anchor : NULL,
                          "3065-3078", A_F0101);
    ok &= expect_contains("anchor.g0698",
                          spec ? spec->redmcsb_g0698_anchor : NULL,
                          "G0698", A_G0698);
    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d3c_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d3c_wall_source_evidence_pc34());

    ok &= test_identity_and_source_lock_scope();
    ok &= test_g0163_frame_and_effective_clip();
    ok &= test_wall_pixel_slice_and_f0101_no_transparency();
    ok &= test_frame_clip_rejects_outside_slice();
    ok &= test_branch_is_wall_only();
    ok &= test_f0100_c10_reference_and_f0101_contrast();
    ok &= test_evidence_strings();
    ok &= expect_int("assertion_count_at_least_50", g_assertions >= 50, 1,
                     A_F0118);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_d3c_wall_pc34_compat assertions=%d failures=0\n",
               g_assertions);
    }

    return (ok && g_failures == 0) ? 0 : 1;
}
