#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel status-hand rotation source-lock contract.
 *
 * ReDMCSB anchors:
 * - CHAMPION.C F0302:677-684 routes C00..C07 status hand slot boxes before
 *   inventory routing, with champion index = slotbox >> 1 and
 *   M070_HAND_SLOT_INDEX(slotboxindex).
 * - CHAMPION.C F0297:243-298 owns leader-hand put side effects and F0298
 *   leader-hand remove side effects.
 * - CHAMPION.C F0298:270-298 removes the leader-hand object before slot
 *   writes in the occupied-slot swap.
 * - CHAMPION.C F0300:511-515 clears C30+ slots through G0425_aT_ChestSlots.
 * - CHAMPION.C F0301:606-614 writes C30+ slots through G0425_aT_ChestSlots.
 * - CHAMPION.C F0302:688-712 performs the occupied-slot swap after routing.
 * - CHAMDRAW.C F0291:621-630 resolves the status-panel action-hand object
 *   icon path before choosing the slot-box border.
 * - CHAMDRAW.C F0292:898-935 is the champion state redraw tuple; this pass
 *   records the tuple only and does not duplicate mouth/eye or food/water
 *   gates.
 * - DEFS.H:780-817 anchors C00/C01 hand slots, backpack, belt, and C30 chest
 *   slots. In this ReDMCSB snapshot the related G0425/G0426/G0423/G0305/M070
 *   and M516 definitions live outside that range; the verifier records their
 *   exact lines without changing the requested DEFS.H:780-817 anchor.
 * - DEFS.H:2088 anchors C10_COLOR_FLESH as the transparency/color constant
 *   noted by the drawing lineage.
 *
 * Contract only: no bitmap data, mouse state, save state, or GRAPHICS.DAT load.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PANEL_MAX_CHAMPIONS_PC34 4
#define DM1_V1_CHAMPION_PANEL_STATUS_HAND_SLOTS_PC34 2
#define DM1_V1_CHAMPION_PANEL_STATUS_HAND_SLOTBOXES_PC34 8
#define DM1_V1_CHAMPION_PANEL_STATUS_HAND_CHAIN_COUNT_PC34 4

#define DM1_V1_CHAMPION_PANEL_C033_SLOT_BOX_NORMAL_PC34 33
#define DM1_V1_CHAMPION_PANEL_C034_SLOT_BOX_WOUNDED_PC34 34
#define DM1_V1_CHAMPION_PANEL_C035_SLOT_BOX_ACTING_HAND_PC34 35

#define DM1_V1_CHAMPION_PANEL_C00_SLOT_READY_HAND_PC34 0
#define DM1_V1_CHAMPION_PANEL_C01_SLOT_ACTION_HAND_PC34 1
#define DM1_V1_CHAMPION_PANEL_C06_SLOT_POUCH_2_PC34 6
#define DM1_V1_CHAMPION_PANEL_C12_SLOT_QUIVER_LINE1_1_PC34 12
#define DM1_V1_CHAMPION_PANEL_C13_SLOT_BACKPACK_LINE1_1_PC34 13
#define DM1_V1_CHAMPION_PANEL_C29_SLOT_BACKPACK_LINE1_9_PC34 29
#define DM1_V1_CHAMPION_PANEL_C30_SLOT_CHEST_1_PC34 30
#define DM1_V1_CHAMPION_PANEL_C37_SLOT_CHEST_8_PC34 37
#define DM1_V1_CHAMPION_PANEL_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_STATUS_HAND_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_LEADER_HAND_PC34,
    DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_BACKPACK_PC34,
    DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_BELT_PC34
} dm1_v1_champion_panel_status_hand_source_pc34_t;

typedef struct {
    const char *champion_f0302_status_route;
    const char *champion_f0297_leader_put;
    const char *champion_f0298_leader_remove;
    const char *champion_f0300_c30_clear;
    const char *champion_f0301_c30_write;
    const char *champion_f0302_occupied_swap;
    const char *chamdraw_f0291_action_hand;
    const char *chamdraw_f0292_redraw_tuple;
    const char *defs_slots_and_globals;
    const char *defs_flesh_transparency;
    const char *non_duplicate_scope;
} dm1_v1_champion_panel_status_hand_rotation_evidence_pc34_t;

typedef struct {
    uint16_t party_champion_count;
    int16_t acting_champion_index;
    uint16_t wound_masks[DM1_V1_CHAMPION_PANEL_MAX_CHAMPIONS_PC34];
} dm1_v1_champion_panel_status_hand_rotation_input_pc34_t;

typedef struct {
    uint16_t slot_box_index;
    int16_t champion_index;
    uint16_t slot_index;
    bool visible_champion;
    bool action_hand;
    bool acting_action_hand;
    bool wounded_hand;
    bool status_hand_route_before_inventory;
    int native_bitmap_index;
} dm1_v1_champion_panel_status_hand_rotation_box_pc34_t;

typedef struct {
    dm1_v1_champion_panel_status_hand_rotation_evidence_pc34_t evidence;
    dm1_v1_champion_panel_status_hand_source_pc34_t
        priority_chain[DM1_V1_CHAMPION_PANEL_STATUS_HAND_CHAIN_COUNT_PC34];
    dm1_v1_champion_panel_status_hand_rotation_box_pc34_t
        boxes[DM1_V1_CHAMPION_PANEL_STATUS_HAND_SLOTBOXES_PC34];
    int priority_chain_count;
    uint16_t party_champion_count;
    int16_t acting_champion_index;
    int box_count;
    int visible_box_count;
    int c033_normal_count;
    int c034_wounded_count;
    int c035_acting_count;
    bool contract_only;
    bool status_hand_precedes_leader_hand;
    bool leader_hand_precedes_backpack;
    bool backpack_precedes_belt;
    bool no_mouth_eye_gate_duplicate;
    bool no_food_water_gate_duplicate;
} dm1_v1_champion_panel_status_hand_rotation_result_pc34_t;

dm1_v1_champion_panel_status_hand_rotation_input_pc34_t
dm1_v1_champion_panel_status_hand_rotation_default_input_pc34(void);

dm1_v1_champion_panel_status_hand_rotation_result_pc34_t
dm1_v1_champion_panel_status_hand_rotation_plan_pc34(
    const dm1_v1_champion_panel_status_hand_rotation_input_pc34_t *input);

const dm1_v1_champion_panel_status_hand_rotation_evidence_pc34_t *
dm1_v1_champion_panel_status_hand_rotation_evidence_pc34(void);

const char *
dm1_v1_champion_panel_status_hand_rotation_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_PC34_COMPAT_H */
