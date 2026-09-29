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
#include "firestaff_l10n.h"
#include "fs_portable_compat.h"
#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include "config_m12.h"
#ifdef _WIN32
#include <direct.h>
#define TEST_RMDIR _rmdir
#define TEST_SETENV(k,v) _putenv_s((k),(v))
#define TEST_UNSETENV(k) _putenv_s((k), "")
#else
#include <unistd.h>
#define TEST_RMDIR rmdir
#define TEST_SETENV(k,v) setenv((k),(v),1)
#define TEST_UNSETENV(k) unsetenv(k)
#endif
#include <string.h>

static int failures = 0;
static char ownedDataDirectory[FSP_PATH_MAX];

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
        state, ownedDataDirectory, NULL);

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
    /* Keep this unit test independent of a config left by an earlier run;
     * the production default is asserted explicitly below. */
    state->languageExplicit = 0;
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
    check(state.languageExplicit == 0,
          "default language policy is AUTO");
    check(strcmp(M12_StartupMenu_GetSettingsValue(
                     &state, M12_STARTUP_SETTINGS_ROW_LANGUAGE),
                 "AUTO") == 0,
          "AUTO is shown instead of the resolved system language");
    check(M12_StartupMenu_GetLanguageCount() == 21 &&
              strcmp(M12_StartupMenu_GetLanguageCode(20), "AUTO") == 0,
          "language picker exposes an explicit AUTO choice");
    for (i = 0; i < M12_StartupMenu_GetLanguageCount(); ++i) {
        const char* name = M12_StartupMenu_GetLanguageName(i);
        check(name && name[0] != '\0', "language picker has a localized display name");
    }
    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = M12_STARTUP_SETTINGS_ROW_LANGUAGE;
    /* AUTO is the picker origin; the first concrete language to its right
     * is English, whose persisted index remains 0 for compatibility. */
    expectedLanguage = 1; /* explicit SV is the second concrete entry */

    /* LANGUAGE opens the picker; select its next concrete language and
     * confirm.  The global setting must update every launch slot. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    check(state.settings.languageIndex == expectedLanguage,
          "global language advances in picker");
    check(state.languageExplicit == 1,
          "confirmed language selection becomes explicit");
    check(state.settings.languageIndex == 1 &&
              fs_l10n_get_language() == FS_LANG_SV,
          "explicit Swedish selection updates the active l10n language");
    for (i = 0; i < M12_CONFIG_GAME_COUNT; ++i) {
        check(state.gameOptions[i].languageIndex == state.settings.languageIndex,
              "global language propagates to game slot");
    }

    /* AUTO is a policy choice, not a new game-data language.  It resolves
     * the current system locale for the runtime handoff while keeping the
     * launcher preference non-explicit. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_LEFT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_LEFT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_LEFT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    check(state.languageExplicit == 0 &&
              strcmp(M12_StartupMenu_GetSettingsValue(
                         &state, M12_STARTUP_SETTINGS_ROW_LANGUAGE),
                     "AUTO") == 0,
          "AUTO selection restores system-language policy");
    check(fs_l10n_get_language() == fs_l10n_detect_system_language(),
          "AUTO selection synchronizes the detected system language");

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

static void test_music_off_preference(void) {
    M12_StartupMenuState state;
    M12_LauncherRuntimeOptions opts;

    memset(&state, 0, sizeof(state));
    seed_dm1_state(&state);
    /* This assertion checks the English label. Do not inherit the host's
     * AUTO locale (for example Swedish "AV") from the test environment. */
    state.languageExplicit = 1;
    state.settings.languageIndex = 0;
    state.settings.audioMusicVolume = 0;
    check(strcmp(M12_StartupMenu_GetSettingsValue(
                     &state, M12_STARTUP_SETTINGS_ROW_AUDIO_MUSIC),
                 "OFF") == 0,
          "music volume zero is presented as an explicit Off preference");
    M12_StartupMenu_ExportLauncherRuntimeOptions(&state, 0, &opts);
    check(opts.audioMusicVolume == 0,
          "music Off is exported through the native launcher handoff");
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
        &state, ownedDataDirectory, NULL);
    state.activatedIndex = -1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    check(intent.valid == 0, "no activated entry -> invalid intent");
    check(intent.launcherOptionsBound == 0,
          "no activated entry -> launcherOptions unbound");
}

static void test_custom_music_directory_selection_validation(void) {
    M12_StartupMenuState state;
    char currentDirectory[FSP_PATH_MAX] = {0};
    char missingDirectory[FSP_PATH_MAX];
    char overlongPath[M12_CONFIG_DATA_DIR_CAPACITY + 32];
    memset(&state, 0, sizeof(state));
    snprintf(state.settings.customMusicPath,
             sizeof(state.settings.customMusicPath),
             "%s", "previous/music/folder");

    check(FSP_ResolvePhysicalPath(currentDirectory, sizeof(currentDirectory), "."),
          "resolve existing directory for custom music path test");
    check(M12_StartupMenu_SetCustomMusicPath(&state, currentDirectory),
          "accept existing custom music directory");
    check(strcmp(state.settings.customMusicPath, currentDirectory) == 0,
          "store selected custom music directory in full");

    check(M12_StartupMenu_SetCustomMusicPath(&state, ".") &&
          strcmp(state.settings.customMusicPath, currentDirectory) == 0,
          "relative selection is stored as an absolute directory");
    check(FSP_JoinPath(missingDirectory, sizeof(missingDirectory),
          currentDirectory, "firestaff-custom-music-folder-that-does-not-exist"),
          "construct missing directory path without truncation");
    check(!FSP_DirExists(missingDirectory),
          "missing custom music directory fixture is absent");
    check(!M12_StartupMenu_SetCustomMusicPath(&state, missingDirectory),
          "reject missing custom music directory");
    check(strcmp(state.settings.customMusicPath, currentDirectory) == 0,
          "missing directory leaves previous custom music path unchanged");

    memset(overlongPath, 'x', sizeof(overlongPath) - 1U);
    overlongPath[sizeof(overlongPath) - 1U] = '\0';
    check(!M12_StartupMenu_SetCustomMusicPath(&state, overlongPath),
          "reject overlong custom music path before truncation");
    check(strcmp(state.settings.customMusicPath, currentDirectory) == 0,
          "overlong path leaves previous custom music path unchanged");
    check(!M12_StartupMenu_SetCustomMusicPath(&state, ""),
          "reject empty custom music path");
}

typedef struct CustomMusicDialogCompletion {
    void* callbackToken;
    const char* selectedPath;
} CustomMusicDialogCompletion;

static int SDLCALL complete_custom_music_dialog_on_worker(void* userdata) {
    CustomMusicDialogCompletion* completion =
        (CustomMusicDialogCompletion*)userdata;
    M12_StartupMenu_CompleteCustomMusicDirDialog(
        completion->callbackToken, completion->selectedPath);
    return 0;
}

static int run_custom_music_dialog_completion_thread(
    void* callbackToken,
    const char* selectedPath) {
    CustomMusicDialogCompletion completion;
    SDL_Thread* thread;
    int threadStatus = 0;
    completion.callbackToken = callbackToken;
    completion.selectedPath = selectedPath;
    thread = SDL_CreateThread(complete_custom_music_dialog_on_worker,
                              "custom-music-dialog-test",
                              &completion);
    if (!thread) {
        M12_StartupMenu_CompleteCustomMusicDirDialog(callbackToken, NULL);
        return 0;
    }
    SDL_WaitThread(thread, &threadStatus);
    return threadStatus == 0;
}

static void test_custom_music_dialog_thread_handoff_and_destroy(void) {
    M12_StartupMenuState state;
    M12_StartupMenuState* destroyedState;
    void* callbackToken;
    int changed;

    memset(&state, 0, sizeof(state));
    snprintf(state.settings.customMusicPath,
             sizeof(state.settings.customMusicPath),
             "%s", "previous/music/folder");
    callbackToken = M12_StartupMenu_BeginCustomMusicDirDialog(&state);
    check(callbackToken != NULL && state.dataDirPickerActive,
          "begin custom music dialog creates callback token");
    if (callbackToken) {
        check(M12_StartupMenu_BeginCustomMusicDirDialog(&state) == NULL,
              "a pending dialog cannot be replaced by another request");
        check(run_custom_music_dialog_completion_thread(callbackToken, NULL),
              "native cancel result can complete on a worker thread");
        check(state.dataDirPickerActive &&
                  strcmp(state.settings.customMusicPath,
                         "previous/music/folder") == 0,
              "worker completion does not mutate menu state");
        changed = M12_StartupMenu_Update(&state);
        check(changed && !state.dataDirPickerActive &&
                  state.customMusicDirDialogJob == NULL,
              "main-thread update consumes the dialog result");
        check(strcmp(state.settings.customMusicPath,
                     "previous/music/folder") == 0,
              "cancelled folder dialog preserves the previous path");
    }

    destroyedState = (M12_StartupMenuState*)SDL_calloc(1U,
                                                       sizeof(*destroyedState));
    check(destroyedState != NULL,
          "allocate menu state for late callback lifetime test");
    if (!destroyedState) {
        return;
    }
    callbackToken = M12_StartupMenu_BeginCustomMusicDirDialog(destroyedState);
    check(callbackToken != NULL,
          "begin second dialog before menu destruction");
    if (callbackToken) {
        M12_StartupMenu_Destroy(destroyedState);
        check(destroyedState->customMusicDirDialogJob == NULL,
              "destroy detaches the menu-owned callback reference");
        SDL_free(destroyedState);
        check(run_custom_music_dialog_completion_thread(callbackToken, "."),
              "late native callback can finish after menu destruction");
    } else {
        M12_StartupMenu_Destroy(destroyedState);
        SDL_free(destroyedState);
    }
}

int main(void) {
    const char* previous = getenv("FIRESTAFF_CONFIG_PATH");
    const char* scratch = getenv("TMPDIR");
    char* saved = NULL;
    char base[FSP_PATH_MAX];
    char configPath[M12_CONFIG_PATH_CAPACITY];
    char tmpPath[M12_CONFIG_PATH_CAPACITY + 4];
    char leaf[80];
    M12_Config isolated;
    int result;
    int dataDirectoryCreated = 0;
    if (previous) {
        saved = (char*)malloc(strlen(previous) + 1u);
        if (!saved) return 1;
        strcpy(saved, previous);
    }
    if (!scratch || !scratch[0]) scratch = "build";
    snprintf(leaf, sizeof(leaf), "launcher-options-%llu.cfg",
             (unsigned long long)SDL_GetTicksNS());
    if (!FSP_CreateDirectoryRecursive(scratch) ||
        !FSP_ResolvePhysicalPath(base, sizeof(base), scratch) ||
        !FSP_JoinPath(configPath, sizeof(configPath), base, leaf)) {
        free(saved);
        fprintf(stderr, "Cannot prepare isolated launcher config path\n");
        return 1;
    }
    snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", configPath);
    if (FSP_PathExists(configPath) || FSP_PathExists(tmpPath) ||
        TEST_SETENV("FIRESTAFF_CONFIG_PATH", configPath) != 0) {
        free(saved);
        fprintf(stderr, "Cannot claim unique launcher config path\n");
        return 1;
    }
    snprintf(leaf, sizeof(leaf), "launcher-empty-data-%llu",
             (unsigned long long)SDL_GetTicksNS());
    if (!FSP_JoinPath(ownedDataDirectory, sizeof(ownedDataDirectory), base, leaf) ||
        FSP_PathExists(ownedDataDirectory) || !FSP_CreateDirectory(ownedDataDirectory)) {
        check(0, "create owned empty launcher data directory");
        goto cleanup;
    }
    dataDirectoryCreated = 1;
    M12_Config_SetDefaults(&isolated);
    check(strcmp(isolated.path, configPath) == 0,
          "config override preserves exact task-local path");
    {
        char overlong[M12_CONFIG_PATH_CAPACITY + 32];
        memset(overlong, 'x', sizeof(overlong) - 1u);
        overlong[sizeof(overlong) - 1u] = '\0';
        check(TEST_SETENV("FIRESTAFF_CONFIG_PATH", overlong) == 0,
              "set overlong config override");
        M12_Config_SetDefaults(&isolated);
        check(isolated.path[0] == '\0' && !M12_Config_Save(&isolated),
              "overlong config override fails closed without saving");
    }
    if (TEST_SETENV("FIRESTAFF_CONFIG_PATH", configPath) != 0) {
        check(0, "restore isolated path before launcher settings actions");
        goto cleanup;
    }
    test_export_null_safety();
    test_export_field_mapping();
    test_export_clamps();
    test_export_negative_slot_leaves_per_game_zero();
    test_launch_intent_carries_options();
    test_launch_intent_options_follow_constraints();
    test_invalid_intent_leaves_options_unbound();
    test_custom_music_directory_selection_validation();
    if (SDL_Init(0)) {
        test_custom_music_dialog_thread_handoff_and_destroy();
        SDL_Quit();
    } else {
        check(0, "initialize SDL for custom music dialog lifetime tests");
    }
    test_global_language_and_preference_rows();
    test_music_off_preference();

cleanup:
    /* Remove only paths claimed above; never touch the user's profile. */
    remove(configPath);
    remove(tmpPath);
    if (dataDirectoryCreated)
        check(TEST_RMDIR(ownedDataDirectory) == 0, "remove owned empty data directory");
    if (saved) result = TEST_SETENV("FIRESTAFF_CONFIG_PATH", saved);
    else result = TEST_UNSETENV("FIRESTAFF_CONFIG_PATH");
    free(saved);
    check(result == 0, "restore previous config override environment");
    if (failures) {
        printf("test_m12_launcher_options_runtime_handoff: FAIL %d\n",
               failures);
        return 1;
    }
    puts("test_m12_launcher_options_runtime_handoff: PASS");
    return 0;
}
