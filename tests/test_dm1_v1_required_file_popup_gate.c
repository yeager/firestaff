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
 * DM1 V1 launcher required-file popup gate.
 *
 * Synthetic, no copyrighted data required. This locks the M12 contract:
 * optional original-file candidates may suppress the broad "no game data"
 * popup, but they must never satisfy the DM1 launch gate. The missing-data
 * popup must name only the unmatched required GRAPHICS/DUNGEON rows.
 */
static const char* kDm1GameId = "dm1";
static const char* kDm1Title = "DUNGEON MASTER";
static const char* kDm1VersionId = "pc34-en";
static const char* kDm1VersionLabel = "PC 3.4 English";
static const char* kDm1VersionShortLabel = "PC 3.4 EN";
static const char* kDm1GraphicsMd5 = "fa6b1aa29e191418713bf2cda93d962e";
static const char* kDm1DungeonMd5 = "766450c940651fc021c92fe5d0d0b3a6";
static const char* kDm1Root = "/tmp/firestaff-test-dm1-required-popup";
static const int kDm1GameIndex = 0;

static void seed_dm1_required_state(M12_StartupMenuState* state,
                                    int graphicsMatched,
                                    int dungeonMatched,
                                    int optionalCandidateFound) {
    M12_AssetVersionStatus* version;
    M12_AssetRequiredFileStatus* graphics;
    M12_AssetRequiredFileStatus* dungeon;

    M12_StartupMenu_InitWithDataDir(state, kDm1Root, NULL);

    state->assetStatus.dm1Available = graphicsMatched && dungeonMatched ? 1 : 0;
    state->assetStatus.csbAvailable = 0;
    state->assetStatus.dm2Available = 0;
    state->assetStatus.nexusAvailable = 0;
    state->assetStatus.theronAvailable = 0;
    state->assetStatus.originalFileCandidateFound = optionalCandidateFound;
    state->assetStatus.v22_modern_assets_installed = 0;

    state->entries[kDm1GameIndex].title = kDm1Title;
    state->entries[kDm1GameIndex].gameId = kDm1GameId;
    state->entries[kDm1GameIndex].kind = M12_MENU_ENTRY_GAME;
    state->entries[kDm1GameIndex].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[kDm1GameIndex].available =
        M12_AssetStatus_GameAvailable(&state->assetStatus, kDm1GameId);

    version = &state->assetStatus.versions[kDm1GameIndex][0];
    memset(version, 0, sizeof(*version));
    version->gameId = kDm1GameId;
    version->versionId = kDm1VersionId;
    version->label = kDm1VersionLabel;
    version->shortLabel = kDm1VersionShortLabel;
    version->matched = graphicsMatched ? 1 : 0;
    if (graphicsMatched) {
        snprintf(version->matchedPath, sizeof(version->matchedPath),
                 "%s/dm1/GRAPHICS.DAT", kDm1Root);
        snprintf(version->matchedMd5, sizeof(version->matchedMd5),
                 "%s", kDm1GraphicsMd5);
    }

    state->assetStatus.requiredFileCounts[kDm1GameIndex] = 2U;

    graphics = &state->assetStatus.requiredFiles[kDm1GameIndex][0];
    memset(graphics, 0, sizeof(*graphics));
    graphics->gameId = kDm1GameId;
    graphics->roleId = "graphics";
    graphics->label = "GRAPHICS.DAT";
    graphics->required = 1;
    graphics->matched = graphicsMatched ? 1 : 0;
    if (graphicsMatched) {
        snprintf(graphics->matchedPath, sizeof(graphics->matchedPath),
                 "%s/dm1/GRAPHICS.DAT", kDm1Root);
        snprintf(graphics->matchedHash, sizeof(graphics->matchedHash),
                 "%s", kDm1GraphicsMd5);
    }

    dungeon = &state->assetStatus.requiredFiles[kDm1GameIndex][1];
    memset(dungeon, 0, sizeof(*dungeon));
    dungeon->gameId = kDm1GameId;
    dungeon->roleId = "dungeon";
    dungeon->label = "DUNGEON.DAT";
    dungeon->required = 1;
    dungeon->matched = dungeonMatched ? 1 : 0;
    if (dungeonMatched) {
        snprintf(dungeon->matchedPath, sizeof(dungeon->matchedPath),
                 "%s/dm1/DUNGEON.DAT", kDm1Root);
        snprintf(dungeon->matchedHash, sizeof(dungeon->matchedHash),
                 "%s", kDm1DungeonMd5);
    }

    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->gameOptions[kDm1GameIndex].versionIndex = 0;
    state->gameOptions[kDm1GameIndex].presentationModeIndex =
        M12_PRESENTATION_V1_ORIGINAL;
    state->activatedIndex = kDm1GameIndex;
    state->selectedIndex = kDm1GameIndex;
    state->view = M12_MENU_VIEW_GAME_OPTIONS;
    state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
}

static void check_dm1_missing_required_popup(int graphicsMatched,
                                             int dungeonMatched,
                                             const char* expectedMissingA,
                                             const char* expectedMissingB,
                                             const char* expectedAbsent) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    seed_dm1_required_state(&state, graphicsMatched, dungeonMatched, 1);

    CHECK(M12_AssetStatus_GameHasCompleteHashSet(kDm1GameId) == 1);
    CHECK(M12_AssetStatus_GameRequiredFileCount(kDm1GameId) == 2U);
    CHECK(M12_AssetStatus_GameVerifiedFileCount(kDm1GameId) == 2U);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, kDm1GameId) == 0);
    CHECK(M12_AssetStatus_HasOriginalFileCandidate(&state.assetStatus) == 1);
    CHECK(state.entries[kDm1GameIndex].available == 0);

    CHECK(M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, kDm1GameId) == 2U);
    graphics = M12_AssetStatus_GetRequiredFile(&state.assetStatus, kDm1GameId, 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&state.assetStatus, kDm1GameId, 1U);
    CHECK(graphics && graphics->label && strcmp(graphics->label, "GRAPHICS.DAT") == 0);
    CHECK(dungeon && dungeon->label && strcmp(dungeon->label, "DUNGEON.DAT") == 0);
    CHECK(graphics && graphics->required == 1);
    CHECK(dungeon && dungeon->required == 1);
    CHECK(graphics && graphics->matched == (graphicsMatched ? 1 : 0));
    CHECK(dungeon && dungeon->matched == (dungeonMatched ? 1 : 0));

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageIsMissingGameData == 1);
    CHECK(strcmp(state.messageGameId, kDm1GameId) == 0);
    CHECK(state.messageLine1 && strstr(state.messageLine1, "DM1") != NULL);
    CHECK(state.messageLine1 && strstr(state.messageLine1, "GAME DATA") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, expectedMissingA) != NULL);
    if (expectedMissingB) {
        CHECK(state.messageLine2 && strstr(state.messageLine2, expectedMissingB) != NULL);
    }
    if (expectedAbsent) {
        CHECK(state.messageLine2 && strstr(state.messageLine2, expectedAbsent) == NULL);
    }
    CHECK(state.messageLine2 && strstr(state.messageLine2, "TITLE") == NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "INTRO") == NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "FTL") == NULL);
    CHECK(state.messageLine3 && strstr(state.messageLine3, "DATA DIR:") != NULL);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);
    CHECK(intent.gameId && strcmp(intent.gameId, kDm1GameId) == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, kDm1VersionId) == 0);
}

static void check_dm1_required_complete_launches_with_optional_candidate(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;

    seed_dm1_required_state(&state, 1, 1, 1);

    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, kDm1GameId) == 1);
    CHECK(M12_AssetStatus_HasOriginalFileCandidate(&state.assetStatus) == 1);
    CHECK(state.entries[kDm1GameIndex].available == 1);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 1);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageIsMissingGameData == 0);
    CHECK(state.messageGameId[0] == '\0');
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "READY TO LAUNCH") == 0);
    CHECK(state.messageLine2 && strcmp(state.messageLine2, kDm1Title) == 0);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 1);
    CHECK(intent.gameId && strcmp(intent.gameId, kDm1GameId) == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, kDm1VersionId) == 0);
    CHECK(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL);
}

static int isolate_home(void) {
#ifdef _WIN32
    char path[256];
    snprintf(path, sizeof(path), ".\\firestaff_dm1_required_popup_home_%lu",
             (unsigned long)rand());
    if (MKDIR(path) != 0) {
        return 0;
    }
    return _putenv_s("HOME", path) == 0 && _putenv_s("USERPROFILE", path) == 0;
#else
    char path[] = "/tmp/firestaff_dm1_required_popup_home_XXXXXX";
    char* made = mkdtemp(path);
    if (!made) {
        return 0;
    }
    return setenv("HOME", made, 1) == 0;
#endif
}

int main(void) {
    CHECK(isolate_home());
    check_dm1_missing_required_popup(0, 1, "GRAPHICS.DAT", NULL, "DUNGEON.DAT");
    check_dm1_missing_required_popup(1, 0, "DUNGEON.DAT", NULL, "GRAPHICS.DAT");
    check_dm1_missing_required_popup(0, 0, "GRAPHICS.DAT", "DUNGEON.DAT", NULL);
    check_dm1_required_complete_launches_with_optional_candidate();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: DM1 V1 popup gate blocks missing GRAPHICS/DUNGEON even when optional candidates exist");
    return 0;
}
