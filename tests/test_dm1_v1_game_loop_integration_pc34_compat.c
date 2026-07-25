#include "dm1_v1_game_loop_integration_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_state_enum(void)
{
    assert(DM1_V1_LOOP_INTEGRATION_STATE_INIT_PC34 == 0);
    assert(DM1_V1_LOOP_INTEGRATION_STATE_TITLE_PC34 == 1);
    assert(DM1_V1_LOOP_INTEGRATION_STATE_ENTRANCE_PC34 == 2);
    assert(DM1_V1_LOOP_INTEGRATION_STATE_DUNGEON_PC34 == 3);
    assert(DM1_V1_LOOP_INTEGRATION_STATE_INVENTORY_PC34 == 4);
    assert(DM1_V1_LOOP_INTEGRATION_STATE_MAP_PC34 == 5);
    assert(DM1_V1_LOOP_INTEGRATION_STATE_ENDGAME_PC34 == 6);
    assert(DM1_V1_LOOP_INTEGRATION_STATE_DEATH_PC34 == 7);
    assert(DM1_V1_LOOP_INTEGRATION_STATE_SAVE_LOAD_PC34 == 8);
    assert(DM1_V1_LOOP_INTEGRATION_STATE_RESTART_PC34 == 9);
}

static void test_init(void)
{
    DM1_V1_LoopIntegrationStatePc34 s;
    DM1_V1_LoopIntegration_InitPc34Compat(&s);
    DM1_V1_LoopIntegrationGameStatePc34 gs = DM1_V1_LoopIntegration_GetStatePc34Compat(&s);
    (void)gs;
    assert(gs == DM1_V1_LOOP_INTEGRATION_STATE_INIT_PC34);
    assert(s.running == true);
    assert(s.restart_requested == false);
    assert(s.transition_pending == false);
    assert(s.timing.max_vblank_wait == 10);
    assert(s.timing.target_frame_ms == 55);
}

static void test_set_state(void)
{
    DM1_V1_LoopIntegrationStatePc34 s;
    DM1_V1_LoopIntegration_InitPc34Compat(&s);
    DM1_V1_LoopIntegration_SetStatePc34Compat(&s, DM1_V1_LOOP_INTEGRATION_STATE_DUNGEON_PC34);
    DM1_V1_LoopIntegrationGameStatePc34 gs = DM1_V1_LoopIntegration_GetStatePc34Compat(&s);
    (void)gs;
    assert(gs == DM1_V1_LOOP_INTEGRATION_STATE_DUNGEON_PC34);
}

static void test_is_dungeon_running(void)
{
    DM1_V1_LoopIntegrationStatePc34 s;
    DM1_V1_LoopIntegration_InitPc34Compat(&s);
    int r0 = DM1_V1_LoopIntegration_IsDungeonRunningPc34Compat(&s);
    (void)r0;
    assert(!r0);
    DM1_V1_LoopIntegration_SetStatePc34Compat(&s, DM1_V1_LOOP_INTEGRATION_STATE_DUNGEON_PC34);
    int r1 = DM1_V1_LoopIntegration_IsDungeonRunningPc34Compat(&s);
    (void)r1;
    assert(r1);
}

static void test_request_restart(void)
{
    DM1_V1_LoopIntegrationStatePc34 s;
    DM1_V1_LoopIntegration_InitPc34Compat(&s);
    DM1_V1_LoopIntegration_RequestRestartPc34Compat(&s);
    assert(s.restart_requested == true);
}

static void test_stop(void)
{
    DM1_V1_LoopIntegrationStatePc34 s;
    DM1_V1_LoopIntegration_InitPc34Compat(&s);
    assert(s.running == true);
    DM1_V1_LoopIntegration_StopPc34Compat(&s);
    assert(s.running == false);
}

static void test_frame_timing_defaults(void)
{
    DM1_V1_LoopIntegrationStatePc34 s;
    DM1_V1_LoopIntegration_InitPc34Compat(&s);
    assert(s.timing.frame_count == 0);
    assert(s.timing.paused == false);
}

int main(void)
{
    test_state_enum();
    test_init();
    test_set_state();
    test_is_dungeon_running();
    test_request_restart();
    test_stop();
    test_frame_timing_defaults();

    puts("ok: DM1 game loop integration (Q-DM1-08) 7 tests passed");
    return 0;
}
