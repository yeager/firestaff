/*
 * DM1 V1 pit/fall regression gate.
 *
 * Source-lock anchors:
 * - ReDMCSB MOVESENS.C F0267_MOVE_GetMoveResult_CPSCE lines 525-604:
 *   an open, non-imaginary pit moves the party through
 *   F0154_DUNGEON_GetLocationAfterLevelChange and applies attack 20 fall
 *   damage to every living champion when the rope is not being used.
 * - ReDMCSB MOVESENS.C F0267 lines 801-818 and F0276 lines 1553-1793:
 *   after the final destination is resolved, the party's source square is
 *   processed as WALK_OFF and the final square is processed as WALK_ON for
 *   sensor consequences.
 *
 * This probe exercises the real F0888 party input path for one cardinal move:
 * origin corridor -> open pit -> lower-map landing sensor.  The assertion
 * surface is intentionally small: one transition, one two-champion party
 * snapshot, one damage/sensor consequence.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#define MAP_W 3
#define MAP_H 3
#define MAP_COUNT 2
#define SFT_COUNT 1

static int g_pass = 0;
static int g_fail = 0;

static void record(const char* id, int ok, const char* msg)
{
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static unsigned char square_byte(int elementType, int attrs)
{
    return (unsigned char)(((elementType & 7) << 5) | (attrs & 0x1F));
}

static int count_emissions(const struct TickResult_Compat* result, uint8_t kind)
{
    int i;
    int count = 0;
    for (i = 0; i < result->emissionCount; ++i) {
        if (result->emissions[i].kind == kind) ++count;
    }
    return count;
}

static int find_emission(const struct TickResult_Compat* result, uint8_t kind)
{
    int i;
    for (i = 0; i < result->emissionCount; ++i) {
        if (result->emissions[i].kind == kind) return i;
    }
    return -1;
}

int main(void)
{
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat maps[MAP_COUNT];
    struct DungeonMapTiles_Compat tiles[MAP_COUNT];
    unsigned char map0[MAP_W * MAP_H];
    unsigned char map1[MAP_W * MAP_H];
    unsigned short squareFirstThings[SFT_COUNT];
    struct DungeonSensor_Compat sensors[1];
    int i;
    int fellIndex;
    int sensorIndex;

    memset(&world, 0, sizeof(world));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(sensors, 0, sizeof(sensors));

    for (i = 0; i < MAP_W * MAP_H; ++i) {
        map0[i] = square_byte(DUNGEON_ELEMENT_CORRIDOR, 0);
        map1[i] = square_byte(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    for (i = 0; i < SFT_COUNT; ++i) {
        squareFirstThings[i] = THING_ENDOFLIST;
    }

    map0[(1 * MAP_H) + 1] = square_byte(DUNGEON_ELEMENT_PIT, 0x08);
    map1[(1 * MAP_H) + 1] = square_byte(
        DUNGEON_ELEMENT_CORRIDOR,
        DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = (unsigned short)((THING_TYPE_SENSOR << 10) | 0);

    sensors[0].sensorType = 13;
    sensors[0].sensorData = 77;
    sensors[0].localEffect = 1;
    sensors[0].next = THING_ENDOFLIST;

    maps[0].level = 0;
    maps[0].width = MAP_W;
    maps[0].height = MAP_H;
    maps[1].level = 1;
    maps[1].width = MAP_W;
    maps[1].height = MAP_H;
    tiles[0].squareCount = MAP_W * MAP_H;
    tiles[0].squareData = map0;
    tiles[1].squareCount = MAP_W * MAP_H;
    tiles[1].squareData = map1;

    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = MAP_COUNT;
    dungeon.maps = maps;
    dungeon.tiles = tiles;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = SFT_COUNT;
    things.sensors = sensors;
    things.sensorCount = 1;
    things.thingCounts[THING_TYPE_SENSOR] = 1;

    if (!F0881_WORLD_InitDefault_Compat(&world, 0x5151u)) {
        record("PIT_WORLD_INIT", 0, "F0881_WORLD_InitDefault_Compat failed");
        printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
        return 1;
    }
    world.dungeon = &dungeon;
    world.things = &things;
    world.ownsDungeon = 0;
    world.partyMapIndex = 0;
    world.party.mapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 1;
    world.party.direction = DIR_EAST;
    world.party.championCount = 2;
    world.party.activeChampionIndex = 0;
    world.party.champions[0].present = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;
    world.party.champions[0].direction = DIR_EAST;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 45;
    world.party.champions[1].hp.maximum = 45;
    world.party.champions[1].direction = DIR_EAST;

    input.command = CMD_MOVE_EAST;
    record("PIT_APPLY_INPUT",
           F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1,
           "CMD_MOVE_EAST dispatched through the real party input path");

    record("PIT_FINAL_PARTY_STATE",
           world.party.mapIndex == 1 &&
               world.party.mapX == 1 &&
               world.party.mapY == 1 &&
               world.party.direction == DIR_EAST,
           "party lands on lower map at the same local coordinate/facing");
    record("PIT_FALL_DAMAGE_ALL_LIVING",
           world.party.champions[0].hp.current == 80 &&
               world.party.champions[1].hp.current == 25,
           "one open-pit transition applies exactly 20 HP damage to every living champion");

    fellIndex = find_emission(&result, EMIT_PARTY_FELL);
    record("PIT_FELL_EMISSION",
           fellIndex >= 0 &&
               result.emissions[fellIndex].payload[0] == 1 &&
               result.emissions[fellIndex].payload[1] == 1 &&
               result.emissions[fellIndex].payload[2] == 1 &&
               result.emissions[fellIndex].payload[3] == 1,
           "EMIT_PARTY_FELL reports map/x/y and pitCount=1");

    sensorIndex = find_emission(&result, EMIT_SENSOR_EFFECT);
    record("PIT_LANDING_SENSOR_COUNT",
           count_emissions(&result, EMIT_SENSOR_EFFECT) == 1,
           "only the lower-map landing sensor emits a consequence");
    record("PIT_LANDING_SENSOR_PAYLOAD",
           sensorIndex >= 0 &&
               result.emissions[sensorIndex].payload[0] == SENSOR_EFFECT_SHOW_TEXT &&
               result.emissions[sensorIndex].payload[1] == 13 &&
               result.emissions[sensorIndex].payload[2] == SENSOR_EVENT_WALK_ON &&
               result.emissions[sensorIndex].payload[3] == 77,
           "landing sensor emits WALK_ON text effect with its text index");
    record("PIT_NO_TELEPORT_EMISSION",
           count_emissions(&result, EMIT_PARTY_TELEPORTED) == 0,
           "single open pit does not report a teleporter transition");

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
