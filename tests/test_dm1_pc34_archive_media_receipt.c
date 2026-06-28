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

#ifdef FIRESTAFF_HAS_ZLIB
#include <zlib.h>
#endif

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

typedef struct {
    const char* name;
    const unsigned char* payload;
    size_t payloadSize;
    unsigned char* packed;
    size_t packedSize;
    unsigned int method;
    unsigned int localOffset;
} ZipEntry;

static int failures;
static int assertions;

static const unsigned char kGraphicsPayload[] =
    "Firestaff synthetic DM1 PC 3.4 English GRAPHICS media receipt payload\n"
    "GRAPHICS-HASH-ONLY-RECEIPT-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
    "GRAPHICS-HASH-ONLY-RECEIPT-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n";

static const unsigned char kDungeonPayload[] =
    "Firestaff synthetic DM1 PC 3.4 English DUNGEON media receipt payload\n"
    "DUNGEON-HASH-ONLY-RECEIPT-BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n"
    "DUNGEON-HASH-ONLY-RECEIPT-BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n";

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

static void check_int(int condition, const char* message) {
    ++assertions;
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_dm1_pc34_receipt_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return FSP_CreateDirectoryRecursive(out);
#else
    char templatePath[] = "/tmp/firestaff-dm1-pc34-receipt-XXXXXX";
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
    unsigned char* buf;
    size_t n;
    FILE* fp = fopen(path, "rb");
    int match;
    if (!fp) {
        return 0;
    }
    buf = (unsigned char*)malloc(payloadSize ? payloadSize : 1U);
    if (!buf) {
        fclose(fp);
        return 0;
    }
    n = fread(buf, 1U, payloadSize, fp);
    fclose(fp);
    match = (n == payloadSize && memcmp(buf, payload, payloadSize) == 0);
    free(buf);
    return match;
}

static int path_has_virtual_entry(const char* path,
                                  const char* archiveName,
                                  const char* entryName) {
    return path && strstr(path, archiveName) && strstr(path, "::") &&
           strstr(path, entryName);
}

static int path_has_cache_leaf(const char* path,
                               const char* cacheRoot,
                               const char* gameId,
                               const char* leaf) {
    return path && strstr(path, cacheRoot) && strstr(path, gameId) &&
           strstr(path, leaf) && !strstr(path, "::");
}

static int prepare_zip_entry(ZipEntry* entry) {
#ifdef FIRESTAFF_HAS_ZLIB
    z_stream zs;
    size_t bound;
    int ret;
    if (!entry || !entry->payload || entry->payloadSize == 0U) {
        return 0;
    }
    bound = (size_t)compressBound((uLong)entry->payloadSize);
    entry->packed = (unsigned char*)malloc(bound ? bound : 1U);
    if (!entry->packed) {
        return 0;
    }
    memset(&zs, 0, sizeof(zs));
    ret = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                       -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        free(entry->packed);
        entry->packed = NULL;
        return 0;
    }
    zs.next_in = (Bytef*)entry->payload;
    zs.avail_in = (uInt)entry->payloadSize;
    zs.next_out = entry->packed;
    zs.avail_out = (uInt)bound;
    ret = deflate(&zs, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&zs);
        free(entry->packed);
        entry->packed = NULL;
        return 0;
    }
    entry->packedSize = (size_t)zs.total_out;
    entry->method = 8U;
    return deflateEnd(&zs) == Z_OK;
#else
    if (!entry || !entry->payload || entry->payloadSize == 0U) {
        return 0;
    }
    entry->packed = (unsigned char*)malloc(entry->payloadSize);
    if (!entry->packed) {
        return 0;
    }
    memcpy(entry->packed, entry->payload, entry->payloadSize);
    entry->packedSize = entry->payloadSize;
    entry->method = 0U;
    return 1;
#endif
}

static int write_zip_entry_local(FILE* fp, ZipEntry* entry) {
    unsigned char local[30] = {0};
    unsigned int nameLen;
    if (!fp || !entry) {
        return 0;
    }
    entry->localOffset = (unsigned int)ftell(fp);
    nameLen = (unsigned int)strlen(entry->name);
    put32(local, 0x04034b50U);
    put16(local + 4, 20U);
    put16(local + 8, entry->method);
    put32(local + 18, (unsigned int)entry->packedSize);
    put32(local + 22, (unsigned int)entry->payloadSize);
    put16(local + 26, nameLen);
    return fwrite(local, 1U, sizeof(local), fp) == sizeof(local) &&
           fwrite(entry->name, 1U, nameLen, fp) == nameLen &&
           fwrite(entry->packed, 1U, entry->packedSize, fp) == entry->packedSize;
}

static int write_zip_entry_central(FILE* fp, const ZipEntry* entry) {
    unsigned char central[46] = {0};
    unsigned int nameLen;
    if (!fp || !entry) {
        return 0;
    }
    nameLen = (unsigned int)strlen(entry->name);
    put32(central, 0x02014b50U);
    put16(central + 4, 20U);
    put16(central + 6, 20U);
    put16(central + 10, entry->method);
    put32(central + 20, (unsigned int)entry->packedSize);
    put32(central + 24, (unsigned int)entry->payloadSize);
    put16(central + 28, nameLen);
    put32(central + 42, entry->localOffset);
    return fwrite(central, 1U, sizeof(central), fp) == sizeof(central) &&
           fwrite(entry->name, 1U, nameLen, fp) == nameLen;
}

static int write_zip_fixture(const char* path, ZipEntry entries[], size_t count) {
    FILE* fp = fopen(path, "wb");
    unsigned char eocd[22] = {0};
    unsigned int centralOffset;
    unsigned int centralSize;
    size_t i;
    if (!fp) {
        return 0;
    }
    for (i = 0U; i < count; ++i) {
        if (!prepare_zip_entry(&entries[i]) || !write_zip_entry_local(fp, &entries[i])) {
            fclose(fp);
            return 0;
        }
    }
    centralOffset = (unsigned int)ftell(fp);
    for (i = 0U; i < count; ++i) {
        if (!write_zip_entry_central(fp, &entries[i])) {
            fclose(fp);
            return 0;
        }
    }
    centralSize = (unsigned int)ftell(fp) - centralOffset;
    put32(eocd, 0x06054b50U);
    put16(eocd + 8, (unsigned int)count);
    put16(eocd + 10, (unsigned int)count);
    put32(eocd + 12, centralSize);
    put32(eocd + 16, centralOffset);
    return fwrite(eocd, 1U, sizeof(eocd), fp) == sizeof(eocd) &&
           fclose(fp) == 0;
}

static void free_zip_entries(ZipEntry entries[], size_t count) {
    size_t i;
    for (i = 0U; i < count; ++i) {
        free(entries[i].packed);
        entries[i].packed = NULL;
    }
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

int main(void) {
    static const char kZipName[] = "dm1-pc34-renamed-media.zip";
    static const char kGraphicsEntry[] = "renamed/by-hash/not-graphics.payload";
    static const char kDungeonEntry[] = "renamed/by-hash/not-dungeon.payload";
    char home[M12_ASSET_DATA_DIR_CAPACITY];
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char zipPath[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsPath[M12_ASSET_DATA_DIR_CAPACITY];
    char dungeonPath[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];
    char foundPath[ASSET_PATH_MAX];
    char userDataDir[M12_ASSET_DATA_DIR_CAPACITY];
    char cacheRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char cachedGraphics[M12_ASSET_DATA_DIR_CAPACITY];
    char cachedDungeon[M12_ASSET_DATA_DIR_CAPACITY];
    M12_AssetStatus status;
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    ZipEntry entries[2];

    memset(entries, 0, sizeof(entries));
    entries[0].name = kGraphicsEntry;
    entries[0].payload = kGraphicsPayload;
    entries[0].payloadSize = sizeof(kGraphicsPayload) - 1U;
    entries[1].name = kDungeonEntry;
    entries[1].payload = kDungeonPayload;
    entries[1].payloadSize = sizeof(kDungeonPayload) - 1U;

    if (!make_isolated_home(home, sizeof(home)) ||
        !FSP_JoinPath(dataRoot, sizeof(dataRoot), home, "configured-data") ||
        !FSP_CreateDirectoryRecursive(dataRoot) ||
        !FSP_JoinPath(zipPath, sizeof(zipPath), dataRoot, kZipName) ||
        !FSP_JoinPath(graphicsPath, sizeof(graphicsPath), home, "graphics.bin") ||
        !FSP_JoinPath(dungeonPath, sizeof(dungeonPath), home, "dungeon.bin") ||
        !write_payload_file(graphicsPath, kGraphicsPayload, sizeof(kGraphicsPayload) - 1U) ||
        !write_payload_file(dungeonPath, kDungeonPayload, sizeof(kDungeonPayload) - 1U) ||
        !m12_file_md5_hex(graphicsPath, graphicsMd5) ||
        !m12_file_md5_hex(dungeonPath, dungeonMd5) ||
        !write_zip_fixture(zipPath, entries, 2U)) {
        free_zip_entries(entries, 2U);
        fprintf(stderr, "fixture setup failed\n");
        return 1;
    }

#ifdef FIRESTAFF_HAS_ZLIB
    check_int(entries[0].method == 8U && entries[0].packedSize < entries[0].payloadSize,
              "GRAPHICS fixture should use a smaller deflated ZIP entry");
    check_int(entries[1].method == 8U && entries[1].packedSize < entries[1].payloadSize,
              "DUNGEON fixture should use a smaller deflated ZIP entry");
#else
    check_int(entries[0].method == 0U && entries[0].packedSize == entries[0].payloadSize,
              "without zlib the GRAPHICS fixture should use a stored ZIP entry");
#endif
    free_zip_entries(entries, 2U);

    if (!test_setenv("HOME", home) ||
        !test_setenv("FIRESTAFF_DATA", dataRoot) ||
        !test_setenv("XDG_DATA_HOME", home) ||
        !test_setenv("APPDATA", home)) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(dataRoot, graphicsMd5, foundPath,
                                (int)sizeof(foundPath), 4),
              "scanner should find renamed DM1 GRAPHICS payload by hash");
    check_int(path_has_virtual_entry(foundPath, kZipName, kGraphicsEntry),
              "GRAPHICS receipt should preserve the archive virtual path");

    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(dataRoot, dungeonMd5, foundPath,
                                (int)sizeof(foundPath), 4),
              "scanner should find renamed DM1 DUNGEON payload by hash");
    check_int(path_has_virtual_entry(foundPath, kZipName, kDungeonEntry),
              "DUNGEON receipt should preserve the archive virtual path");

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);
    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_Scan(&status, dataRoot);

    version = M12_AssetStatus_GetVersion(&status, "dm1", 0U);
    graphics = required_file_by_role(&status, "graphics");
    dungeon = required_file_by_role(&status, "dungeon");

    check_int(M12_AssetStatus_FindVersionIndex("dm1", "pc34-en") == 0,
              "DM1 PC 3.4 English should remain version slot 0");
    check_int(version && version->matched &&
                  version->versionId && strcmp(version->versionId, "pc34-en") == 0,
              "DM1 PC 3.4 English version should be matched");
    check_int(version && strcmp(version->matchedMd5, graphicsMd5) == 0,
              "DM1 PC 3.4 English version receipt should carry the GRAPHICS hash");
    check_int(version && path_has_virtual_entry(version->matchedPath, kZipName, kGraphicsEntry),
              "version receipt should keep the original virtual GRAPHICS path");
    check_int(M12_AssetStatus_GameAvailable(&status, "dm1") == 1,
              "DM1 should be available only after GRAPHICS and DUNGEON hashes match");
    check_int(M12_AssetStatus_GetRequiredFileCount(&status, "dm1") == 2U,
              "DM1 should still require exactly GRAPHICS.DAT and DUNGEON.DAT");

    check_int(FSP_GetUserDataDir(userDataDir, sizeof(userDataDir)) &&
              FSP_JoinPath(cacheRoot, sizeof(cacheRoot), userDataDir, "asset-cache"),
              "asset cache root should resolve under isolated user data");
    check_int(FSP_JoinPath(cachedGraphics, sizeof(cachedGraphics), cacheRoot,
                           "dm1/GRAPHICS.DAT") &&
              FSP_JoinPath(cachedDungeon, sizeof(cachedDungeon), cacheRoot,
                           "dm1/DUNGEON.DAT"),
              "expected DM1 cached DAT paths should resolve");
    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm1"),
                     cacheRoot) == 0,
              "DM1 runtime data root should point at the ordinary asset cache");

    check_int(graphics && graphics->matched &&
                  strcmp(graphics->matchedHash, graphicsMd5) == 0 &&
                  path_has_cache_leaf(graphics->matchedPath, cacheRoot,
                                      "dm1", "GRAPHICS.DAT"),
              "required GRAPHICS receipt should be materialized to dm1/GRAPHICS.DAT");
    check_int(dungeon && dungeon->matched &&
                  strcmp(dungeon->matchedHash, dungeonMd5) == 0 &&
                  path_has_cache_leaf(dungeon->matchedPath, cacheRoot,
                                      "dm1", "DUNGEON.DAT"),
              "required DUNGEON receipt should be materialized to dm1/DUNGEON.DAT");
    check_int(graphics && strcmp(graphics->label, "GRAPHICS.DAT") == 0,
              "materialized graphics leaf should come from the required-file label");
    check_int(dungeon && strcmp(dungeon->label, "DUNGEON.DAT") == 0,
              "materialized dungeon leaf should come from the required-file label");
    check_int(file_matches_payload(cachedGraphics, kGraphicsPayload,
                                   sizeof(kGraphicsPayload) - 1U),
              "cached GRAPHICS.DAT should contain the hash-matched ZIP payload");
    check_int(file_matches_payload(cachedDungeon, kDungeonPayload,
                                   sizeof(kDungeonPayload) - 1U),
              "cached DUNGEON.DAT should contain the hash-matched ZIP payload");

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(NULL, NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);
    if (failures) {
        fprintf(stderr, "%d failure(s), assertions=%d\n", failures, assertions);
        return 1;
    }
    printf("ok: DM1 PC 3.4 archive media receipt hashes materialize GRAPHICS/DUNGEON (%d assertions)\n",
           assertions);
    return 0;
}
