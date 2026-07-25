#include "dm1_v1_door_bash_feedback_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_door_defense_constants(void)
{
    assert(DM1_V1_DOOR_DEFENSE_PORTCULLIS_PC34 == 110);
    assert(DM1_V1_DOOR_DEFENSE_WOODEN_PC34 == 42);
    assert(DM1_V1_DOOR_DEFENSE_IRON_PC34 == 230);
    assert(DM1_V1_DOOR_DEFENSE_RA_PC34 == 255);
}

static void test_door_info_ordinals(void)
{
    assert(DM1_V1_DOOR_INFO_PORTCULLIS_PC34 == 0);
    assert(DM1_V1_DOOR_INFO_WOODEN_PC34 == 1);
    assert(DM1_V1_DOOR_INFO_IRON_PC34 == 2);
    assert(DM1_V1_DOOR_INFO_RA_PC34 == 3);
}

static void test_door_attributes(void)
{
    assert(DM1_V1_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH_PC34 == 0x01);
    assert(DM1_V1_DOOR_ATTR_PROJECTILES_CAN_PASS_THROUGH_PC34 == 0x02);
}

static void test_melee_cap(void)
{
    assert(DM1_V1_DOOR_MELEE_ATTACK_CAP_PC34 == 100);
}

static void test_bash_ticks(void)
{
    assert(DM1_V1_DOOR_BASH_ACTION_DISABLED_TICKS_PC34 == 6);
    assert(DM1_V1_DOOR_BASH_DESTRUCTION_DELAY_TICKS_PC34 == 2);
}

static void test_sound_constants(void)
{
    assert(DM1_V1_SOUND_COMBAT_ATTACK_DOOR_BASH_PC34 == 563);
    assert(DM1_V1_SOUND_WOODEN_THUD_DOOR_BASH_PC34 == 4);
    assert(DM1_V1_SOUND_MODE_PLAY_IF_PRIORITIZED_PC34 == 1);
    assert(DM1_V1_SOUND_MODE_PLAY_ONE_TICK_LATER_PC34 == 2);
}

static void test_door_state_constants(void)
{
    assert(DM1_V1_DOOR_STATE_CLOSED_PC34 == 4);
    assert(DM1_V1_DOOR_STATE_DESTROYED_PC34 == 5);
    assert(DM1_V1_ELEMENT_DOOR_PC34 == 4);
    assert(DM1_V1_EVENT_DOOR_DESTRUCTION_PC34 == 2);
}

static void test_action_ordinals(void)
{
    assert(DM1_V1_ACTION_BASH_PC34 == 0x30);
    assert(DM1_V1_ACTION_HACK_PC34 == 0x18);
    assert(DM1_V1_ACTION_KICK_PC34 == 0x07);
    assert(DM1_V1_ACTION_SWING_PC34 == 0x0D);
    assert(DM1_V1_ACTION_CHOP_PC34 == 0x02);
    assert(DM1_V1_ACTION_BERZERK_PC34 == 0x13);
}

static void test_outcome_enum(void)
{
    assert(DM1_V1_DOOR_BASH_OUTCOME_NO_DOOR_PC34 == 0);
    assert(DM1_V1_DOOR_BASH_OUTCOME_NOT_CLOSED_PC34 == 1);
    assert(DM1_V1_DOOR_BASH_OUTCOME_MELEE_REJECTED_PC34 == 2);
    assert(DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34 == 3);
    assert(DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34 == 4);
    assert(DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BREAK_PC34 == 5);
    assert(DM1_V1_DOOR_BASH_OUTCOME_IRON_REJECT_PC34 == 6);
    assert(DM1_V1_DOOR_BASH_OUTCOME_RA_REJECT_PC34 == 7);
}

static void test_action_is_bash(void)
{
    int r1 = DM1_V1_DoorBash_ActionIsBashPc34Compat(DM1_V1_ACTION_BASH_PC34);
    int r2 = DM1_V1_DoorBash_ActionIsBashPc34Compat(DM1_V1_ACTION_KICK_PC34);
    int r3 = DM1_V1_DoorBash_ActionIsBashPc34Compat(0xFF);
    (void)r1; (void)r2; (void)r3;
    assert(r1 == 1);
    assert(r2 == 1);
    assert(r3 == 0);
}

static void test_input_struct_layout(void)
{
    DM1_V1_DoorBashInputPc34 input;
    memset(&input, 0, sizeof(input));
    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_defense = DM1_V1_DOOR_DEFENSE_WOODEN_PC34;
    input.action_strength = 50;
    input.is_door_target = true;
    assert(input.door_type == 1);
    assert(input.door_defense == 42);
    assert(input.action_strength == 50);
}

static void test_result_struct_layout(void)
{
    DM1_V1_DoorBashResultPc34 result;
    memset(&result, 0, sizeof(result));
    assert(result.outcome == DM1_V1_DOOR_BASH_OUTCOME_NO_DOOR_PC34);
    assert(result.scheduled_destruction_event == false);
    assert(result.applied_state_change == false);
    assert(result.returned_attack == false);
    assert(result.melee_capped_to_100 == false);
}

static void test_source_lock(void)
{
    const char *ev = DM1_V1_DoorBash_SourceLockPc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

int main(void)
{
    test_door_defense_constants();
    test_door_info_ordinals();
    test_door_attributes();
    test_melee_cap();
    test_bash_ticks();
    test_sound_constants();
    test_door_state_constants();
    test_action_ordinals();
    test_outcome_enum();
    test_action_is_bash();
    test_input_struct_layout();
    test_result_struct_layout();
    test_source_lock();

    puts("ok: DM1 door bash feedback (Q-DM1-05) 13 tests passed");
    return 0;
}
