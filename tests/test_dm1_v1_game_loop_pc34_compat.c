#include "dm1_v1_game_loop_pc34_compat.h"

#include <stdio.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *label, int got, int want)
{
    g_assertions++;
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        g_failures++;
    }
}

static void test_init_and_vblank(void)
{
    DM1_V1_GameLoopStatePc34 state;

    DM1_V1_GameLoop_InitPc34Compat(&state, 0);
    expect_int("init.status", state.loopStatus, DM1_LOOP_INIT);
    expect_int("init.timer", state.timerActive, 1);
    expect_int("init.default_vblank_max", state.waitForInputMaxVBlankCount, 10);
    expect_int("init.new_map", state.newPartyMapIndex, -1);
    expect_int("init.frame_us", (int)state.targetFrameTimeUs, 20000);

    for (int i = 0; i < 9; i++) {
        DM1_V1_GameLoop_VBlankTickPc34Compat(&state);
        expect_int("vblank.no_stop", state.stopWaitingForInput, 0);
    }
    DM1_V1_GameLoop_VBlankTickPc34Compat(&state);
    expect_int("vblank.stop", state.stopWaitingForInput, 1);
    expect_int("vblank.count", (int)state.verticalBlankCount, 10);

    DM1_V1_GameLoop_InitPc34Compat(&state, 1);
    expect_int("init.extended_vblank_max", state.waitForInputMaxVBlankCount, 12);
}

static void test_tick_paths(void)
{
    DM1_V1_GameLoopStatePc34 state;
    DM1_V1_GameLoopTickResultPc34 result;

    DM1_V1_GameLoop_InitPc34Compat(&state, 0);
    DM1_V1_GameLoop_RequestNewMapPc34Compat(&state, 4);
    result = DM1_V1_GameLoop_TickPc34Compat(&state, 1234u);
    expect_int("tick.status.running", state.loopStatus, DM1_LOOP_RUNNING);
    expect_int("tick.new_map.processed", result.newMapProcessed, 1);
    expect_int("tick.new_map.index", result.newMapIndex, 4);
    expect_int("tick.timeline", result.timelineEventsProcessed, 1);
    expect_int("tick.dungeon.draw", result.dungeonViewDrawn, 1);
    expect_int("tick.highlight", result.commandHighlightDisabled, 1);
    expect_int("tick.sound", result.soundPlayed, 1);
    expect_int("tick.damage", result.damageApplied, 1);
    expect_int("tick.last_phase", result.lastPhaseCompleted, DM1_PHASE_INPUT_WAIT);
    expect_int("tick.new_map.cleared", state.newPartyMapIndex, -1);
    expect_int("tick.count", (int)state.tickCount, 1);
    expect_int("tick.total_frames", (int)state.frameStats.totalFrames, 1);

    DM1_V1_GameLoop_SetInventoryPc34Compat(&state, 1);
    result = DM1_V1_GameLoop_TickPc34Compat(&state, 1300u);
    expect_int("tick.inventory.open", result.inventoryOpen, 1);
    expect_int("tick.inventory.no_dungeon_draw", result.dungeonViewDrawn, 0);

    DM1_V1_GameLoop_SetInventoryPc34Compat(&state, 0);
    DM1_V1_GameLoop_SetRestingPc34Compat(&state, 1);
    result = DM1_V1_GameLoop_TickPc34Compat(&state, 1400u);
    expect_int("tick.resting", result.partyResting, 1);
    expect_int("tick.resting.no_dungeon_draw", result.dungeonViewDrawn, 0);
}

static void test_pause_exit_and_budget(void)
{
    DM1_V1_GameLoopStatePc34 state;
    DM1_V1_GameLoopTickResultPc34 result;
    DM1_V1_FrameTimingStatsPc34 stats;

    DM1_V1_GameLoop_InitPc34Compat(&state, 0);
    DM1_V1_GameLoop_PausePc34Compat(&state);
    expect_int("pause.status", state.loopStatus, DM1_LOOP_PAUSED);
    expect_int("pause.query", DM1_V1_GameLoop_IsPausedPc34Compat(&state), 1);
    result = DM1_V1_GameLoop_TickPc34Compat(&state, 100u);
    expect_int("pause.tick.phase", result.lastPhaseCompleted, DM1_PHASE_INPUT_WAIT);
    expect_int("pause.tick.no_frames", (int)state.frameStats.totalFrames, 0);
    DM1_V1_GameLoop_VBlankTickPc34Compat(&state);
    expect_int("pause.vblank.global_count", (int)state.verticalBlankCount, 1);
    expect_int("pause.vblank.wait_unchanged", state.waitForInputVBlankCount, 0);

    DM1_V1_GameLoop_ResumePc34Compat(&state);
    expect_int("resume.status", state.loopStatus, DM1_LOOP_RUNNING);
    expect_int("resume.query", DM1_V1_GameLoop_IsPausedPc34Compat(&state), 0);

    DM1_V1_GameLoop_SetPartyDeadPc34Compat(&state);
    expect_int("party_dead.continue", DM1_V1_GameLoop_ShouldContinuePc34Compat(&state), 0);

    DM1_V1_GameLoop_InitPc34Compat(&state, 0);
    DM1_V1_GameLoop_SetTickRatePc34Compat(&state, 100);
    expect_int("tick_rate.100hz", (int)state.targetFrameTimeUs, 10000);
    DM1_V1_GameLoop_RecordPhaseTimePc34Compat(&state, DM1_PHASE_DUNGEON_VIEW_DRAW, 15000u);
    stats = DM1_V1_GameLoop_GetFrameStatsPc34Compat(&state);
    expect_int("budget.longest", (int)stats.longestFrameUs, 15000);
    expect_int("budget.dropped", (int)stats.droppedFrames, 1);
    expect_int("budget.overrun", (int)stats.budgetOverrunCount, 1);
    DM1_V1_GameLoop_ResetFrameStatsPc34Compat(&state);
    stats = DM1_V1_GameLoop_GetFrameStatsPc34Compat(&state);
    expect_int("budget.reset", (int)stats.longestFrameUs, 0);

    DM1_V1_GameLoop_RequestExitPc34Compat(&state);
    expect_int("exit.status", state.loopStatus, DM1_LOOP_STOPPED);
    expect_int("exit.continue", DM1_V1_GameLoop_ShouldContinuePc34Compat(&state), 0);
}

int main(void)
{
    printf("probe=dm1_v1_game_loop_pc34_compat\n");
    printf("source=%s\n", DM1_V1_GameLoop_SourceEvidencePc34Compat());

    test_init_and_vblank();
    test_tick_paths();
    test_pause_exit_and_budget();

    if (g_failures) {
        printf("FAIL dm1_v1_game_loop_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_game_loop_pc34_compat assertions=%d\n", g_assertions);
    return 0;
}
