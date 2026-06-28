/*
 * tests/test_asset_status_zip_cache_nested_deflate.c
 *
 * Focused asset-scanner / asset-cache regression for a NESTED DEFLATED
 * ZIP entry materialized into the Firestaff asset cache.
 *
 * What this gate proves:
 *   1. The recursive hash scanner walks a ZIP and recognizes a DEFLATED
 *      (method=8) entry whose name contains multiple path separators
 *      ("nested/inner/level1/level2/DM2GRAPHICS.DAT"). It must report
 *      the match as a virtual container path of the form
 *      "<archive>.zip::<nested/inner/level1/level2/DM2GRAPHICS.DAT>".
 *   2. The low-level `asset_extract_virtual_path()` helper, given the
 *      virtual path, inflates the deflate stream and writes a byte-
 *      identical ordinary file on disk.
 *   3. The M12 launch-time cache materialization rewrites the matched
 *      path of every required file to an ORDINARY file path inside
 *      the Firestaff asset cache
 *      (<userDataDir>/asset-cache/<gameId>/<label>), so the runtime no
 *      longer needs to understand virtual container paths.
 *   4. The DM2 launch handoff path can open the materialized files via
 *      <runtimeDataDir>/dm2/GRAPHICS.DAT and
 *      <runtimeDataDir>/dm2/DUNGEON.DAT, matching the M11 DM2 scan root.
 *
 * The fixture packs two entries in one ZIP so DM2 (which has two
 * required-files rows, graphics + dungeon) can be reported available:
 *   - graphics at the deeply-nested path (the headline regression),
 *   - dungeon at a flat path (so DM2's hash-pinned required-files row
 *     is also matched and the version reports available).
 * When zlib is available both entries are method=8; when zlib is not
 * available the test falls back to method=0 (stored) for both so the
 * cache-materialization contract still gets exercised end-to-end.
 *
 * Source-locked against the existing asset-loader module:
 *   - src/shared/asset_find_by_hash.c
 *       * zip_stored_entry_md5 / zip_deflated_entry_md5 (md5 over the
 *         uncompressed entry, gated by FIRESTAFF_HAS_ZLIB)
 *       * scan_zip_by_md5 (walks central directory, picks the best
 *         entry name that hashes to the requested MD5)
 *       * zip_deflated_entry_extract / zip_extract_entry_to_path
 *         (inflate-and-write, also gated by FIRESTAFF_HAS_ZLIB)
 *       * asset_extract_virtual_path (the public dispatcher)
 *   - src/shared/asset_status_m12.c
 *       * m12_path_is_virtual_asset (detects "::" in matched paths)
 *       * m12_materialize_required_file (virtual -> ordinary copy)
 *       * m12_materialize_runtime_cache_for_game (cache layout under
 *         <userDataDir>/asset-cache/<gameId>/<label>)
 *
 * Test is data-free: it synthesizes its own ZIP and uses the testing-only
 * M12_AssetStatus_TestSetDm2SyntheticHashes helper to register the
 * synthesized payload MD5 as the canonical DM2 graphics hash.
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
    char templatePath[] = "/tmp/firestaff-zip-nested-deflate-XXXXXX";
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
 * the stored-entry fallback). The "DM2 graphics fixture" / "DM2 dungeon
 * fixture" prefixes match the rest of the M12 synthetic-hash suite so the
 * payloads look like real required-file content from the scanner's
 * perspective. */
static const unsigned char kGraphicsPayload[] =
    "Firestaff DM2 graphics fixture - nested deflate entry payload v1\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n";

static const unsigned char kDungeonPayload[] =
    "Firestaff DM2 dungeon fixture - single deflate entry payload v1\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n";

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
static int file_matches_payload(const char* path,
                                const unsigned char* payload,
                                size_t payloadSize) {
    FILE* fp = fopen(path, "rb");
    unsigned char* buf;
    size_t n;
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
    match = (n == payloadSize) && (memcmp(buf, payload, payloadSize) == 0);
    free(buf);
    return match;
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

static int runtime_cache_file_matches_payload(
    const M12_AssetStatus* status,
    const char* gameId,
    const char* leaf,
    const unsigned char* payload,
    size_t payloadSize) {
    char gameLeaf[M12_ASSET_DATA_DIR_CAPACITY];
    char runtimePath[M12_ASSET_DATA_DIR_CAPACITY];
    const char* runtimeRoot = M12_AssetStatus_GetRuntimeDataDir(status, gameId);
    if (!runtimeRoot || runtimeRoot[0] == '\0' || !gameId || !leaf ||
        !payload) {
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

/* Build a multi-entry ZIP archive with two DEFLATED (method=8) entries.
 * Both payloads are deflated independently and the central directory is
 * laid out after all local file headers, matching the PKZIP spec. Returns
 * 1 on success. The graphics entry MUST be the first argument so the
 * cached files end up in a deterministic order.
 *
 * When zlib is unavailable, the fallback path copies the raw payload
 * bytes (no compression) and the central-directory method field is set
 * to 0 (stored) so the asset scanner can still read the entry. The
 * "nested deflate" headline contract is verified when zlib IS present;
 * the stored-entry path is a graceful degradation that keeps the
 * cache-materialization regression exercising end-to-end. */
static int write_two_entry_nested_zip(const char* path,
                                      const char* entry1Name,
                                      const unsigned char* entry1Payload,
                                      size_t entry1Size,
                                      const char* entry2Name,
                                      const unsigned char* entry2Payload,
                                      size_t entry2Size,
                                      unsigned int* outEntry1Compressed,
                                      unsigned int* outEntry2Compressed) {
    FILE* fp = fopen(path, "wb");
    unsigned char local[30] = {0};
    unsigned char central[46] = {0};
    unsigned char eocd[22] = {0};
    unsigned char* compressed1 = NULL;
    unsigned char* compressed2 = NULL;
    size_t compressed1Size = 0U;
    size_t compressed2Size = 0U;
    unsigned int name1Len, name2Len;
    unsigned int centralOffset;
    unsigned int entry1LocalOffset = 0U;
    unsigned int entry2LocalOffset = 0U;
    unsigned int entry2CentralOffset = 0U;
#ifdef FIRESTAFF_HAS_ZLIB
    int method = 8;
#else
    int method = 0;
#endif
    if (!fp) {
        return 0;
    }
    if (!deflate_payload_owned(entry1Payload, entry1Size,
                               &compressed1, &compressed1Size) ||
        !deflate_payload_owned(entry2Payload, entry2Size,
                               &compressed2, &compressed2Size)) {
        free(compressed1);
        free(compressed2);
        fclose(fp);
        return 0;
    }
    name1Len = (unsigned int)strlen(entry1Name);
    name2Len = (unsigned int)strlen(entry2Name);

    /* Local file headers + compressed payloads. */
    entry1LocalOffset = (unsigned int)ftell(fp);
    memset(local, 0, sizeof(local));
    put32(local, 0x04034b50U);
    put16(local + 4, 20U);
    put16(local + 8, (unsigned int)method);
    put32(local + 18, (unsigned int)compressed1Size);
    put32(local + 22, (unsigned int)entry1Size);
    put16(local + 26, name1Len);
    if (fwrite(local, 1U, sizeof(local), fp) != sizeof(local) ||
        fwrite(entry1Name, 1U, name1Len, fp) != name1Len ||
        fwrite(compressed1, 1U, compressed1Size, fp) != compressed1Size) {
        free(compressed1); free(compressed2); fclose(fp); return 0;
    }
    entry2LocalOffset = (unsigned int)ftell(fp);
    memset(local, 0, sizeof(local));
    put32(local, 0x04034b50U);
    put16(local + 4, 20U);
    put16(local + 8, (unsigned int)method);
    put32(local + 18, (unsigned int)compressed2Size);
    put32(local + 22, (unsigned int)entry2Size);
    put16(local + 26, name2Len);
    if (fwrite(local, 1U, sizeof(local), fp) != sizeof(local) ||
        fwrite(entry2Name, 1U, name2Len, fp) != name2Len ||
        fwrite(compressed2, 1U, compressed2Size, fp) != compressed2Size) {
        free(compressed1); free(compressed2); fclose(fp); return 0;
    }

    /* Central directory: both entries side by side. */
    centralOffset = (unsigned int)ftell(fp);
    memset(central, 0, sizeof(central));
    put32(central, 0x02014b50U);
    put16(central + 4, 20U);
    put16(central + 6, 20U);
    put16(central + 10, (unsigned int)method);
    put32(central + 20, (unsigned int)compressed1Size);
    put32(central + 24, (unsigned int)entry1Size);
    put16(central + 28, name1Len);
    put32(central + 42, entry1LocalOffset);
    if (fwrite(central, 1U, sizeof(central), fp) != sizeof(central) ||
        fwrite(entry1Name, 1U, name1Len, fp) != name1Len) {
        free(compressed1); free(compressed2); fclose(fp); return 0;
    }
    entry2CentralOffset = (unsigned int)ftell(fp);
    memset(central, 0, sizeof(central));
    put32(central, 0x02014b50U);
    put16(central + 4, 20U);
    put16(central + 6, 20U);
    put16(central + 10, (unsigned int)method);
    put32(central + 20, (unsigned int)compressed2Size);
    put32(central + 24, (unsigned int)entry2Size);
    put16(central + 28, name2Len);
    put32(central + 42, entry2LocalOffset);
    if (fwrite(central, 1U, sizeof(central), fp) != sizeof(central) ||
        fwrite(entry2Name, 1U, name2Len, fp) != name2Len) {
        free(compressed1); free(compressed2); fclose(fp); return 0;
    }

    /* End of central directory record. */
    put32(eocd, 0x06054b50U);
    put16(eocd + 8, 2U);
    put16(eocd + 10, 2U);
    put32(eocd + 12, (unsigned int)(entry2CentralOffset + sizeof(central) + name2Len - centralOffset));
    put32(eocd + 16, centralOffset);
    if (fwrite(eocd, 1U, sizeof(eocd), fp) != sizeof(eocd)) {
        free(compressed1); free(compressed2); fclose(fp); return 0;
    }
    free(compressed1);
    free(compressed2);
    if (outEntry1Compressed) {
        *outEntry1Compressed = (unsigned int)compressed1Size;
    }
    if (outEntry2Compressed) {
        *outEntry2Compressed = (unsigned int)compressed2Size;
    }
    return fclose(fp) == 0;
}

int main(void) {
    /* Two deflated entries in one ZIP:
     *   - graphics: deeply nested path (4 segments) so the regression
     *     specifically exercises the nested-name materialization path.
     *   - dungeon:  a flat path so DM2's hash-pinned required-file row
     *     can also be matched (DM2 only reports available when ALL
     *     required files match).
     * Both entries use method=8 (deflate) so the inflate code in
     * asset_find_by_hash / asset_extract_virtual_path / M12 cache
     * materialization is the actual code path under test. */
    static const char kZipName[] = "dm2-required.zip";
    static const char kGraphicsEntry[] =
        "nested/inner/level1/level2/DM2GRAPHICS.DAT";
    static const char kDungeonEntry[] =
        "DM2DUNGEON.DAT";

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
    char extractedPath[M12_ASSET_DATA_DIR_CAPACITY];
    M12_AssetStatus status;
    const M12_AssetRequiredFileStatus* required;
    const M12_AssetVersionStatus* version;
    unsigned int graphicsCompressed = 0U;
    unsigned int dungeonCompressed = 0U;
    size_t graphicsSize = sizeof(kGraphicsPayload) - 1U;
    size_t dungeonSize = sizeof(kDungeonPayload) - 1U;

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
        !m12_file_md5_hex(dungeonPath, dungeonMd5) ||
        !write_two_entry_nested_zip(zipPath,
                                    kGraphicsEntry, kGraphicsPayload,
                                    graphicsSize,
                                    kDungeonEntry, kDungeonPayload,
                                    dungeonSize,
                                    &graphicsCompressed,
                                    &dungeonCompressed)) {
        fprintf(stderr, "fixture setup failed\n");
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
            "nested/inner/level1/level2/DM2GRAPHICS.DAT";
        check_int(strstr(foundPath, nestedNeedle) != NULL,
                  "scanner should preserve every segment of the nested "
                  "entry name in the virtual path");
    }

    /* Layer 2: low-level virtual-path extraction.
     * asset_extract_virtual_path() must inflate the deflate stream (or
     * copy the stored bytes when zlib is absent) and write a byte-
     * identical ordinary file. This is the helper that the M12 cache
     * materializer uses internally. */
    if (!FSP_JoinPath(extractedPath, sizeof(extractedPath), home,
                      "extracted.bin")) {
        fprintf(stderr, "extracted path setup failed\n");
        return 1;
    }
    /* Remove any previous file so we know we wrote it. */
    remove(extractedPath);
    check_int(asset_extract_virtual_path(foundPath, extractedPath),
              "asset_extract_virtual_path should succeed for nested "
              "deflated virtual path");
    check_int(file_matches_payload(extractedPath, kGraphicsPayload,
                                   graphicsSize),
              "asset_extract_virtual_path output must be byte-identical "
              "to the original nested entry payload");

    /* Layer 3: M12 cache materialization.
     * Register the synthesized MD5 as the canonical DM2 graphics hash,
     * scan, and verify:
     *   - DM2 is reported as available (both required files matched),
     *   - the version's matched path is the ORIGINAL virtual path
     *     (we want to confirm the scanner kept the nested prefix),
     *   - the required files have been rewritten to ordinary paths under
     *     <userDataDir>/asset-cache/dm2/<label>,
     *   - the materialized ordinary files exist on disk and contain
     *     exactly the bytes of the original entry. */
    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);

    M12_AssetStatus_Scan(&status, dataRoot);

    check_int(M12_AssetStatus_GameAvailable(&status, "dm2"),
              "DM2 should be available when both required hashes are "
              "matched by ZIP-backed deflated entries");
    check_int(M12_AssetStatus_GetRequiredFileCount(&status, "dm2") == 2U,
              "DM2 should expose exactly the GRAPHICS.DAT and DUNGEON.DAT "
              "required-file launch gate entries");
    version = M12_AssetStatus_GetVersion(&status, "dm2", 0U);
    check_int(version && version->matched &&
              path_has_virtual_entry(version->matchedPath, kZipName,
                                     kGraphicsEntry),
              "M12 version match should preserve the original nested ZIP "
              "virtual path (before materialization)");

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
    check_int(required && required->matched,
              "DM2 graphics required file should be matched after scan");
    check_int(path_has_cache_leaf(required->matchedPath, cacheRoot,
                                  "dm2", "GRAPHICS.DAT"),
              "DM2 graphics required file should be materialized into the "
              "ordinary-file launch cache path under asset-cache/dm2/");
    check_int(strcmp(required->matchedPath, cachedGraphics) == 0,
              "DM2 graphics required-file path should exactly match the "
              "runtimeDataDir/dm2/GRAPHICS.DAT launch path");
    check_int(file_matches_payload(cachedGraphics, kGraphicsPayload,
                                   graphicsSize),
              "cached GRAPHICS.DAT under asset-cache/dm2/ must be "
              "byte-identical to the original nested deflated entry "
              "payload");

    required = M12_AssetStatus_GetRequiredFile(&status, "dm2", 1U);
    check_int(required && required->matched &&
              path_has_cache_leaf(required->matchedPath, cacheRoot,
                                  "dm2", "DUNGEON.DAT"),
              "DM2 dungeon required file should be materialized into the "
              "ordinary-file launch cache path under asset-cache/dm2/");
    check_int(required && strcmp(required->matchedPath, cachedDungeon) == 0,
              "DM2 dungeon required-file path should exactly match the "
              "runtimeDataDir/dm2/DUNGEON.DAT launch path");
    check_int(file_matches_payload(cachedDungeon, kDungeonPayload,
                                   dungeonSize),
              "cached DUNGEON.DAT under asset-cache/dm2/ must be "
              "byte-identical to the original deflated entry payload");

    /* M11's DM2 handoff probes <runtimeDataDir>/dm2 first (see
     * m11_game_view.c M11_GameView_StartDm2), so this asserts the cache
     * root returned by M12 is sufficient for ordinary fopen() based launch
     * code without any virtual-path awareness. */
    check_int(runtime_cache_file_matches_payload(&status, "dm2",
                                                 "GRAPHICS.DAT",
                                                 kGraphicsPayload,
                                                 graphicsSize),
              "DM2 launch lookup should open runtimeDataDir/dm2/GRAPHICS.DAT "
              "as an ordinary materialized cache file");
    check_int(runtime_cache_file_matches_payload(&status, "dm2",
                                                 "DUNGEON.DAT",
                                                 kDungeonPayload,
                                                 dungeonSize),
              "DM2 launch lookup should open runtimeDataDir/dm2/DUNGEON.DAT "
              "as an ordinary materialized cache file");

    /* The cache leafs must NOT contain the virtual "::" separator after
     * materialization; the runtime expects to open them like any other
     * ordinary files. */
    {
        const M12_AssetRequiredFileStatus* g = M12_AssetStatus_GetRequiredFile(
            &status, "dm2", 0U);
        const M12_AssetRequiredFileStatus* d = M12_AssetStatus_GetRequiredFile(
            &status, "dm2", 1U);
        check_int(g && !strstr(g->matchedPath, "::"),
                  "materialized GRAPHICS cache leaf must not retain the "
                  "virtual container separator");
        check_int(d && !strstr(d->matchedPath, "::"),
                  "materialized DUNGEON cache leaf must not retain the "
                  "virtual container separator");
    }

    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
#ifdef FIRESTAFF_HAS_ZLIB
    puts("ok: nested deflated ZIP entry virtual path + asset cache leaf");
#else
    puts("ok: nested ZIP entry virtual path + asset cache leaf (stored)");
#endif
    return 0;
}
