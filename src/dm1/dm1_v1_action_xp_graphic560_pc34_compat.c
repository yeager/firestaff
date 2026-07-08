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

#define DM1_THING_TYPE_WEAPON 5
#define DM1_THING_TYPE_ARMOUR 6
#define DM1_THING_TYPE_JUNK 10
#define DM1_JUNK_MAGICAL_BOX_BLUE 42
#define DM1_JUNK_MAGICAL_BOX_GREEN 43
#define DM1_STATUS_THIEVES_EYE 73
#define DM1_STATUS_SPELL_SHIELD 77
#define DM1_STATUS_FIRE_SHIELD 78

/* ReDMCSB CHAMPION.C F0304 line ~874: base skill = (sub - 4) >> 2.
 * For base skills (0..3) the mapping is identity. */
static int sub_skill_base_index(int skillIndex) {
    if (skillIndex < 0 || skillIndex >= 20) return 0;
    if (skillIndex < 4) return skillIndex;
    return (skillIndex - 4) >> 2;
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
