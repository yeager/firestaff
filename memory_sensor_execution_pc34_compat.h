#ifndef REDMCSB_MEMORY_SENSOR_EXECUTION_PC34_COMPAT_H
#define REDMCSB_MEMORY_SENSOR_EXECUTION_PC34_COMPAT_H

/*
 * Sensor execution layer for ReDMCSB PC 3.4 — Phase 11 of M10.
 *
 * Pure functions that convert a sensor hit + trigger event into a
 * structured list of effects.  NO UI, NO rendering, NO mutation of
 * the input dungeon/things/party state.  The caller decides what to
 * do with the returned effect list (apply, log, queue for timeline).
 *
 * Source: MOVESENS.C, DEFS.H from the original DM/CSB source.
 *
 * v1 coverage (this phase):
 *   - Type 0  TELEPORT      (WALK_ON)  -> EFFECT_TELEPORT
 *   - Type 13 TEXT_MESSAGE  (WALK_ON)  -> EFFECT_SHOW_TEXT (text-id = data)
 *   All other sensor types return EFFECT_UNSUPPORTED so the caller can
 *   distinguish "nothing happened" from "not yet implemented".
 *   WALK_OFF, ITEM_ON, ITEM_OFF, CHAMPION_ACTION return an empty list
 *   for every implemented type in v1 (conservative — will expand as
 *   timeline/combat land).
 */

#include "memory_movement_pc34_compat.h"

/* ---- Trigger events ---- */
#define SENSOR_EVENT_WALK_ON         0
#define SENSOR_EVENT_WALK_OFF        1
#define SENSOR_EVENT_ITEM_ON         2
#define SENSOR_EVENT_ITEM_OFF        3
#define SENSOR_EVENT_CHAMPION_ACTION 4
#define SENSOR_EVENT_COUNT           5

/* ---- Effect kinds ---- */
#define SENSOR_EFFECT_NONE         0
#define SENSOR_EFFECT_TELEPORT     1
#define SENSOR_EFFECT_SHOW_TEXT    2
#define SENSOR_EFFECT_UNSUPPORTED  3

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
};

struct SensorEffectList_Compat {
    int count;
    struct SensorEffect_Compat effects[SENSOR_EFFECT_LIST_MAX_COUNT];
};

/* ---- Serialized sizes (bit-identical round-trip) ---- */
#define SENSOR_EFFECT_SERIALIZED_SIZE      28  /* 7 ints * 4 bytes LE */
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
 * Process a party enter/leave event on a square.  Walks every sensor
 * on the square in source order, runs F0710_SENSOR_Execute_Compat for
 * the given event, and appends each produced effect onto outList (up
 * to SENSOR_EFFECT_LIST_MAX_COUNT).  Pure: does not mutate world.
 *
 * v1 coverage note: for WALK_ON, teleport (type 0) and text (type 13)
 * are fully modeled; other sensor types still produce SENSOR_EFFECT_UNSUPPORTED
 * markers through F0710.  WALK_OFF currently surfaces no effects for any
 * sensor type — matching the conservative policy in F0710.
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

#endif
