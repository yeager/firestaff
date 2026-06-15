#include "dm1_v1_viewport_floor_ornament_d0l2_d0r2_pc34_compat.h"

#include <string.h>

typedef struct {
    int view_square_index;
    int wall_zone_index;
    int f0115_cell_order;
    int documented_boundary_cell;
} DM1_V1_D0SideRouteSpecPc34;

static const DM1_V1_D0SideRouteSpecPc34 s_routes[2] = {
    { 10, 714, 0x0002, 2 },
    { 11, 715, 0x0001, 3 }
};

static const DM1_V1_D0SideRouteSpecPc34 *route_spec(
    DM1_V1_FloorOrnamentD0L2D0R2RoutePc34 route)
{
    if (route == DM1_V1_D0L2_D0R2_ROUTE_D0L2_PC34) return &s_routes[0];
    if (route == DM1_V1_D0L2_D0R2_ROUTE_D0R2_PC34) return &s_routes[1];
    return NULL;
}

bool M11_GameView_FloorOrnamentD0L2D0R2ResolvePc34(
    const DM1_V1_FloorOrnamentD0L2D0R2InputPc34 *input,
    DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 *out)
{
    const DM1_V1_D0SideRouteSpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->unsupported_view_floor_index = -1;
    out->native_bitmap_index = -1;
    out->floor_ornament_index = -1;
    out->transparent_color = DM1_V1_PC34_D0L2_D0R2_TRANSPARENT_COLOR;
    out->source_lines = M11_GameView_FloorOrnamentD0L2D0R2SourceLockPc34();
    if (!input || input->floor_ornament_ordinal < 0) return false;

    spec = route_spec(input->route);
    if (!spec) return false;

    out->view_square_index = spec->view_square_index;
    out->wall_zone_index = spec->wall_zone_index;
    out->f0115_cell_order = spec->f0115_cell_order;
    out->documented_boundary_cell = spec->documented_boundary_cell;

    /*
     * ReDMCSB DUNVIEW.C F0125/F0126 lines 7977-8038 and 8081-8138 have no
     * M558/F0108 read on D0 side routes.  The C01/C16/C05 and C02 fallthrough
     * paths call F0115 at lines 8005 and 8115 instead.
     */
    switch (input->square) {
        case DM1_V1_D0L2_D0R2_SQUARE_WALL_PC34:
            out->wall_case_returns = true;
            return true;
        case DM1_V1_D0L2_D0R2_SQUARE_STAIRS_SIDE_PC34:
            out->stairs_case_returns = true;
            return true;
        case DM1_V1_D0L2_D0R2_SQUARE_PIT_WITH_FLOOR_ORNAMENT_PC34:
            out->pit_falls_through_to_f0115 = true;
            out->calls_f0115 = true;
            out->ceiling_pit_before_f0115 = input->route == DM1_V1_D0L2_D0R2_ROUTE_D0R2_PC34;
            return true;
        case DM1_V1_D0L2_D0R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34:
        case DM1_V1_D0L2_D0R2_SQUARE_DOOR_SIDE_WITH_FLOOR_ORNAMENT_PC34:
        case DM1_V1_D0L2_D0R2_SQUARE_TELEPORTER_WITH_FLOOR_ORNAMENT_PC34:
            out->calls_f0115 = true;
            out->ceiling_pit_before_f0115 = true;
            return true;
        default:
            return false;
    }
}

bool M11_GameView_FloorOrnamentD0L2D0R2ApplyPixelSlicePc34(
    const DM1_V1_FloorOrnamentD0L2D0R2InputPc34 *input,
    uint8_t *viewport,
    size_t viewport_len,
    int row,
    int col,
    DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 *out)
{
    size_t offset;

    if (!M11_GameView_FloorOrnamentD0L2D0R2ResolvePc34(input, out)) return false;
    if (!viewport || row < 0 || col < 0 ||
        row >= DM1_V1_PC34_D0L2_D0R2_VIEWPORT_HEIGHT ||
        col >= DM1_V1_PC34_D0L2_D0R2_VIEWPORT_WIDTH) {
        return false;
    }

    offset = (size_t)row * DM1_V1_PC34_D0L2_D0R2_VIEWPORT_WIDTH + (size_t)col;
    if (offset >= viewport_len) return false;

    out->pixel_before = viewport[offset];
    out->pixel_after_floor_slice = viewport[offset];
    if (out->calls_f0115) {
        viewport[offset] = M11_GameView_FloorOrnamentD0L2D0R2BlendPixelPc34(
            out, viewport[offset], input->object_source_pixel);
    }
    out->pixel_after_object_slice = viewport[offset];
    return true;
}

uint8_t M11_GameView_FloorOrnamentD0L2D0R2BlendPixelPc34(
    const DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 *spec,
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    if (!spec || source_pixel == spec->transparent_color) return destination_pixel;
    return source_pixel;
}

const char *M11_GameView_FloorOrnamentD0L2D0R2SourceLockPc34(void)
{
    return
        "Source-locked D0 side floor-ornament absence gate; ReDMCSB PC34 does "
        "not define D0 floor-view indices and F0125/F0126 do not call F0108. "
        "DUNVIEW.C:3940-4008 F0108 ordinal gate, native bitmap increment, "
        "coordinate-set read, C10 blit, and floor-zone math; "
        "DUNVIEW.C:4787-4831 F0115 setup and ordered view-cell decode; "
        "DUNVIEW.C:5181-5184 F0115 object blit with C10 transparency; "
        "DUNVIEW.C:5295-5296 D0L/D0R quarter-square creature boundary; "
        "DUNVIEW.C:7976-8005 F0125 D0L pit fallthrough/corridor-door-teleporter "
        "route to F0115 without M558/F0108; "
        "DUNVIEW.C:8007-8038 F0125 D0L wall returns before thing drawing; "
        "DUNVIEW.C:8080-8115 F0126 D0R pit fallthrough/corridor-door-teleporter "
        "route to F0115 without M558/F0108; "
        "DUNVIEW.C:8117-8138 F0126 D0R wall returns before thing drawing; "
        "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2587-2589 M609/M610/M611 D0 view "
        "squares; DEFS.H:2642-2645 view cells; DEFS.H:2658-2662 cell orders; "
        "DEFS.H:2739-2747 PC34 floor views stop at D1; DEFS.H:4036-4038 D0 wall zones.";
}
