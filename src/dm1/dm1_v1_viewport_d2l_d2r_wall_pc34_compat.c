#include "dm1_v1_viewport_d2l_d2r_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_VIEW_SQUARE_D2L_PC34 = 4,      /* ReDMCSB DEFS.H:2582 M604_VIEW_SQUARE_D2L */
    DM1_V1_VIEW_SQUARE_D2R_PC34 = 5,      /* ReDMCSB DEFS.H:2583 M605_VIEW_SQUARE_D2R */
    DM1_V1_WALL_D2R_PC34 = 7,             /* ReDMCSB DEFS.H:3430 C07_WALL_D2R */
    DM1_V1_WALL_D2L_PC34 = 8,             /* ReDMCSB DEFS.H:3431 C08_WALL_D2L */
    DM1_V1_ZONE_WALL_D2L_PC34 = 710,      /* ReDMCSB DEFS.H:4050 C710_ZONE_WALL_D2L */
    DM1_V1_ZONE_WALL_D2R_PC34 = 711       /* ReDMCSB DEFS.H:4051 C711_ZONE_WALL_D2R */
};

/*
 * ReDMCSB source-lock anchors:
 * - DUNVIEW.C:6900 F0119_DUNGEONVIEW_DrawSquareD2L routes the D2L wall.
 * - DUNVIEW.C:7051 F0120_DUNGEONVIEW_DrawSquareD2R_CPSF routes the D2R wall.
 * - DUNVIEW.C:3048-3058 F0100_DUNGEONVIEW_DrawWallSetBitmap blits wall
 *   frames through C10_COLOR_FLESH transparency.
 * - DUNVIEW.C:3185-3204 F0105 and DUNVIEW.C:8390/8555 preserve the
 *   party-side horizontal flip contract for D2L/D2R wallset rows.
 * - DUNVIEW.C:581-591 G0163 gives D2L row {0..74, source X=61} and
 *   D2R row {149..223, source X=0}; this contract gates only the
 *   requested side-wall slices: D2L viewport X 0..10/source X 61..71 and
 *   D2R viewport X 224..233/source X 0..9.
 * - DUNVIEW.C:8318-8555 F0128 draws D2L then D2R after the center-field
 *   setup and before D2C/D1/D0 foreground passes.
 * - DUNVIEW.C:6947/7098 use G0699_puc_Bitmap_WallSet_Wall_D2LCR with
 *   G0163_aauc_Graphic558_Frame_Walls[M604/M605].
 *
 * This is a contract-only synthetic regression; it does not claim
 * real-asset bitmap parity or a full D2L/D2R wall renderer.
 */
static const DM1_V1_D2LD2RWallSpecPc34 s_specs[2] = {
    {
        DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34,
        true,
        false,
        2,
        -1,
        DM1_V1_VIEW_SQUARE_D2L_PC34,
        DM1_V1_WALL_D2L_PC34,
        DM1_V1_WALL_D2R_PC34,
        DM1_V1_ZONE_WALL_D2L_PC34,
        10,
        9,
        0,
        74,
        20,
        90,
        72,
        71,
        61,
        0,
        0,
        10,
        20,
        90,
        61,
        71,
        0,
        70,
        DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34,
        DM1_V1_D2L_D2R_WALL_SOURCE_HEIGHT_PC34,
        11,
        71,
        true,
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        "M604_VIEW_SQUARE_D2L",
        "C08_WALL_D2L",
        "C07_WALL_D2R",
        "C710_ZONE_WALL_D2L",
        NULL
    },
    {
        DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34,
        true,
        false,
        2,
        1,
        DM1_V1_VIEW_SQUARE_D2R_PC34,
        DM1_V1_WALL_D2R_PC34,
        DM1_V1_WALL_D2L_PC34,
        DM1_V1_ZONE_WALL_D2R_PC34,
        11,
        9,
        149,
        223,
        20,
        90,
        72,
        71,
        0,
        0,
        224,
        233,
        20,
        90,
        0,
        9,
        0,
        70,
        DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34,
        DM1_V1_D2L_D2R_WALL_SOURCE_HEIGHT_PC34,
        10,
        71,
        true,
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        "M605_VIEW_SQUARE_D2R",
        "C07_WALL_D2R",
        "C08_WALL_D2L",
        "C711_ZONE_WALL_D2R",
        NULL
    }
};

const DM1_V1_D2LD2RWallSpecPc34 *
dm1_v1_viewport_d2l_d2r_wall_spec_pc34(DM1_V1_D2LD2RWallSidePc34 side)
{
    static DM1_V1_D2LD2RWallSpecPc34 specs[2];
    static bool initialized = false;

    if (side != DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34 &&
        side != DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34) {
        return NULL;
    }
    if (!initialized) {
        specs[0] = s_specs[0];
        specs[1] = s_specs[1];
        specs[0].source_lines = dm1_v1_viewport_d2l_d2r_wall_source_evidence_pc34();
        specs[1].source_lines = dm1_v1_viewport_d2l_d2r_wall_source_evidence_pc34();
        initialized = true;
    }
    return &specs[(int)side];
}

bool dm1_v1_viewport_d2l_d2r_wall_map_pixel_pc34(
    const DM1_V1_D2LD2RWallSpecPc34 *spec,
    int row,
    int viewport_x,
    int *source_x,
    int *source_y)
{
    if (!spec || !source_x || !source_y) return false;
    if (row < spec->viewport_y_first ||
        row > spec->viewport_y_last ||
        viewport_x < spec->viewport_x_first ||
        viewport_x > spec->viewport_x_last) {
        return false;
    }

    *source_x = spec->source_x_first + (viewport_x - spec->viewport_x_first);
    *source_y = spec->source_y_first + (row - spec->viewport_y_first);
    if (*source_x < 0 || *source_x >= spec->source_width ||
        *source_y < 0 || *source_y >= spec->source_height) {
        return false;
    }
    return true;
}

bool dm1_v1_viewport_d2l_d2r_wall_map_party_side_pixel_pc34(
    const DM1_V1_D2LD2RWallSpecPc34 *spec,
    int row,
    int viewport_x,
    bool party_side_flipped,
    int *source_x,
    int *source_y)
{
    int mapped_x;
    int mapped_y;

    if (!dm1_v1_viewport_d2l_d2r_wall_map_pixel_pc34(
            spec, row, viewport_x, &mapped_x, &mapped_y)) {
        return false;
    }

    /*
     * ReDMCSB: DUNVIEW.C F0128 lines 8390-8555 swaps
     * G0699_puc_Bitmap_WallSet_Wall_D2LCR to the flipped wallset before
     * F0119/F0120. DUNVIEW.C F0105 lines 3185-3204 performs the same
     * horizontal row flip before the C10-transparent wall blit, so the
     * source-locked column for a flipped row is width - 1 - native_x.
     */
    if (party_side_flipped) {
        mapped_x = spec->source_width - 1 - mapped_x;
    }

    if (source_x) *source_x = mapped_x;
    if (source_y) *source_y = mapped_y;
    return source_x != NULL && source_y != NULL;
}

bool dm1_v1_viewport_d2l_d2r_wall_apply_pixel_pc34(
    const DM1_V1_D2LD2RWallPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D2LD2RWallPixelResultPc34 *out)
{
    const DM1_V1_D2LD2RWallSpecPc34 *spec;
    uint8_t transparent_color;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!input) return false;

    spec = dm1_v1_viewport_d2l_d2r_wall_spec_pc34(input->side);
    if (!spec) return false;
    out->spec = *spec;
    out->row = input->row;
    out->viewport_x = input->viewport_x;
    transparent_color = input->transparent_color;
    if (transparent_color == 0) {
        transparent_color = DM1_V1_D2L_D2R_WALL_C10_COLOR_FLESH_PC34;
    }

    if (!dm1_v1_viewport_d2l_d2r_wall_map_pixel_pc34(
            spec, input->row, input->viewport_x, &out->source_x, &out->source_y)) {
        out->no_write_metadata = true;
        return true;
    }
    if (!source || !viewport) return false;

    out->in_clip = true;
    out->source_offset = (size_t)out->source_y * (size_t)spec->source_width +
        (size_t)out->source_x;
    out->viewport_offset = (size_t)input->row *
        (size_t)DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 +
        (size_t)input->viewport_x;
    if (out->source_offset >= source_len || out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip = out->source_pixel == transparent_color;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] =
        dm1_v1_viewport_d2l_d2r_wall_blend_pixel_pc34(
            viewport[out->viewport_offset], out->source_pixel, transparent_color);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

uint8_t dm1_v1_viewport_d2l_d2r_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_d2l_d2r_wall_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; contract_only=1; "
        "no_asset_parity=1; real-asset bitmap parity is not claimed. "
        "ReDMCSB DUNVIEW.C:6900 F0119_DUNGEONVIEW_DrawSquareD2L is the "
        "D2L wall route; line 6947 draws G0699_puc_Bitmap_WallSet_Wall_D2LCR "
        "with G0163_aauc_Graphic558_Frame_Walls[M604_VIEW_SQUARE_D2L], "
        "lines 6955/6963 route C07_WALL_D2R or C08_WALL_D2L through "
        "C710_ZONE_WALL_D2L, and the wall case returns after the F0107 "
        "alcove probes. ReDMCSB DUNVIEW.C:7051 "
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF is the D2R wall route; line "
        "7098 draws G0699_puc_Bitmap_WallSet_Wall_D2LCR with "
        "G0163_aauc_Graphic558_Frame_Walls[M605_VIEW_SQUARE_D2R], lines "
        "7106/7114 route C08_WALL_D2L or C07_WALL_D2R through "
        "C711_ZONE_WALL_D2R, and the wall case returns after F0107. "
        "ReDMCSB DUNVIEW.C:3048-3058 F0100_DUNGEONVIEW_DrawWallSetBitmap "
        "uses C10_COLOR_FLESH transparency. ReDMCSB DUNVIEW.C:3185-3204 "
        "F0105 gives row-flip parity; DUNVIEW.C:8390/8555 swaps and "
        "restores G0699_puc_Bitmap_WallSet_Wall_D2LCR for party-side flip. "
        "ReDMCSB DUNVIEW.C:581-591 G0163_aauc_Graphic558_Frame_Walls "
        "records M604/M605 frame ordinals; the synthetic side-wall slice "
        "contracts D2L viewport X 0..10/source X 61..71 and D2R viewport "
        "X 224..233/source X 0..9. ReDMCSB DUNVIEW.C:8318-8555 F0128 "
        "draw-order contract places D2L and D2R after center field setup. "
        "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2582-2583 M604/M605; "
        "DEFS.H:3430-3431 C07/C08 wall indices; DEFS.H:4050-4051 "
        "C710/C711 wall zones. F0122-style wall-return evidence is used "
        "only as route-shape evidence; this gate does not over-claim.";
}
