#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
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
    dungeon->maps[0].creatureTypeCount = 2;
    dungeon->maps[0].allowedCreatureTypes[0] = 0;
    dungeon->maps[0].allowedCreatureTypes[1] = 23;
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

static unsigned short test_next_thing(
    const struct DungeonThings_Compat* things,
    unsigned short thing)
{
    int type = THING_GET_TYPE(thing);
    int index = THING_GET_INDEX(thing);
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) return THING_NONE;
    switch (type) {
    case THING_TYPE_SENSOR:
        return (index < things->sensorCount) ? things->sensors[index].next : THING_NONE;
    case THING_TYPE_GROUP:
        return (index < things->groupCount) ? things->groups[index].next : THING_NONE;
    case THING_TYPE_WEAPON:
        return (index < things->weaponCount) ? things->weapons[index].next : THING_NONE;
    case THING_TYPE_ARMOUR:
        return (index < things->armourCount) ? things->armours[index].next : THING_NONE;
    case THING_TYPE_JUNK:
        return (index < things->junkCount) ? things->junks[index].next : THING_NONE;
    case THING_TYPE_PROJECTILE:
        return (index < things->projectileCount) ? things->projectiles[index].next : THING_NONE;
    default:
        return THING_NONE;
    }
}

static int test_count_chain_type(
    const struct DungeonThings_Compat* things,
    unsigned short first,
    int type)
{
    unsigned short thing = first;
    int count = 0;
    int safety = 0;
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        if ((int)THING_GET_TYPE(thing) == type) ++count;
        thing = test_next_thing(things, thing);
    }
    return count;
}

static int test_allocate_unused_junk_pool(
    struct GameWorld_Compat* world,
    int count)
{
    int i;
    if (!world || !world->things || count <= 0) return 0;
    world->things->junks =
        (struct DungeonJunk_Compat*)calloc((size_t)count, sizeof(*world->things->junks));
    if (!world->things->junks) return 0;
    world->things->junkCount = count;
    world->things->thingCounts[THING_TYPE_JUNK] = count;
    for (i = 0; i < count; ++i) {
        world->things->junks[i].next = THING_NONE;
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
    maps[0].creatureTypeCount = 2;
    maps[0].allowedCreatureTypes[0] = 0;
    maps[0].allowedCreatureTypes[1] = 23;
    maps[1].width = 3;
    maps[1].height = 3;
    maps[1].creatureTypeCount = 2;
    maps[1].allowedCreatureTypes[0] = 0;
    maps[1].allowedCreatureTypes[1] = 23;
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


static int rebuild_as_three_map_multi_hop_teleporter_world(struct GameWorld_Compat* world) {
    struct DungeonMapDesc_Compat* maps;
    struct DungeonMapTiles_Compat* tiles;
    unsigned char* map0;
    unsigned char* map1;
    unsigned char* map2;
    unsigned short* squareFirstThings;
    struct DungeonTeleporter_Compat* teleporters;
    int i;

    if (!world || !world->dungeon || !world->things) return 0;
    free(world->dungeon->tiles[0].squareData);
    free(world->dungeon->maps);
    free(world->dungeon->tiles);
    free(world->things->squareFirstThings);

    maps = (struct DungeonMapDesc_Compat*)calloc(3, sizeof(*maps));
    tiles = (struct DungeonMapTiles_Compat*)calloc(3, sizeof(*tiles));
    map0 = (unsigned char*)calloc(9, sizeof(unsigned char));
    map1 = (unsigned char*)calloc(9, sizeof(unsigned char));
    map2 = (unsigned char*)calloc(9, sizeof(unsigned char));
    squareFirstThings = (unsigned short*)calloc(3, sizeof(unsigned short));
    teleporters = (struct DungeonTeleporter_Compat*)calloc(2, sizeof(*teleporters));
    if (!maps || !tiles || !map0 || !map1 || !map2 || !squareFirstThings || !teleporters) {
        free(maps);
        free(tiles);
        free(map0);
        free(map1);
        free(map2);
        free(squareFirstThings);
        free(teleporters);
        return 0;
    }

    for (i = 0; i < 9; ++i) {
        map0[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
        map1[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
        map2[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
    map0[(1 * 3) + 1] = (unsigned char)((DUNGEON_ELEMENT_TELEPORTER << 5) |
                                        DUNGEON_SQUARE_MASK_THING_LIST | 0x08);
    map1[(1 * 3) + 1] = (unsigned char)((DUNGEON_ELEMENT_TELEPORTER << 5) |
                                        DUNGEON_SQUARE_MASK_THING_LIST | 0x08);
    map2[(2 * 3) + 1] |= DUNGEON_SQUARE_MASK_THING_LIST;

    for (i = 0; i < 3; ++i) {
        maps[i].width = 3;
        maps[i].height = 3;
        maps[i].creatureTypeCount = 2;
        maps[i].allowedCreatureTypes[0] = 0;
        maps[i].allowedCreatureTypes[1] = 23;
        tiles[i].squareCount = 9;
    }
    tiles[0].squareData = map0;
    tiles[1].squareData = map1;
    tiles[2].squareData = map2;

    squareFirstThings[0] = (unsigned short)((THING_TYPE_TELEPORTER << 10) | 0);
    squareFirstThings[1] = (unsigned short)((THING_TYPE_TELEPORTER << 10) | 1);
    squareFirstThings[2] = THING_ENDOFLIST;
    teleporters[0].next = (unsigned short)((THING_TYPE_SENSOR << 10) | 0);
    teleporters[0].targetMapX = 1;
    teleporters[0].targetMapY = 1;
    teleporters[0].targetMapIndex = 1;
    teleporters[0].scope = 1;
    teleporters[0].audible = 1;
    teleporters[1].next = THING_ENDOFLIST;
    teleporters[1].targetMapX = 2;
    teleporters[1].targetMapY = 1;
    teleporters[1].targetMapIndex = 2;
    teleporters[1].scope = 1;
    teleporters[1].audible = 1;

    world->dungeon->header.mapCount = 3;
    world->dungeon->maps = maps;
    world->dungeon->tiles = tiles;
    world->things->squareFirstThingCount = 3;
    world->things->squareFirstThings = squareFirstThings;
    world->things->teleporterCount = 2;
    world->things->thingCounts[THING_TYPE_TELEPORTER] = 2;
    world->things->teleporters = teleporters;
    world->things->sensors[0].next = THING_ENDOFLIST;
    world->partyMapIndex = 0;
    world->party.mapIndex = 0;
    world->party.mapX = 0;
    world->party.mapY = 0;
    return 1;
}

static int test_c006_generated_group_multi_hop_teleporter_buzzes_each_audible_hop(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world multi-hop teleporter\n");
        return 1;
    }

    ok &= expect(rebuild_as_three_map_multi_hop_teleporter_world(&world),
                 "build three-map multi-hop creature teleporter fixture");
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    ok &= expect(prime_generator_sensor(&world), "prime generator sensor on first teleporter square");
    ok &= expect(schedule_generator_trigger(&world), "schedule generator on first teleporter square");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance generator multi-hop teleporter side-effect tick");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_TELEPORTER << 10) | 0),
                 "C006 F0267 multi-hop leaves source teleporter chain untouched");
    ok &= expect(world.things->squareFirstThings[1] == (unsigned short)((THING_TYPE_TELEPORTER << 10) | 1),
                 "C006 F0267 multi-hop leaves intermediate teleporter chain untouched");
    ok &= expect(world.things->squareFirstThings[2] == (unsigned short)((THING_TYPE_GROUP << 10) | 0),
                 "C006 F0267 multi-hop links generated group at final target square");
    ok &= expect(result.emissionCount == 4,
                 "C006 F0267 multi-hop emits both teleporter buzzes plus F0185 and sensor buzzes");
    ok &= expect(result.emissions[0].payload[0] == DM1_SND_BUZZ &&
                 result.emissions[0].payload[1] == 1 &&
                 result.emissions[0].payload[2] == 1 &&
                 result.emissions[0].payload[3] == 1,
                 "C006 F0267 first audible teleporter buzzes at intermediate target");
    ok &= expect(result.emissions[1].payload[0] == DM1_SND_BUZZ &&
                 result.emissions[1].payload[1] == 2 &&
                 result.emissions[1].payload[2] == 1 &&
                 result.emissions[1].payload[3] == 2,
                 "C006 F0267 second audible teleporter buzzes at final target");
    ok &= expect(result.emissions[2].payload[0] == DM1_SND_BUZZ &&
                 result.emissions[2].payload[1] == 1 &&
                 result.emissions[2].payload[2] == 1 &&
                 result.emissions[2].payload[3] == 0 &&
                 result.emissions[3].payload[0] == DM1_SND_BUZZ &&
                 result.emissions[3].payload[1] == 1 &&
                 result.emissions[3].payload[2] == 1 &&
                 result.emissions[3].payload[3] == 0,
                 "C006 F0267 multi-hop keeps F0185 and sensor-audible source buzzes after teleporter buzzes");

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


static int rebuild_as_two_map_pit_world(struct GameWorld_Compat* world) {
    struct DungeonMapDesc_Compat* maps;
    struct DungeonMapTiles_Compat* tiles;
    unsigned char* map0;
    unsigned char* map1;
    unsigned short* squareFirstThings;
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
    if (!maps || !tiles || !map0 || !map1 || !squareFirstThings) {
        free(maps);
        free(tiles);
        free(map0);
        free(map1);
        free(squareFirstThings);
        return 0;
    }

    for (i = 0; i < 9; ++i) {
        map0[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
        map1[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
    map0[(1 * 3) + 1] = (unsigned char)((DUNGEON_ELEMENT_PIT << 5) |
                                        DUNGEON_SQUARE_MASK_THING_LIST | 0x08);
    map1[(1 * 3) + 1] |= DUNGEON_SQUARE_MASK_THING_LIST;

    maps[0].level = 0;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].creatureTypeCount = 2;
    maps[0].allowedCreatureTypes[0] = 0;
    maps[0].allowedCreatureTypes[1] = 23;
    maps[1].level = 1;
    maps[1].width = 3;
    maps[1].height = 3;
    maps[1].creatureTypeCount = 2;
    maps[1].allowedCreatureTypes[0] = 0;
    maps[1].allowedCreatureTypes[1] = 23;
    tiles[0].squareCount = 9;
    tiles[0].squareData = map0;
    tiles[1].squareCount = 9;
    tiles[1].squareData = map1;

    squareFirstThings[0] = (unsigned short)((THING_TYPE_SENSOR << 10) | 0);
    squareFirstThings[1] = THING_ENDOFLIST;

    world->dungeon->header.mapCount = 2;
    world->dungeon->maps = maps;
    world->dungeon->tiles = tiles;
    world->things->squareFirstThingCount = 2;
    world->things->squareFirstThings = squareFirstThings;
    world->things->sensors[0].next = THING_ENDOFLIST;
    world->partyMapIndex = 0;
    world->party.mapIndex = 0;
    world->party.mapX = 0;
    world->party.mapY = 0;
    return 1;
}

static int test_c006_generated_group_falls_through_pit_before_link(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world generator pit fall\n");
        return 1;
    }

    ok &= expect(rebuild_as_two_map_pit_world(&world),
                 "build two-map open-pit generator fixture");
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    ok &= expect(prime_generator_sensor(&world), "prime generator sensor on open pit square");
    ok &= expect(schedule_generator_trigger(&world), "schedule generator on open pit square");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance generator pit fall side-effect tick");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "C006 F0267 pit fall leaves source pit sensor chain untouched");
    ok &= expect(world.things->squareFirstThings[1] == (unsigned short)((THING_TYPE_GROUP << 10) | 0),
                 "C006 F0267 pit fall links generated group on lower map square");
    ok &= expect(world.things->groups[0].next == THING_ENDOFLIST,
                 "C006 F0267 pit fall target group links onto empty lower chain");
    ok &= expect(world.things->groups[0].health[0] > 0 &&
                 world.things->groups[0].health[0] < 180,
                 "C006 F0267 pit fall applies source fall damage before insertion");
    ok &= expect(world.creatureAICount == 0,
                 "C006 pit-fallen cross-map group does not seed party-map active state");
    ok &= expect(world.timeline.count == 2 &&
                 world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_TICK &&
                 world.timeline.events[0].mapIndex == 1 &&
                 world.timeline.events[0].mapX == 1 &&
                 world.timeline.events[0].mapY == 1 &&
                 world.timeline.events[1].kind == TIMELINE_EVENT_GROUP_GENERATOR,
                 "C006 pit-fallen group schedules C37 at lower map square");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

static int test_event60_group_pit_fall_death_drops_carried_slot(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    unsigned short dropped;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world event60 pit death\n");
        return 1;
    }

    ok &= expect(rebuild_as_two_map_pit_world(&world),
                 "build two-map open-pit event60 fixture");
    ok &= expect(test_allocate_unused_junk_pool(&world, 10),
                 "allocate red dragon fixed-drop junk pool");
    world.things->weapons = (struct DungeonWeapon_Compat*)calloc(1, sizeof(*world.things->weapons));
    if (!world.things->weapons) {
        F0883_WORLD_Free_Compat(&world);
        fprintf(stderr, "FAIL: allocate carried weapon\n");
        return 1;
    }
    world.things->weaponCount = 1;
    world.things->thingCounts[THING_TYPE_WEAPON] = 1;
    world.things->weapons[0].next = THING_ENDOFLIST;
    world.things->groups[0].next = THING_ENDOFLIST;
    world.things->groups[0].slot = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    world.things->groups[0].creatureType = DM1_CREATURE_TYPE_RED_DRAGON;
    world.things->groups[0].cells = RUNTIME_GROUP_CELLS_SINGLE_CENTERED;
    world.things->groups[0].health[0] = 1;
    world.things->groups[0].count = 0;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_MOVE_GROUP_SILENT;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = 0;

    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule event60 group pit-fall death");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance event60 group pit-fall death");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "event60 pit death leaves source pit sensor chain untouched");
    dropped = world.things->squareFirstThings[1];
    ok &= expect(THING_GET_TYPE(dropped) == THING_TYPE_JUNK,
                 "event60 pit death drops fixed possessions before carried slot");
    ok &= expect(test_count_chain_type(world.things, dropped, THING_TYPE_JUNK) >= 8,
                 "event60 pit death materializes red dragon fixed possessions");
    ok &= expect(test_count_chain_type(world.things, dropped, THING_TYPE_WEAPON) == 1,
                 "event60 pit death also drops carried group slot on lower map square");
    ok &= expect(world.things->weapons[0].next == THING_ENDOFLIST,
                 "event60 pit death dropped weapon terminates lower chain");
    ok &= expect(world.things->groups[0].slot == THING_ENDOFLIST &&
                 world.things->groups[0].health[0] == 0,
                 "event60 pit death clears carried slot and kills group before insertion");
    ok &= expect(world.creatureAICount == 0 && world.timeline.count == 0,
                 "event60 pit death does not insert group or schedule C37 retry");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

static int test_event60_group_pit_fall_partial_death_drops_fixed_possessions(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    unsigned short head;
    unsigned short firstDrop;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world event60 pit partial death\n");
        return 1;
    }

    ok &= expect(rebuild_as_two_map_pit_world(&world),
                 "build two-map open-pit event60 partial-death fixture");
    ok &= expect(test_allocate_unused_junk_pool(&world, 10),
                 "allocate red dragon partial fixed-drop junk pool");
    world.dungeon->maps[1].allowedCreatureTypes[0] = DM1_CREATURE_TYPE_RED_DRAGON;
    world.things->groups[0].next = THING_ENDOFLIST;
    world.things->groups[0].slot = THING_ENDOFLIST;
    world.things->groups[0].creatureType = DM1_CREATURE_TYPE_RED_DRAGON;
    world.things->groups[0].cells = (unsigned char)((1u << 2) | 0u);
    world.things->groups[0].health[0] = 180;
    world.things->groups[0].health[1] = 1;
    world.things->groups[0].count = 1;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_MOVE_GROUP_SILENT;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = 0;

    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule event60 group pit-fall partial death");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance event60 group pit-fall partial death");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "event60 pit partial death leaves source pit sensor chain untouched");
    head = world.things->squareFirstThings[1];
    ok &= expect(head == (unsigned short)((THING_TYPE_GROUP << 10) | 0),
                 "event60 pit partial death still inserts surviving group");
    firstDrop = world.things->groups[0].next;
    ok &= expect(THING_GET_TYPE(firstDrop) == THING_TYPE_JUNK,
                 "event60 pit partial death links fixed drops behind surviving group");
    ok &= expect(test_count_chain_type(world.things, firstDrop, THING_TYPE_JUNK) >= 8,
                 "event60 pit partial death materializes killed creature fixed possessions");
    ok &= expect(world.things->groups[0].count == 0 &&
                 world.things->groups[0].health[0] > 0 &&
                 world.things->groups[0].health[0] < 180 &&
                 world.things->groups[0].health[1] == 0,
                 "event60 pit partial death compacts survivor health/count");
    ok &= expect(world.creatureAICount == 0 &&
                 world.timeline.count == 1 &&
                 world.timeline.events[0].kind == TIMELINE_EVENT_CREATURE_TICK &&
                 world.timeline.events[0].mapIndex == 1,
                 "event60 pit partial death schedules lower-map survivor C37 only");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
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


static unsigned short test_make_thing_ref(int type, int index, int cell) {
    return (unsigned short)(((cell & 3) << 14) | ((type & 15) << 10) | (index & 0x03FF));
}

static int test_add_projectile_on_square(
    struct GameWorld_Compat* world,
    int squareFirstThingIndex,
    unsigned short nextThing)
{
    if (!world || !world->things || squareFirstThingIndex < 0 ||
        squareFirstThingIndex >= world->things->squareFirstThingCount) {
        return 0;
    }
    world->things->projectiles = (struct DungeonProjectile_Compat*)calloc(1, sizeof(*world->things->projectiles));
    if (!world->things->projectiles) return 0;
    world->things->projectileCount = 1;
    world->things->thingCounts[THING_TYPE_PROJECTILE] = 1;
    world->things->projectiles[0].next = nextThing;
    world->things->projectiles[0].slot = THING_ENDOFLIST;
    world->things->projectiles[0].eventIndex = 77;
    world->things->squareFirstThings[squareFirstThingIndex] =
        test_make_thing_ref(THING_TYPE_PROJECTILE, 0, 0);
    return 1;
}

static int test_schedule_future_projectile_move(struct GameWorld_Compat* world) {
    struct TimelineEvent_Compat event;
    if (!world) return 0;
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event.fireAtTick = world->gameTick + 100u;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.cell = 0;
    event.aux0 = 0;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &event) == 1;
}

static int test_timeline_keeps_future_projectile_move(const struct GameWorld_Compat* world) {
    int i;
    if (!world) return 0;
    for (i = 0; i < world->timeline.count; ++i) {
        if (world->timeline.events[i].kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
            world->timeline.events[i].aux0 == 0) {
            return 1;
        }
    }
    return 0;
}

static int test_c006_generated_group_preserves_projectile_without_impact(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world generated projectile preserve\n");
        return 1;
    }

    ok &= expect(test_add_projectile_on_square(
                     &world, 0, (unsigned short)((THING_TYPE_SENSOR << 10) | 0)),
                 "place projectile before C006 generator sensor chain");
    ok &= expect(test_schedule_future_projectile_move(&world),
                 "schedule future projectile move before C006 generation");
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    ok &= expect(prime_generator_sensor(&world),
                 "prime generator sensor behind projectile");
    ok &= expect(schedule_generator_trigger(&world),
                 "schedule generator on projectile square");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance generator projectile-preserve tick");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_GROUP << 10) | 0),
                 "C006 generated group links ahead of projectile square chain");
    ok &= expect(world.things->groups[0].next == test_make_thing_ref(THING_TYPE_PROJECTILE, 0, 0),
                 "C006 generated group preserves projectile as next thing");
    ok &= expect(world.things->projectiles[0].next == (unsigned short)((THING_TYPE_SENSOR << 10) | 0) &&
                 world.things->projectiles[0].eventIndex == 77,
                 "C006 generated insertion does not remove projectile thing/event index");
    ok &= expect(test_timeline_keeps_future_projectile_move(&world),
                 "C006 generated insertion leaves projectile move event queued");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}


static int test_schedule_creature_tick_now(struct GameWorld_Compat* world, int groupIndex) {
    struct TimelineEvent_Compat event;
    if (!world) return 0;
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_TICK;
    event.fireAtTick = world->gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = groupIndex;
    event.aux1 = 0;
    event.aux2 = AI_STATE_WANDER;
    return F0721_TIMELINE_Schedule_Compat(&world->timeline, &event) == 1;
}

static int test_timeline_lacks_projectile_move(const struct GameWorld_Compat* world) {
    int i;
    if (!world) return 0;
    for (i = 0; i < world->timeline.count; ++i) {
        if (world->timeline.events[i].kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
            world->timeline.events[i].aux0 == 0) {
            return 0;
        }
    }
    return 1;
}

static int test_c006_f0267_ordinary_moving_group_source_projectile_is_removed(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct DungeonGroup_Compat* group;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world ordinary group projectile impact\n");
        return 1;
    }

    world.things->projectiles = (struct DungeonProjectile_Compat*)calloc(1, sizeof(*world.things->projectiles));
    if (!world.things->projectiles) {
        F0883_WORLD_Free_Compat(&world);
        return 1;
    }
    world.things->projectileCount = 1;
    world.things->thingCounts[THING_TYPE_PROJECTILE] = 1;

    group = &world.things->groups[0];
    group->next = test_make_thing_ref(THING_TYPE_PROJECTILE, 0, 0);
    group->slot = THING_ENDOFLIST;
    group->creatureType = 0;
    group->cells = 0;
    group->health[0] = 50;
    group->count = 0;
    group->direction = 1;
    world.things->squareFirstThings[0] = test_make_thing_ref(THING_TYPE_GROUP, 0, 0);
    world.things->projectiles[0].next = (unsigned short)((THING_TYPE_SENSOR << 10) | 0);
    world.things->projectiles[0].slot = THING_ENDOFLIST;
    world.things->projectiles[0].attack = 5;
    world.things->projectiles[0].kineticEnergy = 5;
    world.things->projectiles[0].eventIndex = 77;

    memset(&world.creatureAI[0], 0, sizeof(world.creatureAI[0]));
    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = 0;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 1;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].groupCells = group->cells;
    world.creatureAI[0].groupDirection = 1;
    world.creatureAI[0].reserved0 = 0;

    ok &= expect(test_schedule_future_projectile_move(&world),
                 "MOVESENS.C:297/F0214 queues projectile event before ordinary group impact");
    ok &= expect(test_schedule_creature_tick_now(&world, 0),
                 "GROUP.C:1928/2175 ordinary group tick enters F0267 from real source square");
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance ordinary C37/F0267 projectile impact tick");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "MOVESENS.C:289-301 source projectile removed before group leaves source square");
    ok &= expect(world.things->squareFirstThings[1] == test_make_thing_ref(THING_TYPE_GROUP, 0, 0),
                 "MOVESENS.C:432-435 nonlethal ordinary group impact still permits F0267 movement");
    ok &= expect(world.things->groups[0].next == THING_ENDOFLIST &&
                 world.things->groups[0].health[0] == 40,
                 "PROJEXPL.C:515-608 applies projectile hit and unlinks projectile thing");
    ok &= expect(world.things->projectiles[0].next == THING_NONE &&
                 world.things->projectiles[0].eventIndex == 0xFFFFu,
                 "MOVESENS.C:297 plus PROJEXPL.C:607-608 deletes projectile event and thing");
    ok &= expect(test_timeline_lacks_projectile_move(&world),
                 "MOVESENS.C:297/F0214 removes queued projectile move on ordinary group impact");
    ok &= expect(world.creatureAICount == 1 &&
                 world.creatureAI[0].groupMapX == 2 &&
                 world.creatureAI[0].groupMapY == 1,
                 "ordinary moving group active state follows successful F0267 move");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}


static int test_c006_f0267_ordinary_moving_group_lethal_projectile_blocks_move(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct DungeonGroup_Compat* group;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world ordinary group lethal projectile impact\n");
        return 1;
    }

    world.things->projectiles = (struct DungeonProjectile_Compat*)calloc(1, sizeof(*world.things->projectiles));
    if (!world.things->projectiles) {
        F0883_WORLD_Free_Compat(&world);
        return 1;
    }
    world.things->projectileCount = 1;
    world.things->thingCounts[THING_TYPE_PROJECTILE] = 1;

    group = &world.things->groups[0];
    group->next = test_make_thing_ref(THING_TYPE_PROJECTILE, 0, 0);
    group->slot = THING_ENDOFLIST;
    group->creatureType = 0;
    group->cells = 0;
    group->health[0] = 10;
    group->count = 0;
    group->direction = 1;
    world.things->squareFirstThings[0] = test_make_thing_ref(THING_TYPE_GROUP, 0, 0);
    world.things->projectiles[0].next = (unsigned short)((THING_TYPE_SENSOR << 10) | 0);
    world.things->projectiles[0].slot = THING_ENDOFLIST;
    world.things->projectiles[0].attack = 10;
    world.things->projectiles[0].kineticEnergy = 10;
    world.things->projectiles[0].eventIndex = 77;

    memset(&world.creatureAI[0], 0, sizeof(world.creatureAI[0]));
    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = 0;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 1;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].groupCells = group->cells;
    world.creatureAI[0].groupDirection = 1;
    world.creatureAI[0].reserved0 = 0;

    ok &= expect(test_schedule_future_projectile_move(&world),
                 "schedule projectile event before lethal ordinary group impact");
    ok &= expect(test_schedule_creature_tick_now(&world, 0),
                 "schedule ordinary group movement tick for lethal projectile impact");
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance lethal ordinary C37/F0267 projectile impact tick");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "lethal ordinary group impact removes group and projectile from source chain");
    ok &= expect(world.things->squareFirstThings[1] == THING_ENDOFLIST,
                 "lethal ordinary group impact blocks successful F0267 movement");
    ok &= expect(world.things->groups[0].next == THING_NONE &&
                 world.things->groups[0].health[0] == 0,
                 "lethal ordinary group impact marks group slot unused/dead");
    ok &= expect(test_timeline_lacks_projectile_move(&world),
                 "lethal ordinary group impact deletes queued projectile move");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

static int test_c006_generated_group_not_allowed_drops_without_insertion(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world generated disallowed group\n");
        return 1;
    }

    world.dungeon->maps[0].allowedCreatureTypes[0] = 1;
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    ok &= expect(prime_generator_sensor(&world), "prime disallowed generator sensor");
    ok &= expect(schedule_generator_trigger(&world), "schedule disallowed generator");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance disallowed generator tick");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_SENSOR << 10) | 0),
                 "disallowed C006 group is not linked onto target square");
    ok &= expect(world.things->groups[0].next == THING_ENDOFLIST &&
                 world.things->groups[0].creatureType == 0,
                 "disallowed C006 group slot remains unlinked after F0267 rejection");
    ok &= expect(world.creatureAICount == 0,
                 "disallowed C006 group does not seed active state");
    ok &= expect(world.timeline.count == 1 &&
                 world.timeline.events[0].kind == TIMELINE_EVENT_GROUP_GENERATOR &&
                 world.timeline.events[0].aux0 == GENERATOR_EVENT_AUX0_REENABLE,
                 "disallowed C006 keeps only C65 re-enable event");
    ok &= expect(result.emissionCount == 1 &&
                 result.emissions[0].kind == EMIT_SOUND_REQUEST &&
                 result.emissions[0].payload[0] == DM1_SND_BUZZ,
                 "disallowed C006 keeps only sensor-audible buzz");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

static int test_event60_group_not_allowed_drops_carried_slot_without_retry(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world event60 disallowed group\n");
        return 1;
    }

    world.dungeon->maps[0].allowedCreatureTypes[0] = 1;
    world.things->weapons = (struct DungeonWeapon_Compat*)calloc(1, sizeof(*world.things->weapons));
    if (!world.things->weapons) {
        F0883_WORLD_Free_Compat(&world);
        fprintf(stderr, "FAIL: allocate disallowed carried weapon\n");
        return 1;
    }
    world.things->weaponCount = 1;
    world.things->thingCounts[THING_TYPE_WEAPON] = 1;
    world.things->weapons[0].next = THING_ENDOFLIST;
    world.things->groups[0].next = THING_ENDOFLIST;
    world.things->groups[0].slot = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    world.things->groups[0].creatureType = 0;
    world.things->groups[0].cells = RUNTIME_GROUP_CELLS_SINGLE_CENTERED;
    world.things->groups[0].health[0] = 180;
    world.things->groups[0].count = 0;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_MOVE_GROUP_SILENT;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = 0;

    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule event60 disallowed group");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance event60 disallowed group");
    ok &= expect(world.things->squareFirstThings[0] == (unsigned short)((THING_TYPE_SENSOR << 10) | 0) &&
                 test_count_chain_type(world.things, world.things->squareFirstThings[0],
                                       THING_TYPE_WEAPON) == 1,
                 "event60 disallowed group drops carried slot on destination square");
    ok &= expect(world.things->weapons[0].next == THING_ENDOFLIST,
                 "event60 disallowed carried slot terminates destination chain");
    ok &= expect(world.things->groups[0].slot == THING_ENDOFLIST &&
                 world.things->groups[0].next == THING_ENDOFLIST,
                 "event60 disallowed group remains unlinked after drop");
    ok &= expect(world.creatureAICount == 0 && world.timeline.count == 0,
                 "event60 disallowed group does not insert or schedule retry");

    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

static int test_event60_deferred_group_preserves_projectile_without_impact(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    int ok = 1;

    if (!build_world(&world)) {
        fprintf(stderr, "FAIL: build_world event60 projectile preserve\n");
        return 1;
    }

    ok &= expect(test_add_projectile_on_square(&world, 1, THING_ENDOFLIST),
                 "place projectile on event60 target square");
    ok &= expect(test_schedule_future_projectile_move(&world),
                 "schedule future projectile move before event60 insertion");
    world.things->groups[0].next = THING_ENDOFLIST;
    world.things->groups[0].slot = THING_ENDOFLIST;
    world.things->groups[0].creatureType = 0;
    world.things->groups[0].cells = RUNTIME_GROUP_CELLS_SINGLE_CENTERED;
    world.things->groups[0].health[0] = 180;
    world.things->groups[0].count = 0;

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_MOVE_GROUP_SILENT;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 2;
    event.mapY = 1;
    event.aux0 = 0;

    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule event60 insertion onto projectile square");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "advance event60 projectile-preserve tick");
    ok &= expect(world.things->squareFirstThings[1] == (unsigned short)((THING_TYPE_GROUP << 10) | 0),
                 "event60 group links ahead of projectile target chain");
    ok &= expect(world.things->groups[0].next == test_make_thing_ref(THING_TYPE_PROJECTILE, 0, 0),
                 "event60 group preserves projectile as next thing");
    ok &= expect(world.things->projectiles[0].next == THING_ENDOFLIST &&
                 world.things->projectiles[0].eventIndex == 77,
                 "event60 insertion does not remove projectile thing/event index");
    ok &= expect(test_timeline_keeps_future_projectile_move(&world),
                 "event60 insertion leaves projectile move event queued");

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
    if (test_c006_generated_group_multi_hop_teleporter_buzzes_each_audible_hop() != 0) return 1;
    if (test_c006_generated_group_falls_through_pit_before_link() != 0) return 1;
    if (test_event60_group_pit_fall_death_drops_carried_slot() != 0) return 1;
    if (test_event60_group_pit_fall_partial_death_drops_fixed_possessions() != 0) return 1;
    if (test_c006_reuses_first_unused_group_slot() != 0) return 1;
    if (test_c006_no_unused_group_slot_does_not_append() != 0) return 1;
    if (test_c006_generated_group_preserves_projectile_without_impact() != 0) return 1;
    if (test_c006_f0267_ordinary_moving_group_source_projectile_is_removed() != 0) return 1;
    if (test_c006_f0267_ordinary_moving_group_lethal_projectile_blocks_move() != 0) return 1;
    if (test_c006_generated_group_not_allowed_drops_without_insertion() != 0) return 1;
    if (test_event60_group_not_allowed_drops_carried_slot_without_retry() != 0) return 1;
    if (test_event60_deferred_group_preserves_projectile_without_impact() != 0) return 1;
    puts("M10_C006_GENERATOR_REENABLE_AND_AUDIO_DISPATCH_OK");
    return 0;
}
