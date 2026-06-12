#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L_D0R_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L_D0R_F0111_DOOR_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D0L_D0R_F0111_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0L_D0R_F0111_MASK0X4000_PC34 0x4000
#define DM1_V1_D0L_D0R_F0111_C6_UNKNOWN_PC34 6
#define DM1_V1_D0L_D0R_F0111_DOOR_STATE_OPEN_PC34 0
#define DM1_V1_D0L_D0R_F0111_DOOR_STATE_CLOSED_ONE_FOURTH_PC34 1
#define DM1_V1_D0L_D0R_F0111_DOOR_STATE_CLOSED_HALF_PC34 2
#define DM1_V1_D0L_D0R_F0111_DOOR_STATE_CLOSED_THREE_FOURTH_PC34 3
#define DM1_V1_D0L_D0R_F0111_DOOR_STATE_CLOSED_PC34 4
#define DM1_V1_D0L_D0R_F0111_DOOR_STATE_DESTROYED_PC34 5

typedef enum {
    DM1_V1_D0L_D0R_F0111_SIDE_D0L_PC34 = 1,
    DM1_V1_D0L_D0R_F0111_SIDE_D0R_PC34 = 2
} DM1_V1_D0LD0RF0111DoorSidePc34;

typedef struct {
    int x1;
    int x2;
    int y1;
    int y2;
    int byte_width;
    int height;
    int source_x;
    int source_y;
} DM1_V1_D0LD0RF0111DoorFramePc34;

typedef struct {
    int side;
    const char *label;
    int requested_view_square;
    int source_view_square;
    int f0128_update_line;
    int f0128_draw_line;
    int f0125_or_f0126_line_start;
    int f0125_or_f0126_line_end;
    int requested_depth;
    int requested_lateral;
    int source_depth;
    int source_lateral;
    int wall_zone;
    int door_zone;
    int door_frame_left_zone;
    int door_frame_right_zone;
    int floor_view;
    int door_front_bitmap;
    int door_ornament_view;
    unsigned int pass1_cell_order;
    unsigned int pass2_cell_order;
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    bool f0128_dispatches_requested_side;
    bool d0_side_dispatch_is_not_the_f0111_compositor;
    bool f0116_or_f0117_owns_door_front;
    bool f0104_wall_caller_present;
    bool f0105_mirror_wall_caller_present;
    bool f0107_wall_ornament_caller_present;
    bool f0163_not_called_by_draw;
    bool f0164_not_called_by_draw;
    bool f0172_square_aspect_source;
    DM1_V1_D0LD0RF0111DoorFramePc34 closed_or_destroyed;
    DM1_V1_D0LD0RF0111DoorFramePc34 vertical[3];
    DM1_V1_D0LD0RF0111DoorFramePc34 left_horizontal[3];
    DM1_V1_D0LD0RF0111DoorFramePc34 right_horizontal[3];
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_f0125_f0126_anchor;
    const char *redmcsb_f0104_f0105_f0107_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *redmcsb_defs_anchor;
    const char *redmcsb_door_front_caller_anchor;
} DM1_V1_D0LD0RF0111DoorSpecPc34;

typedef struct {
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *spec;
    bool in_closed_clip;
    bool floor_transparent;
    bool pass1_transparent;
    bool door_transparent;
    bool pass2_transparent;
    uint8_t destination_before;
    uint8_t after_floor;
    uint8_t after_pass1;
    uint8_t after_door;
    uint8_t after_pass2;
} DM1_V1_D0LD0RF0111DoorPixelTracePc34;

typedef struct {
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *spec;
    int input_state;
    int decremented_state;
    bool skipped_open_guard;
    bool closed_or_destroyed_frame_selected;
    bool vertical_frame_selected;
    bool left_horizontal_selected;
    bool right_horizontal_selected;
    bool horizontal_c6_transparent_blit;
    bool mask0x4000_applied;
    int first_zone;
    int c6_zone;
    int final_zone;
    int zone_shift_x;
    int zone_shift_y;
    DM1_V1_D0LD0RF0111DoorFramePc34 selected_closed_or_vertical;
    DM1_V1_D0LD0RF0111DoorFramePc34 selected_left_horizontal;
    DM1_V1_D0LD0RF0111DoorFramePc34 selected_right_horizontal;
} DM1_V1_D0LD0RF0111DoorStateTracePc34;

typedef struct {
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *spec;
    int direction;
    int origin_x;
    int origin_y;
    int relative_depth;
    int relative_lateral;
    int updated_x;
    int updated_y;
    int f0128_update_line;
    int f0128_draw_line;
    const char *draw_function;
} DM1_V1_D0LD0RF0111DispatchTracePc34;

size_t dm1_v1_viewport_d0l_d0r_f0111_door_count_pc34(void);
const DM1_V1_D0LD0RF0111DoorSpecPc34 *
dm1_v1_viewport_d0l_d0r_f0111_door_at_pc34(size_t index);
const DM1_V1_D0LD0RF0111DoorSpecPc34 *
dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(int side);

uint8_t dm1_v1_viewport_d0l_d0r_f0111_door_blend_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

bool dm1_v1_viewport_d0l_d0r_f0111_door_compose_closed_pixel_pc34(
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *spec,
    uint8_t destination_before,
    uint8_t floor_pixel,
    uint8_t pass1_pixel,
    uint8_t door_pixel,
    uint8_t pass2_pixel,
    DM1_V1_D0LD0RF0111DoorPixelTracePc34 *out);

bool dm1_v1_viewport_d0l_d0r_f0111_door_state_trace_pc34(
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *spec,
    int door_state,
    bool vertical,
    DM1_V1_D0LD0RF0111DoorStateTracePc34 *out);

int dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(
    unsigned int order,
    int ordinal);

bool dm1_v1_viewport_d0l_d0r_f0111_dispatch_pc34(
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *spec,
    int direction,
    int origin_x,
    int origin_y,
    DM1_V1_D0LD0RF0111DispatchTracePc34 *out);

const char *
dm1_v1_viewport_d0l_d0r_f0111_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
