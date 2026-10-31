#ifndef REDMCSB_MEMORY_SENSOR_EXECUTION_PC34_COMPAT_H
#define REDMCSB_MEMORY_SENSOR_EXECUTION_PC34_COMPAT_H

/*
 * Sensor execution layer for ReDMCSB PC 3.4 — Phase 11 of M10.
 *
 * F0710/F0718/F0719 convert a sensor hit + trigger event into a
 * structured list of effects without mutating their inputs. The caller
 * applies the returned effects. F0271 is the source-shaped, stateful
 * batch-final sensor-chain mutation; F0272 applies the OnceOnly mutation.
 *
 * Source: MOVESENS.C, DEFS.H from the original DM/CSB source.
 *
 * Current bounded coverage:
 *   - The F0710 probe API covers teleport/text plus selected remote sensors.
 *   - F0718/F0719 party passes apply C001-C009 gates and emit remote, C10
 *     split-XP, and deferred local-rotation receipts.
 *   - F0271 applies the final CLEAR/TOGGLE rotation for a sensor batch.
 *   - F0272 disables triggered OnceOnly sensors on party passes.
 *   - Other local effects, once-only behavior outside party passes, wall
 *     C013 and non-party event paths remain incomplete.
 */

#include "memory_movement_pc34_compat.h"

struct GameWorld_Compat;

/* ---- Trigger events ---- */
#define SENSOR_EVENT_WALK_ON         0
#define SENSOR_EVENT_WALK_OFF        1
#define SENSOR_EVENT_ITEM_ON         2
#define SENSOR_EVENT_ITEM_OFF        3
#define SENSOR_EVENT_CHAMPION_ACTION 4
#define SENSOR_EVENT_COUNT           5

/* ---- Effect kinds ---- */
#define SENSOR_EFFECT_NONE           0
#define SENSOR_EFFECT_TELEPORT       1
#define SENSOR_EFFECT_SHOW_TEXT      2
#define SENSOR_EFFECT_UNSUPPORTED    3
#define SENSOR_EFFECT_TOGGLE_REMOTE  4  /* Toggle remote target: door/pit/teleporter/wall */
#define SENSOR_EFFECT_PROJECTILE     5  /* Launch projectile from wall */
#define SENSOR_EFFECT_OBJECT_GEN     6  /* Generate object at sensor location */
#define SENSOR_EFFECT_END_GAME_V2   7  /* Trigger endgame sequence */
#define SENSOR_EFFECT_LOCAL_STEAL_XP 8 /* F0270 C10: local 300 Steal XP */
#define SENSOR_EFFECT_LOCAL_DEFERRED 9 /* F0270/F0271 batch-local effect */
#define SENSOR_EXEC_EFFECT_GENERATOR 14 /* Generator: spawn creature (textIndex = sensorIndex) */

/* ---- Bounds ---- */
#define SENSOR_EFFECT_LIST_MAX_COUNT 8

/* ---- Effect payload ---- */
struct SensorEffect_Compat {
    int kind;                /* SENSOR_EFFECT_* */
    int sensorType;          /* Original sensor type code (0..127) */
    int destMapIndex;        /* TELEPORT: destination map index */
    int destMapX;            /* TELEPORT: destination X */
    int destMapY;            /* TELEPORT: destination Y */
    int destCell;            /* TELEPORT: destination cell */
    int textIndex;           /* SHOW_TEXT: text-table index */
    int delayTicks;          /* F0268: sensor->value delay before event fires */
    int sourceSensorIndex;   /* Transient F0272 owner; not serialized */
    int sourceOnceOnly;      /* Transient F0272 mutation receipt; not serialized */
};

struct SensorEffectList_Compat {
    int count;
    struct SensorEffect_Compat effects[SENSOR_EFFECT_LIST_MAX_COUNT];
};

/* ---- Serialized sizes (bit-identical round-trip) ---- */
#define SENSOR_EFFECT_SERIALIZED_SIZE      28  /* 7 ints; runtime receipts are transient */
#define SENSOR_EFFECT_LIST_SERIALIZED_SIZE (4 + (SENSOR_EFFECT_LIST_MAX_COUNT * SENSOR_EFFECT_SERIALIZED_SIZE))

/*
 * Execute a sensor given its identification (from F0703) and an event.
 *
 * Pure function.  Reads dungeon/things only to look up extra data
 * (e.g. text table for SHOW_TEXT — deferred, v1 returns raw index).
 * Writes result into outList.  Never modifies its inputs.
 *
 * Returns:
 *   1 if outList was populated (may have 0 effects for no-op events)
 *   0 only on invalid arguments (null pointer, out-of-range index)
 */
int F0710_SENSOR_Execute_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct SensorOnSquare_Compat* sensor,
    int triggerEvent,
    struct SensorEffectList_Compat* outList);

/*
 * Serialize a single effect to exactly SENSOR_EFFECT_SERIALIZED_SIZE
 * bytes (MEDIA016 / PC LSB-first, 7 int32-LE fields).
 * Returns 1 on success, 0 if bufSize too small.
 */
int F0711_SENSOR_EffectSerialize_Compat(
    const struct SensorEffect_Compat* effect,
    unsigned char* outBuf,
    int outBufSize);

/*
 * Deserialize a single effect from exactly SENSOR_EFFECT_SERIALIZED_SIZE
 * bytes.  Returns 1 on success, 0 if bufSize too small.
 */
int F0712_SENSOR_EffectDeserialize_Compat(
    struct SensorEffect_Compat* effect,
    const unsigned char* buf,
    int bufSize);

/*
 * Serialize an entire effect list: 4 bytes count (int32-LE) followed by
 * SENSOR_EFFECT_LIST_MAX_COUNT fixed-size effect slots.  Total size is
 * SENSOR_EFFECT_LIST_SERIALIZED_SIZE.  Unused slots are zero-filled.
 */
int F0713_SENSOR_ListSerialize_Compat(
    const struct SensorEffectList_Compat* list,
    unsigned char* outBuf,
    int outBufSize);

int F0714_SENSOR_ListDeserialize_Compat(
    struct SensorEffectList_Compat* list,
    const unsigned char* buf,
    int bufSize);

/* ---- Pass 32: multi-sensor enumeration + party enter/leave processing ---- */

#define SENSOR_ENUM_CAPACITY 8

/*
 * Enumerate up to SENSOR_ENUM_CAPACITY sensors on a square, walking the
 * thing linked list and collecting each sensor in list order.  Unlike
 * F0703 (which reports only the first sensor), this populates one
 * SensorOnSquare entry per sensor encountered.
 *
 * Returns the number of sensors actually populated (0..capacity).  The
 * totalSensorsOnSquare field of each entry is set to the same total
 * count the caller receives.  Safe against malformed thing lists: the
 * walker bounds itself at 64 link steps.
 *
 * Source mapping: sensor traversal inside F0276_SENSOR_ProcessThingAdditionOrRemoval
 * (MOVESENS.C).
 */
int F0717_SENSOR_EnumerateOnSquare_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    struct SensorOnSquare_Compat outSensors[SENSOR_ENUM_CAPACITY]);

/*
 * Process a party enter/leave event on a square. Walks sensors in source
 * order and applies the F0276 floor-sensor admission rules available from
 * the supplied context. The context-free wrapper skips C003 and C008 because
 * they require party direction/champion or inventory state. Pure: it does not
 * mutate the world.
 *
 * triggerEvent must be one of SENSOR_EVENT_WALK_ON or SENSOR_EVENT_WALK_OFF.
 * Other events (ITEM_ON/OFF, CHAMPION_ACTION) are accepted but produce
 * no effects.  Returns 1 on success (outList populated), 0 on invalid
 * arguments.
 *
 * Source mapping: F0276_SENSOR_ProcessThingAdditionOrRemoval (MOVESENS.C).
 */
int F0718_SENSOR_ProcessPartyEnterLeave_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    int triggerEvent,
    struct SensorEffectList_Compat* outList);

/* Party-context version used by live DM1 routes. Supplies party direction and
 * champion count for C003, inventory/G4055 leader-hand object for C008, and
 * ReDMCSB F0276's P0591_B_PartySquare value (true when the party remains on
 * this square, as on an in-place turn). */
int F0719_SENSOR_ProcessPartyEnterLeaveWithParty_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    int triggerEvent,
    int partyOnSquare,
    const struct PartyState_Compat* party,
    unsigned short leaderHandThing,
    struct SensorEffectList_Compat* outList);

/* ReDMCSB MOVESENS.C F0276 object add/remove pass for a floor square.
 * The caller owns the source link/unlink transaction and invokes this after
 * the mutation. Added `thing` is excluded from the pre-existing-weight scan,
 * matching F0276's add-before-link census. This bounded adapter handles floor
 * C001/C002/C004/C007 admission and emits F0272 effect receipts; wall-cell
 * sensor branches remain in the wall/object interaction path. */
int F0276_SENSOR_ProcessObjectFloorChange_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex, int mapX, int mapY,
    unsigned short thing,
    int isAddition,
    struct SensorEffectList_Compat* outList);

/* ReDMCSB MOVESENS.C F0271: apply the final CLEAR/TOGGLE rotation receipt
 * after a complete F0276 sensor pass. Mutates the loaded Thing chain and
 * matching raw Next bytes only. */
int F0271_SENSOR_ProcessRotation_Compat(
    struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    int cell,
    int effect);

/* ReDMCSB MOVESENS.C F0272: disable a triggered OnceOnly sensor before its
 * effect. Updates decoded state and raw Type_Data bytes. */
int F0272_SENSOR_DisableOnceOnly_Compat(
    struct GameWorld_Compat* world,
    int sensorIndex);

/* Read-only F0276 sound receipt for a party square pass. The caller retains
 * the triggering coordinates and requests C01 once per audible trigger. */
int F0720_SENSOR_CountAudiblePartyPass_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex, int mapX, int mapY, int triggerEvent, int partySquare,
    const struct PartyState_Compat* party, unsigned short leaderHandThing,
    int* outCount);

#endif
