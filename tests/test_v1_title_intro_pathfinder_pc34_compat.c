#include "v1_title_intro_pathfinder_pc34_compat.h"
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

const char *M12_StartupMenu_AssetDataDir(const M12_StartupMenuState *state)
{
    (void)state;
    return NULL;
}

const M12_AssetVersionStatus *M12_StartupMenu_AssetVersion(
    const M12_StartupMenuState *state,
    const char *gameId,
    int index)
{
    (void)state;
    return index >= 0 ? M12_AssetStatus_GetVersion(NULL, gameId, (size_t)index)
                      : NULL;
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

static void put16(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8U) & 0xffU);
}

static void put32(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8U) & 0xffU);
    p[2] = (unsigned char)((v >> 16U) & 0xffU);
    p[3] = (unsigned char)((v >> 24U) & 0xffU);
}

static int write_stored_zip_from_file(const char *zip_path,
                                      const char *entry_name,
                                      const char *src_path)
{
    unsigned char *payload;
    long payload_size;
    unsigned char local[30] = {0};
    unsigned char central[46] = {0};
    unsigned char eocd[22] = {0};
    unsigned int name_len;
    unsigned int central_offset;
    FILE *src;
    FILE *zip;
    if (!zip_path || !entry_name || !src_path) {
        return 0;
    }
    src = fopen(src_path, "rb");
    if (!src) {
        return 0;
    }
    if (fseek(src, 0L, SEEK_END) != 0) {
        fclose(src);
        return 0;
    }
    payload_size = ftell(src);
    if (payload_size <= 0L || payload_size > 65535L ||
        fseek(src, 0L, SEEK_SET) != 0) {
        fclose(src);
        return 0;
    }
    payload = (unsigned char *)malloc((size_t)payload_size);
    if (!payload) {
        fclose(src);
        return 0;
    }
    if (fread(payload, 1, (size_t)payload_size, src) != (size_t)payload_size) {
        free(payload);
        fclose(src);
        return 0;
    }
    fclose(src);

    zip = fopen(zip_path, "wb");
    if (!zip) {
        free(payload);
        return 0;
    }
    name_len = (unsigned int)strlen(entry_name);
    put32(local, 0x04034b50U);
    put16(local + 4, 20U);
    put16(local + 8, 0U);
    put32(local + 18, (unsigned int)payload_size);
    put32(local + 22, (unsigned int)payload_size);
    put16(local + 26, name_len);
    if (fwrite(local, 1U, sizeof(local), zip) != sizeof(local) ||
        fwrite(entry_name, 1U, name_len, zip) != name_len ||
        fwrite(payload, 1U, (size_t)payload_size, zip) != (size_t)payload_size) {
        free(payload);
        fclose(zip);
        return 0;
    }
    free(payload);
    central_offset = (unsigned int)ftell(zip);
    put32(central, 0x02014b50U);
    put16(central + 4, 20U);
    put16(central + 6, 20U);
    put16(central + 10, 0U);
    put32(central + 20, (unsigned int)payload_size);
    put32(central + 24, (unsigned int)payload_size);
    put16(central + 28, name_len);
    if (fwrite(central, 1U, sizeof(central), zip) != sizeof(central) ||
        fwrite(entry_name, 1U, name_len, zip) != name_len) {
        fclose(zip);
        return 0;
    }
    put32(eocd, 0x06054b50U);
    put16(eocd + 8, 1U);
    put16(eocd + 10, 1U);
    put32(eocd + 12, (unsigned int)(sizeof(central) + name_len));
    put32(eocd + 16, central_offset);
    if (fwrite(eocd, 1U, sizeof(eocd), zip) != sizeof(eocd)) {
        fclose(zip);
        return 0;
    }
    return fclose(zip) == 0;
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
    char archive_root[512];
    char direct_title[512];
    char renamed_title[512];
    char archive_path[512];
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
    snprintf(archive_root, sizeof(archive_root), "%s%sarchive-title", root, TEST_SEP);
    snprintf(renamed_title,
             sizeof(renamed_title),
             "%s%sintro.payload",
             nested,
             TEST_SEP);
    snprintf(direct_title, sizeof(direct_title), "%s%sTITLE", root, TEST_SEP);
    snprintf(archive_path,
             sizeof(archive_path),
             "%s%srenamed-title-pack.zip",
             archive_root,
             TEST_SEP);

    expect_true(TEST_MKDIR(root) == 0, "temp root created");
    expect_true(TEST_MKDIR(nested) == 0, "nested dir created");
    expect_true(TEST_MKDIR(archive_root) == 0, "archive root created");
    expect_true(copy_file(source_title, renamed_title), "canonical TITLE copied to arbitrary filename");
    expect_true(copy_file(source_title, direct_title), "canonical TITLE copied under its install name");
    expect_true(write_stored_zip_from_file(archive_path,
                                           "nested/not-title.bin",
                                           source_title),
                "canonical TITLE written inside arbitrary ZIP entry");

    memset(found, 0, sizeof(found));
    expect_true(V1_TitleIntro_FindTitleDatPath(NULL,
                                                root,
                                                found,
                                                sizeof(found)) == 1,
                "TITLE finder accepts the conventional loose install file");
    expect_true(strcmp(found, direct_title) == 0,
                "TITLE finder prefers a verified loose install file before archive scanning");

    remove(direct_title);
    memset(found, 0, sizeof(found));
    expect_true(V1_TitleIntro_FindTitleDatPath(NULL,
                                                root,
                                                found,
                                                sizeof(found)) == 1,
                "TITLE scan retains filename-independent hash fallback");
    expect_true(strcmp(found, renamed_title) == 0,
                "TITLE hash fallback resolves arbitrary named payload");

    remove(renamed_title);
    memset(found, 0, sizeof(found));
    expect_true(V1_TitleIntro_FindTitleDatPath(NULL,
                                                archive_root,
                                                found,
                                                sizeof(found)) == 1,
                "TITLE hash scan finds canonical payload inside ZIP");
    expect_true(strstr(found, "asset-cache") != NULL &&
                    strstr(found, "dm1") != NULL &&
                    V1_Title_IsCanonicalPc34Title(found, NULL, 0U),
                "TITLE ZIP payload is materialized as canonical runtime file");

    if (g_failures) {
        return 1;
    }
    printf("ok: TITLE intro pathfinder resolves arbitrary filename by canonical hash\n");
    return 0;
}
