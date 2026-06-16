#ifndef FIRESTAFF_DM1_V2_SHAPE_RUNTIME_PC34_H
#define FIRESTAFF_DM1_V2_SHAPE_RUNTIME_PC34_H

/*
 * dm1_v2_shape_runtime_pc34.h
 *
 * DM1 V2.2 shape runtime dispatch.
 *
 * Wires the M11 viewport renderer to the V2.2 modern shape book
 * (m11_v22_shape_for_cell) when the user has selected V2.2 Modern
 * Graphics via the M12 launcher. When V2.2 is inactive, the
 * runtime reports the V1 path is in use and the renderer can
 * skip the V22 override.
 *
 * Contract:
 *   dm1_v2_shape_runtime_for_cell(raw_square, direction, depth, lateral)
 *     -> DM1_V2_ShapeRuntimeResult
 *        .active = 1 if V22 path is in use and shape was resolved
 *        .active = 0 if V1 path is in use (renderer uses its own
 *                    existing V1 wall/floor selection)
 *        .params, .wall, .floor, .material: V22 shape book data
 *
 * Source-lock references:
 *   - include/dm1_v22_shapes.h         the V2.2 shape book
 *   - include/dm1_v2_presentation_mode_pc34.h  the V22 mode gate
 *   - ReDMCSB DUNVIEW.C:6697-6816      composition draw order
 *   - ReDMCSB DUNGEON.C:2238-2246      square type decode
 *   - ReDMCSB DEFS.H:922 M034_SQUARE_TYPE  cell type field
 *
 * Module: src/dm1v2/dm1_v2_shape_runtime_pc34.c
 * Test:   tests/test_dm1_v2_shape_runtime_pc34.c
 * Probe:  probes/firestaff_dm1_v2_shape_runtime_probe.c
 */

#include <stdint.h>
#include "dm1_v22_shapes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int active;                  /* 1 = V22 path, 0 = V1 path */
    M11_V22_ShapeParams params;
    M11_V22_WallShape  wall;
    M11_V22_FloorShape floor;
    const M11_V22_Material* material;
} DM1_V2_ShapeRuntimeResult;

/* Resolve a V22 shape for one (raw, direction, depth, lateral) cell.
 * When V22 is not the active mode, returns active=0 and the renderer
 * should fall back to its V1 path. The runtime is purely a
 * selector/wrapper around m11_v22_shape_for_cell(); it does not
 * mutate the dungeon or viewport state. */
DM1_V2_ShapeRuntimeResult dm1_v2_shape_runtime_for_cell(int raw_square,
                                                        int direction,
                                                        int depth,
                                                        int lateral);

/* Convenience: resolve a 12-cell viewport composition. The
 * output array is filled in DM1 order (D0..D3, L/C/R per depth).
 * `out->count` is set to 12 on success, 0 on bad input.
 * `out->cells[i].active` tells the renderer whether to override
 * that cell with the V22 shape or use V1. */
typedef struct {
    int count;
    DM1_V2_ShapeRuntimeResult cells[12];
} DM1_V2_ShapeRuntimeComposition;

DM1_V2_ShapeRuntimeComposition dm1_v2_shape_runtime_composition(
    const uint8_t raw_squares[12],
    const int lateral[12],
    const int depth[12],
    int direction);

/* True if the runtime is currently resolving V22 shapes
 * (i.e. the user selected V2.2 Modern and the pack is detected).
 * A synonym for dm1_v2_presentation_mode_is_v22() but kept here
 * so callers don't have to include the presentation-mode header. */
int dm1_v2_shape_runtime_v22_active(void);

/* Source evidence for tests/probes. */
const char* dm1_v2_shape_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_SHAPE_RUNTIME_PC34_H */
