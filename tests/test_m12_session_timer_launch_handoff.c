/*
 * test_m12_session_timer_launch_handoff.c
 *
 * Verifies the M12 launcher carries the user's session timer setting
 * (Off | 15m | 30m | 60m | 120m) into M12_LaunchIntent so M11_GameView_Start
 * can arm the runtime reminder overlay / forced-pause boundary.
 *
 * Source of truth: M12_SessionTimer_MinutesForIndex in
 * src/ui/menu_startup_m12.c. This test does NOT verify the M11 side
 * (covered by test_m11_session_timer_overlay and the headless probe);
 * it only verifies the M12 launch-intent handoff.
 *
 * No game data required.
 */

#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(int ok, const char* name) {
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    } else {
        printf("PASS %s\n", name);
    }
}

/* Drive M12_StartupMenu_SessionTimerLimitMinutes for each ladder value. */
static void test_minutes_for_index(void) {
    check(M12_SessionTimer_MinutesForIndex(0) == 0,
          "SessionTimer_MinutesForIndex(0) = 0 (Off)");
    check(M12_SessionTimer_MinutesForIndex(1) == 15,
          "SessionTimer_MinutesForIndex(1) = 15");
    check(M12_SessionTimer_MinutesForIndex(2) == 30,
          "SessionTimer_MinutesForIndex(2) = 30");
    check(M12_SessionTimer_MinutesForIndex(3) == 60,
          "SessionTimer_MinutesForIndex(3) = 60");
    check(M12_SessionTimer_MinutesForIndex(4) == 120,
          "SessionTimer_MinutesForIndex(4) = 120");
    /* Out-of-bounds clamps to Off. */
    check(M12_SessionTimer_MinutesForIndex(-1) == 0,
          "SessionTimer_MinutesForIndex(-1) clamps to 0");
    check(M12_SessionTimer_MinutesForIndex(99) == 0,
          "SessionTimer_MinutesForIndex(99) clamps to 0");
}

static void test_index_for_minutes(void) {
    check(M12_SessionTimer_IndexForMinutes(0) == 0,
          "IndexForMinutes(0) = 0");
    check(M12_SessionTimer_IndexForMinutes(15) == 1,
          "IndexForMinutes(15) = 1");
    check(M12_SessionTimer_IndexForMinutes(30) == 2,
          "IndexForMinutes(30) = 2");
    check(M12_SessionTimer_IndexForMinutes(60) == 3,
          "IndexForMinutes(60) = 3");
    check(M12_SessionTimer_IndexForMinutes(120) == 4,
          "IndexForMinutes(120) = 4");
    /* Non-canonical values fall back to Off. */
    check(M12_SessionTimer_IndexForMinutes(45) == 0,
          "IndexForMinutes(45) falls back to 0 (Off)");
}

/* Seed a synthetic M12_StartupMenuState with a matched DM1 asset so
 * M12_StartupMenu_GetLaunchIntent returns a valid intent. Mirrors the
 * seed pattern in tests/test_m12_quick_resume_gate.c. */
static void seed_dm1_state(M12_StartupMenuState* state) {
    M12_AssetVersionStatus* version;

    M12_StartupMenu_InitWithDataDir(state, "/tmp/firestaff-test-m12-session-timer-handoff", NULL);

    state->entries[0].title = "DUNGEON MASTER";
    state->entries[0].gameId = "dm1";
    state->entries[0].kind = M12_MENU_ENTRY_GAME;
    state->entries[0].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[0].available = 1;

    version = &state->assetStatus.versions[0][0];
    memset(version, 0, sizeof(*version));
    version->gameId = "dm1";
    version->versionId = "pc34";
    version->label = "PC 3.4";
    version->shortLabel = "PC34";
    version->matched = 1;

    state->assetStatus.dm1Available = 1;
    state->gameOptions[0].versionIndex = 0;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->view = M12_MENU_VIEW_MAIN;
    state->activatedIndex = 0;
    state->launchRequested = 1;
}

static void test_launch_intent_off(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    memset(&state, 0, sizeof(state));
    seed_dm1_state(&state);
    state.settings.sessionTimerIndex = 0; /* Off */
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    check(intent.valid == 1, "GetLaunchIntent(valid) for DM1 Off");
    check(intent.sessionTimerLimitMinutes == 0,
          "LaunchIntent.sessionTimerLimitMinutes = 0 (Off)");
}

static void test_launch_intent_each_ladder(void) {
    int ladder[] = {0, 15, 30, 60, 120};
    int i;
    for (i = 0; i < (int)(sizeof(ladder) / sizeof(ladder[0])); ++i) {
        M12_StartupMenuState state;
        M12_LaunchIntent intent;
        memset(&state, 0, sizeof(state));
        seed_dm1_state(&state);
        state.settings.sessionTimerIndex =
            M12_SessionTimer_IndexForMinutes(ladder[i]);
        intent = M12_StartupMenu_GetLaunchIntent(&state);
        check(intent.valid == 1,
              "GetLaunchIntent(valid) for ladder step");
        check(intent.sessionTimerLimitMinutes == ladder[i],
              "LaunchIntent.sessionTimerLimitMinutes matches ladder");
    }
}

static void test_remaining_seconds_helper(void) {
    /* limit = 60 min = 3600s. */
    M12_StartupMenuState state;
    memset(&state, 0, sizeof(state));
    state.settings.sessionTimerIndex =
        M12_SessionTimer_IndexForMinutes(60);
    check(M12_StartupMenu_SessionTimerLimitMinutes(&state) == 60,
          "LimitMinutes(60 index) = 60");
    check(M12_StartupMenu_SessionTimerRemainingSeconds(&state, -10) == 3600,
          "RemainingSeconds clamps negative elapsed to 0");
    check(M12_StartupMenu_SessionTimerRemainingSeconds(&state, 3590) == 10,
          "RemainingSeconds near limit = 10");
    check(M12_StartupMenu_SessionTimerRemainingSeconds(&state, 3600) == 0,
          "RemainingSeconds at limit = 0");
    check(M12_StartupMenu_SessionTimerRemainingSeconds(&state, 7200) == 0,
          "RemainingSeconds past limit = 0");
    /* Off returns -1. */
    state.settings.sessionTimerIndex = 0;
    check(M12_StartupMenu_SessionTimerRemainingSeconds(&state, 100) == -1,
          "RemainingSeconds when Off = -1");
}

int main(void) {
    test_minutes_for_index();
    test_index_for_minutes();
    test_launch_intent_off();
    test_launch_intent_each_ladder();
    test_remaining_seconds_helper();

    if (failures) {
        printf("test_m12_session_timer_launch_handoff: FAIL %d\n", failures);
        return 1;
    }
    puts("test_m12_session_timer_launch_handoff: PASS");
    return 0;
}
