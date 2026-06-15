#include "dm1_v1_viewport_floor_ornament_d1l2_d1r2_pc34_compat.h"

#include <string.h>

typedef struct {
    int x1;
    int x2;
    int y1;
    int byte_width;
    int height;
} DM1_V1_FloorOrnamentD1CoordinatePc34;

static const DM1_V1_FloorOrnamentD1CoordinatePc34 s_d1l2_coordinates[3] = {
    { 0, 31, 92, 16, 25 },
    { 0, 63, 90, 32, 29 },
    { 0, 15, 97, 8, 12 }
};

static const DM1_V1_FloorOrnamentD1CoordinatePc34 s_d1r2_coordinates[3] = {
    { 192, 223, 92, 16, 25 },
    { 160, 223, 90, 32, 29 },
    { 208, 223, 97, 8, 12 }
};

static const DM1_V1_FloorOrnamentD1CoordinatePc34 *coordinate_for_view(
    DM1_V1_FloorOrnamentD1L2D1R2ViewPc34 view,
    int coordinate_set)
{
    if (coordinate_set < 0 || coordinate_set >= 3) return NULL;
    if (view == DM1_V1_D1L2_D1R2_FLOOR_VIEW_D1L2_PC34) return &s_d1l2_coordinates[coordinate_set];
    if (view == DM1_V1_D1L2_D1R2_FLOOR_VIEW_D1R2_PC34) return &s_d1r2_coordinates[coordinate_set];
    return NULL;
}

bool dm1_v1_viewport_floor_ornament_d1l2_d1r2_resolve_f0108_pc34(
    const DM1_V1_FloorOrnamentD1L2D1R2InputPc34 *input,
    DM1_V1_FloorOrnamentD1L2D1R2ResultPc34 *out)
{
    const DM1_V1_FloorOrnamentD1CoordinatePc34 *coord;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->floor_ornament_index = -1;
    out->transparent_color = DM1_V1_PC34_FLOOR_ORNAMENT_D1L2_D1R2_TRANSPARENT_COLOR;
    out->source_lines = dm1_v1_viewport_floor_ornament_d1l2_d1r2_source_lock_pc34();
    if (!input) return false;

    coord = coordinate_for_view(input->view_floor_index, input->coordinate_set);
    if (!coord || input->native_bitmap_index < 0 || input->floor_ornament_ordinal < 0) {
        return false;
    }

    out->view_floor_index = input->view_floor_index;
    out->side_wall_band_stays_clean = true;
    if (input->square == DM1_V1_D1L2_D1R2_SQUARE_WALL_WITH_SIDE_ORNAMENT_PC34) {
        return true;
    }
    if (input->square != DM1_V1_D1L2_D1R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34) {
        return false;
    }

    out->reads_floor_ornament_flag = true;
    out->calls_f0108 = true;
    if (input->floor_ornament_ordinal == 0) {
        return true;
    }

    out->draws_floor_ornament = true;
    out->floor_ornament_index = input->floor_ornament_ordinal - 1;
    out->native_bitmap_index = input->native_bitmap_index + 4;
    out->zone_index = DM1_V1_PC34_FLOOR_ORNAMENT_D1L2_D1R2_ZONE_BASE +
        input->coordinate_set * DM1_V1_PC34_FLOOR_ORNAMENT_D1L2_D1R2_VIEW_COUNT +
        input->view_floor_index;
    out->source_x = coord->x1;
    out->source_y = coord->y1;
    out->source_x2 = coord->x2;
    out->source_byte_width = coord->byte_width;
    out->source_height = coord->height;
    out->flip_horizontal = input->view_floor_index == DM1_V1_D1L2_D1R2_FLOOR_VIEW_D1R2_PC34;
    return true;
}

uint8_t dm1_v1_viewport_floor_ornament_d1l2_d1r2_blit_pixel_pc34(
    const DM1_V1_FloorOrnamentD1L2D1R2ResultPc34 *spec,
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    if (!spec || !spec->draws_floor_ornament) return destination_pixel;
    if (source_pixel == spec->transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_floor_ornament_d1l2_d1r2_source_lock_pc34(void)
{
    return
        "Source-locked contract gate only; this does not claim full real-asset bitmap parity. "
        "DUNVIEW.C:7436-7460 F0122 D1L wall draws side ornament and returns before F0108; "
        "DUNVIEW.C:7520-7525 F0122 D1L corridor reaches F0108 with M594; "
        "DUNVIEW.C:7604-7628 F0123 D1R wall draws side ornament and returns before F0108; "
        "DUNVIEW.C:7688-7693 F0123 D1R corridor reaches F0108 with M596; "
        "DUNVIEW.C:3959-3966 F0108 ordinal gate, ordinal--, native increment, coordinate-set read; "
        "DUNVIEW.C:3965 F0108 zero-based floor ornament index; "
        "DUNVIEW.C:3967 F0108 D1R horizontal flip; "
        "DUNVIEW.C:3989 F0108 blits with C10 transparency and coordinate byte width; "
        "DUNVIEW.C:3995 F0108 PC zone C1500 + coordinateSet * 9 + viewFloor; "
        "DUNVIEW.C:793-802 G0191 D1L/D1R native bitmap increment 4 for far walls; "
        "DUNVIEW.C:1167-1195 G0206 D1L/D1R floor coordinate sets; "
        "DEFS.H:2544 M558 floor ornament ordinal; DEFS.H:2745-2747 M594/M596 D1 floor views; "
        "DEFS.H:4223 C1500 floor ornament zone";
}
