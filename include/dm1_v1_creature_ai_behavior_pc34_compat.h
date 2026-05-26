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
/* C25 Lord Order / C26 Grey Lord are PROJEXPL/GROUP.C BUG0_13:
 * ReDMCSB has no explicit projectile branch for them. They are not
 * reachable in any original dungeon, but to keep MEDIA529 (PC34) default
 * FIREBALL safe even if a modder places them, dm1_v1_creature_ai_behavior
 * matches them by name through the default fallthrough. */
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

    /* Creature info for the group's type */
    struct DM1CreatureInfo_Compat creatureInfo;

    int creatureType;        /* DEFS.H Cxx_CREATURE_* */

    int groupBehavior;       /* DM1_BEHAVIOR_* */
    int creatureCount;       /* Group.Count (0-based: 0 means 1 creature) */
    int creatureSize;        /* DM1_SIZE_* from attributes */
    int isArchenemy;

    /* Movement testing results */
    int groupMovementTestedDirs[4];

    /* Distance to visible party (0 if not visible) */
    int distanceToVisibleParty;

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
 * Handles half-square creature pair direction setting.
 * Source: GROUP.C F0205_GROUP_SetDirection, F0206_GROUP_SetDirectionGroup
 */
int F0817_DM1_GROUP_SetGroupDirection_Compat(
    struct DM1ActiveGroup_Compat* activeGroup,
    int direction,
    int creatureIndex,
    int creatureSize,
    int creatureCount);

/*
 * F0818: Get distance to visible party considering sight/LoS.
 *
 * Source: GROUP.C F0200_GROUP_GetDistanceToVisibleParty
 */
int F0818_DM1_GROUP_GetDistanceToVisibleParty_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int creatureIndex,
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
