#include "asset_status_m12.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
static int test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
#else
#include <unistd.h>
static int test_setenv(const char* name, const char* value) {
    if (value) {
        return setenv(name, value, 1) == 0;
    }
    return unsetenv(name) == 0;
}
#endif

static int failures;

static void check_int(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_asset_status_irrelevant_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return FSP_CreateDirectoryRecursive(out);
#else
    char templatePath[] = "/tmp/firestaff-asset-status-irrelevant-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static int write_text_file(const char* path, const char* text) {
    FILE* fp = fopen(path, "wb");
    size_t len;
    if (!fp || !text) {
        if (fp) {
            fclose(fp);
        }
        return 0;
    }
    len = strlen(text);
    if (fwrite(text, 1U, len, fp) != len) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_joined_text_file(const char* root,
                                  const char* relativePath,
                                  const char* text) {
    char path[M12_ASSET_DATA_DIR_CAPACITY];
    if (!FSP_JoinPath(path, sizeof(path), root, relativePath)) {
        return 0;
    }
    return write_text_file(path, text);
}

static int setup_irrelevant_data_root(const char* root) {
    char nested[M12_ASSET_DATA_DIR_CAPACITY];
    char misleadingDm1[M12_ASSET_DATA_DIR_CAPACITY];
    char misleadingDm2[M12_ASSET_DATA_DIR_CAPACITY];

    if (!FSP_CreateDirectoryRecursive(root)) {
        return 0;
    }
    if (!FSP_JoinPath(nested, sizeof(nested), root, "notes") ||
        !FSP_CreateDirectoryRecursive(nested) ||
        !FSP_JoinPath(misleadingDm1, sizeof(misleadingDm1), root, "dm1") ||
        !FSP_CreateDirectoryRecursive(misleadingDm1) ||
        !FSP_JoinPath(misleadingDm2, sizeof(misleadingDm2), root, "dm2") ||
        !FSP_CreateDirectoryRecursive(misleadingDm2)) {
        return 0;
    }
    return write_joined_text_file(root, "README.txt",
                                  "not Firestaff game data\n") &&
           write_joined_text_file(root, "notes/todo.txt",
                                  "ordinary user note\n") &&
           write_joined_text_file(root, "dm1/GRAPHICS.DAT",
                                  "wrong bytes despite a familiar file name\n") &&
           write_joined_text_file(root, "dm1/DUNGEON.DAT",
                                  "wrong dungeon payload\n") &&
           write_joined_text_file(root, "dm2/GRAPHICS.DAT",
                                  "not a DM2 graphics file\n") &&
           write_joined_text_file(root, "dm2/DUNGEON.DAT",
                                  "not a DM2 dungeon file\n");
}

static void check_game_unavailable(const M12_AssetStatus* status,
                                   const char* gameId) {
    size_t i;
    size_t versionCount = M12_AssetStatus_GetVersionCount(gameId);
    size_t requiredCount = M12_AssetStatus_GetRequiredFileCount(status, gameId);
    char message[160];

    snprintf(message, sizeof(message), "%s should be unavailable", gameId);
    check_int(M12_AssetStatus_GameAvailable(status, gameId) == 0, message);
    snprintf(message, sizeof(message), "%s should expose required file metadata",
             gameId);
    check_int(requiredCount == M12_AssetStatus_GameRequiredFileCount(gameId),
              message);

    for (i = 0U; i < versionCount; ++i) {
        const M12_AssetVersionStatus* version =
            M12_AssetStatus_GetVersion(status, gameId, i);
        snprintf(message, sizeof(message),
                 "%s version %lu should not match irrelevant bytes",
                 gameId, (unsigned long)i);
        check_int(version && version->matched == 0, message);
    }
    for (i = 0U; i < requiredCount; ++i) {
        const M12_AssetRequiredFileStatus* required =
            M12_AssetStatus_GetRequiredFile(status, gameId, i);
        snprintf(message, sizeof(message),
                 "%s required file %lu should be missing",
                 gameId, (unsigned long)i);
        check_int(required && required->required && required->matched == 0,
                  message);
        snprintf(message, sizeof(message),
                 "%s required file %lu should not keep a matched path",
                 gameId, (unsigned long)i);
        check_int(required && required->matchedPath[0] == '\0', message);
        snprintf(message, sizeof(message),
                 "%s required file %lu should not keep a matched hash",
                 gameId, (unsigned long)i);
        check_int(required && required->matchedHash[0] == '\0', message);
    }
}

int main(void) {
    static const char* const gameIds[] = {
        "dm1", "csb", "dm2", "nexus", "theron"
    };
    char home[M12_ASSET_DATA_DIR_CAPACITY];
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char userDataDir[M12_ASSET_DATA_DIR_CAPACITY];
    char cacheRoot[M12_ASSET_DATA_DIR_CAPACITY];
    M12_AssetStatus status;
    size_t i;

    if (!make_isolated_home(home, sizeof(home)) ||
        !FSP_JoinPath(dataRoot, sizeof(dataRoot), home, "configured-data") ||
        !setup_irrelevant_data_root(dataRoot)) {
        fprintf(stderr, "fixture setup failed\n");
        return 1;
    }
    if (!test_setenv("HOME", home) ||
        !test_setenv("FIRESTAFF_DATA", dataRoot) ||
        !test_setenv("XDG_DATA_HOME", home) ||
        !test_setenv("APPDATA", home)) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);

    check_int(FSP_GetUserDataDir(userDataDir, sizeof(userDataDir)),
              "user data directory should resolve inside the isolated home");
    check_int(FSP_JoinPath(cacheRoot, sizeof(cacheRoot), userDataDir,
                           "asset-cache"),
              "asset cache path should resolve");
    check_int(!FSP_PathExists(cacheRoot),
              "fixture should start without an asset cache");

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, dataRoot);

    check_int(strcmp(M12_AssetStatus_GetDataDir(&status), dataRoot) == 0,
              "scan should keep the configured data root");
    check_int(strcmp(M12_AssetStatus_GetLegacyFallbackDir(&status),
                     dataRoot) == 0,
              "legacy fallback should resolve to FIRESTAFF_DATA, not real user data");

    for (i = 0U; i < sizeof(gameIds) / sizeof(gameIds[0]); ++i) {
        check_game_unavailable(&status, gameIds[i]);
    }

    check_int(!FSP_PathExists(cacheRoot),
              "irrelevant inputs should not create a materialized asset cache");

    (void)test_setenv("FIRESTAFF_DATA", NULL);
    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: irrelevant configured data root leaves required game data missing");
    return 0;
}
