#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D2L2_D2R2_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D2L2_D2R2_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2L2_D2R2_WALL_SOURCE_WIDTH_PC34 36
#define DM1_V1_D2L2_D2R2_WALL_SOURCE_HEIGHT_PC34 71
#define DM1_V1_D2L2_D2R2_WALL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D2L2_D2R2_WALL_CHECK_CAPACITY_PC34 16

typedef enum {
    DM1_V1_D2L2_D2R2_WALL_SIDE_D2L2_PC34 = 0,
    DM1_V1_D2L2_D2R2_WALL_SIDE_D2R2_PC34 = 1
} DM1_V1_D2L2D2R2WallSidePc34;

typedef struct {
    DM1_V1_D2L2D2R2WallSidePc34 side;
    bool contract_only;
    bool real_asset_bitmap_parity;
    int draw_order_index;
    int view_square_index;
    int native_wall_index_pc34;
    int flipped_wall_index_pc34;
    int wall_zone_pc34;
    int frame_left_x;
    int frame_right_x;
    int frame_top_y;
    int frame_bottom_y;
    int frame_byte_width;
    int frame_height;
    int source_x_first;
    int source_x_last;
    int source_y_first;
    int source_y_last;
    int viewport_x_first;
    int viewport_x_last;
    int viewport_y_first;
    int viewport_y_last;
    int visible_width;
    int visible_height;
    int transparent_color;
    bool uses_f0104_native_route;
    bool uses_f0105_flipped_route;
    bool wall_case_returns;
    bool calls_f0113_teleporter_field;
    bool calls_f0108_floor_ornament;
    bool calls_f0111_door;
    bool calls_f0115_thing_pass;
    const char *view_square_symbol;
    const char *native_wall_symbol;
    const char *flipped_wall_symbol;
    const char *wall_zone_symbol;
    const char *source_lines;
} DM1_V1_D2L2D2R2WallSpecPc34;

typedef struct {
    const DM1_V1_D2L2D2R2WallSpecPc34 *spec;
    bool parity_flip;
    bool in_clip;
    bool writes_pixel;
    bool transparent_skip;
    bool no_write_metadata;
    int row;
    int viewport_x;
    int source_x;
    int source_y;
    int selected_source_x;
    size_t source_offset;
    size_t viewport_offset;
    uint8_t pixel_before;
    uint8_t source_pixel;
    uint8_t pixel_after;
} DM1_V1_D2L2D2R2WallPixelPc34;

typedef struct {
    bool ok;
    const DM1_V1_D2L2D2R2WallSpecPc34 *d2l2;
    const DM1_V1_D2L2D2R2WallSpecPc34 *d2r2;
    DM1_V1_D2L2D2R2WallPixelPc34 checks[DM1_V1_D2L2_D2R2_WALL_CHECK_CAPACITY_PC34];
    size_t check_count;
    int c10_palette_index;
    int opaque_left_palette_index;
    int opaque_right_palette_index;
    bool draw_order_left_before_right;
    bool mirrored_route_pair;
    bool same_wall_zone_family;
    bool same_c10_transparency;
    bool same_height_and_row;
    bool excludes_f0108_f0111_f0115;
    const char *source_evidence;
} DM1_V1_D2L2D2R2WallRunPc34;

const DM1_V1_D2L2D2R2WallSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_wall_spec_pc34(DM1_V1_D2L2D2R2WallSidePc34 side);

bool dm1_v1_viewport_d2l2_d2r2_wall_pc34_compat_run(
    DM1_V1_D2L2D2R2WallRunPc34 *out);

uint8_t dm1_v1_viewport_d2l2_d2r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d2l2_d2r2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_WALL_PC34_COMPAT_H */
