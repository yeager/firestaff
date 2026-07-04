/*
 * m11_v22_cell_rects_pc34.c
 *
 * Shared DM1 V2.2 4x3 viewport-cell rectangles for the placeholder
 * overlay and in-place bitmap pass. Source-locked to the V1 viewport
 * composition order used by ReDMCSB DUNVIEW.C F0128/F0115; the V2.2
 * passes must agree on these rectangles so fallback and real-art paths
 * replace the same source-view cells.
 */
#include "m11_v22_render_overlay_pc34.h"

#include <stddef.h>

static const M11_V22_CellRect kV22CellRects[3][3] = {
    /* depth 0 = D1 (closest) */
    {
        {  8, 103, 69, 30 },
        { 78, 103, 61, 30 },
        {139, 103, 69, 30 }
    },
    /* depth 1 = D2 (middle) */
    {
        {  8,  72, 69, 30 },
        { 78,  72, 61, 30 },
        {139,  72, 69, 30 }
    },
    /* depth 2 = D3 (back) */
    {
        {  8,  41, 69, 30 },
        { 78,  41, 61, 30 },
        {139,  41, 69, 30 }
    }
};

const M11_V22_CellRect* m11_v22_cell_rect(int depth, int lateral) {
    if (depth < 1 || depth > 3 || lateral < -1 || lateral > 1) {
        return NULL;
    }
    return &kV22CellRects[depth - 1][lateral + 1];
}
