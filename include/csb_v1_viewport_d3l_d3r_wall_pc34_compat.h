#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3L_D3R_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3L_D3R_WALL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 D3L/D3R side-wall wall-composition source lock.
 * ReDMCSB anchors: DUNVIEW.C F0116 lines 6361-6480, F0117 lines
 * 6500-6622, F0104 lines 3113-3156, F0105 lines 3185-3247, F0107
 * lines 3502-3938, F0128 lines 8478-8500; DEFS.H line 2088,
 * 2608-2609, 2698-2702, 2752-2754, 4045-4046; CSB-lineage
 * Viewport.cpp lines 1192-1209 and 1903-1915.
 */

typedef enum {
    CSB_V1_D3L_D3R_WALL_SIDE_D3L_PC34 = 0,
    CSB_V1_D3L_D3R_WALL_SIDE_D3R_PC34 = 1
} CSB_V1_D3LD3RWallSidePc34;

typedef struct {
    int copied_pixels;
    int transparent_pixels;
    int clipped_pixels;
    int rejected;
} CSB_V1_D3LD3RWallBlitStatsPc34;

typedef struct {
    int side;
    const char *name;
    int redmcsb_function_number;
    int view_square;
    int relative_depth;
    int relative_lateral;
    int f0128_order_index;
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
    int side_ornament_square_aspect_slot;
    int front_ornament_square_aspect_slot;
    int side_wall_ornament_view;
    int front_wall_ornament_view;
    int floor_ornament_view;
    int native_wall_blit_function;
    int flipped_wall_blit_function;
    int wall_blit_order_index;
    int side_f0107_order_index;
    int front_f0107_order_index;
    int front_alcove_f0115_order;
    int c10_transparent_color;
    int f0107_depth3_scale;
    int f0107_depth3_palette;
    int lineage_open_room_shape;
    int lineage_door_front_overlay_shape;
    const char *redmcsb_anchor;
} CSB_V1_D3LD3RWallSpecPc34;

typedef struct {
    int ok;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int d3l_before_d3r;
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
} CSB_V1_D3LD3RWallTracePc34;

size_t csb_v1_viewport_d3l_d3r_wall_spec_count_pc34(void);

const CSB_V1_D3LD3RWallSpecPc34 *
csb_v1_viewport_d3l_d3r_wall_spec_at_pc34(size_t index);

const CSB_V1_D3LD3RWallSpecPc34 *
csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(int side);

int csb_v1_viewport_d3l_d3r_wall_trace_pair_pc34(
    int use_flipped_wall_bitmaps,
    int front_wall_ornament_is_alcove,
    CSB_V1_D3LD3RWallTracePc34 *out_trace);

int csb_v1_viewport_d3l_d3r_wall_apply_c10_frame_clip_pc34(
    const CSB_V1_D3LD3RWallSpecPc34 *spec,
    const uint8_t *source,
    int source_width,
    int source_height,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    int flipped_variant,
    CSB_V1_D3LD3RWallBlitStatsPc34 *stats);

uint8_t csb_v1_viewport_d3l_d3r_wall_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int csb_v1_viewport_d3l_d3r_wall_pc34_compat_run(
    CSB_V1_D3LD3RWallTracePc34 *out_trace);

const char *csb_v1_viewport_d3l_d3r_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
