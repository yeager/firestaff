/*
 * firestaff_csb_v1_csbgraphics_dat_real_scan_probe.c
 *
 * Real-asset CSBWin "CSBgraphics.dat" scan/cache handoff probe.
 *
 * Source-lock boundary:
 *   - CSBWin/Graphics.cpp:1838 OpenCSBgraphicsFile (file name +
 *     MD5-derived signature split across two uint32 words).
 *   - CSBWin/Graphics.cpp:1918 ReadGraphicsIndex (count + parallel
 *     compressed/decompressed size tables, optional 0x8001 LE
 *     sentinel).
 *   - CSBWin/Graphics.cpp:1643 LocateNthGraphic (payload offset =
 *     NumGraphic * 4 + 2 [+2 when LE marker present]).
 *   - CSBWin/data.cpp:1936 Signature (file MD5 split).
 *   - ReDMCSB F0200_DMMISC_ReadCompressedGraphic, GRAPH21.C F0914.
 *
 * What this proves:
 *   - Hash-based discovery of a CSBWin-produced CSBgraphics.dat in
 *     the user's ~/.firestaff/data tree.
 *   - Cache handoff: the parse-view, owned file buffer, resolved
 *     path, and matched MD5/label are all populated after a single
 *     scan.
 *   - Real-asset index invariant: byte-order detection, count,
 *     total compressed / decompressed bytes, payload offset, and
 *     largest single entry are all reported and consistent.
 *   - Determinism: a second scan reuses the same path/MD5 and
 *     produces identical inventory.
 *
 * Skip-safe by design: when the known MD5 list is empty (the
 * default — CSBgraphics.dat is a CSBGraffer/CSBWin Viewport
 * Compiler product, not an original CSB asset) the probe exits 0
 * with a SKIP message so hosts without a user-staged CSBgraphics.dat
 * do not fail. This matches the existing
 * `firestaff_csb_v1_hint_oracle_real_htc_scan_probe` pattern.
 */

#include "csb_v1_csbgraphics_dat_classify.h"
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
    size_t i;
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsIndex index;
    int rc;

    printf("=== CSB V1 CSBgraphics.dat real-asset scan probe ===\n\n");

    known = csb_v1_csbgraphics_dat_real_known_hashes(&known_count);
    printf("known_hashes=%zu\n", known_count);
    for (i = 0u; i < known_count; ++i) {
        printf("  [%zu] %s  md5=%s  size=%zu\n",
               i, known[i].label, known[i].md5, known[i].size_bytes);
    }

    dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
    printf("data_dir=%s\n", dir ? dir : "(none)");

    csb_v1_csbgraphics_dat_real_cache_init(&cache);

    rc = csb_v1_csbgraphics_dat_real_scan_and_load(dir, NULL, 6, &cache);
    printf("scan_and_load rc=%d (%s)\n", rc,
           csb_v1_csbgraphics_dat_real_result_name(rc));

    /* Two legitimate SKIP paths:
     *   1. Empty known-hash list (the default — see header).
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

    printf("resolved_path=%s\n", cache.resolved_path);
    printf("original_path=%s\n", cache.original_path);
    printf("matched_md5=%s\n", cache.matched_md5);
    printf("matched_label=%s\n", cache.matched_label);
    printf("file_size=%zu\n", cache.file_size);
    printf("index.count=%u byte_order=%s\n",
           (unsigned)cache.index.count,
           csb_v1_csbgraphics_dat_byte_order_name(cache.index.byte_order));
    printf("index.total_compressed=%llu total_decompressed=%llu\n",
           (unsigned long long)cache.index.total_compressed,
           (unsigned long long)cache.index.total_decompressed);
    printf("index.payload_offset=%llu payload_bytes_avail=%llu\n",
           (unsigned long long)cache.index.payload_offset,
           (unsigned long long)cache.index.payload_bytes_avail);
    printf("index.max_compressed=%u max_decompressed=%u\n",
           (unsigned)cache.index.max_compressed,
           (unsigned)cache.index.max_decompressed);

    CHECK(cache.loaded == 1, "cache reports loaded");
    CHECK(cache.file_buffer != NULL, "cache owns the file buffer");
    CHECK(cache.file_size > 0u, "cache file size is non-zero");
    CHECK(cache.matched_md5[0] != '\0', "cache records a matched MD5");
    CHECK(cache.matched_label[0] != '\0',
          "cache records a human-readable label");
    CHECK(cache.index.count > 0u,
          "real CSBgraphics.dat reports a positive graphic count");
    CHECK(cache.index.total_compressed <= cache.index.payload_bytes_avail,
          "total_compressed fits inside the available payload region");
    CHECK(cache.index.payload_offset + cache.index.payload_bytes_avail
              == cache.file_size,
          "payload_offset + payload_bytes_avail equals file_size");
    CHECK(cache.index.max_compressed > 0u,
          "real CSBgraphics.dat reports a non-zero largest entry");
    CHECK(cache.index.total_decompressed >=
              (uint64_t)cache.index.max_decompressed,
          "total_decompressed >= max_decompressed");

    /* Cross-check the matched MD5 against the known list. */
    {
        int found = 0;
        size_t j;
        for (j = 0u; j < known_count; ++j) {
            if (strcmp(cache.matched_md5, known[j].md5) == 0) {
                found = 1;
                CHECK(cache.file_size == known[j].size_bytes,
                      "matched CSBgraphics.dat size matches the known list");
                break;
            }
        }
        CHECK(found,
              "matched MD5 is one of the source-cited known hashes");
    }

    /* Real-asset index helper round-trip. */
    memset(&index, 0, sizeof(index));
    rc = csb_v1_csbgraphics_dat_real_index(&cache, &index);
    CHECK(rc == CSB_V1_CSBGRAPHICS_DAT_REAL_OK,
          "csb_v1_csbgraphics_dat_real_index returns OK on a loaded cache");
    CHECK(index.count == cache.index.count,
          "real-index helper round-trips the count");
    CHECK(index.byte_order == cache.index.byte_order,
          "real-index helper round-trips the byte order");
    CHECK(index.payload_offset == cache.index.payload_offset,
          "real-index helper round-trips the payload offset");

    /* Determinism: a second scan reuses the same path/MD5. */
    {
        CSB_V1_CSBGraphicsDatRealCache cache2;
        CSB_V1_CSBGraphicsIndex index2;
        memset(&cache2, 0, sizeof(cache2));
        rc = csb_v1_csbgraphics_dat_real_scan_and_load(
            dir, NULL, 6, &cache2);
        CHECK(rc == CSB_V1_CSBGRAPHICS_DAT_REAL_OK,
              "second scan_and_load returns OK");
        CHECK(strcmp(cache2.resolved_path, cache.resolved_path) == 0,
              "second scan reuses the same resolved path");
        CHECK(strcmp(cache2.matched_md5, cache.matched_md5) == 0,
              "second scan reuses the same matched MD5");
        CHECK(cache2.file_size == cache.file_size,
              "second scan reports the same file size");
        memset(&index2, 0, sizeof(index2));
        rc = csb_v1_csbgraphics_dat_real_index(&cache2, &index2);
        CHECK(rc == CSB_V1_CSBGRAPHICS_DAT_REAL_OK,
              "second scan index helper returns OK");
        CHECK(index2.count == cache.index.count &&
                  index2.total_compressed == cache.index.total_compressed &&
                  index2.total_decompressed ==
                      cache.index.total_decompressed &&
                  index2.payload_offset == cache.index.payload_offset &&
                  index2.max_compressed == cache.index.max_compressed &&
                  index2.max_decompressed == cache.index.max_decompressed,
              "second scan inventory is byte-identical to the first");
        csb_v1_csbgraphics_dat_real_cache_free(&cache2);
    }

    csb_v1_csbgraphics_dat_real_cache_free(&cache);

    printf("\nResult: %d/%d checks passed\n", g_checks - g_failures,
           g_checks);
    return g_failures == 0 ? 0 : 1;
}
