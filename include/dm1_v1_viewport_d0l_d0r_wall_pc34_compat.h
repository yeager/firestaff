#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L_D0R_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L_D0R_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D0L_D0R_WALL_ROUTE_D0L_NATIVE_PC34 = 0,
    DM1_V1_D0L_D0R_WALL_ROUTE_D0R_PARITY_PC34 = 1
} DM1_V1_D0LD0RWallRoutePc34;

typedef struct {
    DM1_V1_D0LD0RWallRoutePc34 route;
    int row;
    uint8_t transparent_color;
} DM1_V1_D0LD0RWallInputPc34;

typedef struct {
    DM1_V1_D0LD0RWallRoutePc34 route;
    int view_square_index;
    int selected_wall_bitmap_index;
    int opposite_wall_bitmap_index;
    int pc34_wall_zone_index;
    int pc34_wall_zone_family_first;
    int pc34_wall_zone_family_last;
    int source_x_first;
    int source_x_last;
    int viewport_x_first;
    int viewport_x_last;
    int source_width;
    int source_height;
    bool uses_f0104_native_blit;
    bool uses_f0105_parity_scratch_flip;
    bool uses_c10_transparency;
    bool wall_case_returns;
    bool calls_f0111_door;
    bool calls_f0115_thing_pass;
    bool calls_f0108_floor_ornament;
    uint8_t transparent_color;
    const char *source_lines;
    const char *contract;
} DM1_V1_D0LD0RWallSpecPc34;

typedef struct {
    DM1_V1_D0LD0RWallSpecPc34 spec;
    int visible;
    int viewport_x;
    int source_x;
    int scratch_x;
    uint8_t pixel_before;
    uint8_t source_pixel;
    uint8_t pixel_after;
} DM1_V1_D0LD0RWallPixelPc34;

#define DM1_V1_D0L_D0R_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D0L_D0R_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D0L_D0R_WALL_SOURCE_WIDTH_PC34 64
#define DM1_V1_D0L_D0R_WALL_SOURCE_HEIGHT_PC34 136
#define DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34 10

bool M11_GameView_D0LD0RWallResolvePc34(
    const DM1_V1_D0LD0RWallInputPc34 *input,
    DM1_V1_D0LD0RWallSpecPc34 *out);

bool M11_GameView_D0LD0RWallMapViewportXToSourcePc34(
    const DM1_V1_D0LD0RWallSpecPc34 *spec,
    int viewport_x,
    int *source_x,
    int *scratch_x);

bool M11_GameView_D0LD0RWallApplyPixelSlicePc34(
    const DM1_V1_D0LD0RWallInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    int viewport_x,
    DM1_V1_D0LD0RWallPixelPc34 *out);

uint8_t M11_GameView_D0LD0RWallBlendPixelPc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *M11_GameView_D0LD0RWallSourceLockPc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0L_D0R_WALL_PC34_COMPAT_H */
