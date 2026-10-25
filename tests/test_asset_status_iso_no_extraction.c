/*
 * tests/test_asset_status_iso_no_extraction.c
 *
 * Focused asset-scanner / in-memory ISO reader regression for
 * archive-backed DM1/CSB/DM2 required files.
 *
 * What this gate proves:
 *   1. The recursive hash scanner walks an ISO 9660 image, recognizes
 *      a Primary Volume Descriptor + root directory + two file records
 *      (GRAPHICS.DAT + DUNGEON.DAT), and reports each match as a
 *      virtual container path of the form
 *      "<image>.iso::<entry-name>".
 *   2. The low-level `asset_read_virtual_path_alloc()` helper reads the
 *      entry bytes into memory and returns bytes identical to the ISO.
 *   3. M12 retains the original virtual source paths, publishes a source
 *      runtime root, and never writes an extracted game-data cache.
 *
 * DM1, CSB, and DM2 are the three "flat-DAT" games that have an
 * m12_prepare_runtime_media_owner() source-owner contract;
 * this gate exercises all three so a future regression in any one of
 * them surfaces here, not as a separate per-game test.
 *
 * The fixture is one synthetic ISO 9660 image with 23 sectors:
 *
 *   sector 0..15  : zero padding (sectors before PVD)
 *   sector 16     : Primary Volume Descriptor (PVD) with type=1, "CD001"
 *   sector 17..19 : zero padding (terminator + Volume Descriptor Set)
 *   sector 20     : root directory (dot, dotdot, GRAPHICS.DAT, DUNGEON.DAT)
 *   sector 21     : GRAPHICS.DAT payload (zero-padded to 2048 bytes)
 *   sector 22     : DUNGEON.DAT payload (zero-padded to 2048 bytes)
 *
 * The payloads are large enough to be unambiguous to the scanner and
 * small enough to keep the fixture under 50 KB on disk. The MD5 of each
 * entry payload is computed at runtime (no hardcoded hashes), so the
 * test cannot silently rot when the fixture layout changes.
 *
 * Source-locked against the existing asset-loader module:
 *   - src/shared/asset_find_by_hash.c
 *       * scan_iso_by_md5 (walks PVD + root directory, matches by MD5)
 *       * iso_read_entry_alloc (sector-by-sector bounded RAM read)
 *       * asset_read_virtual_path_alloc (the public dispatcher)
 *   - src/shared/asset_status_m12.c
 *       * m12_path_is_virtual_asset (detects "::" in matched paths)
 *       * m12_prepare_runtime_media_owner (retains the source media owner)
 *
 * Test is data-free: it synthesizes its own ISO and uses the testing-only
 * M12_AssetStatus_TestSet{Dm1Multilanguage,Csb,Dm2}SyntheticHashes helpers
 * to register the synthesized payload MD5s as the canonical required-file
 * hashes for each of DM1, CSB, and DM2.
 */

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
    const char* scratch = getenv("FIRESTAFF_TEST_SCRATCH");
    if (!scratch || !scratch[0]) return 0;
#ifdef _WIN32
    int rc = snprintf(out, outSize, "%s\\firestaff_iso_no_extract_%lu",
                      scratch, (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return FSP_CreateDirectoryRecursive(out);
#else
    char templatePath[ASSET_PATH_MAX];
    if (snprintf(templatePath, sizeof(templatePath),
                 "%s/.firestaff-iso-no-extraction-XXXXXX", scratch) >=
            (int)sizeof(templatePath)) {
        return 0;
    }
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

/* Deterministic, easily-distinguishable payloads (no deflate so the
 * test is independent of FIRESTAFF_HAS_ZLIB). Both payloads start
 * with a recognisable ASCII header so a future reviewer can confirm
 * which entry a cached file came from without re-reading the source. */
static const unsigned char kDm1GraphicsPayload[] =
    "Firestaff synthetic DM1 GRAPHICS ISO 9660 fixture v1\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n"
    "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n";

static const unsigned char kDm1DungeonPayload[] =
    "Firestaff synthetic DM1 DUNGEON ISO 9660 fixture v1\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n"
    "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789\n";

static const unsigned char kCsbGraphicsPayload[] =
    "Firestaff synthetic CSB GRAPHICS ISO 9660 fixture v1\n"
    "FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210\n"
    "FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210\n"
    "FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210\n";

static const unsigned char kCsbDungeonPayload[] =
    "Firestaff synthetic CSB DUNGEON ISO 9660 fixture v1\n"
    "5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA\n"
    "5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA\n"
    "5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA5555AAAA\n";

static const unsigned char kDm2GraphicsPayload[] =
    "Firestaff synthetic DM2 GRAPHICS ISO 9660 fixture v1\n"
    "DEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEF\n"
    "DEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEF\n"
    "DEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEF\n";

static const unsigned char kDm2DungeonPayload[] =
    "Firestaff synthetic DM2 DUNGEON ISO 9660 fixture v1\n"
    "CAFEBABECAFEBABECAFEBABECAFEBABECAFEBABECAFEBABECAFEBABECAFEBABE\n"
    "CAFEBABECAFEBABECAFEBABECAFEBABECAFEBABECAFEBABECAFEBABECAFEBABE\n"
    "CAFEBABECAFEBABECAFEBABECAFEBABECAFEBABECAFEBABECAFEBABECAFEBABE\n";

static int write_iso_dir_record(unsigned char* dir, int offset,
                                unsigned int lba, unsigned int size,
                                int isDir, const unsigned char* name,
                                int nameLen) {
    int recLen = 33 + nameLen + ((nameLen & 1) ? 1 : 0);
    if (offset + recLen > 2048) return 0;
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

/* Build a 23-sector synthetic ISO 9660 image:
 *
 *   sector 0..15  : zero padding (sectors before PVD)
 *   sector 16     : Primary Volume Descriptor (PVD) with type=1, "CD001"
 *   sector 17..19 : zero padding (terminator + Volume Descriptor Set)
 *   sector 20     : root directory (dot, dotdot, two file records)
 *   sector 21     : GRAPHICS payload (zero-padded to 2048 bytes)
 *   sector 22     : DUNGEON payload (zero-padded to 2048 bytes)
 *
 * graphicsPayload / graphicsSize are written to sector 21, dungeonPayload
 * / dungeonSize to sector 22. Both must be <= 2048 bytes. Returns 1 on
 * success. */
static int write_two_file_synthetic_iso(const char* path,
                                        const unsigned char* graphicsPayload,
                                        size_t graphicsSize,
                                        const unsigned char* dungeonPayload,
                                        size_t dungeonSize) {
    static const unsigned char dot = 0;
    static const unsigned char dotdot = 1;
    FILE* fp = fopen(path, "wb");
    unsigned char zero[2048] = {0};
    unsigned char pvd[2048] = {0};
    unsigned char dir[2048] = {0};
    unsigned char graphicsSector[2048] = {0};
    unsigned char dungeonSector[2048] = {0};
    int offset = 0;
    int recLen;
    if (!fp) return 0;
    if (graphicsSize > sizeof(graphicsSector) ||
        dungeonSize > sizeof(dungeonSector)) {
        fclose(fp);
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
    if (!recLen) {
        fclose(fp);
        return 0;
    }
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
    /* Root directory: dot, dotdot, GRAPHICS.DAT;1, DUNGEON.DAT;1. */
    recLen = write_iso_dir_record(dir, offset, 20U, 2048U, 1, &dot, 1);
    if (!recLen) { fclose(fp); return 0; }
    offset += recLen;
    recLen = write_iso_dir_record(dir, offset, 20U, 2048U, 1, &dotdot, 1);
    if (!recLen) { fclose(fp); return 0; }
    offset += recLen;
    {
        const unsigned char* graphicsName =
            (const unsigned char*)"GRAPHICS.DAT;1";
        recLen = write_iso_dir_record(dir, offset, 21U,
                                      (unsigned int)graphicsSize, 0,
                                      graphicsName,
                                      (int)strlen((const char*)graphicsName));
        if (!recLen) { fclose(fp); return 0; }
        offset += recLen;
    }
    {
        const unsigned char* dungeonName =
            (const unsigned char*)"DUNGEON.DAT;1";
        recLen = write_iso_dir_record(dir, offset, 22U,
                                      (unsigned int)dungeonSize, 0,
                                      dungeonName,
                                      (int)strlen((const char*)dungeonName));
        if (!recLen) { fclose(fp); return 0; }
        offset += recLen;
    }
    if (fwrite(dir, 1U, sizeof(dir), fp) != sizeof(dir)) {
        fclose(fp);
        return 0;
    }
    /* File payload sectors. */
    memcpy(graphicsSector, graphicsPayload, graphicsSize);
    memcpy(dungeonSector, dungeonPayload, dungeonSize);
    if (fwrite(graphicsSector, 1U, sizeof(graphicsSector), fp) !=
            sizeof(graphicsSector) ||
        fwrite(dungeonSector, 1U, sizeof(dungeonSector), fp) !=
            sizeof(dungeonSector)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_single_data_track_cue(const char* path,
                                       const char* payloadName) {
    FILE* fp;
    if (!path || !payloadName) {
        return 0;
    }
    fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    if (fprintf(fp,
                "FILE \"%s\" BINARY\n"
                "  TRACK 01 MODE1/2048\n"
                "    INDEX 01 00:00:00\n",
                payloadName) < 0) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int path_has_virtual_entry(const char* path,
                                  const char* isoName,
                                  const char* entryName) {
    return path && strstr(path, isoName) && strstr(path, "::") &&
           strstr(path, entryName);
}

/* Compute fixture hashes directly from memory; no temporary media file is
 * needed for the hash calculation. */
static int m12_hex_md5_of_bytes(const unsigned char* payload, size_t payloadSize,
                                char outHex[33]) {
    if (!payload || !outHex) return 0;
    return m12_bytes_md5_hex(payload, payloadSize, outHex);
}

static void check_required_path_reads_in_memory(
        const M12_AssetRequiredFileStatus* file,
        const unsigned char* expected, size_t expectedSize,
        const char* label) {
    uint8_t* bytes = NULL;
    size_t byteCount = 0U;
    if (!file || !file->matched) return;
    check_int(strstr(file->matchedPath, "::") != NULL,
              "matched ISO game data must retain its virtual source path");
    check_int(asset_read_path_alloc(file->matchedPath, &bytes, &byteCount),
              label);
    check_int(bytes && byteCount == expectedSize &&
              memcmp(bytes, expected, expectedSize) == 0,
              "runtime bytes must match the original ISO entry");
    free(bytes);
}

/* Verify the M12 ISO -> source-owner contract for one flat-DAT game
 * (DM1, CSB, or DM2). The fixture is the dataRoot of a
 * freshly-prepared isolated scan, with the synthetic ISO image written
 * under isoName (relative to dataRoot). The function:
 *
 *   1. Scans and asserts the recursive hash scanner sees the two
 *      entry virtual paths.
 *   2. Asserts M12 preserves both matched virtual source paths.
 *   3. Asserts the runtime reader loads byte-identical data into RAM
 *      and no asset-cache tree is created.
 *   4. Cleans up the synthetic-hash overrides for the next iteration. */
static void check_flat_dat_iso_no_extraction(
        const char* gameId,
        const char* dataRoot,
        const char* isoName,
        const unsigned char* graphicsPayload,
        size_t graphicsSize,
        const unsigned char* dungeonPayload,
        size_t dungeonSize,
        void (*set_hashes)(const char* graphicsMd5, const char* dungeonMd5)) {
    char isoPath[M12_ASSET_DATA_DIR_CAPACITY];
    char cueDir[M12_ASSET_DATA_DIR_CAPACITY];
    char cuePath[M12_ASSET_DATA_DIR_CAPACITY];
    char cuePayloadPath[M12_ASSET_DATA_DIR_CAPACITY];
    const char cueName[] = "disc.cue";
    const char cuePayloadName[] = "disc.payload";
    char foundGraphics[ASSET_PATH_MAX];
    char foundDungeon[ASSET_PATH_MAX];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];
    char userDataDir[M12_ASSET_DATA_DIR_CAPACITY];
    char cacheRoot[M12_ASSET_DATA_DIR_CAPACITY];
    uint8_t* readBytes = NULL;
    size_t readSize = 0U;
    M12_AssetStatus status;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    const char* runtimeDir;

    if (!FSP_JoinPath(isoPath, sizeof(isoPath), dataRoot, isoName)) {
        check_int(0, "ISO path should fit");
        return;
    }
    check_int(write_two_file_synthetic_iso(isoPath,
                                           graphicsPayload, graphicsSize,
                                           dungeonPayload, dungeonSize),
              "synthetic two-file ISO 9660 fixture should be written");
    check_int(m12_hex_md5_of_bytes(graphicsPayload, graphicsSize, graphicsMd5),
              "graphics payload MD5 should be computable from the entry bytes");
    check_int(m12_hex_md5_of_bytes(dungeonPayload, dungeonSize, dungeonMd5),
              "dungeon payload MD5 should be computable from the entry bytes");

    /* Layer 1: scanner recognises both entries as ISO 9660 virtual paths. */
    memset(foundGraphics, 0, sizeof(foundGraphics));
    check_int(asset_find_by_md5(dataRoot, graphicsMd5, foundGraphics,
                                (int)sizeof(foundGraphics), 4) &&
              path_has_virtual_entry(foundGraphics, isoName, "GRAPHICS.DAT"),
              "scanner should find the GRAPHICS entry as an ISO virtual path");
    memset(foundDungeon, 0, sizeof(foundDungeon));
    check_int(asset_find_by_md5(dataRoot, dungeonMd5, foundDungeon,
                                (int)sizeof(foundDungeon), 4) &&
              path_has_virtual_entry(foundDungeon, isoName, "DUNGEON.DAT"),
              "scanner should find the DUNGEON entry as an ISO virtual path");

    /* Layer 2: the runtime reads the virtual ISO entry into bounded memory. */
    check_int(asset_read_path_alloc(foundGraphics, &readBytes, &readSize),
              "runtime reader should load the ISO GRAPHICS entry into memory");
    check_int(readBytes && readSize == graphicsSize &&
              memcmp(readBytes, graphicsPayload, graphicsSize) == 0,
              "in-memory GRAPHICS bytes should match the ISO entry");
    free(readBytes);
    readBytes = NULL;
    readSize = 0U;

    /* Layer 3: M12 retains source paths and creates no extracted cache. */
    set_hashes(graphicsMd5, dungeonMd5);
    /* Clear unrelated synthetic overrides so the scan focuses on this game. */
    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);
    set_hashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_Scan(&status, dataRoot);

    /* These fixture bytes exercise path handling only. They do not represent
     * a complete authentic release and must never satisfy the launch gate. */
    check_int(!M12_AssetStatus_GameAvailable(&status, gameId),
              "synthetic ISO entries must not make an incomplete game launchable");
    graphics = M12_AssetStatus_GetRequiredFile(&status, gameId, 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&status, gameId, 1U);
    check_required_path_reads_in_memory(graphics, graphicsPayload,
                                        graphicsSize,
                                        "M12 GRAPHICS ISO source must be readable");
    check_required_path_reads_in_memory(dungeon, dungeonPayload,
                                        dungeonSize,
                                        "M12 DUNGEON ISO source must be readable");
    runtimeDir = M12_AssetStatus_GetRuntimeDataDir(&status, gameId);
    check_int(runtimeDir && strstr(runtimeDir, "asset-cache") == NULL &&
              strstr(runtimeDir, "::") == NULL,
              "runtime root should remain an ordinary source-media path");
    check_int(FSP_GetUserDataDir(userDataDir, sizeof(userDataDir)) &&
              FSP_JoinPath(cacheRoot, sizeof(cacheRoot), userDataDir,
                           "asset-cache") && !FSP_PathExists(cacheRoot),
              "ISO scan and runtime routing must not create asset-cache files");

    /* Direct file request: a user may point --data-dir at the ISO itself.
     * That must still reach the recursive hash scanner instead of stopping
     * at the legacy explicit-file-candidate path with no launchable game. */
    M12_AssetStatus_Scan(&status, isoPath);
    check_int(!M12_AssetStatus_GameAvailable(&status, gameId),
              "direct ISO fixture must remain blocked by the authentic launch gate");
    runtimeDir = M12_AssetStatus_GetRuntimeDataDir(&status, gameId);
    check_int(runtimeDir && strstr(runtimeDir, "asset-cache") == NULL &&
              strstr(runtimeDir, "::") == NULL,
              "direct ISO request should publish its original source root");
    graphics = M12_AssetStatus_GetRequiredFile(&status, gameId, 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&status, gameId, 1U);
    check_required_path_reads_in_memory(graphics, graphicsPayload,
                                        graphicsSize,
                                        "direct ISO GRAPHICS must be readable");
    check_required_path_reads_in_memory(dungeon, dungeonPayload,
                                        dungeonSize,
                                        "direct ISO DUNGEON must be readable");

    /* Direct CUE request: users often point the launcher at a CUE sheet.
     * Isolate this fixture and deliberately give the referenced image no
     * .iso/.bin suffix. Without CUE parsing, the scanner can only see the
     * CUE text and a regular payload file whose whole-file MD5 does not
     * match either required entry. */
    check_int(FSP_JoinPath(cueDir, sizeof(cueDir), dataRoot, gameId) &&
              FSP_CreateDirectoryRecursive(cueDir),
              "CUE fixture directory should be creatable");
    check_int(FSP_JoinPath(cuePayloadPath, sizeof(cuePayloadPath),
                           cueDir, cuePayloadName) &&
              FSP_JoinPath(cuePath, sizeof(cuePath), cueDir, cueName),
              "CUE fixture paths should fit");
    check_int(write_two_file_synthetic_iso(cuePayloadPath,
                                           graphicsPayload, graphicsSize,
                                           dungeonPayload, dungeonSize) &&
              write_single_data_track_cue(cuePath, cuePayloadName),
              "CUE fixture should be written with a suffixless payload image");
    memset(foundGraphics, 0, sizeof(foundGraphics));
    check_int(asset_find_by_md5(cueDir, graphicsMd5, foundGraphics,
                                (int)sizeof(foundGraphics), 4) &&
              path_has_virtual_entry(foundGraphics, cuePayloadName,
                                     "GRAPHICS.DAT"),
              "scanner should find GRAPHICS through a CUE-referenced payload");
    memset(foundDungeon, 0, sizeof(foundDungeon));
    check_int(asset_find_by_md5(cueDir, dungeonMd5, foundDungeon,
                                (int)sizeof(foundDungeon), 4) &&
              path_has_virtual_entry(foundDungeon, cuePayloadName,
                                     "DUNGEON.DAT"),
              "scanner should find DUNGEON through a CUE-referenced payload");
    M12_AssetStatus_Scan(&status, cuePath);
    check_int(!M12_AssetStatus_GameAvailable(&status, gameId),
              "direct CUE fixture must remain blocked by the authentic launch gate");
    runtimeDir = M12_AssetStatus_GetRuntimeDataDir(&status, gameId);
    check_int(runtimeDir && strstr(runtimeDir, "asset-cache") == NULL &&
              strstr(runtimeDir, "::") == NULL,
              "direct CUE request should publish its original source root");
    graphics = M12_AssetStatus_GetRequiredFile(&status, gameId, 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&status, gameId, 1U);
    check_required_path_reads_in_memory(graphics, graphicsPayload,
                                        graphicsSize,
                                        "CUE-selected GRAPHICS must be readable");
    check_required_path_reads_in_memory(dungeon, dungeonPayload,
                                        dungeonSize,
                                        "CUE-selected DUNGEON must be readable");

    check_int(FSP_GetUserDataDir(userDataDir, sizeof(userDataDir)) &&
              FSP_JoinPath(cacheRoot, sizeof(cacheRoot), userDataDir,
                           "asset-cache") && !FSP_PathExists(cacheRoot),
              "direct ISO/CUE requests must not create asset-cache files");

    /* Clean up so the next game's scan starts from a neutral state. */
    set_hashes(NULL, NULL);
}

static void check_dm1_iso_no_extraction(const char* dataRoot) {
    const char isoName[] = "dm1_cd.iso";
    char gameDir[M12_ASSET_DATA_DIR_CAPACITY];
    check_int(FSP_JoinPath(gameDir, sizeof(gameDir), dataRoot, "dm1") &&
              FSP_CreateDirectoryRecursive(gameDir),
              "DM1 fixture subdirectory should be creatable");
    check_flat_dat_iso_no_extraction(
        "dm1", dataRoot, isoName,
        kDm1GraphicsPayload, sizeof(kDm1GraphicsPayload) - 1U,
        kDm1DungeonPayload, sizeof(kDm1DungeonPayload) - 1U,
        M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes);
}

static void check_csb_iso_no_extraction(const char* dataRoot) {
    const char isoName[] = "csb_cd.iso";
    char gameDir[M12_ASSET_DATA_DIR_CAPACITY];
    check_int(FSP_JoinPath(gameDir, sizeof(gameDir), dataRoot, "csb") &&
              FSP_CreateDirectoryRecursive(gameDir),
              "CSB fixture subdirectory should be creatable");
    check_flat_dat_iso_no_extraction(
        "csb", dataRoot, isoName,
        kCsbGraphicsPayload, sizeof(kCsbGraphicsPayload) - 1U,
        kCsbDungeonPayload, sizeof(kCsbDungeonPayload) - 1U,
        M12_AssetStatus_TestSetCsbSyntheticHashes);
}

static void check_dm2_iso_stays_in_media(const char* dataRoot) {
    const char isoName[] = "dm2_cd.iso";
    char gameDir[M12_ASSET_DATA_DIR_CAPACITY];
    check_int(FSP_JoinPath(gameDir, sizeof(gameDir), dataRoot, "dm2") &&
              FSP_CreateDirectoryRecursive(gameDir),
              "DM2 fixture subdirectory should be creatable");
    check_flat_dat_iso_no_extraction(
        "dm2", dataRoot, isoName,
        kDm2GraphicsPayload, sizeof(kDm2GraphicsPayload) - 1U,
        kDm2DungeonPayload, sizeof(kDm2DungeonPayload) - 1U,
        M12_AssetStatus_TestSetDm2SyntheticHashes);
}

int main(void) {
    char home[M12_ASSET_DATA_DIR_CAPACITY];
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];

    if (!make_isolated_home(home, sizeof(home)) ||
        !FSP_JoinPath(dataRoot, sizeof(dataRoot), home, "configured-data") ||
        !FSP_CreateDirectoryRecursive(dataRoot)) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }
    if (!test_setenv("HOME", home) ||
        !test_setenv("FIRESTAFF_DATA", dataRoot) ||
        !test_setenv("XDG_DATA_HOME", home) ||
        !test_setenv("APPDATA", home)) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    check_dm1_iso_no_extraction(dataRoot);
    check_csb_iso_no_extraction(dataRoot);
    check_dm2_iso_stays_in_media(dataRoot);

    (void)test_setenv("FIRESTAFF_DATA", NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: DM1/CSB/DM2 ISO entries stay virtual and read from memory without a game-data cache");
    return 0;
}
