#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, label) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", label); \
        return 1; \
    } \
} while (0)

static unsigned short thing_ref(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static struct TimelineEvent_Compat reaction(
    int mapIndex, int mapX, int mapY, int eventType)
{
    struct TimelineEvent_Compat event;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.mapIndex = mapIndex;
    event.mapX = mapX;
    event.mapY = mapY;
    event.aux2 = eventType;
    return event;
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonSensor_Compat sensor;
    struct DungeonGroup_Compat groups[2];
    struct DungeonGroup_Compat capGroups[61];
    unsigned char squares[4] = { 0x10, 0x00, 0x10, 0x00 };
    unsigned char capSquares[61];
    unsigned short sft[2];
    unsigned short capSft[61];
    int index;

    /* GROUP.C F0196 (PC 3.4): a new world owns exactly 110 initialized
     * ACTIVE_GROUP records. M10 maps GroupThingIndex to reserved0 and must
     * leave its four non-source spare records untouched. */
    memset(&world, 0x5a, sizeof(world));
    world.creatureAICount = 17;
    CHECK(F0196_DM1_GROUP_InitializeActiveGroups_Compat(&world),
          "F0196 initializes the PC3.4 active-group storage");
    CHECK(world.creatureAICount == 0,
          "F0196 clears the active-group count");
    for (index = 0; index < DM1_PC34_ACTIVE_GROUP_CAPACITY; ++index) {
        CHECK(world.creatureAI[index].reserved0 == -1 &&
                  world.creatureAI[index].groupMapIndex == 0,
              "F0196 clears a source slot and writes its unused sentinel");
    }
    CHECK(world.creatureAI[DM1_PC34_ACTIVE_GROUP_CAPACITY].reserved0 ==
              (int)0x5a5a5a5a,
          "F0196 leaves non-source spare storage untouched");

    CHECK(F0881_WORLD_InitDefault_Compat(&world, 0x196u),
          "world initialization consumes F0196");
    CHECK(world.creatureAI[0].reserved0 == -1 &&
              world.creatureAI[DM1_PC34_ACTIVE_GROUP_CAPACITY - 1].reserved0 == -1,
          "world initialization retains all sixty F0196 unused sentinels");

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(&sensor, 0, sizeof(sensor));
    memset(groups, 0, sizeof(groups));

    map.width = 2;
    map.height = 2;
    tiles.squareData = squares;
    tiles.squareCount = 4;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;

    sensor.next = thing_ref(THING_TYPE_GROUP, 0);
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 6;
    groups[0].cells = 0xe4;
    groups[0].count = 1;
    groups[0].direction = 2;
    groups[1].next = THING_ENDOFLIST;
    groups[1].creatureType = 12;
    groups[1].cells = 0xff;
    groups[1].count = 0;
    groups[1].direction = 1;
    sft[0] = thing_ref(THING_TYPE_SENSOR, 0);
    sft[1] = thing_ref(THING_TYPE_GROUP, 1);
    things.squareFirstThings = sft;
    things.squareFirstThingCount = 2;
    things.sensors = &sensor;
    things.sensorCount = 1;
    things.groups = groups;
    things.groupCount = 2;
    things.loaded = 1;

    world.dungeon = &dungeon;
    world.things = &things;
    world.partyMapIndex = 0;
    world.party.mapIndex = 0;
    world.gameTick = 200;
    CHECK(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick),
          "timeline initialization");
    world.timeline.events[world.timeline.count++] = reaction(
        0, 0, 0, DM1_EVENT_REACTION_DANGER_ON_SQUARE);
    world.timeline.events[world.timeline.count++] = reaction(
        0, 0, 0, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3 + 1);

    CHECK(F0195_DM1_GROUP_AddAllActiveGroups_Compat(&world) == 2,
          "F0195 admits the first C04 from each current-map chain");
    CHECK(world.creatureAICount == 2,
          "F0195 adds two active C04 states");
    CHECK(world.creatureAI[0].reserved0 == 0 &&
              world.creatureAI[0].groupMapX == 0 &&
              world.creatureAI[0].groupMapY == 0 &&
              world.creatureAI[0].groupCells == 0xe4 &&
              world.creatureAI[0].groupDirection == 0x0a,
          "F0195 preserves first-chain C04 cells and packed directions");
    CHECK(world.creatureAI[1].reserved0 == 1 &&
              world.creatureAI[1].groupMapX == 1 &&
              world.creatureAI[1].groupMapY == 0 &&
              world.creatureAI[1].groupCells == 0xff &&
              world.creatureAI[1].groupDirection == 0x01,
          "F0195 follows the column-major SFT order");
    CHECK(world.timeline.count == 3 &&
              world.timeline.events[0].aux2 ==
                  DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3 + 1,
          "F0195 deletes only C29 through C41 before starting C37");
    CHECK(world.timeline.events[1].kind == TIMELINE_EVENT_CREATURE_TICK &&
              world.timeline.events[1].fireAtTick == 201 &&
              world.timeline.events[1].mapX == 0 &&
              world.timeline.events[1].mapY == 0 &&
              world.timeline.events[1].aux0 == 0 &&
              world.timeline.events[1].aux2 == AI_STATE_WANDER,
          "F0195 starts the first group wandering at GameTime plus one");
    CHECK(world.timeline.events[2].kind == TIMELINE_EVENT_CREATURE_TICK &&
              world.timeline.events[2].mapX == 1 &&
              world.timeline.events[2].mapY == 0 &&
              world.timeline.events[2].aux0 == 1,
          "F0195 starts the second group wandering from its raw C04 square");

    /* PC3.4 F0196 owns 110 ACTIVE_GROUP slots.  This is distinct from the
     * 60-slot Atari branch in GROUP.C. F0195 must admit every C04 here. */
    memset(&world, 0, sizeof(world));
    memset(capGroups, 0, sizeof(capGroups));
    memset(capSquares, 0x10, sizeof(capSquares));
    for (index = 0; index < 61; ++index) {
        capGroups[index].next = THING_ENDOFLIST;
        capGroups[index].creatureType = 6;
        capGroups[index].cells = 0xff;
        capGroups[index].direction = 3;
        capSft[index] = thing_ref(THING_TYPE_GROUP, index);
    }
    map.width = 61;
    map.height = 1;
    tiles.squareData = capSquares;
    tiles.squareCount = 61;
    things.squareFirstThings = capSft;
    things.squareFirstThingCount = 61;
    things.sensors = NULL;
    things.sensorCount = 0;
    things.groups = capGroups;
    things.groupCount = 61;
    world.dungeon = &dungeon;
    world.things = &things;
    world.partyMapIndex = 0;
    CHECK(F0720_TIMELINE_Init_Compat(&world.timeline, 0),
          "capacity timeline initialization");
    CHECK(F0195_DM1_GROUP_AddAllActiveGroups_Compat(&world) == 61 &&
              world.creatureAICount == 61 &&
              world.timeline.count == 61,
          "F0195 admits every PC3.4 C04 below the 110-slot capacity");

    puts("PASS: DM1 F0195 source-locked active-group map activation");
    return 0;
}
