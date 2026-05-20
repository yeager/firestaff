/*
 * DM1 V1 Sensor & Trigger System — source-locked to ReDMCSB.
 *
 * Source audit citations:
 *   MOVESENS.C F0268 (line ~1000): SENSOR_AddEvent — maps effect to timed event
 *   MOVESENS.C F0270 (line ~1081): TriggerLocalEffect — rotation/XP effects
 *   MOVESENS.C F0271 (line ~1100): ProcessRotationEffect — deferred rotation
 *   MOVESENS.C F0272 (line ~1154): TriggerEffect — dispatch local vs remote
 *   MOVESENS.C F0275 (line ~1309): IsTriggeredByClickOnWall — wall click switch
 *   MOVESENS.C F0276 (line ~1553): ProcessThingAdditionOrRemoval — floor sensors
 *   TIMELINE.C F0248 (lines 1136-1350): wall event sensors incl. C006 countdown
 *   DEFS.H (lines 1256-1305): sensor type/effect constants, macros
 *   DATA.C (line ~470): G0059_auc_Graphic562_SquareTypeToEventType[7]
 */

#include <string.h>
#include "dm1_v1_sensor_trigger_pc34_compat.h"

/* ----------------------------------------------------------------
 *  G0059_auc_Graphic562_SquareTypeToEventType[7]
 *  Source: DATA.C line ~470
 *  Index: square type (0=wall..6=fakewall)
 *  Value: event type
 * ---------------------------------------------------------------- */
static const int g_squareTypeToEventType[7] = {
    DM1_EVENT_WALL,        /* 0: C00_ELEMENT_WALL       -> C06_EVENT_WALL       */
    DM1_EVENT_CORRIDOR,    /* 1: C01_ELEMENT_CORRIDOR   -> C05_EVENT_CORRIDOR   */
    DM1_EVENT_PIT,         /* 2: C02_ELEMENT_PIT        -> C09_EVENT_PIT        */
    DM1_EVENT_NONE,        /* 3: C03_ELEMENT_STAIRS     -> C00_EVENT_NONE       */
    DM1_EVENT_DOOR,        /* 4: C04_ELEMENT_DOOR       -> C10_EVENT_DOOR       */
    DM1_EVENT_TELEPORTER,  /* 5: C05_ELEMENT_TELEPORTER -> C08_EVENT_TELEPORTER */
    DM1_EVENT_FAKEWALL     /* 6: C06_ELEMENT_FAKEWALL   -> C07_EVENT_FAKEWALL   */
};

/* ================================================================
 *  F0727 — Square type to event type mapping
 * ================================================================ */
int F0727_SENSOR_SquareTypeToEventType_Compat(int squareType) {
    if (squareType < 0 || squareType > 6) return -1;
    return g_squareTypeToEventType[squareType];
}

/* ================================================================
 *  F0728 — HOLD effect resolution
 *  Source: MOVESENS.C multiple locations:
 *    F0275 line ~1523: if (L0756_i_SensorEffect == C03_EFFECT_HOLD) {
 *        L0756_i_SensorEffect = L0753_B_DoNotTriggerSensor ? C01_EFFECT_CLEAR : C00_EFFECT_SET;
 *    F0276 line ~1670: if ((L0778_i_Effect = ...) == C03_EFFECT_HOLD) {
 *        L0778_i_Effect = L0768_B_TriggerSensor ? C00_EFFECT_SET : C01_EFFECT_CLEAR;
 * ================================================================ */
int F0728_SENSOR_ResolveHoldEffect_Compat(int effect, int triggerActive) {
    if (effect == DM1_EFFECT_HOLD) {
        return triggerActive ? DM1_EFFECT_SET : DM1_EFFECT_CLEAR;
    }
    return effect;
}

/* ================================================================
 *  F0720 — Floor sensor type classification
 *  Source: DEFS.H lines 1256-1265 — C000..C009
 * ================================================================ */
int F0720_SENSOR_ClassifyFloorType_Compat(int sensorType, int* outIsFloorSensor) {
    if (!outIsFloorSensor) return 0;
    /* Floor sensor types: 0-9 (in floor context).
     * Type 0 (DISABLED) is also valid — just never triggers.
     * Note: wall/floor sensor type codes overlap (same numbers, different context). */
    *outIsFloorSensor = (sensorType >= 0 && sensorType <= 9);
    return 1;
}

/* ================================================================
 *  F0721 — Wall sensor type classification
 *  Source: DEFS.H lines 1266-1284 — C001..C018, C127
 * ================================================================ */
int F0721_SENSOR_ClassifyWallType_Compat(int sensorType, int* outIsWallSensor) {
    if (!outIsWallSensor) return 0;
    /* Wall sensor types: 1-18 and 127 (champion portrait).
     * Type 0 (DISABLED) is valid but never triggers.
     * Types 5-10, 14-15, 18 are event-triggered (AND/OR gate, launchers, end game)
     * and don't respond to clicks — F0275 skips them via default: goto. */
    *outIsWallSensor = (sensorType == 0) ||
                       (sensorType >= 1 && sensorType <= 18) ||
                       (sensorType == 127);
    return 1;
}

/* ================================================================
 *  F0722 — Floor sensor evaluation
 *  Source: F0276_SENSOR_ProcessThingAdditionOrRemoval (MOVESENS.C)
 *
 *  Per-type switch at ~line 1600 (floor context, L0770 == CM1_CELL_ANY):
 *    C001: if partySquare || hasObject || hasGroup -> skip
 *    C002: if thingType > GROUP || partySquare || hasGroup -> skip
 *    C003: if thingType != PARTY || championCount==0 -> skip;
 *          if data==0: if partySquare -> skip
 *          else: if !isAdd -> trigger=false; else trigger=(data==ordinal(dir))
 *    C004: if data != objType || hasSameTypeObj -> skip
 *    C005: if thingType != PARTY || squareType != STAIRS -> skip
 *    C006: -> skip (group generator — event-triggered, not floor-triggered)
 *    C007: if thingType > GROUP || thingType == PARTY || hasGroup -> skip
 *    C008: if thingType != PARTY -> skip; trigger=isObjectInPartyPossession(data)
 *    C009: if thingType != PARTY || !isAdd || partySquare -> skip; trigger=(data<=version)
 * ================================================================ */
int F0722_SENSOR_EvaluateFloor_Compat(
    const struct DungeonSensor_Compat* sensor,
    const struct FloorSensorContext_Compat* ctx,
    struct SensorTriggerResult_Compat* outResult)
{
    int triggerSensor;
    int effect;

    if (!sensor || !ctx || !outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));

    if (sensor->sensorType == DM1_SENSOR_DISABLED) return 1; /* Skip, valid but no trigger */

    triggerSensor = ctx->isAddition; /* Default: trigger on addition */
    effect = sensor->effect;

    switch (sensor->sensorType) {
    case DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT:
        /* Source: F0276 case C001
         * if (P0591_B_PartySquare || L0772_B_SquareContainsObject || L0773_B_SquareContainsGroup)
         *     goto T0276079; */
        if (ctx->partyOnSquare || ctx->squareHasObject || ctx->squareHasGroup)
            return 1; /* No trigger */
        break;

    case DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE:
        /* Source: F0276 case C002
         * if ((L0767_i_ThingType > C04_THING_TYPE_GROUP) || P0591_B_PartySquare || L0773_B_SquareContainsGroup)
         *     goto T0276079; */
        if (ctx->thingType == DM1_TRIGGER_SOURCE_OBJECT ||
            ctx->thingType == DM1_TRIGGER_SOURCE_PROJECTILE ||
            ctx->partyOnSquare || ctx->squareHasGroup)
            return 1;
        break;

    case DM1_SENSOR_FLOOR_PARTY:
        /* Source: F0276 case C003
         * if ((L0767_i_ThingType != CM1_THING_TYPE_PARTY) || (G0305_ui_PartyChampionCount == 0))
         *     goto T0276079;
         * if (L0779_i_SensorData == 0) { if (P0591_B_PartySquare) goto skip; }
         * else { if (!P0592_B_AddThing) trigger=false;
         *        else trigger = (data == INDEX_TO_ORDINAL(partyDirection)); } */
        if (ctx->thingType != DM1_TRIGGER_SOURCE_PARTY || ctx->partyChampionCount == 0)
            return 1;
        if (sensor->sensorData == 0) {
            if (ctx->partyOnSquare) return 1;
        } else {
            if (!ctx->isAddition) {
                triggerSensor = 0;
            } else {
                /* M000_INDEX_TO_ORDINAL(dir) = dir + 1 */
                triggerSensor = ((int)sensor->sensorData == (ctx->partyDirection + 1));
            }
        }
        break;

    case DM1_SENSOR_FLOOR_OBJECT:
        /* Source: F0276 case C004
         * if ((L0779_i_SensorData != F0032_OBJECT_GetType(P0590_T_Thing)) || L0775_B_SquareContainsThingOfSameType)
         *     goto T0276079; */
        if ((int)sensor->sensorData != ctx->objectType || ctx->squareHasSameTypeObj)
            return 1;
        break;

    case DM1_SENSOR_FLOOR_PARTY_ON_STAIRS:
        /* Source: F0276 case C005 (CHANGE8_05_FIX version)
         * if ((L0767_i_ThingType != CM1_THING_TYPE_PARTY) || (SQUARE_TYPE(square) != C03_ELEMENT_STAIRS))
         *     goto T0276079; */
        if (ctx->thingType != DM1_TRIGGER_SOURCE_PARTY || ctx->squareType != DM1_SQUARE_STAIRS)
            return 1;
        break;

    case DM1_SENSOR_FLOOR_GROUP_GENERATOR:
        /* Source: F0276 case C006 -> goto T0276079
         * Group generators are event-triggered (F0245), not floor-triggered. */
        return 1;

    case DM1_SENSOR_FLOOR_CREATURE:
        /* Source: F0276 case C007
         * if ((L0767_i_ThingType > C04_THING_TYPE_GROUP) || (L0767_i_ThingType == CM1_THING_TYPE_PARTY) || L0773_B_SquareContainsGroup)
         *     goto T0276079;
         * Only creature groups trigger this — not party, objects, or projectiles. */
        if (ctx->thingType != DM1_TRIGGER_SOURCE_CREATURE ||
            ctx->thingType == DM1_TRIGGER_SOURCE_PARTY ||
            ctx->squareHasGroup)
            return 1;
        break;

    case DM1_SENSOR_FLOOR_PARTY_POSSESSION:
        /* Source: F0276 case C008
         * if (L0767_i_ThingType != CM1_THING_TYPE_PARTY) goto skip;
         * L0768_B_TriggerSensor = F0274_SENSOR_IsObjectInPartyPossession(data);
         * F0274 scans living champion slots, chest contents, then leader hand. */
        if (ctx->thingType != DM1_TRIGGER_SOURCE_PARTY)
            return 1;
        triggerSensor = ctx->partyHasObjectType;
        break;

    case DM1_SENSOR_FLOOR_VERSION_CHECKER:
        /* Source: F0276 case C009
         * if ((L0767_i_ThingType != CM1_THING_TYPE_PARTY) || !P0592_B_AddThing || P0591_B_PartySquare)
         *     goto T0276079;
         * L0768_B_TriggerSensor = (data <= 20); (DM1 version 2.0) */
        if (ctx->thingType != DM1_TRIGGER_SOURCE_PARTY || !ctx->isAddition || ctx->partyOnSquare)
            return 1;
        triggerSensor = ((int)sensor->sensorData <= 20); /* DM1 V1 engine version */
        break;

    default:
        return 1; /* Unknown type — skip */
    }

    /* Source: F0276 line ~1668:
     * L0768_B_TriggerSensor ^= L0769_ps_Sensor->Remote.RevertEffect; */
    triggerSensor ^= sensor->revertEffect;

    /* Source: F0276 line ~1669-1673: HOLD resolution */
    if (effect == DM1_EFFECT_HOLD) {
        effect = triggerSensor ? DM1_EFFECT_SET : DM1_EFFECT_CLEAR;
    } else {
        if (!triggerSensor)
            return 1; /* No trigger */
    }

    /* Sensor triggers */
    outResult->triggered = 1;
    outResult->resolvedEffect = effect;
    outResult->audible = sensor->audible;
    outResult->delayTicks = sensor->value;
    outResult->sensorIndex = -1; /* Caller must set */

    /* Once-only handling: Source F0272 line ~1180:
     * if (P0575_ps_Sensor->Remote.OnceOnly) M044_SET_TYPE_DISABLED(sensor); */
    if (sensor->onceOnly) {
        outResult->sensorDisabled = 1;
    }

    /* Dispatch: local vs remote (Source: F0272 line ~1190-1207) */
    if (sensor->localEffect) {
        outResult->isLocal = 1;
        outResult->localEffectValue = sensor->localMultiple;
        if (sensor->localMultiple == DM1_EFFECT_ADD_300XP_STEAL_SKILL) {
            outResult->effectKind = SENSOR_EFFECT_ADD_XP;
        } else {
            outResult->effectKind = SENSOR_EFFECT_ROTATION;
        }
    } else {
        outResult->isLocal = 0;
        outResult->targetMapX = sensor->targetMapX;
        outResult->targetMapY = sensor->targetMapY;
        outResult->targetCell = sensor->targetCell;
        /* Effect kind based on resolved effect */
        switch (effect) {
        case DM1_EFFECT_SET:    outResult->effectKind = SENSOR_EFFECT_SET_TARGET; break;
        case DM1_EFFECT_CLEAR:  outResult->effectKind = SENSOR_EFFECT_CLEAR_TARGET; break;
        case DM1_EFFECT_TOGGLE: outResult->effectKind = SENSOR_EFFECT_TOGGLE_TARGET; break;
        default: outResult->effectKind = SENSOR_EFFECT_TOGGLE_TARGET; break;
        }
    }

    return 1;
}

/* ================================================================
 *  F0723 — Wall sensor evaluation
 *  Source: F0275_SENSOR_IsTriggeredByClickOnWall (MOVESENS.C line ~1309)
 *
 *  Per-type switch at ~line 1399:
 *    C001: doNotTrigger=false; if effect==HOLD -> skip
 *    C002: doNotTrigger = (leaderEmptyHanded != sensor.revertEffect)
 *    C017, C011: if sensorCountInCell > 0 -> skip (not last sensor)
 *    C003, C004: doNotTrigger = ((data==objType(hand)) == revertEffect)
 *    C012: if sensorCountInCell > 0 -> skip; doNotTrigger = !leaderEmpty
 *    C013: single object storage + rotate
 *    C016: object exchanger
 *    C127: champion portrait -> special handling, always triggers
 *    C005-C010, C014-C015: -> skip (event-triggered, not click-triggered)
 * ================================================================ */
int F0723_SENSOR_EvaluateWall_Compat(
    const struct DungeonSensor_Compat* sensor,
    const struct WallSensorContext_Compat* ctx,
    struct SensorTriggerResult_Compat* outResult)
{
    int doNotTrigger = 0;
    int effect;
    int sensorType;
    int storageAction = 0;
    int storageObjectType = -1;
    int exchangerAction = 0;
    int exchangerSquareObjectType = -1;

    if (!sensor || !ctx || !outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));

    sensorType = sensor->sensorType;
    if (sensorType == DM1_SENSOR_DISABLED) return 1;

    /* Source: F0275 line ~1392:
     * if ((G0411_i_LeaderIndex == CM1_CHAMPION_NONE) && (sensorType != C127))
     *     goto skip; */
    if (ctx->leaderIndex < 0 && sensorType != DM1_SENSOR_WALL_CHAMPION_PORTRAIT)
        return 1;

    effect = sensor->effect;

    switch (sensorType) {
    case DM1_SENSOR_WALL_ORNAMENT_CLICK:
        /* Source: F0275 case C001
         * doNotTrigger = false;
         * if (effect == C03_EFFECT_HOLD) goto skip; */
        doNotTrigger = 0;
        if (effect == DM1_EFFECT_HOLD) return 1;
        break;

    case DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT:
        /* Source: F0275 case C002
         * doNotTrigger = (leaderEmptyHanded != revertEffect) */
        doNotTrigger = (ctx->leaderEmptyHanded != sensor->revertEffect);
        break;

    case DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_REMOVE_SENSOR:
    case DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE:
        /* Source: F0275 cases C017, C011
         * if (sensorCountInCell > 0) goto skip; (not last sensor on cell)
         * Falls through to C003/C004 logic. */
        if (ctx->sensorCountInCell > 0) return 1;
        /* FALLTHROUGH */
    case DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT:
    case DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED:
        /* Source: F0275 cases C003, C004
         * doNotTrigger = ((data == F0032_OBJECT_GetType(leaderHandObj)) == revertEffect) */
        doNotTrigger = (((int)sensor->sensorData == ctx->leaderHandObjectType) == sensor->revertEffect);
        break;

    case DM1_SENSOR_WALL_OBJECT_GENERATOR_ROTATE:
        /* Source: F0275 case C012
         * if (sensorCountInCell > 0) goto skip;
         * doNotTrigger = !leaderEmptyHanded; */
        if (ctx->sensorCountInCell > 0) return 1;
        doNotTrigger = !ctx->leaderEmptyHanded;
        break;

    case DM1_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE:
        /* Source: F0275 case C013 — complex object storage logic.
         * We model the trigger/no-trigger decision; actual object
         * manipulation is for the caller.
         * Source lines 1464-1477:
         *   empty hand: find matching object in the clicked wall cell, unlink it,
         *               and put it in the leader hand; no object means skip.
         *   occupied hand: require matching type and no matching stored object,
         *                  remove from hand, link to the clicked wall cell.
         *   both paths schedule deferred sensor rotation.
         * Source lines 1478-1487:
         *   HOLD resolves from the original hand state: pickup -> SET, store -> CLEAR. */
        if (ctx->leaderEmptyHanded) {
            if (!ctx->cellHasStorageObjectOfType) return 1;
            storageAction = 1; /* take matching object from wall cell into leader hand */
        } else {
            if (ctx->leaderHandObjectType != (int)sensor->sensorData || ctx->cellHasStorageObjectOfType)
                return 1;
            storageAction = 2; /* store leader hand object in wall cell */
        }
        storageObjectType = (int)sensor->sensorData;
        doNotTrigger = (effect == DM1_EFFECT_HOLD) && !ctx->leaderEmptyHanded;
        break;

    case DM1_SENSOR_WALL_OBJECT_EXCHANGER:
        /* Source: F0275 case C016
         * if (sensorCountInCell > 0) goto skip;
         * L0762 = F0162_DUNGEON_GetSquareFirstObject(mapX,mapY);
         * if (objType(hand) != data || L0762 == NONE) goto skip;
         * unlink square object, remove leader-hand object, link leader-hand
         * object into the clicked wall cell, and put the old square object in
         * the leader hand. */
        if (ctx->sensorCountInCell > 0) return 1;
        if (ctx->leaderHandObjectType != (int)sensor->sensorData)
            return 1; /* Wrong object type */
        if (!ctx->squareHasObject)
            return 1;
        exchangerAction = 1;
        exchangerSquareObjectType = ctx->squareObjectType;
        doNotTrigger = 0;
        break;

    case DM1_SENSOR_WALL_CHAMPION_PORTRAIT:
        /* Source: F0275 case C127
         * F0280_CHAMPION_AddCandidateChampionToParty(data);
         * goto skip; (no remote trigger — handled specially) */
        outResult->triggered = 1;
        outResult->effectKind = SENSOR_EFFECT_CHAMPION;
        outResult->resolvedEffect = DM1_EFFECT_NONE;
        outResult->targetMapX = 0;
        outResult->targetMapY = 0;
        outResult->targetCell = 0;
        outResult->audible = 0;
        outResult->sensorIndex = -1;
        return 1;

    /* Event-triggered sensors — don't respond to clicks */
    case DM1_SENSOR_WALL_AND_OR_GATE:
    case DM1_SENSOR_WALL_COUNTDOWN:
    case DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ:
    case DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION:
    case DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ:
    case DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION:
    case DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ:
    case DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ:
    case DM1_SENSOR_WALL_END_GAME:
        return 1; /* Skip — event-triggered only */

    default:
        return 1;
    }

    /* Source: F0275 line ~1523: HOLD resolution */
    if (effect == DM1_EFFECT_HOLD) {
        effect = doNotTrigger ? DM1_EFFECT_CLEAR : DM1_EFFECT_SET;
        doNotTrigger = 0;
    }

    if (doNotTrigger)
        return 1;

    /* Sensor triggers */
    outResult->triggered = 1;
    outResult->resolvedEffect = effect;
    outResult->audible = sensor->audible;
    outResult->delayTicks = sensor->value;
    outResult->sensorIndex = -1;
    outResult->leaderHandObjectRemoved = 0;
    outResult->leaderHandObjectTypeRemoved = -1;
    outResult->leaderHandObjectReceived = 0;
    outResult->leaderHandObjectTypeReceived = -1;
    outResult->wallStorageObjectType = -1;
    outResult->wallObjectTypeTaken = -1;
    outResult->wallObjectTypeStored = -1;

    /* Source: F0275 lines 1527-1531.  C004/C011/C017 key-slot
     * sensors consume the leader hand object only after the wall sensor
     * actually triggers; C003 is a non-consuming specific-object click. */
    if (!ctx->leaderEmptyHanded &&
        (sensorType == DM1_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED ||
         sensorType == DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE ||
         sensorType == DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_REMOVE_SENSOR)) {
        outResult->leaderHandObjectRemoved = 1;
        outResult->leaderHandObjectTypeRemoved = ctx->leaderHandObjectType;
    }

    if (storageAction == 1) {
        outResult->leaderHandObjectReceived = 1;
        outResult->leaderHandObjectTypeReceived = storageObjectType;
        outResult->wallStorageObjectTaken = 1;
        outResult->wallStorageObjectType = storageObjectType;
    } else if (storageAction == 2) {
        outResult->leaderHandObjectRemoved = 1;
        outResult->leaderHandObjectTypeRemoved = ctx->leaderHandObjectType;
        outResult->wallStorageObjectStored = 1;
        outResult->wallStorageObjectType = storageObjectType;
    }

    if (exchangerAction) {
        outResult->leaderHandObjectRemoved = 1;
        outResult->leaderHandObjectTypeRemoved = ctx->leaderHandObjectType;
        outResult->leaderHandObjectReceived = 1;
        outResult->leaderHandObjectTypeReceived = exchangerSquareObjectType;
        outResult->wallObjectTaken = 1;
        outResult->wallObjectTypeTaken = exchangerSquareObjectType;
        outResult->wallObjectStored = 1;
        outResult->wallObjectTypeStored = ctx->leaderHandObjectType;
    }

    if (sensor->onceOnly) {
        outResult->sensorDisabled = 1;
    }

    /* Local vs remote dispatch (same as floor) */
    if (sensor->localEffect) {
        outResult->isLocal = 1;
        outResult->localEffectValue = sensor->localMultiple;
        if (sensor->localMultiple == DM1_EFFECT_ADD_300XP_STEAL_SKILL) {
            outResult->effectKind = SENSOR_EFFECT_ADD_XP;
        } else {
            outResult->effectKind = SENSOR_EFFECT_ROTATION;
        }
    } else {
        outResult->isLocal = 0;
        outResult->targetMapX = sensor->targetMapX;
        outResult->targetMapY = sensor->targetMapY;
        outResult->targetCell = sensor->targetCell;
        switch (effect) {
        case DM1_EFFECT_SET:    outResult->effectKind = SENSOR_EFFECT_SET_TARGET; break;
        case DM1_EFFECT_CLEAR:  outResult->effectKind = SENSOR_EFFECT_CLEAR_TARGET; break;
        case DM1_EFFECT_TOGGLE: outResult->effectKind = SENSOR_EFFECT_TOGGLE_TARGET; break;
        default: outResult->effectKind = SENSOR_EFFECT_TOGGLE_TARGET; break;
        }
    }

    /* If wall sensors C011 or C012 triggered, also request rotation.
     * Source: F0275 lines ~1429, ~1453, ~1461:
     *   F0270_SENSOR_TriggerLocalEffect(C02_EFFECT_TOGGLE, ...) */
    if (sensorType == DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE ||
        sensorType == DM1_SENSOR_WALL_OBJECT_GENERATOR_ROTATE ||
        sensorType == DM1_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE) {
        /* Caller should note that rotation is also triggered */
        /* We'll handle this in ProcessWallClick */
    }

    return 1;
}

/* ================================================================
 *  F0724 — Effect dispatch resolution
 *  Source: F0272_SENSOR_TriggerEffect (MOVESENS.C line ~1154)
 * ================================================================ */
int F0724_SENSOR_ResolveEffectDispatch_Compat(
    const struct DungeonSensor_Compat* sensor,
    int resolvedEffect,
    int targetSquareType,
    int sensorMapX,
    int sensorMapY,
    struct SensorTriggerResult_Compat* outResult)
{
    if (!sensor || !outResult) return 0;

    outResult->resolvedEffect = resolvedEffect;
    outResult->delayTicks = sensor->value;

    /* Source: F0272 line ~1180: once-only check */
    if (sensor->onceOnly) {
        outResult->sensorDisabled = 1;
    }

    /* Source: F0272 line ~1190-1196: if local effect -> TriggerLocalEffect */
    if (sensor->localEffect) {
        outResult->isLocal = 1;
        outResult->localEffectValue = sensor->localMultiple;
        outResult->targetMapX = sensorMapX;
        outResult->targetMapY = sensorMapY;
        if (sensor->localMultiple == DM1_EFFECT_ADD_300XP_STEAL_SKILL) {
            outResult->effectKind = SENSOR_EFFECT_ADD_XP;
        } else {
            outResult->effectKind = SENSOR_EFFECT_ROTATION;
        }
    } else {
        /* Source: F0272 line ~1198-1207: remote target */
        outResult->isLocal = 0;
        outResult->targetMapX = sensor->targetMapX;
        outResult->targetMapY = sensor->targetMapY;

        /* Source: F0272 line ~1200-1205:
         * if (squareType == C00_ELEMENT_WALL) targetCell = sensor->targetCell;
         * else targetCell = C00_CELL_NORTHWEST; */
        if (targetSquareType == DM1_SQUARE_WALL) {
            outResult->targetCell = sensor->targetCell;
        } else {
            outResult->targetCell = 0; /* C00_CELL_NORTHWEST */
        }

        outResult->targetSquareType = targetSquareType;
        outResult->targetEventType = F0727_SENSOR_SquareTypeToEventType_Compat(targetSquareType);

        switch (resolvedEffect) {
        case DM1_EFFECT_SET:    outResult->effectKind = SENSOR_EFFECT_SET_TARGET; break;
        case DM1_EFFECT_CLEAR:  outResult->effectKind = SENSOR_EFFECT_CLEAR_TARGET; break;
        case DM1_EFFECT_TOGGLE: outResult->effectKind = SENSOR_EFFECT_TOGGLE_TARGET; break;
        default: outResult->effectKind = SENSOR_EFFECT_TOGGLE_TARGET; break;
        }
    }

    return 1;
}


/* ================================================================
 *  F0729 -- Wall countdown sensor event evaluation
 *  Source: TIMELINE.C F0248 lines 1198-1266, C006_SENSOR_WALL_COUNTDOWN.
 *
 *  ReDMCSB mutates M040_DATA in place. This pure helper reports the
 *  before/after counter values; the caller applies the sensor-data write
 *  and any F0272-triggered effect to the world/event queue.
 * ================================================================ */
int F0729_SENSOR_EvaluateWallCountdownEvent_Compat(
    const struct DungeonSensor_Compat* sensor,
    int eventEffect,
    int sensorMapX,
    int sensorMapY,
    int sensorCell,
    struct SensorTriggerResult_Compat* outResult)
{
    int data;
    int before;
    int resolvedEffect;

    if (!sensor || !outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    outResult->sensorIndex = -1;
    outResult->sensorDataBefore = (int)sensor->sensorData;
    outResult->sensorDataAfter = (int)sensor->sensorData;

    if (sensor->sensorType != DM1_SENSOR_WALL_COUNTDOWN) return 1;

    before = (int)sensor->sensorData;
    data = before;
    if (data <= 0) return 1;

    /* Source: TIMELINE.C:1203-1211. SET increments up to 511; every
     * other event effect decrements. */
    if (eventEffect == DM1_EFFECT_SET) {
        if (data < 511) {
            data++;
        }
    } else {
        data--;
    }

    outResult->sensorDataAfter = data;
    outResult->sensorDataChanged = (data != before);

    /* Source: TIMELINE.C:1212-1263. HOLD dispatches SET/CLEAR on every
     * nonzero starting count event; non-HOLD dispatches only when the
     * updated counter reaches zero. */
    if (sensor->effect == DM1_EFFECT_HOLD) {
        int triggerSetEffect = ((data == 0) != (sensor->revertEffect != 0));
        resolvedEffect = triggerSetEffect ? DM1_EFFECT_SET : DM1_EFFECT_CLEAR;
    } else {
        if (data != 0) return 1;
        resolvedEffect = sensor->effect;
    }

    outResult->triggered = 1;
    outResult->resolvedEffect = resolvedEffect;
    outResult->audible = sensor->audible;
    outResult->delayTicks = sensor->value;
    if (sensor->onceOnly) {
        outResult->sensorDisabled = 1;
    }

    if (sensor->localEffect) {
        outResult->isLocal = 1;
        outResult->localEffectValue = sensor->localMultiple;
        outResult->targetMapX = sensorMapX;
        outResult->targetMapY = sensorMapY;
        outResult->targetCell = sensorCell;
        if (sensor->localMultiple == DM1_EFFECT_ADD_300XP_STEAL_SKILL) {
            outResult->effectKind = SENSOR_EFFECT_ADD_XP;
        } else {
            outResult->effectKind = SENSOR_EFFECT_ROTATION;
        }
    } else {
        outResult->isLocal = 0;
        outResult->targetMapX = sensor->targetMapX;
        outResult->targetMapY = sensor->targetMapY;
        outResult->targetCell = sensor->targetCell;
        switch (resolvedEffect) {
        case DM1_EFFECT_SET:    outResult->effectKind = SENSOR_EFFECT_SET_TARGET; break;
        case DM1_EFFECT_CLEAR:  outResult->effectKind = SENSOR_EFFECT_CLEAR_TARGET; break;
        case DM1_EFFECT_TOGGLE: outResult->effectKind = SENSOR_EFFECT_TOGGLE_TARGET; break;
        default: outResult->effectKind = SENSOR_EFFECT_TOGGLE_TARGET; break;
        }
    }

    return 1;
}


static int F0730_IsProjectileLauncherType_Compat(int sensorType) {
    return ((sensorType >= DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ &&
             sensorType <= DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION) ||
            sensorType == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ ||
            sensorType == DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ);
}

static int F0730_IsSingleProjectileLauncherType_Compat(int sensorType) {
    return (sensorType == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ ||
            sensorType == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION ||
            sensorType == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ);
}

static int F0730_IsExplosionProjectileLauncherType_Compat(int sensorType) {
    return (sensorType == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION ||
            sensorType == DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION);
}

static int F0730_IsSquareObjectProjectileLauncherType_Compat(int sensorType) {
    return (sensorType == DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ ||
            sensorType == DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ);
}

static int F0730_NormalizeCell_Compat(int cell) {
    return cell & 3;
}

static int F0730_NextCell_Compat(int cell) {
    return (cell + 1) & 3;
}

static int F0730_OppositeCell_Compat(int cell) {
    return (cell + 2) & 3;
}

static unsigned short F0730_SelectSquareProjectileThing_Compat(
    const struct ProjectileLauncherContext_Compat* ctx,
    int eventCell,
    unsigned short skipThing)
{
    int i;
    int nextCell;

    if (!ctx || !ctx->squareThings || ctx->squareThingCount <= 0) return THING_NONE;

    nextCell = F0730_NextCell_Compat(eventCell);
    for (i = 0; i < ctx->squareThingCount; ++i) {
        const struct ProjectileLauncherSquareThing_Compat* t = &ctx->squareThings[i];
        int thingType;
        int thingCell;

        if (t->thing == THING_NONE || t->thing == THING_ENDOFLIST || t->thing == skipThing) continue;
        thingType = (t->thingType >= 0) ? t->thingType : (int)THING_GET_TYPE(t->thing);
        thingCell = (t->cell >= 0) ? t->cell : (int)THING_GET_CELL(t->thing);
        if (thingType > THING_TYPE_SENSOR &&
            (F0730_NormalizeCell_Compat(thingCell) == eventCell ||
             F0730_NormalizeCell_Compat(thingCell) == nextCell)) {
            return t->thing;
        }
    }

    return THING_NONE;
}

static void F0730_RecordProjectileLaunch_Compat(
    struct ProjectileLauncherResult_Compat* outResult,
    int index,
    unsigned short associatedThing,
    int mapX,
    int mapY,
    int cell,
    int direction,
    int kineticEnergy,
    int stepEnergy)
{
    struct ProjectileLauncherLaunch_Compat* launch;

    if (!outResult || index < 0 || index >= DM1_PROJECTILE_LAUNCHER_MAX_LAUNCHES) return;
    launch = &outResult->launches[index];
    memset(launch, 0, sizeof(*launch));
    launch->valid = 1;
    launch->associatedThing = associatedThing;
    launch->mapX = mapX;
    launch->mapY = mapY;
    launch->cell = cell;
    launch->direction = direction;
    launch->kineticEnergy = kineticEnergy;
    launch->attack = DM1_PROJECTILE_LAUNCHER_ATTACK;
    launch->stepEnergy = stepEnergy;
}

/* ================================================================
 *  F0730 -- Wall projectile launcher event evaluation
 *  Source: TIMELINE.C F0247 lines 1033-1133 and F0248 lines 1312-1316.
 *
 *  ReDMCSB creates the projectile(s) immediately from a wall event on the
 *  same cell. This pure helper reports the F0212_PROJECTILE_Create calls
 *  and any source-square object unlinking the caller must apply.
 * ================================================================ */
int F0730_SENSOR_EvaluateWallProjectileLauncherEvent_Compat(
    const struct DungeonSensor_Compat* sensor,
    int sensorCell,
    int eventMapX,
    int eventMapY,
    int eventCell,
    const struct ProjectileLauncherContext_Compat* ctx,
    struct ProjectileLauncherResult_Compat* outResult)
{
    static const int stepEast[4] = { 0, 1, 0, -1 };
    static const int stepNorth[4] = { -1, 0, 1, 0 };
    int sensorType;
    int launchSingle;
    int projectileCell;
    int projectileMapX;
    int projectileMapY;
    int kineticEnergy;
    int stepEnergy;
    unsigned short firstThing = THING_NONE;
    unsigned short secondThing = THING_NONE;

    if (!sensor || !outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));

    sensorType = sensor->sensorType;
    eventCell = F0730_NormalizeCell_Compat(eventCell);
    sensorCell = F0730_NormalizeCell_Compat(sensorCell);
    if (!F0730_IsProjectileLauncherType_Compat(sensorType) || sensorCell != eventCell) {
        return 1;
    }

    outResult->triggered = 1;
    outResult->sensorDisabled = sensor->onceOnly ? 1 : 0;
    outResult->launcherType = sensorType;

    launchSingle = F0730_IsSingleProjectileLauncherType_Compat(sensorType);
    projectileCell = F0730_OppositeCell_Compat(eventCell);
    outResult->projectileCellBase = projectileCell;

    kineticEnergy = (int)(sensor->localMultiple & 0x00FFu);
    stepEnergy = (int)((sensor->localMultiple >> 8) & 0x00FFu);

    if (F0730_IsExplosionProjectileLauncherType_Compat(sensorType)) {
        firstThing = secondThing = (unsigned short)(DM1_THING_FIRST_EXPLOSION + sensor->sensorData);
    } else if (F0730_IsSquareObjectProjectileLauncherType_Compat(sensorType)) {
        firstThing = F0730_SelectSquareProjectileThing_Compat(ctx, eventCell, THING_NONE);
        if (firstThing == THING_NONE) return 1;
        outResult->unlinkThings[outResult->unlinkCount++] = firstThing;
        if (!launchSingle) {
            secondThing = F0730_SelectSquareProjectileThing_Compat(ctx, eventCell, firstThing);
            if (secondThing == THING_NONE) {
                launchSingle = 1;
            } else {
                outResult->unlinkThings[outResult->unlinkCount++] = secondThing;
            }
        }
    } else {
        if (!ctx || ctx->newObjectThings[0] == THING_NONE) return 1;
        firstThing = ctx->newObjectThings[0];
        if (!launchSingle) {
            secondThing = ctx->newObjectThings[1];
            if (secondThing == THING_NONE) {
                launchSingle = 1;
            }
        }
    }

    if (launchSingle) {
        int randomBit = ctx ? (ctx->randomBit & 1) : 0;
        projectileCell = F0730_NormalizeCell_Compat(projectileCell + randomBit);
    }
    outResult->launchSingleProjectile = launchSingle ? 1 : 0;

    projectileMapX = eventMapX + stepEast[eventCell];
    projectileMapY = eventMapY + stepNorth[eventCell];

    F0730_RecordProjectileLaunch_Compat(outResult, 0, firstThing,
        projectileMapX, projectileMapY, projectileCell, eventCell,
        kineticEnergy, stepEnergy);
    outResult->launchCount = 1;

    if (!launchSingle) {
        F0730_RecordProjectileLaunch_Compat(outResult, 1, secondThing,
            projectileMapX, projectileMapY, F0730_NextCell_Compat(projectileCell), eventCell,
            kineticEnergy, stepEnergy);
        outResult->launchCount = 2;
    }

    return 1;
}

/* ================================================================
 *  F0730 - Event-triggered wall AND/OR gate sensor C005
 *  Source: TIMELINE.C F0248 lines 1268-1309.
 *  The low nibble is the current input mask; the high nibble is the
 *  reference mask. The incoming wall event cell chooses the bit to
 *  SET/CLEAR/TOGGLE, then the gate fires when current == reference,
 *  inverted by Remote.RevertEffect. HOLD always dispatches SET/CLEAR;
 *  non-HOLD dispatches only when the comparison is true.
 * ================================================================ */
int F0730_SENSOR_EvaluateWallAndOrGateEvent_Compat(
    const struct DungeonSensor_Compat* sensor,
    int eventCell,
    int eventEffect,
    int targetSquareType,
    int sensorMapX,
    int sensorMapY,
    struct SensorTriggerResult_Compat* outResult)
{
    int data;
    int bitMask;
    int currentMask;
    int referenceMask;
    int triggerSetEffect;
    int resolvedEffect;

    if (!sensor || !outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    outResult->sensorIndex = -1;
    outResult->sensorDataBefore = (int)sensor->sensorData;
    outResult->sensorDataAfter = (int)sensor->sensorData;

    if (sensor->sensorType != DM1_SENSOR_WALL_AND_OR_GATE) return 1;
    if (eventCell < 0 || eventCell > 3) return 0;

    data = (int)sensor->sensorData & 0x01FF;
    bitMask = 1 << eventCell;
    if (eventEffect == DM1_EFFECT_TOGGLE) {
        if ((data & bitMask) != 0) data &= ~bitMask;
        else data |= bitMask;
    } else if (eventEffect != DM1_EFFECT_SET) {
        data &= ~bitMask;
    } else {
        data |= bitMask;
    }
    data &= 0x01FF;

    currentMask = data & 0x000F;
    referenceMask = (data & 0x00F0) >> 4;
    triggerSetEffect = (currentMask == referenceMask) != sensor->revertEffect;

    outResult->sensorDataAfter = data;
    outResult->gateBitMask = bitMask;
    outResult->gateCurrentMask = currentMask;
    outResult->gateReferenceMask = referenceMask;
    outResult->gateTriggerSetEffect = triggerSetEffect;

    if (sensor->effect == DM1_EFFECT_HOLD) {
        resolvedEffect = triggerSetEffect ? DM1_EFFECT_SET : DM1_EFFECT_CLEAR;
    } else {
        if (!triggerSetEffect) return 1;
        resolvedEffect = sensor->effect;
    }

    if (!F0724_SENSOR_ResolveEffectDispatch_Compat(
            sensor, resolvedEffect, targetSquareType,
            sensorMapX, sensorMapY, outResult)) {
        return 0;
    }
    outResult->triggered = 1;
    outResult->sensorDataAfter = data;
    outResult->gateBitMask = bitMask;
    outResult->gateCurrentMask = currentMask;
    outResult->gateReferenceMask = referenceMask;
    outResult->gateTriggerSetEffect = triggerSetEffect;
    return 1;
}

/* ================================================================
 *  F0731 - Event-triggered wall end-game sensor C018
 *  Source: TIMELINE.C F0248 lines 1317-1339.
 *
 *  Unlike projectile launcher sensors, the C018 branch does not test the
 *  sensor thing cell against the incoming wall event cell. It ignores the
 *  incoming event effect and the sensor's remote effect/target fields,
 *  marks the game as won, and enters the end-game presentation path. Some
 *  media branches clear restart permission and/or apply 60 * Value delay
 *  before drawing the end-game screen.
 * ================================================================ */
int F0731_SENSOR_EvaluateWallEndGameEvent_Compat(
    const struct DungeonSensor_Compat* sensor,
    int sensorCell,
    int eventEffect,
    int eventCell,
    struct SensorTriggerResult_Compat* outResult)
{
    (void)sensorCell;
    (void)eventEffect;
    (void)eventCell;

    if (!sensor || !outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    outResult->sensorIndex = -1;

    if (sensor->sensorType != DM1_SENSOR_WALL_END_GAME) return 1;

    outResult->triggered = 1;
    outResult->effectKind = SENSOR_EFFECT_END_GAME;
    outResult->resolvedEffect = DM1_EFFECT_NONE;
    outResult->endGameGameWon = 1;
    outResult->endGameRestartGameAllowedCleared = 1;
    outResult->endGamePresentationRequested = 1;
    outResult->endGameDelayTicks = 60 * (int)sensor->value;
    outResult->delayTicks = outResult->endGameDelayTicks;
    return 1;
}

/* ================================================================
 *  F0725 — Process all floor sensors on a square
 *  Source: F0276_SENSOR_ProcessThingAdditionOrRemoval outer loop
 * ================================================================ */
int F0725_SENSOR_ProcessFloorSquare_Compat(
    const struct SensorOnSquare_Compat* sensors,
    int sensorCount,
    const struct DungeonSensor_Compat* sensorData,
    int sensorDataCount,
    const struct FloorSensorContext_Compat* ctx,
    struct SensorTriggerResultList_Compat* outList)
{
    int i;

    if (!outList) return 0;
    memset(outList, 0, sizeof(*outList));
    outList->rotationPending = 0;
    outList->rotationEffect = DM1_EFFECT_NONE;
    outList->rotationCell = -1;

    if (!sensors || !sensorData || !ctx) return 0;

    for (i = 0; i < sensorCount && i < SENSOR_ENUM_CAPACITY; ++i) {
        struct SensorTriggerResult_Compat result;
        int sIdx;

        if (!sensors[i].found) continue;
        sIdx = sensors[i].sensorIndex;
        if (sIdx < 0 || sIdx >= sensorDataCount) continue;

        memset(&result, 0, sizeof(result));
        if (!F0722_SENSOR_EvaluateFloor_Compat(&sensorData[sIdx], ctx, &result))
            continue;

        if (result.triggered && outList->count < SENSOR_TRIGGER_RESULT_MAX) {
            result.sensorIndex = sIdx;

            /* Source: F0276 line ~1675: audible -> play switch sound */
            /* (We record it; caller plays the sound.) */

            /* Track rotation pending from local effects */
            if (result.isLocal && result.effectKind == SENSOR_EFFECT_ROTATION) {
                outList->rotationPending = 1;
                outList->rotationEffect = result.localEffectValue;
                outList->rotationMapX = result.targetMapX;
                outList->rotationMapY = result.targetMapY;
                outList->rotationCell = -1; /* CM1_CELL_ANY for floor */
            }

            outList->results[outList->count++] = result;
        }
    }

    return 1;
}

/* ================================================================
 *  F0726 — Process wall click on a square
 *  Source: F0275_SENSOR_IsTriggeredByClickOnWall outer loop
 * ================================================================ */
int F0726_SENSOR_ProcessWallClick_Compat(
    const struct SensorOnSquare_Compat* sensors,
    int sensorCount,
    const struct DungeonSensor_Compat* sensorData,
    int sensorDataCount,
    const struct WallSensorContext_Compat* ctx,
    struct SensorTriggerResultList_Compat* outList)
{
    int i;
    int remainingByCell[4] = { 0, 0, 0, 0 };

    if (!outList) return 0;
    memset(outList, 0, sizeof(*outList));
    outList->rotationPending = 0;
    outList->rotationEffect = DM1_EFFECT_NONE;
    outList->rotationCell = -1;

    if (!sensors || !sensorData || !ctx) return 0;

    /* Source: MOVESENS.C:F0275 first pass counts sensors per cell before
     * processing.  The later C011/C012 gates test the remaining count for
     * the clicked cell only, so unrelated sensors on other cells must not
     * postpone or suppress this cell's last-sensor action.
     */
    for (i = 0; i < sensorCount && i < SENSOR_ENUM_CAPACITY; ++i) {
        if (sensors[i].found) {
            int cell = sensors[i].cell & 3;
            remainingByCell[cell]++;
        }
    }

    for (i = 0; i < sensorCount && i < SENSOR_ENUM_CAPACITY; ++i) {
        struct SensorTriggerResult_Compat result;
        struct WallSensorContext_Compat localCtx;
        int sIdx;

        if (!sensors[i].found) continue;
        sIdx = sensors[i].sensorIndex;
        if (sIdx < 0 || sIdx >= sensorDataCount) continue;

        /* Build local context with remaining same-cell count. */
        localCtx = *ctx;
        localCtx.cell = sensors[i].cell & 3;
        if (remainingByCell[localCtx.cell] > 0) {
            remainingByCell[localCtx.cell]--;
        }
        localCtx.sensorCountInCell = remainingByCell[localCtx.cell];

        memset(&result, 0, sizeof(result));
        if (!F0723_SENSOR_EvaluateWall_Compat(&sensorData[sIdx], &localCtx, &result))
            continue;

        if (result.triggered && outList->count < SENSOR_TRIGGER_RESULT_MAX) {
            result.sensorIndex = sIdx;

            /* Track rotation from wall sensors that request it */
            if (result.isLocal && result.effectKind == SENSOR_EFFECT_ROTATION) {
                outList->rotationPending = 1;
                outList->rotationEffect = DM1_EFFECT_TOGGLE;
                outList->rotationMapX = result.targetMapX;
                outList->rotationMapY = result.targetMapY;
                outList->rotationCell = ctx->cell;
            }

            /* Source: F0275 lines ~1429, ~1453, ~1461:
             * Certain wall sensor types also trigger rotation. */
            if (sensorData[sIdx].sensorType == DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE ||
                sensorData[sIdx].sensorType == DM1_SENSOR_WALL_OBJECT_GENERATOR_ROTATE ||
                sensorData[sIdx].sensorType == DM1_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE) {
                outList->rotationPending = 1;
                outList->rotationEffect = DM1_EFFECT_TOGGLE;
                outList->rotationMapX = ctx->mapX;
                outList->rotationMapY = ctx->mapY;
                outList->rotationCell = localCtx.cell;
                outList->rotationDeferredUntilAfterResultCount = outList->count + 1;
            }

            outList->results[outList->count++] = result;
        }
    }

    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — MOVESENS.C remaining function citations
 *
 *   MOVESENS.C:668 F0413_CPSC_G
 * ══════════════════════════════════════════════════════════════════════ */
