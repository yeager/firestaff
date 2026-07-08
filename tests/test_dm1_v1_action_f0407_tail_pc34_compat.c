#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_melee_action_f0402_pc34_compat.h"

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
    DM1_ActionF0407PreludeInputPc34 preludeIn;
    DM1_ActionF0407PreludePlanPc34 preludeOut;
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

    memset(&preludeIn, 0, sizeof(preludeIn));
    preludeIn.actionIndex = DM1_ACTION_CHOP;
    preludeIn.championIndex = 1;
    preludeIn.gameTick = 100u;
    CHECK_EQ(dm1_v1_action_prelude_plan_f0407_pc34(
                 &preludeIn, &preludeOut), 1,
             "chop prelude builds");
    CHECK_EQ(preludeOut.valid, 1, "chop prelude valid");
    CHECK_EQ(preludeOut.actionExperienceGain, 10, "chop prelude xp");
    CHECK_EQ(preludeOut.disabledTicks, 8, "chop prelude disabled");
    CHECK_EQ(preludeOut.staminaCost, 11, "chop prelude stamina");
    CHECK_EQ(preludeOut.isMeleeContact, 1, "chop prelude melee");

    preludeIn.actionIndex = DM1_ACTION_BLOCK;
    CHECK_EQ(dm1_v1_action_prelude_plan_f0407_pc34(
                 &preludeIn, &preludeOut), 1,
             "block prelude builds");
    CHECK_EQ(preludeOut.isMeleeContact, 0, "block prelude not melee");
    CHECK_EQ(preludeOut.disabledTicks, 6, "block prelude disabled");

    preludeIn.actionIndex = 99;
    CHECK_EQ(dm1_v1_action_prelude_plan_f0407_pc34(
                 &preludeIn, &preludeOut), 0,
             "invalid prelude rejected");
}

static void test_tail_adjustments(void) {
    DM1_ActionF0407TailAdjustInputPc34 in;
    DM1_ActionF0407TailAdjustPc34 out;
    DM1_ActionF0407CompletionInputPc34 completionIn;
    DM1_ActionF0407CompletionPlanPc34 completionOut;

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

    memset(&completionIn, 0, sizeof(completionIn));
    completionIn.actionIndex = DM1_ACTION_PARRY;
    completionIn.performed = 0;
    completionIn.actionExperienceGain = 6;
    completionIn.disabledTicks = 5;
    completionIn.meleeFailureTail = 1;
    CHECK_EQ(dm1_v1_action_completion_plan_f0407_pc34(
                 &completionIn, &completionOut), 1,
             "parry completion plan builds");
    CHECK_EQ(completionOut.actionExperienceGain, 3,
             "parry completion halves xp");
    CHECK_EQ(completionOut.disabledTicks, 2,
             "parry completion halves ticks");
    CHECK_EQ(completionOut.actionDisabledIndex, DM1_ACTION_PARRY,
             "parry completion stores action index");

    memset(&completionIn, 0, sizeof(completionIn));
    completionIn.actionIndex = DM1_ACTION_THROW;
    completionIn.performed = 1;
    completionIn.actionExperienceGain = 0;
    completionIn.disabledTicks = 4;
    completionIn.pendingActionEnableSlotOrdinal = 1;
    CHECK_EQ(dm1_v1_action_completion_plan_f0407_pc34(
                 &completionIn, &completionOut), 1,
             "throw completion plan builds");
    CHECK_EQ(completionOut.actionEnableSlotOrdinal, 1,
             "throw completion keeps action-hand slot");

    completionIn.disabledTicks = 0;
    CHECK_EQ(dm1_v1_action_completion_plan_f0407_pc34(
                 &completionIn, &completionOut), 1,
             "throw f0328-owned completion plan builds");
    CHECK_EQ(completionOut.preservesExistingActionDisable, 1,
             "throw preserves f0328 action disable");
    CHECK_EQ(completionOut.actionEnableSlotOrdinal, 1,
             "throw f0328 completion keeps action-hand slot");

    memset(&completionIn, 0, sizeof(completionIn));
    completionIn.actionIndex = DM1_ACTION_SHOOT;
    completionIn.performed = 1;
    completionIn.disabledTicks = 0;
    completionIn.pendingShootReadyHandRefill = 1;
    CHECK_EQ(dm1_v1_action_completion_plan_f0407_pc34(
                 &completionIn, &completionOut), 1,
             "shoot zero completion plan builds");
    CHECK_EQ(completionOut.actionDisabledIndex, 0xFF,
             "shoot zero completion no disabled action");
    CHECK_EQ(completionOut.shouldRefillReadyHandNow, 1,
             "shoot zero completion refills now");
}

static void test_action_state_plans(void) {
    DM1_ActionDefenseInputPc34 defenseIn;
    DM1_ActionDefensePlanPc34 defenseOut;
    DM1_ActionDisableInputPc34 disableIn;
    DM1_ActionDisablePlanPc34 disableOut;

    memset(&defenseIn, 0, sizeof(defenseIn));
    defenseIn.actionIndex = DM1_ACTION_BLOCK;
    CHECK_EQ(dm1_v1_action_defense_apply_plan_f0407_pc34(
                 &defenseIn, &defenseOut), 1,
             "block defense apply builds");
    CHECK_EQ(defenseOut.valid, 1, "block defense apply valid");
    CHECK_EQ(defenseOut.defenseDelta, 36, "block defense delta");
    CHECK_EQ(defenseOut.resultingActionIndex, DM1_ACTION_BLOCK,
             "block defense sets action index");

    CHECK_EQ(dm1_v1_action_defense_remove_plan_f0407_pc34(
                 &defenseIn, &defenseOut), 1,
             "block defense remove builds");
    CHECK_EQ(defenseOut.defenseDelta, -36, "block defense removed");
    CHECK_EQ(defenseOut.resultingActionIndex, 0xFF,
             "defense remove clears action index");

    memset(&disableIn, 0, sizeof(disableIn));
    disableIn.actionIndex = DM1_ACTION_CHOP;
    disableIn.disabledTicks = 8;
    CHECK_EQ(dm1_v1_action_disable_plan_f0407_pc34(
                 &disableIn, &disableOut), 1,
             "disable plan builds");
    CHECK_EQ(disableOut.valid, 1, "disable plan valid");
    CHECK_EQ(disableOut.disabledTicks, 8, "disable ticks kept");
    CHECK_EQ(disableOut.actionDisabledIndex, DM1_ACTION_CHOP,
             "disable stores action index");
    CHECK_EQ(disableOut.actionEnableSlotOrdinal, 0xFF,
             "disable action slot ordinal");
    CHECK_EQ(disableOut.shouldRefillReadyHandNow, 0,
             "nonzero disable does not refill now");

    disableIn.pendingActionEnableSlotOrdinal = 1;
    CHECK_EQ(dm1_v1_action_disable_plan_f0407_pc34(
                 &disableIn, &disableOut), 1,
             "disable preserves pending slot ordinal");
    CHECK_EQ(disableOut.actionEnableSlotOrdinal, 1,
             "disable keeps action-hand slot ordinal");

    disableIn.disabledTicks = 0;
    disableIn.pendingShootReadyHandRefill = 1;
    CHECK_EQ(dm1_v1_action_disable_plan_f0407_pc34(
                 &disableIn, &disableOut), 1,
             "zero disable plan builds");
    CHECK_EQ(disableOut.actionDisabledIndex, 0xFF,
             "zero disable clears action index");
    CHECK_EQ(disableOut.shouldRefillReadyHandNow, 1,
             "zero disable refills now");
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
    DM1_ActionFreezeLifeObjectInputPc34 objectIn;
    DM1_ActionFreezeLifeObjectPlanPc34 objectOut;

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

    memset(&objectIn, 0, sizeof(objectIn));
    objectIn.currentFreezeLifeTicks = 10;
    objectIn.actionHandThingType = 10;
    objectIn.actionHandThingIndex = 3;
    objectIn.actionHandChargeCount = 2;
    objectIn.actionHandJunkType = 42;
    CHECK_EQ(dm1_v1_action_freeze_life_object_plan_f0407_pc34(
                 &objectIn, &objectOut), 1,
             "blue box object freeze plan builds");
    CHECK_EQ(objectOut.newFreezeLifeTicks, 40, "blue object total");
    CHECK_EQ(objectOut.shouldRemoveActionHandObject, 1,
             "blue object remove action hand");
    CHECK_EQ(objectOut.shouldDecrementActionHandCharges, 0,
             "blue object no charge decrement");
    CHECK_EQ(objectOut.targetThingIndex, 3, "blue object index preserved");

    memset(&objectIn, 0, sizeof(objectIn));
    objectIn.currentFreezeLifeTicks = 100;
    objectIn.actionHandThingType = 10;
    objectIn.actionHandThingIndex = 4;
    objectIn.actionHandChargeCount = 1;
    objectIn.actionHandJunkType = 43;
    CHECK_EQ(dm1_v1_action_freeze_life_object_plan_f0407_pc34(
                 &objectIn, &objectOut), 1,
             "green box object freeze plan builds");
    CHECK_EQ(objectOut.newFreezeLifeTicks, 200, "green object caps total");
    CHECK_EQ(objectOut.shouldRemoveActionHandObject, 1,
             "green object remove action hand");

    memset(&objectIn, 0, sizeof(objectIn));
    objectIn.currentFreezeLifeTicks = 140;
    objectIn.actionHandThingType = 5;
    objectIn.actionHandThingIndex = 12;
    objectIn.actionHandChargeCount = 2;
    objectIn.actionHandJunkType = -1;
    CHECK_EQ(dm1_v1_action_freeze_life_object_plan_f0407_pc34(
                 &objectIn, &objectOut), 1,
             "weapon object freeze plan builds");
    CHECK_EQ(objectOut.newFreezeLifeTicks, 200, "weapon object caps total");
    CHECK_EQ(objectOut.shouldRemoveActionHandObject, 0,
             "weapon object not removed");
    CHECK_EQ(objectOut.shouldDecrementActionHandCharges, 1,
             "weapon object decrements charge");
    CHECK_EQ(objectOut.targetThingType, 5, "weapon object type preserved");
    CHECK_EQ(objectOut.targetThingIndex, 12, "weapon object index preserved");

    objectIn.actionHandChargeCount = 0;
    CHECK_EQ(dm1_v1_action_freeze_life_object_plan_f0407_pc34(
                 &objectIn, &objectOut), 1,
             "zero-charge object freeze plan builds");
    CHECK_EQ(objectOut.shouldDecrementActionHandCharges, 0,
             "zero-charge object does not decrement");
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

static void test_climb_down_plan(void) {
    DM1_ActionClimbDownInputPc34 in;
    DM1_ActionClimbDownPlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.frontSquareIsPit = 0;
    CHECK_EQ(dm1_v1_action_climb_down_plan_f0407_pc34(&in, &out), 1,
             "climb no pit plan builds");
    CHECK_EQ(out.performed, 1, "climb no pit remains performed");
    CHECK_EQ(out.cancelActionDisable, 1, "climb no pit cancels disable");
    CHECK_EQ(out.shouldAttemptMove, 0, "climb no pit no move");

    memset(&in, 0, sizeof(in));
    in.frontSquareIsPit = 1;
    in.frontSquareHasGroup = 1;
    CHECK_EQ(dm1_v1_action_climb_down_plan_f0407_pc34(&in, &out), 1,
             "climb group block plan builds");
    CHECK_EQ(out.performed, 1, "climb group block remains performed");
    CHECK_EQ(out.cancelActionDisable, 1, "climb group block cancels disable");
    CHECK_EQ(out.shouldAttemptMove, 0, "climb group block no move");

    memset(&in, 0, sizeof(in));
    in.frontSquareIsPit = 1;
    CHECK_EQ(dm1_v1_action_climb_down_plan_f0407_pc34(&in, &out), 1,
             "climb open pit pre-move plan builds");
    CHECK_EQ(out.shouldAttemptMove, 1, "climb open pit attempts move");
    CHECK_EQ(out.cancelActionDisable, 0, "climb open pit keeps disable before move");

    in.movementAttempted = 1;
    in.movementSucceeded = 1;
    CHECK_EQ(dm1_v1_action_climb_down_plan_f0407_pc34(&in, &out), 1,
             "climb successful move plan builds");
    CHECK_EQ(out.shouldApplyMove, 1, "climb successful move applies move");
    CHECK_EQ(out.cancelActionDisable, 0, "climb successful move keeps disable");

    in.movementSucceeded = 0;
    CHECK_EQ(dm1_v1_action_climb_down_plan_f0407_pc34(&in, &out), 1,
             "climb failed move plan builds");
    CHECK_EQ(out.shouldApplyMove, 0, "climb failed move no apply");
    CHECK_EQ(out.cancelActionDisable, 1, "climb failed move cancels disable");
}

static void test_flip_and_direction_plans(void) {
    DM1_ActionFlipInputPc34 flipIn;
    DM1_ActionFlipPlanPc34 flipOut;
    DM1_ActionDirectionInputPc34 dirIn;
    DM1_ActionDirectionPlanPc34 dirOut;
    DM1_ActionThrowInputPc34 throwIn;
    DM1_ActionThrowPlanPc34 throwOut;

    memset(&flipIn, 0, sizeof(flipIn));
    flipIn.randomDraw = 0;
    CHECK_EQ(dm1_v1_action_flip_plan_f0407_pc34(&flipIn, &flipOut), 1,
             "flip tails plan builds");
    CHECK_EQ(flipOut.performed, 1, "flip tails performed");
    CHECK_EQ(flipOut.comesUpHeads, 0, "flip random zero tails");

    flipIn.randomDraw = 1;
    CHECK_EQ(dm1_v1_action_flip_plan_f0407_pc34(&flipIn, &flipOut), 1,
             "flip heads plan builds");
    CHECK_EQ(flipOut.comesUpHeads, 1, "flip random one heads");

    memset(&dirIn, 0, sizeof(dirIn));
    dirIn.actionIndex = DM1_ACTION_FLUXCAGE;
    dirIn.partyMapX = 10;
    dirIn.partyMapY = 20;
    dirIn.partyDirection = 0;
    dirIn.championDirection = 1;
    CHECK_EQ(dm1_v1_action_direction_plan_f0407_pc34(&dirIn, &dirOut), 1,
             "fluxcage direction plan builds");
    CHECK_EQ(dirOut.setChampionDirectionToParty, 1,
             "fluxcage syncs champion direction");
    CHECK_EQ(dirOut.targetMapX, 11, "fluxcage uses champion dir x");
    CHECK_EQ(dirOut.targetMapY, 20, "fluxcage uses champion dir y");

    memset(&dirIn, 0, sizeof(dirIn));
    dirIn.actionIndex = DM1_ACTION_FUSE;
    dirIn.partyMapX = 10;
    dirIn.partyMapY = 20;
    dirIn.partyDirection = 0;
    dirIn.championDirection = 1;
    CHECK_EQ(dm1_v1_action_direction_plan_f0407_pc34(&dirIn, &dirOut), 1,
             "fuse direction plan builds");
    CHECK_EQ(dirOut.targetMapX, 10, "fuse uses party dir x");
    CHECK_EQ(dirOut.targetMapY, 19, "fuse uses party dir y");

    memset(&dirIn, 0, sizeof(dirIn));
    dirIn.actionIndex = DM1_ACTION_THROW;
    dirIn.partyMapX = 3;
    dirIn.partyMapY = 4;
    dirIn.partyDirection = 0;
    dirIn.championCell = 1;
    CHECK_EQ(dm1_v1_action_direction_plan_f0407_pc34(&dirIn, &dirOut), 1,
             "throw side plan builds");
    CHECK_EQ(dirOut.setChampionDirectionToParty, 1,
             "throw syncs champion direction");
    CHECK_EQ(dirOut.throwSide, 1, "throw side for next cell");

    dirIn.championCell = 3;
    CHECK_EQ(dm1_v1_action_direction_plan_f0407_pc34(&dirIn, &dirOut), 1,
             "throw non-side plan builds");
    CHECK_EQ(dirOut.throwSide, 0, "throw no side for left cell");

    memset(&throwIn, 0, sizeof(throwIn));
    throwIn.partyDirection = 0;
    throwIn.championCell = 1;
    throwIn.actionHandPresent = 0;
    CHECK_EQ(dm1_v1_action_throw_plan_f0407_pc34(&throwIn, &throwOut), 1,
             "throw no-object plan builds");
    CHECK_EQ(throwOut.setChampionDirectionToParty, 1,
             "throw no-object still syncs direction");
    CHECK_EQ(throwOut.noActionHandObject, 1, "throw no-object flagged");
    CHECK_EQ(throwOut.shouldSpawnProjectile, 0,
             "throw no-object does not spawn");

    throwIn.actionHandPresent = 1;
    throwIn.projectileSpawned = 0;
    CHECK_EQ(dm1_v1_action_throw_plan_f0407_pc34(&throwIn, &throwOut), 1,
             "throw pre-spawn plan builds");
    CHECK_EQ(throwOut.shouldSpawnProjectile, 1, "throw tries spawn");
    CHECK_EQ(throwOut.performed, 0, "throw pre-spawn not performed yet");
    CHECK_EQ(throwOut.throwSide, 1, "throw side carried");

    throwIn.projectileSpawned = 1;
    CHECK_EQ(dm1_v1_action_throw_plan_f0407_pc34(&throwIn, &throwOut), 1,
             "throw post-spawn plan builds");
    CHECK_EQ(throwOut.performed, 1, "throw post-spawn performed");
    CHECK_EQ(throwOut.shouldClearActionHand, 1, "throw clears action hand");
    CHECK_EQ(throwOut.actionEnableSlotOrdinal, 1,
             "throw requests action-hand enable slot");
}

static void test_closed_door_melee_plan(void) {
    DM1_ActionClosedDoorMeleeInputPc34 in;
    DM1_ActionClosedDoorMeleePlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_CHOP;
    CHECK_EQ(dm1_v1_action_closed_door_melee_plan_f0407_pc34(&in, &out), 1,
             "closed-door chop plan builds");
    CHECK_EQ(out.isClosedDoorMeleeAction, 1,
             "chop is closed-door melee action");
    CHECK_EQ(out.performed, 0, "chop no observed door effect not performed");
    CHECK_EQ(out.disabledTicksOverride, 0, "chop no override without branch");

    in.observedWoodenThudSound = 1;
    CHECK_EQ(dm1_v1_action_closed_door_melee_plan_f0407_pc34(&in, &out), 1,
             "closed-door chop thud plan builds");
    CHECK_EQ(out.performed, 1, "chop thud makes branch performed");
    CHECK_EQ(out.disabledTicksOverride, 6, "closed-door branch overrides ticks");
    CHECK_EQ(out.destructionDelayTicks, 2, "closed-door branch f0232 delay");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_BASH;
    in.observedDoorDestructionEvent = 1;
    CHECK_EQ(dm1_v1_action_closed_door_melee_plan_f0407_pc34(&in, &out), 1,
             "closed-door bash event plan builds");
    CHECK_EQ(out.performed, 1, "bash event makes branch performed");
    CHECK_EQ(out.disabledTicksOverride, 6, "bash event ticks override");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_PARRY;
    in.observedWoodenThudSound = 1;
    CHECK_EQ(dm1_v1_action_closed_door_melee_plan_f0407_pc34(&in, &out), 1,
             "parry closed-door plan builds");
    CHECK_EQ(out.isClosedDoorMeleeAction, 0,
             "parry is not closed-door branch action");
    CHECK_EQ(out.performed, 0, "parry thud ignored by closed-door plan");
}

static void test_melee_action_tick_plan(void) {
    DM1_MeleeActionTickInputPc34 in;
    DM1_MeleeActionTickPlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.championIndex = 2;
    in.actionIndex = DM1_ACTION_CHOP;
    in.championPresent = 1;
    in.championDirection = 5;
    CHECK_EQ(dm1_v1_melee_action_tick_plan_f0402_pc34(&in, &out), 1,
             "melee tick plan builds");
    CHECK_EQ(out.valid, 1, "melee tick plan valid");
    CHECK_EQ(out.command, CMD_ATTACK, "melee tick command");
    CHECK_EQ(out.commandArg1, 2, "melee tick champion arg");
    CHECK_EQ(out.commandArg2, CMD_ATTACK_TARGET_AUTO_GROUP_PC34,
             "melee tick auto group");
    CHECK_EQ(out.reserved, CMD_ATTACK_CREATURE_AUTO_PC34,
             "melee tick auto creature");
    CHECK_EQ(out.hasTargetDirection, 1, "melee tick target direction flag");
    CHECK_EQ(out.targetDirection, 1, "melee tick target direction normalized");
    CHECK_EQ((out.reserved2 & CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID) != 0u,
             1, "melee tick action valid bit");
    CHECK_EQ((int)(out.reserved2 & CMD_ATTACK_RESERVED2_ACTION_INDEX_MASK),
             DM1_ACTION_CHOP, "melee tick action index bits");
    CHECK_EQ((out.reserved2 & CMD_ATTACK_RESERVED2_TARGET_DIRECTION_VALID) != 0u,
             1, "melee tick direction valid bit");
    CHECK_EQ((int)((out.reserved2 &
                    CMD_ATTACK_RESERVED2_TARGET_DIRECTION_MASK) >>
                   CMD_ATTACK_RESERVED2_TARGET_DIRECTION_SHIFT),
             1, "melee tick direction bits");

    in.actionIndex = DM1_ACTION_BLOCK;
    CHECK_EQ(dm1_v1_melee_action_tick_plan_f0402_pc34(&in, &out), 0,
             "block does not build melee F0402 tick");
    CHECK_EQ(out.valid, 0, "block plan invalid");

    in.actionIndex = DM1_ACTION_CHOP;
    in.championPresent = 0;
    CHECK_EQ(dm1_v1_melee_action_tick_plan_f0402_pc34(&in, &out), 0,
             "missing champion rejected");
}

static void test_melee_damage_emission_plan(void) {
    DM1_MeleeDamageEmissionInputPc34 in;
    DM1_MeleeDamageEmissionPlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.damage = 0;
    in.combatOutcome = COMBAT_OUTCOME_MISS;
    CHECK_EQ(dm1_v1_melee_damage_emission_plan_f0231_pc34(&in, &out), 1,
             "zero damage emission plan builds");
    CHECK_EQ(out.valid, 1, "zero damage emission valid");
    CHECK_EQ(out.performed, 1, "zero damage with outcome is performed");
    CHECK_EQ(out.showDamageFeedback, 0, "zero damage hides feedback");
    CHECK_EQ(out.damage, 0, "zero damage copied");

    in.damage = 17;
    in.combatOutcome = COMBAT_OUTCOME_KILLED_NO_CREATURES;
    CHECK_EQ(dm1_v1_melee_damage_emission_plan_f0231_pc34(&in, &out), 1,
             "positive damage emission plan builds");
    CHECK_EQ(out.performed, 1, "positive damage performed");
    CHECK_EQ(out.showDamageFeedback, 1, "positive damage shows feedback");
    CHECK_EQ(out.damage, 17, "positive damage copied");

    in.damage = 0;
    in.combatOutcome = COMBAT_OUTCOME_INVALID;
    CHECK_EQ(dm1_v1_melee_damage_emission_plan_f0231_pc34(&in, &out), 1,
             "invalid emission plan builds");
    CHECK_EQ(out.performed, 0, "invalid outcome not performed");
    CHECK_EQ(out.showDamageFeedback, 0, "invalid outcome no feedback");
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
    test_action_state_plans();
    test_f0405_charge_plan();
    test_freeze_life_plan();
    test_heal_plan();
    test_light_and_window_plans();
    test_shield_plan();
    test_fright_plan();
    test_projectile_spell_plan();
    test_climb_down_plan();
    test_flip_and_direction_plans();
    test_closed_door_melee_plan();
    test_melee_action_tick_plan();
    test_melee_damage_emission_plan();
    test_invalid_action();
    if (g_failures) {
        fprintf(stderr, "test_dm1_v1_action_f0407_tail_pc34_compat: %d failures\n",
                g_failures);
        return 1;
    }
    printf("test_dm1_v1_action_f0407_tail_pc34_compat: PASS\n");
    return 0;
}
