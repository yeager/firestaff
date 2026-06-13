#include "dm1_v1_champion_panel_leader_swap_food_water_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void expect_int(const char *label, int got, int want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
    }
}

static void expect_u16(const char *label, uint16_t got, uint16_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%04X want=0x%04X anchor=%s\n",
               label, (unsigned)got, (unsigned)want, anchor);
    }
}

static void expect_str_contains(const char *label, const char *haystack,
                                const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=\"%s\" anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
    }
}

static void expect_str_not_contains(const char *label, const char *haystack,
                                    const char *needle, const char *anchor)
{
    ++g_assertions;
    if (haystack && needle && strstr(haystack, needle) != NULL) {
        ++g_failures;
        printf("FAIL %s unexpectedly contains=\"%s\" anchor=%s\n",
               label, needle, anchor);
    }
}

static dm1_v1_champion_panel_leader_swap_food_water_input_t
make_default_input(void)
{
    return M11_GameView_ChampionPanelLeaderSwapFoodWater_DefaultInput();
}

static void test_source_evidence_and_disjoint_contract(void)
{
    const char *source =
        M11_GameView_ChampionPanelLeaderSwapFoodWater_SourceEvidence();

    expect_str_contains("source.f0302_inventory_branch", source,
        "F0302:662-714", "CHAMPION.C F0302:662-714 inventory branch");
    expect_str_contains("source.f0297_internal_f0292", source,
        "F0297:243-268", "CHAMPION.C F0297:243-268 internal F0292 call");
    expect_str_contains("source.f0292_food_water_gate", source,
        "F0292:1060-1061", "CHAMDRAW.C F0292:1060-1061 food/water gate");
    expect_str_contains("source.f0300_panel_mask", source,
        "F0300:485,564,575", "CHAMPION.C F0300 MASK0x0800_PANEL anchor");
    expect_str_contains("source.f0301_panel_mask", source,
        "F0301:622,638", "CHAMPION.C F0301 MASK0x0800_PANEL anchor");
    expect_str_contains("source.bug0_39", source,
        "BUG0_39", "CHAMPION.C F0302:706 BUG0_39 comment");
    expect_str_contains("source.defs_masks", source,
        "MASK0x0200_LOAD", "DEFS.H MASK0x0200_LOAD");
    expect_str_contains("source.defs_masks", source,
        "MASK0x0800_PANEL", "DEFS.H MASK0x0800_PANEL");
    expect_str_contains("source.defs_globals", source,
        "G0411_i_LeaderIndex", "DEFS.H G0411 leader index");
    expect_str_contains("source.defs_globals", source,
        "G0423_i_InventoryChampionOrdinal",
        "DEFS.H G0423 inventory champion ordinal");
    expect_str_contains("source.contract_only", source,
        "contract_only=1", "evidence string");

    /* Disjoint with the food/water recompute clock gate. */
    expect_str_not_contains("disjoint.f0331_clock", source,
        "F0331:2331-2332",
        "F0331 clock slice owned by hud_food_water_recompute");
    expect_str_not_contains("disjoint.f0331_recovery", source,
        "F0331:2450-2473",
        "F0331 recovery slice owned by hud_food_water_recompute");
    expect_str_not_contains("disjoint.f0355_close_hook", source,
        "F0355:2316-2322",
        "F0355 close hook owned by hud_food_water_recompute");
    /* Disjoint with the action-hand slot-priority dispatch order gate. */
    expect_str_not_contains("disjoint.slot_dispatch", source,
        "F0298/F0300/F0297/F0301",
        "dispatch order owned by action_hand_slot_priority");
    expect_str_not_contains("disjoint.chest_close_redraw", source,
        "chest_close",
        "chest close -> status box owned by food_water_status_box");
}

static void test_leader_is_inventory_champion_predicate(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input =
        make_default_input();
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    const char *anchor =
        "CHAMPION.C F0302:685-687 inventory champion route; "
        "F0297:265-268 internal F0292 call only for leader";

    /* Leader == inventory champion. */
    input.leader_index = 0;
    input.inventory_ordinal = 1;
    input.inventory_index = 0;
    result = M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(&input);

    expect_int("leader_is_inventory_champion.true", result.leader_is_inventory_champion, 1,
               anchor);
    expect_int("intermediate_f0292_fires", result.f0297_internal_f0292_fires, 1,
               "F0297:265-268 F0292 call for leader");
    expect_int("intermediate_drew_food_water", result.intermediate_f0292_drew_food_water_panel, 1,
               "F0292:1060-1061 panel redraw with leader == inventory");
    expect_int("bug0_39_predicate.leader_inventory", result.bug0_39_flicker_predicate_holds, 1,
               "BUG0_39 in F0302:706 - leader is inventory, both objects present");
    expect_int("final_drew_food_water", result.final_f0292_drew_food_water_panel, 1,
               "F0301 sets MASK0x0800_PANEL, final F0292:1060-1061 fires");
}

static void test_leader_not_inventory_champion_predicate(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input =
        make_default_input();
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    const char *anchor =
        "F0292:1060-1061 L0863_B_IsInventoryChampion gate; "
        "intermediate F0292 draws leader, not the inventory champion";

    /* Leader is champion 0; inventory champion is champion 2. */
    input.leader_index = 0;
    input.inventory_ordinal = 3;
    input.inventory_index = 2;
    result = M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(&input);

    expect_int("leader_is_inventory_champion.false", result.leader_is_inventory_champion, 0,
               anchor);
    expect_int("intermediate_f0292_fires", result.f0297_internal_f0292_fires, 1,
               "F0297:265-268 still calls F0292(leader)");
    expect_int("intermediate_drew_food_water", result.intermediate_f0292_drew_food_water_panel, 0,
               "F0292:1060-1061 L0863_B_IsInventoryChampion is false for leader draw");
    expect_int("bug0_39_predicate.leader_not_inventory", result.bug0_39_flicker_predicate_holds, 0,
               "BUG0_39 only fires when leader is also the inventory champion");
    /*
     * The final F0292 always draws the inventory champion. F0301
     * sets MASK0x0800_PANEL on the inventory champion when the
     * added object is a chest or scroll, so the food/water panel
     * redraw still fires on the final F0292.
     */
    expect_int("final_drew_food_water", result.final_f0292_drew_food_water_panel, 1,
               "F0301:622,638 sets MASK0x0800_PANEL on inventory champion");
}

static void test_removed_not_chest_or_scroll_breaks_bug0_39(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input =
        make_default_input();
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    const char *anchor =
        "CHAMPION.C F0300:485,564,575 MASK0x0800_PANEL only for "
        "chest or scroll removal";

    input.removed_is_chest_or_scroll = 0;
    input.added_is_chest_or_scroll = 0;
    input.leader_index = 0;
    input.inventory_ordinal = 1;
    input.inventory_index = 0;
    result = M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(&input);

    expect_int("bug0_39_predicate.non_chest_scroll",
               result.bug0_39_flicker_predicate_holds, 0,
               anchor);
    expect_int("intermediate_drew_food_water",
               result.intermediate_f0292_drew_food_water_panel, 0,
               "F0300 did not set MASK0x0800_PANEL, F0292:1060 gate fails");
    expect_int("final_drew_food_water",
               result.final_f0292_drew_food_water_panel, 0,
               "F0301 did not set MASK0x0800_PANEL either");
}

static void test_leader_hand_empty_skips_f0298_and_predicate(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input =
        make_default_input();
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    const char *anchor =
        "CHAMPION.C F0302:704 F0298 only fires for non-empty leader hand";

    input.leader_hand_thing = 0xFFFFu;
    input.leader_index = 0;
    input.inventory_ordinal = 1;
    input.inventory_index = 0;
    result = M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(&input);

    expect_int("bug0_39_predicate.empty_leader_hand",
               result.bug0_39_flicker_predicate_holds, 0,
               anchor);
    expect_int("intermediate_f0292_fires",
               result.f0297_internal_f0292_fires, 1,
               "F0297 still fires when slot thing is non-empty");
    expect_int("intermediate_drew_food_water",
               result.intermediate_f0292_drew_food_water_panel, 1,
               "F0300:485 sets MASK0x0800_PANEL, leader is inventory, "
               "F0292:1060-1061 still fires");
    expect_u16("leader_hand_after", result.leader_hand_after, input.slot_thing_before,
               "F0297 puts slot thing into leader hand");
    expect_u16("slot_thing_after", result.slot_thing_after, 0xFFFFu,
               "F0301 did not run, slot stays empty");
    expect_int("f0301_call_count", result.f0301_add_slot_call_count, 0,
               "F0301 only runs when leader hand was non-empty");
}

static void test_slot_thing_empty_skips_f0297_f0300_and_predicate(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input =
        make_default_input();
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    const char *anchor =
        "CHAMPION.C F0302:705 F0300 only fires for non-empty slot thing";

    input.slot_thing_before = 0xFFFFu;
    input.leader_index = 0;
    input.inventory_ordinal = 1;
    input.inventory_index = 0;
    result = M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(&input);

    expect_int("bug0_39_predicate.empty_slot",
               result.bug0_39_flicker_predicate_holds, 0, anchor);
    expect_int("intermediate_f0292_fires",
               result.f0297_internal_f0292_fires, 0,
               "F0297 did not run, no internal F0292");
    expect_int("intermediate_call_count",
               result.f0292_intermediate_call_count, 0,
               "no intermediate F0292");
    expect_int("f0297_call_count",
               result.f0297_put_leader_hand_call_count, 0,
               "F0297 skipped");
    expect_int("f0300_call_count",
               result.f0300_remove_slot_call_count, 0,
               "F0300 skipped");
    expect_int("final_call_count", result.f0292_final_call_count, 1,
               "final F0292 still fires on the inventory champion");
}

static void test_helper_order_and_outer_bracket(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input =
        make_default_input();
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    const char *anchor =
        "CHAMPION.C F0302:702/713 outer F0077/F0078 bracket; "
        "F0302:702-711 helper order F0298,F0300,F0297,F0301,F0292";

    input.leader_index = 0;
    input.inventory_ordinal = 1;
    input.inventory_index = 0;
    result = M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(&input);

    expect_int("f0077_count", result.f0077_enable_screen_update_call_count, 1, anchor);
    expect_int("f0078_count", result.f0078_disable_screen_update_call_count, 1, anchor);
    expect_int("f0077_first", result.f0077_sequence_index, 0, anchor);
    expect_int("f0078_last", result.f0078_sequence_index,
               result.call_sequence_count - 1, anchor);
    expect_int("helper_order_preserved", result.helper_order_preserved, 1, anchor);
    expect_int("outer_bracket_preserved", result.outer_bracket_preserved, 1, anchor);

    expect_int("call[0]", result.call_sequence[0],
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0077_ENABLE_PC34, anchor);
    expect_int("call[1]", result.call_sequence[1],
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0298_REMOVE_LEADER_HAND_PC34, anchor);
    expect_int("call[2]", result.call_sequence[2],
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0300_REMOVE_SLOT_PC34, anchor);
    expect_int("call[3]", result.call_sequence[3],
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0297_PUT_LEADER_HAND_PC34, anchor);
    expect_int("call[4]", result.call_sequence[4],
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0292_DRAW_STATE_INTERMEDIATE_PC34,
        "F0292 inside F0297 (intermediate call)");
    expect_int("call[5]", result.call_sequence[5],
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0301_ADD_SLOT_PC34, anchor);
    expect_int("call[6]", result.call_sequence[6],
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0292_DRAW_STATE_FINAL_PC34,
        "F0302:711 trailing F0292");
    expect_int("call[7]", result.call_sequence[7],
        DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_CALL_F0078_DISABLE_PC34, anchor);
    expect_int("call_count", result.call_sequence_count, 8, anchor);
}

static void test_attribute_trace_anchors(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input =
        make_default_input();
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    const char *anchor =
        "CHAMPION.C F0300:485,564,575 + F0301:622,638 MASK0x0800_PANEL; "
        "F0297:265-268 MASK0x0200_LOAD on leader";

    input.leader_index = 0;
    input.inventory_ordinal = 1;
    input.inventory_index = 0;
    result = M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(&input);

    expect_u16("leader_load_set",
               result.attribute_trace.leader_attributes_at_f0297_entry,
               DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_LOAD_PC34,
               anchor);
    expect_u16("inventory_panel_set_after_f0300",
               result.attribute_trace.inventory_champion_attributes_at_f0300_entry,
               DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_PANEL_PC34,
               anchor);
    expect_u16("inventory_panel_and_action_hand_set_after_f0301",
               result.attribute_trace.inventory_champion_attributes_after_f0301,
               DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_PANEL_PC34 |
               DM1_V1_CHAMPION_PANEL_LEADER_SWAP_FOOD_WATER_MASK_ACTION_HAND_PC34,
               anchor);
    expect_int("mask_0200_load_set", result.mask_0200_load_set_on_leader_after_f0297, 1,
               "F0297:265 MASK0x0200_LOAD always set on leader");
    expect_int("mask_0800_panel_set_after_f0301",
               result.mask_0800_panel_set_on_inventory_after_f0301, 1,
               "F0301:622,638 MASK0x0800_PANEL when added object is chest/scroll");
}

static void test_f0292_champion_indices(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input =
        make_default_input();
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    const char *anchor =
        "CHAMPION.C F0297:265-268 F0292(leaderIndex); F0302:711 F0292(inventoryIndex)";

    input.leader_index = 0;
    input.inventory_ordinal = 1;
    input.inventory_index = 0;
    result = M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(&input);

    expect_int("f0292_intermediate_champion",
               result.f0292_intermediate_champion_index, 0, anchor);
    expect_int("f0292_final_champion",
               result.f0292_final_champion_index, 0, anchor);
    expect_int("f0292_call_count",
               result.f0292_draw_state_call_count, 2,
               "intermediate + final F0292 call");
    expect_int("f0292_indices[0]",
               result.f0292_draw_state_indices[0], 0,
               "first F0292 target is the leader (0)");
    expect_int("f0292_indices[1]",
               result.f0292_draw_state_indices[1], 0,
               "second F0292 target is the inventory champion (0)");
    expect_int("f0292_index_count",
               result.f0292_draw_state_index_count, 2,
               "two F0292 targets");
}

static void test_leader_not_inventory_f0292_targets(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input =
        make_default_input();
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    const char *anchor =
        "CHAMPION.C F0297:265-268 F0292(leaderIndex); F0302:711 F0292(inventoryIndex)";

    input.leader_index = 0;
    input.inventory_ordinal = 3;
    input.inventory_index = 2;
    result = M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(&input);

    expect_int("f0292_intermediate_champion",
               result.f0292_intermediate_champion_index, 0,
               "intermediate F0292 targets the leader, not the inventory champion");
    expect_int("f0292_final_champion",
               result.f0292_final_champion_index, 2,
               "final F0292 targets the inventory champion");
    expect_int("f0292_call_count",
               result.f0292_draw_state_call_count, 2,
               "intermediate + final F0292 call");
    expect_int("f0292_indices[0]",
               result.f0292_draw_state_indices[0], 0, anchor);
    expect_int("f0292_indices[1]",
               result.f0292_draw_state_indices[1], 2, anchor);
    expect_int("f0292_index_count",
               result.f0292_draw_state_index_count, 2,
               "two F0292 targets, leader then inventory champion");
    expect_int("intermediate_drew_food_water",
               result.intermediate_f0292_drew_food_water_panel, 0,
               "leader draw skips food/water branch at F0292:1060-1061");
    expect_int("intermediate_set_viewport",
               result.intermediate_f0292_set_viewport_dirty, 0,
               "F0292:1078-1087 VIEWPORT set only for inventory champion");
    expect_int("final_set_viewport",
               result.final_f0292_set_viewport_dirty, 1,
               "F0292:1078-1087 VIEWPORT set for inventory champion final draw");
}

static void test_null_input_defaults(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_result_t result =
        M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(NULL);
    const char *anchor =
        "synthetic default probe (NULL input -> default contract)";

    expect_int("null.default_inventory_champion_alive",
               result.inventory_champion_alive, 1, anchor);
    expect_int("null.leader_is_inventory",
               result.leader_is_inventory_champion, 1,
               "default leader_index=0 + inventory_index=0");
    expect_int("null.bug0_39_predicate",
               result.bug0_39_flicker_predicate_holds, 1,
               "defaults: removed + added both chest/scroll, both objects present");
    expect_int("null.f0292_call_count",
               result.f0292_draw_state_call_count, 2, anchor);
    expect_int("null.helper_order_preserved",
               result.helper_order_preserved, 1, anchor);
    expect_int("null.outer_bracket_preserved",
               result.outer_bracket_preserved, 1, anchor);
}

static void test_inventory_champion_out_of_range(void)
{
    dm1_v1_champion_panel_leader_swap_food_water_input_t input =
        make_default_input();
    dm1_v1_champion_panel_leader_swap_food_water_result_t result;
    const char *anchor =
        "synthetic default probe (out-of-range inventory_index)";

    input.inventory_index = -1;
    input.inventory_ordinal = 0;
    result = M11_GameView_ChampionPanelLeaderSwapFoodWater_Dispatch(&input);

    expect_int("oor.inventory_champion_alive",
               result.inventory_champion_alive, 0, anchor);
    expect_int("oor.f0292_call_count",
               result.f0292_draw_state_call_count, 0,
               "no F0292 fires when inventory champion is out of range");
    expect_int("oor.bug0_39_predicate",
               result.bug0_39_flicker_predicate_holds, 0, anchor);
}

int main(void)
{
    test_source_evidence_and_disjoint_contract();
    test_leader_is_inventory_champion_predicate();
    test_leader_not_inventory_champion_predicate();
    test_removed_not_chest_or_scroll_breaks_bug0_39();
    test_leader_hand_empty_skips_f0298_and_predicate();
    test_slot_thing_empty_skips_f0297_f0300_and_predicate();
    test_helper_order_and_outer_bracket();
    test_attribute_trace_anchors();
    test_f0292_champion_indices();
    test_leader_not_inventory_f0292_targets();
    test_null_input_defaults();
    test_inventory_champion_out_of_range();

    printf("dm1_v1_champion_panel_leader_swap_food_water_pc34_compat: "
           "%d assertions, %d failures\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
