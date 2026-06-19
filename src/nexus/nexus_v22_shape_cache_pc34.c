/*
 * nexus_v22_shape_cache_pc34.c
 *
 * Minimal Nexus V22 per-cell cache. Stores raw_cell_type + direction
 * per (depth, lateral). No shape variant resolution (Nexus has no
 * full shape book yet; follow-up).
 */

#include "nexus_v22_shape_cache_pc34.h"

#include <string.h>

static Nexus_V22_ShapeRuntimeResult g_nexus_cache[3][3];
static int g_nexus_cache_populated = 0;

const NEXUS_V22_CellRect nexus_v22_kCellRects[3][3] = {
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

void nexus_v22_shape_cache_update(int direction,
                                  const unsigned char raw_cells[3][3]) {
    int d, l;
    for (d = 0; d < 3; ++d) {
        for (l = -1; l <= 1; ++l) {
            int idx = l + 1;
            g_nexus_cache[d][idx].active = 1;
            g_nexus_cache[d][idx].raw_cell_type = raw_cells[d][idx];
            g_nexus_cache[d][idx].direction = (uint8_t)direction;
            g_nexus_cache[d][idx].reserved[0] = 0;
            g_nexus_cache[d][idx].reserved[1] = 0;
        }
    }
    g_nexus_cache_populated = 1;
}

const Nexus_V22_ShapeRuntimeResult* nexus_v22_shape_cache_get(int depth, int lateral) {
    if (depth < 0 || depth > 2) return NULL;
    if (lateral < -1 || lateral > 1) return NULL;
    return &g_nexus_cache[depth][lateral + 1];
}

int nexus_v22_shape_cache_active(int depth, int lateral) {
    const Nexus_V22_ShapeRuntimeResult* r = nexus_v22_shape_cache_get(depth, lateral);
    return (r && r->active) ? 1 : 0;
}

int nexus_v22_shape_cache_populated(void) {
    return g_nexus_cache_populated;
}

const char* nexus_v22_shape_cache_source_evidence(void) {
    return "nexus_v22_modern_assets_pc34.c (sibling asset module); "
           "include/nexus_v22_modern_assets_pc34.h (asset discovery + path); "
           "include/nexus_v1_iso_reader.h (raw cell type source); "
           "SATURN_DMDF T400/T600 (Saturn dungeon viewport routing); "
           "ReDMCSB DUNVIEW.C:6697-6816 (composition order pattern).";
}
