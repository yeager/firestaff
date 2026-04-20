#ifndef REDMCSB_MEMORY_PROJECTILE_PC34_COMPAT_H
#define REDMCSB_MEMORY_PROJECTILE_PC34_COMPAT_H

/*
 * Projectile & explosion flight data layer for ReDMCSB PC 3.4 —
 * Phase 17 of M10. Authoritative plan: PHASE17_PLAN.md.
 *
 * Pure per-tick state transforms for:
 *   - PROJECTILE_MOVE events (F0811)
 *   - EXPLOSION_ADVANCE events (F0822)
 *
 * Conventions (inherited from Phases 10 – 16):
 *   - All symbols suffixed _pc34_compat / _Compat.
 *   - MEDIA016 / PC LSB-first int32 serialisation. Every struct
 *     round-trips bit-identical.
 *   - NO globals, NO UI, NO IO, NO hidden RNG. Randomness flows
 *     through Phase 13's RngState_Compat (F0732).
 *   - ADDITIVE ONLY: zero edits to Phase 9 / 10 / 11 / 12 / 13 /
 *     14 / 15 / 16 source. We consume their types via #include
 *     and pure composition.
 *
 * Function numbering: F0810 – F0829 (Phase 17 slot).
 *
 * Fontanel primary references (PROJEXPL.C):
 *   - F0212_PROJECTILE_Create                 (launch, schedule C48/C49)
 *   - F0213_EXPLOSION_Create                  (explosion spawn)
 *   - F0216_PROJECTILE_GetImpactAttack        (impact-attack formula)
 *   - F0217_PROJECTILE_HasImpactOccured       (impact classifier)
 *   - F0219_PROJECTILE_ProcessEvents48To49    (per-tick move)
 *   - F0220_EXPLOSION_ProcessEvent25          (per-tick explosion)
 *   - F0221_GROUP_IsFluxcageOnSquare          (fluxcage detection)
 *   - F0224_GROUP_FluxCageAction              (fluxcage spawn; v1 data-only)
 */

#include <stdint.h>

#include "memory_combat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_magic_pc34_compat.h"

/* ==========================================================
 *  Serialised sizes (MEDIA016 / LSB-first, 4-byte int32 fields).
 * ========================================================== */

#define PROJECTILE_INSTANCE_SERIALIZED_SIZE        96  /* 24 int32 */
#define EXPLOSION_INSTANCE_SERIALIZED_SIZE         64  /* 16 int32 */
#define CELL_CONTENT_DIGEST_SERIALIZED_SIZE       100  /* 25 int32 */
#define PROJECTILE_TICK_RESULT_SERIALIZED_SIZE    232  /* see §2.5 */
#define EXPLOSION_TICK_RESULT_SERIALIZED_SIZE     184  /* see §2.6 */

#define PROJECTILE_LIST_CAPACITY                   60
#define EXPLOSION_LIST_CAPACITY                    32

#define PROJECTILE_LIST_SERIALIZED_SIZE \
    (8 + PROJECTILE_LIST_CAPACITY * PROJECTILE_INSTANCE_SERIALIZED_SIZE)
#define EXPLOSION_LIST_SERIALIZED_SIZE \
    (8 + EXPLOSION_LIST_CAPACITY * EXPLOSION_INSTANCE_SERIALIZED_SIZE)

/* ==========================================================
 *  Projectile categories / owner kinds (stable save-blob contract).
 * ========================================================== */

#define PROJECTILE_CATEGORY_KINETIC                 0
#define PROJECTILE_CATEGORY_MAGICAL                 1

#define PROJECTILE_OWNER_CHAMPION                   0
#define PROJECTILE_OWNER_CREATURE                   1
#define PROJECTILE_OWNER_LAUNCHER                   2

/* ==========================================================
 *  Projectile subtypes (mirror of PROJECTILE.Slot semantics).
 * ========================================================== */

#define PROJECTILE_SUBTYPE_FIREBALL              0x80
#define PROJECTILE_SUBTYPE_SLIME                 0x81
#define PROJECTILE_SUBTYPE_LIGHTNING_BOLT        0x82
#define PROJECTILE_SUBTYPE_HARM_NON_MATERIAL     0x83
#define PROJECTILE_SUBTYPE_OPEN_DOOR             0x84
#define PROJECTILE_SUBTYPE_POISON_BOLT           0x86
#define PROJECTILE_SUBTYPE_POISON_CLOUD          0x87
#define PROJECTILE_SUBTYPE_SMOKE                 0xA8
#define PROJECTILE_SUBTYPE_REBIRTH_STEP1         0xE4
#define PROJECTILE_SUBTYPE_FLUXCAGE              0x32  /* C050, data only in v1 */

/* Kinetic sentinel used in v1 table init. */
#define PROJECTILE_SUBTYPE_KINETIC_ARROW         0x00

/* ==========================================================
 *  Explosion types (mirror of Fontanel C000..C101).
 * ========================================================== */

#define C000_EXPLOSION_FIREBALL                     0
#define C001_EXPLOSION_SLIME                        1
#define C002_EXPLOSION_LIGHTNING_BOLT               2
#define C003_EXPLOSION_HARM_NON_MATERIAL            3
#define C004_EXPLOSION_OPEN_DOOR                    4
#define C007_EXPLOSION_POISON_CLOUD                 7
#define C040_EXPLOSION_SMOKE                       40
#define C050_EXPLOSION_FLUXCAGE                    50
#define C100_EXPLOSION_REBIRTH_STEP1              100
#define C101_EXPLOSION_REBIRTH_STEP2              101

/* Centered-cell sentinel (0xFF = single-centered-creature). */
#define EXPLOSION_CELL_CENTERED                  0xFF

/* ==========================================================
 *  Cell-content element types (mirror of DEFS.H C00..C06).
 *  Duplicated here so Phase 17 does not pull Phase 9 types
 *  into its per-tick digest; caller translates once.
 * ========================================================== */

#define PROJECTILE_ELEMENT_WALL                     0
#define PROJECTILE_ELEMENT_CORRIDOR                 1
#define PROJECTILE_ELEMENT_PIT                      2
#define PROJECTILE_ELEMENT_STAIRS                   3
#define PROJECTILE_ELEMENT_DOOR                     4
#define PROJECTILE_ELEMENT_TELEPORTER               5
#define PROJECTILE_ELEMENT_FAKEWALL                 6

/* ==========================================================
 *  Projectile tick-result kinds (stable).
 * ========================================================== */

#define PROJECTILE_RESULT_FLEW                      0
#define PROJECTILE_RESULT_HIT_WALL                  1
#define PROJECTILE_RESULT_HIT_DOOR                  2
#define PROJECTILE_RESULT_HIT_CHAMPION              3
#define PROJECTILE_RESULT_HIT_CREATURE              4
#define PROJECTILE_RESULT_HIT_FLUXCAGE              5
#define PROJECTILE_RESULT_HIT_OTHER_PROJECTILE      6
#define PROJECTILE_RESULT_DESPAWN_ENERGY            7
#define PROJECTILE_RESULT_DESPAWN_BOUNDS            8
#define PROJECTILE_RESULT_INVALID                   9

/* ==========================================================
 *  Explosion tick-result kinds (stable).
 * ========================================================== */

#define EXPLOSION_RESULT_ONE_SHOT                   0
#define EXPLOSION_RESULT_ADVANCED_FRAME             1
#define EXPLOSION_RESULT_PERSISTENT                 2
#define EXPLOSION_RESULT_INVALID                    3

/* ==========================================================
 *  Cell-content-digest blocker codes (F0814 output).
 * ========================================================== */

#define PROJECTILE_BLOCKER_OPEN                     0
#define PROJECTILE_BLOCKER_WALL                     1
#define PROJECTILE_BLOCKER_STAIRS                   2
#define PROJECTILE_BLOCKER_CLOSED_DOOR              3
#define PROJECTILE_BLOCKER_FLUXCAGE                 4
#define PROJECTILE_BLOCKER_BOUNDARY                 5
#define PROJECTILE_BLOCKER_OTHER_PROJECTILE         6

/* ==========================================================
 *  Projectile flags (bitmask).
 * ========================================================== */

#define PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT  0x0001
#define PROJECTILE_FLAG_CREATES_EXPLOSION        0x0002
#define PROJECTILE_FLAG_IGNORE_DOOR_PASS_THROUGH 0x0004

/* ==========================================================
 *  Door state values used in the digest. -1 means "no door".
 * ========================================================== */

#define PROJECTILE_DOOR_STATE_NONE               (-1)
#define PROJECTILE_DOOR_STATE_OPEN                 0
#define PROJECTILE_DOOR_STATE_CLOSED_ONE_FOURTH    1
#define PROJECTILE_DOOR_STATE_CLOSED_HALF          2
#define PROJECTILE_DOOR_STATE_CLOSED_THREE_FOURTH  3
#define PROJECTILE_DOOR_STATE_CLOSED_FULL          4
#define PROJECTILE_DOOR_STATE_DESTROYED            5

/* ==========================================================
 *  Data structures.
 * ========================================================== */

struct ProjectileInstance_Compat {
    int slotIndex;                /* 0..59, -1 = empty */
    int projectileCategory;       /* PROJECTILE_CATEGORY_* */
    int projectileSubtype;
    int ownerKind;                /* PROJECTILE_OWNER_* */
    int ownerIndex;
    int mapIndex;
    int mapX;
    int mapY;
    int cell;                     /* 0..3 */
    int direction;                /* 0..3 */
    int kineticEnergy;
    int attack;
    int stepEnergy;
    int firstMoveGraceFlag;
    int launchedAtTick;
    int scheduledAtTick;
    int associatedPotionPower;
    int poisonAttack;
    int attackTypeCode;           /* COMBAT_ATTACK_* */
    int flags;                    /* PROJECTILE_FLAG_* */
    int reserved0;
    int reserved1;
    int reserved2;
    int reserved3;
};

struct ExplosionInstance_Compat {
    int slotIndex;                /* 0..31, -1 = empty */
    int explosionType;            /* C000..C101 */
    int mapIndex;
    int mapX;
    int mapY;
    int cell;                     /* 0..3; 0xFF = centered */
    int centered;                 /* 1 if centered spawn */
    int attack;
    int currentFrame;
    int maxFrames;
    int poisonAttack;
    int scheduledAtTick;
    int ownerKind;
    int ownerIndex;
    int creatorProjectileSlot;    /* -1 if none */
    int reserved0;
};

struct ProjectileList_Compat {
    int count;                    /* 0..60 */
    int reserved;
    struct ProjectileInstance_Compat entries[PROJECTILE_LIST_CAPACITY];
};

struct ExplosionList_Compat {
    int count;                    /* 0..32 */
    int reserved;
    struct ExplosionInstance_Compat entries[EXPLOSION_LIST_CAPACITY];
};

/*
 * Caller-pre-baked digest of cell content relevant to projectile
 * motion. Phase 17 reads ONLY this struct in the per-tick path.
 *
 * NEEDS DISASSEMBLY REVIEW: teleporter direction rotation.
 *   `destTeleporterNewDirection` replaces a reserved slot from the
 *   plan's §2.4 draft so the struct stays 100 bytes. v1 leaves the
 *   field default -1 and does NOT rotate; caller pre-rotates.
 */
struct CellContentDigest_Compat {
    /* Source cell (where projectile currently sits). */
    int sourceMapIndex;
    int sourceMapX;
    int sourceMapY;
    int sourceSquareType;
    int sourceHasFluxcage;
    int sourceHasOtherProjectile;

    /* Destination cell (source + direction step). */
    int destMapIndex;
    int destMapX;
    int destMapY;
    int destSquareType;
    int destFakeWallIsImaginaryOrOpen;
    int destHasFluxcage;
    int destHasOtherProjectile;
    int destHasChampion;
    int destPartyDirection;
    int destChampionCellMask;
    int destHasCreatureGroup;
    int destCreatureType;
    int destCreatureCellMask;
    int destCreatureIsNonMaterial;
    int destDoorState;
    int destDoorAllowsProjectilePassThrough;
    int destDoorIsDestroyed;
    int destIsMapBoundary;
    int destTeleporterNewDirection;   /* -1 = no rotation */
};

/*
 * Pure output of F0811 per-tick advance. 232 bytes.
 */
struct ProjectileTickResult_Compat {
    int resultKind;               /* PROJECTILE_RESULT_* */
    int despawn;
    int crossedCell;
    int newCell;
    int newMapIndex;
    int newMapX;
    int newMapY;
    int newDirection;
    int newKineticEnergy;
    int newAttack;
    int newFirstMoveGraceFlag;
    int emittedCombatAction;
    int emittedExplosion;
    int emittedDoorDestructionEvent;
    int emittedSoundCode;
    int rngCallCount;
    int reserved0;
    int reserved1;

    struct CombatAction_Compat      outAction;     /* 48 B */
    struct ExplosionInstance_Compat outExplosion;  /* 64 B */
    struct TimelineEvent_Compat     outNextTick;   /* 44 B */
    int outTickPadding;                            /*  4 B */
};

/*
 * Pure output of F0822 per-tick explosion advance. 184 bytes.
 */
struct ExplosionTickResult_Compat {
    int resultKind;               /* EXPLOSION_RESULT_* */
    int despawn;
    int newAttack;
    int newCurrentFrame;
    int emittedCombatActionPartyCount;
    int emittedCombatActionGroupCount;
    int emittedDoorDestructionEvent;
    int rngCallCount;
    int reserved0;
    int reserved1;

    struct CombatAction_Compat  outActionParty;   /* 48 B */
    struct CombatAction_Compat  outActionGroup;   /* 48 B */
    struct TimelineEvent_Compat outNextTick;      /* 44 B */
    int outTickPadding;                            /* 4 B */
};

/*
 * F0810 input (projectile create).
 */
struct ProjectileCreateInput_Compat {
    int category;                 /* PROJECTILE_CATEGORY_* */
    int subtype;
    int ownerKind;                /* PROJECTILE_OWNER_* */
    int ownerIndex;
    int mapIndex;
    int mapX;
    int mapY;
    int cell;                     /* 0..3 */
    int direction;                /* 0..3 */
    int kineticEnergy;
    int attack;
    int stepEnergy;
    int currentTick;
    int poisonAttack;
    int attackTypeCode;
    int potionPower;
    int firstMoveGraceFlag;
};

/*
 * F0821 input (explosion create).
 */
struct ExplosionCreateInput_Compat {
    int explosionType;
    int attack;
    int mapIndex;
    int mapX;
    int mapY;
    int cell;                     /* 0..3; 0xFF for centered */
    int centered;
    int poisonAttack;
    int currentTick;
    int ownerKind;
    int ownerIndex;
    int creatorProjectileSlot;
};

/* ==========================================================
 *  Group A — Projectile lifecycle (F0810 – F0813).
 * ========================================================== */

int F0810_PROJECTILE_Create_Compat(
    const struct ProjectileCreateInput_Compat* in,
    struct ProjectileList_Compat* list,
    int* outSlotIndex,
    struct TimelineEvent_Compat* outFirstMoveEvent);

int F0811_PROJECTILE_Advance_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    uint32_t currentTick,
    struct RngState_Compat* rng,
    struct ProjectileInstance_Compat* outNewState,
    struct ProjectileTickResult_Compat* outResult);

int F0812_PROJECTILE_CreateFromSpellEffect_Compat(
    const struct SpellEffect_Compat* effect,
    int casterChampionIndex,
    int partyMapIndex,
    int partyMapX,
    int partyMapY,
    int partyDirection,
    uint32_t currentTick,
    struct ProjectileList_Compat* list,
    int* outSlotIndex,
    struct TimelineEvent_Compat* outFirstMoveEvent);

int F0813_PROJECTILE_Despawn_Compat(
    struct ProjectileList_Compat* list,
    int slotIndex);

/* ==========================================================
 *  Group B — Cell-content inspection (F0814 – F0816).
 * ========================================================== */

int F0814_PROJECTILE_InspectDestination_Compat(
    const struct CellContentDigest_Compat* digest,
    int* outBlocker);

int F0815_PROJECTILE_ComputeImpactAttack_Compat(
    const struct ProjectileInstance_Compat* in,
    int* outAttack);

int F0816_PROJECTILE_DoesPassThroughDoor_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    struct RngState_Compat* rng,
    int* outPasses);

/* ==========================================================
 *  Group C — Collision resolution (F0817 – F0820).
 * ========================================================== */

int F0817_PROJECTILE_BuildHitCreatureAction_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int impactAttack,
    struct CombatAction_Compat* outAction);

int F0818_PROJECTILE_BuildHitChampionAction_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int impactAttack,
    int championIndex,
    struct CombatAction_Compat* outAction);

int F0819_PROJECTILE_BuildDoorDestructionEvent_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int impactAttack,
    uint32_t currentTick,
    struct RngState_Compat* rng,
    struct TimelineEvent_Compat* outEvent);

int F0820_PROJECTILE_ResolveCollision_Compat(
    const struct ProjectileInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    int impactTarget,
    uint32_t currentTick,
    struct RngState_Compat* rng,
    struct ProjectileTickResult_Compat* outResult);

/* ==========================================================
 *  Group D — Explosion lifecycle (F0821 – F0824).
 * ========================================================== */

int F0821_EXPLOSION_Create_Compat(
    const struct ExplosionCreateInput_Compat* in,
    struct ExplosionList_Compat* list,
    int* outSlotIndex,
    struct TimelineEvent_Compat* outFirstAdvanceEvent);

int F0822_EXPLOSION_Advance_Compat(
    const struct ExplosionInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    uint32_t currentTick,
    struct RngState_Compat* rng,
    struct ExplosionInstance_Compat* outNewState,
    struct ExplosionTickResult_Compat* outResult);

int F0823_EXPLOSION_ComputeAoE_Compat(
    const struct ExplosionInstance_Compat* in,
    const struct CellContentDigest_Compat* digest,
    struct RngState_Compat* rng,
    int* outAttackApplied);

int F0824_EXPLOSION_Despawn_Compat(
    struct ExplosionList_Compat* list,
    int slotIndex);

/* ==========================================================
 *  Group E — Scheduling helpers (F0825 – F0826).
 * ========================================================== */

int F0825_PROJECTILE_ScheduleNextMove_Compat(
    const struct ProjectileInstance_Compat* in,
    int onPartyMap,
    uint32_t currentTick,
    struct TimelineEvent_Compat* outEvent);

int F0826_EXPLOSION_ScheduleNextAdvance_Compat(
    const struct ExplosionInstance_Compat* in,
    uint32_t currentTick,
    int forcedDelay,
    struct TimelineEvent_Compat* outEvent);

/* ==========================================================
 *  Group F — Serialisation (F0827 – F0829).
 *  MEDIA016 / LSB-first, bit-identical round-trip.
 * ========================================================== */

int F0827_PROJECTILE_InstanceSerialize_Compat(
    const struct ProjectileInstance_Compat* in,
    unsigned char* outBuf,
    int outBufSize);

int F0827_PROJECTILE_InstanceDeserialize_Compat(
    struct ProjectileInstance_Compat* out,
    const unsigned char* buf,
    int bufSize);

int F0828_EXPLOSION_InstanceSerialize_Compat(
    const struct ExplosionInstance_Compat* in,
    unsigned char* outBuf,
    int outBufSize);

int F0828_EXPLOSION_InstanceDeserialize_Compat(
    struct ExplosionInstance_Compat* out,
    const unsigned char* buf,
    int bufSize);

int F0829_PROJECTILE_ListSerialize_Compat(
    const struct ProjectileList_Compat* in,
    unsigned char* outBuf,
    int outBufSize);

int F0829_PROJECTILE_ListDeserialize_Compat(
    struct ProjectileList_Compat* out,
    const unsigned char* buf,
    int bufSize);

int F0829_EXPLOSION_ListSerialize_Compat(
    const struct ExplosionList_Compat* in,
    unsigned char* outBuf,
    int outBufSize);

int F0829_EXPLOSION_ListDeserialize_Compat(
    struct ExplosionList_Compat* out,
    const unsigned char* buf,
    int bufSize);

#endif /* REDMCSB_MEMORY_PROJECTILE_PC34_COMPAT_H */
