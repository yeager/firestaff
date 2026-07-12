/* ReDMCSB GROUP.C F0189 -> F0181 runtime consumer regression. */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned short thing_ref(int type, int index) {
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

int main(void) {
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    struct DungeonGroup_Compat groups[1];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[4];
    unsigned short squareFirstThings[4];
    struct TimelineEvent_Compat event;
    struct TickResult_Compat result;
    int i;
    int stale = 0;
    int otherSquare = 0;
    int offPartyMap = getenv("FIRESTAFF_F0190_OFF_PARTY_MAP") != NULL;

    memset(&world, 0, sizeof(world));
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    memset(groups, 0, sizeof(groups));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    for (i = 0; i < 4; ++i) squareFirstThings[i] = THING_NONE;

    squareData[0] = (unsigned char)(DUNGEON_ELEMENT_DOOR << 5 | 1u);
    squareFirstThings[0] = thing_ref(THING_TYPE_GROUP, 0);
    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = thing_ref(THING_TYPE_JUNK, 0);
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].cells = 0xffu;
    groups[0].health[0] = 1;
    junks[0].next = THING_ENDOFLIST;
    things.loaded = 1;
    things.groups = groups;
    things.groupCount = 1;
    things.junks = junks;
    things.junkCount = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 4;
    dungeon.header.mapCount = 1;
    dungeon.header.squareFirstThingCount = 4;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 2;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 4;
    world.dungeon = &dungeon;
    world.things = &things;
    world.gameTick = 60;
    world.partyMapIndex = offPartyMap ? 1 : 0;
    world.party.mapIndex = offPartyMap ? 1 : 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.creatureAICount = 1;
    world.creatureAI[0].reserved0 = 0;

    assert(F0713_DOOR_BuildAnimationEvent_Compat(
        0, 0, 0, DOOR_EFFECT_CLEAR, world.gameTick, &event) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = world.gameTick + 20u;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.aux2 = DM1_EVENT_REACTION_DANGER_ON_SQUARE;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);
    event.mapX = 1;
    event.mapY = 1;
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);

    memset(&result, 0, sizeof(result));
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1);
    /* ReDMCSB GROUP.C F0189: the source group always unlinks, clears Next,
     * and deletes local C29..C41 events. ACTIVE_GROUP retirement is only
     * valid while the current group map is the party map. */
    assert(world.creatureAICount == (offPartyMap ? 1 : 0));
    assert(groups[0].next == THING_NONE);
    assert(squareFirstThings[0] == thing_ref(THING_TYPE_JUNK, 0));
    for (i = 0; i < world.timeline.count; ++i) {
        const struct TimelineEvent_Compat* current = &world.timeline.events[i];
        if (current->kind == TIMELINE_EVENT_CREATURE_REACTION &&
            current->mapIndex == 0 && current->mapX == 0 &&
            current->mapY == 0 &&
            current->aux2 >= DM1_EVENT_REACTION_DANGER_ON_SQUARE &&
            current->aux2 <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
            stale = 1;
        }
        if (current->kind == TIMELINE_EVENT_CREATURE_REACTION &&
            current->mapX == 1 && current->mapY == 1 &&
            current->aux2 == DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
            otherSquare = 1;
        }
    }
    assert(!stale);
    assert(otherSquare);
    puts(offPartyMap
         ? "PASS dm1_v1_f0190_killed_all_runtime_cleanup_off_party_pc34_compat"
         : "PASS dm1_v1_f0190_killed_all_runtime_cleanup_pc34_compat");
    return 0;
}
