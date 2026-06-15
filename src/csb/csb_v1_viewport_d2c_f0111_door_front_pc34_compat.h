#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2C_F0111_DOOR_FRONT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2C_F0111_DOOR_FRONT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 D2C F0111 door-front composition source lock.
 * ReDMCSB anchors: DUNVIEW.C F0121 lines 7244-7389, especially 7313-7341
 * for F0108, rear F0115, D2C door frames, F0111, and the terminal F0115;
 * DUNVIEW.C F0111 lines 4218-4339 for C10-transparent door drawing;
 * DUNVIEW.C F0115 lines 4547-4581 for door-front cell-order semantics;
 * DUNVIEW.C F0128 lines 8508-8533 for D2C dispatch; DUNGEON.C F0163
 * lines 1769-1838, F0164 lines 1840-1905, and F0172 lines 2466-2523 for
 * thing-list mutation boundaries and square-aspect input; DRAWVIEW.C F0097
 * lines 709-722 for the viewport bitmap handoff; DEFS.H lines 2088,
 * 2533-2559, 2602, 2657-2677, 2756, 2790, 4049, and 4256 for C10, D2C
 * square/aspect/floor/cell/zone fields; CSB-lineage Viewport.cpp lines
 * 1865-1879, 1903-1915, and 1192-1209 for the F2/F1 door-facing and open
 * room-object helper shapes.
 */

#define CSB_V1_D2C_F0111_DOOR_FRONT_TRANSPARENT_COLOR_PC34 10

typedef struct {
    int ok;
    int f0108_calls;
    int f0115_calls;
    int f0111_calls;
    uint8_t after_floor;
    uint8_t after_rear_pass;
    uint8_t after_door;
    uint8_t after_front_pass;
    int floor_transparent;
    int rear_transparent;
    int door_transparent;
    int front_transparent;
} CSB_V1_D2CF0111DoorFrontTracePc34;

typedef struct {
    const char *identifier;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int single_lane_d2c;
    int distinct_from_pass665_d0c;
    int distinct_from_pass703_d0l2_d0r2;
    int distinct_from_d2c_partly_open_gate;
    int distinct_from_d2c_wall_ornament_gate;
    int view_square_index;
    int relative_depth;
    int relative_lateral;
    int f0128_dispatch_line;
    int f0121_function_id;
    int element_door_front;
    int floor_view;
    unsigned int f0115_rear_cell_order;
    unsigned int f0115_front_cell_order;
    int f0111_front_bitmap_id;
    int f0111_door_ornament_view;
    int wall_zone_d2c;
    int door_zone_d2c;
    int door_width;
    int door_height;
    int transparent_color;
    int f0108_before_rear_f0115;
    int rear_f0115_before_f0111;
    int f0111_before_front_f0115;
    int f0115_door_marker_nibble;
    int f0115_front_marker_nibble;
    int f0111_open_state_skips_blit;
    int f0111_closed_state_draws_bitmap;
    int f0111_destroyed_state_applies_mask;
    int f0111_partly_open_state_decrements;
    int f0172_square_aspect_source;
    int f0163_not_called_by_draw;
    int f0164_not_called_by_draw;
    int drawview_f0097_viewport_handoff;
    int csb_lineage_f2_door_facing_reference;
    int csb_lineage_f1_door_facing_reference;
    int csb_lineage_open_room_reference;
    const char *redmcsb_dunview_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *redmcsb_drawview_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_anchor;
    const char *source_evidence;
} CSB_V1_D2CF0111DoorFrontSpecPc34;

size_t csb_v1_viewport_d2c_f0111_door_front_spec_count_pc34(void);

const CSB_V1_D2CF0111DoorFrontSpecPc34 *
csb_v1_viewport_d2c_f0111_door_front_spec_pc34(void);

const CSB_V1_D2CF0111DoorFrontSpecPc34 *
csb_v1_viewport_d2c_f0111_door_front_spec_at_pc34(size_t index);

int csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
    unsigned int order,
    int ordinal);

uint8_t csb_v1_viewport_d2c_f0111_door_front_blend_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int csb_v1_viewport_d2c_f0111_door_front_compose_pixel_pc34(
    const CSB_V1_D2CF0111DoorFrontSpecPc34 *spec,
    uint8_t base_pixel,
    uint8_t floor_pixel,
    uint8_t rear_pass_pixel,
    uint8_t door_pixel,
    uint8_t front_pass_pixel,
    CSB_V1_D2CF0111DoorFrontTracePc34 *out_trace);

int csb_v1_viewport_d2c_f0111_door_front_is_draw_mutating_pc34(
    const CSB_V1_D2CF0111DoorFrontSpecPc34 *spec);

const char *csb_v1_viewport_d2c_f0111_door_front_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
