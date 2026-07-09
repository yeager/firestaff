/*
 * dm1_v1_action_xp_graphic560_pc34_compat.c
 *
 * DM1 V1 (PC 3.4 English) action→skill / action→XP routing fixture.
 * Source-locked to ReDMCSB MENU.C G0496 (skill) and G0497 (XP gain)
 * through the shared PC 3.4 EN source-lock accessors.
 *
 * See header for full provenance and citation table.
 */
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "firestaff/dm1/v1/G0496_pc34_compat.h"
#include "firestaff/dm1/v1/G0497_pc34_compat.h"
#include "firestaff/dm1/v1/G0491_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"
#include "firestaff/dm1/v1/G0494_pc34_compat.h"
#include "firestaff/dm1/v1/G0495_pc34_compat.h"

#include <string.h>

#define DM1_THING_TYPE_WEAPON 5
#define DM1_THING_TYPE_ARMOUR 6
#define DM1_THING_TYPE_JUNK 10
#define DM1_JUNK_MAGICAL_BOX_BLUE 42
#define DM1_JUNK_MAGICAL_BOX_GREEN 43
#define DM1_STATUS_THIEVES_EYE 73
#define DM1_STATUS_SPELL_SHIELD 77
#define DM1_STATUS_FIRE_SHIELD 78
#define DM1_IMMUNE_TO_FEAR_PC34 15
#define DM1_DOOR_BASH_DISABLED_TICKS_PC34 6
#define DM1_DOOR_BASH_DESTRUCTION_DELAY_TICKS_PC34 2
#define DM1_F0325_ATTR_LOAD_STATISTICS_PC34 0x0300

static int dm1_step_east_for_dir(int direction) {
    switch (direction & 3) {
        case 1: return 1;
        case 3: return -1;
        default: return 0;
    }
}

static int dm1_step_north_for_dir(int direction) {
    switch (direction & 3) {
        case 0: return -1;
        case 2: return 1;
        default: return 0;
    }
}

/* ReDMCSB CHAMPION.C F0304 line ~874: base skill = (sub - 4) >> 2.
 * For base skills (0..3) the mapping is identity. */
static int sub_skill_base_index(int skillIndex) {
    if (skillIndex < 0 || skillIndex >= 20) return 0;
    if (skillIndex < 4) return skillIndex;
    return (skillIndex - 4) >> 2;
}

static int f0401_fright_base_for_action(int actionIndex, int* outExperience) {
    if (outExperience) *outExperience = 0;
    /* ReDMCSB: MENU.C F0401 lines 946-966 maps action to base fright amount
     * and C14_SKILL_INFLUENCE XP before adding F0303(INFLUENCE). */
    switch (actionIndex) {
        case DM1_ACTION_WAR_CRY:
            if (outExperience) *outExperience = 12;
            return 3;
        case DM1_ACTION_CALM:
            if (outExperience) *outExperience = 35;
            return 7;
        case DM1_ACTION_BRANDISH:
            if (outExperience) *outExperience = 30;
            return 6;
        case DM1_ACTION_BLOW_HORN:
            if (outExperience) *outExperience = 20;
            return 6;
        case DM1_ACTION_CONFUSE:
            if (outExperience) *outExperience = 45;
            return 12;
        default:
            return 0;
    }
}

int dm1_v1_action_xp_route(int actionIndex, DM1_ActionXpRoute* out) {
    int skillIndex;
    int experienceGain;
    if (!out) return 0;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        out->valid = 0;
        out->skillIndex = 0;
        out->baseSkillIndex = 0;
        out->experienceGain = 0;
        return 0;
    }
    skillIndex = dm1_v1_graphic560_action_skill_index_get_pc34(actionIndex);
    experienceGain = dm1_v1_g0497_get_pc34(actionIndex);
    if (skillIndex < 0 || experienceGain < 0) {
        out->valid = 0;
        out->skillIndex = 0;
        out->baseSkillIndex = 0;
        out->experienceGain = 0;
        return 0;
    }
    out->valid = 1;
    out->skillIndex = skillIndex;
    out->baseSkillIndex = sub_skill_base_index(out->skillIndex);
    out->experienceGain = experienceGain;
    return 1;
}

int dm1_v1_action_is_melee_contact_f0407_pc34(int actionIndex) {
    int damageFactor;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) return 0;
    if (actionIndex == DM1_ACTION_BLOCK) return 0;
    /* ReDMCSB: MENU.C G0492 lines 202-245 plus F0407 lines 1308-1342.
     * BLOCK has a damage factor but does not enter the F0402/F0231 melee
     * contact case; PARRY does. */
    damageFactor = dm1_v1_graphic560_action_damage_factor_get_pc34(actionIndex);
    return damageFactor > 0;
}

int dm1_v1_action_is_party_shield_f0407_pc34(int actionIndex) {
    return actionIndex == DM1_ACTION_SPELLSHIELD ||
           actionIndex == DM1_ACTION_FIRESHIELD;
}

int dm1_v1_action_halves_xp_on_f0327_failure_pc34(int actionIndex) {
    /* ReDMCSB: MENU.C F0407 lines 1280-1305 routes FIREBALL, DISPELL,
     * LIGHTNING, and SPIT through F0327 and halves G0497 XP on failure.
     * INVOKE reaches the same T0407014 path from lines 1480-1493. */
    switch (actionIndex) {
        case DM1_ACTION_FIREBALL:
        case DM1_ACTION_DISPELL:
        case DM1_ACTION_LIGHTNING:
        case DM1_ACTION_INVOKE:
        case DM1_ACTION_SPIT:
            return 1;
        default:
            return 0;
    }
}

int dm1_v1_action_stamina_cost_f0407_pc34(int actionIndex,
                                          int championIndex,
                                          unsigned int gameTick) {
    int base;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) return 0;
    if (championIndex < 0) return 0;
    /* ReDMCSB: MENU.C F0407 line 1272 uses G0494[action] + RANDOM(2).
     * M11 supplies deterministic jitter through the same parity model it
     * already used before this helper was introduced. */
    base = dm1_v1_graphic560_action_stamina_get_pc34(actionIndex);
    if (base < 0) return 0;
    return base + (int)((gameTick + (unsigned int)championIndex +
                         (unsigned int)actionIndex) & 1u);
}

int dm1_v1_action_disabled_ticks_f0407_pc34(int actionIndex) {
    int ticks;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) return 0;
    /* ReDMCSB: MENU.C G0491 lines 157-201 supplies F0407's common
     * action-disable tick budget before per-action failure tails adjust it. */
    ticks = dm1_v1_graphic560_action_disabled_ticks_get_pc34(actionIndex);
    return ticks < 0 ? 0 : ticks;
}

int dm1_v1_action_prelude_plan_f0407_pc34(
    const DM1_ActionF0407PreludeInputPc34* in,
    DM1_ActionF0407PreludePlanPc34* out) {
    DM1_ActionXpRoute route;
    if (!in || !out) return 0;
    memset(out, 0, sizeof(*out));
    if (in->actionIndex < 0 ||
        in->actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }
    if (!dm1_v1_action_xp_route(in->actionIndex, &route) || !route.valid) {
        return 0;
    }
    /* ReDMCSB: MENU.C F0407 lines 1267-1275 pulls G0496/G0497, G0491,
     * G0494+RANDOM(2), and the F0402 melee gate from the action index before
     * dispatching the action-specific branch. */
    out->valid = 1;
    out->skillIndex = route.skillIndex;
    out->baseSkillIndex = route.baseSkillIndex;
    out->actionExperienceGain = route.experienceGain;
    out->disabledTicks =
        dm1_v1_action_disabled_ticks_f0407_pc34(in->actionIndex);
    out->staminaCost = dm1_v1_action_stamina_cost_f0407_pc34(
        in->actionIndex, in->championIndex, in->gameTick);
    out->isMeleeContact =
        dm1_v1_action_is_melee_contact_f0407_pc34(in->actionIndex);
    return 1;
}

int dm1_v1_action_adjust_f0407_tail_pc34(
    const DM1_ActionF0407TailAdjustInputPc34* in,
    DM1_ActionF0407TailAdjustPc34* out) {
    int xp;
    int ticks;
    int actionIndex;
    if (!in || !out) return 0;
    actionIndex = in->actionIndex;
    out->valid = 0;
    out->actionExperienceGain = 0;
    out->disabledTicks = 0;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }
    xp = in->actionExperienceGain;
    ticks = in->disabledTicks;
    if (xp < 0) xp = 0;
    if (ticks < 0) ticks = 0;
    /* ReDMCSB: MENU.C F0407 lines 1331-1337 halve XP/ticks when the
     * F0402/F0231 melee route returns FALSE. */
    if (in->meleeFailureTail && !in->performed) {
        xp >>= 1;
        ticks >>= 1;
    } else if (dm1_v1_action_is_party_shield_f0407_pc34(actionIndex) &&
               !in->performed) {
        /* ReDMCSB: MENU.C F0407 lines 1456-1461 quarters G0497 XP and halves
         * disabled ticks when F0403 rejects SPELLSHIELD/FIRESHIELD. */
        xp >>= 2;
        ticks >>= 1;
    } else if (dm1_v1_action_halves_xp_on_f0327_failure_pc34(actionIndex) &&
               !in->performed) {
        /* ReDMCSB: MENU.C F0407 lines 1300-1303 halves projectile-action XP
         * when F0327_CHAMPION_IsProjectileSpellCast returns FALSE. */
        xp >>= 1;
    } else if (actionIndex == DM1_ACTION_SHOOT && !in->performed) {
        /* ReDMCSB: MENU.C F0407 lines 1363-1387 clears G0497 XP for SHOOT's
         * no-ammunition route but preserves the common disabled-tick budget. */
        xp = 0;
    } else if (actionIndex == DM1_ACTION_CLIMB_DOWN &&
               in->cancelActionDisable) {
        /* ReDMCSB: MENU.C F0407 lines 1548-1565 clears ActionDisabledTicks
         * on failed rope CLIMB DOWN while preserving stamina and G0497 XP. */
        ticks = 0;
    }
    out->valid = 1;
    out->actionExperienceGain = xp;
    out->disabledTicks = ticks;
    return 1;
}

int dm1_v1_action_completion_plan_f0407_pc34(
    const DM1_ActionF0407CompletionInputPc34* in,
    DM1_ActionF0407CompletionPlanPc34* out) {
    DM1_ActionF0407TailAdjustInputPc34 adjustIn;
    DM1_ActionF0407TailAdjustPc34 adjustOut;
    DM1_ActionDisableInputPc34 disableIn;
    DM1_ActionDisablePlanPc34 disableOut;
    if (!in || !out) return 0;
    memset(out, 0, sizeof(*out));
    out->actionDisabledIndex = 0xFF;
    out->actionEnableSlotOrdinal = 0xFF;
    if (in->actionIndex < 0 ||
        in->actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }

    memset(&adjustIn, 0, sizeof(adjustIn));
    adjustIn.actionIndex = in->actionIndex;
    adjustIn.performed = in->performed;
    adjustIn.actionExperienceGain = in->actionExperienceGain;
    adjustIn.disabledTicks = in->disabledTicks;
    adjustIn.cancelActionDisable = in->cancelActionDisable;
    adjustIn.meleeFailureTail = in->meleeFailureTail;
    if (!dm1_v1_action_adjust_f0407_tail_pc34(&adjustIn, &adjustOut) ||
        !adjustOut.valid) {
        return 0;
    }

    memset(&disableIn, 0, sizeof(disableIn));
    disableIn.actionIndex = in->actionIndex;
    disableIn.disabledTicks = adjustOut.disabledTicks;
    disableIn.pendingShootReadyHandRefill = in->pendingShootReadyHandRefill;
    disableIn.pendingActionEnableSlotOrdinal =
        in->pendingActionEnableSlotOrdinal;
    if (!dm1_v1_action_disable_plan_f0407_pc34(&disableIn, &disableOut) ||
        !disableOut.valid) {
        return 0;
    }

    /* ReDMCSB: MENU.C F0407 lines 1620-1628 runs final disable/stamina/XP
     * after the action-specific branch has adjusted ActionPerformed,
     * ActionDisabledTicks, and G0497 XP. */
    out->valid = 1;
    out->actionExperienceGain = adjustOut.actionExperienceGain;
    if (in->actionIndex == DM1_ACTION_THROW && in->performed &&
        adjustOut.disabledTicks == 0) {
        /* ReDMCSB: CHAMPION.C F0328 line 2168 calls F0330(4) internally,
         * then MENU.C F0407 lines 1613-1617 stores the action-hand slot
         * ordinal on that existing enable-action event. G0491's THROW entry
         * is zero, so the common F0407 tail must not replace F0328's disable. */
        out->preservesExistingActionDisable = 1;
        out->actionEnableSlotOrdinal = in->pendingActionEnableSlotOrdinal > 0
                                           ? in->pendingActionEnableSlotOrdinal
                                           : 0xFF;
        return 1;
    }
    out->disabledTicks = disableOut.disabledTicks;
    out->actionDisabledIndex = disableOut.actionDisabledIndex;
    out->actionEnableSlotOrdinal = disableOut.actionEnableSlotOrdinal;
    out->shouldRefillReadyHandNow = disableOut.shouldRefillReadyHandNow;
    return 1;
}

int dm1_v1_action_stamina_apply_plan_f0325_pc34(
    const DM1_ActionF0325StaminaInputPc34* in,
    DM1_ActionF0325StaminaPlanPc34* out) {
    int staminaAfter;
    int healthAfter;
    int pendingDamage;
    if (!in || !out) return 0;
    memset(out, 0, sizeof(*out));
    if (in->currentStamina < 0 || in->maximumStamina < 0 ||
        in->currentHealth < 0) {
        return 0;
    }
    out->valid = 1;
    out->currentStaminaAfter = in->currentStamina;
    out->currentHealthAfter = in->currentHealth;
    if (in->decrement <= 0) {
        return 1;
    }

    /* ReDMCSB CHAMPION.C F0325 lines 2025-2049: subtract stamina, clamp at
     * zero/max, convert underflow to pending normal damage, then mark LOAD
     * and STATISTICS dirty. M11 applies the planned immediate HP delta. */
    out->applied = 1;
    out->appliedAttributeMask = DM1_F0325_ATTR_LOAD_STATISTICS_PC34;
    staminaAfter = in->currentStamina - in->decrement;
    if (staminaAfter <= 0) {
        out->currentStaminaAfter = 0;
        pendingDamage = (-staminaAfter) >> 1;
        out->pendingHealthDamage = pendingDamage;
        if (pendingDamage > 0) {
            healthAfter = in->currentHealth - pendingDamage;
            out->currentHealthAfter = healthAfter > 0 ? healthAfter : 0;
            out->shouldDamageFlash = 1;
        }
        return 1;
    }

    if (staminaAfter > in->maximumStamina) {
        out->currentStaminaAfter = in->maximumStamina;
        return 1;
    }

    out->currentStaminaAfter = staminaAfter;
    return 1;
}

int dm1_v1_action_begin_plan_f0407_pc34(
    const DM1_ActionF0407BeginInputPc34* in,
    DM1_ActionF0407BeginPlanPc34* out) {
    DM1_ActionF0407PreludeInputPc34 preludeIn;
    DM1_ActionF0407PreludePlanPc34 preludeOut;
    DM1_ActionDefenseInputPc34 defenseIn;
    DM1_ActionDefensePlanPc34 defenseOut;
    DM1_ActionF0325StaminaInputPc34 staminaIn;
    DM1_ActionF0325StaminaPlanPc34 staminaOut;
    if (!in || !out) return 0;
    memset(out, 0, sizeof(*out));
    if (in->actionIndex < 0 ||
        in->actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }

    memset(&preludeIn, 0, sizeof(preludeIn));
    preludeIn.actionIndex = in->actionIndex;
    preludeIn.championIndex = in->championIndex;
    preludeIn.gameTick = in->gameTick;
    if (!dm1_v1_action_prelude_plan_f0407_pc34(&preludeIn, &preludeOut) ||
        !preludeOut.valid) {
        return 0;
    }

    memset(&defenseIn, 0, sizeof(defenseIn));
    defenseIn.actionIndex = in->actionIndex;
    if (!dm1_v1_action_defense_apply_plan_f0407_pc34(&defenseIn,
                                                     &defenseOut) ||
        !defenseOut.valid) {
        return 0;
    }

    memset(&staminaIn, 0, sizeof(staminaIn));
    staminaIn.currentStamina = in->currentStamina;
    staminaIn.maximumStamina = in->maximumStamina;
    staminaIn.currentHealth = in->currentHealth;
    staminaIn.decrement = preludeOut.staminaCost;
    if (!dm1_v1_action_stamina_apply_plan_f0325_pc34(&staminaIn,
                                                     &staminaOut) ||
        !staminaOut.valid) {
        return 0;
    }

    /* ReDMCSB MENU.C F0407 lines 1267-1275 starts an action by applying
     * G0495 defense/action-index, loading G0496/G0497/G0491/G0494, and later
     * line 1624 routes the planned stamina cost through CHAMPION.C F0325. */
    out->valid = 1;
    out->skillIndex = preludeOut.skillIndex;
    out->baseSkillIndex = preludeOut.baseSkillIndex;
    out->actionExperienceGain = preludeOut.actionExperienceGain;
    out->disabledTicks = preludeOut.disabledTicks;
    out->staminaCost = preludeOut.staminaCost;
    out->isMeleeContact = preludeOut.isMeleeContact;
    out->defenseDelta = defenseOut.defenseDelta;
    out->resultingActionIndex = defenseOut.resultingActionIndex;
    out->staminaApplied = staminaOut.applied;
    out->currentStaminaAfter = staminaOut.currentStaminaAfter;
    out->currentHealthAfter = staminaOut.currentHealthAfter;
    out->pendingHealthDamage = staminaOut.pendingHealthDamage;
    out->shouldDamageFlash = staminaOut.shouldDamageFlash;
    out->appliedAttributeMask = staminaOut.appliedAttributeMask;
    return 1;
}

int dm1_v1_action_xp_award_plan_f0407_pc34(
    const DM1_ActionXpAwardInputPc34* in,
    DM1_ActionXpAwardPlanPc34* out) {
    DM1_ActionXpRoute route;
    if (!in || !out) return 0;
    memset(out, 0, sizeof(*out));
    if (in->actionIndex < 0 ||
        in->actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }
    if (!dm1_v1_action_xp_route(in->actionIndex, &route) || !route.valid) {
        return 0;
    }
    /* ReDMCSB MENU.C F0407 lines 1267-1275 loads G0496/G0497 for the
     * action, then lines 1626-1628 award nonzero XP through F0304. */
    out->valid = 1;
    out->skillIndex = route.skillIndex;
    out->baseSkillIndex = route.baseSkillIndex;
    out->experienceGain = in->experienceGain > 0 ? in->experienceGain : 0;
    out->shouldAward = out->experienceGain > 0;
    return 1;
}

int dm1_v1_action_direct_dispatch_plan_f0407_pc34(
    const DM1_ActionDirectDispatchInputPc34* in,
    DM1_ActionDirectDispatchPlanPc34* out) {
    int isMelee;
    if (!in || !out) return 0;
    memset(out, 0, sizeof(*out));
    if (in->actionIndex < 0 ||
        in->actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }
    /* Firestaff's direct helper is not a ReDMCSB entry point; preserve its
     * regression boundary by rejecting F0402 contact actions except PARRY,
     * whose empty-front failure tail is source-locked to MENU.C F0407 lines
     * 1308-1342 and covered by direct runtime tests. */
    isMelee = dm1_v1_action_is_melee_contact_f0407_pc34(in->actionIndex);
    out->valid = 1;
    out->isMeleeContact = isMelee;
    out->allowsParryEmptyFrontRegression =
        in->actionIndex == DM1_ACTION_PARRY;
    out->mayDispatchDirect =
        !isMelee || out->allowsParryEmptyFrontRegression;
    return 1;
}

int dm1_v1_action_defense_apply_plan_f0407_pc34(
    const DM1_ActionDefenseInputPc34* in,
    DM1_ActionDefensePlanPc34* out) {
    int defense;
    if (!in || !out) return 0;
    out->valid = 0;
    out->defenseDelta = 0;
    out->resultingActionIndex = 0xFF;
    if (in->actionIndex < 0 ||
        in->actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }
    /* ReDMCSB: MENU.C F0407 lines 1268-1275 starts the action frame by
     * applying the G0495 action defense value and setting Champion.ActionIndex. */
    defense = dm1_v1_graphic560_action_defense_get_pc34(in->actionIndex);
    if (defense < -128) return 0;
    out->valid = 1;
    out->defenseDelta = defense;
    out->resultingActionIndex = in->actionIndex;
    return 1;
}

int dm1_v1_action_defense_remove_plan_f0407_pc34(
    const DM1_ActionDefenseInputPc34* in,
    DM1_ActionDefensePlanPc34* out) {
    int ok;
    if (!out) return 0;
    ok = dm1_v1_action_defense_apply_plan_f0407_pc34(in, out);
    if (ok && out->valid) {
        /* ReDMCSB: TIMELINE.C enable-action removes the G0495 defense bonus
         * when the F0407 action disable expires, then clears ActionIndex. */
        out->defenseDelta = -out->defenseDelta;
        out->resultingActionIndex = 0xFF;
    }
    return ok;
}

int dm1_v1_action_disable_plan_f0407_pc34(
    const DM1_ActionDisableInputPc34* in,
    DM1_ActionDisablePlanPc34* out) {
    int ticks;
    if (!in || !out) return 0;
    out->valid = 0;
    out->disabledTicks = 0;
    out->actionDisabledIndex = 0xFF;
    out->actionEnableSlotOrdinal = 0xFF;
    out->shouldRefillReadyHandNow = 0;
    if (in->actionIndex < 0 ||
        in->actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }
    ticks = in->disabledTicks;
    if (ticks < 0) ticks = 0;
    if (ticks > 255) ticks = 255;
    /* ReDMCSB: MENU.C F0407 line 1620 disables actions only when the final
     * tick count is nonzero; Firestaff's SHOOT refill adapter must run
     * immediately when no enable-action event will arrive. */
    out->valid = 1;
    out->disabledTicks = ticks;
    out->actionDisabledIndex = ticks > 0 ? in->actionIndex : 0xFF;
    out->actionEnableSlotOrdinal =
        ticks > 0 && in->pendingActionEnableSlotOrdinal > 0 &&
        in->pendingActionEnableSlotOrdinal <= 255
            ? in->pendingActionEnableSlotOrdinal
            : 0xFF;
    out->shouldRefillReadyHandNow =
        ticks == 0 && in->pendingShootReadyHandRefill;
    return 1;
}

int dm1_v1_action_f0405_charge_plan_pc34(
    const DM1_ActionF0405ChargeInputPc34* in,
    DM1_ActionF0405ChargePlanPc34* out) {
    if (!in || !out) return 0;
    out->valid = 0;
    out->shouldDecrement = 0;
    out->thingType = in->thingType;
    out->thingIndex = in->thingIndex;
    if (in->thingIndex < 0) return 0;
    /* ReDMCSB: MENU.C F0405 lines 1143-1181 decrements ChargeCount only for
     * action-hand weapon, armour, or junk records whose ChargeCount is nonzero. */
    switch (in->thingType) {
        case DM1_THING_TYPE_WEAPON:
        case DM1_THING_TYPE_ARMOUR:
        case DM1_THING_TYPE_JUNK:
            out->valid = 1;
            out->shouldDecrement = in->currentChargeCount > 0;
            return 1;
        default:
            return 0;
    }
}

int dm1_v1_action_freeze_life_plan_f0407_pc34(
    const DM1_ActionFreezeLifeInputPc34* in,
    DM1_ActionFreezeLifePlanPc34* out) {
    int addTicks = 70;
    int current;
    if (!in || !out) return 0;
    out->valid = 0;
    out->addTicks = 0;
    out->newFreezeLifeTicks = 0;
    out->consumesActionHandObject = 0;
    out->decrementsActionHandCharges = 0;
    current = in->currentFreezeLifeTicks;
    if (current < 0) current = 0;
    /* ReDMCSB: MENU.C F0407 lines 1567-1601.  Blue magical box adds 30 and is
     * removed, green adds 125 and is removed, otherwise add 70 and run F0405.
     * The resulting FreezeLifeTicks is capped at 200. */
    if (in->actionHandJunkType == DM1_JUNK_MAGICAL_BOX_BLUE) {
        addTicks = 30;
        out->consumesActionHandObject = 1;
    } else if (in->actionHandJunkType == DM1_JUNK_MAGICAL_BOX_GREEN) {
        addTicks = 125;
        out->consumesActionHandObject = 1;
    } else {
        out->decrementsActionHandCharges = 1;
    }
    out->valid = 1;
    out->addTicks = addTicks;
    out->newFreezeLifeTicks = current + addTicks;
    if (out->newFreezeLifeTicks > 200) out->newFreezeLifeTicks = 200;
    return 1;
}

int dm1_v1_action_freeze_life_object_plan_f0407_pc34(
    const DM1_ActionFreezeLifeObjectInputPc34* in,
    DM1_ActionFreezeLifeObjectPlanPc34* out) {
    DM1_ActionFreezeLifeInputPc34 freezeIn;
    DM1_ActionFreezeLifePlanPc34 freezePlan;
    DM1_ActionF0405ChargeInputPc34 chargeIn;
    DM1_ActionF0405ChargePlanPc34 chargePlan;

    if (!in || !out) return 0;
    out->valid = 0;
    out->addTicks = 0;
    out->newFreezeLifeTicks = 0;
    out->shouldRemoveActionHandObject = 0;
    out->shouldDecrementActionHandCharges = 0;
    out->targetThingType = in->actionHandThingType;
    out->targetThingIndex = in->actionHandThingIndex;

    memset(&freezeIn, 0, sizeof(freezeIn));
    freezeIn.currentFreezeLifeTicks = in->currentFreezeLifeTicks;
    freezeIn.actionHandJunkType = in->actionHandJunkType;
    if (!dm1_v1_action_freeze_life_plan_f0407_pc34(
            &freezeIn, &freezePlan) || !freezePlan.valid) {
        return 0;
    }

    out->valid = 1;
    out->addTicks = freezePlan.addTicks;
    out->newFreezeLifeTicks = freezePlan.newFreezeLifeTicks;
    out->shouldRemoveActionHandObject = freezePlan.consumesActionHandObject;
    if (!freezePlan.decrementsActionHandCharges) {
        return 1;
    }

    memset(&chargeIn, 0, sizeof(chargeIn));
    chargeIn.thingType = in->actionHandThingType;
    chargeIn.thingIndex = in->actionHandThingIndex;
    chargeIn.currentChargeCount = in->actionHandChargeCount;
    /* ReDMCSB: MENU.C F0407 lines 1567-1605 selects box consume vs F0405,
     * and F0405 lines 1143-1181 decrements only charged weapon/armour/junk. */
    if (dm1_v1_action_f0405_charge_plan_pc34(&chargeIn, &chargePlan) &&
        chargePlan.valid && chargePlan.shouldDecrement) {
        out->shouldDecrementActionHandCharges = 1;
        out->targetThingType = chargePlan.thingType;
        out->targetThingIndex = chargePlan.thingIndex;
    }
    return 1;
}

int dm1_v1_action_heal_plan_f0407_pc34(
    const DM1_ActionHealInputPc34* in,
    DM1_ActionHealPlanPc34* out) {
    int missing;
    int healCap;
    int mana;
    int cycles = 0;
    int healed = 0;
    if (!in || !out) return 0;
    out->valid = 0;
    out->performed = 0;
    out->alreadyFullHealth = 0;
    out->noMana = 0;
    out->healedAmount = 0;
    out->manaCost = 0;
    out->actionExperienceGain = 0;
    if (in->maximumHealth <= 0) return 0;
    if (in->currentHealth >= in->maximumHealth) {
        out->valid = 1;
        out->performed = 1;
        out->alreadyFullHealth = 1;
        return 1;
    }
    if (in->currentMana <= 0) {
        out->valid = 1;
        out->performed = 1;
        out->noMana = 1;
        return 1;
    }
    /* ReDMCSB: MENU.C F0407 C036_ACTION_HEAL lines 1502-1531 in the PC34/I34E
     * branch: healCap=min(10,F0303(HEAL)); while missing health and mana
     * remain, heal min(missing, healCap), spend 2 mana, and set XP to
     * 2 + 2 per healing cycle. */
    healCap = in->healSkillLevel;
    if (healCap > 10) healCap = 10;
    if (healCap < 1) healCap = 1;
    missing = in->maximumHealth - in->currentHealth;
    mana = in->currentMana;
    while (mana > 0 && missing > 0) {
        int amount = missing < healCap ? missing : healCap;
        healed += amount;
        cycles++;
        missing -= amount;
        mana -= 2;
        if (mana < 0) mana = 0;
    }
    out->valid = 1;
    out->performed = healed > 0;
    out->healedAmount = healed;
    out->manaCost = in->currentMana - mana;
    if (healed > 0) {
        out->actionExperienceGain = 2 + (cycles * 2);
    }
    return 1;
}

int dm1_v1_action_light_plan_f0407_pc34(DM1_ActionLightPlanPc34* out) {
    if (!out) return 0;
    out->valid = 1;
    /* ReDMCSB: MENU.C F0407 C038_ACTION_LIGHT adds
     * G0039_ai_Graphic562_LightPowerToLightAmount[2] then calls
     * F0404_MENUS_CreateEvent70_Light(-2, 2500) and F0405. */
    out->magicalLightAmountDelta = 12;
    out->eventLightPower = -2;
    out->eventDelayTicks = 2500;
    out->decrementsActionHandCharges = 1;
    return 1;
}

int dm1_v1_action_window_random_range_f0407_pc34(int earthSkillLevel) {
    if (earthSkillLevel < 0) earthSkillLevel = 0;
    /* ReDMCSB: MENU.C F0407 C039_ACTION_WINDOW uses
     * RANDOM(F0303(action skill) + 8) + 5. */
    return earthSkillLevel + 8;
}

int dm1_v1_action_window_plan_f0407_pc34(
    const DM1_ActionWindowInputPc34* in,
    DM1_ActionWindowPlanPc34* out) {
    int range;
    int draw;
    if (!in || !out) return 0;
    out->valid = 0;
    out->randomRange = 0;
    out->durationTicks = 0;
    out->statusEventType = 0;
    out->incrementsThievesEyeCount = 0;
    out->decrementsActionHandCharges = 0;
    range = dm1_v1_action_window_random_range_f0407_pc34(in->earthSkillLevel);
    draw = in->randomDraw;
    if (draw < 0) draw = 0;
    if (range <= 0) range = 1;
    if (draw >= range) draw %= range;
    out->valid = 1;
    out->randomRange = range;
    out->durationTicks = draw + 5;
    out->statusEventType = DM1_STATUS_THIEVES_EYE;
    out->incrementsThievesEyeCount = 1;
    out->decrementsActionHandCharges = 1;
    return 1;
}

int dm1_v1_action_shield_plan_f0403_pc34(
    const DM1_ActionShieldInputPc34* in,
    DM1_ActionShieldPlanPc34* out) {
    int ticks;
    int defense;
    int mana;
    int currentDefense;
    if (!in || !out) return 0;
    out->valid = 0;
    out->successful = 0;
    out->manaCost = 0;
    out->remainingMana = in->currentMana;
    out->statusEventType = 0;
    out->eventDelayTicks = 0;
    out->defenseDelta = 0;
    out->newShieldDefense = in->currentShieldDefense;
    out->decrementsActionHandChargesOnSuccess = 0;
    if (in->baseTicks <= 0) return 0;
    ticks = in->baseTicks;
    mana = in->currentMana;
    if (mana < 0) mana = 0;
    currentDefense = in->currentShieldDefense;
    if (currentDefense < 0) currentDefense = 0;
    out->valid = 1;
    out->successful = 1;
    out->remainingMana = mana;
    out->newShieldDefense = currentDefense;
    out->decrementsActionHandChargesOnSuccess = 1;
    /* ReDMCSB: MENU.C F0403 lines 1080-1122. Mana 0 returns before an event;
     * mana 1..3 halves ticks, drains mana, still schedules defense, and returns
     * false so F0407 can quarter XP and halve disabled ticks. */
    if (in->useMana) {
        if (mana == 0) {
            out->successful = 0;
            out->decrementsActionHandChargesOnSuccess = 0;
            return 1;
        }
        if (mana < 4) {
            ticks >>= 1;
            out->manaCost = mana;
            out->remainingMana = 0;
            out->successful = 0;
        } else {
            out->manaCost = 4;
            out->remainingMana = mana - 4;
        }
    }
    defense = ticks >> 5;
    if (currentDefense > 50) defense >>= 2;
    out->statusEventType =
        in->isSpellShield ? DM1_STATUS_SPELL_SHIELD : DM1_STATUS_FIRE_SHIELD;
    out->eventDelayTicks = ticks;
    out->defenseDelta = defense;
    out->newShieldDefense = currentDefense + defense;
    return 1;
}

int dm1_v1_action_fright_random_range_f0401_pc34(int actionIndex,
                                                 int influenceSkillLevel) {
    int experience = 0;
    int base = f0401_fright_base_for_action(actionIndex, &experience);
    if (base <= 0 || experience <= 0) return 0;
    if (influenceSkillLevel < 0) influenceSkillLevel = 0;
    return base + influenceSkillLevel;
}

int dm1_v1_action_fright_plan_f0401_pc34(
    const DM1_ActionFrightInputPc34* in,
    DM1_ActionFrightPlanPc34* out) {
    int experience = 0;
    int base;
    int total;
    int draw;
    int fearResistance;
    int movementTicks;
    if (!in || !out) return 0;
    out->valid = 0;
    out->baseFrightAmount = 0;
    out->totalFrightAmount = 0;
    out->randomRange = 0;
    out->influenceExperience = 0;
    out->resisted = 0;
    out->frightened = 0;
    out->fleeDelayTicks = 0;
    base = f0401_fright_base_for_action(in->actionIndex, &experience);
    if (base <= 0 || experience <= 0) return 0;
    total = base + (in->influenceSkillLevel < 0 ? 0 : in->influenceSkillLevel);
    if (total <= 0) total = 1;
    draw = in->randomDraw;
    if (draw < 0) draw = 0;
    if (draw >= total) draw %= total;
    fearResistance = in->fearResistance;
    if (fearResistance < 0) fearResistance = 0;
    movementTicks = in->movementTicks;
    if (movementTicks <= 0) movementTicks = 1;
    out->valid = 1;
    out->baseFrightAmount = base;
    out->totalFrightAmount = total;
    out->randomRange = total;
    out->influenceExperience = experience;
    /* ReDMCSB: MENU.C F0401 lines 975-987 halves influence XP when
     * FearResistance beats RANDOM(FrightAmount) or equals C15 immune; otherwise
     * the group enters C5_BEHAVIOR_FLEE and DelayFleeingFromTarget is derived
     * from resistance and MovementTicks. */
    if (fearResistance == DM1_IMMUNE_TO_FEAR_PC34 ||
        fearResistance > draw) {
        out->resisted = 1;
        out->influenceExperience >>= 1;
        return 1;
    }
    out->frightened = 1;
    out->fleeDelayTicks =
        ((16 - fearResistance) << 2) / movementTicks;
    return 1;
}

static int f0407_projectile_base_for_action(int actionIndex,
                                            int invokeEnergyRoll,
                                            int invokeFamilyRoll,
                                            int* outSubtype,
                                            int* outAttackType,
                                            int* outKinetic) {
    int family;
    if (!outSubtype || !outAttackType || !outKinetic) return 0;
    switch (actionIndex) {
        case DM1_ACTION_FIREBALL:
            *outSubtype = PROJECTILE_SUBTYPE_FIREBALL;
            *outAttackType = COMBAT_ATTACK_FIRE;
            *outKinetic = 150;
            return 1;
        case DM1_ACTION_DISPELL:
            *outSubtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
            *outAttackType = COMBAT_ATTACK_MAGIC;
            *outKinetic = 150;
            return 1;
        case DM1_ACTION_LIGHTNING:
            *outSubtype = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
            *outAttackType = COMBAT_ATTACK_LIGHTNING;
            *outKinetic = 180;
            return 1;
        case DM1_ACTION_SPIT:
            *outSubtype = PROJECTILE_SUBTYPE_FIREBALL;
            *outAttackType = COMBAT_ATTACK_FIRE;
            *outKinetic = 250;
            return 1;
        case DM1_ACTION_INVOKE:
            family = invokeFamilyRoll;
            if (family < 0) family = 0;
            if (family >= 6) family %= 6;
            *outKinetic = (invokeEnergyRoll < 0 ? 0 : invokeEnergyRoll) + 100;
            switch (family) {
                case 0:
                    *outSubtype = PROJECTILE_SUBTYPE_POISON_BOLT;
                    *outAttackType = COMBAT_ATTACK_NORMAL;
                    return 1;
                case 1:
                    *outSubtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
                    *outAttackType = COMBAT_ATTACK_NORMAL;
                    return 1;
                case 2:
                    *outSubtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
                    *outAttackType = COMBAT_ATTACK_MAGIC;
                    return 1;
                default:
                    *outSubtype = PROJECTILE_SUBTYPE_FIREBALL;
                    *outAttackType = COMBAT_ATTACK_FIRE;
                    return 1;
            }
        default:
            return 0;
    }
}

int dm1_v1_action_projectile_spell_descriptor_f0407_pc34(
    int actionIndex,
    DM1_ActionProjectileSpellDescriptorPc34* out) {
    DM1_ActionXpRoute route;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!dm1_v1_action_xp_route(actionIndex, &route) || !route.valid) {
        return 0;
    }
    out->actionSkillIndex = route.skillIndex;
    switch (actionIndex) {
        case DM1_ACTION_FIREBALL:
            out->verbKind = DM1_ACTION_PROJECTILE_VERB_FIREBALL_PC34;
            break;
        case DM1_ACTION_DISPELL:
            out->verbKind = DM1_ACTION_PROJECTILE_VERB_DISPELL_PC34;
            break;
        case DM1_ACTION_LIGHTNING:
            out->verbKind = DM1_ACTION_PROJECTILE_VERB_LIGHTNING_PC34;
            break;
        case DM1_ACTION_SPIT:
            out->verbKind = DM1_ACTION_PROJECTILE_VERB_SPIT_PC34;
            break;
        case DM1_ACTION_INVOKE:
            out->verbKind = DM1_ACTION_PROJECTILE_VERB_INVOKE_PC34;
            out->requiresInvokeRolls = 1;
            break;
        default:
            return 0;
    }
    /* ReDMCSB: MENU.C F0407 lines 1272-1305 and 1480-1493 use the G0496
     * action skill route before F0327 projectile creation. */
    out->valid = 1;
    return 1;
}

int dm1_v1_action_projectile_spell_plan_f0407_pc34(
    const DM1_ActionProjectileSpellInputPc34* in,
    DM1_ActionProjectileSpellPlanPc34* out) {
    DM1_ActionProjectileSpellDescriptorPc34 descriptor;
    int subtype;
    int attackType;
    int kinetic;
    int requiredMana;
    int actualEnergy;
    int mana;
    int maxMana;
    int stepEnergy;
    int skillLevel;
    if (!in || !out) return 0;
    out->valid = 0;
    out->actionSkillIndex = 0;
    out->verbKind = DM1_ACTION_PROJECTILE_VERB_NONE_PC34;
    out->subtype = 0;
    out->category = PROJECTILE_CATEGORY_MAGICAL;
    out->attackTypeCode = 0;
    out->baseKineticEnergy = 0;
    out->actualKineticEnergy = 0;
    out->requiredMana = 0;
    out->manaCost = 0;
    out->remainingMana = in->currentMana;
    out->stepEnergy = 1;
    out->impactAttack = 90;
    out->launcherStrength = 90;
    out->poisonAttack = 0;
    out->decrementsActionHandCharges = 0;
    if (!dm1_v1_action_projectile_spell_descriptor_f0407_pc34(
            in->actionIndex, &descriptor) || !descriptor.valid) {
        return 0;
    }
    if (!f0407_projectile_base_for_action(
            in->actionIndex, in->invokeEnergyRoll, in->invokeFamilyRoll,
            &subtype, &attackType, &kinetic)) {
        return 0;
    }
    skillLevel = in->skillLevel;
    if (skillLevel < 0) skillLevel = 0;
    requiredMana = 7 - (skillLevel > 6 ? 6 : skillLevel);
    if (requiredMana < 1) requiredMana = 1;
    mana = in->currentMana;
    if (mana < 0) mana = 0;
    actualEnergy = kinetic;
    /* ReDMCSB: MENU.C F0407 lines 1276-1304 computes required mana from G0496,
     * scales kinetic energy down when CurrentMana is too low, then calls F0327
     * and decrements action-hand charges. INVOKE lines 1480-1493 use the same
     * T0407014 path after choosing kinetic M003_RANDOM(128)+100 and family. */
    if (mana < requiredMana) {
        actualEnergy = (requiredMana > 0) ? (mana * kinetic / requiredMana) : kinetic;
        if (actualEnergy < 2) actualEnergy = 2;
        out->manaCost = mana;
        out->remainingMana = 0;
    } else {
        out->manaCost = requiredMana;
        out->remainingMana = mana - requiredMana;
    }
    maxMana = in->maximumMana;
    if (maxMana < 0) maxMana = 0;
    stepEnergy = 10 - (((maxMana >> 3) > 8) ? 8 : (maxMana >> 3));
    if (stepEnergy < 1) stepEnergy = 1;
    if (actualEnergy < (stepEnergy << 2)) {
        actualEnergy += 3;
        stepEnergy--;
        if (stepEnergy < 1) stepEnergy = 1;
    }
    out->valid = 1;
    out->actionSkillIndex = descriptor.actionSkillIndex;
    out->verbKind = descriptor.verbKind;
    out->subtype = subtype;
    out->category = PROJECTILE_CATEGORY_MAGICAL;
    out->attackTypeCode = attackType;
    out->baseKineticEnergy = kinetic;
    out->actualKineticEnergy = actualEnergy;
    out->requiredMana = requiredMana;
    out->stepEnergy = stepEnergy;
    if (subtype == PROJECTILE_SUBTYPE_POISON_BOLT ||
        subtype == PROJECTILE_SUBTYPE_POISON_CLOUD) {
        out->poisonAttack = out->impactAttack;
    }
    out->decrementsActionHandCharges = 1;
    return 1;
}

int dm1_v1_action_climb_down_plan_f0407_pc34(
    const DM1_ActionClimbDownInputPc34* in,
    DM1_ActionClimbDownPlanPc34* out) {
    if (!in || !out) return 0;
    out->valid = 1;
    out->performed = 1;
    out->shouldAttemptMove = 0;
    out->shouldApplyMove = 0;
    out->cancelActionDisable = 0;
    /* ReDMCSB: MENU.C F0407 lines 1548-1565. CLIMB DOWN starts with
     * ActionPerformed TRUE. If the party-facing square is not a pit, or the
     * later PC34 group-over-pit guard blocks it, only ActionDisabledTicks is
     * cleared (BUG0_79); stamina and G0497 XP remain in the common tail. */
    if (!in->frontSquareIsPit || in->frontSquareHasGroup) {
        out->cancelActionDisable = 1;
        return 1;
    }
    if (!in->movementAttempted) {
        out->shouldAttemptMove = 1;
        return 1;
    }
    if (in->movementSucceeded) {
        out->shouldApplyMove = 1;
    } else {
        out->cancelActionDisable = 1;
    }
    return 1;
}

int dm1_v1_action_flip_plan_f0407_pc34(
    const DM1_ActionFlipInputPc34* in,
    DM1_ActionFlipPlanPc34* out) {
    int draw;
    if (!in || !out) return 0;
    draw = in->randomDraw;
    if (draw < 0) draw = 0;
    out->valid = 1;
    out->performed = 1;
    /* ReDMCSB: MENU.C F0407 C005_ACTION_FLIP lines 1398-1440 prints HEADS
     * when M005_RANDOM(2) is nonzero and TAILS when it is zero. */
    out->comesUpHeads = (draw & 1) ? 1 : 0;
    return 1;
}

int dm1_v1_action_direction_plan_f0407_pc34(
    const DM1_ActionDirectionInputPc34* in,
    DM1_ActionDirectionPlanPc34* out) {
    int dir;
    if (!in || !out) return 0;
    out->valid = 0;
    out->performed = 0;
    out->setChampionDirectionToParty = 0;
    out->targetMapX = in->partyMapX;
    out->targetMapY = in->partyMapY;
    out->throwSide = 0;
    switch (in->actionIndex) {
        case DM1_ACTION_FLUXCAGE:
            /* ReDMCSB: MENU.C F0407 lines 1262-1266 compute L1251/L1252
             * from Champion.Direction before C035 calls F0406 at lines
             * 1494-1497. */
            dir = in->championDirection;
            out->setChampionDirectionToParty = 1;
            break;
        case DM1_ACTION_FUSE:
            /* ReDMCSB: MENU.C F0407 lines 1498-1504 calls F0406, then
             * recomputes the target from G0308_i_PartyDirection. */
            dir = in->partyDirection;
            out->setChampionDirectionToParty = 1;
            break;
        case DM1_ACTION_THROW:
            /* ReDMCSB: MENU.C F0407 lines 1613-1617 calls F0406 and passes
             * side=TRUE to F0328 when Champion.Cell is NEXT(partyDir) or
             * OPPOSITE(partyDir). */
            dir = in->partyDirection;
            out->setChampionDirectionToParty = 1;
            if (((in->partyDirection + 1) & 3) == (in->championCell & 3) ||
                ((in->partyDirection + 2) & 3) == (in->championCell & 3)) {
                out->throwSide = 1;
            }
            break;
        default:
            return 0;
    }
    out->valid = 1;
    out->performed = 1;
    out->targetMapX = in->partyMapX + dm1_step_east_for_dir(dir);
    out->targetMapY = in->partyMapY + dm1_step_north_for_dir(dir);
    return 1;
}

int dm1_v1_action_throw_plan_f0407_pc34(
    const DM1_ActionThrowInputPc34* in,
    DM1_ActionThrowPlanPc34* out) {
    DM1_ActionDirectionInputPc34 dirIn;
    DM1_ActionDirectionPlanPc34 dirPlan;
    if (!in || !out) return 0;
    out->valid = 0;
    out->performed = 0;
    out->noActionHandObject = 0;
    out->setChampionDirectionToParty = 0;
    out->shouldSpawnProjectile = 0;
    out->shouldClearActionHand = 0;
    out->actionEnableSlotOrdinal = 0xFF;
    out->throwSide = 0;

    memset(&dirIn, 0, sizeof(dirIn));
    dirIn.actionIndex = DM1_ACTION_THROW;
    dirIn.partyMapX = in->partyMapX;
    dirIn.partyMapY = in->partyMapY;
    dirIn.partyDirection = in->partyDirection;
    dirIn.championDirection = in->championDirection;
    dirIn.championCell = in->championCell;
    if (!dm1_v1_action_direction_plan_f0407_pc34(&dirIn, &dirPlan) ||
        !dirPlan.valid) {
        return 0;
    }

    out->valid = 1;
    out->setChampionDirectionToParty = dirPlan.setChampionDirectionToParty;
    out->throwSide = dirPlan.throwSide;
    if (!in->actionHandPresent) {
        out->noActionHandObject = 1;
        return 1;
    }
    out->shouldSpawnProjectile = 1;
    if (in->projectileSpawned) {
        /* ReDMCSB: MENU.C F0407 lines 1613-1617 stores action-hand slot
         * ordinal after F0328 accepts the throw; CHAMPION.C F0328 lines
         * 2138-2190 has already removed/spawned the thrown object. */
        out->performed = 1;
        out->shouldClearActionHand = 1;
        out->actionEnableSlotOrdinal = CHAMPION_SLOT_ACTION_HAND;
    }
    return 1;
}

int dm1_v1_action_closed_door_melee_plan_f0407_pc34(
    const DM1_ActionClosedDoorMeleeInputPc34* in,
    DM1_ActionClosedDoorMeleePlanPc34* out) {
    if (!in || !out) return 0;
    out->valid = 1;
    out->isClosedDoorMeleeAction = 0;
    out->performed = 0;
    out->disabledTicksOverride = 0;
    out->destructionDelayTicks = 0;
    /* ReDMCSB: MENU.C F0407 lines 1306-1317 routes only BASH, HACK,
     * BERZERK, KICK, SWING, and CHOP through the closed-door branch before
     * F0402.  That branch sets ActionDisabledTicks=6 and calls F0232 with
     * delay 2, then emits the wooden thud even when F0232 does not destroy
     * the door. */
    switch (in->actionIndex) {
        case DM1_ACTION_BASH:
        case DM1_ACTION_HACK:
        case DM1_ACTION_BERZERK:
        case DM1_ACTION_KICK:
        case DM1_ACTION_SWING:
        case DM1_ACTION_CHOP:
            out->isClosedDoorMeleeAction = 1;
            break;
        default:
            return 1;
    }
    if (in->observedDoorDestructionEvent || in->observedWoodenThudSound) {
        out->performed = 1;
        out->disabledTicksOverride = DM1_DOOR_BASH_DISABLED_TICKS_PC34;
        out->destructionDelayTicks =
            DM1_DOOR_BASH_DESTRUCTION_DELAY_TICKS_PC34;
    }
    return 1;
}

int dm1_v1_action_closed_door_destruction_plan_f0232_pc34(
    const DM1_ActionClosedDoorDestructionInputPc34* in,
    DM1_ActionClosedDoorDestructionPlanPc34* out) {
    if (!in || !out) return 0;
    memset(out, 0, sizeof(*out));
    out->valid = 1;
    out->mapIndex = in->mapIndex;
    out->mapX = in->mapX;
    out->mapY = in->mapY;
    out->destroyedDoorState = 5;
    /* ReDMCSB: PROJEXPL.C F0232 lines 1569-1593 rejects non-destructible
     * doors, then schedules C02 door destruction only when attack reaches the
     * active door-set defense and the target door state is C4 closed.  The
     * closed-door melee branch calls F0232 with delay 2. */
    if (!in->closedDoorState) return 1;
    if (!in->meleeDestructible) return 1;
    if (in->attack < in->defense) return 1;
    if (in->destructionDelayTicks <= 0) return 1;
    out->shouldScheduleDestruction = 1;
    out->fireAtTick =
        in->currentTick + (unsigned int)in->destructionDelayTicks;
    return 1;
}

int dm1_v1_action_f0407_tail_pc34(int actionIndex,
                                  DM1_ActionF0407TailPc34* out) {
    int damageFactor;
    int staminaBase;
    int disabledTicks;
    if (!out) return 0;
    out->valid = 0;
    out->damageFactor = 0;
    out->staminaBase = 0;
    out->disabledTicks = 0;
    out->isMeleeContact = 0;
    out->isPartyShield = 0;
    out->halvesXpOnF0327Failure = 0;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }
    damageFactor = dm1_v1_graphic560_action_damage_factor_get_pc34(actionIndex);
    staminaBase = dm1_v1_graphic560_action_stamina_get_pc34(actionIndex);
    disabledTicks = dm1_v1_action_disabled_ticks_f0407_pc34(actionIndex);
    if (damageFactor < 0 || staminaBase < 0) return 0;
    out->valid = 1;
    out->damageFactor = damageFactor;
    out->staminaBase = staminaBase;
    out->disabledTicks = disabledTicks;
    out->isMeleeContact = dm1_v1_action_is_melee_contact_f0407_pc34(actionIndex);
    out->isPartyShield = dm1_v1_action_is_party_shield_f0407_pc34(actionIndex);
    out->halvesXpOnF0327Failure =
        dm1_v1_action_halves_xp_on_f0327_failure_pc34(actionIndex);
    return 1;
}
