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
    test_invalid_action();
    if (g_failures) {
        fprintf(stderr, "test_dm1_v1_action_f0407_tail_pc34_compat: %d failures\n",
                g_failures);
        return 1;
    }
    printf("test_dm1_v1_action_f0407_tail_pc34_compat: PASS\n");
    return 0;
}
