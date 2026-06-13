#ifndef REDMCSB_MEMORY_CREATURE_AI_PC34_COMPAT_H
#define REDMCSB_MEMORY_CREATURE_AI_PC34_COMPAT_H

/*
 * Creature AI / monster behavior data layer for ReDMCSB PC 3.4 —
 * Phase 16 of M10. Authoritative plan: PHASE16_PLAN.md.
 *
 * Pure, caller-driven creature-tick transformer:
 *     F0804(stateIn, input, rng) -> (stateOut, CombatAction?, movement?,
 *                                    SpellCastRequest? [stub], TimelineEvent)
 *
 * Conventions (inherited from Phases 10 – 15):
 *   - All symbols suffixed _pc34_compat / _Compat.
 *   - NO globals, NO UI, NO IO, NO hidden RNG (all randomness flows
 *     through Phase 13's `RngState_Compat*`, F0732).
 *   - MEDIA016 / PC LSB-first serialisation. Every (de)serialised
 *     struct round-trips bit-identical.
 *   - ADDITIVE ONLY: zero edits to Phase 9 / 10 / 11 / 12 / 13 / 14 / 15
 *     source. We consume their types via #include and pure composition.
 *
 * Function numbering: F0790 – F0809 (Phase 16 slot).
 *
 * v1 scope highlights (plan §1):
 *   - 27 creature types fully implemented (BUG-104 final): all DM1
 *     creature types now FULL tier with per-type behavior in F0804 §(5b).
 *   - Spell-casting creatures (Vexirk, Lord Chaos, Materializer) use
 *     the FULL tier with emittedSpellRequest as a v1 surrogate for the
 *     M11 vision-share/invisibility/projection channels; full effect
 *     dispatch into M11 is deferred to a future pass.
 *   - One-step pathfinding only (primary / secondary / opposite / RNG
 *     fallback). No A*.
 *   - Perception delegates LoS to a caller-provided pre-baked flag.
 *
 * PLAN DEVIATION (documented once here and once in the probe):
 *   PHASE16_PLAN.md §2.2 lists 34 fields for CreatureTickInput_Compat
 *   while simultaneously asserting the serialized size is 128 bytes
 *   (32 int32). The algorithm pseudocode in §4.1, §4.5 and §4.8 also
 *   reads three fields not present in §2.2 (losClearFlag, primaryDir,
 *   secondaryDir). To satisfy both the 128-byte size invariant and
 *   the algorithm's field set, we drop the redundant / unused fields
 *   `groupDirection`, `partyDirection`, `groupCountMinus1`, `eventType`
 *   and `reserved0` (5 int32) and add the three required fields from
 *   the algorithm sections (3 int32). Net: 34 - 5 + 3 = 32 int32 →
 *   128 bytes exactly. This is the smallest-surface change that keeps
 *   invariant 2 PASS and the algorithm implementable without
 *   fabricating mechanics (R3 mitigation).
 */

#include <stdint.h>

#include "memory_combat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

/* ==========================================================
 *  Serialised sizes (MEDIA016 / LSB-first, 4-byte int32 fields)
 * ========================================================== */

#define CREATURE_AI_STATE_SERIALIZED_SIZE      72   /* 18 int32 */
#define CREATURE_TICK_INPUT_SERIALIZED_SIZE   128   /* 32 int32 */
#define CREATURE_TICK_RESULT_SERIALIZED_SIZE  176   /* 16 + 48 + 16 + 44 + 4 */
#define CREATURE_BEHAVIOR_PROFILE_SIZE         64   /* 16 int32 (internal) */

/* ==========================================================
 *  Creature-type inventory (matches DEFS.H C027_CREATURE_TYPE_COUNT)
 * ========================================================== */

#define CREATURE_TYPE_COUNT                    27
#define CREATURE_TYPE_GIANT_SCORPION            0
#define CREATURE_TYPE_SWAMP_SLIME               1
#define CREATURE_TYPE_GIGGLER                   2
#define CREATURE_TYPE_WIZARD_EYE                3
#define CREATURE_TYPE_PAIN_RAT                  4
#define CREATURE_TYPE_RUSTER                    5
#define CREATURE_TYPE_SCREAMER                  6
#define CREATURE_TYPE_ROCKPILE                  7
#define CREATURE_TYPE_GHOST                     8
#define CREATURE_TYPE_STONE_GOLEM               9
#define CREATURE_TYPE_MUMMY                    10
#define CREATURE_TYPE_BLACK_FLAME              11
#define CREATURE_TYPE_SKELETON                 12
#define CREATURE_TYPE_COUATL                   13
#define CREATURE_TYPE_VEXIRK                   14
#define CREATURE_TYPE_MAGENTA_WORM             15
#define CREATURE_TYPE_TROLIN                   16
#define CREATURE_TYPE_GIANT_WASP               17
#define CREATURE_TYPE_ANIMATED_ARMOUR          18
#define CREATURE_TYPE_MATERIALIZER             19
#define CREATURE_TYPE_WATER_ELEMENTAL          20
#define CREATURE_TYPE_OITU                     21
#define CREATURE_TYPE_DEMON                    22
#define CREATURE_TYPE_LORD_CHAOS               23
#define CREATURE_TYPE_RED_DRAGON               24
#define CREATURE_TYPE_LORD_ORDER               25
#define CREATURE_TYPE_GREY_LORD                26

/* ==========================================================
 *  AI state enum (stable — serialised forever)
 * ========================================================== */

#define AI_STATE_IDLE        0
#define AI_STATE_WANDER      1
#define AI_STATE_FLEE        2
#define AI_STATE_ATTACK      3
#define AI_STATE_APPROACH    4
#define AI_STATE_STUN        5
#define AI_STATE_DEAD        6
#define AI_STATE_COUNT       7

/* ==========================================================
 *  Tick-result kinds (stable — not serialised; part of outputs)
 * ========================================================== */

#define AI_RESULT_NO_ACTION         0
#define AI_RESULT_MOVED             1
#define AI_RESULT_ATTACKED          2
#define AI_RESULT_WOKE              3
#define AI_RESULT_FLED              4
#define AI_RESULT_STUNNED           5
#define AI_RESULT_DIED              6
#define AI_RESULT_SKIPPED_FROZEN    7

/* ==========================================================
 *  Creature attribute bit masks (DEFS.H MASKxxxx mirrors)
 * ========================================================== */

#define CREATURE_ATTR_MASK_SIDE_ATTACK     0x0004
#define CREATURE_ATTR_MASK_PREFER_BACK_ROW 0x0008
#define CREATURE_ATTR_MASK_ATTACK_ANY_CHAMPION 0x0010
#define CREATURE_ATTR_MASK_LEVITATION      0x0020
#define CREATURE_ATTR_MASK_NON_MATERIAL    0x0040
#define CREATURE_ATTR_MASK_SEE_INVISIBLE   0x0800
#define CREATURE_ATTR_MASK_NIGHT_VISION    0x1000
#define CREATURE_ATTR_MASK_ARCHENEMY       0x2000
#define CREATURE_ATTR_MASK_DROP_FIXED      0x0200

/* ==========================================================
 *  Implementation tier values
 * ========================================================== */

#define CREATURE_IMPL_TIER_STUB     0
#define CREATURE_IMPL_TIER_FULL     1

/* ==========================================================
 *  Per-group persistent AI state (18 int32 = 72 bytes).
 *  Saved across ticks; serialised for Phase 15 save blobs.
 * ========================================================== */

struct CreatureAIState_Compat {
    int stateKind;               /* AI_STATE_* */
    int creatureType;            /* 0..26 */
    int groupMapIndex;
    int groupMapX;
    int groupMapY;
    int groupCells;              /* mirror of DungeonGroup_Compat.cells */
    int groupDirection;          /* 0..3 */
    int targetChampionIndex;     /* -1 = none, else 0..3 */
    int lastSeenPartyMapX;
    int lastSeenPartyMapY;
    int lastSeenPartyTick;       /* uint32 widened */
    int fearCounter;             /* 0..255, decrements in STUN / FLEE */
    int turnCounter;             /* ticks since last real decision */
    int attackCooldownTicks;     /* 0 when ready to attack */
    int movementCooldownTicks;   /* 0 when ready to move */
    int aggressionScore;         /* 0..100, profile-driven */
    int rngCallCount;            /* diagnostic — summed across ticks */
    int reserved0;               /* keeps struct 18 x int32 = 72 bytes */
};

/* ==========================================================
 *  Per-tick world snapshot (32 int32 = 128 bytes).
 *
 *  See §2.2 / algorithm sections of PHASE16_PLAN.md; deviation from
 *  plan field list is documented at the top of this file.
 * ========================================================== */

struct CreatureTickInput_Compat {
    int groupSlotIndex;                  /* 0..groupCount-1 */
    int creatureType;                    /* 0..26 */
    int groupMapIndex;
    int groupMapX;
    int groupMapY;
    int groupCells;
    int groupCurrentHealth[4];           /* mirror of DungeonGroup_Compat.health */
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int partyChampionsAlive;             /* bitmask 0..15 */
    int partyChampionCurrentHealth[4];
    int adjacencyWallMask;               /* bit i = direction i blocked by wall/fakewall */
    int adjacencyDoorMask;               /* bit i = direction i has closed door */
    int adjacencyPitMask;                /* bit i = direction i has open pit */
    int adjacencyCreatureMask;           /* bit i = direction i has another live group */
    int onFluxcageFlag;                  /* 1 iff group tile carries fluxcage */
    int onPoisonCloudFlag;               /* 1 iff group tile carries poison cloud */
    int onPitFlag;                       /* 1 iff group tile is an open pit */
    int selfDamagePerTick;               /* caller-computed hazard damage */
    int currentTickLow;                  /* lower 32 bits of game time */
    int freezeLifeTicks;                 /* Phase 14 MagicState mirror */
    int partyInvisibility;               /* Phase 14 MagicState mirror (0/1) */
    int losClearFlag;                    /* caller-baked LoS (1 = clear) */
    int primaryDir;                      /* 0..3; sign of dx/dy toward party */
    int secondaryDir;                    /* -1..3; orthogonal fallback */
};

/* ==========================================================
 *  Tick result (176 bytes; see §2.3).
 *
 *  Layout:
 *    [  0..  63]  header        (16 int32)
 *    [ 64..111]  outAction      (CombatAction_Compat, 48 bytes)
 *    [112..127]  movement block (4 int32)
 *    [128..171]  outNextTick    (TimelineEvent_Compat, 44 bytes)
 *    [172..175]  outTickPadding (1 int32)
 * ========================================================== */

struct CreatureTickResult_Compat {
    /* --- header (16 int32 = 64 bytes) --- */
    int resultKind;               /* AI_RESULT_* */
    int newAIState;               /* copy of stateOut.stateKind */
    int emittedCombatAction;      /* 0 or 1 */
    int emittedSpellRequest;      /* 0 in v1 (stub) */
    int emittedMovement;          /* 0 or 1 */
    int emittedSelfDamage;        /* 0 or 1 */
    int reactionPending;          /* 0 or 1 */
    int dropItemsPending;         /* 0 or 1 (only when resultKind == DIED) */
    int nextEventDelayTicks;      /* >= 1 */
    int newMovementCooldown;
    int newAttackCooldown;
    int newFearCounter;
    int rngCallCount;
    int emittedDoubleMove;        /* 0 or 1 — archenemy F0204 second-square move.
                                   * Replaces former reserved0. Layout is still
                                   * 16 int32 = 64 bytes (BUG-115b). */
    int reserved1;
    int reserved2;

    /* --- combat action payload (48 bytes) --- */
    struct CombatAction_Compat outAction;

    /* --- movement block (4 int32 = 16 bytes) --- */
    int outMovementTargetMapX;
    int outMovementTargetMapY;
    int outMovementDirection;
    int outMovementReserved;

    /* --- follow-up timeline event (44 bytes) --- */
    struct TimelineEvent_Compat outNextTick;

    /* --- envelope padding (4 bytes) --- */
    int outTickPadding;
};

/* ==========================================================
 *  Per-creature-type static behavior profile (16 int32 = 64 bytes).
 *  Hand-entered table; NOT serialised — immutable game data.
 * ========================================================== */

struct CreatureBehaviorProfile_Compat {
    int creatureType;            /* 0..26 */
    int sightRange;
    int smellRange;
    int movementTicks;
    int attackTicks;
    int baseAttack;
    int baseDefense;
    int baseHealth;
    int dexterity;
    int poisonAttack;
    int attackType;              /* COMBAT_ATTACK_* */
    int woundProbabilities;      /* raw 16-bit (low 16 used) */
    int attributes;              /* raw 16-bit attribute mask */
    int aggressionBias;          /* 0..100 */
    int implementationTier;      /* 0 stub, 1 full */
    int reserved0;
};

/* ==========================================================
 *  Profile-table accessor (internal table lives in .c).
 * ========================================================== */

const struct CreatureBehaviorProfile_Compat*
CREATURE_GetProfile_Compat(int creatureType);

/* ==========================================================
 *  Group A — Perception (F0790 – F0792)
 * ========================================================== */

int F0790_CREATURE_GetManhattanDistance_Compat(
    int ax, int ay, int bx, int by,
    int* out);

int F0791_CREATURE_IsDestinationVisible_Compat(
    const struct CreatureTickInput_Compat* in,
    int* outDistance);

int F0792_CREATURE_Perceive_Compat(
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int* outPartyVisible,
    int* outDistance,
    int* outCanSmell);

/* ==========================================================
 *  Group B — State machine (F0793 – F0795)
 * ========================================================== */

int F0793_CREATURE_ComputeNextState_Compat(
    const struct CreatureAIState_Compat* s,
    const struct CreatureTickInput_Compat* in,
    int partyVisible,
    int canSmell,
    int* outNextState,
    int* outAggressionDelta);

int F0794_CREATURE_ApplyFreezeLifeGate_Compat(
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int* outSkipTick,
    int* outRescheduleTicks);

int F0795_CREATURE_DecrementCounters_Compat(
    struct CreatureAIState_Compat* inOut,
    int elapsedTicks);

/* ==========================================================
 *  Group C — Target selection (F0796 – F0797)
 * ========================================================== */

int F0796_CREATURE_PickChampion_Compat(
    const struct CreatureTickInput_Compat* in,
    int* outChampionIndex);

int F0797_CREATURE_ScoreCandidates_Compat(
    const struct CreatureTickInput_Compat* in,
    int outScores[4]);

/* ==========================================================
 *  Group D — Pathfinding (F0798 – F0799)
 * ========================================================== */

int F0798_CREATURE_IsDirectionOpen_Compat(
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int direction,
    int allowImaginaryPitsAndFakeWalls,
    int* outBlocker);

int F0799_CREATURE_PickMoveDirection_Compat(
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int primaryDir,
    int secondaryDir,
    int allowFakeWalls,
    struct RngState_Compat* rng,
    int* outDirection);

/* ==========================================================
 *  Group E — Action emission (F0800 – F0803)
 * ========================================================== */

int F0800_CREATURE_EmitCombatAction_Compat(
    const struct CreatureAIState_Compat* s,
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int targetChampionIndex,
    struct CombatAction_Compat* outAction);

int F0801_CREATURE_EmitMovement_Compat(
    const struct CreatureAIState_Compat* s,
    const struct CreatureTickInput_Compat* in,
    int direction,
    struct CreatureTickResult_Compat* outResult);

/* F0801b: F0204 archenemy double-move second-square helper.
 * Source: GROUP.C F0204 lines 1576-1589 + F0202 lines 1457-1554.
 * Computes the second-square target one step further than the
 * first-square target supplied by the caller. The final
 * outResult has the second square as its target and
 * outMovementReserved set to 1 as a "this is a double-move"
 * marker. */
int F0801b_CREATURE_EmitArchenemySecondSquare_Compat(
    int firstSquareX,
    int firstSquareY,
    int direction,
    struct CreatureTickResult_Compat* outResult);

int F0802_CREATURE_EmitNextTickEvent_Compat(
    const struct CreatureAIState_Compat* s,
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    int forcedDelayTicks,
    struct TimelineEvent_Compat* outEvent);

int F0803_CREATURE_EmitSelfDamage_Compat(
    const struct CreatureTickInput_Compat* in,
    const struct CreatureBehaviorProfile_Compat* profile,
    struct CombatAction_Compat* outAction);

/* ==========================================================
 *  Group F — Top-level orchestrator (F0804)
 * ========================================================== */

int F0804_CREATURE_Tick_Compat(
    const struct CreatureAIState_Compat* stateIn,
    const struct CreatureTickInput_Compat* in,
    struct RngState_Compat* rng,
    struct CreatureAIState_Compat* stateOut,
    struct CreatureTickResult_Compat* out);

/* ==========================================================
 *  Group G — Serialisation (F0805 – F0809)
 * ========================================================== */

int F0805_CREATURE_AIStateSerialize_Compat(
    const struct CreatureAIState_Compat* s,
    unsigned char* buf,
    int bufSize);

int F0806_CREATURE_AIStateDeserialize_Compat(
    struct CreatureAIState_Compat* s,
    const unsigned char* buf,
    int bufSize);

int F0807_CREATURE_TickInputSerialize_Compat(
    const struct CreatureTickInput_Compat* s,
    unsigned char* buf,
    int bufSize);

int F0808_CREATURE_TickInputDeserialize_Compat(
    struct CreatureTickInput_Compat* s,
    const unsigned char* buf,
    int bufSize);

int F0809a_CREATURE_TickResultSerialize_Compat(
    const struct CreatureTickResult_Compat* s,
    unsigned char* buf,
    int bufSize);

int F0809b_CREATURE_TickResultDeserialize_Compat(
    struct CreatureTickResult_Compat* s,
    const unsigned char* buf,
    int bufSize);

#endif /* REDMCSB_MEMORY_CREATURE_AI_PC34_COMPAT_H */
