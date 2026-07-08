#include "dm1_v1_action_xp_graphic560_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK_EQ(actual, expected, label) do { \
    int _a = (actual); \
    int _e = (expected); \
    if (_a != _e) { \
        fprintf(stderr, "FAIL %s: expected %d got %d\n", label, _e, _a); \
        ++g_failures; \
    } \
} while (0)

static void test_melee_contact_gate(void) {
    DM1_ActionF0407TailPc34 tail;
    CHECK_EQ(dm1_v1_action_f0407_tail_pc34(DM1_ACTION_BLOCK, &tail), 1,
             "block tail builds");
    CHECK_EQ(tail.damageFactor, 15, "block damage factor");
    CHECK_EQ(tail.isMeleeContact, 0, "block is not contact melee");

    CHECK_EQ(dm1_v1_action_f0407_tail_pc34(DM1_ACTION_PARRY, &tail), 1,
             "parry tail builds");
    CHECK_EQ(tail.damageFactor, 8, "parry damage factor");
    CHECK_EQ(tail.isMeleeContact, 1, "parry is contact melee");

    CHECK_EQ(dm1_v1_action_f0407_tail_pc34(DM1_ACTION_FIREBALL, &tail), 1,
             "fireball tail builds");
    CHECK_EQ(tail.isMeleeContact, 0, "fireball is not contact melee");
}

static void test_stamina_and_special_flags(void) {
    DM1_ActionF0407TailPc34 tail;
    CHECK_EQ(dm1_v1_action_f0407_tail_pc34(DM1_ACTION_CHOP, &tail), 1,
             "chop tail builds");
    CHECK_EQ(tail.staminaBase, 10, "chop stamina base");
    CHECK_EQ(tail.disabledTicks, 8, "chop disabled ticks");
    CHECK_EQ(dm1_v1_action_stamina_cost_f0407_pc34(
                 DM1_ACTION_CHOP, 1, 100u), 11,
             "chop stamina deterministic jitter");

    CHECK_EQ(dm1_v1_action_f0407_tail_pc34(DM1_ACTION_SPELLSHIELD, &tail), 1,
             "spellshield tail builds");
    CHECK_EQ(tail.isPartyShield, 1, "spellshield party shield");
    CHECK_EQ(tail.halvesXpOnF0327Failure, 0,
             "spellshield no f0327 xp halving");

    CHECK_EQ(dm1_v1_action_f0407_tail_pc34(DM1_ACTION_INVOKE, &tail), 1,
             "invoke tail builds");
    CHECK_EQ(tail.halvesXpOnF0327Failure, 1,
             "invoke f0327 xp halving");
    CHECK_EQ(dm1_v1_action_halves_xp_on_f0327_failure_pc34(DM1_ACTION_SPIT),
             1, "spit f0327 xp halving");
    CHECK_EQ(dm1_v1_action_halves_xp_on_f0327_failure_pc34(DM1_ACTION_SHOOT),
             0, "shoot no f0327 xp halving");
}

static void test_tail_adjustments(void) {
    DM1_ActionF0407TailAdjustInputPc34 in;
    DM1_ActionF0407TailAdjustPc34 out;

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_SPELLSHIELD;
    in.performed = 0;
    in.actionExperienceGain = 20;
    in.disabledTicks = 8;
    CHECK_EQ(dm1_v1_action_adjust_f0407_tail_pc34(&in, &out), 1,
             "spellshield adjust builds");
    CHECK_EQ(out.actionExperienceGain, 5, "spellshield failure quarters xp");
    CHECK_EQ(out.disabledTicks, 4, "spellshield failure halves ticks");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_FIREBALL;
    in.performed = 0;
    in.actionExperienceGain = 7;
    in.disabledTicks = 6;
    CHECK_EQ(dm1_v1_action_adjust_f0407_tail_pc34(&in, &out), 1,
             "fireball adjust builds");
    CHECK_EQ(out.actionExperienceGain, 3, "fireball failure halves xp");
    CHECK_EQ(out.disabledTicks, 6, "fireball failure keeps ticks");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_SHOOT;
    in.performed = 0;
    in.actionExperienceGain = 9;
    in.disabledTicks = 4;
    CHECK_EQ(dm1_v1_action_adjust_f0407_tail_pc34(&in, &out), 1,
             "shoot adjust builds");
    CHECK_EQ(out.actionExperienceGain, 0, "shoot failure clears xp");
    CHECK_EQ(out.disabledTicks, 4, "shoot failure keeps ticks");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_CLIMB_DOWN;
    in.performed = 1;
    in.actionExperienceGain = 8;
    in.disabledTicks = 6;
    in.cancelActionDisable = 1;
    CHECK_EQ(dm1_v1_action_adjust_f0407_tail_pc34(&in, &out), 1,
             "climb down adjust builds");
    CHECK_EQ(out.actionExperienceGain, 8, "climb down keeps xp");
    CHECK_EQ(out.disabledTicks, 0, "climb down cancels ticks");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_PARRY;
    in.performed = 0;
    in.actionExperienceGain = 6;
    in.disabledTicks = 5;
    in.meleeFailureTail = 1;
    CHECK_EQ(dm1_v1_action_adjust_f0407_tail_pc34(&in, &out), 1,
             "parry melee failure adjust builds");
    CHECK_EQ(out.actionExperienceGain, 3, "parry melee failure halves xp");
    CHECK_EQ(out.disabledTicks, 2, "parry melee failure halves ticks");
}

static void test_f0405_charge_plan(void) {
    DM1_ActionF0405ChargeInputPc34 in;
    DM1_ActionF0405ChargePlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.thingType = 5;
    in.thingIndex = 12;
    in.currentChargeCount = 2;
    CHECK_EQ(dm1_v1_action_f0405_charge_plan_pc34(&in, &out), 1,
             "weapon charge plan builds");
    CHECK_EQ(out.shouldDecrement, 1, "weapon charge decrements");
    CHECK_EQ(out.thingIndex, 12, "weapon charge index preserved");

    memset(&in, 0, sizeof(in));
    in.thingType = 6;
    in.thingIndex = 4;
    in.currentChargeCount = 0;
    CHECK_EQ(dm1_v1_action_f0405_charge_plan_pc34(&in, &out), 1,
             "armour charge plan builds");
    CHECK_EQ(out.shouldDecrement, 0, "zero charge does not decrement");

    memset(&in, 0, sizeof(in));
    in.thingType = 10;
    in.thingIndex = 3;
    in.currentChargeCount = 1;
    CHECK_EQ(dm1_v1_action_f0405_charge_plan_pc34(&in, &out), 1,
             "junk charge plan builds");
    CHECK_EQ(out.shouldDecrement, 1, "junk charge decrements");

    memset(&in, 0, sizeof(in));
    in.thingType = 2;
    in.thingIndex = 1;
    in.currentChargeCount = 1;
    CHECK_EQ(dm1_v1_action_f0405_charge_plan_pc34(&in, &out), 0,
             "unsupported thing charge plan rejected");
}

static void test_freeze_life_plan(void) {
    DM1_ActionFreezeLifeInputPc34 in;
    DM1_ActionFreezeLifePlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.currentFreezeLifeTicks = 10;
    in.actionHandJunkType = 42;
    CHECK_EQ(dm1_v1_action_freeze_life_plan_f0407_pc34(&in, &out), 1,
             "blue box freeze plan builds");
    CHECK_EQ(out.addTicks, 30, "blue box adds 30");
    CHECK_EQ(out.newFreezeLifeTicks, 40, "blue box total");
    CHECK_EQ(out.consumesActionHandObject, 1, "blue box consumed");
    CHECK_EQ(out.decrementsActionHandCharges, 0, "blue box no charge decrement");

    memset(&in, 0, sizeof(in));
    in.currentFreezeLifeTicks = 100;
    in.actionHandJunkType = 43;
    CHECK_EQ(dm1_v1_action_freeze_life_plan_f0407_pc34(&in, &out), 1,
             "green box freeze plan builds");
    CHECK_EQ(out.addTicks, 125, "green box adds 125");
    CHECK_EQ(out.newFreezeLifeTicks, 200, "green box caps total");
    CHECK_EQ(out.consumesActionHandObject, 1, "green box consumed");

    memset(&in, 0, sizeof(in));
    in.currentFreezeLifeTicks = 140;
    in.actionHandJunkType = -1;
    CHECK_EQ(dm1_v1_action_freeze_life_plan_f0407_pc34(&in, &out), 1,
             "ordinary freeze plan builds");
    CHECK_EQ(out.addTicks, 70, "ordinary freeze adds 70");
    CHECK_EQ(out.newFreezeLifeTicks, 200, "ordinary freeze caps total");
    CHECK_EQ(out.consumesActionHandObject, 0, "ordinary freeze no consume");
    CHECK_EQ(out.decrementsActionHandCharges, 1,
             "ordinary freeze decrements action charges");
}

static void test_heal_plan(void) {
    DM1_ActionHealInputPc34 in;
    DM1_ActionHealPlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.currentHealth = 100;
    in.maximumHealth = 100;
    in.currentMana = 8;
    in.healSkillLevel = 4;
    CHECK_EQ(dm1_v1_action_heal_plan_f0407_pc34(&in, &out), 1,
             "full health heal plan builds");
    CHECK_EQ(out.performed, 1, "full health heal performed");
    CHECK_EQ(out.alreadyFullHealth, 1, "full health flag");
    CHECK_EQ(out.healedAmount, 0, "full health heals zero");

    memset(&in, 0, sizeof(in));
    in.currentHealth = 80;
    in.maximumHealth = 100;
    in.currentMana = 0;
    in.healSkillLevel = 5;
    CHECK_EQ(dm1_v1_action_heal_plan_f0407_pc34(&in, &out), 1,
             "no mana heal plan builds");
    CHECK_EQ(out.performed, 1, "no mana heal performed");
    CHECK_EQ(out.noMana, 1, "no mana flag");
    CHECK_EQ(out.healedAmount, 0, "no mana heals zero");

    memset(&in, 0, sizeof(in));
    in.currentHealth = 80;
    in.maximumHealth = 100;
    in.currentMana = 5;
    in.healSkillLevel = 6;
    CHECK_EQ(dm1_v1_action_heal_plan_f0407_pc34(&in, &out), 1,
             "normal heal plan builds");
    CHECK_EQ(out.performed, 1, "normal heal performed");
    CHECK_EQ(out.healedAmount, 18, "normal heal amount");
    CHECK_EQ(out.manaCost, 5, "normal heal mana cost clamps");
    CHECK_EQ(out.actionExperienceGain, 8, "normal heal xp");

    memset(&in, 0, sizeof(in));
    in.currentHealth = 99;
    in.maximumHealth = 100;
    in.currentMana = 10;
    in.healSkillLevel = 0;
    CHECK_EQ(dm1_v1_action_heal_plan_f0407_pc34(&in, &out), 1,
             "minimum skill heal plan builds");
    CHECK_EQ(out.healedAmount, 1, "minimum skill heals at least one");
    CHECK_EQ(out.manaCost, 2, "minimum skill costs one cycle");
    CHECK_EQ(out.actionExperienceGain, 4, "minimum skill xp");
}

static void test_light_and_window_plans(void) {
    DM1_ActionLightPlanPc34 light;
    DM1_ActionWindowInputPc34 in;
    DM1_ActionWindowPlanPc34 out;

    CHECK_EQ(dm1_v1_action_light_plan_f0407_pc34(&light), 1,
             "light plan builds");
    CHECK_EQ(light.magicalLightAmountDelta, 12, "light amount delta");
    CHECK_EQ(light.eventLightPower, -2, "light event power");
    CHECK_EQ(light.eventDelayTicks, 2500, "light event delay");
    CHECK_EQ(light.decrementsActionHandCharges, 1,
             "light decrements charges");

    CHECK_EQ(dm1_v1_action_window_random_range_f0407_pc34(5), 13,
             "window random range");
    CHECK_EQ(dm1_v1_action_window_random_range_f0407_pc34(-4), 8,
             "window negative skill range clamp");

    memset(&in, 0, sizeof(in));
    in.earthSkillLevel = 5;
    in.randomDraw = 12;
    CHECK_EQ(dm1_v1_action_window_plan_f0407_pc34(&in, &out), 1,
             "window plan builds");
    CHECK_EQ(out.randomRange, 13, "window plan range");
    CHECK_EQ(out.durationTicks, 17, "window duration");
    CHECK_EQ(out.statusEventType, 73, "window event type");
    CHECK_EQ(out.incrementsThievesEyeCount, 1, "window increments count");
    CHECK_EQ(out.decrementsActionHandCharges, 1,
             "window decrements charges");

    memset(&in, 0, sizeof(in));
    in.earthSkillLevel = 1;
    in.randomDraw = 100;
    CHECK_EQ(dm1_v1_action_window_plan_f0407_pc34(&in, &out), 1,
             "window plan clamps draw");
    CHECK_EQ(out.randomRange, 9, "window clamp range");
    CHECK_EQ(out.durationTicks, 6, "window clamped duration");
}

static void test_shield_plan(void) {
    DM1_ActionShieldInputPc34 in;
    DM1_ActionShieldPlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.isSpellShield = 1;
    in.useMana = 1;
    in.currentMana = 10;
    in.baseTicks = 280;
    CHECK_EQ(dm1_v1_action_shield_plan_f0403_pc34(&in, &out), 1,
             "spell shield plan builds");
    CHECK_EQ(out.successful, 1, "spell shield succeeds");
    CHECK_EQ(out.manaCost, 4, "spell shield mana cost");
    CHECK_EQ(out.remainingMana, 6, "spell shield remaining mana");
    CHECK_EQ(out.statusEventType, 77, "spell shield event type");
    CHECK_EQ(out.eventDelayTicks, 280, "spell shield event delay");
    CHECK_EQ(out.defenseDelta, 8, "spell shield defense");
    CHECK_EQ(out.newShieldDefense, 8, "spell shield new defense");
    CHECK_EQ(out.decrementsActionHandChargesOnSuccess, 1,
             "spell shield decrements charges");

    memset(&in, 0, sizeof(in));
    in.isSpellShield = 0;
    in.useMana = 1;
    in.currentMana = 3;
    in.baseTicks = 280;
    CHECK_EQ(dm1_v1_action_shield_plan_f0403_pc34(&in, &out), 1,
             "low mana fire shield plan builds");
    CHECK_EQ(out.successful, 0, "low mana fire shield fails tail");
    CHECK_EQ(out.manaCost, 3, "low mana fire shield drains mana");
    CHECK_EQ(out.remainingMana, 0, "low mana fire shield remaining mana");
    CHECK_EQ(out.statusEventType, 78, "fire shield event type");
    CHECK_EQ(out.eventDelayTicks, 140, "low mana fire shield half delay");
    CHECK_EQ(out.defenseDelta, 4, "low mana fire shield defense");
    CHECK_EQ(out.newShieldDefense, 4, "low mana fire shield new defense");

    memset(&in, 0, sizeof(in));
    in.isSpellShield = 1;
    in.useMana = 1;
    in.currentMana = 8;
    in.baseTicks = 280;
    in.currentShieldDefense = 64;
    CHECK_EQ(dm1_v1_action_shield_plan_f0403_pc34(&in, &out), 1,
             "high existing shield plan builds");
    CHECK_EQ(out.defenseDelta, 2, "high shield quarters defense");
    CHECK_EQ(out.newShieldDefense, 66, "high shield new defense");

    memset(&in, 0, sizeof(in));
    in.isSpellShield = 1;
    in.useMana = 1;
    in.currentMana = 0;
    in.baseTicks = 280;
    CHECK_EQ(dm1_v1_action_shield_plan_f0403_pc34(&in, &out), 1,
             "zero mana shield plan builds");
    CHECK_EQ(out.successful, 0, "zero mana shield fails");
    CHECK_EQ(out.eventDelayTicks, 0, "zero mana shield no event");
    CHECK_EQ(out.defenseDelta, 0, "zero mana shield no defense");
    CHECK_EQ(out.decrementsActionHandChargesOnSuccess, 0,
             "zero mana shield no charge decrement");
}

static void test_fright_plan(void) {
    DM1_ActionFrightInputPc34 in;
    DM1_ActionFrightPlanPc34 out;

    CHECK_EQ(dm1_v1_action_fright_random_range_f0401_pc34(
                 DM1_ACTION_WAR_CRY, 4), 7,
             "war cry fright random range");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_WAR_CRY;
    in.influenceSkillLevel = 4;
    in.fearResistance = 2;
    in.randomDraw = 2;
    in.movementTicks = 4;
    CHECK_EQ(dm1_v1_action_fright_plan_f0401_pc34(&in, &out), 1,
             "war cry fright plan builds");
    CHECK_EQ(out.baseFrightAmount, 3, "war cry base fright");
    CHECK_EQ(out.totalFrightAmount, 7, "war cry total fright");
    CHECK_EQ(out.influenceExperience, 12, "war cry influence xp");
    CHECK_EQ(out.resisted, 0, "war cry not resisted on equal roll");
    CHECK_EQ(out.frightened, 1, "war cry frightens");
    CHECK_EQ(out.fleeDelayTicks, 14, "war cry flee delay");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_CONFUSE;
    in.influenceSkillLevel = 3;
    in.fearResistance = 8;
    in.randomDraw = 2;
    in.movementTicks = 6;
    CHECK_EQ(dm1_v1_action_fright_plan_f0401_pc34(&in, &out), 1,
             "confuse resisted plan builds");
    CHECK_EQ(out.baseFrightAmount, 12, "confuse base fright");
    CHECK_EQ(out.totalFrightAmount, 15, "confuse total fright");
    CHECK_EQ(out.influenceExperience, 22, "confuse resisted halves xp");
    CHECK_EQ(out.resisted, 1, "confuse resisted");
    CHECK_EQ(out.frightened, 0, "confuse resisted no fright");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_BLOW_HORN;
    in.influenceSkillLevel = 0;
    in.fearResistance = 15;
    in.randomDraw = 5;
    in.movementTicks = 2;
    CHECK_EQ(dm1_v1_action_fright_plan_f0401_pc34(&in, &out), 1,
             "immune fear plan builds");
    CHECK_EQ(out.influenceExperience, 10, "immune fear halves xp");
    CHECK_EQ(out.resisted, 1, "immune fear resisted");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_LIGHT;
    CHECK_EQ(dm1_v1_action_fright_plan_f0401_pc34(&in, &out), 0,
             "non fright action rejected");
}

static void test_projectile_spell_plan(void) {
    DM1_ActionProjectileSpellInputPc34 in;
    DM1_ActionProjectileSpellPlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_FIREBALL;
    in.skillLevel = 3;
    in.currentMana = 10;
    in.maximumMana = 64;
    CHECK_EQ(dm1_v1_action_projectile_spell_plan_f0407_pc34(&in, &out), 1,
             "fireball projectile plan builds");
    CHECK_EQ(out.actionSkillIndex, 16, "fireball uses fire skill");
    CHECK_EQ(out.subtype, PROJECTILE_SUBTYPE_FIREBALL, "fireball subtype");
    CHECK_EQ(out.category, PROJECTILE_CATEGORY_MAGICAL, "fireball category");
    CHECK_EQ(out.attackTypeCode, COMBAT_ATTACK_FIRE, "fireball attack type");
    CHECK_EQ(out.baseKineticEnergy, 150, "fireball base kinetic");
    CHECK_EQ(out.actualKineticEnergy, 150, "fireball actual kinetic");
    CHECK_EQ(out.requiredMana, 4, "fireball required mana");
    CHECK_EQ(out.manaCost, 4, "fireball mana cost");
    CHECK_EQ(out.remainingMana, 6, "fireball remaining mana");
    CHECK_EQ(out.stepEnergy, 2, "fireball step energy");
    CHECK_EQ(out.impactAttack, 90, "fireball impact attack");
    CHECK_EQ(out.decrementsActionHandCharges, 1,
             "fireball decrements charges");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_SPIT;
    in.skillLevel = 0;
    in.currentMana = 2;
    in.maximumMana = 16;
    CHECK_EQ(dm1_v1_action_projectile_spell_plan_f0407_pc34(&in, &out), 1,
             "low mana spit projectile plan builds");
    CHECK_EQ(out.actionSkillIndex, 16, "spit uses fire skill");
    CHECK_EQ(out.subtype, PROJECTILE_SUBTYPE_FIREBALL, "spit subtype");
    CHECK_EQ(out.baseKineticEnergy, 250, "spit base kinetic");
    CHECK_EQ(out.requiredMana, 7, "spit required mana");
    CHECK_EQ(out.manaCost, 2, "spit spends available mana");
    CHECK_EQ(out.remainingMana, 0, "spit remaining mana");
    CHECK_EQ(out.actualKineticEnergy, 71, "spit low mana scales kinetic");
    CHECK_EQ(out.stepEnergy, 8, "spit step energy from max mana");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_INVOKE;
    in.skillLevel = 6;
    in.currentMana = 5;
    in.maximumMana = 80;
    in.invokeEnergyRoll = 27;
    in.invokeFamilyRoll = 1;
    CHECK_EQ(dm1_v1_action_projectile_spell_plan_f0407_pc34(&in, &out), 1,
             "invoke projectile plan builds");
    CHECK_EQ(out.actionSkillIndex, 3, "invoke uses wizard skill");
    CHECK_EQ(out.subtype, PROJECTILE_SUBTYPE_POISON_CLOUD,
             "invoke family roll poison cloud");
    CHECK_EQ(out.attackTypeCode, COMBAT_ATTACK_NORMAL,
             "invoke poison keeps normal attack type");
    CHECK_EQ(out.baseKineticEnergy, 127, "invoke base kinetic from roll");
    CHECK_EQ(out.requiredMana, 1, "invoke skilled required mana");
    CHECK_EQ(out.manaCost, 1, "invoke mana cost");
    CHECK_EQ(out.remainingMana, 4, "invoke remaining mana");
    CHECK_EQ(out.actualKineticEnergy, 127, "invoke actual kinetic");
    CHECK_EQ(out.stepEnergy, 2, "invoke step energy max mana clamp");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_LIGHT;
    CHECK_EQ(dm1_v1_action_projectile_spell_plan_f0407_pc34(&in, &out), 0,
             "non projectile action rejected");
}

static void test_invalid_action(void) {
    DM1_ActionF0407TailPc34 tail;
    CHECK_EQ(dm1_v1_action_f0407_tail_pc34(-1, &tail), 0,
             "negative action invalid");
    CHECK_EQ(tail.valid, 0, "negative action leaves invalid");
    CHECK_EQ(dm1_v1_action_f0407_tail_pc34(DM1_GRAPHIC560_ACTION_COUNT, &tail),
             0, "past-end action invalid");
    CHECK_EQ(dm1_v1_action_stamina_cost_f0407_pc34(
                 DM1_GRAPHIC560_ACTION_COUNT, 0, 0u), 0,
             "invalid stamina cost zero");
}

int main(void) {
    test_melee_contact_gate();
    test_stamina_and_special_flags();
    test_tail_adjustments();
    test_f0405_charge_plan();
    test_freeze_life_plan();
    test_heal_plan();
    test_light_and_window_plans();
    test_shield_plan();
    test_fright_plan();
    test_projectile_spell_plan();
    test_invalid_action();
    if (g_failures) {
        fprintf(stderr, "test_dm1_v1_action_f0407_tail_pc34_compat: %d failures\n",
                g_failures);
        return 1;
    }
    printf("test_dm1_v1_action_f0407_tail_pc34_compat: PASS\n");
    return 0;
}
