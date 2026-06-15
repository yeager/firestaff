#include "dm1_v1_viewport_floor_ornament_d2lr_pc34_compat.h"

#include <string.h>

typedef struct {
    int x1;
    int x2;
    int y1;
    int byte_width;
    int height;
} DM1_V1_FloorOrnamentCoordinatePc34;

static const DM1_V1_FloorOrnamentCoordinatePc34 s_d2l_coordinates[3] = {
    { 0, 63, 77, 32, 11 },
    { 0, 79, 75, 40, 15 },
    { 9, 40, 80, 16, 6 }
};

static const DM1_V1_FloorOrnamentCoordinatePc34 s_d2r_coordinates[3] = {
    { 160, 223, 77, 32, 11 },
    { 144, 223, 75, 40, 15 },
    { 183, 214, 80, 16, 6 }
};

static const DM1_V1_FloorOrnamentCoordinatePc34 *coordinate_for_view(
    DM1_V1_FloorOrnamentD2LRViewPc34 view,
    int coordinate_set)
{
    if (coordinate_set < 0 || coordinate_set >= 3) return NULL;
    if (view == DM1_V1_D2LR_FLOOR_VIEW_D2L_PC34) return &s_d2l_coordinates[coordinate_set];
    if (view == DM1_V1_D2LR_FLOOR_VIEW_D2R_PC34) return &s_d2r_coordinates[coordinate_set];
    return NULL;
}

bool dm1_v1_viewport_floor_ornament_d2lr_resolve_f0108_pc34(
    const DM1_V1_FloorOrnamentD2LRInputPc34 *input,
    DM1_V1_FloorOrnamentD2LRResultPc34 *out)
{
    const DM1_V1_FloorOrnamentCoordinatePc34 *coord;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->floor_ornament_index = -1;
    out->transparent_color = DM1_V1_PC34_FLOOR_ORNAMENT_TRANSPARENT_COLOR;
    out->source_lines = dm1_v1_viewport_floor_ornament_d2lr_source_lock_pc34();
    if (!input) return false;

    coord = coordinate_for_view(input->view_floor_index, input->coordinate_set);
    if (!coord || input->native_bitmap_index < 0 || input->floor_ornament_ordinal < 0) {
        return false;
    }

    out->view_floor_index = input->view_floor_index;
    out->side_wall_band_stays_clean = true;
    if (input->square == DM1_V1_D2LR_SQUARE_WALL_WITH_SIDE_ORNAMENT_PC34) {
        return true;
    }
    if (input->square != DM1_V1_D2LR_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34) {
        return false;
    }

    out->reads_floor_ornament_flag = true;
    out->calls_f0108 = true;
    if (input->floor_ornament_ordinal == 0) {
        return true;
    }

    out->draws_floor_ornament = true;
    out->floor_ornament_index = input->floor_ornament_ordinal - 1;
    out->native_bitmap_index = input->native_bitmap_index + 2;
    out->zone_index = DM1_V1_PC34_FLOOR_ORNAMENT_ZONE_BASE +
        input->coordinate_set * DM1_V1_PC34_FLOOR_ORNAMENT_VIEW_COUNT +
        input->view_floor_index;
    out->source_x = coord->x1;
    out->source_y = coord->y1;
    out->source_x2 = coord->x2;
    out->source_byte_width = coord->byte_width;
    out->source_height = coord->height;
    out->flip_horizontal = input->view_floor_index == DM1_V1_D2LR_FLOOR_VIEW_D2R_PC34;
    return true;
}

uint8_t dm1_v1_viewport_floor_ornament_d2lr_blit_pixel_pc34(
    const DM1_V1_FloorOrnamentD2LRResultPc34 *spec,
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    if (!spec || !spec->draws_floor_ornament) return destination_pixel;
    if (source_pixel == spec->transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_floor_ornament_d2lr_source_lock_pc34(void)
{
    return
        "DUNVIEW.C:6900-6913 F0119 D2L square dispatch; "
        "DUNVIEW.C:6945-6973 F0119 D2L wall draws wall/ornaments and returns before F0108; "
        "DUNVIEW.C:7016-7020 F0119 D2L corridor reaches F0108 with M558; "
        "DUNVIEW.C:7181,7213 F0120 D2R reaches F0108 with M558; "
        "DUNVIEW.C:3959-3966 F0108 ordinal gate, ordinal--, native increment, coordinate-set read; "
        "DUNVIEW.C:3967-3970 F0108 D2R horizontal flip, D2L native bitmap path; "
        "DUNVIEW.C:3989 F0108 blits with C10 transparency and coordinate byte width; "
        "DUNVIEW.C:3995 F0108 PC zone C1500 + coordinateSet * 9 + viewFloor; "
        "DUNVIEW.C:793-802 G0191 D2L/D2R native bitmap increment 2; "
        "DUNVIEW.C:1167-1195 G0206 D2L/D2R floor coordinate sets; "
        "DEFS.H:1007-1008 wall/corridor elements; DEFS.H:2544 M558 floor ornament; "
        "DEFS.H:2742-2744 D2L/D2R floor views; DEFS.H:4223 C1500 floor ornament zone";
}
