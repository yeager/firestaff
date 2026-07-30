/* G0382: F0209 derives the primary party direction from each C37 source. */

#include <stdio.h>
#include <string.h>

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#define CHECK(condition, label) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", label); \
        return 0; \
    } \
} while (0)

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

static int run_c37_attack_entry(int source_x, int source_y, int expected_primary)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat group;
    struct TimelineEvent_Compat event;
    struct TickResult_Compat result;
    unsigned char squares[16];
    unsigned char raw_group[16];
    unsigned short square_first_things[1];
    unsigned short columns[4];
    int index;
    int c38_count = 0;

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(&group, 0, sizeof(group));
    memset(squares, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squares));
    memset(columns, 0, sizeof(columns));
    CHECK(F0881_WORLD_InitDefault_Compat(&world, 0x0382u),
          "initialize timeline world");

    map.width = 4;
    map.height = 4;
    map.difficulty = 0;
    tiles.squareData = squares;
    tiles.squareCount = (int)sizeof(squares);
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    /* F0209 only consumes a live C04 that owns the event square. */
    squares[source_x * map.height + source_y] |= DUNGEON_SQUARE_MASK_THING_LIST;
    for (index = source_x + 1; index < map.width; ++index) columns[index] = 1;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
    dungeon.dungeonColumnCount = map.width;

    /* Wizard Eye has the source SIDE_ATTACK attribute: either adjacent C37
     * source can enter attack while its C04 starts identically north-facing. */
    group.next = THING_ENDOFLIST;
    group.creatureType = DM1_CREATURE_TYPE_WIZARD_EYE;
    group.count = 0;
    group.cells = 1;
    group.direction = 0;
    group.behavior = DM1_BEHAVIOR_WANDER;
    group.health[0] = 100;
    things.groups = &group;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    square_first_things[0] = group_thing_ref(0);
    things.squareFirstThings = square_first_things;
    things.squareFirstThingCount = 1;
    things.loaded = 1;
    authenticate_group_c04(&things, &group, raw_group);

    world.dungeon = &dungeon;
    world.things = &things;
    world.gameTick = 10u;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 2;
    world.party.mapY = 2;
    world.party.championCount = 1;
    world.party.champions[0].present = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;
    world.creatureAICount = 1;
    world.creatureAI[0].reserved0 = 0;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = group.creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    /* The event coordinates are the current physical C04 location. */
    world.creatureAI[0].groupMapX = source_x;
    world.creatureAI[0].groupMapY = source_y;
    world.creatureAI[0].groupCells = group.cells;
    world.pc34ActiveGroupSourceCount = 1;
    world.pc34ActiveGroupDirections[0] = group.direction;
    CHECK(F0730_COMBAT_RngInit_Compat(&world.masterRng, 1u),
          "initialize deterministic creature RNG");

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = source_x;
    event.mapY = source_y;
    event.aux0 = 0;
    event.aux1 = group.creatureType;
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    CHECK(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event),
          "schedule source C37 event");
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1,
          "C37 reaches F0209 through public dispatch");
    CHECK(group.behavior == DM1_BEHAVIOR_ATTACK &&
          world.creatureAI[0].stateKind == AI_STATE_ATTACK,
          "adjacent C37 enters attack");
    CHECK(group.direction == expected_primary &&
          (raw_group[15] & 0x03u) == (unsigned int)expected_primary,
          "G0382 source direction publishes to C04 facing");

    for (index = 0; index < world.timeline.count; ++index) {
        const struct TimelineEvent_Compat* scheduled = &world.timeline.events[index];

        CHECK(scheduled->kind != TIMELINE_EVENT_PROJECTILE_MOVE,
              "C37 direction regression has no projectile aftermath");
        if (scheduled->kind == TIMELINE_EVENT_CREATURE_REACTION &&
            scheduled->aux2 == DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0) {
            ++c38_count;
            CHECK(scheduled->fireAtTick == world.gameTick + 1u,
                  "attack entry schedules C38 on the next timeline tick");
        }
    }
    CHECK(c38_count == 1, "attack entry schedules one C38 reaction");
    return 1;
}

int main(void)
{
    /* Only the public C37 source varies; C04 and persistent group state match. */
    if (!run_c37_attack_entry(1, 2, 1)) {
        return 1;
    }
    if (!run_c37_attack_entry(2, 3, 0)) {
        return 1;
    }

    puts("PASS: G0382 F0209 per-event primary direction context");
    return 0;
}
