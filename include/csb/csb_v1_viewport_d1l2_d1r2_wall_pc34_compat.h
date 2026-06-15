#ifndef FIRESTAFF_CSB_CSB_V1_VIEWPORT_D1L2_D1R2_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_CSB_V1_VIEWPORT_D1L2_D1R2_WALL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 D1L2/D1R2 wall source-lock.
 * ReDMCSB anchors: DUNVIEW.C F0122 lines 7391-7557, F0123 lines
 * 7559-7725, F0104 lines 3113-3156, F0105 lines 3185-3218,
 * F0115 lines 4547-4581 and 5668-5671 as the thing-pass keep-out,
 * F0128 lines 8524-8542 for the D1 side dispatch and follow-up,
 * and F0127 line 8294 as an object-pass follow-up boundary. DUNGEON.C
 * F0163 lines 1769-1840, F0164 lines 1840-1905, and F0172 lines
 * 2466-2523 anchor map metadata and thing-list boundaries. DEFS.H
 * lines 2088, 2445-2452, 2596-2601, 2659-2666, 3425-3426,
 * 4052-4054, 4147-4162, and 4205-4207 bind C10, view squares,
 * cell orders, walls, zones, stairs, and pit-zone ordinals.
 */

typedef enum {
    CSB_V1_D1L2_D1R2_WALL_SIDE_D1L2_PC34 = 1,
    CSB_V1_D1L2_D1R2_WALL_SIDE_D1R2_PC34 = 2
} CSB_V1_D1L2D1R2WallSidePc34;

typedef struct {
    int copied_pixels;
    int transparent_pixels;
    int clipped_pixels;
    int rejected;
} CSB_V1_D1L2D1R2WallBlitStatsPc34;

typedef struct {
    int side;
    const char *route_name;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int redmcsb_function_number;
    int f0128_draw_order_index;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int view_square_index;
    int wall_element;
    int teleporter_element;
    int wall_zone;
    int neighboring_d1c_zone;
    int native_wall_index;
    int flipped_wall_index;
    int wall_frame_row;
    int wall_frame_x1;
    int wall_frame_x2;
    int wall_frame_y1;
    int wall_frame_y2;
    int wall_frame_byte_width;
    int wall_frame_height;
    int wall_frame_source_x;
    int wall_frame_source_y;
    int transparent_color;
    int f0100_st_wall_route;
    int f0104_native_route;
    int f0105_flipped_route;
    int f0107_wall_ornament_route;
    int f0111_door_route;
    int f0113_teleporter_route;
    int f0115_thing_pass_route;
    int f0115_thing_pass_keepout;
    int f0128_post_d1_followup_resets_map_coordinates;
    int f0128_post_d1_followup_draws_d1c;
    int f0127_wall_return_boundary;
    int f0172_square_aspect_metadata_read;
    int f0163_link_thing_keepout;
    int f0164_unlink_thing_keepout;
    int zone_math_base;
    int zone_math_view_square_d1c;
    const char *bitmap_symbol;
    const char *frame_symbol;
    const char *redmcsb_function_anchor;
    const char *source_lines;
} CSB_V1_D1L2D1R2WallRouteSpecPc34;

typedef struct {
    int ok;
    int route_count;
    int dispatch_order_ok;
    int wall_variant_binding_ok;
    int c10_transparency_ok;
    int clipped_edge_write_ok;
    int f0128_followup_ok;
    int f0115_keepout_ok;
    int dungeon_metadata_binding_ok;
    int d1l2_copied_pixels;
    int d1r2_copied_pixels;
    uint16_t first_thing_before;
    uint16_t first_thing_after;
} CSB_V1_D1L2D1R2WallRunResultPc34;

size_t csb_v1_viewport_d1l2_d1r2_wall_route_spec_count_pc34(void);

const CSB_V1_D1L2D1R2WallRouteSpecPc34 *
csb_v1_viewport_d1l2_d1r2_wall_route_spec_at_pc34(size_t index);

const CSB_V1_D1L2D1R2WallRouteSpecPc34 *
csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(int side);

int csb_v1_viewport_d1l2_d1r2_wall_expected_zone_for_view_square_pc34(
    int view_square);

int csb_v1_viewport_d1l2_d1r2_wall_map_viewport_x_to_source_pc34(
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *spec,
    int viewport_x,
    int flipped_variant,
    int *out_source_x);

int csb_v1_viewport_d1l2_d1r2_wall_apply_frame_clip_pc34(
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *spec,
    const uint8_t *source,
    int source_width,
    int source_height,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    int flipped_variant,
    CSB_V1_D1L2D1R2WallBlitStatsPc34 *stats);

uint16_t csb_v1_viewport_d1l2_d1r2_wall_preserve_first_thing_pc34(
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *spec,
    uint16_t first_thing);

int csb_v1_viewport_d1l2_d1r2_wall_pc34_compat_run(
    CSB_V1_D1L2D1R2WallRunResultPc34 *out_result);

const char *csb_v1_viewport_d1l2_d1r2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
