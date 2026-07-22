/* G0378/G0379 are F0209's per-event current-group map coordinates. */

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

static int schedule_aspect_event(
    struct GameWorld_Compat* world, int map_x, int map_y)
{
    struct TimelineEvent_Compat event;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = world->gameTick;
    event.mapIndex = 0;
    event.mapX = map_x;
    event.mapY = map_y;
    event.aux0 = 0;
    event.aux2 = DM1_EVENT_UPDATE_ASPECT_GROUP;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &event);
}

/* M10's F0209 route consults the original C04 record. */
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
    CHECK(F0881_WORLD_InitDefault_Compat(&world, 0x0378u),
          "initialize timeline world");

    map.width = 4;
    map.height = 4;
    map.difficulty = 0; /* Source bright-map sight path: no palette dimming. */
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
    group.direction = 0; /* North: deliberately constant for both events. */
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
    world.creatureAI[0].groupMapX = 1;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].groupCells = group.cells;
    CHECK(F0730_COMBAT_RngInit_Compat(&world.masterRng, 1u),
          "initialize deterministic creature RNG");

    /* The party is east of this event source, outside an unchanged north
     * C04 cone. F0209 must leave the source group wandering. */
    CHECK(schedule_aspect_event(&world, 1, 1),
          "schedule hidden-party aspect event");
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1,
          "hidden-party event reaches F0209 through public dispatch");
    CHECK(group.behavior == DM1_BEHAVIOR_WANDER &&
          world.creatureAI[0].stateKind == AI_STATE_WANDER,
          "first event X/Y keeps its north-facing group wandering");
    CHECK(group.direction == 0 && (raw_group[15] & 0x03u) == 0,
          "first event does not alter C04 facing");

    /* C04 and all group state above remain unchanged. Only the event source
     * moves to the party's north-facing column, which makes the same party
     * visible to the next F0209 context. */
    CHECK(schedule_aspect_event(&world, 3, 3),
          "schedule visible-party aspect event");
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1,
          "visible-party event reaches F0209 through public dispatch");
    CHECK(group.behavior == DM1_BEHAVIOR_APPROACH &&
          world.creatureAI[0].stateKind == AI_STATE_APPROACH,
          "second event X/Y constructs a fresh visible F0209 context");
    CHECK(group.direction == 0 && (raw_group[15] & 0x03u) == 0,
          "second event preserves the same C04 facing");

    puts("PASS: G0378/G0379 F0209 event-context lifetime");
    return 0;
}
