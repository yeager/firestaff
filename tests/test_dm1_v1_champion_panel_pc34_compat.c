#include "dm1_v1_champion_panel_pc34_compat.h"

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

static void expect_bool(const char *id, bool got, bool want,
                        const char *anchor)
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

static const dm1_v1_champion_panel_status_hand_rotation_box_pc34_t *
box_for(const dm1_v1_champion_panel_status_hand_rotation_result_pc34_t *result,
        int champion_index,
        int slot_index)
{
    int index;

    for (index = 0; index < result->box_count; ++index) {
        const dm1_v1_champion_panel_status_hand_rotation_box_pc34_t *box =
            &result->boxes[index];
        if (box->champion_index == champion_index &&
            box->slot_index == (uint16_t)slot_index) {
            return box;
        }
    }
    return NULL;
}

static void test_evidence_and_chain(void)
{
    const dm1_v1_champion_panel_status_hand_rotation_evidence_pc34_t *evidence =
        dm1_v1_champion_panel_status_hand_rotation_evidence_pc34();
    const char *source =
        dm1_v1_champion_panel_status_hand_rotation_source_evidence_pc34();
    dm1_v1_champion_panel_status_hand_rotation_result_pc34_t result =
        dm1_v1_champion_panel_status_hand_rotation_plan_pc34(NULL);

    expect_bool("contract.only", result.contract_only, true,
                "contract-only source lock");
    expect_bool("contract.no_pass673_duplicate",
                result.no_mouth_eye_gate_duplicate, true,
                "CHAMDRAW.C F0292:898-935 redraw tuple only");
    expect_bool("contract.no_pass683_duplicate",
                result.no_food_water_gate_duplicate, true,
                "CHAMDRAW.C F0292:898-935 redraw tuple only");
    expect_int("chain.count", result.priority_chain_count, 4,
               "status-hand -> leader-hand -> backpack -> belt");
    expect_int("chain.0", result.priority_chain[0],
               DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_STATUS_HAND_PC34,
               "CHAMPION.C F0302:677-684");
    expect_int("chain.1", result.priority_chain[1],
               DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_LEADER_HAND_PC34,
               "CHAMPION.C F0297:243-298; F0298:270-298");
    expect_int("chain.2", result.priority_chain[2],
               DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_BACKPACK_PC34,
               "DEFS.H:793-809 backpack slots within 780-817");
    expect_int("chain.3", result.priority_chain[3],
               DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_BELT_PC34,
               "DEFS.H:786-792 belt/quick slots within 780-817");
    expect_bool("chain.status_before_leader",
                result.status_hand_precedes_leader_hand, true,
                "CHAMPION.C F0302:677-688");
    expect_bool("chain.leader_before_backpack",
                result.leader_hand_precedes_backpack, true,
                "CHAMPION.C F0297:243-298");
    expect_bool("chain.backpack_before_belt",
                result.backpack_precedes_belt, true,
                "status-hand -> leader-hand -> backpack -> belt");
    expect_contains("evidence.f0302.route",
                    evidence->champion_f0302_status_route,
                    "F0302:677-684", "CHAMPION.C F0302:677-684");
    expect_contains("evidence.f0297", evidence->champion_f0297_leader_put,
                    "F0297:243-298", "CHAMPION.C F0297:243-298");
    expect_contains("evidence.f0298", evidence->champion_f0298_leader_remove,
                    "F0298:270-298", "CHAMPION.C F0298:270-298");
    expect_contains("evidence.f0300", evidence->champion_f0300_c30_clear,
                    "F0300:511-515", "CHAMPION.C F0300:511-515");
    expect_contains("evidence.f0301", evidence->champion_f0301_c30_write,
                    "F0301:606-614", "CHAMPION.C F0301:606-614");
    expect_contains("evidence.swap", evidence->champion_f0302_occupied_swap,
                    "F0302:688-712", "CHAMPION.C F0302:688-712");
    expect_contains("evidence.f0291", evidence->chamdraw_f0291_action_hand,
                    "F0291:621-630", "CHAMDRAW.C F0291:621-630");
    expect_contains("evidence.f0292", evidence->chamdraw_f0292_redraw_tuple,
                    "F0292:898-935", "CHAMDRAW.C F0292:898-935");
    expect_contains("evidence.defs", evidence->defs_slots_and_globals,
                    "DEFS.H:780-817", "DEFS.H:780-817");
    expect_contains("evidence.flesh", evidence->defs_flesh_transparency,
                    "DEFS.H:2088", "DEFS.H:2088 C10_COLOR_FLESH");
    expect_contains("source.c033", source, "C033 normal",
                    "CHAMDRAW.C F0291:634-642");
    expect_contains("source.c034", source, "C034 wounded",
                    "CHAMDRAW.C F0291:638-642");
    expect_contains("source.c035", source, "C035 acting",
                    "CHAMDRAW.C F0291:634-635");
}

static void assert_party_rotation_matrix(void)
{
    uint16_t party_count;

    for (party_count = 1; party_count <= 4; ++party_count) {
        int acting_index;

        for (acting_index = 0; acting_index < 4; ++acting_index) {
            dm1_v1_champion_panel_status_hand_rotation_input_pc34_t input =
                dm1_v1_champion_panel_status_hand_rotation_default_input_pc34();
            dm1_v1_champion_panel_status_hand_rotation_result_pc34_t result;
            int champion;
            int expected_c035 = acting_index < (int)party_count ? 1 : 0;
            char id[96];

            input.party_champion_count = party_count;
            input.acting_champion_index = (int16_t)acting_index;
            input.wound_masks[0] = 1u << DM1_V1_CHAMPION_PANEL_C00_SLOT_READY_HAND_PC34;
            input.wound_masks[1] = 1u << DM1_V1_CHAMPION_PANEL_C01_SLOT_ACTION_HAND_PC34;
            input.wound_masks[2] = (uint16_t)(
                (1u << DM1_V1_CHAMPION_PANEL_C00_SLOT_READY_HAND_PC34) |
                (1u << DM1_V1_CHAMPION_PANEL_C01_SLOT_ACTION_HAND_PC34));
            input.wound_masks[3] = 0u;

            result = dm1_v1_champion_panel_status_hand_rotation_plan_pc34(&input);

            snprintf(id, sizeof(id), "party%u_acting%d.box_count",
                     (unsigned)party_count, acting_index);
            expect_int(id, result.box_count, 8,
                       "CHAMPION.C F0302:677-684 status hand slot boxes 0..7");
            snprintf(id, sizeof(id), "party%u_acting%d.visible_count",
                     (unsigned)party_count, acting_index);
            expect_int(id, result.visible_box_count, (int)party_count * 2,
                       "G0305_ui_PartyChampionCount visible champion guard");
            snprintf(id, sizeof(id), "party%u_acting%d.c035_count",
                     (unsigned)party_count, acting_index);
            expect_int(id, result.c035_acting_count, expected_c035,
                       "CHAMDRAW.C F0291:634-635 acting action hand C035");

            for (champion = 0; champion < 4; ++champion) {
                const dm1_v1_champion_panel_status_hand_rotation_box_pc34_t *ready =
                    box_for(&result, champion,
                            DM1_V1_CHAMPION_PANEL_C00_SLOT_READY_HAND_PC34);
                const dm1_v1_champion_panel_status_hand_rotation_box_pc34_t *action =
                    box_for(&result, champion,
                            DM1_V1_CHAMPION_PANEL_C01_SLOT_ACTION_HAND_PC34);
                int visible = champion < (int)party_count;
                int expected_ready_graphic;
                int expected_action_graphic;

                expected_ready_graphic =
                    visible && (input.wound_masks[champion] & 1u)
                        ? DM1_V1_CHAMPION_PANEL_C034_SLOT_BOX_WOUNDED_PC34
                        : DM1_V1_CHAMPION_PANEL_C033_SLOT_BOX_NORMAL_PC34;
                if (visible && champion == acting_index) {
                    expected_action_graphic =
                        DM1_V1_CHAMPION_PANEL_C035_SLOT_BOX_ACTING_HAND_PC34;
                } else if (visible && (input.wound_masks[champion] & 2u)) {
                    expected_action_graphic =
                        DM1_V1_CHAMPION_PANEL_C034_SLOT_BOX_WOUNDED_PC34;
                } else {
                    expected_action_graphic =
                        DM1_V1_CHAMPION_PANEL_C033_SLOT_BOX_NORMAL_PC34;
                }

                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.ready.exists",
                         (unsigned)party_count, acting_index, champion);
                expect_bool(id, ready != NULL, true,
                            "CHAMPION.C F0302:677-684 slotbox pair");
                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.action.exists",
                         (unsigned)party_count, acting_index, champion);
                expect_bool(id, action != NULL, true,
                            "CHAMPION.C F0302:677-684 slotbox pair");
                if (!ready || !action) {
                    continue;
                }

                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.ready.slotbox",
                         (unsigned)party_count, acting_index, champion);
                expect_int(id, ready->slot_box_index, champion * 2,
                           "CHAMPION.C F0302:680 slotbox >> 1");
                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.action.slotbox",
                         (unsigned)party_count, acting_index, champion);
                expect_int(id, action->slot_box_index, champion * 2 + 1,
                           "CHAMPION.C F0302:683 M070_HAND_SLOT_INDEX");
                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.visible.ready",
                         (unsigned)party_count, acting_index, champion);
                expect_bool(id, ready->visible_champion, visible != 0,
                            "CHAMPION.C F0302:681 party count guard");
                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.visible.action",
                         (unsigned)party_count, acting_index, champion);
                expect_bool(id, action->visible_champion, visible != 0,
                            "CHAMPION.C F0302:681 party count guard");
                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.route.ready",
                         (unsigned)party_count, acting_index, champion);
                expect_bool(id, ready->status_hand_route_before_inventory, true,
                            "CHAMPION.C F0302:677 C00..C07 before C08");
                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.route.action",
                         (unsigned)party_count, acting_index, champion);
                expect_bool(id, action->status_hand_route_before_inventory, true,
                            "CHAMPION.C F0302:677 C00..C07 before C08");
                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.ready.graphic",
                         (unsigned)party_count, acting_index, champion);
                expect_int(id, ready->native_bitmap_index, expected_ready_graphic,
                           "CHAMDRAW.C F0291:638-642 C034/C033");
                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.action.graphic",
                         (unsigned)party_count, acting_index, champion);
                expect_int(id, action->native_bitmap_index, expected_action_graphic,
                           "CHAMDRAW.C F0291:634-642 C035/C034/C033");
                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.action.flag",
                         (unsigned)party_count, acting_index, champion);
                expect_bool(id, action->action_hand, true,
                            "DEFS.H:781 C01_SLOT_ACTION_HAND");
                snprintf(id, sizeof(id), "party%u_acting%d_champ%d.acting.flag",
                         (unsigned)party_count, acting_index, champion);
                expect_bool(id, action->acting_action_hand,
                            visible && champion == acting_index,
                            "CHAMDRAW.C F0291:634 acting ordinal comparison");
            }
        }
    }
}

static void assert_c033_c034_c035_priority_cases(void)
{
    dm1_v1_champion_panel_status_hand_rotation_input_pc34_t input =
        dm1_v1_champion_panel_status_hand_rotation_default_input_pc34();
    dm1_v1_champion_panel_status_hand_rotation_result_pc34_t result;
    const dm1_v1_champion_panel_status_hand_rotation_box_pc34_t *box;

    input.party_champion_count = 4;
    input.acting_champion_index = 2;
    input.wound_masks[2] =
        (uint16_t)(1u << DM1_V1_CHAMPION_PANEL_C01_SLOT_ACTION_HAND_PC34);
    result = dm1_v1_champion_panel_status_hand_rotation_plan_pc34(&input);
    box = box_for(&result, 2, DM1_V1_CHAMPION_PANEL_C01_SLOT_ACTION_HAND_PC34);
    expect_bool("priority.acting_wounded.exists", box != NULL, true,
                "CHAMDRAW.C F0291:634-642");
    if (box) {
        expect_bool("priority.acting_wounded.flag", box->acting_action_hand,
                    true, "CHAMDRAW.C F0291:634");
        expect_bool("priority.acting_wounded.wounded", box->wounded_hand,
                    true, "CHAMDRAW.C F0291:638");
        expect_int("priority.acting_wounded.graphic",
                   box->native_bitmap_index,
                   DM1_V1_CHAMPION_PANEL_C035_SLOT_BOX_ACTING_HAND_PC34,
                   "CHAMDRAW.C F0291:634-635 C035 wins before C034");
    }

    input.acting_champion_index = 0;
    result = dm1_v1_champion_panel_status_hand_rotation_plan_pc34(&input);
    box = box_for(&result, 2, DM1_V1_CHAMPION_PANEL_C01_SLOT_ACTION_HAND_PC34);
    expect_bool("priority.wounded_not_acting.exists", box != NULL, true,
                "CHAMDRAW.C F0291:638-642");
    if (box) {
        expect_bool("priority.wounded_not_acting.flag", box->acting_action_hand,
                    false, "CHAMDRAW.C F0291:634");
        expect_int("priority.wounded_not_acting.graphic",
                   box->native_bitmap_index,
                   DM1_V1_CHAMPION_PANEL_C034_SLOT_BOX_WOUNDED_PC34,
                   "CHAMDRAW.C F0291:638-639 C034 after C035 branch");
    }

    box = box_for(&result, 3, DM1_V1_CHAMPION_PANEL_C01_SLOT_ACTION_HAND_PC34);
    expect_bool("priority.normal.exists", box != NULL, true,
                "CHAMDRAW.C F0291:640-642");
    if (box) {
        expect_bool("priority.normal.acting", box->acting_action_hand, false,
                    "CHAMDRAW.C F0291:634");
        expect_bool("priority.normal.wounded", box->wounded_hand, false,
                    "CHAMDRAW.C F0291:638");
        expect_int("priority.normal.graphic", box->native_bitmap_index,
                   DM1_V1_CHAMPION_PANEL_C033_SLOT_BOX_NORMAL_PC34,
                   "CHAMDRAW.C F0291:640-642 C033 fallback");
    }
}

void pass760_dm1_v1_champion_panel_status_hand_rotation_source_lock(void)
{
    test_evidence_and_chain();
    assert_party_rotation_matrix();
    assert_c033_c034_c035_priority_cases();
}

int main(void)
{
    pass760_dm1_v1_champion_panel_status_hand_rotation_source_lock();

    printf("Assertions: %d\n", g_assertions);
    printf("Failures: %d\n", g_failures);
    if (g_assertions < 80) {
        printf("FAIL assertion floor got=%d want>=80\n", g_assertions);
        return 1;
    }
    if (g_failures) {
        return 1;
    }
    printf("PASS pass760_dm1_v1_champion_panel_status_hand_rotation_source_lock assertions=%d\n",
           g_assertions);
    return 0;
}
