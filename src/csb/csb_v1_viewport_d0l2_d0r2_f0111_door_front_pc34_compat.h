#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_F0111_DOOR_FRONT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D0L2_D0R2_F0111_DOOR_FRONT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only source-lock gate, asset-free.
 * ReDMCSB anchors: DUNVIEW.C F0111_DUNGEONVIEW_DrawDoor lines 4218-4337;
 * DUNVIEW.C F0116/F0117 door-front branches at 6442-6460 and 6578-6602;
 * DUNGEON.C F0163 lines 1769-1838, F0164 lines 1840-1905, and F0172 lines
 * 2466-2523; DEFS.H lines 4045-4046 C705/C706 wall zones. CSB-lineage
 * anchors: Viewport.cpp lines 1903-1915, 1930-1944, and 1192-1209.
 */

typedef enum {
    CSB_V1_D0L2_D0R2_F0111_SIDE_D0L2_PC34 = 1,
    CSB_V1_D0L2_D0R2_F0111_SIDE_D0R2_PC34 = 2
} CSB_V1_D0L2D0R2F0111DoorFrontSidePc34;

typedef struct {
    int side;
    const char *lane_name;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int distinct_from_d0l2_d0r2_wall_gate;
    int distinct_from_f0115_thing_pass_gate;
    int relative_depth;
    int relative_lateral;
    int f0128_draw_index;
    int view_square_index;
    int wall_zone;
    int door_zone;
    int f0108_floor_view;
    unsigned int f0115_rear_cell_order;
    unsigned int f0115_front_cell_order;
    int f0111_front_bitmap_id;
    int f0111_door_ornament_view;
    int f0111_open_state_skips_blit;
    int f0111_closed_state_draws_closed_or_destroyed;
    int f0111_destroyed_state_applies_destroyed_mask;
    int f0111_partly_open_state_decrements_state;
    int f0111_final_blit_uses_c10;
    int transparent_color;
    int rear_pass_before_door;
    int door_before_front_pass;
    int f0172_square_aspect_source;
    int f0163_not_called_by_draw;
    int f0164_not_called_by_draw;
    int csb_lineage_f1_two_pass_reference;
    int csb_lineage_f0l1_f0r1_return_only_reference;
    int csb_lineage_open_room_objects_reference;
    int lineage_rear_draw_order_opcode;
    int lineage_front_draw_order_opcode;
    int lineage_room_objects_opcode;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_anchor;
    const char *source_summary;
} CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34;

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
} CSB_V1_D0L2D0R2F0111DoorFrontTracePc34;

size_t csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_count_pc34(void);

const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *
csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_at_pc34(size_t index);

const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *
csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_for_side_pc34(int side);

int csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(
    unsigned int order,
    int ordinal);

uint8_t csb_v1_viewport_d0l2_d0r2_f0111_door_front_blend_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int csb_v1_viewport_d0l2_d0r2_f0111_door_front_compose_pixel_pc34(
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *spec,
    uint8_t base_pixel,
    uint8_t floor_pixel,
    uint8_t rear_pass_pixel,
    uint8_t door_pixel,
    uint8_t front_pass_pixel,
    CSB_V1_D0L2D0R2F0111DoorFrontTracePc34 *out_trace);

int csb_v1_viewport_d0l2_d0r2_f0111_door_front_is_draw_mutating_pc34(
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *spec);

const char *csb_v1_viewport_d0l2_d0r2_f0111_door_front_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
