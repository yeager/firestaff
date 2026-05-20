/*
 * CTest gate for DM1 V1 Sensor & Trigger System.
 *
 * Tests are source-locked to ReDMCSB MOVESENS.C / DEFS.H / DATA.C.
 * Each test cites the relevant source function and line.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dm1_v1_sensor_trigger_pc34_compat.h"

static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
} while(0)

/* ----------------------------------------------------------------
 *  Test F0727: SquareTypeToEventType mapping
 *  Source: DATA.C G0059_auc_Graphic562_SquareTypeToEventType[7]
 * ---------------------------------------------------------------- */
static void test_square_type_to_event_type(void) {
    CHECK(F0727_SENSOR_SquareTypeToEventType_Compat(DM1_SQUARE_WALL) == DM1_EVENT_WALL,
          "Wall -> EVENT_WALL (6)");
    CHECK(F0727_SENSOR_SquareTypeToEventType_Compat(DM1_SQUARE_CORRIDOR) == DM1_EVENT_CORRIDOR,
          "Corridor -> EVENT_CORRIDOR (5)");
    CHECK(F0727_SENSOR_SquareTypeToEventType_Compat(DM1_SQUARE_PIT) == DM1_EVENT_PIT,
          "Pit -> EVENT_PIT (9)");
    CHECK(F0727_SENSOR_SquareTypeToEventType_Compat(DM1_SQUARE_STAIRS) == DM1_EVENT_NONE,
          "Stairs -> EVENT_NONE (0)");
    CHECK(F0727_SENSOR_SquareTypeToEventType_Compat(DM1_SQUARE_DOOR) == DM1_EVENT_DOOR,
          "Door -> EVENT_DOOR (10)");
    CHECK(F0727_SENSOR_SquareTypeToEventType_Compat(DM1_SQUARE_TELEPORTER) == DM1_EVENT_TELEPORTER,
          "Teleporter -> EVENT_TELEPORTER (8)");
    CHECK(F0727_SENSOR_SquareTypeToEventType_Compat(DM1_SQUARE_FAKEWALL) == DM1_EVENT_FAKEWALL,
          "FakeWall -> EVENT_FAKEWALL (7)");
    CHECK(F0727_SENSOR_SquareTypeToEventType_Compat(-1) == -1,
          "Invalid (-1) -> -1");
    CHECK(F0727_SENSOR_SquareTypeToEventType_Compat(7) == -1,
          "Invalid (7) -> -1");
}

/* ----------------------------------------------------------------
 *  Test F0728: HOLD effect resolution
 *  Source: MOVESENS.C multiple sites
 * ---------------------------------------------------------------- */
static void test_hold_effect_resolution(void) {
    CHECK(F0728_SENSOR_ResolveHoldEffect_Compat(DM1_EFFECT_HOLD, 1) == DM1_EFFECT_SET,
          "HOLD + active -> SET");
    CHECK(F0728_SENSOR_ResolveHoldEffect_Compat(DM1_EFFECT_HOLD, 0) == DM1_EFFECT_CLEAR,
          "HOLD + inactive -> CLEAR");
    CHECK(F0728_SENSOR_ResolveHoldEffect_Compat(DM1_EFFECT_SET, 0) == DM1_EFFECT_SET,
          "SET passthrough");
    CHECK(F0728_SENSOR_ResolveHoldEffect_Compat(DM1_EFFECT_TOGGLE, 1) == DM1_EFFECT_TOGGLE,
          "TOGGLE passthrough");
}

/* ----------------------------------------------------------------
 *  Test F0720/F0721: Sensor type classification
 *  Source: DEFS.H lines 1256-1284
 * ---------------------------------------------------------------- */
static void test_sensor_classification(void) {
    int isFloor, isWall;

    F0720_SENSOR_ClassifyFloorType_Compat(0, &isFloor);
    CHECK(isFloor == 1, "Type 0 (DISABLED) is valid floor sensor");

    F0720_SENSOR_ClassifyFloorType_Compat(1, &isFloor);
    CHECK(isFloor == 1, "Type 1 (THERON) is floor sensor");

    F0720_SENSOR_ClassifyFloorType_Compat(9, &isFloor);
    CHECK(isFloor == 1, "Type 9 (VERSION_CHECKER) is floor sensor");

    F0720_SENSOR_ClassifyFloorType_Compat(10, &isFloor);
    CHECK(isFloor == 0, "Type 10 is NOT a floor sensor");

    F0721_SENSOR_ClassifyWallType_Compat(1, &isWall);
    CHECK(isWall == 1, "Type 1 is wall sensor");

    F0721_SENSOR_ClassifyWallType_Compat(18, &isWall);
    CHECK(isWall == 1, "Type 18 (END_GAME) is wall sensor");

    F0721_SENSOR_ClassifyWallType_Compat(127, &isWall);
    CHECK(isWall == 1, "Type 127 (CHAMPION_PORTRAIT) is wall sensor");

    F0721_SENSOR_ClassifyWallType_Compat(19, &isWall);
    CHECK(isWall == 0, "Type 19 is NOT a wall sensor");
}

/* ----------------------------------------------------------------
 *  Helper: build a test sensor
 * ---------------------------------------------------------------- */
static struct DungeonSensor_Compat make_sensor(
    int type, int data, int effect, int revertEffect,
    int onceOnly, int audible, int value,
    int localEffect, int targetMapX, int targetMapY, int targetCell)
{
    struct DungeonSensor_Compat s;
    memset(&s, 0, sizeof(s));
    s.sensorType = (unsigned char)type;
    s.sensorData = (unsigned short)data;
    s.effect = (unsigned char)effect;
    s.revertEffect = (unsigned char)revertEffect;
    s.onceOnly = (unsigned char)onceOnly;
    s.audible = (unsigned char)audible;
    s.value = (unsigned char)value;
    s.localEffect = (unsigned char)localEffect;
    s.targetMapX = (unsigned char)targetMapX;
    s.targetMapY = (unsigned char)targetMapY;
    s.targetCell = (unsigned char)targetCell;
    s.localMultiple = 0;
    return s;
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — pressure plate (party step on)
 *  Source: F0276 case C001 (THERON_PARTY_CREATURE_OBJECT)
 * ---------------------------------------------------------------- */
static void test_floor_theron_pressure_plate(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    /* Sensor: type 1 (any), TOGGLE effect, remote target at (3,5) */
    sensor = make_sensor(1, 0, DM1_EFFECT_TOGGLE, 0, 0, 1, 2,
                         0, 3, 5, 0);

    /* Party steps onto empty square */
    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.objectType = -1;
    ctx.partyOnSquare = 0;
    ctx.squareHasObject = 0;
    ctx.squareHasGroup = 0;
    ctx.isAddition = 1;
    ctx.partyChampionCount = 4;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Theron plate: party triggers on empty square");
    CHECK(result.resolvedEffect == DM1_EFFECT_TOGGLE, "Theron plate: TOGGLE effect");
    CHECK(result.audible == 1, "Theron plate: audible");
    CHECK(result.targetMapX == 3, "Theron plate: target X=3");
    CHECK(result.targetMapY == 5, "Theron plate: target Y=5");

    /* Source: F0276 case C001:
     * if (P0591_B_PartySquare || L0772_B_SquareContainsObject || L0773_B_SquareContainsGroup)
     *     goto T0276079; */
    ctx.partyOnSquare = 1;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Theron plate: no re-trigger if party already on square");

    ctx.partyOnSquare = 0;
    ctx.squareHasObject = 1;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Theron plate: no trigger if object already on square");

    ctx.squareHasObject = 0;
    ctx.squareHasGroup = 1;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Theron plate: no trigger if group already on square");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — party-only pressure plate
 *  Source: F0276 case C003 (FLOOR_PARTY)
 * ---------------------------------------------------------------- */
static void test_floor_party_only_plate(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    /* Sensor: type 3, data=0 (any direction), SET effect */
    sensor = make_sensor(3, 0, DM1_EFFECT_SET, 0, 0, 0, 1,
                         0, 7, 8, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.partyChampionCount = 2;
    ctx.isAddition = 1;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Party plate: triggers for party");
    CHECK(result.resolvedEffect == DM1_EFFECT_SET, "Party plate: SET");

    /* Creature should NOT trigger party-only sensor */
    ctx.thingType = DM1_TRIGGER_SOURCE_CREATURE;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Party plate: creature does NOT trigger");

    /* Object should NOT trigger party-only sensor */
    ctx.thingType = DM1_TRIGGER_SOURCE_OBJECT;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Party plate: object does NOT trigger");

    /* Party with 0 champions should NOT trigger */
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.partyChampionCount = 0;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Party plate: 0 champions does NOT trigger");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — direction-sensitive party plate
 *  Source: F0276 case C003 with data != 0
 * ---------------------------------------------------------------- */
static void test_floor_direction_sensitive_plate(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    /* Sensor: type 3, data=2 (expect direction ordinal 2 = direction 1 = east), HOLD effect */
    sensor = make_sensor(3, 2, DM1_EFFECT_HOLD, 0, 0, 0, 0,
                         0, 4, 4, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.partyChampionCount = 1;
    ctx.isAddition = 1;
    ctx.partyDirection = 1; /* East -> ordinal = 2 */

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Dir plate: triggers for matching direction");
    CHECK(result.resolvedEffect == DM1_EFFECT_SET, "Dir plate: HOLD resolved to SET");

    /* Wrong direction */
    ctx.partyDirection = 0; /* North -> ordinal = 1 != data=2 */
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    /* HOLD + triggerSensor=false -> CLEAR, and still triggers (HOLD always fires) */
    CHECK(result.triggered == 1, "Dir plate: HOLD still fires for wrong direction");
    CHECK(result.resolvedEffect == DM1_EFFECT_CLEAR, "Dir plate: HOLD resolved to CLEAR for wrong dir");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — specific object
 *  Source: F0276 case C004 (FLOOR_OBJECT)
 * ---------------------------------------------------------------- */
static void test_floor_specific_object(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    /* Sensor: type 4, data=8 (water icon type), TOGGLE */
    sensor = make_sensor(4, 8, DM1_EFFECT_TOGGLE, 0, 0, 1, 0,
                         0, 2, 3, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_OBJECT;
    ctx.objectType = 8; /* Matching object type */
    ctx.isAddition = 1;
    ctx.squareHasSameTypeObj = 0;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Obj plate: triggers for matching object");

    /* Wrong object type */
    ctx.objectType = 9;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Obj plate: no trigger for wrong object type");

    /* Same type already on square */
    ctx.objectType = 8;
    ctx.squareHasSameTypeObj = 1;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Obj plate: no trigger if same type already on square");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — creature only
 *  Source: F0276 case C007 (FLOOR_CREATURE)
 * ---------------------------------------------------------------- */
static void test_floor_creature_only(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(7, 0, DM1_EFFECT_TOGGLE, 0, 0, 0, 0,
                         0, 5, 5, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_CREATURE;
    ctx.isAddition = 1;
    ctx.squareHasGroup = 0;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Creature plate: triggers for creature");

    /* Party should NOT trigger creature-only sensor */
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Creature plate: party does NOT trigger");

    /* Already a group on square */
    ctx.thingType = DM1_TRIGGER_SOURCE_CREATURE;
    ctx.squareHasGroup = 1;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Creature plate: no trigger if group already on square");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — once-only
 *  Source: F0272 line ~1180: OnceOnly -> SET_TYPE_DISABLED
 * ---------------------------------------------------------------- */
static void test_floor_once_only(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(1, 0, DM1_EFFECT_SET, 0, 1, 0, 0,
                         0, 1, 1, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.isAddition = 1;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Once-only: triggers");
    CHECK(result.sensorDisabled == 1, "Once-only: sensor disabled after trigger");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — revert effect
 *  Source: F0276 line ~1668: triggerSensor ^= revertEffect
 * ---------------------------------------------------------------- */
static void test_floor_revert_effect(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    /* Revert=1: invert trigger. TOGGLE effect means it still needs triggerSensor. */
    sensor = make_sensor(1, 0, DM1_EFFECT_TOGGLE, 1, 0, 0, 0,
                         0, 2, 2, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.isAddition = 1; /* triggerSensor starts as 1 */

    /* triggerSensor ^= revertEffect(1) -> triggerSensor=0 -> no trigger for non-HOLD */
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Revert: addition with revert=1 does NOT trigger TOGGLE");

    /* Removal: triggerSensor starts as 0, XOR with 1 -> 1 -> triggers */
    ctx.isAddition = 0;
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Revert: removal with revert=1 DOES trigger TOGGLE");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — local effect (rotation)
 *  Source: F0272 line ~1190-1196: localEffect -> TriggerLocalEffect
 * ---------------------------------------------------------------- */
static void test_floor_local_effect(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(1, 0, DM1_EFFECT_SET, 0, 0, 0, 0,
                         1, 0, 0, 0); /* localEffect=1 */
    sensor.localMultiple = DM1_EFFECT_TOGGLE;

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.isAddition = 1;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Local: triggers");
    CHECK(result.isLocal == 1, "Local: isLocal=1");
    CHECK(result.effectKind == SENSOR_EFFECT_ROTATION, "Local: rotation effect");
}

/* ----------------------------------------------------------------
 *  Test F0723: Wall sensor — simple click
 *  Source: F0275 case C001 (WALL_ORNAMENT_CLICK)
 * ---------------------------------------------------------------- */
static void test_wall_ornament_click(void) {
    struct DungeonSensor_Compat sensor;
    struct WallSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(1, 0, DM1_EFFECT_TOGGLE, 0, 0, 1, 3,
                         0, 10, 12, 1);

    memset(&ctx, 0, sizeof(ctx));
    ctx.cell = 0;
    ctx.leaderHandObjectType = -1;
    ctx.leaderEmptyHanded = 1;
    ctx.leaderIndex = 0;
    ctx.sensorCountInCell = 0;

    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Wall click: triggers");
    CHECK(result.resolvedEffect == DM1_EFFECT_TOGGLE, "Wall click: TOGGLE");
    CHECK(result.targetMapX == 10, "Wall click: target X=10");
    CHECK(result.targetMapY == 12, "Wall click: target Y=12");

    /* HOLD effect on wall click -> skip (source: F0275 case C001) */
    sensor.effect = DM1_EFFECT_HOLD;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Wall click: HOLD -> skip");
}

/* ----------------------------------------------------------------
 *  Test F0723: Wall sensor — click with specific object
 *  Source: F0275 case C003 (ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT)
 * ---------------------------------------------------------------- */
static void test_wall_click_specific_object(void) {
    struct DungeonSensor_Compat sensor;
    struct WallSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    /* Sensor expects object type 8 */
    sensor = make_sensor(3, 8, DM1_EFFECT_SET, 0, 0, 0, 0,
                         0, 5, 6, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.leaderHandObjectType = 8; /* Matching */
    ctx.leaderEmptyHanded = 0;
    ctx.leaderIndex = 0;
    ctx.sensorCountInCell = 0;

    /* Source: doNotTrigger = ((data==objType(hand)) == revertEffect)
     * = (true == 0) = 0 -> triggers */
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Wall specific obj: matching object triggers");

    /* Wrong object */
    ctx.leaderHandObjectType = 9;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    /* doNotTrigger = ((8==9) == 0) = (0==0) = 1 -> no trigger */
    CHECK(result.triggered == 0, "Wall specific obj: wrong object does NOT trigger");
}


/* ----------------------------------------------------------------
 *  Test F0723: Wall sensor C003/C004 leader-hand side effects
 *  Source: MOVESENS.C F0275 lines 1412-1414, 1527-1531
 * ---------------------------------------------------------------- */
static void test_wall_click_specific_object_removed(void) {
    struct DungeonSensor_Compat sensor;
    struct WallSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    memset(&ctx, 0, sizeof(ctx));
    ctx.leaderHandObjectType = 8;
    ctx.leaderEmptyHanded = 0;
    ctx.leaderIndex = 0;
    ctx.sensorCountInCell = 0;

    sensor = make_sensor(DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT,
                         8, DM1_EFFECT_SET, 0, 0, 0, 0, 0, 5, 6, 0);
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Wall C003: matching object triggers");
    CHECK(result.leaderHandObjectRemoved == 0, "Wall C003: matching object is not consumed");

    sensor = make_sensor(DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED,
                         8, DM1_EFFECT_SET, 0, 0, 0, 0, 0, 5, 6, 0);
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Wall C004: matching object triggers");
    CHECK(result.leaderHandObjectRemoved == 1, "Wall C004: matching object is consumed");
    CHECK(result.leaderHandObjectTypeRemoved == 8, "Wall C004: consumed object type is reported");

    ctx.leaderHandObjectType = 9;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Wall C004: wrong object does not trigger");
    CHECK(result.leaderHandObjectRemoved == 0, "Wall C004: wrong object is not consumed");
}

/* ----------------------------------------------------------------
 *  Test F0723: Wall sensor C013 single-object storage + rotate
 *  Source: MOVESENS.C F0275 lines 1464-1477, 1478-1487, 1549
 * ---------------------------------------------------------------- */
static void test_wall_single_object_storage_rotate(void) {
    struct DungeonSensor_Compat sensor;
    struct WallSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(DM1_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE,
                         8, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 5, 6, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.leaderIndex = 0;
    ctx.leaderEmptyHanded = 1;
    ctx.leaderHandObjectType = -1;
    ctx.cellHasStorageObjectOfType = 0;
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Wall C013: empty hand skips when no stored object exists");

    ctx.cellHasStorageObjectOfType = 1;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Wall C013: empty hand takes matching stored object");
    CHECK(result.leaderHandObjectReceived == 1, "Wall C013: taken object is put in leader hand");
    CHECK(result.leaderHandObjectTypeReceived == 8, "Wall C013: received object type matches sensor data");
    CHECK(result.wallStorageObjectTaken == 1, "Wall C013: stored wall object is removed from cell");
    CHECK(result.wallStorageObjectType == 8, "Wall C013: stored wall object type reported");

    ctx.leaderEmptyHanded = 0;
    ctx.leaderHandObjectType = 8;
    ctx.cellHasStorageObjectOfType = 1;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Wall C013: occupied matching hand skips if cell already stores that type");

    ctx.cellHasStorageObjectOfType = 0;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Wall C013: matching leader object stores into empty cell");
    CHECK(result.leaderHandObjectRemoved == 1, "Wall C013: stored object leaves leader hand");
    CHECK(result.leaderHandObjectTypeRemoved == 8, "Wall C013: stored object type matches sensor data");
    CHECK(result.wallStorageObjectStored == 1, "Wall C013: leader object is linked into wall cell");
    CHECK(result.wallStorageObjectType == 8, "Wall C013: stored wall object type reported");

    ctx.leaderHandObjectType = 9;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Wall C013: wrong leader object type skips");

    sensor.effect = DM1_EFFECT_HOLD;
    ctx.leaderEmptyHanded = 1;
    ctx.leaderHandObjectType = -1;
    ctx.cellHasStorageObjectOfType = 1;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Wall C013 HOLD: pickup still triggers");
    CHECK(result.resolvedEffect == DM1_EFFECT_SET, "Wall C013 HOLD: pickup resolves to SET");

    ctx.leaderEmptyHanded = 0;
    ctx.leaderHandObjectType = 8;
    ctx.cellHasStorageObjectOfType = 0;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Wall C013 HOLD: store still triggers");
    CHECK(result.resolvedEffect == DM1_EFFECT_CLEAR, "Wall C013 HOLD: store resolves to CLEAR");
}

/* ----------------------------------------------------------------
 *  Test F0723: Wall sensor — champion portrait
 *  Source: F0275 case C127 (CHAMPION_PORTRAIT)
 * ---------------------------------------------------------------- */
static void test_wall_champion_portrait(void) {
    struct DungeonSensor_Compat sensor;
    struct WallSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(127, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.leaderIndex = -1; /* No leader — portraits don't need one */

    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Champion portrait: triggers even without leader");
    CHECK(result.effectKind == SENSOR_EFFECT_CHAMPION, "Champion portrait: CHAMPION effect");
}

/* ----------------------------------------------------------------
 *  Test F0723: Wall sensor — event-triggered types skip on click
 *  Source: F0275 default: goto T0275058_ProceedToNextThing
 * ---------------------------------------------------------------- */
static void test_wall_event_triggered_skip(void) {
    struct DungeonSensor_Compat sensor;
    struct WallSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    memset(&ctx, 0, sizeof(ctx));
    ctx.leaderIndex = 0;

    /* AND/OR gate (type 5) — event-triggered only */
    sensor = make_sensor(5, 0, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 0, 0, 0);
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "AND/OR gate: not triggered by click");

    /* Launcher (type 7) */
    sensor = make_sensor(7, 0, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Launcher: not triggered by click");

    /* End game (type 18) */
    sensor = make_sensor(18, 0, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "End game: not triggered by click");
}

/* ----------------------------------------------------------------
 *  Test F0724: Effect dispatch resolution
 *  Source: F0272_SENSOR_TriggerEffect (MOVESENS.C line ~1154)
 * ---------------------------------------------------------------- */
static void test_effect_dispatch(void) {
    struct DungeonSensor_Compat sensor;
    struct SensorTriggerResult_Compat result;

    /* Remote sensor targeting a door square */
    sensor = make_sensor(1, 0, DM1_EFFECT_TOGGLE, 0, 0, 0, 5,
                         0, 6, 7, 0);

    memset(&result, 0, sizeof(result));
    F0724_SENSOR_ResolveEffectDispatch_Compat(&sensor, DM1_EFFECT_TOGGLE,
        DM1_SQUARE_DOOR, 2, 3, &result);
    CHECK(result.effectKind == SENSOR_EFFECT_TOGGLE_TARGET, "Dispatch: TOGGLE -> TOGGLE_TARGET");
    CHECK(result.targetEventType == DM1_EVENT_DOOR, "Dispatch: door -> EVENT_DOOR (10)");
    CHECK(result.targetMapX == 6, "Dispatch: target X from sensor");
    CHECK(result.targetMapY == 7, "Dispatch: target Y from sensor");
    CHECK(result.delayTicks == 5, "Dispatch: delay ticks from sensor value");

    /* Local sensor */
    sensor.localEffect = 1;
    sensor.localMultiple = DM1_EFFECT_TOGGLE;
    memset(&result, 0, sizeof(result));
    F0724_SENSOR_ResolveEffectDispatch_Compat(&sensor, DM1_EFFECT_SET,
        DM1_SQUARE_CORRIDOR, 2, 3, &result);
    CHECK(result.isLocal == 1, "Dispatch: local flag set");
    CHECK(result.effectKind == SENSOR_EFFECT_ROTATION, "Dispatch: local -> ROTATION");

    /* Once-only */
    sensor.localEffect = 0;
    sensor.onceOnly = 1;
    memset(&result, 0, sizeof(result));
    F0724_SENSOR_ResolveEffectDispatch_Compat(&sensor, DM1_EFFECT_SET,
        DM1_SQUARE_WALL, 2, 3, &result);
    CHECK(result.sensorDisabled == 1, "Dispatch: once-only -> disabled");
}

/* ----------------------------------------------------------------
 *  Test F0725: Process floor square with multiple sensors
 *  Source: F0276 outer loop
 * ---------------------------------------------------------------- */
static void test_process_floor_square(void) {
    struct SensorOnSquare_Compat sensors[SENSOR_ENUM_CAPACITY];
    struct DungeonSensor_Compat sensorData[3];
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResultList_Compat results;

    memset(sensors, 0, sizeof(sensors));
    memset(sensorData, 0, sizeof(sensorData));

    /* Sensor 0: type 1 (theron), TOGGLE, remote (3,4) */
    sensorData[0] = make_sensor(1, 0, DM1_EFFECT_TOGGLE, 0, 0, 1, 0, 0, 3, 4, 0);
    /* Sensor 1: type 3 (party), SET, remote (5,6) */
    sensorData[1] = make_sensor(3, 0, DM1_EFFECT_SET, 0, 0, 0, 0, 0, 5, 6, 0);
    /* Sensor 2: type 7 (creature), TOGGLE - should NOT trigger for party */
    sensorData[2] = make_sensor(7, 0, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 7, 8, 0);

    sensors[0].found = 1; sensors[0].sensorIndex = 0;
    sensors[1].found = 1; sensors[1].sensorIndex = 1;
    sensors[2].found = 1; sensors[2].sensorIndex = 2;

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.isAddition = 1;
    ctx.partyChampionCount = 3;

    F0725_SENSOR_ProcessFloorSquare_Compat(sensors, 3, sensorData, 3, &ctx, &results);
    CHECK(results.count == 2, "Floor square: 2 of 3 sensors triggered (theron + party)");
    CHECK(results.results[0].targetMapX == 3, "Floor square: first result targets (3,4)");
    CHECK(results.results[1].targetMapX == 5, "Floor square: second result targets (5,6)");
}

/* ----------------------------------------------------------------
 *  Test F0726: Process wall click
 *  Source: F0275 outer loop
 * ---------------------------------------------------------------- */
static void test_process_wall_click(void) {
    struct SensorOnSquare_Compat sensors[SENSOR_ENUM_CAPACITY];
    struct DungeonSensor_Compat sensorData[2];
    struct WallSensorContext_Compat ctx;
    struct SensorTriggerResultList_Compat results;

    memset(sensors, 0, sizeof(sensors));
    memset(sensorData, 0, sizeof(sensorData));

    /* Sensor 0: click (type 1), TOGGLE */
    sensorData[0] = make_sensor(1, 0, DM1_EFFECT_TOGGLE, 0, 0, 1, 0, 0, 10, 11, 0);
    /* Sensor 1: launcher (type 7) -- should NOT trigger on click */
    sensorData[1] = make_sensor(7, 0, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 0, 0, 0);

    sensors[0].found = 1; sensors[0].sensorIndex = 0;
    sensors[1].found = 1; sensors[1].sensorIndex = 1;

    memset(&ctx, 0, sizeof(ctx));
    ctx.cell = 0;
    ctx.leaderEmptyHanded = 1;
    ctx.leaderHandObjectType = -1;
    ctx.leaderIndex = 0;

    F0726_SENSOR_ProcessWallClick_Compat(sensors, 2, sensorData, 2, &ctx, &results);
    CHECK(results.count == 1, "Wall click: only 1 of 2 sensors triggered");
    CHECK(results.results[0].resolvedEffect == DM1_EFFECT_TOGGLE, "Wall click: TOGGLE effect");
    CHECK(results.results[0].targetMapX == 10, "Wall click: target X=10");
}

/* ----------------------------------------------------------------
 *  Test F0726: wall sensor rotation is per-cell and deferred
 *  Source: MOVESENS.C:F0275 counts sensors per cell at 1368-1384,
 *  C011 schedules F0270 at 1429-1454, and F0271 applies the pending
 *  rotation after the full sensor scan at 1549.
 * ---------------------------------------------------------------- */
static void test_process_wall_click_rotation_per_cell_deferred(void) {
    struct SensorOnSquare_Compat sensors[SENSOR_ENUM_CAPACITY];
    struct DungeonSensor_Compat sensorData[2];
    struct WallSensorContext_Compat ctx;
    struct SensorTriggerResultList_Compat results;

    memset(sensors, 0, sizeof(sensors));
    memset(sensorData, 0, sizeof(sensorData));

    sensorData[0] = make_sensor(11, 8, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 4, 5, 0);
    sensorData[1] = make_sensor(1, 0, DM1_EFFECT_SET, 0, 0, 0, 0, 0, 6, 7, 0);

    sensors[0].found = 1; sensors[0].sensorIndex = 0; sensors[0].cell = 0;
    sensors[1].found = 1; sensors[1].sensorIndex = 1; sensors[1].cell = 1;

    memset(&ctx, 0, sizeof(ctx));
    ctx.mapX = 12;
    ctx.mapY = 9;
    ctx.cell = 0;
    ctx.leaderEmptyHanded = 0;
    ctx.leaderHandObjectType = 8;
    ctx.leaderIndex = 0;

    F0726_SENSOR_ProcessWallClick_Compat(sensors, 2, sensorData, 2, &ctx, &results);
    CHECK(results.count == 2, "Wall rotate: unrelated later cell does not suppress C011 last-in-cell trigger");
    CHECK(results.results[0].sensorIndex == 0, "Wall rotate: C011 result remains first");
    CHECK(results.results[1].sensorIndex == 1, "Wall rotate: later sensor still processes before deferred rotation");
    CHECK(results.rotationPending == 1, "Wall rotate: rotation pending");
    CHECK(results.rotationEffect == DM1_EFFECT_TOGGLE, "Wall rotate: deferred rotation effect is TOGGLE");
    CHECK(results.rotationMapX == 12, "Wall rotate: rotation source X preserved");
    CHECK(results.rotationMapY == 9, "Wall rotate: rotation source Y preserved");
    CHECK(results.rotationCell == 0, "Wall rotate: rotation cell preserved");
    CHECK(results.rotationDeferredUntilAfterResultCount == 1, "Wall rotate: rotation is scheduled after triggering sensor");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — version checker
 *  Source: F0276 case C009
 * ---------------------------------------------------------------- */
static void test_floor_version_checker(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    /* data=20 -> version <= 20 -> triggers for DM1 V1 */
    sensor = make_sensor(9, 20, DM1_EFFECT_SET, 0, 0, 0, 0, 0, 1, 1, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.isAddition = 1;
    ctx.partyOnSquare = 0;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Version checker: data=20 triggers for V1 (<=20)");

    /* data=21 -> version > 20 -> does NOT trigger */
    sensor.sensorData = 21;
    memset(&result, 0, sizeof(result));
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Version checker: data=21 does NOT trigger for V1");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — disabled sensor
 *  Source: F0276: if (M039_TYPE == C000_SENSOR_DISABLED) goto skip
 * ---------------------------------------------------------------- */
static void test_floor_disabled(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(0, 0, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 0, 0, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.isAddition = 1;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Disabled sensor: never triggers");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor — group generator skipped
 *  Source: F0276 case C006 -> goto skip
 * ---------------------------------------------------------------- */
static void test_floor_group_generator_skipped(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(6, 0, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 0, 0, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.isAddition = 1;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Group generator: skipped in floor processing");
}

/* ================================================================ */

int main(void) {
    printf("=== DM1 V1 Sensor/Trigger System Tests ===\n\n");

    test_square_type_to_event_type();
    test_hold_effect_resolution();
    test_sensor_classification();
    test_floor_theron_pressure_plate();
    test_floor_party_only_plate();
    test_floor_direction_sensitive_plate();
    test_floor_specific_object();
    test_floor_creature_only();
    test_floor_once_only();
    test_floor_revert_effect();
    test_floor_local_effect();
    test_wall_ornament_click();
    test_wall_click_specific_object();
    test_wall_click_specific_object_removed();
    test_wall_single_object_storage_rotate();
    test_wall_champion_portrait();
    test_wall_event_triggered_skip();
    test_effect_dispatch();
    test_process_floor_square();
    test_process_wall_click();
    test_process_wall_click_rotation_per_cell_deferred();
    test_floor_version_checker();
    test_floor_disabled();
    test_floor_group_generator_skipped();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
