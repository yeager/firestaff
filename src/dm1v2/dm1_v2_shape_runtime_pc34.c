/*
 * dm1_v2_shape_runtime_pc34.c
 *
 * DM1 V2.2 shape runtime dispatch.
 * See include/dm1_v2_shape_runtime_pc34.h for the contract.
 *
 * This is a thin wrapper over m11_v22_shape_for_cell() and
 * m11_v22_wall_shape_get() / m11_v22_floor_shape_get(), gated
 * by dm1_v2_presentation_mode_is_v22(). When V22 is not the
 * active mode, the runtime reports active=0 and the renderer
 * can use its existing V1 selection. The runtime is stateless
 * apart from reading the presentation mode global.
 */

#include "dm1_v2_shape_runtime_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"

#include <string.h>

DM1_V2_ShapeRuntimeResult dm1_v2_shape_runtime_for_cell(int raw_square,
                                                        int direction,
                                                        int depth,
                                                        int lateral) {
    DM1_V2_ShapeRuntimeResult r;
    memset(&r, 0, sizeof(r));

    if (!dm1_v2_presentation_mode_is_v22()) {
        /* V1 path: renderer uses its own wall/floor tables. */
        r.active = 0;
        return r;
    }

    /* V22 path: resolve the V22 shape book for this cell. */
    r.active = 1;
    r.params = m11_v22_shape_for_cell(raw_square, direction, depth, lateral);
    r.material = m11_v22_material_get(r.params.material_id);

    /* Pick a wall variant from the (depth, lateral) position. DM1
     * V2.2 has 4 depths (D0..D3); map lateral -1/0/+1 to L/C/R. */
    {
        int variant = 0;
        if (depth == 3)      variant = (lateral < 0) ? 0 : (lateral > 0) ? 1 : 2;
        else if (depth == 2) variant = (lateral < 0) ? 3 : (lateral > 0) ? 4 : 5;
        else if (depth == 1) variant = (lateral < 0) ? 6 : (lateral > 0) ? 7 : 8;
        else                 variant = 12;  /* D0 + door: default door variant */
        r.wall = m11_v22_wall_shape_get((M11_V22_WallVariant)variant);
    }

    /* Floor shape for the same cell. */
    r.floor = m11_v22_floor_shape_get(raw_square, direction);
    return r;
}

DM1_V2_ShapeRuntimeComposition dm1_v2_shape_runtime_composition(
    const uint8_t raw_squares[12],
    const int lateral[12],
    const int depth[12],
    int direction)
{
    DM1_V2_ShapeRuntimeComposition out;
    memset(&out, 0, sizeof(out));
    if (!raw_squares || !lateral || !depth) {
        return out;
    }
    out.count = 12;
    for (int i = 0; i < 12; i++) {
        out.cells[i] = dm1_v2_shape_runtime_for_cell(
            raw_squares[i], direction, depth[i], lateral[i]);
    }
    return out;
}

int dm1_v2_shape_runtime_v22_active(void) {
    return dm1_v2_presentation_mode_is_v22();
}

const char* dm1_v2_shape_runtime_source_evidence(void) {
    return
        "DM1 V2.2 shape runtime: dispatches m11_v22_shape_for_cell() when\n"
        "dm1_v2_presentation_mode_is_v22() returns 1. When the runtime is\n"
        "inactive, the renderer uses the V1 wall/floor path.\n"
        "Source-lock:\n"
        "  ReDMCSB DUNVIEW.C:6697-6816 - D3/D2/D1/D0 composition order\n"
        "  ReDMCSB DUNGEON.C:2238-2246 - M034 square type decode\n"
        "  ReDMCSB DEFS.H:922 M034_SQUARE_TYPE - cell type field\n"
        "  include/dm1_v22_shapes.h - V2.2 shape book (m11_v22_* API)\n"
        "  include/dm1_v2_presentation_mode_pc34.h - V22 mode gate\n";
}
