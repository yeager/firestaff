/*
 * csb_v22_shapes_runtime_gate.c
 *
 * Product implementation of the CSB V2.2 shape API.
 *
 * The old `csb_v22_shapes.c` book assigns invented texture ids, PBR values,
 * tints and centimetre dimensions.  Neither PC 3.4 GRAPHICS.DAT nor
 * DUNGEON.DAT contains that modern material vocabulary, and no reviewed
 * extraction maps it to original pixels.  Do not turn those guesses into
 * game art.  ReDMCSB DUNVIEW.C F0128 composes source-owned bitmaps; until a
 * provenance-checked modern-art binding is available, callers receive no
 * V2.2 material and must retain the V1/V2.1 route.
 *
 * `csb_v22_shapes.c` remains explicitly compiled only by its historical
 * contract probes.  This file is the implementation linked by Firestaff.
 */

#include "csb_v22_shapes.h"

#include <string.h>

static CSB_V22_ShapeParams csb_v22_unavailable_params(void) {
    CSB_V22_ShapeParams result;
    memset(&result, 0, sizeof(result));
    return result;
}

CSB_V22_ShapeParams csb_v22_shape_for_cell(int csb_cell_type,
                                            int view_direction,
                                            int depth,
                                            int lateral) {
    (void)csb_cell_type;
    (void)view_direction;
    (void)depth;
    (void)lateral;
    return csb_v22_unavailable_params();
}

CSB_V22_ShapeParams csb_v22_shape_for_view_square(int view_square,
                                                    int element,
                                                    int direction) {
    (void)view_square;
    (void)element;
    (void)direction;
    return csb_v22_unavailable_params();
}

CSB_V22_WallShape csb_v22_wall_shape_get(CSB_V22_WallVariant variant) {
    CSB_V22_WallShape result;
    (void)variant;
    memset(&result, 0, sizeof(result));
    return result;
}

CSB_V22_FloorShape csb_v22_floor_shape_get(int csb_cell_type, int view_direction) {
    CSB_V22_FloorShape result;
    (void)csb_cell_type;
    (void)view_direction;
    memset(&result, 0, sizeof(result));
    return result;
}

const CSB_V22_Material* csb_v22_material_get(int material_id) {
    (void)material_id;
    return NULL;
}

int csb_v22_material_count(void) {
    return 0;
}

CSB_V22_ShapeParams csb_v22_shape_for_prison_door(int open_progress) {
    (void)open_progress;
    return csb_v22_unavailable_params();
}

CSB_V22_ShapeParams csb_v22_shape_for_chaos_rune(int rune_index) {
    (void)rune_index;
    return csb_v22_unavailable_params();
}

CSB_V22_ShapeParams csb_v22_shape_for_dsa_scroll(int scroll_index) {
    (void)scroll_index;
    return csb_v22_unavailable_params();
}

void csb_v22_shapes_init(void) {
    /* No source-derived modern material data is presently admitted. */
}

const char* csb_v22_shapes_source_evidence(void) {
    return "CSB V2.2 product material gate: ReDMCSB DUNVIEW.C F0128; "
           "PC 3.4 GRAPHICS.DAT/DUNGEON.DAT contain source pixels/cells, "
           "not a modern PBR material book. No reviewed binding admitted.";
}
