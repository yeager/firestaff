#include "nexus_v1_engine.h"
#include "firestaff_nexus_v1_boot_profile.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define TEST_GETPID() _getpid()
#define TEST_SEP "\\"
#else
#include <unistd.h>
#define TEST_GETPID() getpid()
#define TEST_SEP "/"
#endif

static int failures;

static void check_int(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int copy_file_bytes(const char* src, const char* dst) {
    unsigned char buf[8192];
    FILE* in = fopen(src, "rb");
    FILE* out;
    size_t n;
    if (!in) return 0;
    out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }
    while ((n = fread(buf, 1U, sizeof(buf), in)) > 0U) {
        if (fwrite(buf, 1U, n, out) != n) {
            fclose(in);
            fclose(out);
            return 0;
        }
    }
    fclose(in);
    return fclose(out) == 0;
}

static int make_root(char* out, size_t outBytes) {
    int rc = snprintf(out,
                      outBytes,
                      "%s%sfirestaff-nexus-boot-hash-%ld",
                      getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp",
                      TEST_SEP,
                      (long)TEST_GETPID());
    return rc > 0 && (size_t)rc < outBytes && FSP_CreateDirectoryRecursive(out);
}

static int local_file_exists(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}

int main(void) {
    const char* home = getenv("HOME");
    char root[FSP_PATH_MAX];
    char src[FSP_PATH_MAX];
    char dst[FSP_PATH_MAX];
    char dm_bin_src[FSP_PATH_MAX];
    char dm_bin_dst[FSP_PATH_MAX];
    char profile_root[FSP_PATH_MAX];
    char profile_nexus_dir[FSP_PATH_MAX];
    char profile_dm_bin_dst[FSP_PATH_MAX];
    Nexus_V1_Engine engine;
    Nexus_V1_BootProfile profile;
    Nexus_V1_Diagnostic diags[4];
    uint8_t* data;
    int size = 0;

    if (!home || !home[0]) {
        puts("SKIP: HOME unset");
        return 0;
    }
    if (!FSP_JoinPath(src, sizeof(src), home, ".firestaff/data/nexus/TITLE.CG") ||
        !local_file_exists(src)) {
        puts("SKIP: local Nexus TITLE.CG not present");
        return 0;
    }
    check_int(make_root(root, sizeof(root)), "temp Nexus root created");
    check_int(FSP_JoinPath(dst, sizeof(dst), root, "renamed-title.payload"),
              "renamed path built");
    check_int(copy_file_bytes(src, dst), "TITLE.CG copied under arbitrary name");

    memset(&engine, 0, sizeof(engine));
    engine.source = NEXUS_SRC_EXTRACTED;
    snprintf(engine.data_dir, sizeof(engine.data_dir), "%s", root);

    data = nexus_v1_read_file(&engine, "TITLE.CG", &size);
    check_int(data != NULL, "Nexus TITLE.CG resolves by hash when renamed");
    check_int(size > 100000, "renamed Nexus TITLE.CG size is plausible");
    free(data);

    if (FSP_JoinPath(dm_bin_src, sizeof(dm_bin_src), home, ".firestaff/data/nexus/DM.BIN") &&
        local_file_exists(dm_bin_src) &&
        FSP_JoinPath(dm_bin_dst, sizeof(dm_bin_dst), root, "renamed-saturn-data.payload") &&
        copy_file_bytes(dm_bin_src, dm_bin_dst)) {
        memset(&engine, 0, sizeof(engine));
        check_int(nexus_v1_init(&engine, root) == 0,
                  "Nexus init accepts renamed DM.BIN marker by hash");
        check_int(engine.source == NEXUS_SRC_EXTRACTED,
                  "renamed DM.BIN selects extracted Nexus source");
        nexus_v1_shutdown(&engine);

        check_int(FSP_JoinPath(profile_root, sizeof(profile_root), root, "profile-root") &&
                  FSP_JoinPath(profile_nexus_dir, sizeof(profile_nexus_dir), profile_root, "nexus") &&
                  FSP_CreateDirectoryRecursive(profile_nexus_dir) &&
                  FSP_JoinPath(profile_dm_bin_dst,
                               sizeof(profile_dm_bin_dst),
                               profile_nexus_dir,
                               "renamed-dm-bin.marker") &&
                  copy_file_bytes(dm_bin_src, profile_dm_bin_dst),
                  "renamed DM.BIN profile fixture written");
        memset(&profile, 0, sizeof(profile));
        memset(diags, 0, sizeof(diags));
        check_int(Nexus_V1_BootProfile_Init(&profile, profile_root, profile_root, 0U) == 0,
                  "Nexus boot profile initialized for renamed DM.BIN root");
        (void)Nexus_V1_BootProfile_ValidateAssets(&profile, diags, 4U);
        check_int(strstr(diags[0].detail, "DM.BIN") == NULL,
                  "Nexus boot profile accepts renamed DM.BIN by hash");
    } else {
        puts("SKIP: local Nexus DM.BIN not present for init hash test");
    }

    if (failures) return 1;
    puts("ok: Nexus boot file resolver finds renamed startup files by hash");
    return 0;
}
