/*
 * theron_v22_shape_cache_pc34.h
 *
 * Theron V2.2 GPU render path: per-frame V22 shape cache.
 *
 * Theron has a 4x3 viewport (D0/D1/D2 x L/C/R), vs DM1's 4x3.
 * This cache wraps theron_v22_shape_for_cell + a minimal per-cell
 * material/wall/floor triple (analogous to DM1's DM1_V2_ShapeRuntimeResult)
 * so the in-place bitmap renderer can look up cell types by
 * (depth, lateral) without recomputing per draw pass.
 *
 * Source-lock: theron_v22_shapes.h (shape book),
 * THQUEST.ASM T400/T520/T600 (9-square viewport layout).
 */

#ifndef FIRESTAFF_Theron_V22_SHAPE_CACHE_PC34_H
#define FIRESTAFF_Theron_V22_SHAPE_CACHE_PC34_H

#include "theron_v22_shapes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Cell rectangle on the Theron 4x3 viewport (1920x1080 presentation
 * canvas). x/y is top-left, w/h is size. D0 is closest (bottom of
 * screen), D2 is farthest (top of screen). L=-1, C=0, R=+1. */
typedef struct {
    int x, y, w, h;
} Theron_V22_CellRect;

/* The 9-cell rect table (D0..D2 x L/C/R). Exposed for the in-place
 * renderer so the bitmap blit lands on the same screen coords as
 * the (future) overlay placeholder pass. */
extern const Theron_V22_CellRect theron_v22_kCellRects[3][3];

typedef struct {
    int active;                  /* 1 = V22 path, 0 = V1 path */
    Theron_V22_ShapeParams params;
    Theron_V22_WallShape  wall;
    Theron_V22_FloorShape floor;
    const Theron_V22_Material* material;
} Theron_V22_ShapeRuntimeResult;

/* Populate the cache from a 3x3 array of raw cell types
 * (D0..D2, L/C/R order). direction is the party facing (0..3).
 * When V22 is not the active mode, all cells are marked active=0. */
void theron_v22_shape_cache_update(int direction,
                                 const unsigned char raw_cells[3][3]);

/* Read a cached cell. depth in {0,1,2}, lateral in {-1,0,1}. */
const Theron_V22_ShapeRuntimeResult* theron_v22_shape_cache_get(int depth,
                                                         int lateral);

/* True if the V22 cell is active (V22 path) for (depth, lateral). */
int theron_v22_shape_cache_active(int depth, int lateral);

/* True if at least one cache_update has been called. */
int theron_v22_shape_cache_populated(void);

/* Source evidence for tests/probes. */
const char* theron_v22_shape_cache_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_Theron_V22_SHAPE_CACHE_PC34_H */
