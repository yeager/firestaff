#include "dm1_v1_viewport_floor_ornament_d3l2_d3r2_pc34_compat.h"

#include <string.h>

typedef struct {
    int view_square_index;
    int depth;
    int lane;
    int wall_zone_index;
    bool flip_horizontal;
} DM1_V1_FloorOrnamentD3L2D3R2ViewSpecPc34;

static const DM1_V1_FloorOrnamentD3L2D3R2ViewSpecPc34 s_d3l2_spec = {
    DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_VIEW_SQUARE_D3L2,
    DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_DEPTH,
    DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_LANE_D3L2,
    DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_WALL_D3L2,
    false
};

static const DM1_V1_FloorOrnamentD3L2D3R2ViewSpecPc34 s_d3r2_spec = {
    DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_VIEW_SQUARE_D3R2,
    DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_DEPTH,
    DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_LANE_D3R2,
    DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_WALL_D3R2,
    true
};

static const DM1_V1_FloorOrnamentD3L2D3R2ViewSpecPc34 *spec_for_view(
    DM1_V1_FloorOrnamentD3L2D3R2ViewPc34 view)
{
    if (view == DM1_V1_D3L2_D3R2_FLOOR_VIEW_D3L2_PC34) return &s_d3l2_spec;
    if (view == DM1_V1_D3L2_D3R2_FLOOR_VIEW_D3R2_PC34) return &s_d3r2_spec;
    return NULL;
}

bool dm1_v1_viewport_floor_ornament_d3l2_d3r2_resolve_f0108_pc34(
    const DM1_V1_FloorOrnamentD3L2D3R2InputPc34 *input,
    DM1_V1_FloorOrnamentD3L2D3R2ResultPc34 *out)
{
    const DM1_V1_FloorOrnamentD3L2D3R2ViewSpecPc34 *view;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->floor_ornament_index = -1;
    out->transparent_color = DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_TRANSPARENT_COLOR;
    out->source_lines = dm1_v1_viewport_floor_ornament_d3l2_d3r2_source_lock_pc34();
    if (!input) return false;

    view = spec_for_view(input->view_floor_index);
    if (!view || input->native_bitmap_index < 0 || input->floor_ornament_ordinal < 0 ||
        input->coordinate_set < 0) {
        return false;
    }

    out->view_square_index = view->view_square_index;
    out->view_floor_index = input->view_floor_index;
    out->depth = view->depth;
    out->lane = view->lane;
    out->wall_zone_index = view->wall_zone_index;
    out->flip_horizontal = view->flip_horizontal;
    out->d0_wall_zone_reused =
        view->wall_zone_index == DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_WALL_D0L ||
        view->wall_zone_index == DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_WALL_D0R;

    if (input->square == DM1_V1_D3L2_D3R2_SQUARE_WALL_WITH_SIDE_ORNAMENT_PC34) {
        return true;
    }
    if (input->square != DM1_V1_D3L2_D3R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34 &&
        input->square != DM1_V1_D3L2_D3R2_SQUARE_PIT_WITH_FLOOR_ORNAMENT_BUG64_PC34) {
        return false;
    }

    /*
     * Source-lock: ReDMCSB DUNVIEW.C F0676:8478-8482 and F0677:8483-8486
     * dispatch depth 3 lanes -2/+2; F0108:3959-3966 performs the ordinal
     * gate and pre-decrement before the C10-transparent blit.  This contract
     * is intentionally the floor-ornament pixel slice only: it does not model
     * the later F0111 door route or F0115 thing pass.
     */
    out->reads_floor_ornament_flag = true;
    out->calls_f0108 = true;
    out->open_pit_bug64_path =
        input->square == DM1_V1_D3L2_D3R2_SQUARE_PIT_WITH_FLOOR_ORNAMENT_BUG64_PC34;
    if (input->floor_ornament_ordinal == 0) {
        return true;
    }

    out->draws_floor_ornament = true;
    out->floor_ornament_index = input->floor_ornament_ordinal - 1;
    out->native_bitmap_index = input->native_bitmap_index;
    out->floor_ornament_zone_index = DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_BASE +
        input->coordinate_set * DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_STRIDE +
        input->view_floor_index;
    return true;
}

uint8_t dm1_v1_viewport_floor_ornament_d3l2_d3r2_blit_pixel_pc34(
    const DM1_V1_FloorOrnamentD3L2D3R2ResultPc34 *spec,
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    if (!spec || !spec->draws_floor_ornament) return destination_pixel;
    if (source_pixel == spec->transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_floor_ornament_d3l2_d3r2_source_lock_pc34(void)
{
    /*
     * ReDMCSB anchors for this contract:
     * - DUNVIEW.C F0676:8478-8482 dispatches D3L2 after depth 3/lane -2.
     * - DUNVIEW.C F0677:8483-8486 dispatches D3R2 after depth 3/lane +2.
     * - DUNVIEW.C F0108:3959-3966 gates and decrements floor ornaments.
     * - DEFS.H:4042-4043 defines C702/C703 for D3L2/D3R2 wall zones.
     * - DEFS.H:4056-4057 defines C716/C717 for D0L/D0R, not this route.
     * - DEFS.H:2088 defines C10_COLOR_FLESH transparency.
     * The pixel-slice contract deliberately stops at F0108; it is not an
     * F0111 door or F0115 thing-pass gate.
     */
    return
        "Source-locked contract gate only; this does not claim full real-asset floor-ornament bitmap parity. "
        "DUNVIEW.C:8478-8482 F0676_DrawD3L2 dispatcher after F0150 depth 3 lane -2; "
        "DUNVIEW.C:8483-8486 F0677_DrawD3R2 dispatcher after F0150 depth 3 lane +2; "
        "DUNVIEW.C:6282-6284 F0676 pit/corridor reaches F0108 with M558 and C00_VIEW_FLOOR_D3L2; "
        "DUNVIEW.C:6349-6351 F0677 pit/corridor reaches F0108 with M558 and C01_VIEW_FLOOR_D3R2; "
        "DUNVIEW.C:3959-3966 F0108 ordinal gate, ordinal--, native increment, floor-ornament thing pass; "
        "DUNVIEW.C:3980-3983 F0108 flips C01_VIEW_FLOOR_D3R2, not C00_VIEW_FLOOR_D3L2; "
        "DUNVIEW.C:3998 F0108 I34/PC34 F0791 zone C1500 + coordinateSet * 11 + viewFloor; "
        "DEFS.H:4042-4043 C702_ZONE_WALL_D3L2 / C703_ZONE_WALL_D3R2 are the wall zones used here; "
        "DEFS.H:4056-4057 C716_ZONE_WALL_D0L / C717_ZONE_WALL_D0R are not used for D3L2/D3R2; "
        "DEFS.H:2088 C10_COLOR_FLESH transparency; "
        "DEFS.H:2750-2751 C00_VIEW_FLOOR_D3L2 / C01_VIEW_FLOOR_D3R2; "
        "non-F0111/non-F0115 pixel-slice contract: the floor-ornament gate stops at F0108.";
}
