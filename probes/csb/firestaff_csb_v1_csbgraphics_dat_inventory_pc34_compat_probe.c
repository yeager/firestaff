/*
 * firestaff_csb_v1_csbgraphics_dat_inventory_pc34_compat_probe.c
 *
 * Real-asset CSBWin "CSBgraphics.dat" custom-graphics shape +
 * inventory probe. This is the companion probe to
 * `firestaff_csb_v1_csbgraphics_dat_real_scan_probe`: the real-
 * scan probe verifies that the file is discovered, classified,
 * and cached; the inventory probe re-uses the same cache to
 * exercise the on-disk shape classifier and the inventory
 * walker against real bytes.
 *
 * Source-lock boundary (see header for full references):
 *   - CSBWin/Graphics.cpp:1838 OpenCSBgraphicsFile
 *   - CSBWin/Graphics.cpp:1918 ReadGraphicsIndex
 *   - CSBWin/Graphics.cpp:1643 LocateNthGraphic
 *   - CSBWin/data.cpp:1936 Signature (per-file MD5 split)
 *   - ReDMCSB F0200_DMMISC_ReadCompressedGraphic
 *
 * What this probe proves on a user-staged CSBgraphics.dat:
 *   - The on-disk shape classifier reports
 *     CSB_V1_CSBGRAPHICS_SHAPE_CSBGRAPHICS for the file the real-
 *     scan module has loaded.
 *   - The inventory walker reports consistent count / sparse /
 *     dense / zero-length / identical totals whose sum equals
 *     the parsed count.
 *   - The inventory's end_aligned flag, payload_offset,
 *     payload_used, payload_avail, and payload_tail_bytes all
 *     match the real-index view (the inventory walk is
 *     deterministic — running it twice produces the same
 *     numbers).
 *   - The shape classifier also rejects the off-target buffer
 *     shapes a launcher might pick up by mistake (DM1 RDMCSB15,
 *     CSB v2.0/v2.1 save, CSBWin 512-byte XOR pad, truncated
 *     too-small bytes) — even when those bytes happen to live
 *     inside the same data directory as a real CSBgraphics.dat.
 *
 * Skip-safe by design: when the known MD5 list is empty (the
 * default — CSBgraphics.dat is a CSBGraffer/CSBWin Viewport
 * Compiler product, not an original CSB asset) the probe exits
 * 0 with a SKIP message so hosts without a user-staged
 * CSBgraphics.dat do not fail.
 */

#include "csb_v1_csbgraphics_dat_classify.h"
#include "csb_v1_csbgraphics_dat_inventory_pc34_compat.h"
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

/* Cross-check the on-disk shape classifier against a few
 * trivially-constructed synthetic byte buffers so the real-asset
 * path also surfaces a deterministic contract pass. */
static int run_off_target_shape_contract(void)
{
    uint8_t buf16[16];
    int local_pass = 0;
    int local_fail = 0;

    /* RDMCSB15 — DM1 raw save. */
    {
        memset(buf16, 0, sizeof(buf16));
        memcpy(buf16, "RDMCSB15", 8);
        CHECK(csb_v1_csbgraphics_dat_shape_classify(buf16, sizeof(buf16))
                  == CSB_V1_CSBGRAPHICS_SHAPE_DM1_RAW_RDMCSB15,
              "shape classifier recognises RDMCSB15 as DM1 raw save");
    }
    /* CSBGAME\0 + v2.0 version word — CSB save. */
    {
        memset(buf16, 0, sizeof(buf16));
        memcpy(buf16, "CSBGAME\0", 8);
        buf16[8] = 0x00u; buf16[9] = 0x02u;
        CHECK(csb_v1_csbgraphics_dat_shape_classify(buf16, sizeof(buf16))
                  == CSB_V1_CSBGRAPHICS_SHAPE_CSB_SAVE_CSBGAME,
              "shape classifier recognises CSBGAME\\0 as CSB save");
    }
    /* CSB\1 — CSBWin 512-byte XOR pad. */
    {
        memset(buf16, 0, sizeof(buf16));
        buf16[0] = 'C'; buf16[1] = 'S'; buf16[2] = 'B'; buf16[3] = 0x01u;
        CHECK(csb_v1_csbgraphics_dat_shape_classify(buf16, sizeof(buf16))
                  == CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_CSB1,
              "shape classifier recognises CSB\\1 as CSBWin 512-byte");
    }
    /* DM\0\1 — CSBWin 512-byte XOR pad. */
    {
        memset(buf16, 0, sizeof(buf16));
        buf16[0] = 'D'; buf16[1] = 'M';
        buf16[2] = 0x00u; buf16[3] = 0x01u;
        CHECK(csb_v1_csbgraphics_dat_shape_classify(buf16, sizeof(buf16))
                  == CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_DM01,
              "shape classifier recognises DM\\0\\1 as CSBWin 512-byte");
    }
    /* CEDT — CSBWin 512-byte XOR pad. */
    {
        memset(buf16, 0, sizeof(buf16));
        buf16[0] = 'C'; buf16[1] = 'E'; buf16[2] = 'D'; buf16[3] = 'T';
        CHECK(csb_v1_csbgraphics_dat_shape_classify(buf16, sizeof(buf16))
                  == CSB_V1_CSBGRAPHICS_SHAPE_CSBWIN_512_CEDT,
              "shape classifier recognises CEDT as CSBWin 512-byte");
    }
    /* Truncated — TOO_SMALL. */
    {
        uint8_t buf2[2] = { 0x80u, 0x01u };
        CHECK(csb_v1_csbgraphics_dat_shape_classify(buf2, 1u)
                  == CSB_V1_CSBGRAPHICS_SHAPE_TOO_SMALL,
              "shape classifier reports TOO_SMALL for size=1");
    }
    (void)local_pass;
    (void)local_fail;
    return 0;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir;
    size_t known_count = 0u;
    const CSB_V1_CSBGraphicsDatRealKnownHash *known;
    size_t i;
    CSB_V1_CSBGraphicsDatRealCache cache;
    int rc;

    printf("=== CSB V1 CSBgraphics.dat shape + inventory probe ===\n\n");

    /* ── Always-on synthetic contract pass ── */
    run_off_target_shape_contract();

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

    if (known_count == 0u) {
        printf("SKIP: no source-cited CSBgraphics.dat MD5 is registered. "
               "Stage a real CSBWin-produced CSBgraphics.dat under "
               "~/.firestaff/data/csbwin-custom/<label>/ and extend "
               "g_known_hashes[] in src/csb/csb_v1_csbgraphics_dat_real_scan.c "
               "to enable this gate.\n");
        csb_v1_csbgraphics_dat_real_cache_free(&cache);
        printf("\nResult: %d/%d checks passed\n",
               g_checks - g_failures, g_checks);
        return 0;
    }
    if (rc == CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND) {
        printf("SKIP: known-hash list has %zu entries but no "
               "CSBgraphics.dat matching any registered hash was found "
               "under data_dir. Set FIRESTAFF_CSBWIN_CSBGRAPHICS_DATA to "
               "a directory containing a verified CSBgraphics.dat to "
               "enable this gate.\n", known_count);
        csb_v1_csbgraphics_dat_real_cache_free(&cache);
        printf("\nResult: %d/%d checks passed\n",
               g_checks - g_failures, g_checks);
        return 0;
    }
    if (rc != CSB_V1_CSBGRAPHICS_DAT_REAL_OK) {
        printf("FAIL: scan_and_load returned %d (%s); expected OK or "
               "NOT_FOUND.\n", rc,
               csb_v1_csbgraphics_dat_real_result_name(rc));
        csb_v1_csbgraphics_dat_real_cache_free(&cache);
        printf("\nResult: %d/%d checks passed\n",
               g_checks - g_failures, g_checks);
        return 1;
    }

    printf("resolved_path=%s\n", cache.resolved_path);
    printf("matched_md5=%s\n", cache.matched_md5);
    printf("matched_label=%s\n", cache.matched_label);
    printf("file_size=%zu\n", cache.file_size);

    /* ── Shape classifier on the cached bytes ── */
    {
        CSB_V1_CSBGraphicsShape shape =
            csb_v1_csbgraphics_dat_shape_classify(
                cache.file_buffer, cache.file_size);
        printf("shape=%s\n",
               csb_v1_csbgraphics_dat_shape_name(shape));
        CHECK(shape == CSB_V1_CSBGRAPHICS_SHAPE_CSBGRAPHICS,
              "cached bytes classified as CSBGRAPHICS by the shape classifier");
    }

    /* ── Inventory walker on the cached bytes ── */
    {
        CSB_V1_CSBGraphicsIndex inv_index;
        CSB_V1_CSBGraphicsInventory inv;
        int inv_rc;
        memset(&inv_index, 0, sizeof(inv_index));
        memset(&inv, 0, sizeof(inv));
        inv_rc = csb_v1_csbgraphics_dat_inventory_from_bytes(
            cache.file_buffer, cache.file_size, &inv_index, &inv);
        CHECK(inv_rc == CSB_V1_CSBGRAPHICS_INVENTORY_OK,
              "inventory walker returns OK on the cached file");

        printf("inventory.count=%u sparse=%u dense=%u zero=%u identical=%u\n",
               (unsigned)inv.count,
               (unsigned)inv.sparse_count,
               (unsigned)inv.dense_count,
               (unsigned)inv.zero_length_count,
               (unsigned)inv.identical_count);
        printf("inventory.payload_offset=%llu payload_used=%llu "
               "payload_avail=%llu payload_tail=%llu end_aligned=%d\n",
               (unsigned long long)inv.payload_offset,
               (unsigned long long)inv.payload_used,
               (unsigned long long)inv.payload_avail,
               (unsigned long long)inv.payload_tail_bytes,
               inv.end_aligned);

        CHECK(inv.count == cache.index.count,
              "inventory.count matches the real-index count");
        CHECK(inv.payload_offset == cache.index.payload_offset,
              "inventory.payload_offset matches the real-index payload_offset");
        CHECK(inv.payload_avail == cache.index.payload_bytes_avail,
              "inventory.payload_avail matches real-index payload_bytes_avail");
        CHECK(inv.payload_used == cache.index.total_compressed,
              "inventory.payload_used matches real-index total_compressed");
        CHECK(inv.payload_tail_bytes + inv.payload_used == inv.payload_avail,
              "inventory.payload_tail + payload_used = payload_avail");
        CHECK((uint32_t)(inv.sparse_count + inv.dense_count +
                         inv.zero_length_count) == inv.count,
              "sparse + dense + zero-length counts sum to inventory.count");
        CHECK(inv.end_aligned == (inv.payload_tail_bytes == 0u ? 1 : 0),
              "inventory.end_aligned matches the tail-bytes invariant");

        /* Determinism: a second walk produces the same numbers. */
        {
            CSB_V1_CSBGraphicsInventory inv2;
            CSB_V1_CSBGraphicsIndex inv2_index;
            memset(&inv2, 0, sizeof(inv2));
            memset(&inv2_index, 0, sizeof(inv2_index));
            inv_rc = csb_v1_csbgraphics_dat_inventory_from_bytes(
                cache.file_buffer, cache.file_size, &inv2_index, &inv2);
            CHECK(inv_rc == CSB_V1_CSBGRAPHICS_INVENTORY_OK,
                  "second inventory walk returns OK");
            CHECK(inv2.count == inv.count &&
                  inv2.sparse_count == inv.sparse_count &&
                  inv2.dense_count == inv.dense_count &&
                  inv2.zero_length_count == inv.zero_length_count &&
                  inv2.identical_count == inv.identical_count &&
                  inv2.payload_offset == inv.payload_offset &&
                  inv2.payload_used == inv.payload_used &&
                  inv2.payload_avail == inv.payload_avail &&
                  inv2.payload_tail_bytes == inv.payload_tail_bytes &&
                  inv2.end_aligned == inv.end_aligned,
                  "second inventory walk produces byte-identical numbers");
        }
    }

    /* ── Inventory via parsed index (no byte buffer needed) ── */
    {
        CSB_V1_CSBGraphicsInventory inv;
        int inv_rc = csb_v1_csbgraphics_dat_inventory(
            &cache.index, &inv);
        CHECK(inv_rc == CSB_V1_CSBGRAPHICS_INVENTORY_OK,
              "inventory(index-only) returns OK on the real-index view");
        CHECK(inv.count == cache.index.count,
              "inventory(index-only).count matches the parsed count");
        CHECK(inv.payload_offset == cache.index.payload_offset,
              "inventory(index-only).payload_offset matches the parsed view");
        CHECK(inv.payload_avail == cache.index.payload_bytes_avail,
              "inventory(index-only).payload_avail matches the parsed view");
    }

    csb_v1_csbgraphics_dat_real_cache_free(&cache);

    printf("\nResult: %d/%d checks passed\n",
           g_checks - g_failures, g_checks);
    return g_failures == 0 ? 0 : 1;
}
