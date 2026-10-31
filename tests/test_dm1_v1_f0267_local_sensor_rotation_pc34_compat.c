/* ReDMCSB MOVESENS.C F0270/F0271/F0276: local floor sensor rotation. */
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); return 1; } \
} while (0)

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonWeapon_Compat weapon;
    struct DungeonSensor_Compat sensors[3];
    unsigned char squareData[2];
    unsigned short firstThings[2];
    unsigned char rawWeapon[4];
    unsigned char rawSensors[24];
    struct F0267ThingMoveRequestPc34Compat request;
    struct F0267ThingMoveResultPc34Compat result;
    unsigned short weaponThing = (unsigned short)(THING_TYPE_WEAPON << 10);
    unsigned short sensor0 = (unsigned short)(THING_TYPE_SENSOR << 10);
    unsigned short sensor1 = (unsigned short)((THING_TYPE_SENSOR << 10) | 1u);
    unsigned short sensor2 = (unsigned short)((THING_TYPE_SENSOR << 10) | 2u);
    unsigned short sensor1Cell1 = (unsigned short)(sensor1 | (1u << 14));

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&weapon, 0, sizeof(weapon));
    memset(sensors, 0, sizeof(sensors));
    memset(rawWeapon, 0, sizeof(rawWeapon));
    memset(rawSensors, 0, sizeof(rawSensors));
    squareData[0] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                    DUNGEON_SQUARE_MASK_THING_LIST);
    squareData[1] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                    DUNGEON_SQUARE_MASK_THING_LIST);
    firstThings[0] = weaponThing;
    firstThings[1] = sensor0;
    map.width = 2;
    map.height = 1;
    tiles.squareData = squareData;
    tiles.squareCount = 2;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    weapon.next = THING_ENDOFLIST;
    weapon.type = 7;
    rawWeapon[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawWeapon[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    rawWeapon[2] = 7;
    sensors[0].next = sensor1;
    sensors[1].next = THING_ENDOFLIST;
    sensors[0].sensorType = DM1_SENSOR_FLOOR_OBJECT;
    sensors[1].sensorType = DM1_SENSOR_FLOOR_OBJECT;
    sensors[0].sensorData = 7;
    sensors[1].sensorData = 7;
    sensors[0].localEffect = 1;
    sensors[1].localEffect = 1;
    sensors[0].localMultiple = DM1_EFFECT_CLEAR;
    sensors[1].localMultiple = DM1_EFFECT_TOGGLE;
    rawSensors[0] = (unsigned char)(sensor1 & 0xffu);
    rawSensors[1] = (unsigned char)(sensor1 >> 8);
    rawSensors[8] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawSensors[9] = (unsigned char)(THING_ENDOFLIST >> 8);
    things.loaded = 1;
    things.squareFirstThings = firstThings;
    things.squareFirstThingCount = 2;
    things.weapons = &weapon;
    things.weaponCount = 1;
    things.sensors = sensors;
    things.sensorCount = 2;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapon;
    things.rawThingData[THING_TYPE_SENSOR] = rawSensors;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    things.thingCounts[THING_TYPE_SENSOR] = 2;
    world.dungeon = &dungeon;
    world.things = &things;
    memset(&request, 0, sizeof(request));
    request.thing = weaponThing;
    request.sourceMapIndex = 0;
    request.sourceMapX = 0;
    request.sourceMapY = 0;
    request.destinationMapIndex = 0;
    request.destinationMapX = 1;
    request.destinationMapY = 0;
    request.sourceMapX = -1;
    CHECK(!F0267_MOVE_MoveThingOnLoadedChain_Compat(&world, &request, &result),
          "public F0267 rejects off-square claims for an already-owned tail");
    CHECK(firstThings[0] == weaponThing && firstThings[1] == sensor0 &&
          sensors[0].next == sensor1 && sensors[1].next == THING_ENDOFLIST &&
          weapon.next == THING_ENDOFLIST && result.destinationSensorPasses == 0,
          "rejected off-square claim neither duplicates the item nor rotates sensors");
    request.sourceMapX = 0;
    CHECK(F0267_MOVE_MoveThingOnLoadedChain_Compat(&world, &request, &result),
          "F0267 moves a raw object onto loaded local floor sensors");
    CHECK(result.destinationSensorPasses == 2 && result.localSensorRotations == 1 &&
          result.lastLocalSensorRotation.effect == DM1_EFFECT_TOGGLE &&
          result.lastLocalSensorRotation.mapX == 1 &&
          result.lastLocalSensorRotation.mapY == 0 &&
          result.lastLocalSensorRotation.cell == -1,
          "F0270 preserves the final G0403-G0406 local receipt");
    CHECK(firstThings[1] == sensor1 && sensors[1].next == sensor0 &&
          sensors[0].next == weaponThing &&
          rawSensors[8] == (unsigned char)(sensor0 & 0xffu),
          "F0271 rotates the complete loaded sensor run after F0276");

    /* ReDMCSB F0271 scans to the end of the contiguous sensor run even when
     * another cell's sensor occurs between matching sensors. */
    sensors[0].next = sensor1Cell1;
    sensors[1].next = sensor2;
    sensors[2].next = weaponThing;
    rawSensors[0] = (unsigned char)(sensor1Cell1 & 0xffu);
    rawSensors[1] = (unsigned char)(sensor1Cell1 >> 8);
    rawSensors[8] = (unsigned char)(sensor2 & 0xffu);
    rawSensors[9] = (unsigned char)(sensor2 >> 8);
    rawSensors[16] = (unsigned char)(weaponThing & 0xffu);
    rawSensors[17] = (unsigned char)(weaponThing >> 8);
    rawWeapon[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawWeapon[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    sensors[0].sensorType = DM1_SENSOR_FLOOR_OBJECT;
    sensors[2].sensorType = DM1_SENSOR_FLOOR_OBJECT;
    sensors[0].sensorData = 7;
    sensors[2].sensorData = 7;
    firstThings[1] = sensor0;
    things.sensorCount = 3;
    things.thingCounts[THING_TYPE_SENSOR] = 3;
    CHECK(F0271_SENSOR_ProcessRotation_Compat(
              &world, 0, 1, 0, 0, DM1_EFFECT_TOGGLE),
          "F0271 rotates matching sensors across other-cell entries");
    CHECK(firstThings[1] == sensor1Cell1 && sensors[1].next == sensor2 &&
          sensors[2].next == sensor0 && sensors[0].next == weaponThing &&
          rawSensors[8] == (unsigned char)(sensor2 & 0xffu) &&
          rawSensors[9] == (unsigned char)(sensor2 >> 8) &&
          rawSensors[16] == (unsigned char)(sensor0 & 0xffu) &&
          rawSensors[17] == (unsigned char)(sensor0 >> 8),
          "F0271 moves cell-0 head after the last match in the sensor run");
    sensors[0].sensorType = DM1_SENSOR_FLOOR_OBJECT;
    sensors[0].sensorData = 7;
    sensors[0].onceOnly = 1;
    rawSensors[2] = 0x84u; /* type 4 + data bit 0 */
    rawSensors[3] = 0x03u; /* remaining data bits */
    rawSensors[4] = 0x04u; /* OnceOnly */
    CHECK(F0272_SENSOR_DisableOnceOnly_Compat(&world, 0) == 1 &&
              sensors[0].sensorType == DM1_SENSOR_DISABLED &&
              sensors[0].sensorData == 7 && rawSensors[2] == 0x80u &&
              rawSensors[3] == 0x03u && rawSensors[4] == 0x04u,
          "F0272 disables only M039_TYPE and preserves sensor data and flags");
    puts("ok: F0267 consumes the source-owned F0270/F0271 floor rotation receipt");
    return 0;
}
