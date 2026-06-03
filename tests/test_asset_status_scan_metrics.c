#include "asset_status_m12.h"
#include "asset_find_by_hash.h"

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

static void check_int(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static int make_dir_if_needed(const char* path) {
    return MKDIR(path) == 0;
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_asset_status_metrics_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir_if_needed(out);
#else
    char templatePath[] = "/tmp/firestaff-asset-status-metrics-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static void put16(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8U) & 0xffU);
}

static void put32(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8U) & 0xffU);
    p[2] = (unsigned char)((v >> 16U) & 0xffU);
    p[3] = (unsigned char)((v >> 24U) & 0xffU);
}

static int write_stored_zip_file(const char* path, const char* entryName,
                                 const char* payload) {
    FILE* fp = fopen(path, "wb");
    unsigned char local[30] = {0};
    unsigned char central[46] = {0};
    unsigned char eocd[22] = {0};
    unsigned int payloadSize = (unsigned int)strlen(payload);
    unsigned int nameLen = (unsigned int)strlen(entryName);
    unsigned int centralOffset;
    if (!fp) return 0;

    put32(local, 0x04034b50U);
    put16(local + 4, 20U);
    put16(local + 8, 0U);
    put32(local + 18, payloadSize);
    put32(local + 22, payloadSize);
    put16(local + 26, nameLen);
    if (fwrite(local, 1U, sizeof(local), fp) != sizeof(local) ||
        fwrite(entryName, 1U, nameLen, fp) != nameLen ||
        fwrite(payload, 1U, payloadSize, fp) != payloadSize) {
        fclose(fp);
        return 0;
    }

    centralOffset = (unsigned int)ftell(fp);
    put32(central, 0x02014b50U);
    put16(central + 4, 20U);
    put16(central + 6, 20U);
    put16(central + 10, 0U);
    put32(central + 20, payloadSize);
    put32(central + 24, payloadSize);
    put16(central + 28, nameLen);
    if (fwrite(central, 1U, sizeof(central), fp) != sizeof(central) ||
        fwrite(entryName, 1U, nameLen, fp) != nameLen) {
        fclose(fp);
        return 0;
    }

    put32(eocd, 0x06054b50U);
    put16(eocd + 8, 1U);
    put16(eocd + 10, 1U);
    put32(eocd + 12, (unsigned int)(sizeof(central) + nameLen));
    put32(eocd + 16, centralOffset);
    if (fwrite(eocd, 1U, sizeof(eocd), fp) != sizeof(eocd)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_iso_record(unsigned char* dir, int offset, unsigned int lba,
                            unsigned int size, int isDir,
                            const unsigned char* name, int nameLen) {
    int recLen = 33 + nameLen + ((nameLen & 1) ? 1 : 0);
    if (offset + recLen > 2048) return 0;
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

static int write_iso_file(const char* path, const char* entryName,
                          const char* payload) {
    static const unsigned char dot = 0;
    static const unsigned char dotdot = 1;
    FILE* fp = fopen(path, "wb");
    unsigned char zero[2048] = {0};
    unsigned char pvd[2048] = {0};
    unsigned char dir[2048] = {0};
    unsigned char fileSector[2048] = {0};
    int offset = 0;
    int recLen;
    if (!fp) return 0;
    for (int i = 0; i < 16; ++i) {
        if (fwrite(zero, 1U, sizeof(zero), fp) != sizeof(zero)) {
            fclose(fp);
            return 0;
        }
    }
    pvd[0] = 1;
    memcpy(pvd + 1, "CD001", 5);
    pvd[6] = 1;
    (void)write_iso_record(pvd, 156, 20U, 2048U, 1, &dot, 1);
    if (fwrite(pvd, 1U, sizeof(pvd), fp) != sizeof(pvd)) {
        fclose(fp);
        return 0;
    }
    for (int i = 17; i < 20; ++i) {
        if (fwrite(zero, 1U, sizeof(zero), fp) != sizeof(zero)) {
            fclose(fp);
            return 0;
        }
    }
    recLen = write_iso_record(dir, offset, 20U, 2048U, 1, &dot, 1);
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    offset += recLen;
    recLen = write_iso_record(dir, offset, 20U, 2048U, 1, &dotdot, 1);
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    offset += recLen;
    recLen = write_iso_record(dir, offset, 21U, (unsigned int)strlen(payload),
                              0, (const unsigned char*)entryName,
                              (int)strlen(entryName));
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    if (fwrite(dir, 1U, sizeof(dir), fp) != sizeof(dir)) {
        fclose(fp);
        return 0;
    }
    memcpy(fileSector, payload, strlen(payload));
    if (fwrite(fileSector, 1U, sizeof(fileSector), fp) != sizeof(fileSector)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int file_matches_payload(const char* path, const char* payload) {
    char buf[128];
    FILE* fp = fopen(path, "rb");
    size_t expected = strlen(payload);
    size_t n;
    if (!fp) return 0;
    n = fread(buf, 1U, sizeof(buf), fp);
    fclose(fp);
    return n == expected && memcmp(buf, payload, expected) == 0;
}

static void check_dm2_virtual_required_files_materialize(const char* root) {
    static const char graphicsPayload[] =
        "Firestaff synthetic DM2 graphics fixture v1\n";
    static const char dungeonPayload[] =
        "Firestaff synthetic DM2 dungeon fixture v1\n";
    char zipPath[512];
    char isoPath[512];
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    M12_AssetStatus status;
    char foundPath[ASSET_PATH_MAX];

    snprintf(zipPath, sizeof(zipPath), "%s/dm2_graphics.zip", root);
    snprintf(isoPath, sizeof(isoPath), "%s/dm2_dungeon.iso", root);
    check_int(write_stored_zip_file(zipPath, "dm2/GRAPHICS.DAT", graphicsPayload),
              "synthetic DM2 ZIP fixture should be written");
    check_int(write_iso_file(isoPath, "DUNGEON.DAT;1", dungeonPayload),
              "synthetic DM2 ISO fixture should be written");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, "1fa389d21b761938e01dd3aa0c26b35d",
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "dm2_graphics.zip") != NULL &&
                  strstr(foundPath, "::") != NULL,
              "synthetic DM2 GRAPHICS hash should be found inside ZIP");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, "8c01c1def7ee3df8de50a3cd9cf05b36",
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "dm2_dungeon.iso") != NULL &&
                  strstr(foundPath, "::") != NULL,
              "synthetic DM2 DUNGEON hash should be found inside ISO");

    M12_AssetStatus_TestSetDm2SyntheticHashes("1fa389d21b761938e01dd3aa0c26b35d",
                                              "8c01c1def7ee3df8de50a3cd9cf05b36");
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);

    check_int(M12_AssetStatus_GameAvailable(&status, "dm2") == 1,
              "DM2 should be available when synthetic GRAPHICS and DUNGEON hashes match");
    check_int(M12_AssetStatus_GetRequiredFileCount(&status, "dm2") == 2U,
              "DM2 should still report GRAPHICS and DUNGEON as required files");
    graphics = M12_AssetStatus_GetRequiredFile(&status, "dm2", 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&status, "dm2", 1U);
    check_int(graphics && graphics->matched && strstr(graphics->matchedPath, "::") == NULL,
              "DM2 virtual GRAPHICS should be materialized to a flat runtime file");
    check_int(dungeon && dungeon->matched && strstr(dungeon->matchedPath, "::") == NULL,
              "DM2 virtual DUNGEON should be materialized to a flat runtime file");
    check_int(graphics && strstr(graphics->matchedPath, "asset-cache") != NULL &&
                  strstr(graphics->matchedPath, "dm2") != NULL,
              "materialized DM2 GRAPHICS should live in the asset cache");
    check_int(dungeon && strstr(dungeon->matchedPath, "asset-cache") != NULL &&
                  strstr(dungeon->matchedPath, "dm2") != NULL,
              "materialized DM2 DUNGEON should live in the asset cache");
    check_int(graphics && file_matches_payload(graphics->matchedPath, graphicsPayload),
              "materialized DM2 GRAPHICS should match the ZIP payload");
    check_int(dungeon && file_matches_payload(dungeon->matchedPath, dungeonPayload),
              "materialized DM2 DUNGEON should match the ISO payload");
}

int main(void) {
    enum {
        VERSION_SCAN_GROUPS = 5,
        FIXED_REQUIRED_HASHES = 3
    };
    char home[512];
    char requestRoot[512];
    M12_AssetStatus status;
    M12_AssetStatusScanMetrics metrics;

    if (!make_isolated_home(home, sizeof(home))) {
        fprintf(stderr, "fixture home setup failed\n");
        return 1;
    }
    snprintf(requestRoot, sizeof(requestRoot), "%s/custom-data", home);
    if (!make_dir_if_needed(requestRoot) ||
        !test_setenv("HOME", home) ||
        !test_setenv("FIRESTAFF_DATA", requestRoot)) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, requestRoot);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(metrics.rootCount == 2U,
              "requested root and default root should be scanned once each");
    check_int(metrics.duplicateRootSkips == 1U,
              "FIRESTAFF_DATA duplicate of requested root should be skipped");
    check_int(metrics.versionHashLookups ==
                  (size_t)VERSION_SCAN_GROUPS * metrics.rootCount,
              "empty scan should batch version-hash searches once per game/root");
    check_int(metrics.requiredHashLookups ==
                  (size_t)FIXED_REQUIRED_HASHES * metrics.rootCount,
              "empty scan should show one fixed required-file search per hash/root");
    check_int(!M12_AssetStatus_HasOriginalFileCandidate(&status),
              "empty fixture should not report original asset candidates");

    check_dm2_virtual_required_files_materialize(requestRoot);

    (void)test_setenv("FIRESTAFF_DATA", NULL);
    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: asset-status scan metrics pin duplicate-root prefilter and hash fan-out");
    return 0;
}
