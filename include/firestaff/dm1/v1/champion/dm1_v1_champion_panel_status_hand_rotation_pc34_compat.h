#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_STATUS_HAND_ROTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_STATUS_HAND_ROTATION_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel status-hand rotation source lock.
 *
 * ReDMCSB anchors: CHAMPION.C F0297:243-298, F0298:270-298,
 * F0300:511-515, F0301:606-614, F0302:662-714; CHAMDRAW.C F0291/F0292;
 * DEFS.H:2088 C30/C033/C034/C035/M070/M516 requested anchor.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CP_STATUS_HAND_ROTATION_CHAMPION_COUNT_PC34 4
#define DM1_V1_CP_STATUS_HAND_ROTATION_ICON_STATE_COUNT_PC34 3
#define DM1_V1_CP_STATUS_HAND_ROTATION_NONE_THING_PC34 0xFFFFu

enum {
    DM1_V1_CP_STATUS_HAND_ROTATION_NORMAL_PC34 = 0,
    DM1_V1_CP_STATUS_HAND_ROTATION_WOUNDED_PC34 = 1,
    DM1_V1_CP_STATUS_HAND_ROTATION_ACTING_PC34 = 2
};

typedef struct {
    int leader_before;
    int leader_after;
    int m516_champion_index;
    int m516_status_hand_zone_pointer;
    int slot_box_index;
    int hand_slot_index;
    int status_box_zone;
    int status_name_zone;
    int status_text_zone;
    int status_box_left;
    int status_box_top;
    int status_box_right;
    int status_box_bottom;
    int action_hand_zone;
    int action_hand_x;
    int action_hand_y;
    int action_hand_width;
    int action_hand_height;
    int f0291_blit_left;
    int f0291_blit_top;
    int f0291_blit_right;
    int f0291_blit_bottom;
    int f0292_text_zone;
    int f0292_text_left;
    int f0292_text_top;
    int f0292_text_right;
    int f0292_text_bottom;
    int border_graphic_shield;
    int border_graphic_fire_shield;
    int border_graphic_spell_shield;
    int border_destination_zone;
    uint16_t leader_hand_before;
    uint16_t slot_thing_before;
    uint16_t leader_hand_after;
    uint16_t slot_thing_after;
    bool transaction_accepted;
    bool transaction_status_route;
    bool transaction_uses_new_leader_m516;
    bool transaction_uses_c30_g0425_when_chest_slot;
    bool transaction_preserves_g0426_open_chest_marker;
    bool f0302_resolved_before_f0291;
    bool f0291_resolved_before_f0292;
    bool f0292_destination_matches_f0291_frame;
    uint32_t outside_left_before;
    uint32_t outside_left_after;
    uint32_t outside_right_before;
    uint32_t outside_right_after;
} dm1_v1_champion_panel_status_hand_rotation_frame_t;

typedef struct {
    int leader_index;
    int state;
    int hand_slot_index;
    uint16_t actual_hand_thing;
    uint16_t wounds;
    int acting_ordinal;
    int selected_graphic;
    int expected_graphic;
    int selected_zone;
    bool selected_from_actual_new_leader_hand;
} dm1_v1_champion_panel_status_hand_rotation_icon_t;

typedef struct {
    int leader_index;
    int action_hand_zone_before;
    int action_hand_zone_after;
    uint16_t g0426_open_chest_before;
    uint16_t g0426_open_chest_after;
    uint16_t g0425_chest_slot_before;
    uint16_t g0425_chest_slot_after;
    bool rotated_state_before;
    bool rotated_state_after;
    bool f0300_clears_c30_or_m516_slot_before_close;
    bool f0301_skipped_for_empty_close;
    bool f0292_close_redraw_after_clear;
    uint32_t outside_left_before;
    uint32_t outside_left_after;
    uint32_t outside_right_before;
    uint32_t outside_right_after;
} dm1_v1_champion_panel_status_hand_rotation_close_t;

typedef struct {
    bool contract_only;
    bool disjoint_from_slot_priority_gate;
    bool disjoint_from_second_leader_gate;
    int source_chain_count;
    int source_chain[8];
    dm1_v1_champion_panel_status_hand_rotation_frame_t
        frames[DM1_V1_CP_STATUS_HAND_ROTATION_CHAMPION_COUNT_PC34];
    dm1_v1_champion_panel_status_hand_rotation_icon_t
        icons[DM1_V1_CP_STATUS_HAND_ROTATION_ICON_STATE_COUNT_PC34];
    dm1_v1_champion_panel_status_hand_rotation_close_t close_path;
    uint32_t deterministic_hash;
} dm1_v1_champion_panel_status_hand_rotation_model_t;

const char *
dm1_v1_champion_panel_status_hand_rotation_source_pc34(void);

bool dm1_v1_champion_panel_status_hand_rotation_build_pc34(
    dm1_v1_champion_panel_status_hand_rotation_model_t *out_model);

#ifdef __cplusplus
}
#endif

#endif
