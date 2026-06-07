#ifndef FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D1L2_D1R2_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D1L2_D1R2_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D1L2_D1R2_FLOOR_VIEW_D1L2_PC34 = 6,
    DM1_V1_D1L2_D1R2_FLOOR_VIEW_D1R2_PC34 = 8
} DM1_V1_FloorOrnamentD1L2D1R2ViewPc34;

typedef enum {
    DM1_V1_D1L2_D1R2_SQUARE_WALL_WITH_SIDE_ORNAMENT_PC34 = 0,
    DM1_V1_D1L2_D1R2_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34 = 1
} DM1_V1_FloorOrnamentD1L2D1R2SquarePc34;

typedef struct {
    DM1_V1_FloorOrnamentD1L2D1R2SquarePc34 square;
    DM1_V1_FloorOrnamentD1L2D1R2ViewPc34 view_floor_index;
    int floor_ornament_ordinal;
    int coordinate_set;
    int native_bitmap_index;
} DM1_V1_FloorOrnamentD1L2D1R2InputPc34;

typedef struct {
    bool reads_floor_ornament_flag;
    bool calls_f0108;
    bool draws_floor_ornament;
    bool open_pit_bug64_path;
    int floor_ornament_index;
    int view_floor_index;
    int native_bitmap_index;
    int zone_index;
    int source_x;
    int source_y;
    int source_x2;
    int source_byte_width;
    int source_height;
    bool flip_horizontal;
    uint8_t transparent_color;
    bool side_wall_band_stays_clean;
    const char *source_lines;
} DM1_V1_FloorOrnamentD1L2D1R2ResultPc34;

#define DM1_V1_PC34_FLOOR_ORNAMENT_D1L2_D1R2_ZONE_BASE 1500
#define DM1_V1_PC34_FLOOR_ORNAMENT_D1L2_D1R2_VIEW_COUNT 9
#define DM1_V1_PC34_FLOOR_ORNAMENT_D1L2_D1R2_TRANSPARENT_COLOR 10

bool dm1_v1_viewport_floor_ornament_d1l2_d1r2_resolve_f0108_pc34(
    const DM1_V1_FloorOrnamentD1L2D1R2InputPc34 *input,
    DM1_V1_FloorOrnamentD1L2D1R2ResultPc34 *out);

uint8_t dm1_v1_viewport_floor_ornament_d1l2_d1r2_blit_pixel_pc34(
    const DM1_V1_FloorOrnamentD1L2D1R2ResultPc34 *spec,
    uint8_t destination_pixel,
    uint8_t source_pixel);

const char *dm1_v1_viewport_floor_ornament_d1l2_d1r2_source_lock_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D1L2_D1R2_PC34_COMPAT_H */
