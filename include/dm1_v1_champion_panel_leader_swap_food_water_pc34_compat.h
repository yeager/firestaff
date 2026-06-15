#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_PC34_COMPAT_H

/*
 * DM1 V1 champion panel leader-swap (F0302 inventory champion branch) and
 * the food/water/poisoned panel redraw contract (BUG0_39 in F0302:706).
 *
 * Source-lock anchors:
 * - ReDMCSB CHAMPION.C F0302:662-714 dispatches slot-box clicks for the
 *   inventory champion (slotBoxIndex >= C08_SLOT_BOX_INVENTORY_FIRST_SLOT)
 *   to the leader-vs-follower helper chain F0298 -> F0300 -> F0297 -> F0301.
 *   The inventory-champion branch ALWAYS targets the inventory champion
 *   (M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal)).
 * - ReDMCSB CHAMPION.C F0297:243-268 calls F0292_CHAMPION_DrawState only
 *   for the leader (G0411_i_LeaderIndex). The intermediate F0292 call
 *   inside F0297 is the only call that can draw the food/water panel
 *   BEFORE the action hand is filled back in by F0301.
 * - ReDMCSB CHAMPION.C F0300:485,564,575 sets MASK0x0800_PANEL on the
 *   inventory champion when the removed object is a chest or a scroll
 *   (the F0299 modifier path uses CATEGORY_OBJECT_REQUIRES_PANEL_REDRAW).
 * - ReDMCSB CHAMPION.C F0301:622,638 also sets MASK0x8000_ACTION_HAND
 *   and MASK0x0800_PANEL on the inventory champion when adding a chest
 *   or scroll to the action hand slot.
 * - ReDMCSB CHAMDRAW.C F0292:1060-1061 routes the food/water/poisoned
 *   panel redraw through F0345_INVENTORY_DrawPanel_FoodWaterPoisoned
 *   ONLY when (MASK0x0800_PANEL && L0863_B_IsInventoryChampion), and
 *   the F0292:1078-1087 VIEWPORT dirty chain fires for the
 *   inventory champion ONLY. For non-inventory champion draws, the
 *   MASK0x0800_PANEL branch is skipped (L0863_B_IsInventoryChampion=0)
 *   so the food/water panel is not repainted.
 * - ReDMCSB DEFS.H:726,728,729,732 supply MASK0x0200_LOAD, MASK0x0800_PANEL,
 *   MASK0x1000_STATUS_BOX, MASK0x8000_ACTION_HAND, and DEFS.H:3869-3872
 *   supply C500_ZONE_FOOD, C501_ZONE_WATER, C502_ZONE_POISONED, and
 *   C503_ZONE_ARROW_OR_EYE for the panel redraw.
 * - ReDMCSB DEFS.H:5861,5863,5876 anchor G0411_i_LeaderIndex,
 *   G0415_ui_LeaderEmptyHanded, and G0423_i_InventoryChampionOrdinal.
 *
 * Contract only: this slice models the F0302 inventory-champion branch
 * dispatch order, the F0297 internal F0292 call routing, and the
 * MASK0x0800_PANEL / inventory-champion gate for the food/water panel
 * redraw. It does not call real M11 graphics or claim real-asset
 * bitmap parity.
 *
 * Disjoint scope (must not duplicate):
 * - test_dm1_v1_champion_panel_hud_food_water_recompute_pc34_compat
 *   pins F0331 clock-driven recompute + F0355 close hook + status-box
 *   cascade (not the F0302 inventory-champion leader-swap path).
 * - test_dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat
 *   pins the F0302 slot-box dispatch order and helper-call sequence
 *   (not the food/water panel redraw gate or the leader-vs-follower
 *   inventory-champion split).
 * - test_dm1_v1_champion_panel_food_water_status_box_pc34_compat
 *   pins the chest-close -> 67x29 status-box -> food/water panel
 *   draw order (not the F0302 leader-swap intermediate redraw).
 * - test_dm1_v1_champion_panel_pressing_mouth_eye_statusbox_pc34_compat
 *   pins the pressing mouth/eye routes that gate scroll-panel and
 *   object-description transitions (not the F0302 swap path).
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MAX_CHAMPIONS_PC34 4
#define DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_TRACE_MAX_PC34 8

/*
 * Champion attribute masks. ReDMCSB DEFS.H:726-732, lifted to local
 * constants so the gate stays self-contained and so the contract
 * ties back to the ReDMCSB bit positions.
 */
#define DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_LOAD_PC34      0x0200u
#define DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_PANEL_PC34     0x0800u
#define DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_STATUS_BOX_PC34 0x1000u
#define DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_ACTION_HAND_PC34 0x8000u

/*
 * Panel-content sentinel for the food/water/poisoned mouth-press
 * state. ReDMCSB DEFS.H:3869-3872 + PANEL.C F0345.
 */
#define DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_PANEL_PRESSED_PC34 565

typedef enum {
    DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_NONE_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0077_ENABLE_PC34,
    DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0298_REMOVE_LEADER_HAND_PC34,
    DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0300_REMOVE_SLOT_PC34,
    DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0297_PUT_LEADER_HAND_PC34,
    DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0292_DRAW_STATE_INTERMEDIATE_PC34,
    DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0301_ADD_SLOT_PC34,
    DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0292_DRAW_STATE_FINAL_PC34,
    DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0078_DISABLE_PC34
} dm1_v1_champion_panel_leader_swap_food_water_call_t;

typedef struct {
    /*
     * The synthetic champion attribute mask carried forward into the
     * F0302 inventory-champion branch. The gate models MASK0x0200_LOAD
     * and MASK0x0800_PANEL for both the leader (F0297's internal
     * F0292 draw) and the inventory champion (F0301's MASK0x0800_PANEL
     * plus the trailing F0292 draw at F0302:711).
     */
    uint16_t leader_attributes_at_f0297_entry;
    uint16_t inventory_champion_attributes_at_f0300_entry;
    uint16_t inventory_champion_attributes_after_f0301;
} dm1_v1_champion_panel_leader_swap_food_water_attribute_trace_t;

typedef struct {
    /*
     * ReDMCSB CHAMPION.C F0302:678-689 inventory-champion branch.
     * slot_box_index  -> always routes to G0423_i_InventoryChampionOrdinal.
     * leader_index    -> G0411_i_LeaderIndex (the only champion F0297
     *                    draws via its internal F0292 call).
     * inventory_ordinal -> G0423_i_InventoryChampionOrdinal (1..4).
     * inventory_index   -> M001_ORDINAL_TO_INDEX(G0423).
     * is_inventory_champion_leader -> (leader_index == inventory_index).
     * panel_content_pressed_mouth   -> G0333_B_PressingMouth (the food/
     *                    water/poisoned mouth-press state used by
     *                    F0345 in F0292:1060-1062).
     * removed_is_chest_or_scroll -> the F0300 path that sets
     *                    MASK0x0800_PANEL on the inventory champion
     *                    via F0299 (ReDMCSB CHAMPION.C F0300:485,564,
     *                    575). Drives the BUG0_39 flicker predicate.
     * added_is_chest_or_scroll    -> the F0301 path that sets
     *                    MASK0x0800_PANEL on the inventory champion
     *                    via F0299 (ReDMCSB CHAMPION.C F0301:622,638).
     * slot_thing_before -> the object that lived in the action hand
     *                    slot before F0302 fired (C0xFFFF if empty).
     * leader_hand_thing -> G4055_s_LeaderHandObject.Thing before F0298.
     */
    int16_t leader_index;
    int16_t inventory_ordinal;
    int16_t inventory_index;
    int16_t party_champion_count;
    uint16_t slot_box_index;
    uint16_t slot_thing_before;
    uint16_t leader_hand_thing;
    int panel_content_pressed_mouth;
    int removed_is_chest_or_scroll;
    int added_is_chest_or_scroll;
} dm1_v1_champion_panel_leader_swap_food_water_input_t;

typedef struct {
    /*
     * Boolean flags derived from the input contract.
     */
    int leader_is_inventory_champion;
    int inventory_champion_alive;
    int f0297_internal_f0292_fires;
    int intermediate_f0292_drew_food_water_panel;
    int intermediate_f0292_set_viewport_dirty;
    int final_f0292_drew_food_water_panel;
    int final_f0292_set_viewport_dirty;
    int bug0_39_flicker_predicate_holds;
    int mask_0800_panel_set_on_inventory_after_f0301;
    int mask_0200_load_set_on_leader_after_f0297;
    int helper_order_preserved;
    int outer_bracket_preserved;
    /*
     * Synthetic state derived from the input. The gate only models
     * values that are reachable through the F0302 inventory-champion
     * branch in a single call (no queued rotations, no resurrect
     * state, no mirror-candidate chain).
     */
    uint16_t slot_thing_after;
    uint16_t leader_hand_after;
    uint16_t f0292_draw_state_indices[DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MAX_CHAMPIONS_PC34];
    int f0292_draw_state_index_count;
    int f0292_intermediate_champion_index;
    int f0292_final_champion_index;
    int f0300_remove_slot_call_count;
    int f0297_put_leader_hand_call_count;
    int f0301_add_slot_call_count;
    int f0292_draw_state_call_count;
    int f0292_intermediate_call_count;
    int f0292_final_call_count;
    int f0077_enable_screen_update_call_count;
    int f0078_disable_screen_update_call_count;
    int f0077_sequence_index;
    int f0078_sequence_index;
    dm1_v1_champion_panel_leader_swap_food_water_call_t
        call_sequence[DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_TRACE_MAX_PC34];
    int call_sequence_count;
    dm1_v1_champion_panel_leader_swap_food_water_attribute_trace_t attribute_trace;
} dm1_v1_champion_panel_leader_swap_food_water_result_t;

dm1_v1_champion_panel_leader_swap_food_water_input_t
M11_GameView_ChampionPanelLeaderSwapFoodWater_DefaultInput(void);

dm1_v1_champion_panel_leader_swap_food_water_result_t
M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(
    const dm1_v1_champion_panel_leader_swap_food_water_input_t *input);

const char *M11_GameView_ChampionPanelLeaderSwapFoodWater_SourceEvidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_PC34_COMPAT_H */
