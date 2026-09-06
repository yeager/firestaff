#ifndef DM1_V1_CREATURE_AI_BEHAVIOR_PC34_COMPAT_H
#define DM1_V1_CREATURE_AI_BEHAVIOR_PC34_COMPAT_H

/*
 * DM1 V1 Creature AI Behavior System — source-locked to ReDMCSB
 *
 * This module implements the full DM1 V1 creature AI behavioral layer:
 *   - Behavior type dispatch (wander, approach, attack, flee)
 *   - Movement decision (toward party, away, random wander, scent follow)
 *   - Attack decision (melee range check, projectile/spell use)
 *   - Group tactics (formation, multi-creature coordination)
 *   - Event timer integration (C29-C41 event processing)
 *
 * Source-locked to ReDMCSB:
 *   - GROUP.C:  F0175–F0209 (group management, AI behavior dispatch)
 *   - MOVESENS.C: F0262–F0267 (movement sensing, teleporter rotation)
 *   - TIMELINE.C: F0233–F0238 (event queue management)
 *   - DEFS.H:  behavior constants (C0_BEHAVIOR_WANDER..C7_BEHAVIOR_APPROACH),
 *              creature info (CREATURE_INFO struct, attribute masks),
 *              event types (C29..C41), ACTIVE_GROUP struct
 *
 * Conventions:
 *   - Pure functions: NO globals, NO UI, NO IO.
 *   - All randomness through RngState_Compat (F0732).
 *   - ADDITIVE ONLY to existing Phase 16 creature AI module.
 */

#include <stdint.h>
#include "memory_creature_ai_pc34_compat.h"

struct DungeonGroup_Compat;

/* ==========================================================
 *  DM1 V1 Behavior Constants (source: DEFS.H lines 1372-1378)
 *
 *  ReDMCSB GROUP.C F0209 dispatches on these values.
 * ========================================================== */

#define DM1_BEHAVIOR_WANDER   0  /* F0180 GROUP_StartWandering sets this */
#define DM1_BEHAVIOR_USELESS2 2  /* Never used (BUG0_00 in ReDMCSB) */
#define DM1_BEHAVIOR_USELESS3 3  /* Never used */
#define DM1_BEHAVIOR_USELESS4 4  /* F0184 GROUP_RemoveActiveGroup resets >=4 */
#define DM1_BEHAVIOR_FLEE     5  /* F0190: fear triggers flee */
#define DM1_BEHAVIOR_ATTACK   6  /* F0207 IsCreatureAttacking */
#define DM1_BEHAVIOR_APPROACH 7  /* F0209: visible party but not in range */

/* ==========================================================
 *  DM1 V1 Creature Size Constants (source: DEFS.H lines 1612-1614)
 * ========================================================== */

#define DM1_SIZE_QUARTER_SQUARE 0
#define DM1_SIZE_HALF_SQUARE    1
#define DM1_SIZE_FULL_SQUARE    2

/* ==========================================================
 *  DM1 V1 Reaction Event Types (source: DEFS.H lines 948-965)
 *
 *  These are the creature-specific events C29-C41 that F0209
 *  (GROUP_ProcessEvents29to41) handles in GROUP.C.
 * ========================================================== */

#define DM1_EVENT_REACTION_DANGER_ON_SQUARE   29
#define DM1_EVENT_REACTION_HIT_BY_PROJECTILE  30
#define DM1_EVENT_REACTION_PARTY_IS_ADJACENT  31
#define DM1_EVENT_UPDATE_ASPECT_GROUP         32
#define DM1_EVENT_UPDATE_ASPECT_CREATURE_0    33
#define DM1_EVENT_UPDATE_ASPECT_CREATURE_3    36
#define DM1_EVENT_UPDATE_BEHAVIOR_GROUP       37
#define DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0  38
#define DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3  41

/* Negative event types for reaction creation
 * (source: GROUP.C F0209, around line 1900) */
#define DM1_CM1_REACTION_PARTY_IS_ADJACENT   (-1)
#define DM1_CM2_REACTION_HIT_BY_PROJECTILE   (-2)
#define DM1_CM3_REACTION_DANGER_ON_SQUARE    (-3)

/* ==========================================================
 *  Creature Attribute Masks (source: DEFS.H lines 1597-1607)
 * ========================================================== */

#define DM1_ATTR_SIZE_MASK             0x0003
#define DM1_ATTR_SIDE_ATTACK           0x0004
#define DM1_ATTR_PREFER_BACK_ROW       0x0008
#define DM1_ATTR_ATTACK_ANY_CHAMPION   0x0010
#define DM1_ATTR_LEVITATION            0x0020
#define DM1_ATTR_NON_MATERIAL          0x0040
#define DM1_ATTR_DROP_FIXED_POSS       0x0200
#define DM1_ATTR_SEE_INVISIBLE         0x0800
#define DM1_ATTR_NIGHT_VISION          0x1000
#define DM1_ATTR_ARCHENEMY             0x2000

/* ==========================================================
 *  DM1 V1 Creature Info (matches DEFS.H CREATURE_INFO struct)
 *
 *  Source: DEFS.H lines 1575-1594
 * ========================================================== */

struct DM1CreatureInfo_Compat {
    int creatureAspectIndex;
    int attackSoundOrdinal;
    int attributes;          /* 16-bit MASK flags */
    int graphicInfo;
    int movementTicks;       /* 255 = immobile */
    int attackTicks;         /* Minimum ticks between attacks */
    int defense;
    int baseHealth;
    int attack;
    int poisonAttack;
    int dexterity;
    int ranges;              /* sight[3:0], smell[11:8], attack[15:12] */
    int properties;          /* fear[7:4], experience[11:8], wariness[15:12] */
    int resistances;
    int animationTicks;      /* nextBehavior[3:0], nonAttackAspect[7:4], attackAspect[11:8] */
    int woundProbabilities;
    int attackType;
};

/* Range extraction macros (source: DEFS.H lines 1651-1654) */
#define DM1_SIGHT_RANGE(r)   ((r) & 0x000F)
#define DM1_XXX_RANGE(r)     (((r) >> 4) & 0x000F)
#define DM1_SMELL_RANGE(r)   (((r) >> 8) & 0x000F)
#define DM1_ATTACK_RANGE(r)  ((r) >> 12)

/* Property extraction (source: DEFS.H lines 1657-1659) */
#define DM1_FEAR_RESISTANCE(p)  (((p) >> 4) & 0x000F)
#define DM1_WARINESS(p)         ((p) >> 12)

/* AnimationTicks extraction (source: DEFS.H lines 1669-1671) */
#define DM1_NEXT_BEHAVIOR_TICKS(a)     ((a) & 0x000F)
#define DM1_NON_ATTACK_ASPECT_TICKS(a) (((a) >> 4) & 0x000F)
#define DM1_ATTACK_ASPECT_TICKS(a)     (((a) >> 8) & 0x000F)

#define DM1_IMMOBILE 255
#define DM1_IMMUNE_TO_FEAR 15

/* Creature/slot constants used by type-specific attack behavior.
 * Sources: DEFS.H C02_CREATURE_GIGGLER and C00/C01 slot constants. */
#define DM1_CREATURE_TYPE_GIGGLER 2
#define DM1_CREATURE_TYPE_SWAMP_SLIME 1
#define DM1_CREATURE_TYPE_WIZARD_EYE 3
#define DM1_CREATURE_TYPE_PAIN_RAT 4
#define DM1_CREATURE_TYPE_SCREAMER 6
#define DM1_CREATURE_TYPE_ROCKPILE 7
#define DM1_CREATURE_TYPE_STONE_GOLEM 9
#define DM1_CREATURE_TYPE_SKELETON 12
#define DM1_CREATURE_TYPE_VEXIRK 14
#define DM1_CREATURE_TYPE_MAGENTA_WORM 15
#define DM1_CREATURE_TYPE_TROLIN 16
#define DM1_CREATURE_TYPE_ANIMATED_ARMOUR 18
#define DM1_CREATURE_TYPE_MATERIALIZER 19
#define DM1_CREATURE_TYPE_DEMON 22
#define DM1_CREATURE_TYPE_LORD_CHAOS 23
#define DM1_CREATURE_TYPE_RED_DRAGON 24
/* C25 Lord Order / C26 Grey Lord reach ReDMCSB GROUP.C BUG0_13: the original
 * leaves their projectile Thing uninitialized. Firestaff rejects that route
 * rather than inventing a projectile. */
#define DM1_CREATURE_TYPE_LORD_ORDER 25
#define DM1_CREATURE_TYPE_GREY_LORD  26
#define DM1_SLOT_READY_HAND       0
#define DM1_SLOT_ACTION_HAND      1

#define DM1_SINGLE_CENTERED_CREATURE_CELL 0xFF

/* Thing types and object-info bases used by fixed creature possessions.
 * Sources: ReDMCSB DEFS.H thing type constants and DUNGEON.C object-info
 * index tables consumed by GROUP.C F0186. */
#define DM1_DROP_THING_TYPE_WEAPON 5
#define DM1_DROP_THING_TYPE_ARMOUR 6
#define DM1_DROP_THING_TYPE_JUNK   10

#define DM1_DROP_OBJECT_FIRST_WEAPON 23
#define DM1_DROP_OBJECT_FIRST_ARMOUR 69
#define DM1_DROP_OBJECT_FIRST_JUNK   127
#define DM1_DROP_RANDOM_FLAG         0x8000
#define DM1_MAX_FIXED_POSSESSION_DROPS 10

struct DM1FixedPossessionDrop_Compat {
    int thingType;
    int itemType;
    int cell;
    int cursed;
    int sourceOrdinal;
    int sourceHadRandomFlag;
};

/* Special projectile-associated EXPLOSION thing values.
 * Source: ReDMCSB DEFS.H:421-428. */
#define DM1_PROJECTILE_THING_FIREBALL          0xFF80
#define DM1_PROJECTILE_THING_SLIME             0xFF81
#define DM1_PROJECTILE_THING_LIGHTNING_BOLT    0xFF82
#define DM1_PROJECTILE_THING_HARM_NON_MATERIAL 0xFF83
#define DM1_PROJECTILE_THING_OPEN_DOOR         0xFF84
#define DM1_PROJECTILE_THING_POISON_CLOUD      0xFF87
#define DM1_PROJECTILE_THING_POISON_BOLT       0xFF86

/* ==========================================================
 *  DM1 V1 Active Group State (matches DEFS.H ACTIVE_GROUP)
 *
 *  Source: DEFS.H lines 578-587
 * ========================================================== */

struct DM1ActiveGroup_Compat {
    int groupThingIndex;
    int directions;          /* packed 2-bit per creature */
    int cells;               /* packed 2-bit per creature */
    int lastMoveTime;        /* lower 8 bits of game time */
    int delayFleeingFromTarget;
    int targetMapX;
    int targetMapY;
    int priorMapX;
    int priorMapY;
    int homeMapX;
    int homeMapY;
    int aspect[4];           /* per-creature aspect flags */
};

/* Source-shaped result for GROUP.C F0208_GROUP_AddEvent. `mapTime` is the
 * EVENT.Map_Time timestamp and `ticks` is the C.Ticks union member. */
struct DM1GroupAddEventPlan_Compat {
    int valid;
    int eventType;
    uint32_t mapTime;
    uint32_t ticks;
    int promotedAspectEvent;
};

/* ReDMCSB GROUP.C F0208 lines 3145-3157. `eventMapTime` is the caller's
 * prepared EVENT.Map_Time and `requestedTime` is F0179's aspect-update
 * timestamp. F0238 insertion remains caller-owned. */
int F0208_DM1_GROUP_BuildAddEventPlan_Compat(
    int eventType,
    uint32_t eventMapTime,
    uint32_t requestedTime,
    struct DM1GroupAddEventPlan_Compat* out);

/* ReDMCSB GROUP.C F0226 lines 13762-13770: source Manhattan distance used
 * by F0199/F0200/F0209. The caller supplies real loaded-map coordinates. */
int F0226_DM1_GROUP_GetDistanceBetweenSquares_Compat(
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY);

/* ReDMCSB GROUP.C F0227 lines 13772-13808. Tests the unblocked 90-degree
 * view cone only; F0199 still owns the loaded-map path walk. */
int F0227_DM1_GROUP_IsDestinationVisibleFromSource_Compat(
    int direction,
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY);

/* ReDMCSB GROUP.C F0228 lines 13810-13859. Produces the primary direction
 * and the source global's secondary alternative, consuming the original RNG
 * only in its cardinal/diagonal tie branches. */
int F0228_DM1_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource_Compat(
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY,
    struct RngState_Compat* rng,
    int* outPrimaryDirection,
    int* outSecondaryDirection);

/* ReDMCSB GROUP.C F0229 / PROJEXPL.C F0229: resolve the actual G0023
 * target-cell priority row. The target is the group square and the attacker
 * is the party square, exactly as F0177 supplies them in the original. */
int F0229_DM1_GROUP_SetOrderedCellsToAttack_Compat(
    int outOrderedCells[4],
    int targetMapX,
    int targetMapY,
    int attackerMapX,
    int attackerMapY,
    unsigned int cellSource,
    struct RngState_Compat* rng);

/* ReDMCSB MOVESENS.C F0264 lines 11919-11936. A C04 group levitates only
 * when its raw G0243 Attributes has MASK0x0020; C14 projectiles and C15
 * explosions always levitate. Every other Thing type does not. */
int F0264_DM1_MOVE_IsLevitating_Compat(
    int thingType,
    int creatureAttributes);

/* One destination-square snapshot for GROUP.C F0202. M10 owns tile/Thing
 * decoding and supplies these facts; DM1 owns the original branch order. */
struct DM1GroupMovementFacts_Compat {
    int available;
    int inBounds;
    int isWall;
    int isStairs;
    int isOpenPit;
    int isImaginaryPit;
    int isFakeWall;
    int isOpenFakeWall;
    int isImaginaryFakeWall;
    int hasFluxcage;
    int teleporterBlocksCreature;
    int occupiedByParty;
    int doorBlocksCreature;
    int occupiedByGroup;
};

/* ==========================================================
 *  DM1 V1 Group Behavior Context
 *
 *  Input snapshot for behavior dispatch, built from dungeon state.
 *  Mirrors the globals F0209 reads: G0378..G0390, G0381..G0383.
 * ========================================================== */

struct DM1GroupBehaviorContext_Compat {
    int currentGroupMapX;
    int currentGroupMapY;
    int currentGroupDistanceToParty;
    int currentGroupPrimaryDirToParty;   /* 0..3 */
    int currentGroupSecondaryDirToParty; /* 0..3 */

    int partyMapX;
    int partyMapY;
    int partyMapIndex;
    int currentMapIndex;
    int partyChampionCount;
    int partyInvisibilityEventCount;
    int dungeonViewPaletteIndex;
    int (*isViewSquareBlocked)(int mapX, int mapY, void* context);
    void* viewBlockerContext;
    int (*isSmellSquareBlocked)(int mapX, int mapY, void* context);
    void* smellBlockerContext;

    /* Creature info for the group's type */
    struct DM1CreatureInfo_Compat creatureInfo;

    int creatureType;        /* DEFS.H Cxx_CREATURE_* */

    int groupBehavior;       /* DM1_BEHAVIOR_* */
    int creatureCount;       /* Group.Count (0-based: 0 means 1 creature) */
    int creatureSize;        /* DM1_SIZE_* from attributes */
    int isArchenemy;

    /* Movement testing results */
    int groupMovementTestedDirs[4];
    struct DM1GroupMovementFacts_Compat groupMovementFacts[4];
    /* GROUP.C F0204 reads a second loaded destination only after F0202 has
     * admitted the first square.  M10 publishes this separately so a double
     * step cannot be inferred from the first result. */
    struct DM1GroupMovementFacts_Compat archenemySecondStepMovementFacts[4];

    /* Distance to visible party (0 if not visible) */
    int distanceToVisibleParty;
    /* Published by the live F0201 owner after it has read the loaded map and
     * an authenticated G0407 scent receipt.  The pure dispatcher never
     * fabricates this value. */
    int smelledPartyDirectionOrdinal;
    int smelledPartySecondaryDirection;

    /* Freeze life */
    int freezeLifeTicks;

    /* Timing */
    int ticksSinceLastMove;
    int movementTicks;       /* effective movement ticks for this group */
    int currentTickLow;

    /* Event context */
    int eventType;           /* C29..C41 or negative for reactions */
    int eventTicks;          /* P0430_ui_Ticks from the event */

    /* Giggler steal snapshot for F0193-compatible pure resolution. */
    int targetChampionDexterity;
    uint32_t targetChampionOccupiedSlotMask;
    int targetChampionLuckyAttemptMask;
};

/* ==========================================================
 *  DM1 V1 Behavior Dispatch Result
 *
 *  Output of the behavior dispatch — tells caller what the
 *  creature decided to do this tick.
 * ========================================================== */

#define DM1_ACTION_NONE           0
#define DM1_ACTION_MOVE           1
#define DM1_ACTION_ATTACK         2
#define DM1_ACTION_FLEE_MOVE      3
#define DM1_ACTION_SET_DIRECTION  4
#define DM1_ACTION_SKIP_FROZEN    5
#define DM1_ACTION_CAST_SPELL     6
#define DM1_ACTION_STEAL          7
#define DM1_ACTION_ADJUST_CELL    8

struct DM1GigglerStealResult_Compat {
    int objectStolen;
    int stealSlotIndex;
    uint32_t stolenSlotMask;
    int stolenCount;
    int attemptedSlotCount;
    int initialCounter;
    int shouldFlee;
    int fleeDelayTicks;
    int newBehavior;
};

struct DM1CreatureProjectileAttack_Compat {
    int shouldLaunch;
    int projectileThing;
    int targetCell;
    int direction;
    int kineticEnergy;
    int attack;
    int stepEnergy;
    int useSpellSoundFallback;
    int rngCallCount;
};

struct DM1BehaviorResult_Compat {
    int actionKind;          /* DM1_ACTION_* */
    int newBehavior;         /* updated DM1_BEHAVIOR_* */
    int moveDirection;       /* 0..3 or -1 */
    int moveDestMapX;
    int moveDestMapY;
    int attackTargetCell;    /* target cell for attack */
    int attackIsProjectile;  /* 1 if ranged/spell, 0 if melee */
    int newDirectionForGroup;/* direction to face after action */
    int nextEventType;       /* event type to schedule next */
    int nextEventDelayTicks; /* ticks until next event */
    int setDirectionOnly;    /* 1 if just turning, no move */
    int stopAttacking;       /* 1 if group should stop attacking */
    int startWandering;      /* 1 if group should transition to wander */
    int deleteEvents;        /* 1 if existing events should be purged */
    int fearDecrement;       /* amount to decrement fear counter */
    int stealSlotIndex;      /* first stolen slot, or -1 */
    uint32_t stolenSlotMask; /* all slots stolen during this attack */
    int stolenCount;
    int gigglerFleeDelayTicks;
    int gigglerInitialStealCounter;
    int projectileThing;
    int projectileKineticEnergy;
    int projectileAttack;
    int projectileStepEnergy;
    int projectileDirection;
    int projectileUseSpellSoundFallback;
    int meleeCellAdjustment;   /* 1 when F0209 deferred melee to shift cells */
    int updatedGroupCells;     /* activeGroup->cells after source cell update */
    int adjustedCreatureCell;  /* new cell, or -1 when centered */
    int archenemyDoubleMove;   /* GROUP.C F0204 two-square archenemy move */
};

struct DM1BehaviorReactionApplyPlan_Compat {
    int valid;
    int newAiStateKind;
    int groupMapIndex;
    int groupMapX;
    int groupMapY;
    int groupCells;
    int lastSeenPartyMapX;
    int lastSeenPartyMapY;
    int lastSeenPartyTick;
    int groupBehavior;
    int shouldScheduleNextEvent;
    uint32_t nextEventFireAtTick;
    int nextEventMapIndex;
    int nextEventMapX;
    int nextEventMapY;
    int nextEventGroupIndex;
    int nextEventCreatureType;
    int nextEventType;
};

struct DM1BehaviorReactionSchedulePlan_Compat {
    int shouldSchedule;
    uint32_t fireAtTick;
    int mapIndex;
    int mapX;
    int mapY;
    int groupIndex;
    int creatureType;
    int eventType;
};

/* ==========================================================
 *  API — Behavior Type Dispatch
 *
 *  Source-locked to GROUP.C F0209_GROUP_ProcessEvents29to41
 * ========================================================== */

/*
 * F0810: Top-level behavior dispatch (mirrors F0209 logic).
 *
 * Takes group context + active group state + RNG, produces behavior result.
 * This is the main entry point for creature AI each tick.
 *
 * Source: GROUP.C F0209 lines ~1850-2500
 */
int F0810_DM1_GROUP_DispatchBehavior_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    struct DM1ActiveGroup_Compat* activeGroup,
    struct RngState_Compat* rng,
    struct DM1BehaviorResult_Compat* result);

int F0810b_DM1_GROUP_PlanReactionApply_Compat(
    const struct DM1BehaviorResult_Compat* behavior,
    const struct DM1ActiveGroup_Compat* activeGroup,
    int groupIndex,
    int creatureType,
    int eventMapIndex,
    int eventMapX,
    int eventMapY,
    int groupCells,
    int aiStateWander,
    int aiStateAttack,
    int aiStateApproach,
    int aiStateFlee,
    uint32_t currentTick,
    struct DM1BehaviorReactionApplyPlan_Compat* out);

int F0810c_DM1_GROUP_PlanReactionSchedule_Compat(
    const struct DM1BehaviorResult_Compat* behavior,
    int groupIndex,
    int creatureType,
    int mapIndex,
    int mapX,
    int mapY,
    uint32_t currentTick,
    struct DM1BehaviorReactionSchedulePlan_Compat* out);

/* ==========================================================
 *  API — Movement Decision
 *
 *  Source-locked to GROUP.C F0202, F0203, F0204, F0201
 * ========================================================== */

/*
 * F0811: Check if movement in a given direction is possible.
 *
 * Source: GROUP.C F0202_GROUP_IsMovementPossible
 */
int F0811_DM1_GROUP_IsMovementPossible_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int direction,
    int allowImaginaryPitsAndFakeWalls,
    int* outBlockedByWall,
    int* outBlockedByDoor,
    int* outBlockedByParty,
    int* outBlockedByGroup);

/*
 * F0812: Get first possible movement direction ordinal.
 *
 * Source: GROUP.C F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal
 */
int F0812_DM1_GROUP_GetFirstPossibleMovementDir_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int allowImaginaryPitsAndFakeWalls,
    int* outDirection);

/* Live GROUP.C F0203 form. `testedDirections` is the caller-owned G0384
 * state and is marked before each F0202 evaluation, even for a blocker. */
int F0812a_DM1_GROUP_GetFirstPossibleMovementDirWithTestState_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int testedDirections[4],
    int allowImaginaryPitsAndFakeWalls,
    int* outDirection);

/* GROUP.C F0204 tests the second square only after the first F0202 pass.
 * `firstStepHasFluxcage` is the real G0385 result for the already-tested
 * first square; `secondStepFacts` must describe the loaded square beyond it. */
int F0812b_DM1_GROUP_IsArchenemyDoubleMovementPossible_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int direction,
    int firstStepHasFluxcage,
    const struct DM1GroupMovementFacts_Compat* secondStepFacts,
    int* outBlockedByWall,
    int* outBlockedByDoor,
    int* outBlockedByParty,
    int* outBlockedByGroup);

/*
 * F0813: Pick single-square movement direction toward target.
 *
 * Implements the primary/secondary/opposite/random fallback from
 * GROUP.C F0209 labels T0209085_SingleSquareMove
 */
int F0813_DM1_GROUP_PickSingleSquareMove_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int primaryDir,
    int secondaryDir,
    int allowFakeWalls,
    struct RngState_Compat* rng,
    int* outDirection);

/* ==========================================================
 *  API — Attack Decision
 *
 *  Source-locked to GROUP.C F0207, F0177, F0200
 * ========================================================== */

/*
 * F0814: Determine if creature should attack.
 *
 * Checks distance, attack range, row/column alignment, and cooldowns.
 * Source: GROUP.C F0209 attack transition at T0209044_SetBehavior6_Attack
 */
int F0814_DM1_GROUP_ShouldAttack_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int* outShouldAttack);

/*
 * F0815: Check if creature is in melee range of party.
 *
 * Source: GROUP.C F0207 — distance == 1 check + cell alignment
 */
int F0815_DM1_GROUP_IsMeleeRange_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int* outInRange);

/*
 * F0816: Check if creature should use projectile/spell.
 *
 * Source: GROUP.C F0207 — attack range > 1 check + 50% chance
 */
int F0816_DM1_GROUP_ShouldUseProjectile_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    struct RngState_Compat* rng,
    int* outUseProjectile);

/* ==========================================================
 *  API — Group Tactics
 *
 *  Source-locked to GROUP.C F0205, F0206, F0176, F0229
 * ========================================================== */

/*
 * F0817: Set direction for creature(s) in group.
 *
 * Legacy direct-turn helper. It handles half-square pair setting but rejects
 * opposite turns because the original F0205 requires the live RNG variant.
 * Source: GROUP.C F0205_GROUP_SetDirection, F0206_GROUP_SetDirectionGroup
 */
int F0817_DM1_GROUP_SetGroupDirection_Compat(
    struct DM1ActiveGroup_Compat* activeGroup,
    int direction,
    int creatureIndex,
    int creatureSize,
    int creatureCount);

/*
 * F0817a: Live-RNG form of GROUP.C F0205/F0206. It consumes the original
 * random gates:
 * F0206 visits the group from its highest creature index down and gives
 * non-zero indices the M005_RANDOM(2) gate; F0205 consumes M006_RANDOM(65536)
 * only for an opposite-direction turn.  M10 uses this form when it persists
 * ACTIVE_GROUP::Directions between C29-C41 events.
 */
int F0817a_DM1_GROUP_SetGroupDirectionsWithRng_Compat(
    struct DM1ActiveGroup_Compat* activeGroup,
    int direction,
    int creatureSize,
    int creatureCount,
    struct RngState_Compat* rng);

/* F0817b is the single-creature F0205 form used by C38-C41. */
int F0817b_DM1_GROUP_SetCreatureDirectionWithRng_Compat(
    struct DM1ActiveGroup_Compat* activeGroup,
    int direction,
    int creatureIndex,
    int creatureSize,
    int creatureCount,
    struct RngState_Compat* rng);

/* ReDMCSB GROUP.C F0194 calls F0184 for every live active group before a
 * map/runtime handoff. The supplied group array must be decoded original
 * C04 data; malformed active references are rejected before any write. */
int F0817c_DM1_GROUP_RemoveAllActiveGroups_Compat(
    struct DM1ActiveGroup_Compat* activeGroups,
    int activeGroupCapacity,
    int* currentActiveGroupCount,
    struct DungeonGroup_Compat* groups,
    int groupCount);

/* GROUP.C F0197/F0198 consume one already-decoded square.  The caller owns
 * F0157 door lookup and supplies its real C04/door attributes; this layer
 * never fabricates an empty square or a transparent door. */
struct DM1GroupSightSquare_Compat {
    int elementType;
    int doorState;
    int creaturesCanSeeThrough;
    int fakeWallOpen;
    int fakeWallImaginary;
};

typedef int (*DM1GroupSquareBlockedCallback_Compat)(
    int mapX, int mapY, void* context);

int F0817d_DM1_GROUP_IsViewPartyBlocked_Compat(
    const struct DM1GroupSightSquare_Compat* square);
int F0817e_DM1_GROUP_IsSmellPartyBlocked_Compat(
    const struct DM1GroupSightSquare_Compat* square);

/* GROUP.C F0199. The callback must consult the loaded original map at every
 * requested square. It returns the Manhattan distance on a clear source
 * route, or zero when a source-tested square blocks the route. */
int F0817f_DM1_GROUP_GetDistanceBetweenUnblockedSquares_Compat(
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY,
    DM1GroupSquareBlockedCallback_Compat isBlocked,
    void* context);

/*
 * F0818: Get distance to visible party considering sight/LoS.
 *
 * Source: GROUP.C F0200_GROUP_GetDistanceToVisibleParty
 */
int F0818_DM1_GROUP_GetDistanceToVisibleParty_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int creatureIndex,
    int* outDistance);

/* Exact GROUP.C F0200 form. The active group, palette/invisibility state,
 * and F0197-backed loaded-map callback must be supplied by the runtime.
 * Returns the source F0199 distance, or zero when no creature has sight. */
int F0818a_DM1_GROUP_GetDistanceToVisiblePartyWithRoute_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    const struct DM1ActiveGroup_Compat* activeGroup,
    int creatureIndex,
    struct RngState_Compat* rng,
    int* outDistance);

/*
 * F0819: Compute smelled party direction ordinal.
 *
 * Source: GROUP.C F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal
 */
int F0819_DM1_GROUP_GetSmelledPartyDirOrdinal_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int* outDirectionOrdinal);

/*
 * F0819a: Resolve F0201's direct-party scent branch from the already
 * source-walked F0199 smell route. A positive route distance means that
 * F0198/F0199 found no smell-path blocker. This never infers a route from
 * Manhattan distance.
 *
 * Source: ReDMCSB GROUP.C F0201 lines 1430-1437.
 */
int F0819a_DM1_GROUP_GetSmelledPartyDirOrdinalFromRoute_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int smellRouteDistance,
    int* outDirectionOrdinal);

/* F0201's stored-scent fallback. The caller supplies only an already-read
 * party scent entry; this DM1 layer never fabricates a scent trail. */
struct DM1GroupScent_Compat {
    int present;
    int strength;
    int mapX;
    int mapY;
};

struct DM1GroupSmellDirectionPlan_Compat {
    int valid;
    int directionOrdinal;
    int primaryDirection;
    int secondaryDirection;
    int usedDirectPartyRoute;
    int usedStoredScent;
};

/*
 * F0819b: Complete F0201 direction choice after M10 supplies the F0199
 * direct smell-route result and, when available, the current-square scent.
 * Source: ReDMCSB GROUP.C F0201 and PROJEXPL.C F0228.
 */
int F0819b_DM1_GROUP_BuildSmelledPartyDirectionPlan_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int smellRouteDistance,
    const struct DM1GroupScent_Compat* scent,
    struct RngState_Compat* rng,
    struct DM1GroupSmellDirectionPlan_Compat* out);

/* Live GROUP.C F0201 form. Direct party scent consumes F0198/F0199 through
 * the loaded-map callback; only a supplied original party-scent record may
 * serve as the later source fallback. */
int F0819c_DM1_GROUP_BuildSmelledPartyDirectionPlanWithRoute_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    const struct DM1GroupScent_Compat* scent,
    struct RngState_Compat* rng,
    struct DM1GroupSmellDirectionPlan_Compat* out);

/*
 * F0820: Calculate flee direction (opposite of toward-party).
 *
 * Source: GROUP.C F0209 label T0209094_FleeFromTarget
 */
int F0820_DM1_GROUP_GetFleeDirection_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int* outPrimaryDir,
    int* outSecondaryDir);

/*
 * F0821: Determine if a creature death should frighten the group.
 *
 * Source: GROUP.C F0190 fear test in GetDamageCreatureOutcome
 * Uses fear resistance + creature count vs random roll.
 */
int F0821_DM1_GROUP_ShouldFrighten_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int creatureCount,
    struct RngState_Compat* rng,
    int* outShouldFlee,
    int* outFleeDelay);

/*
 * F0822: Resolve Giggler steal/flee attack semantics.
 *
 * Source: GROUP.C F0193_GROUP_StealFromChampion; PC/I34 slot table is
 * DATA.C G0025_auc_Graphic562_StealFromSlotIndices.
 */
int F0822_DM1_GIGGLER_ResolveStealAttempt_Compat(
    int championDexterity,
    uint32_t occupiedSlotMask,
    int luckyAttemptMask,
    struct RngState_Compat* rng,
    struct DM1GigglerStealResult_Compat* out);

/*
 * F0823: Resolve creature projectile attack launch parameters.
 *
 * Source: ReDMCSB GROUP.C F0207 lines 1695-1770 and PROJEXPL.C
 * F0212 lines 43-92. This pure helper returns the exact special thing,
 * source target cell, direction, bounded kinetic energy, dexterity attack,
 * and step energy that the caller must pass to F0212_PROJECTILE_Create.
 */
int F0823_DM1_GROUP_ResolveProjectileAttack_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    const struct DM1ActiveGroup_Compat* activeGroup,
    int creatureIndex,
    struct RngState_Compat* rng,
    struct DM1CreatureProjectileAttack_Compat* out);

/*
 * F0824: Resolve source creature fixed-possession drops.
 *
 * Source: ReDMCSB GROUP.C F0186 lines 580-645 plus DUNGEON.C
 * G0245-G0253 fixed possession tables lines 518-557. This pure helper
 * does not allocate dungeon things; it returns the type/item/cell/cursed
 * payloads the caller must materialize through F0166/F0267.
 */
int F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat(
    int creatureType,
    int sourceCell,
    struct RngState_Compat* rng,
    struct DM1FixedPossessionDrop_Compat* outDrops,
    int maxDrops,
    int* outDropCount,
    int* outWeaponDropped);

#endif /* DM1_V1_CREATURE_AI_BEHAVIOR_PC34_COMPAT_H */
