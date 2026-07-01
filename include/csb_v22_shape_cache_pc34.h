/*
 * csb_v22_shape_cache_pc34.h
 *
 * CSB V2.2 GPU render path: per-frame V22 shape cache.
 *
 * CSB has a 3x3 viewport (D0/D1/D2 x L/C/R), vs DM1's 4x3.
 * This cache wraps csb_v22_shape_for_cell + a minimal per-cell
 * material/wall/floor triple (analogous to DM1's DM1_V2_ShapeRuntimeResult)
 * so the in-place bitmap renderer can look up cell types by
 * (depth, lateral) without recomputing per draw pass.
 *
 * Source-lock: csb_v22_shapes.h (shape book),
 * CSBWin/Viewport.cpp:7290 (9-square viewport layout).
 */

#ifndef FIRESTAFF_CSB_V22_SHAPE_CACHE_PC34_H
#define FIRESTAFF_CSB_V22_SHAPE_CACHE_PC34_H

#include "csb_v22_shapes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Cell rectangle on the CSB 3x3 viewport (1920x1080 presentation
 * canvas). x/y is top-left, w/h is size. D0 is closest (bottom of
 * screen), D2 is farthest (top of screen). L=-1, C=0, R=+1. */
typedef struct {
    int x, y, w, h;
} CSB_V22_CellRect;

/* The 9-cell rect table (D0..D2 x L/C/R). Exposed for the in-place
 * renderer so the bitmap blit lands on the same screen coords as
 * the (future) overlay placeholder pass. */
extern const CSB_V22_CellRect csb_v22_kCellRects[3][3];

typedef struct {
    int active;                  /* 1 = V22 path, 0 = V1 path */
    CSB_V22_ShapeParams params;
    CSB_V22_WallShape  wall;
    CSB_V22_FloorShape floor;
    const CSB_V22_Material* material;
} CSB_V22_ShapeRuntimeResult;

/* Populate the cache from a 3x3 array of raw cell types
 * (D0..D2, L/C/R order). direction is the party facing (0..3).
 * When V22 is not the active mode, all cells are marked active=0. */
void csb_v22_shape_cache_update(int direction,
                                 const unsigned char raw_cells[3][3]);

/* Read a cached cell. depth in {0,1,2}, lateral in {-1,0,1}. */
const CSB_V22_ShapeRuntimeResult* csb_v22_shape_cache_get(int depth,
                                                         int lateral);

/* True if the V22 cell is active (V22 path) for (depth, lateral). */
int csb_v22_shape_cache_active(int depth, int lateral);

/* True if at least one cache_update has been called. */
int csb_v22_shape_cache_populated(void);

/* Read the raw M034 cell type that was last supplied to
 * csb_v22_shape_cache_update for the given (depth, lateral) cell.
 * Returns -1 if the cache has not been populated yet or the
 * coords are out of range. The per-cell modern-art routing gate
 * (csb_v22_inplace_route_cell) re-decodes this raw cell type to
 * its shape type internally; the route gate's contract is
 * "give me a raw cell type" so callers do not need to consult
 * the shape book first. */
int csb_v22_shape_cache_get_raw_cell(int depth, int lateral);

/* Source evidence for tests/probes. */
const char* csb_v22_shape_cache_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V22_SHAPE_CACHE_PC34_H */
