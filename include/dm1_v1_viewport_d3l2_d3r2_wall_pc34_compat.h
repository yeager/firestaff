#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D3L2_D3R2_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3L2_D3R2_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3L2_D3R2_WALL_SOURCE_WIDTH_PC34 16
#define DM1_V1_D3L2_D3R2_WALL_SOURCE_HEIGHT_PC34 49
#define DM1_V1_D3L2_D3R2_WALL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D3L2_D3R2_WALL_CHECK_CAPACITY_PC34 16

typedef enum {
    DM1_V1_D3L2_D3R2_WALL_SIDE_D3L2_PC34 = 0,
    DM1_V1_D3L2_D3R2_WALL_SIDE_D3R2_PC34 = 1
} DM1_V1_D3L2D3R2WallSidePc34;

typedef struct {
    DM1_V1_D3L2D3R2WallSidePc34 side;
    bool contract_only;
    bool real_asset_bitmap_parity;
    int draw_order_index;
    int view_square_index;
    int relative_depth;
    int relative_lateral;
    int native_wall_index_pc34;
    int flipped_wall_index_pc34;
    int wall_set_global_index_pc34;
    int wall_zone_pc34;
    int frame_left_x;
    int frame_right_x;
    int frame_top_y;
    int frame_bottom_y;
    int frame_byte_width;
    int frame_height;
    int frame_source_x;
    int frame_source_y;
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
    bool uses_f0100_wallset_bitmap;
    bool uses_f0104_native_wall_route;
    bool uses_f0105_flipped_wall_route;
    bool f0104_f0105_route_is_wall_not_pit;
    bool calls_f0107_wall_ornament;
    bool calls_f0111_door;
    bool calls_f0115_alcove_or_thing_pass;
    bool calls_f0104_f0105_pit_route;
    bool calls_f0108_floor_ornament;
    bool preserves_c10_flesh_transparency;
    bool non_overlap_d3l_d3r_gate;
    bool non_overlap_d3c_gate;
    const char *view_square_symbol;
    const char *native_wall_symbol;
    const char *flipped_wall_symbol;
    const char *wall_set_global_symbol;
    const char *wall_zone_symbol;
    const char *bitmap_symbol;
    const char *frame_symbol;
    const char *source_lines;
} DM1_V1_D3L2D3R2WallSpecPc34;

typedef struct {
    const DM1_V1_D3L2D3R2WallSpecPc34 *spec;
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
} DM1_V1_D3L2D3R2WallPixelPc34;

typedef struct {
    bool ok;
    const DM1_V1_D3L2D3R2WallSpecPc34 *d3l2;
    const DM1_V1_D3L2D3R2WallSpecPc34 *d3r2;
    DM1_V1_D3L2D3R2WallPixelPc34 checks[DM1_V1_D3L2_D3R2_WALL_CHECK_CAPACITY_PC34];
    size_t check_count;
    int c10_palette_index;
    int c112_byte_width_viewport;
    int exact_d3l2_zone_pc34;
    int exact_d3r2_zone_pc34;
    int exact_d3l_zone_pc34;
    int exact_d3r_zone_pc34;
    int exact_d3c_zone_pc34;
    int exact_d3l_view_square_pc34;
    int exact_d3r_view_square_pc34;
    int exact_d3c_view_square_pc34;
    bool draw_order_left_before_right;
    bool mirrored_route_pair;
    bool d3l2_d3r2_zone_pair;
    bool non_overlap_with_d3l_d3r_wall_gate;
    bool non_overlap_with_d3c_wall_gate;
    bool no_f0107_ornament;
    bool no_f0111_door;
    bool no_f0115_alcove;
    bool no_f0104_f0105_pit_route;
    bool no_f0108_floor_ornament;
    bool same_c10_transparency;
    bool same_height_and_row;
    bool c10_flesh_pixels_preserve_destination;
    const char *source_evidence;
} DM1_V1_D3L2D3R2WallRunPc34;

const DM1_V1_D3L2D3R2WallSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_wall_spec_pc34(
    DM1_V1_D3L2D3R2WallSidePc34 side);

bool dm1_v1_viewport_d3l2_d3r2_wall_pc34_compat_run(
    DM1_V1_D3L2D3R2WallRunPc34 *out);

uint8_t dm1_v1_viewport_d3l2_d3r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d3l2_d3r2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_WALL_PC34_COMPAT_H */
