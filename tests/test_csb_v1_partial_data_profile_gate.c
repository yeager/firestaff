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

enum {
    CSB_GAME_INDEX = 1
};

static const char kCsbGraphicsPayload[] =
    "Firestaff synthetic CSB partial-data GRAPHICS fixture\n";
static const char kCsbDungeonPayload[] =
    "Firestaff synthetic CSB partial-data DUNGEON fixture\n";
static const char kCsbWrongDungeonPayload[] =
    "Firestaff synthetic CSB partial-data wrong DUNGEON fixture\n";

static int make_dir_if_needed(const char* path) {
    return MKDIR(path) == 0;
}

static int join_path(char* out, size_t outSize,
                     const char* left, const char* right) {
    int rc = snprintf(out, outSize, "%s/%s", left, right);
    return rc > 0 && (size_t)rc < outSize;
}

static int write_plain_file(const char* path, const char* payload) {
    FILE* fp = fopen(path, "wb");
    size_t size = strlen(payload);
    if (!fp) {
        return 0;
    }
    if (fwrite(payload, 1U, size, fp) != size) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_csb_partial_gate_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir_if_needed(out);
#else
    char templatePath[] = "/tmp/firestaff-csb-partial-gate-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static const M12_AssetRequiredFileStatus* required_file_by_role(
    const M12_AssetStatus* status,
    const char* roleId) {
    size_t i;
    size_t count = M12_AssetStatus_GetRequiredFileCount(status, "csb");
    for (i = 0U; i < count; ++i) {
        const M12_AssetRequiredFileStatus* file =
            M12_AssetStatus_GetRequiredFile(status, "csb", i);
        if (file && file->roleId && strcmp(file->roleId, roleId) == 0) {
            return file;
        }
    }
    return NULL;
}

static int scan_fixture_hashes(const char* root,
                               char graphicsMd5[M12_ASSET_MD5_CAPACITY],
                               char dungeonMd5[M12_ASSET_MD5_CAPACITY]) {
    char graphicsPath[512];
    char dungeonPath[512];
    return join_path(graphicsPath, sizeof(graphicsPath), root, "GRAPHICS.DAT") &&
           join_path(dungeonPath, sizeof(dungeonPath), root, "DUNGEON.DAT") &&
           write_plain_file(graphicsPath, kCsbGraphicsPayload) &&
           write_plain_file(dungeonPath, kCsbDungeonPayload) &&
           m12_file_md5_hex(graphicsPath, graphicsMd5) &&
           m12_file_md5_hex(dungeonPath, dungeonMd5);
}

static void write_partial_fixture(const char* root) {
    char graphicsPath[512];
    char wrongDungeonPath[512];
    CHECK(join_path(graphicsPath, sizeof(graphicsPath), root, "renamed-graphics.asset"));
    CHECK(join_path(wrongDungeonPath, sizeof(wrongDungeonPath), root, "DUNGEON.DAT"));
    CHECK(write_plain_file(graphicsPath, kCsbGraphicsPayload));
    CHECK(write_plain_file(wrongDungeonPath, kCsbWrongDungeonPayload));
}

static void write_complete_fixture(const char* root) {
    char graphicsPath[512];
    char dungeonPath[512];
    CHECK(join_path(graphicsPath, sizeof(graphicsPath), root, "GRAPHICS.DAT"));
    CHECK(join_path(dungeonPath, sizeof(dungeonPath), root, "renamed-dungeon.asset"));
    CHECK(write_plain_file(graphicsPath, kCsbGraphicsPayload));
    CHECK(write_plain_file(dungeonPath, kCsbDungeonPayload));
}

static void select_csb_launch_row(M12_StartupMenuState* state) {
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->activatedIndex = CSB_GAME_INDEX;
    state->selectedIndex = CSB_GAME_INDEX;
    state->gameOptions[CSB_GAME_INDEX].versionIndex = 0;
    state->view = M12_MENU_VIEW_GAME_OPTIONS;
    state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
}

static void check_partial_scan_blocks_profile_and_launch(
    const char* partialRoot,
    const char* graphicsMd5,
    const char* dungeonMd5) {
    M12_AssetStatus status;
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    const M12_MenuEntry* entry;

    M12_AssetStatus_TestSetCsbSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_Scan(&status, partialRoot);

    version = M12_AssetStatus_GetVersion(&status, "csb", 0U);
    graphics = required_file_by_role(&status, "graphics");
    dungeon = required_file_by_role(&status, "dungeon");

    CHECK(M12_AssetStatus_GameAvailable(&status, "csb") == 0);
    CHECK(version && version->matched == 1);
    CHECK(version && version->versionId && strcmp(version->versionId, "pc34-en") == 0);
    CHECK(version && strcmp(version->matchedMd5, graphicsMd5) == 0);
    CHECK(M12_AssetStatus_GetRequiredFileCount(&status, "csb") == 2U);
    CHECK(graphics && graphics->matched == 1);
    CHECK(graphics && strcmp(graphics->matchedHash, graphicsMd5) == 0);
    CHECK(dungeon && dungeon->matched == 0);
    CHECK(dungeon && dungeon->label && strcmp(dungeon->label, "DUNGEON.DAT") == 0);

    M12_StartupMenu_InitWithDataDir(&menu, partialRoot, "csb");
    select_csb_launch_row(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, CSB_GAME_INDEX);
    CHECK(entry && entry->gameId && strcmp(entry->gameId, "csb") == 0);
    CHECK(entry && entry->available == 0);

    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_ACCEPT);
    CHECK(menu.launchRequested == 0);
    CHECK(menu.quickResumeLaunchRequested == 0);
    CHECK(menu.view == M12_MENU_VIEW_MESSAGE);
    CHECK(menu.messageLine2 && strstr(menu.messageLine2, "DUNGEON.DAT") != NULL);

    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 0);
    CHECK(intent.gameId && strcmp(intent.gameId, "csb") == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, "pc34-en") == 0);
}

static void check_complete_scan_allows_profile_launch(
    const char* completeRoot,
    const char* graphicsMd5,
    const char* dungeonMd5) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    const M12_MenuEntry* entry;

    M12_AssetStatus_TestSetCsbSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_StartupMenu_InitWithDataDir(&menu, completeRoot, "csb");
    select_csb_launch_row(&menu);

    graphics = required_file_by_role(&menu.assetStatus, "graphics");
    dungeon = required_file_by_role(&menu.assetStatus, "dungeon");
    entry = M12_StartupMenu_GetEntry(&menu, CSB_GAME_INDEX);

    CHECK(M12_AssetStatus_GameAvailable(&menu.assetStatus, "csb") == 1);
    CHECK(graphics && graphics->matched == 1);
    CHECK(dungeon && dungeon->matched == 1);
    CHECK(entry && entry->available == 1);

    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 1);
    CHECK(intent.gameId && strcmp(intent.gameId, "csb") == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, "pc34-en") == 0);
}

int main(void) {
    char home[512];
    char hashRoot[512];
    char partialRoot[512];
    char completeRoot[512];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];

    if (!make_isolated_home(home, sizeof(home)) ||
        !join_path(hashRoot, sizeof(hashRoot), home, "hash-source") ||
        !join_path(partialRoot, sizeof(partialRoot), home, "partial-data") ||
        !join_path(completeRoot, sizeof(completeRoot), home, "complete-data") ||
        !make_dir_if_needed(hashRoot) ||
        !make_dir_if_needed(partialRoot) ||
        !make_dir_if_needed(completeRoot) ||
        !test_setenv("HOME", home) ||
        !test_setenv("FIRESTAFF_DATA", "")) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    CHECK(scan_fixture_hashes(hashRoot, graphicsMd5, dungeonMd5));
    write_partial_fixture(partialRoot);
    write_complete_fixture(completeRoot);

    check_partial_scan_blocks_profile_and_launch(partialRoot,
                                                 graphicsMd5,
                                                 dungeonMd5);
    check_complete_scan_allows_profile_launch(completeRoot,
                                              graphicsMd5,
                                              dungeonMd5);

    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: CSB V1 synthetic partial data blocks profile launch until GRAPHICS and DUNGEON hashes both match");
    return 0;
}
