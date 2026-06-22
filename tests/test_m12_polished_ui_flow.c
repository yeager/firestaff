#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "menu_startup_m12.h"
#include "config_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
static int test_setenv(const char *name, const char *value) { return _putenv_s(name, value) == 0; }
static char *test_mkdtemp(char *templ) {
    char *marker = strstr(templ, "XXXXXX");
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
static int test_setenv(const char *name, const char *value) { return setenv(name, value, 1) == 0; }
static char *test_mkdtemp(char *templ) { return mkdtemp(templ); }
#endif

static int expect(int cond, const char *msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 0;
    }
    return 1;
}

static void dismiss_initial_message(M12_StartupMenuState *state) {
    if (state && state->view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
    }
}

static void force_dm1_available(M12_StartupMenuState *state) {
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
    state->gameOptions[0].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
}

static int setup_home(void) {
    char templ[512];
    const char *base = getenv("TMPDIR");
    if (!base || !base[0]) base = getenv("TEMP");
    if (!base || !base[0]) base = "/tmp";
    snprintf(templ, sizeof(templ), "%s/firestaff-m12-flow-XXXXXX", base);
    if (!test_mkdtemp(templ)) {
        perror("mkdtemp");
        return 0;
    }
    if (!test_setenv("HOME", templ)) {
        fprintf(stderr, "FAIL: setting temporary HOME failed\n");
        return 0;
    }
    (void)test_setenv("SDL_VIDEODRIVER", "dummy");
    return 1;
}

int main(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    int originalSetting;

    if (!setup_home()) {
        return 1;
    }
    M12_Config_SetLastSavePath("");

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    dismiss_initial_message(&state);
    if (!expect(state.view == M12_MENU_VIEW_MAIN, "initial no-data message should dismiss to main")) return 1;
    if (!expect(M12_StartupMenu_GetEntryCount() >= 7, "main menu should expose games, museum, and settings")) return 1;

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_LEFT);
    if (!expect(state.view == M12_MENU_VIEW_MAIN && state.shouldExit == 0,
                "top-level LEFT should be a no-op, not an exit")) return 1;

    state.selectedIndex = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_MESSAGE && state.launchRequested == 0,
                "unavailable game accept should show a message instead of launching")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_MAIN, "message accept should return to main")) return 1;

    state.selectedIndex = 5;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_MUSEUM, "museum card should open Museum of Lore")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    if (!expect(state.museumPageIndex == 1, "museum RIGHT should advance the page")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    if (!expect(state.museumSelectedIndex == 1 && state.museumPageIndex == 0,
                "museum category change should reset page index")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.view == M12_MENU_VIEW_MAIN, "museum BACK should return to main")) return 1;

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_MAP_TOGGLE);
    if (!expect(state.view == M12_MENU_VIEW_CHANGELOG, "map-toggle shortcut should open changelog")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    if (!expect(state.changelog.scrollOffset == 1, "changelog DOWN should scroll one line")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_LEFT);
    if (!expect(state.changelog.scrollOffset == 0, "changelog LEFT should page back to top when near top")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.view == M12_MENU_VIEW_MAIN, "changelog BACK should return to main")) return 1;

    state.selectedIndex = 6;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_SETTINGS, "settings card should open settings")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    if (!expect(state.settingsSelectedIndex == 1, "settings DOWN should move to the next row")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    if (!expect(state.settingsTabIndex == 1 && state.settingsSelectedIndex == 0,
                "settings RIGHT should switch tab and reset row")) return 1;
    originalSetting = state.settings.languageIndex;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.settings.languageIndex != originalSetting,
                "settings ACCEPT on row zero should cycle the selected row value")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.view == M12_MENU_VIEW_MAIN, "settings BACK should return to main")) return 1;

    force_dm1_available(&state);
    state.selectedIndex = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS && state.activatedIndex == 0,
                "available DM1 accept should enter game options")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    if (!expect(state.gameOptSelectedRow == 1, "game options DOWN should move to version row")) return 1;
    state.gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1 && state.view == M12_MENU_VIEW_MESSAGE,
                "launch row should request launch and show ready message")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 && intent.gameId && strcmp(intent.gameId, "dm1") == 0,
                "ready launch should produce a valid DM1 intent")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_MAIN && state.launchRequested == 0,
                "ready message accept should clear launch and return to main")) return 1;

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.shouldExit == 1, "top-level BACK should request exit")) return 1;

    puts("m12 polished ui flow: PASS");
    return 0;
}
