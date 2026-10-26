#ifndef DM1_V1_FMTOWNS_WALL_ORNAMENT_H
#define DM1_V1_FMTOWNS_WALL_ORNAMENT_H

#include <stdint.h>

typedef struct DM1_V1_FmtownsWallOrnament {
    int global_index;
    int view_index;
    int coordinate_set;
    int zone_index;
    int graphic_index;
    int scale;
    int flip_horizontal;
    int palette_depth;
    const int16_t *layout;
} DM1_V1_FmtownsWallOrnament;

int dm1_v1_fmtowns_wall_ornament_resolve(
    int global_index, int view_index, int japanese,
    DM1_V1_FmtownsWallOrnament *out);

#endif
