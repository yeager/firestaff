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

enum {
    DM2_GAME_INDEX = 2
};

static int failures;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static const char kDm2GraphicsPayload[] =
    "Firestaff synthetic DM2 missing-GRAPHICS profile gate GRAPHICS fixture\n";
static const char kDm2DungeonPayload[] =
    "Firestaff synthetic DM2 missing-GRAPHICS profile gate DUNGEON fixture\n";
static const char kDm2WrongGraphicsPayload[] =
    "Firestaff synthetic DM2 missing-GRAPHICS profile gate wrong GRAPHICS fixture\n";
static const char kDm2Pc98DemoGraphicsPayload[] =
    "Firestaff synthetic DM2 PC-9801 demo classification GRAPHICS fixture\n";

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
    int rc = snprintf(out, outSize, ".\\firestaff_dm2_missing_graphics_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir_if_needed(out);
#else
    char templatePath[] = "/tmp/firestaff-dm2-missing-graphics-XXXXXX";
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
    size_t count = M12_AssetStatus_GetRequiredFileCount(status, "dm2");
    for (i = 0U; i < count; ++i) {
        const M12_AssetRequiredFileStatus* file =
            M12_AssetStatus_GetRequiredFile(status, "dm2", i);
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
           write_plain_file(graphicsPath, kDm2GraphicsPayload) &&
           write_plain_file(dungeonPath, kDm2DungeonPayload) &&
           m12_file_md5_hex(graphicsPath, graphicsMd5) &&
           m12_file_md5_hex(dungeonPath, dungeonMd5);
}

static void write_dungeon_only_fixture(const char* root) {
    char dungeonPath[512];
    CHECK(join_path(dungeonPath, sizeof(dungeonPath), root, "renamed-dungeon.asset"));
    CHECK(write_plain_file(dungeonPath, kDm2DungeonPayload));
}

static void write_wrong_graphics_fixture(const char* root) {
    char graphicsPath[512];
    char dungeonPath[512];
    CHECK(join_path(graphicsPath, sizeof(graphicsPath), root, "GRAPHICS.DAT"));
    CHECK(join_path(dungeonPath, sizeof(dungeonPath), root, "renamed-dungeon.asset"));
    CHECK(write_plain_file(graphicsPath, kDm2WrongGraphicsPayload));
    CHECK(write_plain_file(dungeonPath, kDm2DungeonPayload));
}

static int write_pc98_demo_plus_pc_dungeon_fixture(
    const char* root,
    char demoGraphicsMd5[M12_ASSET_MD5_CAPACITY]) {
    char graphicsPath[512];
    char dungeonPath[512];
    return join_path(graphicsPath, sizeof(graphicsPath), root, "GRAPHICS.DAT") &&
           join_path(dungeonPath, sizeof(dungeonPath), root, "DUNGEON.DAT") &&
           write_plain_file(graphicsPath, kDm2Pc98DemoGraphicsPayload) &&
           write_plain_file(dungeonPath, kDm2DungeonPayload) &&
           m12_file_md5_hex(graphicsPath, demoGraphicsMd5);
}

static void select_dm2_launch_row(M12_StartupMenuState* state) {
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->activatedIndex = DM2_GAME_INDEX;
    state->selectedIndex = DM2_GAME_INDEX;
    state->gameOptions[DM2_GAME_INDEX].versionIndex = 0;
    state->gameOptions[DM2_GAME_INDEX].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->view = M12_MENU_VIEW_GAME_OPTIONS;
    state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
}

static void check_dm2_pc98_demo_classifies_without_satisfying_launch_graphics(
    const char* demoRoot,
    const char* demoGraphicsMd5,
    const char* dungeonMd5) {
    M12_AssetStatus status;
    const M12_AssetVersionStatus* pcEnglish;
    const M12_AssetVersionStatus* pc98Demo;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    int pc98Index = M12_AssetStatus_FindVersionIndex("dm2", "pc98-ja-demo");

    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, dungeonMd5);
    M12_AssetStatus_TestSetDm2Pc98DemoSyntheticHash(demoGraphicsMd5);
    M12_AssetStatus_Scan(&status, demoRoot);

    pcEnglish = M12_AssetStatus_GetVersion(&status, "dm2", 0U);
    pc98Demo = pc98Index >= 0
        ? M12_AssetStatus_GetVersion(&status, "dm2", (size_t)pc98Index)
        : NULL;
    graphics = required_file_by_role(&status, "graphics");
    dungeon = required_file_by_role(&status, "dungeon");

    CHECK(pc98Index == 3);
    CHECK(M12_AssetStatus_GetVersionCount("dm2") == 4U);
    CHECK(M12_AssetStatus_GameKnownHashCount("dm2") == 4U);
    CHECK(M12_AssetStatus_GameRequiredFileCount("dm2") == 2U);
    CHECK(M12_AssetStatus_GameAvailable(&status, "dm2") == 0);

    CHECK(pcEnglish && pcEnglish->versionId &&
          strcmp(pcEnglish->versionId, "pc-en") == 0);
    CHECK(pcEnglish && pcEnglish->matched == 0);
    CHECK(pc98Demo && pc98Demo->versionId &&
          strcmp(pc98Demo->versionId, "pc98-ja-demo") == 0);
    CHECK(pc98Demo && pc98Demo->label &&
          strcmp(pc98Demo->label, "PC-9801 Japanese Demo") == 0);
    CHECK(pc98Demo && pc98Demo->matched == 1);
    CHECK(pc98Demo && strcmp(pc98Demo->matchedMd5, demoGraphicsMd5) == 0);
    CHECK(pc98Demo && strstr(pc98Demo->matchedPath, "GRAPHICS.DAT") != NULL);

    CHECK(graphics && graphics->matched == 0);
    CHECK(graphics && graphics->matchedHash[0] == '\0');
    CHECK(dungeon && dungeon->matched == 1);
    CHECK(dungeon && strcmp(dungeon->matchedHash, dungeonMd5) == 0);
}

static void check_dm2_matched_dungeon_unmatched_graphics_scan_blocks_availability(
    const char* partialRoot,
    const char* graphicsMd5,
    const char* dungeonMd5) {
    M12_AssetStatus status;
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    M12_AssetStatus_TestSetDm2SyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_Scan(&status, partialRoot);

    version = M12_AssetStatus_GetVersion(&status, "dm2", 0U);
    graphics = required_file_by_role(&status, "graphics");
    dungeon = required_file_by_role(&status, "dungeon");

    CHECK(M12_AssetStatus_FindVersionIndex("dm2", "pc-en") == 0);
    CHECK(M12_AssetStatus_GameHasCompleteHashSet("dm2") == 1);
    CHECK(M12_AssetStatus_GameRequiredFileCount("dm2") == 2U);
    CHECK(M12_AssetStatus_GameVerifiedFileCount("dm2") == 2U);
    CHECK(M12_AssetStatus_GameAvailable(&status, "dm2") == 0);

    CHECK(version && version->gameId && strcmp(version->gameId, "dm2") == 0);
    CHECK(version && version->versionId && strcmp(version->versionId, "pc-en") == 0);
    CHECK(version && version->label && strcmp(version->label, "PC English") == 0);
    CHECK(version && version->matched == 0);
    CHECK(version && version->matchedMd5[0] == '\0');

    CHECK(M12_AssetStatus_GetRequiredFileCount(&status, "dm2") == 2U);
    CHECK(graphics && graphics->matched == 0);
    CHECK(graphics && graphics->label && strcmp(graphics->label, "GRAPHICS.DAT") == 0);
    CHECK(graphics && graphics->matchedPath[0] == '\0');
    CHECK(dungeon && dungeon->matched == 1);
    CHECK(dungeon && dungeon->label && strcmp(dungeon->label, "DUNGEON.DAT") == 0);
    CHECK(dungeon && strcmp(dungeon->matchedHash, dungeonMd5) == 0);
    CHECK(dungeon && strstr(dungeon->matchedPath, "renamed-dungeon.asset") != NULL);
}

static void check_dm2_matched_dungeon_unmatched_graphics_launch_is_blocked(
    const char* partialRoot,
    const char* graphicsMd5,
    const char* dungeonMd5) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    const M12_MenuEntry* entry;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    M12_AssetStatus_TestSetDm2SyntheticHashes(graphicsMd5, dungeonMd5);
    M12_StartupMenu_InitWithDataDir(&menu, partialRoot, "dm2");
    select_dm2_launch_row(&menu);

    entry = M12_StartupMenu_GetEntry(&menu, DM2_GAME_INDEX);
    graphics = required_file_by_role(&menu.assetStatus, "graphics");
    dungeon = required_file_by_role(&menu.assetStatus, "dungeon");

    CHECK(entry && entry->gameId && strcmp(entry->gameId, "dm2") == 0);
    CHECK(entry && entry->available == 0);
    CHECK(M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm2") == 0);
    CHECK(graphics && graphics->matched == 0);
    CHECK(dungeon && dungeon->matched == 1);

    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_ACCEPT);
    CHECK(menu.launchRequested == 0);
    CHECK(menu.quickResumeLaunchRequested == 0);
    CHECK(menu.view == M12_MENU_VIEW_MESSAGE);
    CHECK(menu.messageLine1 && strstr(menu.messageLine1, "DM2") != NULL);
    CHECK(menu.messageLine2 && strstr(menu.messageLine2, "GRAPHICS.DAT") != NULL);
    CHECK(menu.messageLine2 && strstr(menu.messageLine2, "DUNGEON.DAT") == NULL);

    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 0);
    CHECK(intent.gameId && strcmp(intent.gameId, "dm2") == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, "pc-en") == 0);
    CHECK(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL);
    CHECK(intent.rendererBackendAvailable == 1);
    CHECK(intent.options.versionIndex == 0);
}

int main(void) {
    char home[512];
    char hashRoot[512];
    char dungeonOnlyRoot[512];
    char wrongGraphicsRoot[512];
    char pc98DemoRoot[512];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];
    char pc98DemoGraphicsMd5[M12_ASSET_MD5_CAPACITY];

    if (!make_isolated_home(home, sizeof(home)) ||
        !join_path(hashRoot, sizeof(hashRoot), home, "hash-source") ||
        !join_path(dungeonOnlyRoot, sizeof(dungeonOnlyRoot), home, "dungeon-only") ||
        !join_path(wrongGraphicsRoot, sizeof(wrongGraphicsRoot), home, "wrong-graphics") ||
        !join_path(pc98DemoRoot, sizeof(pc98DemoRoot), home, "pc98-demo") ||
        !make_dir_if_needed(hashRoot) ||
        !make_dir_if_needed(dungeonOnlyRoot) ||
        !make_dir_if_needed(wrongGraphicsRoot) ||
        !make_dir_if_needed(pc98DemoRoot) ||
        !test_setenv("HOME", home) ||
        !test_setenv("FIRESTAFF_DATA", "")) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    CHECK(scan_fixture_hashes(hashRoot, graphicsMd5, dungeonMd5));
    write_dungeon_only_fixture(dungeonOnlyRoot);
    write_wrong_graphics_fixture(wrongGraphicsRoot);
    CHECK(write_pc98_demo_plus_pc_dungeon_fixture(pc98DemoRoot,
                                                  pc98DemoGraphicsMd5));

    check_dm2_pc98_demo_classifies_without_satisfying_launch_graphics(
        pc98DemoRoot,
        pc98DemoGraphicsMd5,
        dungeonMd5);
    M12_AssetStatus_TestSetDm2Pc98DemoSyntheticHash(NULL);
    check_dm2_matched_dungeon_unmatched_graphics_scan_blocks_availability(
        dungeonOnlyRoot,
        graphicsMd5,
        dungeonMd5);
    check_dm2_matched_dungeon_unmatched_graphics_launch_is_blocked(
        dungeonOnlyRoot,
        graphicsMd5,
        dungeonMd5);
    check_dm2_matched_dungeon_unmatched_graphics_scan_blocks_availability(
        wrongGraphicsRoot,
        graphicsMd5,
        dungeonMd5);
    check_dm2_matched_dungeon_unmatched_graphics_launch_is_blocked(
        wrongGraphicsRoot,
        graphicsMd5,
        dungeonMd5);

    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2Pc98DemoSyntheticHash(NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: DM2 V1 launch/profile gate stays blocked with matched DUNGEON but missing or wrong GRAPHICS");
    return 0;
}
