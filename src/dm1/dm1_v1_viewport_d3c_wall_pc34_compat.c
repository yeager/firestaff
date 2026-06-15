#include "dm1_v1_viewport_d3c_wall_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_VIEW_SQUARE_D3C_PC34 = 0,       /* ReDMCSB DEFS.H:2578 M600_VIEW_SQUARE_D3C */
    DM1_V1_D3C_FRAME_X1_PC34 = 74,         /* ReDMCSB DUNVIEW.C:583 G0163 D3C X1 */
    DM1_V1_D3C_FRAME_X2_PC34 = 149,        /* ReDMCSB DUNVIEW.C:583 G0163 D3C X2 */
    DM1_V1_D3C_FRAME_Y1_PC34 = 25,         /* ReDMCSB DUNVIEW.C:583 G0163 D3C Y1 */
    DM1_V1_D3C_FRAME_Y2_PC34 = 75,         /* ReDMCSB DUNVIEW.C:583 G0163 D3C Y2 */
    DM1_V1_D3C_FRAME_BYTE_WIDTH_PC34 = 64, /* ReDMCSB DUNVIEW.C:583 G0163 D3C C4 */
    DM1_V1_D3C_FRAME_HEIGHT_PC34 = 51,     /* ReDMCSB DUNVIEW.C:583 G0163 D3C C5 */
    DM1_V1_D3C_FRAME_SOURCE_X_PC34 = 18,   /* ReDMCSB DUNVIEW.C:583 G0163 D3C C6 */
    DM1_V1_D3C_FRAME_SOURCE_Y_PC34 = 0,    /* ReDMCSB DUNVIEW.C:583 G0163 D3C C7 */
    DM1_V1_M552_FRONT_ORNAMENT_PC34 = 3,   /* ReDMCSB DEFS.H:2538 */
    DM1_V1_M578_VIEW_WALL_D3C_FRONT_PC34 = 3,
    DM1_V1_CELL_ORDER_ALCOVE_PC34 = 0
};

static const DM1_V1_D3CWallSpecPc34 s_spec = {
    true,
    false,
    DM1_V1_VIEW_SQUARE_D3C_PC34,
    DM1_V1_VIEW_SQUARE_D3C_PC34,
    DM1_V1_D3C_FRAME_X1_PC34,
    DM1_V1_D3C_FRAME_X2_PC34,
    DM1_V1_D3C_FRAME_Y1_PC34,
    DM1_V1_D3C_FRAME_Y2_PC34,
    DM1_V1_D3C_FRAME_BYTE_WIDTH_PC34,
    DM1_V1_D3C_FRAME_HEIGHT_PC34,
    DM1_V1_D3C_FRAME_SOURCE_X_PC34,
    DM1_V1_D3C_FRAME_SOURCE_Y_PC34,
    DM1_V1_D3C_FRAME_BYTE_WIDTH_PC34,
    DM1_V1_D3C_WALL_C112_BYTE_WIDTH_VIEWPORT_PC34,
    DM1_V1_D3C_WALL_C10_COLOR_FLESH_PC34,
    DM1_V1_D3C_WALL_SOURCE_WIDTH_PC34,
    DM1_V1_D3C_WALL_SOURCE_HEIGHT_PC34,
    DM1_V1_D3C_FRAME_SOURCE_X_PC34,
    DM1_V1_D3C_FRAME_SOURCE_X_PC34 + (DM1_V1_D3C_FRAME_X2_PC34 - DM1_V1_D3C_FRAME_X1_PC34),
    DM1_V1_D3C_FRAME_SOURCE_Y_PC34,
    DM1_V1_D3C_FRAME_SOURCE_Y_PC34 + DM1_V1_D3C_FRAME_HEIGHT_PC34 - 1,
    112,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    DM1_V1_M552_FRONT_ORNAMENT_PC34,
    DM1_V1_M578_VIEW_WALL_D3C_FRONT_PC34,
    DM1_V1_CELL_ORDER_ALCOVE_PC34,
    false,
    false,
    false,
    false,
    "G0698_puc_Bitmap_WallSet_Wall_D3LCR",
    "G0163_aauc_Graphic558_Frame_Walls[M600_VIEW_SQUARE_D3C]",
    NULL
};

static bool element_draws_d3c_wall_pixels(DM1_V1_D3CWallElementPc34 element)
{
    return element == DM1_V1_D3C_ELEMENT_WALL_PC34;
}

static bool in_frame_clip(const DM1_V1_D3CWallSpecPc34 *spec, int row, int x)
{
    return row >= spec->frame_y1 && row <= spec->frame_y2 &&
        x >= spec->frame_x1 && x <= spec->frame_x2;
}

const DM1_V1_D3CWallSpecPc34 *dm1_v1_viewport_d3c_wall_spec_pc34(void)
{
    static DM1_V1_D3CWallSpecPc34 spec;
    spec = s_spec;
    spec.source_lock_evidence =
        dm1_v1_viewport_d3c_wall_source_evidence_pc34();
    return &spec;
}

bool dm1_v1_viewport_d3c_wall_apply_pixel_pc34(
    const DM1_V1_D3CWallPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D3CWallPixelResultPc34 *out)
{
    const DM1_V1_D3CWallSpecPc34 *spec;
    uint8_t transparent_color;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d3c_wall_spec_pc34();
    out->spec = *spec;
    out->source_lock_evidence = spec->source_lock_evidence;
    out->alcove_cell_order = -1;
    if (!input) return false;

    out->row = input->row;
    out->viewport_x = input->viewport_x;
    out->element_is_wall = element_draws_d3c_wall_pixels(input->element);
    out->draws_d3c_wall_pixels = out->element_is_wall;
    out->calls_f0107 = out->element_is_wall;
    out->f0107_alcove_result = input->f0107_alcove_result;
    out->f0107_ornament_ordinal_arg = out->element_is_wall ?
        spec->f0107_front_wall_ornament_ordinal : -1;
    out->f0107_view_wall_arg = out->element_is_wall ?
        spec->f0107_view_wall_index : -1;
    out->f0107_alcove_return_path =
        out->element_is_wall && input->f0107_alcove_result;
    out->returns_after_wall_blit =
        out->element_is_wall && !input->f0107_alcove_result;
    if (out->f0107_alcove_return_path) {
        out->alcove_cell_order = spec->f0107_alcove_cell_order;
    }
    if (!out->element_is_wall) {
        out->no_write_metadata = true;
        return true;
    }

    transparent_color = input->transparent_color;
    if (transparent_color == 0) {
        transparent_color = DM1_V1_D3C_WALL_C10_COLOR_FLESH_PC34;
    }
    out->spec.transparent_color = transparent_color;
    if (!in_frame_clip(spec, input->row, input->viewport_x)) {
        out->no_write_metadata = true;
        return true;
    }
    if (!source || !viewport) return false;

    out->in_clip = true;
    out->source_x = spec->frame_source_x + (input->viewport_x - spec->frame_x1);
    out->source_y = spec->frame_source_y + (input->row - spec->frame_y1);
    out->source_offset = (size_t)out->source_y *
        (size_t)DM1_V1_D3C_WALL_SOURCE_WIDTH_PC34 + (size_t)out->source_x;
    out->viewport_offset = (size_t)input->row *
        (size_t)DM1_V1_D3C_WALL_VIEWPORT_WIDTH_PC34 + (size_t)input->viewport_x;
    if (out->source_offset >= source_len || out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip = out->source_pixel == transparent_color;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] = dm1_v1_viewport_d3c_wall_blend_pixel_pc34(
        viewport[out->viewport_offset], out->source_pixel, transparent_color);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

uint8_t dm1_v1_viewport_d3c_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_d3c_wall_source_evidence_pc34(void)
{
    return
        "Source-locked contract gate only; no real-asset pixel parity is "
        "claimed. DUNVIEW.C F0118 C00_ELEMENT_WALL path: lines 6642-6720 "
        "dispatch D3C, line 6699 calls F0100_DUNGEONVIEW_DrawWallSetBitmap "
        "with G0698_puc_Bitmap_WallSet_Wall_D3LCR and "
        "G0163_aauc_Graphic558_Frame_Walls[M600_VIEW_SQUARE_D3C], line "
        "6716 calls F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF, "
        "line 6717 selects C0x0000_CELL_ORDER_ALCOVE, and line 6720 returns "
        "when the alcove probe is false. F0107 C00_ELEMENT_WALL alcove "
        "return. DUNVIEW.C:3048-3058 F0100 passes C10_COLOR_FLESH, "
        "P0106_puc_Frame[C4_BYTE_WIDTH], C112_BYTE_WIDTH_VIEWPORT, and "
        "P0106_puc_Frame[C5_HEIGHT] to the blit. DUNVIEW.C:581-583 G0163 "
        "resolves M600_VIEW_SQUARE_D3C to {74,149,25,75,64,51,18,0}, a "
        "center D3 viewport column. DEFS.H:1009 C02_ELEMENT_PIT; "
        "DEFS.H:1015-1017 C17/C18/C19; DEFS.H:2088 C10_COLOR_FLESH; "
        "DEFS.H:2478 C112_BYTE_WIDTH_VIEWPORT; DEFS.H:2534 C0_ELEMENT; "
        "DEFS.H:2538 M552_FRONT_WALL_ORNAMENT_ORDINAL; DEFS.H:2578 "
        "M600_VIEW_SQUARE_D3C=0; DEFS.H:2684 M578_VIEW_WALL_D3C_FRONT=3.";
}
