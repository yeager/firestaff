#include "csb_v1_csbgraphics_dat_real_scan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#endif

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int actual, int expected)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d\n", label, actual, expected);
    } else {
        printf("ok %s = %d\n", label, actual);
    }
}

static void check_true(const char *label, int condition)
{
    check_int(label, condition ? 1 : 0, 1);
}

static void write_be16(unsigned char *buf, size_t off, unsigned value)
{
    buf[off] = (unsigned char)((value >> 8) & 0xffu);
    buf[off + 1u] = (unsigned char)(value & 0xffu);
}

static int write_file(const char *path, const void *buf, size_t size)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    if (size > 0u && fwrite(buf, 1u, size, fp) != size) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int make_temp_dir(char *path, size_t path_size)
{
    const char *tmp = getenv("TMPDIR");
    if (!tmp || tmp[0] == '\0') {
        tmp = "/tmp";
    }
    if (snprintf(path, path_size,
                 "%s/firestaff-csbgraphics-manifest-XXXXXX", tmp) <= 0) {
        return 0;
    }
#ifdef _WIN32
    {
        size_t len = strlen(path);
        if (len < 6u) {
            return 0;
        }
        memcpy(path + len - 6u, "win001", 6u);
        return TEST_MKDIR(path) == 0;
    }
#else
    return mkdtemp(path) != NULL;
#endif
}

static int join_path(char *out, size_t out_size,
                     const char *a, const char *b)
{
    return snprintf(out, out_size, "%s/%s", a, b) > 0;
}

static int build_c040_header_fixture(unsigned char *buf, size_t buf_size)
{
    enum {
        ENTRY_INDEX = 40,
        COUNT = ENTRY_INDEX + 1,
        HEADER_SIZE = 2 + COUNT * 4,
        DECOMPRESSED_SIZE = 112 * 29
    };
    if (!buf || buf_size != (size_t)(HEADER_SIZE + 1)) {
        return 0;
    }
    memset(buf, 0, buf_size);
    write_be16(buf, 0u, COUNT);
    write_be16(buf, 2u + (size_t)ENTRY_INDEX * 2u, 1u);
    write_be16(buf, 2u + (size_t)COUNT * 2u +
                    (size_t)ENTRY_INDEX * 2u,
               DECOMPRESSED_SIZE);
    return 1;
}

static void test_manifest_row_enables_hash_scan(void)
{
    static const char manifest[] =
        "# md5 size label\n"
        "8bbc5d9b41cf2f65eabe6a607d0d0918 167 "
        "synthetic-c040-header\n";
    char root[512];
    char nested[512];
    char manifest_path[512];
    char asset_path[512];
    unsigned char fixture[167];
    CSB_V1_CSBGraphicsDatRealCache cache;
    int rc;

    check_true("tmpdir", make_temp_dir(root, sizeof(root)));
    if (g_failures != 0) {
        return;
    }
    check_true("nested.path", join_path(nested, sizeof(nested),
                                        root, "custom"));
    check_int("nested.mkdir", TEST_MKDIR(nested), 0);
    check_true("manifest.path", join_path(manifest_path,
                                          sizeof(manifest_path),
                                          root, "csbgraphics.hashes"));
    check_true("asset.path", join_path(asset_path, sizeof(asset_path),
                                       nested, "not-a-trusted-name.bin"));
    check_true("fixture.build",
               build_c040_header_fixture(fixture, sizeof(fixture)));
    check_true("manifest.write",
               write_file(manifest_path, manifest, strlen(manifest)));
    check_true("asset.write",
               write_file(asset_path, fixture, sizeof(fixture)));

    csb_v1_csbgraphics_dat_real_cache_init(&cache);
    rc = csb_v1_csbgraphics_dat_real_scan_and_load(root, NULL, 4, &cache);
    check_int("scan.rc", rc, CSB_V1_CSBGRAPHICS_DAT_REAL_OK);
    check_int("scan.loaded", cache.loaded, 1);
    check_int("scan.file_size", (int)cache.file_size, 167);
    check_int("scan.index_count", (int)cache.index.count, 41);
    check_int("scan.max_decompressed",
              (int)cache.index.max_decompressed, 112 * 29);
    check_true("scan.md5",
               strcmp(cache.matched_md5,
                      "8bbc5d9b41cf2f65eabe6a607d0d0918") == 0);
    check_true("scan.label",
               strcmp(cache.matched_label,
                      "synthetic-c040-header") == 0);
    check_true("scan.path",
               strstr(cache.resolved_path,
                      "not-a-trusted-name.bin") != NULL);
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
}

static void test_manifest_absent_still_skips(void)
{
    char root[512];
    CSB_V1_CSBGraphicsDatRealCache cache;
    int rc;

    check_true("absent.tmpdir", make_temp_dir(root, sizeof(root)));
    if (g_failures != 0) {
        return;
    }
    csb_v1_csbgraphics_dat_real_cache_init(&cache);
    rc = csb_v1_csbgraphics_dat_real_scan_and_load(root, NULL, 4, &cache);
    check_int("absent.scan.rc", rc,
              CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND);
    check_int("absent.loaded", cache.loaded, 0);
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
}

int main(void)
{
    test_manifest_absent_still_skips();
    test_manifest_row_enables_hash_scan();

    printf("csbgraphics real-scan manifest tests: %d assertions, %d failures\n",
           g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
