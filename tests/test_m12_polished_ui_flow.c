#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "menu_startup_m12.h"
#include "menu_hit_m12.h"
#include "config_m12.h"

#include <stddef.h>
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

static int rendered_rows_differ(const unsigned char *left,
                                const unsigned char *right,
                                int width,
                                int firstRow,
                                int afterLastRow) {
    int row;
    if (!left || !right || width <= 0 || firstRow < 0 ||
        afterLastRow <= firstRow) {
        return 0;
    }
    for (row = firstRow; row < afterLastRow; ++row) {
        size_t offset = (size_t)row * (size_t)width;
        if (memcmp(left + offset, right + offset, (size_t)width) != 0) {
            return 1;
        }
    }
    return 0;
}

static int rendered_has_content(const unsigned char *framebuffer,
                                size_t pixelCount) {
    size_t i;
    if (!framebuffer) return 0;
    for (i = 0; i < pixelCount; ++i) {
        if (framebuffer[i] != 0) return 1;
    }
    return 0;
}

static int find_entry_kind(const M12_StartupMenuState *state,
                           M12_MenuEntryKind kind) {
    int i;
    for (i = 0; i < M12_StartupMenu_GetEntryCount(); ++i) {
        const M12_MenuEntry *entry = M12_StartupMenu_GetEntry(state, i);
        if (entry && entry->kind == kind) return i;
    }
    return -1;
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
    unsigned char platformCardView[320 * 200];
    unsigned char presentationCardView[320 * 200];
    unsigned char selectedPresentationView[320 * 200];
    unsigned char compactPresentationView[320 * 100];
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
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        if (!expect(state.launchRequested == 0,
                    "unavailable game accept should not launch")) return 1;
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
        if (!expect(state.view == M12_MENU_VIEW_MAIN,
                    "unavailable-data message accept should return to main")) return 1;
    } else {
        /* A configured user data root can legitimately make DM1 available
         * even when this test's nominal directory is empty. */
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
        if (!expect(state.view == M12_MENU_VIEW_MAIN,
                    "available game BACK should return to main")) return 1;
    }

    state.selectedIndex = find_entry_kind(&state, M12_MENU_ENTRY_MUSEUM);
    if (!expect(state.selectedIndex >= 0, "museum entry should exist")) return 1;
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

    state.selectedIndex = find_entry_kind(&state, M12_MENU_ENTRY_SETTINGS);
    if (!expect(state.selectedIndex >= 0, "settings entry should exist")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_SETTINGS, "settings card should open settings")) return 1;
    /* v2.7.15 settings UX: UP/DOWN cycles the visible rows of the active
     * tab (GAME tab starts on LANGUAGE; the next visible row is DATA_DIR),
     * LEFT/RIGHT cycles the tab strip and resets the row cursor to the
     * first visible row of the new tab, and ACCEPT/VALUE_RIGHT cycles the
     * value of the selected row (LANGUAGE opens the language popup). */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    if (!expect(state.settingsSelectedIndex == M12_STARTUP_SETTINGS_ROW_DATA_DIR,
                "settings DOWN should move to the next visible GAME-tab row")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    if (!expect(state.settingsTabIndex == M12_SETTINGS_TAB_GRAPHICS &&
                state.settingsSelectedIndex == M12_STARTUP_SETTINGS_ROW_GRAPHICS,
                "settings RIGHT should switch tab and reset row to the first visible row")) return 1;
    originalSetting = state.settings.graphicsIndex;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.settings.graphicsIndex != originalSetting,
                "settings ACCEPT should cycle the selected row value")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.view == M12_MENU_VIEW_MAIN, "settings BACK should return to main")) return 1;

    force_dm1_available(&state);
    state.selectedIndex = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS && state.activatedIndex == 0,
                "available DM1 accept should enter platform cards")) return 1;
    if (!expect(state.gameCardFlowStage == 0 && state.launchRequested == 0,
                "game card should wait for a verified platform choice")) return 1;
    if (!expect(M12_LegacyMenu_HandleCompactCardPointer(&state, 20, 20, 1) == 0 &&
                state.gameCardFlowStage == 0,
                "compact legacy pointer should ignore clicks outside the visible card")) return 1;
    if (!expect(M12_LegacyMenu_HandleCompactCardPointer(&state, 240, 135, 1) == 1 &&
                state.gameCardFlowStage == 1 && state.launchRequested == 0,
                "compact legacy platform card click should advance without launching")) return 1;
    if (!expect(M12_LegacyMenu_HandleCompactCardPointer(&state, 240, 135, 1) == 1 &&
                state.view == M12_MENU_VIEW_MESSAGE && state.launchRequested == 1,
                "compact legacy presentation card click should activate the visible choice")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS,
                "compact pointer ready message should return to game options")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.view == M12_MENU_VIEW_MAIN,
                "compact pointer smoke flow should return to the main menu")) return 1;
    state.selectedIndex = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS &&
                state.gameCardFlowStage == 0,
                "keyboard regression should restart from the platform card")) return 1;
    M12_StartupMenu_Draw(&state, platformCardView, 320, 200);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.gameCardFlowStage == 1 && state.launchRequested == 0,
                "verified platform card should advance to presentation cards")) return 1;
    M12_StartupMenu_Draw(&state, presentationCardView, 320, 200);
    if (!expect(rendered_rows_differ(platformCardView, presentationCardView,
                                    320, 68, 110),
                "legacy renderer should draw the active card and its value")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_Draw(&state, selectedPresentationView, 320, 200);
    if (!expect(rendered_rows_differ(presentationCardView,
                                    selectedPresentationView, 320, 68, 110),
                "legacy renderer should draw the newly selected card value")) return 1;
    M12_StartupMenu_Draw(&state, compactPresentationView, 320, 100);
    if (!expect(rendered_has_content(compactPresentationView + 22 * 320,
                                     56 * 320),
                "compact card details should render inside a short window")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1 && state.view == M12_MENU_VIEW_MESSAGE,
                "Original card should request launch and show ready message")) return 1;
    if (!expect(state.settings.scaleModeIndex == 4 &&
                state.settings.displayAspectMode == 2 &&
                state.settings.integerScaling == 1 &&
                state.settings.scalingFilterIndex == 0,
                "Original card should apply pixel-perfect source presentation")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 && intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL &&
                intent.gameId && strcmp(intent.gameId, "dm1") == 0,
                "Original card should produce a valid V1 DM1 intent")) return 1;
    /* The ready message returns to the view it was raised from
     * (game options), not straight to main. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS && state.launchRequested == 0,
                "ready message accept should clear launch and return to game options")) return 1;

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.view == M12_MENU_VIEW_MAIN,
                "game options BACK should return to main")) return 1;

    state.selectedIndex = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    if (!expect(state.gameCardFlowStage == 1 && state.gameCardSelected == 1,
                "presentation-card navigation should select Modern")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(state.launchRequested == 1 && intent.valid == 1 &&
                intent.presentationMode == M12_PRESENTATION_V21_UPSCALED,
                "Modern card should produce a valid V2.1 DM1 intent")) return 1;
    if (!expect(state.settings.scaleModeIndex == 5 &&
                state.settings.displayAspectMode == 2 &&
                state.settings.integerScaling == 0 &&
                state.settings.scalingFilterIndex == 1,
                "Modern card should apply the high-resolution smooth preset")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.view == M12_MENU_VIEW_MAIN,
                "Modern ready message should return through detailed options")) return 1;

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.shouldExit == 1, "top-level BACK should request exit")) return 1;

    puts("m12 polished ui flow: PASS");
    return 0;
}
