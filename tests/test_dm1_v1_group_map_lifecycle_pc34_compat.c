#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

static unsigned short group_thing_ref(int index)
{
    return (unsigned short)((THING_TYPE_GROUP << 10) | index);
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned char map0Squares[1];
    unsigned char map1Squares[1];
    unsigned short squareFirstThings[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));

    maps[0].width = maps[1].width = 1;
    maps[0].height = maps[1].height = 1;
    map0Squares[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    map1Squares[0] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                     DUNGEON_SQUARE_MASK_THING_LIST);
    tiles[0].squareData = map0Squares;
    tiles[1].squareData = map1Squares;
    tiles[0].squareCount = tiles[1].squareCount = 1;
    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_SKELETON;
    groups[0].cells = 0xffu;
    groups[0].direction = 2;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    squareFirstThings[0] = group_thing_ref(0);
    things.groups = groups;
    things.groupCount = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.loaded = 1;
    world.dungeon = &dungeon;
    world.things = &things;
    world.gameTick = 100u;
    world.partyMapIndex = 0;
    world.party.mapIndex = 0;
    world.newPartyMapIndex = 1;
    world.creatureAICount = 1;
    world.creatureAI[0].reserved0 = 99; /* stale active group from map 0 */

    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) ==
              ORCH_OK,
          "map transition tick succeeds");
    CHECK(world.partyMapIndex == 1 && world.party.mapIndex == 1,
          "map transition commits party map before F0195 scan");
    CHECK(world.creatureAICount == 1,
          "F0194 clears stale active state before F0195 adds current-map group");
    CHECK(world.creatureAI[0].reserved0 == 0 &&
          world.creatureAI[0].groupMapIndex == 1 &&
          world.creatureAI[0].groupMapX == 0 &&
          world.creatureAI[0].groupMapY == 0,
          "F0195 materializes current-map C04 active state");
    CHECK(world.timeline.count == 1 &&
          world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_TICK &&
          world.timeline.events[0].aux0 == 0 &&
          world.timeline.events[0].mapIndex == 1 &&
          world.timeline.events[0].fireAtTick == 101u,
          "F0195 restarts C37 wandering at game time plus one");

    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM1 F0194/F0195 active-group map lifecycle");
    return 0;
}
