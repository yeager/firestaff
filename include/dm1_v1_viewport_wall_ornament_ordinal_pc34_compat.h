#ifndef FIRESTAFF_DM1_V1_VIEWPORT_WALL_ORNAMENT_ORDINAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_WALL_ORNAMENT_ORDINAL_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_VIEW_WALL_D3L2_RIGHT_PC34 = 0,
    DM1_V1_VIEW_WALL_D3R2_LEFT_PC34 = 1,
    DM1_V1_VIEW_WALL_D3L_RIGHT_PC34 = 2,
    DM1_V1_VIEW_WALL_D3R_LEFT_PC34 = 3,
    DM1_V1_VIEW_WALL_D3L_FRONT_PC34 = 4,
    DM1_V1_VIEW_WALL_D3C_FRONT_PC34 = 5,
    DM1_V1_VIEW_WALL_D3R_FRONT_PC34 = 6,
    DM1_V1_VIEW_WALL_D2L_RIGHT_PC34 = 7,
    DM1_V1_VIEW_WALL_D2R_LEFT_PC34 = 8,
    DM1_V1_VIEW_WALL_D2L_FRONT_PC34 = 9,
    DM1_V1_VIEW_WALL_D2C_FRONT_PC34 = 10,
    DM1_V1_VIEW_WALL_D2R_FRONT_PC34 = 11,
    DM1_V1_VIEW_WALL_D1L_RIGHT_PC34 = 12,
    DM1_V1_VIEW_WALL_D1R_LEFT_PC34 = 13,
    DM1_V1_VIEW_WALL_D1C_FRONT_PC34 = 14
} DM1_V1_ViewWallIndexPc34;

typedef struct {
    int wall_ornament_ordinal;
    int view_wall_index;
    int coordinate_set;
    int native_bitmap_index;
    bool is_alcove;
} DM1_V1_WallOrnamentOrdinalInputPc34;

typedef struct {
    bool draws_ornament;
    int wall_ornament_index;
    int zone_index;
    int native_bitmap_index;
    bool flip_horizontal;
    int scale_x32;
    int palette_depth;
    bool returns_alcove;
    const char *source_lines;
} DM1_V1_WallOrnamentOrdinalResultPc34;

#define DM1_V1_PC34_WALL_ORNAMENT_ZONE_BASE 1004
#define DM1_V1_PC34_WALL_ORNAMENT_VIEW_COUNT 15

bool dm1_v1_viewport_wall_ornament_resolve_f0107_pc34(
    const DM1_V1_WallOrnamentOrdinalInputPc34 *input,
    DM1_V1_WallOrnamentOrdinalResultPc34 *out);

const char *dm1_v1_viewport_wall_ornament_ordinal_source_lock_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_WALL_ORNAMENT_ORDINAL_PC34_COMPAT_H */
