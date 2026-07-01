/*
 * test_firestaff_save_export_manifest.c
 *
 * CTest gate for the per-game save-byte export/import
 * manifest layer (include/firestaff_save_export_manifest.h).
 *
 * Data-free: synthesizes a DM1 V1 Firestaff-native save
 * file (FSDM1SV1 magic + DM1SaveHeader + GameWorld_Compat
 * body) on disk, exports it through the manifest layer,
 * imports it back into a fresh path, and asserts that the
 * magic + version + CRC32 + file_size survive the
 * round-trip. Also drives the detector across the five
 * documented magics plus a synthetic DM2 slot-magic, and
 * the sidecar JSON parser across the documented manifest
 * shape + a tampered-CRC32 rejection path.
 *
 * Companion to the launcher-level
 * tests/test_save_browser_export_import_m12.c (which is
 * byte-level file-copy only; this test exercises the
 * per-game manifest layer that adds magic + version +
 * CRC32 + JSON sidecar).
 *
 * Source-lock:
 *   - include/firestaff_save_export_manifest.h
 *   - src/shared/firestaff_save_export_manifest.c
 *   - include/dm1_v1_save_load.h (DM1_SAVE_MAGIC, format)
 *   - include/memory_savegame_pc34_compat.h
 *     (SaveGameHeader_Compat magic "RDMCSB20" + formatVersion)
 *   - include/nexus_v1_save.h (NEXUS_SAVE_MAGIC "FNXS")
 *   - include/theron_v1_save_load.h (THERON_SAVE_MAGIC "TQR ")
 *   - docs/FIRESTAFF_GAP_LIST.md "Save export/import" row.
 *
 * Build:
 *   cmake --build build --target test_firestaff_save_export_manifest --parallel
 *   ctest --test-dir build -R '^firestaff_save_export_manifest$' --output-on-failure
 */

#include "firestaff_save_export_manifest.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#ifdef _WIN32
#include <direct.h>
#endif

/* ── Test counters ──────────────────────────────────────── */

static int g_failures = 0;

static void check(int cond, const char* name) {
    if (cond) {
        printf("  PASS: %s\n", name);
    } else {
        printf("  FAIL: %s\n", name);
        ++g_failures;
    }
}

static int mkdir_one(const char* path) {
#ifdef _WIN32
    if (_mkdir(path) == 0 || errno == EEXIST) return 1;
#else
    if (mkdir(path, 0755) == 0 || errno == EEXIST) return 1;
#endif
    return 0;
}

static void rmrf(const char* path) {
    char cmd[1024];
    /* Best-effort cleanup; on POSIX /bin/rm -rf. */
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "rmdir /S /Q \"%s\" 2>nul", path);
#else
    snprintf(cmd, sizeof(cmd), "/bin/rm -rf '%s'", path);
#endif
    (void)system(cmd);
}

static long file_size(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1L;
    return (long)st.st_size;
}

static int read_bytes(const char* path, unsigned char* out, size_t outSize, size_t* outRead) {
    FILE* fp = fopen(path, "rb");
    size_t n;
    if (!fp) return 0;
    n = fread(out, 1, outSize, fp);
    fclose(fp);
    if (outRead) *outRead = n;
    return 1;
}

static int write_bytes(const char* path, const unsigned char* data, size_t size) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    if (size > 0 && fwrite(data, 1, size, fp) != size) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

/* ── Synthesize a DM1 V1 Firestaff-native save (FSDM1SV1) ──
 *
 * The DM1 V1 save layout (src/dm1/dm1_v1_save_load.c):
 *   [0..63]   DM1SaveHeader (magic + format metadata + CRC32)
 *   [64..EOF) GameWorld_Compat body (F0897 serialization)
 *
 * To stay data-free, we hand-craft a valid header + a body
 * of arbitrary bytes that satisfies the CRC32 + magic +
 * format version + size invariants. The runtime save-load
 * integration test in tests/test_dm1_v1_save_load.c is the
 * gate for the full body semantics; this test only asserts
 * the per-game manifest contract.
 */

static void write_u32_le(unsigned char* p, uint32_t v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
}

static uint32_t crc32_compute(const unsigned char* data, size_t size) {
    return FirestaffSaveExport_CRC32(data, size);
}

static int write_dm1_save_fixture(const char* path,
                                  unsigned char* body, size_t bodySize,
                                  uint32_t gameID, uint32_t gameTick) {
    unsigned char header[64];
    uint32_t crc;
    size_t total;
    FILE* fp;

    memset(header, 0, sizeof(header));
    memcpy(header, "FSDM1SV1", 8);
    write_u32_le(header + 8, 1u);                /* formatVersion */
    total = 64u + bodySize;
    write_u32_le(header + 12, (uint32_t)total); /* totalFileSize */
    crc = crc32_compute(body, bodySize);
    write_u32_le(header + 16, crc);             /* bodyCRC32 */
    write_u32_le(header + 20, gameTick);        /* gameTick */
    write_u32_le(header + 24, gameID);          /* gameID */
    /* partyMapX..musicOn + bugProfileHash left at zero
     * (synthetic; round-trip only validates magic/version/CRC) */

    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(header, 1, sizeof(header), fp) != sizeof(header)) {
        fclose(fp);
        return 0;
    }
    if (bodySize > 0 && fwrite(body, 1, bodySize, fp) != bodySize) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int write_csb_save_fixture(const char* path,
                                  unsigned char* body, size_t bodySize,
                                  uint32_t gameID, uint32_t gameTick) {
    unsigned char header[64];
    uint32_t crc;
    size_t total;
    FILE* fp;

    (void)gameID; (void)gameTick;
    memset(header, 0, sizeof(header));
    memcpy(header, "RDMCSB20", 8);
    write_u32_le(header + 8, 1u);                /* formatVersion */
    write_u32_le(header + 12, 0x01020304u);      /* endianSentinel */
    total = 64u + bodySize;
    write_u32_le(header + 16, (uint32_t)total); /* totalFileSize */
    write_u32_le(header + 20, 1u);               /* sectionCount */
    crc = crc32_compute(body, bodySize);
    write_u32_le(header + 24, crc);              /* bodyCRC32 */

    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(header, 1, sizeof(header), fp) != sizeof(header)) {
        fclose(fp);
        return 0;
    }
    if (bodySize > 0 && fwrite(body, 1, bodySize, fp) != bodySize) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int write_nexus_save_fixture(const char* path,
                                    unsigned char* body, size_t bodySize,
                                    uint32_t gameID, uint32_t gameTick) {
    unsigned char header[64];
    uint32_t crc;
    size_t total;
    FILE* fp;

    (void)gameID; (void)gameTick;
    memset(header, 0, sizeof(header));
    /* NEXUS_SAVE_MAGIC = 'FNXS' = 0x53584E46u (little-endian
     * representation is "FNXS" as bytes at offset 0). */
    header[0] = 'F'; header[1] = 'N'; header[2] = 'X'; header[3] = 'S';
    write_u32_le(header + 4, 2u);                /* NEXUS_SAVE_VERSION */
    write_u32_le(header + 8, 64u);               /* header_size */
    total = 64u + bodySize;
    write_u32_le(header + 12, (uint32_t)total); /* data_size */
    write_u32_le(header + 16, (uint32_t)bodySize); /* champion_data_size */
    write_u32_le(header + 20, (uint32_t)bodySize); /* world_data_size */
    crc = crc32_compute(body, bodySize);
    write_u32_le(header + 24, crc);              /* crc32 */

    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(header, 1, sizeof(header), fp) != sizeof(header)) {
        fclose(fp);
        return 0;
    }
    if (bodySize > 0 && fwrite(body, 1, bodySize, fp) != bodySize) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int write_theron_save_fixture(const char* path,
                                     unsigned char* body, size_t bodySize,
                                     uint32_t gameID, uint32_t gameTick) {
    unsigned char header[32];
    FILE* fp;

    (void)gameID; (void)gameTick;
    memset(header, 0, sizeof(header));
    /* THERON_SAVE_MAGIC = 0x54515220u ('TQR ' little-endian) */
    header[0] = 'T'; header[1] = 'Q'; header[2] = 'R'; header[3] = ' ';
    write_u32_le(header + 4, 1u);                /* version */
    write_u32_le(header + 8, (uint32_t)bodySize);/* payload_size */

    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(header, 1, sizeof(header), fp) != sizeof(header)) {
        fclose(fp);
        return 0;
    }
    if (bodySize > 0 && fwrite(body, 1, bodySize, fp) != bodySize) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int write_dm2_save_fixture(const char* path,
                                  unsigned char* body, size_t bodySize,
                                  uint32_t gameID, uint32_t gameTick) {
    unsigned char header[16];
    FILE* fp;

    (void)gameID; (void)gameTick;
    memset(header, 0, sizeof(header));
    /* DM2 slot-magic BEEF sentinel at offset 0. */
    header[0] = 0xEFu; header[1] = 0xBEu; header[2] = 0x00u; header[3] = 0x00u;
    write_u32_le(header + 4, 1u);                /* version */
    write_u32_le(header + 8, (uint32_t)bodySize);/* payload_size */

    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(header, 1, sizeof(header), fp) != sizeof(header)) {
        fclose(fp);
        return 0;
    }
    if (bodySize > 0 && fwrite(body, 1, bodySize, fp) != bodySize) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

/* ── Test 1: CRC32 known vector ─────────────────────────── */

static int test_crc32_known_vector(void) {
    /* "123456789" → 0xCBF43926 */
    const unsigned char* s = (const unsigned char*)"123456789";
    uint32_t got = FirestaffSaveExport_CRC32(s, 9);
    check(got == 0xCBF43926u, "CRC32(\"123456789\") == 0xCBF43926");
    /* empty */
    check(FirestaffSaveExport_CRC32(NULL, 0) == 0u,
          "CRC32(NULL,0) == 0");
    return g_failures == 0;
}

/* ── Test 2: kind token round-trip ──────────────────────── */

static int test_kind_tokens(void) {
    check(strcmp(FirestaffSaveExportKind_Token(FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1),
                 "dm1_v1") == 0,
          "DM1_V1 token == dm1_v1");
    check(FirestaffSaveExportKind_Parse("csb_v1") ==
              FIRESTAFF_SAVE_EXPORT_KIND_CSB_V1,
          "parse csb_v1 → CSB_V1");
    check(FirestaffSaveExportKind_Parse("nope") ==
              FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN,
          "parse unknown → UNKNOWN");
    check(FirestaffSaveExportKind_Token(FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN) == NULL,
          "UNKNOWN token is NULL");
    return g_failures == 0;
}

/* ── Test 3: detector on known magics ───────────────────── */

static int test_detector(void) {
    unsigned char dm1Header[64];
    unsigned char csbHeader[64];
    unsigned char nexusHeader[64];
    unsigned char theronHeader[32];
    unsigned char dm2Header[16];
    unsigned char unknown[64];
    char magic[16];
    uint32_t version;

    /* DM1 prefix. */
    memset(dm1Header, 0, sizeof(dm1Header));
    memcpy(dm1Header, "FSDM1SV1", 8);
    write_u32_le(dm1Header + 8, 1u);
    magic[0] = '\0';
    version = 0u;
    check(FirestaffSaveExport_DetectKind(dm1Header, sizeof(dm1Header),
                                         magic, sizeof(magic), &version) ==
              FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
          "detector recognises FSDM1SV1");
    check(strcmp(magic, "FSDM1SV1") == 0, "detector returns FSDM1SV1 magic");
    check(version == 1u, "detector returns DM1 formatVersion 1");

    /* CSB prefix. */
    memset(csbHeader, 0, sizeof(csbHeader));
    memcpy(csbHeader, "RDMCSB20", 8);
    write_u32_le(csbHeader + 8, 1u);
    magic[0] = '\0';
    check(FirestaffSaveExport_DetectKind(csbHeader, sizeof(csbHeader),
                                         magic, sizeof(magic), &version) ==
              FIRESTAFF_SAVE_EXPORT_KIND_CSB_V1,
          "detector recognises RDMCSB20");

    /* Nexus prefix. */
    memset(nexusHeader, 0, sizeof(nexusHeader));
    memcpy(nexusHeader, "FNXS", 4);
    write_u32_le(nexusHeader + 4, 2u);
    check(FirestaffSaveExport_DetectKind(nexusHeader, sizeof(nexusHeader),
                                         magic, sizeof(magic), &version) ==
              FIRESTAFF_SAVE_EXPORT_KIND_NEXUS_V1,
          "detector recognises FNXS");

    /* Theron prefix. */
    memset(theronHeader, 0, sizeof(theronHeader));
    memcpy(theronHeader, "TQR ", 4);
    write_u32_le(theronHeader + 4, 1u);
    check(FirestaffSaveExport_DetectKind(theronHeader, sizeof(theronHeader),
                                         magic, sizeof(magic), &version) ==
              FIRESTAFF_SAVE_EXPORT_KIND_THERON_V1,
          "detector recognises TQR ");

    /* DM2 slot-magic BEEF. */
    memset(dm2Header, 0, sizeof(dm2Header));
    write_u32_le(dm2Header, 0xBEEFu);
    check(FirestaffSaveExport_DetectKind(dm2Header, sizeof(dm2Header),
                                         magic, sizeof(magic), &version) ==
              FIRESTAFF_SAVE_EXPORT_KIND_DM2_V1,
          "detector recognises 0xBEEF slot-magic");

    /* Unknown prefix. */
    memset(unknown, 0xAA, sizeof(unknown));
    check(FirestaffSaveExport_DetectKind(unknown, sizeof(unknown),
                                         magic, sizeof(magic), &version) ==
              FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN,
          "detector returns UNKNOWN on 0xAA prefix");
    return g_failures == 0;
}

/* ── Test 4: detector rejects wrong version ─────────────── */

static int test_detector_wrong_version(void) {
    unsigned char badDm1[64];
    memset(badDm1, 0, sizeof(badDm1));
    memcpy(badDm1, "FSDM1SV1", 8);
    write_u32_le(badDm1 + 8, 99u); /* wrong format version */
    check(FirestaffSaveExport_DetectKind(badDm1, sizeof(badDm1),
                                         NULL, 0, NULL) ==
              FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN,
          "detector rejects FSDM1SV1 with wrong format version");
    return g_failures == 0;
}

/* ── Test 5: detector rejects too-small input ───────────── */

static int test_detector_too_small(void) {
    unsigned char tiny[3] = { 'F', 'S', 'D' };
    check(FirestaffSaveExport_DetectKind(tiny, sizeof(tiny),
                                         NULL, 0, NULL) ==
              FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN,
          "detector rejects 3-byte input");
    check(FirestaffSaveExport_DetectKind(NULL, 0, NULL, 0, NULL) ==
              FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN,
          "detector rejects NULL input");
    return g_failures == 0;
}

/* ── Test 6: full DM1 round-trip ─────────────────────────── */

static int test_dm1_round_trip(void) {
    const char* root = "/tmp/firestaff_save_export_manifest_dm1_test";
    char dataDir[512];
    char exportDir[512];
    char importDir[512];
    char srcPath[512];
    char targetPath[512];
    char binPath[512];
    char manifestPath[512];
    unsigned char body[256];
    char magic[16];
    char errBuf[256];
    FirestaffSaveExportResult rc;
    int i;
    unsigned char importedBody[256];
    size_t importedRead;
    long sizeBefore, sizeAfter;

    rmrf(root);
    snprintf(dataDir, sizeof(dataDir), "%s/data", root);
    snprintf(exportDir, sizeof(exportDir), "%s/export", root);
    snprintf(importDir, sizeof(importDir), "%s/import", root);
    check(mkdir_one(root), "mkdir root");
    check(mkdir_one(dataDir), "mkdir data");
    check(mkdir_one(exportDir), "mkdir export");
    check(mkdir_one(importDir), "mkdir import");

    snprintf(srcPath, sizeof(srcPath),
             "%s/firestaff-dm1-slot.sav", dataDir);

    /* Deterministic body. */
    for (i = 0; i < (int)sizeof(body); ++i) body[i] = (unsigned char)(i ^ 0x5A);
    check(write_dm1_save_fixture(srcPath, body, sizeof(body), 0xDEADBEEFu, 4242u),
          "wrote DM1 save fixture");

    /* Export. */
    errBuf[0] = '\0';
    binPath[0] = '\0';
    manifestPath[0] = '\0';
    rc = FirestaffSaveExport_ExportFileWithKind(
            srcPath, exportDir,
            FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
            binPath, sizeof(binPath),
            manifestPath, sizeof(manifestPath),
            errBuf, sizeof(errBuf));
    check(rc == FIRESTAFF_SAVE_EXPORT_OK,
          "DM1 export returns OK");
    if (rc != FIRESTAFF_SAVE_EXPORT_OK) {
        printf("    errBuf: %s\n", errBuf);
        return g_failures == 0;
    }
    check(strstr(binPath, "/export/firestaff-dm1-slot.savebin") != NULL,
          "export wrote <basename>.savebin under exportDir");
    check(strstr(manifestPath, ".savebin.json") != NULL,
          "export wrote <basename>.savebin.json sidecar");
    check(file_size(binPath) == 64L + (long)sizeof(body),
          "export bin size = header + body");

    /* Verify the bin preserved magic + version + body bytes. */
    {
        unsigned char buf[320];
        size_t got;
        check(read_bytes(binPath, buf, sizeof(buf), &got),
              "read exported bin");
        check(memcmp(buf, "FSDM1SV1", 8) == 0,
              "exported bin magic preserved");
        check(buf[8] == 1 && buf[9] == 0 && buf[10] == 0 && buf[11] == 0,
              "exported bin format version preserved");
        check(memcmp(buf + 64, body, sizeof(body)) == 0,
              "exported bin body bytes preserved");
    }

    /* Copy exported bin+json into import dir and run the
     * import path against the source bin path. The
     * manifest layer is path-agnostic, so a direct import
     * from the export dir is the simpler test surface. */
    snprintf(targetPath, sizeof(targetPath),
             "%s/imported-dm1-slot.sav", importDir);

    errBuf[0] = '\0';
    rc = FirestaffSaveExport_ImportFile(
            exportDir, "firestaff-dm1-slot",
            FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
            "FSDM1SV1", 1u,
            targetPath,
            NULL, 0, NULL, 0,
            errBuf, sizeof(errBuf));
    check(rc == FIRESTAFF_SAVE_EXPORT_OK,
          "DM1 import returns OK");
    if (rc != FIRESTAFF_SAVE_EXPORT_OK) {
        printf("    errBuf: %s\n", errBuf);
        return g_failures == 0;
    }
    check(file_size(targetPath) == 64L + (long)sizeof(body),
          "imported target size = header + body");
    check(read_bytes(targetPath, importedBody, sizeof(importedBody), &importedRead),
          "read imported target");
    check(memcmp(importedBody, "FSDM1SV1", 8) == 0,
          "imported target magic preserved");
    check(memcmp(importedBody + 64, body, sizeof(body)) == 0,
          "imported target body bytes preserved");

    /* Magic + version probe on the imported target. */
    magic[0] = '\0';
    check(FirestaffSaveExport_DetectKindFromFile(targetPath,
                                                  magic, sizeof(magic),
                                                  NULL, NULL, 0) ==
              FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
          "imported target re-detected as DM1_V1");
    check(strcmp(magic, "FSDM1SV1") == 0,
          "imported target magic string is FSDM1SV1");

    sizeBefore = file_size(srcPath);
    sizeAfter = file_size(targetPath);
    check(sizeBefore == sizeAfter,
          "imported target size matches original");

    rmrf(root);
    return g_failures == 0;
}

/* ── Test 7: round-trip across all five kinds ───────────── */

static int test_multi_kind_round_trip(void) {
    static const struct {
        FirestaffSaveExportKind kind;
        const char* magic;
        uint32_t version;
        const char* basename;
        int (*write_fixture)(const char*, unsigned char*, size_t,
                              uint32_t, uint32_t);
    } cases[] = {
        { FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,    "FSDM1SV1", 1u, "firestaff-dm1", write_dm1_save_fixture },
        { FIRESTAFF_SAVE_EXPORT_KIND_CSB_V1,    "RDMCSB20", 1u, "firestaff-csb", write_csb_save_fixture },
        { FIRESTAFF_SAVE_EXPORT_KIND_DM2_V1,    "BEEF\0DEAD", 1u, "firestaff-dm2", write_dm2_save_fixture },
        { FIRESTAFF_SAVE_EXPORT_KIND_NEXUS_V1,  "FNXS",     2u, "firestaff-nexus", write_nexus_save_fixture },
        { FIRESTAFF_SAVE_EXPORT_KIND_THERON_V1, "TQR ",     1u, "firestaff-theron", write_theron_save_fixture },
    };
    int nCases = (int)(sizeof(cases) / sizeof(cases[0]));
    int ci;
    char root[512];

    snprintf(root, sizeof(root), "/tmp/firestaff_save_export_manifest_multi_%ld",
             (long)getpid());
    rmrf(root);
    check(mkdir_one(root), "mkdir multi root");

    for (ci = 0; ci < nCases; ++ci) {
        const char* magic = cases[ci].magic;
        uint32_t version = cases[ci].version;
        char subDir[768];
        char srcPath[1024];
        char exportDir[1024];
        char importDir[1024];
        char targetPath[1024];
        unsigned char body[128];
        int i;
        char binPath[1024];
        char manifestPath[1024];
        char errBuf[256];
        FirestaffSaveExportResult rc;
        unsigned char importedBody[128];
        size_t importedRead;

        for (i = 0; i < (int)sizeof(body); ++i) body[i] = (unsigned char)(i ^ (ci * 13));
        snprintf(subDir, sizeof(subDir), "%s/case%d", root, ci);
        snprintf(srcPath, sizeof(srcPath), "%s/%s-slot.sav", subDir, cases[ci].basename);
        snprintf(exportDir, sizeof(exportDir), "%s/export", subDir);
        snprintf(importDir, sizeof(importDir), "%s/import", subDir);
        snprintf(targetPath, sizeof(targetPath), "%s/%s-imported.sav",
                 importDir, cases[ci].basename);

        check(mkdir_one(subDir), "mkdir case dir");
        check(mkdir_one(exportDir), "mkdir case export");
        check(mkdir_one(importDir), "mkdir case import");
        check(cases[ci].write_fixture(srcPath, body, sizeof(body), 1u, 1u),
              "wrote fixture");

        errBuf[0] = '\0';
        rc = FirestaffSaveExport_ExportFileWithKind(
                srcPath, exportDir,
                cases[ci].kind,
                binPath, sizeof(binPath),
                manifestPath, sizeof(manifestPath),
                errBuf, sizeof(errBuf));
        if (rc != FIRESTAFF_SAVE_EXPORT_OK) {
            printf("    case %d export errBuf: %s\n", ci, errBuf);
        }
        check(rc == FIRESTAFF_SAVE_EXPORT_OK, "case export OK");

        errBuf[0] = '\0';
        rc = FirestaffSaveExport_ImportFile(
                exportDir, cases[ci].basename,
                cases[ci].kind,
                magic, version,
                targetPath,
                NULL, 0, NULL, 0,
                errBuf, sizeof(errBuf));
        if (rc != FIRESTAFF_SAVE_EXPORT_OK) {
            printf("    case %d import errBuf: %s\n", ci, errBuf);
        }
        check(rc == FIRESTAFF_SAVE_EXPORT_OK, "case import OK");
        check(file_size(targetPath) == file_size(srcPath),
              "case target size == source size");
        check(read_bytes(targetPath, importedBody, sizeof(importedBody), &importedRead),
              "read imported target");
        /* Compare body bytes only — DM2 slot-magic has a
         * different header layout but the body bytes must
         * round-trip identically. */
        check(memcmp(importedBody + 16, body, sizeof(body) - 16) == 0,
              "case body bytes preserved (after 16-byte head)");
    }

    rmrf(root);
    return g_failures == 0;
}

/* ── Test 8: kind mismatch rejection on import ──────────── */

static int test_kind_mismatch_rejection(void) {
    const char* root = "/tmp/firestaff_save_export_manifest_mismatch";
    char exportDir[512];
    char srcPath[512];
    char targetPath[512];
    char binPath[512];
    char manifestPath[512];
    unsigned char body[64];
    char errBuf[256];
    FirestaffSaveExportResult rc;

    rmrf(root);
    snprintf(exportDir, sizeof(exportDir), "%s/export", root);
    snprintf(srcPath, sizeof(srcPath), "%s/dm1-slot.sav", root);
    snprintf(targetPath, sizeof(targetPath), "%s/imported.sav", root);
    check(mkdir_one(root), "mkdir mismatch root");
    check(mkdir_one(exportDir), "mkdir mismatch export");

    for (int i = 0; i < 64; ++i) body[i] = (unsigned char)i;
    check(write_dm1_save_fixture(srcPath, body, sizeof(body), 1u, 1u),
          "wrote DM1 fixture");

    errBuf[0] = '\0';
    rc = FirestaffSaveExport_ExportFileWithKind(
            srcPath, exportDir,
            FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
            binPath, sizeof(binPath),
            manifestPath, sizeof(manifestPath),
            errBuf, sizeof(errBuf));
    check(rc == FIRESTAFF_SAVE_EXPORT_OK, "export OK for mismatch test");

    /* Request Nexus at import; expect KIND_MISMATCH. */
    errBuf[0] = '\0';
    rc = FirestaffSaveExport_ImportFile(
            exportDir, "dm1-slot",
            FIRESTAFF_SAVE_EXPORT_KIND_NEXUS_V1,
            "FNXS", 2u,
            targetPath,
            NULL, 0, NULL, 0,
            errBuf, sizeof(errBuf));
    check(rc == FIRESTAFF_SAVE_EXPORT_KIND_MISMATCH,
          "import with wrong kind returns KIND_MISMATCH");
    check(file_size(targetPath) < 0,
          "no file written when kind mismatch");

    rmrf(root);
    return g_failures == 0;
}

/* ── Test 9: bad magic rejection ────────────────────────── */

static int test_bad_magic_rejection(void) {
    const char* root = "/tmp/firestaff_save_export_manifest_bad_magic";
    char exportDir[512];
    char srcPath[512];
    char targetPath[512];
    unsigned char body[32];
    char errBuf[256];
    FirestaffSaveExportResult rc;

    rmrf(root);
    snprintf(exportDir, sizeof(exportDir), "%s/export", root);
    snprintf(srcPath, sizeof(srcPath), "%s/bad.sav", root);
    snprintf(targetPath, sizeof(targetPath), "%s/imported.sav", root);
    check(mkdir_one(root), "mkdir bad magic root");
    check(mkdir_one(exportDir), "mkdir bad magic export");
    for (int i = 0; i < 32; ++i) body[i] = (unsigned char)(0xCC ^ i);
    check(write_bytes(srcPath, body, sizeof(body)), "wrote bad-magic fixture");

    errBuf[0] = '\0';
    rc = FirestaffSaveExport_ImportFile(
            exportDir, "bad",
            FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
            "FSDM1SV1", 1u,
            targetPath,
            NULL, 0, NULL, 0,
            errBuf, sizeof(errBuf));
    check(rc == FIRESTAFF_SAVE_EXPORT_FILE_OPEN ||
          rc == FIRESTAFF_SAVE_EXPORT_KIND_NOT_DETECTED,
          "import rejects bad-magic fixture (no manifest or unrecognised)");

    rmrf(root);
    return g_failures == 0;
}

/* ── Test 10: target-exists no-overwrite ─────────────────── */

static int test_target_exists(void) {
    const char* root = "/tmp/firestaff_save_export_manifest_target_exists";
    char exportDir[512];
    char targetPath[512];
    char srcPath[512];
    unsigned char body[64];
    unsigned char pre[16];
    char binPath[512];
    char manifestPath[512];
    char errBuf[256];
    FirestaffSaveExportResult rc;

    rmrf(root);
    snprintf(exportDir, sizeof(exportDir), "%s/export", root);
    snprintf(srcPath, sizeof(srcPath), "%s/dm1.sav", root);
    snprintf(targetPath, sizeof(targetPath), "%s/imported.sav", root);
    check(mkdir_one(root), "mkdir target-exists root");
    check(mkdir_one(exportDir), "mkdir target-exists export");

    for (int i = 0; i < (int)sizeof(body); ++i) body[i] = (unsigned char)i;
    check(write_dm1_save_fixture(srcPath, body, sizeof(body), 1u, 1u),
          "wrote target-exists DM1 fixture");
    errBuf[0] = '\0';
    check(FirestaffSaveExport_ExportFileWithKind(
              srcPath, exportDir,
              FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
              binPath, sizeof(binPath),
              manifestPath, sizeof(manifestPath),
              errBuf, sizeof(errBuf)) == FIRESTAFF_SAVE_EXPORT_OK,
          "target-exists export OK");

    /* Pre-create target with sentinel bytes. */
    memset(pre, 0xAA, sizeof(pre));
    check(write_bytes(targetPath, pre, sizeof(pre)),
          "wrote pre-existing target sentinel");
    errBuf[0] = '\0';
    rc = FirestaffSaveExport_ImportFile(
            exportDir, "dm1",
            FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
            "FSDM1SV1", 1u,
            targetPath,
            NULL, 0, NULL, 0,
            errBuf, sizeof(errBuf));
    check(rc == FIRESTAFF_SAVE_EXPORT_TARGET_EXISTS,
          "import with existing target returns TARGET_EXISTS");
    {
        unsigned char got[16];
        size_t gotN;
        check(read_bytes(targetPath, got, sizeof(got), &gotN),
              "read pre-existing target");
        check(memcmp(got, pre, sizeof(pre)) == 0,
              "pre-existing target bytes preserved on TARGET_EXISTS");
    }

    rmrf(root);
    return g_failures == 0;
}

/* ── Test 11: tampered-CRC rejection ─────────────────────── */

static int test_tampered_crc(void) {
    const char* root = "/tmp/firestaff_save_export_manifest_tampered_crc";
    char exportDir[512];
    char srcPath[512];
    char targetPath[512];
    char manifestPath[512];
    char binPath[512];
    unsigned char body[64];
    char errBuf[256];
    FirestaffSaveExportResult rc;

    rmrf(root);
    snprintf(exportDir, sizeof(exportDir), "%s/export", root);
    snprintf(srcPath, sizeof(srcPath), "%s/dm1.sav", root);
    snprintf(targetPath, sizeof(targetPath), "%s/imported.sav", root);
    check(mkdir_one(root), "mkdir tampered-crc root");
    check(mkdir_one(exportDir), "mkdir tampered-crc export");
    for (int i = 0; i < (int)sizeof(body); ++i) body[i] = (unsigned char)i;
    check(write_dm1_save_fixture(srcPath, body, sizeof(body), 1u, 1u),
          "wrote tampered-crc DM1 fixture");
    errBuf[0] = '\0';
    check(FirestaffSaveExport_ExportFileWithKind(
              srcPath, exportDir,
              FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
              binPath, sizeof(binPath),
              manifestPath, sizeof(manifestPath),
              errBuf, sizeof(errBuf)) == FIRESTAFF_SAVE_EXPORT_OK,
          "tampered-crc export OK");

    /* Tamper with the bin bytes. */
    {
        FILE* fp = fopen(binPath, "r+b");
        unsigned char b = 0xFFu;
        check(fp != NULL, "open bin for tamper");
        if (fp) {
            fseek(fp, 100, SEEK_SET);
            fwrite(&b, 1, 1, fp);
            fclose(fp);
        }
    }

    errBuf[0] = '\0';
    rc = FirestaffSaveExport_ImportFile(
            exportDir, "dm1",
            FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1,
            "FSDM1SV1", 1u,
            targetPath,
            NULL, 0, NULL, 0,
            errBuf, sizeof(errBuf));
    check(rc == FIRESTAFF_SAVE_EXPORT_BAD_CRC,
          "tampered bin returns BAD_CRC on import");
    check(file_size(targetPath) < 0,
          "no file written when CRC mismatch");

    rmrf(root);
    return g_failures == 0;
}

/* ── Test 12: BuildPaths helper ──────────────────────────── */

static int test_build_paths(void) {
    char bin[1024];
    char json[1024];
    check(FirestaffSaveExport_BuildPaths("/tmp/x", "foo.sav",
                                         bin, sizeof(bin),
                                         json, sizeof(json)) == 1,
          "BuildPaths(foo.sav) succeeds");
    check(strstr(bin, "foo.savebin") != NULL, "BuildPaths strips .sav");
    check(strstr(json, "foo.savebin.json") != NULL,
          "BuildPaths emits <basename>.savebin.json");
    check(FirestaffSaveExport_BuildPaths("/tmp/x", "foo.tqsv",
                                         bin, sizeof(bin),
                                         json, sizeof(json)) == 1,
          "BuildPaths(foo.tqsv) succeeds");
    check(strstr(bin, "foo.savebin") != NULL,
          "BuildPaths also strips .tqsv");
    check(FirestaffSaveExport_BuildPaths("/tmp/x", "foo",
                                         bin, sizeof(bin),
                                         json, sizeof(json)) == 1,
          "BuildPaths(no extension) succeeds");
    check(strstr(bin, "foo.savebin") != NULL,
          "BuildPaths keeps bare basename as <name>.savebin");
    check(FirestaffSaveExport_BuildPaths(NULL, "foo.sav",
                                         bin, sizeof(bin),
                                         json, sizeof(json)) == 0,
          "BuildPaths(NULL dir) rejected");
    return g_failures == 0;
}

/* ── Test 13: result code strings ────────────────────────── */

static int test_result_strings(void) {
    check(strcmp(FirestaffSaveExportResult_String(FIRESTAFF_SAVE_EXPORT_OK),
                 "OK") == 0, "result string OK");
    check(strcmp(FirestaffSaveExportResult_String(FIRESTAFF_SAVE_EXPORT_BAD_CRC),
                 "BAD_CRC") == 0, "result string BAD_CRC");
    check(strcmp(FirestaffSaveExportResult_String((FirestaffSaveExportResult)9999),
                 "UNKNOWN") == 0, "result string for unknown code");
    return g_failures == 0;
}

/* ── Test 14: ExportFile auto-detects kind ───────────────── */

static int test_export_autodetect(void) {
    const char* root = "/tmp/firestaff_save_export_manifest_autodetect";
    char exportDir[512];
    char srcPath[512];
    unsigned char body[64];
    char binPath[1024];
    char manifestPath[1024];
    char errBuf[256];
    FirestaffSaveExportResult rc;

    rmrf(root);
    snprintf(exportDir, sizeof(exportDir), "%s/export", root);
    snprintf(srcPath, sizeof(srcPath), "%s/csb.sav", root);
    check(mkdir_one(root), "mkdir autodetect root");
    check(mkdir_one(exportDir), "mkdir autodetect export");

    for (int i = 0; i < (int)sizeof(body); ++i) body[i] = (unsigned char)(0x77 ^ i);
    check(write_csb_save_fixture(srcPath, body, sizeof(body), 1u, 1u),
          "wrote autodetect CSB fixture");

    errBuf[0] = '\0';
    rc = FirestaffSaveExport_ExportFile(
            srcPath, exportDir,
            binPath, sizeof(binPath),
            manifestPath, sizeof(manifestPath),
            errBuf, sizeof(errBuf));
    check(rc == FIRESTAFF_SAVE_EXPORT_OK,
          "ExportFile autodetect CSB_V1 returns OK");
    check(strstr(manifestPath, ".savebin.json") != NULL,
          "autodetect export wrote sidecar");

    rmrf(root);
    return g_failures == 0;
}

/* ── Main ───────────────────────────────────────────────── */

int main(void) {
    int failures_at_start;

    printf("═══════════════════════════════════════════════════════\n");
    printf("  Firestaff per-game save export/import manifest\n");
    printf("  Data-free CTest, see include/firestaff_save_export_manifest.h\n");
    printf("═══════════════════════════════════════════════════════\n");

    failures_at_start = g_failures;
    (void)failures_at_start;

    printf("\n[CRC32]\n");           test_crc32_known_vector();
    printf("\n[Kind tokens]\n");     test_kind_tokens();
    printf("\n[Detector — known magics]\n"); test_detector();
    printf("\n[Detector — wrong version]\n"); test_detector_wrong_version();
    printf("\n[Detector — too-small input]\n"); test_detector_too_small();
    printf("\n[DM1 round-trip]\n");  test_dm1_round_trip();
    printf("\n[Multi-kind round-trip]\n"); test_multi_kind_round_trip();
    printf("\n[Kind mismatch rejection]\n"); test_kind_mismatch_rejection();
    printf("\n[Bad magic rejection]\n"); test_bad_magic_rejection();
    printf("\n[Target-exists no-overwrite]\n"); test_target_exists();
    printf("\n[Tampered CRC rejection]\n"); test_tampered_crc();
    printf("\n[BuildPaths helper]\n"); test_build_paths();
    printf("\n[Result code strings]\n"); test_result_strings();
    printf("\n[ExportFile autodetect]\n"); test_export_autodetect();

    printf("\n═══════════════════════════════════════════════════════\n");
    printf("  Total failures: %d\n", g_failures);
    printf("═══════════════════════════════════════════════════════\n");
    return g_failures == 0 ? 0 : 1;
}
