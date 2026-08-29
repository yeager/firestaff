/*
 * tests/test_asset_status_dm1_zip_required_materialization.c
 *
 * Focused asset-scanner / direct-RAM-reader regression for a nested,
 * deflated ZIP entry.
 *
 * What this gate proves:
 *   1. The recursive hash scanner walks a ZIP and recognizes a DEFLATED
 *      (method=8) entry whose name contains multiple path separators
 *      ("nested/inner/level1/level2/GRAPHICS.DAT"). It must report
 *      the match as a virtual container path of the form
 *      "<archive>.zip::<nested/inner/level1/level2/GRAPHICS.DAT>".
 *   2. `asset_read_path_alloc()` inflates that virtual member directly into
 *      bounded RAM without creating game-data files on disk.
 *   3. M12 preserves the original ZIP as its runtime owner and preserves
 *      every required asset as a virtual member path.
 *
 * The fixture packs two entries in one ZIP so DM1 (which has two
 * required-files rows, graphics + dungeon) can be reported available:
 *   - graphics at the deeply-nested path (the headline regression),
 *   - dungeon at a flat path (so DM1's hash-pinned required-files row
 *     is also matched and the version reports available).
 * When zlib is available both entries are method=8; when zlib is not
 * available the test falls back to method=0 (stored) for both so the
 * direct-reader contract still gets exercised end-to-end.
 *
 * Source-locked against the existing asset-loader module:
 *   - src/shared/asset_find_by_hash.c
 *       * zip_stored_entry_md5 / zip_deflated_entry_md5 (md5 over the
 *         uncompressed entry, gated by FIRESTAFF_HAS_ZLIB)
 *       * scan_zip_by_md5 (walks central directory, picks the best
 *         entry name that hashes to the requested MD5)
 *       * asset_read_path_alloc (the bounded virtual-member reader)
 *   - src/shared/asset_status_m12.c
 *       * m12_path_is_virtual_asset (detects "::" in matched paths)
 *       * m12_publish_source_runtime_root (original container handoff)
 *
 * Test is data-free: it synthesizes its own ZIP and uses the testing-only
 * M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes helper to
 * register the synthesized payload MD5 as a canonical DM1 graphics hash.
 */

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
    int rc = snprintf(out, outSize, ".\\firestaff_zip_nested_deflate_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return FSP_CreateDirectoryRecursive(out);
#else
    char templatePath[] = "./firestaff-zip-nested-deflate-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

/* Build deterministic, slightly compressible payloads so deflate actually
 * shrinks the bytes (verifies we are exercising the inflate path, not just
 * the stored-entry fallback). The "DM1 graphics fixture" / "DM1 dungeon
 * fixture" prefixes match the rest of the M12 synthetic-hash suite so the
 * payloads look like real required-file content from the scanner's
 * perspective. */
static const unsigned char kGraphicsPayload[] =
    "Firestaff DM1 graphics fixture - nested deflate entry payload v1\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n";

static const unsigned char kDungeonPayload[] =
    "Firestaff DM1 dungeon fixture - single deflate entry payload v1\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n";

static const unsigned char kTitlePayload[] =
    "Firestaff DM1 optional TITLE fixture - startup cache payload v1\n"
    "TITLETITLETITLETITLETITLETITLETITLETITLETITLETITLETITLETITLETITLE\n";

static const unsigned char kSwooshPayload[] =
    "Firestaff DM1 optional SWOOSH fixture - startup cache payload v1\n"
    "SWSHSWSHSWSHSWSHSWSHSWSHSWSHSWSHSWSHSWSHSWSHSWSHSWSHSWSHSWSH\n";

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

/* Compare a file on disk to an in-memory payload. Reads the file fully and
 * compares byte-for-byte. */
static int virtual_path_matches_payload(const char* path,
                                const unsigned char* payload,
                                size_t payloadSize) {
    uint8_t* bytes = NULL;
    size_t byteCount = 0U;
    int match = asset_read_path_alloc(path, &bytes, &byteCount) && bytes &&
                byteCount == payloadSize &&
                memcmp(bytes, payload, payloadSize) == 0;
    free(bytes);
    return match;
}

static int path_has_virtual_entry(const char* path,
                                  const char* zipName,
                                  const char* entryName) {
    return path && strstr(path, zipName) && strstr(path, "::") &&
           strstr(path, entryName);
}

/* Internal: deflate a payload into a freshly malloc()'d buffer and return
 * it through *outCompressed / *outSize. The buffer is owned by the caller
 * and must be released with free(). Returns 1 on success, 0 otherwise.
 *
 * When zlib is unavailable the function falls back to copying the raw
 * payload bytes so the test still exercises the cache-materialization
 * path. Whether the entry is stored or deflated is incidental when zlib
 * is absent; the contract being verified is the cache handoff itself. */
static int deflate_payload_owned(const unsigned char* payload,
                                size_t payloadSize,
                                unsigned char** outCompressed,
                                size_t* outSize) {
#ifdef FIRESTAFF_HAS_ZLIB
    z_stream zs;
    int ret;
    size_t bound;
    unsigned char* buf;
    if (!payload || !outCompressed || !outSize || payloadSize == 0U) {
        return 0;
    }
    bound = (size_t)compressBound((uLong)payloadSize);
    buf = (unsigned char*)malloc(bound ? bound : 1U);
    if (!buf) {
        return 0;
    }
    memset(&zs, 0, sizeof(zs));
    ret = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                       -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        free(buf);
        return 0;
    }
    zs.next_in = (Bytef*)payload;
    zs.avail_in = (uInt)payloadSize;
    zs.next_out = buf;
    zs.avail_out = (uInt)bound;
    ret = deflate(&zs, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&zs);
        free(buf);
        return 0;
    }
    *outSize = (size_t)zs.total_out;
    *outCompressed = buf;
    return deflateEnd(&zs) == Z_OK;
#else
    unsigned char* buf;
    if (!payload || !outCompressed || !outSize || payloadSize == 0U) {
        return 0;
    }
    buf = (unsigned char*)malloc(payloadSize);
    if (!buf) {
        return 0;
    }
    memcpy(buf, payload, payloadSize);
    *outSize = payloadSize;
    *outCompressed = buf;
    return 1;
#endif
}

typedef struct TestZipEntry {
    const char* name;
    const unsigned char* payload;
    size_t payloadSize;
    unsigned char* compressed;
    size_t compressedSize;
    unsigned int localOffset;
    unsigned int centralOffset;
} TestZipEntry;

/* Build a multi-entry ZIP archive with DEFLATED (method=8) entries.
 * Payloads are deflated independently and the central directory is
 * laid out after all local file headers, matching the PKZIP spec. Returns
 * 1 on success. The graphics entry MUST be first so the cached files end up
 * in a deterministic order.
 *
 * When zlib is unavailable, the fallback path copies the raw payload
 * to 0 (stored) so the asset scanner can still read the entry. The
 * "nested deflate" headline contract is verified when zlib is present;
 * the stored-entry path is a graceful degradation that keeps the
 * cache-materialization regression exercising end-to-end. */
static int write_dm1_startup_nested_zip(const char* path,
                                        TestZipEntry* entries,
                                        size_t entryCount,
                                        unsigned int* outEntry1Compressed,
                                        unsigned int* outEntry2Compressed) {
    FILE* fp = fopen(path, "wb");
    unsigned char local[30] = {0};
    unsigned char central[46] = {0};
    unsigned char eocd[22] = {0};
    unsigned int centralOffset;
    unsigned int centralEnd;
    size_t i;
#ifdef FIRESTAFF_HAS_ZLIB
    int method = 8;
#else
    int method = 0;
#endif
    if (!fp || !entries || entryCount == 0U) {
        if (fp) fclose(fp);
        return 0;
    }
    for (i = 0U; i < entryCount; ++i) {
        if (!deflate_payload_owned(entries[i].payload,
                                   entries[i].payloadSize,
                                   &entries[i].compressed,
                                   &entries[i].compressedSize)) {
            size_t j;
            for (j = 0U; j <= i && j < entryCount; ++j) {
                free(entries[j].compressed);
                entries[j].compressed = NULL;
            }
            fclose(fp);
            return 0;
        }
    }

    for (i = 0U; i < entryCount; ++i) {
        unsigned int nameLen = (unsigned int)strlen(entries[i].name);
        entries[i].localOffset = (unsigned int)ftell(fp);
        memset(local, 0, sizeof(local));
        put32(local, 0x04034b50U);
        put16(local + 4, 20U);
        put16(local + 8, (unsigned int)method);
        put32(local + 18, (unsigned int)entries[i].compressedSize);
        put32(local + 22, (unsigned int)entries[i].payloadSize);
        put16(local + 26, nameLen);
        if (fwrite(local, 1U, sizeof(local), fp) != sizeof(local) ||
            fwrite(entries[i].name, 1U, nameLen, fp) != nameLen ||
            fwrite(entries[i].compressed, 1U, entries[i].compressedSize, fp) !=
                entries[i].compressedSize) {
            size_t j;
            for (j = 0U; j < entryCount; ++j) free(entries[j].compressed);
            fclose(fp);
            return 0;
        }
    }

    centralOffset = (unsigned int)ftell(fp);
    for (i = 0U; i < entryCount; ++i) {
        unsigned int nameLen = (unsigned int)strlen(entries[i].name);
        entries[i].centralOffset = (unsigned int)ftell(fp);
        memset(central, 0, sizeof(central));
        put32(central, 0x02014b50U);
        put16(central + 4, 20U);
        put16(central + 6, 20U);
        put16(central + 10, (unsigned int)method);
        put32(central + 20, (unsigned int)entries[i].compressedSize);
        put32(central + 24, (unsigned int)entries[i].payloadSize);
        put16(central + 28, nameLen);
        put32(central + 42, entries[i].localOffset);
        if (fwrite(central, 1U, sizeof(central), fp) != sizeof(central) ||
            fwrite(entries[i].name, 1U, nameLen, fp) != nameLen) {
            size_t j;
            for (j = 0U; j < entryCount; ++j) free(entries[j].compressed);
            fclose(fp);
            return 0;
        }
    }

    centralEnd = (unsigned int)ftell(fp);
    put32(eocd, 0x06054b50U);
    put16(eocd + 8, (unsigned int)entryCount);
    put16(eocd + 10, (unsigned int)entryCount);
    put32(eocd + 12, centralEnd - centralOffset);
    put32(eocd + 16, centralOffset);
    if (fwrite(eocd, 1U, sizeof(eocd), fp) != sizeof(eocd)) {
        for (i = 0U; i < entryCount; ++i) free(entries[i].compressed);
        fclose(fp);
        return 0;
    }
    if (outEntry1Compressed) {
        *outEntry1Compressed = (unsigned int)entries[0].compressedSize;
    }
    if (outEntry2Compressed && entryCount > 1U) {
        *outEntry2Compressed = (unsigned int)entries[1].compressedSize;
    }
    for (i = 0U; i < entryCount; ++i) free(entries[i].compressed);
    return fclose(fp) == 0;
}

int main(void) {
    /* Two deflated entries in one ZIP:
     *   - graphics: deeply nested path (4 segments) so the regression
     *     specifically exercises the nested-name materialization path.
     *   - dungeon:  a flat path so DM1's hash-pinned required-file row
     *     can also be matched (DM1 only reports available when ALL
     *     required files match).
     * Both entries use method=8 (deflate) so the inflate code in
     * asset_find_by_hash / asset_extract_virtual_path / M12 cache
     * materialization is the actual code path under test. */
    static const char kZipName[] = "dm1-required.zip";
    static const char kGraphicsEntry[] =
        "nested/inner/level1/level2/GRAPHICS.DAT";
    static const char kDungeonEntry[] =
        "DUNGEON.DAT";
    static const char kTitleEntry[] =
        "nested/inner/level1/TITLE";
    static const char kSwooshEntry[] =
        "nested/inner/level1/SWOOSH";

    char home[M12_ASSET_DATA_DIR_CAPACITY];
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char zipPath[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsPath[M12_ASSET_DATA_DIR_CAPACITY];
    char dungeonPath[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];
    char foundPath[ASSET_PATH_MAX];
    char titlePath[ASSET_PATH_MAX];
    char swooshPath[ASSET_PATH_MAX];
    char firestaffDir[M12_ASSET_DATA_DIR_CAPACITY];
    char scanCacheDir[M12_ASSET_DATA_DIR_CAPACITY];
    char scanCachePath[M12_ASSET_DATA_DIR_CAPACITY];
    M12_AssetStatus status;
    const M12_AssetRequiredFileStatus* required;
    const M12_AssetVersionStatus* version;
    unsigned int graphicsCompressed = 0U;
    unsigned int dungeonCompressed = 0U;
    size_t graphicsSize = sizeof(kGraphicsPayload) - 1U;
    size_t dungeonSize = sizeof(kDungeonPayload) - 1U;
    size_t titleSize = sizeof(kTitlePayload) - 1U;
    size_t swooshSize = sizeof(kSwooshPayload) - 1U;
    TestZipEntry entries[4];

    /* Stage an isolated $HOME so the asset cache resolves under our temp
     * directory instead of touching the user's real ~/.firestaff. */
    if (!make_isolated_home(home, sizeof(home)) ||
        !FSP_JoinPath(dataRoot, sizeof(dataRoot), home, "configured-data") ||
        !FSP_CreateDirectoryRecursive(dataRoot) ||
        !FSP_JoinPath(zipPath, sizeof(zipPath), dataRoot, kZipName) ||
        !FSP_JoinPath(graphicsPath, sizeof(graphicsPath), home,
                       "graphics.bin") ||
        !FSP_JoinPath(dungeonPath, sizeof(dungeonPath), home,
                       "dungeon.bin") ||
        !write_payload_file(graphicsPath, kGraphicsPayload, graphicsSize) ||
        !write_payload_file(dungeonPath, kDungeonPayload, dungeonSize) ||
        !m12_file_md5_hex(graphicsPath, graphicsMd5) ||
        !m12_file_md5_hex(dungeonPath, dungeonMd5)) {
        fprintf(stderr, "fixture setup failed\n");
        return 1;
    }
    memset(entries, 0, sizeof(entries));
    entries[0].name = kGraphicsEntry;
    entries[0].payload = kGraphicsPayload;
    entries[0].payloadSize = graphicsSize;
    entries[1].name = kDungeonEntry;
    entries[1].payload = kDungeonPayload;
    entries[1].payloadSize = dungeonSize;
    entries[2].name = kTitleEntry;
    entries[2].payload = kTitlePayload;
    entries[2].payloadSize = titleSize;
    entries[3].name = kSwooshEntry;
    entries[3].payload = kSwooshPayload;
    entries[3].payloadSize = swooshSize;
    if (!write_dm1_startup_nested_zip(zipPath,
                                      entries,
                                      sizeof(entries) / sizeof(entries[0]),
                                      &graphicsCompressed,
                                      &dungeonCompressed)) {
        fprintf(stderr, "zip fixture setup failed\n");
        return 1;
    }
#ifdef FIRESTAFF_HAS_ZLIB
    check_int(graphicsCompressed > 0U && graphicsCompressed < (unsigned int)graphicsSize,
              "graphics payload must deflate to a smaller-than-original byte stream "
              "(verifies we exercised the inflate path, not a stored fallback)");
    check_int(dungeonCompressed > 0U && dungeonCompressed < (unsigned int)dungeonSize,
              "dungeon payload must deflate to a smaller-than-original byte stream");
#else
    check_int(graphicsCompressed == (unsigned int)graphicsSize,
              "without zlib the test falls back to a stored entry of identical size");
#endif

    /* Sanity: the on-disk payload must round-trip MD5 so the scanner has a
     * known target hash to match against. */
    {
        char rereadMd5[M12_ASSET_MD5_CAPACITY];
        check_int(m12_file_md5_hex(graphicsPath, rereadMd5) &&
                  strcmp(rereadMd5, graphicsMd5) == 0,
                  "MD5 of staged graphics payload must be stable across reads");
        check_int(m12_file_md5_hex(dungeonPath, rereadMd5) &&
                  strcmp(rereadMd5, dungeonMd5) == 0,
                  "MD5 of staged dungeon payload must be stable across reads");
    }

    /* Make sure FSP_GetUserDataDir() and M12_AssetStatus_Scan() agree on
     * $HOME. The cache root resolves to <userDataDir>/asset-cache. */
    if (!test_setenv("HOME", home) ||
        !test_setenv("FIRESTAFF_DATA", dataRoot) ||
        !test_setenv("XDG_DATA_HOME", home) ||
        !test_setenv("APPDATA", home)) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    /* Layer 1: recursive hash scanner.
     * asset_find_by_md5 must find the entry, recognize that it lives
     * inside a ZIP container, and report the match as a virtual path of
     * the form "<zip>::<nested/.../ENTRY>". */
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(dataRoot, graphicsMd5, foundPath,
                                (int)sizeof(foundPath), 4),
              "scanner should find the nested deflated ZIP entry by MD5");
    check_int(path_has_virtual_entry(foundPath, kZipName, kGraphicsEntry),
              "scanner should report the match as a ZIP virtual path "
              "including the nested entry name");

    /* The virtual path must keep every path segment of the entry name,
     * including the nested prefix. We verify that explicitly so a future
     * refactor that flattens the path on output would fail the test. */
    {
        const char* nestedNeedle =
            "nested/inner/level1/level2/GRAPHICS.DAT";
        check_int(strstr(foundPath, nestedNeedle) != NULL,
                  "scanner should preserve every segment of the nested "
                  "entry name in the virtual path");
    }

    /* Layer 2: launch admission keeps the verified ZIP members intact.
     * Nested DEFLATE data is read through the bounded RAM reader; normal
     * Firestaff builds must not create a flattened asset-cache game tree. */
    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);

    M12_AssetStatus_Scan(&status, dataRoot);

    check_int(M12_AssetStatus_GameAvailable(&status, "dm1"),
              "DM1 should be available when both required hashes are "
              "matched by ZIP-backed deflated entries");
    /* FM Towns EN/JP occupy the first two catalogue slots.  Resolve the
     * multilanguage PC profile by identity so this provenance assertion does
     * not silently follow catalogue reordering. */
    version = M12_AssetStatus_GetVersion(
        &status, "dm1",
        (size_t)M12_AssetStatus_FindVersionIndex("dm1", "pc34-multi"));
    check_int(version && version->matched &&
              path_has_virtual_entry(version->matchedPath, kZipName,
                                     kGraphicsEntry),
              "M12 version match should preserve the original nested ZIP "
              "virtual path (before materialization)");

    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm1"),
                     zipPath) == 0,
              "DM1 runtime data root should remain the original ZIP container");

    required = M12_AssetStatus_GetRequiredFile(&status, "dm1", 0U);
    check_int(required && required->matched,
              "DM1 graphics required file should be matched after scan");
    check_int(path_has_virtual_entry(required->matchedPath, kZipName,
                                     kGraphicsEntry),
              "DM1 graphics required file should retain its nested ZIP member");
    check_int(virtual_path_matches_payload(required->matchedPath,
                                           kGraphicsPayload, graphicsSize),
              "nested GRAPHICS payload should be read directly into RAM");

    required = M12_AssetStatus_GetRequiredFile(&status, "dm1", 1U);
    check_int(required && required->matched &&
              path_has_virtual_entry(required->matchedPath, kZipName,
                                     kDungeonEntry),
              "DM1 dungeon required file should retain its ZIP member");
    check_int(required && virtual_path_matches_payload(required->matchedPath,
                                                       kDungeonPayload,
                                                       dungeonSize),
              "DUNGEON payload should be read directly into RAM");
    check_int(snprintf(titlePath, sizeof(titlePath), "%s::%s", zipPath,
                       kTitleEntry) > 0 &&
              virtual_path_matches_payload(titlePath, kTitlePayload, titleSize),
              "TITLE sibling should be read directly into RAM");
    check_int(snprintf(swooshPath, sizeof(swooshPath), "%s::%s", zipPath,
                       kSwooshEntry) > 0 &&
              virtual_path_matches_payload(swooshPath, kSwooshPayload, swooshSize),
              "SWOOSH sibling should be read directly into RAM");

    /* A repeat scan must preserve the ZIP-backed owner and bytes. */
    M12_AssetStatus_Scan(&status, dataRoot);
    check_int(M12_AssetStatus_GameAvailable(&status, "dm1"),
              "re-scanning the original ZIP must retain DM1 availability");
    check_int(virtual_path_matches_payload(foundPath, kGraphicsPayload,
                                           graphicsSize),
              "re-scanning must not change the nested GRAPHICS payload");

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);
    (void)remove(zipPath);
    (void)remove(graphicsPath);
    (void)remove(dungeonPath);
    (void)remove(dataRoot);
    if (FSP_JoinPath(firestaffDir, sizeof(firestaffDir), home, ".firestaff") &&
        FSP_JoinPath(scanCacheDir, sizeof(scanCacheDir), firestaffDir, "cache") &&
        FSP_JoinPath(scanCachePath, sizeof(scanCachePath), scanCacheDir,
                     "asset_scan_cache.dat")) {
        (void)remove(scanCachePath);
        (void)remove(scanCacheDir);
        (void)remove(firestaffDir);
    }
    (void)remove(home);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
#ifdef FIRESTAFF_HAS_ZLIB
    puts("ok: nested deflated ZIP entry virtual path + direct RAM reader");
#else
    puts("ok: nested ZIP entry virtual path + direct RAM reader (stored)");
#endif
    return 0;
}
