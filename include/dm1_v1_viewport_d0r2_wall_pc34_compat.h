#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0R2_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0R2_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D0R2_WALL_SYNTHETIC_WIDTH_PC34 8
#define DM1_V1_D0R2_WALL_SYNTHETIC_HEIGHT_PC34 4
#define DM1_V1_D0R2_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D0R2_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D0R2_WALL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0R2_WALL_ANCHOR_COUNT_PC34 11
#define DM1_V1_D0R2_WALL_ROUTE_COUNT_PC34 2
#define DM1_V1_D0R2_WALL_CHECK_CAPACITY_PC34 14

typedef enum {
    DM1_V1_D0R2_WALL_ROUTE_NATIVE_PC34 = 0,
    DM1_V1_D0R2_WALL_ROUTE_PARITY_FLIPPED_PC34 = 1
} DM1_V1_D0R2WallRoutePc34;

typedef struct {
    const char *id;
    const char *file;
    const char *function;
    int line_first;
    int line_last;
    const char *claim;
} DM1_V1_D0R2WallAnchorPc34;

typedef struct {
    DM1_V1_D0R2WallRoutePc34 route;
    bool contract_only;
    bool real_asset_bitmap_parity;
    bool wall_case;
    bool parity_flip;
    int relative_depth;
    int relative_lateral;
    int view_square_index;
    int native_wall_index_pc34;
    int opposite_wall_index_pc34;
    int native_wall_set_index_pc34;
    int opposite_wall_set_index_pc34;
    int old_media_wall_zone_pc34;
    int wall_zone_pc34;
    int frame_left_x;
    int frame_right_x;
    int frame_top_y;
    int frame_bottom_y;
    int frame_byte_width;
    int frame_height;
    int frame_source_x;
    int frame_source_y;
    int field_aspect_index;
    int field_native_bitmap_relative_index;
    int field_base_start_unit_index;
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
    bool wall_case_returns_before_f0112;
    bool wall_case_returns_before_f0115;
    bool calls_f0111_door;
    bool disjoint_from_d0l2_wall_anchor;
    bool thing_list_link_unlink_excluded;
    const char *route_name;
    const char *source_lines;
} DM1_V1_D0R2WallRouteSpecPc34;

typedef struct {
    const DM1_V1_D0R2WallRouteSpecPc34 *spec;
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
} DM1_V1_D0R2WallPixelPc34;

typedef struct {
    bool ok;
    const DM1_V1_D0R2WallRouteSpecPc34 *native;
    const DM1_V1_D0R2WallRouteSpecPc34 *parity;
    DM1_V1_D0R2WallPixelPc34 checks[DM1_V1_D0R2_WALL_CHECK_CAPACITY_PC34];
    size_t check_count;
    int c10_palette_index;
    int c112_byte_width_viewport;
    int d0r_view_square_pc34;
    int d0l_view_square_pc34;
    int d0r_wall_pc34;
    int d0l_wall_pc34;
    int d0r_zone_old_media_pc34;
    int d0r_zone_pc34;
    int d0l_zone_pc34;
    int ceiling_pit_d0r_zone_pc34;
    bool f0128_dispatch_d0l_before_d0r;
    bool f0126_wall_returns_before_ceiling_pit;
    bool f0126_wall_returns_before_thing_pass;
    bool f0126_corridor_branch_has_f0112_and_f0115;
    bool f0117_family_right_wall_return_anchor;
    bool f0163_f0164_not_part_of_wall_return;
    bool f0172_square_aspect_feeds_wall_switch;
    bool c10_flesh_pixels_preserve_destination;
    bool right_edge_clipped;
    bool no_f0111_marker;
    const char *source_evidence;
} DM1_V1_D0R2WallRunPc34;

const DM1_V1_D0R2WallAnchorPc34 *
dm1_v1_viewport_d0r2_wall_anchor_table_pc34(size_t *count);

const DM1_V1_D0R2WallRouteSpecPc34 *
dm1_v1_viewport_d0r2_wall_route_spec_pc34(DM1_V1_D0R2WallRoutePc34 route);

bool dm1_v1_viewport_d0r2_wall_pc34_compat_run(DM1_V1_D0R2WallRunPc34 *out);

uint8_t dm1_v1_viewport_d0r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d0r2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0R2_WALL_PC34_COMPAT_H */
