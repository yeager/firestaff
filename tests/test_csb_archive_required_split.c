#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "asset_find_by_hash.h"
#include "asset_status_m12.h"
#include "fs_portable_compat.h"

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

static const char kCsbGraphicsPayload[] =
    "Firestaff synthetic CSB graphics archive fixture v1\n";
static const char kCsbDungeonPayload[] =
    "Firestaff synthetic CSB dungeon plain fixture v1\n";
static const char kCsbWrongGraphicsPayload[] =
    "Firestaff synthetic CSB wrong graphics archive fixture v1\n";
static const char kCsbGraphicsMd5[] = "5b5922a7c89d7a885f7334000df4846a";
static const char kCsbDungeonMd5[] = "157772fb82d2b82195878bdc58f286d7";
static const char kCsbWrongGraphicsMd5[] = "51332f60ebc4f0ac37f9f341c8c62240";

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
    int rc = snprintf(out, outSize, ".\\firestaff_csb_archive_required_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir_if_needed(out);
#else
    char templatePath[] = "/tmp/firestaff-csb-archive-required-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static int join_path(char* out, size_t outSize,
                     const char* left, const char* right) {
    int rc = snprintf(out, outSize, "%s/%s", left, right);
    return rc > 0 && (size_t)rc < outSize;
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

static int write_iso_dir_record(unsigned char* dir, int offset,
                                unsigned int lba, unsigned int size,
                                int isDir, const unsigned char* name,
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

static int write_single_entry_iso_file(const char* path,
                                       const char* entryName,
                                       const char* payload) {
    FILE* fp = fopen(path, "wb");
    unsigned char zero[2048] = {0};
    unsigned char pvd[2048] = {0};
    unsigned char dir[2048] = {0};
    unsigned char fileSector[2048] = {0};
    unsigned char dot = 0;
    unsigned char dotdot = 1;
    unsigned int payloadSize = (unsigned int)strlen(payload);
    int recLen;
    int offset;
    if (!fp || payloadSize > sizeof(fileSector)) {
        if (fp) {
            fclose(fp);
        }
        return 0;
    }
    for (int i = 0; i < 16; ++i) {
        if (fwrite(zero, 1U, sizeof(zero), fp) != sizeof(zero)) {
            fclose(fp);
            return 0;
        }
    }
    pvd[0] = 1;
    memcpy(pvd + 1, "CD001", 5);
    pvd[6] = 1;
    recLen = write_iso_dir_record(pvd, 156, 20U, 2048U, 1, &dot, 1);
    if (!recLen || fwrite(pvd, 1U, sizeof(pvd), fp) != sizeof(pvd)) {
        fclose(fp);
        return 0;
    }
    for (int i = 17; i < 20; ++i) {
        if (fwrite(zero, 1U, sizeof(zero), fp) != sizeof(zero)) {
            fclose(fp);
            return 0;
        }
    }
    offset = 0;
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
    recLen = write_iso_dir_record(dir, offset, 21U, payloadSize, 0,
                                  (const unsigned char*)entryName,
                                  (int)strlen(entryName));
    if (!recLen || fwrite(dir, 1U, sizeof(dir), fp) != sizeof(dir)) {
        fclose(fp);
        return 0;
    }
    memcpy(fileSector, payload, payloadSize);
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
    if (!fp) {
        return 0;
    }
    n = fread(buf, 1U, sizeof(buf), fp);
    fclose(fp);
    return n == expected && memcmp(buf, payload, expected) == 0;
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

static int path_has_virtual_entry(const char* path,
                                  const char* container,
                                  const char* entry) {
    return path && strstr(path, container) && strstr(path, "::") &&
           strstr(path, entry);
}

static int path_has_asset_cache(const char* path) {
    return path && strstr(path, "asset-cache");
}

static int path_has_runtime_cache_leaf(const char* path,
                                       const char* runtimeDir,
                                       const char* leaf) {
    char gameDir[512];
    char expected[512];
    return path && runtimeDir && leaf &&
           FSP_JoinPath(gameDir, sizeof(gameDir), runtimeDir, "csb") &&
           FSP_JoinPath(expected, sizeof(expected), gameDir, leaf) &&
           strcmp(path, expected) == 0 &&
           strstr(path, "asset-cache") &&
           strstr(path, "csb") &&
           !strstr(path, "::");
}

static void check_csb_zip_graphics_iso_dungeon_materializes(
    const char* root) {
    char zipPath[512];
    char isoPath[512];
    char foundPath[ASSET_PATH_MAX];
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    const char* runtimeDir;
    M12_AssetStatus status;

    check_int(join_path(zipPath, sizeof(zipPath), root, "csb_graphics.zip"),
              "positive ZIP path should fit");
    check_int(join_path(isoPath, sizeof(isoPath), root, "csb_required.iso"),
              "positive ISO path should fit");
    check_int(write_stored_zip_file(zipPath, "archive/GRAPHICS.DAT",
                                    kCsbGraphicsPayload),
              "synthetic CSB GRAPHICS ZIP fixture should be written");
    check_int(write_single_entry_iso_file(isoPath, "dungeon.dat;1",
                                          kCsbDungeonPayload),
              "synthetic CSB DUNGEON ISO fixture should be written");

    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, kCsbGraphicsMd5,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  path_has_virtual_entry(foundPath, "csb_graphics.zip",
                                         "archive/GRAPHICS.DAT"),
              "CSB GRAPHICS hash should be found through a ZIP virtual path");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, kCsbDungeonMd5,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  path_has_virtual_entry(foundPath, "csb_required.iso",
                                         "dungeon.dat"),
              "CSB DUNGEON hash should be found through an ISO virtual path");

    M12_AssetStatus_TestSetCsbSyntheticHashes(kCsbGraphicsMd5, kCsbDungeonMd5);
    M12_AssetStatus_Scan(&status, root);

    check_int(M12_AssetStatus_GameAvailable(&status, "csb") == 1,
              "CSB should be available when ZIP GRAPHICS and plain DUNGEON both match");
    check_int(M12_AssetStatus_GetRequiredFileCount(&status, "csb") == 2U,
              "CSB should report GRAPHICS and DUNGEON required files");
    version = M12_AssetStatus_GetVersion(&status, "csb", 0U);
    check_int(version && version->matched &&
                  path_has_virtual_entry(version->matchedPath,
                                         "csb_graphics.zip",
                                         "archive/GRAPHICS.DAT"),
              "CSB version identity should come from the archive-backed GRAPHICS hash");

    graphics = required_file_by_role(&status, "graphics");
    dungeon = required_file_by_role(&status, "dungeon");
    check_int(graphics && graphics->matched && graphics->label &&
                  strcmp(graphics->label, "GRAPHICS.DAT") == 0,
              "CSB required GRAPHICS should be matched");
    check_int(dungeon && dungeon->matched && dungeon->label &&
                  strcmp(dungeon->label, "DUNGEON.DAT") == 0,
              "CSB required DUNGEON should be matched");
    runtimeDir = M12_AssetStatus_GetRuntimeDataDir(&status, "csb");
    check_int(path_has_asset_cache(runtimeDir),
              "CSB runtime data dir should point at the asset cache when a required file was virtual");
    check_int(graphics &&
                  path_has_runtime_cache_leaf(graphics->matchedPath,
                                              runtimeDir,
                                              "GRAPHICS.DAT"),
              "ZIP-backed CSB GRAPHICS should be materialized to runtimeDataDir/csb/GRAPHICS.DAT");
    check_int(dungeon &&
                  path_has_runtime_cache_leaf(dungeon->matchedPath,
                                              runtimeDir,
                                              "DUNGEON.DAT"),
              "ISO-backed CSB DUNGEON should be materialized to runtimeDataDir/csb/DUNGEON.DAT");
    check_int(graphics && file_matches_payload(graphics->matchedPath,
                                               kCsbGraphicsPayload),
              "materialized CSB GRAPHICS should match the ZIP payload");
    check_int(dungeon && file_matches_payload(dungeon->matchedPath,
                                              kCsbDungeonPayload),
              "materialized CSB DUNGEON should match the ISO payload");
}

static void check_csb_wrong_archive_graphics_blocks_launch(const char* root) {
    char zipPath[512];
    char isoPath[512];
    char foundPath[ASSET_PATH_MAX];
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    M12_AssetStatus status;

    check_int(join_path(zipPath, sizeof(zipPath), root, "csb_wrong_graphics.zip"),
              "negative ZIP path should fit");
    check_int(join_path(isoPath, sizeof(isoPath), root, "csb_required.iso"),
              "negative ISO path should fit");
    check_int(write_stored_zip_file(zipPath, "archive/GRAPHICS.DAT",
                                    kCsbWrongGraphicsPayload),
              "wrong CSB GRAPHICS ZIP fixture should be written");
    check_int(write_single_entry_iso_file(isoPath, "dungeon.dat;1",
                                          kCsbDungeonPayload),
              "negative CSB DUNGEON ISO fixture should be written");

    memset(foundPath, 0, sizeof(foundPath));
    check_int(!asset_find_by_md5(root, kCsbGraphicsMd5,
                                 foundPath, (int)sizeof(foundPath), 32),
              "expected CSB GRAPHICS hash should be absent from the wrong ZIP");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, kCsbWrongGraphicsMd5,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  path_has_virtual_entry(foundPath, "csb_wrong_graphics.zip",
                                         "archive/GRAPHICS.DAT"),
              "wrong GRAPHICS payload should still prove the archive was scanned");

    M12_AssetStatus_TestSetCsbSyntheticHashes(kCsbGraphicsMd5, kCsbDungeonMd5);
    M12_AssetStatus_Scan(&status, root);

    check_int(M12_AssetStatus_GameAvailable(&status, "csb") == 0,
              "CSB should be unavailable when archive-backed GRAPHICS is wrong");
    version = M12_AssetStatus_GetVersion(&status, "csb", 0U);
    check_int(version && !version->matched,
              "CSB PC 3.4 version should not match the wrong archive payload");
    graphics = required_file_by_role(&status, "graphics");
    dungeon = required_file_by_role(&status, "dungeon");
    check_int(graphics && !graphics->matched,
              "CSB required GRAPHICS should remain missing for the wrong archive");
    check_int(dungeon && dungeon->matched &&
                  path_has_virtual_entry(dungeon->matchedPath,
                                         "csb_required.iso",
                                         "dungeon.dat") &&
                  !strstr(dungeon->matchedPath, "asset-cache"),
              "ISO CSB DUNGEON may match, but should not materialize when CSB is unavailable");
}

int main(void) {
    char home[512];
    char positiveRoot[512];
    char negativeRoot[512];

    if (!make_isolated_home(home, sizeof(home)) ||
        !join_path(positiveRoot, sizeof(positiveRoot), home, "positive-data") ||
        !join_path(negativeRoot, sizeof(negativeRoot), home, "negative-data") ||
        !make_dir_if_needed(positiveRoot) ||
        !make_dir_if_needed(negativeRoot) ||
        !test_setenv("HOME", home)) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    check_int(test_setenv("FIRESTAFF_DATA", positiveRoot),
              "positive FIRESTAFF_DATA should be set");
    check_csb_zip_graphics_iso_dungeon_materializes(positiveRoot);

    check_int(test_setenv("FIRESTAFF_DATA", negativeRoot),
              "negative FIRESTAFF_DATA should be set");
    check_csb_wrong_archive_graphics_blocks_launch(negativeRoot);
    (void)test_setenv("FIRESTAFF_DATA", NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: CSB required-file discovery materializes ZIP/ISO archive sources and blocks wrong archive payloads");
    return 0;
}
