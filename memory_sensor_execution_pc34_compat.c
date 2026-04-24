/*
 * Sensor execution layer for ReDMCSB PC 3.4 — Phase 11 of M10.
 * See header for full documentation.
 */

#include <string.h>
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
    /* We intentionally avoid warnings about unused parameters — dungeon
     * and things are part of the stable API so later phases (text
     * lookup, door resolution) can use them without changing callers. */
    (void)dungeon;
    (void)things;

    if (outList == 0 || sensor == 0) return 0;
    memset(outList, 0, sizeof(*outList));

    if (!sensor->found) return 1; /* Empty list, valid. */

    /* v1 fires effects ONLY on WALK_ON.  Other events produce an empty
     * list for every implemented type; callers can tell "nothing to do"
     * from "not implemented yet" by looking at SENSOR_EFFECT_UNSUPPORTED.
     */
    if (triggerEvent != SENSOR_EVENT_WALK_ON) {
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
    default: {
        /* UNSUPPORTED — explicit marker so callers distinguish from
         * "no effect".  Types 1, 2, 3, 4, 5, 6, 8, 10, 127 will be
         * implemented as timeline / combat / actuator phases land. */
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
                out->isLocal = sensor->localEffect;
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

int F0718_SENSOR_ProcessPartyEnterLeave_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    int triggerEvent,
    struct SensorEffectList_Compat* outList)
{
    struct SensorOnSquare_Compat sensors[SENSOR_ENUM_CAPACITY];
    int sensorCount, i;

    if (!outList) return 0;
    memset(outList, 0, sizeof(*outList));
    if (!dungeon || !things) return 0;

    /* Only WALK_ON / WALK_OFF are meaningful for party enter/leave in
     * v1.  Other events are accepted but produce no effects. */
    if (triggerEvent < 0 || triggerEvent >= SENSOR_EVENT_COUNT) return 0;

    sensorCount = F0717_SENSOR_EnumerateOnSquare_Compat(
        dungeon, things, mapIndex, mapX, mapY, sensors);

    for (i = 0; i < sensorCount && i < SENSOR_ENUM_CAPACITY; ++i) {
        struct SensorEffectList_Compat tmp;
        memset(&tmp, 0, sizeof(tmp));
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
