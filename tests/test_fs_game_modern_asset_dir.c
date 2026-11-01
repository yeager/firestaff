/*
 * Verify V2.2 asset roots when the default Firestaff data directory is a
 * symlink to an external volume, plus the legacy mapping for custom roots.
 */

#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <unistd.h>
#endif

static int failures;

#define CHECK(expr, message) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, message); \
        ++failures; \
    } \
} while (0)

static int make_dir(const char* path) {
    return FSP_CreateDirectoryRecursive(path);
}

static int make_file(const char* path) {
    FILE* file = fopen(path, "wb");
    if (!file) return 0;
    fclose(file);
    return 1;
}

#ifndef _WIN32
static void remove_fixture(const char* root, const char* home,
                           const char* logical_firestaff,
                           const char* logical_data, const char* external_data,
                           const char* custom_data) {
    static const char* const games[] = {
        "dm1", "csb", "dm2", "nexus", "theron"
    };
    char path[FSP_PATH_MAX];
    char assets_root[FSP_PATH_MAX];
    char custom_parent[FSP_PATH_MAX];
    char custom_assets[FSP_PATH_MAX];
    char custom_game[FSP_PATH_MAX];
    size_t i;
    for (i = 0U; i < sizeof(games) / sizeof(games[0]); ++i) {
        char game[FSP_PATH_MAX];
        char edition[FSP_PATH_MAX];
        char assets[FSP_PATH_MAX];
        char modern[FSP_PATH_MAX];
        char manifest[FSP_PATH_MAX];
        FSP_JoinPath(game, sizeof(game), external_data, games[i]);
        FSP_JoinPath(edition, sizeof(edition), game, "edition");
        FSP_JoinPath(assets, sizeof(assets), logical_firestaff, "assets");
        FSP_JoinPath(assets, sizeof(assets), assets, games[i]);
        FSP_JoinPath(modern, sizeof(modern), assets, "modern");
        FSP_JoinPath(manifest, sizeof(manifest), modern,
                     "modern_asset_manifest.json");
        unlink(manifest);
        rmdir(modern);
        rmdir(assets);
        rmdir(edition);
        rmdir(game);
    }
    FSP_JoinPath(path, sizeof(path), external_data, "dm1");
    rmdir(path);
    FSP_JoinPath(path, sizeof(path), external_data, "csb");
    rmdir(path);
    FSP_JoinPath(path, sizeof(path), external_data, "dm2");
    rmdir(path);
    FSP_JoinPath(path, sizeof(path), external_data, "nexus");
    rmdir(path);
    FSP_JoinPath(path, sizeof(path), external_data, "theron");
    rmdir(path);
    rmdir(external_data);
    unlink(logical_data);
    FSP_JoinPath(assets_root, sizeof(assets_root), logical_firestaff, "assets");
    rmdir(assets_root);
    rmdir(logical_firestaff);
    rmdir(home);
    FSP_ParentDir(custom_parent, sizeof(custom_parent), custom_data);
    rmdir(custom_data);
    FSP_JoinPath(custom_assets, sizeof(custom_assets), custom_parent, "assets");
    FSP_JoinPath(custom_game, sizeof(custom_game), custom_assets, "dm1");
    FSP_JoinPath(path, sizeof(path), custom_game, "modern");
    rmdir(path);
    rmdir(custom_game);
    rmdir(custom_assets);
    rmdir(custom_parent);
    rmdir(root);
}
#endif

int main(void) {
#ifdef _WIN32
    puts("SKIP: symlinked data-root fixture is POSIX-only");
    return 0;
#else
    static const char* const games[] = {
        "dm1", "csb", "dm2", "nexus", "theron"
    };
    char cwd[FSP_PATH_MAX];
    char root[FSP_PATH_MAX];
    char home[FSP_PATH_MAX];
    char logical_firestaff[FSP_PATH_MAX];
    char logical_data[FSP_PATH_MAX];
    char external_data[FSP_PATH_MAX];
    char custom_data[FSP_PATH_MAX];
    char custom_dm1[FSP_PATH_MAX];
    char expected[FSP_PATH_MAX];
    char actual[FSP_PATH_MAX];
    size_t i;

    CHECK(getcwd(cwd, sizeof(cwd)) != NULL, "get current test directory");
    snprintf(root, sizeof(root), "%s/v22-modern-root-%ld", cwd,
             (long)getpid());
    FSP_JoinPath(home, sizeof(home), root, "home");
    FSP_JoinPath(logical_firestaff, sizeof(logical_firestaff), home,
                 ".firestaff");
    FSP_JoinPath(logical_data, sizeof(logical_data), logical_firestaff, "data");
    FSP_JoinPath(external_data, sizeof(external_data), root, "external-data");
    FSP_JoinPath(custom_data, sizeof(custom_data), root, "custom/data");
    FSP_JoinPath(custom_dm1, sizeof(custom_dm1), custom_data, "dm1");

    CHECK(make_dir(logical_firestaff), "create logical Firestaff profile");
    CHECK(make_dir(external_data), "create external data root");
    CHECK(make_dir(custom_dm1), "create custom scratch data root");
    CHECK(symlink(external_data, logical_data) == 0,
          "link default data directory to external volume");

    CHECK(FSP_SetEnv("HOME", home, 1) == 0, "set isolated HOME");
    CHECK(FSP_UnsetEnv("FIRESTAFF_ORIGINALS_DIR") == 0,
          "clear originals override");
    for (i = 0U; i < sizeof(games) / sizeof(games[0]); ++i) {
        char physical_game_dir[FSP_PATH_MAX];
        char expected_game_root[FSP_PATH_MAX];
        char manifest[FSP_PATH_MAX];
        FSP_JoinPath(physical_game_dir, sizeof(physical_game_dir),
                     external_data, games[i]);
        FSP_JoinPath(physical_game_dir, sizeof(physical_game_dir),
                     physical_game_dir, "edition");
        CHECK(make_dir(physical_game_dir), "create physical game data dir");
        FSP_JoinPath(expected_game_root, sizeof(expected_game_root),
                     logical_firestaff, "assets");
        FSP_JoinPath(expected_game_root, sizeof(expected_game_root),
                     expected_game_root, games[i]);
        FSP_JoinPath(expected, sizeof(expected), expected_game_root, "modern");
        FSP_JoinPath(manifest, sizeof(manifest), expected,
                     "modern_asset_manifest.json");
        CHECK(make_dir(expected), "create user-profile modern asset dir");
        CHECK(make_file(manifest), "write user-profile manifest fixture");
        CHECK(FSP_ResolveGameModernAssetDir(actual, sizeof(actual),
                                            physical_game_dir, games[i]) == 1,
              "resolve modern asset directory");
        CHECK(strcmp(actual, expected) == 0,
              "canonical external data resolves to logical user assets");
    }

    FSP_JoinPath(expected, sizeof(expected), root, "custom/assets/dm1/modern");
    CHECK(FSP_ResolveGameModernAssetDir(actual, sizeof(actual), custom_dm1,
                                        "dm1") == 1,
          "resolve custom scratch root");
    CHECK(strcmp(actual, expected) == 0,
          "unrelated custom roots keep the established sibling mapping");

    remove_fixture(root, home, logical_firestaff, logical_data,
                   external_data, custom_data);
    puts(failures ? "FAIL: game modern asset root resolution" :
                    "PASS: game modern asset root resolution");
    return failures ? 1 : 0;
#endif
}
