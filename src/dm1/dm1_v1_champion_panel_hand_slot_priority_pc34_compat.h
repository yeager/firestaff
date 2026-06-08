#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_PC34_COMPAT_H

/*
 * DM1 V1 champion panel hand-slot priority source-lock contract.
 *
 * Source-lock anchors:
 * - ReDMCSB CHAMPION.C F0302:677-684 routes status-box hand slot boxes
 *   0..7 before inventory slot boxes, using M070_HAND_SLOT_INDEX.
 * - ReDMCSB CHAMPION.C F0297:243-298 owns leader-hand put/remove side
 *   effects, including the M516_CHAMPIONS[].Load update and F0292 redraw.
 * - ReDMCSB CHAMPION.C F0300:511-515 clears C30+ slots through
 *   G0425_aT_ChestSlots; CHAMPION.C F0301:606-614 writes C30+ slots through
 *   the same chest-slot array.
 * - ReDMCSB DEFS.H anchors C30_SLOT_CHEST_1, G0425_aT_ChestSlots,
 *   G0426_T_OpenChest, G0423_i_InventoryChampionOrdinal,
 *   G0305_ui_PartyChampionCount, M070_HAND_SLOT_INDEX, and M516_CHAMPIONS.
 * - ReDMCSB CHAMDRAW.C F0291/F0292 owns the panel redraw contract used by
 *   F0297/F0300/F0301/F0302 after hand-slot mutations.
 * - ReDMCSB PANEL.C F0344/F0345 remains a panel-redraw dependency for the
 *   leader action-hand BUG0_39 path; this gate records the dependency only.
 *
 * Contract only: this module models routing, guard priority, helper order, and
 * synthetic slot state. It does not render bitmaps, load game data, or lift
 * original ReDMCSB implementation code.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34 4
#define DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_MAX_SLOTS_PC34 38
#define DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CHAIN_COUNT_PC34 4
#define DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_TRACE_MAX_PC34 8

#define DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_THING_NONE_PC34 0xFFFFu
#define DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_C08_INVENTORY_FIRST_PC34 8
#define DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_C30_CHEST_FIRST_PC34 30
#define DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_C38_CHEST_SLOTBOX_FIRST_PC34 38

typedef enum {
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_STATUS_HAND_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_LEADER_HAND_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_BACKPACK_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_BELT_PC34
} dm1_v1_champion_panel_hand_slot_priority_source_t;

typedef enum {
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_STATUS_HAND_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_INVENTORY_HAND_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_BACKPACK_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_BELT_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_BODY_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_CHEST_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_INVALID_PC34
} dm1_v1_champion_panel_hand_slot_priority_family_t;

typedef enum {
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_NONE_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_CANDIDATE_ACTIVE_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_PARTY_BOUND_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_INVENTORY_CHAMPION_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_DEAD_CHAMPION_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_INVALID_INVENTORY_ORDINAL_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_INVALID_SLOT_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_EMPTY_HAND_EMPTY_SLOT_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_MASK_REJECT_PC34
} dm1_v1_champion_panel_hand_slot_priority_return_t;

typedef enum {
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_NONE_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0077_ENABLE_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0298_REMOVE_LEADER_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0300_CLEAR_SLOT_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0297_PUT_LEADER_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0301_WRITE_SLOT_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0292_DRAW_STATE_PC34,
    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0078_DISABLE_PC34
} dm1_v1_champion_panel_hand_slot_priority_call_t;

typedef struct {
    const char *f0302_status_route_anchor;
    const char *leader_hand_anchor;
    const char *c30_clear_anchor;
    const char *c30_write_anchor;
    const char *defs_anchor;
    const char *chamdraw_anchor;
    const char *panel_anchor;
    const char *dunview_non_claim_anchor;
    const char *contract_scope;
} dm1_v1_champion_panel_hand_slot_priority_evidence_t;

typedef struct {
    bool contract_only;
    bool models_status_hand_before_inventory;
    bool models_leader_hand_before_storage_write;
    bool models_backpack_before_belt_priority;
    bool models_c30_chest_array_clear_and_write;
    bool records_f0292_redraw_contract;
    bool records_panel_dependency_only;
    bool no_real_graphics_or_data_load;
} dm1_v1_champion_panel_hand_slot_priority_invariant_t;

typedef struct {
    uint16_t slot_box_index;
    uint16_t party_champion_count;
    int16_t inventory_champion_ordinal;
    uint16_t candidate_champion_ordinal;
    int16_t leader_champion_index;
    uint16_t current_health[DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34];
    uint16_t leader_hand_thing;
    uint16_t slot_thing;
    uint16_t leader_object_allowed_slots;
    uint16_t slot_mask;
    bool action_hand_contains_panel_object;
} dm1_v1_champion_panel_hand_slot_priority_input_t;

typedef struct {
    dm1_v1_champion_panel_hand_slot_priority_invariant_t invariant;
    dm1_v1_champion_panel_hand_slot_priority_evidence_t evidence;
    dm1_v1_champion_panel_hand_slot_priority_source_t
        source_chain[DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CHAIN_COUNT_PC34];
    dm1_v1_champion_panel_hand_slot_priority_call_t
        call_sequence[DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_TRACE_MAX_PC34];
    int source_chain_count;
    int call_sequence_count;
    bool accepted;
    bool null_input_defaults_used;
    dm1_v1_champion_panel_hand_slot_priority_return_t return_reason;
    dm1_v1_champion_panel_hand_slot_priority_family_t slot_family;
    uint16_t slot_box_index;
    int16_t target_champion_index;
    int16_t target_champion_ordinal;
    uint16_t target_slot_index;
    bool status_hand_route;
    bool inventory_route;
    bool c30_chest_slot_route;
    bool leader_is_target;
    bool leader_hand_checked_before_storage_write;
    bool c30_clear_uses_g0425;
    bool c30_write_uses_g0425;
    bool f0292_final_draw_state;
    bool f0297_intermediate_draw_state;
    bool panel_redraw_dependency;
    bool backpack_precedes_belt_in_priority_chain;
    uint16_t leader_hand_before;
    uint16_t slot_thing_before;
    uint16_t leader_hand_after;
    uint16_t slot_thing_after;
} dm1_v1_champion_panel_hand_slot_priority_result_t;

dm1_v1_champion_panel_hand_slot_priority_input_t
dm1_v1_champion_panel_hand_slot_priority_default_input_pc34(void);

dm1_v1_champion_panel_hand_slot_priority_result_t
dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(
    const dm1_v1_champion_panel_hand_slot_priority_input_t *input);

dm1_v1_champion_panel_hand_slot_priority_family_t
dm1_v1_champion_panel_hand_slot_priority_family_for_slot_pc34(
    uint16_t slot_index,
    bool status_hand_route);

const dm1_v1_champion_panel_hand_slot_priority_evidence_t *
dm1_v1_champion_panel_hand_slot_priority_evidence_pc34(void);

const char *
dm1_v1_champion_panel_hand_slot_priority_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_PC34_COMPAT_H */
