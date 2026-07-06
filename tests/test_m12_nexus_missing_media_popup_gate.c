#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
static int test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0700)
static int test_setenv(const char* name, const char* value) {
    if (value) {
        return setenv(name, value, 1) == 0;
    }
    return unsetenv(name) == 0;
}
#endif

static int failures;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static int isolate_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_nexus_missing_media_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize || MKDIR(out) != 0) {
        return 0;
    }
    return test_setenv("HOME", out) && test_setenv("USERPROFILE", out);
#else
    char tmp[] = "/tmp/firestaff_nexus_missing_media_XXXXXX";
    char* made = mkdtemp(tmp);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return test_setenv("HOME", out);
#endif
}

static void seed_nexus_missing_media_state(M12_StartupMenuState* state,
                                           const char* dataRoot) {
    const int gi = 3;
    M12_AssetVersionStatus* version;
    M12_AssetRequiredFileStatus* required;

    M12_StartupMenu_InitWithDataDir(state, dataRoot, NULL);

    state->assetStatus.dm1Available = 0;
    state->assetStatus.csbAvailable = 0;
    state->assetStatus.dm2Available = 0;
    state->assetStatus.nexusAvailable = 0;
    state->assetStatus.theronAvailable = 0;
    state->assetStatus.originalFileCandidateFound = 1;

    state->entries[gi].title = "DUNGEON MASTER NEXUS";
    state->entries[gi].gameId = "nexus";
    state->entries[gi].kind = M12_MENU_ENTRY_GAME;
    state->entries[gi].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[gi].available = 0;

    version = &state->assetStatus.versions[gi][0];
    memset(version, 0, sizeof(*version));
    version->gameId = "nexus";
    version->versionId = "nexus-saturn-jp";
    version->label = "Saturn JP";
    version->shortLabel = "Saturn JP";
    version->matched = 0;

    state->assetStatus.requiredFileCounts[gi] = 1U;
    required = &state->assetStatus.requiredFiles[gi][0];
    memset(required, 0, sizeof(*required));
    required->gameId = "nexus";
    required->roleId = "data";
    required->label = "DM.BIN / Saturn data marker";
    required->required = 1;
    required->matched = 0;

    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->gameOptions[gi].versionIndex = 0;
    state->gameOptions[gi].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->activatedIndex = gi;
    state->selectedIndex = gi;
    state->view = M12_MENU_VIEW_GAME_OPTIONS;
    state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
}

static void check_nexus_missing_media_popup(void) {
    char home[M12_ASSET_DATA_DIR_CAPACITY];
    M12_StartupMenuState state;
    M12_LaunchIntent intent;

    CHECK(isolate_home(home, sizeof(home)));
    seed_nexus_missing_media_state(&state, home);

    CHECK(M12_AssetStatus_GameHasCompleteHashSet("nexus") == 1);
    CHECK(M12_AssetStatus_GameRequiredFileCount("nexus") == 1U);
    CHECK(M12_AssetStatus_GameVerifiedFileCount("nexus") == 1U);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "nexus") == 0);
    CHECK(M12_AssetStatus_HasOriginalFileCandidate(&state.assetStatus) == 1);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);

    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageIsMissingGameData == 1);
    CHECK(strcmp(state.messageGameId, "nexus") == 0);
    CHECK(state.messageLine1 && strstr(state.messageLine1, "NEXUS") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "ISO/BIN/CUE") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "DM.BIN") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "Saturn data marker") == NULL);
    CHECK(state.messageLine3 && strstr(state.messageLine3, "DATA DIR:") != NULL);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);
    CHECK(intent.gameId && strcmp(intent.gameId, "nexus") == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, "nexus-saturn-jp") == 0);
}

int main(void) {
    check_nexus_missing_media_popup();
    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: Nexus missing-media popup gives actionable ISO/BIN/CUE guidance");
    return 0;
}
