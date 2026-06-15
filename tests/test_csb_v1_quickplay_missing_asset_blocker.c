#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

enum {
    CSB_GAME_INDEX = 1
};

static const char* const kCsbGraphicsMd5 = "61fbfd56887c94adc26888a9491c6611";
static const char* const kCsbDungeonMd5 = "6695d2acebce49f95db1d8f3a5c733de";

static void seed_csb_quickplay_state(M12_StartupMenuState* state,
                                     int graphicsMatched,
                                     int dungeonMatched) {
    M12_AssetVersionStatus* version;
    M12_AssetRequiredFileStatus* graphics;
    M12_AssetRequiredFileStatus* dungeon;

    memset(state, 0, sizeof(*state));
    state->selectedIndex = 0;
    state->activatedIndex = -1;
    snprintf(state->assetStatus.dataDir,
             sizeof(state->assetStatus.dataDir),
             "/tmp/firestaff-test-csb-quickplay");

    state->entries[CSB_GAME_INDEX].title = "CHAOS STRIKES BACK";
    state->entries[CSB_GAME_INDEX].gameId = "csb";
    state->entries[CSB_GAME_INDEX].kind = M12_MENU_ENTRY_GAME;
    state->entries[CSB_GAME_INDEX].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;

    version = &state->assetStatus.versions[CSB_GAME_INDEX][0];
    memset(version, 0, sizeof(*version));
    version->gameId = "csb";
    version->versionId = "pc34-en";
    version->label = "PC 3.4 English";
    version->shortLabel = "PC 3.4 EN";
    version->matched = graphicsMatched;
    snprintf(version->matchedPath, sizeof(version->matchedPath),
             "/tmp/firestaff-test-csb-quickplay/csb/GRAPHICS.DAT");
    snprintf(version->matchedMd5, sizeof(version->matchedMd5), "%s", kCsbGraphicsMd5);

    state->assetStatus.requiredFileCounts[CSB_GAME_INDEX] = 2U;

    graphics = &state->assetStatus.requiredFiles[CSB_GAME_INDEX][0];
    memset(graphics, 0, sizeof(*graphics));
    graphics->gameId = "csb";
    graphics->roleId = "graphics";
    graphics->label = "GRAPHICS.DAT";
    graphics->required = 1;
    graphics->matched = graphicsMatched;
    if (graphicsMatched) {
        snprintf(graphics->matchedPath, sizeof(graphics->matchedPath),
                 "/tmp/firestaff-test-csb-quickplay/csb/GRAPHICS.DAT");
    }
    snprintf(graphics->matchedHash, sizeof(graphics->matchedHash), "%s", kCsbGraphicsMd5);

    dungeon = &state->assetStatus.requiredFiles[CSB_GAME_INDEX][1];
    memset(dungeon, 0, sizeof(*dungeon));
    dungeon->gameId = "csb";
    dungeon->roleId = "dungeon";
    dungeon->label = "DUNGEON.DAT";
    dungeon->required = 1;
    dungeon->matched = dungeonMatched;
    if (dungeonMatched) {
        snprintf(dungeon->matchedPath, sizeof(dungeon->matchedPath),
                 "/tmp/firestaff-test-csb-quickplay/csb/DUNGEON.DAT");
    }
    snprintf(dungeon->matchedHash, sizeof(dungeon->matchedHash), "%s", kCsbDungeonMd5);

    state->assetStatus.csbAvailable = graphicsMatched && dungeonMatched ? 1 : 0;
    state->entries[CSB_GAME_INDEX].available = state->assetStatus.csbAvailable;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->gameOptions[CSB_GAME_INDEX].versionIndex = 0;

    state->selectedIndex = -1;
    state->activatedIndex = CSB_GAME_INDEX;
    state->quickResumeAvailable = 1;
    state->quickResumeLaunchRequested = 0;
    snprintf(state->quickResumeGameId, sizeof(state->quickResumeGameId), "csb");
    snprintf(state->quickResumeSavePath, sizeof(state->quickResumeSavePath),
             "/tmp/firestaff-test-csb-quickplay/firestaff-csb-quicksave.sav");
    state->view = M12_MENU_VIEW_MAIN;
}

static void check_csb_quickplay_blocks_missing_asset(
    int graphicsMatched,
    int dungeonMatched,
    const char* expectedMissingLabel) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;

    seed_csb_quickplay_state(&state, graphicsMatched, dungeonMatched);

    CHECK(state.quickResumeAvailable == 1);
    CHECK(state.selectedIndex == -1);
    CHECK(state.entries[CSB_GAME_INDEX].available == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "csb") == 0);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);

    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strstr(state.messageLine1, "GAME DATA NOT FOUND") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, expectedMissingLabel) != NULL);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);
    CHECK(intent.gameId && strcmp(intent.gameId, "csb") == 0);
    CHECK(intent.savePath == NULL);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MAIN);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "") == 0);
}

static void check_csb_quickplay_ready_return_clears_launch_latches(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;

    seed_csb_quickplay_state(&state, 1, 1);

    CHECK(state.quickResumeAvailable == 1);
    CHECK(state.selectedIndex == -1);
    CHECK(state.entries[CSB_GAME_INDEX].available == 1);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "csb") == 1);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);

    CHECK(state.launchRequested == 1);
    CHECK(state.quickResumeLaunchRequested == 1);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "RESUMING SAVE") == 0);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 1);
    CHECK(intent.gameId && strcmp(intent.gameId, "csb") == 0);
    CHECK(intent.savePath && strcmp(intent.savePath, state.quickResumeSavePath) == 0);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MAIN);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "") == 0);
    CHECK(state.messageLine2 && strcmp(state.messageLine2, "") == 0);
    CHECK(state.messageLine3 && strcmp(state.messageLine3, "") == 0);
}

int main(void) {
    /*
     * CSB quickplay must use the same required-asset gate as normal launch:
     * GRAPHICS.DAT selects the verified version/profile, while DUNGEON.DAT
     * is the required runtime map load boundary. Source-lock references:
     * ReDMCSB ENTRANCE.C F0806 lines 409-441 and LOADSAVE.C F0435
     * lines 1940-1944.
     */
    check_csb_quickplay_blocks_missing_asset(1, 0, "DUNGEON.DAT");
    check_csb_quickplay_blocks_missing_asset(0, 1, "GRAPHICS.DAT");
    check_csb_quickplay_ready_return_clears_launch_latches();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: CSB V1 quickplay blocks missing required files and clears launch latches on menu return");
    return 0;
}
