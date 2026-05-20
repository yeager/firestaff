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

static void cleanup_fixture(void) {
    remove("asset_find_by_hash_test_tmp/nested/renamed.asset");
    RMDIR("asset_find_by_hash_test_tmp/nested");
    RMDIR("asset_find_by_hash_test_tmp");
}

static int path_has_fixture_name(const char* path) {
    return path && strstr(path, "renamed.asset") != NULL;
}

int main(void) {
    char outPath[ASSET_PATH_MAX];
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

    cleanup_fixture();
    return 0;
}
