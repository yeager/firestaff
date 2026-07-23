/* ReDMCSB GROUP.C F0190 -> MOVESENS.C F0267 moving death handoff. */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned short group_thing(int index)
{
    return (unsigned short)((THING_TYPE_GROUP << 10) | (index & 0x03ff));
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonExplosion_Compat explosions[1];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char squareData0[1];
    unsigned char squareData1[1];
    unsigned short firstThings[2];
    unsigned short columns[2] = { 0, 1 };
    unsigned char rawC15[4];
    struct TimelineEvent_Compat event;
    struct TickResult_Compat result;

    memset(&world, 0, sizeof(world));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(explosions, 0, sizeof(explosions));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData0, 0, sizeof(squareData0));
    memset(squareData1, 0, sizeof(squareData1));
    memset(rawC15, 0xff, sizeof(rawC15));
    memset(&event, 0, sizeof(event));
    memset(&result, 0, sizeof(result));

    /* The C08 event opens this source pit; F0249 then moves its C04 group
     * through F0267.  One low-health creature makes F0191 reach F0190's
     * moving killed-all branch on the real source-square route. */
    squareData0[0] = (unsigned char)(DUNGEON_ELEMENT_PIT << 5);
    squareData1[0] = 0;
    firstThings[0] = group_thing(0);
    firstThings[1] = THING_ENDOFLIST;
    maps[0].width = maps[1].width = 1;
    maps[0].height = maps[1].height = 1;
    maps[0].level = 0;
    maps[1].level = 1;
    tiles[0].squareData = squareData0;
    tiles[0].squareCount = 1;
    tiles[1].squareData = squareData1;
    tiles[1].squareCount = 1;
    dungeon.header.mapCount = 2;
    dungeon.header.squareFirstThingCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.dungeonColumnCount = 2;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;

    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = THING_ENDOFLIST;
    groups[0].creatureType = DM1_CREATURE_TYPE_SKELETON;
    groups[0].count = 0;
    groups[0].cells = 0xffu;
    groups[0].health[0] = 1;
    explosions[0].next = THING_NONE;
    things.loaded = 1;
    things.groups = groups;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    /* F0821's moving-death smoke must reserve an original unused C15 row;
     * the runtime path has no synthetic explosion fallback. */
    things.explosions = explosions;
    things.explosionCount = 1;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    things.rawThingData[THING_TYPE_EXPLOSION] = rawC15;
    things.squareFirstThings = firstThings;
    things.squareFirstThingCount = 2;
    world.dungeon = &dungeon;
    world.things = &things;
    world.gameTick = 40;
    world.partyMapIndex = 0;
    world.party.mapIndex = 0;
    F0730_COMBAT_RngInit_Compat(&world.masterRng, 1u);

    event.kind = TIMELINE_EVENT_SQUARE_STATE;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.aux0 = DM1_EVENT_PIT;
    event.aux1 = DOOR_EFFECT_SET;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event));
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) >= 1);

    assert((squareData0[0] & 0x08u) != 0);
    assert(firstThings[0] == THING_ENDOFLIST);
    /* This small movement fixture deliberately lacks a complete source C15
     * square-list corpus after the group move. F0821 must leave both the raw
     * candidate and runtime untouched rather than synthesizing C040/C25. */
    assert(world.explosions.count == 0);
    assert(explosions[0].next == THING_NONE);
    assert(rawC15[0] == 0xff && rawC15[1] == 0xff);
    assert(world.timeline.count == 0);

    puts("PASS dm1_v1_f0190_moving_killed_all_m10_handoff_pc34_compat");
    return 0;
}
