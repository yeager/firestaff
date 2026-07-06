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
 * DM2 V1 launcher required-file popup gate.
 *
 * Synthetic, no copyrighted data required. This locks the M12 contract:
 * DM2 V1 launch requires exactly the verified GRAPHICS + DUNGEON rows,
 * the missing-data popup names all and only unmatched required rows, and
 * optional extras/marker files must not become launch blockers once
 * the required pair is matched.
 */
static const char* kDm2GameId = "dm2";
static const char* kDm2Title = "DUNGEON MASTER II";
static const char* kDm2VersionId = "pc-en";
static const char* kDm2VersionLabel = "PC English";
static const char* kDm2VersionShortLabel = "PC EN";
static const char* kDm2GraphicsMd5 = "25247ede4dabb6a71e5dabdfbcd5907d";
static const char* kDm2DungeonMd5 = "6caccd7875009e82fe2e28e7f6d6adc0";
static const char* kDm2Root = "/tmp/firestaff-test-dm2-required-popup";
static const int kDm2GameIndex = 2;

static void seed_dm2_required_state(M12_StartupMenuState* state,
                                    int graphicsMatched,
                                    int dungeonMatched,
                                    int optionalMarkerFound) {
    M12_AssetVersionStatus* version;
    M12_AssetRequiredFileStatus* graphics;
    M12_AssetRequiredFileStatus* dungeon;

    M12_StartupMenu_InitWithDataDir(state, kDm2Root, NULL);

    state->assetStatus.dm1Available = 0;
    state->assetStatus.csbAvailable = 0;
    state->assetStatus.dm2Available = graphicsMatched && dungeonMatched ? 1 : 0;
    state->assetStatus.nexusAvailable = 0;
    state->assetStatus.theronAvailable = 0;
    state->assetStatus.originalFileCandidateFound = optionalMarkerFound;
    state->assetStatus.v22_modern_assets_installed = 0;

    state->entries[kDm2GameIndex].title = kDm2Title;
    state->entries[kDm2GameIndex].gameId = kDm2GameId;
    state->entries[kDm2GameIndex].kind = M12_MENU_ENTRY_GAME;
    state->entries[kDm2GameIndex].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[kDm2GameIndex].available =
        M12_AssetStatus_GameAvailable(&state->assetStatus, kDm2GameId);

    version = &state->assetStatus.versions[kDm2GameIndex][0];
    memset(version, 0, sizeof(*version));
    version->gameId = kDm2GameId;
    version->versionId = kDm2VersionId;
    version->label = kDm2VersionLabel;
    version->shortLabel = kDm2VersionShortLabel;
    version->matched = graphicsMatched ? 1 : 0;
    if (graphicsMatched) {
        snprintf(version->matchedPath, sizeof(version->matchedPath),
                 "%s/dm2/GRAPHICS.DAT", kDm2Root);
        snprintf(version->matchedMd5, sizeof(version->matchedMd5),
                 "%s", kDm2GraphicsMd5);
    }

    state->assetStatus.requiredFileCounts[kDm2GameIndex] = 2U;

    graphics = &state->assetStatus.requiredFiles[kDm2GameIndex][0];
    memset(graphics, 0, sizeof(*graphics));
    graphics->gameId = kDm2GameId;
    graphics->roleId = "graphics";
    graphics->label = "GRAPHICS.DAT";
    graphics->required = 1;
    graphics->matched = graphicsMatched ? 1 : 0;
    if (graphicsMatched) {
        snprintf(graphics->matchedPath, sizeof(graphics->matchedPath),
                 "%s/dm2/GRAPHICS.DAT", kDm2Root);
        snprintf(graphics->matchedHash, sizeof(graphics->matchedHash),
                 "%s", kDm2GraphicsMd5);
    }

    dungeon = &state->assetStatus.requiredFiles[kDm2GameIndex][1];
    memset(dungeon, 0, sizeof(*dungeon));
    dungeon->gameId = kDm2GameId;
    dungeon->roleId = "dungeon";
    dungeon->label = "DUNGEON.DAT";
    dungeon->required = 1;
    dungeon->matched = dungeonMatched ? 1 : 0;
    if (dungeonMatched) {
        snprintf(dungeon->matchedPath, sizeof(dungeon->matchedPath),
                 "%s/dm2/DUNGEON.DAT", kDm2Root);
        snprintf(dungeon->matchedHash, sizeof(dungeon->matchedHash),
                 "%s", kDm2DungeonMd5);
    }

    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->gameOptions[kDm2GameIndex].versionIndex = 0;
    state->gameOptions[kDm2GameIndex].presentationModeIndex =
        M12_PRESENTATION_V1_ORIGINAL;
    state->activatedIndex = kDm2GameIndex;
    state->selectedIndex = kDm2GameIndex;
    state->view = M12_MENU_VIEW_GAME_OPTIONS;
    state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
}

static void check_dm2_missing_required_popup(int graphicsMatched,
                                             int dungeonMatched,
                                             const char* expectedMissing,
                                             const char* expectedAbsent) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    seed_dm2_required_state(&state, graphicsMatched, dungeonMatched, 1);

    CHECK(M12_AssetStatus_GameHasCompleteHashSet(kDm2GameId) == 1);
    CHECK(M12_AssetStatus_GameRequiredFileCount(kDm2GameId) == 2U);
    CHECK(M12_AssetStatus_GameVerifiedFileCount(kDm2GameId) == 2U);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, kDm2GameId) == 0);
    CHECK(state.entries[kDm2GameIndex].available == 0);

    CHECK(M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, kDm2GameId) == 2U);
    graphics = M12_AssetStatus_GetRequiredFile(&state.assetStatus, kDm2GameId, 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&state.assetStatus, kDm2GameId, 1U);
    CHECK(graphics && graphics->label && strcmp(graphics->label, "GRAPHICS.DAT") == 0);
    CHECK(dungeon && dungeon->label && strcmp(dungeon->label, "DUNGEON.DAT") == 0);
    CHECK(graphics && graphics->matched == (graphicsMatched ? 1 : 0));
    CHECK(dungeon && dungeon->matched == (dungeonMatched ? 1 : 0));

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageIsMissingGameData == 1);
    CHECK(strcmp(state.messageGameId, kDm2GameId) == 0);
    CHECK(state.messageLine1 && strstr(state.messageLine1, "DM2") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, expectedMissing) != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, expectedAbsent) == NULL);
    CHECK(state.messageLine3 && strstr(state.messageLine3, "DATA DIR:") != NULL);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);
    CHECK(intent.gameId && strcmp(intent.gameId, kDm2GameId) == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, kDm2VersionId) == 0);
}

static void check_dm2_all_required_missing_popup(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;

    seed_dm2_required_state(&state, 0, 0, 1);

    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, kDm2GameId) == 0);
    CHECK(state.entries[kDm2GameIndex].available == 0);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageIsMissingGameData == 1);
    CHECK(strcmp(state.messageGameId, kDm2GameId) == 0);
    CHECK(state.messageLine1 && strstr(state.messageLine1, "DM2") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "GRAPHICS.DAT") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "DUNGEON.DAT") != NULL);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);
    CHECK(intent.gameId && strcmp(intent.gameId, kDm2GameId) == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, kDm2VersionId) == 0);
}

static void check_dm2_required_complete_launches_without_optional_marker(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    seed_dm2_required_state(&state, 1, 1, 0);

    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, kDm2GameId) == 1);
    CHECK(state.entries[kDm2GameIndex].available == 1);
    CHECK(M12_AssetStatus_HasOriginalFileCandidate(&state.assetStatus) == 0);

    graphics = M12_AssetStatus_GetRequiredFile(&state.assetStatus, kDm2GameId, 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&state.assetStatus, kDm2GameId, 1U);
    CHECK(graphics && graphics->matched == 1);
    CHECK(dungeon && dungeon->matched == 1);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 1);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageIsMissingGameData == 0);
    CHECK(state.messageGameId[0] == '\0');
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "READY TO LAUNCH") == 0);
    CHECK(state.messageLine2 && strcmp(state.messageLine2, kDm2Title) == 0);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 1);
    CHECK(intent.gameId && strcmp(intent.gameId, kDm2GameId) == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, kDm2VersionId) == 0);
    CHECK(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL);
    CHECK(intent.rendererBackendAvailable == 1);
    CHECK(intent.savePath == NULL);
    CHECK(intent.options.versionIndex == 0);
    CHECK(intent.options.presentationModeIndex == M12_PRESENTATION_V1_ORIGINAL);
}

static int isolate_home(void) {
#ifdef _WIN32
    char path[256];
    snprintf(path, sizeof(path), ".\\firestaff_dm2_required_popup_home_%lu",
             (unsigned long)rand());
    if (MKDIR(path) != 0) {
        return 0;
    }
    return _putenv_s("HOME", path) == 0 && _putenv_s("USERPROFILE", path) == 0;
#else
    char path[] = "/tmp/firestaff_dm2_required_popup_home_XXXXXX";
    char* made = mkdtemp(path);
    if (!made) {
        return 0;
    }
    return setenv("HOME", made, 1) == 0;
#endif
}

int main(void) {
    CHECK(isolate_home());
    check_dm2_all_required_missing_popup();
    check_dm2_missing_required_popup(0, 1, "GRAPHICS.DAT", "DUNGEON.DAT");
    check_dm2_missing_required_popup(1, 0, "DUNGEON.DAT", "GRAPHICS.DAT");
    check_dm2_required_complete_launches_without_optional_marker();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: DM2 V1 popup gate blocks missing GRAPHICS/DUNGEON and ignores absent optional markers when required data is complete");
    return 0;
}
