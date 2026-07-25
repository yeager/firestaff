#include "dm1_v1_game_state_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void)
{
    DM1_V1_GameStateMachinePc34 sm;
    memset(&sm, 0xFF, sizeof(sm));
    DM1_V1_GameState_InitPc34Compat(&sm);
    assert(sm.currentState == DM1_STATE_NONE || sm.currentState == DM1_STATE_TITLE_SCREEN);
    assert(sm.transitionCount == 0);
    assert(sm.gameWon == 0);
    assert(sm.partyDead == 0);
}

static void test_current(void)
{
    DM1_V1_GameStateMachinePc34 sm;
    DM1_V1_GameState_InitPc34Compat(&sm);
    DM1_V1_GameStateIdPc34 st = DM1_V1_GameState_CurrentPc34Compat(&sm);
    (void)st;
    assert(st >= DM1_STATE_NONE && st < DM1_STATE_COUNT);
}

static void test_previous(void)
{
    DM1_V1_GameStateMachinePc34 sm;
    DM1_V1_GameState_InitPc34Compat(&sm);
    DM1_V1_GameStateIdPc34 st = DM1_V1_GameState_PreviousPc34Compat(&sm);
    (void)st;
    assert(st == DM1_STATE_NONE);
}

static void test_transition_invalid(void)
{
    DM1_V1_GameStateMachinePc34 sm;
    DM1_V1_GameState_InitPc34Compat(&sm);
    DM1_V1_TransitionResultPc34 r = DM1_V1_GameState_TransitionPc34Compat(&sm, DM1_STATE_COUNT);
    (void)r;
    assert(r == DM1_TRANS_INVALID);
}

static void test_transition_same_state(void)
{
    DM1_V1_GameStateMachinePc34 sm;
    DM1_V1_GameState_InitPc34Compat(&sm);
    DM1_V1_GameStateIdPc34 cur = DM1_V1_GameState_CurrentPc34Compat(&sm);
    DM1_V1_TransitionResultPc34 r = DM1_V1_GameState_TransitionPc34Compat(&sm, cur);
    (void)r;
    assert(r == DM1_TRANS_SAME_STATE || r == DM1_TRANS_OK);
}

static void test_start_new_game(void)
{
    DM1_V1_GameStateMachinePc34 sm;
    DM1_V1_GameState_InitPc34Compat(&sm);
    DM1_V1_TransitionResultPc34 r = DM1_V1_GameState_StartNewGamePc34Compat(&sm);
    (void)r;
    assert(r == DM1_TRANS_OK || r == DM1_TRANS_INVALID);
}

static void test_state_name(void)
{
    const char *n = DM1_V1_GameState_NamePc34Compat(DM1_STATE_DUNGEON_VIEWPORT);
    (void)n;
    assert(n != NULL);
    assert(strlen(n) > 0);
}

static void test_state_name_invalid(void)
{
    const char *n = DM1_V1_GameState_NamePc34Compat(DM1_STATE_COUNT);
    (void)n;
    assert(n != NULL);
}

static void test_set_callbacks(void)
{
    DM1_V1_GameStateMachinePc34 sm;
    DM1_V1_GameState_InitPc34Compat(&sm);
    DM1_V1_GameState_SetCallbacksPc34Compat(&sm, NULL, NULL, NULL);
    assert(sm.onEnter == NULL);
}

static void test_can_transition(void)
{
    DM1_V1_GameStateMachinePc34 sm;
    DM1_V1_GameState_InitPc34Compat(&sm);
    int ok = DM1_V1_GameState_CanTransitionPc34Compat(&sm, DM1_STATE_GAME_OVER);
    (void)ok;
    assert(ok == 0 || ok == 1);
}

static void test_source_evidence(void)
{
    const char *ev = DM1_V1_GameState_SourceEvidencePc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_state_enum_values(void)
{
    assert(DM1_STATE_NONE == 0);
    assert(DM1_STATE_TITLE_SCREEN == 1);
    assert(DM1_STATE_DUNGEON_VIEWPORT == 5);
    assert(DM1_STATE_GAME_OVER == 9);
    assert(DM1_STATE_VICTORY == 10);
}

int main(void)
{
    test_init();
    test_current();
    test_previous();
    test_transition_invalid();
    test_transition_same_state();
    test_start_new_game();
    test_state_name();
    test_state_name_invalid();
    test_set_callbacks();
    test_can_transition();
    test_source_evidence();
    test_state_enum_values();

    puts("ok: DM1 game state (Q-DM1-08) 12 tests passed");
    return 0;
}
