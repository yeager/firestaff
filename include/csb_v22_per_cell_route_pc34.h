/*
 * csb_v22_per_cell_route_pc34.h
 *
 * CSB V2.2 per-cell modern-art routing gate.
 *
 * Maps (shape_type, depth, lateral, direction) onto a distinct
 * (asset_id, category, mirror_variant, rotation_deg) tuple so that
 * each of the 9 cells in CSB's 3x3 viewport can pick a different
 * modern-art asset when the V22 in-place renderer paints it.
 *
 * The 9-square viewport (CSBWin/Viewport.cpp:7290):
 *
 *   depth D2 (farthest):  L  C  R
 *   depth D1 (middle):    L  C  R
 *   depth D0 (closest):   L  C  R
 *
 * The previous (first-cut) V22 routing in csb_v22_inplace_draw_pc34.c
 * collapsed every shape into one of three buckets:
 *   - any wall shape  -> wall_dungeon_01
 *   - any floor shape -> floor_plain_01 or floor_cracked_01
 *   - any creature    -> creature_demon_01
 * That meant every cell in a dungeon viewport drew the same wall PNG,
 * regardless of depth (D0 vs D1 vs D2), lateral (L vs C vs R), or
 * even shape sub-variant (WALL_STRAIGHT vs WALL_CORNER_INNER vs
 * WALL_DOORWAY vs WALL_ALCOVE vs WALL_INSCRIPTION, etc.). The result
 * was a 9-cell grid that all looked identical under V22.
 *
 * This module introduces the per-cell modern-art swap: every
 * (shape_type, depth, lateral) combination is mapped to its own
 * (asset_id, category) pair, and direction feeds a (mirror_variant,
 * rotation_deg) pair that the GPU renderer can use to flip/rotate
 * the bitmap at blit time. The asset_id scheme is intentionally
 * prefix-stable ("<category>_<sub>_<depth>_<lateral>") so the
 * existing v22_inplace_cache.bin hash lookup keeps working when
 * the pack author only stages a subset of the cells.
 *
 * Source-lock:
 *   - CSBWin/Viewport.cpp:7290 (9-square viewport layout)
 *   - CSBWin/Chaos.cpp:60-69   (chaos rune shape, CSB-only)
 *   - ReDMCSB DUNGEON.C:35-44  (direction step tables N/E/S/W)
 *   - ReDMCSB DEFS.H:922       (M034_SQUARE_TYPE cell decode)
 *   - ReDMCSB DUNVIEW.C F0128  (CSB viewport routing)
 *   - include/csb_v22_shapes.h (shape type enum used here verbatim)
 *   - include/csb_v22_shape_cache_pc34.h (per-cell V22 cache)
 *   - include/csb_v22_inplace_draw_pc34.h (consumer / sibling)
 *
 * Module: src/csb/csb_v22_per_cell_route_pc34.c
 * Test:   tests/test_csb_v22_per_cell_route_pc34.c
 * Probe:  probes/firestaff_csb_v22_per_cell_route_probe.c
 */

#ifndef FIRESTAFF_CSB_V22_PER_CELL_ROUTE_PC34_H
#define FIRESTAFF_CSB_V22_PER_CELL_ROUTE_PC34_H

#include "csb_v22_shapes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Mirror variants — orthogonal X/Y flip on top of base sprite. */
typedef enum {
    CSB_V22_PER_CELL_MIRROR_NONE  = 0,
    CSB_V22_PER_CELL_MIRROR_X     = 1,  /* left/right mirror */
    CSB_V22_PER_CELL_MIRROR_Y     = 2,  /* up/down mirror */
    CSB_V22_PER_CELL_MIRROR_BOTH  = 3
} CSB_V22_PerCellMirror;

/* Per-cell route — the seam between the V22 shape book and the
 * modern-art asset pack. The renderer consults asset_id + category
 * to fetch a bitmap from the v22_inplace_cache.bin hash table, then
 * applies (mirror_variant, rotation_deg) at blit time. */
typedef struct {
    const char* asset_id;          /* e.g. "wall_dungeon_01_d0_left" */
    const char* category;          /* e.g. "wall_shapes" */
    int         mirror_variant;    /* CSB_V22_PerCellMirror */
    int         rotation_deg;      /* 0/90/180/270 */
    int         depth;             /* echoed: 0..2 */
    int         lateral;           /* echoed: -1/0/+1 */
    int         shape_type;        /* echoed: CSB_V22_ShapeType */
    int         direction;         /* echoed: 0..3 */
} CSB_V22_PerCellRoute;

/* Initialize the routing table. Currently stateless, but keeps the
 * same shape as the rest of the V22 modules so a follow-up that
 * loads per-cell manifest entries from disk can hang off this. */
void csb_v22_per_cell_route_init(void);

/* Reset the routing table (test/probe isolation helper). */
void csb_v22_per_cell_route_reset(void);

/* Route lookup for a single cell.
 *   shape_type: CSB_V22_ShapeType (cast to int)
 *   depth:      0..2 (D0 closest, D2 farthest)
 *   lateral:   -1/0/+1 (L/C/R)
 *   direction:  0..3 (N/E/S/W; affects mirror_variant + rotation)
 *
 * Returns a pointer to a static route on success; never NULL except
 * for out-of-range shape_type/depth/lateral. The pointer remains
 * valid for the program lifetime. */
const CSB_V22_PerCellRoute* csb_v22_per_cell_route_for_cell(int shape_type,
                                                              int depth,
                                                              int lateral,
                                                              int direction);

/* Total number of distinct (asset_id, category) pairs the routing
 * table can produce. Used by tests to lock the table size and by
 * tooling to size the asset-pack stage budget. */
int csb_v22_per_cell_route_distinct_asset_count(void);

/* Asset_id string for a single (shape_type, depth, lateral). This is
 * the same string the route lookup returns, exposed for callers that
 * already have the shape_type from the shape cache and only need the
 * asset_id (no direction-dependent mirror/rotation). */
const char* csb_v22_per_cell_route_asset_id(int shape_type,
                                             int depth,
                                             int lateral);

/* Category string for a single (shape_type, depth, lateral). */
const char* csb_v22_per_cell_route_category(int shape_type,
                                              int depth,
                                              int lateral);

/* Source evidence for tests/probes. */
const char* csb_v22_per_cell_route_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V22_PER_CELL_ROUTE_PC34_H */
