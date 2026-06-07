#include "dm1_v1_viewport_d1l2_wall_pc34_compat.h"

#include <string.h>

static const char s_non_overlap_note[] =
    "non-overlap: D1L2 floor ornament gate covers the D1L2 floor route; "
    "this gate covers the D1L2/D1L wall route, its frame/clip metadata, "
    "C10 transparency, clipped edge, neighboring pixels, and no-write metadata.";

static const DM1_V1_D1L2WallSpecPc34 s_spec = {
    true,
    false,
    4,
    3,
    713,
    4,
    4,
    0,
    DM1_V1_D1L2_WALL_C10_COLOR_FLESH_PC34,
    0x82,
    0,
    63,
    0,
    110,
    0,
    63,
    0,
    110,
    DM1_V1_D1L2_WALL_SOURCE_WIDTH_PC34,
    DM1_V1_D1L2_WALL_SOURCE_HEIGHT_PC34,
    32,
    111,
    true,
    true,
    false,
    true,
    true,
    false,
    false,
    false,
    NULL,
    s_non_overlap_note
};

const DM1_V1_D1L2WallSpecPc34 *dm1_v1_viewport_d1l2_wall_spec_pc34(void)
{
    static DM1_V1_D1L2WallSpecPc34 spec;
    spec = s_spec;
    spec.source_lines = dm1_v1_viewport_d1l2_wall_source_evidence_pc34();
    return &spec;
}

bool dm1_v1_viewport_d1l2_wall_apply_pixel_pc34(
    const DM1_V1_D1L2WallPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D1L2WallPixelResultPc34 *out)
{
    const DM1_V1_D1L2WallSpecPc34 *spec;
    uint8_t transparent_color;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d1l2_wall_spec_pc34();
    out->spec = *spec;
    if (!input) return false;

    out->row = input->row;
    out->viewport_x = input->viewport_x;
    transparent_color = input->transparent_color;
    if (transparent_color == 0) {
        transparent_color = DM1_V1_D1L2_WALL_C10_COLOR_FLESH_PC34;
    }
    out->spec.transparent_color = transparent_color;

    if (input->row < spec->viewport_y_first ||
        input->row > spec->viewport_y_last ||
        input->viewport_x < spec->viewport_x_first ||
        input->viewport_x > spec->viewport_x_last) {
        out->no_write_metadata = true;
        return true;
    }
    if (!source || !viewport) return false;

    out->in_clip = true;
    out->source_x = spec->source_x_first + (input->viewport_x - spec->viewport_x_first);
    out->source_y = spec->source_y_first + (input->row - spec->viewport_y_first);
    out->source_offset = (size_t)out->source_y * (size_t)spec->source_width +
        (size_t)out->source_x;
    out->viewport_offset = (size_t)input->row *
        (size_t)DM1_V1_D1L2_WALL_VIEWPORT_WIDTH_PC34 +
        (size_t)input->viewport_x;
    if (out->source_offset >= source_len || out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip = out->source_pixel == transparent_color;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] = dm1_v1_viewport_d1l2_wall_blend_pixel_pc34(
        viewport[out->viewport_offset], out->source_pixel, transparent_color);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

uint8_t dm1_v1_viewport_d1l2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_d1l2_wall_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; contract_only=1; "
        "no_asset_parity=1; real-asset bitmap parity is not claimed. "
        "No literal D1L2 wall entry exists in this DM1 DUNVIEW.C snapshot, "
        "so this covers the next uncovered D1L2-related wall slot: F0122 "
        "D1L wall frame/clip pixel contract. "
        "DUNVIEW.C:718-731 G0188 field aspects, D1L entry line 727 gives "
        "NativeBitmapRelativeIndex=0, BaseStartUnitIndex=63, C10 transparency "
        "0x0A, mask 0x82, byte_width=32, height=111, X=0. "
        "DUNVIEW.C:3048-3058 F0100_DUNGEONVIEW_DrawWallSetBitmap uses frame "
        "bitmap/clip metadata and blits with C10_COLOR_FLESH. "
        "DUNVIEW.C:3113-3129 F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap "
        "uses the same C10 transparent clipped blit path for native bitmap "
        "frames. DUNVIEW.C:7436-7460 F0122 D1L wall route draws "
        "G0700_puc_Bitmap_WallSet_Wall_D1LCR with "
        "G0163_aauc_Graphic558_Frame_Walls[M607_VIEW_SQUARE_D1L], PC34 "
        "zone C713_ZONE_WALL_D1L / C03_WALL_D1L, calls F0107 side ornament "
        "probe, then returns. DUNVIEW.C:7520-7525 F0122 corridor route is "
        "the separate F0108 floor-ornament path and is not covered here. "
        "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2596-2601 M607_VIEW_SQUARE_D1L=4; "
        "DEFS.H:3423-3427 C03_WALL_D1L=3; DEFS.H:4052-4054 "
        "C713_ZONE_WALL_D1L. "
        "non-overlap: D1L2 floor ornament gate covers the D1L2 floor route; "
        "this gate covers the D1L2/D1L wall route only, including clipped "
        "edge, neighboring pixels, and no-write metadata.";
}
