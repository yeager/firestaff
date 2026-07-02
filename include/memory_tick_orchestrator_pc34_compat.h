#ifndef REDMCSB_MEMORY_TICK_ORCHESTRATOR_PC34_COMPAT_H
#define REDMCSB_MEMORY_TICK_ORCHESTRATOR_PC34_COMPAT_H

/*
 * Tick orchestrator & deterministic harness for ReDMCSB PC 3.4 —
 * Phase 20 of M10 (the integration milestone).
 *
 * Pure-function tick orchestrator:
 *     F0884(world, input) -> (world', TickResult)
 *
 * Conventions (inherited from Phases 10-19):
 *   - All symbols suffixed _pc34_compat / _Compat.
 *   - MEDIA016 / PC LSB-first int32 serialisation. Every struct
 *     round-trips bit-identical.
 *   - NO globals, NO UI, NO IO (except the headless driver's
 *     explicit file IO and the dungeon loader).
 *   - Single master RngState_Compat lives in the world; Borland LCG.
 *   - Function numbering claims F0880..F0899.
 *   - ADDITIVE ONLY: consumes Phase 1-19 interfaces; never edits them.
 *
 * ------ Documented deviations from PHASE20_PLAN.md (§2.1) ------
 *
 * (D1) GameWorld_Compat holds POINTERS to DungeonDatState_Compat and
 *      DungeonThings_Compat (owned by the world and freed on F0883),
 *      not inline copies. The plan's §2.1 lists the dungeon static
 *      layer inline (`DungeonDatState_Compat dungeon;`) but those types
 *      already own heap pointers (maps, tiles, rawThingData, textData),
 *      so inline composition would break value semantics. We carry a
 *      uint32_t dungeonFingerprint instead for hashing — the dungeon
 *      itself is immutable in v1, so a fingerprint is equivalent to
 *      hashing the raw bytes.
 *
 * (D2) The plan's §2.1 also lists separate inline arrays for
 *      `ProjectileInstance projectiles[PROJECTILE_LIST_CAPACITY]` etc.
 *      Phase 17 already wraps these in `ProjectileList_Compat` /
 *      `ExplosionList_Compat`; we reuse the wrappers so we inherit the
 *      existing serialisers (F0829).
 *
 * (D3) Phase 9's monster groups live inside DungeonThings_Compat.groups
 *      — Phase 20 does not redeclare a MonsterGroupList_Compat; it
 *      mutates the existing slice in place (cells/health) when a group
 *      dies or moves. The dungeon fingerprint is recomputed lazily.
 *
 * (D4) Phase 18's LifecycleState_Compat is carried instead of the plan's
 *      inline food/water scalars; Phase 18 already owns the int16 range
 *      semantics. Phase 10 ChampionState_Compat lives inside PartyState_Compat
 *      (plan §2.1's `champions[CHAMPION_MAX_PARTY]` is the same data).
 *
 * (D5) Phase 19 "active lights / fluxcages / generators instance lists"
 *      don't exist as separate structs — the Phase 19 header explicitly
 *      documents tracking via timeline chains + Phase 17 explosion slots
 *      + sensor toggling. Phase 20 follows that convention (consistent
 *      with the pre-work note at the top of this task).
 *
 * These deviations preserve determinism and round-trip integrity.
 *
 * ReDMCSB GAMELOOP.C:67-78 (F0003_MAIN_ProcessNewPartyMap): the full
 * behaviour of F0003 is not reproduced in v1 (see §1 "Out of
 * scope"); we toggle partyMapIndex only.  Map-transition
 * re-dispatch is bounded to 4 iterations per tick via
 * ORCH_MAX_MAP_TRANSITIONS_PER_TICK (see below).  See
 * GAMELOOP.C:67-78 for the original F0003 dispatch loop.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_magic_pc34_compat.h"
#include "memory_savegame_pc34_compat.h"
#include "memory_creature_ai_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
#include "memory_champion_lifecycle_pc34_compat.h"
#include "memory_runtime_dynamics_pc34_compat.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_combat_pc34_compat.h"

/* ================================================================
 *  Commands (TickInput.command)
 * ================================================================ */

#define CMD_NONE              0x00
#define CMD_MOVE_NORTH        0x01
#define CMD_MOVE_EAST         0x02
#define CMD_MOVE_SOUTH        0x03
#define CMD_MOVE_WEST         0x04
#define CMD_TURN_LEFT         0x05
#define CMD_TURN_RIGHT        0x06
#define CMD_ATTACK            0x10
#define CMD_CAST_SPELL        0x11
#define CMD_USE_ITEM          0x12
#define CMD_EAT               0x13
#define CMD_DRINK             0x14
#define CMD_REST_TOGGLE       0x20
#define CMD_THROW_ITEM        0x21

/* CMD_ATTACK reserved2 encoding.
 * If bit 31 is set, low byte carries the ReDMCSB action index
 * C000_ACTION_N..C043_ACTION_FUSE.  Bit 30 is an explicit legacy-test
 * marker for the old weapon-class snapshot fallback; live melee callers
 * should pass group/creature target data instead. */
#define CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID 0x80000000u
#define CMD_ATTACK_RESERVED2_LEGACY_MARKER_VALID 0x40000000u
#define CMD_ATTACK_RESERVED2_TARGET_DIRECTION_VALID 0x20000000u
#define CMD_ATTACK_RESERVED2_TARGET_DIRECTION_SHIFT 8u
#define CMD_ATTACK_RESERVED2_TARGET_DIRECTION_MASK  0x00000300u
#define CMD_ATTACK_RESERVED2_ACTION_INDEX_MASK  0x000000FFu
#define CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34    DM1_ACTION_MELEE
#define CMD_ATTACK_TARGET_AUTO_GROUP_PC34       0xFFu
#define CMD_ATTACK_CREATURE_AUTO_PC34           0xFFu

/* CMD_CAST_SPELL reserved2 encoding. */
#define CMD_CAST_SPELL_RESERVED2_HAS_EMPTY_FLASK        0x00000001u
#define CMD_CAST_SPELL_RESERVED2_EMPTY_FLASK_SLOT_SHIFT 8u
#define CMD_CAST_SPELL_RESERVED2_EMPTY_FLASK_SLOT_MASK  0x0000FF00u

/* ================================================================
 *  Emission kinds (TickEmission.kind)
 * ================================================================ */

#define EMIT_DAMAGE_DEALT     0x01
#define EMIT_SOUND_REQUEST    0x02
#define EMIT_XP_AWARD         0x03
#define EMIT_KILL_NOTIFY      0x04
#define EMIT_DOOR_STATE       0x05
#define EMIT_PARTY_MOVED      0x06
#define EMIT_CHAMPION_DOWN    0x07
#define EMIT_GAME_WON         0x08
#define EMIT_PARTY_DEAD       0x09
#define EMIT_SPELL_EFFECT     0x0A
#define EMIT_PARTY_FELL       0x0B
#define EMIT_PARTY_TELEPORTED 0x0C
#define EMIT_SENSOR_EFFECT    0x0D  /* pass-37: party enter/leave sensor effects */

/* ================================================================
 *  Capacities
 * ================================================================ */

#define TICK_EMISSION_CAPACITY          64
#define GAMEWORLD_CREATURE_AI_CAPACITY  64
#define TICK_INPUT_SERIALIZED_SIZE      16
#define TICK_EMISSION_SERIALIZED_SIZE   20
#define TICK_STREAM_RECORD_SERIALIZED_SIZE 24
#define GAME_CONFIG_SERIALIZED_SIZE     64

/* Orchestrator return codes (F0884). */
#define ORCH_OK                  1
#define ORCH_FAIL                0
#define ORCH_PARTY_DEAD         (-1)
#define ORCH_GAME_WON           (-2)

/* Map-transition safety bound (see ReDMCSB GAMELOOP.C:67-78
 * source-locked citation above). */
#define ORCH_MAX_MAP_TRANSITIONS_PER_TICK 4

/* ================================================================
 *  Data structures
 * ================================================================ */

struct TickInput_Compat {
    uint32_t tick;
    uint8_t  command;
    uint8_t  commandArg1;
    uint8_t  commandArg2;
    uint8_t  reserved;
    uint32_t forcedRngAdvance;
    uint32_t reserved2;
};

struct TickEmission_Compat {
    uint8_t  kind;
    uint8_t  reserved;
    uint16_t payloadSize;
    int32_t  payload[4];
};

struct TickResult_Compat {
    uint32_t preTick;
    uint32_t postTick;
    uint32_t worldHashPost;
    int      emissionCount;
    struct TickEmission_Compat emissions[TICK_EMISSION_CAPACITY];
};

struct TickStreamRecord_Compat {
    struct TickInput_Compat input;
    uint32_t worldHashPost;
    uint16_t emissionCount;
    uint16_t reserved;
};

struct GameConfig_Compat {
    char     dungeonPath[48];
    uint32_t startingSeed;
    uint32_t flags;
    uint32_t reserved[2];
};

/*
 * GameWorld_Compat — THE aggregate. See header comment above for
 * documented deviations from PHASE20_PLAN.md §2.1.
 */
struct GameWorld_Compat {
    /* ---- Orchestrator scalars (mirror GAMELOOP.C globals) ---- */
    uint32_t gameTick;
    int32_t  partyDead;
    int32_t  gameWon;
    int32_t  partyMapIndex;
    int32_t  newPartyMapIndex;
    int32_t  partyIsResting;
    int32_t  freezeLifeTicks;
    int32_t  disabledMovementTicks;
    int32_t  projectileDisabledMovementTicks;
    int32_t  lastProjectileDisabledMovementDirection;

    /* ---- Dungeon static layer (pointer, see D1) ---- */
    struct DungeonDatState_Compat* dungeon;
    struct DungeonThings_Compat*   things;
    uint32_t dungeonFingerprint;   /* CRC32 of DUNGEON.DAT contents */
    int32_t  ownsDungeon;          /* 1 iff F0883 must free dungeon/things */

    /* ---- Sub-phase live state ---- */
    struct PartyState_Compat           party;              /* Phase 10 */
    struct SensorEffectList_Compat     pendingSensorEffects; /* Phase 11 */
    struct TimelineQueue_Compat        timeline;           /* Phase 12 */
    struct RngState_Compat             masterRng;          /* Phase 13 */
    struct CombatResult_Compat         pendingCombat;      /* Phase 13 */
    struct MagicState_Compat           magic;              /* Phase 14 */
    struct SaveGameHeader_Compat       saveHeader;         /* Phase 15 */
    struct DungeonMutationList_Compat  dungeonMutations;   /* Phase 15 */
    struct CreatureAIState_Compat      creatureAI[GAMEWORLD_CREATURE_AI_CAPACITY];
    int32_t                            creatureAICount;    /* Phase 16 */
    struct ProjectileList_Compat       projectiles;        /* Phase 17 */
    struct ExplosionList_Compat        explosions;         /* Phase 17 */
    struct LifecycleState_Compat       lifecycle;          /* Phase 18 */
    int32_t                            candidateAttackInvulnerableEnabled;
    int32_t                            candidateAttackInvulnerableGroupIndex;
    int32_t                            candidateAttackInvulnerableCreatureIndex;

    /* Phase 19 state is tracked via timeline chains, explosion slots
       and sensor toggling (see D5). */
};

/* ================================================================
 *  Group A — Construct / Destruct / Clone (F0880-F0883, F0880b)
 * ================================================================ */

struct GameWorld_Compat* F0880_WORLD_AllocDefault_Compat(void);

int F0881_WORLD_InitDefault_Compat(
    struct GameWorld_Compat* world,
    uint32_t seed);

int F0882_WORLD_InitFromDungeonDat_Compat(
    const char* dungeonPath,
    uint32_t seed,
    struct GameWorld_Compat* outWorld);

void F0883_WORLD_Free_Compat(struct GameWorld_Compat* world);

int F0880b_WORLD_Clone_Compat(
    const struct GameWorld_Compat* src,
    struct GameWorld_Compat* dst);

/* ================================================================
 *  Group B — Tick Orchestrator (F0884-F0886)
 * ================================================================ */

int F0884_ORCH_AdvanceOneTick_Compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* input,
    struct TickResult_Compat* outResult);

int F0885_ORCH_RunNTicks_Compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* inputs,
    int tickCount,
    struct TickStreamRecord_Compat* outRecords,
    uint32_t* outFinalHash);

/* F0284 public probe wrapper.
 *
 * Sets the party direction (0..3) and, in doing so, rotates every
 * present champion's per-cell Direction and Cell ordinal by the
 * delta (new - old) mod 4.  This is the public entry point for
 * MOV-05 (DM1 V1 functional-divergence-report.md): the
 *   `set party_direction_redmcsb_compat` static function in the
 * .c file is now exposed here so unit tests can exercise the
 * cell-rotation invariants without spinning up the full
 * F0884_ORCH_AdvanceOneTick_Compat path (which has the side
 * effect of scheduling the M010 / watchdog-tick events).
 *
 * Returns 1 if the direction actually changed, 0 if it was a
 * no-op (newDirection == oldDirection).  Idempotent.
 *
 * Source: ReDMCSB CHAMPION.C:117-130, F0284_CHAMPION_SetPartyDirection. */
int F0284_CHAMPION_SetPartyDirection_Compat(
    struct GameWorld_Compat* world,
    int newDirection);

int F0886_ORCH_RunUntilCondition_Compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* inputs,
    int maxTicks,
    int (*condition)(const struct GameWorld_Compat*),
    struct TickStreamRecord_Compat* outRecords,
    uint32_t* outFinalHash);

/* ================================================================
 *  Group C — Dispatch Internals (F0887-F0890)
 * ================================================================ */

int F0887_ORCH_DispatchTimelineEvents_Compat(
    struct GameWorld_Compat* world,
    struct TickResult_Compat* result);

int F0888_ORCH_ApplyPlayerInput_Compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* input,
    struct TickResult_Compat* result);

int F0888_ORCH_GetChampionF0303SkillLevel_Compat(
    const struct GameWorld_Compat* world,
    int championIndex,
    int skillIndex);
int F0888_ORCH_GetChampionF0312SkillBonus_Compat(
    const struct GameWorld_Compat* world,
    int championIndex,
    int weaponClass);
int F0888_ORCH_GetChampionActionHandWeaponClass_Compat(
    const struct GameWorld_Compat* world,
    int championIndex);
int F0888_ORCH_GetChampionActionHandWeaponInfo_Compat(
    const struct GameWorld_Compat* world,
    int championIndex,
    DM1_WeaponInfo* outInfo);
int F0888_ORCH_GetCreatureSnapshot_Compat(
    const struct GameWorld_Compat* world,
    int groupIndex,
    int creatureIndex,
    int doubledMapDifficulty,
    struct CombatantCreatureSnapshot_Compat* outSnapshot);

void F0889_ORCH_ApplyPendingDamage_Compat(
    struct GameWorld_Compat* world,
    struct TickResult_Compat* result);

void F0890_ORCH_ApplyPeriodicEffects_Compat(
    struct GameWorld_Compat* world,
    struct TickResult_Compat* result);

int F0890a_ORCH_ApplyProjectileCreatureImpact_Compat(
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    const struct ProjectileInstance_Compat* projectile);

/* ================================================================
 *  Group D — Determinism + Hash (F0891-F0893)
 * ================================================================ */

int F0891_ORCH_WorldHash_Compat(
    const struct GameWorld_Compat* world,
    uint32_t* outHash);

int F0892_ORCH_VerifyDeterminism_Compat(
    const struct GameWorld_Compat* initialWorld,
    const struct TickInput_Compat* inputs,
    int tickCount);

int F0893_ORCH_VerifyResumeEquivalence_Compat(
    const struct GameWorld_Compat* initialWorld,
    const struct TickInput_Compat* inputs,
    int tickCount,
    int resumeAtTick);

/* ================================================================
 *  Group E — Headless Driver Primitives (F0894-F0896)
 * ================================================================ */

int F0894_DRIVER_LoadTickStream_Compat(
    const char* path,
    struct TickInput_Compat** outInputs,
    int* outCount);

int F0895_DRIVER_RunStream_Compat(
    struct GameWorld_Compat* world,
    const struct TickInput_Compat* inputs,
    int inputCount,
    struct TickStreamRecord_Compat* outRecords,
    uint32_t* outFinalHash);

void F0896_DRIVER_WriteSummary_Compat(
    const struct GameWorld_Compat* world,
    uint32_t finalHash,
    int ticksRun,
    FILE* outFile);

/* ================================================================
 *  Group F — Serialise / Deserialise GameWorld (F0897-F0899)
 * ================================================================ */

int F0897_WORLD_Serialize_Compat(
    const struct GameWorld_Compat* world,
    unsigned char* outBuf,
    int outBufSize,
    int* outBytesWritten);

int F0898_WORLD_Deserialize_Compat(
    struct GameWorld_Compat* world,
    const unsigned char* buf,
    int bufSize,
    int* outBytesRead);

int F0899_WORLD_SerializedSize_Compat(
    const struct GameWorld_Compat* world);

/* ================================================================
 *  Supporting small serialisers for TickInput/Emission/Record/Config
 *  (round-trip invariants rely on these)
 * ================================================================ */

int F0897a_TickInput_Serialize_Compat(
    const struct TickInput_Compat* in,
    unsigned char* outBuf, int outBufSize);
int F0897a_TickInput_Deserialize_Compat(
    struct TickInput_Compat* out,
    const unsigned char* buf, int bufSize);

int F0897b_TickEmission_Serialize_Compat(
    const struct TickEmission_Compat* in,
    unsigned char* outBuf, int outBufSize);
int F0897b_TickEmission_Deserialize_Compat(
    struct TickEmission_Compat* out,
    const unsigned char* buf, int bufSize);

int F0897c_TickStreamRecord_Serialize_Compat(
    const struct TickStreamRecord_Compat* in,
    unsigned char* outBuf, int outBufSize);
int F0897c_TickStreamRecord_Deserialize_Compat(
    struct TickStreamRecord_Compat* out,
    const unsigned char* buf, int bufSize);

int F0897d_GameConfig_Serialize_Compat(
    const struct GameConfig_Compat* in,
    unsigned char* outBuf, int outBufSize);
int F0897d_GameConfig_Deserialize_Compat(
    struct GameConfig_Compat* out,
    const unsigned char* buf, int bufSize);

#endif /* REDMCSB_MEMORY_TICK_ORCHESTRATOR_PC34_COMPAT_H */
