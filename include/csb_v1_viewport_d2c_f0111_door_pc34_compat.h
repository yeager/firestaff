#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2C_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2C_F0111_DOOR_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked contract-only gate, not real-asset bitmap parity.
 * ReDMCSB anchors: DUNVIEW.C F0111_DUNGEONVIEW_DrawDoor lines 4218-4337;
 * DUNVIEW.C F0121_DUNGEONVIEW_DrawSquareD2C lines 7244-7389, especially
 * 7313-7341 for the D2C center-door dispatch; DUNVIEW.C F0128 lines
 * 8508-8533 for the D2C relative-movement dispatch order. CSB-lineage
 * Viewport.cpp anchors: requested lines 1903-1915, with the local D2 center
 * array at StdDrawF2DoorFacing lines 1865-1879. ReDMCSB DUNGEON.C F0163
 * and F0164 lines 1769-1840 anchor thing-list link/unlink non-interference.
 */

#define CSB_V1_D2C_F0111_DOOR_PC34_TRANSPARENT_COLOR 10

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int view_square_d2c;
    int view_depth;
    int view_lane;
    int element_door_front;
    int door_native_width;
    int door_native_height;
    int door_native_byte_count;
    int rejects_d1_96x88_byte_count;
    int view_door_ornament_d2lcr;
    int door_button_view_d2c;
    int door_zone_d2c;
    int door_graphic_depth_index;
    int doorpass1_order;
    int doorpass2_order;
    int floor_ornament_before_rear_pass;
    int rear_pass_before_frames;
    int top_track_before_side_frames;
    int side_frames_before_button;
    int button_before_f0111;
    int f0111_before_front_pass;
    int terminal_front_pass_ordered;
    int door_frame_top_zone;
    int door_frame_left_zone;
    int door_frame_right_zone;
    int open_state_skips_f0111;
    int closed_state_uses_base_zone;
    int destroyed_state_uses_base_zone;
    int destroyed_state_applies_c15_mask;
    int partial_state_shifts_zone;
    int horizontal_second_half_mask;
    int transparent_color;
    int f0128_dispatch_after_d2l_d2r;
    int f0128_dispatches_d2c;
    int f0128_dispatch_before_d1l_d1r_d1c;
    int uses_f0119_d2l;
    int uses_f0120_d2r;
    int uses_f0124_d1c;
    int dungeon_f0163_link_noninterference;
    int dungeon_f0164_unlink_noninterference;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_f0121_anchor;
    const char *redmcsb_defs_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *csb_lineage_viewport_anchor;
    const char *door_bitmap_index_symbol;
    const char *door_byte_count_macro;
    const char *door_view_symbol;
    const char *door_frame_symbol;
    const char *door_zone_symbol;
    const char *source_evidence;
} CSB_V1_ViewportD2CF0111DoorPc34Contract;

const CSB_V1_ViewportD2CF0111DoorPc34Contract *
csb_v1_viewport_d2c_f0111_door_pc34_contract(void);

int csb_v1_viewport_d2c_f0111_door_byte_count_pc34(int width, int height);

int csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *contract,
    int door_state);

int csb_v1_viewport_d2c_f0111_door_horizontal_half_zone_pc34(
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *contract,
    int door_state,
    int right_half);

int csb_v1_viewport_d2c_f0111_door_apply_c10_blit_pc34(
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *contract,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

const char *csb_v1_viewport_d2c_f0111_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
