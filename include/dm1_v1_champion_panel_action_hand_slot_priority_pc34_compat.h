#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_PC34_COMPAT_H

/*
 * DM1 V1 champion panel action-hand slot-box click priority contract.
 *
 * Source-lock anchors:
 * - ReDMCSB CHAMPION.C F0302:662-714 dispatches slot-box clicks, including
 *   candidate/inventory/dead champion guards, status hand slot mapping,
 *   chest slot redirection, AllowedSlots mask tests, and the
 *   F0298/F0300/F0297/F0301/F0292 call order.
 * - ReDMCSB CHAMPION.C F0297:243-268, F0298, F0300, F0301, and F0292 own
 *   the helper side effects that this gate records synthetically.
 * - ReDMCSB DEFS.H:434,810,1874-1878,5332,6886-6894,7946 anchors
 *   C0xFFFF_THING_NONE, C30_SLOT_CHEST_1, C08_SLOT_BOX_INVENTORY_FIRST_SLOT,
 *   M070_HAND_SLOT_INDEX, G0038_ai_Graphic562_SlotMasks, F0077/F0078, F0302.
 * - ReDMCSB COMPILE.H:1038-1039 anchors M000/M001 ordinal conversion.
 * - ReDMCSB COMMAND.C F0359:1978-1982 and 2173-2177 route C020..C065
 *   click commands into F0302 using the slot-box index delta.
 *
 * Contract only: this slice models dispatch, trace order, guard priority, and
 * resulting synthetic hand/slot state. It does not call real M11 graphics or
 * claim real-asset bitmap parity.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34 4
#define DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_SLOTS_PC34 38
#define DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_OBJECT_INFO_COUNT_PC34 16
#define DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_TRACE_MAX_PC34 10

#define DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34 0xFFFFu
#define DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C08_INVENTORY_FIRST_PC34 8
#define DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C30_CHEST_FIRST_PC34 30

typedef enum {
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_NONE_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0077_ENABLE_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0298_REMOVE_LEADER_HAND_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0300_REMOVE_SLOT_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0297_PUT_LEADER_HAND_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0301_ADD_SLOT_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0292_DRAW_STATE_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0078_DISABLE_PC34
} dm1_v1_champion_panel_action_hand_slot_priority_call_t;

typedef enum {
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_NONE_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_CANDIDATE_ACTIVE_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_PARTY_BOUND_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_INVENTORY_CHAMPION_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_DEAD_CHAMPION_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_INVALID_INVENTORY_ORDINAL_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_EMPTY_HAND_EMPTY_SLOT_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_LEADER_OBJECT_SLOT_MASK_REJECT_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_INVALID_SLOT_INDEX_PC34
} dm1_v1_champion_panel_action_hand_slot_priority_return_t;

typedef struct {
    uint16_t thing;
    uint16_t allowed_slots;
} dm1_v1_champion_panel_action_hand_slot_priority_object_info_t;

typedef struct {
    const char *function_name;
    const char *slot_dispatch_anchor;
    const char *helper_anchor;
    const char *defs_anchor;
    const char *mouse_anchor;
    const char *command_anchor;
    const char *contract_scope;
    const char *no_real_graphics_claim;
} dm1_v1_champion_panel_action_hand_slot_priority_evidence_t;

typedef struct {
    bool contract_only;
    bool real_m11_graphics_called;
    bool models_status_slotbox_0_to_7;
    bool models_inventory_slotbox_8_to_37;
    bool models_chest_slot_branch;
    bool models_leader_hand_allowed_slots_mask;
    bool records_f0298_f0300_f0297_f0301_order;
    bool records_f0077_f0078_outer_bracket;
    bool records_leader_vs_follower_draw_priority;
    int max_champions;
    int max_slots;
    int chest_slot_count;
} dm1_v1_champion_panel_action_hand_slot_priority_invariant_t;

typedef struct {
    uint16_t slot_box_index;
    uint16_t leader_hand_thing;
    uint16_t party_champion_count;
    int16_t inventory_champion_ordinal;
    uint16_t candidate_champion_ordinal;
    int16_t leader_champion_index;
    uint16_t current_health[DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34];
    uint16_t slots[DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34]
                  [DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_SLOTS_PC34];
    uint16_t chest_slots[DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CHEST_SLOT_COUNT_PC34];
    dm1_v1_champion_panel_action_hand_slot_priority_object_info_t
        object_info[DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_OBJECT_INFO_COUNT_PC34];
    uint16_t slot_masks[DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_SLOTS_PC34];
} dm1_v1_champion_panel_action_hand_slot_priority_input_t;

typedef struct {
    dm1_v1_champion_panel_action_hand_slot_priority_invariant_t invariant;
    dm1_v1_champion_panel_action_hand_slot_priority_evidence_t evidence;
    bool accepted;
    bool null_input_defaults_used;
    dm1_v1_champion_panel_action_hand_slot_priority_return_t return_reason;
    uint16_t slot_box_index;
    int16_t target_champion_index;
    int16_t target_champion_ordinal;
    int16_t leader_champion_index;
    int16_t leader_champion_ordinal;
    uint16_t target_slot_index;
    bool status_slot_box_route;
    bool inventory_slot_box_route;
    bool chest_slot_branch;
    bool leader_is_target_champion;
    bool bug0_39_panel_flicker_route;
    bool f0297_draw_state_suppressed_when_leader_not_target;
    uint16_t leader_hand_before;
    uint16_t slot_thing_before;
    uint16_t leader_hand_after;
    uint16_t slots_after[DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34]
                        [DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_SLOTS_PC34];
    uint16_t chest_slots_after[DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CHEST_SLOT_COUNT_PC34];
    dm1_v1_champion_panel_action_hand_slot_priority_call_t
        call_sequence[DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_TRACE_MAX_PC34];
    int call_sequence_count;
    int f0298_remove_leader_hand_call_count;
    int f0300_remove_slot_call_count;
    int f0297_put_leader_hand_call_count;
    int f0301_add_slot_call_count;
    int f0292_draw_state_call_count;
    int f0292_final_draw_state_call_count;
    int f0292_intermediate_leader_draw_call_count;
    int f0292_draw_state_indices[DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34];
    int f0077_enable_screen_update_call_count;
    int f0078_disable_screen_update_call_count;
    int f0077_sequence_index;
    int f0078_sequence_index;
} dm1_v1_champion_panel_action_hand_slot_priority_result_t;

dm1_v1_champion_panel_action_hand_slot_priority_input_t
M11_GameView_ChampionPanelActionHandSlotPriority_DefaultInput(void);

dm1_v1_champion_panel_action_hand_slot_priority_result_t
M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(
    const dm1_v1_champion_panel_action_hand_slot_priority_input_t *input);

const dm1_v1_champion_panel_action_hand_slot_priority_evidence_t *
M11_GameView_ChampionPanelActionHandSlotPriority_Evidence(void);

const char *M11_GameView_ChampionPanelActionHandSlotPriority_SourceEvidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_PC34_COMPAT_H */
