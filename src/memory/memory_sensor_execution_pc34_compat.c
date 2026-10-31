/*
 * Sensor execution layer for ReDMCSB PC 3.4 — Phase 11 of M10.
 * See header for full documentation.
 */

#include <string.h>
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"

/* ---- LE int32 helpers (MEDIA016 / PC LSB-first) ---- */

static void write_i32_le(unsigned char* p, int value) {
    unsigned int u = (unsigned int)value;
    p[0] = (unsigned char)(u & 0xFF);
    p[1] = (unsigned char)((u >> 8) & 0xFF);
    p[2] = (unsigned char)((u >> 16) & 0xFF);
    p[3] = (unsigned char)((u >> 24) & 0xFF);
}

static int read_i32_le(const unsigned char* p) {
    unsigned int u =
        ((unsigned int)p[0]) |
        ((unsigned int)p[1] << 8) |
        ((unsigned int)p[2] << 16) |
        ((unsigned int)p[3] << 24);
    return (int)u;
}

/* ---- Decode cell + map index from a SensorOnSquare ---- */

/*
 * A SensorOnSquare reports the first sensor found on a given tile.
 * For teleport sensors the identified fields give us destination
 * (targetMapX, targetMapY, targetCell).  v1 assumes destination is
 * on the SAME map as the triggering square — the on-disk sensor data
 * does not encode a remote map index for teleporters in DM1, which
 * matches the DM1 design (maps are discrete levels, teleport stays
 * within a level unless a later sensor type is involved).
 */

/* ---- Execute ---- */

int F0710_SENSOR_Execute_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct SensorOnSquare_Compat* sensor,
    int triggerEvent,
    struct SensorEffectList_Compat* outList)
{
    /* Dungeon and things are part of the stable API so later phases (text
     * lookup, door resolution) can use them without changing callers. */
    (void)dungeon;

    if (outList == 0 || sensor == 0) return 0;
    memset(outList, 0, sizeof(*outList));

    if (!sensor->found) return 1; /* Empty list, valid. */

    /* v1 fires effects on WALK_ON and WALL_CLICK (via CHAMPION_ACTION).
     * Other events produce an empty list. */
    if (triggerEvent != SENSOR_EVENT_WALK_ON &&
        triggerEvent != SENSOR_EVENT_CHAMPION_ACTION) {
        return 1;
    }

    switch (sensor->sensorType) {
    case 0: {
        /* TELEPORT — destination from sensor's remote target fields. */
        struct SensorEffect_Compat* e = &outList->effects[0];
        e->kind = SENSOR_EFFECT_TELEPORT;
        e->sensorType = sensor->sensorType;
        e->destMapIndex = -1; /* Same-map teleport in v1 — caller keeps current mapIndex. */
        e->destMapX = sensor->targetMapX;
        e->destMapY = sensor->targetMapY;
        e->destCell = sensor->targetCell;
        e->textIndex = 0;
        outList->count = 1;
        return 1;
    }
    case 13: {
        /* TEXT / MESSAGE — sensor->sensorData carries the text index. */
        struct SensorEffect_Compat* e = &outList->effects[0];
        e->kind = SENSOR_EFFECT_SHOW_TEXT;
        e->sensorType = sensor->sensorType;
        e->destMapIndex = 0;
        e->destMapX = 0;
        e->destMapY = 0;
        e->destCell = 0;
        e->textIndex = (int)sensor->sensorData;
        outList->count = 1;
        return 1;
    }
    case 1:  /* DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT */
    case 2:  /* DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE */
    case 3:  /* DM1_SENSOR_FLOOR_PARTY — pressure plates */
    case 4:  /* DM1_SENSOR_FLOOR_OBJECT */
    case 7:  /* DM1_SENSOR_FLOOR_CREATURE */
    case 8: {/* DM1_SENSOR_FLOOR_PARTY_POSSESSION */
        /* Floor sensors with remote target → toggle target square.
         * ReDMCSB MOVESENS.C F0276: these sensor types fire F0268
         * which maps the sensor's effect+target to a timed event
         * on the target square.  The target square type determines
         * the event type (door=10, pit=9, teleporter=8, etc.).
         * We emit SENSOR_EFFECT_TOGGLE_REMOTE and let the caller
         * resolve the target square and apply the effect. */
        struct SensorEffect_Compat* e = &outList->effects[0];
        e->kind = SENSOR_EFFECT_TOGGLE_REMOTE;
        e->sensorType = sensor->sensorType;
        e->destMapIndex = -1; /* same map */
        e->destMapX = sensor->targetMapX;
        e->destMapY = sensor->targetMapY;
        e->destCell = sensor->targetCell;
        e->textIndex = sensor->effect == 3 ? 0 : sensor->effect;
        e->delayTicks = sensor->value;
        outList->count = 1;
        return 1;
    }
    case 5: {/* DM1_SENSOR_FLOOR_PARTY_ON_STAIRS */
        /* Stairs sensor — typically triggers when party is on stairs.
         * Usually toggles a remote target. */
        struct SensorEffect_Compat* e = &outList->effects[0];
        e->kind = SENSOR_EFFECT_TOGGLE_REMOTE;
        e->sensorType = sensor->sensorType;
        e->destMapIndex = -1;
        e->destMapX = sensor->targetMapX;
        e->destMapY = sensor->targetMapY;
        e->destCell = sensor->targetCell;
        e->textIndex = sensor->effect == 3 ? 0 : sensor->effect;
        e->delayTicks = sensor->value;
        outList->count = 1;
        return 1;
    }
    case 6:
        /* ReDMCSB MOVESENS.C F0276:1704-1705 skips C006 in the floor-sensor
         * addition/removal path. Group generation is driven by its timed
         * event owner, not by a party walking onto this square. */
        return 1;
    case 9: {
        /* ReDMCSB MOVESENS.C F0276:1716-1723: C009 checks the DM1 engine
         * version, then the common F0276 path XORs the result with
         * RevertEffect before resolving HOLD. Use the original sensor record
         * so an inverted version gate is not lost in SensorOnSquare. */
        const struct DungeonSensor_Compat* sourceSensor;
        struct FloorSensorContext_Compat floorContext;
        struct SensorTriggerResult_Compat triggerResult;
        if (!things || !things->sensors || sensor->sensorIndex < 0 ||
            sensor->sensorIndex >= things->sensorCount) {
            return 1;
        }
        sourceSensor = &things->sensors[sensor->sensorIndex];
        if (sourceSensor->sensorType != DM1_SENSOR_FLOOR_VERSION_CHECKER) {
            return 1;
        }
        memset(&floorContext, 0, sizeof(floorContext));
        floorContext.thingType = DM1_TRIGGER_SOURCE_PARTY;
        floorContext.isAddition = (triggerEvent == SENSOR_EVENT_WALK_ON);
        if (!F0722_SENSOR_EvaluateFloor_Compat(
                sourceSensor, &floorContext, &triggerResult)) {
            return 0;
        }
        if (!triggerResult.triggered) return 1;

        struct SensorEffect_Compat* e = &outList->effects[0];
        e->kind = SENSOR_EFFECT_TOGGLE_REMOTE;
        e->sensorType = sensor->sensorType;
        e->destMapIndex = -1;
        e->destMapX = sensor->targetMapX;
        e->destMapY = sensor->targetMapY;
        e->destCell = sensor->targetCell;
        e->textIndex = triggerResult.resolvedEffect;
        e->delayTicks = triggerResult.delayTicks;
        outList->count = 1;
        return 1;
    }
    default: {
        /* UNSUPPORTED — explicit marker so callers distinguish from
         * "no effect".  Remaining types will be implemented as
         * timeline / combat / actuator phases land. */
        struct SensorEffect_Compat* e = &outList->effects[0];
        e->kind = SENSOR_EFFECT_UNSUPPORTED;
        e->sensorType = sensor->sensorType;
        e->destMapIndex = 0;
        e->destMapX = 0;
        e->destMapY = 0;
        e->destCell = 0;
        e->textIndex = 0;
        outList->count = 1;
        return 1;
    }
    }
}

/* ---- Serialization (bit-identical round-trip) ---- */

int F0711_SENSOR_EffectSerialize_Compat(
    const struct SensorEffect_Compat* effect,
    unsigned char* outBuf,
    int outBufSize)
{
    if (effect == 0 || outBuf == 0) return 0;
    if (outBufSize < SENSOR_EFFECT_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf +  0, effect->kind);
    write_i32_le(outBuf +  4, effect->sensorType);
    write_i32_le(outBuf +  8, effect->destMapIndex);
    write_i32_le(outBuf + 12, effect->destMapX);
    write_i32_le(outBuf + 16, effect->destMapY);
    write_i32_le(outBuf + 20, effect->destCell);
    write_i32_le(outBuf + 24, effect->textIndex);
    return 1;
}

int F0712_SENSOR_EffectDeserialize_Compat(
    struct SensorEffect_Compat* effect,
    const unsigned char* buf,
    int bufSize)
{
    if (effect == 0 || buf == 0) return 0;
    if (bufSize < SENSOR_EFFECT_SERIALIZED_SIZE) return 0;

    effect->kind         = read_i32_le(buf +  0);
    effect->sensorType   = read_i32_le(buf +  4);
    effect->destMapIndex = read_i32_le(buf +  8);
    effect->destMapX     = read_i32_le(buf + 12);
    effect->destMapY     = read_i32_le(buf + 16);
    effect->destCell     = read_i32_le(buf + 20);
    effect->textIndex    = read_i32_le(buf + 24);
    effect->delayTicks   = 0;
    return 1;
}

int F0713_SENSOR_ListSerialize_Compat(
    const struct SensorEffectList_Compat* list,
    unsigned char* outBuf,
    int outBufSize)
{
    int i;
    if (list == 0 || outBuf == 0) return 0;
    if (outBufSize < SENSOR_EFFECT_LIST_SERIALIZED_SIZE) return 0;

    memset(outBuf, 0, SENSOR_EFFECT_LIST_SERIALIZED_SIZE);
    write_i32_le(outBuf, list->count);
    for (i = 0; i < SENSOR_EFFECT_LIST_MAX_COUNT; i++) {
        unsigned char* slot = outBuf + 4 + (i * SENSOR_EFFECT_SERIALIZED_SIZE);
        if (i < list->count) {
            F0711_SENSOR_EffectSerialize_Compat(&list->effects[i], slot, SENSOR_EFFECT_SERIALIZED_SIZE);
        }
        /* Unused slots stay zero-filled. */
    }
    return 1;
}

/* ---- Pass 32: multi-sensor enumeration + party enter/leave ---- */

static int sensor_find_sft_index(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat* map;
    int squareIdx;
    int m, sq, count, total = 0;

    if (!dungeon || !dungeon->tilesLoaded || !dungeon->tiles) return -1;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return -1;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) return -1;
    if (!dungeon->tiles[mapIndex].squareData) return -1;
    squareIdx = mapX * map->height + mapY;
    if (!(dungeon->tiles[mapIndex].squareData[squareIdx] & DUNGEON_SQUARE_MASK_THING_LIST)) return -1;

    /* Count thing-list squares across all maps up to and including the target. */
    for (m = 0; m < mapIndex; ++m) {
        count = dungeon->maps[m].width * dungeon->maps[m].height;
        for (sq = 0; sq < count; ++sq) {
            if (dungeon->tiles[m].squareData[sq] & DUNGEON_SQUARE_MASK_THING_LIST) ++total;
        }
    }
    count = map->width * map->height;
    for (sq = 0; sq <= squareIdx && sq < count; ++sq) {
        if (dungeon->tiles[mapIndex].squareData[sq] & DUNGEON_SQUARE_MASK_THING_LIST) ++total;
    }
    return total - 1;
}

static unsigned short sensor_step_thing(
    const struct DungeonThings_Compat* things,
    unsigned short thingRef,
    int* outType,
    int* outIndex)
{
    int type = THING_GET_TYPE(thingRef);
    int index = THING_GET_INDEX(thingRef);
    unsigned short nextThing = THING_NONE;
    if (outType) *outType = type;
    if (outIndex) *outIndex = index;

    switch (type) {
        case THING_TYPE_DOOR:
            if (index < things->doorCount) nextThing = things->doors[index].next;
            break;
        case THING_TYPE_TELEPORTER:
            if (index < things->teleporterCount) nextThing = things->teleporters[index].next;
            break;
        case THING_TYPE_TEXTSTRING:
            if (index < things->textStringCount) nextThing = things->textStrings[index].next;
            break;
        case THING_TYPE_SENSOR:
            if (index < things->sensorCount) nextThing = things->sensors[index].next;
            break;
        case THING_TYPE_GROUP:
            if (index < things->groupCount) nextThing = things->groups[index].next;
            break;
        case THING_TYPE_WEAPON:
            if (index < things->weaponCount) nextThing = things->weapons[index].next;
            break;
        case THING_TYPE_ARMOUR:
            if (index < things->armourCount) nextThing = things->armours[index].next;
            break;
        case THING_TYPE_SCROLL:
            if (index < things->scrollCount) nextThing = things->scrolls[index].next;
            break;
        case THING_TYPE_POTION:
            if (index < things->potionCount) nextThing = things->potions[index].next;
            break;
        case THING_TYPE_CONTAINER:
            if (index < things->containerCount) nextThing = things->containers[index].next;
            break;
        case THING_TYPE_JUNK:
            if (index < things->junkCount) nextThing = things->junks[index].next;
            break;
        case THING_TYPE_PROJECTILE:
            if (index < things->projectileCount) nextThing = things->projectiles[index].next;
            break;
        case THING_TYPE_EXPLOSION:
            if (index < things->explosionCount) nextThing = things->explosions[index].next;
            break;
        default:
            nextThing = THING_NONE;
            break;
    }
    return nextThing;
}

static int sensor_square_has_existing_floor_weight(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY)
{
    int sftIdx;
    unsigned short thingRef;
    int safety = 0;

    if (!dungeon || !things || !things->loaded) return 0;
    sftIdx = sensor_find_sft_index(dungeon, mapIndex, mapX, mapY);
    if (sftIdx < 0 || sftIdx >= things->squareFirstThingCount) return 0;

    thingRef = things->squareFirstThings[sftIdx];
    while (thingRef != THING_NONE && thingRef != THING_ENDOFLIST && safety < 64) {
        int type, index;
        unsigned short nextThing = sensor_step_thing(things, thingRef, &type, &index);
        (void)index;

        /* ReDMCSB MOVESENS.C:F0276 lines 1624-1648 computes
         * L0772_B_SquareContainsObject and L0773_B_SquareContainsGroup
         * before party floor sensors run.  Case C001 at lines 1664-1667
         * suppresses a party/object/creature pressure pad when an object
         * or group already weighs the square. */
        if (type == THING_TYPE_GROUP ||
            (type > THING_TYPE_GROUP && type < THING_TYPE_PROJECTILE)) {
            return 1;
        }

        thingRef = nextThing;
        ++safety;
    }

    return 0;
}

static int sensor_square_has_existing_group(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex, int mapX, int mapY)
{
    int sftIdx;
    unsigned short thingRef;
    int safety = 0;
    if (!dungeon || !things || !things->loaded) return 0;
    sftIdx = sensor_find_sft_index(dungeon, mapIndex, mapX, mapY);
    if (sftIdx < 0 || sftIdx >= things->squareFirstThingCount) return 0;
    thingRef = things->squareFirstThings[sftIdx];
    while (thingRef != THING_NONE && thingRef != THING_ENDOFLIST && safety++ < 64) {
        int type, index;
        unsigned short nextThing = sensor_step_thing(things, thingRef, &type, &index);
        (void)index;
        if (type == THING_TYPE_GROUP) return 1;
        thingRef = nextThing;
    }
    return 0;
}

int F0717_SENSOR_EnumerateOnSquare_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    struct SensorOnSquare_Compat outSensors[SENSOR_ENUM_CAPACITY])
{
    int sftIdx;
    unsigned short thingRef;
    int count = 0;
    int safety = 0;
    int i;

    if (!outSensors) return 0;
    for (i = 0; i < SENSOR_ENUM_CAPACITY; ++i) {
        memset(&outSensors[i], 0, sizeof(outSensors[i]));
    }
    if (!dungeon || !things || !things->loaded) return 0;

    sftIdx = sensor_find_sft_index(dungeon, mapIndex, mapX, mapY);
    if (sftIdx < 0 || sftIdx >= things->squareFirstThingCount) return 0;
    thingRef = things->squareFirstThings[sftIdx];

    while (thingRef != THING_NONE && thingRef != THING_ENDOFLIST && safety < 64) {
        int type, index;
        unsigned short nextThing = sensor_step_thing(things, thingRef, &type, &index);
        if (type == THING_TYPE_SENSOR && index >= 0 && index < things->sensorCount) {
            const struct DungeonSensor_Compat* sensor = &things->sensors[index];
            if (count < SENSOR_ENUM_CAPACITY) {
                struct SensorOnSquare_Compat* out = &outSensors[count];
                out->found = 1;
                out->sensorIndex = index;
                out->sensorType = sensor->sensorType;
                out->sensorData = sensor->sensorData;
                out->effect = sensor->effect;
                out->isLocal = sensor->localEffect;
                out->cell = THING_GET_CELL(thingRef);
                out->value = sensor->value;
                if (!sensor->localEffect) {
                    out->targetMapX = sensor->targetMapX;
                    out->targetMapY = sensor->targetMapY;
                    out->targetCell = sensor->targetCell;
                } else {
                    out->targetMapX = mapX;
                    out->targetMapY = mapY;
                    out->targetCell = 0;
                }
            }
            ++count;
        }
        thingRef = nextThing;
        ++safety;
    }

    /* Back-fill totalSensorsOnSquare on every populated entry. */
    for (i = 0; i < count && i < SENSOR_ENUM_CAPACITY; ++i) {
        outSensors[i].totalSensorsOnSquare = count;
    }
    return (count > SENSOR_ENUM_CAPACITY) ? SENSOR_ENUM_CAPACITY : count;
}

static int sensor_floor_object_type(
    const struct DungeonThings_Compat* things,
    unsigned short thingRef)
{
    int type;
    int index;
    if (!things) return -1;
    type = THING_GET_TYPE(thingRef);
    index = THING_GET_INDEX(thingRef);
    switch (type) {
    case THING_TYPE_WEAPON:
        if (index >= 0 && index < things->weaponCount) {
            if (things->rawThingData[THING_TYPE_WEAPON] &&
                index < things->thingCounts[THING_TYPE_WEAPON]) {
                const unsigned char* raw =
                    things->rawThingData[THING_TYPE_WEAPON] + index * 4;
                return (int)(raw[2] & 0x7fu);
            }
            return (int)things->weapons[index].type;
        }
        return -1;
    case THING_TYPE_ARMOUR:
        return index >= 0 && index < things->armourCount
            ? (int)things->armours[index].type : -1;
    case THING_TYPE_SCROLL:
        return index >= 0 && index < things->scrollCount
            ? (int)things->scrolls[index].textStringThingIndex : -1;
    case THING_TYPE_POTION:
        return index >= 0 && index < things->potionCount
            ? (int)things->potions[index].type : -1;
    case THING_TYPE_CONTAINER:
        return index >= 0 && index < things->containerCount
            ? (int)things->containers[index].type : -1;
    case THING_TYPE_JUNK:
        return index >= 0 && index < things->junkCount
            ? (int)things->junks[index].type : -1;
    default:
        return -1;
    }
}

static int sensor_floor_object_class(int thingType)
{
    return thingType >= THING_TYPE_WEAPON &&
        thingType <= THING_TYPE_JUNK;
}

int F0276_SENSOR_ProcessObjectFloorChange_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex, int mapX, int mapY,
    unsigned short changedThing,
    int isAddition,
    struct SensorEffectList_Compat* outList)
{
    struct SensorOnSquare_Compat sensors[SENSOR_ENUM_CAPACITY];
    int changedType;
    int changedObjectType;
    int sensorCount;
    int existingObject = 0;
    int existingGroup = 0;
    int sameTypeObject = 0;
    int sftIdx;
    int safety = 0;
    int i;

    if (!outList) return 0;
    memset(outList, 0, sizeof(*outList));
    if (!dungeon || !things || !things->loaded ||
        (isAddition != 0 && isAddition != 1)) return 0;
    changedType = THING_GET_TYPE(changedThing);
    changedObjectType = sensor_floor_object_type(things, changedThing);
    if (changedType != THING_TYPE_GROUP &&
        !sensor_floor_object_class(changedType)) return 1;

    /* ReDMCSB MOVESENS.C F0276: the add path counts the old floor chain
     * before linking the incoming object. M11 calls after its mutation, so
     * exclude that exact record from the census to preserve the source view. */
    sftIdx = sensor_find_sft_index(dungeon, mapIndex, mapX, mapY);
    if (sftIdx >= 0 && sftIdx < things->squareFirstThingCount) {
        unsigned short ref = things->squareFirstThings[sftIdx];
        while (ref != THING_NONE && ref != THING_ENDOFLIST && safety < 64) {
            int type;
            int index;
            unsigned short next = sensor_step_thing(things, ref, &type, &index);
            if (type == THING_TYPE_GROUP) {
                if (!(THING_GET_TYPE(changedThing) == type &&
                      THING_GET_INDEX(changedThing) == index)) {
                    existingGroup = 1;
                }
            } else if (sensor_floor_object_class(type)) {
                int objectType = sensor_floor_object_type(things, ref);
                if (!(THING_GET_TYPE(changedThing) == type &&
                      THING_GET_INDEX(changedThing) == index)) {
                    existingObject = 1;
                    if (changedObjectType >= 0 &&
                        objectType == changedObjectType) {
                        sameTypeObject = 1;
                    }
                }
            }
            ref = next;
            ++safety;
        }
        if (ref != THING_NONE && ref != THING_ENDOFLIST) return 1;
    }

    sensorCount = F0717_SENSOR_EnumerateOnSquare_Compat(
        dungeon, things, mapIndex, mapX, mapY, sensors);
    for (i = 0; i < sensorCount; ++i) {
        const struct SensorOnSquare_Compat* onSquare = &sensors[i];
        const struct DungeonSensor_Compat* source;
        struct FloorSensorContext_Compat context;
        struct SensorTriggerResult_Compat result;
        int triggerType;
        if (onSquare->sensorType != DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT &&
            onSquare->sensorType != DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE &&
            onSquare->sensorType != DM1_SENSOR_FLOOR_OBJECT &&
            onSquare->sensorType != DM1_SENSOR_FLOOR_CREATURE) continue;
        if (!things->sensors || onSquare->sensorIndex < 0 ||
            onSquare->sensorIndex >= things->sensorCount) continue;
        source = &things->sensors[onSquare->sensorIndex];
        if (source->sensorType != onSquare->sensorType) continue;

        if (changedType == THING_TYPE_GROUP) {
            triggerType = DM1_TRIGGER_SOURCE_CREATURE;
        } else if (changedType == THING_TYPE_PROJECTILE) {
            triggerType = DM1_TRIGGER_SOURCE_PROJECTILE;
        } else {
            triggerType = DM1_TRIGGER_SOURCE_OBJECT;
        }
        memset(&context, 0, sizeof(context));
        context.mapX = mapX;
        context.mapY = mapY;
        context.thingType = triggerType;
        context.objectType = changedObjectType;
        context.squareHasObject = existingObject;
        context.squareHasGroup = existingGroup;
        context.squareHasSameTypeObj = sameTypeObject;
        context.isAddition = isAddition;
        if (!F0722_SENSOR_EvaluateFloor_Compat(source, &context, &result) ||
            !result.triggered) continue;
        if (outList->count < SENSOR_EFFECT_LIST_MAX_COUNT) {
            struct SensorEffect_Compat* effect =
                &outList->effects[outList->count++];
            effect->kind = source->localEffect
                ? (source->localMultiple == DM1_EFFECT_ADD_300XP_STEAL_SKILL
                       ? SENSOR_EFFECT_LOCAL_STEAL_XP
                       : SENSOR_EFFECT_LOCAL_DEFERRED)
                : SENSOR_EFFECT_TOGGLE_REMOTE;
            effect->sensorType = onSquare->sensorType;
            effect->destMapIndex = source->localEffect ? mapIndex : -1;
            effect->destMapX = onSquare->targetMapX;
            effect->destMapY = onSquare->targetMapY;
            effect->destCell = source->localEffect ? -1 : onSquare->targetCell;
            effect->textIndex = source->localEffect
                ? source->localMultiple : result.resolvedEffect;
            effect->delayTicks = source->value;
            effect->sourceSensorIndex = onSquare->sensorIndex;
            effect->sourceOnceOnly = source->onceOnly ? 1 : 0;
            effect->sourceAudible = source->audible;
        }
    }
    return 1;
}

static int sensor_build_party_possession_context(
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    unsigned short leaderHandThing,
    struct PartyPossessionContext_Compat* outContext)
{
    int champion;
    int slot;

    if (!things || !party || !outContext ||
        party->championCount < 0 ||
        party->championCount > DM1_SENSOR_POSSESSION_MAX_CHAMPIONS) {
        return 0;
    }
    memset(outContext, 0, sizeof(*outContext));
    outContext->championCount = party->championCount;
    outContext->things = things;
    outContext->leaderHandThing = leaderHandThing;
    for (champion = 0; champion < DM1_SENSOR_POSSESSION_MAX_CHAMPIONS;
         ++champion) {
        const struct ChampionState_Compat* source = &party->champions[champion];
        outContext->championAlive[champion] =
            source->present && source->hp.current > 0;
        for (slot = 0; slot < DM1_SENSOR_POSSESSION_SLOT_LAST; ++slot) {
            outContext->championSlots[champion][slot] = source->inventory[slot];
        }
    }
    return 1;
}

static int sensor_process_party_enter_leave(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    int triggerEvent,
    int partyOnSquare,
    const struct PartyState_Compat* party,
    unsigned short leaderHandThing,
    struct SensorEffectList_Compat* outList)
{
    struct SensorOnSquare_Compat sensors[SENSOR_ENUM_CAPACITY];
    struct PartyPossessionContext_Compat possessionContext;
    int possessionContextValid;
    int sensorCount, i;
    int squareType = -1;
    int squareHasExistingFloorWeight = 0;

    if (!outList) return 0;
    memset(outList, 0, sizeof(*outList));
    if (!dungeon || !things) return 0;

    /* Only WALK_ON / WALK_OFF are meaningful for party enter/leave in
     * v1.  Other events are accepted but produce no effects. */
    if (triggerEvent < 0 || triggerEvent >= SENSOR_EVENT_COUNT) return 0;

    possessionContextValid = sensor_build_party_possession_context(
        things, party, leaderHandThing, &possessionContext);

    sensorCount = F0717_SENSOR_EnumerateOnSquare_Compat(
        dungeon, things, mapIndex, mapX, mapY, sensors);
    squareHasExistingFloorWeight = sensor_square_has_existing_floor_weight(
        dungeon, things, mapIndex, mapX, mapY);

    if (mapIndex >= 0 && mapIndex < (int)dungeon->header.mapCount) {
        const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
        if (dungeon->tiles && mapX >= 0 && mapX < map->width && mapY >= 0 && mapY < map->height) {
            unsigned char squareByte = dungeon->tiles[mapIndex].squareData[mapX * map->height + mapY];
            squareType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
        }
    }

    for (i = 0; i < sensorCount && i < SENSOR_ENUM_CAPACITY; ++i) {
        struct SensorEffectList_Compat tmp;
        memset(&tmp, 0, sizeof(tmp));
        /* ReDMCSB MOVESENS.C F0276 lines 1695-1702: floor C005 only
         * survives the switch when the party is the triggering thing and
         * M034_SQUARE_TYPE(current square) == C03_ELEMENT_STAIRS.  This
         * party-enter/leave wrapper already models party triggers, so it
         * must preserve the stairs-square predicate before execution. */
        if (sensors[i].sensorType == 5 && squareType != DUNGEON_ELEMENT_STAIRS) {
            continue;
        }
        if (sensors[i].sensorType == 1 && squareHasExistingFloorWeight) {
            continue;
        }
        /* ReDMCSB MOVESENS.C F0276:1695-1702, case C007 at 1703-1705:
         * floor-creature sensors do not fire for the party. This wrapper
         * only processes party enter/leave, so C007 is never applicable. */
        if (sensors[i].sensorType == 7) {
            continue;
        }
        if (sensors[i].sensorType ==
                DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT ||
            sensors[i].sensorType == DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE ||
            sensors[i].sensorType == DM1_SENSOR_FLOOR_PARTY ||
            sensors[i].sensorType == DM1_SENSOR_FLOOR_OBJECT) {
            struct FloorSensorContext_Compat floorContext;
            struct SensorTriggerResult_Compat triggerResult;
            const struct DungeonSensor_Compat* sourceSensor;
            if (!things->sensors || sensors[i].sensorIndex < 0 ||
                sensors[i].sensorIndex >= things->sensorCount ||
                (sensors[i].sensorType == DM1_SENSOR_FLOOR_PARTY && !party)) {
                continue;
            }
            sourceSensor = &things->sensors[sensors[i].sensorIndex];
            memset(&floorContext, 0, sizeof(floorContext));
            floorContext.mapX = mapX;
            floorContext.mapY = mapY;
            floorContext.thingType = DM1_TRIGGER_SOURCE_PARTY;
            floorContext.partyOnSquare = partyOnSquare ? 1 : 0;
            floorContext.objectType = -1;
            floorContext.partyDirection = party ? party->direction : 0;
            floorContext.partyChampionCount = party ? party->championCount : 1;
            floorContext.editionPolicyBound =
                dungeon->dm1SensorEditionPolicyBound;
            floorContext.floorC005PartyTypeFix =
                dungeon->dm1SensorFloorC005PartyTypeFix;
            floorContext.floorC009Supported =
                dungeon->dm1SensorFloorC009Supported;
            floorContext.floorC009VersionLimit =
                dungeon->dm1SensorFloorC009VersionLimit;
            floorContext.squareHasGroup = sensor_square_has_existing_group(
                dungeon, things, mapIndex, mapX, mapY);
            floorContext.squareHasObject =
                squareHasExistingFloorWeight && !floorContext.squareHasGroup;
            floorContext.squareType = squareType;
            floorContext.isAddition = (triggerEvent == SENSOR_EVENT_WALK_ON);
            if (!F0722_SENSOR_EvaluateFloor_Compat(
                    sourceSensor, &floorContext, &triggerResult) ||
                !triggerResult.triggered) {
                continue;
            }
            if (outList->count < SENSOR_EFFECT_LIST_MAX_COUNT) {
                struct SensorEffect_Compat* effect =
                    &outList->effects[outList->count++];
                /* ReDMCSB MOVESENS.C:F0276 passes CELL_ANY to F0272 for
                 * party floor sensors; preserve that receipt for local C10. */
                effect->kind = sourceSensor->localEffect
                    ? (sourceSensor->localMultiple ==
                           DM1_EFFECT_ADD_300XP_STEAL_SKILL
                           ? SENSOR_EFFECT_LOCAL_STEAL_XP
                           : SENSOR_EFFECT_LOCAL_DEFERRED)
                    : SENSOR_EFFECT_TOGGLE_REMOTE;
                effect->sensorType = sensors[i].sensorType;
                effect->destMapIndex = sourceSensor->localEffect
                    ? mapIndex : -1;
                effect->destMapX = sensors[i].targetMapX;
                effect->destMapY = sensors[i].targetMapY;
                effect->destCell = sourceSensor->localEffect
                    ? -1 : sensors[i].targetCell;
                effect->textIndex = sourceSensor->localEffect
                    ? sourceSensor->localMultiple
                    : triggerResult.resolvedEffect;
                effect->delayTicks = sensors[i].value;
                effect->sourceSensorIndex = sensors[i].sensorIndex;
                effect->sourceOnceOnly = sourceSensor->onceOnly ? 1 : 0;
                effect->sourceAudible = sourceSensor->audible ? 1 : 0;
            }
            continue;
        }
        if (sensors[i].sensorType == DM1_SENSOR_FLOOR_PARTY_POSSESSION) {
            struct FloorSensorContext_Compat floorContext;
            struct SensorTriggerResult_Compat triggerResult;
            const struct DungeonSensor_Compat* sourceSensor;
            int hasObject;
            if (triggerEvent != SENSOR_EVENT_WALK_ON ||
                !possessionContextValid || !things->sensors ||
                sensors[i].sensorIndex < 0 ||
                sensors[i].sensorIndex >= things->sensorCount) {
                continue;
            }
            sourceSensor = &things->sensors[sensors[i].sensorIndex];
            hasObject = F0274_SENSOR_IsObjectInPartyPossession_Compat(
                sourceSensor->sensorData, &possessionContext);
            memset(&floorContext, 0, sizeof(floorContext));
            floorContext.mapX = mapX;
            floorContext.mapY = mapY;
            floorContext.thingType = DM1_TRIGGER_SOURCE_PARTY;
            floorContext.partyOnSquare = partyOnSquare ? 1 : 0;
            floorContext.objectType = -1;
            floorContext.partyDirection = party->direction;
            floorContext.partyChampionCount = party->championCount;
            floorContext.partyHasObjectType = hasObject;
            floorContext.isAddition = 1;
            if (!F0722_SENSOR_EvaluateFloor_Compat(
                    sourceSensor, &floorContext, &triggerResult) ||
                !triggerResult.triggered) {
                continue;
            }
            if (outList->count < SENSOR_EFFECT_LIST_MAX_COUNT) {
                struct SensorEffect_Compat* effect =
                    &outList->effects[outList->count++];
                effect->kind = SENSOR_EFFECT_TOGGLE_REMOTE;
                effect->sensorType = sensors[i].sensorType;
                effect->destMapIndex = -1;
                effect->destMapX = sensors[i].targetMapX;
                effect->destMapY = sensors[i].targetMapY;
                effect->destCell = sensors[i].targetCell;
                effect->textIndex = triggerResult.resolvedEffect;
                effect->delayTicks = sensors[i].value;
            }
            continue;
        }
        if (!F0710_SENSOR_Execute_Compat(dungeon, things, &sensors[i],
                                         triggerEvent, &tmp)) {
            continue;
        }
        /* Append each effect from tmp onto outList while there's space. */
        {
            int j;
            for (j = 0; j < tmp.count; ++j) {
                if (outList->count >= SENSOR_EFFECT_LIST_MAX_COUNT) break;
                outList->effects[outList->count++] = tmp.effects[j];
            }
        }
        if (outList->count >= SENSOR_EFFECT_LIST_MAX_COUNT) break;
    }
    return 1;
}

int F0718_SENSOR_ProcessPartyEnterLeave_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    int triggerEvent,
    struct SensorEffectList_Compat* outList)
{
    return sensor_process_party_enter_leave(
        dungeon, things, mapIndex, mapX, mapY, triggerEvent, 0, NULL,
        THING_NONE, outList);
}

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
    struct SensorEffectList_Compat* outList)
{
    return sensor_process_party_enter_leave(
        dungeon, things, mapIndex, mapX, mapY, triggerEvent, partyOnSquare,
        party, leaderHandThing, outList);
}

int F0720_SENSOR_CountAudiblePartyPass_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex, int mapX, int mapY, int triggerEvent, int partySquare,
    const struct PartyState_Compat* party, unsigned short leaderHandThing,
    int* outCount)
{
    struct SensorOnSquare_Compat sensors[SENSOR_ENUM_CAPACITY];
    struct PartyPossessionContext_Compat possession;
    struct FloorSensorContext_Compat context;
    int count, i;
    if (!outCount || !dungeon || !things || !party ||
        (triggerEvent != SENSOR_EVENT_WALK_ON &&
         triggerEvent != SENSOR_EVENT_WALK_OFF)) return 0;
    *outCount = 0;
    memset(&context, 0, sizeof(context));
    context.mapX = mapX;
    context.mapY = mapY;
    context.thingType = DM1_TRIGGER_SOURCE_PARTY;
    context.objectType = -1;
    context.partyDirection = party->direction;
    context.partyChampionCount = party->championCount;
    context.isAddition = triggerEvent == SENSOR_EVENT_WALK_ON;
    context.partyOnSquare = partySquare ? 1 : 0;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount ||
        !dungeon->maps || !dungeon->tiles ||
        mapX < 0 || mapY < 0 || mapX >= dungeon->maps[mapIndex].width ||
        mapY >= dungeon->maps[mapIndex].height ||
        !dungeon->tiles[mapIndex].squareData) return 0;
    context.squareType =
        (dungeon->tiles[mapIndex].squareData[
            mapX * dungeon->maps[mapIndex].height + mapY] &
            DUNGEON_SQUARE_MASK_TYPE) >> 5;
    /* F0276 checks pre-existing floor weight before walking the sensor
     * chain. A party pass does not qualify for creature/object-only C004/C007. */
    context.squareHasGroup = sensor_square_has_existing_group(
        dungeon, things, mapIndex, mapX, mapY);
    context.squareHasObject = sensor_square_has_existing_floor_weight(
        dungeon, things, mapIndex, mapX, mapY) && !context.squareHasGroup;
    count = F0717_SENSOR_EnumerateOnSquare_Compat(
        dungeon, things, mapIndex, mapX, mapY, sensors);
    for (i = 0; i < count && i < SENSOR_ENUM_CAPACITY; ++i) {
        const struct DungeonSensor_Compat* sensor;
        struct SensorTriggerResult_Compat result;
        if (sensors[i].sensorIndex < 0 ||
            sensors[i].sensorIndex >= things->sensorCount ||
            !things->sensors) continue;
        sensor = &things->sensors[sensors[i].sensorIndex];
        if (!sensor->audible) continue;
        if (sensor->sensorType == DM1_SENSOR_FLOOR_PARTY_POSSESSION) {
            if (!sensor_build_party_possession_context(
                    things, party, leaderHandThing, &possession)) continue;
            context.partyHasObjectType = F0274_SENSOR_IsObjectInPartyPossession_Compat(
                sensor->sensorData, &possession);
        }
        /* F0722 preserves F0276's RevertEffect/HOLD ordering. Unmodelled
         * sensor classes produce no receipt, rather than a guessed sound. */
        if (F0722_SENSOR_EvaluateFloor_Compat(sensor, &context, &result) &&
            result.triggered && result.audible) ++*outCount;
    }
    return 1;
}

int F0714_SENSOR_ListDeserialize_Compat(
    struct SensorEffectList_Compat* list,
    const unsigned char* buf,
    int bufSize)
{
    int i;
    if (list == 0 || buf == 0) return 0;
    if (bufSize < SENSOR_EFFECT_LIST_SERIALIZED_SIZE) return 0;

    memset(list, 0, sizeof(*list));
    list->count = read_i32_le(buf);
    if (list->count < 0 || list->count > SENSOR_EFFECT_LIST_MAX_COUNT) return 0;
    for (i = 0; i < SENSOR_EFFECT_LIST_MAX_COUNT; i++) {
        const unsigned char* slot = buf + 4 + (i * SENSOR_EFFECT_SERIALIZED_SIZE);
        if (i < list->count) {
            F0712_SENSOR_EffectDeserialize_Compat(&list->effects[i], slot, SENSOR_EFFECT_SERIALIZED_SIZE);
        }
    }
    return 1;
}
