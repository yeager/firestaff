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
    event.aux1 = world->things->groups[0].creatureType;
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

static unsigned short group_thing_ref(int index)
{
    return (unsigned short)((THING_TYPE_GROUP << 10) | index);
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
    unsigned short square_first_things[2];
    unsigned short columns[4] = { 0, 0, 1, 1 };

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
    /* The two legitimate physical C04 locations retain their compact SFT
     * slots while the fixture advances the group between source events. */
    squares[1 * map.height + 1] |= DUNGEON_SQUARE_MASK_THING_LIST;
    squares[3 * map.height + 3] |= DUNGEON_SQUARE_MASK_THING_LIST;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
    dungeon.dungeonColumnCount = map.width;

    group.next = THING_ENDOFLIST;
    group.creatureType = 9;
    group.count = 0;
    group.cells = 1;
    group.direction = 0; /* North: deliberately constant for both events. */
    group.behavior = DM1_BEHAVIOR_WANDER;
    group.health[0] = 100;
    things.groups = &group;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    square_first_things[0] = group_thing_ref(0);
    square_first_things[1] = THING_ENDOFLIST;
    things.squareFirstThings = square_first_things;
    things.squareFirstThingCount = 2;
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
    world.pc34ActiveGroupSourceCount = 1;
    world.pc34ActiveGroupDirections[0] = group.direction;
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

    /* Move the physical C04 owner into the next loaded SFT slot. The event
     * coordinates and active-group location advance together, as in F0267. */
    square_first_things[0] = THING_ENDOFLIST;
    square_first_things[1] = group_thing_ref(0);
    world.creatureAI[0].groupMapX = 3;
    world.creatureAI[0].groupMapY = 3;
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
