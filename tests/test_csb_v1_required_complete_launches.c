#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0700)
#endif

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

/*
 * CSB V1 launcher/profile regression.
 *
 * This mirrors the DM1 required-complete launch gate:
 * when the required CSB hash set is complete (GRAPHICS + DUNGEON),
 * the V1 launcher must request a launch even if optional CSB extras
 * are absent. The optional extras here are the same non-gating pieces
 * the CSB runtime may use for presentation only (intro/title/media
 * variations), but they are intentionally not part of the launch gate.
 *
 * Source-lock boundary:
 *   ReDMCSB ENTRANCE.C F0806 lines 409-441
 *   ReDMCSB LOADSAVE.C F0435 lines 1940-1944
 */
static const char* kCsbGameId = "csb";
static const char* kCsbTitle = "CHAOS STRIKES BACK";
static const char* kCsbVersionId = "pc34-en";
static const char* kCsbVersionLabel = "PC 3.4 English";
static const char* kCsbVersionShortLabel = "PC 3.4 EN";
static const char* kCsbGraphicsMd5 = "61fbfd56887c94adc26888a9491c6611";
static const char* kCsbDungeonMd5 = "6695d2acebce49f95db1d8f3a5c733de";
static const int kCsbGameIndex = 1;

static void seed_csb_v1_complete_required_state(M12_StartupMenuState* state) {
    M12_AssetVersionStatus* version;
    M12_AssetRequiredFileStatus* graphics;
    M12_AssetRequiredFileStatus* dungeon;

    M12_StartupMenu_InitWithDataDir(state, "/tmp/firestaff-test-csb-required", NULL);

    /* Required files matched; optional assets are absent and must not
     * affect the launch gate. We model that by leaving the original-file
     * candidate marker clear and only populating the required CSB set. */
    state->assetStatus.csbAvailable = 1;
    state->assetStatus.dm1Available = 0;
    state->assetStatus.dm2Available = 0;
    state->assetStatus.nexusAvailable = 0;
    state->assetStatus.theronAvailable = 0;
    state->assetStatus.originalFileCandidateFound = 0;
    state->assetStatus.v22_modern_assets_installed = 0;

    state->entries[kCsbGameIndex].title = kCsbTitle;
    state->entries[kCsbGameIndex].gameId = kCsbGameId;
    state->entries[kCsbGameIndex].kind = M12_MENU_ENTRY_GAME;
    state->entries[kCsbGameIndex].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[kCsbGameIndex].available = 1;

    version = &state->assetStatus.versions[kCsbGameIndex][0];
    memset(version, 0, sizeof(*version));
    version->gameId = kCsbGameId;
    version->versionId = kCsbVersionId;
    version->label = kCsbVersionLabel;
    version->shortLabel = kCsbVersionShortLabel;
    version->matched = 1;
    snprintf(version->matchedPath, sizeof(version->matchedPath),
             "/tmp/firestaff-test-csb-required/csb/GRAPHICS.DAT");
    snprintf(version->matchedMd5, sizeof(version->matchedMd5), "%s", kCsbGraphicsMd5);

    state->assetStatus.requiredFileCounts[kCsbGameIndex] = 2U;

    graphics = &state->assetStatus.requiredFiles[kCsbGameIndex][0];
    memset(graphics, 0, sizeof(*graphics));
    graphics->gameId = kCsbGameId;
    graphics->roleId = "graphics";
    graphics->label = "GRAPHICS.DAT";
    graphics->required = 1;
    graphics->matched = 1;
    snprintf(graphics->matchedPath, sizeof(graphics->matchedPath),
             "/tmp/firestaff-test-csb-required/csb/GRAPHICS.DAT");
    snprintf(graphics->matchedHash, sizeof(graphics->matchedHash), "%s", kCsbGraphicsMd5);

    dungeon = &state->assetStatus.requiredFiles[kCsbGameIndex][1];
    memset(dungeon, 0, sizeof(*dungeon));
    dungeon->gameId = kCsbGameId;
    dungeon->roleId = "dungeon";
    dungeon->label = "DUNGEON.DAT";
    dungeon->required = 1;
    dungeon->matched = 1;
    snprintf(dungeon->matchedPath, sizeof(dungeon->matchedPath),
             "/tmp/firestaff-test-csb-required/csb/DUNGEON.DAT");
    snprintf(dungeon->matchedHash, sizeof(dungeon->matchedHash), "%s", kCsbDungeonMd5);

    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->gameOptions[kCsbGameIndex].versionIndex = 0;
    state->gameOptions[kCsbGameIndex].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->activatedIndex = kCsbGameIndex;
    state->selectedIndex = kCsbGameIndex;
    state->view = M12_MENU_VIEW_GAME_OPTIONS;
    state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
}

static void check_csb_v1_required_complete_launches(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    seed_csb_v1_complete_required_state(&state);

    CHECK(M12_AssetStatus_GameHasCompleteHashSet(kCsbGameId) == 1);
    CHECK(M12_AssetStatus_GameRequiredFileCount(kCsbGameId) == 2U);
    CHECK(M12_AssetStatus_GameVerifiedFileCount(kCsbGameId) == 2U);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, kCsbGameId) == 1);
    CHECK(state.entries[kCsbGameIndex].available == 1);

    CHECK(M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, kCsbGameId) == 2U);
    graphics = M12_AssetStatus_GetRequiredFile(&state.assetStatus, kCsbGameId, 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&state.assetStatus, kCsbGameId, 1U);
    CHECK(graphics && graphics->matched == 1);
    CHECK(graphics && graphics->label && strcmp(graphics->label, "GRAPHICS.DAT") == 0);
    CHECK(dungeon && dungeon->matched == 1);
    CHECK(dungeon && dungeon->label && strcmp(dungeon->label, "DUNGEON.DAT") == 0);

    CHECK(M12_AssetStatus_HasOriginalFileCandidate(&state.assetStatus) == 0);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 1);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "READY TO LAUNCH") == 0);
    CHECK(state.messageLine2 && strcmp(state.messageLine2, kCsbTitle) == 0);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 1);
    CHECK(intent.gameId && strcmp(intent.gameId, kCsbGameId) == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, kCsbVersionId) == 0);
    CHECK(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL);
    CHECK(intent.rendererBackendAvailable == 1);
    CHECK(intent.savePath == NULL);
    CHECK(intent.options.versionIndex == 0);
    CHECK(intent.options.presentationModeIndex == M12_PRESENTATION_V1_ORIGINAL);
}

static int isolate_home(void) {
#ifdef _WIN32
    char path[256];
    snprintf(path, sizeof(path), ".\\firestaff_csb_v1_required_home_%lu",
             (unsigned long)rand());
    if (MKDIR(path) != 0) {
        return 0;
    }
    return _putenv_s("HOME", path) == 0 && _putenv_s("USERPROFILE", path) == 0;
#else
    char path[] = "/tmp/firestaff_csb_v1_required_home_XXXXXX";
    char* made = mkdtemp(path);
    if (!made) {
        return 0;
    }
    return setenv("HOME", made, 1) == 0;
#endif
}

int main(void) {
    CHECK(isolate_home());
    check_csb_v1_required_complete_launches();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: CSB V1 launcher launches when GRAPHICS+DUNGEON required hash set is complete, even with optional assets absent");
    return 0;
}
