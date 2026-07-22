/* G0381: F0209 derives current group distance from each timeline event. */

#include <stdio.h>
#include <string.h>

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#define CHECK(condition, label) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", label); \
        return 1; \
    } \
} while (0)

static int schedule_c37(struct GameWorld_Compat* world, int map_x, int map_y)
{
    struct TimelineEvent_Compat event;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = world->gameTick;
    event.mapIndex = 0;
    event.mapX = map_x;
    event.mapY = map_y;
    event.aux0 = 0;
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &event);
}

static void authenticate_group_c04(struct DungeonThings_Compat* things,
                                   const struct DungeonGroup_Compat* group,
                                   unsigned char raw_group[16])
{
    unsigned short packed;

    memset(raw_group, 0, 16);
    packed = (unsigned short)((group->behavior & 0x0fu) |
                              ((group->count & 0x03u) << 5) |
                              ((group->direction & 0x03u) << 8));
    raw_group[0] = (unsigned char)(group->next & 0xffu);
    raw_group[1] = (unsigned char)(group->next >> 8);
    raw_group[4] = group->creatureType;
    raw_group[5] = group->cells;
    raw_group[6] = (unsigned char)(group->health[0] & 0xffu);
    raw_group[7] = (unsigned char)(group->health[0] >> 8);
    raw_group[14] = (unsigned char)(packed & 0xffu);
    raw_group[15] = (unsigned char)(packed >> 8);
    things->rawThingData[THING_TYPE_GROUP] = raw_group;
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat group;
    struct TickResult_Compat result;
    unsigned char squares[16];
    unsigned char raw_group[16];

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(&group, 0, sizeof(group));
    memset(squares, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squares));
    CHECK(F0881_WORLD_InitDefault_Compat(&world, 0x0381u),
          "initialize timeline world");

    map.width = 4;
    map.height = 4;
    map.difficulty = 0; /* Preserve the source bright-map sight range. */
    tiles.squareData = squares;
    tiles.squareCount = (int)sizeof(squares);
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;

    group.next = THING_ENDOFLIST;
    group.creatureType = 9;
    group.count = 0;
    group.cells = 0xff;
    group.direction = 0; /* North for both C37 events. */
    group.behavior = DM1_BEHAVIOR_WANDER;
    group.health[0] = 100;
    things.groups = &group;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.loaded = 1;
    authenticate_group_c04(&things, &group, raw_group);

    world.dungeon = &dungeon;
    world.things = &things;
    world.gameTick = 10u;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 3;
    world.party.mapY = 1;
    world.party.championCount = 1;
    world.party.champions[0].present = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;
    world.creatureAICount = 1;
    world.creatureAI[0].reserved0 = 0;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = group.creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 3;
    world.creatureAI[0].groupMapY = 3;
    world.creatureAI[0].groupCells = group.cells;
    CHECK(F0730_COMBAT_RngInit_Compat(&world.masterRng, 1u),
          "initialize deterministic creature RNG");

    /* The party is two squares north of this event source. G0381 must make
     * C37 enter approach, not attack, with the unchanged north-facing C04. */
    CHECK(schedule_c37(&world, 3, 3), "schedule distance-two C37");
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1,
          "distance-two C37 reaches F0209 through public dispatch");
    CHECK(group.behavior == DM1_BEHAVIOR_APPROACH &&
          world.creatureAI[0].stateKind == AI_STATE_APPROACH,
          "G0381 distance two selects approach");
    CHECK(group.direction == 0 && (raw_group[15] & 0x03u) == 0,
          "distance-two event preserves C04 facing");

    /* C04 and all static group fields stay fixed. The source advances one
     * square, so the same party is now at G0381 distance one. */
    CHECK(schedule_c37(&world, 3, 2), "schedule distance-one C37");
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1,
          "distance-one C37 reaches F0209 through public dispatch");
    CHECK(group.behavior == DM1_BEHAVIOR_ATTACK &&
          world.creatureAI[0].stateKind == AI_STATE_ATTACK,
          "G0381 distance one selects attack");
    CHECK(group.direction == 0 && (raw_group[15] & 0x03u) == 0,
          "distance-one event preserves the same C04 facing");

    puts("PASS: G0381 F0209 per-event distance context");
    return 0;
}
