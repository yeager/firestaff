/*
 * Regression: CSB V1 launcher / profile gating must remain honest when only
 * the GRAPHICS file is present. Specifically, the CSB version identity
 * (versionId, label, shortLabel, matchedMd5) must be preserved from the
 * matched GRAPHICS hash, but the game must still be marked unavailable and
 * the launch must be blocked because DUNGEON.DAT is missing.
 *
 * This is the "version/profile identity remains honest for partial CSB
 * data" branch of the launcher/profile gating contract:
 *   - The user can see which CSB variant the matched GRAPHICS identifies
 *     (so the menu can show e.g. "PC 3.4 English" honestly).
 *   - But the overall game availability stays 0 until DUNGEON.DAT also
 *     matches a known hash.
 *   - A launch attempt must produce a message that names the missing
 *     DUNGEON.DAT, and the launch intent must be invalid.
 *
 * Synthetic state only — no copyrighted game data is required.
 *
 * Source evidence: ReDMCSB ENTRANCE.C F0806 (CSB entrance / load
 * boundary) and LOADSAVE.C F0435 (new-game load boundary).
 */
#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include "asset_status_m12.h"
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

#define CSB_GAME_INDEX 1  /* DM1=0, CSB=1, DM2=2, Nexus=3, Theron=4 */
#define CSB_PC34_EN_VERSION_ID   "pc34-en"
#define CSB_PC34_EN_VERSION_HASH "61fbfd56887c94adc26888a9491c6611"
#define CSB_DUNGEON_HASH         "6695d2acebce49f95db1d8f3a5c733de"

static void seed_csb_graphics_only_state(M12_StartupMenuState* state) {
    M12_AssetVersionStatus* version;
    M12_AssetRequiredFileStatus* graphics;
    M12_AssetRequiredFileStatus* dungeon;

    M12_StartupMenu_InitWithDataDir(state, "/tmp/firestaff-test-no-assets", NULL);

    state->assetStatus.dm1Available = 0;
    state->assetStatus.csbAvailable = 0;
    state->assetStatus.dm2Available = 0;

    state->entries[CSB_GAME_INDEX].title = "CHAOS STRIKES BACK";
    state->entries[CSB_GAME_INDEX].gameId = "csb";
    state->entries[CSB_GAME_INDEX].kind = M12_MENU_ENTRY_GAME;
    state->entries[CSB_GAME_INDEX].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;

    /* Synthesize: only the CSB PC 3.4 English GRAPHICS hash is matched.
     * The matched version identity is preserved (versionId, label,
     * shortLabel, matchedMd5) but DUNGEON.DAT is absent. */
    version = &state->assetStatus.versions[CSB_GAME_INDEX][0];
    memset(version, 0, sizeof(*version));
    version->gameId = "csb";
    version->versionId = CSB_PC34_EN_VERSION_ID;
    version->label = "PC 3.4 English";
    version->shortLabel = "PC 3.4 EN";
    version->matched = 1;
    snprintf(version->matchedPath, sizeof(version->matchedPath),
             "/tmp/firestaff-test-no-assets/csb/GRAPHICS.DAT");
    snprintf(version->matchedMd5, sizeof(version->matchedMd5), "%s",
             CSB_PC34_EN_VERSION_HASH);

    state->assetStatus.requiredFileCounts[CSB_GAME_INDEX] = 2U;

    /* Required: CSB GRAPHICS — present and matched (any CSB version's
     * GRAPHICS hash is accepted for the "graphics" role). */
    graphics = &state->assetStatus.requiredFiles[CSB_GAME_INDEX][0];
    memset(graphics, 0, sizeof(*graphics));
    graphics->gameId = "csb";
    graphics->roleId = "graphics";
    graphics->label = "GRAPHICS.DAT";
    graphics->required = 1;
    graphics->matched = 1;
    snprintf(graphics->matchedPath, sizeof(graphics->matchedPath),
             "/tmp/firestaff-test-no-assets/csb/GRAPHICS.DAT");
    snprintf(graphics->matchedHash, sizeof(graphics->matchedHash), "%s",
             CSB_PC34_EN_VERSION_HASH);

    /* Required: CSB DUNGEON — the known hash is recorded so the launcher
     * can name the missing file, but matched=0 keeps the launch blocked. */
    dungeon = &state->assetStatus.requiredFiles[CSB_GAME_INDEX][1];
    memset(dungeon, 0, sizeof(*dungeon));
    dungeon->gameId = "csb";
    dungeon->roleId = "dungeon";
    dungeon->label = "DUNGEON.DAT";
    dungeon->required = 1;
    dungeon->matched = 0;
    snprintf(dungeon->matchedHash, sizeof(dungeon->matchedHash), "%s",
             CSB_DUNGEON_HASH);

    state->assetStatus.originalFileCandidateFound = 1;
    state->entries[CSB_GAME_INDEX].available =
        M12_AssetStatus_GameAvailable(&state->assetStatus, "csb");
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->gameOptions[CSB_GAME_INDEX].versionIndex = 0;
    state->activatedIndex = CSB_GAME_INDEX;
    state->view = M12_MENU_VIEW_GAME_OPTIONS;
    state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
}

static void check_csb_partial_data_keeps_identity_honest(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    const M12_MenuEntry* entry;

    seed_csb_graphics_only_state(&state);

    /* 1. The CSB version identity is preserved from the matched GRAPHICS.
     *    The user can see "PC 3.4 English" honestly in the menu even when
     *    DUNGEON.DAT is missing. */
    version = M12_AssetStatus_GetVersion(&state.assetStatus, "csb", 0U);
    CHECK(version != NULL);
    CHECK(version->matched == 1);
    CHECK(version->versionId != NULL &&
          strcmp(version->versionId, CSB_PC34_EN_VERSION_ID) == 0);
    CHECK(version->label != NULL &&
          strcmp(version->label, "PC 3.4 English") == 0);
    CHECK(version->shortLabel != NULL &&
          strcmp(version->shortLabel, "PC 3.4 EN") == 0);
    CHECK(strcmp(version->matchedMd5, CSB_PC34_EN_VERSION_HASH) == 0);

    /* 2. The required file accounting still reports GRAPHICS matched and
     *    DUNGEON missing, with the right labels. */
    CHECK(M12_AssetStatus_GetRequiredFileCount(&state.assetStatus, "csb") == 2U);
    graphics = M12_AssetStatus_GetRequiredFile(&state.assetStatus, "csb", 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&state.assetStatus, "csb", 1U);
    CHECK(graphics && graphics->matched == 1);
    CHECK(dungeon && dungeon->matched == 0);
    CHECK(dungeon && dungeon->label &&
          strcmp(dungeon->label, "DUNGEON.DAT") == 0);
    CHECK(dungeon && strcmp(dungeon->matchedHash, CSB_DUNGEON_HASH) == 0);

    /* 3. Overall CSB availability is blocked because DUNGEON is missing. */
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "csb") == 0);
    CHECK(state.assetStatus.csbAvailable == 0);
    entry = M12_StartupMenu_GetEntry(&state, CSB_GAME_INDEX);
    CHECK(entry && entry->available == 0);
    CHECK(entry && entry->gameId && strcmp(entry->gameId, "csb") == 0);
    CHECK(entry && entry->title && strcmp(entry->title, "CHAOS STRIKES BACK") == 0);

    /* 4. Pressing ACCEPT on the launch row must NOT request a runtime
     *    launch. The launcher must show a message that names the missing
     *    DUNGEON.DAT, so the user can act on the gap. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine2 != NULL &&
          strstr(state.messageLine2, "DUNGEON.DAT") != NULL);

    /* 5. The launch intent must be invalid (no runtime launch) but the
     *    gameId and versionId must remain honestly set to the matched CSB
     *    variant. Downstream diagnostics (boot probes, save scanners) can
     *    still attribute the failure to "csb pc34-en". */
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);
    CHECK(intent.gameId != NULL && strcmp(intent.gameId, "csb") == 0);
    CHECK(intent.versionId != NULL &&
          strcmp(intent.versionId, CSB_PC34_EN_VERSION_ID) == 0);
}

static int isolate_home(void) {
#ifdef _WIN32
    char path[256];
    snprintf(path, sizeof(path), ".\\firestaff_csb_partial_home_%lu",
             (unsigned long)rand());
    if (MKDIR(path) != 0) {
        return 0;
    }
    return _putenv_s("HOME", path) == 0 &&
           _putenv_s("USERPROFILE", path) == 0;
#else
    /* Use a deterministic but unique-enough temp path; tests run
     * sequentially and only read the synthetic state we seed. */
    if (MKDIR("/tmp/firestaff_csb_partial_home") != 0) {
        /* Directory may already exist from a prior run; that's fine. */
    }
    return setenv("HOME", "/tmp/firestaff_csb_partial_home", 1) == 0;
#endif
}

int main(void) {
    CHECK(isolate_home());

    check_csb_partial_data_keeps_identity_honest();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: CSB V1 partial data keeps version identity honest and blocks launch");
    puts("sourceEvidence=ReDMCSB ENTRANCE.C F0806 launch state and LOADSAVE.C F0435 new-game load boundary");
    return 0;
}
