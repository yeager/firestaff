#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_WALL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D2L/D2R wall-composition source lock.
 * ReDMCSB anchors: DUNVIEW.C F0119 lines 6900-6973, F0120 lines
 * 7051-7166, F0104 lines 3113-3156, F0105 lines 3185-3247, F0107
 * lines 3502-3938, F0128 lines 8503-8521; DEFS.H lines 2088,
 * 2582-2583, 2703-2707, 3430-3431, 4050-4051; CSB-lineage
 * Viewport.cpp lines 1192-1209 and 1903-1915.
 */

#define DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 72
#define DM1_V1_D2L_D2R_WALL_SOURCE_PIXEL_WIDTH_PC34 144
#define DM1_V1_D2L_D2R_WALL_SOURCE_HEIGHT_PC34 71
#define DM1_V1_D2L_D2R_WALL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D2L_D2R_WALL_MAX_BLITS_PC34 16

typedef enum {
    DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34 = 0,
    DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34 = 1
} DM1_V1_D2LD2RWallSidePc34;

typedef enum {
    DM1_V1_D2L_D2R_WALL_BLIT_REAR_BACKDROP_PC34 = 0,
    DM1_V1_D2L_D2R_WALL_BLIT_C10_FRAME_TOP_PC34 = 1,
    DM1_V1_D2L_D2R_WALL_BLIT_C10_FRAME_SIDE_PC34 = 2,
    DM1_V1_D2L_D2R_WALL_BLIT_WALL_BITMAP_PC34 = 3,
    DM1_V1_D2L_D2R_WALL_BLIT_SIDE_ORNAMENT_PC34 = 4,
    DM1_V1_D2L_D2R_WALL_BLIT_FRONT_ORNAMENT_PC34 = 5,
    DM1_V1_D2L_D2R_WALL_BLIT_FRONT_FIRST_BACKDROP_PC34 = 6
} DM1_V1_D2LD2RWallBlitKindPc34;

typedef struct {
    int copied_pixels;
    int transparent_pixels;
    int clipped_pixels;
    int rejected;
} DM1_V1_D2LD2RWallBlitStatsPc34;

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
    int c10_transparent_color;
    int rear_backdrop_order_index;
    int frame_top_order_index;
    int frame_side_order_index;
    int wall_bitmap_order_index;
    int side_ornament_order_index;
    int front_ornament_order_index;
    int front_first_backdrop_order_index;
    int c10_frame_side_is_flipped;
    int lineage_open_room_shape;
    int lineage_door_front_overlay_shape;
    const char *redmcsb_anchor;
} DM1_V1_D2LD2RWallSpecPc34;

typedef struct {
    int side;
    int kind;
    int order_index;
    int view_square;
    int view_wall_index;
    int wall_index;
    int wall_zone;
    int blit_function;
    int transparent_color;
    int flipped;
    int dst_x;
    int dst_y;
    int source_x;
    int source_y;
    uint8_t source_transparent_sample;
    uint8_t source_opaque_sample;
    uint8_t destination_before_transparent;
    uint8_t destination_after_transparent;
    uint8_t destination_after_opaque;
    DM1_V1_D2LD2RWallBlitStatsPc34 stats;
} DM1_V1_D2LD2RWallBlitRecordPc34;

typedef struct {
    int use_flipped_wall_bitmaps;
    int front_wall_ornament_is_alcove;
} DM1_V1_D2LD2RWallComposeStatePc34;

typedef struct {
    const DM1_V1_D2LD2RWallSpecPc34 *spec;
    int in_clip;
    int writes_pixel;
    int transparent_skip;
    int no_write_metadata;
    int row;
    int viewport_x;
    int source_x;
    int source_y;
    size_t source_offset;
    size_t viewport_offset;
    uint8_t pixel_before;
    uint8_t source_pixel;
    uint8_t pixel_after;
} DM1_V1_D2LD2RWallFramePixelPc34;

typedef struct {
    int ok;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int blit_count;
    int d2l_blit_count;
    int d2r_blit_count;
    int d2l_before_d2r;
    int d2l_before_d2c;
    int d2r_before_d2c;
    int d2c_order_index;
    int all_blits_use_c10;
    int all_blits_preserve_c10;
    int f0104_calls;
    int f0105_calls;
    int f0107_side_calls;
    int f0107_front_calls;
    int f0115_first_backdrop_calls;
    int first_wall_zone;
    int second_wall_zone;
    int first_wall_index;
    int second_wall_index;
    int d2l_view_square;
    int d2r_view_square;
    int d2l_side_ornament_view;
    int d2r_side_ornament_view;
    int d2l_front_ornament_view;
    int d2r_front_ornament_view;
    DM1_V1_D2LD2RWallBlitRecordPc34 blits[DM1_V1_D2L_D2R_WALL_MAX_BLITS_PC34];
} DM1_V1_D2LD2RWallTracePc34;

size_t dm1_v1_viewport_d2l_d2r_wall_spec_count_pc34(void);

const DM1_V1_D2LD2RWallSpecPc34 *
dm1_v1_viewport_d2l_d2r_wall_spec_at_pc34(size_t index);

const DM1_V1_D2LD2RWallSpecPc34 *
dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(int side);

int dm1_v1_viewport_d2l_d2r_wall_compose(
    const DM1_V1_D2LD2RWallComposeStatePc34 *state,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    DM1_V1_D2LD2RWallTracePc34 *out_trace);

uint8_t dm1_v1_viewport_d2l_d2r_wall_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int dm1_v1_viewport_d2l_d2r_wall_apply_frame_pixel_pc34(
    const DM1_V1_D2LD2RWallSpecPc34 *spec,
    int viewport_y,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D2LD2RWallFramePixelPc34 *out);

int dm1_v1_viewport_d2l_d2r_wall_pc34_compat_run(
    DM1_V1_D2LD2RWallTracePc34 *out_trace);

const char *dm1_v1_viewport_d2l_d2r_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
