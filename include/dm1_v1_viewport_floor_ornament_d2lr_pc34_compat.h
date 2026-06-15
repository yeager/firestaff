#ifndef FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D2LR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D2LR_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D2LR_FLOOR_VIEW_D2L_PC34 = 3,
    DM1_V1_D2LR_FLOOR_VIEW_D2R_PC34 = 5
} DM1_V1_FloorOrnamentD2LRViewPc34;

typedef enum {
    DM1_V1_D2LR_SQUARE_WALL_WITH_SIDE_ORNAMENT_PC34 = 0,
    DM1_V1_D2LR_SQUARE_CORRIDOR_WITH_FLOOR_ORNAMENT_PC34 = 1
} DM1_V1_FloorOrnamentD2LRSquarePc34;

typedef struct {
    DM1_V1_FloorOrnamentD2LRSquarePc34 square;
    DM1_V1_FloorOrnamentD2LRViewPc34 view_floor_index;
    int floor_ornament_ordinal;
    int coordinate_set;
    int native_bitmap_index;
} DM1_V1_FloorOrnamentD2LRInputPc34;

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
} DM1_V1_FloorOrnamentD2LRResultPc34;

#define DM1_V1_PC34_FLOOR_ORNAMENT_ZONE_BASE 1500
#define DM1_V1_PC34_FLOOR_ORNAMENT_VIEW_COUNT 9
#define DM1_V1_PC34_FLOOR_ORNAMENT_TRANSPARENT_COLOR 10

bool dm1_v1_viewport_floor_ornament_d2lr_resolve_f0108_pc34(
    const DM1_V1_FloorOrnamentD2LRInputPc34 *input,
    DM1_V1_FloorOrnamentD2LRResultPc34 *out);

uint8_t dm1_v1_viewport_floor_ornament_d2lr_blit_pixel_pc34(
    const DM1_V1_FloorOrnamentD2LRResultPc34 *spec,
    uint8_t destination_pixel,
    uint8_t source_pixel);

const char *dm1_v1_viewport_floor_ornament_d2lr_source_lock_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_ORNAMENT_D2LR_PC34_COMPAT_H */
