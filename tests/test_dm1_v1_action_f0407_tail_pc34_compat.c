#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_melee_action_f0402_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"
#include "firestaff/dm1/v1/G0493_pc34_compat.h"

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
    DM1_ActionF0407BeginInputPc34 beginIn;
    DM1_ActionF0407BeginPlanPc34 beginOut;
    DM1_ActionF0325StaminaInputPc34 staminaIn;
    DM1_ActionF0325StaminaPlanPc34 staminaOut;
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

    memset(&staminaIn, 0, sizeof(staminaIn));
    staminaIn.currentStamina = 30;
    staminaIn.maximumStamina = 70;
    staminaIn.currentHealth = 40;
    staminaIn.decrement = 11;
    CHECK_EQ(dm1_v1_action_stamina_apply_plan_f0325_pc34(
                 &staminaIn, &staminaOut), 1,
             "F0325 normal stamina plan builds");
    CHECK_EQ(staminaOut.valid, 1, "F0325 normal valid");
    CHECK_EQ(staminaOut.applied, 1, "F0325 normal applied");
    CHECK_EQ(staminaOut.currentStaminaAfter, 19,
             "F0325 normal stamina decrement");
    CHECK_EQ(staminaOut.currentHealthAfter, 40,
             "F0325 normal health unchanged");
    CHECK_EQ(staminaOut.pendingHealthDamage, 0,
             "F0325 normal no underflow damage");

    staminaIn.currentStamina = 7;
    staminaIn.maximumStamina = 70;
    staminaIn.currentHealth = 40;
    staminaIn.decrement = 18;
    CHECK_EQ(dm1_v1_action_stamina_apply_plan_f0325_pc34(
                 &staminaIn, &staminaOut), 1,
             "F0325 underflow stamina plan builds");
    CHECK_EQ(staminaOut.currentStaminaAfter, 0,
             "F0325 underflow clamps stamina");
    CHECK_EQ(staminaOut.pendingHealthDamage, 5,
             "F0325 underflow damage is half deficit");
    CHECK_EQ(staminaOut.currentHealthAfter, 35,
             "F0325 underflow applies health delta");
    CHECK_EQ(staminaOut.shouldDamageFlash, 1,
             "F0325 underflow requests damage flash");
    CHECK_EQ(staminaOut.appliedAttributeMask, 0x0300,
             "F0325 marks load/statistics attributes");

    staminaIn.currentStamina = 90;
    staminaIn.maximumStamina = 70;
    staminaIn.currentHealth = 40;
    staminaIn.decrement = 5;
    CHECK_EQ(dm1_v1_action_stamina_apply_plan_f0325_pc34(
                 &staminaIn, &staminaOut), 1,
             "F0325 max clamp plan builds");
    CHECK_EQ(staminaOut.currentStaminaAfter, 70,
             "F0325 clamps stamina at maximum");
    CHECK_EQ(staminaOut.currentHealthAfter, 40,
             "F0325 max clamp keeps health");

    memset(&beginIn, 0, sizeof(beginIn));
    beginIn.actionIndex = DM1_ACTION_CHOP;
    beginIn.championIndex = 1;
    beginIn.gameTick = 100u;
    beginIn.currentStamina = 30;
    beginIn.maximumStamina = 70;
    beginIn.currentHealth = 40;
    CHECK_EQ(dm1_v1_action_begin_plan_f0407_pc34(&beginIn, &beginOut), 1,
             "CHOP begin plan builds");
    CHECK_EQ(beginOut.valid, 1, "CHOP begin valid");
    CHECK_EQ(beginOut.actionExperienceGain, 10, "CHOP begin xp");
    CHECK_EQ(beginOut.disabledTicks, 8, "CHOP begin disabled ticks");
    CHECK_EQ(beginOut.staminaCost, 11, "CHOP begin stamina cost");
    CHECK_EQ(beginOut.currentStaminaAfter, 19,
             "CHOP begin applies F0325 stamina");
    CHECK_EQ(beginOut.currentHealthAfter, 40,
             "CHOP begin keeps health");
    CHECK_EQ(beginOut.defenseDelta, 0, "CHOP begin defense delta");
    CHECK_EQ(beginOut.resultingActionIndex, DM1_ACTION_CHOP,
             "CHOP begin action index");
    CHECK_EQ(beginOut.isMeleeContact, 1, "CHOP begin melee contact");

    beginIn.actionIndex = 99;
    CHECK_EQ(dm1_v1_action_begin_plan_f0407_pc34(&beginIn, &beginOut), 0,
             "invalid begin action rejected");
}

static void test_xp_award_and_direct_dispatch_plans(void) {
    DM1_ActionXpAwardInputPc34 xpIn;
    DM1_ActionXpAwardPlanPc34 xpOut;
    DM1_ActionDirectDispatchInputPc34 dispatchIn;
    DM1_ActionDirectDispatchPlanPc34 dispatchOut;

    memset(&xpIn, 0, sizeof(xpIn));
    xpIn.actionIndex = DM1_ACTION_CHOP;
    xpIn.experienceGain = 10;
    CHECK_EQ(dm1_v1_action_xp_award_plan_f0407_pc34(&xpIn, &xpOut), 1,
             "CHOP XP award plan builds");
    CHECK_EQ(xpOut.valid, 1, "CHOP XP award valid");
    CHECK_EQ(xpOut.shouldAward, 1, "CHOP XP should award");
    CHECK_EQ(xpOut.skillIndex, 6, "CHOP XP skill route");
    CHECK_EQ(xpOut.baseSkillIndex, 0, "CHOP XP base skill");
    CHECK_EQ(xpOut.experienceGain, 10, "CHOP XP copied");

    xpIn.experienceGain = 0;
    CHECK_EQ(dm1_v1_action_xp_award_plan_f0407_pc34(&xpIn, &xpOut), 1,
             "zero XP award plan builds");
    CHECK_EQ(xpOut.shouldAward, 0, "zero XP skips award");
    CHECK_EQ(xpOut.experienceGain, 0, "zero XP clamped");

    xpIn.actionIndex = DM1_GRAPHIC560_ACTION_COUNT;
    xpIn.experienceGain = 10;
    CHECK_EQ(dm1_v1_action_xp_award_plan_f0407_pc34(&xpIn, &xpOut), 0,
             "invalid XP award action rejected");

    memset(&dispatchIn, 0, sizeof(dispatchIn));
    dispatchIn.actionIndex = DM1_ACTION_HEAL;
    CHECK_EQ(dm1_v1_action_direct_dispatch_plan_f0407_pc34(
                 &dispatchIn, &dispatchOut), 1,
             "HEAL direct dispatch plan builds");
    CHECK_EQ(dispatchOut.mayDispatchDirect, 1,
             "HEAL may dispatch directly");
    CHECK_EQ(dispatchOut.isMeleeContact, 0,
             "HEAL not melee contact");

    dispatchIn.actionIndex = DM1_ACTION_CHOP;
    CHECK_EQ(dm1_v1_action_direct_dispatch_plan_f0407_pc34(
                 &dispatchIn, &dispatchOut), 1,
             "CHOP direct dispatch plan builds");
    CHECK_EQ(dispatchOut.isMeleeContact, 1,
             "CHOP is melee contact");
    CHECK_EQ(dispatchOut.mayDispatchDirect, 0,
             "CHOP direct dispatch rejected");

    dispatchIn.actionIndex = DM1_ACTION_PARRY;
    CHECK_EQ(dm1_v1_action_direct_dispatch_plan_f0407_pc34(
                 &dispatchIn, &dispatchOut), 1,
             "PARRY direct dispatch plan builds");
    CHECK_EQ(dispatchOut.isMeleeContact, 1,
             "PARRY is melee contact");
    CHECK_EQ(dispatchOut.mayDispatchDirect, 1,
             "PARRY direct dispatch kept for regression");
    CHECK_EQ(dispatchOut.allowsParryEmptyFrontRegression, 1,
             "PARRY empty-front regression allowed");
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
    DM1_ActionProjectileSpellDescriptorPc34 desc;
    DM1_ActionProjectileSpellPlanPc34 out;

    CHECK_EQ(dm1_v1_action_projectile_spell_descriptor_f0407_pc34(
                 DM1_ACTION_FIREBALL, &desc), 1,
             "fireball projectile descriptor builds");
    CHECK_EQ(desc.valid, 1, "fireball descriptor valid");
    CHECK_EQ(desc.actionSkillIndex, 16, "fireball descriptor fire skill");
    CHECK_EQ(desc.verbKind, DM1_ACTION_PROJECTILE_VERB_FIREBALL_PC34,
             "fireball descriptor verb");
    CHECK_EQ(desc.requiresInvokeRolls, 0,
             "fireball descriptor no invoke rolls");

    CHECK_EQ(dm1_v1_action_projectile_spell_descriptor_f0407_pc34(
                 DM1_ACTION_INVOKE, &desc), 1,
             "invoke projectile descriptor builds");
    CHECK_EQ(desc.actionSkillIndex, 3, "invoke descriptor wizard skill");
    CHECK_EQ(desc.verbKind, DM1_ACTION_PROJECTILE_VERB_INVOKE_PC34,
             "invoke descriptor verb");
    CHECK_EQ(desc.requiresInvokeRolls, 1,
             "invoke descriptor requires rolls");

    CHECK_EQ(dm1_v1_action_projectile_spell_descriptor_f0407_pc34(
                 DM1_ACTION_LIGHT, &desc), 0,
             "light projectile descriptor rejected");

    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_FIREBALL;
    in.skillLevel = 3;
    in.currentMana = 10;
    in.maximumMana = 64;
    CHECK_EQ(dm1_v1_action_projectile_spell_plan_f0407_pc34(&in, &out), 1,
             "fireball projectile plan builds");
    CHECK_EQ(out.actionSkillIndex, 16, "fireball uses fire skill");
    CHECK_EQ(out.verbKind, DM1_ACTION_PROJECTILE_VERB_FIREBALL_PC34,
             "fireball verb kind");
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
    CHECK_EQ(out.poisonAttack, 0, "fireball no poison attack");
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
    CHECK_EQ(out.verbKind, DM1_ACTION_PROJECTILE_VERB_INVOKE_PC34,
             "invoke verb kind");
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
    CHECK_EQ(out.poisonAttack, 90, "invoke poison attack");

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
    CHECK_EQ(throwOut.actionEnableSlotOrdinal, CHAMPION_SLOT_ACTION_HAND,
             "throw requests action-hand enable slot");
}

static void test_closed_door_melee_plan(void) {
    DM1_ActionClosedDoorMeleeInputPc34 in;
    DM1_ActionClosedDoorMeleePlanPc34 out;
    DM1_ActionClosedDoorDestructionInputPc34 destructionIn;
    DM1_ActionClosedDoorDestructionPlanPc34 destructionOut;

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

    memset(&destructionIn, 0, sizeof(destructionIn));
    destructionIn.closedDoorState = 1;
    destructionIn.meleeDestructible = 1;
    destructionIn.attack = 42;
    destructionIn.defense = 42;
    destructionIn.destructionDelayTicks = 2;
    destructionIn.currentTick = 100u;
    destructionIn.mapIndex = 3;
    destructionIn.mapX = 4;
    destructionIn.mapY = 5;
    CHECK_EQ(dm1_v1_action_closed_door_destruction_plan_f0232_pc34(
                 &destructionIn, &destructionOut), 1,
             "F0232 closed-door destruction plan builds");
    CHECK_EQ(destructionOut.valid, 1, "F0232 destruction plan valid");
    CHECK_EQ(destructionOut.shouldScheduleDestruction, 1,
             "F0232 attack reaches defense schedules destruction");
    CHECK_EQ((int)destructionOut.fireAtTick, 102,
             "F0232 destruction fire tick");
    CHECK_EQ(destructionOut.mapIndex, 3, "F0232 destruction map");
    CHECK_EQ(destructionOut.mapX, 4, "F0232 destruction x");
    CHECK_EQ(destructionOut.mapY, 5, "F0232 destruction y");
    CHECK_EQ(destructionOut.destroyedDoorState, 5,
             "F0232 destroyed door state");

    destructionIn.attack = 41;
    CHECK_EQ(dm1_v1_action_closed_door_destruction_plan_f0232_pc34(
                 &destructionIn, &destructionOut), 1,
             "F0232 weak attack plan builds");
    CHECK_EQ(destructionOut.shouldScheduleDestruction, 0,
             "F0232 weak attack suppresses destruction");

    destructionIn.attack = 42;
    destructionIn.meleeDestructible = 0;
    CHECK_EQ(dm1_v1_action_closed_door_destruction_plan_f0232_pc34(
                 &destructionIn, &destructionOut), 1,
             "F0232 indestructible door plan builds");
    CHECK_EQ(destructionOut.shouldScheduleDestruction, 0,
             "F0232 indestructible door suppresses destruction");

    destructionIn.meleeDestructible = 1;
    destructionIn.closedDoorState = 0;
    CHECK_EQ(dm1_v1_action_closed_door_destruction_plan_f0232_pc34(
                 &destructionIn, &destructionOut), 1,
             "F0232 non-closed door plan builds");
    CHECK_EQ(destructionOut.shouldScheduleDestruction, 0,
             "F0232 non-closed door suppresses destruction");
}

static void test_melee_action_tick_plan(void) {
    DM1_MeleeActionTickInputPc34 in;
    DM1_MeleeActionTickPlanPc34 out;
    DM1_MeleeF0402CommandDecodeInputPc34 decodeIn;
    DM1_MeleeF0402CommandDecodePlanPc34 decodeOut;

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

    memset(&decodeIn, 0, sizeof(decodeIn));
    decodeIn.commandArg2 = CMD_ATTACK_TARGET_AUTO_GROUP_PC34;
    decodeIn.reserved = CMD_ATTACK_CREATURE_AUTO_PC34;
    decodeIn.partyMapIndex = 4;
    decodeIn.partyMapX = 10;
    decodeIn.partyMapY = 20;
    decodeIn.partyDirection = 2;
    decodeIn.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
                         (unsigned int)DM1_ACTION_CHOP |
                         CMD_ATTACK_RESERVED2_TARGET_DIRECTION_VALID |
                         (3u << CMD_ATTACK_RESERVED2_TARGET_DIRECTION_SHIFT);
    CHECK_EQ(dm1_v1_melee_command_decode_plan_f0402_pc34(
                 &decodeIn, &decodeOut), 1,
             "F0402 command decode builds");
    CHECK_EQ(decodeOut.valid, 1, "F0402 command decode valid");
    CHECK_EQ(decodeOut.hasLiveActionIndex, 1,
             "F0402 command decode live action bit");
    CHECK_EQ(decodeOut.actionIndex, DM1_ACTION_CHOP,
             "F0402 command decode action");
    CHECK_EQ(decodeOut.actionSkillIndex, 6,
             "F0402 command decode skill route");
    CHECK_EQ(decodeOut.hasTargetDirection, 1,
             "F0402 command decode direction bit");
    CHECK_EQ(decodeOut.targetDirection, 3,
             "F0402 command decode target direction");
    CHECK_EQ(decodeOut.requestedAutoTarget, 1,
             "F0402 command decode auto target");
    CHECK_EQ(decodeOut.requestedAutoCreature, 1,
             "F0402 command decode auto creature");
    CHECK_EQ(decodeOut.targetMapIndex, 4,
             "F0402 command decode target map");
    CHECK_EQ(decodeOut.targetMapX, 9,
             "F0402 command decode west target x");
    CHECK_EQ(decodeOut.targetMapY, 20,
             "F0402 command decode west target y");

    decodeIn.commandArg2 = 7;
    decodeIn.reserved = 2;
    decodeIn.reserved2 = CMD_ATTACK_RESERVED2_ACTION_INDEX_VALID |
                         (unsigned int)DM1_GRAPHIC560_ACTION_COUNT |
                         CMD_ATTACK_RESERVED2_LEGACY_MARKER_VALID;
    CHECK_EQ(dm1_v1_melee_command_decode_plan_f0402_pc34(
                 &decodeIn, &decodeOut), 1,
             "F0402 invalid action decode builds");
    CHECK_EQ(decodeOut.actionIndex, CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34,
             "F0402 invalid action defaults to MELEE");
    CHECK_EQ(decodeOut.hasLiveActionIndex, 1,
             "F0402 invalid action keeps live bit");
    CHECK_EQ(decodeOut.hasLegacyMarker, 1,
             "F0402 legacy marker bit");
    CHECK_EQ(decodeOut.targetDirection, 2,
             "F0402 missing direction falls back to party");
    CHECK_EQ(decodeOut.requestedAutoTarget, 0,
             "F0402 direct target flag");
    CHECK_EQ(decodeOut.directGroupIndex, 7,
             "F0402 direct group index");
    CHECK_EQ(decodeOut.requestedAutoCreature, 0,
             "F0402 direct creature flag");
    CHECK_EQ(decodeOut.directCreatureIndex, 2,
             "F0402 direct creature index");
    CHECK_EQ(decodeOut.targetMapX, 10,
             "F0402 south target x");
    CHECK_EQ(decodeOut.targetMapY, 21,
             "F0402 south target y");

    decodeIn.commandArg2 = 0;
    decodeIn.reserved = 0;
    decodeIn.reserved2 = 0u;
    decodeIn.partyDirection = 7;
    CHECK_EQ(dm1_v1_melee_command_decode_plan_f0402_pc34(
                 &decodeIn, &decodeOut), 1,
             "F0402 default command decode builds");
    CHECK_EQ(decodeOut.hasLiveActionIndex, 0,
             "F0402 default command no live action");
    CHECK_EQ(decodeOut.actionIndex, CMD_ATTACK_DEFAULT_ACTION_INDEX_PC34,
             "F0402 default command action");
    CHECK_EQ(decodeOut.targetDirection, 3,
             "F0402 default command party direction masked");
    CHECK_EQ(decodeOut.targetMapX, 9,
             "F0402 default command west target x");
}

static void test_melee_damage_emission_plan(void) {
    DM1_MeleeDamageEmissionInputPc34 in;
    DM1_MeleeDamageEmissionPlanPc34 out;
    DM1_MeleeRuntimeOutcomeInputPc34 runtimeIn;
    DM1_MeleeRuntimeOutcomePlanPc34 runtimeOut;
    DM1_MeleeKillNotifyInputPc34 killIn;
    DM1_MeleeKillNotifyPlanPc34 killOut;

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

    memset(&runtimeIn, 0, sizeof(runtimeIn));
    runtimeIn.actionIndex = DM1_ACTION_CHOP;
    runtimeIn.defaultDisabledTicks = 8;
    runtimeIn.observedAttackDamage = 13;
    runtimeIn.combatOutcome = COMBAT_OUTCOME_KILLED_NO_CREATURES;
    CHECK_EQ(dm1_v1_melee_runtime_outcome_plan_f0407_f0231_pc34(
                 &runtimeIn, &runtimeOut), 1,
             "melee runtime damage outcome builds");
    CHECK_EQ(runtimeOut.performed, 1, "melee runtime damage performed");
    CHECK_EQ(runtimeOut.meleeFailureTail, 0,
             "melee runtime damage no failure tail");
    CHECK_EQ(runtimeOut.showDamageFeedback, 1,
             "melee runtime positive damage feedback");
    CHECK_EQ(runtimeOut.damage, 13, "melee runtime damage copied");
    CHECK_EQ(runtimeOut.disabledTicks, 8,
             "melee runtime damage keeps disabled ticks");

    runtimeIn.observedAttackDamage = 0;
    runtimeIn.combatOutcome = COMBAT_OUTCOME_MISS;
    CHECK_EQ(dm1_v1_melee_runtime_outcome_plan_f0407_f0231_pc34(
                 &runtimeIn, &runtimeOut), 1,
             "melee runtime miss outcome builds");
    CHECK_EQ(runtimeOut.performed, 1, "melee runtime miss performed");
    CHECK_EQ(runtimeOut.showDamageFeedback, 0,
             "melee runtime miss hides damage");
    CHECK_EQ(runtimeOut.meleeFailureTail, 0,
             "melee runtime miss no failure tail");

    memset(&runtimeIn, 0, sizeof(runtimeIn));
    runtimeIn.actionIndex = DM1_ACTION_BASH;
    runtimeIn.defaultDisabledTicks = 9;
    runtimeIn.combatOutcome = COMBAT_OUTCOME_INVALID;
    runtimeIn.closedDoorBranchPerformed = 1;
    runtimeIn.closedDoorDisabledTicks = 6;
    CHECK_EQ(dm1_v1_melee_runtime_outcome_plan_f0407_f0231_pc34(
                 &runtimeIn, &runtimeOut), 1,
             "melee runtime closed-door outcome builds");
    CHECK_EQ(runtimeOut.performed, 1, "closed-door branch performed");
    CHECK_EQ(runtimeOut.disabledTicks, 6,
             "closed-door branch overrides disabled ticks");
    CHECK_EQ(runtimeOut.meleeFailureTail, 0,
             "closed-door branch no failure tail");

    memset(&runtimeIn, 0, sizeof(runtimeIn));
    runtimeIn.actionIndex = DM1_ACTION_PARRY;
    runtimeIn.defaultDisabledTicks = 5;
    runtimeIn.combatOutcome = COMBAT_OUTCOME_INVALID;
    CHECK_EQ(dm1_v1_melee_runtime_outcome_plan_f0407_f0231_pc34(
                 &runtimeIn, &runtimeOut), 1,
             "melee runtime failure outcome builds");
    CHECK_EQ(runtimeOut.performed, 0, "empty-front parry not performed");
    CHECK_EQ(runtimeOut.meleeFailureTail, 1,
             "empty-front parry uses failure tail");
    CHECK_EQ(runtimeOut.disabledTicks, 5,
             "failure outcome keeps base ticks before tail adjustment");

    memset(&killIn, 0, sizeof(killIn));
    killIn.creatureType = 6;
    killIn.creatureBaseHealth = 30;
    killIn.activeChampionIndex = 2;
    killIn.activeChampionPresent = 1;
    CHECK_EQ(dm1_v1_melee_kill_notify_plan_f0231_pc34(
                 &killIn, &killOut), 1,
             "kill notify plan builds");
    CHECK_EQ(killOut.valid, 1, "kill notify valid");
    CHECK_EQ(killOut.shouldLogDefeated, 1, "kill notify logs defeated");
    CHECK_EQ(killOut.shouldAwardKillXp, 1, "kill notify awards legacy xp");
    CHECK_EQ(killOut.championIndex, 2, "kill notify champion");
    CHECK_EQ(killOut.creatureType, 6, "kill notify creature");
    CHECK_EQ(killOut.xpBonus, 15, "kill notify base health xp");

    killIn.creatureBaseHealth = 3;
    CHECK_EQ(dm1_v1_melee_kill_notify_plan_f0231_pc34(
                 &killIn, &killOut), 1,
             "kill notify minimum xp builds");
    CHECK_EQ(killOut.xpBonus, 5, "kill notify minimum xp");

    killIn.activeChampionPresent = 0;
    CHECK_EQ(dm1_v1_melee_kill_notify_plan_f0231_pc34(
                 &killIn, &killOut), 1,
             "kill notify missing champion builds");
    CHECK_EQ(killOut.shouldLogDefeated, 1,
             "kill notify missing champion still logs");
    CHECK_EQ(killOut.shouldAwardKillXp, 0,
             "kill notify missing champion no xp");
}

static void test_melee_pre_f0231_gates(void) {
    DM1_MeleeReachGateInputPc34 reachIn;
    DM1_MeleeReachGatePlanPc34 reachOut;
    DM1_MeleeDisruptMaterialGateInputPc34 disruptIn;
    DM1_MeleeDisruptMaterialGatePlanPc34 disruptOut;

    memset(&reachIn, 0, sizeof(reachIn));
    reachIn.championIndex = 0;
    reachIn.championPresent = 1;
    reachIn.championCurrentHealth = 100;
    reachIn.championCell = 2;
    reachIn.targetDirection = 0;
    reachIn.partyChampionCount = 4;
    reachIn.otherChampionPresent[1] = 1;
    reachIn.otherChampionCurrentHealth[1] = 100;
    reachIn.otherChampionCell[1] = 1;
    CHECK_EQ(dm1_v1_melee_reach_gate_plan_f0402_pc34(
                 &reachIn, &reachOut), 1,
             "back-row reach gate builds");
    CHECK_EQ(reachOut.valid, 1, "back-row reach gate valid");
    CHECK_EQ(reachOut.blocked, 1, "front champion blocks back-row melee");
    CHECK_EQ(reachOut.sourceFixedBackRowGate, 1,
             "PC34 uses fixed back-row reach gate");
    CHECK_EQ(reachOut.scannedFrontCell, 1,
             "back-row reach scans front cell");
    CHECK_EQ(reachOut.relativeCell, 2, "back-right relative cell");
    CHECK_EQ(reachOut.blockingCell, 1, "blocking front cell");
    CHECK_EQ(reachOut.blockingChampionIndex, 1, "blocking champion index");
    CHECK_EQ(reachOut.combatOutcome, COMBAT_OUTCOME_INVALID,
             "blocked reach reports pre-F0231 invalid outcome");

    reachIn.otherChampionPresent[1] = 0;
    CHECK_EQ(dm1_v1_melee_reach_gate_plan_f0402_pc34(
                 &reachIn, &reachOut), 1,
             "unblocked back-row reach gate builds");
    CHECK_EQ(reachOut.blocked, 0, "empty front cell allows F0231");

    reachIn.otherChampionPresent[1] = 1;
    reachIn.otherChampionCurrentHealth[1] = 0;
    CHECK_EQ(dm1_v1_melee_reach_gate_plan_f0402_pc34(
                 &reachIn, &reachOut), 1,
             "dead front champion reach gate builds");
    CHECK_EQ(reachOut.blocked, 0,
             "dead front champion does not block F0402 reach");
    CHECK_EQ(reachOut.scannedFrontCell, 1,
             "dead front champion still proves source front-cell scan");

    reachIn.otherChampionCurrentHealth[1] = 100;
    reachIn.championCell = 3;
    reachIn.otherChampionCell[1] = 0;
    CHECK_EQ(dm1_v1_melee_reach_gate_plan_f0402_pc34(
                 &reachIn, &reachOut), 1,
             "back-left reach gate builds");
    CHECK_EQ(reachOut.blocked, 1, "front-left champion blocks back-left");
    CHECK_EQ(reachOut.relativeCell, 3, "back-left relative cell");
    CHECK_EQ(reachOut.blockingCell, 0, "back-left blocking front cell");

    reachIn.championCell = 0;
    CHECK_EQ(dm1_v1_melee_reach_gate_plan_f0402_pc34(
                 &reachIn, &reachOut), 1,
             "front-row reach gate builds");
    CHECK_EQ(reachOut.blocked, 0, "front-row melee not reach-blocked");

    memset(&disruptIn, 0, sizeof(disruptIn));
    disruptIn.actionIndex = DM1_ACTION_DISRUPT;
    disruptIn.targetCreatureAttributes = 0;
    CHECK_EQ(dm1_v1_melee_disrupt_material_gate_plan_f0402_pc34(
                 &disruptIn, &disruptOut), 1,
             "material DISRUPT gate builds");
    CHECK_EQ(disruptOut.valid, 1, "material DISRUPT gate valid");
    CHECK_EQ(disruptOut.blocked, 1, "material creature blocks DISRUPT");
    CHECK_EQ(disruptOut.combatOutcome, COMBAT_OUTCOME_INVALID,
             "material DISRUPT reports pre-F0231 invalid outcome");

    disruptIn.targetCreatureAttributes = 0x0040;
    CHECK_EQ(dm1_v1_melee_disrupt_material_gate_plan_f0402_pc34(
                 &disruptIn, &disruptOut), 1,
             "non-material DISRUPT gate builds");
    CHECK_EQ(disruptOut.blocked, 0, "non-material creature allows DISRUPT");

    disruptIn.actionIndex = DM1_ACTION_CHOP;
    disruptIn.targetCreatureAttributes = 0;
    CHECK_EQ(dm1_v1_melee_disrupt_material_gate_plan_f0402_pc34(
                 &disruptIn, &disruptOut), 1,
             "non-DISRUPT gate builds");
    CHECK_EQ(disruptOut.blocked, 0, "non-DISRUPT action not blocked");
}

static void test_melee_weapon_profile_plan(void) {
    DM1_MeleeWeaponProfileInputPc34 in;
    DM1_MeleeWeaponProfilePlanPc34 out;
    int chopHit = dm1_v1_graphic560_action_hit_probability_get_pc34(
        DM1_ACTION_CHOP);
    int chopDamage = dm1_v1_graphic560_action_damage_factor_get_pc34(
        DM1_ACTION_CHOP);
    int meleeHit = dm1_v1_graphic560_action_hit_probability_get_pc34(
        DM1_ACTION_MELEE);
    int meleeDamage = dm1_v1_graphic560_action_damage_factor_get_pc34(
        DM1_ACTION_MELEE);
    int disruptHit = dm1_v1_graphic560_action_hit_probability_get_pc34(
        DM1_ACTION_DISRUPT);
    int disruptDamage = dm1_v1_graphic560_action_damage_factor_get_pc34(
        DM1_ACTION_DISRUPT);

    memset(&in, 0, sizeof(in));
    in.weaponType = 12;
    in.weaponClass = 3;
    in.weaponStrength = 24;
    in.kineticEnergy = 18;
    in.weaponAttributes = 0x55;
    in.actionIndex = DM1_ACTION_CHOP;
    in.actionSkillIndex = 6;
    CHECK_EQ(dm1_v1_melee_weapon_profile_plan_f0402_f0231_pc34(
                 &in, &out), 1,
             "CHOP weapon profile builds");
    CHECK_EQ(out.valid, 1, "CHOP weapon profile valid");
    CHECK_EQ(out.normalizedActionIndex, DM1_ACTION_CHOP,
             "CHOP action stays normalized");
    CHECK_EQ(out.hitProbability, chopHit,
             "CHOP hit probability from G0493");
    CHECK_EQ(out.damageFactor, chopDamage,
             "CHOP damage factor from G0492");
    CHECK_EQ(out.hitNonMaterialFlagSet, 0,
             "normal weapon does not set non-material flag");
    CHECK_EQ(out.weaponProfile.weaponType, 12, "weapon type copied");
    CHECK_EQ(out.weaponProfile.weaponClass, 3, "weapon class copied");
    CHECK_EQ(out.weaponProfile.weaponStrength, 24, "weapon strength copied");
    CHECK_EQ(out.weaponProfile.kineticEnergy, 18, "kinetic energy copied");
    CHECK_EQ(out.weaponProfile.hitProbability, chopHit,
             "profile CHOP hit probability");
    CHECK_EQ(out.weaponProfile.damageFactor, chopDamage,
             "profile CHOP damage factor");
    CHECK_EQ(out.weaponProfile.skillIndex, 6, "profile skill index");
    CHECK_EQ(out.weaponProfile.attributes, 0x55, "profile attributes");

    in.weaponType = COMBAT_ICON_VORPAL_BLADE;
    CHECK_EQ(dm1_v1_melee_weapon_profile_plan_f0402_f0231_pc34(
                 &in, &out), 1,
             "Vorpal weapon profile builds");
    CHECK_EQ(out.hitNonMaterialFlagSet, 1, "Vorpal sets non-material flag");
    CHECK_EQ(out.weaponProfile.hitProbability,
             chopHit | 0x8000,
             "Vorpal hit probability carries MASK0x8000");

    in.weaponType = 12;
    in.actionIndex = DM1_ACTION_DISRUPT;
    CHECK_EQ(dm1_v1_melee_weapon_profile_plan_f0402_f0231_pc34(
                 &in, &out), 1,
             "DISRUPT weapon profile builds");
    CHECK_EQ(out.normalizedActionIndex, DM1_ACTION_DISRUPT,
             "DISRUPT action normalized");
    CHECK_EQ(out.hitNonMaterialFlagSet, 1,
             "DISRUPT sets non-material flag");
    CHECK_EQ(out.weaponProfile.hitProbability,
             disruptHit | 0x8000,
             "DISRUPT hit probability carries MASK0x8000");
    CHECK_EQ(out.weaponProfile.damageFactor, disruptDamage,
             "DISRUPT damage factor from G0492");

    in.actionIndex = 999;
    CHECK_EQ(dm1_v1_melee_weapon_profile_plan_f0402_f0231_pc34(
                 &in, &out), 1,
             "invalid action profile falls back");
    CHECK_EQ(out.normalizedActionIndex, DM1_ACTION_MELEE,
             "invalid action falls back to MELEE");
    CHECK_EQ(out.weaponProfile.hitProbability, meleeHit,
             "fallback hit probability");
    CHECK_EQ(out.weaponProfile.damageFactor, meleeDamage,
             "fallback damage factor");
}

static void test_melee_f0231_side_effect_plan(void) {
    DM1_MeleeF0231SideEffectInputPc34 in;
    DM1_MeleeF0231SideEffectPlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.championIndex = 1;
    in.actionSkillIndex = 6;
    in.damageApplied = 32;
    in.creatureProperties = 0x0500;
    in.mapDifficulty = 7;
    in.currentTick = 1234u;
    in.lastCreatureAttackTime = 1200u;
    in.currentStamina = 100;
    in.maximumStamina = 200;
    in.currentHealth = 50;
    in.staminaRandomValue = 3;
    CHECK_EQ(dm1_v1_melee_side_effect_plan_f0231_pc34(&in, &out), 1,
             "F0231 damage side-effect plan builds");
    CHECK_EQ(out.valid, 1, "F0231 damage side-effect valid");
    CHECK_EQ(out.shouldWriteChampionState, 1,
             "F0231 damage writes champion state");
    CHECK_EQ(out.championIndex, 1, "F0231 damage write champion");
    CHECK_EQ(out.shouldAwardXp, 1, "F0231 damage awards XP");
    CHECK_EQ(out.xpChampionIndex, 1, "F0231 damage XP champion");
    CHECK_EQ(out.skillIndex, 6, "F0231 XP skill index");
    CHECK_EQ(out.experienceGain, 13, "F0231 XP formula");
    CHECK_EQ(out.xpMapDifficulty, 7, "F0231 XP map difficulty receipt");
    CHECK_EQ((int)out.xpCurrentTick, 1234, "F0231 XP current tick receipt");
    CHECK_EQ((int)out.xpLastCreatureAttackTime, 1200,
             "F0231 XP last creature attack receipt");
    CHECK_EQ(out.staminaRandomModulus, 4, "F0231 damage random modulus");
    CHECK_EQ(out.staminaBaseCost, 4, "F0231 damage stamina base");
    CHECK_EQ(out.staminaCost, 7, "F0231 damage stamina cost");
    CHECK_EQ(out.currentStaminaAfter, 93, "F0231 damage stamina after");
    CHECK_EQ(out.currentHealthAfter, 50, "F0231 damage health unchanged");

    in.damageApplied = 0;
    in.actionSkillIndex = -1;
    in.currentStamina = 5;
    in.maximumStamina = 200;
    in.currentHealth = 9;
    in.staminaRandomValue = 5;
    CHECK_EQ(dm1_v1_melee_side_effect_plan_f0231_pc34(&in, &out), 1,
             "F0231 miss side-effect plan builds");
    CHECK_EQ(out.shouldAwardXp, 0, "F0231 miss no XP");
    CHECK_EQ(out.staminaRandomModulus, 2, "F0231 miss random modulus");
    CHECK_EQ(out.staminaBaseCost, 2, "F0231 miss stamina base");
    CHECK_EQ(out.staminaCost, 3, "F0231 miss stamina random clamps");
    CHECK_EQ(out.currentStaminaAfter, 2, "F0231 miss stamina after");
    CHECK_EQ(out.currentHealthAfter, 9, "F0231 miss health unchanged");

    in.currentStamina = 1;
    in.currentHealth = 2;
    in.staminaRandomValue = 1;
    CHECK_EQ(dm1_v1_melee_side_effect_plan_f0231_pc34(&in, &out), 1,
             "F0231 miss underflow plan builds");
    CHECK_EQ(out.staminaCost, 3, "F0231 underflow stamina cost");
    CHECK_EQ(out.currentStaminaAfter, 0, "F0231 underflow stamina clamps");
    CHECK_EQ(out.pendingHealthDamage, 1, "F0231 underflow health damage");
    CHECK_EQ(out.currentHealthAfter, 1, "F0231 underflow health after");

    in.championIndex = CHAMPION_MAX_PARTY;
    CHECK_EQ(dm1_v1_melee_side_effect_plan_f0231_pc34(&in, &out), 0,
             "F0231 side-effect rejects invalid champion");
}

static void test_melee_f0402_weapon_availability_and_preflight(void) {
    DM1_MeleeF0402WeaponAvailabilityInputPc34 availIn;
    DM1_MeleeF0402WeaponAvailabilityPlanPc34 availOut;
    DM1_MeleeF0402PreflightInputPc34 preIn;
    DM1_MeleeF0402PreflightPlanPc34 preOut;

    memset(&availIn, 0, sizeof(availIn));
    availIn.hasWeaponInfo = 1;
    CHECK_EQ(dm1_v1_melee_weapon_availability_plan_f0402_pc34(
                 &availIn, &availOut), 1,
             "F0402 weapon availability builds");
    CHECK_EQ(availOut.valid, 1, "F0402 weapon availability valid");
    CHECK_EQ(availOut.useActionHandWeaponInfo, 1,
             "F0402 action-hand weapon info used");
    CHECK_EQ(availOut.hasUsableF0231WeaponInfo, 1,
             "F0402 action-hand weapon usable");
    CHECK_EQ(availOut.weaponClass, -1,
             "F0402 action-hand path leaves class to caller");

    memset(&availIn, 0, sizeof(availIn));
    availIn.hasLiveActionIndex = 1;
    availIn.actionHandEmpty = 1;
    CHECK_EQ(dm1_v1_melee_weapon_availability_plan_f0402_pc34(
                 &availIn, &availOut), 1,
             "F0402 empty-hand availability builds");
    CHECK_EQ(availOut.useEmptyHandWeaponInfo, 1,
             "F0402 empty hand uses unarmed weapon info");
    CHECK_EQ(availOut.hasUsableF0231WeaponInfo, 1,
             "F0402 empty hand usable");
    CHECK_EQ(availOut.weaponClass, 255, "F0402 empty hand weapon class");

    memset(&availIn, 0, sizeof(availIn));
    availIn.hasLiveActionIndex = 1;
    availIn.actionHandEmpty = 0;
    CHECK_EQ(dm1_v1_melee_weapon_availability_plan_f0402_pc34(
                 &availIn, &availOut), 1,
             "F0402 unavailable hand builds");
    CHECK_EQ(availOut.hasUsableF0231WeaponInfo, 0,
             "F0402 no weapon and non-empty hand unusable");

    memset(&preIn, 0, sizeof(preIn));
    preIn.requestedAutoTarget = 1;
    preIn.hasLiveActionIndex = 1;
    preIn.targetResolved = 0;
    CHECK_EQ(dm1_v1_melee_preflight_plan_f0402_pc34(&preIn, &preOut), 1,
             "F0402 live no-target preflight builds");
    CHECK_EQ(preOut.shouldReturnHandled, 1,
             "F0402 live no-target handled no-op");
    CHECK_EQ(preOut.canUseLegacyMarker, 0,
             "F0402 live no-target does not use marker");

    memset(&preIn, 0, sizeof(preIn));
    preIn.targetResolved = 0;
    CHECK_EQ(dm1_v1_melee_preflight_plan_f0402_pc34(&preIn, &preOut), 1,
             "F0402 marker fallback preflight builds");
    CHECK_EQ(preOut.shouldReturnHandled, 0,
             "F0402 marker fallback not handled yet");
    CHECK_EQ(preOut.canUseLegacyMarker, 1,
             "F0402 marker fallback allowed");

    memset(&preIn, 0, sizeof(preIn));
    preIn.targetResolved = 1;
    preIn.reachBlocked = 1;
    CHECK_EQ(dm1_v1_melee_preflight_plan_f0402_pc34(&preIn, &preOut), 1,
             "F0402 reach-block preflight builds");
    CHECK_EQ(preOut.shouldReturnHandled, 1,
             "F0402 reach-block handled");
    CHECK_EQ(preOut.shouldEmitDamageDealt, 1,
             "F0402 reach-block emits invalid damage marker");
    CHECK_EQ(preOut.emitOutcome, COMBAT_OUTCOME_INVALID,
             "F0402 reach-block invalid outcome");

    memset(&preIn, 0, sizeof(preIn));
    preIn.targetResolved = 1;
    preIn.disruptBlocked = 1;
    CHECK_EQ(dm1_v1_melee_preflight_plan_f0402_pc34(&preIn, &preOut), 1,
             "F0402 disrupt-block preflight builds");
    CHECK_EQ(preOut.shouldEmitDamageDealt, 1,
             "F0402 disrupt-block emits invalid damage marker");

    memset(&preIn, 0, sizeof(preIn));
    preIn.targetResolved = 1;
    preIn.candidateInvulnerable = 1;
    CHECK_EQ(dm1_v1_melee_preflight_plan_f0402_pc34(&preIn, &preOut), 1,
             "F0402 candidate preflight builds");
    CHECK_EQ(preOut.shouldReturnHandled, 1,
             "F0402 candidate panel handled before F0312");
    CHECK_EQ(preOut.shouldEmitDamageDealt, 0,
             "F0402 candidate panel emits nothing");

    memset(&preIn, 0, sizeof(preIn));
    preIn.targetResolved = 1;
    preIn.creatureSnapshotReady = 1;
    preIn.championSnapshotReady = 0;
    CHECK_EQ(dm1_v1_melee_preflight_plan_f0402_pc34(&preIn, &preOut), 1,
             "F0402 missing champion snapshot preflight builds");
    CHECK_EQ(preOut.shouldReturnHandled, 1,
             "F0402 missing champion snapshot handled");

    memset(&preIn, 0, sizeof(preIn));
    preIn.targetResolved = 1;
    preIn.creatureSnapshotReady = 1;
    preIn.championSnapshotReady = 1;
    CHECK_EQ(dm1_v1_melee_preflight_plan_f0402_pc34(&preIn, &preOut), 1,
             "F0402 ready preflight builds");
    CHECK_EQ(preOut.canResolveDamage, 1,
             "F0402 ready target can resolve F0231 damage");
}

static void test_melee_f0177_target_creature_plan(void) {
    DM1_MeleeF0177TargetCreatureInputPc34 in;
    DM1_MeleeF0177TargetCreaturePlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.groupCount = 2;
    in.groupCells = DM1_GROUP_CELLS_SINGLE_CENTERED;
    in.creatureHealth[0] = 0;
    in.creatureHealth[1] = 14;
    in.creatureHealth[2] = 9;
    CHECK_EQ(dm1_v1_melee_target_creature_plan_f0177_pc34(&in, &out), 1,
             "F0177 single-centered plan builds");
    CHECK_EQ(out.valid, 1, "F0177 single-centered valid");
    CHECK_EQ(out.singleCenteredGroup, 1, "F0177 single-centered flag");
    CHECK_EQ(out.firstLivingCreatureIndex, 1,
             "F0177 first living creature");
    CHECK_EQ(out.selectedCreatureIndex, 1,
             "F0177 single-centered selects first living");

    memset(&in, 0, sizeof(in));
    in.groupCount = 1;
    in.groupCells = (1 << 2) | 3;
    in.creatureHealth[0] = 8;
    in.creatureHealth[1] = 12;
    in.championCell = 0;
    in.targetDirection = 0;
    in.groupDirection = 0;
    CHECK_EQ(dm1_v1_melee_target_creature_plan_f0177_pc34(&in, &out), 1,
             "F0177 full-square plan builds");
    CHECK_EQ(out.selectedCreatureIndex, 1,
             "F0177 full-square ordered cell selects higher ordinal");

    memset(&in, 0, sizeof(in));
    in.groupCount = 0;
    in.groupCells = 0;
    in.groupDirection = 1;
    in.creatureSize = DM1_CREATURE_SIZE_HALF_SQUARE;
    in.creatureHealth[0] = 20;
    in.championCell = 0;
    in.targetDirection = 1;
    CHECK_EQ(dm1_v1_melee_target_creature_plan_f0177_pc34(&in, &out), 1,
             "F0177 half-square plan builds");
    CHECK_EQ(out.selectedCreatureIndex, 0,
             "F0177 half-square occupancy selects creature");
}

static void test_melee_f0312_strength_plan(void) {
    DM1_MeleeF0312StrengthInputPc34 in;
    DM1_MeleeF0312StrengthPlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.championStrength = 50;
    in.currentStamina = 80;
    in.maximumStamina = 100;
    in.maximumLoad = 400;
    in.random16 = 7;
    in.objectWeight = 30;
    in.hasActionHandWeapon = 1;
    in.weaponStrength = 12;
    in.weaponSkillBonus = 2;
    CHECK_EQ(dm1_v1_melee_strength_plan_f0312_pc34(&in, &out), 1,
             "F0312 action-hand strength plan builds");
    CHECK_EQ(out.valid, 1, "F0312 strength plan valid");
    CHECK_EQ(out.oneSixteenthMaximumLoad, 25,
             "F0312 one-sixteenth max load");
    CHECK_EQ(out.loadThreshold, 31, "F0312 load threshold");
    CHECK_EQ(out.strengthBeforeStamina, 75,
             "F0312 strength before stamina");
    CHECK_EQ(out.strengthAfterStamina, 75,
             "F0312 full stamina unchanged");
    CHECK_EQ(out.strengthAfterWound, 75,
             "F0312 no wound unchanged");
    CHECK_EQ(out.strengthActionHand, 37,
             "F0312 final bounded strength");

    memset(&in, 0, sizeof(in));
    in.championStrength = 40;
    in.currentStamina = 20;
    in.maximumStamina = 100;
    in.maximumLoad = 0;
    in.random16 = 0;
    in.hasActionHandWeapon = 0;
    in.actionHandWounded = 1;
    CHECK_EQ(dm1_v1_melee_strength_plan_f0312_pc34(&in, &out), 1,
             "F0312 empty-hand low-stamina plan builds");
    CHECK_EQ(out.oneSixteenthMaximumLoad, 26,
             "F0312 fallback max load from strength");
    CHECK_EQ(out.strengthBeforeStamina, 28,
             "F0312 empty hand object weight zero penalty");
    CHECK_EQ(out.strengthAfterStamina, 19,
             "F0312 stamina adjustment uses halved operand");
    CHECK_EQ(out.strengthAfterWound, 9,
             "F0312 action-hand wound halves strength");
    CHECK_EQ(out.strengthActionHand, 4,
             "F0312 wounded low-stamina final strength");

    memset(&in, 0, sizeof(in));
    in.championStrength = 5;
    in.currentStamina = 100;
    in.maximumStamina = 100;
    in.maximumLoad = 80;
    in.random16 = 0;
    in.objectWeight = 80;
    in.hasActionHandWeapon = 1;
    CHECK_EQ(dm1_v1_melee_strength_plan_f0312_pc34(&in, &out), 1,
             "F0312 heavy object penalty plan builds");
    CHECK_EQ(out.strengthActionHand, 0,
             "F0312 heavy penalty clamps negative strength");
}

static void test_melee_f0231_champion_snapshot_plan(void) {
    DM1_MeleeF0231ChampionSnapshotInputPc34 in;
    DM1_MeleeF0231ChampionSnapshotPlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.championIndex = 2;
    in.championPresent = 1;
    in.currentHealth = 44;
    in.actionSkillIndex = -1;
    in.weaponClass = DM1_WEAPON_CLASS_FIRST_BOW;
    CHECK_EQ(dm1_v1_melee_champion_snapshot_plan_f0231_pc34(&in, &out), 1,
             "F0231 bow fallback snapshot plan builds");
    CHECK_EQ(out.valid, 1, "F0231 snapshot valid");
    CHECK_EQ(out.normalizedActionSkillIndex, DM1_SKILL_IDX_SHOOT,
             "F0231 bow fallback skill");

    in.weaponClass = 0;
    CHECK_EQ(dm1_v1_melee_champion_snapshot_plan_f0231_pc34(&in, &out), 1,
             "F0231 swing fallback snapshot plan builds");
    CHECK_EQ(out.normalizedActionSkillIndex, DM1_SKILL_IDX_SWING,
             "F0231 swing fallback skill");

    in.weaponClass = 2;
    CHECK_EQ(dm1_v1_melee_champion_snapshot_plan_f0231_pc34(&in, &out), 1,
             "F0231 throw fallback snapshot plan builds");
    CHECK_EQ(out.normalizedActionSkillIndex, DM1_SKILL_IDX_THROW,
             "F0231 throw fallback skill");

    memset(&in, 0, sizeof(in));
    in.championIndex = 1;
    in.championPresent = 1;
    in.currentHealth = 77;
    in.dexterity = 66;
    in.strengthActionHand = 55;
    in.actionSkillIndex = 6;
    in.weaponClass = 3;
    in.skillLevelParry = 4;
    in.skillLevelAction = 5;
    in.statisticVitality = 40;
    in.statisticAntifire = 41;
    in.statisticAntimagic = 42;
    in.statisticWisdom = 43;
    in.statisticLuck = 44;
    in.statisticLuckMax = 99;
    in.statisticLuckMin = 3;
    in.actionHandIcon = 12;
    in.wounds = 0x22;
    in.isResting = 1;
    in.partyShieldDefense = 9;
    CHECK_EQ(dm1_v1_melee_champion_snapshot_plan_f0231_pc34(&in, &out), 1,
             "F0231 full champion snapshot plan builds");
    CHECK_EQ(out.normalizedActionSkillIndex, 6,
             "F0231 explicit action skill retained");
    CHECK_EQ(out.snapshot.championIndex, 1, "F0231 snapshot champion");
    CHECK_EQ(out.snapshot.currentHealth, 77, "F0231 snapshot health");
    CHECK_EQ(out.snapshot.dexterity, 66, "F0231 snapshot dexterity");
    CHECK_EQ(out.snapshot.strengthActionHand, 55,
             "F0231 snapshot strength");
    CHECK_EQ(out.snapshot.skillLevelParry, 4, "F0231 snapshot parry");
    CHECK_EQ(out.snapshot.skillLevelAction, 5, "F0231 snapshot action skill");
    CHECK_EQ(out.snapshot.statisticLuck, 44, "F0231 snapshot luck");
    CHECK_EQ(out.snapshot.statisticLuckMax, 99, "F0231 snapshot luck max");
    CHECK_EQ(out.snapshot.statisticLuckMin, 3, "F0231 snapshot luck min");
    CHECK_EQ(out.snapshot.actionHandIcon, 12, "F0231 snapshot hand icon");
    CHECK_EQ(out.snapshot.wounds, 0x22, "F0231 snapshot wounds");
    CHECK_EQ(out.snapshot.isResting, 1, "F0231 snapshot resting");
    CHECK_EQ(out.snapshot.partyShieldDefense, 9,
             "F0231 snapshot shield defense");

    in.currentHealth = 0;
    CHECK_EQ(dm1_v1_melee_champion_snapshot_plan_f0231_pc34(&in, &out), 0,
             "F0231 dead champion snapshot rejected");
}

static void test_melee_f0231_creature_snapshot_plan(void) {
    DM1_MeleeF0231CreatureSnapshotInputPc34 in;
    DM1_MeleeF0231CreatureSnapshotPlanPc34 out;

    memset(&in, 0, sizeof(in));
    in.groupIndex = 7;
    in.groupCount = 2;
    in.groupCreatureType = 12;
    in.creatureIndex = 1;
    in.creatureHealth = 88;
    in.profileAttack = 11;
    in.profileDefense = 22;
    in.profileDexterity = 33;
    in.profileBaseHealth = 44;
    in.profilePoisonAttack = 55;
    in.profileAttackType = 66;
    in.profileAttributes = 0x40;
    in.profileWoundProbabilities = 0x1234;
    in.profileProperties = 0x5678;
    in.doubledMapDifficulty = 6;
    in.candidateInvulnerableEnabled = 1;
    in.candidateInvulnerableGroupIndex = 7;
    in.candidateInvulnerableCreatureIndex = 1;
    CHECK_EQ(dm1_v1_melee_creature_snapshot_plan_f0231_pc34(&in, &out), 1,
             "F0231 creature snapshot plan builds");
    CHECK_EQ(out.valid, 1, "F0231 creature snapshot valid");
    CHECK_EQ(out.snapshot.creatureType, 12, "F0231 creature type");
    CHECK_EQ(out.snapshot.attack, 11, "F0231 creature attack");
    CHECK_EQ(out.snapshot.defense, 22, "F0231 creature defense");
    CHECK_EQ(out.snapshot.dexterity, 33, "F0231 creature dexterity");
    CHECK_EQ(out.snapshot.baseHealth, 44, "F0231 creature base health");
    CHECK_EQ(out.snapshot.poisonAttack, 55, "F0231 creature poison");
    CHECK_EQ(out.snapshot.attackType, 66, "F0231 creature attack type");
    CHECK_EQ(out.snapshot.attributes, 0x40, "F0231 creature attributes");
    CHECK_EQ(out.snapshot.woundProbabilities, 0x1234,
             "F0231 creature wounds");
    CHECK_EQ(out.snapshot.properties, 0x5678, "F0231 creature properties");
    CHECK_EQ(out.snapshot.doubledMapDifficulty, 6,
             "F0231 creature difficulty");
    CHECK_EQ(out.snapshot.creatureIndex, 1, "F0231 creature index");
    CHECK_EQ(out.snapshot.healthBefore, 88, "F0231 creature health");
    CHECK_EQ(out.snapshot.isCandidateInvulnerable, 1,
             "F0231 creature candidate invulnerable");

    in.candidateInvulnerableCreatureIndex = 2;
    CHECK_EQ(dm1_v1_melee_creature_snapshot_plan_f0231_pc34(&in, &out), 1,
             "F0231 creature snapshot noncandidate builds");
    CHECK_EQ(out.snapshot.isCandidateInvulnerable, 0,
             "F0231 creature noncandidate vulnerable");

    in.creatureIndex = 3;
    CHECK_EQ(dm1_v1_melee_creature_snapshot_plan_f0231_pc34(&in, &out), 0,
             "F0231 creature snapshot rejects absent group slot");
}

static void test_melee_f0231_damage_gate_plan(void) {
    DM1_MeleeF0231DamageGateInputPc34 in;
    DM1_MeleeF0231DamageGatePlanPc34 out;
    struct CombatantChampionSnapshot_Compat attacker;
    struct CombatantCreatureSnapshot_Compat defender;
    struct WeaponProfile_Compat weapon;
    struct CombatResult_Compat result;
    struct RngState_Compat rng;

    memset(&in, 0, sizeof(in));
    in.championIndex = 0;
    in.championCurrentHealth = 10;
    in.creatureType = 1;
    in.creatureDexterity = 20;
    in.creatureAttributes = 0;
    in.actionHitProbability = 0x8055;
    CHECK_EQ(dm1_v1_melee_damage_gate_plan_f0231_pc34(&in, &out), 1,
             "F0231 damage gate builds");
    CHECK_EQ(out.canEnterDamageBlock, 1, "F0231 damage gate enters RNG block");
    CHECK_EQ(out.normalizedHitProbability, 0x55,
             "F0231 damage gate normalizes hit probability");
    CHECK_EQ(out.actionHitsNonMaterial, 1,
             "F0231 damage gate detects non-material flag");

    in.isCandidateInvulnerable = 1;
    CHECK_EQ(dm1_v1_melee_damage_gate_plan_f0231_pc34(&in, &out), 1,
             "F0231 candidate gate builds");
    CHECK_EQ(out.shouldReturnResolved, 1,
             "F0231 candidate gate resolves before RNG");
    CHECK_EQ(out.resolvedOutcome, COMBAT_OUTCOME_NO_ACTION,
             "F0231 candidate gate returns no-action");

    in.isCandidateInvulnerable = 0;
    in.creatureDexterity = 255;
    CHECK_EQ(dm1_v1_melee_damage_gate_plan_f0231_pc34(&in, &out), 1,
             "F0231 dexterity-255 gate builds");
    CHECK_EQ(out.shouldReturnResolved, 1,
             "F0231 dexterity-255 gate resolves before RNG");
    CHECK_EQ(out.resolvedOutcome, COMBAT_OUTCOME_MISS,
             "F0231 dexterity-255 gate returns miss");

    in.creatureDexterity = 20;
    in.creatureAttributes = 0x0040;
    in.actionHitProbability = 0x0030;
    CHECK_EQ(dm1_v1_melee_damage_gate_plan_f0231_pc34(&in, &out), 1,
             "F0231 non-material gate builds");
    CHECK_EQ(out.creatureIsNonMaterial, 1,
             "F0231 non-material gate detects creature attribute");
    CHECK_EQ(out.shouldReturnResolved, 1,
             "F0231 non-material gate resolves before RNG");

    memset(&attacker, 0, sizeof(attacker));
    memset(&defender, 0, sizeof(defender));
    memset(&weapon, 0, sizeof(weapon));
    memset(&result, 0, sizeof(result));
    attacker.championIndex = 0;
    attacker.currentHealth = 10;
    defender.creatureType = 1;
    defender.dexterity = 255;
    weapon.hitProbability = 50;
    CHECK_EQ(F0730_COMBAT_RngInit_Compat(&rng, 0x1234u), 1,
             "F0231 gate rng init");
    CHECK_EQ(dm1_v1_melee_resolve_damage_f0231_pc34(
                 &attacker, &weapon, &defender, &rng, &result), 1,
             "F0231 DM1 wrapper resolves dexterity-255");
    CHECK_EQ(result.outcome, COMBAT_OUTCOME_MISS,
             "F0231 DM1 wrapper returns dexterity-255 miss");
    CHECK_EQ(result.rngCallCount, 0,
             "F0231 DM1 wrapper dexterity-255 consumes no RNG");

    defender.dexterity = 20;
    defender.attributes = 0x0040;
    CHECK_EQ(dm1_v1_melee_resolve_damage_f0231_pc34(
                 &attacker, &weapon, &defender, &rng, &result), 1,
             "F0231 DM1 wrapper resolves non-material block");
    CHECK_EQ(result.outcome, COMBAT_OUTCOME_MISS,
             "F0231 DM1 wrapper returns non-material miss");
    CHECK_EQ(result.rngCallCount, 0,
             "F0231 DM1 wrapper non-material block consumes no RNG");
}

static void test_melee_f0231_aftermath_plan(void) {
    DM1_MeleeF0231AftermathInputPc34 in;
    DM1_MeleeF0231AftermathPlanPc34 out;
    DM1_MeleeF0231RawGroupWritebackPlanPc34 rawWritebackOut;
    DM1_MeleeF0231AftermathApplyPlanPc34 applyOut;

    memset(&in, 0, sizeof(in));
    in.groupIndex = 4;
    in.creatureIndex = 2;
    in.creatureType = 6;
    in.creatureProperties = 0x0230;
    in.groupBehavior = DM1_BEHAVIOR_ATTACK;
    in.originalGroupCount = 3;
    in.partyMapIndex = 3;
    in.partyMapX = 7;
    in.partyMapY = 9;
    in.targetMapIndex = 3;
    in.targetMapX = 8;
    in.targetMapY = 9;
    in.currentTick = 77u;
    in.creatureAttributes = DM1_SIZE_FULL_SQUARE;
    in.killedCell = EXPLOSION_CELL_CENTERED;
    in.damageOutcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    in.fallbackCombatOutcome = COMBAT_OUTCOME_HIT_DAMAGE;
    CHECK_EQ(dm1_v1_melee_aftermath_plan_f0231_pc34(&in, &out), 1,
             "F0231 killed-all aftermath builds");
    CHECK_EQ(out.valid, 1, "F0231 aftermath valid");
    CHECK_EQ(out.outcome, COMBAT_OUTCOME_KILLED_ALL_CREATURES,
             "F0231 killed-all outcome");
    CHECK_EQ(out.shouldDropPossessions, 1, "F0231 killed-all drops");
    CHECK_EQ(out.shouldCreateDeathSmoke, 1, "F0231 killed-all smoke");
    CHECK_EQ(out.smokeAttack, 255, "F0231 full-square smoke attack");
    CHECK_EQ(out.smokeCell, EXPLOSION_CELL_CENTERED,
             "F0231 centered smoke cell");
    CHECK_EQ(out.smokeCreateInput.attack, 255,
             "F0231 killed-all smoke create attack");
    CHECK_EQ(out.smokeCreateInput.mapIndex, 3,
             "F0231 killed-all smoke create map");
    CHECK_EQ(out.smokeCreateInput.mapX, 8,
             "F0231 killed-all smoke create x");
    CHECK_EQ(out.smokeCreateInput.mapY, 9,
             "F0231 killed-all smoke create y");
    CHECK_EQ(out.smokeCreateInput.currentTick, 77,
             "F0231 killed-all smoke create tick");
    CHECK_EQ(out.shouldApplyKilledSomeState, 0,
             "F0231 killed-all skips killed-some state");
    CHECK_EQ(out.shouldApplyKilledAllSideEffects, 1,
             "F0231 killed-all unlinks group");
    CHECK_EQ(out.mutationOutcome, COMBAT_OUTCOME_KILLED_ALL_CREATURES,
             "F0231 killed-all mutation outcome");
    CHECK_EQ(out.mutationGroupIndex, 4,
             "F0231 killed-all mutation group");
    CHECK_EQ(out.mutationOriginalGroupCount, 3,
             "F0231 killed-all mutation original count");
    CHECK_EQ(out.mutationPartyMapX, 7,
             "F0231 killed-all mutation party x");
    CHECK_EQ(out.shouldWriteRawGroup, 1, "F0231 killed-all writeback");
    CHECK_EQ(dm1_v1_melee_aftermath_raw_group_writeback_plan_f0231_pc34(
                 &out, &rawWritebackOut), 1,
             "F0231 killed-all raw writeback receipt builds");
    CHECK_EQ(rawWritebackOut.valid, 1,
             "F0231 killed-all raw writeback valid");
    CHECK_EQ(rawWritebackOut.shouldWriteRawGroup, 1,
             "F0231 killed-all raw writeback enabled");
    CHECK_EQ(rawWritebackOut.groupIndex, 4,
             "F0231 killed-all raw writeback group");
    CHECK_EQ(dm1_v1_melee_aftermath_apply_plan_f0231_pc34(
                 &out, &applyOut), 1,
             "F0231 killed-all aftermath apply receipt builds");
    CHECK_EQ(applyOut.valid, 1,
             "F0231 killed-all aftermath apply receipt valid");
    CHECK_EQ(applyOut.shouldCreateDeathSmoke, 1,
             "F0231 killed-all apply smoke");
    CHECK_EQ(applyOut.shouldApplyMutationDispatch, 1,
             "F0231 killed-all apply mutation dispatch");
    CHECK_EQ(applyOut.mutationDispatchPlan.valid, 1,
             "F0231 killed-all apply mutation dispatch valid");
    CHECK_EQ(applyOut.mutationDispatchPlan.shouldApplyKilledAllSideEffects, 1,
             "F0231 killed-all apply mutation killed-all side effects");
    CHECK_EQ(applyOut.shouldWriteRawGroup, 1,
             "F0231 killed-all apply raw writeback");
    CHECK_EQ(applyOut.rawGroupWritebackPlan.groupIndex, 4,
             "F0231 killed-all apply raw group");
    CHECK_EQ(out.shouldEmitKillNotify, 1, "F0231 killed-all notifies");
    CHECK_EQ(out.killNotifyGroupIndex, 4,
             "F0231 killed-all notify group");
    CHECK_EQ(out.killNotifyCreatureIndex, 2,
             "F0231 killed-all notify creature");
    CHECK_EQ(out.killNotifyOutcome, COMBAT_OUTCOME_KILLED_ALL_CREATURES,
             "F0231 killed-all notify outcome");
    CHECK_EQ(out.killNotifyCreatureType, 6,
             "F0231 killed-all notify creature type");
    CHECK_EQ(applyOut.shouldEmitKillNotify, 1,
             "F0231 killed-all apply notify");
    CHECK_EQ(applyOut.killNotifyGroupIndex, 4,
             "F0231 killed-all apply notify group");
    CHECK_EQ(out.shouldScheduleReaction, 0,
             "F0231 killed-all suppresses reaction");

    in.creatureAttributes = DM1_SIZE_HALF_SQUARE;
    in.killedCell = 6;
    in.damageOutcome = COMBAT_OUTCOME_KILLED_SOME_CREATURES;
    in.fearTriggered = 0;
    CHECK_EQ(dm1_v1_melee_aftermath_plan_f0231_pc34(&in, &out), 1,
             "F0231 killed-some aftermath builds");
    CHECK_EQ(out.outcome, COMBAT_OUTCOME_KILLED_SOME_CREATURES,
             "F0231 killed-some outcome");
    CHECK_EQ(out.smokeAttack, 190, "F0231 half-square smoke attack");
    CHECK_EQ(out.smokeCell, 2, "F0231 smoke cell masked");
    CHECK_EQ(out.smokeCreateInput.cell, 2,
             "F0231 killed-some smoke create cell");
    CHECK_EQ(out.shouldDropPossessions, 1, "F0231 killed-some drops");
    CHECK_EQ(out.shouldCreateDeathSmoke, 1, "F0231 killed-some smoke");
    CHECK_EQ(out.shouldApplyKilledSomeState, 1,
             "F0231 killed-some state/fear");
    CHECK_EQ(out.mutationKilledCreatureIndex, 2,
             "F0231 killed-some mutation creature");
    CHECK_EQ(out.mutationCreatureProperties, 0x0230,
             "F0231 killed-some mutation properties");
    CHECK_EQ(out.mutationKilledCell, 6,
             "F0231 killed-some mutation killed cell");
    CHECK_EQ(out.shouldApplyKilledAllSideEffects, 0,
             "F0231 killed-some keeps group linked");
    CHECK_EQ(out.shouldScheduleReaction, 1,
             "F0231 killed-some reaction without fear");
    CHECK_EQ((int)out.reactionFireAtTick, 78,
             "F0231 killed-some reaction tick");
    CHECK_EQ(out.reactionMapIndex, 3,
             "F0231 killed-some reaction map");
    CHECK_EQ(out.reactionMapX, 8,
             "F0231 killed-some reaction x");
    CHECK_EQ(out.reactionMapY, 9,
             "F0231 killed-some reaction y");
    CHECK_EQ(out.reactionGroupIndex, 4,
             "F0231 killed-some reaction group");
    CHECK_EQ(out.reactionCreatureType, 6,
             "F0231 killed-some reaction creature type");
    CHECK_EQ(out.reactionEventKind, DM1_EVENT_REACTION_PARTY_IS_ADJACENT,
             "F0231 reaction event kind");

    in.fearTriggered = 1;
    CHECK_EQ(dm1_v1_melee_aftermath_plan_f0231_pc34(&in, &out), 1,
             "F0231 fear aftermath builds");
    CHECK_EQ(out.shouldScheduleReaction, 0,
             "F0231 fear suppresses reaction");
    CHECK_EQ(out.reactionGroupIndex, -1,
             "F0231 fear clears reaction group");

    in.creatureAttributes = 0;
    in.damageOutcome = COMBAT_OUTCOME_KILLED_NO_CREATURES;
    in.fallbackCombatOutcome = COMBAT_OUTCOME_HIT_DAMAGE;
    in.fearTriggered = 0;
    CHECK_EQ(dm1_v1_melee_aftermath_plan_f0231_pc34(&in, &out), 1,
             "F0231 no-kill aftermath builds");
    CHECK_EQ(out.outcome, COMBAT_OUTCOME_KILLED_NO_CREATURES,
             "F0231 no-kill outcome");
    CHECK_EQ(out.smokeAttack, 110, "F0231 quarter/default smoke attack");
    CHECK_EQ(out.shouldDropPossessions, 0, "F0231 no-kill no drops");
    CHECK_EQ(out.mutationGroupIndex, -1,
             "F0231 no-kill clears mutation group");
    CHECK_EQ(out.shouldCreateDeathSmoke, 0, "F0231 no-kill no smoke");
    CHECK_EQ(out.shouldWriteRawGroup, 1, "F0231 no-kill writeback");
    CHECK_EQ(dm1_v1_melee_aftermath_raw_group_writeback_plan_f0231_pc34(
                 &out, &rawWritebackOut), 1,
             "F0231 no-kill raw writeback receipt builds");
    CHECK_EQ(rawWritebackOut.shouldWriteRawGroup, 1,
             "F0231 no-kill raw writeback enabled");
    CHECK_EQ(rawWritebackOut.groupIndex, 4,
             "F0231 no-kill raw writeback uses reaction group");
    CHECK_EQ(dm1_v1_melee_aftermath_apply_plan_f0231_pc34(
                 &out, &applyOut), 1,
             "F0231 no-kill aftermath apply receipt builds");
    CHECK_EQ(applyOut.shouldCreateDeathSmoke, 0,
             "F0231 no-kill apply no smoke");
    CHECK_EQ(applyOut.shouldApplyMutationDispatch, 0,
             "F0231 no-kill apply no mutation dispatch");
    CHECK_EQ(applyOut.mutationDispatchPlan.valid, 0,
             "F0231 no-kill apply no mutation plan");
    CHECK_EQ(applyOut.shouldWriteRawGroup, 1,
             "F0231 no-kill apply raw writeback");
    CHECK_EQ(applyOut.shouldScheduleReaction, 1,
             "F0231 no-kill apply reaction");
    CHECK_EQ(out.shouldEmitKillNotify, 0, "F0231 no-kill no notify");
    CHECK_EQ(out.killNotifyGroupIndex, -1,
             "F0231 no-kill clears notify group");
    CHECK_EQ(out.shouldScheduleReaction, 1, "F0231 no-kill reaction");

    in.damageOutcome = COMBAT_OUTCOME_INVALID;
    in.fallbackCombatOutcome = COMBAT_OUTCOME_MISS;
    CHECK_EQ(dm1_v1_melee_aftermath_plan_f0231_pc34(&in, &out), 1,
             "F0231 miss aftermath builds");
    CHECK_EQ(out.outcome, COMBAT_OUTCOME_MISS, "F0231 miss fallback outcome");
    CHECK_EQ(out.shouldDropPossessions, 0, "F0231 miss no drops");
    CHECK_EQ(out.shouldCreateDeathSmoke, 0, "F0231 miss no smoke");
    CHECK_EQ(out.shouldWriteRawGroup, 0, "F0231 miss no writeback");
    CHECK_EQ(dm1_v1_melee_aftermath_raw_group_writeback_plan_f0231_pc34(
                 &out, &rawWritebackOut), 1,
             "F0231 miss raw writeback receipt builds");
    CHECK_EQ(rawWritebackOut.shouldWriteRawGroup, 0,
             "F0231 miss raw writeback disabled");
    CHECK_EQ(dm1_v1_melee_aftermath_apply_plan_f0231_pc34(
                 &out, &applyOut), 1,
             "F0231 miss aftermath apply receipt builds");
    CHECK_EQ(applyOut.shouldApplyMutationDispatch, 0,
             "F0231 miss apply no mutation dispatch");
    CHECK_EQ(applyOut.shouldWriteRawGroup, 0,
             "F0231 miss apply no raw writeback");
    CHECK_EQ(applyOut.shouldScheduleReaction, 1,
             "F0231 miss apply reaction");
    CHECK_EQ(out.shouldEmitKillNotify, 0, "F0231 miss no notify");
    CHECK_EQ(out.shouldScheduleReaction, 1, "F0231 miss reaction");
}

static void test_melee_f0231_reaction_and_group_apply(void) {
    DM1_MeleeF0231ReactionInputPc34 reactionIn;
    DM1_MeleeF0231ReactionPlanPc34 reactionOut;
    DM1_MeleeF0190DeathSmokeInputPc34 smokeIn;
    DM1_MeleeF0190DeathSmokePlanPc34 smokeOut;
    DM1_MeleeF0190PossessionDropInputPc34 dropIn;
    DM1_MeleeF0190PossessionDropPlanPc34 dropOut;
    DM1_MeleeF0190PossessionDropApplyPlanPc34 dropApplyOut;
    DM1_MeleeF0190KilledSomeStateInputPc34 stateIn;
    DM1_MeleeF0190KilledSomeStatePlanPc34 stateOut;
    DM1_MeleeF0190KilledSomeStatePlanPc34 stateApplyOut;
    DM1_MeleeF0190KilledSomeStateApplyPlanPc34 killedSomeApplyOut;
    DM1_MeleeF0190KilledAllStateInputPc34 killedAllIn;
    DM1_MeleeF0190KilledAllStatePlanPc34 killedAllOut;
    DM1_MeleeF0190KilledAllStateApplyPlanPc34 killedAllApplyOut;
    DM1_MeleeF0190TimelineCleanupInputPc34 cleanupIn;
    DM1_MeleeF0190TimelineCleanupPlanPc34 cleanupOut;
    DM1_MeleeF0190TimelineCleanupBatchInputPc34 cleanupBatchIn;
    DM1_MeleeF0190TimelineCleanupBatchPlanPc34 cleanupBatchOut;
    DM1_MeleeF0190FixedDropCellsPlanPc34 fixedCellsOut;
    DM1_MeleeF0190MutationDispatchInputPc34 dispatchIn;
    DM1_MeleeF0190MutationDispatchPlanPc34 dispatchOut;
    DM1_MeleeF0190MutationDispatchApplyPlanPc34 dispatchApplyOut;
    DM1_MeleeF0231AftermathInputPc34 aftermathIn;
    DM1_MeleeF0231AftermathPlanPc34 aftermathOut;
    DM1_MeleeF0190MutationDispatchPlanPc34 aftermathDispatchOut;
    DM1_MeleeF0190FearRollPlanPc34 fearRollOut;
    DM1_MeleeF0190KilledSomeStatePlanPc34 fearApplyOut;
    DM1_MeleeF0188GroupSlotDropInputPc34 slotDropIn;
    DM1_MeleeF0188GroupSlotDropPlanPc34 slotDropOut;
    struct DungeonGroup_Compat fixedGroup;
    unsigned char movingCells[4];
    struct CombatResult_Compat resultA;
    struct CombatResult_Compat resultB;
    struct DungeonGroup_Compat groupA;
    struct DungeonGroup_Compat groupB;
    DM1_MeleeF0190GroupDamageApplyPlanPc34 applyPlan;
    int outcomeA = -1;
    int outcomeB = -1;

    memset(&reactionIn, 0, sizeof(reactionIn));
    reactionIn.groupIndex = 8;
    reactionIn.creatureType = 12;
    reactionIn.mapIndex = 2;
    reactionIn.mapX = 4;
    reactionIn.mapY = 5;
    reactionIn.currentTick = 123u;
    reactionIn.outcome = COMBAT_OUTCOME_HIT_DAMAGE;
    CHECK_EQ(dm1_v1_melee_reaction_plan_f0231_pc34(
                 &reactionIn, &reactionOut), 1,
             "F0231 reaction plan builds");
    CHECK_EQ(reactionOut.valid, 1, "F0231 reaction valid");
    CHECK_EQ(reactionOut.shouldSchedule, 1, "F0231 reaction schedules");
    CHECK_EQ((int)reactionOut.fireAtTick, 124, "F0231 reaction tick");
    CHECK_EQ(reactionOut.mapIndex, 2, "F0231 reaction map");
    CHECK_EQ(reactionOut.mapX, 4, "F0231 reaction x");
    CHECK_EQ(reactionOut.mapY, 5, "F0231 reaction y");
    CHECK_EQ(reactionOut.groupIndex, 8, "F0231 reaction group");
    CHECK_EQ(reactionOut.creatureType, 12, "F0231 reaction creature");
    CHECK_EQ(reactionOut.eventKind, DM1_EVENT_REACTION_PARTY_IS_ADJACENT,
             "F0231 reaction event kind");

    reactionIn.outcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    CHECK_EQ(dm1_v1_melee_reaction_plan_f0231_pc34(
                 &reactionIn, &reactionOut), 1,
             "F0231 killed-all reaction plan builds");
    CHECK_EQ(reactionOut.shouldSchedule, 0,
             "F0231 killed-all suppresses reaction");

    memset(&smokeIn, 0, sizeof(smokeIn));
    smokeIn.shouldCreate = 1;
    smokeIn.smokeAttack = 190;
    smokeIn.smokeCell = 6;
    smokeIn.mapIndex = 3;
    smokeIn.mapX = 9;
    smokeIn.mapY = 10;
    smokeIn.currentTick = 222;
    CHECK_EQ(dm1_v1_melee_death_smoke_plan_f0190_pc34(
                 &smokeIn, &smokeOut), 1,
             "F0190 death smoke plan builds");
    CHECK_EQ(smokeOut.valid, 1, "F0190 death smoke valid");
    CHECK_EQ(smokeOut.shouldCreate, 1, "F0190 death smoke creates");
    CHECK_EQ(smokeOut.createInput.explosionType, C040_EXPLOSION_SMOKE,
             "F0190 death smoke type");
    CHECK_EQ(smokeOut.createInput.attack, 190, "F0190 death smoke attack");
    CHECK_EQ(smokeOut.createInput.mapIndex, 3, "F0190 death smoke map");
    CHECK_EQ(smokeOut.createInput.mapX, 9, "F0190 death smoke x");
    CHECK_EQ(smokeOut.createInput.mapY, 10, "F0190 death smoke y");
    CHECK_EQ(smokeOut.createInput.cell, 2, "F0190 death smoke cell mask");
    CHECK_EQ(smokeOut.createInput.centered, 0, "F0190 death smoke side cell");
    CHECK_EQ(smokeOut.createInput.currentTick, 222,
             "F0190 death smoke tick");
    CHECK_EQ(smokeOut.createInput.ownerKind, PROJECTILE_OWNER_CHAMPION,
             "F0190 death smoke owner kind");
    CHECK_EQ(smokeOut.createInput.ownerIndex, -1,
             "F0190 death smoke owner index");

    smokeIn.smokeCell = EXPLOSION_CELL_CENTERED;
    CHECK_EQ(dm1_v1_melee_death_smoke_plan_f0190_pc34(
                 &smokeIn, &smokeOut), 1,
             "F0190 centered death smoke plan builds");
    CHECK_EQ(smokeOut.createInput.cell, EXPLOSION_CELL_CENTERED,
             "F0190 death smoke centered cell");
    CHECK_EQ(smokeOut.createInput.centered, 1,
             "F0190 death smoke centered flag");
    CHECK_EQ(dm1_v1_melee_death_smoke_attack_f0190_pc34(
                 DM1_SIZE_QUARTER_SQUARE), 110,
             "F0190 quarter-square smoke attack helper");
    CHECK_EQ(dm1_v1_melee_death_smoke_attack_f0190_pc34(
                 DM1_SIZE_HALF_SQUARE), 190,
             "F0190 half-square smoke attack helper");
    CHECK_EQ(dm1_v1_melee_death_smoke_attack_f0190_pc34(
                 DM1_SIZE_FULL_SQUARE), 255,
             "F0190 full-square smoke attack helper");

    smokeIn.shouldCreate = 0;
    CHECK_EQ(dm1_v1_melee_death_smoke_plan_f0190_pc34(
                 &smokeIn, &smokeOut), 1,
             "F0190 no-smoke plan builds");
    CHECK_EQ(smokeOut.shouldCreate, 0, "F0190 no-smoke suppresses create");

    memset(&dropIn, 0, sizeof(dropIn));
    dropIn.outcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    dropIn.creatureType = 12;
    dropIn.creatureAttributes = DM1_ATTR_DROP_FIXED_POSS;
    dropIn.killedCell = EXPLOSION_CELL_CENTERED;
    dropIn.mapIndex = 4;
    dropIn.mapX = 5;
    dropIn.mapY = 6;
    CHECK_EQ(dm1_v1_melee_possession_drop_plan_f0190_pc34(
                 &dropIn, &dropOut), 1,
             "F0190 killed-all drop plan builds");
    CHECK_EQ(dropOut.valid, 1, "F0190 killed-all drop valid");
    CHECK_EQ(dropOut.shouldDropGroupFixedPossessions, 1,
             "F0190 killed-all drops group fixed");
    CHECK_EQ(dropOut.shouldDropGroupSlotPossessions, 1,
             "F0190 killed-all drops group slot");
    CHECK_EQ(dropOut.shouldDropCreatureFixedPossessions, 0,
             "F0190 killed-all skips per-creature fixed branch");
    CHECK_EQ(dropOut.creatureCell, EXPLOSION_CELL_CENTERED,
             "F0190 killed-all centered cell");
    CHECK_EQ(dropOut.mapIndex, 4, "F0190 drop map");
    CHECK_EQ(dropOut.mapX, 5, "F0190 drop x");
    CHECK_EQ(dropOut.mapY, 6, "F0190 drop y");
    memset(&fixedGroup, 0, sizeof(fixedGroup));
    fixedGroup.creatureType = 12;
    fixedGroup.count = 2;
    fixedGroup.cells = (unsigned char)((3u << 4) | (2u << 2) | 1u);
    CHECK_EQ(dm1_v1_melee_possession_drop_apply_plan_f0190_pc34(
                 &dropOut, &fixedGroup, &dropApplyOut), 1,
             "F0190 killed-all drop apply receipt builds");
    CHECK_EQ(dropApplyOut.valid, 1, "F0190 killed-all drop apply valid");
    CHECK_EQ(dropApplyOut.shouldDropGroupFixedPossessions, 1,
             "F0190 killed-all apply drops group fixed");
    CHECK_EQ(dropApplyOut.shouldDropGroupSlotPossessions, 1,
             "F0190 killed-all apply drops group slot");
    CHECK_EQ(dropApplyOut.groupFixedCellCount, 3,
             "F0190 killed-all apply group cell count");
    CHECK_EQ(dropApplyOut.groupFixedCells[0], 3,
             "F0190 killed-all apply first group cell");
    CHECK_EQ(dropApplyOut.groupFixedCells[2], 1,
             "F0190 killed-all apply last group cell");
    CHECK_EQ(dropApplyOut.mapX, 5, "F0190 killed-all apply map x");

    dropIn.outcome = COMBAT_OUTCOME_KILLED_SOME_CREATURES;
    dropIn.killedCell = 7;
    CHECK_EQ(dm1_v1_melee_possession_drop_plan_f0190_pc34(
                 &dropIn, &dropOut), 1,
             "F0190 killed-some drop plan builds");
    CHECK_EQ(dropOut.shouldDropGroupFixedPossessions, 0,
             "F0190 killed-some no group fixed");
    CHECK_EQ(dropOut.shouldDropGroupSlotPossessions, 0,
             "F0190 killed-some no group slot");
    CHECK_EQ(dropOut.shouldDropCreatureFixedPossessions, 1,
             "F0190 killed-some drops creature fixed");
    CHECK_EQ(dropOut.creatureType, 12, "F0190 killed-some creature type");
    CHECK_EQ(dropOut.creatureCell, 3, "F0190 killed-some cell mask");
    CHECK_EQ(dm1_v1_melee_possession_drop_apply_plan_f0190_pc34(
                 &dropOut, &fixedGroup, &dropApplyOut), 1,
             "F0190 killed-some drop apply receipt builds");
    CHECK_EQ(dropApplyOut.shouldDropCreatureFixedPossessions, 1,
             "F0190 killed-some apply drops creature fixed");
    CHECK_EQ(dropApplyOut.shouldDropGroupFixedPossessions, 0,
             "F0190 killed-some apply skips group fixed");
    CHECK_EQ(dropApplyOut.creatureCell, 3,
             "F0190 killed-some apply keeps creature cell");

    dropIn.creatureAttributes = 0;
    CHECK_EQ(dm1_v1_melee_possession_drop_plan_f0190_pc34(
                 &dropIn, &dropOut), 1,
             "F0190 killed-some no-fixed plan builds");
    CHECK_EQ(dropOut.shouldDropCreatureFixedPossessions, 0,
             "F0190 killed-some no fixed attr suppresses drop");

    memset(&fixedGroup, 0, sizeof(fixedGroup));
    fixedGroup.count = 2;
    fixedGroup.cells = (unsigned char)((1u << 0) | (3u << 2) | (2u << 4));
    CHECK_EQ(dm1_v1_melee_group_fixed_drop_cells_plan_f0190_pc34(
                 &fixedGroup, &fixedCellsOut), 1,
             "F0190 group fixed cells plan builds");
    CHECK_EQ(fixedCellsOut.valid, 1, "F0190 group fixed cells valid");
    CHECK_EQ(fixedCellsOut.dropCellCount, 3,
             "F0190 group fixed cells count down from group count");
    CHECK_EQ(fixedCellsOut.dropCells[0], 2,
             "F0190 group fixed cells first highest creature");
    CHECK_EQ(fixedCellsOut.dropCells[1], 3,
             "F0190 group fixed cells second middle creature");
    CHECK_EQ(fixedCellsOut.dropCells[2], 1,
             "F0190 group fixed cells last lowest creature");

    fixedGroup.cells = 0xFFu;
    CHECK_EQ(dm1_v1_melee_group_fixed_drop_cells_plan_f0190_pc34(
                 &fixedGroup, &fixedCellsOut), 1,
             "F0190 centered group fixed cells plan builds");
    CHECK_EQ(fixedCellsOut.dropCells[0], EXPLOSION_CELL_CENTERED,
             "F0190 centered group uses centered fixed-drop cell");

    movingCells[0] = 1;
    movingCells[1] = 2;
    movingCells[2] = 3;
    CHECK_EQ(dm1_v1_melee_moving_fixed_drop_cells_plan_f0187_pc34(
                 movingCells, 3, &fixedCellsOut), 1,
             "F0187 moving fixed cells plan builds");
    CHECK_EQ(fixedCellsOut.dropCellCount, 3,
             "F0187 moving fixed cells count");
    CHECK_EQ(fixedCellsOut.dropCells[0], 3,
             "F0187 moving fixed cells consumed as stack first");
    CHECK_EQ(fixedCellsOut.dropCells[2], 1,
             "F0187 moving fixed cells consumed as stack last");

    memset(&slotDropIn, 0, sizeof(slotDropIn));
    slotDropIn.slotHead = (unsigned short)((THING_TYPE_WEAPON << 10) | 7);
    slotDropIn.chainEntryCount = 3;
    slotDropIn.chain[0].thing =
        (unsigned short)((THING_TYPE_WEAPON << 10) | 7);
    slotDropIn.chain[0].nextThing =
        (unsigned short)((THING_TYPE_JUNK << 10) | 2);
    slotDropIn.chain[1].thing =
        (unsigned short)((THING_TYPE_JUNK << 10) | 2);
    slotDropIn.chain[1].nextThing =
        (unsigned short)((THING_TYPE_ARMOUR << 10) | 5);
    slotDropIn.chain[2].thing =
        (unsigned short)((THING_TYPE_ARMOUR << 10) | 5);
    slotDropIn.chain[2].nextThing = THING_ENDOFLIST;
    slotDropIn.randomCellCount = 3;
    slotDropIn.randomCells[0] = 3;
    slotDropIn.randomCells[1] = 1;
    slotDropIn.randomCells[2] = 2;
    CHECK_EQ(dm1_v1_melee_group_slot_drop_plan_f0188_pc34(
                 &slotDropIn, &slotDropOut), 1,
             "F0188 group slot drop plan builds");
    CHECK_EQ(slotDropOut.valid, 1, "F0188 group slot valid");
    CHECK_EQ(slotDropOut.shouldDrop, 1, "F0188 group slot should drop");
    CHECK_EQ(slotDropOut.stepCount, 3, "F0188 group slot step count");
    CHECK_EQ(slotDropOut.truncated, 0, "F0188 group slot not truncated");
    CHECK_EQ(slotDropOut.weaponDropped, 1,
             "F0188 group slot records weapon thud");
    CHECK_EQ(slotDropOut.soundId, 0, "F0188 group slot metallic sound");
    CHECK_EQ(slotDropOut.steps[0].sourceThing, slotDropIn.chain[0].thing,
             "F0188 group slot first source");
    CHECK_EQ((int)THING_GET_CELL(slotDropOut.steps[0].droppedThing), 3,
             "F0188 group slot first random cell");
    CHECK_EQ(slotDropOut.steps[1].nextThing, slotDropIn.chain[2].thing,
             "F0188 group slot snapshots next thing");
    CHECK_EQ((int)THING_GET_CELL(slotDropOut.steps[2].droppedThing), 2,
             "F0188 group slot third random cell");
    CHECK_EQ(slotDropOut.shouldClearGroupSlot, 1,
             "F0188 group slot clears source slot");

    slotDropIn.slotHead = (unsigned short)((THING_TYPE_JUNK << 10) | 1);
    slotDropIn.chainEntryCount = 1;
    slotDropIn.chain[0].thing = slotDropIn.slotHead;
    slotDropIn.chain[0].nextThing = THING_ENDOFLIST;
    slotDropIn.randomCellCount = 1;
    slotDropIn.randomCells[0] = 6;
    CHECK_EQ(dm1_v1_melee_group_slot_drop_plan_f0188_pc34(
                 &slotDropIn, &slotDropOut), 1,
             "F0188 non-weapon group slot drop plan builds");
    CHECK_EQ(slotDropOut.weaponDropped, 0,
             "F0188 non-weapon group slot no metallic thud");
    CHECK_EQ(slotDropOut.soundId, 4, "F0188 non-weapon wooden sound");
    CHECK_EQ((int)THING_GET_CELL(slotDropOut.steps[0].droppedThing), 2,
             "F0188 random cell masks to two bits");

    slotDropIn.slotHead = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    slotDropIn.chainEntryCount = 1;
    slotDropIn.chain[0].thing = (unsigned short)((THING_TYPE_JUNK << 10) | 1);
    CHECK_EQ(dm1_v1_melee_group_slot_drop_plan_f0188_pc34(
                 &slotDropIn, &slotDropOut), 1,
             "F0188 mismatched chain plan builds");
    CHECK_EQ(slotDropOut.truncated, 1,
             "F0188 mismatched chain reports truncation");

    memset(&stateIn, 0, sizeof(stateIn));
    stateIn.outcome = COMBAT_OUTCOME_KILLED_SOME_CREATURES;
    stateIn.groupBehavior = DM1_BEHAVIOR_ATTACK;
    stateIn.groupIndex = 3;
    stateIn.killedCreatureIndex = 2;
    stateIn.originalGroupCount = 3;
    stateIn.mapIndex = 1;
    stateIn.mapX = 10;
    stateIn.mapY = 11;
    stateIn.partyMapIndex = 1;
    stateIn.partyMapX = 9;
    stateIn.partyMapY = 11;
    stateIn.creatureType = 12;
    stateIn.creatureProperties = 0x0230;
    CHECK_EQ(dm1_v1_melee_killed_some_state_plan_f0190_pc34(
                 &stateIn, &stateOut), 1,
             "F0190 killed-some state plan builds");
    CHECK_EQ(stateOut.valid, 1, "F0190 killed-some state valid");
    CHECK_EQ(stateOut.shouldCleanupCreatureEvents, 1,
             "F0190 killed-some cleans creature events");
    CHECK_EQ(stateOut.shouldEvaluateFear, 1,
             "F0190 killed-some evaluates fear on party map");
    CHECK_EQ(stateOut.killedCreatureIndex, 2,
             "F0190 killed-some killed creature index");
    CHECK_EQ(stateOut.fearContext.creatureType, 12,
             "F0190 killed-some fear creature type");
    CHECK_EQ(stateOut.fearContext.creatureInfo.properties, 0x0230,
             "F0190 killed-some fear properties");
    CHECK_EQ(stateOut.fearContext.creatureCount, 3,
             "F0190 killed-some fear creature count");

    CHECK_EQ(dm1_v1_melee_killed_some_fear_apply_plan_f0190_pc34(
                 &stateIn, 1, 44, &stateOut), 1,
             "F0190 killed-some fear apply plan builds");
    CHECK_EQ(stateOut.shouldApplyFear, 1, "F0190 fear applies");
    CHECK_EQ(stateOut.newGroupBehavior, DM1_BEHAVIOR_FLEE,
             "F0190 fear behavior");
    CHECK_EQ(stateOut.newAiStateKind, AI_STATE_FLEE,
             "F0190 fear ai state");
    CHECK_EQ(stateOut.fearCounter, 44, "F0190 fear delay");
    CHECK_EQ(dm1_v1_melee_killed_some_fear_apply_from_state_plan_f0190_pc34(
                 &stateOut, 1, 55, &stateApplyOut), 1,
             "F0190 fear apply from state plan builds");
    CHECK_EQ(stateApplyOut.shouldApplyFear, 1,
             "F0190 fear from state applies");
    CHECK_EQ(stateApplyOut.groupIndex, stateOut.groupIndex,
             "F0190 fear from state keeps group");
    CHECK_EQ(stateApplyOut.killedCreatureIndex, stateOut.killedCreatureIndex,
             "F0190 fear from state keeps killed creature");
    CHECK_EQ(stateApplyOut.fearCounter, 55,
             "F0190 fear from state uses new delay");
    CHECK_EQ(dm1_v1_melee_killed_some_state_apply_plan_f0190_pc34(
                 &stateApplyOut, &killedSomeApplyOut), 1,
             "F0190 killed-some apply receipt builds");
    CHECK_EQ(killedSomeApplyOut.valid, 1,
             "F0190 killed-some apply receipt valid");
    CHECK_EQ(killedSomeApplyOut.shouldCleanupCreatureEvents, 1,
             "F0190 killed-some apply cleans events");
    CHECK_EQ(killedSomeApplyOut.shouldApplyFear, 1,
             "F0190 killed-some apply fear write");
    CHECK_EQ(killedSomeApplyOut.groupIndex, 3,
             "F0190 killed-some apply group");
    CHECK_EQ(killedSomeApplyOut.killedCreatureIndex, 2,
             "F0190 killed-some apply killed index");
    CHECK_EQ(killedSomeApplyOut.newGroupBehavior, DM1_BEHAVIOR_FLEE,
             "F0190 killed-some apply behavior");
    CHECK_EQ(killedSomeApplyOut.fearCounter, 55,
             "F0190 killed-some apply fear delay");

    stateIn.partyMapIndex = 2;
    CHECK_EQ(dm1_v1_melee_killed_some_state_plan_f0190_pc34(
                 &stateIn, &stateOut), 1,
             "F0190 off-party-map state plan builds");
    CHECK_EQ(stateOut.shouldCleanupCreatureEvents, 1,
             "F0190 off-party-map still cleans events");
    CHECK_EQ(stateOut.shouldEvaluateFear, 0,
             "F0190 off-party-map skips fear");

    stateIn.partyMapIndex = 1;
    stateIn.groupBehavior = DM1_BEHAVIOR_APPROACH;
    CHECK_EQ(dm1_v1_melee_killed_some_state_plan_f0190_pc34(
                 &stateIn, &stateOut), 1,
             "F0190 non-attack state plan builds");
    CHECK_EQ(stateOut.shouldCleanupCreatureEvents, 0,
             "F0190 non-attack skips cleanup");
    CHECK_EQ(stateOut.shouldEvaluateFear, 0,
             "F0190 non-attack skips fear");

    memset(&cleanupIn, 0, sizeof(cleanupIn));
    cleanupIn.eventKind = TIMELINE_EVENT_CREATURE_REACTION;
    cleanupIn.eventMapIndex = 1;
    cleanupIn.eventMapX = 10;
    cleanupIn.eventMapY = 11;
    cleanupIn.targetMapIndex = 1;
    cleanupIn.targetMapX = 10;
    cleanupIn.targetMapY = 11;
    cleanupIn.killedCreatureIndex = 1;
    cleanupIn.eventType = DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1;
    CHECK_EQ(dm1_v1_melee_timeline_cleanup_plan_f0190_pc34(
                 &cleanupIn, &cleanupOut), 1,
             "F0190 cleanup killed aspect event builds");
    CHECK_EQ(cleanupOut.valid, 1, "F0190 cleanup killed aspect valid");
    CHECK_EQ(cleanupOut.shouldKeepEvent, 0,
             "F0190 cleanup deletes killed aspect event");
    CHECK_EQ(cleanupOut.eventCreatureIndex, 1,
             "F0190 cleanup aspect creature index");

    cleanupIn.eventType = DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 2;
    CHECK_EQ(dm1_v1_melee_timeline_cleanup_plan_f0190_pc34(
                 &cleanupIn, &cleanupOut), 1,
             "F0190 cleanup shifted aspect event builds");
    CHECK_EQ(cleanupOut.shouldKeepEvent, 1,
             "F0190 cleanup keeps later aspect event");
    CHECK_EQ(cleanupOut.newEventType, DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1,
             "F0190 cleanup shifts later aspect event");

    cleanupIn.eventType = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 1;
    CHECK_EQ(dm1_v1_melee_timeline_cleanup_plan_f0190_pc34(
                 &cleanupIn, &cleanupOut), 1,
             "F0190 cleanup killed behavior event builds");
    CHECK_EQ(cleanupOut.shouldKeepEvent, 0,
             "F0190 cleanup deletes killed behavior event");

    cleanupIn.eventType = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 3;
    CHECK_EQ(dm1_v1_melee_timeline_cleanup_plan_f0190_pc34(
                 &cleanupIn, &cleanupOut), 1,
             "F0190 cleanup shifted behavior event builds");
    CHECK_EQ(cleanupOut.shouldKeepEvent, 1,
             "F0190 cleanup keeps later behavior event");
    CHECK_EQ(cleanupOut.newEventType, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 2,
             "F0190 cleanup shifts later behavior event");

    cleanupIn.eventMapX = 99;
    CHECK_EQ(dm1_v1_melee_timeline_cleanup_plan_f0190_pc34(
                 &cleanupIn, &cleanupOut), 1,
             "F0190 cleanup other square builds");
    CHECK_EQ(cleanupOut.shouldKeepEvent, 1,
             "F0190 cleanup keeps other square");
    CHECK_EQ(cleanupOut.newEventType, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 3,
             "F0190 cleanup keeps other square type");

    cleanupIn.eventMapX = 10;
    cleanupIn.eventKind = 0;
    cleanupIn.eventType = DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1;
    CHECK_EQ(dm1_v1_melee_timeline_cleanup_plan_f0190_pc34(
                 &cleanupIn, &cleanupOut), 1,
             "F0190 cleanup other kind builds");
    CHECK_EQ(cleanupOut.shouldKeepEvent, 1,
             "F0190 cleanup keeps other kind");

    memset(&cleanupBatchIn, 0, sizeof(cleanupBatchIn));
    cleanupBatchIn.eventCount = 4;
    cleanupBatchIn.targetMapIndex = 1;
    cleanupBatchIn.targetMapX = 10;
    cleanupBatchIn.targetMapY = 11;
    cleanupBatchIn.killedCreatureIndex = 1;
    cleanupBatchIn.events[0].kind = TIMELINE_EVENT_CREATURE_REACTION;
    cleanupBatchIn.events[0].mapIndex = 1;
    cleanupBatchIn.events[0].mapX = 10;
    cleanupBatchIn.events[0].mapY = 11;
    cleanupBatchIn.events[0].aux2 = DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 0;
    cleanupBatchIn.events[1].kind = TIMELINE_EVENT_CREATURE_REACTION;
    cleanupBatchIn.events[1].mapIndex = 1;
    cleanupBatchIn.events[1].mapX = 10;
    cleanupBatchIn.events[1].mapY = 11;
    cleanupBatchIn.events[1].aux2 = DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1;
    cleanupBatchIn.events[2].kind = TIMELINE_EVENT_CREATURE_REACTION;
    cleanupBatchIn.events[2].mapIndex = 1;
    cleanupBatchIn.events[2].mapX = 10;
    cleanupBatchIn.events[2].mapY = 11;
    cleanupBatchIn.events[2].aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 3;
    cleanupBatchIn.events[3].kind = TIMELINE_EVENT_CREATURE_TICK;
    cleanupBatchIn.events[3].mapIndex = 1;
    cleanupBatchIn.events[3].mapX = 10;
    cleanupBatchIn.events[3].mapY = 11;
    cleanupBatchIn.events[3].aux2 = DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1;
    CHECK_EQ(dm1_v1_melee_timeline_cleanup_batch_plan_f0190_pc34(
                 &cleanupBatchIn, &cleanupBatchOut), 1,
             "F0190 cleanup batch builds");
    CHECK_EQ(cleanupBatchOut.valid, 1, "F0190 cleanup batch valid");
    CHECK_EQ(cleanupBatchOut.oldEventCount, 4,
             "F0190 cleanup batch old count");
    CHECK_EQ(cleanupBatchOut.newEventCount, 3,
             "F0190 cleanup batch compacts count");
    CHECK_EQ(cleanupBatchOut.deletedEventCount, 1,
             "F0190 cleanup batch deleted killed event");
    CHECK_EQ(cleanupBatchOut.events[0].aux2,
             DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 0,
             "F0190 cleanup batch keeps earlier event");
    CHECK_EQ(cleanupBatchOut.events[1].aux2,
             DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 2,
             "F0190 cleanup batch shifts later behavior event");
    CHECK_EQ(cleanupBatchOut.events[2].kind, TIMELINE_EVENT_CREATURE_TICK,
             "F0190 cleanup batch keeps other event kind");
    CHECK_EQ(cleanupBatchOut.events[2].aux2,
             DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1,
             "F0190 cleanup batch leaves other event kind type");

    memset(&killedAllIn, 0, sizeof(killedAllIn));
    killedAllIn.outcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    killedAllIn.groupIndex = 9;
    killedAllIn.targetMapIndex = 2;
    killedAllIn.targetMapX = 13;
    killedAllIn.targetMapY = 14;
    CHECK_EQ(dm1_v1_melee_killed_all_state_plan_f0190_pc34(
                 &killedAllIn, &killedAllOut), 1,
             "F0190 killed-all state plan builds");
    CHECK_EQ(killedAllOut.valid, 1, "F0190 killed-all state valid");
    CHECK_EQ(killedAllOut.shouldUnlinkGroupFromSquare, 1,
             "F0190 killed-all unlinks group");
    CHECK_EQ(killedAllOut.shouldClearGroupNext, 1,
             "F0190 killed-all clears next");
    CHECK_EQ(killedAllOut.shouldRemoveActiveGroupState, 1,
             "F0190 killed-all removes active state");
    CHECK_EQ(killedAllOut.shouldWriteRawGroup, 1,
             "F0190 killed-all writes raw group");
    CHECK_EQ(killedAllOut.groupIndex, 9, "F0190 killed-all group");
    CHECK_EQ(killedAllOut.mapIndex, 2, "F0190 killed-all map");
    CHECK_EQ(killedAllOut.mapX, 13, "F0190 killed-all x");
    CHECK_EQ(killedAllOut.mapY, 14, "F0190 killed-all y");
    CHECK_EQ(dm1_v1_melee_killed_all_state_apply_plan_f0190_pc34(
                 &killedAllOut, &killedAllApplyOut), 1,
             "F0190 killed-all apply receipt builds");
    CHECK_EQ(killedAllApplyOut.valid, 1,
             "F0190 killed-all apply receipt valid");
    CHECK_EQ(killedAllApplyOut.shouldUnlinkGroupFromSquare, 1,
             "F0190 killed-all apply unlinks group");
    CHECK_EQ(killedAllApplyOut.shouldClearGroupNext, 1,
             "F0190 killed-all apply clears next");
    CHECK_EQ(killedAllApplyOut.shouldRemoveActiveGroupState, 1,
             "F0190 killed-all apply removes active state");
    CHECK_EQ(killedAllApplyOut.groupThing,
             (unsigned short)(((unsigned short)THING_TYPE_GROUP << 10) | 9u),
             "F0190 killed-all apply group thing");
    CHECK_EQ(killedAllApplyOut.clearedNextThing, THING_NONE,
             "F0190 killed-all apply next none");

    killedAllIn.outcome = COMBAT_OUTCOME_KILLED_SOME_CREATURES;
    CHECK_EQ(dm1_v1_melee_killed_all_state_plan_f0190_pc34(
                 &killedAllIn, &killedAllOut), 1,
             "F0190 killed-some killed-all state plan builds");
    CHECK_EQ(killedAllOut.shouldUnlinkGroupFromSquare, 0,
             "F0190 killed-some no unlink");

    memset(&dispatchIn, 0, sizeof(dispatchIn));
    dispatchIn.outcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    dispatchIn.groupIndex = 9;
    dispatchIn.groupBehavior = DM1_BEHAVIOR_ATTACK;
    dispatchIn.killedCreatureIndex = 1;
    dispatchIn.originalGroupCount = 2;
    dispatchIn.creatureType = 12;
    dispatchIn.creatureAttributes = DM1_ATTR_DROP_FIXED_POSS;
    dispatchIn.creatureProperties = 0x0230;
    dispatchIn.killedCell = EXPLOSION_CELL_CENTERED;
    dispatchIn.mapIndex = 2;
    dispatchIn.mapX = 13;
    dispatchIn.mapY = 14;
    dispatchIn.partyMapIndex = 2;
    dispatchIn.partyMapX = 12;
    dispatchIn.partyMapY = 14;
    CHECK_EQ(dm1_v1_melee_mutation_dispatch_plan_f0190_pc34(
                 &dispatchIn, &dispatchOut), 1,
             "F0190 mutation dispatch killed-all builds");
    CHECK_EQ(dispatchOut.valid, 1, "F0190 mutation dispatch valid");
    CHECK_EQ(dispatchOut.shouldDropPossessions, 1,
             "F0190 mutation dispatch killed-all drops");
    CHECK_EQ(dispatchOut.possessionDropPlan.shouldDropGroupFixedPossessions, 1,
             "F0190 mutation dispatch group fixed drop");
    CHECK_EQ(dispatchOut.possessionDropPlan.shouldDropGroupSlotPossessions, 1,
             "F0190 mutation dispatch group slot drop");
    CHECK_EQ(dispatchOut.shouldApplyKilledAllSideEffects, 1,
             "F0190 mutation dispatch killed-all side effects");
    CHECK_EQ(dispatchOut.killedAllStatePlan.groupIndex, 9,
             "F0190 mutation dispatch killed-all group");
    CHECK_EQ(dispatchOut.shouldApplyKilledSomeState, 0,
             "F0190 mutation dispatch killed-all no killed-some cleanup");
    CHECK_EQ(dm1_v1_melee_mutation_dispatch_apply_plan_f0190_pc34(
                 &dispatchOut, &dispatchApplyOut), 1,
             "F0190 mutation dispatch killed-all apply receipt builds");
    CHECK_EQ(dispatchApplyOut.valid, 1,
             "F0190 mutation dispatch killed-all apply valid");
    CHECK_EQ(dispatchApplyOut.shouldDropPossessions, 1,
             "F0190 mutation dispatch killed-all apply drops");
    CHECK_EQ(dispatchApplyOut.shouldApplyKilledAllSideEffects, 1,
             "F0190 mutation dispatch killed-all apply side effects");
    CHECK_EQ(dispatchApplyOut.killedAllStatePlan.groupIndex, 9,
             "F0190 mutation dispatch killed-all apply group");

    dispatchIn.outcome = COMBAT_OUTCOME_KILLED_SOME_CREATURES;
    dispatchIn.killedCell = 6;
    CHECK_EQ(dm1_v1_melee_mutation_dispatch_plan_f0190_pc34(
                 &dispatchIn, &dispatchOut), 1,
             "F0190 mutation dispatch killed-some builds");
    CHECK_EQ(dispatchOut.shouldDropPossessions, 1,
             "F0190 mutation dispatch killed-some fixed drops");
    CHECK_EQ(dispatchOut.possessionDropPlan.shouldDropCreatureFixedPossessions,
             1, "F0190 mutation dispatch killed-some creature drop");
    CHECK_EQ(dispatchOut.possessionDropPlan.creatureCell, 2,
             "F0190 mutation dispatch killed-some cell mask");
    CHECK_EQ(dispatchOut.shouldApplyKilledSomeState, 1,
             "F0190 mutation dispatch killed-some cleanup/fear");
    CHECK_EQ(dispatchOut.killedSomeStatePlan.shouldCleanupCreatureEvents, 1,
             "F0190 mutation dispatch cleanup events");
    CHECK_EQ(dispatchOut.killedSomeStatePlan.shouldEvaluateFear, 1,
             "F0190 mutation dispatch evaluates fear on party map");
    CHECK_EQ(dispatchOut.shouldApplyKilledAllSideEffects, 0,
             "F0190 mutation dispatch killed-some no killed-all side effects");
    CHECK_EQ(dm1_v1_melee_mutation_dispatch_apply_plan_f0190_pc34(
                 &dispatchOut, &dispatchApplyOut), 1,
             "F0190 mutation dispatch killed-some apply receipt builds");
    CHECK_EQ(dispatchApplyOut.shouldApplyKilledSomeState, 1,
             "F0190 mutation dispatch killed-some apply state");
    CHECK_EQ(dispatchApplyOut.shouldEvaluateFear, 1,
             "F0190 mutation dispatch killed-some apply fear gate");
    CHECK_EQ(dispatchApplyOut.killedSomeStatePlan.killedCreatureIndex, 1,
             "F0190 mutation dispatch killed-some apply killed index");
    CHECK_EQ(dm1_v1_melee_mutation_dispatch_fear_roll_plan_f0190_pc34(
                 &dispatchOut, &fearRollOut), 1,
             "F0190 mutation dispatch fear roll plan builds");
    CHECK_EQ(fearRollOut.valid, 1,
             "F0190 mutation dispatch fear roll valid");
    CHECK_EQ(fearRollOut.shouldEvaluateFear, 1,
             "F0190 mutation dispatch fear roll enabled");
    CHECK_EQ(fearRollOut.originalGroupCount, 2,
             "F0190 mutation dispatch fear roll original count");
    CHECK_EQ(fearRollOut.fearContext.creatureInfo.properties, 0x0230,
             "F0190 mutation dispatch fear roll properties");
    CHECK_EQ(dm1_v1_melee_mutation_dispatch_fear_apply_plan_f0190_pc34(
                 &dispatchOut, 1, 61, &fearApplyOut), 1,
             "F0190 mutation dispatch fear apply plan builds");
    CHECK_EQ(fearApplyOut.valid, 1,
             "F0190 mutation dispatch fear apply valid");
    CHECK_EQ(fearApplyOut.shouldApplyFear, 1,
             "F0190 mutation dispatch fear applies");
    CHECK_EQ(fearApplyOut.newGroupBehavior, DM1_BEHAVIOR_FLEE,
             "F0190 mutation dispatch fear behavior");
    CHECK_EQ(fearApplyOut.fearCounter, 61,
             "F0190 mutation dispatch fear delay");

    memset(&aftermathIn, 0, sizeof(aftermathIn));
    aftermathIn.groupIndex = 9;
    aftermathIn.creatureIndex = 1;
    aftermathIn.creatureType = 12;
    aftermathIn.creatureAttributes = DM1_ATTR_DROP_FIXED_POSS;
    aftermathIn.creatureProperties = 0x0230;
    aftermathIn.groupBehavior = DM1_BEHAVIOR_ATTACK;
    aftermathIn.originalGroupCount = 2;
    aftermathIn.partyMapIndex = 2;
    aftermathIn.partyMapX = 12;
    aftermathIn.partyMapY = 14;
    aftermathIn.targetMapIndex = 2;
    aftermathIn.targetMapX = 13;
    aftermathIn.targetMapY = 14;
    aftermathIn.killedCell = 6;
    aftermathIn.damageOutcome = COMBAT_OUTCOME_KILLED_SOME_CREATURES;
    aftermathIn.fallbackCombatOutcome = COMBAT_OUTCOME_KILLED_SOME_CREATURES;
    CHECK_EQ(dm1_v1_melee_aftermath_plan_f0231_pc34(
                 &aftermathIn, &aftermathOut), 1,
             "F0231 aftermath for F0190 dispatch builds");
    CHECK_EQ(dm1_v1_melee_aftermath_mutation_dispatch_plan_f0231_pc34(
                 &aftermathOut, &aftermathDispatchOut), 1,
             "F0231 aftermath maps to F0190 dispatch");
    CHECK_EQ(aftermathDispatchOut.shouldDropPossessions, 1,
             "F0231 aftermath dispatch keeps possession drops");
    CHECK_EQ(aftermathDispatchOut.possessionDropPlan.creatureCell, 2,
             "F0231 aftermath dispatch keeps killed cell mask");
    CHECK_EQ(aftermathDispatchOut.shouldApplyKilledSomeState, 1,
             "F0231 aftermath dispatch keeps killed-some state");
    CHECK_EQ(aftermathDispatchOut.killedSomeStatePlan.shouldEvaluateFear, 1,
             "F0231 aftermath dispatch keeps fear evaluation");
    CHECK_EQ(aftermathDispatchOut.killedSomeStatePlan.originalGroupCount, 2,
             "F0231 aftermath dispatch keeps original count");

    aftermathIn.damageOutcome = COMBAT_OUTCOME_KILLED_NO_CREATURES;
    aftermathIn.fallbackCombatOutcome = COMBAT_OUTCOME_HIT_DAMAGE;
    CHECK_EQ(dm1_v1_melee_aftermath_plan_f0231_pc34(
                 &aftermathIn, &aftermathOut), 1,
             "F0231 no-kill aftermath for F0190 dispatch builds");
    CHECK_EQ(dm1_v1_melee_aftermath_mutation_dispatch_plan_f0231_pc34(
                 &aftermathOut, &aftermathDispatchOut), 1,
             "F0231 no-kill aftermath maps to empty F0190 dispatch");
    CHECK_EQ(aftermathDispatchOut.valid, 1,
             "F0231 no-kill dispatch receipt valid");
    CHECK_EQ(aftermathDispatchOut.shouldDropPossessions, 0,
             "F0231 no-kill dispatch drops nothing");
    CHECK_EQ(aftermathDispatchOut.shouldApplyKilledSomeState, 0,
             "F0231 no-kill dispatch skips killed-some state");
    CHECK_EQ(aftermathDispatchOut.shouldApplyKilledAllSideEffects, 0,
             "F0231 no-kill dispatch skips killed-all state");

    memset(&resultA, 0, sizeof(resultA));
    resultA.damageApplied = 30;
    resultB = resultA;
    memset(&groupA, 0, sizeof(groupA));
    groupA.count = 1;
    groupA.cells = 0x09;
    groupA.health[0] = 100;
    groupB = groupA;
    CHECK_EQ(dm1_v1_melee_apply_group_damage_plan_f0190_pc34(
                 &resultA, &groupA, 0, &applyPlan), 1,
             "DM1 F0190 group damage receipt builds");
    CHECK_EQ(applyPlan.valid, 1, "DM1 F0190 group damage receipt valid");
    CHECK_EQ(applyPlan.shouldApplyDamage, 1,
             "DM1 F0190 group damage receipt applies");
    CHECK_EQ(applyPlan.originalGroupCount, 1,
             "DM1 F0190 group damage original count");
    CHECK_EQ(applyPlan.killedCell, 1,
             "DM1 F0190 group damage killed cell");
    CHECK_EQ(applyPlan.damageApplied, 30,
             "DM1 F0190 group damage amount");
    outcomeA = applyPlan.outcome;
    CHECK_EQ(F0738_COMBAT_ApplyDamageToGroup_Compat(
                 &resultB, &groupB, 0, &outcomeB), 1,
             "shared F0738 group damage builds");
    CHECK_EQ(groupA.health[0], groupB.health[0],
             "DM1 F0190 group damage mirrors health");
    CHECK_EQ(groupA.count, groupB.count,
             "DM1 F0190 group damage mirrors count");
    CHECK_EQ(outcomeA, outcomeB,
             "DM1 F0190 group damage mirrors outcome");

    memset(&resultA, 0, sizeof(resultA));
    resultA.outcome = COMBAT_OUTCOME_MISS;
    groupA = groupB;
    CHECK_EQ(dm1_v1_melee_apply_group_damage_plan_f0190_pc34(
                 &resultA, &groupA, 0, &applyPlan), 1,
             "DM1 F0190 zero-damage receipt builds");
    CHECK_EQ(applyPlan.shouldApplyDamage, 0,
             "DM1 F0190 zero-damage receipt skips apply");
    CHECK_EQ(applyPlan.outcome, COMBAT_OUTCOME_MISS,
             "DM1 F0190 zero-damage keeps fallback outcome");
}

static void test_melee_f0231_runtime_result_plan(void) {
    DM1_MeleeF0231RuntimeResultInputPc34 in;
    DM1_MeleeF0231RuntimeResultPlanPc34 out;
    DM1_MeleeF0231RuntimeApplyPlanPc34 applyOut;
    DM1_MeleeF0231LuckWritebackInputPc34 luckIn;
    DM1_MeleeF0231LuckWritebackPlanPc34 luckOut;

    memset(&in, 0, sizeof(in));
    in.combatOutcome = COMBAT_OUTCOME_NO_ACTION;
    in.championIndex = 1;
    in.damageApplied = 5;
    in.groupIndex = 0;
    in.groupCount = 1;
    CHECK_EQ(dm1_v1_melee_runtime_result_plan_f0231_pc34(&in, &out), 1,
             "F0231 no-action runtime result builds");
    CHECK_EQ(out.shouldReturnHandledNoAction, 1,
             "F0231 no-action returns handled");
    CHECK_EQ(out.shouldApplySideEffects, 0,
             "F0231 no-action skips side effects");

    memset(&in, 0, sizeof(in));
    in.championIndex = 1;
    in.combatOutcome = COMBAT_OUTCOME_HIT_DAMAGE;
    in.damageApplied = 12;
    in.groupIndex = 0;
    in.creatureIndex = 2;
    in.groupCount = 1;
    CHECK_EQ(dm1_v1_melee_runtime_result_plan_f0231_pc34(&in, &out), 1,
             "F0231 damage runtime result builds");
    CHECK_EQ(out.shouldWriteBackLuck, 1, "F0231 damage writes luck");
    CHECK_EQ(out.shouldApplySideEffects, 1,
             "F0231 damage applies side effects");
    CHECK_EQ(out.shouldApplyGroupDamage, 1,
             "F0231 damage applies group damage");
    CHECK_EQ(out.groupDamageGroupIndex, 0,
             "F0231 damage carries group damage group");
    CHECK_EQ(out.groupDamageCreatureIndex, 2,
             "F0231 damage carries group damage creature");
    CHECK_EQ(out.groupDamageApplied, 12,
             "F0231 damage carries group damage amount");
    CHECK_EQ(out.groupDamageFallbackOutcome, COMBAT_OUTCOME_HIT_DAMAGE,
             "F0231 damage carries group damage fallback outcome");
    CHECK_EQ(out.shouldEmitDamageDealt, 1,
             "F0231 damage emits damage result");
    CHECK_EQ(out.emitChampionIndex, 1,
             "F0231 damage emit carries champion");
    CHECK_EQ(out.emitGroupIndex, 0,
             "F0231 damage emit carries group");
    CHECK_EQ(out.emitDamageApplied, 12,
             "F0231 damage emit carries damage");
    CHECK_EQ(dm1_v1_melee_runtime_apply_plan_f0231_pc34(&out, &applyOut), 1,
             "F0231 runtime apply receipt builds");
    CHECK_EQ(applyOut.valid, 1, "F0231 runtime apply valid");
    CHECK_EQ(applyOut.shouldWriteBackLuck, 1,
             "F0231 runtime apply writes luck");
    CHECK_EQ(applyOut.shouldApplySideEffects, 1,
             "F0231 runtime apply side effects");
    CHECK_EQ(applyOut.shouldApplyGroupDamage, 1,
             "F0231 runtime apply group damage");
    CHECK_EQ(applyOut.groupDamageApplied, 12,
             "F0231 runtime apply damage amount");
    CHECK_EQ(applyOut.shouldEmitDamageDealt, 1,
             "F0231 runtime apply emits damage");

    in.damageApplied = 0;
    CHECK_EQ(dm1_v1_melee_runtime_result_plan_f0231_pc34(&in, &out), 1,
             "F0231 zero-damage runtime result builds");
    CHECK_EQ(out.shouldApplyGroupDamage, 0,
             "F0231 zero damage skips group damage");
    CHECK_EQ(out.shouldApplySideEffects, 1,
             "F0231 zero damage still applies stamina side effects");
    CHECK_EQ(out.shouldEmitDamageDealt, 1,
             "F0231 zero damage emits handled damage result");

    in.damageApplied = 12;
    in.groupIndex = 3;
    in.creatureIndex = 0;
    in.groupCount = 1;
    CHECK_EQ(dm1_v1_melee_runtime_result_plan_f0231_pc34(&in, &out), 1,
             "F0231 out-of-range runtime result builds");
    CHECK_EQ(out.shouldApplyGroupDamage, 0,
             "F0231 out-of-range skips group damage");

    memset(&luckIn, 0, sizeof(luckIn));
    luckIn.championIndex = 1;
    luckIn.championCount = 4;
    luckIn.snapshotLuck = 280;
    CHECK_EQ(dm1_v1_melee_luck_writeback_plan_f0231_pc34(
                 &luckIn, &luckOut), 1,
             "F0231 luck writeback high clamp builds");
    CHECK_EQ(luckOut.valid, 1, "F0231 luck writeback valid");
    CHECK_EQ(luckOut.shouldWriteBack, 1,
             "F0231 luck writeback applies valid champion");
    CHECK_EQ(luckOut.championIndex, 1,
             "F0231 luck writeback champion");
    CHECK_EQ(luckOut.clampedLuck, 255,
             "F0231 luck writeback clamps high");

    luckIn.snapshotLuck = -9;
    CHECK_EQ(dm1_v1_melee_luck_writeback_plan_f0231_pc34(
                 &luckIn, &luckOut), 1,
             "F0231 luck writeback low clamp builds");
    CHECK_EQ(luckOut.clampedLuck, 0,
             "F0231 luck writeback clamps low");

    luckIn.championIndex = 4;
    luckIn.snapshotLuck = 77;
    CHECK_EQ(dm1_v1_melee_luck_writeback_plan_f0231_pc34(
                 &luckIn, &luckOut), 1,
             "F0231 luck writeback out-of-range builds");
    CHECK_EQ(luckOut.shouldWriteBack, 0,
             "F0231 luck writeback skips invalid champion");
}

static void test_melee_f0231_damage_resolver_entrypoint(void) {
    struct CombatantChampionSnapshot_Compat attackerA;
    struct CombatantChampionSnapshot_Compat attackerB;
    struct CombatantCreatureSnapshot_Compat defender;
    struct WeaponProfile_Compat weapon;
    struct CombatResult_Compat outA;
    struct CombatResult_Compat outB;
    struct RngState_Compat rngA;
    struct RngState_Compat rngB;

    memset(&attackerA, 0, sizeof(attackerA));
    memset(&attackerB, 0, sizeof(attackerB));
    memset(&defender, 0, sizeof(defender));
    memset(&weapon, 0, sizeof(weapon));
    memset(&outA, 0, sizeof(outA));
    memset(&outB, 0, sizeof(outB));

    attackerA.championIndex = 0;
    attackerA.currentHealth = 100;
    attackerA.dexterity = 255;
    attackerA.strengthActionHand = 120;
    attackerA.skillLevelAction = 0;
    attackerA.statisticLuck = 40;
    attackerA.statisticLuckMax = 100;
    attackerA.statisticLuckMin = 0;
    attackerB = attackerA;

    defender.creatureType = CREATURE_TYPE_GIANT_SCORPION;
    defender.defense = 0;
    defender.dexterity = 0;
    defender.attributes = 0;
    defender.doubledMapDifficulty = 0;
    defender.healthBefore = 200;

    weapon.hitProbability = 0;
    weapon.damageFactor = 32;

    CHECK_EQ(F0730_COMBAT_RngInit_Compat(&rngA, 0x630u), 1,
             "DM1 F0231 resolver rng A init");
    CHECK_EQ(F0730_COMBAT_RngInit_Compat(&rngB, 0x630u), 1,
             "DM1 F0231 resolver rng B init");
    CHECK_EQ(dm1_v1_melee_resolve_damage_f0231_pc34(
                 &attackerA, &weapon, &defender, &rngA, &outA), 1,
             "DM1 F0231 damage resolver entrypoint builds");
    CHECK_EQ(F0735_COMBAT_ResolveChampionMelee_Compat(
                 &attackerB, &weapon, &defender, &rngB, &outB), 1,
             "shared F0735 resolver builds");
    CHECK_EQ(outA.outcome, outB.outcome, "DM1 F0231 outcome mirrors F0735");
    CHECK_EQ(outA.damageApplied, outB.damageApplied,
             "DM1 F0231 damage mirrors F0735");
    CHECK_EQ(outA.hitLanded, outB.hitLanded,
             "DM1 F0231 hit flag mirrors F0735");
    CHECK_EQ(outA.rngCallCount, outB.rngCallCount,
             "DM1 F0231 RNG count mirrors F0735");
    CHECK_EQ(attackerA.statisticLuck, attackerB.statisticLuck,
             "DM1 F0231 luck mutation mirrors F0735");

    {
        DM1_MeleeF0231ResolveRuntimeInputPc34 runtimeIn;
        DM1_MeleeF0231ResolveRuntimePlanPc34 runtimeOut;
        DM1_MeleeF0231RuntimeResultInputPc34 separateRuntimeIn;
        DM1_MeleeF0231RuntimeResultPlanPc34 separateRuntimeOut;
        struct CombatantChampionSnapshot_Compat attackerC = attackerB;
        struct CombatResult_Compat separateResult;
        struct RngState_Compat rngC;
        struct RngState_Compat rngD;

        memset(&runtimeIn, 0, sizeof(runtimeIn));
        memset(&runtimeOut, 0, sizeof(runtimeOut));
        memset(&separateRuntimeIn, 0, sizeof(separateRuntimeIn));
        memset(&separateRuntimeOut, 0, sizeof(separateRuntimeOut));
        memset(&separateResult, 0, sizeof(separateResult));
        runtimeIn.championIndex = 1;
        runtimeIn.groupIndex = 0;
        runtimeIn.creatureIndex = 0;
        runtimeIn.groupCount = 1;
        CHECK_EQ(F0730_COMBAT_RngInit_Compat(&rngC, 0x630u), 1,
                 "DM1 F0231 runtime entry rng C init");
        CHECK_EQ(F0730_COMBAT_RngInit_Compat(&rngD, 0x630u), 1,
                 "DM1 F0231 runtime entry rng D init");
        CHECK_EQ(dm1_v1_melee_resolve_runtime_f0231_pc34(
                     &attackerB, &weapon, &defender, &rngC,
                     &runtimeIn, &runtimeOut), 1,
                 "DM1 F0231 runtime entrypoint builds");
        CHECK_EQ(dm1_v1_melee_resolve_damage_f0231_pc34(
                     &attackerC, &weapon, &defender, &rngD,
                     &separateResult), 1,
                 "DM1 F0231 separate damage builds");
        separateRuntimeIn.combatOutcome = separateResult.outcome;
        separateRuntimeIn.championIndex = runtimeIn.championIndex;
        separateRuntimeIn.damageApplied = separateResult.damageApplied;
        separateRuntimeIn.groupIndex = runtimeIn.groupIndex;
        separateRuntimeIn.creatureIndex = runtimeIn.creatureIndex;
        separateRuntimeIn.groupCount = runtimeIn.groupCount;
        CHECK_EQ(dm1_v1_melee_runtime_result_plan_f0231_pc34(
                     &separateRuntimeIn, &separateRuntimeOut), 1,
                 "DM1 F0231 separate runtime plan builds");
        CHECK_EQ(runtimeOut.valid, 1,
                 "DM1 F0231 runtime entrypoint valid");
        CHECK_EQ(runtimeOut.combatResult.outcome, separateResult.outcome,
                 "DM1 F0231 runtime entry outcome mirrors separate path");
        CHECK_EQ(runtimeOut.combatResult.damageApplied,
                 separateResult.damageApplied,
                 "DM1 F0231 runtime entry damage mirrors separate path");
        CHECK_EQ(runtimeOut.runtimeResultPlan.shouldApplyGroupDamage,
                 separateRuntimeOut.shouldApplyGroupDamage,
                 "DM1 F0231 runtime entry group gate mirrors separate path");
        CHECK_EQ(runtimeOut.runtimeResultPlan.shouldEmitDamageDealt,
                 separateRuntimeOut.shouldEmitDamageDealt,
                 "DM1 F0231 runtime entry emit gate mirrors separate path");
        CHECK_EQ(attackerB.statisticLuck, attackerC.statisticLuck,
                 "DM1 F0231 runtime entry luck mirrors separate path");
    }

    for (unsigned int seed = 1; seed < 96; ++seed) {
        memset(&attackerA, 0, sizeof(attackerA));
        memset(&defender, 0, sizeof(defender));
        memset(&weapon, 0, sizeof(weapon));
        memset(&outA, 0, sizeof(outA));
        memset(&outB, 0, sizeof(outB));
        attackerA.championIndex = 0;
        attackerA.currentHealth = 100;
        attackerA.dexterity = (seed & 1u) ? 0 : 255;
        attackerA.strengthActionHand = (seed & 2u) ? 1 : 100;
        attackerA.skillLevelAction = (int)(seed & 15u);
        attackerA.statisticLuck = (seed & 4u) ? 0 : 80;
        attackerA.statisticLuckMax = 100;
        attackerA.statisticLuckMin = 0;
        attackerA.actionHandIcon =
            (seed & 8u) ? COMBAT_ICON_VORPAL_BLADE : 0;
        attackerB = attackerA;

        defender.creatureType = CREATURE_TYPE_GIANT_SCORPION;
        defender.defense = (seed & 2u) ? 200 : 0;
        defender.dexterity = (seed & 1u) ? 80 : 0;
        defender.attributes = (seed & 16u) ? 0x0040 : 0;
        defender.doubledMapDifficulty = (seed & 1u) ? 30 : 0;
        defender.healthBefore = 200;

        weapon.hitProbability =
            (seed & 16u) ? (0x8000 | 50) : (int)(seed & 63u);
        weapon.damageFactor = (seed & 2u) ? 1 : 32;
        CHECK_EQ(F0730_COMBAT_RngInit_Compat(&rngA, seed), 1,
                 "DM1 F0231 mirror rng A init");
        CHECK_EQ(F0730_COMBAT_RngInit_Compat(&rngB, seed), 1,
                 "DM1 F0231 mirror rng B init");
        CHECK_EQ(dm1_v1_melee_resolve_damage_f0231_pc34(
                     &attackerA, &weapon, &defender, &rngA, &outA), 1,
                 "DM1 F0231 mirror resolver builds");
        CHECK_EQ(F0735_COMBAT_ResolveChampionMelee_Compat(
                     &attackerB, &weapon, &defender, &rngB, &outB), 1,
                 "shared F0735 mirror resolver builds");
        CHECK_EQ(outA.outcome, outB.outcome,
                 "DM1 F0231 mirror outcome");
        CHECK_EQ(outA.damageApplied, outB.damageApplied,
                 "DM1 F0231 mirror damage");
        CHECK_EQ(outA.rngCallCount, outB.rngCallCount,
                 "DM1 F0231 mirror rng count");
        CHECK_EQ(outA.luckyHit, outB.luckyHit,
                 "DM1 F0231 mirror lucky hit");
        CHECK_EQ(attackerA.statisticLuck, attackerB.statisticLuck,
                 "DM1 F0231 mirror luck mutation");
    }
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
    test_xp_award_and_direct_dispatch_plans();
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
    test_melee_pre_f0231_gates();
    test_melee_weapon_profile_plan();
    test_melee_f0231_side_effect_plan();
    test_melee_f0402_weapon_availability_and_preflight();
    test_melee_f0177_target_creature_plan();
    test_melee_f0312_strength_plan();
    test_melee_f0231_champion_snapshot_plan();
    test_melee_f0231_creature_snapshot_plan();
    test_melee_f0231_damage_gate_plan();
    test_melee_f0231_aftermath_plan();
    test_melee_f0231_reaction_and_group_apply();
    test_melee_f0231_runtime_result_plan();
    test_melee_f0231_damage_resolver_entrypoint();
    test_invalid_action();
    if (g_failures) {
        fprintf(stderr, "test_dm1_v1_action_f0407_tail_pc34_compat: %d failures\n",
                g_failures);
        return 1;
    }
    printf("test_dm1_v1_action_f0407_tail_pc34_compat: PASS\n");
    return 0;
}
