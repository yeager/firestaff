#include "dm1_v1_door_bash_stamina_feedback_chain_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_resolve_null_input(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    memset(&out, 0, sizeof(out));
    bool ok = DM1_V1_DoorBashStaminaFeedbackChain_ResolvePc34Compat(NULL, &out);
    (void)ok;
    assert(!ok);
}

static void test_resolve_null_output(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    memset(&in, 0, sizeof(in));
    bool ok = DM1_V1_DoorBashStaminaFeedbackChain_ResolvePc34Compat(&in, NULL);
    (void)ok;
    assert(!ok);
}

static void test_resolve_non_door(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.is_door_target = false;
    in.action_ordinal = 3;
    in.current_stamina = 100;
    in.maximum_stamina = 200;
    in.base_strength = 50;
    bool ok = DM1_V1_DoorBashStaminaFeedbackChain_ResolvePc34Compat(&in, &out);
    (void)ok;
    assert(ok);
    assert(out.resolved);
}

static void test_resolve_closed_door_bash(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.is_door_target = true;
    in.door_state = 4;
    in.door_type = 0;
    in.door_defense = 10;
    in.action_ordinal = 3;
    in.magic_attack = false;
    in.current_stamina = 100;
    in.maximum_stamina = 200;
    in.base_strength = 50;
    in.random_bit = 0;
    bool ok = DM1_V1_DoorBashStaminaFeedbackChain_ResolvePc34Compat(&in, &out);
    (void)ok;
    assert(ok);
    assert(out.resolved);
    assert(out.action_ordinal_match);
}

static void test_constants(void)
{
    assert(DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34 == 6);
    assert(DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DESTRUCTION_DELAY_TICKS_PC34 == 2);
}

static void test_source_lock(void)
{
    const char *ev = DM1_V1_DoorBashStaminaFeedbackChain_SourceLockPc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

int main(void)
{
    test_resolve_null_input();
    test_resolve_null_output();
    test_resolve_non_door();
    test_resolve_closed_door_bash();
    test_constants();
    test_source_lock();

    puts("ok: DM1 door bash stamina feedback chain (Q-DM1-04) 6 tests passed");
    return 0;
}
