/* ReDMCSB MOVESENS.C F0269/F0270 C10 local Steal XP regression. */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"

#include <assert.h>
#include <string.h>

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
    struct TickResult_Compat result;
    int64_t leaderBefore;
    int64_t otherBefore;

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
    firstThing = (unsigned short)((1u << 14) | (THING_TYPE_SENSOR << 10));
    sensor.next = THING_ENDOFLIST;
    sensor.sensorType = DM1_SENSOR_WALL_COUNTDOWN;
    sensor.sensorData = 1;
    sensor.effect = DM1_EFFECT_SET;
    sensor.localEffect = 1;
    sensor.localMultiple = DM1_EFFECT_ADD_300XP_STEAL_SKILL;
    things.loaded = 1;
    things.sensors = &sensor;
    things.sensorCount = 1;
    things.squareFirstThings = &firstThing;
    things.squareFirstThingCount = 1;

    memset(&world, 0, sizeof(world));
    world.dungeon = &dungeon;
    world.things = &things;
    assert(F0881_WORLD_InitDefault_Compat(&world, 0x6789u));
    world.dungeon = &dungeon;
    world.things = &things;
    world.party.championCount = 2;
    world.party.activeChampionIndex = 1;
    world.party.champions[0].present = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 100;
    leaderBefore = world.lifecycle.champions[1].skills20[DM1_SKILL_IDX_STEAL].experience;
    otherBefore = world.lifecycle.champions[0].skills20[DM1_SKILL_IDX_STEAL].experience;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_SQUARE_STATE;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.cell = 1;
    event.aux0 = DM1_EVENT_WALL;
    event.aux1 = DM1_EFFECT_CLEAR;
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event));
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);

    assert(sensor.sensorData == 0);
    assert(world.lifecycle.champions[1].skills20[DM1_SKILL_IDX_STEAL].experience >
           leaderBefore);
    assert(world.lifecycle.champions[0].skills20[DM1_SKILL_IDX_STEAL].experience ==
           otherBefore);
    assert(world.party.champions[1].skillExperience[DM1_SKILL_IDX_NINJA] ==
           (unsigned long)world.lifecycle.champions[1]
               .skills20[DM1_SKILL_IDX_NINJA].experience);
    assert(result.emissionCount == 1);
    assert(result.emissions[0].kind == EMIT_XP_AWARD);
    assert(result.emissions[0].payload[0] == 1);
    assert(result.emissions[0].payload[1] == DM1_SKILL_IDX_STEAL);
    assert(result.emissions[0].payload[2] == 300);
    return 0;
}
