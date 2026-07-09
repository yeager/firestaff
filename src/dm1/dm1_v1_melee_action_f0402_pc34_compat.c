#include "dm1_v1_melee_action_f0402_pc34_compat.h"

#include <string.h>

#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_champion_needs_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"
#include "firestaff/dm1/v1/G0493_pc34_compat.h"

enum {
    DM1_MELEE_CREATURE_ATTR_NON_MATERIAL_PC34 = 0x0040
};

static int dm1_v1_group_creature_cell_f0190_pc34(
    const struct DungeonGroup_Compat* group,
    int creatureIndex) {
    if (!group) return EXPLOSION_CELL_CENTERED;
    if (group->cells == 0xFFu) return EXPLOSION_CELL_CENTERED;
    return (group->cells >> (creatureIndex << 1)) & 0x03;
}

static const unsigned char s_dm1_f0177_ordered_cells_pc34[8][4] = {
    { 0, 1, 3, 2 },
    { 1, 0, 2, 3 },
    { 1, 2, 0, 3 },
    { 2, 1, 3, 0 },
    { 3, 2, 0, 1 },
    { 2, 3, 1, 0 },
    { 0, 3, 1, 2 },
    { 3, 0, 2, 1 }
};

static int dm1_v1_f0176_creature_occupies_cell_pc34(
    const DM1_MeleeF0177TargetCreatureInputPc34* in,
    int creatureIndex,
    int cell) {
    int creatureCell;
    int queryCell;
    if (!in) return 0;
    if (creatureIndex < 0 || creatureIndex > in->groupCount ||
        creatureIndex >= 4) {
        return 0;
    }
    creatureCell = (in->groupCells >> (creatureIndex << 1)) & 0x03;
    queryCell = cell & 3;
    if ((in->creatureSize & 3) == DM1_CREATURE_SIZE_HALF_SQUARE) {
        if (((in->groupDirection & 1) == (queryCell & 1))) {
            queryCell = (queryCell + 3) & 3;
        }
        return creatureCell == queryCell ||
               creatureCell == ((queryCell + 1) & 3);
    }
    return creatureCell == queryCell;
}

int dm1_v1_melee_action_tick_plan_f0402_pc34(
    const DM1_MeleeActionTickInputPc34* in,
    DM1_MeleeActionTickPlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;
    if (!in->championPresent) return 0;
    if (in->championIndex < 0 || in->championIndex >= CHAMPION_MAX_PARTY) {
        return 0;
    }
    if (in->actionIndex < 0 ||
        in->actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }
    if (!dm1_v1_action_is_melee_contact_f0407_pc34(in->actionIndex)) {
        return 0;
    }

    /* ReDMCSB: MENU.C F0407 lines 1266-1269 computes the target square from
     * the acting champion direction, then lines 1331-1334 dispatch F0402.
     * M10 still resolves F0177/F0231; this receipt keeps M11 from assembling
     * the source-shaped CMD_ATTACK transport inline. */
    out->valid = 1;
    out->command = CMD_ATTACK;
    out->commandArg1 = (unsigned char)in->championIndex;
    out->commandArg2 = CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    out->reserved = CMD_ATTACK_CREATURE_AUTO_PC34;
    out->targetDirection = in->championDirection & 3;
    out->hasTargetDirection = 1;
    out->reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
                     (unsigned int)(in->actionIndex &
                                    CMD_ATTACK_RESERVED2_ACTION_INDEX_MASK) |
                     CMD_ATTACK_RESERVED2_TARGET_DIRECTION_VALID |
                     (((unsigned int)out->targetDirection
                       << CMD_ATTACK_RESERVED2_TARGET_DIRECTION_SHIFT) &
                      CMD_ATTACK_RESERVED2_TARGET_DIRECTION_MASK);
    return 1;
}

int dm1_v1_melee_damage_emission_plan_f0231_pc34(
    const DM1_MeleeDamageEmissionInputPc34* in,
    DM1_MeleeDamageEmissionPlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;

    /* ReDMCSB: MENU.C F0402 lines 1053-1056 treats F0231 as performed once a
     * target creature exists.  PROJEXPL.C F0231 lines 1514-1517 then may
     * return zero damage; only positive G0513 damage is rendered as feedback. */
    out->valid = 1;
    out->damage = in->damage;
    out->performed = in->combatOutcome != COMBAT_OUTCOME_INVALID;
    out->showDamageFeedback = in->damage > 0;
    return 1;
}

int dm1_v1_melee_runtime_outcome_plan_f0407_f0231_pc34(
    const DM1_MeleeRuntimeOutcomeInputPc34* in,
    DM1_MeleeRuntimeOutcomePlanPc34* out) {
    DM1_MeleeDamageEmissionInputPc34 damageIn;
    DM1_MeleeDamageEmissionPlanPc34 damagePlan;
    int ticks;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;
    if (in->actionIndex < 0 ||
        in->actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }
    if (!dm1_v1_action_is_melee_contact_f0407_pc34(in->actionIndex)) {
        return 0;
    }

    ticks = in->defaultDisabledTicks;
    if (ticks < 0) ticks = 0;
    if (ticks > 255) ticks = 255;

    memset(&damageIn, 0, sizeof(damageIn));
    damageIn.damage = in->observedAttackDamage;
    damageIn.combatOutcome = in->combatOutcome;
    if (!dm1_v1_melee_damage_emission_plan_f0231_pc34(
            &damageIn, &damagePlan) ||
        !damagePlan.valid) {
        return 0;
    }

    out->valid = 1;
    out->performed = damagePlan.performed ||
                     in->closedDoorBranchPerformed ||
                     in->directParryEmptyFront;
    out->showDamageFeedback = damagePlan.showDamageFeedback;
    out->damage = damagePlan.damage;

    /* ReDMCSB: MENU.C F0407 lines 1308-1342 treat the closed-door
     * BASH/HACK/BERZERK/KICK/SWING/CHOP branch as performed before F0402 and
     * override ActionDisabledTicks to 6.  Other failed F0402 melee actions
     * enter the halved XP/tick tail at lines 1331-1337. */
    if (in->closedDoorBranchPerformed && in->closedDoorDisabledTicks > 0) {
        ticks = in->closedDoorDisabledTicks;
        if (ticks > 255) ticks = 255;
    }
    out->disabledTicks = ticks;
    out->meleeFailureTail = !out->performed;
    return 1;
}

int dm1_v1_melee_kill_notify_plan_f0231_pc34(
    const DM1_MeleeKillNotifyInputPc34* in,
    DM1_MeleeKillNotifyPlanPc34* out) {
    int xp;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;

    /* ReDMCSB: PROJEXPL.C F0231 lines 1531-1536 already performs the
     * source XP/stamina side effects before any UI kill notification reaches
     * M11.  Firestaff's legacy kill bonus is kept behind this receipt so the
     * presentation path no longer owns gameplay arithmetic inline. */
    out->valid = 1;
    out->shouldLogDefeated = 1;
    out->creatureType = in->creatureType;
    if (in->activeChampionIndex < 0 ||
        in->activeChampionIndex >= CHAMPION_MAX_PARTY ||
        !in->activeChampionPresent) {
        return 1;
    }

    xp = in->creatureBaseHealth > 0 ? in->creatureBaseHealth / 2 : 10;
    if (xp < 5) xp = 5;
    out->shouldAwardKillXp = 1;
    out->championIndex = in->activeChampionIndex;
    out->xpBonus = xp;
    return 1;
}

int dm1_v1_melee_reach_gate_plan_f0402_pc34(
    const DM1_MeleeReachGateInputPc34* in,
    DM1_MeleeReachGatePlanPc34* out) {
    int i;
    int relativeCell;
    int blockingCell = -1;
    int count;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->blockingChampionIndex = -1;
    out->blockingCell = -1;
    out->combatOutcome = COMBAT_OUTCOME_INVALID;
    if (!in) return 0;
    if (in->championIndex < 0 || in->championIndex >= CHAMPION_MAX_PARTY) {
        return 0;
    }

    out->valid = 1;
    if (!in->championPresent || in->championCurrentHealth <= 0) {
        return 1;
    }

    relativeCell = ((in->championCell & 3) + 4 - (in->targetDirection & 3)) & 3;
    out->relativeCell = relativeCell;
    if (relativeCell == 2) {
        blockingCell = ((in->championCell & 3) + 3) & 3;
    } else if (relativeCell == 3) {
        blockingCell = ((in->championCell & 3) + 1) & 3;
    } else {
        return 1;
    }
    out->blockingCell = blockingCell;

    /* ReDMCSB: MENU.C F0402 lines 1029-1041 rejects back-row melee when
     * another living champion stands in the front cell, sets G0513 to
     * CM1_DAMAGE_CANT_REACH, then returns false before F0231. */
    count = in->partyChampionCount;
    if (count < 0 || count > CHAMPION_MAX_PARTY) count = CHAMPION_MAX_PARTY;
    for (i = 0; i < count; ++i) {
        if (i == in->championIndex) continue;
        if (in->otherChampionPresent[i] &&
            in->otherChampionCurrentHealth[i] > 0 &&
            ((in->otherChampionCell[i] & 3) == blockingCell)) {
            out->blocked = 1;
            out->blockingChampionIndex = i;
            out->damage = 0;
            out->combatOutcome = COMBAT_OUTCOME_INVALID;
            return 1;
        }
    }
    return 1;
}

int dm1_v1_melee_weapon_profile_plan_f0402_f0231_pc34(
    const DM1_MeleeWeaponProfileInputPc34* in,
    DM1_MeleeWeaponProfilePlanPc34* out) {
    int actionIndex;
    int hitProbability;
    int damageFactor;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;

    actionIndex = in->actionIndex;
    hitProbability = dm1_v1_graphic560_action_hit_probability_get_pc34(
        actionIndex);
    damageFactor = dm1_v1_graphic560_action_damage_factor_get_pc34(
        actionIndex);
    if (hitProbability < 0 || damageFactor < 0) {
        actionIndex = CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;
        hitProbability = dm1_v1_graphic560_action_hit_probability_get_pc34(
            actionIndex);
        damageFactor = dm1_v1_graphic560_action_damage_factor_get_pc34(
            actionIndex);
    }
    if (hitProbability < 0 || damageFactor < 0) {
        return 0;
    }

    out->valid = 1;
    out->normalizedActionIndex = actionIndex;
    out->hitProbability = hitProbability;
    out->damageFactor = damageFactor;
    if (in->weaponType == COMBAT_ICON_VORPAL_BLADE ||
        actionIndex == DM1_ACTION_DISRUPT) {
        hitProbability |= 0x8000;
        out->hitNonMaterialFlagSet = 1;
    }

    /* ReDMCSB: MENU.C F0402 lines 1045-1056 reads G0493 hit probability
     * and G0492 damage factor, sets MASK0x8000_HIT_NON_MATERIAL_CREATURES
     * for Vorpal Blade or DISRUPT, then calls PROJEXPL.C F0231 with those
     * action parameters. */
    out->weaponProfile.weaponType = in->weaponType;
    out->weaponProfile.weaponClass = in->weaponClass;
    out->weaponProfile.weaponStrength = in->weaponStrength;
    out->weaponProfile.kineticEnergy = in->kineticEnergy;
    out->weaponProfile.hitProbability = hitProbability;
    out->weaponProfile.damageFactor = damageFactor;
    out->weaponProfile.skillIndex = in->actionSkillIndex;
    out->weaponProfile.attributes = in->weaponAttributes;
    return 1;
}

int dm1_v1_melee_side_effect_plan_f0231_pc34(
    const DM1_MeleeF0231SideEffectInputPc34* in,
    DM1_MeleeF0231SideEffectPlanPc34* out) {
    int16_t stamina;
    int pendingDamage;
    int randomValue;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;
    if (in->championIndex < 0 || in->championIndex >= CHAMPION_MAX_PARTY) {
        return 0;
    }

    out->valid = 1;
    out->currentStaminaAfter = in->currentStamina;
    out->currentHealthAfter = in->currentHealth;
    if (in->damageApplied > 0) {
        int creatureExperience = (in->creatureProperties >> 8) & 0x000F;
        out->shouldAwardXp = in->actionSkillIndex >= 0;
        out->skillIndex = in->actionSkillIndex;
        out->experienceGain =
            ((in->damageApplied * creatureExperience) >> 4) + 3;
        out->staminaRandomModulus = 4;
        out->staminaBaseCost = 4;
    } else {
        out->skillIndex = in->actionSkillIndex;
        out->staminaRandomModulus = 2;
        out->staminaBaseCost = 2;
    }

    randomValue = in->staminaRandomValue;
    if (out->staminaRandomModulus > 0) {
        randomValue %= out->staminaRandomModulus;
        if (randomValue < 0) randomValue += out->staminaRandomModulus;
    } else {
        randomValue = 0;
    }
    out->staminaCost = randomValue + out->staminaBaseCost;

    /* ReDMCSB: PROJEXPL.C F0231 lines 1534-1539 awards damage XP, then
     * decrements stamina by M004_RANDOM(4)+4 on damage, or
     * M005_RANDOM(2)+2 on the miss/no-damage tail. */
    stamina = (int16_t)in->currentStamina;
    pendingDamage = dm1_needs_decrement_stamina(
        &stamina, (int16_t)in->maximumStamina, (int16_t)out->staminaCost);
    out->currentStaminaAfter = (int)stamina;
    out->pendingHealthDamage = pendingDamage;
    if (pendingDamage > 0) {
        int health = in->currentHealth - pendingDamage;
        out->currentHealthAfter = health > 0 ? health : 0;
    }
    return 1;
}

int dm1_v1_melee_weapon_availability_plan_f0402_pc34(
    const DM1_MeleeF0402WeaponAvailabilityInputPc34* in,
    DM1_MeleeF0402WeaponAvailabilityPlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->weaponClass = -1;
    if (!in) return 0;

    out->valid = 1;
    if (in->hasWeaponInfo) {
        out->useActionHandWeaponInfo = 1;
        out->hasUsableF0231WeaponInfo = 1;
        return 1;
    }
    if (in->hasLiveActionIndex && in->actionHandEmpty) {
        /* ReDMCSB: MENU.C F0389 lines 717-718 opens action set 2
         * (PUNCH/KICK/WAR CRY) when the action hand is empty.  MENU.C F0402
         * then still reaches F0231 with F0312(action hand), but without
         * WEAPON_INFO strength/class additions. */
        out->useEmptyHandWeaponInfo = 1;
        out->hasUsableF0231WeaponInfo = 1;
        out->weaponClass = 255;
    }
    return 1;
}

int dm1_v1_melee_command_decode_plan_f0402_pc34(
    const DM1_MeleeF0402CommandDecodeInputPc34* in,
    DM1_MeleeF0402CommandDecodePlanPc34* out) {
    DM1_ActionXpRoute route;
    int actionIndex;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->actionIndex = CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;
    out->actionSkillIndex = -1;
    out->directGroupIndex = -1;
    out->directCreatureIndex = -1;
    if (!in) return 0;

    out->valid = 1;
    out->targetDirection = in->partyDirection & 3;
    out->targetMapIndex = in->partyMapIndex;
    out->targetMapX = in->partyMapX;
    out->targetMapY = in->partyMapY;
    out->hasLiveActionIndex =
        (in->reserved2 & CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID) != 0u;
    out->hasLegacyMarker =
        (in->reserved2 & CMD_ATTACK_RESERVED2_LEGACY_MARKER_VALID) != 0u;
    out->hasTargetDirection =
        (in->reserved2 & CMD_ATTACK_RESERVED2_TARGET_DIRECTION_VALID) != 0u;
    if (out->hasTargetDirection) {
        out->targetDirection =
            (int)((in->reserved2 & CMD_ATTACK_RESERVED2_TARGET_DIRECTION_MASK) >>
                  CMD_ATTACK_RESERVED2_TARGET_DIRECTION_SHIFT) & 3;
    }
    switch (out->targetDirection & 3) {
    case DIR_NORTH: out->targetMapY--; break;
    case DIR_EAST:  out->targetMapX++; break;
    case DIR_SOUTH: out->targetMapY++; break;
    case DIR_WEST:  out->targetMapX--; break;
    }

    out->requestedAutoTarget =
        in->commandArg2 == CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    out->requestedAutoCreature =
        in->reserved == CMD_ATTACK_CREATURE_AUTO_PC34;
    if (!out->requestedAutoTarget) {
        out->directGroupIndex = (int)in->commandArg2;
    }
    if (!out->requestedAutoCreature) {
        out->directCreatureIndex = (int)in->reserved;
    }

    actionIndex = out->hasLiveActionIndex
        ? (int)(in->reserved2 & CMD_ATTACK_RESERVED2_ACTION_INDEX_MASK)
        : CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;
    if (dm1_v1_graphic560_action_damage_factor_get_pc34(actionIndex) < 0 ||
        dm1_v1_graphic560_action_hit_probability_get_pc34(actionIndex) < 0) {
        actionIndex = CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34;
    }
    out->actionIndex = actionIndex;

    if (!dm1_v1_action_xp_route(actionIndex, &route) || !route.valid) {
        if (!dm1_v1_action_xp_route(
                CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34, &route) ||
            !route.valid) {
            return 1;
        }
    }
    out->actionSkillIndex = route.skillIndex;

    /* ReDMCSB: MENU.C F0407 lines 1266-1272 selects target square/direction,
     * G0496 skill route, and G0492/G0493 action tables before F0402.  The
     * Firestaff CMD_ATTACK transport stores those source facts in arg/reserved2
     * fields; DM1 owns the decode/default policy, M10 supplies the raw tick and
     * later resolves live thing-list data. */
    return 1;
}

int dm1_v1_melee_target_creature_plan_f0177_pc34(
    const DM1_MeleeF0177TargetCreatureInputPc34* in,
    DM1_MeleeF0177TargetCreaturePlanPc34* out) {
    unsigned int cellSource;
    unsigned int tableIndex;
    const unsigned char* row;
    int i;
    int c;
    int groupCount;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->firstLivingCreatureIndex = -1;
    out->selectedCreatureIndex = -1;
    if (!in) return 0;
    if (in->groupCount < 0) return 0;

    out->valid = 1;
    groupCount = in->groupCount;
    if (groupCount >= 4) groupCount = 3;
    for (i = 0; i <= groupCount; ++i) {
        if (out->firstLivingCreatureIndex < 0 &&
            in->creatureHealth[i] > 0) {
            out->firstLivingCreatureIndex = i;
        }
    }

    /* ReDMCSB: GROUP.C F0177 lines 109-158 builds the ordered target cells
     * through F0229, then calls F0176 lines 69-107.  F0176 treats a
     * C0xFF single-centered group as occupying all cells; otherwise it scans
     * creatures from Count down to 0 and applies the half-square two-cell
     * occupancy rule before returning an ordinal. */
    if (in->groupCells == DM1_GROUP_CELLS_SINGLE_CENTERED) {
        out->singleCenteredGroup = 1;
        out->selectedCreatureIndex = out->firstLivingCreatureIndex;
        return 1;
    }

    cellSource = (unsigned int)(in->championCell & 3);
    if (((in->targetDirection & 1) == 0) && ((in->targetDirection & 3) < 4)) {
        cellSource = (cellSource + 1) & 3;
    }
    tableIndex = ((unsigned int)(in->targetDirection & 3) << 1) |
                 ((cellSource >> 1) & 1u);
    if (tableIndex > 7u) tableIndex = 0u;

    row = s_dm1_f0177_ordered_cells_pc34[tableIndex];
    for (c = 0; c < 4; ++c) {
        int want = (int)row[c];
        for (i = groupCount; i >= 0; --i) {
            if (dm1_v1_f0176_creature_occupies_cell_pc34(in, i, want)) {
                out->selectedCreatureIndex = i;
                return 1;
            }
        }
    }
    if (out->selectedCreatureIndex < 0) {
        out->selectedCreatureIndex = out->firstLivingCreatureIndex;
    }
    return 1;
}

int dm1_v1_melee_preflight_plan_f0402_pc34(
    const DM1_MeleeF0402PreflightInputPc34* in,
    DM1_MeleeF0402PreflightPlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->emitOutcome = COMBAT_OUTCOME_INVALID;
    if (!in) return 0;

    out->valid = 1;
    if (!in->targetResolved) {
        if (in->requestedAutoTarget ||
            in->hasLiveActionIndex ||
            in->hasLiveGroupTable) {
            /* ReDMCSB: MENU.C F0402 lines 1021-1057 reaches F0231 only
             * after a concrete G0517 action-target group and creature
             * ordinal exist.  Live runtime calls without that target are
             * handled no-ops, not synthetic marker damage. */
            out->shouldReturnHandled = 1;
            return 1;
        }
        out->canUseLegacyMarker = 1;
        return 1;
    }

    if (in->reachBlocked || in->disruptBlocked) {
        out->shouldReturnHandled = 1;
        out->shouldEmitDamageDealt = 1;
        out->emitOutcome = COMBAT_OUTCOME_INVALID;
        return 1;
    }
    if (in->candidateInvulnerable) {
        out->shouldReturnHandled = 1;
        return 1;
    }
    if (!in->championSnapshotReady || !in->creatureSnapshotReady) {
        /* ReDMCSB: F0402 enters F0231 only after a target creature ordinal
         * exists; PROJEXPL.C F0231 then returns before side effects when the
         * champion snapshot is invalid or dead. */
        out->shouldReturnHandled = 1;
        return 1;
    }

    out->canResolveDamage = 1;
    return 1;
}

int dm1_v1_melee_strength_plan_f0312_pc34(
    const DM1_MeleeF0312StrengthInputPc34* in,
    DM1_MeleeF0312StrengthPlanPc34* out) {
    int strength;
    int maxLoad;
    int oneSixteenthMaximumLoad;
    int objectWeight;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;

    out->valid = 1;
    strength = (in->random16 & 15) + in->championStrength;
    maxLoad = in->maximumLoad;
    if (maxLoad <= 0) {
        maxLoad = (in->championStrength << 3) + 100;
    }
    oneSixteenthMaximumLoad = maxLoad >> 4;
    out->oneSixteenthMaximumLoad = oneSixteenthMaximumLoad;
    objectWeight = in->hasActionHandWeapon ? in->objectWeight : 0;
    if (objectWeight <= oneSixteenthMaximumLoad) {
        strength += objectWeight - 12;
    } else {
        int loadThreshold =
            oneSixteenthMaximumLoad + ((oneSixteenthMaximumLoad - 12) >> 1);
        out->loadThreshold = loadThreshold;
        if (objectWeight <= loadThreshold) {
            strength += (objectWeight - oneSixteenthMaximumLoad) >> 1;
        } else {
            strength -= (objectWeight - loadThreshold) << 1;
        }
    }

    if (in->hasActionHandWeapon) {
        strength += in->weaponStrength;
        strength += in->weaponSkillBonus << 1;
    }
    out->strengthBeforeStamina = strength;

    if (in->maximumStamina > 0 &&
        in->currentStamina < (in->maximumStamina >> 1)) {
        int halfValue = strength >> 1;
        int halfMaximumStamina = in->maximumStamina >> 1;
        if (halfMaximumStamina > 0) {
            strength = halfValue +
                (int)(((long)halfValue * (long)in->currentStamina) /
                      (long)halfMaximumStamina);
        }
    }
    out->strengthAfterStamina = strength;

    if (in->actionHandWounded) {
        strength >>= 1;
    }
    out->strengthAfterWound = strength;
    strength >>= 1;
    if (strength < 0) strength = 0;
    if (strength > 100) strength = 100;
    out->strengthActionHand = strength;

    /* ReDMCSB: CHAMPION.C F0312 lines 1264-1306 starts with RANDOM(16)
     * plus current Strength, applies held-object weight/max-load pressure,
     * weapon strength, F0303 skill bonus << 1, F0306 stamina adjustment,
     * action-hand wound halving, then returns bounded strength >> 1. */
    return 1;
}

int dm1_v1_melee_champion_snapshot_plan_f0231_pc34(
    const DM1_MeleeF0231ChampionSnapshotInputPc34* in,
    DM1_MeleeF0231ChampionSnapshotPlanPc34* out) {
    int actionSkillIndex;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->normalizedActionSkillIndex = -1;
    if (!in) return 0;
    if (in->championIndex < 0 || in->championIndex >= CHAMPION_MAX_PARTY) {
        return 0;
    }
    if (!in->championPresent || in->currentHealth == 0) {
        return 0;
    }

    actionSkillIndex = in->actionSkillIndex;
    if (actionSkillIndex < 0 || actionSkillIndex >= DM1_TOTAL_SKILL_COUNT) {
        if (in->weaponClass >= DM1_WEAPON_CLASS_FIRST_BOW &&
            in->weaponClass < DM1_WEAPON_CLASS_FIRST_MAGIC_WEAPON) {
            actionSkillIndex = DM1_SKILL_IDX_SHOOT;
        } else if (in->weaponClass == 0) {
            actionSkillIndex = DM1_SKILL_IDX_SWING;
        } else {
            actionSkillIndex = DM1_SKILL_IDX_THROW;
        }
    }

    out->valid = 1;
    out->normalizedActionSkillIndex = actionSkillIndex;
    out->snapshot.championIndex = in->championIndex;
    out->snapshot.currentHealth = in->currentHealth;
    out->snapshot.dexterity = in->dexterity;
    out->snapshot.strengthActionHand = in->strengthActionHand;
    out->snapshot.skillLevelParry = in->skillLevelParry;
    out->snapshot.skillLevelAction = in->skillLevelAction;
    out->snapshot.statisticVitality = in->statisticVitality;
    out->snapshot.statisticAntifire = in->statisticAntifire;
    out->snapshot.statisticAntimagic = in->statisticAntimagic;
    out->snapshot.statisticWisdom = in->statisticWisdom;
    out->snapshot.statisticLuck = in->statisticLuck;
    out->snapshot.statisticLuckMax = in->statisticLuckMax;
    out->snapshot.statisticLuckMin = in->statisticLuckMin;
    out->snapshot.actionHandIcon = in->actionHandIcon;
    out->snapshot.wounds = in->wounds;
    out->snapshot.isResting = in->isResting;
    out->snapshot.partyShieldDefense = in->partyShieldDefense;

    /* ReDMCSB: MENU.C F0402 passes the live champion and action skill into
     * PROJEXPL.C F0231.  Firestaff snapshots only the immutable facts F0231
     * consumes, including F0312 strength, F0303 parry/action skill levels,
     * current Luck for F0308, and action-hand icon special cases. */
    return 1;
}

int dm1_v1_melee_creature_snapshot_plan_f0231_pc34(
    const DM1_MeleeF0231CreatureSnapshotInputPc34* in,
    DM1_MeleeF0231CreatureSnapshotPlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->snapshot.creatureType = -1;
    out->snapshot.creatureIndex = -1;
    if (!in) return 0;
    if (in->groupIndex < 0) return 0;
    if (in->creatureIndex < 0 || in->creatureIndex > 3) return 0;
    if (in->creatureIndex > in->groupCount) return 0;
    if (in->groupCreatureType < 0) return 0;

    out->valid = 1;
    out->snapshot.creatureType = in->groupCreatureType;
    out->snapshot.attack = in->profileAttack;
    out->snapshot.defense = in->profileDefense;
    out->snapshot.dexterity = in->profileDexterity;
    out->snapshot.baseHealth = in->profileBaseHealth;
    out->snapshot.poisonAttack = in->profilePoisonAttack;
    out->snapshot.attackType = in->profileAttackType;
    out->snapshot.attributes = in->profileAttributes;
    out->snapshot.woundProbabilities = in->profileWoundProbabilities;
    out->snapshot.properties = in->profileProperties;
    out->snapshot.doubledMapDifficulty = in->doubledMapDifficulty;
    out->snapshot.creatureIndex = in->creatureIndex;
    out->snapshot.healthBefore = in->creatureHealth;
    out->snapshot.isCandidateInvulnerable =
        in->candidateInvulnerableEnabled &&
        in->candidateInvulnerableGroupIndex == in->groupIndex &&
        in->candidateInvulnerableCreatureIndex == in->creatureIndex;

    /* ReDMCSB: PROJEXPL.C F0231 lines 1438-1457 reads the live GROUP slot
     * and G0243 creature info before hit gating.  Firestaff keeps M10 as the
     * live data reader, while DM1 owns the F0231-facing creature snapshot
     * shape, including C040 candidate-panel invulnerability. */
    return 1;
}

int dm1_v1_melee_runtime_result_plan_f0231_pc34(
    const DM1_MeleeF0231RuntimeResultInputPc34* in,
    DM1_MeleeF0231RuntimeResultPlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;

    out->valid = 1;
    if (in->combatOutcome == COMBAT_OUTCOME_NO_ACTION) {
        out->shouldReturnHandledNoAction = 1;
        return 1;
    }

    /* ReDMCSB: PROJEXPL.C F0231 lines 1531-1547 applies champion luck,
     * XP, and stamina side effects after the attack resolves, calls
     * GROUP.C F0190 only when damage is positive, and still treats zero
     * damage as a handled melee result before F0209 reaction scheduling. */
    out->shouldWriteBackLuck = 1;
    out->shouldApplySideEffects = 1;
    out->shouldApplyGroupDamage =
        in->damageApplied > 0 &&
        in->groupIndex >= 0 &&
        in->groupIndex < in->groupCount;
    out->shouldEmitDamageDealt = 1;
    return 1;
}

int dm1_v1_melee_aftermath_plan_f0231_pc34(
    const DM1_MeleeF0231AftermathInputPc34* in,
    DM1_MeleeF0231AftermathPlanPc34* out) {
    int size;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->outcome = COMBAT_OUTCOME_INVALID;
    out->smokeAttack = 110;
    out->smokeCell = EXPLOSION_CELL_CENTERED;
    out->reactionEventKind = DM1_EVENT_REACTION_PARTY_IS_ADJACENT;
    if (!in) return 0;

    out->valid = 1;
    out->outcome = (in->damageOutcome != COMBAT_OUTCOME_INVALID)
        ? in->damageOutcome
        : in->fallbackCombatOutcome;
    out->smokeCell = (in->killedCell == EXPLOSION_CELL_CENTERED)
        ? EXPLOSION_CELL_CENTERED
        : (in->killedCell & 3);
    size = in->creatureAttributes & DM1_ATTR_SIZE_MASK;
    if (size == DM1_SIZE_FULL_SQUARE) {
        out->smokeAttack = 255;
    } else if (size == DM1_SIZE_HALF_SQUARE) {
        out->smokeAttack = 190;
    }

    if (in->damageOutcome != COMBAT_OUTCOME_INVALID) {
        out->shouldWriteRawGroup = 1;
        if (in->damageOutcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES ||
            in->damageOutcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
            out->shouldDropPossessions = 1;
            out->shouldCreateDeathSmoke = 1;
            out->shouldEmitKillNotify = 1;
        }
        out->shouldApplyKilledSomeState =
            in->damageOutcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES;
        out->shouldApplyKilledAllSideEffects =
            in->damageOutcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    }

    /* ReDMCSB: PROJEXPL.C F0231 lines 1548-1549 schedules the physical
     * attack reaction event after every melee action except killed-all.
     * GROUP.C F0190 lines 814-917 owns the killed creature drops, group
     * compaction/unlink outcome and smoke attack 110/190/255 by size. */
    out->shouldScheduleReaction =
        !in->fearTriggered &&
        out->outcome != COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    return 1;
}

int dm1_v1_melee_reaction_plan_f0231_pc34(
    const DM1_MeleeF0231ReactionInputPc34* in,
    DM1_MeleeF0231ReactionPlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->eventKind = DM1_EVENT_REACTION_PARTY_IS_ADJACENT;
    if (!in) return 0;

    out->valid = 1;
    out->groupIndex = in->groupIndex;
    out->creatureType = in->creatureType;
    out->mapIndex = in->mapIndex;
    out->mapX = in->mapX;
    out->mapY = in->mapY;
    out->fireAtTick = in->currentTick + 1u;
    if (in->groupIndex < 0) return 1;
    if (in->outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) return 1;
    out->shouldSchedule = 1;

    /* ReDMCSB: PROJEXPL.C F0231 lines 1548-1549 queues
     * CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT after physical
     * melee unless all creatures in the group were killed. */
    return 1;
}

int dm1_v1_melee_death_smoke_plan_f0190_pc34(
    const DM1_MeleeF0190DeathSmokeInputPc34* in,
    DM1_MeleeF0190DeathSmokePlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;

    out->valid = 1;
    if (!in->shouldCreate) return 1;
    out->shouldCreate = 1;
    out->createInput.explosionType = C040_EXPLOSION_SMOKE;
    out->createInput.attack = in->smokeAttack;
    out->createInput.mapIndex = in->mapIndex;
    out->createInput.mapX = in->mapX;
    out->createInput.mapY = in->mapY;
    out->createInput.cell =
        (in->smokeCell == EXPLOSION_CELL_CENTERED)
            ? EXPLOSION_CELL_CENTERED
            : (in->smokeCell & 3);
    out->createInput.centered =
        out->createInput.cell == EXPLOSION_CELL_CENTERED;
    out->createInput.currentTick = in->currentTick;
    out->createInput.ownerKind = PROJECTILE_OWNER_CHAMPION;
    out->createInput.ownerIndex = -1;
    out->createInput.creatorProjectileSlot = -1;

    /* ReDMCSB: GROUP.C F0190 lines 907-917 creates smoke with the
     * size-derived attack and killed cell; PROJEXPL.C F0213 then owns the
     * explosion allocation/timeline.  DM1 owns the create input, M10 owns
     * the live allocation. */
    return 1;
}

int dm1_v1_melee_possession_drop_plan_f0190_pc34(
    const DM1_MeleeF0190PossessionDropInputPc34* in,
    DM1_MeleeF0190PossessionDropPlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;

    out->valid = 1;
    out->creatureType = in->creatureType;
    out->creatureCell = (in->killedCell == EXPLOSION_CELL_CENTERED)
        ? EXPLOSION_CELL_CENTERED
        : (in->killedCell & 3);
    out->mapIndex = in->mapIndex;
    out->mapX = in->mapX;
    out->mapY = in->mapY;
    if (in->outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
        out->shouldDropGroupFixedPossessions = 1;
        out->shouldDropGroupSlotPossessions = 1;
    } else if (in->outcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES &&
               (in->creatureAttributes & DM1_ATTR_DROP_FIXED_POSS) != 0) {
        out->shouldDropCreatureFixedPossessions = 1;
    }

    /* ReDMCSB: GROUP.C F0190 lines 824-847 drops all group possessions
     * when the group dies, or the killed creature fixed possessions when a
     * multi-creature group loses one not-moving creature.  M10 materializes
     * the thing chains; DM1 owns this source branch policy. */
    return 1;
}

int dm1_v1_melee_killed_some_state_plan_f0190_pc34(
    const DM1_MeleeF0190KilledSomeStateInputPc34* in,
    DM1_MeleeF0190KilledSomeStatePlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->newGroupBehavior = -1;
    out->newAiStateKind = -1;
    if (!in) return 0;

    out->valid = 1;
    out->groupIndex = in->groupIndex;
    out->killedCreatureIndex = in->killedCreatureIndex;
    out->originalGroupCount = in->originalGroupCount;
    out->mapIndex = in->mapIndex;
    out->mapX = in->mapX;
    out->mapY = in->mapY;
    if (in->outcome != COMBAT_OUTCOME_KILLED_SOME_CREATURES) return 1;
    if (in->groupBehavior != DM1_BEHAVIOR_ATTACK) return 1;
    if (in->killedCreatureIndex < 0 || in->killedCreatureIndex > 3) return 1;

    out->shouldCleanupCreatureEvents = 1;
    if (in->mapIndex == in->partyMapIndex) {
        out->shouldEvaluateFear = 1;
        out->fearContext.currentMapIndex = in->mapIndex;
        out->fearContext.currentGroupMapX = in->mapX;
        out->fearContext.currentGroupMapY = in->mapY;
        out->fearContext.partyMapIndex = in->partyMapIndex;
        out->fearContext.partyMapX = in->partyMapX;
        out->fearContext.partyMapY = in->partyMapY;
        out->fearContext.creatureType = in->creatureType;
        out->fearContext.creatureInfo.properties = in->creatureProperties;
        out->fearContext.groupBehavior = in->groupBehavior;
        out->fearContext.creatureCount = in->originalGroupCount;
    }

    /* ReDMCSB: GROUP.C F0190 lines 848-889 only cleans C33-C36/C38-C41
     * events and evaluates fear on killed-some groups already in ATTACK.
     * Fear is evaluated only on the party map. */
    return 1;
}

int dm1_v1_melee_killed_some_fear_apply_plan_f0190_pc34(
    const DM1_MeleeF0190KilledSomeStateInputPc34* in,
    int shouldFlee,
    int fleeDelay,
    DM1_MeleeF0190KilledSomeStatePlanPc34* out) {
    if (!dm1_v1_melee_killed_some_state_plan_f0190_pc34(in, out)) {
        return 0;
    }
    if (!out->valid || !out->shouldEvaluateFear || !shouldFlee) return 1;

    out->shouldApplyFear = 1;
    out->newGroupBehavior = DM1_BEHAVIOR_FLEE;
    out->newAiStateKind = AI_STATE_FLEE;
    out->fearCounter = fleeDelay;

    /* ReDMCSB: GROUP.C F0190 lines 887-889 stores
     * DelayFleeingFromTarget and switches the group to C5 FLEE after a
     * successful fear roll. */
    return 1;
}

int dm1_v1_melee_killed_all_state_plan_f0190_pc34(
    const DM1_MeleeF0190KilledAllStateInputPc34* in,
    DM1_MeleeF0190KilledAllStatePlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in) return 0;

    out->valid = 1;
    out->groupIndex = in->groupIndex;
    out->mapIndex = in->targetMapIndex;
    out->mapX = in->targetMapX;
    out->mapY = in->targetMapY;
    if (in->outcome != COMBAT_OUTCOME_KILLED_ALL_CREATURES) return 1;
    if (in->groupIndex < 0) return 1;

    out->shouldUnlinkGroupFromSquare = 1;
    out->shouldClearGroupNext = 1;
    out->shouldRemoveActiveGroupState = 1;
    out->shouldWriteRawGroup = 1;

    /* ReDMCSB: GROUP.C F0190 lines 824-829 drops possessions and calls
     * F0189_GROUP_Delete when the last creature dies.  Firestaff keeps the
     * live unlink/raw-write operations in M10, but DM1 owns the branch
     * policy and target square. */
    return 1;
}

static int dm1_v1_melee_event_creature_index_f0190_pc34(int eventType) {
    if (eventType >= DM1_EVENT_UPDATE_ASPECT_CREATURE_0 &&
        eventType < DM1_EVENT_UPDATE_BEHAVIOR_GROUP) {
        return eventType - DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
    }
    if (eventType >= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
        eventType <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
        return eventType - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    }
    return -1;
}

int dm1_v1_melee_timeline_cleanup_plan_f0190_pc34(
    const DM1_MeleeF0190TimelineCleanupInputPc34* in,
    DM1_MeleeF0190TimelineCleanupPlanPc34* out) {
    int eventCreatureIndex;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->shouldKeepEvent = 1;
    out->newEventType = -1;
    out->eventCreatureIndex = -1;
    if (!in) return 0;
    if (in->killedCreatureIndex < 0 || in->killedCreatureIndex > 3) return 0;

    out->valid = 1;
    out->newEventType = in->eventType;
    if (in->eventKind != TIMELINE_EVENT_CREATURE_REACTION) return 1;
    if (in->eventMapIndex != in->targetMapIndex ||
        in->eventMapX != in->targetMapX ||
        in->eventMapY != in->targetMapY) {
        return 1;
    }

    eventCreatureIndex =
        dm1_v1_melee_event_creature_index_f0190_pc34(in->eventType);
    out->eventCreatureIndex = eventCreatureIndex;
    if (eventCreatureIndex < 0) return 1;
    if (eventCreatureIndex == in->killedCreatureIndex) {
        out->shouldKeepEvent = 0;
        return 1;
    }
    if (eventCreatureIndex > in->killedCreatureIndex) {
        out->newEventType = in->eventType - 1;
    }

    /* ReDMCSB: GROUP.C F0190 lines 848-872 scans C33-C36 aspect and C38-C41
     * behavior events on the killed group square, deletes the killed creature's
     * event, and decrements later creature event types after group compaction. */
    return 1;
}

int dm1_v1_melee_resolve_damage_f0231_pc34(
    struct CombatantChampionSnapshot_Compat* attacker,
    const struct WeaponProfile_Compat* weapon,
    const struct CombatantCreatureSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct CombatResult_Compat* out) {
    /* ReDMCSB: PROJEXPL.C F0231 lines 1416-1546 owns the champion melee
     * hit gate, damage RNG, weak-damage recovery, Vorpal/non-material
     * handling, skill critical bonus, and final damage value before F0190.
     * Keep the entry point DM1-owned while the shared M10 resolver still
     * carries the source-locked arithmetic used by DM1 and CSB. */
    return F0735_COMBAT_ResolveChampionMelee_Compat(
        attacker, weapon, defender, rng, out);
}

int dm1_v1_melee_apply_group_damage_f0190_pc34(
    const struct CombatResult_Compat* result,
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    int* outOutcome) {
    DM1_MeleeF0190GroupDamageApplyPlanPc34 plan;
    if (!dm1_v1_melee_apply_group_damage_plan_f0190_pc34(
            result, group, creatureIndex, &plan) ||
        !plan.valid) {
        return 0;
    }
    if (outOutcome) *outOutcome = plan.outcome;
    return plan.shouldApplyDamage;
}

int dm1_v1_melee_apply_group_damage_plan_f0190_pc34(
    const struct CombatResult_Compat* result,
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    DM1_MeleeF0190GroupDamageApplyPlanPc34* out) {
    int outcome = COMBAT_OUTCOME_INVALID;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->killedCell = EXPLOSION_CELL_CENTERED;
    out->outcome = COMBAT_OUTCOME_INVALID;
    if (!result || !group || creatureIndex < 0 || creatureIndex > (int)group->count) {
        return 0;
    }
    out->valid = 1;
    out->originalGroupCount = (int)group->count;
    out->killedCell =
        dm1_v1_group_creature_cell_f0190_pc34(group, creatureIndex);
    out->damageApplied = result->damageApplied;
    if (result->damageApplied <= 0) {
        out->outcome = result->outcome;
        return 1;
    }

    /* ReDMCSB: PROJEXPL.C F0231 line 1533 applies final damage through
     * GROUP.C F0190.  GROUP.C F0190 lines 787-917 reads the original group
     * count/cell before mutating health, compacting survivors, dropping death
     * smoke/possessions, and returning the killed-some/killed-all outcome.
     * Firestaff still uses the shared compact group-slot mutator for DM1/CSB,
     * but this DM1 receipt owns the F0231->F0190 apply gate and facts M10 must
     * consume after mutation. */
    if (!F0738_COMBAT_ApplyDamageToGroup_Compat(
            result, group, creatureIndex, &outcome)) {
        out->outcome = COMBAT_OUTCOME_INVALID;
        return 0;
    }
    out->shouldApplyDamage = 1;
    out->outcome = outcome;
    return 1;
}

int dm1_v1_melee_disrupt_material_gate_plan_f0402_pc34(
    const DM1_MeleeDisruptMaterialGateInputPc34* in,
    DM1_MeleeDisruptMaterialGatePlanPc34* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->combatOutcome = COMBAT_OUTCOME_INVALID;
    if (!in) return 0;
    if (in->actionIndex < 0 ||
        in->actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }

    out->valid = 1;
    if (in->actionIndex != DM1_ACTION_DISRUPT) {
        return 1;
    }

    /* ReDMCSB: MENU.C F0402 lines 1042-1043 rejects DISRUPT before F0231
     * unless F0144 reports MASK0x0040_NON_MATERIAL for the target group. */
    if ((in->targetCreatureAttributes &
         DM1_MELEE_CREATURE_ATTR_NON_MATERIAL_PC34) == 0) {
        out->blocked = 1;
        out->damage = 0;
        out->combatOutcome = COMBAT_OUTCOME_INVALID;
    }
    return 1;
}
