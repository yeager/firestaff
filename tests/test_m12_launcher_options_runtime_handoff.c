/*
 * test_m12_launcher_options_runtime_handoff.c
 *
 * Jobb F2: verifies the M12 launcher hands its options (game selection
 * context, language, cheats, speed, minimap/automap/combat log,
 * soundtrack/ambient audio, UI scale, streamer mode, custom music /
 * custom dungeon / screenshot paths, session timer, audio device and volumes,
 * display brightness, font scale) over to the runtime at game start.
 *
 * Covers the M12 side of the boundary:
 *   M12_StartupMenu_ExportLauncherRuntimeOptions() extraction + clamps
 *   M12_StartupMenu_GetLaunchIntent() -> M12_LaunchIntent.launcherOptions
 *
 * The M11 consumption side (M11_GameLaunchSpec.launcherOptions ->
 * M11_GameViewState.launcherOptions, readable through
 * M11_GameView_GetLauncherRuntimeOptions) is exercised by the
 * M12->M11 launcher handoff boundary tests that boot with real data.
 *
 * No game data required (synthetic matched-version seed pattern from
 * tests/test_m12_session_timer_launch_handoff.c).
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

/* Seed a synthetic M12_StartupMenuState with a matched DM1 asset so
 * M12_StartupMenu_GetLaunchIntent returns a valid intent. */
static void seed_dm1_state(M12_StartupMenuState* state) {
    M12_AssetVersionStatus* version;

    M12_StartupMenu_InitWithDataDir(
        state, "/tmp/firestaff-test-m12-launcher-options-handoff", NULL);

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

static void seed_distinctive_settings(M12_StartupMenuState* state) {
    state->gameOptions[0].languageIndex = 3;
    state->gameOptions[0].cheatsEnabled = 1;
    state->gameOptions[0].gameSpeed = M12_GAME_SPEED_FASTER;
    state->settings.quickResumeEnabled = 1;
    state->settings.minimapEnabled = 1;
    state->settings.minimapSize = 192;
    state->settings.minimapCorner = 2;
    state->settings.autoMapEnabled = 1;
    state->settings.combatLogEnabled = 1;
    state->settings.combatLogMaxLines = 250;
    state->settings.soundtrackMode = 2;
    state->settings.ambientEnabled = 1;
    state->settings.ambientVolume = 70;
    state->settings.uiScale = 2;
    state->settings.streamerMode = 1;
    state->settings.audioMasterVolume = 80;
    state->settings.audioMusicVolume = 60;
    state->settings.audioSfxVolume = 90;
    state->settings.audioMuted = 0;
    state->settings.displayBrightness = 130;
    snprintf(state->settings.audioDeviceName,
             sizeof(state->settings.audioDeviceName), "%s", "Verified SDL device");
    state->settings.wasdMovementEnabled = 1;
    state->settings.controlSchemeIndex = 1;
    state->settings.gameSpeedMultiplier = 150;
    state->settings.fontScale = 2;
    state->settings.sessionTimerIndex = M12_SessionTimer_IndexForMinutes(30);
    snprintf(state->settings.customMusicPath,
             sizeof(state->settings.customMusicPath),
             "%s", "/music/custom.ogg");
    snprintf(state->settings.customDungeonPath,
             sizeof(state->settings.customDungeonPath),
             "%s", "/dungeons/custom");
    snprintf(state->settings.screenshotPath,
             sizeof(state->settings.screenshotPath),
             "%s", "/shots/out");
}

static void test_export_null_safety(void) {
    M12_LauncherRuntimeOptions opts;
    memset(&opts, 0xAA, sizeof(opts));
    M12_StartupMenu_ExportLauncherRuntimeOptions(NULL, 0, &opts);
    check(opts.minimapSize == 0 && opts.customMusicPath[0] == '\0',
          "export(NULL state) zeroes the snapshot");
    M12_StartupMenu_ExportLauncherRuntimeOptions(NULL, 0, NULL);
    check(1, "export(NULL out) is a safe no-op");
}

static void test_export_field_mapping(void) {
    M12_StartupMenuState state;
    M12_LauncherRuntimeOptions opts;
    memset(&state, 0, sizeof(state));
    seed_dm1_state(&state);
    seed_distinctive_settings(&state);

    M12_StartupMenu_ExportLauncherRuntimeOptions(&state, 0, &opts);
    check(opts.languageIndex == 3, "export languageIndex from game slot");
    check(opts.cheatsEnabled == 1, "export cheatsEnabled from game slot");
    check(opts.gameSpeed == M12_GAME_SPEED_FASTER,
          "export gameSpeed from game slot");
    check(opts.quickResumeEnabled == 1, "export quickResumeEnabled");
    check(opts.minimapEnabled == 1, "export minimapEnabled");
    check(opts.minimapSize == 192, "export minimapSize");
    check(opts.minimapCorner == 2, "export minimapCorner");
    check(opts.autoMapEnabled == 1, "export autoMapEnabled");
    check(opts.combatLogEnabled == 1, "export combatLogEnabled");
    check(opts.combatLogMaxLines == 250, "export combatLogMaxLines");
    check(opts.soundtrackMode == 2, "export soundtrackMode");
    check(opts.ambientEnabled == 1, "export ambientEnabled");
    check(opts.ambientVolume == 70, "export ambientVolume");
    check(opts.uiScale == 2, "export uiScale");
    check(opts.streamerMode == 1, "export streamerMode");
    check(opts.audioMasterVolume == 80, "export audioMasterVolume");
    check(opts.audioMusicVolume == 60, "export audioMusicVolume");
    check(opts.audioSfxVolume == 90, "export audioSfxVolume");
    check(opts.audioMuted == 0, "export audioMuted");
    check(opts.displayBrightness == 130, "export displayBrightness");
    check(strcmp(opts.audioDeviceName, "Verified SDL device") == 0,
          "export audioDeviceName");
    check(opts.wasdMovementEnabled == 1, "export wasdMovementEnabled");
    check(opts.controlSchemeIndex == 1, "export controlSchemeIndex");
    check(opts.gameSpeedMultiplier == 150, "export gameSpeedMultiplier");
    check(opts.fontScale == 2, "export fontScale");
    check(opts.sessionTimerLimitMinutes == 30,
          "export sessionTimerLimitMinutes resolves ladder");
    check(strcmp(opts.customMusicPath, "/music/custom.ogg") == 0,
          "export customMusicPath");
    check(strcmp(opts.customDungeonPath, "/dungeons/custom") == 0,
          "export customDungeonPath");
    check(strcmp(opts.screenshotPath, "/shots/out") == 0,
          "export screenshotPath");
}

static void test_global_language_and_preference_rows(void) {
    M12_StartupMenuState state;
    const int* rows;
    int count = 0;
    int i;
    int audioDeviceFound = 0;
    int brightnessFound = 0;
    int expectedLanguage;

    memset(&state, 0, sizeof(state));
    seed_dm1_state(&state);
    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = M12_STARTUP_SETTINGS_ROW_LANGUAGE;
    expectedLanguage = (state.settings.languageIndex + 1) %
                       M12_StartupMenu_GetLanguageCount();

    /* LANGUAGE opens the picker; select its next concrete language and
     * confirm.  The global setting must update every launch slot. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    check(state.settings.languageIndex == expectedLanguage,
          "global language advances in picker");
    for (i = 0; i < M12_CONFIG_GAME_COUNT; ++i) {
        check(state.gameOptions[i].languageIndex == state.settings.languageIndex,
              "global language propagates to game slot");
    }

    rows = M12_StartupMenu_GetSettingsRowsForTab(1, &count);
    for (i = 0; rows && i < count; ++i) {
        if (rows[i] == M12_STARTUP_SETTINGS_ROW_DISPLAY_BRIGHTNESS) {
            brightnessFound = 1;
        }
    }
    rows = M12_StartupMenu_GetSettingsRowsForTab(3, &count);
    for (i = 0; rows && i < count; ++i) {
        if (rows[i] == M12_STARTUP_SETTINGS_ROW_AUDIO_DEVICE) {
            audioDeviceFound = 1;
        }
    }
    check(brightnessFound, "brightness is listed on graphics settings tab");
    check(audioDeviceFound, "audio device is listed on audio settings tab");
}

static void test_export_clamps(void) {
    M12_StartupMenuState state;
    M12_LauncherRuntimeOptions opts;
    memset(&state, 0, sizeof(state));
    seed_dm1_state(&state);

    state.settings.minimapSize = 8;
    state.settings.minimapCorner = 9;
    state.settings.combatLogMaxLines = 100000;
    state.settings.fontScale = 42;
    M12_StartupMenu_ExportLauncherRuntimeOptions(&state, 0, &opts);
    check(opts.minimapSize == 64, "clamp minimapSize floor 64");
    check(opts.minimapCorner == 0, "clamp minimapCorner range");
    check(opts.combatLogMaxLines == 500, "clamp combatLogMaxLines ceil 500");
    check(opts.fontScale == 3, "clamp fontScale ceil 3");

    state.settings.minimapSize = 4096;
    state.settings.combatLogMaxLines = 1;
    state.settings.fontScale = -2;
    M12_StartupMenu_ExportLauncherRuntimeOptions(&state, 0, &opts);
    check(opts.minimapSize == 256, "clamp minimapSize ceil 256");
    check(opts.combatLogMaxLines == 50, "clamp combatLogMaxLines floor 50");
    check(opts.fontScale == 1, "clamp fontScale floor 1");
}

static void test_export_negative_slot_leaves_per_game_zero(void) {
    M12_StartupMenuState state;
    M12_LauncherRuntimeOptions opts;
    memset(&state, 0, sizeof(state));
    seed_dm1_state(&state);
    state.gameOptions[0].languageIndex = 7;
    state.gameOptions[0].cheatsEnabled = 1;
    M12_StartupMenu_ExportLauncherRuntimeOptions(&state, -1, &opts);
    check(opts.languageIndex == 0 && opts.cheatsEnabled == 0 &&
              opts.gameSpeed == 0,
          "negative game slot leaves per-game fields zero");
}

static void test_launch_intent_carries_options(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    memset(&state, 0, sizeof(state));
    seed_dm1_state(&state);
    seed_distinctive_settings(&state);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    check(intent.valid == 1, "GetLaunchIntent(valid) for seeded DM1");
    check(intent.launcherOptionsBound == 1,
          "intent launcherOptionsBound set");
    check(intent.launcherOptions.languageIndex == 3,
          "intent launcherOptions.languageIndex");
    check(intent.launcherOptions.cheatsEnabled == 1,
          "intent launcherOptions.cheatsEnabled");
    check(intent.launcherOptions.gameSpeed == M12_GAME_SPEED_FASTER,
          "intent launcherOptions.gameSpeed");
    check(intent.launcherOptions.minimapEnabled == 1 &&
              intent.launcherOptions.minimapSize == 192,
          "intent launcherOptions minimap");
    check(intent.launcherOptions.streamerMode == 1,
          "intent launcherOptions.streamerMode");
    check(intent.launcherOptions.sessionTimerLimitMinutes == 30,
          "intent launcherOptions.sessionTimerLimitMinutes");
    check(strcmp(intent.launcherOptions.screenshotPath, "/shots/out") == 0,
          "intent launcherOptions.screenshotPath");
}

static void test_launch_intent_options_follow_constraints(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    memset(&state, 0, sizeof(state));
    seed_dm1_state(&state);
    /* m12_enforce_mode_constraints normalizes intent.options before the
     * handoff snapshot is folded in; the snapshot must follow the
     * constrained values, not the raw stored ones. */
    state.gameOptions[0].languageIndex = 5;
    state.gameOptions[0].cheatsEnabled = 0;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    check(intent.valid == 1, "GetLaunchIntent(valid) constraint case");
    check(intent.launcherOptions.languageIndex ==
              intent.options.languageIndex,
          "launcherOptions.languageIndex matches constrained options");
    check(intent.launcherOptions.cheatsEnabled ==
              (intent.options.cheatsEnabled ? 1 : 0),
          "launcherOptions.cheatsEnabled matches constrained options");
    check(intent.launcherOptions.gameSpeed == intent.options.gameSpeed,
          "launcherOptions.gameSpeed matches constrained options");
}

static void test_invalid_intent_leaves_options_unbound(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    memset(&state, 0, sizeof(state));
    /* No activated entry: GetLaunchIntent bails before the gate. */
    M12_StartupMenu_InitWithDataDir(
        &state, "/tmp/firestaff-test-m12-launcher-options-handoff", NULL);
    state.activatedIndex = -1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    check(intent.valid == 0, "no activated entry -> invalid intent");
    check(intent.launcherOptionsBound == 0,
          "no activated entry -> launcherOptions unbound");
}

int main(void) {
    test_export_null_safety();
    test_export_field_mapping();
    test_export_clamps();
    test_export_negative_slot_leaves_per_game_zero();
    test_launch_intent_carries_options();
    test_launch_intent_options_follow_constraints();
    test_invalid_intent_leaves_options_unbound();
    test_global_language_and_preference_rows();

    if (failures) {
        printf("test_m12_launcher_options_runtime_handoff: FAIL %d\n",
               failures);
        return 1;
    }
    puts("test_m12_launcher_options_runtime_handoff: PASS");
    return 0;
}
