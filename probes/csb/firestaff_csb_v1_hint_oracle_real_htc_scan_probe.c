/*
 * firestaff_csb_v1_hint_oracle_real_htc_scan_probe.c
 *
 * Real Utility Disk HCSB.HTC scan/cache handoff probe.
 *
 * Source-lock boundary:
 *   - ReDMCSB HINTLOAD.C:11-18 names HCSB.HTC as the canonical CSB
 *     Utility Disk Hint Oracle content file.
 *   - ReDMCSB HINTHTC.C:177-358 validates the format 2 / dungeon 13
 *     big-endian table the parser expects.
 *   - dmweb Hint Oracle Files page describes the same layout
 *     (location records, hint records, page compressed-length pool,
 *     LZW-compressed content).
 *   - docs/VERIFIED_HASHES.md is updated separately; this probe
 *     records the local Atari ST 2.x PP 2009-02-22 HCSB.HTC
 *     hash explicitly so the scan path is self-checking.
 *
 * What this proves:
 *   - Hash-based discovery of a real HCSB.HTC in the user's
 *     ~/.firestaff/data tree (or the explicit data-dir override).
 *   - Cache handoff: parser view, owned file buffer, resolved path,
 *     and matched MD5/label are all populated after a single scan.
 *   - Real-asset location lookup: at least one (level, x, y) tuple
 *     returns a non-empty hint list using the 255/255 any-XY rule
 *     that the real Atari ST 2.x file uses for level-wide hints.
 *   - Real-asset hint lookup: hint name and first-page decompressed
 *     text are non-empty ASCII (the parser's HTC LZW decompression
 *     round-trips the real CSB hint content).
 *   - Determinism: a second scan reuses the same path/MD5 and
 *     produces identical lookup results.
 *
 * Skip-safe by design: when no known HCSB.HTC is present, the probe
 * exits 0 with a SKIP message so it does not block hosts without
 * CSB Utility Disk assets. This matches the existing
 * `firestaff_csb_v1_pc_real_asset_launch_probe` pattern.
 */

#include "csb_hint_oracle_htc_real_scan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_checks;
static int g_failures;

#define CHECK(cond, msg) do {                                              \
    ++g_checks;                                                            \
    if (cond) {                                                            \
        printf("  PASS: %s\n", msg);                                       \
    } else {                                                               \
        ++g_failures;                                                      \
        printf("  FAIL: %s\n", msg);                                       \
    }                                                                      \
} while (0)

static const char *data_dir_arg(int argc, char **argv,
                                char *buf, size_t buf_size)
{
    const char *env;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0] != '\0') {
        return argv[1];
    }
    env = getenv("FIRESTAFF_CSB_HTC_DATA");
    if (env && env[0] != '\0') {
        return env;
    }
    env = getenv("FIRESTAFF_DATA_DIR");
    if (env && env[0] != '\0') {
        return env;
    }
    home = getenv("HOME");
    if (!home || home[0] == '\0') {
        return NULL;
    }
    snprintf(buf, buf_size, "%s/.firestaff/data", home);
    return buf;
}

/* Sanity-print `buf` of `len` as ASCII with non-printables masked.
 * Returns 1 if every byte is printable ASCII or whitespace. */
static int is_printable_ascii(const uint8_t *buf, size_t len)
{
    size_t i;
    if (!buf || len == 0u) {
        return 0;
    }
    for (i = 0u; i < len; ++i) {
        unsigned char c = buf[i];
        if (c == '\r' || c == '\n' || c == '\t') {
            continue;
        }
        if (c < 0x20u || c > 0x7eu) {
            return 0;
        }
    }
    return 1;
}

static void print_ascii_preview(const uint8_t *buf, size_t len, size_t max)
{
    size_t i;
    size_t show = len < max ? len : max;
    for (i = 0u; i < show; ++i) {
        unsigned char c = buf[i];
        if (c == '\r' || c == '\n' || c == '\t' ||
            (c >= 0x20u && c <= 0x7eu)) {
            putchar((int)c);
        } else {
            putchar('.');
        }
    }
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    char scratch_buf[256];
    char hint_name[64];
    uint8_t page_buf[2048];
    size_t page_size = 0u;
    uint16_t indices[8];
    size_t count = 0u;
    size_t known_count = 0u;
    const CSB_HintOracleHTC_RealKnownHash *known;
    size_t i;
    const char *dir;
    CSB_HintOracleHTC_RealCache cache;
    int rc;
    int rc2;
    int saw_real_load = 0;

    printf("=== CSB V1 Hint Oracle real HCSB.HTC scan probe ===\n\n");

    known = csb_hint_oracle_htc_real_known_hashes(&known_count);
    printf("known_hashes=%zu\n", known_count);
    for (i = 0u; i < known_count; ++i) {
        printf("  [%zu] %s  md5=%s  size=%zu\n",
               i, known[i].label, known[i].md5, known[i].size_bytes);
    }
    CHECK(known_count >= 1u,
          "at least one source-cited HCSB.HTC MD5 is registered");

    dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
    printf("data_dir=%s\n", dir ? dir : "(none)");

    csb_hint_oracle_htc_real_cache_init(&cache);

    rc = csb_hint_oracle_htc_real_scan_and_load(dir, NULL, 6, &cache);
    printf("scan_and_load rc=%d (%s)\n", rc,
           csb_hint_oracle_htc_real_result_name(rc));
    if (rc == CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND) {
        printf("SKIP: no known HCSB.HTC found under data_dir; "
               "set FIRESTAFF_CSB_HTC_DATA to a directory containing "
               "a verified HCSB.HTC to enable this gate.\n");
        csb_hint_oracle_htc_real_cache_free(&cache);
        return 0;
    }
    if (rc != CSB_HINT_ORACLE_HTC_REAL_OK) {
        printf("FAIL: scan_and_load returned %d (%s); expected OK or "
               "NOT_FOUND.\n", rc,
               csb_hint_oracle_htc_real_result_name(rc));
        csb_hint_oracle_htc_real_cache_free(&cache);
        return 1;
    }
    saw_real_load = 1;

    printf("resolved_path=%s\n", cache.resolved_path);
    printf("original_path=%s\n", cache.original_path);
    printf("matched_md5=%s\n", cache.matched_md5);
    printf("matched_label=%s\n", cache.matched_label);
    printf("file_size=%zu\n", cache.file_size);
    printf("location_count=%zu hint_count=%zu page_count=%zu "
           "content_size=%zu\n",
           cache.htc.location_count, cache.htc.hint_count,
           cache.htc.page_count, cache.htc.content_size);

    CHECK(cache.loaded == 1, "cache reports loaded");
    CHECK(cache.file_buffer != NULL, "cache owns the file buffer");
    CHECK(cache.file_size > 0u, "cache file size is non-zero");
    CHECK(cache.matched_md5[0] != '\0', "cache records a matched MD5");
    CHECK(cache.matched_label[0] != '\0',
          "cache records a human-readable label");
    CHECK(cache.htc.format_word == CSB_HINT_ORACLE_HTC_FORMAT_WORD,
          "format word matches the parser contract (==2)");
    CHECK(cache.htc.dungeon_id == CSB_HINT_ORACLE_HTC_DUNGEON_ID,
          "dungeon id matches the parser contract (==13)");
    CHECK(cache.htc.location_count > 0u,
          "real HCSB.HTC exposes at least one location record");
    CHECK(cache.htc.hint_count > 0u,
          "real HCSB.HTC exposes at least one hint record");
    CHECK(cache.htc.page_count > 0u,
          "real HCSB.HTC exposes at least one compressed page");

    /* Cross-check the matched MD5 against the known list. */
    {
        int found = 0;
        size_t j;
        for (j = 0u; j < known_count; ++j) {
            if (strcmp(cache.matched_md5, known[j].md5) == 0) {
                found = 1;
                CHECK(cache.file_size == known[j].size_bytes,
                      "matched HCSB.HTC size matches the known list");
                break;
            }
        }
        CHECK(found,
              "matched MD5 is one of the source-cited known hashes");
    }

    /* Hint 0: name + first-page decompression. */
    rc = csb_hint_oracle_htc_real_get_hint_name(&cache, 0u, hint_name,
                                                sizeof(hint_name));
    CHECK(rc == CSB_HINT_ORACLE_HTC_REAL_OK,
          "real-asset hint 0 name lookup succeeds");
    printf("hint[0].name='%s'\n", hint_name);
    CHECK(hint_name[0] != '\0',
          "real-asset hint 0 name is non-empty");

    page_size = 0u;
    memset(page_buf, 0, sizeof(page_buf));
    rc = csb_hint_oracle_htc_real_decompress_first_page(
        &cache, 0u, page_buf, sizeof(page_buf) - 1u, &page_size);
    CHECK(rc == CSB_HINT_ORACLE_HTC_REAL_OK,
          "real-asset hint 0 first-page decompression succeeds");
    CHECK(page_size > 0u,
          "real-asset hint 0 first page produces non-empty content");
    /* The HTC LZW decompressor includes the trailing NUL terminator in
 * the output size. Treat printable ASCII as "all bytes up to the
 * first NUL (or end) are printable"; ignore any trailing NUL bytes. */
    {
        size_t content_end = page_size;
        size_t j;
        for (j = 0u; j < page_size; ++j) {
            if (page_buf[j] == 0u) {
                content_end = j;
                break;
            }
        }
        page_buf[page_size] = '\0';
        CHECK(content_end > 0u && is_printable_ascii(page_buf, content_end),
              "real-asset hint 0 first page is printable ASCII");
    }
    printf("hint[0].page0 first %zu bytes: \"",
           page_size < 96u ? page_size : 96u);
    print_ascii_preview(page_buf, page_size, 96u);
    printf("\"\n");

    /* Save hint 0 name before the next block overwrites it, so we can
 * compare it against the second-scan name later. */
    {
        size_t saved_len = strnlen(hint_name, sizeof(hint_name) - 1u);
        if (saved_len > sizeof(hint_name) - 1u) {
            saved_len = sizeof(hint_name) - 1u;
        }
        memmove(scratch_buf, hint_name, saved_len);
        scratch_buf[saved_len] = '\0';
    }

    /* (level, x, y) lookup using the real location table.
     * The Atari ST 2.x file uses (255, 255) as a wildcard XY at
     * every level, so we exercise that path explicitly. */
    rc = csb_hint_oracle_htc_real_find_hints_for_location(
        &cache, 0u,
        CSB_HINT_ORACLE_HTC_ANY_XY, CSB_HINT_ORACLE_HTC_ANY_XY,
        indices, sizeof(indices) / sizeof(indices[0]), &count);
    CHECK(rc == CSB_HINT_ORACLE_HTC_REAL_OK,
          "real-asset level-0 wildcard lookup succeeds");
    CHECK(count > 0u,
          "real-asset level-0 wildcard returns at least one hint");
    printf("level=0 (x=255,y=255) -> %zu hints:", count);
    for (i = 0u; i < count; ++i) {
        printf(" %u", (unsigned)indices[i]);
    }
    printf("\n");

    /* Hint 1 (different hint): name + first-page decompression. */
    if (cache.htc.hint_count > 1u) {
        rc = csb_hint_oracle_htc_real_get_hint_name(
            &cache, 1u, hint_name, sizeof(hint_name));
        CHECK(rc == CSB_HINT_ORACLE_HTC_REAL_OK,
              "real-asset hint 1 name lookup succeeds");
        printf("hint[1].name='%s'\n", hint_name);
        CHECK(hint_name[0] != '\0' &&
              strcmp(hint_name, "hint[0].name") != 0,
              "real-asset hint 1 name differs from hint 0");

        page_size = 0u;
        memset(page_buf, 0, sizeof(page_buf));
        rc = csb_hint_oracle_htc_real_decompress_first_page(
            &cache, 1u, page_buf, sizeof(page_buf) - 1u, &page_size);
        CHECK(rc == CSB_HINT_ORACLE_HTC_REAL_OK,
              "real-asset hint 1 first-page decompression succeeds");
        CHECK(page_size > 0u,
              "real-asset hint 1 first page produces non-empty content");
    }

    /* Determinism: a second scan reuses the same path and MD5 and
     * gives the same lookup results. */
    {
        CSB_HintOracleHTC_RealCache cache2;
        uint16_t indices2[8];
        size_t count2 = 0u;
        char hint_name2[64];

        csb_hint_oracle_htc_real_cache_init(&cache2);
        rc2 = csb_hint_oracle_htc_real_scan_and_load(
            dir, NULL, 6, &cache2);
        CHECK(rc2 == CSB_HINT_ORACLE_HTC_REAL_OK,
              "second scan_and_load succeeds (deterministic gate)");

        if (rc2 == CSB_HINT_ORACLE_HTC_REAL_OK) {
            CHECK(strcmp(cache2.matched_md5, cache.matched_md5) == 0,
                  "second scan matches the same MD5");
            CHECK(cache2.htc.location_count == cache.htc.location_count &&
                  cache2.htc.hint_count == cache.htc.hint_count,
                  "second scan reports the same location/hint counts");

            rc2 = csb_hint_oracle_htc_real_find_hints_for_location(
                &cache2, 0u,
                CSB_HINT_ORACLE_HTC_ANY_XY, CSB_HINT_ORACLE_HTC_ANY_XY,
                indices2, sizeof(indices2) / sizeof(indices2[0]),
                &count2);
            CHECK(rc2 == CSB_HINT_ORACLE_HTC_REAL_OK &&
                  count2 == count &&
                  memcmp(indices, indices2,
                         count * sizeof(indices[0])) == 0,
                  "second scan returns identical wildcard lookup");

            rc2 = csb_hint_oracle_htc_real_get_hint_name(
                &cache2, 0u, hint_name2, sizeof(hint_name2));
            CHECK(rc2 == CSB_HINT_ORACLE_HTC_REAL_OK &&
                  strcmp(hint_name2, scratch_buf) == 0,
                  "second scan returns identical hint 0 name");
        }
        csb_hint_oracle_htc_real_cache_free(&cache2);
    }

    /* Negative: empty data_dir (no discovery) returns NOT_FOUND. */
    {
        CSB_HintOracleHTC_RealCache cache3;
        csb_hint_oracle_htc_real_cache_init(&cache3);
        rc2 = csb_hint_oracle_htc_real_scan_and_load(
            "/tmp/firestaff-htc-definitely-missing-12345",
            NULL, 0, &cache3);
        CHECK(rc2 == CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND,
              "scan with empty/invalid data_dir returns NOT_FOUND");
        CHECK(cache3.loaded == 0,
              "failed scan leaves cache unloaded");
        csb_hint_oracle_htc_real_cache_free(&cache3);
    }

    csb_hint_oracle_htc_real_cache_free(&cache);

    (void)scratch_buf;
    printf("\nchecks=%d failures=%d saw_real_load=%d\n",
           g_checks, g_failures, saw_real_load);
    return g_failures == 0 ? 0 : 1;
}
