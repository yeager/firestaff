/*
 * Production must reject a text-only forcefield handoff. Display names do
 * not own the selected champions' numeric Track 02 records. The positive
 * path is covered by test_theron_v1_startup_media_palette_bind with
 * hash-authenticated US/JP media.
 */

#include "theron_v1_dungeon_progression.h"
#include "theron_v1_startup_flow.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    Theron_StartupFlow flow;
    Theron_StartupFlow flow_before;
    Theron_DungeonProgression progression;
    Theron_V1_Party party;
    Theron_V1_Party party_before;

    theron_v1_startup_flow_init(&flow);
    theron_v1_dungeon_progression_init(&progression);
    assert(theron_v1_startup_choose_stage(
               &flow, &progression, THERON_DUNGEON_1_AKUTUBA) ==
           THERON_STARTUP_OK);
    assert(theron_v1_startup_select_mirror(&flow, 6) == THERON_STARTUP_OK);
    assert(theron_v1_startup_select_mirror(&flow, 2) == THERON_STARTUP_OK);
    memset(&party, 0, sizeof(party));
    flow_before = flow;
    party_before = party;

    assert(theron_v1_startup_enter_forcefield_with_roster(
               &flow, &party, NULL, 0) == THERON_STARTUP_ERR_NOT_READY);
    assert(memcmp(&flow, &flow_before, sizeof(flow)) == 0);
    assert(memcmp(&party, &party_before, sizeof(party)) == 0);

    puts("PASS: text-only champion names cannot bypass the authenticated Track 02 forcefield handoff");
    return 0;
}
