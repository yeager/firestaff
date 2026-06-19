/*
 * csb_v22_shape_cache_pc34.c
 *
 * Wraps csb_v22_shape_for_cell + provides per-cell material lookup.
 * Mirrors the DM1 m11_v22_shape_cache_pc34 API but sized for CSB's
 * 3x3 viewport (depth 0..2, lateral -1..+1 = 9 cells).
 */

#include "csb_v22_shape_cache_pc34.h"

#include <string.h>

static CSB_V22_ShapeRuntimeResult g_csb_cache[3][3];
static int g_csb_cache_populated = 0;

/* 9-cell rect table (D0..D2 x L/C/R). At 1920x1080, each cell is
 * roughly 640x360 with the 4:3 dungeon viewport centered. These
 * are placeholder rectangles (for first cut) — they will be refined
 * to match CSBWin/Viewport.cpp:7290 exact coords once V1 draw
 * integration is in place. */
const CSB_V22_CellRect csb_v22_kCellRects[3][3] = {
    /* depth 0 = D0 (closest) */ {
        { 320, 720, 640, 360 },
        { 960, 720, 640, 360 },
        {1600, 720, 640, 360 }
    },
    /* depth 1 = D1 (middle) */ {
        { 320, 360, 640, 360 },
        { 960, 360, 640, 360 },
        {1600, 360, 640, 360 }
    },
    /* depth 2 = D2 (farthest) */ {
        { 320,   0, 640, 360 },
        { 960,   0, 640, 360 },
        {1600,   0, 640, 360 }
    }
};

void csb_v22_shape_cache_update(int direction,
                                 const unsigned char raw_cells[3][3]) {
    int d, l;
    for (d = 0; d < 3; ++d) {
        for (l = -1; l <= 1; ++l) {
            int idx = l + 1;
            CSB_V22_ShapeParams p = csb_v22_shape_for_cell(
                (int)raw_cells[d][idx], direction, d, l);
            g_csb_cache[d][idx].active = 1;
            g_csb_cache[d][idx].params = p;
            g_csb_cache[d][idx].wall = csb_v22_wall_shape_get(CSB_V22_WALL_VARIANT_D0_LEFT);
            g_csb_cache[d][idx].floor = csb_v22_floor_shape_get((int)raw_cells[d][idx], direction);
            g_csb_cache[d][idx].material = csb_v22_material_get(p.material_id);
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

const char* csb_v22_shape_cache_source_evidence(void) {
    return "csb_v22_shapes.h (CSB V22 shape book); "
           "CSBWin/Viewport.cpp:7290 (9-square viewport layout); "
           "ReDMCSB DUNVIEW.C F0128 (CSB viewport routing).";
}
