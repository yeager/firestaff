#ifndef REDMCSB_MEMORY_RUNTIME_DYNAMICS_PC34_COMPAT_H
#define REDMCSB_MEMORY_RUNTIME_DYNAMICS_PC34_COMPAT_H

/*
 * Runtime dynamics data layer for ReDMCSB PC 3.4 — Phase 19 of M10.
 *
 * Pure, caller-driven handlers for the four "liveness" event kinds
 * scheduled elsewhere in the timeline: GROUP_GENERATOR (spawn creature
 * groups), MAGIC_LIGHT_DECAY (age magical light amount), REMOVE_FLUXCAGE
 * (despawn a stored fluxcage explosion), and the companion
 * group-generator re-enable path (C65 equivalent).
 *
 * Authoritative plan: PHASE19_PLAN.md. All algorithms mirror Fontanel
 * primaries — see the §4 cites in that document for exact source
 * line references.
 *
 * Conventions (inherited from Phases 10 – 18):
 *   - All symbols suffixed _pc34_compat / _Compat.
 *   - MEDIA016 / PC LSB-first int32 serialisation. Every struct
 *     round-trips bit-identical.
 *   - NO globals, NO UI, NO IO. Randomness flows through Phase 13's
 *     RngState_Compat (F0732).
 *   - ADDITIVE ONLY: zero edits to Phase 9 / 10 / 11 / 12 / 13 / 14 /
 *     15 / 16 / 17 / 18 source. We consume their types via #include
 *     and pure composition.
 *
 * Function numbering: F0860 – F0879 (Phase 19 slot).
 *
 * ----------------------------------------------------------------
 * Plan-reader note (R1 mitigation):
 *   Phase 14 (memory_magic_pc34_compat.h) does NOT define an
 *   ActiveLightInstance_Compat or FluxcageInstance_Compat struct —
 *   Fontanel tracks both via timeline event chains (light) and the
 *   Phase-17 ExplosionList_Compat slot (fluxcage). Phase 19 therefore
 *   does NOT redeclare either kind of instance; it consumes
 *   ExplosionList_Compat / ExplosionInstance_Compat from Phase 17
 *   and the shared MagicState_Compat.magicalLightAmount scalar from
 *   Phase 14 unchanged.
 * ----------------------------------------------------------------
 */

#include <stdint.h>

#include "memory_combat_pc34_compat.h"         /* RngState_Compat  */
#include "memory_timeline_pc34_compat.h"       /* TimelineEvent_Compat */
#include "memory_projectile_pc34_compat.h"     /* ExplosionList_Compat, F0824 */
#include "memory_dungeon_dat_pc34_compat.h"    /* DungeonSensor_Compat */
#include "memory_magic_pc34_compat.h"          /* MagicState_Compat */

/* ==========================================================
 *  Serialised sizes (MEDIA016 / LSB-first, 4-byte int32 fields).
 * ========================================================== */

#define GENERATOR_CONTEXT_SERIALIZED_SIZE           64  /* 16 int32 */
#define GENERATOR_RESULT_SERIALIZED_SIZE           112  /* 17 int32 + 44 B event */
#define LIGHT_DECAY_INPUT_SERIALIZED_SIZE           12  /* 3 int32 */
#define LIGHT_DECAY_RESULT_SERIALIZED_SIZE          56  /* 3 int32 + 44 B event */
#define FLUXCAGE_REMOVE_INPUT_SERIALIZED_SIZE       16  /* 4 int32 */
#define FLUXCAGE_REMOVE_RESULT_SERIALIZED_SIZE      20  /* 5 int32 */
#define GENERATOR_RE_ENABLE_INPUT_SERIALIZED_SIZE   12  /* 3 int32 */
#define GENERATOR_RE_ENABLE_RESULT_SERIALIZED_SIZE   8  /* 2 int32 */

/* ==========================================================
 *  Suppression reason codes (stable contract).
 * ========================================================== */

#define GENERATOR_SUPPRESSION_NONE               0
#define GENERATOR_SUPPRESSION_ACTIVE_GROUP_CAP   1  /* GROUP.C:512 */
#define GENERATOR_SUPPRESSION_NO_SLOT            2  /* caller-detected list overflow */
#define GENERATOR_SUPPRESSION_INVALID_TYPE       3  /* creatureType out of range */

/* ==========================================================
 *  Sensor type values referenced by the generator re-enable
 *  path. Mirror of DEFS.H C000..C006.
 * ========================================================== */

#define RUNTIME_SENSOR_TYPE_DISABLED              0  /* C000 */
#define RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR 6  /* C006 */

/* ==========================================================
 *  Generator-event aux0 sentinels. Used by F0863 to distinguish
 *  the "re-enable" flavour of the GROUP_GENERATOR timeline event
 *  from the "initial trigger" flavour so the dispatcher can route
 *  correctly. Bit 0 = is-re-enable.
 * ========================================================== */

#define GENERATOR_EVENT_AUX0_TRIGGER    0
#define GENERATOR_EVENT_AUX0_REENABLE   1

/* ==========================================================
 *  PowerOrdinalToLightAmount sentinel bounds.
 *
 *  Fontanel stores 16 entries in GRAPHICS.DAT entry 562
 *  (G0039_ai_Graphic562_LightPowerToLightAmount). v1 mirrors
 *  Phase 14's placeholder for indices 1..6 and adds index 0 = 0.
 *  The full 16-entry table is deferred to post-M10 when the
 *  GRAPHICS.DAT loader lands. NEEDS DISASSEMBLY REVIEW.
 * ========================================================== */

#define RUNTIME_LIGHT_POWER_MAX   6

/* ==========================================================
 *  Data structures.
 * ========================================================== */

/*
 * Caller-built snapshot of a generator sensor + dungeon context.
 *
 * The caller does all the up-front decoding from the raw
 * DungeonSensor_Compat (M040_DATA / M045 / M046 bit fields,
 * sensor.value masking, etc.); Phase 19 is pure state transform.
 *
 * 16 int32 → 64 bytes. All fields little-endian on-disk.
 */
struct GeneratorContext_Compat {
    int sensorIndex;               /* index into things.sensors[]          */
    int mapIndex;
    int mapX;
    int mapY;
    int creatureType;              /* M040_DATA(sensor): 0..26             */
    int creatureCountRaw;          /* sensor.value (4 bits)                */
    int randomizeCount;            /* bit 3 of creatureCountRaw            */
    int healthMultiplier;          /* M045: localMultiple & 0x000F         */
    int ticksRaw;                  /* M046: localMultiple >> 4             */
    int onceOnly;                  /* sensor.onceOnly                      */
    int audible;                   /* sensor.audible                       */
    int mapDifficulty;             /* fallback multiplier if M045 == 0     */
    int isOnPartyMap;              /* 1 if generator map == party map      */
    int currentActiveGroupCount;   /* global live group count              */
    int maxActiveGroupCount;       /* dungeon header max                   */
    int reserved0;
};

/*
 * Output of F0860. 17 int32 (68 B) + 44-byte TimelineEvent = 112 B.
 *
 * spawnedCreatureCount is 0-based like Fontanel
 * (count+1 is the *actual* number of creatures).
 */
struct GeneratorResult_Compat {
    int spawned;                   /* 1 = group was generated              */
    int spawnedCreatureType;       /* creature type of new group           */
    int spawnedCreatureCount;      /* count (0-based like Fontanel)        */
    int spawnedDirection;          /* random direction 0..3                */
    int spawnedHealthMultiplier;   /* effective multiplier used            */
    int sensorDisabled;            /* 1 if sensor was set to DISABLED      */
    int reEnableScheduled;         /* 1 if a re-enable event was emitted   */
    uint32_t reEnableAtTick;       /* fireAtTick of re-enable event        */
    int soundRequested;            /* 1 if audible flag was set            */
    int suppressionReason;         /* GENERATOR_SUPPRESSION_*              */
    int rngCallCount;              /* how many RNG calls consumed          */
    int reserved0;
    int spawnedGroupHealth[4];     /* HP for each creature (up to 4)       */
    int reserved1;                 /* padding to 68 B                      */
    struct TimelineEvent_Compat reEnableEvent; /* 44 B follow-up           */
};

struct LightDecayInput_Compat {
    int lightPower;                /* from event.aux0; negative=darkness   */
    uint32_t nowTick;              /* current game time                    */
    int partyMapIndex;             /* needed for follow-up event           */
};

struct LightDecayResult_Compat {
    int magicalLightAmountDelta;   /* add to MagicState.magicalLightAmount */
    int expired;                   /* 1 if this was the last decay step    */
    int followupScheduled;         /* 1 if a new C70 event was emitted     */
    struct TimelineEvent_Compat followupEvent; /* 44 B follow-up           */
};

struct FluxcageRemoveInput_Compat {
    int explosionSlotIndex;        /* into ExplosionList_Compat            */
    int mapIndex;
    int mapX;
    int mapY;
};

struct FluxcageRemoveResult_Compat {
    int removed;                   /* 1 = actually removed; 0 = already gone */
    int mapIndex;
    int mapX;
    int mapY;
    int squareContentChanged;      /* 1 if removed, else 0                 */
};

struct GeneratorReEnableInput_Compat {
    int mapIndex;
    int mapX;
    int mapY;
};

struct GeneratorReEnableResult_Compat {
    int reEnabled;                 /* 1 = found & re-enabled               */
    int sensorIndex;               /* -1 if none                           */
};

/* ==========================================================
 *  Group A — Generator lifecycle (F0860 – F0863).
 * ========================================================== */

int F0860_RUNTIME_HandleGroupGenerator_Compat(
    const struct GeneratorContext_Compat* ctx,
    struct RngState_Compat* rng,
    uint32_t nowTick,
    struct GeneratorResult_Compat* outResult);

int F0861_RUNTIME_CheckGeneratorSuppression_Compat(
    const struct GeneratorContext_Compat* ctx,
    int* outSuppressed,
    int* outReason);

int F0862_RUNTIME_ComputeSpawnedGroupHealth_Compat(
    int creatureType,
    int healthMultiplier,
    int creatureCount,
    struct RngState_Compat* rng,
    int outHealth[4],
    int* outRngCallCount);

int F0863_RUNTIME_BuildGeneratorReEnableEvent_Compat(
    const struct GeneratorContext_Compat* ctx,
    uint32_t nowTick,
    struct TimelineEvent_Compat* outEvent);

/* ==========================================================
 *  Group B — Light decay (F0864 – F0867).
 * ========================================================== */

int F0864_RUNTIME_HandleLightDecay_Compat(
    int lightPower,
    uint32_t nowTick,
    int partyMapIndex,
    struct LightDecayResult_Compat* outResult);

int F0865_RUNTIME_ComputeLightDecayDelta_Compat(
    int lightPower,
    int* outDelta);

int F0866_RUNTIME_BuildLightDecayFollowupEvent_Compat(
    int weakerPower,
    uint32_t nowTick,
    int partyMapIndex,
    struct TimelineEvent_Compat* outEvent);

int F0867_RUNTIME_ComputeTotalLightAmount_Compat(
    int currentAmount,
    int delta,
    int* outNewAmount);

/* ==========================================================
 *  Group C — Fluxcage removal (F0868 – F0871).
 * ========================================================== */

int F0868_RUNTIME_HandleRemoveFluxcage_Compat(
    const struct FluxcageRemoveInput_Compat* in,
    struct ExplosionList_Compat* explosions,
    struct FluxcageRemoveResult_Compat* outResult);

int F0869_RUNTIME_IsFluxcageSlotLive_Compat(
    const struct ExplosionList_Compat* explosions,
    int slotIndex,
    int* outIsLive);

int F0870_RUNTIME_FluxcageRemoveByAbsorption_Compat(
    struct ExplosionList_Compat* explosions,
    int slotIndex,
    struct FluxcageRemoveResult_Compat* outResult);

int F0871_RUNTIME_CountFluxcagesOnSquare_Compat(
    const struct ExplosionList_Compat* explosions,
    int mapIndex,
    int mapX,
    int mapY,
    int* outCount);

/* ==========================================================
 *  Group D — Generator re-enable (F0872 – F0874).
 * ========================================================== */

int F0872_RUNTIME_HandleGeneratorReEnable_Compat(
    const struct GeneratorReEnableInput_Compat* in,
    struct DungeonSensor_Compat* sensors,
    int sensorCount,
    struct GeneratorReEnableResult_Compat* outResult);

int F0873_RUNTIME_FindDisabledSensorOnSquare_Compat(
    const struct DungeonSensor_Compat* sensors,
    int sensorCount,
    int mapIndex,
    int mapX,
    int mapY,
    int* outSensorIndex);

int F0874_RUNTIME_ReEnableSensor_Compat(
    struct DungeonSensor_Compat* sensor,
    int newSensorType);

/* ==========================================================
 *  Group E — Serialisation (F0875 – F0879).
 *  MEDIA016 / LSB-first, bit-identical round-trip.
 * ========================================================== */

int F0875_RUNTIME_GeneratorContextSerialize_Compat(
    const struct GeneratorContext_Compat* ctx,
    unsigned char* outBuf,
    int outBufSize);

int F0875_RUNTIME_GeneratorContextDeserialize_Compat(
    struct GeneratorContext_Compat* ctx,
    const unsigned char* buf,
    int bufSize);

int F0876_RUNTIME_GeneratorResultSerialize_Compat(
    const struct GeneratorResult_Compat* result,
    unsigned char* outBuf,
    int outBufSize);

int F0876_RUNTIME_GeneratorResultDeserialize_Compat(
    struct GeneratorResult_Compat* result,
    const unsigned char* buf,
    int bufSize);

int F0877_RUNTIME_LightDecayResultSerialize_Compat(
    const struct LightDecayResult_Compat* result,
    unsigned char* outBuf,
    int outBufSize);

int F0877_RUNTIME_LightDecayResultDeserialize_Compat(
    struct LightDecayResult_Compat* result,
    const unsigned char* buf,
    int bufSize);

int F0878_RUNTIME_FluxcageRemoveResultSerialize_Compat(
    const struct FluxcageRemoveResult_Compat* result,
    unsigned char* outBuf,
    int outBufSize);

int F0878_RUNTIME_FluxcageRemoveResultDeserialize_Compat(
    struct FluxcageRemoveResult_Compat* result,
    const unsigned char* buf,
    int bufSize);

int F0879_RUNTIME_GeneratorReEnableResultSerialize_Compat(
    const struct GeneratorReEnableResult_Compat* result,
    unsigned char* outBuf,
    int outBufSize);

int F0879_RUNTIME_GeneratorReEnableResultDeserialize_Compat(
    struct GeneratorReEnableResult_Compat* result,
    const unsigned char* buf,
    int bufSize);

#endif /* REDMCSB_MEMORY_RUNTIME_DYNAMICS_PC34_COMPAT_H */
