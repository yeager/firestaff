/*
 * m11_v22_shape_cache_pc34.c
 *
 * DM1 V2.2 per-frame V22 shape cache. Extracted from m11_game_view.c
 * to keep the data-flow seam (V22 shape resolution + per-frame
 * caching) in its own module so it can be tested without pulling
 * in the full M11 game view + image frontend.
 *
 * Contract:
 *   The cache holds the resolved V22 shape (params, wall, floor,
 *   material) for each of the 9 sampled cells (D1..D3, L/C/R).
 *   When V22 is not the active mode, all entries are marked
 *   active=0 and the V1 renderer falls through to its existing
 *   selection.
 *
 *   The cache is module-static (single-threaded by design). The
 *   M11 draw path calls m11_v22_shape_cache_update once per frame
 *   after sampling the viewport cells, then consults the cache via
 *   m11_v22_shape_cache_get(depth, lateral) from the per-cell draw
 *   passes.
 *
 *   The actual swap of draw primitives (replacing the V1
 *   GRAPHICS.DAT bitmap blit with a V22 modern art draw call) is
 *   a follow-up; this commit adds the data flow so the per-cell
 *   pass has the V22 params available when the swap is implemented.
 *
 * Source-lock: ReDMCSB DUNVIEW.C:6697-6816 (composition draw order).
 */
#include "m11_v22_shape_cache_pc34.h"
#include "dm1_v2_shape_runtime_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"

#include <string.h>

static DM1_V2_ShapeRuntimeResult s_v22_shape_cache[3][3];  /* [depth 0..2][side 0..2] = [D1..D3][L/C/R] */
static int s_v22_shape_cache_populated = 0;

void m11_v22_shape_cache_update(int direction,
                                const unsigned char raw_squares[3][3]) {
    int d, s;
    if (!raw_squares) return;
    s_v22_shape_cache_populated = 1;
    for (d = 0; d < 3; ++d) {
        for (s = 0; s < 3; ++s) {
            DM1_V2_ShapeRuntimeResult* r = &s_v22_shape_cache[d][s];
            memset(r, 0, sizeof(*r));
            if (!dm1_v2_shape_runtime_v22_active()) {
                r->active = 0;  /* V1 path */
                continue;
            }
            *r = dm1_v2_shape_runtime_for_cell(
                (int)raw_squares[d][s],
                direction,
                d + 1,         /* relForward: D1..D3 */
                s - 1);        /* relSide: -1..+1 */
        }
    }
}

const DM1_V2_ShapeRuntimeResult* m11_v22_shape_cache_get(int depth, int lateral) {
    if (depth < 1 || depth > 3) return NULL;
    if (lateral < -1 || lateral > 1) return NULL;
    if (!s_v22_shape_cache_populated) return NULL;
    return &s_v22_shape_cache[depth - 1][lateral + 1];
}

int m11_v22_shape_cache_active(int depth, int lateral) {
    const DM1_V2_ShapeRuntimeResult* r = m11_v22_shape_cache_get(depth, lateral);
    return (r != NULL) ? r->active : 0;
}

int m11_v22_shape_cache_populated(void) {
    return s_v22_shape_cache_populated;
}

const char* m11_v22_shape_cache_source_evidence(void) {
    return
        "V2.2 GPU render path: per-frame V22 shape cache (m11_v22_shape_cache).\n"
        "  Module-static s_v22_shape_cache[3][3] = [D1..D3][L/C/R] resolved\n"
        "  once per m11_draw_viewport frame via dm1_v2_shape_runtime_for_cell.\n"
        "  V1 path: all cells active=0 (V1 draw primitives unchanged).\n"
        "  V22 path: cell.params + cell.wall + cell.floor + cell.material\n"
        "  available via m11_v22_shape_cache_get(depth, lateral).\n"
        "  Source: include/dm1_v2_shape_runtime_pc34.h + ReDMCSB DUNVIEW.C:6697-6816\n"
        "  + DUNGEON.C:2238-2246 + DEFS.H:922 M034_SQUARE_TYPE.\n";
}
