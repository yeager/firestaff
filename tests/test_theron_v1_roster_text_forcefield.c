#include "theron_v1_champions.h"
#include "theron_v1_dungeon_progression.h"
#include "theron_v1_startup_flow.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    static const char names[THERON_STARTUP_LAYOUT_ROSTER_CAPACITY]
        [THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY] = {
            "THERON", "MARA", "LINOS", "HEXA", "HAKAR", "TIRAN",
            "DOTAN", "PENTAI"
        };
    const char *name_ptrs[THERON_STARTUP_LAYOUT_ROSTER_CAPACITY];
    Theron_StartupFlow flow;
    Theron_DungeonProgression progression;
    Theron_V1_Party party;

    for (unsigned int i = 0u; i < THERON_STARTUP_LAYOUT_ROSTER_CAPACITY; ++i)
        name_ptrs[i] = names[i];

    theron_v1_startup_flow_init(&flow);
    theron_v1_dungeon_progression_init(&progression);
    assert(theron_v1_startup_choose_stage(
               &flow, &progression, THERON_DUNGEON_1_AKUTUBA) ==
           THERON_STARTUP_OK);
    assert(theron_v1_startup_select_mirror(&flow, 6) == THERON_STARTUP_OK);
    assert(theron_v1_startup_select_mirror(&flow, 2) == THERON_STARTUP_OK);
    memset(&party, 0, sizeof(party));
    assert(theron_v1_startup_enter_forcefield_with_roster(
               &flow, &party, name_ptrs,
               THERON_STARTUP_LAYOUT_ROSTER_CAPACITY) == THERON_STARTUP_OK);
    assert(strcmp(party.champions[0].name, "THERON") == 0);
    assert(strcmp(party.champions[1].name, "PENTAI") == 0);
    assert(strcmp(party.champions[2].name, "TIRAN") == 0);
    assert(party.champions[1].health == 550);
    assert(party.champions[2].health == 450);
    puts("PASS: authenticated roster text reaches Theron forcefield party");
    return 0;
}
