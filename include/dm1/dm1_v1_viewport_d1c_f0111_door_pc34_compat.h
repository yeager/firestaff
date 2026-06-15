#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1C_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1C_F0111_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D1C_F0111_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D1C_F0111_MASK0X4000_PC34 0x4000
#define DM1_V1_D1C_F0111_C6_UNKNOWN_PC34 6

typedef enum {
    DM1_V1_D1C_F0111_DOOR_BRANCH_OPEN_PC34 = 0,
    DM1_V1_D1C_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34 = 1,
    DM1_V1_D1C_F0111_DOOR_BRANCH_CLOSED_PC34 = 2,
    DM1_V1_D1C_F0111_DOOR_BRANCH_DESTROYED_PC34 = 3,
    DM1_V1_D1C_F0111_DOOR_BRANCH_INVALID_PC34 = -1
} DM1_V1_D1CF0111DoorBranchPc34;

typedef struct {
    int x1;
    int x2;
    int y1;
    int y2;
    int byte_width;
    int height;
    int source_x;
    int source_y;
} DM1_V1_D1CF0111DoorFramePc34;

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_pixel_parity;
    int no_game_data_load;
    int view_square_d1c;
    int element_door_front;
    int f0128_dispatch_order;
    int f0128_relative_depth;
    int f0128_relative_lane;
    int f0124_function_number;
    int sibling_f0121_is_d2c_not_d1c;
    int hidden_f0118_can_name_f0124;
    int wall_zone_d1c;
    int door_zone_d1c;
    int door_frame_top_zone;
    int door_frame_left_zone;
    int door_frame_right_zone;
    int floor_view_d1c;
    int door_native_width;
    int door_native_height;
    int door_native_byte_count;
    int door_ornament_view;
    int doorpass1_order;
    int doorpass2_order;
    int door_state_open;
    int door_state_closed_one_fourth;
    int door_state_closed_half;
    int door_state_closed_three_fourth;
    int door_state_closed;
    int door_state_destroyed;
    int first_door_set_graphic;
    int door_set_graphic_count;
    int d1c_door_bitmap_offset;
    const char *route_name;
    const char *f0111_anchor;
    const char *f0124_anchor;
    const char *f0128_anchor;
    const char *f0118_anchor;
    const char *f0121_sibling_anchor;
    const char *defs_anchor;
    const char *bitmap_indices_anchor;
    const char *lineage_anchor;
} DM1_V1_D1CF0111DoorSpecPc34;

typedef struct {
    int input_state;
    int decremented_state;
    int branch;
    int draws_any_bitmap;
    int closed_frame_selected;
    int left_horizontal_selected;
    int right_horizontal_selected;
    int first_half_zone;
    int first_half_clip_zone;
    int second_half_zone;
    int horizontal_mask_applied;
    int first_half_transparent_color;
    int second_half_transparent_color;
    int native_bitmap_index;
    int selected_frame_state_index;
    DM1_V1_D1CF0111DoorFramePc34 closed_or_destroyed;
    DM1_V1_D1CF0111DoorFramePc34 selected_left_horizontal;
    DM1_V1_D1CF0111DoorFramePc34 selected_right_horizontal;
} DM1_V1_D1CF0111DoorStateTracePc34;

size_t dm1_v1_viewport_d1c_f0111_door_spec_count_pc34(void);
const DM1_V1_D1CF0111DoorSpecPc34 *
dm1_v1_viewport_d1c_f0111_door_spec_at_pc34(size_t index);
const DM1_V1_D1CF0111DoorSpecPc34 *
dm1_v1_viewport_d1c_f0111_door_spec_for_square_pc34(int view_square);

DM1_V1_D1CF0111DoorBranchPc34
dm1_v1_viewport_d1c_f0111_door_branch_pc34(
    const DM1_V1_D1CF0111DoorSpecPc34 *spec,
    int door_state);

int dm1_v1_viewport_d1c_f0111_door_native_bitmap_index_pc34(
    const DM1_V1_D1CF0111DoorSpecPc34 *spec,
    int door_set0,
    int door_set1,
    int door_type);

const char *dm1_v1_viewport_d1c_f0111_door_frame_name_pc34(
    const DM1_V1_D1CF0111DoorSpecPc34 *spec,
    int door_state,
    int right_half);

int dm1_v1_viewport_d1c_f0111_door_state_trace_pc34(
    const DM1_V1_D1CF0111DoorSpecPc34 *spec,
    int door_state,
    int horizontal_door,
    int door_set0,
    int door_set1,
    int door_type,
    DM1_V1_D1CF0111DoorStateTracePc34 *out);

int dm1_v1_viewport_d1c_f0111_door_synthetic_blit_pc34(
    const uint8_t *source,
    uint8_t *destination,
    size_t pixel_count,
    int *out_c10_skipped);

const char *dm1_v1_viewport_d1c_f0111_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
