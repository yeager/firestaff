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
    check_int("init phase", flow.phase, THERON_STARTUP_PHASE_STAGE_SELECT);
    check_int("init selected dungeon", flow.selected_dungeon, THERON_DUNGEON_INVALID);
    check_int("init companion count", flow.companion_count, 0);

    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror before stage rejected", result, THERON_STARTUP_ERR_NO_STAGE);

    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
    check_int("locked stage rejected", result, THERON_STARTUP_ERR_STAGE_LOCKED);

    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_1_HALL_OF_RECORDS);
    check_int("stage 1 accepted", result, THERON_STARTUP_OK);
    check_int("stage phase soul room", flow.phase, THERON_STARTUP_PHASE_SOUL_ROOM);
    check_int("stage theron present", flow.theron_present, 1);

    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("mirror 0 accepted", result, THERON_STARTUP_OK);
    result = theron_v1_startup_select_mirror(&flow, 0);
    check_int("duplicate mirror rejected", result, THERON_STARTUP_ERR_DUPLICATE_MIRROR);
    result = theron_v1_startup_select_mirror(&flow, 2);
    check_int("mirror 2 accepted", result, THERON_STARTUP_OK);
    result = theron_v1_startup_select_mirror(&flow, 6);
    check_int("mirror 6 accepted", result, THERON_STARTUP_OK);
    check_int("three companions selected", flow.companion_count, 3);
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
    check_contains("slot 1 mirror name", party.champions[1].name, "Hero Mirror 1");
    check_contains("slot 2 mirror name", party.champions[2].name, "Hero Mirror 3");
    check_contains("slot 3 mirror name", party.champions[3].name, "Hero Mirror 7");
    check_int("slot 1 portrait", party.champions[1].portrait_index, 1);
    check_int("slot 2 portrait", party.champions[2].portrait_index, 3);
    check_int("slot 3 portrait", party.champions[3].portrait_index, 7);

    theron_v1_startup_flow_init(&flow);
    result = theron_v1_startup_choose_stage(&flow,
                                            &progression,
                                            THERON_DUNGEON_1_HALL_OF_RECORDS);
    check_int("stage accepted for Theron-only run", result, THERON_STARTUP_OK);
    memset(&party, 0, sizeof(party));
    result = theron_v1_startup_enter_forcefield(&flow, &party);
    check_int("Theron-only forcefield accepted", result, THERON_STARTUP_OK);
    check_int("Theron-only party count", party.champion_count, 1);

    check_contains("phase name", theron_v1_startup_phase_name(THERON_STARTUP_PHASE_SOUL_ROOM), "soul");
    check_contains("result name", theron_v1_startup_result_name(THERON_STARTUP_ERR_PARTY_FULL), "party");
    check_contains("source evidence", theron_v1_startup_flow_source_evidence(), "dmweb");

    printf("# total=%d passed=%d failed=%d\n", g_pass + g_fail, g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
