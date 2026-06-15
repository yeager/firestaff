#include "dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-locked contract gate only.
 *
 * CHAMPION.C F0302:662-714 owns the slot-box click dispatcher. This module
 * models only guard priority, slot/chest selection, AllowedSlots rejection,
 * synthetic object moves, outer F0077/F0078 bracketing, and helper order.
 */

static const char s_source_evidence[] =
    "contract_only=1; no real M11 graphics calls. "
    "CHAMPION.C F0302:662-714 splits slotBoxIndex into champion/slot, guards "
    "candidate, party count, inventory champion, and dead champions, redirects "
    "C30+ slots to G0425_aT_ChestSlots, rejects empty-hand+empty-slot and "
    "leader-object mask failures, then brackets helper calls with F0077/F0078. "
    "Helper order is F0298, F0300, F0297, F0301, final F0292. "
    "F0302 preserves BUG0_39 order by removing the target slot before putting "
    "that object into the leader hand. F0297:243-268 may draw the leader state; "
    "the target-panel intermediate draw is therefore present only when the "
    "leader champion is also the target champion. COMMAND.C F0359 routes "
    "C020..C065 click commands into F0302 by subtracting the first slot-box "
    "command id.";

static const dm1_v1_champion_panel_action_hand_slot_priority_evidence_t s_evidence = {
    "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
    "CHAMPION.C F0302:662-714",
    "CHAMPION.C F0297:243-268; F0298; F0300; F0301; F0292",
    "DEFS.H:434/810/1874-1878/5332/7946; COMPILE.H:1038-1039",
    "DEFS.H:6886-6894 MOUSE.C F0077/F0078 prototypes",
    "COMMAND.C F0359:1978-1982 and 2173-2177",
    "contract-only slot-box click dispatch/order/state gate",
    "no real M11 graphics; no real-asset bitmap parity"
};

static const dm1_v1_champion_panel_action_hand_slot_priority_invariant_t s_invariant = {
    true,
    false,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_SLOTS_PC34,
    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CHEST_SLOT_COUNT_PC34
};

static int16_t index_to_ordinal(int16_t index)
{
    return (int16_t)(index + 1);
}

static int16_t ordinal_to_index(int16_t ordinal)
{
    return (int16_t)(ordinal - 1);
}

static void copy_inventory_state(
    dm1_v1_champion_panel_action_hand_slot_priority_result_t *result,
    const dm1_v1_champion_panel_action_hand_slot_priority_input_t *input)
{
    memcpy(result->slots_after, input->slots, sizeof(result->slots_after));
    memcpy(result->chest_slots_after, input->chest_slots,
           sizeof(result->chest_slots_after));
}

static void set_default_empty_inventory(
    dm1_v1_champion_panel_action_hand_slot_priority_input_t *input)
{
    int champion;
    int slot;

    for (champion = 0;
         champion < DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34;
         ++champion) {
        for (slot = 0;
             slot < DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_SLOTS_PC34;
             ++slot) {
            input->slots[champion][slot] =
                DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34;
        }
    }
    for (slot = 0;
         slot < DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CHEST_SLOT_COUNT_PC34;
         ++slot) {
        input->chest_slots[slot] =
            DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    }
}

static void append_call(
    dm1_v1_champion_panel_action_hand_slot_priority_result_t *result,
    dm1_v1_champion_panel_action_hand_slot_priority_call_t call)
{
    if (result->call_sequence_count <
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_TRACE_MAX_PC34) {
        result->call_sequence[result->call_sequence_count++] = call;
    }
}

static uint16_t allowed_slots_for_thing(
    const dm1_v1_champion_panel_action_hand_slot_priority_input_t *input,
    uint16_t thing)
{
    int index;

    for (index = 0;
         index < DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_OBJECT_INFO_COUNT_PC34;
         ++index) {
        if (input->object_info[index].thing == thing) {
            return input->object_info[index].allowed_slots;
        }
    }
    return 0;
}

static void reject_with(
    dm1_v1_champion_panel_action_hand_slot_priority_result_t *result,
    dm1_v1_champion_panel_action_hand_slot_priority_return_t reason)
{
    result->accepted = false;
    result->return_reason = reason;
}

dm1_v1_champion_panel_action_hand_slot_priority_input_t
M11_GameView_ChampionPanelActionHandSlotPriority_DefaultInput(void)
{
    dm1_v1_champion_panel_action_hand_slot_priority_input_t input;
    int index;

    memset(&input, 0, sizeof(input));
    input.slot_box_index = DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C08_INVENTORY_FIRST_PC34 + 1u;
    input.leader_hand_thing =
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    input.party_champion_count =
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34;
    input.inventory_champion_ordinal = 1;
    input.leader_champion_index = 0;
    for (index = 0;
         index < DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34;
         ++index) {
        input.current_health[index] = 100;
    }
    set_default_empty_inventory(&input);
    for (index = 0;
         index < DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_SLOTS_PC34;
         ++index) {
        input.slot_masks[index] = (uint16_t)(1u << (index & 15));
    }
    return input;
}

dm1_v1_champion_panel_action_hand_slot_priority_result_t
M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(
    const dm1_v1_champion_panel_action_hand_slot_priority_input_t *input)
{
    dm1_v1_champion_panel_action_hand_slot_priority_result_t result;
    dm1_v1_champion_panel_action_hand_slot_priority_input_t local_input;
    uint16_t allowed_slots;
    uint16_t slot_mask;
    bool slot_index_valid;

    memset(&result, 0, sizeof(result));
    result.invariant = s_invariant;
    result.evidence = s_evidence;
    result.target_champion_index = -1;
    result.target_champion_ordinal = 0;
    result.leader_champion_index = -1;
    result.leader_champion_ordinal = 0;
    result.f0077_sequence_index = -1;
    result.f0078_sequence_index = -1;
    result.return_reason =
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_NONE_PC34;

    for (slot_mask = 0;
         slot_mask < DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34;
         ++slot_mask) {
        result.f0292_draw_state_indices[slot_mask] = -1;
    }

    if (!input) {
        local_input = M11_GameView_ChampionPanelActionHandSlotPriority_DefaultInput();
        input = &local_input;
        result.null_input_defaults_used = true;
    }

    copy_inventory_state(&result, input);
    result.slot_box_index = input->slot_box_index;
    result.leader_hand_before = input->leader_hand_thing;
    result.leader_hand_after = input->leader_hand_thing;
    result.leader_champion_index = input->leader_champion_index;
    if (input->leader_champion_index >= 0 &&
        input->leader_champion_index <
            DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34) {
        result.leader_champion_ordinal = index_to_ordinal(input->leader_champion_index);
    }

    if (input->slot_box_index <
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C08_INVENTORY_FIRST_PC34) {
        result.status_slot_box_route = true;
        if (input->candidate_champion_ordinal) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_CANDIDATE_ACTIVE_PC34);
            return result;
        }
        result.target_champion_index = (int16_t)(input->slot_box_index >> 1);
        result.target_champion_ordinal = index_to_ordinal(result.target_champion_index);
        if (result.target_champion_index >= (int16_t)input->party_champion_count ||
            result.target_champion_index >=
                DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_PARTY_BOUND_PC34);
            return result;
        }
        if (result.target_champion_ordinal == input->inventory_champion_ordinal) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_INVENTORY_CHAMPION_PC34);
            return result;
        }
        if (!input->current_health[result.target_champion_index]) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_DEAD_CHAMPION_PC34);
            return result;
        }
        result.target_slot_index = (uint16_t)(input->slot_box_index & 1u);
    } else {
        result.inventory_slot_box_route = true;
        result.target_champion_index = ordinal_to_index(input->inventory_champion_ordinal);
        result.target_champion_ordinal = input->inventory_champion_ordinal;
        if (result.target_champion_index < 0 ||
            result.target_champion_index >=
                DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_INVALID_INVENTORY_ORDINAL_PC34);
            return result;
        }
        result.target_slot_index = (uint16_t)(
            input->slot_box_index -
            DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C08_INVENTORY_FIRST_PC34);
    }

    result.leader_is_target_champion =
        (result.leader_champion_index == result.target_champion_index);
    result.chest_slot_branch =
        (result.target_slot_index >=
         DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C30_CHEST_FIRST_PC34);
    slot_index_valid =
        result.target_slot_index <
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_MAX_SLOTS_PC34;
    if (!slot_index_valid) {
        reject_with(&result,
                    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_INVALID_SLOT_INDEX_PC34);
        return result;
    }

    if (result.chest_slot_branch) {
        result.slot_thing_before = input->chest_slots[
            result.target_slot_index -
            DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C30_CHEST_FIRST_PC34];
    } else {
        result.slot_thing_before =
            input->slots[result.target_champion_index][result.target_slot_index];
    }

    if (result.slot_thing_before ==
            DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34 &&
        input->leader_hand_thing ==
            DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34) {
        reject_with(&result,
                    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_EMPTY_HAND_EMPTY_SLOT_PC34);
        return result;
    }

    if (input->leader_hand_thing !=
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34) {
        allowed_slots = allowed_slots_for_thing(input, input->leader_hand_thing);
        slot_mask = input->slot_masks[result.target_slot_index];
        if (!(allowed_slots & slot_mask)) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_LEADER_OBJECT_SLOT_MASK_REJECT_PC34);
            return result;
        }
    }

    result.accepted = true;
    append_call(&result,
                DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0077_ENABLE_PC34);
    result.f0077_enable_screen_update_call_count = 1;
    result.f0077_sequence_index = 0;

    if (input->leader_hand_thing !=
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34) {
        append_call(&result,
                    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0298_REMOVE_LEADER_HAND_PC34);
        result.f0298_remove_leader_hand_call_count = 1;
        result.leader_hand_after =
            DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    }

    if (result.slot_thing_before !=
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34) {
        append_call(&result,
                    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0300_REMOVE_SLOT_PC34);
        result.f0300_remove_slot_call_count = 1;
        if (result.chest_slot_branch) {
            result.chest_slots_after[
                result.target_slot_index -
                DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C30_CHEST_FIRST_PC34] =
                DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34;
        } else {
            result.slots_after[result.target_champion_index][result.target_slot_index] =
                DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34;
        }

        append_call(&result,
                    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0297_PUT_LEADER_HAND_PC34);
        result.f0297_put_leader_hand_call_count = 1;
        result.leader_hand_after = result.slot_thing_before;
        if (result.leader_is_target_champion) {
            result.f0292_intermediate_leader_draw_call_count = 1;
            result.bug0_39_panel_flicker_route =
                (input->leader_hand_thing !=
                 DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34);
        } else {
            result.f0297_draw_state_suppressed_when_leader_not_target = true;
        }
    }

    if (input->leader_hand_thing !=
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34) {
        append_call(&result,
                    DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0301_ADD_SLOT_PC34);
        result.f0301_add_slot_call_count = 1;
        if (result.chest_slot_branch) {
            result.chest_slots_after[
                result.target_slot_index -
                DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C30_CHEST_FIRST_PC34] =
                input->leader_hand_thing;
        } else {
            result.slots_after[result.target_champion_index][result.target_slot_index] =
                input->leader_hand_thing;
        }
    }

    append_call(&result,
                DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0292_DRAW_STATE_PC34);
    result.f0292_final_draw_state_call_count = 1;
    result.f0292_draw_state_call_count =
        result.f0292_final_draw_state_call_count +
        result.f0292_intermediate_leader_draw_call_count;
    result.f0292_draw_state_indices[0] = result.target_champion_index;
    if (result.f0292_intermediate_leader_draw_call_count) {
        result.f0292_draw_state_indices[1] = result.target_champion_index;
    }

    append_call(&result,
                DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0078_DISABLE_PC34);
    result.f0078_disable_screen_update_call_count = 1;
    result.f0078_sequence_index = result.call_sequence_count - 1;

    return result;
}

const dm1_v1_champion_panel_action_hand_slot_priority_evidence_t *
M11_GameView_ChampionPanelActionHandSlotPriority_Evidence(void)
{
    return &s_evidence;
}

const char *M11_GameView_ChampionPanelActionHandSlotPriority_SourceEvidence(void)
{
    return s_source_evidence;
}
