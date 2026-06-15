#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "asset_find_by_hash.h"
#include "asset_status_m12.h"

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

static int path_has_asset_cache_for_csb(const char* path) {
    return path && strstr(path, "asset-cache") && strstr(path, "csb");
}

static int path_has_asset_cache(const char* path) {
    return path && strstr(path, "asset-cache");
}

static void check_csb_zip_graphics_plain_dungeon_materializes(
    const char* root) {
    char zipPath[512];
    char dungeonPath[512];
    char foundPath[ASSET_PATH_MAX];
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    const char* runtimeDir;
    M12_AssetStatus status;

    check_int(join_path(zipPath, sizeof(zipPath), root, "csb_graphics.zip"),
              "positive ZIP path should fit");
    check_int(join_path(dungeonPath, sizeof(dungeonPath), root,
                        "renamed-csb-dungeon.asset"),
              "positive DUNGEON path should fit");
    check_int(write_stored_zip_file(zipPath, "archive/GRAPHICS.DAT",
                                    kCsbGraphicsPayload),
              "synthetic CSB GRAPHICS ZIP fixture should be written");
    check_int(write_plain_file(dungeonPath, kCsbDungeonPayload),
              "synthetic CSB DUNGEON plain fixture should be written");

    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, kCsbGraphicsMd5,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  path_has_virtual_entry(foundPath, "csb_graphics.zip",
                                         "archive/GRAPHICS.DAT"),
              "CSB GRAPHICS hash should be found through a ZIP virtual path");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, kCsbDungeonMd5,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "renamed-csb-dungeon.asset") &&
                  !strstr(foundPath, "::"),
              "CSB DUNGEON hash should be found as a plain file");

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
    check_int(graphics && !strstr(graphics->matchedPath, "::") &&
                  path_has_asset_cache_for_csb(graphics->matchedPath),
              "archive-backed CSB GRAPHICS should be materialized to asset-cache/csb");
    check_int(dungeon && !strstr(dungeon->matchedPath, "::") &&
                  path_has_asset_cache_for_csb(dungeon->matchedPath),
              "plain CSB DUNGEON should be copied beside the materialized archive file");
    check_int(graphics && file_matches_payload(graphics->matchedPath,
                                               kCsbGraphicsPayload),
              "materialized CSB GRAPHICS should match the ZIP payload");
    check_int(dungeon && file_matches_payload(dungeon->matchedPath,
                                              kCsbDungeonPayload),
              "materialized CSB DUNGEON should match the plain payload");
    runtimeDir = M12_AssetStatus_GetRuntimeDataDir(&status, "csb");
    check_int(path_has_asset_cache(runtimeDir),
              "CSB runtime data dir should point at the asset cache when a required file was virtual");
}

static void check_csb_wrong_archive_graphics_blocks_launch(const char* root) {
    char zipPath[512];
    char dungeonPath[512];
    char foundPath[ASSET_PATH_MAX];
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    M12_AssetStatus status;

    check_int(join_path(zipPath, sizeof(zipPath), root, "csb_wrong_graphics.zip"),
              "negative ZIP path should fit");
    check_int(join_path(dungeonPath, sizeof(dungeonPath), root,
                        "renamed-csb-dungeon.asset"),
              "negative DUNGEON path should fit");
    check_int(write_stored_zip_file(zipPath, "archive/GRAPHICS.DAT",
                                    kCsbWrongGraphicsPayload),
              "wrong CSB GRAPHICS ZIP fixture should be written");
    check_int(write_plain_file(dungeonPath, kCsbDungeonPayload),
              "negative CSB DUNGEON plain fixture should be written");

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
                  strstr(dungeon->matchedPath, "renamed-csb-dungeon.asset") &&
                  !strstr(dungeon->matchedPath, "asset-cache"),
              "plain CSB DUNGEON may match, but should not materialize when CSB is unavailable");
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
    check_csb_zip_graphics_plain_dungeon_materializes(positiveRoot);

    check_int(test_setenv("FIRESTAFF_DATA", negativeRoot),
              "negative FIRESTAFF_DATA should be set");
    check_csb_wrong_archive_graphics_blocks_launch(negativeRoot);
    (void)test_setenv("FIRESTAFF_DATA", NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: CSB required-file discovery handles split archive/plain sources and blocks wrong archive payloads");
    return 0;
}
