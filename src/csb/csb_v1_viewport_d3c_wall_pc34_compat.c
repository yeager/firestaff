#include "csb_v1_viewport_d3c_wall_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_M600_VIEW_SQUARE_D3C = 11,       /* ReDMCSB: DEFS.H:2607 CSB/I34 M600. */
    CSB_M600_G0163_FRAME_ORDINAL = 0,    /* ReDMCSB: DUNVIEW.C:583 G0163 D3C row. */
    CSB_D3C_VIEW_DEPTH = 3,              /* ReDMCSB: DUNVIEW.C:371-377 G2027. */
    CSB_D3C_VIEW_LANE = 0,               /* ReDMCSB: DUNVIEW.C:371-377 G2026. */
    CSB_D3C_FRAME_X1 = 74,               /* ReDMCSB: DUNVIEW.C:583 G0163 D3C X1. */
    CSB_D3C_FRAME_X2 = 149,              /* ReDMCSB: DUNVIEW.C:583 G0163 D3C X2. */
    CSB_D3C_FRAME_Y1 = 25,               /* ReDMCSB: DUNVIEW.C:583 G0163 D3C Y1. */
    CSB_D3C_FRAME_Y2 = 75,               /* ReDMCSB: DUNVIEW.C:583 G0163 D3C Y2. */
    CSB_D3C_FRAME_BYTE_WIDTH = 64,       /* ReDMCSB: DUNVIEW.C:583 G0163 C4. */
    CSB_D3C_FRAME_HEIGHT = 51,           /* ReDMCSB: DUNVIEW.C:583 G0163 C5. */
    CSB_D3C_FRAME_SOURCE_X = 18,         /* ReDMCSB: DUNVIEW.C:583 G0163 C6. */
    CSB_D3C_FRAME_SOURCE_Y = 0,          /* ReDMCSB: DUNVIEW.C:583 G0163 C7. */
    CSB_D3C_EFFECTIVE_SOURCE_X2 = 63,
    CSB_D3C_EFFECTIVE_VIEWPORT_X2 = 119,
    CSB_C112_BYTE_WIDTH_VIEWPORT = 112,  /* ReDMCSB: DEFS.H:2478. */
    CSB_C10_COLOR_FLESH = 10,            /* ReDMCSB: DEFS.H:2088. */
    CSB_CM1_COLOR_NO_TRANSPARENCY = -1
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; no real-asset pixel parity is claimed. "
    "ReDMCSB DUNVIEW.C:6642-6720 F0118_DUNGEONVIEW_DrawSquareD3C_CPSF "
    "selects the D3C wall branch for C00_ELEMENT_WALL. Lines 6699-6702 "
    "show the wall-set route: legacy F0100_DUNGEONVIEW_DrawWallSetBitmap "
    "and the center-wall F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency "
    "both use G0698_puc_Bitmap_WallSet_Wall_D3LCR and "
    "G0163_aauc_Graphic558_Frame_Walls[M600_VIEW_SQUARE_D3C]. "
    "ReDMCSB DUNVIEW.C:3048-3058 F0100 preserves C10_COLOR_FLESH as "
    "transparent; DUNVIEW.C:3065-3078 F0101 uses CM1_COLOR_NO_TRANSPARENCY "
    "for the optimized center-wall route. DUNVIEW.C:581-583 G0163 gives "
    "M600 D3C frame ordinal 0 as {74,149,25,75,64,51,18,0}; copied "
    "through the row clip, source X 18..63 maps to viewport X 74..119. "
    "DUNVIEW.C:6716-6720 calls F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF "
    "and returns when no alcove path is active. This is the wall branch "
    "only and rejects the D3C no-wall center-field slice and the D2C "
    "F0121/F0101 center-wall route. CSB-lineage Viewport.cpp:1903-1915 "
    "is the requested center dispatch cross-reference; Viewport.cpp:1824-1835 "
    "and 2057-2065 keep the F3/D3C door/wall slot distinct, while "
    "Viewport.cpp:6972-6984 dispatches relative (3,0) to RF3/DrawCellF3.";

static const CSB_V1_D3CWallSpecPc34 s_spec = {
    true,
    false,
    CSB_M600_VIEW_SQUARE_D3C,
    CSB_M600_G0163_FRAME_ORDINAL,
    CSB_D3C_VIEW_DEPTH,
    CSB_D3C_VIEW_LANE,
    {
        CSB_D3C_FRAME_X1,
        CSB_D3C_FRAME_X2,
        CSB_D3C_FRAME_Y1,
        CSB_D3C_FRAME_Y2,
        CSB_D3C_FRAME_BYTE_WIDTH,
        CSB_D3C_FRAME_HEIGHT,
        CSB_D3C_FRAME_SOURCE_X,
        CSB_D3C_FRAME_SOURCE_Y
    },
    CSB_D3C_FRAME_SOURCE_X,
    CSB_D3C_EFFECTIVE_SOURCE_X2,
    CSB_D3C_FRAME_X1,
    CSB_D3C_EFFECTIVE_VIEWPORT_X2,
    (CSB_D3C_EFFECTIVE_SOURCE_X2 - CSB_D3C_FRAME_SOURCE_X) + 1,
    CSB_D3C_FRAME_HEIGHT,
    CSB_C112_BYTE_WIDTH_VIEWPORT,
    CSB_C10_COLOR_FLESH,
    CSB_CM1_COLOR_NO_TRANSPARENCY,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_ABSENT,
    CSB_ABSENT,
    CSB_ABSENT,
    CSB_ABSENT,
    "ReDMCSB DUNVIEW.C:6642-6720 F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
    "ReDMCSB DUNVIEW.C:3048-3058 F0100_DUNGEONVIEW_DrawWallSetBitmap",
    "ReDMCSB DUNVIEW.C:3065-3078 F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency",
    "ReDMCSB DUNVIEW.C:581-583 G0163_aauc_Graphic558_Frame_Walls[M600_VIEW_SQUARE_D3C]",
    "ReDMCSB DEFS.H:5419 G0698_puc_Bitmap_WallSet_Wall_D3LCR; DUNVIEW.C:6699-6702",
    "CSB-lineage Viewport.cpp:1903-1915 requested; D3C/F3 cross-check 1824-1835,2057-2065,6972-6984",
    s_source_evidence
};

const CSB_V1_D3CWallSpecPc34 *
csb_v1_viewport_d3c_wall_spec_pc34(void)
{
    return &s_spec;
}

uint8_t csb_v1_viewport_d3c_wall_blend_f0100_transparent_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

uint8_t csb_v1_viewport_d3c_wall_blend_f0101_no_transparency_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    (void)destination_pixel;
    return source_pixel;
}

const char *csb_v1_viewport_d3c_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
