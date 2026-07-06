#include "asset_find_by_hash.h"

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

static int write_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(payload, 1U, sizeof(payload) - 1U, fp) != sizeof(payload) - 1U) {
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

static int write_stored_zip_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    static const char name[] = "dm2/RENAMED.BIN";
    FILE* fp = fopen(path, "wb");
    unsigned char local[30] = {0};
    unsigned char central[46] = {0};
    unsigned char eocd[22] = {0};
    unsigned int payloadSize = (unsigned int)(sizeof(payload) - 1U);
    unsigned int nameLen = (unsigned int)(sizeof(name) - 1U);
    unsigned int centralOffset;
    if (!fp) return 0;

    put32(local, 0x04034b50U);
    put16(local + 4, 20U);
    put16(local + 8, 0U);
    put32(local + 18, payloadSize);
    put32(local + 22, payloadSize);
    put16(local + 26, nameLen);
    if (fwrite(local, 1U, sizeof(local), fp) != sizeof(local) ||
        fwrite(name, 1U, nameLen, fp) != nameLen ||
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
        fwrite(name, 1U, nameLen, fp) != nameLen) {
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

static int write_stored_zip_duplicate_hash_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    static const char* names[] = {"dm2/Z_DUPLICATE.BIN", "dm2/A_DUPLICATE.BIN"};
    FILE* fp = fopen(path, "wb");
    unsigned char local[30] = {0};
    unsigned char central[46] = {0};
    unsigned char eocd[22] = {0};
    unsigned int payloadSize = (unsigned int)(sizeof(payload) - 1U);
    unsigned int centralOffset;
    unsigned int centralSize = 0U;
    int i;
    if (!fp) return 0;

    for (i = 0; i < 2; ++i) {
        unsigned int nameLen = (unsigned int)strlen(names[i]);
        put32(local, 0x04034b50U);
        put16(local + 4, 20U);
        put16(local + 8, 0U);
        put32(local + 18, payloadSize);
        put32(local + 22, payloadSize);
        put16(local + 26, nameLen);
        if (fwrite(local, 1U, sizeof(local), fp) != sizeof(local) ||
            fwrite(names[i], 1U, nameLen, fp) != nameLen ||
            fwrite(payload, 1U, payloadSize, fp) != payloadSize) {
            fclose(fp);
            return 0;
        }
        memset(local, 0, sizeof(local));
    }

    centralOffset = (unsigned int)ftell(fp);
    for (i = 0; i < 2; ++i) {
        unsigned int nameLen = (unsigned int)strlen(names[i]);
        memset(central, 0, sizeof(central));
        put32(central, 0x02014b50U);
        put16(central + 4, 20U);
        put16(central + 6, 20U);
        put16(central + 10, 0U);
        put32(central + 20, payloadSize);
        put32(central + 24, payloadSize);
        put16(central + 28, nameLen);
        if (fwrite(central, 1U, sizeof(central), fp) != sizeof(central) ||
            fwrite(names[i], 1U, nameLen, fp) != nameLen) {
            fclose(fp);
            return 0;
        }
        centralSize += (unsigned int)(sizeof(central) + nameLen);
    }

    put32(eocd, 0x06054b50U);
    put16(eocd + 8, 2U);
    put16(eocd + 10, 2U);
    put32(eocd + 12, centralSize);
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

static int write_iso_fixture_payload(const char* path,
                                     const char* payload,
                                     size_t payloadSize);

static int write_iso_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    return write_iso_fixture_payload(path, payload, sizeof(payload) - 1U);
}

static int write_iso_fixture_payload(const char* path,
                                     const char* payload,
                                     size_t payloadSize) {
    static const unsigned char dot = 0;
    static const unsigned char dotdot = 1;
    static const unsigned char fileName[] = "DUNGEON.DAT;1";
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
    if (payloadSize > sizeof(fileSector)) {
        fclose(fp);
        return 0;
    }
    recLen = write_iso_record(dir, offset, 21U, (unsigned int)payloadSize,
                              0, fileName, (int)(sizeof(fileName) - 1U));
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    if (fwrite(dir, 1U, sizeof(dir), fp) != sizeof(dir)) {
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

static int write_cue_fixture(const char* path,
                             const char* firstPayload,
                             const char* secondPayload) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fprintf(fp,
                "FILE \"%s\" BINARY\n"
                "  TRACK 01 AUDIO\n"
                "    INDEX 01 00:00:00\n"
                "FILE \"%s\" BINARY\n"
                "  Track 02 Mode1/2048\n"
                "    INDEX 01 00:00:00\n",
                firstPayload, secondPayload) < 0) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void cleanup_fixture(void) {
    remove("asset_find_by_hash_test_tmp/nested/renamed.asset");
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/archive.zip");
    remove("asset_find_by_hash_test_tmp/disc.iso");
    remove("asset_find_by_hash_test_tmp/cue_a.payload");
    remove("asset_find_by_hash_test_tmp/cue_b.payload");
    remove("asset_find_by_hash_test_tmp/split.cue");
    RMDIR("asset_find_by_hash_test_tmp/nested");
    RMDIR("asset_find_by_hash_test_tmp");
}

static int path_has_fixture_name(const char* path) {
    return path && strstr(path, "renamed.asset") != NULL;
}

static int path_has_virtual_name(const char* path, const char* container, const char* entry) {
    return path && strstr(path, container) != NULL && strstr(path, "::") != NULL &&
           strstr(path, entry) != NULL;
}

static int path_has_virtual_entry(const char* path, const char* container, const char* entry) {
    char expected[ASSET_PATH_MAX];
    size_t pathLen;
    size_t expectedLen;
    const char* suffix;
    if (!path || !container || !entry) return 0;
    if (snprintf(expected, sizeof(expected), "%s::%s", container, entry) >= (int)sizeof(expected)) {
        return 0;
    }
    pathLen = strlen(path);
    expectedLen = strlen(expected);
    if (pathLen < expectedLen) return 0;
    suffix = path + (pathLen - expectedLen);
    return strcmp(suffix, expected) == 0;
}

static int file_matches_fixture_payload(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    char buf[128];
    FILE* fp = fopen(path, "rb");
    size_t n;
    if (!fp) return 0;
    n = fread(buf, 1U, sizeof(buf), fp);
    fclose(fp);
    return n == sizeof(payload) - 1U && memcmp(buf, payload, sizeof(payload) - 1U) == 0;
}

int main(void) {
    char outPath[ASSET_PATH_MAX];
    char outPaths[2][ASSET_PATH_MAX];
    int matched[2];
    const char* md5Upper = "08C53652F85ABFE8A075D5DE4D3C8287";
    const char* md5List[] = {
        "00000000000000000000000000000000",
        "08C53652F85ABFE8A075D5DE4D3C8287",
        NULL
    };
    int matchIndex = -1;

    cleanup_fixture();
    if (MKDIR("asset_find_by_hash_test_tmp") != 0 ||
        MKDIR("asset_find_by_hash_test_tmp/nested") != 0 ||
        !write_fixture("asset_find_by_hash_test_tmp/nested/renamed.asset")) {
        cleanup_fixture();
        fprintf(stderr, "fixture setup failed\n");
        return 1;
    }

    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_fixture_name(outPath)) {
        cleanup_fixture();
        fprintf(stderr, "uppercase MD5 recursive lookup failed: %s\n", outPath);
        return 1;
    }

    memset(outPath, 0, sizeof(outPath));
    if (asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                          outPath, 8, 2)) {
        cleanup_fixture();
        fprintf(stderr, "truncated output path should not be reported as a match\n");
        return 1;
    }

    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                outPath, (int)sizeof(outPath),
                                &matchIndex, 2) ||
        matchIndex != 1 ||
        !path_has_fixture_name(outPath)) {
        cleanup_fixture();
        fprintf(stderr, "MD5 list lookup failed: index=%d path=%s\n", matchIndex, outPath);
        return 1;
    }

    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_fixture_name(outPaths[1])) {
        cleanup_fixture();
        fprintf(stderr, "MD5 all-list file lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }

    remove("asset_find_by_hash_test_tmp/nested/renamed.asset");
    if (!write_stored_zip_fixture("asset_find_by_hash_test_tmp/archive.zip")) {
        cleanup_fixture();
        fprintf(stderr, "ZIP fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "archive.zip", "dm2/RENAMED.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "stored ZIP entry lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_name(outPaths[1], "archive.zip", "dm2/RENAMED.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "MD5 all-list ZIP lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual ZIP extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");

    remove("asset_find_by_hash_test_tmp/archive.zip");
    if (!write_stored_zip_duplicate_hash_fixture("asset_find_by_hash_test_tmp/archive.zip")) {
        cleanup_fixture();
        fprintf(stderr, "duplicate ZIP fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_entry(outPath, "archive.zip", "dm2/A_DUPLICATE.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "duplicate ZIP deterministic lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_entry(outPaths[1], "archive.zip", "dm2/A_DUPLICATE.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "duplicate ZIP all-list lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }

    remove("asset_find_by_hash_test_tmp/archive.zip");
    if (!write_iso_fixture("asset_find_by_hash_test_tmp/disc.iso")) {
        cleanup_fixture();
        fprintf(stderr, "ISO fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "disc.iso", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "ISO entry lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_name(outPaths[1], "disc.iso", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "MD5 all-list ISO lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual ISO extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/disc.iso");

    if (!write_iso_fixture_payload("asset_find_by_hash_test_tmp/cue_a.payload",
                                   "Firestaff non-matching CUE payload v1\n",
                                   strlen("Firestaff non-matching CUE payload v1\n")) ||
        !write_iso_fixture("asset_find_by_hash_test_tmp/cue_b.payload") ||
        !write_cue_fixture("asset_find_by_hash_test_tmp/split.cue",
                           "cue_a.payload",
                           "cue_b.payload")) {
        cleanup_fixture();
        fprintf(stderr, "split CUE fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "cue_b.payload", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "split CUE mixed-case data track lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_name(outPaths[1], "cue_b.payload", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "split CUE all-list lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "split CUE virtual extraction failed: %s\n", outPath);
        return 1;
    }

    cleanup_fixture();
    return 0;
}
