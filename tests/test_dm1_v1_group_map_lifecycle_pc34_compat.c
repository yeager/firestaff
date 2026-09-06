#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

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
    struct DungeonGroup_Compat groups[2];
    unsigned char map0Squares[1];
    unsigned char map1Squares[4];
    unsigned short squareFirstThings[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    static struct GameWorld_Compat before;
    struct DungeonGroup_Compat groupBefore;
    unsigned short columns[3] = {0, 0, 0};
    unsigned char rawGroups[32];
    unsigned char rawBefore[32];

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(rawGroups, 0xa5, sizeof(rawGroups));
    memcpy(rawBefore, rawGroups, sizeof(rawGroups));
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
    groups[0].behavior = DM1_BEHAVIOR_USELESS4;
    squareFirstThings[0] = group_thing_ref(0);
    things.groups = groups;
    things.groupCount = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.loaded = 1;
    things.rawThingData[THING_TYPE_GROUP] = rawGroups;
    things.thingCounts[THING_TYPE_GROUP] = 2;
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
    CHECK(memcmp(rawGroups, rawBefore, sizeof(rawGroups)) == 0,
          "failed transition does not publish decoded C04 changes to raw storage");
    squareFirstThings[0] = group_thing_ref(0);
    world.creatureAICount = 0; /* successful entry starts from empty map 0 */

    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) ==
              ORCH_OK,
          "map transition tick succeeds");
    CHECK(world.partyMapIndex == 1 && world.party.mapIndex == 1,
          "map transition commits party map before F0195 scan");
    CHECK(groups[0].behavior == DM1_BEHAVIOR_WANDER &&
          (rawGroups[14] & 0x0f) == DM1_BEHAVIOR_WANDER,
          "F0180 resets nonpersistent behavior in decoded and raw C04");
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
          world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_REACTION &&
          world.timeline.events[0].aux2 == DM1_EVENT_UPDATE_BEHAVIOR_GROUP &&
          world.timeline.events[0].aux3 == 0 &&
          (world.timeline.events[0].aux4 & 0x100) != 0 &&
          world.timeline.events[0].aux0 == 0 &&
          world.timeline.events[0].mapIndex == 1 &&
          world.timeline.events[0].fireAtTick == 101u,
          "F0195 restarts C37 wandering at game time plus one");

    /* GROUP.C F0195 scans columns, not C04 indices. Admit group 1 first,
     * then fail on the next column after its F0179 draws and queued event. */
    world = before;
    world.creatureAICount = 0;
    groups[1] = groups[0];
    groups[1].creatureType = 6; /* I34 Rockpile GraphicInfo=0x0020: one draw */
    groups[1].behavior = DM1_BEHAVIOR_USELESS4;
    things.groupCount = 2;
    things.squareFirstThingCount = 2;
    map1Squares[1] |= DUNGEON_SQUARE_MASK_THING_LIST;
    columns[2] = 1;
    squareFirstThings[0] = group_thing_ref(1);
    squareFirstThings[1] = group_thing_ref(99);
    before = world;
    groupBefore = groups[0];
    memcpy(rawBefore, rawGroups, sizeof(rawGroups));
    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_FAIL,
          "second-column failure rejects partial admission");
    CHECK(memcmp(&world, &before, sizeof(world)) == 0 &&
          memcmp(&groups[0], &groupBefore, sizeof(groupBefore)) == 0 &&
          groups[1].cells == groupBefore.cells &&
          groups[1].behavior == DM1_BEHAVIOR_USELESS4,
          "partial admission publishes no RNG, aspects, events or C04 changes");
    CHECK(memcmp(rawGroups, rawBefore, sizeof(rawGroups)) == 0,
          "partial admission leaves shared raw C04 storage unchanged");
    squareFirstThings[1] = group_thing_ref(0);
    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
          "two-column admission succeeds after correcting second owner");
    CHECK(world.creatureAICount == 2 &&
          world.creatureAI[0].reserved0 == 1 &&
          world.creatureAI[1].reserved0 == 0 &&
          world.creatureAI[0].groupMapX == 0 &&
          world.creatureAI[1].groupMapX == 1,
          "admission follows column order rather than group table order");
    CHECK(world.masterRng.seed == UINT32_C(3861356397) &&
          world.creatureAI[0].aspect[0] == 0 &&
          world.creatureAI[1].aspect[0] == 1 &&
          world.timeline.count == 2,
          "column-ordered original metadata consumes six draws with exact aspects");

    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM1 F0194/F0195 active-group map lifecycle");
    return 0;
}
