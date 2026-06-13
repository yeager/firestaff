#include "dm1_v1_champion_panel_leader_swap_food_water_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-locked contract gate only.
 *
 * CHAMPION.C F0302:702-711 owns the F0298/F0300/F0297/F0301 helper chain
 * for inventory-champion slot-box clicks. F0297:243-268 internally calls
 * F0292_CHAMPION_DrawState for the leader only. CHAMDRAW.C F0292:1060-1061
 * gates the food/water/poisoned panel redraw on (MASK0x0800_PANEL &&
 * L0863_B_IsInventoryChampion). This module models that chain and the
 * BUG0_39 flicker predicate synthetically.
 */

static const char s_source_evidence[] =
    "contract_only=1; no real M11 graphics calls. "
    "CHAMPION.C F0302:662-714 inventory-champion branch routes every "
    "slotBoxIndex >= C08_SLOT_BOX_INVENTORY_FIRST_SLOT through M001 to "
    "G0423_i_InventoryChampionOrdinal. Helper order is F0298, F0300, "
    "F0297, F0301, final F0292 bracketed by F0077/F0078. F0297:243-268 "
    "calls F0292 only for the leader (G0411_i_LeaderIndex) and only when "
    "G0411_i_LeaderIndex != CM1_CHAMPION_NONE. F0297:265-268 sets "
    "MASK0x0200_LOAD on the leader before the F0292 call. CHAMDRAW.C F0292:1060-1061 "
    "gates the food/water/poisoned panel redraw on MASK0x0800_PANEL && "
    "L0863_B_IsInventoryChampion. The intermediate F0292 call inside "
    "F0297 only draws the food/water panel when the leader is also the "
    "inventory champion (L0863_B_IsInventoryChampion is true only for "
    "the inventory champion). CHAMPION.C F0300:485,564,575 sets "
    "MASK0x0800_PANEL on the inventory champion when the removed object "
    "is a chest or scroll. CHAMPION.C F0301:622,638 sets MASK0x0800_PANEL "
    "on the inventory champion when the added object is a chest or "
    "scroll. The BUG0_39 flicker predicate is the conjunction "
    "(leader_is_inventory_champion && removed_is_chest_or_scroll && "
    "leader_hand_non_empty && slot_thing_non_empty) at the F0297 call "
    "site. The final F0292 draw at F0302:711 always redraws the "
    "inventory champion and goes through the food/water panel branch "
    "when MASK0x0800_PANEL is set on the inventory champion after F0301.";

dm1_v1_champion_panel_leader_swap_food_water_input_t
M11_GameView_ChampionPanelLeaderSwapFoodWater_DefaultInput(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input;

    memset(&input, 0, sizeof(input));
    input.leader_index = 0;
    input.inventory_ordinal = 1;
    input.inventory_index = 0;
    input.party_champion_count =
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MAX_CHAMPIONS_PC34;
    input.slot_box_index = 9;
    input.slot_thing_before = 0x0101u;
    input.leader_hand_thing = 0x0202u;
    input.panel_content_pressed_mouth = 0;
    input.removed_is_chest_or_scroll = 1;
    input.added_is_chest_or_scroll = 1;
    return input;
}

static void append_call(
    dm1_v1_champion_panel_leader_swap_food_water_result_t *result,
    dm1_v1_champion_panel_leader_swap_food_water_call_t call)
{
    if (result->call_sequence_count <
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_TRACE_MAX_PC34) {
        result->call_sequence[result->call_sequence_count++] = call;
    }
}

dm1_v1_champion_panel_leader_swap_food_water_result_t
M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(
    const dm1_v1_champion_panel_leader_swap_food_water_input_t *input)
{
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    dm1_v1_champion_panel_leader_swap_food_water_input_t local_input;
    int i;

    memset(&result, 0, sizeof(result));
    for (i = 0; i < DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MAX_CHAMPIONS_PC34;
         ++i) {
        result.f0292_draw_state_indices[i] = -1;
    }
    result.f0292_intermediate_champion_index = -1;
    result.f0292_final_champion_index = -1;
    result.f0077_sequence_index = -1;
    result.f0078_sequence_index = -1;
    result.slot_thing_after = 0xFFFFu;
    result.leader_hand_after = 0xFFFFu;

    if (!input) {
        local_input =
            M11_GameView_ChampionPanelLeaderSwapFoodWater_DefaultInput();
        input = &local_input;
    }

    result.leader_is_inventory_champion =
        (input->leader_index == input->inventory_index);
    result.inventory_champion_alive = (input->party_champion_count > 0 &&
        input->inventory_index >= 0 &&
        input->inventory_index < input->party_champion_count &&
        input->inventory_index <
            DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MAX_CHAMPIONS_PC34);

    if (!result.inventory_champion_alive) {
        return result;
    }

    /*
     * CHAMPION.C F0302:702 outer F0077 bracket. CHAMPION.C F0302:713
     * outer F0078 bracket. The helper chain between them is
     * F0298 (leader hand removal) -> F0300 (slot removal) -> F0297
     * (put leader hand, with internal F0292) -> F0301 (add slot) ->
     * final F0292.
     */
    append_call(&result,
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0077_ENABLE_PC34);
    result.f0077_enable_screen_update_call_count = 1;
    result.f0077_sequence_index = 0;

    /*
     * CHAMPION.C F0302:704 F0298 only fires if the leader hand is
     * non-empty. After this call the leader hand is empty and the
     * action hand (slot thing) still holds the inventory champion's
     * old object.
     */
    if (input->leader_hand_thing != 0xFFFFu) {
        append_call(&result,
            DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0298_REMOVE_LEADER_HAND_PC34);
        result.leader_hand_after = 0xFFFFu;
    } else {
        result.leader_hand_after = 0xFFFFu;
    }

    /*
     * CHAMPION.C F0302:705-706 F0300 sets MASK0x0800_PANEL on the
     * inventory champion when the removed object is a chest or
     * scroll (F0299 modifier path).
     */
    if (input->removed_is_chest_or_scroll) {
        result.attribute_trace.inventory_champion_attributes_at_f0300_entry |=
            DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_PANEL_PC34;
    }
    if (input->slot_thing_before != 0xFFFFu) {
        append_call(&result,
            DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0300_REMOVE_SLOT_PC34);
        result.f0300_remove_slot_call_count = 1;
    }

    /*
     * CHAMPION.C F0302:706 F0297 puts the slot thing into the leader
     * hand. F0297:265-268 sets MASK0x0200_LOAD on the leader and
     * calls F0292_DrawState(leaderIndex). This is the intermediate
     * F0292 call. The food/water panel redraw at F0292:1060-1061
     * only fires when MASK0x0800_PANEL && L0863_B_IsInventoryChampion
     * for the champion being drawn. The intermediate F0292 draws
     * the leader. The leader is the inventory champion only when
     * result.leader_is_inventory_champion holds; otherwise the
     * intermediate F0292 call does NOT draw the food/water panel.
     *
     * The BUG0_39 flicker predicate requires:
     *  - the leader IS the inventory champion (so the food/water
     *    panel redraw branch at F0292:1060-1061 actually fires),
     *  - the removed object was a chest or scroll (so MASK0x0800_PANEL
     *    is set on the inventory champion's attributes by F0300),
     *  - the leader hand was non-empty (so F0298 ran and the action
     *    hand is now empty, which is exactly when the food/water
     *    panel is the natural content of the panel), and
     *  - the slot thing was non-empty (so F0300 ran and the
     *    inventory champion's action hand is empty at the moment
     *    F0297's intermediate F0292 fires).
     */
    if (input->slot_thing_before != 0xFFFFu) {
        append_call(&result,
            DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0297_PUT_LEADER_HAND_PC34);
        result.f0297_put_leader_hand_call_count = 1;
        result.leader_hand_after = input->slot_thing_before;
        result.attribute_trace.leader_attributes_at_f0297_entry |=
            DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_LOAD_PC34;

        if (input->leader_index >= 0 &&
            input->leader_index <
                DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MAX_CHAMPIONS_PC34) {
            result.f0297_internal_f0292_fires = 1;
            append_call(&result,
                DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0292_DRAW_STATE_INTERMEDIATE_PC34);
            result.f0292_intermediate_call_count = 1;
            result.f0292_intermediate_champion_index = input->leader_index;
            result.f0292_draw_state_indices[
                result.f0292_draw_state_index_count++] = input->leader_index;

            /*
             * The food/water panel redraw at F0292:1060-1061 only
             * fires when the leader being drawn is also the
             * inventory champion. The "panel content" branch in
             * F0292:1061-1077 reads L0863_B_IsInventoryChampion
             * which is set by F0292 for the inventory champion
             * only.
             */
            if (result.leader_is_inventory_champion &&
                (result.attribute_trace.inventory_champion_attributes_at_f0300_entry &
                 DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_PANEL_PC34)) {
                result.intermediate_f0292_drew_food_water_panel = 1;
                result.intermediate_f0292_set_viewport_dirty = 1;
            } else {
                result.intermediate_f0292_drew_food_water_panel = 0;
                result.intermediate_f0292_set_viewport_dirty = 0;
            }

            result.bug0_39_flicker_predicate_holds =
                result.leader_is_inventory_champion &&
                input->removed_is_chest_or_scroll &&
                input->leader_hand_thing != 0xFFFFu &&
                input->slot_thing_before != 0xFFFFu;
        }
    } else {
        result.leader_hand_after = 0xFFFFu;
    }

    /*
     * CHAMPION.C F0302:709 F0301 adds the prior leader hand object
     * to the inventory champion's action hand slot. F0301:622,638
     * sets MASK0x8000_ACTION_HAND and, when the added object is a
     * chest or scroll, MASK0x0800_PANEL.
     */
    if (input->leader_hand_thing != 0xFFFFu) {
        append_call(&result,
            DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0301_ADD_SLOT_PC34);
        result.f0301_add_slot_call_count = 1;
        result.slot_thing_after = input->leader_hand_thing;
        if (input->added_is_chest_or_scroll) {
            result.attribute_trace.inventory_champion_attributes_after_f0301 |=
                DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_PANEL_PC34 |
                DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_ACTION_HAND_PC34;
        } else {
            result.attribute_trace.inventory_champion_attributes_after_f0301 |=
                DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_ACTION_HAND_PC34;
        }
        if (input->added_is_chest_or_scroll) {
            result.mask_0800_panel_set_on_inventory_after_f0301 = 1;
        }
    } else if (input->slot_thing_before != 0xFFFFu) {
        result.slot_thing_after = 0xFFFFu;
    } else {
        result.slot_thing_after = 0xFFFFu;
    }

    if (result.attribute_trace.leader_attributes_at_f0297_entry &
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_LOAD_PC34) {
        result.mask_0200_load_set_on_leader_after_f0297 = 1;
    }

    /*
     * CHAMPION.C F0302:711 trailing F0292 always draws the
     * inventory champion. F0292:1060-1061 routes the food/water
     * panel redraw through F0345 only when MASK0x0800_PANEL is set
     * on the inventory champion. The final F0292 also sets
     * MASK0x4000_VIEWPORT (F0292:1078-1087) for the inventory
     * champion only.
     */
    append_call(&result,
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0292_DRAW_STATE_FINAL_PC34);
    result.f0292_final_call_count = 1;
    result.f0292_final_champion_index = input->inventory_index;
    result.f0292_draw_state_indices[
        result.f0292_draw_state_index_count++] = input->inventory_index;
    result.f0292_draw_state_call_count =
        result.f0292_intermediate_call_count +
        result.f0292_final_call_count;
    result.final_f0292_drew_food_water_panel =
        (result.attribute_trace.inventory_champion_attributes_after_f0301 &
         DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_PANEL_PC34) ?
        1 : 0;
    result.final_f0292_set_viewport_dirty = 1;

    append_call(&result,
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0078_DISABLE_PC34);
    result.f0078_disable_screen_update_call_count = 1;
    result.f0078_sequence_index = result.call_sequence_count - 1;

    result.helper_order_preserved =
        (result.call_sequence_count >= 4) &&
        (result.call_sequence[0] ==
            DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0077_ENABLE_PC34) &&
        (result.f0078_sequence_index == result.call_sequence_count - 1);
    result.outer_bracket_preserved =
        (result.f0077_sequence_index == 0) &&
        (result.f0078_sequence_index == result.call_sequence_count - 1);

    return result;
}

const char *M11_GameView_ChampionPanelLeaderSwapFoodWater_SourceEvidence(void)
{
    return s_source_evidence;
}
