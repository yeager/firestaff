#ifndef FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D0L2_D0R2_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D0L2_D0R2_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D0L2_D0R2_ROUTE_D0L2_PC34 = 0,
    DM1_V1_D0L2_D0R2_ROUTE_D0R2_PC34 = 1
} DM1_V1_FloorOrnamentD0L2D0R2RoutePc34;

typedef enum {
    DM1_V1_D0L2_D0R2_SQUARE_WALL_PC34 = 0,
    DM1_V1_D0L2_D0R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34 = 1,
    DM1_V1_D0L2_D0R2_SQUARE_PIT_WITH_FLOOR_ORNAMENT_PC34 = 2,
    DM1_V1_D0L2_D0R2_SQUARE_TELEPORTER_WITH_FLOOR_ORNAMENT_PC34 = 5,
    DM1_V1_D0L2_D0R2_SQUARE_DOOR_SIDE_WITH_FLOOR_ORNAMENT_PC34 = 16,
    DM1_V1_D0L2_D0R2_SQUARE_STAIRS_SIDE_PC34 = 18
} DM1_V1_FloorOrnamentD0L2D0R2SquarePc34;

typedef struct {
    DM1_V1_FloorOrnamentD0L2D0R2RoutePc34 route;
    DM1_V1_FloorOrnamentD0L2D0R2SquarePc34 square;
    int floor_ornament_ordinal;
    uint8_t floor_source_pixel;
    uint8_t object_source_pixel;
} DM1_V1_FloorOrnamentD0L2D0R2InputPc34;

typedef struct {
    bool reads_floor_ornament_flag;
    bool calls_f0108;
    bool draws_floor_ornament;
    bool calls_f0115;
    bool wall_case_returns;
    bool stairs_case_returns;
    bool pit_falls_through_to_f0115;
    bool ceiling_pit_before_f0115;
    int view_square_index;
    int wall_zone_index;
    int f0115_cell_order;
    int documented_boundary_cell;
    int unsupported_view_floor_index;
    int native_bitmap_index;
    int floor_ornament_index;
    uint8_t transparent_color;
    uint8_t pixel_before;
    uint8_t pixel_after_floor_slice;
    uint8_t pixel_after_object_slice;
    const char *source_lines;
} DM1_V1_FloorOrnamentD0L2D0R2ResultPc34;

#define DM1_V1_PC34_D0L2_D0R2_VIEWPORT_WIDTH 224
#define DM1_V1_PC34_D0L2_D0R2_VIEWPORT_HEIGHT 136
#define DM1_V1_PC34_D0L2_D0R2_TRANSPARENT_COLOR 10

bool DM1_V1_FloorOrnamentD0L2D0R2_ResolvePc34Compat(
    const DM1_V1_FloorOrnamentD0L2D0R2InputPc34 *input,
    DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 *out);

bool DM1_V1_FloorOrnamentD0L2D0R2_ApplyPixelSlicePc34Compat(
    const DM1_V1_FloorOrnamentD0L2D0R2InputPc34 *input,
    uint8_t *viewport,
    size_t viewport_len,
    int row,
    int col,
    DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 *out);

uint8_t DM1_V1_FloorOrnamentD0L2D0R2_BlendPixelPc34Compat(
    const DM1_V1_FloorOrnamentD0L2D0R2ResultPc34 *spec,
    uint8_t destination_pixel,
    uint8_t source_pixel);

const char *DM1_V1_FloorOrnamentD0L2D0R2_SourceLockPc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D0L2_D0R2_PC34_COMPAT_H */
