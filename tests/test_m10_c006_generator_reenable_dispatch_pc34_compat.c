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
    dungeon->tiles[0].squareData[(2 * 3) + 1] |= DUNGEON_SQUARE_MASK_THING_LIST;

    things->loaded = 1;
    things->squareFirstThingCount = 2;
    things->squareFirstThings = (unsigned short*)calloc(2, sizeof(unsigned short));
    things->sensorCount = 3;
    things->thingCounts[THING_TYPE_SENSOR] = 3;
    things->sensors = (struct DungeonSensor_Compat*)calloc(3, sizeof(*things->sensors));
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    things->groups = (struct DungeonGroup_Compat*)calloc(1, sizeof(*things->groups));
    if (!things->squareFirstThings || !things->sensors || !things->groups) goto fail;

    things->squareFirstThings[0] = (unsigned short)((THING_TYPE_SENSOR << 10) | 0);
    things->squareFirstThings[1] = THING_ENDOFLIST;
    things->sensors[0].sensorType = 3;
    things->sensors[0].next = (unsigned short)((THING_TYPE_SENSOR << 10) | 1);
    things->sensors[1].sensorType = RUNTIME_SENSOR_TYPE_DISABLED;
    things->sensors[1].sensorData = 7;
    things->sensors[1].next = (unsigned short)((THING_TYPE_SENSOR << 10) | 2);
    things->sensors[2].sensorType = RUNTIME_SENSOR_TYPE_DISABLED;
    things->sensors[2].sensorData = 9;
    things->sensors[2].next = THING_ENDOFLIST;
    things->groups[0].next = THING_NONE;

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
        free(things->groups);
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

static int test_lord_chaos_adjacent_random_retry(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    struct DungeonGroup_Compat* group;
    int ok = 1;
    int rc;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world Lord Chaos retry\n");
        return 1;
    }

    if (!world.things->groups || world.things->groupCount < 1) {
        world.things->groups = (struct DungeonGroup_Compat*)calloc(1, sizeof(*world.things->groups));
        world.things->groupCount = 1;
        world.things->thingCounts[THING_TYPE_GROUP] = 1;
    }
    if (!world.things->groups) {
        F0883_WORLD_Free_Compat(&world);
        fprintf(stderr, "FAIL: allocate Lord Chaos group\n");
        return 1;
    }
    group = &world.things->groups[0];
    group->next = THING_ENDOFLIST;
    group->slot = THING_ENDOFLIST;
    group->creatureType = 23;
    group->cells = RUNTIME_GROUP_CELLS_SINGLE_CENTERED;
    group->health[0] = 10000;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    world.party.mapX = 1;
    world.party.mapY = 1;
    F0730_COMBAT_RngInit_Compat(&world.masterRng, 2u);

    event.kind = TIMELINE_EVENT_MOVE_GROUP_SILENT;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = 0;

    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule Lord Chaos blocked event60 retry");
    rc = F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result);
    ok &= expect(rc == ORCH_OK, "advance Lord Chaos blocked event60 retry");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "Lord Chaos retry leaves original blocked square chain untouched");
    ok &= expect(world.things->squareFirstThings[1] == (unsigned short)((THING_TYPE_GROUP << 10) | 0),
                 "Lord Chaos random adjacent retry links group to allowed east square");
    ok &= expect(world.things->groups[0].next == THING_ENDOFLIST,
                 "Lord Chaos random adjacent retry links onto empty adjacent chain");
    ok &= expect(world.creatureAICount == 1 &&
                 world.creatureAI[0].groupMapX == 2 &&
                 world.creatureAI[0].groupMapY == 1,
                 "Lord Chaos random adjacent retry seeds active state at adjacent square");
    ok &= expect(world.timeline.count == 1 &&
                 world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_TICK &&
                 world.timeline.events[0].mapX == 2 &&
                 world.timeline.events[0].mapY == 1,
                 "Lord Chaos random adjacent retry schedules C37 at adjacent square");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

static int prime_generator_sensor(struct GameWorld_Compat* world);
static int schedule_generator_trigger(struct GameWorld_Compat* world);

static int rebuild_as_two_map_teleporter_world(struct GameWorld_Compat* world) {
    struct DungeonMapDesc_Compat* maps;
    struct DungeonMapTiles_Compat* tiles;
    unsigned char* map0;
    unsigned char* map1;
    unsigned short* squareFirstThings;
    struct DungeonTeleporter_Compat* teleporters;
    int i;

    if (!world || !world->dungeon || !world->things) return 0;
    free(world->dungeon->tiles[0].squareData);
    free(world->dungeon->maps);
    free(world->dungeon->tiles);
    free(world->things->squareFirstThings);

    maps = (struct DungeonMapDesc_Compat*)calloc(2, sizeof(*maps));
    tiles = (struct DungeonMapTiles_Compat*)calloc(2, sizeof(*tiles));
    map0 = (unsigned char*)calloc(9, sizeof(unsigned char));
    map1 = (unsigned char*)calloc(9, sizeof(unsigned char));
    squareFirstThings = (unsigned short*)calloc(2, sizeof(unsigned short));
    teleporters = (struct DungeonTeleporter_Compat*)calloc(1, sizeof(*teleporters));
    if (!maps || !tiles || !map0 || !map1 || !squareFirstThings || !teleporters) {
        free(maps);
        free(tiles);
        free(map0);
        free(map1);
        free(squareFirstThings);
        free(teleporters);
        return 0;
    }

    for (i = 0; i < 9; ++i) {
        map0[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
        map1[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
    map0[(1 * 3) + 1] = (unsigned char)((DUNGEON_ELEMENT_TELEPORTER << 5) |
                                        DUNGEON_SQUARE_MASK_THING_LIST | 0x08);
    map1[(2 * 3) + 1] |= DUNGEON_SQUARE_MASK_THING_LIST;

    maps[0].width = 3;
    maps[0].height = 3;
    maps[1].width = 3;
    maps[1].height = 3;
    tiles[0].squareCount = 9;
    tiles[0].squareData = map0;
    tiles[1].squareCount = 9;
    tiles[1].squareData = map1;

    squareFirstThings[0] = (unsigned short)((THING_TYPE_TELEPORTER << 10) | 0);
    squareFirstThings[1] = THING_ENDOFLIST;
    teleporters[0].next = (unsigned short)((THING_TYPE_SENSOR << 10) | 0);
    teleporters[0].targetMapX = 2;
    teleporters[0].targetMapY = 1;
    teleporters[0].targetMapIndex = 1;
    teleporters[0].scope = 1;
    teleporters[0].audible = 1;

    world->dungeon->header.mapCount = 2;
    world->dungeon->maps = maps;
    world->dungeon->tiles = tiles;
    world->things->squareFirstThingCount = 2;
    world->things->squareFirstThings = squareFirstThings;
    world->things->teleporterCount = 1;
    world->things->thingCounts[THING_TYPE_TELEPORTER] = 1;
    world->things->teleporters = teleporters;
    world->things->sensors[0].next = THING_ENDOFLIST;
    world->partyMapIndex = 0;
    world->party.mapIndex = 0;
    world->party.mapX = 0;
    world->party.mapY = 0;
    return 1;
}

static int test_c006_generated_group_teleports_cross_map_before_link(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world teleporter cross-map\n");
        return 1;
    }

    ok &= expect(rebuild_as_two_map_teleporter_world(&world),
                 "build two-map creature teleporter fixture");
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    ok &= expect(prime_generator_sensor(&world), "prime generator sensor on teleporter square");
    ok &= expect(schedule_generator_trigger(&world), "schedule generator on teleporter square");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance generator teleporter side-effect tick");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_TELEPORTER << 10) | 0),
                 "C006 F0267 teleporter leaves source teleporter chain untouched");
    ok &= expect(world.things->squareFirstThings[1] == (unsigned short)((THING_TYPE_GROUP << 10) | 0),
                 "C006 F0267 teleporter links generated group at target map square");
    ok &= expect(world.things->groups[0].next == THING_ENDOFLIST,
                 "C006 F0267 teleporter target group links onto empty target chain");
    ok &= expect(world.creatureAICount == 0,
                 "C006 F0267 cross-map group does not seed party-map active state");
    ok &= expect(world.timeline.count == 2 &&
                 world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_TICK &&
                 world.timeline.events[0].mapIndex == 1 &&
                 world.timeline.events[0].mapX == 2 &&
                 world.timeline.events[0].mapY == 1 &&
                 world.timeline.events[1].kind == TIMELINE_EVENT_GROUP_GENERATOR,
                 "C006 F0267 teleporter schedules C37 at target map square");
    ok &= expect(result.emissionCount == 3 &&
                 result.emissions[0].payload[0] == DM1_SND_BUZZ &&
                 result.emissions[0].payload[1] == 2 &&
                 result.emissions[0].payload[2] == 1 &&
                 result.emissions[0].payload[3] == 1,
                 "C006 F0267 audible teleporter buzzes at target map square before F0185 buzzes");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

static int prime_generator_sensor(struct GameWorld_Compat* world) {
    if (!world || !world->things || !world->things->sensors) return 0;
    world->things->sensors[0].sensorType = RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR;
    world->things->sensors[0].sensorData = 0;
    world->things->sensors[0].value = 1;
    world->things->sensors[0].audible = 1;
    world->things->sensors[0].onceOnly = 0;
    world->things->sensors[0].localMultiple = (unsigned short)((2u << 4) | 1u);
    return 1;
}

static int schedule_generator_trigger(struct GameWorld_Compat* world) {
    struct TimelineEvent_Compat event;
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_GROUP_GENERATOR;
    event.fireAtTick = world->gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = GENERATOR_EVENT_AUX0_TRIGGER;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &event) == 1;
}

static int test_c006_reuses_first_unused_group_slot(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct DungeonGroup_Compat* pool;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world unused-slot reuse\n");
        return 1;
    }

    free(world.things->groups);
    world.things->groups = NULL;
    pool = (struct DungeonGroup_Compat*)calloc(2, sizeof(*pool));
    if (!pool) {
        F0883_WORLD_Free_Compat(&world);
        fprintf(stderr, "FAIL: allocate two-slot group pool\n");
        return 1;
    }
    world.things->groups = pool;
    world.things->groupCount = 2;
    world.things->thingCounts[THING_TYPE_GROUP] = 2;
    world.things->groups[0].next = THING_ENDOFLIST;
    world.things->groups[0].slot = THING_ENDOFLIST;
    world.things->groups[0].creatureType = 9;
    world.things->groups[1].next = THING_NONE;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    ok &= expect(prime_generator_sensor(&world), "prime generator sensor for unused-slot reuse");
    ok &= expect(schedule_generator_trigger(&world), "schedule generator for unused-slot reuse");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance generator unused-slot reuse");
    ok &= expect(world.things->groupCount == 2,
                 "C006 generator keeps fixed source group-slot capacity");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_GROUP << 10) | 1),
                 "C006 generator reuses first Next=THING_NONE group slot");
    ok &= expect(world.things->groups[0].next == THING_ENDOFLIST &&
                 world.things->groups[0].creatureType == 9,
                 "C006 generator leaves earlier used group slot untouched");
    ok &= expect(world.things->groups[1].next == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "C006 reused group slot links previous square chain");
    ok &= expect(world.creatureAICount == 1 && world.creatureAI[0].reserved0 == 1,
                 "C006 reused group slot seeds active state with reused index");
    ok &= expect(world.timeline.count == 2 &&
                 world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_TICK &&
                 world.timeline.events[0].aux0 == 1,
                 "C006 reused group schedules C37 for reused index");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

static int test_c006_no_unused_group_slot_does_not_append(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world no-unused-slot\n");
        return 1;
    }

    world.things->groups[0].next = THING_ENDOFLIST;
    world.things->groups[0].slot = THING_ENDOFLIST;
    world.things->groups[0].creatureType = 12;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    ok &= expect(prime_generator_sensor(&world), "prime generator sensor without unused group slot");
    ok &= expect(schedule_generator_trigger(&world), "schedule generator without unused group slot");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance generator without unused group slot");
    ok &= expect(world.things->groupCount == 1,
                 "C006 generator does not append beyond source group-slot capacity");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "C006 generator leaves square chain unchanged when no group slot exists");
    ok &= expect(world.things->groups[0].next == THING_ENDOFLIST &&
                 world.things->groups[0].creatureType == 12,
                 "C006 generator leaves occupied group slot untouched when no free slot exists");
    ok &= expect(result.emissionCount == 1 &&
                 result.emissions[0].kind == EMIT_SOUND_REQUEST &&
                 result.emissions[0].payload[0] == DM1_SND_BUZZ,
                 "C006 no-slot path still keeps sensor-audible buzz");
    ok &= expect(world.creatureAICount == 0,
                 "C006 no-slot path does not seed active group state");
    ok &= expect(world.timeline.count == 1 &&
                 world.timeline.events[0].kind == TIMELINE_EVENT_GROUP_GENERATOR &&
                 world.timeline.events[0].aux0 == GENERATOR_EVENT_AUX0_REENABLE,
                 "C006 no-slot path keeps only delayed C65 re-enable");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
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

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: rebuild_world_blocked_defer\n");
        return 1;
    }

    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    world.party.mapX = 1;
    world.party.mapY = 1;
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
                 "schedule blocked C006 generator trigger event");
    generatorTriggerTick = world.gameTick;
    rc = F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result);
    ok &= expect(rc == ORCH_OK, "advance blocked C006 generator trigger tick");
    ok &= expect(result.emissionCount == 1,
                 "blocked C006 generator emits only sensor-audible buzz before deferred insertion");
    ok &= expect(result.emissions[0].kind == EMIT_SOUND_REQUEST &&
                 result.emissions[0].payload[0] == DM1_SND_BUZZ,
                 "blocked C006 generator keeps audible sensor buzz");
    ok &= expect(world.things->groupCount == 1,
                 "blocked C006 generator keeps initialized group slot for event60");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "blocked C006 generator does not link group onto occupied square");
    ok &= expect(world.things->groups[0].next == THING_ENDOFLIST,
                 "blocked C006 generator leaves deferred group unlinked");
    ok &= expect(world.creatureAICount == 0,
                 "blocked C006 generator does not seed active group state before insertion");
    ok &= expect(world.timeline.count == 2,
                 "blocked C006 generator schedules C65 re-enable and event60 retry");
    ok &= expect(world.timeline.events[0].kind == TIMELINE_EVENT_GROUP_GENERATOR &&
                 world.timeline.events[0].aux0 == GENERATOR_EVENT_AUX0_REENABLE &&
                 world.timeline.events[0].fireAtTick == generatorTriggerTick + 2u,
                 "blocked C006 generator keeps delayed C65 re-enable first");
    ok &= expect(world.timeline.events[1].kind == TIMELINE_EVENT_MOVE_GROUP_SILENT &&
                 world.timeline.events[1].fireAtTick == generatorTriggerTick + 5u &&
                 world.timeline.events[1].aux0 == 0,
                 "blocked C006 generator schedules silent event60 insertion retry");

    world.party.mapX = 0;
    world.party.mapY = 0;
    while (world.gameTick <= generatorTriggerTick + 5u) {
        memset(&result, 0, sizeof(result));
        rc = F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result);
        ok &= expect(rc == ORCH_OK, "advance to deferred event60 insertion tick");
    }
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_GROUP << 10) | 0),
                 "event60 retry links deferred group when destination clears");
    ok &= expect(world.things->groups[0].next == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "event60 retry preserves target square sensor chain");
    ok &= expect(world.creatureAICount == 1 &&
                 world.creatureAI[0].groupMapX == 1 &&
                 world.creatureAI[0].groupMapY == 1,
                 "event60 retry seeds party-map active group state after insertion");
    ok &= expect(world.timeline.count == 1 &&
                 world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_TICK &&
                 world.timeline.events[0].fireAtTick == generatorTriggerTick + 6u,
                 "event60 retry schedules source C37 wander after insertion");

    F0883_WORLD_Free_Compat(&world);
    if (!ok) return 1;
    if (test_lord_chaos_adjacent_random_retry() != 0) return 1;
    if (test_c006_generated_group_teleports_cross_map_before_link() != 0) return 1;
    if (test_c006_reuses_first_unused_group_slot() != 0) return 1;
    if (test_c006_no_unused_group_slot_does_not_append() != 0) return 1;
    puts("M10_C006_GENERATOR_REENABLE_AND_AUDIO_DISPATCH_OK");
    return 0;
}
