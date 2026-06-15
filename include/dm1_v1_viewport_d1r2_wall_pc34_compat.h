#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1R2_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1R2_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D1R2_WALL_SYNTHETIC_WIDTH_PC34 8
#define DM1_V1_D1R2_WALL_SYNTHETIC_HEIGHT_PC34 4
#define DM1_V1_D1R2_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D1R2_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D1R2_WALL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D1R2_WALL_ANCHOR_COUNT_PC34 9
#define DM1_V1_D1R2_WALL_ROUTE_COUNT_PC34 2
#define DM1_V1_D1R2_WALL_CHECK_CAPACITY_PC34 18

typedef enum {
    DM1_V1_D1R2_WALL_ROUTE_NATIVE_PC34 = 0,
    DM1_V1_D1R2_WALL_ROUTE_PARITY_FLIPPED_PC34 = 1
} DM1_V1_D1R2WallRoutePc34;

typedef struct {
    const char *id;
    const char *file;
    const char *function;
    int line_first;
    int line_last;
    const char *claim;
} DM1_V1_D1R2WallAnchorPc34;

typedef struct {
    DM1_V1_D1R2WallRoutePc34 route;
    bool contract_only;
    bool real_asset_bitmap_parity;
    bool wall_case;
    bool parity_flip;
    int draw_order_index;
    int relative_depth;
    int relative_lateral;
    int view_square_index;
    int native_wall_index_pc34;
    int opposite_wall_index_pc34;
    int native_wall_set_index_pc34;
    int opposite_wall_set_index_pc34;
    int wall_zone_pc34;
    int old_media_wall_zone_pc34;
    int frame_left_x;
    int frame_right_x;
    int frame_top_y;
    int frame_bottom_y;
    int frame_byte_width;
    int frame_height;
    int frame_source_x;
    int frame_source_y;
    int field_aspect_index;
    int field_mask;
    int field_byte_width;
    int field_height;
    int field_source_x;
    int field_bitplane_word_count;
    int transparent_color;
    bool uses_f0100_frame_blit;
    bool uses_f0104_c10_wall_blit;
    bool uses_f0105_c10_flipped_wall_blit;
    bool parity_uses_opposite_native_bitmap;
    bool calls_f0107_wall_ornament_probe;
    int f0107_wall_ornament_ordinal;
    bool wall_case_returns_before_f0111;
    bool wall_case_returns_before_f0115;
    bool calls_f0111_door;
    bool calls_f0115_thing_pass;
    bool thing_pass_marker_excluded;
    const char *route_name;
    const char *source_lines;
} DM1_V1_D1R2WallRouteSpecPc34;

typedef struct {
    const DM1_V1_D1R2WallRouteSpecPc34 *spec;
    bool parity_flip;
    bool in_clip;
    bool writes_pixel;
    bool transparent_skip;
    bool no_write_metadata;
    bool uses_scratch;
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
} DM1_V1_D1R2WallPixelPc34;

typedef struct {
    bool ok;
    const DM1_V1_D1R2WallRouteSpecPc34 *native;
    const DM1_V1_D1R2WallRouteSpecPc34 *parity;
    DM1_V1_D1R2WallPixelPc34 checks[DM1_V1_D1R2_WALL_CHECK_CAPACITY_PC34];
    size_t check_count;
    int c10_palette_index;
    int c112_byte_width_viewport;
    int d1r_view_square_pc34;
    int d1l_view_square_pc34;
    int d1r_wall_pc34;
    int d1l_wall_pc34;
    int d1r_zone_pc34;
    int d1l_zone_pc34;
    int d1c_zone_pc34;
    int f0107_ordinal_pc34;
    bool d1r2_is_right_side_mirror;
    bool native_route_uses_f0104;
    bool parity_route_uses_f0105;
    bool parity_scratch_flips_opposite_native_wall;
    bool c10_flesh_pixels_preserve_destination;
    bool right_edge_clipped;
    bool f0107_then_return;
    bool f0128_dispatch_left_before_right;
    bool no_f0111_marker;
    bool no_f0115_thing_pass_marker;
    const char *source_evidence;
} DM1_V1_D1R2WallRunPc34;

const DM1_V1_D1R2WallAnchorPc34 *
dm1_v1_viewport_d1r2_wall_anchor_table_pc34(size_t *count);

const DM1_V1_D1R2WallRouteSpecPc34 *
dm1_v1_viewport_d1r2_wall_route_spec_pc34(DM1_V1_D1R2WallRoutePc34 route);

bool dm1_v1_viewport_d1r2_wall_pc34_compat_run(DM1_V1_D1R2WallRunPc34 *out);

bool dm1_v1_viewport_d1r2_wall_apply_frame_pixel_pc34(
    const DM1_V1_D1R2WallRouteSpecPc34 *spec,
    int viewport_y,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D1R2WallPixelPc34 *out);

uint8_t dm1_v1_viewport_d1r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d1r2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D1R2_WALL_PC34_COMPAT_H */
