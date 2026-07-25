#include "dm1_v1_door_bash_stamina_feedback_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_stamina_cost_constants(void)
{
    assert(DM1_V1_DOOR_BASH_STAMINA_COST_BASH_PC34 == 9);
    assert(DM1_V1_DOOR_BASH_STAMINA_COST_HACK_PC34 == 6);
    assert(DM1_V1_DOOR_BASH_STAMINA_COST_BERZERK_PC34 == 40);
    assert(DM1_V1_DOOR_BASH_STAMINA_COST_KICK_PC34 == 3);
    assert(DM1_V1_DOOR_BASH_STAMINA_COST_SWING_PC34 == 2);
    assert(DM1_V1_DOOR_BASH_STAMINA_COST_CHOP_PC34 == 10);
}

static void test_action_ordinal_constants(void)
{
    assert(DM1_V1_DOOR_BASH_STAMINA_ACTION_BASH_PC34 == 0x30);
    assert(DM1_V1_DOOR_BASH_STAMINA_ACTION_HACK_PC34 == 0x18);
    assert(DM1_V1_DOOR_BASH_STAMINA_ACTION_BERZERK_PC34 == 0x13);
    assert(DM1_V1_DOOR_BASH_STAMINA_ACTION_KICK_PC34 == 0x07);
    assert(DM1_V1_DOOR_BASH_STAMINA_ACTION_SWING_PC34 == 0x0D);
    assert(DM1_V1_DOOR_BASH_STAMINA_ACTION_CHOP_PC34 == 0x02);
}

static void test_misc_constants(void)
{
    assert(DM1_V1_DOOR_BASH_STAMINA_RANDOM_BITS_PC34 == 2);
    assert(DM1_V1_DOOR_BASH_STAMINA_MELEE_CAP_PC34 == 100);
    assert(DM1_V1_DOOR_BASH_STAMINA_DISABLED_TICKS_PC34 == 6);
    assert(DM1_V1_DOOR_BASH_STAMINA_DESTRUCTION_DELAY_TICKS_PC34 == 2);
    assert(DM1_V1_DOOR_BASH_STAMINA_OVERFLOW_DAMAGE_SHIFT_PC34 == 1);
    assert(DM1_V1_DOOR_BASH_STAMINA_ACTION_INDEX_MAX_PC34 == 43);
}

static void test_attr_redraw_mask(void)
{
    assert(DM1_V1_DOOR_BASH_STAMINA_ATTR_LOAD_PC34 == 0x0200);
    assert(DM1_V1_DOOR_BASH_STAMINA_ATTR_STATISTICS_PC34 == 0x0100);
    assert(DM1_V1_DOOR_BASH_STAMINA_ATTR_REDRAW_MASK_PC34 == 0x0300);
}

static void test_defense_mirror_constants(void)
{
    assert(DM1_V1_DOOR_BASH_STAMINA_DEFENSE_PORTCULLIS_PC34 == 110);
    assert(DM1_V1_DOOR_BASH_STAMINA_DEFENSE_WOODEN_PC34 == 42);
    assert(DM1_V1_DOOR_BASH_STAMINA_DEFENSE_IRON_PC34 == 230);
    assert(DM1_V1_DOOR_BASH_STAMINA_DEFENSE_RA_PC34 == 255);
}

static void test_outcome_enum(void)
{
    assert(DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NOT_BASH_PC34 == 0);
    assert(DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NO_DECREMENT_PC34 == 1);
    assert(DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_OK_PC34 == 2);
    assert(DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_EXACT_PC34 == 3);
    assert(DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_OVERFLOW_PC34 == 4);
    assert(DM1_V1_DOOR_BASH_STAMINA_OUTCOME_CLAMP_AT_MAX_PC34 == 5);
}

static void test_adjusted_strength_full_stamina(void)
{
    int16_t r = DM1_V1_DoorBashStamina_AdjustedStrengthPc34Compat(100, 100, 200);
    (void)r;
    assert(r == 200);
}

static void test_adjusted_strength_half_stamina(void)
{
    int16_t r = DM1_V1_DoorBashStamina_AdjustedStrengthPc34Compat(25, 100, 200);
    (void)r;
    assert(r == 100 + (100 * 25) / 50);
}

static void test_adjusted_strength_zero_stamina(void)
{
    int16_t r = DM1_V1_DoorBashStamina_AdjustedStrengthPc34Compat(0, 100, 200);
    (void)r;
    assert(r == 100);
}

static void test_action_cost_bash(void)
{
    uint8_t cost = 0, rbit = 0;
    int ok = DM1_V1_DoorBashStamina_ActionCostPc34Compat(
        DM1_V1_DOOR_BASH_STAMINA_ACTION_BASH_PC34, &cost, &rbit);
    (void)ok; (void)cost;
    assert(ok);
    assert(cost == DM1_V1_DOOR_BASH_STAMINA_COST_BASH_PC34);
}

static void test_action_cost_non_bash(void)
{
    uint8_t cost = 0, rbit = 0;
    int ok = DM1_V1_DoorBashStamina_ActionCostPc34Compat(0xFF, &cost, &rbit);
    (void)ok;
    assert(!ok);
}

static void test_action_is_bash(void)
{
    int r1 = DM1_V1_DoorBashStamina_ActionIsBashPc34Compat(
        DM1_V1_DOOR_BASH_STAMINA_ACTION_BASH_PC34);
    int r2 = DM1_V1_DoorBashStamina_ActionIsBashPc34Compat(
        DM1_V1_DOOR_BASH_STAMINA_ACTION_KICK_PC34);
    int r3 = DM1_V1_DoorBashStamina_ActionIsBashPc34Compat(0xFF);
    (void)r1; (void)r2; (void)r3;
    assert(r1);
    assert(r2);
    assert(!r3);
}

static void test_source_lock(void)
{
    const char *ev = DM1_V1_DoorBashStamina_SourceLockPc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

int main(void)
{
    test_stamina_cost_constants();
    test_action_ordinal_constants();
    test_misc_constants();
    test_attr_redraw_mask();
    test_defense_mirror_constants();
    test_outcome_enum();
    test_adjusted_strength_full_stamina();
    test_adjusted_strength_half_stamina();
    test_adjusted_strength_zero_stamina();
    test_action_cost_bash();
    test_action_cost_non_bash();
    test_action_is_bash();
    test_source_lock();

    puts("ok: DM1 door bash stamina feedback (Q-DM1-05) 13 tests passed");
    return 0;
}
