#ifndef FIRESTAFF_M11_V22_SHAPE_CACHE_PC34_H
#define FIRESTAFF_M11_V22_SHAPE_CACHE_PC34_H

/*
 * m11_v22_shape_cache_pc34.h
 *
 * DM1 V2.2 GPU render path: per-frame V22 shape cache.
 *
 * This is the data-flow seam between the V22 shape book
 * (m11_v22_shape_for_cell) and the M11 game view per-cell draw
 * passes. The cache is populated once per m11_draw_viewport frame
 * and consulted by the per-cell passes to read the V22 shape.
 *
 * Source-lock: include/dm1_v2_shape_runtime_pc34.h + ReDMCSB
 * DUNVIEW.C:6697-6816 + DUNGEON.C:2238-2246.
 *
 * Module: src/dm1v2/m11_v22_shape_cache_pc34.c
 * Test:   tests/test_m11_v22_shape_cache_pc34.c
 * Probe:  probes/firestaff_m11_v22_shape_cache_probe.c
 */

#include <stdint.h>
#include "dm1_v2_shape_runtime_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Populate the cache from a 3x3 array of raw square bytes
 * (D1..D3, L/C/R order). direction is the party facing (0..3).
 * When V22 is not the active mode, all cells are marked active=0. */
void m11_v22_shape_cache_update(int direction,
                                const unsigned char raw_squares[3][3]);

/* Read a cached cell. depth in {1,2,3}, lateral in {-1,0,1}.
 * Returns NULL if the depth/lateral is out of range or the cache
 * has not been populated yet. */
const DM1_V2_ShapeRuntimeResult* m11_v22_shape_cache_get(int depth,
                                                        int lateral);

/* Convenience: returns 1 if the V22 cell is active, 0 otherwise. */
int m11_v22_shape_cache_active(int depth, int lateral);

/* True if at least one cache_update has been called. */
int m11_v22_shape_cache_populated(void);

/* Source evidence for tests/probes. */
const char* m11_v22_shape_cache_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_V22_SHAPE_CACHE_PC34_H */
