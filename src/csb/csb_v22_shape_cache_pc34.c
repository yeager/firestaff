/*
 * csb_v22_shape_cache_pc34.c
 *
 * Wraps csb_v22_shape_for_cell + provides per-cell material lookup.
 * Mirrors the DM1 m11_v22_shape_cache_pc34 API but sized for CSB's
 * 3x3 viewport (depth 0..2, lateral -1..+1 = 9 cells).
 */

#include "csb_v22_shape_cache_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"

#include <string.h>

static CSB_V22_ShapeRuntimeResult g_csb_cache[3][3];
static int g_csb_cache_raw[3][3];
static int g_csb_cache_populated = 0;

/* ReDMCSB PC34 viewport cells (D1..D3 x L/C/R) in the 320x200 source
 * framebuffer.  The in-place renderer and swap consumer must share these
 * coordinates; output resolution scaling is M11's later responsibility. */
const CSB_V22_CellRect csb_v22_kCellRects[3][3] = {
    /* depth 0 = D1 (closest) */ {
        {  8, 103, 69, 30 },
        { 78, 103, 61, 30 },
        {139, 103, 69, 30 }
    },
    /* depth 1 = D2 (middle) */ {
        {  8,  72, 69, 30 },
        { 78,  72, 61, 30 },
        {139,  72, 69, 30 }
    },
    /* depth 2 = D3 (back) */ {
        {  8,  41, 69, 30 },
        { 78,  41, 61, 30 },
        {139,  41, 69, 30 }
    }
};

void csb_v22_shape_cache_update(int direction,
                                 const unsigned char raw_cells[3][3]) {
    int d, l;
    const int v22_active = csb_v2_presentation_mode_is_v22();
    for (d = 0; d < 3; ++d) {
        for (l = -1; l <= 1; ++l) {
            int idx = l + 1;
            g_csb_cache_raw[d][idx] = (int)raw_cells[d][idx];
            if (!v22_active) {
                memset(&g_csb_cache[d][idx], 0, sizeof(g_csb_cache[d][idx]));
                continue;
            }
            CSB_V22_ShapeParams p = csb_v22_shape_for_cell(
                (int)raw_cells[d][idx], direction, d, l);
            g_csb_cache[d][idx].params = p;
            g_csb_cache[d][idx].wall = csb_v22_wall_shape_get(CSB_V22_WALL_VARIANT_D0_LEFT);
            g_csb_cache[d][idx].floor = csb_v22_floor_shape_get((int)raw_cells[d][idx], direction);
            g_csb_cache[d][idx].material = csb_v22_material_get(p.material_id);
            /* No product V2.2 cell may become drawable merely because a
             * synthetic shape book assigned it a material id.  The runtime
             * implementation returns NULL until source-derived material
             * binding has been reviewed. */
            g_csb_cache[d][idx].active = (g_csb_cache[d][idx].material != NULL);
        }
    }
    g_csb_cache_populated = 1;
}

const CSB_V22_ShapeRuntimeResult* csb_v22_shape_cache_get(int depth, int lateral) {
    if (depth < 0 || depth > 2) return NULL;
    if (lateral < -1 || lateral > 1) return NULL;
    return &g_csb_cache[depth][lateral + 1];
}

int csb_v22_shape_cache_active(int depth, int lateral) {
    const CSB_V22_ShapeRuntimeResult* r = csb_v22_shape_cache_get(depth, lateral);
    return (r && r->active) ? 1 : 0;
}

int csb_v22_shape_cache_populated(void) {
    return g_csb_cache_populated;
}

int csb_v22_shape_cache_get_raw_cell(int depth, int lateral) {
    if (depth < 0 || depth > 2) return -1;
    if (lateral < -1 || lateral > 1) return -1;
    if (!g_csb_cache_populated) return -1;
    return g_csb_cache_raw[depth][lateral + 1];
}

const char* csb_v22_shape_cache_source_evidence(void) {
    return "csb_v22_shapes.h (CSB V22 shape book); "
           "CSBWin/Viewport.cpp:7290 (9-square viewport layout); "
           "ReDMCSB DUNVIEW.C F0128 (CSB viewport routing).";
}
