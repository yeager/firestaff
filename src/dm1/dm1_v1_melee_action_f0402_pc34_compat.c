#include "dm1_v1_melee_action_f0402_pc34_compat.h"

#include <string.h>

#include "dm1_v1_champion_needs_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"
#include "firestaff/dm1/v1/G0493_pc34_compat.h"

enum {
    DM1_MELEE_CREATURE_ATTR_NON_MATERIAL_PC34 = 0x0040
};

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
