/* ReDMCSB MENU.C F0412 creates C70 from the cast map; TIMELINE.C F0257
 * line 1763 must create every weaker C70 on the current party map. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"

static void init_world(struct GameWorld_Compat* world,
                       struct DungeonThings_Compat* things)
{
    memset(world, 0, sizeof(*world));
    memset(things, 0, sizeof(*things));
    world->things = things;
    world->newPartyMapIndex = -1;
    world->party.championCount = 1;
    world->party.champions[0].present = 1;
    world->party.champions[0].hp.current = 100;
    world->party.champions[0].hp.maximum = 100;
    world->party.champions[0].mana.current = 100;
    world->party.champions[0].mana.maximum = 100;
    world->party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 60;
    world->lifecycle.champions[0].skills20[DM1_SKILL_IDX_AIR].experience = 5000;
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    uint32_t firstDecayTick;

    init_world(&world, &things);
    world.gameTick = 90;
    world.party.mapIndex = 4;
    world.partyMapIndex = 4;
    world.party.mapX = 9;
    world.party.mapY = 4;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_CAST_SPELL;
    input.commandArg1 = 0;
    input.commandArg2 = 6; /* Oh Ir Ra: LIGHT. */
    input.reserved = 1;    /* Lo -> C70 light power -3. */
    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_MAGIC_LIGHT_DECAY);
    assert(world.timeline.events[0].mapIndex == 4);
    firstDecayTick = world.timeline.events[0].fireAtTick;

    /* The live party has moved since F0412 scheduled its first C70. */
    world.party.mapIndex = 7;
    world.partyMapIndex = 2; /* Deliberately stale compatibility mirror. */
    world.gameTick = firstDecayTick;
    memset(&result, 0, sizeof(result));
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_MAGIC_LIGHT_DECAY);
    assert(world.timeline.events[0].fireAtTick == firstDecayTick + 4);
    assert(world.timeline.events[0].mapIndex == 7);
    assert(world.timeline.events[0].mapIndex != world.partyMapIndex);

    printf("PASS dm1_v1_f0412_light_timeline_map_pc34_compat\n");
    return 0;
}
