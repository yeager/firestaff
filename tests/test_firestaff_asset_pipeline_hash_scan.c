#include "asset_status_m12.h"
#include "firestaff_asset_pipeline.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define TEST_GETPID() _getpid()
#define TEST_SEP "\\"
static int test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
#else
#include <unistd.h>
#define TEST_GETPID() getpid()
#define TEST_SEP "/"
static int test_setenv(const char* name, const char* value) {
    return setenv(name, value ? value : "", 1) == 0;
}
#endif

static int failures;

static void check_int(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int write_payload(const char* path, const char* payload) {
    FILE* fp = fopen(path, "wb");
    size_t n;
    if (!fp) return 0;
    n = strlen(payload);
    if (fwrite(payload, 1U, n, fp) != n) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int make_root(char* out, size_t outBytes) {
    int rc = snprintf(out,
                      outBytes,
                      "%s%sfirestaff-asset-pipeline-hash-%ld",
                      getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp",
                      TEST_SEP,
                      (long)TEST_GETPID());
    return rc > 0 && (size_t)rc < outBytes && FSP_CreateDirectoryRecursive(out);
}

static int write_renamed_pair(const char* root,
                              const char* dirName,
                              const char* graphicsPayload,
                              const char* dungeonPayload,
                              char graphicsMd5[M12_ASSET_MD5_CAPACITY],
                              char dungeonMd5[M12_ASSET_MD5_CAPACITY]) {
    char dir[FSP_PATH_MAX];
    char nested[FSP_PATH_MAX];
    char graphicsPath[FSP_PATH_MAX];
    char dungeonPath[FSP_PATH_MAX];
    if (!FSP_JoinPath(dir, sizeof(dir), root, dirName) ||
        !FSP_JoinPath(nested, sizeof(nested), dir, "renamed") ||
        !FSP_CreateDirectoryRecursive(nested) ||
        !FSP_JoinPath(graphicsPath, sizeof(graphicsPath), nested, "art.payload") ||
        !FSP_JoinPath(dungeonPath, sizeof(dungeonPath), nested, "map.payload") ||
        !write_payload(graphicsPath, graphicsPayload) ||
        !write_payload(dungeonPath, dungeonPayload) ||
        !m12_file_md5_hex(graphicsPath, graphicsMd5) ||
        !m12_file_md5_hex(dungeonPath, dungeonMd5)) {
        return 0;
    }
    return 1;
}

static void check_loaded_game(const char* root,
                              const char* gameId,
                              const char* expectedGraphics,
                              const char* expectedDungeon) {
    FS_AssetBundle bundle;
    memset(&bundle, 0, sizeof(bundle));
    check_int(fs_assets_load_game(&bundle, root, gameId) == 0,
              "asset pipeline loads renamed hash files");
    check_int(bundle.loaded == 1, "bundle is marked loaded");
    check_int(bundle.graphics_size == (int)strlen(expectedGraphics) &&
              memcmp(bundle.graphics_data,
                     expectedGraphics,
                     strlen(expectedGraphics)) == 0,
              "graphics bytes came from hash-discovered file");
    check_int(bundle.dungeon_size == (int)strlen(expectedDungeon) &&
              memcmp(bundle.dungeon_data,
                     expectedDungeon,
                     strlen(expectedDungeon)) == 0,
              "dungeon bytes came from hash-discovered file");
    fs_assets_free(&bundle);
}

int main(void) {
    char root[FSP_PATH_MAX];
    char dm1GraphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dm1DungeonMd5[M12_ASSET_MD5_CAPACITY];
    char csbGraphicsMd5[M12_ASSET_MD5_CAPACITY];
    char csbDungeonMd5[M12_ASSET_MD5_CAPACITY];
    char dm2GraphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dm2DungeonMd5[M12_ASSET_MD5_CAPACITY];

    check_int(make_root(root, sizeof(root)), "temp root created");
    check_int(test_setenv("HOME", root), "isolated HOME set");

    check_int(write_renamed_pair(root,
                                 "dm1-any-layout",
                                 "dm1 graphics bytes",
                                 "dm1 dungeon bytes",
                                 dm1GraphicsMd5,
                                 dm1DungeonMd5),
              "renamed DM1 pair written");
    check_int(write_renamed_pair(root,
                                 "csb-any-layout",
                                 "csb graphics bytes",
                                 "csb dungeon bytes",
                                 csbGraphicsMd5,
                                 csbDungeonMd5),
              "renamed CSB pair written");
    check_int(write_renamed_pair(root,
                                 "dm2-any-layout",
                                 "dm2 graphics bytes",
                                 "dm2 dungeon bytes",
                                 dm2GraphicsMd5,
                                 dm2DungeonMd5),
              "renamed DM2 pair written");

    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(dm1GraphicsMd5,
                                                         dm1DungeonMd5);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    check_loaded_game(root, "dm1", "dm1 graphics bytes", "dm1 dungeon bytes");

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetCsbSyntheticHashes(csbGraphicsMd5, csbDungeonMd5);
    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    check_loaded_game(root, "csb", "csb graphics bytes", "csb dungeon bytes");

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(dm2GraphicsMd5, dm2DungeonMd5);
    check_loaded_game(root, "dm2", "dm2 graphics bytes", "dm2 dungeon bytes");

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);

    if (failures) return 1;
    puts("ok: asset pipeline loads renamed required files by hash");
    return 0;
}
