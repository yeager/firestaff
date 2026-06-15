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
 * DM1 V1 launcher/profile gating regression.
 *
 * Companion to test_m12_missing_dungeon_launch_blocker.c (which proves the
 * negative side: missing DUNGEON blocks launch). This test proves the
 * positive side: when the DM1 *required* hash set is complete (GRAPHICS
 * + DUNGEON), the V1 launcher must request a launch — and the launch
 * intent must validate even when optional non-gating assets (title
 * animation, intro, FTL logo, originalFileCandidate marker) are absent.
 *
 * The DM1 launch gate is intentionally narrow: only the required files
 * in g_requiredFiles (graphics + dungeon) block availability. The
 * regression this test guards against is someone accidentally widening
 * the gate to depend on title/intro/FTL existence, or on the
 * originalFileCandidate marker, and breaking the V1 original path
 * for users with just the two required .DAT files.
 *
 * Synthetic, no copyrighted data required.
 */
static const char* kDm1GameId = "dm1";
static const char* kDm1Title = "DUNGEON MASTER";
static const char* kDm1VersionId = "pc34-en";
static const char* kDm1VersionLabel = "PC 3.4 English";
static const char* kDm1VersionShortLabel = "PC 3.4 EN";
static const char* kDm1GraphicsMd5 = "fa6b1aa29e191418713bf2cda93d962e";
static const char* kDm1DungeonMd5 = "766450c940651fc021c92fe5d0d0b3a6";
static const int kDm1GameIndex = 0;

static void seed_dm1_v1_complete_required_state(M12_StartupMenuState* state) {
    M12_AssetVersionStatus* version;
    M12_AssetRequiredFileStatus* graphics;
    M12_AssetRequiredFileStatus* dungeon;

    M12_StartupMenu_InitWithDataDir(state, "/tmp/firestaff-test-dm1-required", NULL);

    /* The gating system is "required files matched -> game available".
     * The required set is {GRAPHICS.DAT, DUNGEON.DAT} for DM1; title,
     * intro, FTL logo, etc. are not part of the gate. We deliberately
     * leave originalFileCandidateFound=0 to model "no optional assets
     * present on disk" — this must not block V1 launch. */
    state->assetStatus.dm1Available = 1;
    state->assetStatus.csbAvailable = 0;
    state->assetStatus.dm2Available = 0;
    state->assetStatus.nexusAvailable = 0;
    state->assetStatus.theronAvailable = 0;
    state->assetStatus.originalFileCandidateFound = 0;
    state->assetStatus.v22_modern_assets_installed = 0;

    state->entries[kDm1GameIndex].title = kDm1Title;
    state->entries[kDm1GameIndex].gameId = kDm1GameId;
    state->entries[kDm1GameIndex].kind = M12_MENU_ENTRY_GAME;
    state->entries[kDm1GameIndex].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[kDm1GameIndex].available = 1;

    version = &state->assetStatus.versions[kDm1GameIndex][0];
    memset(version, 0, sizeof(*version));
    version->gameId = kDm1GameId;
    version->versionId = kDm1VersionId;
    version->label = kDm1VersionLabel;
    version->shortLabel = kDm1VersionShortLabel;
    version->matched = 1;
    snprintf(version->matchedPath, sizeof(version->matchedPath),
             "/tmp/firestaff-test-dm1-required/dm1/GRAPHICS.DAT");
    snprintf(version->matchedMd5, sizeof(version->matchedMd5), "%s", kDm1GraphicsMd5);

    state->assetStatus.requiredFileCounts[kDm1GameIndex] = 2U;

    graphics = &state->assetStatus.requiredFiles[kDm1GameIndex][0];
    memset(graphics, 0, sizeof(*graphics));
    graphics->gameId = kDm1GameId;
    graphics->roleId = "graphics";
    graphics->label = "GRAPHICS.DAT";
    graphics->required = 1;
    graphics->matched = 1;
    snprintf(graphics->matchedPath, sizeof(graphics->matchedPath),
             "/tmp/firestaff-test-dm1-required/dm1/GRAPHICS.DAT");
    snprintf(graphics->matchedHash, sizeof(graphics->matchedHash), "%s", kDm1GraphicsMd5);

    dungeon = &state->assetStatus.requiredFiles[kDm1GameIndex][1];
    memset(dungeon, 0, sizeof(*dungeon));
    dungeon->gameId = kDm1GameId;
    dungeon->roleId = "dungeon";
    dungeon->label = "DUNGEON.DAT";
    dungeon->required = 1;
    dungeon->matched = 1;
    snprintf(dungeon->matchedPath, sizeof(dungeon->matchedPath),
             "/tmp/firestaff-test-dm1-required/dm1/DUNGEON.DAT");
    snprintf(dungeon->matchedHash, sizeof(dungeon->matchedHash), "%s", kDm1DungeonMd5);

    /* V1 original path, software renderer, PC 3.4 English version, on
     * the launch row of the game options view. */
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->gameOptions[kDm1GameIndex].versionIndex = 0;
    state->gameOptions[kDm1GameIndex].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->activatedIndex = kDm1GameIndex;
    state->view = M12_MENU_VIEW_GAME_OPTIONS;
    state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
}

static void check_dm1_v1_required_complete_launches(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    seed_dm1_v1_complete_required_state(&state);

    /* Required hash-set completeness, mirroring the assertions in
     * test_m12_missing_dungeon_launch_blocker.c. */
    CHECK(M12_AssetStatus_GameHasCompleteHashSet(kDm1GameId) == 1);
    CHECK(M12_AssetStatus_GameRequiredFileCount(kDm1GameId) == 2U);
    CHECK(M12_AssetStatus_GameVerifiedFileCount(kDm1GameId) == 2U);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, kDm1GameId) == 1);
    CHECK(state.entries[kDm1GameIndex].available == 1);

    CHECK(M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, kDm1GameId) == 2U);
    graphics = M12_AssetStatus_GetRequiredFile(&state.assetStatus, kDm1GameId, 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&state.assetStatus, kDm1GameId, 1U);
    CHECK(graphics && graphics->matched == 1);
    CHECK(graphics && graphics->label && strcmp(graphics->label, "GRAPHICS.DAT") == 0);
    CHECK(dungeon && dungeon->matched == 1);
    CHECK(dungeon && dungeon->label && strcmp(dungeon->label, "DUNGEON.DAT") == 0);

    /* Optional assets (title, intro, FTL) are not part of the required
     * set — originalFileCandidateFound is intentionally 0 in this
     * fixture, and the gate must still let DM1 through. The launch
     * runtime decides how to render title/intro/FTL independently. */
    CHECK(M12_AssetStatus_HasOriginalFileCandidate(&state.assetStatus) == 0);

    /* Pressing ACCEPT on the launch row with both required files
     * matched must request a launch and surface the "READY TO LAUNCH"
     * message. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 1);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "READY TO LAUNCH") == 0);
    CHECK(state.messageLine2 && strcmp(state.messageLine2, kDm1Title) == 0);

    /* The resulting launch intent must validate for the V1 original
     * pipeline: V1 mode, software renderer, DM1 game id, PC 3.4 EN
     * version, no quick-resume save path, options carried through. */
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 1);
    CHECK(intent.gameId && strcmp(intent.gameId, kDm1GameId) == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, kDm1VersionId) == 0);
    CHECK(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL);
    CHECK(intent.rendererBackendAvailable == 1);
    CHECK(intent.savePath == NULL);
    CHECK(intent.options.versionIndex == 0);
    CHECK(intent.options.presentationModeIndex == M12_PRESENTATION_V1_ORIGINAL);
}

static int isolate_home(void) {
#ifdef _WIN32
    char path[256];
    snprintf(path, sizeof(path), ".\\firestaff_dm1_v1_required_home_%lu", (unsigned long)rand());
    if (MKDIR(path) != 0) {
        return 0;
    }
    return _putenv_s("HOME", path) == 0 && _putenv_s("USERPROFILE", path) == 0;
#else
    char path[] = "/tmp/firestaff_dm1_v1_required_home_XXXXXX";
    char* made = mkdtemp(path);
    if (!made) {
        return 0;
    }
    return setenv("HOME", made, 1) == 0;
#endif
}

int main(void) {
    CHECK(isolate_home());
    check_dm1_v1_required_complete_launches();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: DM1 V1 launcher launches when GRAPHICS+DUNGEON required hash set is complete, even with optional title/intro absent");
    return 0;
}
