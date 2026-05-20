#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"

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

    F0883_WORLD_Free_Compat(&world);
    if (!ok) return 1;
    puts("M10_C006_GENERATOR_REENABLE_DISPATCH_OK");
    return 0;
}
