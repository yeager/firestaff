#include "asset_find_by_hash.h"
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

static const unsigned char kGraphicsPayload[] =
    "synthetic DM2 graphics payload for ISO required cache gate\n";
static const unsigned char kDungeonPayload[] =
    "synthetic DM2 dungeon payload for ISO required cache gate\n";

static void put32(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8U) & 0xffU);
    p[2] = (unsigned char)((v >> 16U) & 0xffU);
    p[3] = (unsigned char)((v >> 24U) & 0xffU);
}

static void check_int(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_dm2_iso_cache_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return FSP_CreateDirectoryRecursive(out);
#else
    char templatePath[] = "/tmp/firestaff-dm2-iso-cache-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static int write_payload_file(const char* path,
                              const unsigned char* payload,
                              size_t payloadSize) {
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    if (fwrite(payload, 1U, payloadSize, fp) != payloadSize) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int file_matches_payload(const char* path,
                                const unsigned char* payload,
                                size_t payloadSize) {
    unsigned char buf[128];
    FILE* fp = fopen(path, "rb");
    size_t n;
    if (!fp || payloadSize > sizeof(buf)) {
        if (fp) {
            fclose(fp);
        }
        return 0;
    }
    n = fread(buf, 1U, sizeof(buf), fp);
    fclose(fp);
    return n == payloadSize && memcmp(buf, payload, payloadSize) == 0;
}

static int path_has_virtual_entry(const char* path,
                                  const char* isoName,
                                  const char* entryName) {
    return path && strstr(path, isoName) && strstr(path, "::") &&
           strstr(path, entryName);
}

static int path_has_cache_leaf(const char* path,
                               const char* cacheRoot,
                               const char* gameId,
                               const char* leaf) {
    return path && strstr(path, cacheRoot) && strstr(path, gameId) &&
           strstr(path, leaf) && !strstr(path, "::");
}

static int write_iso_dir_record(unsigned char* dir,
                                int offset,
                                unsigned int lba,
                                unsigned int size,
                                int isDir,
                                const unsigned char* name,
                                int nameLen) {
    int recLen = 33 + nameLen + ((nameLen & 1) ? 1 : 0);
    if (offset + recLen > 2048) {
        return 0;
    }
    memset(dir + offset, 0, (size_t)recLen);
    dir[offset] = (unsigned char)recLen;
    put32(dir + offset + 2, lba);
    put32(dir + offset + 6, lba);
    put32(dir + offset + 10, size);
    put32(dir + offset + 14, size);
    dir[offset + 25] = isDir ? 0x02 : 0x00;
    dir[offset + 28] = 1;
    dir[offset + 32] = (unsigned char)nameLen;
    memcpy(dir + offset + 33, name, (size_t)nameLen);
    return recLen;
}

static int write_two_file_iso(const char* path) {
    static const unsigned char dot = 0;
    static const unsigned char dotdot = 1;
    static const unsigned char graphicsName[] = "GRAPHICS.DAT;1";
    static const unsigned char dungeonName[] = "DUNGEON.DAT;1";
    FILE* fp = fopen(path, "wb");
    unsigned char zero[2048] = {0};
    unsigned char pvd[2048] = {0};
    unsigned char dir[2048] = {0};
    unsigned char graphicsSector[2048] = {0};
    unsigned char dungeonSector[2048] = {0};
    int offset = 0;
    int recLen;
    int i;
    if (!fp) {
        return 0;
    }
    for (i = 0; i < 16; ++i) {
        if (fwrite(zero, 1U, sizeof(zero), fp) != sizeof(zero)) {
            fclose(fp);
            return 0;
        }
    }
    pvd[0] = 1;
    memcpy(pvd + 1, "CD001", 5);
    pvd[6] = 1;
    if (!write_iso_dir_record(pvd, 156, 20U, 2048U, 1, &dot, 1)) {
        fclose(fp);
        return 0;
    }
    if (fwrite(pvd, 1U, sizeof(pvd), fp) != sizeof(pvd)) {
        fclose(fp);
        return 0;
    }
    for (i = 17; i < 20; ++i) {
        if (fwrite(zero, 1U, sizeof(zero), fp) != sizeof(zero)) {
            fclose(fp);
            return 0;
        }
    }
    recLen = write_iso_dir_record(dir, offset, 20U, 2048U, 1, &dot, 1);
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    offset += recLen;
    recLen = write_iso_dir_record(dir, offset, 20U, 2048U, 1, &dotdot, 1);
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    offset += recLen;
    recLen = write_iso_dir_record(dir, offset, 21U,
                                  (unsigned int)(sizeof(kGraphicsPayload) - 1U),
                                  0, graphicsName,
                                  (int)(sizeof(graphicsName) - 1U));
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    offset += recLen;
    recLen = write_iso_dir_record(dir, offset, 22U,
                                  (unsigned int)(sizeof(kDungeonPayload) - 1U),
                                  0, dungeonName,
                                  (int)(sizeof(dungeonName) - 1U));
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    if (fwrite(dir, 1U, sizeof(dir), fp) != sizeof(dir)) {
        fclose(fp);
        return 0;
    }
    memcpy(graphicsSector, kGraphicsPayload, sizeof(kGraphicsPayload) - 1U);
    memcpy(dungeonSector, kDungeonPayload, sizeof(kDungeonPayload) - 1U);
    if (fwrite(graphicsSector, 1U, sizeof(graphicsSector), fp) != sizeof(graphicsSector) ||
        fwrite(dungeonSector, 1U, sizeof(dungeonSector), fp) != sizeof(dungeonSector)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int runtime_cache_file_matches_payload(const M12_AssetStatus* status,
                                              const char* gameId,
                                              const char* leaf,
                                              const unsigned char* payload,
                                              size_t payloadSize) {
    char gameLeaf[M12_ASSET_DATA_DIR_CAPACITY];
    char runtimePath[M12_ASSET_DATA_DIR_CAPACITY];
    const char* runtimeRoot = M12_AssetStatus_GetRuntimeDataDir(status, gameId);
    if (!runtimeRoot || !gameId || !leaf || !payload) {
        return 0;
    }
    if (snprintf(gameLeaf, sizeof(gameLeaf), "%s/%s", gameId, leaf) >=
        (int)sizeof(gameLeaf)) {
        return 0;
    }
    if (!FSP_JoinPath(runtimePath, sizeof(runtimePath), runtimeRoot, gameLeaf)) {
        return 0;
    }
    return file_matches_payload(runtimePath, payload, payloadSize);
}

static int block_asset_cache_root(void) {
    char userDataDir[M12_ASSET_DATA_DIR_CAPACITY];
    char cacheRoot[M12_ASSET_DATA_DIR_CAPACITY];
    FILE* fp;
    if (!FSP_GetUserDataDir(userDataDir, sizeof(userDataDir)) ||
        !FSP_CreateDirectoryRecursive(userDataDir) ||
        !FSP_JoinPath(cacheRoot, sizeof(cacheRoot), userDataDir, "asset-cache")) {
        return 0;
    }
    fp = fopen(cacheRoot, "wb");
    if (!fp) {
        return 0;
    }
    if (fputs("not a directory\n", fp) < 0) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void scan_iso_fixture_case(int blockCache) {
    char home[M12_ASSET_DATA_DIR_CAPACITY];
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char isoPath[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsPath[M12_ASSET_DATA_DIR_CAPACITY];
    char dungeonPath[M12_ASSET_DATA_DIR_CAPACITY];
    char userDataDir[M12_ASSET_DATA_DIR_CAPACITY];
    char cacheRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char cachedGraphics[M12_ASSET_DATA_DIR_CAPACITY];
    char cachedDungeon[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];
    char foundPath[ASSET_PATH_MAX];
    M12_AssetStatus status;
    const M12_AssetRequiredFileStatus* required;
    const M12_AssetVersionStatus* version;

    if (!make_isolated_home(home, sizeof(home)) ||
        !FSP_JoinPath(dataRoot, sizeof(dataRoot), home, "configured-data") ||
        !FSP_CreateDirectoryRecursive(dataRoot) ||
        !FSP_JoinPath(graphicsPath, sizeof(graphicsPath), home, "graphics.bin") ||
        !FSP_JoinPath(dungeonPath, sizeof(dungeonPath), home, "dungeon.bin") ||
        !FSP_JoinPath(isoPath, sizeof(isoPath), dataRoot, "dm2-required.iso") ||
        !write_payload_file(graphicsPath, kGraphicsPayload,
                            sizeof(kGraphicsPayload) - 1U) ||
        !write_payload_file(dungeonPath, kDungeonPayload,
                            sizeof(kDungeonPayload) - 1U) ||
        !m12_file_md5_hex(graphicsPath, graphicsMd5) ||
        !m12_file_md5_hex(dungeonPath, dungeonMd5) ||
        !write_two_file_iso(isoPath)) {
        check_int(0, "fixture setup should succeed");
        return;
    }

    if (!test_setenv("HOME", home) ||
        !test_setenv("FIRESTAFF_DATA", dataRoot) ||
        !test_setenv("XDG_DATA_HOME", home) ||
        !test_setenv("APPDATA", home)) {
        check_int(0, "fixture environment should be installed");
        return;
    }
    if (blockCache) {
        check_int(block_asset_cache_root(),
                  "asset-cache blocker file should be writable");
    }

    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(dataRoot, graphicsMd5, foundPath,
                                (int)sizeof(foundPath), 4),
              "scanner should find ISO-backed DM2 graphics hash");
    check_int(path_has_virtual_entry(foundPath, "dm2-required.iso",
                                     "GRAPHICS.DAT"),
              "graphics match should be reported as an ISO virtual path");

    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(dataRoot, dungeonMd5, foundPath,
                                (int)sizeof(foundPath), 4),
              "scanner should find ISO-backed DM2 dungeon hash");
    check_int(path_has_virtual_entry(foundPath, "dm2-required.iso",
                                     "DUNGEON.DAT"),
              "dungeon match should be reported as an ISO virtual path");

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);

    M12_AssetStatus_Scan(&status, dataRoot);

    version = M12_AssetStatus_GetVersion(&status, "dm2", 0U);
    check_int(version && version->matched &&
              path_has_virtual_entry(version->matchedPath, "dm2-required.iso",
                                     "GRAPHICS.DAT"),
              "version match should preserve the original ISO virtual path");

    if (blockCache) {
        required = M12_AssetStatus_GetRequiredFile(&status, "dm2", 0U);
        check_int(!M12_AssetStatus_GameAvailable(&status, "dm2"),
                  "DM2 launch availability should be cleared when ISO "
                  "required files cannot materialize into the cache");
        check_int(required && required->matched &&
                  path_has_virtual_entry(required->matchedPath,
                                         "dm2-required.iso", "GRAPHICS.DAT"),
                  "failed materialization should leave the required row's "
                  "source virtual path visible for diagnostics");
        check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm2"),
                         dataRoot) == 0,
                  "failed materialization must not advertise asset-cache as "
                  "the DM2 runtime data root");
        return;
    }

    check_int(M12_AssetStatus_GameAvailable(&status, "dm2"),
              "DM2 should be available when both required hashes are ISO-backed "
              "and materialized");
    check_int(FSP_GetUserDataDir(userDataDir, sizeof(userDataDir)) &&
              FSP_JoinPath(cacheRoot, sizeof(cacheRoot), userDataDir,
                           "asset-cache") &&
              FSP_JoinPath(cachedGraphics, sizeof(cachedGraphics),
                           cacheRoot, "dm2/GRAPHICS.DAT") &&
              FSP_JoinPath(cachedDungeon, sizeof(cachedDungeon),
                           cacheRoot, "dm2/DUNGEON.DAT"),
              "asset cache leaf paths should resolve");
    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm2"),
                     cacheRoot) == 0,
              "DM2 runtime data root should point at the asset cache root");

    required = M12_AssetStatus_GetRequiredFile(&status, "dm2", 0U);
    check_int(required && required->matched &&
              path_has_cache_leaf(required->matchedPath, cacheRoot,
                                  "dm2", "GRAPHICS.DAT"),
              "DM2 graphics required file should be materialized into cache");
    check_int(required && strcmp(required->matchedPath, cachedGraphics) == 0,
              "DM2 graphics required path should match runtimeDataDir/dm2/GRAPHICS.DAT");
    required = M12_AssetStatus_GetRequiredFile(&status, "dm2", 1U);
    check_int(required && required->matched &&
              path_has_cache_leaf(required->matchedPath, cacheRoot,
                                  "dm2", "DUNGEON.DAT"),
              "DM2 dungeon required file should be materialized into cache");
    check_int(required && strcmp(required->matchedPath, cachedDungeon) == 0,
              "DM2 dungeon required path should match runtimeDataDir/dm2/DUNGEON.DAT");
    check_int(file_matches_payload(cachedGraphics, kGraphicsPayload,
                                   sizeof(kGraphicsPayload) - 1U),
              "cached GRAPHICS.DAT should contain the ISO entry payload");
    check_int(file_matches_payload(cachedDungeon, kDungeonPayload,
                                   sizeof(kDungeonPayload) - 1U),
              "cached DUNGEON.DAT should contain the ISO entry payload");
    check_int(runtime_cache_file_matches_payload(&status, "dm2",
                                                 "GRAPHICS.DAT",
                                                 kGraphicsPayload,
                                                 sizeof(kGraphicsPayload) - 1U),
              "DM2 launch lookup should open cached GRAPHICS.DAT as an ordinary file");
    check_int(runtime_cache_file_matches_payload(&status, "dm2",
                                                 "DUNGEON.DAT",
                                                 kDungeonPayload,
                                                 sizeof(kDungeonPayload) - 1U),
              "DM2 launch lookup should open cached DUNGEON.DAT as an ordinary file");
}

int main(void) {
    scan_iso_fixture_case(0);
    scan_iso_fixture_case(1);

    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: DM2 ISO virtual required files materialize before launch and block on cache failure");
    return 0;
}
