#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"

static unsigned short test_thing_ref(int type, int index) {
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static int expect(int condition, const char* label) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        return 0;
    }
    return 1;
}

/* The destination has a real thing-list flag, but its decoded table entry is
 * absent.  This is a record-capacity failure after GROUP.C F0209/F0202 has
 * selected the east LoS step, not a wall/door/party movement rejection. */
static int build_world(struct GameWorld_Compat* world) {
    struct DungeonDatState_Compat* dungeon;
    struct DungeonThings_Compat* things;
    int i;

    memset(world, 0, sizeof(*world));
    if (!F0881_WORLD_InitDefault_Compat(world, 0xF0190u)) return 0;

    dungeon = (struct DungeonDatState_Compat*)calloc(1, sizeof(*dungeon));
    things = (struct DungeonThings_Compat*)calloc(1, sizeof(*things));
    if (!dungeon || !things) goto fail;

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
        dungeon->tiles[0].squareData[i] =
            (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
    dungeon->tiles[0].squareData[4] |= DUNGEON_SQUARE_MASK_THING_LIST;
    dungeon->tiles[0].squareData[7] |= DUNGEON_SQUARE_MASK_THING_LIST;

    things->loaded = 1;
    things->squareFirstThingCount = 1;
    things->squareFirstThings = (unsigned short*)calloc(1, sizeof(unsigned short));
    things->sensorCount = 2;
    things->thingCounts[THING_TYPE_SENSOR] = 2;
    things->sensors = (struct DungeonSensor_Compat*)calloc(2, sizeof(*things->sensors));
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    things->groups = (struct DungeonGroup_Compat*)calloc(1, sizeof(*things->groups));
    if (!things->squareFirstThings || !things->sensors || !things->groups) goto fail;

    things->squareFirstThings[0] = test_thing_ref(THING_TYPE_SENSOR, 0);
    things->sensors[0].next = test_thing_ref(THING_TYPE_GROUP, 0);
    things->groups[0].next = test_thing_ref(THING_TYPE_SENSOR, 1);
    things->groups[0].direction = 3;
    things->groups[0].creatureType = 0;
    things->groups[0].count = 0;
    things->groups[0].health[0] = 40;
    things->sensors[1].next = THING_ENDOFLIST;

    world->dungeon = dungeon;
    world->things = things;
    world->ownsDungeon = 1;
    world->party.mapIndex = 0;
    world->party.mapX = 0;
    world->party.mapY = 0;
    world->partyMapIndex = 0;
    world->creatureAICount = 1;
    world->creatureAI[0].stateKind = AI_STATE_WANDER;
    world->creatureAI[0].creatureType = 0;
    world->creatureAI[0].groupMapIndex = 0;
    world->creatureAI[0].groupMapX = 1;
    world->creatureAI[0].groupMapY = 1;
    world->creatureAI[0].groupCells = 0;
    world->creatureAI[0].groupDirection = 1;
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
        free(things->groups);
    }
    free(dungeon);
    free(things);
    return 0;
}

int main(void) {
    struct GameWorld_Compat world;
    struct TimelineEvent_Compat event;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build world\n");
        return 1;
    }
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_TICK;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = 0;
    event.aux2 = AI_STATE_WANDER;
    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule F0209-selected ordinary group step");
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "destination record-capacity failure remains a completed tick");
    ok &= expect(world.things->squareFirstThings[0] ==
                     test_thing_ref(THING_TYPE_SENSOR, 0),
                 "rollback preserves original source head");
    ok &= expect(world.things->sensors[0].next ==
                     test_thing_ref(THING_TYPE_GROUP, 0),
                 "rollback restores group after its original predecessor");
    ok &= expect(world.things->groups[0].next ==
                     test_thing_ref(THING_TYPE_SENSOR, 1) &&
                     world.things->sensors[1].next == THING_ENDOFLIST,
                 "rollback restores exact group successor and tail");
    ok &= expect(world.things->groups[0].direction == 3,
                 "failed F0267 record mutation leaves raw group direction unchanged");
    ok &= expect(world.creatureAI[0].groupMapIndex == 0 &&
                     world.creatureAI[0].groupMapX == 1 &&
                     world.creatureAI[0].groupMapY == 1,
                 "failed F0267 record mutation leaves active-group location unchanged");

    F0883_WORLD_Free_Compat(&world);
    if (!ok) return 1;
    puts("DM1_V1_GROUP_LOS_MOVE_ROLLBACK_PC34_COMPAT_OK");
    return 0;
}
