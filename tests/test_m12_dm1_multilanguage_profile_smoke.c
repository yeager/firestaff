#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

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
    return setenv(name, value ? value : "", 1) == 0;
}
#endif

static int failures;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static const char* kGraphicsPayload =
    "Firestaff synthetic M12 DM1 PC 3.4 multilanguage GRAPHICS profile fixture\n";
static const char* kDungeonPayload =
    "Firestaff synthetic M12 DM1 PC 3.4 multilanguage DUNGEON profile fixture\n";

static int make_dir(const char* path) {
    return MKDIR(path) == 0;
}

static int write_payload(const char* path, const char* payload) {
    FILE* fp;
    size_t size;
    if (!path || !payload) {
        return 0;
    }
    fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    size = strlen(payload);
    if (fwrite(payload, 1U, size, fp) != size) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_m12_dm1_ml_home_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir(out);
#else
    char templatePath[] = "/tmp/firestaff-m12-dm1-ml-home-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static int make_child_dir(char* out, size_t outSize,
                          const char* parent,
                          const char* child) {
    int rc = snprintf(out, outSize, "%s/%s", parent, child);
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir(out);
}

static const M12_AssetRequiredFileStatus* required_file_by_role(
    const M12_AssetStatus* status,
    const char* roleId) {
    size_t i;
    size_t count = M12_AssetStatus_GetRequiredFileCount(status, "dm1");
    for (i = 0U; i < count; ++i) {
        const M12_AssetRequiredFileStatus* file =
            M12_AssetStatus_GetRequiredFile(status, "dm1", i);
        if (file && file->roleId && strcmp(file->roleId, roleId) == 0) {
            return file;
        }
    }
    return NULL;
}

static void write_complete_fixture(const char* root,
                                   char graphicsMd5[M12_ASSET_MD5_CAPACITY],
                                   char dungeonMd5[M12_ASSET_MD5_CAPACITY]) {
    char graphicsPath[512];
    char dungeonPath[512];
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/multilanguage-graphics.fixture", root);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/multilanguage-dungeon.fixture", root);
    CHECK(write_payload(graphicsPath, kGraphicsPayload));
    CHECK(write_payload(dungeonPath, kDungeonPayload));
    CHECK(m12_file_md5_hex(graphicsPath, graphicsMd5));
    CHECK(m12_file_md5_hex(dungeonPath, dungeonMd5));
}

static void write_graphics_only_fixture(const char* root) {
    char graphicsPath[512];
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/multilanguage-graphics.fixture", root);
    CHECK(write_payload(graphicsPath, kGraphicsPayload));
}

static void check_complete_multilanguage_profile(const char* root,
                                                 const char* graphicsMd5,
                                                 const char* dungeonMd5) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const M12_MenuEntry* dm1Entry;
    const M12_AssetVersionStatus* english;
    const M12_AssetVersionStatus* multi;
    const M12_AssetVersionStatus* firstMatched;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_StartupMenu_InitWithDataDir(&state, root, NULL);

    dm1Entry = M12_StartupMenu_GetEntry(&state, 0);
    english = M12_AssetStatus_GetVersion(&state.assetStatus, "dm1", 0U);
    multi = M12_AssetStatus_GetVersion(&state.assetStatus, "dm1", 1U);
    firstMatched = M12_AssetStatus_GetFirstMatchedVersion(&state.assetStatus, "dm1");
    graphics = required_file_by_role(&state.assetStatus, "graphics");
    dungeon = required_file_by_role(&state.assetStatus, "dungeon");

    CHECK(state.view == M12_MENU_VIEW_MAIN);
    CHECK(dm1Entry && dm1Entry->gameId && strcmp(dm1Entry->gameId, "dm1") == 0);
    CHECK(dm1Entry && dm1Entry->available == 1);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 1);
    CHECK(strcmp(M12_StartupMenu_GetDataStatusValue(&state), "1 GAME READY") == 0);

    CHECK(M12_AssetStatus_FindVersionIndex("dm1", "pc34-multi") == 1);
    CHECK(english && english->matched == 0);
    CHECK(multi && multi->matched == 1);
    CHECK(multi && multi->versionId && strcmp(multi->versionId, "pc34-multi") == 0);
    CHECK(multi && multi->label && strcmp(multi->label, "PC 3.4 Multilanguage") == 0);
    CHECK(multi && multi->shortLabel && strcmp(multi->shortLabel, "PC 3.4 ML") == 0);
    CHECK(multi && strcmp(multi->matchedMd5, graphicsMd5) == 0);
    CHECK(firstMatched == multi);

    CHECK(graphics && graphics->matched == 1);
    CHECK(graphics && strcmp(graphics->matchedHash, graphicsMd5) == 0);
    CHECK(dungeon && dungeon->matched == 1);
    CHECK(dungeon && strcmp(dungeon->matchedHash, dungeonMd5) == 0);

    state.selectedIndex = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_GAME_OPTIONS);
    CHECK(state.activatedIndex == 0);

    state.gameOptSelectedRow = M12_GAME_OPT_ROW_VERSION;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    CHECK(state.gameOptions[0].versionIndex == 1);

    state.gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 1);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "READY TO LAUNCH") == 0);
    CHECK(state.messageLine2 && strcmp(state.messageLine2, "DUNGEON MASTER") == 0);

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 1);
    CHECK(intent.gameId && strcmp(intent.gameId, "dm1") == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, "pc34-multi") == 0);
    CHECK(intent.options.versionIndex == 1);
    CHECK(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL);
}

static void check_graphics_only_missing_copy(const char* root,
                                             const char* graphicsMd5,
                                             const char* dungeonMd5) {
    M12_StartupMenuState state;
    const M12_MenuEntry* dm1Entry;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_StartupMenu_InitWithDataDir(&state, root, NULL);

    dm1Entry = M12_StartupMenu_GetEntry(&state, 0);
    graphics = required_file_by_role(&state.assetStatus, "graphics");
    dungeon = required_file_by_role(&state.assetStatus, "dungeon");

    CHECK(state.view == M12_MENU_VIEW_MAIN);
    CHECK(dm1Entry && dm1Entry->available == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);
    CHECK(strcmp(M12_StartupMenu_GetDataStatusValue(&state), "FILES FOUND") == 0);
    CHECK(graphics && graphics->matched == 1);
    CHECK(dungeon && dungeon->matched == 0);

    state.selectedIndex = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.launchRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "DM1 GAME DATA NOT FOUND") == 0);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "DUNGEON.DAT") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "GRAPHICS.DAT") == NULL);
    CHECK(state.messageLine3 && strstr(state.messageLine3, "DATA DIR:") == state.messageLine3);
}

int main(void) {
    char home[512];
    char completeRoot[512];
    char graphicsOnlyRoot[512];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY] = "";
    char dungeonMd5[M12_ASSET_MD5_CAPACITY] = "";

    if (!make_isolated_home(home, sizeof(home)) ||
        !test_setenv("HOME", home) ||
        !test_setenv("USERPROFILE", home) ||
        !test_setenv("FIRESTAFF_DATA", "") ||
        !make_child_dir(completeRoot, sizeof(completeRoot), home, "complete") ||
        !make_child_dir(graphicsOnlyRoot, sizeof(graphicsOnlyRoot), home, "graphics-only")) {
        fprintf(stderr, "fixture setup failed\n");
        return 1;
    }

    write_complete_fixture(completeRoot, graphicsMd5, dungeonMd5);
    write_graphics_only_fixture(graphicsOnlyRoot);
    if (!failures) {
        check_complete_multilanguage_profile(completeRoot, graphicsMd5, dungeonMd5);
        check_graphics_only_missing_copy(graphicsOnlyRoot, graphicsMd5, dungeonMd5);
    }
    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: M12 DM1 PC 3.4 multilanguage profile scan, availability copy, and launch intent are hash-gated");
    return 0;
}
