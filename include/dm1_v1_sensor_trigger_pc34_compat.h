#ifndef REDMCSB_DM1_V1_SENSOR_TRIGGER_PC34_COMPAT_H
#define REDMCSB_DM1_V1_SENSOR_TRIGGER_PC34_COMPAT_H

/*
 * DM1 V1 Sensor & Trigger System — source-locked to ReDMCSB.
 *
 * Source audit:
 *   MOVESENS.C — F0268-F0276: sensor processing
 *     F0268_SENSOR_AddEvent (line ~1000): creates timed event from sensor trigger
 *     F0270_SENSOR_TriggerLocalEffect (line ~1081): local effect (rotation, XP)
 *     F0271_SENSOR_ProcessRotationEffect (line ~1100): deferred sensor rotation
 *     F0272_SENSOR_TriggerEffect (line ~1154): dispatch: local vs remote target
 *     F0275_SENSOR_IsTriggeredByClickOnWall (line ~1309): wall switch click handling
 *     F0276_SENSOR_ProcessThingAdditionOrRemoval (line ~1553): floor sensor processing
 *   DEFS.H:
 *     Sensor struct (line ~1191): union { Remote, Local } with Type_Data bitfield
 *     Sensor type constants (lines 1256-1284): C000..C018, C127
 *     Effect constants (lines 1288-1295): SET/CLEAR/TOGGLE/HOLD
 *     M039_TYPE, M040_DATA macros (lines 1295-1296)
 *   DATA.C:
 *     G0059_auc_Graphic562_SquareTypeToEventType[7] (line ~470): maps square type -> event type
 *
 * Design: Pure functions. NO UI, NO rendering, NO world mutation.
 * The caller applies effects to the game state.
 */

#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"

/* ================================================================
 *  Sensor type constants — source-locked to DEFS.H lines 1256-1284
 * ================================================================ */

/* Floor sensor types (used in F0276 floor-context switch) */
#define DM1_SENSOR_DISABLED                             0
#define DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT   1
#define DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE          2
#define DM1_SENSOR_FLOOR_PARTY                          3
#define DM1_SENSOR_FLOOR_OBJECT                         4
#define DM1_SENSOR_FLOOR_PARTY_ON_STAIRS                5
#define DM1_SENSOR_FLOOR_GROUP_GENERATOR                6
#define DM1_SENSOR_FLOOR_CREATURE                       7
#define DM1_SENSOR_FLOOR_PARTY_POSSESSION               8
#define DM1_SENSOR_FLOOR_VERSION_CHECKER                9

/* Wall sensor types (used in F0275 wall-context switch) */
#define DM1_SENSOR_WALL_ORNAMENT_CLICK                              1
#define DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT              2
#define DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT         3
#define DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED 4
#define DM1_SENSOR_WALL_AND_OR_GATE                                 5
#define DM1_SENSOR_WALL_COUNTDOWN                                   6
#define DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ                7
#define DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION              8
#define DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ                9
#define DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION             10
#define DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE                  11
#define DM1_SENSOR_WALL_OBJECT_GENERATOR_ROTATE                    12
#define DM1_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE               13
#define DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ            14
#define DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ            15
#define DM1_SENSOR_WALL_OBJECT_EXCHANGER                           16
#define DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_REMOVE_SENSOR            17
#define DM1_SENSOR_WALL_END_GAME                                   18
#define DM1_SENSOR_WALL_CHAMPION_PORTRAIT                         127

/* ================================================================
 *  Effect constants — source-locked to DEFS.H lines 1288-1295
 * ================================================================ */
#define DM1_EFFECT_NONE                  (-1)
#define DM1_EFFECT_SET                    0
#define DM1_EFFECT_CLEAR                  1
#define DM1_EFFECT_TOGGLE                 2
#define DM1_EFFECT_HOLD                   3
#define DM1_EFFECT_ADD_300XP_STEAL_SKILL 10

/* ================================================================
 *  Square types and event types
 *  Source: DATA.C G0059_auc_Graphic562_SquareTypeToEventType[7]
 * ================================================================ */
#define DM1_SQUARE_WALL       0
#define DM1_SQUARE_CORRIDOR   1
#define DM1_SQUARE_PIT        2
#define DM1_SQUARE_STAIRS     3
#define DM1_SQUARE_DOOR       4
#define DM1_SQUARE_TELEPORTER 5
#define DM1_SQUARE_FAKEWALL   6

#define DM1_EVENT_NONE         0
#define DM1_EVENT_CORRIDOR     5
#define DM1_EVENT_WALL         6
#define DM1_EVENT_FAKEWALL     7
#define DM1_EVENT_TELEPORTER   8
#define DM1_EVENT_PIT          9
#define DM1_EVENT_DOOR        10

/* ================================================================
 *  Trigger source classification
 * ================================================================ */
#define DM1_TRIGGER_SOURCE_PARTY      0
#define DM1_TRIGGER_SOURCE_CREATURE   1
#define DM1_TRIGGER_SOURCE_OBJECT     2
#define DM1_TRIGGER_SOURCE_PROJECTILE 3
#define DM1_TRIGGER_SOURCE_WALL_CLICK 4

/* ================================================================
 *  Extended effect types
 * ================================================================ */
#define SENSOR_EFFECT_TOGGLE_TARGET   4
#define SENSOR_EFFECT_SET_TARGET      5
#define SENSOR_EFFECT_CLEAR_TARGET    6
#define SENSOR_EFFECT_ROTATION        7
#define SENSOR_EFFECT_ADD_XP          8
#define SENSOR_EFFECT_DISABLE_SENSOR  9
#define SENSOR_EFFECT_GENERATOR      10
#define SENSOR_EFFECT_LAUNCHER       11
#define SENSOR_EFFECT_END_GAME       12
#define SENSOR_EFFECT_CHAMPION       13

/* ================================================================
 *  Floor sensor context
 * ================================================================ */
struct FloorSensorContext_Compat {
    int thingType;
    int objectType;
    int partyOnSquare;
    int squareHasObject;
    int squareHasGroup;
    int squareHasSameTypeObj;
    int squareHasDiffTypeObj;
    int squareType;
    int partyDirection;
    int partyChampionCount;
    int partyHasObjectType;
    int isAddition;
};

/* ================================================================
 *  Wall sensor context
 * ================================================================ */
struct WallSensorContext_Compat {
    int mapX;
    int mapY;
    int cell;
    int leaderHandObjectType;
    int leaderEmptyHanded;
    int leaderIndex;
    int sensorCountInCell;
    int cellHasStorageObjectOfType;
    int squareHasObject;
    int squareObjectType;
};

/* ================================================================
 *  Sensor trigger result
 * ================================================================ */
struct SensorTriggerResult_Compat {
    int triggered;
    int effectKind;
    int resolvedEffect;
    int targetSquareType;
    int targetEventType;
    int targetMapX;
    int targetMapY;
    int targetCell;
    int isLocal;
    int localEffectValue;
    int sensorDisabled;
    int audible;
    int sensorIndex;
    int delayTicks;
    int leaderHandObjectRemoved;
    int leaderHandObjectTypeRemoved;
    int leaderHandObjectReceived;
    int leaderHandObjectTypeReceived;
    int wallStorageObjectTaken;
    int wallStorageObjectStored;
    int wallStorageObjectType;
    int wallObjectTaken;
    int wallObjectTypeTaken;
    int wallObjectStored;
    int wallObjectTypeStored;
    int sensorDataBefore;
    int sensorDataAfter;
    int sensorDataChanged;
    int gateBitMask;
    int gateCurrentMask;
    int gateReferenceMask;
    int gateTriggerSetEffect;
};

#define SENSOR_TRIGGER_RESULT_MAX 16

struct SensorTriggerResultList_Compat {
    int count;
    struct SensorTriggerResult_Compat results[SENSOR_TRIGGER_RESULT_MAX];
    int rotationPending;
    int rotationEffect;
    int rotationMapX;
    int rotationMapY;
    int rotationCell;
    int rotationDeferredUntilAfterResultCount;
};

/* ================================================================
 *  Projectile launcher event model (TIMELINE.C F0247/F0248)
 * ================================================================ */

#define DM1_PROJECTILE_LAUNCHER_MAX_LAUNCHES 2
#define DM1_PROJECTILE_LAUNCHER_ATTACK       100
#define DM1_THING_FIRST_EXPLOSION            0xFF80u

struct ProjectileLauncherSquareThing_Compat {
    unsigned short thing;
    int cell;       /* 0..3; pass -1 to derive from THING bits */
    int thingType;  /* 0..15; pass -1 to derive from THING bits */
};

struct ProjectileLauncherContext_Compat {
    int randomBit;  /* M005_RANDOM(2), used only after single-launch collapse */
    unsigned short newObjectThings[DM1_PROJECTILE_LAUNCHER_MAX_LAUNCHES];
    const struct ProjectileLauncherSquareThing_Compat* squareThings;
    int squareThingCount;
};

struct ProjectileLauncherLaunch_Compat {
    int valid;
    unsigned short associatedThing;
    int mapX;
    int mapY;
    int cell;
    int direction;
    int kineticEnergy;
    int attack;
    int stepEnergy;
};

struct ProjectileLauncherResult_Compat {
    int triggered;
    int sensorDisabled;
    int launcherType;
    int launchSingleProjectile;
    int projectileCellBase;
    int launchCount;
    struct ProjectileLauncherLaunch_Compat launches[DM1_PROJECTILE_LAUNCHER_MAX_LAUNCHES];
    int unlinkCount;
    unsigned short unlinkThings[DM1_PROJECTILE_LAUNCHER_MAX_LAUNCHES];
};

/* ================================================================
 *  API Functions
 * ================================================================ */

int F0720_SENSOR_ClassifyFloorType_Compat(int sensorType, int* outIsFloorSensor);
int F0721_SENSOR_ClassifyWallType_Compat(int sensorType, int* outIsWallSensor);

int F0722_SENSOR_EvaluateFloor_Compat(
    const struct DungeonSensor_Compat* sensor,
    const struct FloorSensorContext_Compat* ctx,
    struct SensorTriggerResult_Compat* outResult);

int F0723_SENSOR_EvaluateWall_Compat(
    const struct DungeonSensor_Compat* sensor,
    const struct WallSensorContext_Compat* ctx,
    struct SensorTriggerResult_Compat* outResult);

int F0724_SENSOR_ResolveEffectDispatch_Compat(
    const struct DungeonSensor_Compat* sensor,
    int resolvedEffect,
    int targetSquareType,
    int sensorMapX,
    int sensorMapY,
    struct SensorTriggerResult_Compat* outResult);

int F0725_SENSOR_ProcessFloorSquare_Compat(
    const struct SensorOnSquare_Compat* sensors,
    int sensorCount,
    const struct DungeonSensor_Compat* sensorData,
    int sensorDataCount,
    const struct FloorSensorContext_Compat* ctx,
    struct SensorTriggerResultList_Compat* outList);

int F0726_SENSOR_ProcessWallClick_Compat(
    const struct SensorOnSquare_Compat* sensors,
    int sensorCount,
    const struct DungeonSensor_Compat* sensorData,
    int sensorDataCount,
    const struct WallSensorContext_Compat* ctx,
    struct SensorTriggerResultList_Compat* outList);

int F0727_SENSOR_SquareTypeToEventType_Compat(int squareType);

int F0728_SENSOR_ResolveHoldEffect_Compat(int effect, int triggerActive);

int F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
    const struct DungeonSensor_Compat* sensor,
    int eventEffect,
    int sensorMapX,
    int sensorMapY,
    int sensorCell,
    struct SensorTriggerResult_Compat* outResult);

int F0730_SENSOR_EvaluateWallAndOrGateEvent_Compat(
    const struct DungeonSensor_Compat* sensor,
    int eventCell,
    int eventEffect,
    int targetSquareType,
    int sensorMapX,
    int sensorMapY,
    struct SensorTriggerResult_Compat* outResult);

int F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
    const struct DungeonSensor_Compat* sensor,
    int sensorCell,
    int eventMapX,
    int eventMapY,
    int eventCell,
    const struct ProjectileLauncherContext_Compat* ctx,
    struct ProjectileLauncherResult_Compat* outResult);

#endif
