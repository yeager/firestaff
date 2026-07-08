#include "title_intro_pathfinder_m11.h"
#include "title_dat_loader_v1.h"

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

static int copy_file(const char *src, const char *dst)
{
    FILE *in;
    FILE *out;
    unsigned char buf[4096];
    size_t n;
    in = fopen(src, "rb");
    if (!in) {
        return 0;
    }
    out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0U) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(in);
            fclose(out);
            return 0;
        }
    }
    fclose(in);
    fclose(out);
    return 1;
}

static int find_local_canonical_title(char *out, size_t out_bytes)
{
    const char *home = getenv("HOME");
    char candidate[1024];
    size_t i;
    static const char *suffixes[] = {
        ".firestaff/data/dm1/TITLE",
        ".firestaff/data/TITLE",
        ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE",
        ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/TITLE",
        ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34Multilingual/TITLE"
    };
    if (!home || !out || out_bytes == 0U) {
        return 0;
    }
    for (i = 0U; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        snprintf(candidate, sizeof(candidate), "%s%s%s", home, TEST_SEP, suffixes[i]);
        if (V1_Title_IsCanonicalPc34Title(candidate, NULL, 0U)) {
            snprintf(out, out_bytes, "%s", candidate);
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    char source_title[1024];
    char root[512];
    char nested[512];
    char renamed_title[512];
    char found[512];

    if (!find_local_canonical_title(source_title, sizeof(source_title))) {
        printf("skip: no local canonical DM1 TITLE fixture\n");
        return 0;
    }

    snprintf(root,
             sizeof(root),
             "%s%sfirestaff_title_path_%ld",
             getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp",
             TEST_SEP,
             (long)TEST_GETPID());
    snprintf(nested, sizeof(nested), "%s%srenamed-title", root, TEST_SEP);
    snprintf(renamed_title,
             sizeof(renamed_title),
             "%s%sintro.payload",
             nested,
             TEST_SEP);

    expect_true(TEST_MKDIR(root) == 0, "temp root created");
    expect_true(TEST_MKDIR(nested) == 0, "nested dir created");
    expect_true(copy_file(source_title, renamed_title), "canonical TITLE copied to arbitrary filename");

    memset(found, 0, sizeof(found));
    expect_true(M11_TitleIntro_FindTitleDatPath(NULL,
                                                root,
                                                found,
                                                sizeof(found)) == 1,
                "TITLE scan finds canonical payload without filename dependency");
    expect_true(strcmp(found, renamed_title) == 0,
                "TITLE scan resolves arbitrary named payload by hash");

    if (g_failures) {
        return 1;
    }
    printf("ok: TITLE intro pathfinder resolves arbitrary filename by canonical hash\n");
    return 0;
}
