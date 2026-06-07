#ifndef FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D3L2_D3R2_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D3L2_D3R2_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D3L2_D3R2_FLOOR_VIEW_D3L2_PC34 = 0,
    DM1_V1_D3L2_D3R2_FLOOR_VIEW_D3R2_PC34 = 1
} DM1_V1_FloorOrnamentD3L2D3R2ViewPc34;

typedef enum {
    DM1_V1_D3L2_D3R2_SQUARE_WALL_WITH_SIDE_ORNAMENT_PC34 = 0,
    DM1_V1_D3L2_D3R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34 = 1,
    DM1_V1_D3L2_D3R2_SQUARE_PIT_WITH_FLOOR_ORNAMENT_BUG64_PC34 = 2
} DM1_V1_FloorOrnamentD3L2D3R2SquarePc34;

typedef struct {
    DM1_V1_FloorOrnamentD3L2D3R2SquarePc34 square;
    DM1_V1_FloorOrnamentD3L2D3R2ViewPc34 view_floor_index;
    int floor_ornament_ordinal;
    int coordinate_set;
    int native_bitmap_index;
} DM1_V1_FloorOrnamentD3L2D3R2InputPc34;

typedef struct {
    bool reads_floor_ornament_flag;
    bool calls_f0108;
    bool draws_floor_ornament;
    bool open_pit_bug64_path;
    bool calls_f0111_in_pixel_slice;
    bool calls_f0115_in_pixel_slice;
    int floor_ornament_index;
    int view_square_index;
    int view_floor_index;
    int depth;
    int lane;
    int native_bitmap_index;
    int wall_zone_index;
    int floor_ornament_zone_index;
    bool flip_horizontal;
    uint8_t transparent_color;
    bool d0_wall_zone_reused;
    const char *source_lines;
} DM1_V1_FloorOrnamentD3L2D3R2ResultPc34;

#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_VIEW_SQUARE_D3L2 (-101)
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_VIEW_SQUARE_D3R2 (-102)
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_DEPTH 3
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_LANE_D3L2 (-2)
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_LANE_D3R2 2
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_WALL_D3L2 702
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_WALL_D3R2 703
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_WALL_D0L 716
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_WALL_D0R 717
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_BASE 1500
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_ZONE_STRIDE 11
#define DM1_V1_PC34_FLOOR_ORNAMENT_D3L2_D3R2_TRANSPARENT_COLOR 10

bool dm1_v1_viewport_floor_ornament_d3l2_d3r2_resolve_f0108_pc34(
    const DM1_V1_FloorOrnamentD3L2D3R2InputPc34 *input,
    DM1_V1_FloorOrnamentD3L2D3R2ResultPc34 *out);

uint8_t dm1_v1_viewport_floor_ornament_d3l2_d3r2_blit_pixel_pc34(
    const DM1_V1_FloorOrnamentD3L2D3R2ResultPc34 *spec,
    uint8_t destination_pixel,
    uint8_t source_pixel);

const char *dm1_v1_viewport_floor_ornament_d3l2_d3r2_source_lock_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D3L2_D3R2_PC34_COMPAT_H */
