/*
 * nexus_v22_shape_cache_pc34.h
 *
 * Nexus V2.2 GPU render path: minimal per-cell V22 cache.
 *
 * Nexus does not yet have a full V22 shape book (csb_v22_shapes.h /
 * theron_v22_shapes.h pattern). For the first-cut in-place pipeline,
 * this cache stores only the raw cell type + direction per cell,
 * and the in-place renderer maps raw cell type -> asset_id via a
 * simple static table (no WallShape/FloorShape/Material structs).
 *
 * This is intentionally a minimal scaffold. A full Nexus V22 shape
 * book (parallel to dm1_v22_shapes.h / csb_v22_shapes.h / theron_v22_shapes.h)
 * is the planned follow-up; once that lands, the in-place renderer
 * can switch to M11_V22_ShapeType-style discriminator-based mapping.
 *
 * Source-lock: nexus_v22_modern_assets_pc34.c (sibling asset module),
 * include/nexus_v22_modern_assets_pc34.h (asset discovery + path),
 * include/nexus_v1_iso_reader.h (raw cell type source).
 */

#ifndef FIRESTAFF_NEXUS_V22_SHAPE_CACHE_PC34_H
#define FIRESTAFF_NEXUS_V22_SHAPE_CACHE_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Cell rectangle on the Nexus 3x3 viewport (1920x1080).
 * Same placeholder layout as CSB: 640x360 per cell. */
typedef struct {
    int x, y, w, h;
} NEXUS_V22_CellRect;

/* The 9-cell rect table (D0..D2 x L/C/R). Exposed for the in-place
 * renderer so the bitmap blit lands on the same coords as the
 * (future) overlay placeholder pass. */
extern const NEXUS_V22_CellRect nexus_v22_kCellRects[3][3];

/* Minimal per-cell cache: raw cell type + direction + active flag. */
typedef struct {
    int active;                  /* 1 = V22 path, 0 = V1 path */
    uint8_t raw_cell_type;
    uint8_t direction;
    uint8_t reserved[2];
} Nexus_V22_ShapeRuntimeResult;

/* Populate the cache from a 3x3 array of raw cell types
 * (D0..D2, L/C/R order). direction is the party facing (0..3).
 * When V22 is not the active mode, all cells are marked active=0. */
void nexus_v22_shape_cache_update(int direction,
                                  const unsigned char raw_cells[3][3]);

/* Read a cached cell. depth in {0,1,2}, lateral in {-1,0,1}. */
const Nexus_V22_ShapeRuntimeResult* nexus_v22_shape_cache_get(int depth,
                                                              int lateral);

/* True if the V22 cell is active (V22 path) for (depth, lateral). */
int nexus_v22_shape_cache_active(int depth, int lateral);

/* True if at least one cache_update has been called. */
int nexus_v22_shape_cache_populated(void);

/* Source evidence for tests/probes. */
const char* nexus_v22_shape_cache_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V22_SHAPE_CACHE_PC34_H */
