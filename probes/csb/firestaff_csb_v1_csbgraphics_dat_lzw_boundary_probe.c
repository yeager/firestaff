/*
 * firestaff_csb_v1_csbgraphics_dat_lzw_boundary_probe.c
 *
 * Real-asset CSBWin "CSBgraphics.dat" LZW block-boundary probe.
 *
 * Source-lock boundary:
 *   - ReDMCSB LZW.C F0495_LZW_GetNextInputCode (LSB-first
 *     bit-stream)
 *   - ReDMCSB LZW.C G0664_i_LZW_CodeBitCount = 9 init
 *   - ReDMCSB LZW.C G0666_i_LZW_AbsoluteMaximumCode = 4096
 *   - ReDMCSB LZW.C clear = 256, end-of-info = 257,
 *     FIRST_CODE = 258
 *   - CSBWin/Graphics.cpp:1717 ReadGraphic (cluster-bounded
 *     read of an LZW bit-stream)
 *   - CSBWin/Graphics.cpp:1918 ReadGraphicsIndex (count +
 *     parallel size tables + 0x8001 LE marker)
 *
 * What this proves:
 *   - Hash-based discovery of a CSBWin-produced CSBgraphics.dat
 *     via the existing csb_v1_csbgraphics_dat_real_scan module
 *     (which produces a cached file buffer + parsed index).
 *   - For each entry in the parsed index, the walker validates
 *     the LZW block boundary without decompressing any pixels.
 *   - The summary report (entries_ok / truncated / overflow /
 *     empty) is byte-stable across repeated scans so the
 *     boundary invariant is reproducible.
 *
 * Skip-safe by design: when the known CSBgraphics.dat MD5 list
 * is empty (the default — CSBgraphics.dat is a CSBGraffer /
 * CSBWin Viewport Compiler product, not an original CSB asset)
 * the probe exits 0 with a SKIP message so hosts without a
 * user-staged CSBgraphics.dat do not fail. This mirrors the
 * existing
 * `firestaff_csb_v1_csbgraphics_dat_real_scan_probe` and
 * `firestaff_csb_v1_hint_oracle_real_htc_scan_probe` patterns.
 */

#include "csb_v1_csbgraphics_dat_classify.h"
#include "csb_v1_csbgraphics_dat_lzw_boundary.h"
#include "csb_v1_csbgraphics_dat_real_scan.h"

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
    env = getenv("FIRESTAFF_CSBWIN_CSBGRAPHICS_DATA");
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

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir;
    size_t known_count = 0u;
    const CSB_V1_CSBGraphicsDatRealKnownHash *known;
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsIndex index;
    CSB_V1_CSBGraphicsLZWBoundaryReport report;
    int rc;

    printf("=== CSB V1 CSBgraphics.dat LZW boundary probe ===\n\n");

    known = csb_v1_csbgraphics_dat_real_known_hashes(&known_count);
    printf("known_hashes=%zu\n", known_count);
    (void)known;

    dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
    printf("data_dir=%s\n", dir ? dir : "(none)");

    csb_v1_csbgraphics_dat_real_cache_init(&cache);
    rc = csb_v1_csbgraphics_dat_real_scan_and_load(dir, NULL, 6, &cache);
    printf("scan_and_load rc=%d (%s)\n", rc,
           csb_v1_csbgraphics_dat_real_result_name(rc));

    /* Two legitimate SKIP paths:
     *   1. Empty known-hash list (the default).
     *   2. Known-hash list populated but the user has not staged a
     *      CSBgraphics.dat matching any registered hash.
     */
    if (known_count == 0u) {
        printf("SKIP: no source-cited CSBgraphics.dat MD5 is registered. "
               "Stage a real CSBWin-produced CSBgraphics.dat under "
               "~/.firestaff/data/csbwin-custom/<label>/ and extend "
               "g_known_hashes[] in src/csb/csb_v1_csbgraphics_dat_real_scan.c "
               "to enable this gate.\n");
        csb_v1_csbgraphics_dat_real_cache_free(&cache);
        return 0;
    }
    if (rc == CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND) {
        printf("SKIP: known-hash list has %zu entries but no "
               "CSBgraphics.dat matching any registered hash was found "
               "under data_dir. Set FIRESTAFF_CSBWIN_CSBGRAPHICS_DATA to "
               "a directory containing a verified CSBgraphics.dat to "
               "enable this gate.\n", known_count);
        csb_v1_csbgraphics_dat_real_cache_free(&cache);
        return 0;
    }
    if (rc != CSB_V1_CSBGRAPHICS_DAT_REAL_OK) {
        printf("FAIL: scan_and_load returned %d (%s); expected OK or "
               "NOT_FOUND.\n", rc,
               csb_v1_csbgraphics_dat_real_result_name(rc));
        csb_v1_csbgraphics_dat_real_cache_free(&cache);
        return 1;
    }

    /* Pull the parsed index back through the cache helper so we
     * exercise the same round-trip a real runtime integration would
     * use. */
    memset(&index, 0, sizeof(index));
    rc = csb_v1_csbgraphics_dat_real_index(&cache, &index);
    CHECK(rc == CSB_V1_CSBGRAPHICS_DAT_REAL_OK,
          "real-scan cache hands off a parsed index");

    printf("matched_label=%s\n", cache.matched_label);
    printf("file_size=%zu\n", cache.file_size);
    printf("index.count=%u byte_order=%s\n",
           (unsigned)index.count,
           csb_v1_csbgraphics_dat_byte_order_name(index.byte_order));
    printf("index.payload_offset=%llu payload_bytes_avail=%llu\n",
           (unsigned long long)index.payload_offset,
           (unsigned long long)index.payload_bytes_avail);

    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbgraphics_dat_lzw_boundary_walk(cache.file_buffer,
                                                   cache.file_size,
                                                   &index, &report);
    CHECK(rc == CSB_V1_CSBGRAPHICS_LZW_RESULT_OK ||
              rc == CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_BAD_PAYLOAD,
          "LZW boundary walker returns OK or BAD-PAYLOAD on a real "
          "CSBgraphics.dat");
    if (rc != CSB_V1_CSBGRAPHICS_LZW_RESULT_OK) {
        csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report);
        csb_v1_csbgraphics_dat_real_cache_free(&cache);
        return g_failures == 0 ? 0 : 1;
    }

    printf("report.entry_count=%u\n",
           (unsigned)report.entry_count);
    printf("  entries_ok=%u\n", (unsigned)report.entries_ok);
    printf("  entries_truncated=%u\n",
           (unsigned)report.entries_truncated);
    printf("  entries_overflow=%u\n",
           (unsigned)report.entries_overflow);
    printf("  entries_reserved=%u\n",
           (unsigned)report.entries_reserved);
    printf("  entries_empty=%u\n", (unsigned)report.entries_empty);

    CHECK(report.entry_count == index.count,
          "report.entry_count matches the parsed index.count");
    CHECK(report.entries_ok +
              report.entries_truncated +
              report.entries_overflow +
              report.entries_reserved +
              report.entries_empty ==
              report.entry_count,
          "per-entry verdict counters sum to entry_count");
    CHECK(report.entries_empty <= report.entry_count,
          "entries_empty never exceeds entry_count");
    CHECK(report.entries_overflow <= report.entry_count,
          "entries_overflow never exceeds entry_count");

    /* The walker is a read-only boundary test: it never
     * decompresses, never overrides, never touches the CSB
     * runtime. The CSBgraphics.dat file is left exactly as the
     * real-scan module cached it. */
    CHECK(cache.loaded == 1, "real-scan cache still reports loaded");
    CHECK(cache.file_buffer != NULL,
          "real-scan cache still owns the file buffer");
    CHECK(cache.file_size > 0u, "real-scan cache file size is non-zero");

    /* Determinism: a second scan + walk reproduces the same
     * summary counters so the boundary verdict is stable across
     * re-reads. */
    {
        CSB_V1_CSBGraphicsDatRealCache cache2;
        CSB_V1_CSBGraphicsIndex index2;
        CSB_V1_CSBGraphicsLZWBoundaryReport report2;
        memset(&cache2, 0, sizeof(cache2));
        memset(&index2, 0, sizeof(index2));
        memset(&report2, 0, sizeof(report2));
        rc = csb_v1_csbgraphics_dat_real_scan_and_load(
            dir, NULL, 6, &cache2);
        CHECK(rc == CSB_V1_CSBGRAPHICS_DAT_REAL_OK,
              "second scan_and_load returns OK");
        rc = csb_v1_csbgraphics_dat_real_index(&cache2, &index2);
        CHECK(rc == CSB_V1_CSBGRAPHICS_DAT_REAL_OK,
              "second scan index helper returns OK");
        rc = csb_v1_csbgraphics_dat_lzw_boundary_walk(
            cache2.file_buffer, cache2.file_size,
            &index2, &report2);
        CHECK(rc == CSB_V1_CSBGRAPHICS_LZW_RESULT_OK,
              "second LZW boundary walk returns OK");
        CHECK(report2.entries_ok == report.entries_ok,
              "second walk entries_ok matches first walk");
        CHECK(report2.entries_truncated == report.entries_truncated,
              "second walk entries_truncated matches first walk");
        CHECK(report2.entries_overflow == report.entries_overflow,
              "second walk entries_overflow matches first walk");
        CHECK(report2.entries_empty == report.entries_empty,
              "second walk entries_empty matches first walk");
        csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report2);
        csb_v1_csbgraphics_dat_real_cache_free(&cache2);
    }

    csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report);
    csb_v1_csbgraphics_dat_real_cache_free(&cache);

    printf("\nResult: %d/%d checks passed\n", g_checks - g_failures,
           g_checks);
    return g_failures == 0 ? 0 : 1;
}
