/*
 * dm2_v22_shape_cache_pc34.c
 *
 * Minimal DM2 V22 per-cell cache. Stores raw_cell_type + direction
 * per (depth, lateral). No shape variant resolution (DM2 has no
 * full shape book yet; follow-up).
 */

#include "dm2_v22_shape_cache_pc34.h"

#include <string.h>

static Dm2_V22_ShapeRuntimeResult g_dm2_cache[3][3];
static int g_dm2_cache_populated = 0;

const DM2_V22_CellRect dm2_v22_kCellRects[3][3] = {
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

void dm2_v22_shape_cache_update(int direction,
                                  const unsigned char raw_cells[3][3]) {
    int d, l;
    for (d = 0; d < 3; ++d) {
        for (l = -1; l <= 1; ++l) {
            int idx = l + 1;
            g_dm2_cache[d][idx].active = 1;
            g_dm2_cache[d][idx].raw_cell_type = raw_cells[d][idx];
            g_dm2_cache[d][idx].direction = (uint8_t)direction;
            g_dm2_cache[d][idx].reserved[0] = 0;
            g_dm2_cache[d][idx].reserved[1] = 0;
        }
    }
    g_dm2_cache_populated = 1;
}

const Dm2_V22_ShapeRuntimeResult* dm2_v22_shape_cache_get(int depth, int lateral) {
    if (depth < 0 || depth > 2) return NULL;
    if (lateral < -1 || lateral > 1) return NULL;
    return &g_dm2_cache[depth][lateral + 1];
}

int dm2_v22_shape_cache_active(int depth, int lateral) {
    const Dm2_V22_ShapeRuntimeResult* r = dm2_v22_shape_cache_get(depth, lateral);
    return (r && r->active) ? 1 : 0;
}

int dm2_v22_shape_cache_populated(void) {
    return g_dm2_cache_populated;
}

const char* dm2_v22_shape_cache_source_evidence(void) {
    return "dm2_v22_modern_assets_pc34.c (sibling asset module); "
           "include/dm2_v22_modern_assets_pc34.h (asset discovery + path); "
           "include/dm2_v1_weather.c (raw cell type source); "
           "SKULL.ASM T520/T560/T600 (DM2 dungeon viewport (indoor T560 + outdoor T600) routing); "
           "ReDMCSB DUNVIEW.C:6697-6816 (composition order pattern).";
}
