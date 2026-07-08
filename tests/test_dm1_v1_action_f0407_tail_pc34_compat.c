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
    test_invalid_action();
    if (g_failures) {
        fprintf(stderr, "test_dm1_v1_action_f0407_tail_pc34_compat: %d failures\n",
                g_failures);
        return 1;
    }
    printf("test_dm1_v1_action_f0407_tail_pc34_compat: PASS\n");
    return 0;
}
