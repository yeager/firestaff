#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2L_D2R_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2L_D2R_WALL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 D2L/D2R side-wall wall-composition source lock.
 * ReDMCSB anchors: DUNVIEW.C F0119 lines 6900-7049, F0120 lines
 * 7051-7242, F0104 lines 3113-3156, F0105 lines 3185-3247, F0107
 * lines 3502-3938, F0128 lines 8504-8521; DEFS.H line 2088,
 * 2603-2604, 2703-2707, 2755-2757, 3430-3431, 4050-4051;
 * CSB-lineage Viewport.cpp lines 1192-1209 and 1903-1915.
 */

typedef enum {
    CSB_V1_D2L_D2R_WALL_SIDE_D2L_PC34 = 0,
    CSB_V1_D2L_D2R_WALL_SIDE_D2R_PC34 = 1
} CSB_V1_D2LD2RWallSidePc34;

typedef struct {
    int copied_pixels;
    int transparent_pixels;
    int clipped_pixels;
    int rejected;
} CSB_V1_D2LD2RWallBlitStatsPc34;

typedef struct {
    int side;
    const char *name;
    int redmcsb_function_number;
    int view_square;
    int rejected_d3_view_square;
    int relative_depth;
    int relative_lateral;
    int f0128_order_index;
    int d2c_f0128_order_index;
    int wall_element;
    int wall_zone;
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
    int wall_bitmap_source_width;
    int side_ornament_square_aspect_slot;
    int front_ornament_square_aspect_slot;
    int side_wall_ornament_view;
    int front_wall_ornament_view;
    int floor_ornament_view;
    int native_wall_blit_function;
    int flipped_wall_blit_function;
    int rear_d2_outer_order_index;
    int transparent_frame_order_index;
    int wall_blit_order_index;
    int side_f0107_order_index;
    int front_f0107_order_index;
    int front_alcove_f0115_order;
    int c10_transparent_color;
    int f0107_depth2_scale;
    int f0107_depth2_palette;
    int lineage_open_room_shape;
    int lineage_door_front_overlay_shape;
    const char *redmcsb_anchor;
} CSB_V1_D2LD2RWallSpecPc34;

typedef struct {
    int ok;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int d2l_before_d2r;
    int d2l_d2r_before_d2c;
    int rejected_c12_c13_as_d2;
    int rear_d2_outer_calls;
    int transparent_frame_calls;
    int wall_blit_calls;
    int f0104_calls;
    int f0105_calls;
    int f0107_side_calls;
    int f0107_front_calls;
    int f0115_calls;
    int first_wall_zone;
    int second_wall_zone;
    int first_wall_index;
    int second_wall_index;
    int first_blit_function;
    int second_blit_function;
    int front_alcove_uses_zero_order;
    int wall_returns_without_front_alcove;
    int side_ornament_view;
    int front_ornament_view;
    int c10_transparency_preserved;
    int d2_row_origin_preserved;
} CSB_V1_D2LD2RWallTracePc34;

size_t csb_v1_viewport_d2l_d2r_wall_spec_count_pc34(void);

const CSB_V1_D2LD2RWallSpecPc34 *
csb_v1_viewport_d2l_d2r_wall_spec_at_pc34(size_t index);

const CSB_V1_D2LD2RWallSpecPc34 *
csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(int side);

int csb_v1_viewport_d2l_d2r_wall_trace_pair_pc34(
    int use_flipped_wall_bitmaps,
    int front_wall_ornament_is_alcove,
    CSB_V1_D2LD2RWallTracePc34 *out_trace);

int csb_v1_viewport_d2l_d2r_wall_apply_c10_frame_clip_pc34(
    const CSB_V1_D2LD2RWallSpecPc34 *spec,
    const uint8_t *source,
    int source_width,
    int source_height,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    int flipped_variant,
    CSB_V1_D2LD2RWallBlitStatsPc34 *stats);

uint8_t csb_v1_viewport_d2l_d2r_wall_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int csb_v1_viewport_d2l_d2r_wall_pc34_compat_run(
    CSB_V1_D2LD2RWallTracePc34 *out_trace);

const char *csb_v1_viewport_d2l_d2r_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
