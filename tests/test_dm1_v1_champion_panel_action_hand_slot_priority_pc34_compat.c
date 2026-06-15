#include "dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    }
}

static void expect_u16(const char *id, uint16_t got, uint16_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%04X want=0x%04X at %s\n",
               id, (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_str_eq(const char *id, const char *got, const char *want,
                          const char *anchor)
{
    ++g_assertions;
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=\"%s\" want=\"%s\" at %s\n",
               id, got ? got : "(null)", want ? want : "(null)", anchor);
        ++g_failures;
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    }
}

static void allow_thing(
    dm1_v1_champion_panel_action_hand_slot_priority_input_t *input,
    int object_info_index,
    uint16_t thing,
    uint16_t allowed_slots)
{
    input->object_info[object_info_index].thing = thing;
    input->object_info[object_info_index].allowed_slots = allowed_slots;
}

static dm1_v1_champion_panel_action_hand_slot_priority_input_t base_input(void)
{
    dm1_v1_champion_panel_action_hand_slot_priority_input_t input =
        M11_GameView_ChampionPanelActionHandSlotPriority_DefaultInput();
    input.slot_box_index =
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C08_INVENTORY_FIRST_PC34 + 1u;
    input.inventory_champion_ordinal = 1;
    input.leader_champion_index = 0;
    input.slot_masks[0] = 0x0001u;
    input.slot_masks[1] = 0x0002u;
    input.slot_masks[30] = 0x4000u;
    allow_thing(&input, 0, 0x0101u, 0x0003u);
    allow_thing(&input, 1, 0x0202u, 0x0003u);
    allow_thing(&input, 2, 0x0303u, 0x4000u);
    return input;
}

static void test_evidence_and_invariants(void)
{
    const dm1_v1_champion_panel_action_hand_slot_priority_evidence_t *evidence =
        M11_GameView_ChampionPanelActionHandSlotPriority_Evidence();
    dm1_v1_champion_panel_action_hand_slot_priority_result_t result =
        M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(NULL);
    const char *source =
        M11_GameView_ChampionPanelActionHandSlotPriority_SourceEvidence();

    expect_bool("invariant.contract_only", result.invariant.contract_only, true,
                "CHAMPION.C F0302:662-714 contract-only dispatcher");
    expect_bool("invariant.no_real_graphics",
                result.invariant.real_m11_graphics_called, false,
                "CHAMPION.C F0302:662-714 no real M11 graphics");
    expect_bool("invariant.status_route",
                result.invariant.models_status_slotbox_0_to_7, true,
                "CHAMPION.C F0302:677-684 status slot boxes");
    expect_bool("invariant.inventory_route",
                result.invariant.models_inventory_slotbox_8_to_37, true,
                "CHAMPION.C F0302:684-687 inventory slot boxes");
    expect_bool("invariant.chest_branch", result.invariant.models_chest_slot_branch, true,
                "CHAMPION.C F0302:690-694 C30 chest branch");
    expect_bool("invariant.mask_check",
                result.invariant.models_leader_hand_allowed_slots_mask, true,
                "CHAMPION.C F0302:699-701 AllowedSlots mask");
    expect_bool("invariant.helper_order",
                result.invariant.records_f0298_f0300_f0297_f0301_order, true,
                "CHAMPION.C F0302:702-711 helper order");
    expect_bool("invariant.bracket",
                result.invariant.records_f0077_f0078_outer_bracket, true,
                "CHAMPION.C F0302:702 and 713");
    expect_bool("invariant.leader_follower",
                result.invariant.records_leader_vs_follower_draw_priority, true,
                "CHAMPION.C F0297:263-268 leader draw state route");
    expect_int("invariant.max_champions", result.invariant.max_champions, 4,
               "DEFS.H champion slots");
    expect_int("invariant.max_slots", result.invariant.max_slots, 38,
               "DEFS.H C00..C37 slots");
    expect_int("invariant.chest_count", result.invariant.chest_slot_count, 8,
               "DEFS.H C30..C37 chest slots");
    expect_bool("null.defaults", result.null_input_defaults_used, true,
                "synthetic default probe");
    expect_str_eq("evidence.function", evidence->function_name,
                  "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
                  "CHAMPION.C F0302:662-714");
    expect_str_eq("evidence.slot_anchor", evidence->slot_dispatch_anchor,
                  "CHAMPION.C F0302:662-714",
                  "CHAMPION.C F0302:662-714");
    expect_contains("evidence.helper", evidence->helper_anchor,
                    "F0297:243-268", "CHAMPION.C F0297:243-268");
    expect_contains("evidence.defs", evidence->defs_anchor,
                    "1874-1878", "DEFS.H C08/M070");
    expect_contains("evidence.mouse", evidence->mouse_anchor,
                    "F0077/F0078", "MOUSE.C F0077/F0078");
    expect_contains("evidence.command", evidence->command_anchor,
                    "F0359", "COMMAND.C F0359");
    expect_contains("source.contract", source, "contract_only=1",
                    "evidence string");
    expect_contains("source.order", source, "F0298, F0300, F0297, F0301",
                    "CHAMPION.C F0302:704-710");
    expect_contains("source.bug", source, "BUG0_39",
                    "CHAMPION.C F0302:706 comment");
}

static void test_empty_hand_empty_slot_early_return(void)
{
    dm1_v1_champion_panel_action_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_action_hand_slot_priority_result_t result;

    input.leader_hand_thing =
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    input.slots[0][1] =
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    result = M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(&input);

    expect_bool("empty.accepted", result.accepted, false,
                "CHAMPION.C F0302:696-698 empty hand and empty slot return");
    expect_int("empty.reason", result.return_reason,
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_EMPTY_HAND_EMPTY_SLOT_PC34,
               "CHAMPION.C F0302:696-698");
    expect_int("empty.call_count", result.call_sequence_count, 0,
               "CHAMPION.C F0302:696 return before F0077");
    expect_int("empty.f0077", result.f0077_enable_screen_update_call_count, 0,
               "CHAMPION.C F0302:702 not reached");
    expect_int("empty.f0078", result.f0078_disable_screen_update_call_count, 0,
               "CHAMPION.C F0302:713 not reached");
    expect_u16("empty.leader_after", result.leader_hand_after,
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34,
               "CHAMPION.C F0302:696 state unchanged");
    expect_u16("empty.slot_after", result.slots_after[0][1],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34,
               "CHAMPION.C F0302:696 state unchanged");
}

static void test_put_leader_object_into_empty_action_slot(void)
{
    dm1_v1_champion_panel_action_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_action_hand_slot_priority_result_t result;

    input.leader_hand_thing = 0x0101u;
    result = M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(&input);

    expect_bool("put.accepted", result.accepted, true,
                "CHAMPION.C F0302:699-713 accepted put");
    expect_bool("put.inventory_route", result.inventory_slot_box_route, true,
                "CHAMPION.C F0302:684-687 inventory champion route");
    expect_int("put.target_champion", result.target_champion_index, 0,
               "CHAMPION.C F0302:685 M001 inventory ordinal");
    expect_int("put.target_slot", result.target_slot_index, 1,
               "CHAMPION.C F0302:686 slot index delta");
    expect_u16("put.slot_before", result.slot_thing_before,
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34,
               "CHAMPION.C F0302:690-694 slot read");
    expect_u16("put.leader_after", result.leader_hand_after,
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34,
               "CHAMPION.C F0302:704 F0298 removes leader hand");
    expect_u16("put.slot_after", result.slots_after[0][1], 0x0101u,
               "CHAMPION.C F0302:710 F0301 adds leader object to slot");
    expect_int("put.calls", result.call_sequence_count, 5,
               "CHAMPION.C F0302:702-713 F0077/F0298/F0301/F0292/F0078 subset");
    expect_int("put.call0", result.call_sequence[0],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0077_ENABLE_PC34,
               "CHAMPION.C F0302:702");
    expect_int("put.call1", result.call_sequence[1],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0298_REMOVE_LEADER_HAND_PC34,
               "CHAMPION.C F0302:704");
    expect_int("put.call2", result.call_sequence[2],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0301_ADD_SLOT_PC34,
               "CHAMPION.C F0302:710");
    expect_int("put.call3", result.call_sequence[3],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0292_DRAW_STATE_PC34,
               "CHAMPION.C F0302:712");
    expect_int("put.call4", result.call_sequence[4],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0078_DISABLE_PC34,
               "CHAMPION.C F0302:713");
    expect_int("put.f0301", result.f0301_add_slot_call_count, 1,
               "CHAMPION.C F0302:710");
    expect_int("put.final_draw", result.f0292_final_draw_state_call_count, 1,
               "CHAMPION.C F0302:712");
    expect_int("put.draw_count", result.f0292_draw_state_call_count, 1,
               "CHAMPION.C F0302:712 no F0297 draw on empty slot");
}

static void test_swap_preserves_bug039_helper_order(void)
{
    dm1_v1_champion_panel_action_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_action_hand_slot_priority_result_t result;

    input.leader_hand_thing = 0x0101u;
    input.slots[0][1] = 0x0202u;
    result = M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(&input);

    expect_bool("swap.accepted", result.accepted, true,
                "CHAMPION.C F0302:702-713 accepted swap");
    expect_u16("swap.leader_after", result.leader_hand_after, 0x0202u,
               "CHAMPION.C F0302:706 F0297 puts slot object in leader hand");
    expect_u16("swap.slot_after", result.slots_after[0][1], 0x0101u,
               "CHAMPION.C F0302:710 F0301 puts saved leader object in slot");
    expect_int("swap.f0298", result.f0298_remove_leader_hand_call_count, 1,
               "CHAMPION.C F0302:703-705");
    expect_int("swap.f0300", result.f0300_remove_slot_call_count, 1,
               "CHAMPION.C F0302:706");
    expect_int("swap.f0297", result.f0297_put_leader_hand_call_count, 1,
               "CHAMPION.C F0302:707");
    expect_int("swap.f0301", result.f0301_add_slot_call_count, 1,
               "CHAMPION.C F0302:709-711");
    expect_int("swap.call0", result.call_sequence[0],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0077_ENABLE_PC34,
               "CHAMPION.C F0302:702");
    expect_int("swap.call1", result.call_sequence[1],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0298_REMOVE_LEADER_HAND_PC34,
               "CHAMPION.C F0302:704");
    expect_int("swap.call2", result.call_sequence[2],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0300_REMOVE_SLOT_PC34,
               "CHAMPION.C F0302:706 BUG0_39 order");
    expect_int("swap.call3", result.call_sequence[3],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0297_PUT_LEADER_HAND_PC34,
               "CHAMPION.C F0302:707 BUG0_39 order");
    expect_int("swap.call4", result.call_sequence[4],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0301_ADD_SLOT_PC34,
               "CHAMPION.C F0302:710 BUG0_39 order");
    expect_bool("swap.leader_is_target", result.leader_is_target_champion, true,
                "CHAMPION.C F0297:263-268 leader draw target");
    expect_bool("swap.bug039", result.bug0_39_panel_flicker_route, true,
                "CHAMPION.C F0302:706 BUG0_39 comment");
    expect_int("swap.intermediate_draw",
               result.f0292_intermediate_leader_draw_call_count, 1,
               "CHAMPION.C F0297:263-268 leader F0292");
    expect_int("swap.draw_total", result.f0292_draw_state_call_count, 2,
               "CHAMPION.C F0297 plus F0302 final F0292");
}

static void test_slot_mask_zero_rejects_leader_object(void)
{
    dm1_v1_champion_panel_action_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_action_hand_slot_priority_result_t result;

    input.leader_hand_thing = 0x0101u;
    input.slot_masks[1] = 0x0000u;
    result = M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(&input);

    expect_bool("mask.accepted", result.accepted, false,
                "CHAMPION.C F0302:699-701 mask reject");
    expect_int("mask.reason", result.return_reason,
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_LEADER_OBJECT_SLOT_MASK_REJECT_PC34,
               "CHAMPION.C F0302:699-701");
    expect_int("mask.calls", result.call_sequence_count, 0,
               "CHAMPION.C F0302:699-701 return before F0077");
    expect_u16("mask.leader_after", result.leader_hand_after, 0x0101u,
               "CHAMPION.C F0302:699 state unchanged");
    expect_u16("mask.slot_after", result.slots_after[0][1],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34,
               "CHAMPION.C F0302:699 state unchanged");
}

static void test_status_slot_candidate_dead_inventory_guards(void)
{
    dm1_v1_champion_panel_action_hand_slot_priority_input_t candidate = base_input();
    dm1_v1_champion_panel_action_hand_slot_priority_input_t dead = base_input();
    dm1_v1_champion_panel_action_hand_slot_priority_input_t inventory = base_input();
    dm1_v1_champion_panel_action_hand_slot_priority_result_t result;

    candidate.slot_box_index = 1;
    candidate.candidate_champion_ordinal = 2;
    result = M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(&candidate);
    expect_bool("candidate.status_route", result.status_slot_box_route, true,
                "CHAMPION.C F0302:677 status route");
    expect_bool("candidate.accepted", result.accepted, false,
                "CHAMPION.C F0302:678-679 candidate return");
    expect_int("candidate.reason", result.return_reason,
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_CANDIDATE_ACTIVE_PC34,
               "CHAMPION.C F0302:678-679");
    expect_int("candidate.calls", result.call_sequence_count, 0,
               "CHAMPION.C F0302:679 return");

    dead.slot_box_index = 3;
    dead.inventory_champion_ordinal = 1;
    dead.current_health[1] = 0;
    result = M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(&dead);
    expect_int("dead.target", result.target_champion_index, 1,
               "CHAMPION.C F0302:680 slotBoxIndex >> 1");
    expect_bool("dead.accepted", result.accepted, false,
                "CHAMPION.C F0302:681 dead champion return");
    expect_int("dead.reason", result.return_reason,
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_DEAD_CHAMPION_PC34,
               "CHAMPION.C F0302:681");
    expect_int("dead.slot", result.target_slot_index, 0,
               "CHAMPION.C F0302:682 slot assignment after guard not reached");

    inventory.slot_box_index = 1;
    inventory.inventory_champion_ordinal = 1;
    result = M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(&inventory);
    expect_int("inventory.target", result.target_champion_index, 0,
               "CHAMPION.C F0302:680 slotBoxIndex >> 1");
    expect_bool("inventory.accepted", result.accepted, false,
                "CHAMPION.C F0302:681 inventory champion return");
    expect_int("inventory.reason", result.return_reason,
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_RETURN_INVENTORY_CHAMPION_PC34,
               "CHAMPION.C F0302:681");
    expect_int("inventory.calls", result.call_sequence_count, 0,
               "CHAMPION.C F0302:681 return before screen update");
}

static void test_chest_branch_uses_g0425_slots(void)
{
    dm1_v1_champion_panel_action_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_action_hand_slot_priority_result_t result;

    input.slot_box_index =
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C08_INVENTORY_FIRST_PC34 +
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_C30_CHEST_FIRST_PC34;
    input.leader_hand_thing = 0x0303u;
    input.chest_slots[0] =
        DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    result = M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(&input);

    expect_bool("chest.accepted", result.accepted, true,
                "CHAMPION.C F0302:690-694 chest branch");
    expect_bool("chest.branch", result.chest_slot_branch, true,
                "CHAMPION.C F0302:690 C30 slot threshold");
    expect_int("chest.slot", result.target_slot_index, 30,
               "CHAMPION.C F0302:686 slot box index delta");
    expect_u16("chest.leader_after", result.leader_hand_after,
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34,
               "CHAMPION.C F0302:704 F0298");
    expect_u16("chest.slot_after", result.chest_slots_after[0], 0x0303u,
               "CHAMPION.C F0302:710 F0301 G0425_aT_ChestSlots");
    expect_u16("chest.champion_slot_unchanged", result.slots_after[0][30],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_THING_NONE_PC34,
               "CHAMPION.C F0302:690-694 chest not champion slot array");
    expect_int("chest.f0301", result.f0301_add_slot_call_count, 1,
               "CHAMPION.C F0302:710");
}

static void test_screen_update_brackets_order(void)
{
    dm1_v1_champion_panel_action_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_action_hand_slot_priority_result_t result;

    input.leader_hand_thing = 0x0101u;
    input.slots[0][1] = 0x0202u;
    result = M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(&input);

    expect_int("bracket.enable_count", result.f0077_enable_screen_update_call_count, 1,
               "CHAMPION.C F0302:702 F0077");
    expect_int("bracket.disable_count", result.f0078_disable_screen_update_call_count, 1,
               "CHAMPION.C F0302:713 F0078");
    expect_int("bracket.enable_index", result.f0077_sequence_index, 0,
               "CHAMPION.C F0302:702 before helpers");
    expect_int("bracket.disable_index", result.f0078_sequence_index, 6,
               "CHAMPION.C F0302:713 after F0292");
    expect_int("bracket.call0", result.call_sequence[result.f0077_sequence_index],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0077_ENABLE_PC34,
               "CHAMPION.C F0302:702");
    expect_int("bracket.last", result.call_sequence[result.f0078_sequence_index],
               DM1_V1_CHAMPION_PANEL_ACTION_HAND_SLOT_PRIORITY_CALL_F0078_DISABLE_PC34,
               "CHAMPION.C F0302:713");
    expect_bool("bracket.after_helpers",
                result.f0078_sequence_index > result.f0077_sequence_index, true,
                "CHAMPION.C F0302:702-713 bracket order");
}

static void test_follower_swap_suppresses_intermediate_target_draw(void)
{
    dm1_v1_champion_panel_action_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_action_hand_slot_priority_result_t result;

    input.slot_box_index = 3;
    input.inventory_champion_ordinal = 1;
    input.party_champion_count = 2;
    input.leader_champion_index = 0;
    input.leader_hand_thing = 0x0101u;
    input.slots[1][1] = 0x0202u;
    result = M11_GameView_ChampionPanelActionHandSlotPriority_Dispatch(&input);

    expect_bool("follower.accepted", result.accepted, true,
                "CHAMPION.C F0302:677-713 follower status hand swap");
    expect_bool("follower.status_route", result.status_slot_box_route, true,
                "CHAMPION.C F0302:677 status hand route");
    expect_int("follower.target", result.target_champion_index, 1,
               "CHAMPION.C F0302:680 slotBoxIndex >> 1");
    expect_int("follower.target_ordinal", result.target_champion_ordinal, 2,
               "COMPILE.H:1038 M000 index to ordinal");
    expect_int("follower.leader", result.leader_champion_index, 0,
               "CHAMPION.C F0297 leader index draw route");
    expect_int("follower.leader_ordinal", result.leader_champion_ordinal, 1,
               "COMPILE.H:1038 M000 index to ordinal");
    expect_bool("follower.leader_is_target", result.leader_is_target_champion, false,
                "CHAMPION.C F0297:263-268 follower target");
    expect_bool("follower.suppressed",
                result.f0297_draw_state_suppressed_when_leader_not_target, true,
                "CHAMPION.C F0302 BUG0_39 comment: non-leader target has no intermediate target draw");
    expect_bool("follower.bug039", result.bug0_39_panel_flicker_route, false,
                "CHAMPION.C F0302 BUG0_39 leader-only flicker");
    expect_int("follower.intermediate_draw",
               result.f0292_intermediate_leader_draw_call_count, 0,
               "CHAMPION.C F0297:263-268 suppressed for target when leader differs");
    expect_int("follower.final_draw", result.f0292_final_draw_state_call_count, 1,
               "CHAMPION.C F0302:712 final F0292 remains");
    expect_int("follower.total_draw", result.f0292_draw_state_call_count, 1,
               "CHAMPION.C F0302:712 only final target draw");
    expect_u16("follower.leader_after", result.leader_hand_after, 0x0202u,
               "CHAMPION.C F0302:707 F0297");
    expect_u16("follower.slot_after", result.slots_after[1][1], 0x0101u,
               "CHAMPION.C F0302:710 F0301");
}

int main(void)
{
    test_evidence_and_invariants();
    test_empty_hand_empty_slot_early_return();
    test_put_leader_object_into_empty_action_slot();
    test_swap_preserves_bug039_helper_order();
    test_slot_mask_zero_rejects_leader_object();
    test_status_slot_candidate_dead_inventory_guards();
    test_chest_branch_uses_g0425_slots();
    test_screen_update_brackets_order();
    test_follower_swap_suppresses_intermediate_target_draw();

    if (g_failures) {
        printf("FAIL dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
