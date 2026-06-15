#include "dm1_v1_champion_panel_hand_slot_priority_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "contract_only=1; no real graphics or game-data load. "
    "Source-of-truth hand-slot priority chain is status hand -> leader hand -> "
    "backpack -> belt. CHAMPION.C F0302:677-684 resolves status hand slot boxes "
    "0..7 with candidate, party-count, inventory-champion, and dead-champion "
    "guards before inventory routing. CHAMPION.C F0297:243-298 and F0298 keep "
    "the leader hand object/name/pointer/load side effects outside storage "
    "writes. CHAMPION.C F0300:511-515 clears C30+ through G0425_aT_ChestSlots; "
    "F0301:606-614 writes C30+ through G0425_aT_ChestSlots and marks "
    "M516_CHAMPIONS[].Load. CHAMDRAW.C F0291/F0292 provide the redraw contract; "
    "PANEL.C F0344/F0345 are only recorded as the panel redraw dependency for "
    "the action-hand BUG0_39 leader path. DUNVIEW.C is a non-claim anchor: this "
    "gate does not alter viewport floor/ceiling/present order.";

static const dm1_v1_champion_panel_hand_slot_priority_evidence_t s_evidence = {
    "CHAMPION.C F0302:677-684 status hand slot 0..7 routing",
    "CHAMPION.C F0297:243-298 leader-hand put/remove and load redraw",
    "CHAMPION.C F0300:511-515 C30+ slot clear via G0425_aT_ChestSlots",
    "CHAMPION.C F0301:606-614 C30+ slot write via G0425_aT_ChestSlots",
    "DEFS.H C30/G0425/G0426/G0423/G0305/M070/M516 slot masks and load byte",
    "CHAMDRAW.C F0291/F0292 panel redraw contract",
    "PANEL.C F0344/F0345 panel redraw dependency only",
    "DUNVIEW.C F0128 viewport redraw order non-claim",
    "contract-only status hand -> leader hand -> backpack -> belt source-lock"
};

static const dm1_v1_champion_panel_hand_slot_priority_invariant_t s_invariant = {
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true
};

static int16_t index_to_ordinal(int16_t index)
{
    return (int16_t)(index + 1);
}

static int16_t ordinal_to_index(int16_t ordinal)
{
    return (int16_t)(ordinal - 1);
}

static void append_call(
    dm1_v1_champion_panel_hand_slot_priority_result_t *result,
    dm1_v1_champion_panel_hand_slot_priority_call_t call)
{
    if (result->call_sequence_count <
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_TRACE_MAX_PC34) {
        result->call_sequence[result->call_sequence_count++] = call;
    }
}

static void reject_with(
    dm1_v1_champion_panel_hand_slot_priority_result_t *result,
    dm1_v1_champion_panel_hand_slot_priority_return_t reason)
{
    result->accepted = false;
    result->return_reason = reason;
}

static void seed_chain(
    dm1_v1_champion_panel_hand_slot_priority_result_t *result)
{
    result->source_chain[0] =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_STATUS_HAND_PC34;
    result->source_chain[1] =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_LEADER_HAND_PC34;
    result->source_chain[2] =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_BACKPACK_PC34;
    result->source_chain[3] =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_BELT_PC34;
    result->source_chain_count =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CHAIN_COUNT_PC34;
    result->backpack_precedes_belt_in_priority_chain = true;
}

dm1_v1_champion_panel_hand_slot_priority_input_t
dm1_v1_champion_panel_hand_slot_priority_default_input_pc34(void)
{
    dm1_v1_champion_panel_hand_slot_priority_input_t input;
    int index;

    memset(&input, 0, sizeof(input));
    input.slot_box_index = 1;
    input.party_champion_count =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34;
    input.inventory_champion_ordinal = 2;
    input.leader_champion_index = 0;
    input.leader_hand_thing =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    input.slot_thing = 0x0101u;
    input.leader_object_allowed_slots = 0x0003u;
    input.slot_mask = 0x0002u;
    input.action_hand_contains_panel_object = true;
    for (index = 0;
         index < DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34;
         ++index) {
        input.current_health[index] = 100;
    }
    return input;
}

dm1_v1_champion_panel_hand_slot_priority_family_t
dm1_v1_champion_panel_hand_slot_priority_family_for_slot_pc34(
    uint16_t slot_index,
    bool status_hand_route)
{
    if (status_hand_route) {
        return DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_STATUS_HAND_PC34;
    }
    if (slot_index <= 1u) {
        return DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_INVENTORY_HAND_PC34;
    }
    if (slot_index >= 13u && slot_index < 30u) {
        return DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_BACKPACK_PC34;
    }
    if (slot_index >= 6u && slot_index <= 12u) {
        return DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_BELT_PC34;
    }
    if (slot_index >=
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_C30_CHEST_FIRST_PC34) {
        return DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_CHEST_PC34;
    }
    if (slot_index < DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_MAX_SLOTS_PC34) {
        return DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_BODY_PC34;
    }
    return DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_INVALID_PC34;
}

dm1_v1_champion_panel_hand_slot_priority_result_t
dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(
    const dm1_v1_champion_panel_hand_slot_priority_input_t *input)
{
    dm1_v1_champion_panel_hand_slot_priority_input_t local_input;
    dm1_v1_champion_panel_hand_slot_priority_result_t result;
    bool leader_has_object;
    bool slot_has_object;

    memset(&result, 0, sizeof(result));
    result.invariant = s_invariant;
    result.evidence = s_evidence;
    seed_chain(&result);
    result.target_champion_index = -1;
    result.target_champion_ordinal = 0;
    result.return_reason =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_NONE_PC34;
    result.slot_family =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_INVALID_PC34;

    if (!input) {
        local_input = dm1_v1_champion_panel_hand_slot_priority_default_input_pc34();
        input = &local_input;
        result.null_input_defaults_used = true;
    }

    result.slot_box_index = input->slot_box_index;
    result.leader_hand_before = input->leader_hand_thing;
    result.slot_thing_before = input->slot_thing;
    result.leader_hand_after = input->leader_hand_thing;
    result.slot_thing_after = input->slot_thing;

    if (input->slot_box_index <
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_C08_INVENTORY_FIRST_PC34) {
        result.status_hand_route = true;
        if (input->candidate_champion_ordinal) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_CANDIDATE_ACTIVE_PC34);
            return result;
        }
        result.target_champion_index = (int16_t)(input->slot_box_index >> 1);
        result.target_champion_ordinal =
            index_to_ordinal(result.target_champion_index);
        if (result.target_champion_index >= (int16_t)input->party_champion_count ||
            result.target_champion_index >=
                DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_PARTY_BOUND_PC34);
            return result;
        }
        if (result.target_champion_ordinal == input->inventory_champion_ordinal) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_INVENTORY_CHAMPION_PC34);
            return result;
        }
        if (!input->current_health[result.target_champion_index]) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_DEAD_CHAMPION_PC34);
            return result;
        }
        result.target_slot_index = (uint16_t)(input->slot_box_index & 1u);
    } else {
        result.inventory_route = true;
        result.target_champion_index =
            ordinal_to_index(input->inventory_champion_ordinal);
        result.target_champion_ordinal = input->inventory_champion_ordinal;
        if (result.target_champion_index < 0 ||
            result.target_champion_index >=
                DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_MAX_CHAMPIONS_PC34) {
            reject_with(&result,
                        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_INVALID_INVENTORY_ORDINAL_PC34);
            return result;
        }
        result.target_slot_index = (uint16_t)(
            input->slot_box_index -
            DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_C08_INVENTORY_FIRST_PC34);
    }

    result.slot_family =
        dm1_v1_champion_panel_hand_slot_priority_family_for_slot_pc34(
            result.target_slot_index,
            result.status_hand_route);
    result.c30_chest_slot_route =
        result.target_slot_index >=
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_C30_CHEST_FIRST_PC34;
    if (result.slot_family ==
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_INVALID_PC34) {
        reject_with(&result,
                    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_INVALID_SLOT_PC34);
        return result;
    }

    result.leader_is_target =
        input->leader_champion_index == result.target_champion_index;
    result.leader_hand_checked_before_storage_write = true;
    result.c30_clear_uses_g0425 = result.c30_chest_slot_route;
    result.c30_write_uses_g0425 = result.c30_chest_slot_route;
    result.panel_redraw_dependency =
        result.leader_is_target &&
        result.target_slot_index == 1u &&
        input->action_hand_contains_panel_object;

    leader_has_object =
        input->leader_hand_thing !=
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    slot_has_object =
        input->slot_thing !=
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    if (!leader_has_object && !slot_has_object) {
        reject_with(&result,
                    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_EMPTY_HAND_EMPTY_SLOT_PC34);
        return result;
    }
    if (leader_has_object &&
        !(input->leader_object_allowed_slots & input->slot_mask)) {
        reject_with(&result,
                    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_MASK_REJECT_PC34);
        return result;
    }

    result.accepted = true;
    append_call(&result,
                DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0077_ENABLE_PC34);
    if (leader_has_object) {
        append_call(&result,
                    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0298_REMOVE_LEADER_PC34);
        result.leader_hand_after =
            DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    }
    if (slot_has_object) {
        append_call(&result,
                    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0300_CLEAR_SLOT_PC34);
        result.slot_thing_after =
            DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_THING_NONE_PC34;
        append_call(&result,
                    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0297_PUT_LEADER_PC34);
        result.leader_hand_after = input->slot_thing;
        result.f0297_intermediate_draw_state = input->leader_champion_index >= 0;
    }
    if (leader_has_object) {
        append_call(&result,
                    DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0301_WRITE_SLOT_PC34);
        result.slot_thing_after = input->leader_hand_thing;
    }
    append_call(&result,
                DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0292_DRAW_STATE_PC34);
    result.f0292_final_draw_state = true;
    append_call(&result,
                DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0078_DISABLE_PC34);

    return result;
}

const dm1_v1_champion_panel_hand_slot_priority_evidence_t *
dm1_v1_champion_panel_hand_slot_priority_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_champion_panel_hand_slot_priority_source_evidence_pc34(void)
{
    return s_source_evidence;
}
