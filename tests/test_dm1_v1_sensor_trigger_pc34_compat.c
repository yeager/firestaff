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

static unsigned short make_thing(int type, int index, int cell) {
    return (unsigned short)(((cell & 3) << 14) | ((type & 15) << 10) | (index & 0x03FF));
}

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
 *  Test F0722: Floor sensor -- party possession
 *  Source: MOVESENS.C F0276 C008 at 1710-1714 calls
 *  F0274_SENSOR_IsObjectInPartyPossession; F0274 at 1272-1306 scans
 *  living champion slots, chest contents, then leader hand.
 * ---------------------------------------------------------------- */
static void test_floor_party_possession(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(DM1_SENSOR_FLOOR_PARTY_POSSESSION, 8,
                         DM1_EFFECT_TOGGLE, 0, 0, 0, 0,
                         0, 4, 6, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.isAddition = 1;
    ctx.partyHasObjectType = 1;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Party possession: present object triggers");
    CHECK(result.resolvedEffect == DM1_EFFECT_TOGGLE, "Party possession: keeps TOGGLE effect");

    ctx.partyHasObjectType = 0;
    memset(&result, 0, sizeof(result));
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Party possession: absent object does not trigger TOGGLE");

    sensor.effect = DM1_EFFECT_HOLD;
    memset(&result, 0, sizeof(result));
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Party possession: HOLD fires with absent object");
    CHECK(result.resolvedEffect == DM1_EFFECT_CLEAR, "Party possession: absent object resolves HOLD to CLEAR");
}

/* ----------------------------------------------------------------
 *  Test F0722: Floor sensor -- party on stairs
 *  Source: MOVESENS.C F0276 case C005 at lines 1695-1702.
 * ---------------------------------------------------------------- */
static void test_floor_party_on_stairs(void) {
    struct DungeonSensor_Compat sensor;
    struct FloorSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(DM1_SENSOR_FLOOR_PARTY_ON_STAIRS, 0,
                         DM1_EFFECT_TOGGLE, 0, 0, 0, 0,
                         0, 9, 10, 1);

    memset(&ctx, 0, sizeof(ctx));
    ctx.thingType = DM1_TRIGGER_SOURCE_PARTY;
    ctx.isAddition = 1;
    ctx.squareType = DM1_SQUARE_STAIRS;

    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Party-on-stairs: party triggers on stairs square");
    CHECK(result.resolvedEffect == DM1_EFFECT_TOGGLE, "Party-on-stairs: keeps TOGGLE effect");
    CHECK(result.targetMapX == 9, "Party-on-stairs: target X preserved");
    CHECK(result.targetMapY == 10, "Party-on-stairs: target Y preserved");

    ctx.squareType = DM1_SQUARE_CORRIDOR;
    memset(&result, 0, sizeof(result));
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Party-on-stairs: party on corridor does not trigger");

    ctx.squareType = DM1_SQUARE_STAIRS;
    ctx.thingType = DM1_TRIGGER_SOURCE_CREATURE;
    memset(&result, 0, sizeof(result));
    F0722_SENSOR_EvaluateFloor_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Party-on-stairs: creature on stairs does not trigger");
}

/* ----------------------------------------------------------------
 *  Test F0718: Runtime floor C005 gate -- party on stairs only
 *  Source: MOVESENS.C F0276 case C005 at lines 1695-1702.
 * ---------------------------------------------------------------- */
static void test_floor_party_on_stairs_runtime_gate(void) {
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    unsigned char squares[4];
    unsigned short squareFirstThings[4];
    struct DungeonSensor_Compat sensors[1];
    struct SensorEffectList_Compat effects;
    int i;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(sensors, 0, sizeof(sensors));
    for (i = 0; i < 4; ++i) {
        squares[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
        squareFirstThings[i] = THING_ENDOFLIST;
    }

    map.width = 2;
    map.height = 2;
    tiles.squareData = squares;
    tiles.squareCount = 4;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;

    squares[2] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_SENSOR, 0, 0);
    sensors[0].sensorType = DM1_SENSOR_FLOOR_PARTY_ON_STAIRS;
    sensors[0].targetMapX = 1;
    sensors[0].targetMapY = 1;
    sensors[0].targetCell = 2;
    sensors[0].next = THING_ENDOFLIST;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 4;
    things.sensors = sensors;
    things.sensorCount = 1;
    things.loaded = 1;

    F0718_SENSOR_ProcessPartyEnterLeave_Compat(&dungeon, &things, 0, 1, 0,
                                               SENSOR_EVENT_WALK_ON, &effects);
    CHECK(effects.count == 0, "Runtime C005: corridor square is skipped");

    squares[2] = (unsigned char)((DUNGEON_ELEMENT_STAIRS << 5) | DUNGEON_SQUARE_MASK_THING_LIST);
    F0718_SENSOR_ProcessPartyEnterLeave_Compat(&dungeon, &things, 0, 1, 0,
                                               SENSOR_EVENT_WALK_ON, &effects);
    CHECK(effects.count == 1, "Runtime C005: stairs square triggers");
    CHECK(effects.effects[0].kind == SENSOR_EFFECT_TOGGLE_REMOTE,
          "Runtime C005: stairs effect is remote toggle");
    CHECK(effects.effects[0].sensorType == DM1_SENSOR_FLOOR_PARTY_ON_STAIRS,
          "Runtime C005: effect carries sensor type");
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
 *  Test F0723: Wall sensor C016 object exchanger
 *  Source: MOVESENS.C F0275 lines 1489-1499
 * ---------------------------------------------------------------- */
static void test_wall_object_exchanger(void) {
    struct DungeonSensor_Compat sensor;
    struct WallSensorContext_Compat ctx;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(DM1_SENSOR_WALL_OBJECT_EXCHANGER,
                         8, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 5, 6, 0);

    memset(&ctx, 0, sizeof(ctx));
    ctx.leaderIndex = 0;
    ctx.leaderEmptyHanded = 0;
    ctx.leaderHandObjectType = 8;
    ctx.sensorCountInCell = 0;
    ctx.squareHasObject = 1;
    ctx.squareObjectType = 12;
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Wall C016: matching leader object and square object trigger exchange");
    CHECK(result.resolvedEffect == DM1_EFFECT_TOGGLE, "Wall C016: keeps sensor effect");
    CHECK(result.leaderHandObjectRemoved == 1, "Wall C016: leader hand object is removed");
    CHECK(result.leaderHandObjectTypeRemoved == 8, "Wall C016: removed leader object matches sensor data");
    CHECK(result.leaderHandObjectReceived == 1, "Wall C016: square object enters leader hand");
    CHECK(result.leaderHandObjectTypeReceived == 12, "Wall C016: received object is the old square object");
    CHECK(result.wallObjectTaken == 1, "Wall C016: old square object is unlinked");
    CHECK(result.wallObjectTypeTaken == 12, "Wall C016: unlinked square object type is reported");
    CHECK(result.wallObjectStored == 1, "Wall C016: leader object is linked into clicked wall cell");
    CHECK(result.wallObjectTypeStored == 8, "Wall C016: stored wall object type is reported");

    ctx.squareHasObject = 0;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Wall C016: no square object skips exchange");

    ctx.squareHasObject = 1;
    ctx.leaderHandObjectType = 9;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Wall C016: wrong leader object type skips exchange");

    ctx.leaderHandObjectType = 8;
    ctx.sensorCountInCell = 1;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Wall C016: only the last same-cell sensor may exchange");

    sensor.effect = DM1_EFFECT_HOLD;
    ctx.sensorCountInCell = 0;
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 1, "Wall C016 HOLD: exchange still triggers");
    CHECK(result.resolvedEffect == DM1_EFFECT_SET, "Wall C016 HOLD: exchange resolves to SET");
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

    /* Countdown (type 6) */
    sensor = make_sensor(6, 2, DM1_EFFECT_TOGGLE, 0, 0, 0, 0, 0, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    F0723_SENSOR_EvaluateWall_Compat(&sensor, &ctx, &result);
    CHECK(result.triggered == 0, "Countdown: not triggered by click");

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
 *  Test F0729: Wall sensor C006 countdown/timer event
 *  Source: TIMELINE.C F0248 lines 1198-1266
 * ---------------------------------------------------------------- */
static void test_wall_countdown_event(void) {
    struct DungeonSensor_Compat sensor;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(DM1_SENSOR_WALL_COUNTDOWN, 2,
                         DM1_EFFECT_TOGGLE, 0, 0, 1, 4,
                         0, 7, 8, 2);
    F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
        &sensor, DM1_EFFECT_CLEAR, 3, 4, 1, &result);
    CHECK(result.sensorDataBefore == 2, "Countdown: records starting data");
    CHECK(result.sensorDataAfter == 1, "Countdown: CLEAR decrements data");
    CHECK(result.sensorDataChanged == 1, "Countdown: data changed on decrement");
    CHECK(result.triggered == 0, "Countdown: non-HOLD does not trigger until zero");

    sensor.sensorData = 1;
    memset(&result, 0, sizeof(result));
    F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
        &sensor, DM1_EFFECT_CLEAR, 3, 4, 1, &result);
    CHECK(result.sensorDataAfter == 0, "Countdown: last decrement reaches zero");
    CHECK(result.triggered == 1, "Countdown: non-HOLD triggers when data reaches zero");
    CHECK(result.resolvedEffect == DM1_EFFECT_TOGGLE, "Countdown: non-HOLD dispatches sensor effect");
    CHECK(result.audible == 1, "Countdown: audible flag is preserved");
    CHECK(result.delayTicks == 4, "Countdown: delay ticks come from sensor value");
    CHECK(result.targetMapX == 7, "Countdown: remote target X comes from sensor");
    CHECK(result.targetMapY == 8, "Countdown: remote target Y comes from sensor");
    CHECK(result.targetCell == 2, "Countdown: remote target cell comes from sensor");

    sensor.effect = DM1_EFFECT_HOLD;
    sensor.revertEffect = 0;
    sensor.sensorData = 1;
    memset(&result, 0, sizeof(result));
    F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
        &sensor, DM1_EFFECT_CLEAR, 3, 4, 1, &result);
    CHECK(result.triggered == 1, "Countdown HOLD: dispatches when decremented to zero");
    CHECK(result.resolvedEffect == DM1_EFFECT_SET, "Countdown HOLD: zero resolves to SET without revert");

    sensor.revertEffect = 1;
    sensor.sensorData = 1;
    memset(&result, 0, sizeof(result));
    F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
        &sensor, DM1_EFFECT_CLEAR, 3, 4, 1, &result);
    CHECK(result.resolvedEffect == DM1_EFFECT_CLEAR, "Countdown HOLD: zero resolves to CLEAR with revert");

    sensor.revertEffect = 0;
    sensor.sensorData = 511;
    memset(&result, 0, sizeof(result));
    F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
        &sensor, DM1_EFFECT_SET, 3, 4, 1, &result);
    CHECK(result.sensorDataAfter == 511, "Countdown: SET caps increment at 511");
    CHECK(result.sensorDataChanged == 0, "Countdown: capped SET reports no data change");
    CHECK(result.triggered == 1, "Countdown HOLD: still dispatches after capped SET event");
    CHECK(result.resolvedEffect == DM1_EFFECT_CLEAR, "Countdown HOLD: nonzero resolves to CLEAR without revert");

    sensor.sensorData = 0;
    memset(&result, 0, sizeof(result));
    F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
        &sensor, DM1_EFFECT_CLEAR, 3, 4, 1, &result);
    CHECK(result.sensorDataAfter == 0, "Countdown: zero counter stays zero");
    CHECK(result.triggered == 0, "Countdown: zero starting counter is skipped");

    sensor.sensorData = 1;
    sensor.localEffect = 1;
    sensor.localMultiple = DM1_EFFECT_TOGGLE;
    sensor.onceOnly = 1;
    memset(&result, 0, sizeof(result));
    F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
        &sensor, DM1_EFFECT_CLEAR, 3, 4, 1, &result);
    CHECK(result.isLocal == 1, "Countdown: local effect dispatch is reported");
    CHECK(result.effectKind == SENSOR_EFFECT_ROTATION, "Countdown: local effect maps to rotation kind");
    CHECK(result.targetMapX == 3, "Countdown: local effect uses source X");
    CHECK(result.targetMapY == 4, "Countdown: local effect uses source Y");
    CHECK(result.targetCell == 1, "Countdown: local effect uses event cell");
    CHECK(result.sensorDisabled == 1, "Countdown: once-only disables only on dispatch");
}


/* ----------------------------------------------------------------
 *  Test F0730: wall projectile launcher C008 single explosion
 *  Source: TIMELINE.C F0247 lines 1068-1077, 1114-1123 and
 *  F0248 lines 1312-1316.
 * ---------------------------------------------------------------- */
static void test_wall_projectile_launcher_single_explosion(void) {
    struct DungeonSensor_Compat sensor;
    struct ProjectileLauncherContext_Compat ctx;
    struct ProjectileLauncherResult_Compat result;

    sensor = make_sensor(DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION,
                         3, DM1_EFFECT_TOGGLE, 0, 1, 0, 0,
                         0, 0, 0, 0);
    sensor.localMultiple = (unsigned short)((4 << 8) | 55);
    memset(&ctx, 0, sizeof(ctx));
    ctx.randomBit = 1;
    ctx.newObjectThings[0] = THING_NONE;
    ctx.newObjectThings[1] = THING_NONE;

    F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
        &sensor, 1, 10, 20, 1, &ctx, &result);

    CHECK(result.triggered == 1, "Launcher C008: matching wall event triggers");
    CHECK(result.sensorDisabled == 1, "Launcher C008: once-only disables after event");
    CHECK(result.launchSingleProjectile == 1, "Launcher C008: single launcher uses one projectile");
    CHECK(result.launchCount == 1, "Launcher C008: creates one projectile");
    CHECK(result.launches[0].associatedThing == (unsigned short)(DM1_THING_FIRST_EXPLOSION + 3),
          "Launcher C008: explosion thing comes from data + first explosion");
    CHECK(result.launches[0].mapX == 11 && result.launches[0].mapY == 20,
          "Launcher C008: launch square is one step in event cell direction");
    CHECK(result.launches[0].cell == 0, "Launcher C008: single projectile cell applies random bit");
    CHECK(result.launches[0].direction == 1, "Launcher C008: direction is event cell");
    CHECK(result.launches[0].kineticEnergy == 55, "Launcher C008: kinetic energy is localMultiple low byte");
    CHECK(result.launches[0].stepEnergy == 4, "Launcher C008: PC34 step energy is localMultiple high byte");
    CHECK(result.launches[0].attack == DM1_PROJECTILE_LAUNCHER_ATTACK,
          "Launcher C008: attack is fixed at 100");
}

/* ----------------------------------------------------------------
 *  Test F0730: wall projectile launcher C010 double explosion
 *  Source: TIMELINE.C F0247 lines 1073-1077, 1125-1130.
 * ---------------------------------------------------------------- */
static void test_wall_projectile_launcher_double_explosion(void) {
    struct DungeonSensor_Compat sensor;
    struct ProjectileLauncherContext_Compat ctx;
    struct ProjectileLauncherResult_Compat result;

    sensor = make_sensor(DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION,
                         7, DM1_EFFECT_TOGGLE, 0, 0, 0, 0,
                         0, 0, 0, 0);
    sensor.localMultiple = (unsigned short)((9 << 8) | 80);
    memset(&ctx, 0, sizeof(ctx));

    F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
        &sensor, 0, 4, 4, 0, &ctx, &result);

    CHECK(result.launchCount == 2, "Launcher C010: double explosion creates two projectiles");
    CHECK(result.launchSingleProjectile == 0, "Launcher C010: double launcher does not randomize cell");
    CHECK(result.launches[0].associatedThing == (unsigned short)(DM1_THING_FIRST_EXPLOSION + 7),
          "Launcher C010: first projectile uses explosion thing");
    CHECK(result.launches[1].associatedThing == (unsigned short)(DM1_THING_FIRST_EXPLOSION + 7),
          "Launcher C010: second projectile uses same explosion thing");
    CHECK(result.launches[0].mapX == 4 && result.launches[0].mapY == 3,
          "Launcher C010: north-facing launcher moves to y-1");
    CHECK(result.launches[0].cell == 2 && result.launches[1].cell == 3,
          "Launcher C010: double cells are opposite and next cell");
    CHECK(result.launches[1].stepEnergy == 9, "Launcher C010: second projectile keeps step energy");
}

/* ----------------------------------------------------------------
 *  Test F0730: wall projectile launcher C007/C009 new objects
 *  Source: TIMELINE.C F0247 lines 1105-1115.
 * ---------------------------------------------------------------- */
static void test_wall_projectile_launcher_new_objects(void) {
    struct DungeonSensor_Compat sensor;
    struct ProjectileLauncherContext_Compat ctx;
    struct ProjectileLauncherResult_Compat result;
    unsigned short arrow = make_thing(THING_TYPE_WEAPON, 3, 0);

    sensor = make_sensor(DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ,
                         12, DM1_EFFECT_TOGGLE, 0, 0, 0, 0,
                         0, 0, 0, 0);
    sensor.localMultiple = (unsigned short)((2 << 8) | 33);
    memset(&ctx, 0, sizeof(ctx));
    ctx.randomBit = 1;
    ctx.newObjectThings[0] = arrow;
    ctx.newObjectThings[1] = THING_NONE;

    F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
        &sensor, 2, 8, 9, 2, &ctx, &result);

    CHECK(result.triggered == 1, "Launcher C009: matching event triggers");
    CHECK(result.launchCount == 1, "Launcher C009: missing second object collapses to one projectile");
    CHECK(result.launchSingleProjectile == 1, "Launcher C009: collapsed double uses single-launch cell randomization");
    CHECK(result.launches[0].associatedThing == arrow, "Launcher C009: uses first F0167 object");
    CHECK(result.launches[0].mapX == 8 && result.launches[0].mapY == 10,
          "Launcher C009: south-facing launcher moves to y+1");
    CHECK(result.launches[0].cell == 1, "Launcher C009: collapsed single cell includes random bit");
    CHECK(result.unlinkCount == 0, "Launcher C009: generated objects are not square unlinks");

    sensor.sensorType = DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ;
    sensor.onceOnly = 1;
    ctx.newObjectThings[0] = THING_NONE;
    memset(&result, 0, sizeof(result));
    F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
        &sensor, 2, 8, 9, 2, &ctx, &result);
    CHECK(result.triggered == 1, "Launcher C007: event still matches with no generated object");
    CHECK(result.sensorDisabled == 1, "Launcher C007: once-only disables even when F0247 returns early");
    CHECK(result.launchCount == 0, "Launcher C007: no generated object means no projectile create");
}

/* ----------------------------------------------------------------
 *  Test F0730: wall projectile launcher C014/C015 square objects
 *  Source: TIMELINE.C F0247 lines 1079-1103.
 * ---------------------------------------------------------------- */
static void test_wall_projectile_launcher_square_objects(void) {
    struct DungeonSensor_Compat sensor;
    struct ProjectileLauncherContext_Compat ctx;
    struct ProjectileLauncherSquareThing_Compat things[4];
    struct ProjectileLauncherResult_Compat result;
    unsigned short ignoredSensor = make_thing(THING_TYPE_SENSOR, 1, 3);
    unsigned short firstObj = make_thing(THING_TYPE_WEAPON, 4, 0);
    unsigned short ignoredObj = make_thing(THING_TYPE_ARMOUR, 5, 2);
    unsigned short secondObj = make_thing(THING_TYPE_JUNK, 6, 3);

    things[0].thing = ignoredSensor; things[0].cell = 3; things[0].thingType = THING_TYPE_SENSOR;
    things[1].thing = firstObj; things[1].cell = 0; things[1].thingType = THING_TYPE_WEAPON;
    things[2].thing = ignoredObj; things[2].cell = 2; things[2].thingType = THING_TYPE_ARMOUR;
    things[3].thing = secondObj; things[3].cell = 3; things[3].thingType = THING_TYPE_JUNK;

    sensor = make_sensor(DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ,
                         0, DM1_EFFECT_TOGGLE, 0, 0, 0, 0,
                         0, 0, 0, 0);
    sensor.localMultiple = (unsigned short)((6 << 8) | 44);
    memset(&ctx, 0, sizeof(ctx));
    ctx.squareThings = things;
    ctx.squareThingCount = 4;

    F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
        &sensor, 3, 12, 6, 3, &ctx, &result);

    CHECK(result.launchCount == 2, "Launcher C015: two matching square objects create two projectiles");
    CHECK(result.unlinkCount == 2, "Launcher C015: selected square objects are unlinked");
    CHECK(result.unlinkThings[0] == firstObj && result.unlinkThings[1] == secondObj,
          "Launcher C015: square object selection follows thing-list order and cell filter");
    CHECK(result.launches[0].mapX == 11 && result.launches[0].mapY == 6,
          "Launcher C015: west-facing launcher moves to x-1");
    CHECK(result.launches[0].cell == 1 && result.launches[1].cell == 2,
          "Launcher C015: double square-object cells are opposite and next");

    sensor.sensorType = DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ;
    ctx.randomBit = 1;
    memset(&result, 0, sizeof(result));
    F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
        &sensor, 3, 12, 6, 3, &ctx, &result);
    CHECK(result.launchCount == 1, "Launcher C014: single square-object launcher creates one projectile");
    CHECK(result.unlinkCount == 1 && result.unlinkThings[0] == firstObj,
          "Launcher C014: single square-object launcher unlinks one object");
    CHECK(result.launches[0].cell == 2, "Launcher C014: single square-object cell includes random bit");

    memset(&result, 0, sizeof(result));
    F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
        &sensor, 0, 12, 6, 3, &ctx, &result);
    CHECK(result.triggered == 0, "Launcher C014: sensor cell must match wall event cell");
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
 *  Test F0729: Wall AND/OR gate sensor C005 event handling
 *  Source: TIMELINE.C F0248 lines 1268-1309; DEFS.H M042/M043
 *  lines 1298-1299.
 * ---------------------------------------------------------------- */
static void test_wall_and_or_gate_event(void) {
    struct DungeonSensor_Compat sensor;
    struct SensorTriggerResult_Compat result;

    /* current=0001, reference=0011. SET on cell 1 makes current match. */
    sensor = make_sensor(DM1_SENSOR_WALL_AND_OR_GATE,
                         0x31, DM1_EFFECT_TOGGLE, 0, 0, 1, 0, 0, 6, 7, 2);
    F0730_SENSOR_EvaluateWallAndOrGateEvent_Compat(
        &sensor, 1, DM1_EFFECT_SET, DM1_SQUARE_WALL, 4, 5, &result);
    CHECK(result.triggered == 1, "Wall C005: SET matching bit triggers non-HOLD gate");
    CHECK(result.sensorDataAfter == 0x33, "Wall C005: low-nibble current mask updated");
    CHECK(result.gateCurrentMask == 0x3, "Wall C005: current mask is low nibble");
    CHECK(result.gateReferenceMask == 0x3, "Wall C005: reference mask is high nibble");
    CHECK(result.gateTriggerSetEffect == 1, "Wall C005: current==reference triggers set effect");
    CHECK(result.resolvedEffect == DM1_EFFECT_TOGGLE, "Wall C005: non-HOLD dispatches sensor effect");
    CHECK(result.targetMapX == 6, "Wall C005: target X comes from sensor remote target");
    CHECK(result.targetCell == 2, "Wall C005: wall targets preserve sensor target cell");

    /* TOGGLE clears an already-set bit, so current no longer matches. */
    sensor.sensorData = 0x33;
    memset(&result, 0, sizeof(result));
    F0730_SENSOR_EvaluateWallAndOrGateEvent_Compat(
        &sensor, 1, DM1_EFFECT_TOGGLE, DM1_SQUARE_WALL, 4, 5, &result);
    CHECK(result.triggered == 0, "Wall C005: non-HOLD does not dispatch when mask differs");
    CHECK(result.sensorDataAfter == 0x31, "Wall C005: TOGGLE clears active event-cell bit");
    CHECK(result.gateTriggerSetEffect == 0, "Wall C005: mismatch clears trigger-set flag");

    /* HOLD always dispatches SET/CLEAR from the comparison, after revert. */
    sensor.sensorData = 0x33;
    sensor.effect = DM1_EFFECT_HOLD;
    sensor.revertEffect = 1;
    memset(&result, 0, sizeof(result));
    F0730_SENSOR_EvaluateWallAndOrGateEvent_Compat(
        &sensor, 0, DM1_EFFECT_SET, DM1_SQUARE_DOOR, 4, 5, &result);
    CHECK(result.triggered == 1, "Wall C005 HOLD: dispatches even when reverted comparison is false");
    CHECK(result.resolvedEffect == DM1_EFFECT_CLEAR, "Wall C005 HOLD: false comparison resolves to CLEAR");
    CHECK(result.targetEventType == DM1_EVENT_DOOR, "Wall C005 HOLD: dispatch resolves target square event type");
    CHECK(result.targetCell == 0, "Wall C005 HOLD: non-wall targets force northwest cell");
}

/* ----------------------------------------------------------------
 *  Test F0731: Wall end-game sensor C018 event handling
 *  Source: TIMELINE.C F0248 lines 1317-1339; DEFS.H line 1283.
 * ---------------------------------------------------------------- */
static void test_wall_endgame_event(void) {
    struct DungeonSensor_Compat sensor;
    struct SensorTriggerResult_Compat result;

    sensor = make_sensor(DM1_SENSOR_WALL_END_GAME,
                         12, DM1_EFFECT_TOGGLE, 0, 1, 1, 3,
                         0, 6, 7, 2);

    F0731_SENSOR_EvaluateWallEndGameEvent_Compat(
        &sensor, 0, DM1_EFFECT_CLEAR, 3, &result);
    CHECK(result.triggered == 1, "Wall C018: event triggers end-game sensor");
    CHECK(result.effectKind == SENSOR_EFFECT_END_GAME, "Wall C018: reports END_GAME effect kind");
    CHECK(result.resolvedEffect == DM1_EFFECT_NONE, "Wall C018: ignores sensor remote effect");
    CHECK(result.sensorDisabled == 0, "Wall C018: once-only does not disable in F0248 branch");
    CHECK(result.endGameGameWon == 1, "Wall C018: marks game won");
    CHECK(result.endGameRestartGameAllowedCleared == 1, "Wall C018: clears restart permission on DM1 media branch");
    CHECK(result.endGamePresentationRequested == 1, "Wall C018: requests end-game presentation");
    CHECK(result.endGameDelayTicks == 180, "Wall C018: optional delay is 60 * sensor value");

    memset(&result, 0, sizeof(result));
    F0731_SENSOR_EvaluateWallEndGameEvent_Compat(
        &sensor, 2, DM1_EFFECT_SET, 1, &result);
    CHECK(result.triggered == 1, "Wall C018: unlike projectiles, mismatched event cell still triggers");

    sensor.sensorType = DM1_SENSOR_WALL_COUNTDOWN;
    memset(&result, 0, sizeof(result));
    F0731_SENSOR_EvaluateWallEndGameEvent_Compat(
        &sensor, 0, DM1_EFFECT_CLEAR, 0, &result);
    CHECK(result.triggered == 0, "Wall C018: other wall sensor types are skipped");
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
    test_floor_party_possession();
    test_floor_party_on_stairs();
    test_floor_party_on_stairs_runtime_gate();
    test_wall_ornament_click();
    test_wall_click_specific_object();
    test_wall_click_specific_object_removed();
    test_wall_single_object_storage_rotate();
    test_wall_object_exchanger();
    test_wall_champion_portrait();
    test_wall_event_triggered_skip();
    test_wall_countdown_event();
    test_wall_projectile_launcher_single_explosion();
    test_wall_projectile_launcher_double_explosion();
    test_wall_projectile_launcher_new_objects();
    test_wall_projectile_launcher_square_objects();
    test_effect_dispatch();
    test_wall_and_or_gate_event();
    test_wall_endgame_event();
    test_process_floor_square();
    test_process_wall_click();
    test_process_wall_click_rotation_per_cell_deferred();
    test_floor_version_checker();
    test_floor_disabled();
    test_floor_group_generator_skipped();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
