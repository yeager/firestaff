#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D1L_D1R_WALL_ROUTE_D1L_NATIVE_PC34 = 0,
    DM1_V1_D1L_D1R_WALL_ROUTE_D1R_PARITY_PC34 = 1
} DM1_V1_D1LD1RWallRoutePc34;

typedef struct {
    DM1_V1_D1LD1RWallRoutePc34 route;
    int row;
    int viewport_x;
    uint8_t transparent_color;
} DM1_V1_D1LD1RWallInputPc34;

typedef struct {
    DM1_V1_D1LD1RWallRoutePc34 route;
    bool contract_only;
    bool real_asset_bitmap_parity;
    int depth;
    int lateral;
    int view_square_index;
    int selected_wall_bitmap_index;
    int parity_partner_wall_bitmap_index;
    int wall_zone_index;
    int wall_zone_family_first;
    int wall_zone_family_last;
    int frame_index;
    int field_aspect_index;
    int field_mask;
    int frame_viewport_x_first;
    int frame_viewport_x_last;
    int frame_viewport_y_first;
    int frame_viewport_y_last;
    int frame_source_x;
    int frame_source_y;
    int frame_byte_width;
    int frame_height;
    int source_x_first;
    int source_x_last;
    int source_y_first;
    int source_y_last;
    int source_width;
    int source_height;
    bool uses_f0100_frame_blit;
    bool uses_f0104_native_blit;
    bool uses_f0105_parity_scratch_flip;
    bool uses_c10_transparency;
    bool wall_case_returns;
    bool calls_f0107_side_ornament_probe;
    bool calls_f0108_floor_ornament;
    bool calls_f0111_door;
    bool calls_f0115_thing_pass;
    uint8_t transparent_color;
    const char *source_lines;
    const char *contract;
} DM1_V1_D1LD1RWallSpecPc34;

typedef struct {
    DM1_V1_D1LD1RWallSpecPc34 spec;
    bool in_clip;
    bool writes_pixel;
    bool transparent_skip;
    bool no_write_metadata;
    int row;
    int viewport_x;
    int source_x;
    int source_y;
    int scratch_x;
    size_t source_offset;
    size_t viewport_offset;
    uint8_t pixel_before;
    uint8_t source_pixel;
    uint8_t pixel_after;
} DM1_V1_D1LD1RWallPixelPc34;

#define DM1_V1_D1L_D1R_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D1L_D1R_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D1L_D1R_WALL_SOURCE_WIDTH_PC34 320
#define DM1_V1_D1L_D1R_WALL_SOURCE_HEIGHT_PC34 111
#define DM1_V1_D1L_D1R_WALL_C10_COLOR_FLESH_PC34 10

bool M11_GameView_D1LD1RWallResolvePc34(
    const DM1_V1_D1LD1RWallInputPc34 *input,
    DM1_V1_D1LD1RWallSpecPc34 *out);

bool M11_GameView_D1LD1RWallMapViewportToSourcePc34(
    const DM1_V1_D1LD1RWallSpecPc34 *spec,
    int row,
    int viewport_x,
    int *source_x,
    int *source_y,
    int *scratch_x);

bool M11_GameView_D1LD1RWallApplyPixelPc34(
    const DM1_V1_D1LD1RWallInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D1LD1RWallPixelPc34 *out);

uint8_t M11_GameView_D1LD1RWallBlendPixelPc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *M11_GameView_D1LD1RWallSourceLockPc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_WALL_PC34_COMPAT_H */
