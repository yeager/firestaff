/* G0383: F0209 derives the secondary movement direction from each C37. */

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

static int run_diagonal_c37(int source_x, int source_y,
                            int party_x, int party_y,
                            int blocked_x, int blocked_y,
                            int expected_secondary,
                            unsigned int rng_seed)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[2];
    struct TimelineEvent_Compat event;
    struct TickResult_Compat result;
    struct RngState_Compat direction_rng;
    unsigned char squares[25];
    unsigned char raw_group[32];
    unsigned short square_first_things[25];
    unsigned short columns[5];
    int primary = -1;
    int secondary = -1;
    int event_index;

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(squares, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squares));
    memset(raw_group, 0, sizeof(raw_group));
    memset(square_first_things, 0xff, sizeof(square_first_things));
    memset(columns, 0, sizeof(columns));
    CHECK(F0881_WORLD_InitDefault_Compat(&world, 0x0383u),
          "initialize timeline world");

    map.width = 5;
    map.height = 5;
    map.difficulty = 0;
    /* Primary is occupied by a real C04 group. The source group also owns the
     * C37 square, as required by F0209's loaded C04/SFT admission. */
    squares[blocked_x * map.height + blocked_y] |= DUNGEON_SQUARE_MASK_THING_LIST;
    squares[source_x * map.height + source_y] |= DUNGEON_SQUARE_MASK_THING_LIST;
    for (int x = 0, count = 0; x < map.width; ++x) {
        columns[x] = (unsigned short)count;
        for (int y = 0; y < map.height; ++y) {
            if (squares[x * map.height + y] & DUNGEON_SQUARE_MASK_THING_LIST) {
                square_first_things[count++] =
                    (x == source_x && y == source_y) ? group_thing_ref(0) : group_thing_ref(1);
            }
        }
    }
    tiles.squareData = squares;
    tiles.squareCount = (int)sizeof(squares);
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
    dungeon.dungeonColumnCount = map.width;

    /* C04 and persistent active-group data remain equal across both routes. */
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = DM1_CREATURE_TYPE_STONE_GOLEM;
    groups[0].count = 0;
    groups[0].cells = 1;
    groups[0].direction = 0;
    groups[0].behavior = DM1_BEHAVIOR_APPROACH;
    groups[0].health[0] = 100;
    groups[1].next = THING_ENDOFLIST;
    groups[1].creatureType = DM1_CREATURE_TYPE_STONE_GOLEM;
    groups[1].count = 0;
    groups[1].cells = 1;
    groups[1].health[0] = 100;
    things.groups = groups;
    things.groupCount = 2;
    things.thingCounts[THING_TYPE_GROUP] = 2;
    things.squareFirstThings = square_first_things;
    things.squareFirstThingCount = (int)(sizeof(square_first_things) /
                                         sizeof(square_first_things[0]));
    things.loaded = 1;
    authenticate_group_c04(&things, &groups[0], raw_group);

    world.dungeon = &dungeon;
    world.things = &things;
    world.gameTick = 100u;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = party_x;
    world.party.mapY = party_y;
    world.party.championCount = 1;
    world.party.champions[0].present = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;
    world.creatureAICount = 1;
    world.creatureAI[0].reserved0 = 0;
    world.creatureAI[0].stateKind = AI_STATE_APPROACH;
    world.creatureAI[0].creatureType = groups[0].creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = source_x;
    world.creatureAI[0].groupMapY = source_y;
    world.creatureAI[0].groupCells = groups[0].cells;
    world.pc34ActiveGroupSourceCount = 1;
    world.pc34ActiveGroupDirections[0] = groups[0].direction;
    CHECK(F0730_COMBAT_RngInit_Compat(&world.masterRng, rng_seed),
          "initialize live F0209 RNG");
    CHECK(F0730_COMBAT_RngInit_Compat(&direction_rng, rng_seed),
          "initialize source direction RNG");
    CHECK(F0228_DM1_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource_Compat(
              source_x, source_y, party_x, party_y, &direction_rng,
              &primary, &secondary),
          "derive source F0228 diagonal directions");
    CHECK(primary != expected_secondary && secondary == expected_secondary,
          "fixture admits only the source-proven secondary direction");

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = source_x;
    event.mapY = source_y;
    event.aux0 = 0;
    event.aux1 = groups[0].creatureType;
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    CHECK(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event),
          "schedule diagonal C37 event");
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1,
          "diagonal C37 reaches F0209 through public dispatch");

    if (groups[0].direction != expected_secondary ||
        world.creatureAI[0].groupDirection != expected_secondary ||
        (raw_group[15] & 0x03u) != (unsigned int)expected_secondary) {
        fprintf(stderr, "FAIL: expected %d, C04 %d, AI %d, raw %u\\n",
                expected_secondary, groups[0].direction,
                world.creatureAI[0].groupDirection,
                (unsigned int)(raw_group[15] & 0x03u));
        return 0;
    }
    for (event_index = 0; event_index < world.timeline.count; ++event_index) {
        CHECK(world.timeline.events[event_index].kind != TIMELINE_EVENT_PROJECTILE_MOVE,
              "G0383 movement route has no projectile aftermath");
    }
    return 1;
}

int main(void)
{
    /* Diagonal C37, seed 1: north then east. Occupy north, move east. */
    if (!run_diagonal_c37(2, 3, 3, 1, 2, 2, 1, 1u)) return 1;
    /* Same source geometry with the other original F0228 tie order: east
     * then north. Occupy east, so F0209 moves north. */
    if (!run_diagonal_c37(2, 3, 3, 1, 3, 3, 0, 3u)) return 1;

    puts("PASS: G0383 F0209 per-event secondary direction context");
    return 0;
}
