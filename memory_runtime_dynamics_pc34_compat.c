/*
 * Runtime dynamics data layer for ReDMCSB PC 3.4 — Phase 19 of M10.
 *
 * Pure state transforms for the four liveness event kinds scheduled
 * elsewhere in the timeline:
 *
 *   - GROUP_GENERATOR (F0860)     — mirror of TIMELINE.C:962 path
 *   - MAGIC_LIGHT_DECAY (F0864)   — mirror of F0257 (TIMELINE.C:1720)
 *   - REMOVE_FLUXCAGE  (F0868)    — mirror of C24 inline (TIMELINE.C:1906)
 *   - Generator re-enable (F0872) — mirror of F0246 (TIMELINE.C:1010)
 *
 * Authoritative plan: PHASE19_PLAN.md.
 *
 * ADDITIVE ONLY: consumes ExplosionList_Compat / ExplosionInstance_Compat
 * (Phase 17), TimelineEvent_Compat (Phase 12), RngState_Compat (Phase 13),
 * DungeonSensor_Compat (Phase 9), and MagicState_Compat (Phase 14)
 * without modification.
 */

#include <string.h>
#include <stdint.h>

#include "memory_runtime_dynamics_pc34_compat.h"

/* -------- Invariants guaranteed by platform / plan -------- */

_Static_assert(sizeof(int) == 4,
               "Phase 19 assumes 32-bit int on LE platforms.");
_Static_assert(sizeof(uint32_t) == 4,
               "Phase 19 assumes 4-byte uint32_t.");

/* -------- int32 LE serialisation helpers -------- */

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

/* ==========================================================
 *  PowerOrdinalToLightAmount — Phase 14 placeholder, consumed
 *  verbatim for indices 0..6.
 *
 *  NEEDS DISASSEMBLY REVIEW: real table is
 *  G0039_ai_Graphic562_LightPowerToLightAmount[16] loaded from
 *  GRAPHICS.DAT entry 562. Only indices 0..6 are used by
 *  DM-era spells (power ordinal range). The full 16-entry table
 *  is deferred to post-M10 when the GRAPHICS.DAT loader lands.
 * ========================================================== */

static const int s_PowerOrdinalToLightAmount[RUNTIME_LIGHT_POWER_MAX + 1] = {
    0,   /* index 0: no light / boundary */
    3,   /* index 1 */
    6,   /* index 2 */
    10,  /* index 3 */
    16,  /* index 4 */
    24,  /* index 5 */
    40   /* index 6 */
};

/* ==========================================================
 *  Creature base-health lookup (minimal v1).
 *
 *  Fontanel stores per-creature baseHealth in the CREATURE_INFO
 *  table, indexed by creatureType. Phase 9/16 expose sensor
 *  decode only — no CreatureInfo_Compat table yet. Phase 19 v1
 *  ships a minimal lookup that covers DM1's 27 creature types
 *  (0..26); values are placeholder plausible-range integers
 *  mirroring Fontanel's order-of-magnitude for each kind.
 *  Real values come from CREATURE.C:creatureInfo[] initialiser.
 *
 *  NEEDS DISASSEMBLY REVIEW — specific per-type health values.
 * ========================================================== */

static int runtime_get_creature_base_health(int creatureType) {
    /* Plausible baseline values drawn from Fontanel ranges
     * (Mummy ~75, Dragon ~500, Screamer ~10, etc.). Exact
     * calibration is deferred to CreatureInfo_Compat delivery. */
    static const int s_base[27] = {
        /*  0 Mummy          */  75,
        /*  1 Ghost          */  35,
        /*  2 RockPile       */  55,
        /*  3 Giggler        */  25,
        /*  4 Wasp           */  15,
        /*  5 GiantScorpion  */  85,
        /*  6 AntMan         */  65,
        /*  7 PainRat        */  30,
        /*  8 Ruster         */  40,
        /*  9 Screamer       */  10,
        /* 10 Skeleton       */  45,
        /* 11 Trolin         */  60,
        /* 12 Worm           */  50,
        /* 13 Couatl         */ 110,
        /* 14 StoneGolem     */ 125,
        /* 15 Mummy2         */  95,
        /* 16 BlackFlame     */  80,
        /* 17 Wizard         */ 150,
        /* 18 AnimatedArmour */ 135,
        /* 19 Zytaz          */  90,
        /* 20 Demon          */ 200,
        /* 21 Lord Chaos     */ 250,
        /* 22 RedDragon      */ 500,
        /* 23 Knight         */ 160,
        /* 24 Swamp Slime    */  70,
        /* 25 Giant          */ 180,
        /* 26 Minion         */  40
    };
    if (creatureType < 0 || creatureType > 26) return 0;
    return s_base[creatureType];
}

/* ==========================================================
 *  Group A — Generator lifecycle (F0860 – F0863).
 * ========================================================== */

int F0861_RUNTIME_CheckGeneratorSuppression_Compat(
    const struct GeneratorContext_Compat* ctx,
    int* outSuppressed,
    int* outReason)
{
    if (ctx == 0 || outSuppressed == 0 || outReason == 0) return 0;

    /* Creature type range check (0..26 for DM1). */
    if (ctx->creatureType < 0 || ctx->creatureType > 26) {
        *outSuppressed = 1;
        *outReason = GENERATOR_SUPPRESSION_INVALID_TYPE;
        return 1;
    }

    /*
     * Fontanel GROUP.C:512 — global active-group-count cap on the
     * party map only. No party-proximity/adjacency check exists in
     * WIP source.
     *
     * NEEDS DISASSEMBLY REVIEW — confirm no DM/CSB variant adds
     * adjacency suppression; v1 strictly follows Fontanel WIP.
     */
    if (ctx->isOnPartyMap
        && ctx->currentActiveGroupCount
           >= (ctx->maxActiveGroupCount - 5)) {
        *outSuppressed = 1;
        *outReason = GENERATOR_SUPPRESSION_ACTIVE_GROUP_CAP;
        return 1;
    }

    *outSuppressed = 0;
    *outReason = GENERATOR_SUPPRESSION_NONE;
    return 1;
}

int F0862_RUNTIME_ComputeSpawnedGroupHealth_Compat(
    int creatureType,
    int healthMultiplier,
    int creatureCount,
    struct RngState_Compat* rng,
    int outHealth[4],
    int* outRngCallCount)
{
    int base;
    int i;
    int calls = 0;
    int total = creatureCount + 1; /* 0-based → 1..4 */

    if (outHealth == 0 || outRngCallCount == 0) return 0;
    if (total < 1) total = 1;
    if (total > 4) total = 4;

    base = runtime_get_creature_base_health(creatureType);
    if (base <= 0) base = 1;
    if (healthMultiplier <= 0) healthMultiplier = 1;

    for (i = 0; i < 4; i++) outHealth[i] = 0;

    /* Fontanel GROUP.C:539-546: health = base*mult + random(base>>2 + 1). */
    for (i = 0; i < total; i++) {
        int jitter = 0;
        int jitterRange = (base >> 2) + 1;
        if (rng != 0 && jitterRange > 1) {
            jitter = F0732_COMBAT_RngRandom_Compat(rng, jitterRange);
            calls++;
        }
        outHealth[i] = base * healthMultiplier + jitter;
    }

    *outRngCallCount = calls;
    return 1;
}

int F0863_RUNTIME_BuildGeneratorReEnableEvent_Compat(
    const struct GeneratorContext_Compat* ctx,
    uint32_t nowTick,
    struct TimelineEvent_Compat* outEvent)
{
    int ticks;
    int delay;

    if (ctx == 0 || outEvent == 0) return 0;

    ticks = ctx->ticksRaw;
    /* Fontanel TIMELINE.C:990-991: extended tick decoding. */
    if (ticks > 127) {
        delay = (ticks - 126) << 6;
    } else {
        delay = ticks;
    }

    memset(outEvent, 0, sizeof(*outEvent));
    outEvent->kind       = TIMELINE_EVENT_GROUP_GENERATOR;
    outEvent->fireAtTick = nowTick + (uint32_t)delay;
    outEvent->mapIndex   = ctx->mapIndex;
    outEvent->mapX       = ctx->mapX;
    outEvent->mapY       = ctx->mapY;
    outEvent->cell       = 0;
    outEvent->aux0       = GENERATOR_EVENT_AUX0_REENABLE;
    outEvent->aux1       = ctx->sensorIndex;
    return 1;
}

int F0860_RUNTIME_HandleGroupGenerator_Compat(
    const struct GeneratorContext_Compat* ctx,
    struct RngState_Compat* rng,
    uint32_t nowTick,
    struct GeneratorResult_Compat* outResult)
{
    int suppressed = 0;
    int reason = GENERATOR_SUPPRESSION_NONE;
    int rawCount;
    int creatureCount;
    int healthMult;
    int direction = 0;
    int rngHealthCalls = 0;

    if (ctx == 0 || outResult == 0) return 0;

    memset(outResult, 0, sizeof(*outResult));

    if (!F0861_RUNTIME_CheckGeneratorSuppression_Compat(
            ctx, &suppressed, &reason)) {
        return 0;
    }
    if (suppressed) {
        outResult->spawned = 0;
        outResult->suppressionReason = reason;
        /* No re-enable event: the corridor trigger re-fires next tick
         * until suppression clears. Sensor stays armed. */
        outResult->sensorDisabled = 0;
        outResult->reEnableScheduled = 0;
        return 1;
    }

    /* §4.1.2 — creature count resolution (MASK0x0007 + randomise bit). */
    rawCount = ctx->creatureCountRaw & 0x07;
    if (ctx->randomizeCount) {
        /* random(rawCount): 0..rawCount-1. Empty domain falls back
         * to Fontanel's `L0612_ui_CreatureCount--` path (rawCount-1). */
        if (rng != 0 && rawCount > 0) {
            creatureCount = F0732_COMBAT_RngRandom_Compat(rng, rawCount);
            outResult->rngCallCount++;
        } else {
            creatureCount = rawCount > 0 ? rawCount - 1 : 0;
        }
    } else {
        creatureCount = rawCount > 0 ? rawCount - 1 : 0;
    }
    if (creatureCount < 0) creatureCount = 0;
    if (creatureCount > 3) creatureCount = 3; /* groups cap at 4 members */

    /* §4.1.3 — health multiplier (0 → map difficulty fallback). */
    healthMult = ctx->healthMultiplier;
    if (healthMult == 0) {
        healthMult = ctx->mapDifficulty > 0 ? ctx->mapDifficulty : 1;
    }

    /* §4.1.4 — direction random 0..3. */
    if (rng != 0) {
        direction = F0732_COMBAT_RngRandom_Compat(rng, 4);
        outResult->rngCallCount++;
    }

    /* Health per creature. */
    if (!F0862_RUNTIME_ComputeSpawnedGroupHealth_Compat(
            ctx->creatureType, healthMult, creatureCount,
            rng, outResult->spawnedGroupHealth, &rngHealthCalls)) {
        return 0;
    }
    outResult->rngCallCount += rngHealthCalls;

    outResult->spawned = 1;
    outResult->spawnedCreatureType = ctx->creatureType;
    outResult->spawnedCreatureCount = creatureCount;
    outResult->spawnedDirection = direction & 0x03;
    outResult->spawnedHealthMultiplier = healthMult;
    outResult->soundRequested = ctx->audible ? 1 : 0;
    outResult->suppressionReason = GENERATOR_SUPPRESSION_NONE;

    /* §4.1.5 — sensor state + re-enable scheduling. */
    if (ctx->onceOnly) {
        outResult->sensorDisabled = 1;
        outResult->reEnableScheduled = 0;
    } else {
        if (ctx->ticksRaw > 0) {
            outResult->sensorDisabled = 1;
            outResult->reEnableScheduled = 1;
            if (!F0863_RUNTIME_BuildGeneratorReEnableEvent_Compat(
                    ctx, nowTick, &outResult->reEnableEvent)) {
                return 0;
            }
            outResult->reEnableAtTick = outResult->reEnableEvent.fireAtTick;
        } else {
            /* ticks == 0: sensor stays armed, next corridor event
             * re-triggers it. No disable. */
            outResult->sensorDisabled = 0;
            outResult->reEnableScheduled = 0;
        }
    }

    return 1;
}

/* ==========================================================
 *  Group B — Light decay (F0864 – F0867).
 * ========================================================== */

int F0865_RUNTIME_ComputeLightDecayDelta_Compat(
    int lightPower,
    int* outDelta)
{
    int absPower;
    int weakerAbs;
    int delta;

    if (outDelta == 0) return 0;

    if (lightPower == 0) {
        *outDelta = 0;
        return 1;
    }

    absPower = lightPower < 0 ? -lightPower : lightPower;
    if (absPower > RUNTIME_LIGHT_POWER_MAX) absPower = RUNTIME_LIGHT_POWER_MAX;
    weakerAbs = absPower - 1;
    if (weakerAbs < 0) weakerAbs = 0;

    delta = s_PowerOrdinalToLightAmount[absPower]
          - s_PowerOrdinalToLightAmount[weakerAbs];
    if (lightPower < 0) delta = -delta;

    *outDelta = delta;
    return 1;
}

int F0866_RUNTIME_BuildLightDecayFollowupEvent_Compat(
    int weakerPower,
    uint32_t nowTick,
    int partyMapIndex,
    struct TimelineEvent_Compat* outEvent)
{
    if (outEvent == 0) return 0;

    memset(outEvent, 0, sizeof(*outEvent));
    outEvent->kind       = TIMELINE_EVENT_MAGIC_LIGHT_DECAY;
    /* Fontanel TIMELINE.C:1764 — G0313_ul_GameTime + 4. */
    outEvent->fireAtTick = nowTick + 4;
    outEvent->mapIndex   = partyMapIndex;
    outEvent->mapX       = 0;
    outEvent->mapY       = 0;
    outEvent->cell       = 0;
    outEvent->aux0       = weakerPower;
    return 1;
}

int F0867_RUNTIME_ComputeTotalLightAmount_Compat(
    int currentAmount,
    int delta,
    int* outNewAmount)
{
    int v;
    if (outNewAmount == 0) return 0;
    v = currentAmount + delta;
    /* Light amount clamps at 0 downward (darkness is tracked
     * separately via event70LightDirection in Phase 14). */
    if (v < 0) v = 0;
    *outNewAmount = v;
    return 1;
}

int F0864_RUNTIME_HandleLightDecay_Compat(
    int lightPower,
    uint32_t nowTick,
    int partyMapIndex,
    struct LightDecayResult_Compat* outResult)
{
    int isNegative;
    int absPower;
    int weakerAbs;
    int weakerPower;
    int delta = 0;

    if (outResult == 0) return 0;

    memset(outResult, 0, sizeof(*outResult));

    if (lightPower == 0) {
        outResult->magicalLightAmountDelta = 0;
        outResult->expired = 1;
        outResult->followupScheduled = 0;
        return 1;
    }

    isNegative = (lightPower < 0);
    absPower = isNegative ? -lightPower : lightPower;
    if (absPower > RUNTIME_LIGHT_POWER_MAX) absPower = RUNTIME_LIGHT_POWER_MAX;
    weakerAbs = absPower - 1;
    if (weakerAbs < 0) weakerAbs = 0;

    /* §4.2.1 — delta via F0865. */
    if (!F0865_RUNTIME_ComputeLightDecayDelta_Compat(lightPower, &delta)) {
        return 0;
    }
    outResult->magicalLightAmountDelta = delta;

    weakerPower = isNegative ? -weakerAbs : weakerAbs;

    /* §4.2.2 — follow-up scheduling. */
    if (weakerPower != 0) {
        outResult->expired = 0;
        outResult->followupScheduled = 1;
        if (!F0866_RUNTIME_BuildLightDecayFollowupEvent_Compat(
                weakerPower, nowTick, partyMapIndex,
                &outResult->followupEvent)) {
            return 0;
        }
    } else {
        outResult->expired = 1;
        outResult->followupScheduled = 0;
    }

    return 1;
}

/* ==========================================================
 *  Group C — Fluxcage removal (F0868 – F0871).
 *
 *  Phase 17 convention (observed in memory_projectile_pc34_compat.c):
 *    - ExplosionInstance_Compat.reserved0 == 1  → live
 *    - ExplosionInstance_Compat.reserved0 == 0  → free slot
 *    - F0824_EXPLOSION_Despawn_Compat zeroes the slot and sets
 *      slotIndex = -1; it also refuses to act on already-free slots.
 * ========================================================== */

int F0869_RUNTIME_IsFluxcageSlotLive_Compat(
    const struct ExplosionList_Compat* explosions,
    int slotIndex,
    int* outIsLive)
{
    if (explosions == 0 || outIsLive == 0) return 0;
    if (slotIndex < 0 || slotIndex >= EXPLOSION_LIST_CAPACITY) return 0;

    if (explosions->entries[slotIndex].reserved0 != 0
        && explosions->entries[slotIndex].explosionType == C050_EXPLOSION_FLUXCAGE) {
        *outIsLive = 1;
    } else {
        *outIsLive = 0;
    }
    return 1;
}

int F0871_RUNTIME_CountFluxcagesOnSquare_Compat(
    const struct ExplosionList_Compat* explosions,
    int mapIndex,
    int mapX,
    int mapY,
    int* outCount)
{
    int i;
    int count = 0;

    if (explosions == 0 || outCount == 0) return 0;

    for (i = 0; i < EXPLOSION_LIST_CAPACITY; i++) {
        const struct ExplosionInstance_Compat* e = &explosions->entries[i];
        if (e->reserved0 == 0) continue;
        if (e->explosionType != C050_EXPLOSION_FLUXCAGE) continue;
        if (e->mapIndex != mapIndex) continue;
        if (e->mapX != mapX) continue;
        if (e->mapY != mapY) continue;
        count++;
    }
    *outCount = count;
    return 1;
}

int F0868_RUNTIME_HandleRemoveFluxcage_Compat(
    const struct FluxcageRemoveInput_Compat* in,
    struct ExplosionList_Compat* explosions,
    struct FluxcageRemoveResult_Compat* outResult)
{
    int isLive = 0;

    if (in == 0 || explosions == 0 || outResult == 0) return 0;
    if (in->explosionSlotIndex < 0
        || in->explosionSlotIndex >= EXPLOSION_LIST_CAPACITY) {
        return 0;
    }

    memset(outResult, 0, sizeof(*outResult));
    outResult->mapIndex = in->mapIndex;
    outResult->mapX     = in->mapX;
    outResult->mapY     = in->mapY;

    if (!F0869_RUNTIME_IsFluxcageSlotLive_Compat(
            explosions, in->explosionSlotIndex, &isLive)) {
        return 0;
    }

    if (!isLive) {
        /* Already consumed (R5 idempotency): timer-expiry-then-absorption
         * and absorption-then-timer both route through this branch on the
         * second call. */
        outResult->removed = 0;
        outResult->squareContentChanged = 0;
        return 1;
    }

    if (!F0824_EXPLOSION_Despawn_Compat(
            explosions, in->explosionSlotIndex)) {
        /* F0824 refused (slot was already clean) — treat as already gone. */
        outResult->removed = 0;
        outResult->squareContentChanged = 0;
        return 1;
    }

    outResult->removed = 1;
    outResult->squareContentChanged = 1;
    return 1;
}

int F0870_RUNTIME_FluxcageRemoveByAbsorption_Compat(
    struct ExplosionList_Compat* explosions,
    int slotIndex,
    struct FluxcageRemoveResult_Compat* outResult)
{
    struct FluxcageRemoveInput_Compat in;
    int preservedMap;
    int preservedX;
    int preservedY;

    if (explosions == 0 || outResult == 0) return 0;
    if (slotIndex < 0 || slotIndex >= EXPLOSION_LIST_CAPACITY) return 0;

    /* Capture position BEFORE despawn since F0824 zeroes the slot. */
    preservedMap = explosions->entries[slotIndex].mapIndex;
    preservedX   = explosions->entries[slotIndex].mapX;
    preservedY   = explosions->entries[slotIndex].mapY;

    memset(&in, 0, sizeof(in));
    in.explosionSlotIndex = slotIndex;
    in.mapIndex = preservedMap;
    in.mapX     = preservedX;
    in.mapY     = preservedY;

    return F0868_RUNTIME_HandleRemoveFluxcage_Compat(&in, explosions, outResult);
}

/* ==========================================================
 *  Group D — Generator re-enable (F0872 – F0874).
 * ========================================================== */

int F0873_RUNTIME_FindDisabledSensorOnSquare_Compat(
    const struct DungeonSensor_Compat* sensors,
    int sensorCount,
    int mapIndex,
    int mapX,
    int mapY,
    int* outSensorIndex)
{
    int i;

    /* mapIndex/mapX/mapY are not stored inside DungeonSensor_Compat
     * (the sensor is placed via square first-thing linkage in
     * DUNGEON.DAT). For Phase 19's pure path the caller pre-filters
     * the sensor array to those located on the target square; this
     * helper then linear-scans for the first disabled sensor. The
     * mapIndex/mapX/mapY arguments are kept for API symmetry and
     * for a future cached table that stores (sensorIndex, position).
     */
    (void)mapIndex; (void)mapX; (void)mapY;

    if (sensors == 0 || outSensorIndex == 0) return 0;

    *outSensorIndex = -1;
    for (i = 0; i < sensorCount; i++) {
        if (sensors[i].sensorType == RUNTIME_SENSOR_TYPE_DISABLED) {
            *outSensorIndex = i;
            return 1;
        }
    }
    return 1;
}

int F0874_RUNTIME_ReEnableSensor_Compat(
    struct DungeonSensor_Compat* sensor,
    int newSensorType)
{
    if (sensor == 0) return 0;
    if (newSensorType < 0 || newSensorType > 127) return 0;
    sensor->sensorType = (unsigned char)newSensorType;
    return 1;
}

int F0872_RUNTIME_HandleGeneratorReEnable_Compat(
    const struct GeneratorReEnableInput_Compat* in,
    struct DungeonSensor_Compat* sensors,
    int sensorCount,
    struct GeneratorReEnableResult_Compat* outResult)
{
    int foundIndex = -1;

    if (in == 0 || sensors == 0 || outResult == 0) return 0;

    outResult->reEnabled = 0;
    outResult->sensorIndex = -1;

    if (!F0873_RUNTIME_FindDisabledSensorOnSquare_Compat(
            sensors, sensorCount, in->mapIndex, in->mapX, in->mapY,
            &foundIndex)) {
        return 0;
    }
    if (foundIndex < 0) {
        outResult->reEnabled = 0;
        outResult->sensorIndex = -1;
        return 1;
    }

    if (!F0874_RUNTIME_ReEnableSensor_Compat(
            &sensors[foundIndex],
            RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR)) {
        return 0;
    }

    outResult->reEnabled = 1;
    outResult->sensorIndex = foundIndex;
    return 1;
}

/* ==========================================================
 *  Group E — Serialisation (F0875 – F0879).
 *  MEDIA016 / LSB-first. Returns 1 on success per prior-phase
 *  convention (not byte count).
 * ========================================================== */

int F0875_RUNTIME_GeneratorContextSerialize_Compat(
    const struct GeneratorContext_Compat* ctx,
    unsigned char* outBuf,
    int outBufSize)
{
    int o = 0;
    if (ctx == 0 || outBuf == 0) return 0;
    if (outBufSize < GENERATOR_CONTEXT_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf + o, ctx->sensorIndex);             o += 4;
    write_i32_le(outBuf + o, ctx->mapIndex);                o += 4;
    write_i32_le(outBuf + o, ctx->mapX);                    o += 4;
    write_i32_le(outBuf + o, ctx->mapY);                    o += 4;
    write_i32_le(outBuf + o, ctx->creatureType);            o += 4;
    write_i32_le(outBuf + o, ctx->creatureCountRaw);        o += 4;
    write_i32_le(outBuf + o, ctx->randomizeCount);          o += 4;
    write_i32_le(outBuf + o, ctx->healthMultiplier);        o += 4;
    write_i32_le(outBuf + o, ctx->ticksRaw);                o += 4;
    write_i32_le(outBuf + o, ctx->onceOnly);                o += 4;
    write_i32_le(outBuf + o, ctx->audible);                 o += 4;
    write_i32_le(outBuf + o, ctx->mapDifficulty);           o += 4;
    write_i32_le(outBuf + o, ctx->isOnPartyMap);            o += 4;
    write_i32_le(outBuf + o, ctx->currentActiveGroupCount); o += 4;
    write_i32_le(outBuf + o, ctx->maxActiveGroupCount);     o += 4;
    write_i32_le(outBuf + o, ctx->reserved0);               o += 4;
    return 1;
}

int F0875_RUNTIME_GeneratorContextDeserialize_Compat(
    struct GeneratorContext_Compat* ctx,
    const unsigned char* buf,
    int bufSize)
{
    int o = 0;
    if (ctx == 0 || buf == 0) return 0;
    if (bufSize < GENERATOR_CONTEXT_SERIALIZED_SIZE) return 0;

    ctx->sensorIndex             = read_i32_le(buf + o); o += 4;
    ctx->mapIndex                = read_i32_le(buf + o); o += 4;
    ctx->mapX                    = read_i32_le(buf + o); o += 4;
    ctx->mapY                    = read_i32_le(buf + o); o += 4;
    ctx->creatureType            = read_i32_le(buf + o); o += 4;
    ctx->creatureCountRaw        = read_i32_le(buf + o); o += 4;
    ctx->randomizeCount          = read_i32_le(buf + o); o += 4;
    ctx->healthMultiplier        = read_i32_le(buf + o); o += 4;
    ctx->ticksRaw                = read_i32_le(buf + o); o += 4;
    ctx->onceOnly                = read_i32_le(buf + o); o += 4;
    ctx->audible                 = read_i32_le(buf + o); o += 4;
    ctx->mapDifficulty           = read_i32_le(buf + o); o += 4;
    ctx->isOnPartyMap            = read_i32_le(buf + o); o += 4;
    ctx->currentActiveGroupCount = read_i32_le(buf + o); o += 4;
    ctx->maxActiveGroupCount     = read_i32_le(buf + o); o += 4;
    ctx->reserved0               = read_i32_le(buf + o); o += 4;
    return 1;
}

int F0876_RUNTIME_GeneratorResultSerialize_Compat(
    const struct GeneratorResult_Compat* result,
    unsigned char* outBuf,
    int outBufSize)
{
    int o = 0;
    int i;
    if (result == 0 || outBuf == 0) return 0;
    if (outBufSize < GENERATOR_RESULT_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf + o, result->spawned);                 o += 4;
    write_i32_le(outBuf + o, result->spawnedCreatureType);     o += 4;
    write_i32_le(outBuf + o, result->spawnedCreatureCount);    o += 4;
    write_i32_le(outBuf + o, result->spawnedDirection);        o += 4;
    write_i32_le(outBuf + o, result->spawnedHealthMultiplier); o += 4;
    write_i32_le(outBuf + o, result->sensorDisabled);          o += 4;
    write_i32_le(outBuf + o, result->reEnableScheduled);       o += 4;
    write_i32_le(outBuf + o, (int)result->reEnableAtTick);     o += 4;
    write_i32_le(outBuf + o, result->soundRequested);          o += 4;
    write_i32_le(outBuf + o, result->suppressionReason);       o += 4;
    write_i32_le(outBuf + o, result->rngCallCount);            o += 4;
    write_i32_le(outBuf + o, result->reserved0);               o += 4;
    for (i = 0; i < 4; i++) {
        write_i32_le(outBuf + o, result->spawnedGroupHealth[i]);
        o += 4;
    }
    write_i32_le(outBuf + o, result->reserved1);               o += 4;
    /* reEnableEvent via F0725 (44 B). */
    if (!F0725_TIMELINE_EventSerialize_Compat(
            &result->reEnableEvent, outBuf + o,
            TIMELINE_EVENT_SERIALIZED_SIZE)) {
        return 0;
    }
    o += TIMELINE_EVENT_SERIALIZED_SIZE;
    (void)o;
    return 1;
}

int F0876_RUNTIME_GeneratorResultDeserialize_Compat(
    struct GeneratorResult_Compat* result,
    const unsigned char* buf,
    int bufSize)
{
    int o = 0;
    int i;
    if (result == 0 || buf == 0) return 0;
    if (bufSize < GENERATOR_RESULT_SERIALIZED_SIZE) return 0;

    result->spawned                 = read_i32_le(buf + o); o += 4;
    result->spawnedCreatureType     = read_i32_le(buf + o); o += 4;
    result->spawnedCreatureCount    = read_i32_le(buf + o); o += 4;
    result->spawnedDirection        = read_i32_le(buf + o); o += 4;
    result->spawnedHealthMultiplier = read_i32_le(buf + o); o += 4;
    result->sensorDisabled          = read_i32_le(buf + o); o += 4;
    result->reEnableScheduled       = read_i32_le(buf + o); o += 4;
    result->reEnableAtTick          = (uint32_t)read_i32_le(buf + o); o += 4;
    result->soundRequested          = read_i32_le(buf + o); o += 4;
    result->suppressionReason       = read_i32_le(buf + o); o += 4;
    result->rngCallCount            = read_i32_le(buf + o); o += 4;
    result->reserved0               = read_i32_le(buf + o); o += 4;
    for (i = 0; i < 4; i++) {
        result->spawnedGroupHealth[i] = read_i32_le(buf + o);
        o += 4;
    }
    result->reserved1               = read_i32_le(buf + o); o += 4;
    if (!F0726_TIMELINE_EventDeserialize_Compat(
            &result->reEnableEvent, buf + o,
            TIMELINE_EVENT_SERIALIZED_SIZE)) {
        return 0;
    }
    o += TIMELINE_EVENT_SERIALIZED_SIZE;
    (void)o;
    return 1;
}

int F0877_RUNTIME_LightDecayResultSerialize_Compat(
    const struct LightDecayResult_Compat* result,
    unsigned char* outBuf,
    int outBufSize)
{
    int o = 0;
    if (result == 0 || outBuf == 0) return 0;
    if (outBufSize < LIGHT_DECAY_RESULT_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf + o, result->magicalLightAmountDelta); o += 4;
    write_i32_le(outBuf + o, result->expired);                 o += 4;
    write_i32_le(outBuf + o, result->followupScheduled);       o += 4;
    if (!F0725_TIMELINE_EventSerialize_Compat(
            &result->followupEvent, outBuf + o,
            TIMELINE_EVENT_SERIALIZED_SIZE)) {
        return 0;
    }
    o += TIMELINE_EVENT_SERIALIZED_SIZE;
    (void)o;
    return 1;
}

int F0877_RUNTIME_LightDecayResultDeserialize_Compat(
    struct LightDecayResult_Compat* result,
    const unsigned char* buf,
    int bufSize)
{
    int o = 0;
    if (result == 0 || buf == 0) return 0;
    if (bufSize < LIGHT_DECAY_RESULT_SERIALIZED_SIZE) return 0;

    result->magicalLightAmountDelta = read_i32_le(buf + o); o += 4;
    result->expired                 = read_i32_le(buf + o); o += 4;
    result->followupScheduled       = read_i32_le(buf + o); o += 4;
    if (!F0726_TIMELINE_EventDeserialize_Compat(
            &result->followupEvent, buf + o,
            TIMELINE_EVENT_SERIALIZED_SIZE)) {
        return 0;
    }
    o += TIMELINE_EVENT_SERIALIZED_SIZE;
    (void)o;
    return 1;
}

int F0878_RUNTIME_FluxcageRemoveResultSerialize_Compat(
    const struct FluxcageRemoveResult_Compat* result,
    unsigned char* outBuf,
    int outBufSize)
{
    int o = 0;
    if (result == 0 || outBuf == 0) return 0;
    if (outBufSize < FLUXCAGE_REMOVE_RESULT_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf + o, result->removed);              o += 4;
    write_i32_le(outBuf + o, result->mapIndex);             o += 4;
    write_i32_le(outBuf + o, result->mapX);                 o += 4;
    write_i32_le(outBuf + o, result->mapY);                 o += 4;
    write_i32_le(outBuf + o, result->squareContentChanged); o += 4;
    return 1;
}

int F0878_RUNTIME_FluxcageRemoveResultDeserialize_Compat(
    struct FluxcageRemoveResult_Compat* result,
    const unsigned char* buf,
    int bufSize)
{
    int o = 0;
    if (result == 0 || buf == 0) return 0;
    if (bufSize < FLUXCAGE_REMOVE_RESULT_SERIALIZED_SIZE) return 0;

    result->removed              = read_i32_le(buf + o); o += 4;
    result->mapIndex             = read_i32_le(buf + o); o += 4;
    result->mapX                 = read_i32_le(buf + o); o += 4;
    result->mapY                 = read_i32_le(buf + o); o += 4;
    result->squareContentChanged = read_i32_le(buf + o); o += 4;
    return 1;
}

int F0879_RUNTIME_GeneratorReEnableResultSerialize_Compat(
    const struct GeneratorReEnableResult_Compat* result,
    unsigned char* outBuf,
    int outBufSize)
{
    int o = 0;
    if (result == 0 || outBuf == 0) return 0;
    if (outBufSize < GENERATOR_RE_ENABLE_RESULT_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf + o, result->reEnabled);   o += 4;
    write_i32_le(outBuf + o, result->sensorIndex); o += 4;
    return 1;
}

int F0879_RUNTIME_GeneratorReEnableResultDeserialize_Compat(
    struct GeneratorReEnableResult_Compat* result,
    const unsigned char* buf,
    int bufSize)
{
    int o = 0;
    if (result == 0 || buf == 0) return 0;
    if (bufSize < GENERATOR_RE_ENABLE_RESULT_SERIALIZED_SIZE) return 0;

    result->reEnabled   = read_i32_le(buf + o); o += 4;
    result->sensorIndex = read_i32_le(buf + o); o += 4;
    return 1;
}
