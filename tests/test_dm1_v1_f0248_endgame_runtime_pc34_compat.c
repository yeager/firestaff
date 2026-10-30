/* ReDMCSB TIMELINE.C F0248 C018 end-game M10 regression. */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void test_wall_countdown_c10_awards_active_leader(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char square;
    struct DungeonThings_Compat things;
    struct DungeonSensor_Compat sensor;
    unsigned short firstThing;
    struct GameWorld_Compat world;
    struct TimelineEvent_Compat event;
    struct TickResult_Compat result;
    unsigned long leaderBefore;
    unsigned long otherBefore;
    int foundXpAward = 0;
    int i;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    square = (unsigned char)((DUNGEON_ELEMENT_WALL << 5) |
                             DUNGEON_SQUARE_MASK_THING_LIST);
    map.width = 1;
    map.height = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    tiles.squareData = &square;

    memset(&things, 0, sizeof(things));
    memset(&sensor, 0, sizeof(sensor));
    firstThing = (unsigned short)((2u << 14) | (THING_TYPE_SENSOR << 10));
    sensor.next = THING_ENDOFLIST;
    sensor.sensorType = DM1_SENSOR_WALL_COUNTDOWN;
    sensor.sensorData = 1;
    sensor.effect = DM1_EFFECT_SET;
    sensor.localEffect = 1;
    sensor.localMultiple = DM1_EFFECT_ADD_300XP_STEAL_SKILL;
    sensor.onceOnly = 1;
    things.loaded = 1;
    things.sensors = &sensor;
    things.sensorCount = 1;
    things.thingCounts[THING_TYPE_SENSOR] = 1;
    things.squareFirstThings = &firstThing;
    things.squareFirstThingCount = 1;

    memset(&world, 0, sizeof(world));
    world.dungeon = &dungeon;
    world.things = &things;
    assert(F0881_WORLD_InitDefault_Compat(&world, 0x9876u));
    world.dungeon = &dungeon;
    world.things = &things;
    world.party.championCount = 2;
    world.party.activeChampionIndex = 1;
    for (i = 0; i < world.party.championCount; ++i) {
        world.party.champions[i].present = 1;
        world.party.champions[i].hp.current = 100;
        world.party.champions[i].hp.maximum = 100;
        world.lifecycle.champions[i].skills20[DM1_SKILL_IDX_STEAL].experience =
            (unsigned long)(100 + i);
    }
    leaderBefore = world.lifecycle.champions[1]
        .skills20[DM1_SKILL_IDX_STEAL].experience;
    otherBefore = world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_STEAL].experience;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_SQUARE_STATE;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.cell = 1; /* F0248/F0272 pass the event cell to F0270. */
    event.aux0 = DM1_EVENT_WALL;
    event.aux1 = DM1_EFFECT_CLEAR;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event));

    memset(&result, 0, sizeof(result));
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) >= 0);
    assert(sensor.sensorType == DM1_SENSOR_DISABLED);
    assert(world.lifecycle.champions[1]
               .skills20[DM1_SKILL_IDX_STEAL].experience > leaderBefore);
    assert(world.lifecycle.champions[0]
               .skills20[DM1_SKILL_IDX_STEAL].experience == otherBefore);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_XP_AWARD &&
            result.emissions[i].payload[0] == 1 &&
            result.emissions[i].payload[1] == DM1_SKILL_IDX_STEAL &&
            result.emissions[i].payload[2] == 300) {
            foundXpAward = 1;
            break;
        }
    }
    assert(foundXpAward);
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char square;
    struct DungeonThings_Compat things;
    struct DungeonSensor_Compat sensor;
    unsigned short firstThing;
    struct GameWorld_Compat world;
    struct TimelineEvent_Compat event;
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    square = (unsigned char)((DUNGEON_ELEMENT_WALL << 5) |
                             DUNGEON_SQUARE_MASK_THING_LIST);
    map.width = 1;
    map.height = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    tiles.squareData = &square;

    memset(&things, 0, sizeof(things));
    memset(&sensor, 0, sizeof(sensor));
    firstThing = (unsigned short)((2u << 14) | (THING_TYPE_SENSOR << 10));
    sensor.next = THING_ENDOFLIST;
    sensor.sensorType = DM1_SENSOR_WALL_END_GAME;
    sensor.value = 3; /* Optional F0248 delay must not defer gameWon. */
    things.loaded = 1;
    things.sensors = &sensor;
    things.sensorCount = 1;
    things.squareFirstThings = &firstThing;
    things.squareFirstThingCount = 1;

    memset(&world, 0, sizeof(world));
    world.dungeon = &dungeon;
    world.things = &things;
    assert(F0881_WORLD_InitDefault_Compat(&world, 0x3456u));
    world.dungeon = &dungeon;
    world.things = &things;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_SQUARE_STATE;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.cell = 1; /* Deliberately different from C018's Thing cell. */
    event.aux0 = DM1_EVENT_WALL;
    event.aux1 = DM1_EFFECT_CLEAR;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event));

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) ==
           ORCH_GAME_WON);
    assert(world.gameWon == 1);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_GAME_WON);
    test_wall_countdown_c10_awards_active_leader();
    return 0;
}
