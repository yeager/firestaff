#include "dm1_v1_champion_panel_hand_slot_priority_pc34_compat.h"

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

static dm1_v1_champion_panel_hand_slot_priority_input_t base_input(void)
{
    dm1_v1_champion_panel_hand_slot_priority_input_t input =
        dm1_v1_champion_panel_hand_slot_priority_default_input_pc34();
    input.slot_box_index = 1;
    input.inventory_champion_ordinal = 2;
    input.leader_champion_index = 0;
    input.leader_hand_thing =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    input.slot_thing = 0x0202u;
    input.leader_object_allowed_slots = 0xFFFFu;
    input.slot_mask = 0x0002u;
    return input;
}

static void test_evidence_and_priority_chain(void)
{
    const dm1_v1_champion_panel_hand_slot_priority_evidence_t *evidence =
        dm1_v1_champion_panel_hand_slot_priority_evidence_pc34();
    const char *source =
        dm1_v1_champion_panel_hand_slot_priority_source_evidence_pc34();
    dm1_v1_champion_panel_hand_slot_priority_result_t result =
        dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(NULL);

    expect_bool("invariant.contract_only", result.invariant.contract_only, true,
                "contract-only header synthesis");
    expect_bool("invariant.status_first",
                result.invariant.models_status_hand_before_inventory, true,
                "CHAMPION.C F0302:677-684");
    expect_bool("invariant.leader_before_storage",
                result.invariant.models_leader_hand_before_storage_write, true,
                "CHAMPION.C F0302:688-710");
    expect_bool("invariant.backpack_belt",
                result.invariant.models_backpack_before_belt_priority, true,
                "source-of-truth hand-slot priority chain");
    expect_bool("invariant.c30",
                result.invariant.models_c30_chest_array_clear_and_write, true,
                "CHAMPION.C F0300/F0301 C30+");
    expect_bool("invariant.redraw",
                result.invariant.records_f0292_redraw_contract, true,
                "CHAMDRAW.C F0291/F0292");
    expect_bool("invariant.panel_dependency",
                result.invariant.records_panel_dependency_only, true,
                "PANEL.C F0344/F0345 dependency only");
    expect_bool("invariant.no_data",
                result.invariant.no_real_graphics_or_data_load, true,
                "no GRAPHICS.DAT/DUNGEON.DAT load");
    expect_bool("null.default", result.null_input_defaults_used, true,
                "default result");
    expect_int("chain.count", result.source_chain_count, 4,
               "status hand -> leader hand -> backpack -> belt");
    expect_int("chain.0", result.source_chain[0],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_STATUS_HAND_PC34,
               "CHAMPION.C F0302:677");
    expect_int("chain.1", result.source_chain[1],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_LEADER_HAND_PC34,
               "CHAMPION.C F0302:688");
    expect_int("chain.2", result.source_chain[2],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_BACKPACK_PC34,
               "CHAMDRAW.C F0296 inventory scan storage chain");
    expect_int("chain.3", result.source_chain[3],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_BELT_PC34,
               "source-of-truth hand-slot priority chain");
    expect_bool("chain.backpack_before_belt",
                result.backpack_precedes_belt_in_priority_chain, true,
                "source-of-truth hand-slot priority chain");
    expect_contains("evidence.f0302", evidence->f0302_status_route_anchor,
                    "F0302:677-684", "CHAMPION.C F0302:677-684");
    expect_contains("evidence.leader", evidence->leader_hand_anchor,
                    "F0297:243-298", "CHAMPION.C F0297:243-298");
    expect_contains("evidence.c30.clear", evidence->c30_clear_anchor,
                    "F0300:511-515", "CHAMPION.C F0300:511-515");
    expect_contains("evidence.c30.write", evidence->c30_write_anchor,
                    "F0301:606-614", "CHAMPION.C F0301:606-614");
    expect_contains("evidence.defs", evidence->defs_anchor,
                    "G0425", "DEFS.H G0425/G0426/G0423/G0305/M070/M516");
    expect_contains("evidence.chamdraw", evidence->chamdraw_anchor,
                    "F0291/F0292", "CHAMDRAW.C F0291/F0292");
    expect_contains("evidence.panel", evidence->panel_anchor,
                    "F0344/F0345", "PANEL.C F0344/F0345");
    expect_contains("evidence.dunview", evidence->dunview_non_claim_anchor,
                    "DUNVIEW.C", "DUNVIEW.C non-claim");
    expect_contains("source.chain", source,
                    "status hand -> leader hand -> backpack -> belt",
                    "source evidence priority chain");
}

static void test_status_hand_slotbox_0_to_7_routes(void)
{
    uint16_t slot_box;

    for (slot_box = 0; slot_box < 8; ++slot_box) {
        dm1_v1_champion_panel_hand_slot_priority_input_t input = base_input();
        dm1_v1_champion_panel_hand_slot_priority_result_t result;
        char id[64];

        input.slot_box_index = slot_box;
        input.inventory_champion_ordinal = 0;
        result = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);

        snprintf(id, sizeof(id), "slot%u.status_route", (unsigned)slot_box);
        expect_bool(id, result.status_hand_route, true,
                    "CHAMPION.C F0302:677 status route");
        snprintf(id, sizeof(id), "slot%u.champion", (unsigned)slot_box);
        expect_int(id, result.target_champion_index, (int)(slot_box >> 1),
                   "CHAMPION.C F0302:680 champion index = slotbox >> 1");
        snprintf(id, sizeof(id), "slot%u.hand", (unsigned)slot_box);
        expect_int(id, result.target_slot_index, (int)(slot_box & 1u),
                   "CHAMPION.C F0302:683 M070_HAND_SLOT_INDEX");
        snprintf(id, sizeof(id), "slot%u.family", (unsigned)slot_box);
        expect_int(id, result.slot_family,
                   DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_STATUS_HAND_PC34,
                   "CHAMPION.C F0302:677-684 slot-order chain");
        snprintf(id, sizeof(id), "slot%u.accepted", (unsigned)slot_box);
        expect_bool(id, result.accepted, true,
                    "CHAMPION.C F0302:688-712 accepted status hand move");
    }
}

static void test_status_hand_guard_priority(void)
{
    dm1_v1_champion_panel_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_hand_slot_priority_result_t result;

    input.slot_box_index = 3;
    input.candidate_champion_ordinal = 1;
    input.party_champion_count = 1;
    input.inventory_champion_ordinal = 2;
    input.current_health[1] = 0;
    result = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);
    expect_bool("guard.candidate.status_route", result.status_hand_route, true,
                "CHAMPION.C F0302:677");
    expect_int("guard.candidate.first", result.return_reason,
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_CANDIDATE_ACTIVE_PC34,
               "CHAMPION.C F0302:678-679 candidate returns before other guards");
    expect_int("guard.candidate.calls", result.call_sequence_count, 0,
               "CHAMPION.C F0302:679 return before helper calls");

    input = base_input();
    input.slot_box_index = 6;
    input.party_champion_count = 3;
    input.inventory_champion_ordinal = 0;
    result = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);
    expect_int("guard.party_bound", result.return_reason,
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_PARTY_BOUND_PC34,
               "CHAMPION.C F0302:681 party count guard");
    expect_int("guard.party.calls", result.call_sequence_count, 0,
               "CHAMPION.C F0302:682 return before helper calls");

    input = base_input();
    input.slot_box_index = 2;
    input.inventory_champion_ordinal = 2;
    result = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);
    expect_int("guard.inventory_open", result.return_reason,
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_INVENTORY_CHAMPION_PC34,
               "CHAMPION.C F0302:681 inventory champion guard");
    expect_int("guard.inventory.calls", result.call_sequence_count, 0,
               "CHAMPION.C F0302:682 return before helper calls");

    input = base_input();
    input.slot_box_index = 4;
    input.inventory_champion_ordinal = 0;
    input.current_health[2] = 0;
    result = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);
    expect_int("guard.dead", result.return_reason,
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_DEAD_CHAMPION_PC34,
               "CHAMPION.C F0302:681 current health guard");
    expect_int("guard.dead.calls", result.call_sequence_count, 0,
               "CHAMPION.C F0302:682 return before helper calls");
}

static void test_leader_hand_transaction_order(void)
{
    dm1_v1_champion_panel_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_hand_slot_priority_result_t result;

    input.slot_box_index = 1;
    input.inventory_champion_ordinal = 0;
    input.leader_hand_thing = 0x0101u;
    input.slot_thing = 0x0202u;
    input.leader_object_allowed_slots = 0x0002u;
    input.slot_mask = 0x0002u;
    result = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);

    expect_bool("swap.accepted", result.accepted, true,
                "CHAMPION.C F0302:700-712 accepted swap");
    expect_bool("swap.leader_checked", result.leader_hand_checked_before_storage_write, true,
                "CHAMPION.C F0302:688 before F0300/F0301");
    expect_u16("swap.leader_before", result.leader_hand_before, 0x0101u,
               "CHAMPION.C F0302:688 leader hand snapshot");
    expect_u16("swap.slot_before", result.slot_thing_before, 0x0202u,
               "CHAMPION.C F0302:690-692 slot snapshot");
    expect_u16("swap.leader_after", result.leader_hand_after, 0x0202u,
               "CHAMPION.C F0302:705-706 F0300 then F0297");
    expect_u16("swap.slot_after", result.slot_thing_after, 0x0101u,
               "CHAMPION.C F0302:708-710 F0301 writes saved leader object");
    expect_int("swap.calls", result.call_sequence_count, 7,
               "CHAMPION.C F0302:700-712 full helper order");
    expect_int("swap.call0", result.call_sequence[0],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0077_ENABLE_PC34,
               "CHAMPION.C F0302:700");
    expect_int("swap.call1", result.call_sequence[1],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0298_REMOVE_LEADER_PC34,
               "CHAMPION.C F0302:701-702");
    expect_int("swap.call2", result.call_sequence[2],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0300_CLEAR_SLOT_PC34,
               "CHAMPION.C F0302:704-705");
    expect_int("swap.call3", result.call_sequence[3],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0297_PUT_LEADER_PC34,
               "CHAMPION.C F0302:706 BUG0_39 order");
    expect_int("swap.call4", result.call_sequence[4],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0301_WRITE_SLOT_PC34,
               "CHAMPION.C F0302:708-710");
    expect_int("swap.call5", result.call_sequence[5],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0292_DRAW_STATE_PC34,
               "CHAMPION.C F0302:711");
    expect_int("swap.call6", result.call_sequence[6],
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_CALL_F0078_DISABLE_PC34,
               "CHAMPION.C F0302:712");
    expect_bool("swap.f0292.final", result.f0292_final_draw_state, true,
                "CHAMPION.C F0302:711 final redraw");
    expect_bool("swap.f0297.intermediate", result.f0297_intermediate_draw_state, true,
                "CHAMPION.C F0297:263-267 leader-hand redraw");
    expect_bool("swap.panel_dependency", result.panel_redraw_dependency, true,
                "PANEL.C F0344/F0345 dependency on leader action-hand panel");
}

static void test_storage_families_and_c30_route(void)
{
    dm1_v1_champion_panel_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_hand_slot_priority_result_t result;

    expect_int("family.ready",
               dm1_v1_champion_panel_hand_slot_priority_family_for_slot_pc34(0, false),
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_INVENTORY_HAND_PC34,
               "DEFS.H C00_SLOT_READY_HAND");
    expect_int("family.action",
               dm1_v1_champion_panel_hand_slot_priority_family_for_slot_pc34(1, false),
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_INVENTORY_HAND_PC34,
               "DEFS.H C01_SLOT_ACTION_HAND");
    expect_int("family.body",
               dm1_v1_champion_panel_hand_slot_priority_family_for_slot_pc34(4, false),
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_BODY_PC34,
               "DEFS.H body slots");
    expect_int("family.belt",
               dm1_v1_champion_panel_hand_slot_priority_family_for_slot_pc34(11, false),
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_BELT_PC34,
               "DEFS.H pouch/quiver/neck quick slots");
    expect_int("family.backpack",
               dm1_v1_champion_panel_hand_slot_priority_family_for_slot_pc34(13, false),
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_BACKPACK_PC34,
               "DEFS.H C13_SLOT_BACKPACK_LINE1_1");
    expect_int("family.chest",
               dm1_v1_champion_panel_hand_slot_priority_family_for_slot_pc34(30, false),
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_CHEST_PC34,
               "DEFS.H C30_SLOT_CHEST_1");
    expect_int("family.invalid",
               dm1_v1_champion_panel_hand_slot_priority_family_for_slot_pc34(38, false),
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_CHEST_PC34,
               "C38_SLOT_BOX_CHEST_FIRST_SLOT maps through slot-box delta first");

    input.slot_box_index =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_C08_INVENTORY_FIRST_PC34 + 30u;
    input.inventory_champion_ordinal = 1;
    input.leader_hand_thing = 0x0333u;
    input.slot_thing = 0x0444u;
    input.leader_object_allowed_slots = 0x8000u;
    input.slot_mask = 0x8000u;
    result = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);
    expect_bool("c30.accepted", result.accepted, true,
                "CHAMPION.C F0302 routes inventory C30");
    expect_bool("c30.route", result.c30_chest_slot_route, true,
                "CHAMPION.C F0302:689-690 C30 branch");
    expect_int("c30.family", result.slot_family,
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_FAMILY_CHEST_PC34,
               "DEFS.H C30_SLOT_CHEST_1");
    expect_bool("c30.clear.g0425", result.c30_clear_uses_g0425, true,
                "CHAMPION.C F0300:511-515 G0425_aT_ChestSlots clear");
    expect_bool("c30.write.g0425", result.c30_write_uses_g0425, true,
                "CHAMPION.C F0301:606-614 G0425_aT_ChestSlots write");
}

static void test_early_returns_and_inventory_route(void)
{
    dm1_v1_champion_panel_hand_slot_priority_input_t input = base_input();
    dm1_v1_champion_panel_hand_slot_priority_result_t result;

    input.slot_box_index = 9;
    input.inventory_champion_ordinal = 1;
    input.leader_hand_thing =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    input.slot_thing =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    result = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);
    expect_bool("empty.inventory_route", result.inventory_route, true,
                "CHAMPION.C F0302:684-687 inventory route");
    expect_int("empty.champion", result.target_champion_index, 0,
               "CHAMPION.C F0302:685 M001 inventory ordinal");
    expect_int("empty.slot", result.target_slot_index, 1,
               "CHAMPION.C F0302:686 slot-box delta");
    expect_int("empty.reason", result.return_reason,
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_EMPTY_HAND_EMPTY_SLOT_PC34,
               "CHAMPION.C F0302:694-695");
    expect_int("empty.calls", result.call_sequence_count, 0,
               "CHAMPION.C F0302:695 return before F0077");

    input = base_input();
    input.slot_box_index = 9;
    input.inventory_champion_ordinal = 1;
    input.leader_hand_thing = 0x0101u;
    input.slot_thing =
        DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_THING_NONE_PC34;
    input.leader_object_allowed_slots = 0x0001u;
    input.slot_mask = 0x0002u;
    result = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);
    expect_int("mask.reason", result.return_reason,
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_MASK_REJECT_PC34,
               "CHAMPION.C F0302:697-698");
    expect_int("mask.calls", result.call_sequence_count, 0,
               "CHAMPION.C F0302:698 return before helper calls");
    expect_u16("mask.leader_unchanged", result.leader_hand_after, 0x0101u,
               "CHAMPION.C F0302:697 state unchanged");

    input = base_input();
    input.slot_box_index = 9;
    input.inventory_champion_ordinal = 0;
    result = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);
    expect_int("invalid.ordinal", result.return_reason,
               DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_RETURN_INVALID_INVENTORY_ORDINAL_PC34,
               "CHAMPION.C F0302:685 requires open inventory champion");
}

int main(void)
{
    test_evidence_and_priority_chain();
    test_status_hand_slotbox_0_to_7_routes();
    test_status_hand_guard_priority();
    test_leader_hand_transaction_order();
    test_storage_families_and_c30_route();
    test_early_returns_and_inventory_route();

    printf("Assertions: %d\n", g_assertions);
    printf("Failures: %d\n", g_failures);
    if (g_assertions < 40) {
        printf("FAIL assertion floor got=%d want>=40\n", g_assertions);
        return 1;
    }
    if (g_failures) {
        return 1;
    }
    printf("DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_PC34_COMPAT_OK\n");
    return 0;
}
