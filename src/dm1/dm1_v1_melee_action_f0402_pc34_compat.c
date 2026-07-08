#include "dm1_v1_melee_action_f0402_pc34_compat.h"

#include <string.h>

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
