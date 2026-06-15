#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_SECOND_LEADER_HAND_SLOT_PRIORITY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_SECOND_LEADER_HAND_SLOT_PRIORITY_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel second-leader hand-slot priority source lock.
 *
 * Contract-only extension of the general hand-slot priority gate.  This slice
 * pins the case where the leader is champion index 1 and the clicked status
 * hand slot is slot-box 3, so the F0302 status-hand route targets champion 1's
 * action hand while F0292 redraws the second status box.
 *
 * ReDMCSB anchors:
 * - CHAMPION.C F0302:677-684,688-712: status hand -> leader hand -> storage
 *   transaction order and final F0292 redraw.
 * - CHAMPION.C F0297/F0298:243-298, F0300:511-515, F0301:606-614: leader
 *   hand, C30/G0425 clear, and slot write side effects.
 * - CHAMDRAW.C F0287:307-342: champion-indexed C195+bar zones and C12/colour
 *   blank/fill split.
 * - CHAMDRAW.C F0291:632-651: C033/C034/C035 18x18 hand-slot box cascade.
 * - CHAMDRAW.C F0292:771-815,843-895,1019-1051: C151..C154 status-box fill,
 *   PC34 C11/C09 name-color cascade, champion-icon redraw.
 * - DEFS.H:780-817,1878,2178-2199,3779-3807,5700-5881: slot, M070, graphic,
 *   zone, party, inventory, chest, and M516 anchors.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CP_SECOND_LEADER_CHAMPION_INDEX_PC34 1
#define DM1_V1_CP_SECOND_LEADER_SLOT_BOX_INDEX_PC34 3
#define DM1_V1_CP_SECOND_LEADER_STATUS_ZONE_PC34 152
#define DM1_V1_CP_SECOND_LEADER_NAME_ZONE_PC34 160
#define DM1_V1_CP_SECOND_LEADER_NAME_TEXT_ZONE_PC34 164
#define DM1_V1_CP_SECOND_LEADER_ICON_ZONE_PC34 114
#define DM1_V1_CP_SECOND_LEADER_READY_HAND_ZONE_PC34 213
#define DM1_V1_CP_SECOND_LEADER_ACTION_HAND_ZONE_PC34 214

typedef struct {
    int champion_index;
    int leader_index;
    int slot_box_index;
    int target_slot_index;
    int status_zone;
    int status_x;
    int status_y;
    int status_width;
    int status_height;
    int status_fill_color;
    int name_zone;
    int name_text_zone;
    int leader_name_color;
    int nonleader_name_color;
    int hp_bar_zone;
    int hp_bar_x;
    int hp_bar_y;
    int hp_blank_height;
    int hp_fill_height;
    int hp_blank_color;
    int hp_fill_color;
    int icon_zone;
    int icon_width;
    int icon_height;
    int icon_fill_color;
    int action_hand_zone;
    int action_hand_x;
    int action_hand_y;
    int action_hand_width;
    int action_hand_height;
    int action_hand_graphic;
    bool status_hand_precedes_inventory;
    bool leader_hand_precedes_storage_write;
    bool backpack_precedes_belt;
    bool f0292_final_draw_state;
    bool contract_only;
    bool transaction_accepted;
    bool transaction_status_hand_route;
    bool transaction_inventory_route;
    bool transaction_leader_is_target;
    int transaction_target_champion_index;
    int transaction_target_champion_ordinal;
    int transaction_target_slot_index;
    int transaction_call_sequence_count;
    int transaction_call_sequence[8];
    uint32_t deterministic_hash;
} dm1_v1_champion_panel_second_leader_hand_slot_priority_model_t;

const char *
dm1_v1_champion_panel_second_leader_hand_slot_priority_source_pc34(void);

bool dm1_v1_champion_panel_second_leader_hand_slot_priority_build_pc34(
    dm1_v1_champion_panel_second_leader_hand_slot_priority_model_t *out_model);

#ifdef __cplusplus
}
#endif

#endif
