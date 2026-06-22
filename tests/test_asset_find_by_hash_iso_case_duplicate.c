/*
 * test_asset_find_by_hash_iso_case_duplicate.c
 *
 * Narrow ISO 9660 scanner regression covering two virtual-path edge cases
 * that the existing test_asset_find_by_hash gate does not exercise:
 *
 *   1. ISO 9660 directory entries whose on-disk names differ only in case
 *      (e.g. mixed-case Joliet-like "dungeon.dat" vs ISO-level "DUNGEON.DAT")
 *      must still hash-resolve correctly, and the reported virtual path
 *      must preserve the actual on-disk case (no silent uppercasing).
 *
 *   2. Two ISO directory entries with the SAME MD5 (duplicate matching hash
 *      across an ISO image) must resolve to a DETERMINISTIC virtual path,
 *      not "first match wins" which depends on the directory walk order
 *      chosen by whatever tool wrote the ISO. This mirrors the tiebreak
 *      that the ZIP scanner (scan_zip_by_md5_list) already enforces via
 *      is_better_zip_entry().
 *
 * Both regression cases are constructed as synthetic ISO 9660 PVD +
 * root-directory fixtures with hand-rolled directory records. No real
 * game assets are required, so this gate is data-free.
 *
 * Reference: ISO 9660:1988 §6.8.1 (Directory Record),
 *            ReDMCSB CEDTINCA.C F7059_ReadDungeonPartWithChecksum
 *            (hash-verified asset identity contract).
 */

#include "asset_find_by_hash.h"
#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0700)
#define RMDIR(path) rmdir(path)
#endif

/* Mirrors the fixture payload used by test_asset_find_by_hash.c so the
 * MD5 stays identical across the two regression suites — this also lets
 * us reuse the same documented hex digest as a lookup key. */
static const char kIsoPayload[] = "Firestaff hash identity fixture v1\n";
static const char kIsoPayloadMd5[] = "08c53652f85abfe8a075d5de4d3c8287";
static const char kIsoPayloadMd5Upper[] = "08C53652F85ABFE8A075D5DE4D3C8287";

static int failures;

static void check_int(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static void put32(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8U) & 0xffU);
    p[2] = (unsigned char)((v >> 16U) & 0xffU);
    p[3] = (unsigned char)((v >> 24U) & 0xffU);
}

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

/* Builds a 24-sector synthetic ISO image:
 *
 *   sector 0..15  : zero padding (sectors before PVD)
 *   sector 16     : Primary Volume Descriptor (PVD) with type=1, "CD001"
 *   sector 17..19 : zero padding (terminator + Volume Descriptor Set)
 *   sector 20     : root directory (dot, dotdot, and one or more files)
 *   sector 21     : file payload (zero-padded to 2048 bytes)
 *
 * `rootRecords` is a caller-owned byte buffer that will be appended into
 * the root directory; it must already include the dot, dotdot, and file
 * records packed back-to-back. The on-disk file payload is the SAME for
 * every file record, so every file's MD5 will match kIsoPayloadMd5.
 *
 * `rootRecordsSize` must be <= 2048. */
static int write_synthetic_iso(const char* path,
                               const unsigned char* rootRecords,
                               int rootRecordsSize) {
    FILE* fp = fopen(path, "wb");
    unsigned char zero[2048] = {0};
    unsigned char pvd[2048] = {0};
    unsigned char dir[2048] = {0};
    unsigned char fileSector[2048] = {0};
    unsigned char dot = 0;
    unsigned char dotdot = 1;
    int recLen;
    int offset;
    if (!fp) return 0;
    for (int i = 0; i < 16; ++i) {
        if (fwrite(zero, 1U, sizeof(zero), fp) != sizeof(zero)) {
            fclose(fp);
            return 0;
        }
    }
    /* PVD: type=1, magic "CD001", version=1, root dir record at byte 156. */
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
    /* Root directory: dot, dotdot, then caller-provided file records. */
    if (rootRecordsSize > 2048) {
        fclose(fp);
        return 0;
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
    if (offset + rootRecordsSize > 2048) {
        fclose(fp);
        return 0;
    }
    memcpy(dir + offset, rootRecords, (size_t)rootRecordsSize);
    if (fwrite(dir, 1U, sizeof(dir), fp) != sizeof(dir)) {
        fclose(fp);
        return 0;
    }
    /* File payload sector — both records point here. */
    memcpy(fileSector, kIsoPayload, sizeof(kIsoPayload) - 1U);
    if (fwrite(fileSector, 1U, sizeof(fileSector), fp) != sizeof(fileSector)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

/* ---- Regression 1: case-insensitive virtual-path reporting ----
 *
 * Two ISO directory records that BOTH point to the same payload but
 * have on-disk names that differ only in case ("dungeon.dat" vs
 * "DUNGEON.DAT"). The scanner's MD5 lookup must succeed against
 * either name, and the reported virtual path must preserve the
 * actual on-disk case (no silent uppercasing). */
static void check_iso_case_insensitive_virtual_path(const char* root) {
    char isoPath[512];
    unsigned char rootRecords[2048] = {0};
    int offset = 0;
    int recLen;
    const unsigned char* lowerName = (const unsigned char*)"dungeon.dat;1";
    const unsigned char* upperName = (const unsigned char*)"DUNGEON.DAT;1";
    char outPath[ASSET_PATH_MAX];

    if (snprintf(isoPath, sizeof(isoPath), "%s/case_insensitive.iso", root) >=
        (int)sizeof(isoPath)) {
        check_int(0, "case-insensitive ISO path should fit");
        return;
    }

    /* Write the two case-variant file records pointing at the same LBA. */
    recLen = write_iso_dir_record(rootRecords, offset, 21U,
                                  (unsigned int)(sizeof(kIsoPayload) - 1U), 0,
                                  lowerName, (int)strlen((const char*)lowerName));
    check_int(recLen > 0, "lower-case ISO dir record should fit");
    offset += recLen;
    recLen = write_iso_dir_record(rootRecords, offset, 21U,
                                  (unsigned int)(sizeof(kIsoPayload) - 1U), 0,
                                  upperName, (int)strlen((const char*)upperName));
    check_int(recLen > 0, "upper-case ISO dir record should fit");

    check_int(write_synthetic_iso(isoPath, rootRecords, offset + recLen),
              "case-insensitive ISO fixture should be written");

    /* MD5 lookup against the lowercase-only fixture must succeed and
     * report the lowercase on-disk entry name in the virtual path. */
    memset(outPath, 0, sizeof(outPath));
    check_int(asset_find_by_md5(root, kIsoPayloadMd5,
                                outPath, (int)sizeof(outPath), 2) &&
                  strstr(outPath, "case_insensitive.iso::dungeon.dat") != NULL,
              "MD5 lookup against mixed-case ISO should resolve to the "
              "lowercase on-disk entry name (case-preserving virtual path)");

    /* MD5 lookup with uppercase hex digits (the asset_status_m12 path
     * lowercases before comparing) must also succeed and still report
     * the on-disk lowercase case. */
    memset(outPath, 0, sizeof(outPath));
    check_int(asset_find_by_md5(root, kIsoPayloadMd5Upper,
                                outPath, (int)sizeof(outPath), 2) &&
                  strstr(outPath, "case_insensitive.iso::dungeon.dat") != NULL,
              "uppercase-hex MD5 lookup must still resolve to the "
              "lowercase on-disk entry name in the virtual path");

    /* asset_find_by_md5_list must report the same lowercase entry. */
    memset(outPath, 0, sizeof(outPath));
    {
        const char* md5List[] = {kIsoPayloadMd5, NULL};
        int matchIndex = -1;
        check_int(asset_find_by_md5_list(root, md5List,
                                         outPath, (int)sizeof(outPath),
                                         &matchIndex, 2) &&
                  matchIndex == 0 &&
                  strstr(outPath, "case_insensitive.iso::dungeon.dat") != NULL,
                  "MD5-list lookup against mixed-case ISO must report the "
                  "lowercase on-disk entry name");
    }

    /* asset_extract_virtual_path on the lowercase virtual path must
     * extract the same payload that asset_find_by_md5 found. */
    {
        char extractedPath[512];
        char buf[128];
        FILE* fp;
        size_t n;
        if (snprintf(extractedPath, sizeof(extractedPath),
                     "%s/case_insensitive_extracted.dat", root) >=
            (int)sizeof(extractedPath)) {
            check_int(0, "extracted ISO path should fit");
            return;
        }
        check_int(asset_extract_virtual_path(outPath, extractedPath),
                  "lowercase virtual path extraction should succeed");
        fp = fopen(extractedPath, "rb");
        check_int(fp != NULL, "extracted case-insensitive ISO entry should exist");
        if (fp) {
            n = fread(buf, 1U, sizeof(buf), fp);
            fclose(fp);
            check_int(n == sizeof(kIsoPayload) - 1U &&
                      memcmp(buf, kIsoPayload, sizeof(kIsoPayload) - 1U) == 0,
                      "extracted case-insensitive ISO entry should match the payload");
        }
    }

    /* The fixture's MD5 must match what we hard-code — protects against
     * accidental payload edits breaking the gate. m12_file_md5_hex always
     * emits lowercase hex, so the only meaningful sanity check is the
     * lowercase hex match. The uppercase-hex MD5 path was already
     * exercised by the asset_find_by_md5(kIsoPayloadMd5Upper, ...) call
     * above. */
    {
        char scratchPath[512];
        FILE* fp;
        char md5Out[33];
        if (snprintf(scratchPath, sizeof(scratchPath),
                     "%s/case_insensitive_md5_check.dat", root) >=
            (int)sizeof(scratchPath)) {
            check_int(0, "scratch MD5 path should fit");
            return;
        }
        fp = fopen(scratchPath, "wb");
        check_int(fp != NULL, "scratch MD5 file should be writable");
        if (fp) {
            size_t payloadSize = sizeof(kIsoPayload) - 1U;
            check_int(fwrite(kIsoPayload, 1U, payloadSize, fp) == payloadSize,
                      "scratch MD5 payload should be writable");
            fclose(fp);
        }
        check_int(m12_file_md5_hex(scratchPath, md5Out) &&
                  strcmp(md5Out, kIsoPayloadMd5) == 0,
                  "internal: fixture payload MD5 must match kIsoPayloadMd5");
        remove(scratchPath);
    }
}

/* ---- Regression 2: duplicate-hash tiebreak ----
 *
 * Two ISO directory records ("Z_DUPLICATE.DAT;1" first, then
 * "A_DUPLICATE.DAT;1") pointing at the same payload. The scanner
 * must report a DETERMINISTIC virtual path — and, mirroring the ZIP
 * duplicate-aware logic (is_better_zip_entry), that must be the
 * lexicographically smaller entry name so re-scans across ISO images
 * that swap directory-entry order produce the same virtual path.
 *
 * This guards against "first-match wins" non-determinism that would
 * cause CSB/DM2/Nexus materialization to write to different asset-cache
 * filenames depending on the tool that produced the ISO. */
static void check_iso_duplicate_hash_tiebreak(const char* root) {
    char isoPath[512];
    unsigned char rootRecords[2048] = {0};
    int offset = 0;
    int recLen;
    const unsigned char* zName = (const unsigned char*)"Z_DUPLICATE.DAT;1";
    const unsigned char* aName = (const unsigned char*)"A_DUPLICATE.DAT;1";
    char outPath[ASSET_PATH_MAX];

    if (snprintf(isoPath, sizeof(isoPath), "%s/duplicate_hash.iso", root) >=
        (int)sizeof(isoPath)) {
        check_int(0, "duplicate-hash ISO path should fit");
        return;
    }

    /* Z first, then A — so "first match wins" would report Z. */
    recLen = write_iso_dir_record(rootRecords, offset, 21U,
                                  (unsigned int)(sizeof(kIsoPayload) - 1U), 0,
                                  zName, (int)strlen((const char*)zName));
    check_int(recLen > 0, "Z_DUPLICATE dir record should fit");
    offset += recLen;
    recLen = write_iso_dir_record(rootRecords, offset, 21U,
                                  (unsigned int)(sizeof(kIsoPayload) - 1U), 0,
                                  aName, (int)strlen((const char*)aName));
    check_int(recLen > 0, "A_DUPLICATE dir record should fit");

    check_int(write_synthetic_iso(isoPath, rootRecords, offset + recLen),
              "duplicate-hash ISO fixture should be written");

    /* asset_find_by_md5 must deterministically report the lexicographically
     * smaller entry — "A_DUPLICATE.DAT" — not "Z_DUPLICATE.DAT". */
    memset(outPath, 0, sizeof(outPath));
    check_int(asset_find_by_md5(root, kIsoPayloadMd5,
                                outPath, (int)sizeof(outPath), 2) &&
                  strstr(outPath, "duplicate_hash.iso::A_DUPLICATE.DAT") != NULL,
              "duplicate-hash ISO must resolve to the lexicographically "
              "smaller entry name (mirrors ZIP is_better_zip_entry)");
    check_int(asset_find_by_md5(root, kIsoPayloadMd5,
                                outPath, (int)sizeof(outPath), 2) &&
                  strstr(outPath, "Z_DUPLICATE.DAT") == NULL,
              "duplicate-hash ISO must NOT resolve to the lexicographically "
              "larger entry name (first-match-wins regression)");

    /* asset_find_by_md5_list with a single-element list must agree. */
    {
        const char* md5List[] = {kIsoPayloadMd5, NULL};
        int matchIndex = -1;
        memset(outPath, 0, sizeof(outPath));
        check_int(asset_find_by_md5_list(root, md5List,
                                         outPath, (int)sizeof(outPath),
                                         &matchIndex, 2) &&
                  matchIndex == 0 &&
                  strstr(outPath, "duplicate_hash.iso::A_DUPLICATE.DAT") != NULL,
                  "duplicate-hash ISO md5-list lookup must resolve to A_DUPLICATE.DAT");
    }
}

static int make_isolated_root(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_iso_case_duplicate_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) return 0;
    return MKDIR(out) == 0;
#else
    char templatePath[] = "/tmp/firestaff-iso-case-duplicate-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static void cleanup_root(const char* root) {
    char p1[512], p2[512], p3[512], p4[512];
    if (snprintf(p1, sizeof(p1), "%s/case_insensitive.iso", root) <
        (int)sizeof(p1)) {
        remove(p1);
    }
    if (snprintf(p2, sizeof(p2), "%s/duplicate_hash.iso", root) <
        (int)sizeof(p2)) {
        remove(p2);
    }
    if (snprintf(p3, sizeof(p3), "%s/case_insensitive_extracted.dat", root) <
        (int)sizeof(p3)) {
        remove(p3);
    }
    if (snprintf(p4, sizeof(p4), "%s/case_insensitive_md5_check.dat", root) <
        (int)sizeof(p4)) {
        remove(p4);
    }
    RMDIR(root);
}

int main(void) {
    char root1[512];
    char root2[512];

    /* Each check gets its OWN isolated root so the MD5-match scan does
     * not see fixtures from the other check. Without this, the scanner
     * could find the same MD5 in both fixtures' .iso files and the
     * virtual-path reporting would race across checks. */
    if (!make_isolated_root(root1, sizeof(root1)) ||
        !make_isolated_root(root2, sizeof(root2))) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    check_iso_case_insensitive_virtual_path(root1);
    check_iso_duplicate_hash_tiebreak(root2);

    cleanup_root(root1);
    cleanup_root(root2);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: ISO scanner handles mixed-case Joliet-like names and duplicate-hash tiebreak deterministically");
    return 0;
}
