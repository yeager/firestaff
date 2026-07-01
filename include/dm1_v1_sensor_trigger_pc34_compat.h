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
 *   TIMELINE.C:
 *     F0248 lines 1317-1339: C018 end-game wall event sensor branch
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
    int endGameGameWon;
    int endGameRestartGameAllowedCleared;
    int endGamePresentationRequested;
    int endGameDelayTicks;
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
 *  Floor sensor possession context (F0274_SENSOR_IsObjectInPartyPossession)
 *
 *  Source: MOVESENS.C lines 1234-1306.  The C008 floor sensor type
 *  ("DM1_SENSOR_FLOOR_PARTY_POSSESSION") asks "does the party carry an
 *  object whose type matches sensorData?" before toggling its remote
 *  target.  The source-locked scan walks living champion slots from
 *  C00_SLOT_READY_HAND through C29_SLOT_BACKPACK_17, recursing into
 *  any closed chest found in those slots, and finally falling back to
 *  the leader's hand object (G4055_s_LeaderHandObject).
 *
 *  Source-locked scan order:
 *    1. For each living champion (CurrentHealth > 0):
 *       1a. Walk slots [READY_HAND..CHEST_1) looking for objectType match
 *           and recurse into any C144_ICON_CONTAINER_CHEST_CLOSED container.
 *       1b. The leader's hand slot is also reachable via this path.
 *    2. If the leader hand object has not yet been inspected
 *       (L0748_B_LeaderHandObjectProcessed), inspect it once.
 *
 *  ReDMCSB CHAMPION.C slot counts:
 *    - C00_SLOT_READY_HAND = 0
 *    - C30_SLOT_CHEST_1    = 30 (sentinel; slots 0..29 are inspected)
 *  The champion_state_pc34_compat defines CHAMPION_SLOT_COUNT = 30 with
 *  CHAMPION_SLOT_HEAD..CHAMPION_SLOT_HAND_LEFT/RIGHT covering 0..20 and
 *  CHAMPION_SLOT_BACKPACK_9..17 covering 21..29, mapping cleanly onto the
 *  ReDMCSB ready-hand..chest-1 range.
 * ================================================================ */
#define DM1_SENSOR_POSSESSION_SLOT_FIRST  0   /* ReDMCSB C00_SLOT_READY_HAND */
#define DM1_SENSOR_POSSESSION_SLOT_LAST   30  /* ReDMCSB C30_SLOT_CHEST_1 (exclusive) */
#define DM1_SENSOR_POSSESSION_MAX_CHAMPIONS 4
#define DM1_SENSOR_POSSESSION_MAX_SCAN_STEPS 64

/* Lightweight snapshot of the party for the F0274 scan.  The caller
 * (typically a tick orchestrator or a regression test) supplies either
 * a precomputed PartyState_Compat-style view or per-champion slot arrays
 * + the leader-hand-object thing ref.  The helper does not mutate. */
struct PartyPossessionContext_Compat {
    /* Per-champion slot scan data.  Champion slots are stored as 30
     * THING references (0..29).  Use CHAMPION_SLOT_HEAD..CHAMPION_SLOT_BACKPACK_17
     * indices from memory_champion_state_pc34_compat.h. */
    int championCount;
    unsigned short championSlots[DM1_SENSOR_POSSESSION_MAX_CHAMPIONS][DM1_SENSOR_POSSESSION_SLOT_LAST];
    /* CurrentHealth > 0 marks a living champion.  Dead champions are
     * skipped, matching MOVESENS.C lines 1272-1273. */
    int championAlive[DM1_SENSOR_POSSESSION_MAX_CHAMPIONS];
    /* THING_ENDOFLIST ends each container's slot chain.  The helper
     * recurses into C144_ICON_CONTAINER_CHEST_CLOSED icon types, so
     * callers should expose the world->thing data needed to read
     * container->slot and follow thing chains. */
    const struct DungeonThings_Compat* things;
    /* ReDMCSB G4055_s_LeaderHandObject.Thing — the leader's hand object,
     * scanned last if the slot scan did not already touch it. */
    unsigned short leaderHandThing;
};

/* ================================================================
 *  Pressure plate actuator dispatch (F0268_SENSOR_AddEvent + actuator)
 *
 *  Source: MOVESENS.C F0268_SENSOR_AddEvent line ~1000 maps a sensor
 *  effect to a timed event using G0059_auc_Graphic562_SquareTypeToEventType.
 *  In ReDMCSB the actuator (door / pit / fake wall / teleporter) lives
 *  in the door mechanics / wall event subsystem and consumes the event
 *  on the target square.
 *
 *  Firestaff models this contract as a structured SensorActuatorDispatch:
 *    - target event type  (door=10, pit=9, fakewall=7, wall=6, teleporter=8)
 *    - target square type (0..6 -> DM1_SQUARE_*)
 *    - target action      (open/close/toggle)
 *    - delay ticks        (sensor->value passed through F0268 queueing)
 *
 *  The helper takes a sensor trigger result plus the target square's
 *  square-type and returns the structured dispatch.  Pure function.
 * ================================================================ */
#define DM1_ACTUATOR_ACTION_NONE    0
#define DM1_ACTUATOR_ACTION_OPEN    1
#define DM1_ACTUATOR_ACTION_CLOSE   2
#define DM1_ACTUATOR_ACTION_TOGGLE  3

#define DM1_ACTUATOR_DELAY_UNSCALED 0   /* F0268 schedules event at gameTick+1 */

/* Bitmask of actuator kinds this dispatch is meaningful for.  ReDMCSB
 * C008/C007 floor sensors in front of a door toggle the door; C008 in
 * front of a pit toggles the pit; etc.  The helper reports each. */
#define DM1_ACTUATOR_KIND_DOOR        0x01
#define DM1_ACTUATOR_KIND_PIT         0x02
#define DM1_ACTUATOR_KIND_FAKEWALL    0x04
#define DM1_ACTUATOR_KIND_WALL        0x08
#define DM1_ACTUATOR_KIND_TELEPORTER  0x10
#define DM1_ACTUATOR_KIND_CORRIDOR    0x20

struct SensorActuatorDispatch_Compat {
    int valid;             /* 1 if dispatch applies to a real actuator */
    int targetMapX;        /* Sensor remote target X (targetMapX) */
    int targetMapY;        /* Sensor remote target Y (targetMapY) */
    int targetCell;        /* Sensor remote target cell */
    int targetSquareType;  /* DM1_SQUARE_DOOR/PIT/... at the target */
    int targetEventType;   /* DM1_EVENT_DOOR/PIT/... */
    int actuatorKindMask;  /* OR of DM1_ACTUATOR_KIND_* relevant bits */
    int action;            /* DM1_ACTUATOR_ACTION_* */
    int resolvedEffect;    /* DM1_EFFECT_SET/CLEAR/TOGGLE/HOLD-resolved */
    int delayTicks;        /* Delay (from sensor->value) before actuator runs */
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

int F0731_SENSOR_EvaluateWallEndGameEvent_Compat(
    const struct DungeonSensor_Compat* sensor,
    int sensorCell,
    int eventEffect,
    int eventCell,
    struct SensorTriggerResult_Compat* outResult);

/* ----------------------------------------------------------------
 *  F0274_SENSOR_IsObjectInPartyPossession_Compat
 *
 *  Source-locked replica of MOVESENS.C F0274 lines 1234-1306.  Returns
 *  1 if any living champion carries an object whose icon type equals
 *  objectType, where the scan walks every champion slot in [READY_HAND,
 *  CHEST_1) and recurses into any C144_ICON_CONTAINER_CHEST_CLOSED
 *  container found.  The leader hand object (ReDMCSB
 *  G4055_s_LeaderHandObject) is also inspected exactly once if the slot
 *  scan did not already see it, matching L0748_B_LeaderHandObjectProcessed.
 *
 *  Used by F0722_SENSOR_EvaluateFloor_Compat case C008
 *  (DM1_SENSOR_FLOOR_PARTY_POSSESSION) and by the runtime
 *  F0718_SENSOR_ProcessPartyEnterLeave_Compat pressure-plate caller.
 *
 *  Pure function.  Things (containers) are inspected read-only via the
 *  caller-supplied DungeonThings_Compat snapshot.
 *
 *  Returns 1 on match, 0 on no match or invalid args.
 * ---------------------------------------------------------------- */
int F0274_SENSOR_IsObjectInPartyPossession_Compat(
    int objectType,
    const struct PartyPossessionContext_Compat* ctx);

/* ----------------------------------------------------------------
 *  F0732_SENSOR_ResolvePressurePlateActuatorDispatch_Compat
 *
 *  Pure projector of a pressure-plate SensorTriggerResult into a
 *  SensorActuatorDispatch.  This is the testable part of the
 *  F0268_SENSOR_AddEvent + actuator dispatch contract that turns a
 *  remote effect into a door/pit/wall/teleporter consequence.
 *
 *  Behavior:
 *    - targetMapX/Y/cell, targetSquareType, targetEventType come from
 *      the sensor's remote target + the caller-supplied target square
 *      type (looked up on the world map by the caller).
 *    - resolvedEffect -> action mapping:
 *        SET    -> OPEN
 *        CLEAR  -> CLOSE
 *        TOGGLE -> TOGGLE
 *        HOLD-with-resolved SET    -> OPEN
 *        HOLD-with-resolved CLEAR  -> CLOSE
 *    - actuatorKindMask is derived from targetSquareType; door/pit/fakewall
 *      are the typical pressure-plate actuators in DM1.
 *    - delayTicks comes from sensor->value (the F0268 queue delay).
 *
 *  Returns 1 on valid dispatch (always, except on null args).
 * ---------------------------------------------------------------- */
int F0732_SENSOR_ResolvePressurePlateActuatorDispatch_Compat(
    const struct DungeonSensor_Compat* sensor,
    const struct SensorTriggerResult_Compat* result,
    int targetSquareType,
    struct SensorActuatorDispatch_Compat* outDispatch);

#endif
