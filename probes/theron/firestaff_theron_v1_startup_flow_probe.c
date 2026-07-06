#include "theron_v1_startup_flow.h"

#include <stdio.h>
#include <string.h>

static int g_pass;
static int g_fail;

static void check_int(const char *label, int got, int expected) {
    if (got == expected) {
        printf("PASS %s=%d\n", label, got);
        ++g_pass;
    } else {
        printf("FAIL %s got=%d expected=%d\n", label, got, expected);
        ++g_fail;
    }
}

static void check_contains(const char *label, const char *haystack, const char *needle) {
    if (haystack && needle && strstr(haystack, needle)) {
        printf("PASS %s contains %s\n", label, needle);
        ++g_pass;
    } else {
        printf("FAIL %s missing %s in %s\n",
               label,
               needle ? needle : "(null)",
               haystack ? haystack : "(null)");
        ++g_fail;
    }
}

int main(void) {
    Theron_DungeonProgression progression;
    Theron_StartupFlow flow;
    Theron_V1_Party party;
    Theron_StartupResult result;

    printf("=== Theron V1 startup flow probe ===\n");
    printf("source: %s\n", theron_v1_startup_flow_source_evidence());

    theron_v1_dungeon_progression_init(&progression);
    theron_v1_startup_flow_init(&flow);
    check_int("init phase", flow.phase, THERON_STARTUP_PHASE_TITLE);
    check_int("init selected dungeon", flow.selected_dungeon, THERON_DUNGEON_INVALID);
    check_int("init companion count", flow.companion_count, 0);

    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror before stage rejected", result, THERON_STARTUP_ERR_NO_STAGE);

    {
        Theron_StartupAction action;
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_TITLE,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("title accept action rc", result, THERON_STARTUP_OK);
        check_int("title accept shows stage select",
                  action.kind,
                  THERON_STARTUP_ACTION_SHOW_STAGE_SELECT);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            1,
            THERON_STARTUP_INPUT_UP,
            &action);
        check_int("stage up wraps to continue rc", result, THERON_STARTUP_OK);
        check_int("stage up action moves cursor",
                  action.kind,
                  THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR);
        check_int("stage up focuses continue", action.continue_focus, 1);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_3_ABYSS_OF_FLAMES,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_DOWN,
            &action);
        check_int("stage down rc", result, THERON_STARTUP_OK);
        check_int("stage down selected dungeon",
                  action.selected_dungeon,
                  THERON_DUNGEON_4_TOMB_OF_WOE);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_4_TOMB_OF_WOE,
            0,
            1,
            1,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("stage continue accept rc", result, THERON_STARTUP_OK);
        check_int("stage continue accept action",
                  action.kind,
                  THERON_STARTUP_ACTION_CONTINUE_SAVE);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_STAGE_SELECT,
            THERON_DUNGEON_4_TOMB_OF_WOE,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_ACTION,
            &action);
        check_int("stage choose action rc", result, THERON_STARTUP_OK);
        check_int("stage choose action kind",
                  action.kind,
                  THERON_STARTUP_ACTION_CHOOSE_STAGE);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_SOUL_ROOM,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            0,
            0,
            THERON_STARTUP_INPUT_LEFT,
            &action);
        check_int("soul left rc", result, THERON_STARTUP_OK);
        check_int("soul left wraps to forcefield",
                  action.cursor,
                  THERON_STARTUP_HERO_MIRROR_COUNT);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            THERON_STARTUP_HERO_MIRROR_COUNT,
            0,
            0,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("ready forcefield accept rc", result, THERON_STARTUP_OK);
        check_int("ready forcefield accept action",
                  action.kind,
                  THERON_STARTUP_ACTION_ENTER_FORCEFIELD);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            3,
            0,
            0,
            THERON_STARTUP_INPUT_ACCEPT,
            &action);
        check_int("ready mirror accept rc", result, THERON_STARTUP_OK);
        check_int("ready mirror accept action",
                  action.kind,
                  THERON_STARTUP_ACTION_TOGGLE_MIRROR);
        check_int("ready mirror accept index", action.mirror_index, 3);
        result = theron_v1_startup_handle_input(
            THERON_STARTUP_PHASE_READY,
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            3,
            0,
            0,
            THERON_STARTUP_INPUT_BACK,
            &action);
        check_int("ready back rc", result, THERON_STARTUP_OK);
        check_int("ready back action",
                  action.kind,
                  THERON_STARTUP_ACTION_SHOW_STAGE_SELECT);
        check_contains("action label",
                       theron_v1_startup_action_name(
                           THERON_STARTUP_ACTION_ENTER_FORCEFIELD),
                       "forcefield");
    }

    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
    check_int("locked stage rejected", result, THERON_STARTUP_ERR_STAGE_LOCKED);

    theron_v1_dungeon_advance(&progression);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_5_VAULT_OF_SECRETS);
    check_int("middle stage accepted after first completion", result, THERON_STARTUP_OK);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_7_TOWER_OF_EPILOGUE);
    check_int("final stage still locked after first completion",
              result,
              THERON_STARTUP_ERR_STAGE_LOCKED);

    theron_v1_dungeon_progression_init(&progression);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_1_HALL_OF_RECORDS);
    check_int("stage 1 accepted", result, THERON_STARTUP_OK);
    check_int("stage phase soul room", flow.phase, THERON_STARTUP_PHASE_SOUL_ROOM);
    check_int("stage theron present", flow.theron_present, 1);

    result = theron_v1_startup_select_mirror(&flow, 6);
    check_int("mirror 6 accepted first", result, THERON_STARTUP_OK);
    result = theron_v1_startup_select_mirror(&flow, 6);
    check_int("duplicate mirror rejected", result, THERON_STARTUP_ERR_DUPLICATE_MIRROR);
    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror 0 accepted second", result, THERON_STARTUP_OK);
    result = theron_v1_startup_select_mirror(&flow, 2);
    check_int("mirror 2 accepted third", result, THERON_STARTUP_OK);
    result = theron_v1_startup_deselect_mirror(&flow, 0);
    check_int("mirror 0 deselected", result, THERON_STARTUP_OK);
    check_int("companion count after middle deselect", flow.companion_count, 2);
    check_int("first order after middle deselect", flow.selected_mirror_order[0], 6);
    check_int("second order after middle deselect", flow.selected_mirror_order[1], 2);
    check_int("third order cleared after middle deselect",
              flow.selected_mirror_order[2],
              0xff);
    result = theron_v1_startup_deselect_mirror(&flow, 0);
    check_int("unselected mirror deselect rejected",
              result,
              THERON_STARTUP_ERR_MIRROR_NOT_SELECTED);
    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror 0 reaccepted third", result, THERON_STARTUP_OK);
    check_int("three companions selected", flow.companion_count, 3);
    check_int("first selected mirror order", flow.selected_mirror_order[0], 6);
    check_int("second selected mirror order", flow.selected_mirror_order[1], 2);
    check_int("third selected mirror order", flow.selected_mirror_order[2], 0);
    check_int("ready phase", flow.phase, THERON_STARTUP_PHASE_READY);

    result = theron_v1_startup_select_mirror(&flow, 4);
    check_int("fourth companion rejected", result, THERON_STARTUP_ERR_PARTY_FULL);

    memset(&party, 0, sizeof(party));
    result = theron_v1_startup_enter_forcefield(&flow, &party);
    check_int("forcefield accepted", result, THERON_STARTUP_OK);
    check_int("forcefield flag", flow.forcefield_entered, 1);
    check_int("in dungeon phase", flow.phase, THERON_STARTUP_PHASE_IN_DUNGEON);
    check_int("party count theron plus three", party.champion_count, 4);
    check_int("leader slot is Theron", party.active_slot, THERON_CHAMPION_SLOT_THERON);
    check_contains("slot 0 name", party.champions[0].name, "Theron");
    check_contains("slot 1 mirror name", party.champions[1].name, "Pental");
    check_contains("slot 2 mirror name", party.champions[2].name, "Tiran");
    check_contains("slot 3 mirror name", party.champions[3].name, "Hakar");
    check_int("slot 1 portrait", party.champions[1].portrait_index, 7);
    check_int("slot 2 portrait", party.champions[2].portrait_index, 3);
    check_int("slot 3 portrait", party.champions[3].portrait_index, 1);
    check_int("slot 1 class", party.champions[1].primary_class, THERON_CLASS_FIGHTER);
    check_int("slot 2 class", party.champions[2].primary_class, THERON_CLASS_FIGHTER);
    check_int("slot 3 class", party.champions[3].primary_class, THERON_CLASS_FIGHTER);
    check_contains("mirror 0 meta", theron_v1_startup_mirror_meta(0)->name, "Hakar");
    check_contains("mirror 1 meta", theron_v1_startup_mirror_meta(1)->name, "Mara");
    check_contains("mirror 3 meta", theron_v1_startup_mirror_meta(3)->name, "Linos");
    check_contains("mirror 6 meta", theron_v1_startup_mirror_meta(6)->name, "Pental");
    check_int("mirror 0 roster index", theron_v1_startup_roster_index_for_mirror(0), 4);
    check_int("mirror 1 roster index", theron_v1_startup_roster_index_for_mirror(1), 1);
    check_int("mirror 2 roster index", theron_v1_startup_roster_index_for_mirror(2), 5);
    check_int("mirror 3 roster index", theron_v1_startup_roster_index_for_mirror(3), 2);
    check_int("mirror 4 roster index", theron_v1_startup_roster_index_for_mirror(4), 6);
    check_int("mirror 5 roster index", theron_v1_startup_roster_index_for_mirror(5), 3);
    check_int("mirror 6 roster index", theron_v1_startup_roster_index_for_mirror(6), 7);
    check_int("bad mirror roster index", theron_v1_startup_roster_index_for_mirror(99), -1);
    check_int("roster index 4 mirror", theron_v1_startup_mirror_index_for_roster(4), 0);
    check_int("roster index 1 mirror", theron_v1_startup_mirror_index_for_roster(1), 1);
    check_int("roster index 5 mirror", theron_v1_startup_mirror_index_for_roster(5), 2);
    check_int("roster index 2 mirror", theron_v1_startup_mirror_index_for_roster(2), 3);
    check_int("roster index 6 mirror", theron_v1_startup_mirror_index_for_roster(6), 4);
    check_int("roster index 3 mirror", theron_v1_startup_mirror_index_for_roster(3), 5);
    check_int("roster index 7 mirror", theron_v1_startup_mirror_index_for_roster(7), 6);
    check_int("Theron roster index has no mirror", theron_v1_startup_mirror_index_for_roster(0), -1);
    check_contains("class label", theron_v1_startup_class_name(THERON_CLASS_PRIEST), "PRIEST");

    theron_v1_startup_flow_init(&flow);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_1_HALL_OF_RECORDS);
    check_int("stage accepted for Theron-only run", result, THERON_STARTUP_OK);
    memset(&party, 0, sizeof(party));
    result = theron_v1_startup_enter_forcefield(&flow, &party);
    check_int("Theron-only forcefield accepted", result, THERON_STARTUP_OK);
    check_int("Theron-only party count", party.champion_count, 1);

    check_contains("title phase name",
                   theron_v1_startup_phase_name(THERON_STARTUP_PHASE_TITLE),
                   "title");
    check_contains("phase name",
                   theron_v1_startup_phase_name(THERON_STARTUP_PHASE_SOUL_ROOM),
                   "soul");
    check_contains("result name", theron_v1_startup_result_name(THERON_STARTUP_ERR_PARTY_FULL), "party");
    check_contains("deselect result name",
                   theron_v1_startup_result_name(THERON_STARTUP_ERR_MIRROR_NOT_SELECTED),
                   "mirror");
    check_contains("source evidence", theron_v1_startup_flow_source_evidence(), "dmweb");

    printf("# total=%d passed=%d failed=%d\n", g_pass + g_fail, g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
