#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include "menu_hit_m12.h"
#include "menu_startup_m12.h"
#include "config_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
static int test_mkdir(const char* path) { return _mkdir(path) == 0; }
static int test_setenv(const char* name, const char* value) { return _putenv_s(name, value) == 0; }
static char* test_mkdtemp(char* templ) {
    char* marker = strstr(templ, "XXXXXX");
    int i;
    if (!marker) return NULL;
    for (i = 0; i < 1000; ++i) {
        snprintf(marker, 7, "%06ld", ((long)_getpid() + i) % 1000000L);
        if (_mkdir(templ) == 0) return templ;
    }
    return NULL;
}
#else
#include <unistd.h>
static int test_mkdir(const char* path) { return mkdir(path, 0777) == 0; }
static int test_setenv(const char* name, const char* value) { return setenv(name, value, 1) == 0; }
static char* test_mkdtemp(char* templ) { return mkdtemp(templ); }
#endif

static int test_file_exists(const char* path) {
    struct stat st;
    return path && stat(path, &st) == 0;
}

static void force_dm1_available(M12_StartupMenuState* state) {
    state->entries[0].title = "DUNGEON MASTER";
    state->entries[0].gameId = "dm1";
    state->entries[0].kind = M12_MENU_ENTRY_GAME;
    state->entries[0].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[0].available = 1;
    state->assetStatus.dm1Available = 1;
    state->assetStatus.versions[0][0].gameId = "dm1";
    state->assetStatus.versions[0][0].versionId = "pc34-en";
    state->assetStatus.versions[0][0].label = "PC 3.4 English";
    state->assetStatus.versions[0][0].shortLabel = "PC 3.4 EN";
    state->assetStatus.versions[0][0].matched = 1;
    state->gameOptions[0].versionIndex = 0;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
}

static int expect(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 0;
    }
    return 1;
}

int main(void) {
    M12_StartupMenuState state;
    M12_MouseHit hit;
    M12_Config config;
    int changed;
    char homeTemplate[] = "/tmp/firestaff-m12-hit-home-XXXXXX";
    char* homeDir = test_mkdtemp(homeTemplate);
    char manualDir[512];
    const int gridLeft = 42 + 390 + 44;
    const int cardW = (1920 - gridLeft - 48 - 22 * 2) / 3;
    const int cardH = ((1080 - 130) - 40 - 22) / 2;
    const int dm1CardCenterX = gridLeft + cardW / 2;
    const int cardCenterY = 40 + cardH / 2;
    const int launchCenterX = 960;
    const int launchCenterY = 190 + 780 - 54 - 24 + 27;
    const int originalModeCenterX = 132 + 408;
    const int customModeCenterX = 132 + 817 + 22 + 408;
    const int modeChoiceCenterY = 190 + 34 + 78;
    const int settingsDataDirCenterX = 960;
    const int settingsCycleCenterX = 1240;
    const int settingsSmoothTurnPanCenterY = 260 + 36 + 3 * 70 + 25;
    const int settingsDataDirCenterY = 260 + 36 + 4 * 70 + 25;
    const int settingsExportCenterY = 260 + 36 + 5 * 70 + 25;
    const int settingsImportCenterY = 260 + 36 + 6 * 70 + 25;

    if (!homeDir || !test_setenv("HOME", homeDir) ||
        !test_setenv("SDL_VIDEODRIVER", "dummy")) {
        fprintf(stderr, "FAIL: temporary HOME setup failed\n");
        return 1;
    }
    snprintf(manualDir, sizeof(manualDir), "%s/manual-data-root", homeDir);
    if (!test_mkdir(manualDir)) {
        fprintf(stderr, "FAIL: temporary manual data directory setup failed\n");
        return 1;
    }

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    force_dm1_available(&state);

    changed = M12_ModernMenu_HandlePointer(&state, dm1CardCenterX, cardCenterY, 0, NULL);
    if (!expect(changed == 0 || state.selectedIndex == 0, "DM1 hover should keep/select DM1 card")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, dm1CardCenterX, cardCenterY, 1, NULL);
    if (!expect(changed == 1, "DM1 card direct click should change menu state")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS, "DM1 card direct click should enter game options")) return 1;
    if (!expect(state.activatedIndex == 0, "DM1 direct click should activate DM1")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, customModeCenterX, modeChoiceCenterY, 1, NULL);
    if (!expect(changed == 1, "Custom mode column should be clickable")) return 1;
    if (!expect(state.gameOptions[0].presentationModeIndex != M12_PRESENTATION_V1_ORIGINAL,
                "Custom mode column should switch away from Original")) return 1;
    changed = M12_ModernMenu_HandlePointer(&state, originalModeCenterX, modeChoiceCenterY, 1, NULL);
    if (!expect(changed == 1, "Original mode column should be clickable")) return 1;
    if (!expect(state.gameOptions[0].presentationModeIndex == M12_PRESENTATION_V1_ORIGINAL,
                "Original mode column should restore original presentation")) return 1;

    hit = M12_ModernMenu_HitTest(&state, launchCenterX, launchCenterY);
    if (!expect(hit.kind == M12_HIT_GAMEOPT_LAUNCH, "visible centered V1 Launch button should hit launch action")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, launchCenterX, launchCenterY, 1, NULL);
    if (!expect(changed == 1, "Launch direct click should be applied")) return 1;
    if (!expect(state.launchRequested == 1, "Launch direct click should request launch")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_MESSAGE, "Launch direct click should show ready-to-launch message")) return 1;

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    {
        const int settingsCenterX = gridLeft + 2 * (cardW + 22) + cardW / 2;
        const int settingsCenterY = 40 + cardH + 22 + cardH / 2;
        changed = M12_ModernMenu_HandlePointer(&state, settingsCenterX, settingsCenterY, 0, NULL);
        if (!expect(changed == 1 && state.selectedIndex == 6, "Firestaff hover should navigate to global settings card")) return 1;
        changed = M12_ModernMenu_HandlePointer(&state, settingsCenterX, settingsCenterY, 1, NULL);
        if (!expect(changed == 1 && state.view == M12_MENU_VIEW_SETTINGS, "Firestaff click should open settings view")) return 1;
    }

    state.settings.dm1V2SmoothTurnPanEnabled = 0;
    state.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    hit = M12_ModernMenu_HitTest(&state, settingsCycleCenterX, settingsSmoothTurnPanCenterY);
    if (!expect(hit.kind == M12_HIT_SETTINGS_CYCLE && hit.index == 14,
                "visible Smooth Turn Pan settings row should hit the V2 turn-pan toggle")) return 1;
    changed = M12_ModernMenu_HandlePointer(&state, settingsCycleCenterX, settingsSmoothTurnPanCenterY, 1, NULL);
    if (!expect(changed == 1 && state.settings.dm1V2SmoothTurnPanEnabled == 1,
                "Smooth Turn Pan click should toggle the V2 turn-pan setting on")) return 1;
    M12_StartupMenu_SaveConfig(&state);
    M12_Config_Load(&config, NULL);
    if (!expect(config.dm1V2SmoothTurnPanEnabled == 1,
                "Smooth Turn Pan click should persist through M12 config")) return 1;
    if (!expect(config.graphicsIndex == M12_PRESENTATION_V1_ORIGINAL,
                "Smooth Turn Pan persistence should preserve original graphics mode")) return 1;

    hit = M12_ModernMenu_HitTest(&state, settingsDataDirCenterX, settingsDataDirCenterY);
    if (!expect(hit.kind == M12_HIT_SETTINGS_CYCLE && hit.index == 15,
                "visible Data Directory settings row should hit the browse action")) return 1;
    hit = M12_ModernMenu_HitTest(&state, settingsDataDirCenterX, settingsExportCenterY);
    if (!expect(hit.kind == M12_HIT_SETTINGS_CYCLE && hit.index == 41,
                "visible Export Settings row should hit the save action")) return 1;
    changed = M12_ModernMenu_HandlePointer(&state, settingsDataDirCenterX, settingsExportCenterY, 1, NULL);
    if (!expect(changed == 1 && state.view == M12_MENU_VIEW_MESSAGE,
                "Export Settings click should show a public result message")) return 1;
    if (!expect(test_file_exists(M12_Config_GetExportPath()) == 1,
                "Export Settings click should create the default settings JSON file")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    state.view = M12_MENU_VIEW_SETTINGS;
    hit = M12_ModernMenu_HitTest(&state, settingsDataDirCenterX, settingsImportCenterY);
    if (!expect(hit.kind == M12_HIT_SETTINGS_CYCLE && hit.index == 42,
                "visible Import Settings row should hit the load action")) return 1;
    changed = M12_ModernMenu_HandlePointer(&state, settingsDataDirCenterX, settingsImportCenterY, 1, NULL);
    if (!expect(changed == 1 && state.view == M12_MENU_VIEW_MESSAGE,
                "Import Settings click should show a public result message")) return 1;
    if (!expect(strcmp(state.messageLine1, "SETTINGS IMPORTED") == 0,
                "Import Settings click should load the default settings JSON file")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    state.view = M12_MENU_VIEW_SETTINGS;
    if (!expect(M12_StartupMenu_SetDataDirectory(&state, manualDir) == 1,
                "manual data directory setter should accept an existing arbitrary folder")) return 1;
    if (!expect(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), manualDir) == 0,
                "manual data directory setter should rescan the chosen folder")) return 1;
    M12_Config_Load(&config, NULL);
    if (!expect(strcmp(config.dataDir, manualDir) == 0,
                "manual data directory setter should persist the chosen folder")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    state.view = M12_MENU_VIEW_SETTINGS;
    remove(M12_Config_GetExportPath());
    changed = M12_ModernMenu_HandlePointer(&state, settingsDataDirCenterX, settingsImportCenterY, 1, NULL);
    if (!expect(changed == 1 && state.view == M12_MENU_VIEW_MESSAGE,
                "Import Settings missing-file click should show a public result message")) return 1;
    if (!expect(strcmp(state.messageLine1, "IMPORT FAILED") == 0,
                "Import Settings missing-file click should report import failure")) return 1;
    if (!expect(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), manualDir) == 0,
                "failed settings import should preserve the active data directory")) return 1;
    M12_Config_Load(&config, NULL);
    if (!expect(strcmp(config.dataDir, manualDir) == 0,
                "failed settings import should preserve the persisted data directory")) return 1;

    puts("ok: mouse hover navigates main cards; clicks open DM1, Firestaff settings and launch DM1; Smooth Turn Pan toggles/persists; settings rows export/import JSON, missing import preserves data directory, and data directory accepts an arbitrary selected folder");
    return 0;
}
