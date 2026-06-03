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

typedef struct {
    const char* gameId;
    const char* title;
    const char* versionId;
    const char* versionLabel;
    const char* versionShortLabel;
    const char* graphicsMd5;
    const char* dungeonMd5;
    int gameIndex;
} GraphicsOnlyFixture;

static const GraphicsOnlyFixture fixtures[] = {
    {"dm1", "DUNGEON MASTER", "pc34-en", "PC 3.4 English", "PC 3.4 EN",
     "fa6b1aa29e191418713bf2cda93d962e", "766450c940651fc021c92fe5d0d0b3a6", 0},
    {"csb", "CHAOS STRIKES BACK", "pc34-en", "PC 3.4 English", "PC 3.4 EN",
     "61fbfd56887c94adc26888a9491c6611", "6695d2acebce49f95db1d8f3a5c733de", 1},
    {"dm2", "DUNGEON MASTER II", "pc-en", "PC English", "PC EN",
     "25247ede4dabb6a71e5dabdfbcd5907d", "6caccd7875009e82fe2e28e7f6d6adc0", 2}
};

static void seed_graphics_only_state(M12_StartupMenuState* state,
                                     const GraphicsOnlyFixture* fixture) {
    M12_AssetVersionStatus* version;
    M12_AssetRequiredFileStatus* graphics;
    M12_AssetRequiredFileStatus* dungeon;

    M12_StartupMenu_InitWithDataDir(state, "/tmp/firestaff-test-no-assets", NULL);

    state->assetStatus.dm1Available = 0;
    state->assetStatus.csbAvailable = 0;
    state->assetStatus.dm2Available = 0;

    state->entries[fixture->gameIndex].title = fixture->title;
    state->entries[fixture->gameIndex].gameId = fixture->gameId;
    state->entries[fixture->gameIndex].kind = M12_MENU_ENTRY_GAME;
    state->entries[fixture->gameIndex].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;

    version = &state->assetStatus.versions[fixture->gameIndex][0];
    memset(version, 0, sizeof(*version));
    version->gameId = fixture->gameId;
    version->versionId = fixture->versionId;
    version->label = fixture->versionLabel;
    version->shortLabel = fixture->versionShortLabel;
    version->matched = 1;
    snprintf(version->matchedPath, sizeof(version->matchedPath),
             "/tmp/firestaff-test-no-assets/%s/GRAPHICS.DAT", fixture->gameId);
    snprintf(version->matchedMd5, sizeof(version->matchedMd5), "%s", fixture->graphicsMd5);

    state->assetStatus.requiredFileCounts[fixture->gameIndex] = 2U;

    graphics = &state->assetStatus.requiredFiles[fixture->gameIndex][0];
    memset(graphics, 0, sizeof(*graphics));
    graphics->gameId = fixture->gameId;
    graphics->roleId = "graphics";
    graphics->label = "GRAPHICS.DAT";
    graphics->required = 1;
    graphics->matched = 1;
    snprintf(graphics->matchedPath, sizeof(graphics->matchedPath),
             "/tmp/firestaff-test-no-assets/%s/GRAPHICS.DAT", fixture->gameId);
    snprintf(graphics->matchedHash, sizeof(graphics->matchedHash), "%s", fixture->graphicsMd5);

    dungeon = &state->assetStatus.requiredFiles[fixture->gameIndex][1];
    memset(dungeon, 0, sizeof(*dungeon));
    dungeon->gameId = fixture->gameId;
    dungeon->roleId = "dungeon";
    dungeon->label = "DUNGEON.DAT";
    dungeon->required = 1;
    dungeon->matched = 0;
    snprintf(dungeon->matchedHash, sizeof(dungeon->matchedHash), "%s", fixture->dungeonMd5);

    state->assetStatus.originalFileCandidateFound = 1;
    state->entries[fixture->gameIndex].available =
        M12_AssetStatus_GameAvailable(&state->assetStatus, fixture->gameId);
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->gameOptions[fixture->gameIndex].versionIndex = 0;
    state->activatedIndex = fixture->gameIndex;
    state->view = M12_MENU_VIEW_GAME_OPTIONS;
    state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
}

static void check_graphics_only_blocks_launch(const GraphicsOnlyFixture* fixture) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    seed_graphics_only_state(&state, fixture);

    CHECK(M12_AssetStatus_GameHasCompleteHashSet(fixture->gameId) == 1);
    CHECK(M12_AssetStatus_GameRequiredFileCount(fixture->gameId) == 2U);
    CHECK(M12_AssetStatus_GameVerifiedFileCount(fixture->gameId) == 2U);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, fixture->gameId) == 0);
    CHECK(state.entries[fixture->gameIndex].available == 0);

    CHECK(M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, fixture->gameId) == 2U);
    graphics = M12_AssetStatus_GetRequiredFile(&state.assetStatus, fixture->gameId, 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&state.assetStatus, fixture->gameId, 1U);
    CHECK(graphics && graphics->matched == 1);
    CHECK(dungeon && dungeon->matched == 0);
    CHECK(dungeon && dungeon->label && strcmp(dungeon->label, "DUNGEON.DAT") == 0);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "DUNGEON.DAT") != NULL);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);
    CHECK(intent.gameId && strcmp(intent.gameId, fixture->gameId) == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, fixture->versionId) == 0);
}

static int isolate_home(void) {
#ifdef _WIN32
    char path[256];
    snprintf(path, sizeof(path), ".\\firestaff_missing_dungeon_home_%lu", (unsigned long)rand());
    if (MKDIR(path) != 0) {
        return 0;
    }
    return _putenv_s("HOME", path) == 0 && _putenv_s("USERPROFILE", path) == 0;
#else
    char path[] = "/tmp/firestaff_missing_dungeon_home_XXXXXX";
    char* made = mkdtemp(path);
    if (!made) {
        return 0;
    }
    return setenv("HOME", made, 1) == 0;
#endif
}

int main(void) {
    size_t i;
    CHECK(isolate_home());
    for (i = 0U; i < sizeof(fixtures) / sizeof(fixtures[0]); ++i) {
        check_graphics_only_blocks_launch(&fixtures[i]);
    }

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: DM1, CSB, and DM2 require DUNGEON.DAT even when GRAPHICS.DAT is matched");
    return 0;
}
