#include "dm1_v1_viewport_d2c_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_VIEW_SQUARE_D2C_PC34 = 6,   /* ReDMCSB DEFS.H:2602 M603_VIEW_SQUARE_D2C */
    DM1_V1_WALL_D2C_PC34 = 9,          /* ReDMCSB DEFS.H:3432 C09_WALL_D2C */
    DM1_V1_ZONE_WALL_D2C_PC34 = 709    /* ReDMCSB DEFS.H:4049 C709_ZONE_WALL_D2C */
};

static const char s_non_overlap_note[] =
    "non-overlap: D2C F0108 floor-ornament coverage stays in "
    "test_dm1_v1_viewport_floor_ornament_d2lr_pc34_compat; D2C center "
    "field coverage stays in the broad 3D integration. This gate covers "
    "only the D2C center wall pixel slice, clipped edges, C10 preservation, "
    "and no-write metadata; it does not claim real-asset bitmap parity.";

/*
 * ReDMCSB source lock:
 * DUNVIEW.C:3048-3058 F0100 keeps the C10 transparent frame blit contract.
 * DUNVIEW.C:3064-3076 F0101 is the PC center-wall opaque optimization.
 * DUNVIEW.C:3113-3129 F0104 is the PC34 native bitmap route shape.
 * DUNVIEW.C:581-594 G0163 gives the D2C frame clip row.
 * DUNVIEW.C:7244-7312 F0121 routes the D2C wall and returns.
 */
static const DM1_V1_D2CWallSpecPc34 s_spec = {
    true,
    false,
    DM1_V1_VIEW_SQUARE_D2C_PC34,
    DM1_V1_WALL_D2C_PC34,
    DM1_V1_ZONE_WALL_D2C_PC34,
    DM1_V1_VIEW_SQUARE_D2C_PC34,
    72,
    32,
    DM1_V1_D2C_WALL_SOURCE_HEIGHT_PC34,
    DM1_V1_D2C_WALL_C10_COLOR_FLESH_PC34,
    16,
    71,
    0,
    70,
    60,
    115,
    20,
    90,
    DM1_V1_D2C_WALL_SOURCE_WIDTH_PC34,
    DM1_V1_D2C_WALL_SOURCE_HEIGHT_PC34,
    true,
    true,
    true,
    true,
    true,
    true,
    false,
    false,
    false,
    false,
    "M603_VIEW_SQUARE_D2C",
    NULL,
    s_non_overlap_note
};

const DM1_V1_D2CWallSpecPc34 *dm1_v1_viewport_d2c_wall_spec_pc34(void)
{
    static DM1_V1_D2CWallSpecPc34 spec;
    spec = s_spec;
    spec.source_lines = dm1_v1_viewport_d2c_wall_source_evidence_pc34();
    return &spec;
}

bool dm1_v1_viewport_d2c_wall_apply_pixel_pc34(
    const DM1_V1_D2CWallPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D2CWallPixelResultPc34 *out)
{
    const DM1_V1_D2CWallSpecPc34 *spec;
    uint8_t transparent_color;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d2c_wall_spec_pc34();
    out->spec = *spec;
    if (!input) return false;

    out->row = input->row;
    out->viewport_x = input->viewport_x;
    transparent_color = input->transparent_color;
    if (transparent_color == 0) {
        transparent_color = DM1_V1_D2C_WALL_C10_COLOR_FLESH_PC34;
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
    out->source_x = spec->source_x_first +
        (input->viewport_x - spec->viewport_x_first);
    out->source_y = spec->source_y_first +
        (input->row - spec->viewport_y_first);
    out->source_offset = (size_t)out->source_y * (size_t)spec->source_width +
        (size_t)out->source_x;
    out->viewport_offset = (size_t)input->row *
        (size_t)DM1_V1_D2C_WALL_VIEWPORT_WIDTH_PC34 +
        (size_t)input->viewport_x;
    if (out->source_offset >= source_len || out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip = out->source_pixel == transparent_color;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] = dm1_v1_viewport_d2c_wall_blend_pixel_pc34(
        viewport[out->viewport_offset], out->source_pixel, transparent_color);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

uint8_t dm1_v1_viewport_d2c_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_d2c_wall_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; contract_only=1; "
        "no_asset_parity=1; real-asset bitmap parity is not claimed. "
        "DUNVIEW.C:3048-3058 F0100_DUNGEONVIEW_DrawWallSetBitmap blits "
        "wall frames with C10_COLOR_FLESH transparency. DUNVIEW.C:3064-3076 "
        "F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency is the PC34 "
        "center-wall opaque optimization and is recorded as route metadata, "
        "not as a full bitmap parity claim. DUNVIEW.C:3113-3129 F0104 "
        "DUNGEONVIEW_DrawFloorPitOrStairsBitmap shows the PC34 native bitmap "
        "route shape used by layout zones. DUNVIEW.C:581-594 "
        "G0163_aauc_Graphic558_Frame_Walls gives D2C at line 586: "
        "{60,163,20,90,72,71,16,0}; this slice gates the clipped center "
        "wall band byte_width=32, height=71, source X 16..71, viewport X "
        "60..115. DUNVIEW.C:7244-7312 F0121_DUNGEONVIEW_DrawSquareD2C "
        "handles C00_ELEMENT_WALL; lines 7291/7294 draw "
        "G0699_puc_Bitmap_WallSet_Wall_D2LCR with "
        "G0163_aauc_Graphic558_Frame_Walls[M603_VIEW_SQUARE_D2C], lines "
        "7299-7306 use G2107_WallSet[C09_WALL_D2C] / C709_ZONE_WALL_D2C "
        "for PC34 native routes, line 7308 probes the front wall ornament, "
        "and line 7312 returns. DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2602 "
        "M603_VIEW_SQUARE_D2C=6; DEFS.H:2600 M607_VIEW_SQUARE_D1L=4, so "
        "the requested M607_VIEW_SQUARE_D2C label is corrected to M603 by "
        "source. DEFS.H:3432 C09_WALL_D2C; DEFS.H:4049 C709_ZONE_WALL_D2C. "
        "COORD.C:2542-2569 F0640/F0641 load graphic layout ranges; "
        "COORD.C:2390-2409 F0635 clips PC34 layout zones and source offsets. "
        "non-overlap: this D2C wall slice excludes F0108 floor ornaments, "
        "F0111 doors, F0113 center-field integration, and F0115 thing pass.";
}
