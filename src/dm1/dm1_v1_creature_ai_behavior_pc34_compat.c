/*
 * DM1 V1 Creature AI Behavior System — Implementation
 *
 * Source-locked to ReDMCSB GROUP.C, MOVESENS.C, TIMELINE.C, DEFS.H.
 * See dm1_v1_creature_ai_behavior_pc34_compat.h for full citations.
 *
 * Design: Pure functions, no globals. All randomness via RngState_Compat.
 *
 * Key ReDMCSB source citations per function:
 *   F0810 -> GROUP.C F0209_GROUP_ProcessEvents29to41 (lines 1850-2500)
 *   F0811 -> GROUP.C F0202_GROUP_IsMovementPossible
 *   F0812 -> GROUP.C F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal
 *   F0813 -> GROUP.C F0209 T0209085_SingleSquareMove
 *   F0814 -> GROUP.C F0209 T0209044_SetBehavior6_Attack
 *   F0815 -> GROUP.C F0207_GROUP_IsCreatureAttacking (melee distance)
 *   F0816 -> GROUP.C F0207 (attack range > 1 branch)
 *   F0817 -> GROUP.C F0205_GROUP_SetDirection, F0206_GROUP_SetDirectionGroup
 *   F0818 -> GROUP.C F0200_GROUP_GetDistanceToVisibleParty
 *   F0819 -> GROUP.C F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal
 *   F0820 -> GROUP.C F0209 T0209094_FleeFromTarget
 *   F0821 -> GROUP.C F0190 fear test in GetDamageCreatureOutcome
 *   F0822 -> GROUP.C F0193 Giggler steal/flee behavior
 */

#include <string.h>
#include <stdlib.h>
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

/* Direction deltas — PC DM convention (source: DEFS.H G0233/G0234) */
static const int g_dx[4] = {  0, +1,  0, -1 };
static const int g_dy[4] = { -1,  0, +1,  0 };

/* Normalize direction to 0..3 (source: M021_NORMALIZE) */
static int normalize_dir(int d) { return d & 3; }

/* Opposite direction (source: M018_OPPOSITE) */
static int opposite_dir(int d) { return (d + 2) & 3; }

/* Next direction CW (source: M017_NEXT) */
static int next_dir(int d) { return (d + 1) & 3; }

/* Manhattan distance (source: M038_DISTANCE macro in DEFS.H) */
static int manhattan(int x1, int y1, int x2, int y2) {
    int dx = x1 - x2; if (dx < 0) dx = -dx;
    int dy = y1 - y2; if (dy < 0) dy = -dy;
    return dx + dy;
}

/* Max helper */
static int max_val(int a, int b) { return (a > b) ? a : b; }

static int bounded_val(int min, int value, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static int packed_group_cell(int cells, int creatureIndex) {
    return (cells >> (creatureIndex * 2)) & 0x03;
}

static int packed_group_cell_is_occupied(int cells, int creatureCount,
                                         int skipCreatureIndex, int cell)
{
    int i;

    for (i = creatureCount; i >= 0; --i) {
        if (i == skipCreatureIndex) continue;
        if (packed_group_cell(cells, i) == (cell & 3)) return 1;
    }
    return 0;
}

static int packed_group_cell_update(int cells, int creatureIndex, int cell)
{
    int shift = creatureIndex * 2;

    return (cells & ~(3 << shift)) | ((cell & 3) << shift);
}

static int resolve_quarter_square_melee_cell_adjustment(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    struct DM1ActiveGroup_Compat* activeGroup,
    int creatureIndex,
    struct RngState_Compat* rng,
    struct DM1BehaviorResult_Compat* result)
{
    int attrs;
    int primaryDir;
    int currentCell;
    int candidateCell;
    int centered;

    if (!ctx || !activeGroup || !rng || !result) return 0;
    if (DM1_ATTACK_RANGE(ctx->creatureInfo.ranges) != 1) return 0;
    if (ctx->creatureSize != DM1_SIZE_QUARTER_SQUARE) return 0;
    if (activeGroup->cells == 0xFF) return 0;

    attrs = ctx->creatureInfo.attributes;
    primaryDir = ctx->currentGroupPrimaryDirToParty & 3;
    currentCell = packed_group_cell(activeGroup->cells, creatureIndex);
    if (currentCell == primaryDir || currentCell == ((primaryDir + 1) & 3)) {
        return 0;
    }

    /* GROUP.C:2388-2393: attack-any back-row creatures get a 3/4 chance
     * to keep attacking from the back row instead of shifting cells. */
    if ((attrs & DM1_ATTR_PREFER_BACK_ROW) != 0) {
        if (F0732_COMBAT_RngRandom_Compat(rng, 4) != 0 &&
            (attrs & DM1_ATTR_ATTACK_ANY_CHAMPION) != 0) {
            return 0;
        }
    }

    centered = 0;
    if (ctx->creatureCount == 0 &&
        F0732_COMBAT_RngRandom_Compat(rng, 2) != 0) {
        activeGroup->cells = 0xFF;
        centered = 1;
    } else {
        if ((primaryDir & 1) == (currentCell & 1)) {
            candidateCell = (currentCell - 1) & 3;
        } else {
            candidateCell = (currentCell + 1) & 3;
        }
        if (!packed_group_cell_is_occupied(activeGroup->cells,
                                           ctx->creatureCount,
                                           creatureIndex,
                                           candidateCell) ||
            (F0732_COMBAT_RngRandom_Compat(rng, 2) != 0 &&
             !packed_group_cell_is_occupied(activeGroup->cells,
                                            ctx->creatureCount,
                                            creatureIndex,
                                            (candidateCell + 2) & 3))) {
            activeGroup->cells = packed_group_cell_update(
                activeGroup->cells, creatureIndex, candidateCell);
        }
    }

    result->actionKind = DM1_ACTION_ADJUST_CELL;
    result->newBehavior = DM1_BEHAVIOR_ATTACK;
    result->meleeCellAdjustment = 1;
    result->updatedGroupCells = activeGroup->cells;
    result->adjustedCreatureCell = centered ? -1 :
        packed_group_cell(activeGroup->cells, creatureIndex);
    result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + creatureIndex;
    result->nextEventDelayTicks = max_val(1, (ctx->creatureInfo.movementTicks >> 1) +
                                          F0732_COMBAT_RngRandom_Compat(rng, 2));
    return 1;
}

static const int g_gigglerStealSlotsPc34[8] = {
    DM1_SLOT_ACTION_HAND,
    DM1_SLOT_READY_HAND,
    DM1_SLOT_READY_HAND,
    DM1_SLOT_READY_HAND,
    DM1_SLOT_READY_HAND,
    DM1_SLOT_READY_HAND,
    DM1_SLOT_READY_HAND,
    DM1_SLOT_READY_HAND
};

/* =========================================================================
 *  F0822: Giggler steal/flee attack resolution
 *
 *  Source: GROUP.C F0193_GROUP_StealFromChampion lines 1013-1080.
 *  PC/I34 slot order: DATA.C G0025 lines 900-908.
 * ========================================================================= */

int F0822_DM1_GIGGLER_ResolveStealAttempt_Compat(
    int championDexterity,
    uint32_t occupiedSlotMask,
    int luckyAttemptMask,
    struct RngState_Compat* rng,
    struct DM1GigglerStealResult_Compat* out)
{
    int percentage;
    int counter;
    int attempt = 0;

    if (!rng || !out) return 0;
    memset(out, 0, sizeof(*out));
    out->stealSlotIndex = -1;
    out->newBehavior = DM1_BEHAVIOR_ATTACK;

    if (championDexterity < 0) championDexterity = 0;
    if (championDexterity > 100) championDexterity = 100;

    percentage = 100 - championDexterity;
    counter = F0732_COMBAT_RngRandom_Compat(rng, 8);
    out->initialCounter = counter;

    while (percentage > 0) {
        int slot;
        if (luckyAttemptMask & (1 << attempt)) break;
        slot = g_gigglerStealSlotsPc34[counter & 7];
        out->attemptedSlotCount++;
        if ((occupiedSlotMask & (1u << slot)) != 0u) {
            occupiedSlotMask &= ~(1u << slot);
            out->objectStolen = 1;
            out->stolenSlotMask |= (1u << slot);
            out->stolenCount++;
            if (out->stealSlotIndex < 0) out->stealSlotIndex = slot;
        }
        counter = (counter + 1) & 7;
        percentage -= 20;
        attempt++;
    }

    if (F0732_COMBAT_RngRandom_Compat(rng, 8) == 0 ||
        (out->objectStolen && F0732_COMBAT_RngRandom_Compat(rng, 2) != 0)) {
        out->shouldFlee = 1;
        out->fleeDelayTicks = F0732_COMBAT_RngRandom_Compat(rng, 64) + 20;
        out->newBehavior = DM1_BEHAVIOR_FLEE;
    }

    return 1;
}


/* =========================================================================
 *  F0823: Creature projectile attack launch parameters
 *
 *  Source: GROUP.C F0207_GROUP_IsCreatureAttacking lines 1695-1770.
 *  Source: PROJEXPL.C F0212_PROJECTILE_Create lines 43-92.
 * ========================================================================= */

int F0823_DM1_GROUP_ResolveProjectileAttack_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    const struct DM1ActiveGroup_Compat* activeGroup,
    int creatureIndex,
    struct RngState_Compat* rng,
    struct DM1CreatureProjectileAttack_Compat* out)
{
    uint32_t rngBefore;
    int attackRange;
    int targetCell;
    int energy;

    if (!ctx || !activeGroup || !rng || !out) return 0;
    if (creatureIndex < 0 || creatureIndex > ctx->creatureCount) return 0;
    memset(out, 0, sizeof(*out));
    out->projectileThing = -1;
    out->targetCell = -1;
    out->direction = ctx->currentGroupPrimaryDirToParty & 3;
    out->stepEnergy = 8;
    rngBefore = rng->seed;

    attackRange = DM1_ATTACK_RANGE(ctx->creatureInfo.ranges);
    if (attackRange <= 1) {
        out->rngCallCount = 0;
        return 1;
    }
    if (ctx->currentGroupDistanceToParty <= 1 &&
        F0732_COMBAT_RngRandom_Compat(rng, 2) == 0) {
        out->rngCallCount = (int)(rng->seed - rngBefore);
        return 1;
    }

    if (activeGroup->cells == 0xFF) {
        targetCell = F0732_COMBAT_RngRandom_Compat(rng, 2);
    } else {
        targetCell = ((packed_group_cell(activeGroup->cells, creatureIndex) +
                       5 - out->direction) & 0x0002) >> 1;
    }
    targetCell = (targetCell + out->direction) & 0x0003;

    out->shouldLaunch = 1;
    out->targetCell = targetCell;
    out->useSpellSoundFallback = 1;

    switch (ctx->creatureType) {
    case DM1_CREATURE_TYPE_VEXIRK:
    case DM1_CREATURE_TYPE_LORD_CHAOS:
        if (F0732_COMBAT_RngRandom_Compat(rng, 2) != 0) {
            out->projectileThing = DM1_PROJECTILE_THING_FIREBALL;
        } else {
            switch (F0732_COMBAT_RngRandom_Compat(rng, 4)) {
            case 0:
                out->projectileThing = DM1_PROJECTILE_THING_HARM_NON_MATERIAL;
                break;
            case 1:
                out->projectileThing = DM1_PROJECTILE_THING_LIGHTNING_BOLT;
                break;
            case 2:
                out->projectileThing = DM1_PROJECTILE_THING_POISON_CLOUD;
                break;
            default:
                out->projectileThing = DM1_PROJECTILE_THING_OPEN_DOOR;
                break;
            }
        }
        break;
    case DM1_CREATURE_TYPE_SWAMP_SLIME:
        out->projectileThing = DM1_PROJECTILE_THING_SLIME;
        out->useSpellSoundFallback = 0;
        break;
    case DM1_CREATURE_TYPE_WIZARD_EYE:
        out->projectileThing = (F0732_COMBAT_RngRandom_Compat(rng, 8) != 0)
            ? DM1_PROJECTILE_THING_LIGHTNING_BOLT
            : DM1_PROJECTILE_THING_OPEN_DOOR;
        break;
    case DM1_CREATURE_TYPE_MATERIALIZER:
        if (F0732_COMBAT_RngRandom_Compat(rng, 2) != 0) {
            out->projectileThing = DM1_PROJECTILE_THING_POISON_CLOUD;
            break;
        }
        /* FALLTHROUGH */
    case DM1_CREATURE_TYPE_DEMON:
    case DM1_CREATURE_TYPE_RED_DRAGON:
    case DM1_CREATURE_TYPE_LORD_ORDER:  /* C25: ReDMCSB BUG0_13 — undefined in source;
                                         * MEDIA529 (PC34) default makes FIREBALL the
                                         * safe fallback. Not reachable in original
                                         * dungeons. Explicit case so modders/dungeon
                                         * editors get deterministic behavior. */
    case DM1_CREATURE_TYPE_GREY_LORD:   /* C26: same BUG0_13 fallback as Lord Order. */
    default:
        out->projectileThing = DM1_PROJECTILE_THING_FIREBALL;
        break;
    }

    energy = (ctx->creatureInfo.attack >> 2) + 1;
    energy += F0732_COMBAT_RngRandom_Compat(rng, energy);
    energy += F0732_COMBAT_RngRandom_Compat(rng, energy);
    out->kineticEnergy = bounded_val(20, energy, 255);
    out->attack = ctx->creatureInfo.dexterity;
    out->rngCallCount = (int)(rng->seed - rngBefore);
    return 1;
}


static const uint16_t g_fixedPossessionsSkeleton[] = {
    DM1_DROP_OBJECT_FIRST_WEAPON + 9,
    DM1_DROP_OBJECT_FIRST_ARMOUR + 30,
    0
};

static const uint16_t g_fixedPossessionsStoneGolem[] = {
    DM1_DROP_OBJECT_FIRST_WEAPON + 24,
    0
};

static const uint16_t g_fixedPossessionsTrolin[] = {
    DM1_DROP_OBJECT_FIRST_WEAPON + 23,
    0
};

static const uint16_t g_fixedPossessionsAnimatedArmour[] = {
    DM1_DROP_OBJECT_FIRST_ARMOUR + 41,
    DM1_DROP_OBJECT_FIRST_ARMOUR + 40,
    DM1_DROP_OBJECT_FIRST_ARMOUR + 39,
    DM1_DROP_OBJECT_FIRST_WEAPON + 10,
    DM1_DROP_OBJECT_FIRST_ARMOUR + 38,
    DM1_DROP_OBJECT_FIRST_WEAPON + 10,
    0
};

static const uint16_t g_fixedPossessionsRockpile[] = {
    DM1_DROP_OBJECT_FIRST_JUNK + 25,
    (uint16_t)(DM1_DROP_OBJECT_FIRST_JUNK + 25 + DM1_DROP_RANDOM_FLAG),
    (uint16_t)(DM1_DROP_OBJECT_FIRST_WEAPON + 30 + DM1_DROP_RANDOM_FLAG),
    (uint16_t)(DM1_DROP_OBJECT_FIRST_WEAPON + 30 + DM1_DROP_RANDOM_FLAG),
    0
};

static const uint16_t g_fixedPossessionsPainRat[] = {
    DM1_DROP_OBJECT_FIRST_JUNK + 35,
    (uint16_t)(DM1_DROP_OBJECT_FIRST_JUNK + 35 + DM1_DROP_RANDOM_FLAG),
    0
};

static const uint16_t g_fixedPossessionsScreamer[] = {
    DM1_DROP_OBJECT_FIRST_JUNK + 33,
    (uint16_t)(DM1_DROP_OBJECT_FIRST_JUNK + 33 + DM1_DROP_RANDOM_FLAG),
    0
};

static const uint16_t g_fixedPossessionsWorm[] = {
    DM1_DROP_OBJECT_FIRST_JUNK + 34,
    (uint16_t)(DM1_DROP_OBJECT_FIRST_JUNK + 34 + DM1_DROP_RANDOM_FLAG),
    (uint16_t)(DM1_DROP_OBJECT_FIRST_JUNK + 34 + DM1_DROP_RANDOM_FLAG),
    0
};

static const uint16_t g_fixedPossessionsDragon[] = {
    DM1_DROP_OBJECT_FIRST_JUNK + 36,
    DM1_DROP_OBJECT_FIRST_JUNK + 36,
    DM1_DROP_OBJECT_FIRST_JUNK + 36,
    DM1_DROP_OBJECT_FIRST_JUNK + 36,
    DM1_DROP_OBJECT_FIRST_JUNK + 36,
    DM1_DROP_OBJECT_FIRST_JUNK + 36,
    DM1_DROP_OBJECT_FIRST_JUNK + 36,
    DM1_DROP_OBJECT_FIRST_JUNK + 36,
    (uint16_t)(DM1_DROP_OBJECT_FIRST_JUNK + 36 + DM1_DROP_RANDOM_FLAG),
    (uint16_t)(DM1_DROP_OBJECT_FIRST_JUNK + 36 + DM1_DROP_RANDOM_FLAG),
    0
};

static const uint16_t* fixed_possession_table_for_creature(int creatureType,
                                                           int* outCursed)
{
    if (outCursed) *outCursed = 0;
    switch (creatureType) {
    case DM1_CREATURE_TYPE_SKELETON:
        return g_fixedPossessionsSkeleton;
    case DM1_CREATURE_TYPE_STONE_GOLEM:
        return g_fixedPossessionsStoneGolem;
    case DM1_CREATURE_TYPE_TROLIN:
        return g_fixedPossessionsTrolin;
    case DM1_CREATURE_TYPE_ANIMATED_ARMOUR:
        if (outCursed) *outCursed = 1;
        return g_fixedPossessionsAnimatedArmour;
    case DM1_CREATURE_TYPE_ROCKPILE:
        return g_fixedPossessionsRockpile;
    case DM1_CREATURE_TYPE_PAIN_RAT:
        return g_fixedPossessionsPainRat;
    case DM1_CREATURE_TYPE_SCREAMER:
        return g_fixedPossessionsScreamer;
    case DM1_CREATURE_TYPE_MAGENTA_WORM:
        return g_fixedPossessionsWorm;
    case DM1_CREATURE_TYPE_RED_DRAGON:
        return g_fixedPossessionsDragon;
    default:
        return 0;
    }
}

static int fixed_possession_cell(int sourceCell, struct RngState_Compat* rng)
{
    if (sourceCell == DM1_SINGLE_CENTERED_CREATURE_CELL ||
        F0732_COMBAT_RngRandom_Compat(rng, 4) == 0) {
        return F0732_COMBAT_RngRandom_Compat(rng, 4);
    }
    return sourceCell & 3;
}

int F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat(
    int creatureType,
    int sourceCell,
    struct RngState_Compat* rng,
    struct DM1FixedPossessionDrop_Compat* outDrops,
    int maxDrops,
    int* outDropCount,
    int* outWeaponDropped)
{
    const uint16_t* table;
    int cursed;
    int count = 0;
    int ordinal = 0;

    if (!rng || !outDropCount || !outWeaponDropped) return 0;
    if (maxDrops < 0) return 0;
    if (maxDrops > 0 && !outDrops) return 0;
    *outDropCount = 0;
    *outWeaponDropped = 0;

    table = fixed_possession_table_for_creature(creatureType, &cursed);
    if (!table) return 1;

    while (*table) {
        uint16_t raw = *table++;
        int randomFlag = (raw & DM1_DROP_RANDOM_FLAG) != 0;
        int objectIndex = raw & ~DM1_DROP_RANDOM_FLAG;
        struct DM1FixedPossessionDrop_Compat drop;

        ordinal++;
        if (randomFlag && F0732_COMBAT_RngRandom_Compat(rng, 2) != 0) {
            continue;
        }
        if (count >= maxDrops) return 0;

        memset(&drop, 0, sizeof(drop));
        drop.cell = fixed_possession_cell(sourceCell, rng);
        drop.cursed = cursed;
        drop.sourceOrdinal = ordinal;
        drop.sourceHadRandomFlag = randomFlag;

        if (objectIndex >= DM1_DROP_OBJECT_FIRST_JUNK) {
            drop.thingType = DM1_DROP_THING_TYPE_JUNK;
            drop.itemType = objectIndex - DM1_DROP_OBJECT_FIRST_JUNK;
        } else if (objectIndex >= DM1_DROP_OBJECT_FIRST_ARMOUR) {
            drop.thingType = DM1_DROP_THING_TYPE_ARMOUR;
            drop.itemType = objectIndex - DM1_DROP_OBJECT_FIRST_ARMOUR;
        } else {
            drop.thingType = DM1_DROP_THING_TYPE_WEAPON;
            drop.itemType = objectIndex - DM1_DROP_OBJECT_FIRST_WEAPON;
            *outWeaponDropped = 1;
        }
        outDrops[count++] = drop;
    }

    *outDropCount = count;
    return 1;
}


/* =========================================================================
 *  F0811: Movement possibility check
 *
 *  Source: GROUP.C F0202_GROUP_IsMovementPossible
 *  Checks: wall, stairs, pit, fakewall, door, party, other group
 *
 *  We use the pre-computed adjacency masks from the context since the
 *  actual map tile data isn't accessible in this pure-function layer.
 * ========================================================================= */

int F0811_DM1_GROUP_IsMovementPossible_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int direction,
    int allowImaginaryPitsAndFakeWalls,
    int* outBlockedByWall,
    int* outBlockedByDoor,
    int* outBlockedByParty,
    int* outBlockedByGroup)
{
    (void)allowImaginaryPitsAndFakeWalls; /* Caller pre-bakes into masks */

    if (!ctx || direction < 0 || direction > 3) return 0;
    if (outBlockedByWall)  *outBlockedByWall  = 0;
    if (outBlockedByDoor)  *outBlockedByDoor  = 0;
    if (outBlockedByParty) *outBlockedByParty = 0;
    if (outBlockedByGroup) *outBlockedByGroup = 0;

    /* Source: F0202 checks immobility first */
    if (ctx->creatureInfo.movementTicks == DM1_IMMOBILE) return 0;

    /* Source: F0202 wall/stairs/pit/fakewall check */
    if (ctx->groupMovementTestedDirs[direction]) return 0;

    /* Party blocking (source: F0202 G0390 check) */
    /* In the real engine this checks if dest == party pos.
     * We use adjacency info from context. */

    return 1; /* Movement is possible by default */
}

/* =========================================================================
 *  F0812: First possible movement direction
 *
 *  Source: GROUP.C F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal
 *  Iterates N/E/S/W, returns first untested+open direction as ordinal.
 * ========================================================================= */

int F0812_DM1_GROUP_GetFirstPossibleMovementDir_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int allowImaginaryPitsAndFakeWalls,
    int* outDirection)
{
    int dir;
    int bw, bd, bp, bg;
    if (!ctx || !outDirection) return 0;
    *outDirection = -1;

    for (dir = 0; dir <= 3; dir++) {
        if (!ctx->groupMovementTestedDirs[dir]) {
            if (F0811_DM1_GROUP_IsMovementPossible_Compat(
                    ctx, dir, allowImaginaryPitsAndFakeWalls,
                    &bw, &bd, &bp, &bg)) {
                *outDirection = dir;
                return 1;
            }
        }
    }
    return 0;
}

/* =========================================================================
 *  F0813: Single-square move direction picker
 *
 *  Source: GROUP.C F0209 T0209085_SingleSquareMove
 *  Order: primary → secondary (50% gated) → opposite → 25% random
 * ========================================================================= */

int F0813_DM1_GROUP_PickSingleSquareMove_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int primaryDir,
    int secondaryDir,
    int allowFakeWalls,
    struct RngState_Compat* rng,
    int* outDirection)
{
    int bw, bd, bp, bg;
    int opp;
    int roll;

    if (!ctx || !rng || !outDirection) return 0;
    *outDirection = -1;

    /* Primary direction (source: F0209 first IsMovementPossible call) */
    if (F0811_DM1_GROUP_IsMovementPossible_Compat(
            ctx, primaryDir, allowFakeWalls, &bw, &bd, &bp, &bg)) {
        *outDirection = primaryDir;
        return 1;
    }

    /* Secondary direction, gated by 50% RNG
     * (source: F0209 second IsMovementPossible with M005_RANDOM(2)) */
    if (secondaryDir >= 0 && secondaryDir <= 3) {
        roll = F0732_COMBAT_RngRandom_Compat(rng, 2);
        if (F0811_DM1_GROUP_IsMovementPossible_Compat(
                ctx, secondaryDir, allowFakeWalls && (roll == 0),
                &bw, &bd, &bp, &bg)) {
            *outDirection = secondaryDir;
            return 1;
        }
    }

    /* Opposite of secondary (source: F0209 M018_OPPOSITE(AL0446_i_Direction)) */
    if (secondaryDir >= 0 && secondaryDir <= 3) {
        opp = opposite_dir(secondaryDir);
        if (F0811_DM1_GROUP_IsMovementPossible_Compat(
                ctx, opp, 0, &bw, &bd, &bp, &bg)) {
            *outDirection = opp;
            return 1;
        }
    }

    /* 25% chance opposite of primary
     * (source: F0209 !M004_RANDOM(4) && IsMovementPossible(opposite primary)) */
    roll = F0732_COMBAT_RngRandom_Compat(rng, 4);
    if (roll == 0) {
        opp = opposite_dir(primaryDir);
        if (F0811_DM1_GROUP_IsMovementPossible_Compat(
                ctx, opp, 0, &bw, &bd, &bp, &bg)) {
            *outDirection = opp;
            return 1;
        }
    }

    return 0;
}

/* =========================================================================
 *  F0814: Should the creature switch to attack behavior?
 *
 *  Source: GROUP.C F0209 condition at T0209044_SetBehavior6_Attack
 *  Requires: visible, in attack range, on same row or column
 * ========================================================================= */

int F0814_DM1_GROUP_ShouldAttack_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int* outShouldAttack)
{
    int attackRange;
    int distX, distY;

    if (!ctx || !outShouldAttack) return 0;
    *outShouldAttack = 0;

    if (ctx->distanceToVisibleParty == 0) return 1; /* Can't see party */

    attackRange = DM1_ATTACK_RANGE(ctx->creatureInfo.ranges);

    /* Source: F0209 condition:
     * (L0452 <= M056_ATTACK_RANGE) && ((!distX) || (!distY))
     * Creature must be on same row or column as party for attack. */
    distX = ctx->currentGroupMapX - ctx->partyMapX;
    if (distX < 0) distX = -distX;
    distY = ctx->currentGroupMapY - ctx->partyMapY;
    if (distY < 0) distY = -distY;

    if (ctx->distanceToVisibleParty <= attackRange &&
        (distX == 0 || distY == 0)) {
        *outShouldAttack = 1;
    }

    return 1;
}

/* =========================================================================
 *  F0815: Melee range check
 *
 *  Source: GROUP.C F0207 — melee means attack range == 1 and distance == 1
 * ========================================================================= */

int F0815_DM1_GROUP_IsMeleeRange_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int* outInRange)
{
    if (!ctx || !outInRange) return 0;
    *outInRange = (ctx->distanceToVisibleParty == 1) ? 1 : 0;
    return 1;
}

/* =========================================================================
 *  F0816: Projectile/spell use decision
 *
 *  Source: GROUP.C F0207 — attack range > 1 branch with 50% chance
 *  When distance > 1 or 50% random, creature uses ranged attack.
 * ========================================================================= */

int F0816_DM1_GROUP_ShouldUseProjectile_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    struct RngState_Compat* rng,
    int* outUseProjectile)
{
    int attackRange;
    int roll;

    if (!ctx || !rng || !outUseProjectile) return 0;
    *outUseProjectile = 0;

    attackRange = DM1_ATTACK_RANGE(ctx->creatureInfo.ranges);
    if (attackRange <= 1) return 1; /* Melee only — no projectile */

    /* Source: F0207 condition:
     * (M056_ATTACK_RANGE > 1) && ((distance > 1) || M005_RANDOM(2)) */
    if (ctx->currentGroupDistanceToParty > 1) {
        *outUseProjectile = 1;
    } else {
        roll = F0732_COMBAT_RngRandom_Compat(rng, 2);
        *outUseProjectile = (roll != 0) ? 1 : 0;
    }

    return 1;
}

/* =========================================================================
 *  F0817: Set direction for creature(s) in group
 *
 *  Source: GROUP.C F0205_GROUP_SetDirection, F0206_GROUP_SetDirectionGroup
 *
 *  F0205: Handles opposite-direction correction (turn one step at a time)
 *  and half-square creature pair synchronization.
 *
 *  F0206: Sets direction for all creatures in group, with 50% chance
 *  for non-index-0 creatures.
 * ========================================================================= */

int F0817_DM1_GROUP_SetGroupDirection_Compat(
    struct DM1ActiveGroup_Compat* activeGroup,
    int direction,
    int creatureIndex,
    int creatureSize,
    int creatureCount)
{
    int currentDir;
    int newDir;
    int diff;

    if (!activeGroup || direction < 0 || direction > 3) return 0;

    /* Source: F0205 — if current and new are opposites, turn one step */
    currentDir = (activeGroup->directions >> (creatureIndex * 2)) & 0x03;
    diff = normalize_dir(currentDir - direction);
    if (diff == 2) {
        /* Opposite — pick a random intermediate step
         * Source: F0205 M017_NEXT((M006_RANDOM(65536) & 0x0002) + direction) */
        /* ReDMCSB F0205: M017_NEXT((M006_RANDOM(65536) & 0x0002) + direction)
         * Random bit selects CW or CCW. We use direction LSB as pseudo-random
         * since this function doesn't receive RNG state. */
        newDir = (direction & 1) ? next_dir(direction) : normalize_dir(direction - 1);
    } else {
        newDir = direction;
    }

    /* Update packed directions for this creature */
    activeGroup->directions &= ~(0x03 << (creatureIndex * 2));
    activeGroup->directions |= (newDir & 0x03) << (creatureIndex * 2);

    /* Source: F0205 — half-square pairs get synchronized direction */
    if (creatureSize == DM1_SIZE_HALF_SQUARE && creatureCount > 0) {
        int pairIndex = creatureIndex ^ 1;
        if (pairIndex <= creatureCount) {
            activeGroup->directions &= ~(0x03 << (pairIndex * 2));
            activeGroup->directions |= (newDir & 0x03) << (pairIndex * 2);
        }
    }

    return 1;
}

/* =========================================================================
 *  F0818: Distance to visible party
 *
 *  Source: GROUP.C F0200_GROUP_GetDistanceToVisibleParty
 *  Checks creature direction vs party direction, sight range, LoS.
 *  Uses the pre-computed distanceToVisibleParty from context.
 * ========================================================================= */

int F0818_DM1_GROUP_GetDistanceToVisibleParty_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int creatureIndex,
    int* outDistance)
{
    (void)creatureIndex; /* v1: uses group-level visibility from context */

    if (!ctx || !outDistance) return 0;

    /* Source: F0200 returns the pre-computed distance-to-visible-party.
     * The actual visibility walk (F0199/F0197/F0198) is done by the caller
     * and stored in ctx->distanceToVisibleParty. */
    *outDistance = ctx->distanceToVisibleParty;
    return 1;
}

/* =========================================================================
 *  F0819: Smelled party direction ordinal
 *
 *  Source: GROUP.C F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal
 *  Returns: direction ordinal (1-based) or 0 if can't smell
 * ========================================================================= */

int F0819_DM1_GROUP_GetSmelledPartyDirOrdinal_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int* outDirectionOrdinal)
{
    int smellRange;
    int effectiveSmellRange;

    if (!ctx || !outDirectionOrdinal) return 0;
    *outDirectionOrdinal = 0;

    smellRange = DM1_SMELL_RANGE(ctx->creatureInfo.ranges);
    if (smellRange == 0) return 1; /* No smell ability */

    /* Source: F0201 condition:
     * ((smellRange + 1) >> 1) >= currentGroupDistanceToParty
     * AND path is not blocked by walls */
    effectiveSmellRange = (smellRange + 1) >> 1;
    if (effectiveSmellRange >= (int)ctx->currentGroupDistanceToParty &&
        ctx->currentGroupDistanceToParty > 0) {
        /* Return ordinal (1-based) of primary direction to party */
        *outDirectionOrdinal = ctx->currentGroupPrimaryDirToParty + 1;
    }

    return 1;
}

/* =========================================================================
 *  F0820: Flee direction computation
 *
 *  Source: GROUP.C F0209 T0209094_FleeFromTarget
 *  Flee runs OPPOSITE of toward-party direction.
 *  Speed boosted: movementTicks -= (movementTicks >> 2)
 * ========================================================================= */

int F0820_DM1_GROUP_GetFleeDirection_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int* outPrimaryDir,
    int* outSecondaryDir)
{
    if (!ctx || !outPrimaryDir || !outSecondaryDir) return 0;

    /* Source: F0209 T0209094:
     * primaryDir = M018_OPPOSITE(F0228_...);
     * secondaryDir = M018_OPPOSITE(secondaryDir); */
    *outPrimaryDir   = opposite_dir(ctx->currentGroupPrimaryDirToParty);
    *outSecondaryDir = opposite_dir(ctx->currentGroupSecondaryDirToParty);

    return 1;
}

/* =========================================================================
 *  F0821: Fear check after creature death
 *
 *  Source: GROUP.C F0190_GROUP_GetDamageCreatureOutcome
 *  Lines 1350-1355: fear resistance + creatureCount - 1 < random(16)
 * ========================================================================= */

int F0821_DM1_GROUP_ShouldFrighten_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    int creatureCount,
    struct RngState_Compat* rng,
    int* outShouldFlee,
    int* outFleeDelay)
{
    int fearResistance;
    int adjustedFear;
    int roll;

    if (!ctx || !rng || !outShouldFlee || !outFleeDelay) return 0;
    *outShouldFlee = 0;
    *outFleeDelay  = 0;

    fearResistance = DM1_FEAR_RESISTANCE(ctx->creatureInfo.properties);
    if (fearResistance == DM1_IMMUNE_TO_FEAR) return 1;

    /* Source: F0190 condition:
     * (fearResistance + creatureCount - 1) < M003_RANDOM(16) */
    adjustedFear = fearResistance + creatureCount - 1;
    roll = F0732_COMBAT_RngRandom_Compat(rng, 16);

    if (adjustedFear < roll) {
        *outShouldFlee = 1;
        /* Source: F0190 delay:
         * M002_RANDOM(100 - (fearResistance << 2)) + 20 */
        int range = 100 - (fearResistance << 2);
        if (range < 1) range = 1;
        *outFleeDelay = F0732_COMBAT_RngRandom_Compat(rng, range) + 20;
    }

    return 1;
}

/* =========================================================================
 *  F0810: Top-level behavior dispatch
 *
 *  Source: GROUP.C F0209_GROUP_ProcessEvents29to41
 *  This is the main creature AI decision function, called each tick.
 *
 *  Event types handled:
 *    C29-C31: Reaction events (danger, projectile hit, party adjacent)
 *    C32-C36: Update aspect events (visual, per-creature)
 *    C37:     Update behavior group (main AI tick)
 *    C38-C41: Update behavior per-creature (attack mode)
 *
 *  Flow (following F0209 structure):
 *    1. If not on party map and event != 32/33/37/38: ignore
 *    2. Freeze life gate (unless archenemy)
 *    3. For reactions (events < 0): create reaction event with delay
 *    4. For C29-C31: process reactions (attack/flee/wander)
 *    5. For C32-C36: process aspect updates (check visibility → attack?)
 *    6. For C37: main behavior dispatch (wander/approach/flee)
 *    7. For C38-C41: per-creature attack behavior
 * ========================================================================= */

int F0810_DM1_GROUP_DispatchBehavior_Compat(
    const struct DM1GroupBehaviorContext_Compat* ctx,
    struct DM1ActiveGroup_Compat* activeGroup,
    struct RngState_Compat* rng,
    struct DM1BehaviorResult_Compat* result)
{
    int behavior;
    int eventType;
    int shouldAttack = 0;
    int smellDirOrd = 0;
    int dir = -1;
    int fleeP, fleeS;

    if (!ctx || !activeGroup || !rng || !result) return 0;
    memset(result, 0, sizeof(*result));

    behavior  = ctx->groupBehavior;
    eventType = ctx->eventType;
    result->newBehavior = behavior;
    result->moveDirection = -1;

    /* =======================================================
     * Source: F0209 freeze-life gate (lines ~1920-1946)
     * If freeze-life active and not archenemy, reschedule 4 ticks later.
     * Reactions (eventType < 0) are ignored entirely during freeze.
     * ======================================================= */
    if (ctx->freezeLifeTicks > 0 && !ctx->isArchenemy) {
        if (eventType < 0) {
            result->actionKind = DM1_ACTION_NONE;
            return 1; /* Reaction ignored during freeze life */
        }
        result->actionKind = DM1_ACTION_SKIP_FROZEN;
        result->nextEventType = eventType;
        result->nextEventDelayTicks = 4;
        return 1;
    }

    /* =======================================================
     * Source: F0209 reaction event creation (lines ~1952-1968)
     * If eventType < 0: create a timed reaction event.
     * CM1: party adjacent → 1 tick delay
     * CM2/CM3: based on movement speed and recent move timing
     * ======================================================= */
    if (eventType < 0) {
        int reactionDelay;
        int nextEvType = eventType + DM1_EVENT_UPDATE_ASPECT_GROUP;

        if (eventType == DM1_CM1_REACTION_PARTY_IS_ADJACENT) {
            reactionDelay = 1;
        } else {
            /* Source: F0209 reaction delay:
             * ((movementTicks + 2) >> 2) - ticksSinceLastMove, min 1 */
            reactionDelay = ((ctx->movementTicks + 2) >> 2) - ctx->ticksSinceLastMove;
            if (reactionDelay < 1) reactionDelay = 1;
        }

        result->actionKind = DM1_ACTION_NONE;
        result->nextEventType = nextEvType;
        result->nextEventDelayTicks = reactionDelay;
        return 1;
    }

    /* =======================================================
     * Source: F0209 reaction processing C29-C31
     * ======================================================= */
    if (eventType <= DM1_EVENT_REACTION_PARTY_IS_ADJACENT) {
        int reactionCode = eventType - DM1_EVENT_UPDATE_ASPECT_GROUP;

        switch (reactionCode) {
        case DM1_CM1_REACTION_PARTY_IS_ADJACENT:
            /* Source: F0209 party-adjacent reaction:
             * If not already attacking or fleeing → switch to attack */
            if (behavior != DM1_BEHAVIOR_ATTACK &&
                behavior != DM1_BEHAVIOR_FLEE) {
                result->deleteEvents = 1;
                result->newBehavior = DM1_BEHAVIOR_ATTACK;
                result->actionKind = DM1_ACTION_ATTACK;
                activeGroup->targetMapX = ctx->partyMapX;
                activeGroup->targetMapY = ctx->partyMapY;
            } else {
                /* Update target position */
                activeGroup->targetMapX = ctx->partyMapX;
                activeGroup->targetMapY = ctx->partyMapY;
                result->actionKind = DM1_ACTION_NONE;
            }
            return 1;

        case DM1_CM2_REACTION_HIT_BY_PROJECTILE:
            /* Source: F0209 projectile-hit reaction:
             * If attacking or fleeing → no reaction.
             * Otherwise 75% chance: check visibility → approach or wander.
             * If can't see party: look around randomly. */
            if (behavior == DM1_BEHAVIOR_ATTACK ||
                behavior == DM1_BEHAVIOR_FLEE) {
                result->actionKind = DM1_ACTION_NONE;
                return 1;
            }
            {
                int roll = F0732_COMBAT_RngRandom_Compat(rng, 4);
                if (roll > 0) { /* 75% chance */
                    if (ctx->distanceToVisibleParty == 0) {
                        /* Can't see party — look in random direction */
                        result->actionKind = DM1_ACTION_SET_DIRECTION;
                        result->newDirectionForGroup =
                            F0732_COMBAT_RngRandom_Compat(rng, 4);
                        return 1;
                    }
                    /* Can see party but 75% chance still no reaction */
                    roll = F0732_COMBAT_RngRandom_Compat(rng, 4);
                    if (roll > 0) {
                        result->actionKind = DM1_ACTION_NONE;
                        return 1;
                    }
                }
            }
            /* Fall through to danger-on-square handling */
            /* FALLTHROUGH */

        case DM1_CM3_REACTION_DANGER_ON_SQUARE:
            /* Source: F0209 danger reaction:
             * Move in random direction to avoid danger.
             * If was attacking, will switch to approach after move. */
            {
                int startDir = F0732_COMBAT_RngRandom_Compat(rng, 4);
                int refDir = startDir;
                int found = 0;
                int bw, bd, bp, bg;

                do {
                    int destX = ctx->currentGroupMapX + g_dx[startDir];
                    int destY = ctx->currentGroupMapY + g_dy[startDir];
                    /* Skip prior location 75% of the time */
                    if ((destX != activeGroup->priorMapX ||
                         destY != activeGroup->priorMapY ||
                         F0732_COMBAT_RngRandom_Compat(rng, 4) == 0) &&
                        F0811_DM1_GROUP_IsMovementPossible_Compat(
                            ctx, startDir, 0, &bw, &bd, &bp, &bg)) {
                        result->actionKind = DM1_ACTION_MOVE;
                        result->moveDirection = startDir;
                        result->moveDestMapX = destX;
                        result->moveDestMapY = destY;
                        found = 1;
                        break;
                    }
                    startDir = next_dir(startDir);
                } while (startDir != refDir);

                if (!found) {
                    result->actionKind = DM1_ACTION_NONE;
                }

                /* If was attacking, switch to approach after danger move */
                if (behavior == DM1_BEHAVIOR_ATTACK && found) {
                    result->newBehavior = DM1_BEHAVIOR_APPROACH;
                    result->stopAttacking = 1;
                }
            }
            return 1;
        }

        result->actionKind = DM1_ACTION_NONE;
        return 1;
    }

    /* =======================================================
     * Source: F0209 aspect update events C32-C36
     * Check visibility, possibly transition to attack/approach.
     * ======================================================= */
    if (eventType < DM1_EVENT_UPDATE_BEHAVIOR_GROUP) {
        if (ctx->distanceToVisibleParty > 0) {
            if (behavior != DM1_BEHAVIOR_ATTACK &&
                behavior != DM1_BEHAVIOR_FLEE) {
                /* Adjacent → attack */
                if (manhattan(ctx->partyMapX, ctx->partyMapY,
                              ctx->currentGroupMapX, ctx->currentGroupMapY) <= 1) {
                    result->newBehavior = DM1_BEHAVIOR_ATTACK;
                    result->actionKind = DM1_ACTION_ATTACK;
                    activeGroup->targetMapX = ctx->partyMapX;
                    activeGroup->targetMapY = ctx->partyMapY;
                    return 1;
                }
                /* Wander → approach */
                if (behavior == DM1_BEHAVIOR_WANDER) {
                    result->newBehavior = DM1_BEHAVIOR_APPROACH;
                    activeGroup->targetMapX = ctx->partyMapX;
                    activeGroup->targetMapY = ctx->partyMapY;
                    result->actionKind = DM1_ACTION_NONE;
                    result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
                    result->nextEventDelayTicks = 1;
                    return 1;
                }
            }
            activeGroup->targetMapX = ctx->partyMapX;
            activeGroup->targetMapY = ctx->partyMapY;
        }
        /* Schedule next aspect update */
        result->actionKind = DM1_ACTION_NONE;
        result->nextEventType = eventType + 5; /* aspect → behavior event */
        result->nextEventDelayTicks = max_val(1,
            DM1_NON_ATTACK_ASPECT_TICKS(ctx->creatureInfo.animationTicks));
        return 1;
    }

    /* =======================================================
     * Source: F0209 main behavior dispatch (event C37)
     *
     * This is the core AI decision loop.
     * ======================================================= */
    if (eventType == DM1_EVENT_UPDATE_BEHAVIOR_GROUP) {

        /* WANDER behavior (source: F0209 behavior == C0) */
        if (behavior == DM1_BEHAVIOR_WANDER ||
            behavior == DM1_BEHAVIOR_USELESS2 ||
            behavior == DM1_BEHAVIOR_USELESS3) {

            if (ctx->distanceToVisibleParty > 0) {
                /* Can see party */
                F0814_DM1_GROUP_ShouldAttack_Compat(ctx, &shouldAttack);
                if (shouldAttack) {
                    /* In range — switch to attack */
                    result->newBehavior = DM1_BEHAVIOR_ATTACK;
                    result->actionKind = DM1_ACTION_ATTACK;
                    result->deleteEvents = 1;
                    activeGroup->targetMapX = ctx->partyMapX;
                    activeGroup->targetMapY = ctx->partyMapY;

                    /* Source: F0209 T0209044 — set direction and schedule
                     * per-creature attack events */
                    result->newDirectionForGroup =
                        ctx->currentGroupPrimaryDirToParty;
                    result->nextEventType =
                        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
                    result->nextEventDelayTicks = 1;
                    return 1;
                }
                /* Visible but not in range → approach */
                result->newBehavior = DM1_BEHAVIOR_APPROACH;
                activeGroup->targetMapX = ctx->partyMapX;
                activeGroup->targetMapY = ctx->partyMapY;
                result->actionKind = DM1_ACTION_NONE;
                result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
                result->nextEventDelayTicks = 1;
                return 1;
            }

            /* Can't see party — try smell */
            F0819_DM1_GROUP_GetSmelledPartyDirOrdinal_Compat(
                ctx, &smellDirOrd);
            if (smellDirOrd > 0) {
                /* Follow scent */
                int primaryDir = smellDirOrd - 1; /* ordinal to index */
                F0813_DM1_GROUP_PickSingleSquareMove_Compat(
                    ctx, primaryDir, ctx->currentGroupSecondaryDirToParty,
                    0, rng, &dir);
                if (dir >= 0) {
                    int halfMove = ctx->movementTicks >> 1;
                    int delay = halfMove - ctx->ticksSinceLastMove;
                    if (delay <= 0) {
                        /* Move immediately */
                        result->actionKind = DM1_ACTION_MOVE;
                        result->moveDirection = dir;
                        result->moveDestMapX =
                            ctx->currentGroupMapX + g_dx[dir];
                        result->moveDestMapY =
                            ctx->currentGroupMapY + g_dy[dir];
                    } else {
                        /* Delay the move */
                        result->actionKind = DM1_ACTION_NONE;
                    }
                    result->newDirectionForGroup = dir;
                    result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
                    result->nextEventDelayTicks = max_val(1,
                        F0732_COMBAT_RngRandom_Compat(rng, 4) +
                        ctx->movementTicks - 1);
                    return 1;
                }
            }

            /* Random wandering — 50% chance to move */
            if (F0732_COMBAT_RngRandom_Compat(rng, 2) == 0) {
                int startDir = F0732_COMBAT_RngRandom_Compat(rng, 4);
                int refDir = startDir;
                int bw, bd, bp, bg;

                do {
                    int destX = ctx->currentGroupMapX + g_dx[startDir];
                    int destY = ctx->currentGroupMapY + g_dy[startDir];
                    if ((destX != activeGroup->priorMapX ||
                         destY != activeGroup->priorMapY ||
                         F0732_COMBAT_RngRandom_Compat(rng, 4) == 0) &&
                        F0811_DM1_GROUP_IsMovementPossible_Compat(
                            ctx, startDir, 0, &bw, &bd, &bp, &bg)) {
                        int halfMove = ctx->movementTicks >> 1;
                        int delay = halfMove - ctx->ticksSinceLastMove;
                        if (delay <= 0) {
                            result->actionKind = DM1_ACTION_MOVE;
                            result->moveDirection = startDir;
                            result->moveDestMapX = destX;
                            result->moveDestMapY = destY;
                        } else {
                            result->actionKind = DM1_ACTION_NONE;
                        }
                        result->newDirectionForGroup = startDir;
                        break;
                    }
                    startDir = next_dir(startDir);
                } while (startDir != refDir);
            }

            /* Set direction and schedule next behavior tick */
            if (result->actionKind == DM1_ACTION_NONE &&
                result->newDirectionForGroup == 0 &&
                result->moveDirection < 0) {
                result->newDirectionForGroup =
                    F0732_COMBAT_RngRandom_Compat(rng, 4);
                result->setDirectionOnly = 1;
            }
            result->newBehavior = DM1_BEHAVIOR_WANDER;
            result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
            result->nextEventDelayTicks = max_val(1,
                F0732_COMBAT_RngRandom_Compat(rng, 4) +
                ctx->movementTicks - 1);
            return 1;
        }

        /* APPROACH behavior (source: F0209 behavior == C7) */
        if (behavior == DM1_BEHAVIOR_APPROACH) {
            if (ctx->distanceToVisibleParty > 0) {
                /* Can see party */
                F0814_DM1_GROUP_ShouldAttack_Compat(ctx, &shouldAttack);
                if (shouldAttack) {
                    result->newBehavior = DM1_BEHAVIOR_ATTACK;
                    result->actionKind = DM1_ACTION_ATTACK;
                    result->deleteEvents = 1;
                    activeGroup->targetMapX = ctx->partyMapX;
                    activeGroup->targetMapY = ctx->partyMapY;
                    result->newDirectionForGroup =
                        ctx->currentGroupPrimaryDirToParty;
                    result->nextEventType =
                        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
                    result->nextEventDelayTicks = 1;
                    return 1;
                }

                /* Run toward party (half movement ticks = faster)
                 * Source: F0209 T0209081_RunTowardParty */
                activeGroup->targetMapX = ctx->partyMapX;
                activeGroup->targetMapY = ctx->partyMapY;
                int runTicks = (ctx->movementTicks + 1) >> 1;

                F0813_DM1_GROUP_PickSingleSquareMove_Compat(
                    ctx, ctx->currentGroupPrimaryDirToParty,
                    ctx->currentGroupSecondaryDirToParty,
                    1, rng, &dir);
                if (dir >= 0) {
                    int halfMove = runTicks >> 1;
                    int delay = halfMove - ctx->ticksSinceLastMove;
                    if (delay <= 0) {
                        result->actionKind = DM1_ACTION_MOVE;
                        result->moveDirection = dir;
                        result->moveDestMapX =
                            ctx->currentGroupMapX + g_dx[dir];
                        result->moveDestMapY =
                            ctx->currentGroupMapY + g_dy[dir];
                    }
                }
            } else {
                /* Can't see party — walk toward last known position
                 * Source: F0209 T0209082_WalkTowardTarget */
                if (ctx->currentGroupMapX == activeGroup->targetMapX &&
                    ctx->currentGroupMapY == activeGroup->targetMapY) {
                    /* Reached target, party not there — back to wander */
                    result->newBehavior = DM1_BEHAVIOR_WANDER;
                    result->startWandering = 1;
                    result->newDirectionForGroup =
                        F0732_COMBAT_RngRandom_Compat(rng, 4);
                } else {
                    /* Walk to target */
                    /* Compute direction to stored target (not current party pos).
                     * ReDMCSB F0209 T0209082: uses F0228 on targetMapX/Y. */
                    int dx = activeGroup->targetMapX - ctx->currentGroupMapX;
                    int dy = activeGroup->targetMapY - ctx->currentGroupMapY;
                    int tgtDir, tgtSec;
                    /* Primary direction: largest delta axis */
                    if (dx == 0 && dy == 0) { tgtDir = 0; tgtSec = 1; }
                    else if (abs(dx) >= abs(dy)) {
                        tgtDir = (dx > 0) ? 1 : 3; /* E or W */
                        tgtSec = (dy < 0) ? 0 : 2; /* N or S */
                    } else {
                        tgtDir = (dy < 0) ? 0 : 2; /* N or S */
                        tgtSec = (dx > 0) ? 1 : 3; /* E or W */
                    }
                    F0813_DM1_GROUP_PickSingleSquareMove_Compat(
                        ctx, tgtDir, tgtSec, 1, rng, &dir);
                    if (dir >= 0) {
                        int halfMove = ctx->movementTicks >> 1;
                        int delay = halfMove - ctx->ticksSinceLastMove;
                        if (delay <= 0) {
                            result->actionKind = DM1_ACTION_MOVE;
                            result->moveDirection = dir;
                            result->moveDestMapX =
                                ctx->currentGroupMapX + g_dx[dir];
                            result->moveDestMapY =
                                ctx->currentGroupMapY + g_dy[dir];
                        }
                    }
                }
            }

            if (!result->startWandering && result->newBehavior == 0 &&
                result->actionKind != DM1_ACTION_ATTACK)
                result->newBehavior = DM1_BEHAVIOR_APPROACH;
            result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
            result->nextEventDelayTicks = max_val(1,
                F0732_COMBAT_RngRandom_Compat(rng, 4) +
                ctx->movementTicks - 1);
            return 1;
        }

        /* FLEE behavior (source: F0209 behavior == C5)
         * Handled in per-creature events, but group tick also processes */
        if (behavior == DM1_BEHAVIOR_FLEE) {
            F0820_DM1_GROUP_GetFleeDirection_Compat(ctx, &fleeP, &fleeS);
            /* Speed boost: movementTicks -= (movementTicks >> 2)
             * Source: F0209 T0209094: L0461 -= (L0461 >> 2) */
            int fleeTicks = ctx->movementTicks -
                            (ctx->movementTicks >> 2);

            if (ctx->distanceToVisibleParty > 0) {
                activeGroup->targetMapX = ctx->partyMapX;
                activeGroup->targetMapY = ctx->partyMapY;
            } else {
                if (activeGroup->delayFleeingFromTarget > 0) {
                    activeGroup->delayFleeingFromTarget--;
                } else {
                    /* No longer afraid — back to wander */
                    result->newBehavior = DM1_BEHAVIOR_WANDER;
                    result->startWandering = 1;
                    result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
                    result->nextEventDelayTicks = max_val(1, fleeTicks);
                    return 1;
                }
            }

            /* Flee: move opposite direction */
            F0813_DM1_GROUP_PickSingleSquareMove_Compat(
                ctx, fleeP, fleeS, 1, rng, &dir);
            if (dir >= 0) {
                result->actionKind = DM1_ACTION_FLEE_MOVE;
                result->moveDirection = dir;
                result->moveDestMapX = ctx->currentGroupMapX + g_dx[dir];
                result->moveDestMapY = ctx->currentGroupMapY + g_dy[dir];
            }

            result->newBehavior = DM1_BEHAVIOR_FLEE;
            result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
            result->nextEventDelayTicks = max_val(1, fleeTicks);
            return 1;
        }

        /* Default for unhandled behavior */
        result->actionKind = DM1_ACTION_NONE;
        result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
        result->nextEventDelayTicks = max_val(1, ctx->movementTicks);
        return 1;
    }

    /* =======================================================
     * Source: F0209 per-creature attack events C38-C41
     *
     * When in ATTACK behavior, each creature gets individual
     * attack/move decisions.
     * ======================================================= */
    if (eventType >= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
        eventType <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {

        int creatureIndex = eventType - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;

        /* FLEE override: if group switched to flee, stop attacking */
        if (behavior == DM1_BEHAVIOR_FLEE) {
            if (ctx->creatureCount > 0) {
                result->stopAttacking = 1;
            }
            /* Process as flee */
            F0820_DM1_GROUP_GetFleeDirection_Compat(ctx, &fleeP, &fleeS);
            F0813_DM1_GROUP_PickSingleSquareMove_Compat(
                ctx, fleeP, fleeS, 1, rng, &dir);
            if (dir >= 0) {
                result->actionKind = DM1_ACTION_FLEE_MOVE;
                result->moveDirection = dir;
                result->moveDestMapX = ctx->currentGroupMapX + g_dx[dir];
                result->moveDestMapY = ctx->currentGroupMapY + g_dy[dir];
            }
            result->newBehavior = DM1_BEHAVIOR_FLEE;
            int fleeTicks = ctx->movementTicks - (ctx->movementTicks >> 2);
            result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
            result->nextEventDelayTicks = max_val(1, fleeTicks);
            return 1;
        }

        /* Already attacking — check if creature is still in attack state */
        if (activeGroup->aspect[creatureIndex] & 0x80) {
            /* Currently marked as attacking → schedule next attack tick */
            result->actionKind = DM1_ACTION_NONE;
            result->nextEventType = eventType;
            result->nextEventDelayTicks = ctx->creatureInfo.attackTicks +
                F0732_COMBAT_RngRandom_Compat(rng, 4) - 1;
            if (result->nextEventDelayTicks < 1)
                result->nextEventDelayTicks = 1;
            return 1;
        }

        /* Not currently attacking — try to attack */
        if (creatureIndex > ctx->creatureCount) {
            /* Invalid creature index */
            result->actionKind = DM1_ACTION_NONE;
            return 1;
        }

        /* Check visibility and range for this creature */
        if (ctx->distanceToVisibleParty > 0) {
            activeGroup->targetMapX = ctx->partyMapX;
            activeGroup->targetMapY = ctx->partyMapY;

            int attackRange = DM1_ATTACK_RANGE(ctx->creatureInfo.ranges);
            int distX = ctx->currentGroupMapX - ctx->partyMapX;
            if (distX < 0) distX = -distX;
            int distY = ctx->currentGroupMapY - ctx->partyMapY;
            if (distY < 0) distY = -distY;

            /* In range and on same row/column */
            if (ctx->distanceToVisibleParty <= attackRange &&
                (distX == 0 || distY == 0)) {

                /* Random attack probability — ReDMCSB F0209:
                 * Longer range creatures are more likely to attack.
                 * Condition: random(16) + 1 >= range means attack.
                 * So range 1 (melee) always attacks, range 15 rarely. */
                int roll = F0732_COMBAT_RngRandom_Compat(rng, 16) + 1;
                if (roll >= attackRange) {
                    if (resolve_quarter_square_melee_cell_adjustment(
                            ctx, activeGroup, creatureIndex, rng, result)) {
                        return 1;
                    }

                    struct DM1CreatureProjectileAttack_Compat projectile;
                    if (ctx->creatureType == DM1_CREATURE_TYPE_GIGGLER) {
                        struct DM1GigglerStealResult_Compat steal;
                        F0822_DM1_GIGGLER_ResolveStealAttempt_Compat(
                            ctx->targetChampionDexterity,
                            ctx->targetChampionOccupiedSlotMask,
                            ctx->targetChampionLuckyAttemptMask,
                            rng, &steal);
                        result->actionKind = DM1_ACTION_STEAL;
                        result->attackIsProjectile = 0;
                        result->newBehavior = steal.newBehavior;
                        result->stealSlotIndex = steal.stealSlotIndex;
                        result->stolenSlotMask = steal.stolenSlotMask;
                        result->stolenCount = steal.stolenCount;
                        result->gigglerInitialStealCounter =
                            steal.initialCounter;
                        result->gigglerFleeDelayTicks = steal.fleeDelayTicks;
                        if (steal.shouldFlee) {
                            activeGroup->delayFleeingFromTarget =
                                steal.fleeDelayTicks;
                            result->nextEventType =
                                DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
                            result->nextEventDelayTicks =
                                steal.fleeDelayTicks;
                        } else {
                            result->nextEventType = eventType;
                            result->nextEventDelayTicks =
                                DM1_NEXT_BEHAVIOR_TICKS(
                                    ctx->creatureInfo.animationTicks) +
                                F0732_COMBAT_RngRandom_Compat(rng, 2);
                            if (result->nextEventDelayTicks < 1)
                                result->nextEventDelayTicks = 1;
                        }
                        return 1;
                    }
                    F0823_DM1_GROUP_ResolveProjectileAttack_Compat(
                        ctx, activeGroup, creatureIndex, rng, &projectile);
                    result->actionKind = DM1_ACTION_ATTACK;
                    result->attackIsProjectile = projectile.shouldLaunch;
                    if (projectile.shouldLaunch) {
                        result->attackTargetCell = projectile.targetCell;
                        result->projectileThing = projectile.projectileThing;
                        result->projectileKineticEnergy = projectile.kineticEnergy;
                        result->projectileAttack = projectile.attack;
                        result->projectileStepEnergy = projectile.stepEnergy;
                        result->projectileDirection = projectile.direction;
                        result->projectileUseSpellSoundFallback =
                            projectile.useSpellSoundFallback;
                    }
                    result->newBehavior = DM1_BEHAVIOR_ATTACK;
                    result->nextEventType = eventType;
                    result->nextEventDelayTicks =
                        DM1_NEXT_BEHAVIOR_TICKS(
                            ctx->creatureInfo.animationTicks) +
                        F0732_COMBAT_RngRandom_Compat(rng, 2);
                    if (result->nextEventDelayTicks < 1)
                        result->nextEventDelayTicks = 1;
                    return 1;
                }
            }

            /* Not in attack range — switch to approach */
            result->newBehavior = DM1_BEHAVIOR_APPROACH;
            if (ctx->creatureCount > 0) {
                result->stopAttacking = 1;
            }
            result->actionKind = DM1_ACTION_NONE;
            result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
            result->nextEventDelayTicks = 1;
            return 1;
        }

        /* Can't see party — switch to approach toward last known pos */
        result->newBehavior = DM1_BEHAVIOR_APPROACH;
        if (ctx->creatureCount > 0) {
            result->stopAttacking = 1;
        }
        result->actionKind = DM1_ACTION_NONE;
        result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
        result->nextEventDelayTicks = 1;
        return 1;
    }

    /* Unhandled event type */
    result->actionKind = DM1_ACTION_NONE;
    result->nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    result->nextEventDelayTicks = max_val(1, ctx->movementTicks);
    return 1;
}
