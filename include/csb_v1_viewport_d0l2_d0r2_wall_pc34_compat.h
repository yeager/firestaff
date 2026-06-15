#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_WALL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 D0L2/D0R2 wall row source-lock.
 * ReDMCSB anchors: DUNVIEW.C F0125 lines 7960-8062, F0126 lines
 * 8064-8162, F0104 lines 3113-3156, F0105 lines 3185-3247,
 * F0115 keep-out lines 4547-4581, F0116 lines 6361-6480 as a
 * D3L body anchor only, F0128 lines 8478-8508 and 8534-8542,
 * and F0127 line 8294 as a D0C follow-up anchor only. DUNGEON.C
 * F0163 lines 1769-1838, F0164 lines 1840-1905, and F0172 lines
 * 2466-2523 anchor thing-list and aspect boundaries. DEFS.H line
 * 2088 anchors C10_COLOR_FLESH and lines 4040-4057 anchor the
 * D*-L/D*-R wall-zone family. CSB-lineage Viewport.cpp lines
 * 1192-1209 and 1903-1915 anchor open side room-object and
 * door-facing overlay bindings.
 */

typedef enum {
    CSB_V1_D0L2_D0R2_WALL_SIDE_D0L2_PC34 = 1,
    CSB_V1_D0L2_D0R2_WALL_SIDE_D0R2_PC34 = 2
} CSB_V1_D0L2D0R2WallSidePc34;

typedef struct {
    int side;
    const char *route_name;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int f0128_draw_order_index;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int view_square_index;
    int wall_element;
    int wall_zone;
    int wall_frame_row;
    int wall_frame_x1;
    int wall_frame_x2;
    int wall_frame_y1;
    int wall_frame_y2;
    int wall_frame_byte_width;
    int wall_frame_height;
    int native_wall_index;
    int flipped_wall_index;
    int f0104_native_route;
    int f0105_flipped_route;
    int f0104_zone_binding_before_f0128_followup;
    int f0105_zone_binding_before_f0128_followup;
    int transparent_color;
    int preserves_c10_transparency;
    int f0115_call_count_for_wall;
    int f0115_thing_pass_keepout;
    int f0111_door_keepout;
    int f0108_floor_ornament_keepout;
    int thing_list_link_mutation;
    int thing_list_unlink_mutation;
    int f0172_square_aspect_read_only;
    int f0127_d0c_followup_after_pair;
    int lineage_relative_cell;
    int lineage_contents_opcode;
    int lineage_draw_order_opcode;
    int lineage_room_objects_opcode;
    int lineage_door_facing_first_order_opcode;
    int lineage_door_facing_second_order_opcode;
    const char *redmcsb_function_anchor;
    const char *redmcsb_wall_anchor;
    const char *source_lines;
} CSB_V1_D0L2D0R2WallRouteSpecPc34;

typedef struct {
    int ok;
    int route_count;
    int dispatch_order_ok;
    int wall_variant_binding_ok;
    int c10_transparency_ok;
    int f0115_keepout_ok;
    int thing_list_keepout_ok;
    int row_followup_ok;
    int lineage_binding_ok;
    int copied_pixels;
    uint16_t first_thing_before;
    uint16_t first_thing_after;
} CSB_V1_D0L2D0R2WallRunResultPc34;

size_t csb_v1_viewport_d0l2_d0r2_wall_route_spec_count_pc34(void);

const CSB_V1_D0L2D0R2WallRouteSpecPc34 *
csb_v1_viewport_d0l2_d0r2_wall_route_spec_at_pc34(size_t index);

const CSB_V1_D0L2D0R2WallRouteSpecPc34 *
csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(int side);

int csb_v1_viewport_d0l2_d0r2_wall_map_viewport_x_to_source_pc34(
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *spec,
    int viewport_x,
    int flipped_variant,
    int *out_source_x);

uint8_t csb_v1_viewport_d0l2_d0r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

int csb_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *spec,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    int viewport_x,
    int viewport_y,
    int flipped_variant);

uint16_t csb_v1_viewport_d0l2_d0r2_wall_preserve_first_thing_pc34(
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *spec,
    uint16_t first_thing);

int csb_v1_viewport_d0l2_d0r2_wall_pc34_compat_run(
    CSB_V1_D0L2D0R2WallRunResultPc34 *out_result);

const char *csb_v1_viewport_d0l2_d0r2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
