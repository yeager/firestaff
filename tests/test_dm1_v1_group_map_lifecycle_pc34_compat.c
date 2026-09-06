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
    unsigned char map1Squares[4];
    unsigned short squareFirstThings[1];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    static struct GameWorld_Compat before;
    struct DungeonGroup_Compat groupBefore;
    unsigned short columns[3] = {0, 0, 0};

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
    maps[1].width = maps[1].height = 2;
    map0Squares[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    memset(map1Squares, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(map1Squares));
    map1Squares[3] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                     DUNGEON_SQUARE_MASK_THING_LIST);
    tiles[0].squareData = map0Squares;
    tiles[1].squareData = map1Squares;
    tiles[0].squareCount = tiles[1].squareCount = 1;
    tiles[1].squareCount = 4;
    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    dungeon.dungeonColumnCount = 3;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
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
    world.masterRng.seed = 7u;
    world.partyMapIndex = 0;
    world.party.mapIndex = 0;
    world.newPartyMapIndex = 1;
    world.creatureAICount = 1;
    world.creatureAI[0].reserved0 = 99; /* stale active group from map 0 */
    world.pc34ActiveGroupHistory[0].valid = 1;
    world.pc34ActiveGroupHistory[0].groupThingIndex = 99;
    world.pc34ActiveGroupHistory[0].priorMapX = 17;
    world.pc34ActiveGroupHistory[0].priorMapY = 23;
    world.pc34ActiveGroupHistory[0].lastMoveTime = 71;

    before = world;
    groupBefore = groups[0];
    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_FAIL,
          "invalid outgoing owner rejects map transition");
    CHECK(memcmp(&world, &before, sizeof(world)) == 0 &&
          memcmp(&groups[0], &groupBefore, sizeof(groupBefore)) == 0,
          "invalid owner leaves world, history and C04 unchanged");

    /* GROUP.C F0194/F0195: removal precedes destination admission. Reject
     * a bad destination after staging, without publishing cleared history. */
    world.creatureAI[0].reserved0 = 0;
    world.creatureAI[0].groupCells = 0x42; /* staged F0184 must not leak this */
    world.pc34ActiveGroupHistory[0].groupThingIndex = 0;
    squareFirstThings[0] = group_thing_ref(99);
    before = world;
    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_FAIL,
          "invalid destination owner rejects staged admission");
    CHECK(memcmp(&world, &before, sizeof(world)) == 0 &&
          memcmp(&groups[0], &groupBefore, sizeof(groupBefore)) == 0,
          "failed staged admission preserves world, history and C04");
    squareFirstThings[0] = group_thing_ref(0);
    world.creatureAICount = 0; /* successful entry starts from empty map 0 */

    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) ==
              ORCH_OK,
          "map transition tick succeeds");
    CHECK(world.partyMapIndex == 1 && world.party.mapIndex == 1,
          "map transition commits party map before F0195 scan");
    CHECK(world.creatureAICount == 1,
          "F0195 adds current-map group after empty-map retirement");
    /* I34 Skeleton GraphicInfo=0x6038: X offset/sign, Y offset/sign,
     * then the deadline draw. BASE.C F0027 from seed 7 yields five draws. */
    CHECK(world.masterRng.seed == UINT32_C(3257846826) &&
          world.creatureAI[0].aspect[0] == 7 &&
          world.creatureAI[0].aspect[1] == 0,
          "F0183 publishes original aspect and exact admission RNG");
    CHECK(world.pc34ActiveGroupSourceCount >= 1 &&
          world.pc34ActiveGroupDirections[0] == 2 &&
          world.pc34ActiveGroupHomeMapX[0] == 1 &&
          world.pc34ActiveGroupHomeMapY[0] == 1,
          "F0183 publishes directions and home ownership for reaction consumers");
    CHECK(world.pc34ActiveGroupHistory[0].valid &&
          world.pc34ActiveGroupHistory[0].groupThingIndex == 0 &&
          world.pc34ActiveGroupHistory[0].priorMapX == 1 &&
          world.pc34ActiveGroupHistory[0].priorMapY == 1 &&
          world.pc34ActiveGroupHistory[0].lastMoveTime == (unsigned char)(100u - 127u),
          "F0183 admission replaces stale history with source position and time");
    CHECK(world.creatureAI[0].reserved0 == 0 &&
          world.creatureAI[0].groupMapIndex == 1 &&
          world.creatureAI[0].groupMapX == 1 &&
          world.creatureAI[0].groupMapY == 1,
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
