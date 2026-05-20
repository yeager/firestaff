#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_sound_pc34_compat.h"

static int build_world(struct GameWorld_Compat* world) {
    struct DungeonDatState_Compat* dungeon;
    struct DungeonThings_Compat* things;
    int i;

    memset(world, 0, sizeof(*world));
    if (!F0881_WORLD_InitDefault_Compat(world, 0xC006u)) return 0;

    dungeon = (struct DungeonDatState_Compat*)calloc(1, sizeof(*dungeon));
    things = (struct DungeonThings_Compat*)calloc(1, sizeof(*things));
    if (!dungeon || !things) {
        free(dungeon);
        free(things);
        return 0;
    }

    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    dungeon->header.mapCount = 1;
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(*dungeon->maps));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(*dungeon->tiles));
    if (!dungeon->maps || !dungeon->tiles) goto fail;

    dungeon->maps[0].width = 3;
    dungeon->maps[0].height = 3;
    dungeon->tiles[0].squareCount = 9;
    dungeon->tiles[0].squareData = (unsigned char*)calloc(9, sizeof(unsigned char));
    if (!dungeon->tiles[0].squareData) goto fail;

    for (i = 0; i < 9; ++i) {
        dungeon->tiles[0].squareData[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
    dungeon->tiles[0].squareData[(1 * 3) + 1] |= DUNGEON_SQUARE_MASK_THING_LIST;

    things->loaded = 1;
    things->squareFirstThingCount = 1;
    things->squareFirstThings = (unsigned short*)calloc(1, sizeof(unsigned short));
    things->sensorCount = 3;
    things->thingCounts[THING_TYPE_SENSOR] = 3;
    things->sensors = (struct DungeonSensor_Compat*)calloc(3, sizeof(*things->sensors));
    if (!things->squareFirstThings || !things->sensors) goto fail;

    things->squareFirstThings[0] = (unsigned short)((THING_TYPE_SENSOR << 10) | 0);
    things->sensors[0].sensorType = 3;
    things->sensors[0].next = (unsigned short)((THING_TYPE_SENSOR << 10) | 1);
    things->sensors[1].sensorType = RUNTIME_SENSOR_TYPE_DISABLED;
    things->sensors[1].sensorData = 7;
    things->sensors[1].next = (unsigned short)((THING_TYPE_SENSOR << 10) | 2);
    things->sensors[2].sensorType = RUNTIME_SENSOR_TYPE_DISABLED;
    things->sensors[2].sensorData = 9;
    things->sensors[2].next = THING_ENDOFLIST;

    world->dungeon = dungeon;
    world->things = things;
    world->ownsDungeon = 1;
    world->party.mapIndex = 0;
    world->partyMapIndex = 0;
    return 1;

fail:
    if (dungeon) {
        if (dungeon->tiles) free(dungeon->tiles[0].squareData);
        free(dungeon->maps);
        free(dungeon->tiles);
    }
    if (things) {
        free(things->squareFirstThings);
        free(things->sensors);
    }
    free(dungeon);
    free(things);
    return 0;
}

static int expect(int cond, const char* label) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", label);
        return 0;
    }
    return 1;
}

int main(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    int ok = 1;
    int rc;
    unsigned int generatorTriggerTick;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world\n");
        return 1;
    }

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_GROUP_GENERATOR;
    event.fireAtTick = 0;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = GENERATOR_EVENT_AUX0_REENABLE;

    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule C65 generator re-enable event");
    rc = F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result);
    ok &= expect(rc == ORCH_OK, "advance one tick");
    ok &= expect(world.things->sensors[0].sensorType == 3,
                 "armed sensor before disabled sensor is unchanged");
    ok &= expect(world.things->sensors[1].sensorType == RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR,
                 "first disabled sensor is re-enabled as C006");
    ok &= expect(world.things->sensors[1].sensorData == 7,
                 "re-enabled sensor preserves payload data");
    ok &= expect(world.things->sensors[2].sensorType == RUNTIME_SENSOR_TYPE_DISABLED,
                 "second disabled sensor remains disabled");
    ok &= expect(world.timeline.count == 0, "re-enable event is consumed");

    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    world.things->sensors[0].sensorType = RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR;
    world.things->sensors[0].sensorData = 0;
    world.things->sensors[0].value = 1;
    world.things->sensors[0].audible = 1;
    world.things->sensors[0].onceOnly = 0;
    world.things->sensors[0].localMultiple = (unsigned short)((2u << 4) | 1u);
    event.kind = TIMELINE_EVENT_GROUP_GENERATOR;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = GENERATOR_EVENT_AUX0_TRIGGER;

    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule C006 generator trigger event");
    generatorTriggerTick = world.gameTick;
    rc = F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result);
    ok &= expect(rc == ORCH_OK, "advance C006 generator trigger tick");
    ok &= expect(result.emissionCount == 2,
                 "audible C006 generator emits F0185 buzz plus sensor-audible buzz");
    ok &= expect(result.emissions[0].kind == EMIT_SOUND_REQUEST &&
                 result.emissions[1].kind == EMIT_SOUND_REQUEST,
                 "audible C006 generator emits sound request kinds");
    ok &= expect(result.emissions[0].payload[0] == DM1_SND_BUZZ &&
                 result.emissions[1].payload[0] == DM1_SND_BUZZ,
                 "audible C006 generator requests M560_SOUND_BUZZ twice");
    ok &= expect(result.emissions[0].payload[1] == 1 && result.emissions[0].payload[2] == 1 &&
                 result.emissions[1].payload[1] == 1 && result.emissions[1].payload[2] == 1,
                 "audible C006 generator sounds keep source square coordinates");
    ok &= expect(world.things->groupCount == 1,
                 "C006 generator materializes one group slot");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_GROUP << 10) | 0),
                 "C006 generator links group at square head");
    ok &= expect(world.things->groups[0].next == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "C006 generated group preserves prior sensor chain as next thing");
    ok &= expect(world.things->groups[0].creatureType == 0 &&
                 world.things->groups[0].count == 0 &&
                 world.things->groups[0].cells == RUNTIME_GROUP_CELLS_SINGLE_CENTERED,
                 "C006 generated single creature group has source fields");
    ok &= expect(world.things->groups[0].health[0] > 0 &&
                 world.things->groups[0].health[1] == 0 &&
                 world.things->groups[0].health[2] == 0 &&
                 world.things->groups[0].health[3] == 0,
                 "C006 generated group writes HP only for live creature slots");
    ok &= expect(world.creatureAICount == 1 &&
                 world.creatureAI[0].stateKind == AI_STATE_WANDER &&
                 world.creatureAI[0].groupMapX == 1 &&
                 world.creatureAI[0].groupMapY == 1,
                 "C006 generated party-map group seeds active group state");
    ok &= expect(world.things->sensors[0].sensorType == RUNTIME_SENSOR_TYPE_DISABLED,
                 "C006 generator disables sensor while re-enable delay is pending");
    ok &= expect(world.timeline.count == 2,
                 "C006 generator schedules C37 wander and C65 re-enable events");
    ok &= expect(world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_TICK &&
                 world.timeline.events[0].fireAtTick == generatorTriggerTick + 1u &&
                 world.timeline.events[0].mapIndex == 0 &&
                 world.timeline.events[0].mapX == 1 &&
                 world.timeline.events[0].mapY == 1 &&
                 world.timeline.events[0].aux0 == 0 &&
                 world.timeline.events[0].aux1 == 0 &&
                 world.timeline.events[0].aux2 == AI_STATE_WANDER,
                 "C006 generated group schedules source C37 wander at game time +1");
    ok &= expect(world.timeline.events[1].kind == TIMELINE_EVENT_GROUP_GENERATOR &&
                 world.timeline.events[1].aux0 == GENERATOR_EVENT_AUX0_REENABLE,
                 "C006 generator keeps delayed C65 re-enable event after wander event");

    F0883_WORLD_Free_Compat(&world);
    if (!ok) return 1;
    puts("M10_C006_GENERATOR_REENABLE_AND_AUDIO_DISPATCH_OK");
    return 0;
}
