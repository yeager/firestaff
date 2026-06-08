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
    unsigned char* compressed;
    size_t compressedSize;
    unsigned int method;
    unsigned int localOffset;
} ZipEntryFixture;

static int failures;

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
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_asset_status_zip_cache_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return FSP_CreateDirectoryRecursive(out);
#else
    char templatePath[] = "/tmp/firestaff-asset-status-zip-cache-XXXXXX";
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
                                  const char* zipName,
                                  const char* entryName) {
    return path && strstr(path, zipName) && strstr(path, "::") &&
           strstr(path, entryName);
}

static int path_has_cache_leaf(const char* path,
                               const char* cacheRoot,
                               const char* gameId,
                               const char* leaf) {
    return path && strstr(path, cacheRoot) && strstr(path, gameId) &&
           strstr(path, leaf) && !strstr(path, "::");
}

static int prepare_entry_payload(ZipEntryFixture* entry) {
#ifdef FIRESTAFF_HAS_ZLIB
    z_stream zs;
    size_t bound;
    int ret;
    if (!entry) {
        return 0;
    }
    bound = (size_t)compressBound((uLong)entry->payloadSize);
    entry->compressed = (unsigned char*)malloc(bound);
    if (!entry->compressed) {
        return 0;
    }
    memset(&zs, 0, sizeof(zs));
    ret = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                       -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        free(entry->compressed);
        entry->compressed = NULL;
        return 0;
    }
    zs.next_in = (Bytef*)entry->payload;
    zs.avail_in = (uInt)entry->payloadSize;
    zs.next_out = entry->compressed;
    zs.avail_out = (uInt)bound;
    ret = deflate(&zs, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&zs);
        free(entry->compressed);
        entry->compressed = NULL;
        return 0;
    }
    entry->compressedSize = (size_t)zs.total_out;
    deflateEnd(&zs);
    entry->method = 8U;
    return 1;
#else
    if (!entry) {
        return 0;
    }
    entry->compressed = NULL;
    entry->compressedSize = entry->payloadSize;
    entry->method = 0U;
    return 1;
#endif
}

static int write_zip_entry_local(FILE* fp, ZipEntryFixture* entry) {
    unsigned char local[30] = {0};
    unsigned int nameLen;
    const unsigned char* bytes;
    size_t byteCount;
    if (!fp || !entry) {
        return 0;
    }
    entry->localOffset = (unsigned int)ftell(fp);
    nameLen = (unsigned int)strlen(entry->name);
    bytes = entry->method == 8U ? entry->compressed : entry->payload;
    byteCount = entry->compressedSize;
    put32(local, 0x04034b50U);
    put16(local + 4, 20U);
    put16(local + 8, entry->method);
    put32(local + 18, (unsigned int)byteCount);
    put32(local + 22, (unsigned int)entry->payloadSize);
    put16(local + 26, nameLen);
    return fwrite(local, 1U, sizeof(local), fp) == sizeof(local) &&
           fwrite(entry->name, 1U, nameLen, fp) == nameLen &&
           fwrite(bytes, 1U, byteCount, fp) == byteCount;
}

static int write_zip_entry_central(FILE* fp, const ZipEntryFixture* entry) {
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
    put32(central + 20, (unsigned int)entry->compressedSize);
    put32(central + 24, (unsigned int)entry->payloadSize);
    put16(central + 28, nameLen);
    put32(central + 42, entry->localOffset);
    return fwrite(central, 1U, sizeof(central), fp) == sizeof(central) &&
           fwrite(entry->name, 1U, nameLen, fp) == nameLen;
}

static int write_zip_fixture(const char* path, ZipEntryFixture entries[], size_t count) {
    FILE* fp = fopen(path, "wb");
    unsigned char eocd[22] = {0};
    unsigned int centralOffset;
    unsigned int centralSize;
    size_t i;
    if (!fp) {
        return 0;
    }
    for (i = 0U; i < count; ++i) {
        if (!prepare_entry_payload(&entries[i]) ||
            !write_zip_entry_local(fp, &entries[i])) {
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
    if (fwrite(eocd, 1U, sizeof(eocd), fp) != sizeof(eocd)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void free_zip_entries(ZipEntryFixture entries[], size_t count) {
    size_t i;
    for (i = 0U; i < count; ++i) {
        free(entries[i].compressed);
        entries[i].compressed = NULL;
    }
}

int main(void) {
    static const unsigned char graphicsPayload[] =
        "synthetic DM2 graphics payload for archive cache regression\n";
    static const unsigned char dungeonPayload[] =
        "synthetic DM2 dungeon payload for archive cache regression\n";
    char home[M12_ASSET_DATA_DIR_CAPACITY];
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char zipPath[M12_ASSET_DATA_DIR_CAPACITY];
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
    ZipEntryFixture entries[2];

    memset(entries, 0, sizeof(entries));
    entries[0].name = "renamed/inside/DM2GRAPHICS.RENAMED";
    entries[0].payload = graphicsPayload;
    entries[0].payloadSize = sizeof(graphicsPayload) - 1U;
    entries[1].name = "renamed/inside/DM2DUNGEON.RENAMED";
    entries[1].payload = dungeonPayload;
    entries[1].payloadSize = sizeof(dungeonPayload) - 1U;

    if (!make_isolated_home(home, sizeof(home)) ||
        !FSP_JoinPath(dataRoot, sizeof(dataRoot), home, "configured-data") ||
        !FSP_CreateDirectoryRecursive(dataRoot) ||
        !FSP_JoinPath(graphicsPath, sizeof(graphicsPath), home, "graphics.bin") ||
        !FSP_JoinPath(dungeonPath, sizeof(dungeonPath), home, "dungeon.bin") ||
        !FSP_JoinPath(zipPath, sizeof(zipPath), dataRoot, "dm2-required.zip") ||
        !write_payload_file(graphicsPath, graphicsPayload, sizeof(graphicsPayload) - 1U) ||
        !write_payload_file(dungeonPath, dungeonPayload, sizeof(dungeonPayload) - 1U) ||
        !m12_file_md5_hex(graphicsPath, graphicsMd5) ||
        !m12_file_md5_hex(dungeonPath, dungeonMd5) ||
        !write_zip_fixture(zipPath, entries, 2U)) {
        free_zip_entries(entries, 2U);
        fprintf(stderr, "fixture setup failed\n");
        return 1;
    }
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
              "scanner should find ZIP-backed graphics hash");
    check_int(path_has_virtual_entry(foundPath, "dm2-required.zip",
                                     "renamed/inside/DM2GRAPHICS.RENAMED"),
              "graphics match should be reported as a ZIP virtual path");

    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(dataRoot, dungeonMd5, foundPath,
                                (int)sizeof(foundPath), 4),
              "scanner should find ZIP-backed dungeon hash");
    check_int(path_has_virtual_entry(foundPath, "dm2-required.zip",
                                     "renamed/inside/DM2DUNGEON.RENAMED"),
              "dungeon match should be reported as a ZIP virtual path");

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);

    M12_AssetStatus_Scan(&status, dataRoot);

    check_int(M12_AssetStatus_GameAvailable(&status, "dm2"),
              "DM2 should be available when both required hashes are ZIP-backed");
    version = M12_AssetStatus_GetVersion(&status, "dm2", 0U);
    check_int(version && version->matched &&
              path_has_virtual_entry(version->matchedPath, "dm2-required.zip",
                                     "renamed/inside/DM2GRAPHICS.RENAMED"),
              "version match should preserve the original ZIP virtual path");

    check_int(FSP_GetUserDataDir(userDataDir, sizeof(userDataDir)) &&
              FSP_JoinPath(cacheRoot, sizeof(cacheRoot), userDataDir, "asset-cache"),
              "asset cache root should resolve");
    check_int(FSP_JoinPath(cachedGraphics, sizeof(cachedGraphics),
                           cacheRoot, "dm2/GRAPHICS.DAT") &&
              FSP_JoinPath(cachedDungeon, sizeof(cachedDungeon),
                           cacheRoot, "dm2/DUNGEON.DAT"),
              "expected cached DM2 paths should resolve");
    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm2"),
                     cacheRoot) == 0,
              "DM2 runtime data root should point at the asset cache root");

    required = M12_AssetStatus_GetRequiredFile(&status, "dm2", 0U);
    check_int(required && required->matched &&
              path_has_cache_leaf(required->matchedPath, cacheRoot,
                                  "dm2", "GRAPHICS.DAT"),
              "required graphics path should be materialized into cache");
    required = M12_AssetStatus_GetRequiredFile(&status, "dm2", 1U);
    check_int(required && required->matched &&
              path_has_cache_leaf(required->matchedPath, cacheRoot,
                                  "dm2", "DUNGEON.DAT"),
              "required dungeon path should be materialized into cache");
    check_int(file_matches_payload(cachedGraphics, graphicsPayload,
                                   sizeof(graphicsPayload) - 1U),
              "cached GRAPHICS.DAT should contain the ZIP entry payload");
    check_int(file_matches_payload(cachedDungeon, dungeonPayload,
                                   sizeof(dungeonPayload) - 1U),
              "cached DUNGEON.DAT should contain the ZIP entry payload");

    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);
    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
#ifdef FIRESTAFF_HAS_ZLIB
    puts("ok: ZIP deflate virtual required files materialize into asset cache");
#else
    puts("ok: ZIP stored virtual required files materialize into asset cache");
#endif
    return 0;
}
