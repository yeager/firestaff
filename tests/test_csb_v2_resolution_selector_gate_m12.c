#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static int m12_test_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value);
#else
    return setenv(name, value, 1);
#endif
}

static void force_csb_available(M12_StartupMenuState* state) {
    if (!state) {
        return;
    }
    state->entries[M12_GAME_SELECT_CSB].title = "CHAOS STRIKES BACK";
    state->entries[M12_GAME_SELECT_CSB].gameId = "csb";
    state->entries[M12_GAME_SELECT_CSB].kind = M12_MENU_ENTRY_GAME;
    state->entries[M12_GAME_SELECT_CSB].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[M12_GAME_SELECT_CSB].available = 1;
    state->assetStatus.csbAvailable = 1;
    state->assetStatus.versions[M12_GAME_SELECT_CSB][0].gameId = "csb";
    state->assetStatus.versions[M12_GAME_SELECT_CSB][0].versionId = "pc34-en";
    state->assetStatus.versions[M12_GAME_SELECT_CSB][0].label = "PC 3.4 English";
    state->assetStatus.versions[M12_GAME_SELECT_CSB][0].shortLabel = "PC 3.4 EN";
    state->assetStatus.versions[M12_GAME_SELECT_CSB][0].matched = 1;
    state->gameOptions[M12_GAME_SELECT_CSB].versionIndex = 0;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->activatedIndex = M12_GAME_SELECT_CSB;
    state->launchRequested = 1;
}

static void set_csb_mode(M12_StartupMenuState* state, int mode, int resolution) {
    state->settings.graphicsIndex = mode;
    state->gameOptions[M12_GAME_SELECT_CSB].presentationModeIndex = mode;
    state->gameOptions[M12_GAME_SELECT_CSB].resolution = resolution;
}

static void check_launch_resolution(M12_StartupMenuState* state,
                                    int mode,
                                    int requestedResolution,
                                    int expectedResolution,
                                    int expectedWidth,
                                    int expectedHeight) {
    M12_LaunchIntent intent;

    set_csb_mode(state, mode, requestedResolution);
    intent = M12_StartupMenu_GetLaunchIntent(state);
    CHECK(intent.valid == 1);
    CHECK(intent.gameId && strcmp(intent.gameId, "csb") == 0);
    CHECK(intent.presentationMode == mode);
    CHECK(intent.options.presentationModeIndex == mode);
    CHECK(intent.options.resolution == expectedResolution);
    CHECK(intent.resolutionWidth == expectedWidth);
    CHECK(intent.resolutionHeight == expectedHeight);
}

int main(void) {
    char tmpTemplate[] = "/tmp/firestaff-csb-v2-resolution-XXXXXX";
    M12_StartupMenuState menu;
    M12_StartupMenuState reloaded;
    M12_LaunchIntent intent;
    int width = 0;
    int height = 0;

    if (!mkdtemp(tmpTemplate)) {
        perror("mkdtemp");
        return 1;
    }
    CHECK(m12_test_setenv("HOME", tmpTemplate) == 0);

    M12_StartupMenu_InitWithDataDir(&menu, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&menu);

    CHECK(M12_Resolution_Dimensions(M12_RES_3840x2160, &width, &height));
    CHECK(width == 3840);
    CHECK(height == 2160);
    CHECK(M12_PresentationMode_AllowsResolutionChoice(M12_PRESENTATION_V20_FILTERED));
    CHECK(M12_PresentationMode_AllowsResolutionChoice(M12_PRESENTATION_V21_UPSCALED));
    CHECK(M12_PresentationMode_AllowsResolutionChoice(M12_PRESENTATION_V22_MODERN));
    /* V2.0/V2.1/V2.2 all share the 640x400..3840x2160 selector */
    CHECK(!M12_GameOptions_RowLockedByMode(M12_GAME_OPT_ROW_RESOLUTION,
                                           M12_PRESENTATION_V20_FILTERED));
    CHECK(!M12_GameOptions_RowLockedByMode(M12_GAME_OPT_ROW_RESOLUTION,
                                           M12_PRESENTATION_V21_UPSCALED));
    CHECK(!M12_GameOptions_RowLockedByMode(M12_GAME_OPT_ROW_RESOLUTION,
                                           M12_PRESENTATION_V22_MODERN));

    check_launch_resolution(&menu,
                            M12_PRESENTATION_V21_UPSCALED,
                            M12_RES_3840x2160,
                            M12_RES_3840x2160,
                            3840,
                            2160);
    M12_StartupMenu_SaveConfig(&menu);
    M12_StartupMenu_InitWithDataDir(&reloaded, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&reloaded);
    intent = M12_StartupMenu_GetLaunchIntent(&reloaded);
    CHECK(intent.valid == 1);
    CHECK(intent.options.resolution == M12_RES_3840x2160);
    CHECK(intent.resolutionWidth == 3840);
    CHECK(intent.resolutionHeight == 2160);

    check_launch_resolution(&menu,
                            M12_PRESENTATION_V21_UPSCALED,
                            M12_RES_320x200,
                            M12_RES_320x200,
                            320,
                            200);

    check_launch_resolution(&menu,
                            M12_PRESENTATION_V20_FILTERED,
                            M12_RES_3840x2160,
                            M12_RES_3840x2160,
                            3840,
                            2160);
    M12_StartupMenu_SaveConfig(&menu);
    M12_StartupMenu_InitWithDataDir(&reloaded, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&reloaded);
    intent = M12_StartupMenu_GetLaunchIntent(&reloaded);
    CHECK(intent.valid == 1);
    CHECK(intent.options.resolution == M12_RES_3840x2160);
    CHECK(intent.resolutionWidth == 3840);
    CHECK(intent.resolutionHeight == 2160);

    set_csb_mode(&menu, M12_PRESENTATION_V22_MODERN, M12_RES_3840x2160);
    M12_StartupMenu_SaveConfig(&menu);
    M12_StartupMenu_InitWithDataDir(&reloaded, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&reloaded);
    CHECK(reloaded.settings.graphicsIndex == M12_PRESENTATION_V22_MODERN);
    CHECK(reloaded.gameOptions[M12_GAME_SELECT_CSB].presentationModeIndex ==
          M12_PRESENTATION_V22_MODERN);
    CHECK(reloaded.gameOptions[M12_GAME_SELECT_CSB].resolution ==
          M12_RES_3840x2160);
    intent = M12_StartupMenu_GetLaunchIntent(&reloaded);
    CHECK(intent.valid == 0);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("csb_v2_resolution_selector_gate_m12: ok");
    return 0;
}
