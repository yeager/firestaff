#include "swsh_intro_pathfinder_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t M12_AssetStatus_GetVersionCount(const char *gameId)
{
    (void)gameId;
    return 0U;
}

const M12_AssetVersionStatus *M12_AssetStatus_GetVersion(
    const M12_AssetStatus *status,
    const char *gameId,
    size_t index)
{
    (void)status;
    (void)gameId;
    (void)index;
    return NULL;
}

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_GETPID() _getpid()
#define TEST_SEP "\\"
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_GETPID() getpid()
#define TEST_SEP "/"
#endif

static int g_failures = 0;

static void expect_true(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static size_t build_raw_swsh(unsigned char *out, size_t out_bytes)
{
    unsigned int pos = 0U;
    unsigned int row;
    if (!out || out_bytes < 2048U) {
        return 0U;
    }
    memset(out, 0, out_bytes);
    out[pos++] = 0x40U;
    out[pos++] = 0x01U;
    out[pos++] = 0xc8U;
    out[pos++] = 0x00U;
    for (row = 0U; row < 51U; ++row) {
        out[pos++] = 0xc0U;
        out[pos++] = 0x01U;
        out[pos++] = 0x3fU;
    }
    for (row = 0U; row < 119U; ++row) {
        out[pos++] = 0x80U;
        out[pos++] = 0x17U;
        out[pos++] = 0x8fU;
        out[pos++] = 0x51U;
        out[pos++] = 0x80U;
        out[pos++] = 0xa2U;
        out[pos++] = 0x0fU;
        out[pos++] = 0x80U;
        out[pos++] = 0x31U;
    }
    for (row = 0U; row < 30U; ++row) {
        out[pos++] = 0xc0U;
        out[pos++] = 0x01U;
        out[pos++] = 0x3fU;
    }
    return (size_t)pos;
}

static int write_swsh(const char *path)
{
    unsigned char raw[2048];
    size_t bytes = build_raw_swsh(raw, sizeof(raw));
    FILE *f;
    if (!path || bytes == 0U) {
        return 0;
    }
    f = fopen(path, "wb");
    if (!f) {
        return 0;
    }
    if (fwrite(raw, 1, bytes, f) != bytes) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

int main(void)
{
    char root[512];
    char dm1_dir[512];
    char csb_dir[512];
    char dm1_swsh[512];
    char csb_swsh[512];
    char found[512];

    snprintf(root,
             sizeof(root),
             "%s%sfirestaff_swsh_path_%ld",
             getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp",
             TEST_SEP,
             (long)TEST_GETPID());
    snprintf(dm1_dir, sizeof(dm1_dir), "%s%sdm1", root, TEST_SEP);
    snprintf(csb_dir, sizeof(csb_dir), "%s%scsb", root, TEST_SEP);
    snprintf(dm1_swsh, sizeof(dm1_swsh), "%s%sSWOOSH", dm1_dir, TEST_SEP);
    snprintf(csb_swsh, sizeof(csb_swsh), "%s%sSWOOSH", csb_dir, TEST_SEP);

    expect_true(TEST_MKDIR(root) == 0, "temp root created");
    expect_true(TEST_MKDIR(dm1_dir) == 0, "dm1 dir created");
    expect_true(TEST_MKDIR(csb_dir) == 0, "csb dir created");
    expect_true(write_swsh(dm1_swsh), "dm1 SWOOSH written");
    expect_true(write_swsh(csb_swsh), "csb SWOOSH written");

    memset(found, 0, sizeof(found));
    expect_true(M11_SWSH_Intro_FindLogoPathForGame(NULL,
                                                   root,
                                                   "csb",
                                                   found,
                                                   sizeof(found)) == 1,
                "csb SWOOSH found");
    expect_true(strcmp(found, csb_swsh) == 0,
                "csb SWOOSH path is preferred");

    memset(found, 0, sizeof(found));
    expect_true(M11_SWSH_Intro_FindLogoPath(NULL,
                                            root,
                                            found,
                                            sizeof(found)) == 1,
                "dm1 SWOOSH found through legacy wrapper");
    expect_true(strcmp(found, dm1_swsh) == 0,
                "legacy wrapper still resolves dm1 SWOOSH");

    if (g_failures) {
        return 1;
    }
    printf("ok: SWSH intro pathfinder resolves game-specific SWOOSH roots\n");
    return 0;
}
