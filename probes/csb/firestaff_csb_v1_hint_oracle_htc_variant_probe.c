/*
 * firestaff_csb_v1_hint_oracle_htc_variant_probe.c
 *
 * Real-asset variant classification probe for the CSB Hint Oracle
 * HCSB.HTC content file.
 *
 * Source-lock boundary:
 *   - dmweb Hint Oracle Files page (the canonical "all known
 *     HCSB.HTC files" reference, including per-variant MD5s
 *     and per-variant location/hint/page counts).
 *   - ReDMCSB HINTLOAD.C:11-18 names HCSB.HTC as the
 *     canonical CSB Utility Disk Hint Oracle file.
 *   - ReDMCSB HINTHTC.C:177-358 validates the format 2 /
 *     dungeon 13 big-endian table every variant shares.
 *   - ReDMCSB HINTLZW.C:122-212 decompresses hint content.
 *
 * What this proves:
 *   - The variant catalog (5 documented dmweb variants) is
 *     well-formed and every row carries the expected
 *     MD5 / size / count / language metadata.
 *   - Real HCSB.HTC + HCSBF.HTC data staged under
 *     ~/.firestaff/data classifies correctly through both
 *     the MD5 path (preferred) and the parsed-content
 *     fallback path.
 *   - Drift detail between the catalog contract and the
 *     observed parse reports matches=1 for both locally
 *     staged variants.
 *   - Variant names are stable strings so a future M12
 *     popup can rely on them as keys.
 *
 * Skip-safe by design: when no known HCSB.HTC is present,
 * the probe exits 0 with a SKIP message after the catalog
 * identity checks have run, so the catalog row stays
 * machine-checkable even on hosts without CSB Utility
 * Disk assets.
 */

#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_htc_real_scan.h"
#include "csb_hint_oracle_htc_variant.h"

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

/* ── Catalog identity (always run, even on hosts without assets) ── */

static int probe_catalog_identity(void)
{
    size_t count = 0u;
    const CSB_HintOracleHTC_VariantCatalog *cat;
    size_t i;
    int saw_r1 = 0, saw_r2 = 0, saw_r3 = 0, saw_fr = 0, saw_ge = 0;

    cat = csb_hint_oracle_htc_variant_catalog(&count);
    printf("=== catalog_identity ===\n");
    printf("  catalog_count=%zu\n", count);
    CHECK(count == 5u,
          "catalog carries exactly the 5 documented dmweb variants");
    for (i = 0u; i < count; ++i) {
        printf("  [%zu] variant=%s release=%s lang=%s md5=%s "
               "size=%zu loc=%zu hint=%zu page=%zu high_glyphs=%d\n",
               i,
               csb_hint_oracle_htc_variant_name(cat[i].variant),
               cat[i].release_name ? cat[i].release_name : "?",
               cat[i].language ? cat[i].language : "?",
               cat[i].md5 ? cat[i].md5 : "(null)",
               cat[i].expected_size,
               cat[i].expected_location_count,
               cat[i].expected_hint_count,
               cat[i].expected_page_count,
               cat[i].uses_high_glyphs);
        CHECK(cat[i].label && cat[i].label[0] != '\0',
              "catalog row has a non-empty label");
        CHECK(cat[i].md5 && strlen(cat[i].md5) == 32u,
              "catalog row has a 32-char hex MD5");
        switch (cat[i].variant) {
        case CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN: saw_r1 = 1; break;
        case CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_2_EN: saw_r2 = 1; break;
        case CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN: saw_r3 = 1; break;
        case CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR:     saw_fr = 1; break;
        case CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE:     saw_ge = 1; break;
        default: break;
        }
    }
    CHECK(saw_r1 && saw_r2 && saw_r3 && saw_fr && saw_ge,
          "all five documented variants present in catalog");
    return g_failures == 0;
}

/* ── MD5 classification (always run) ───────────────────────────── */

static int probe_md5_classify(void)
{
    printf("=== md5_classify ===\n");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "8ce69b54cf255a15e98e909bb45b9742") ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
          "R1 EN MD5 classifies as RELEASE_1_EN");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "803ede61136ccfc2bff8e266d8dc3935") ==
              CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR,
          "Amiga FR MD5 classifies as AMIGA_FR");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "00000000000000000000000000000000") ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "off-list MD5 returns UNKNOWN");
    return g_failures == 0;
}

/* ── Real-asset scan + variant classification ───────────────────── */

static int probe_real_asset_variant(const char *dir)
{
    CSB_HintOracleHTC_RealCache cache;
    CSB_HintOracleHTC_Variant variant;
    CSB_HintOracleHTC_VariantDrift drift;
    int rc;
    int saw_real_load = 0;

    if (!dir) {
        printf("SKIP: no data_dir available\n");
        return 1;
    }

    printf("=== real_asset_variant ===\n");
    printf("data_dir=%s\n", dir);

    csb_hint_oracle_htc_real_cache_init(&cache);
    rc = csb_hint_oracle_htc_real_scan_and_load(dir, NULL, 6, &cache);
    printf("scan_and_load rc=%d (%s)\n", rc,
           csb_hint_oracle_htc_real_result_name(rc));
    if (rc == CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND) {
        printf("SKIP: no known HCSB.HTC found under data_dir; "
               "set FIRESTAFF_CSB_HTC_DATA to a directory containing "
               "a verified HCSB.HTC to enable this gate.\n");
        csb_hint_oracle_htc_real_cache_free(&cache);
        return 1;
    }
    if (rc != CSB_HINT_ORACLE_HTC_REAL_OK) {
        printf("FAIL: scan_and_load returned %d (%s)\n", rc,
               csb_hint_oracle_htc_real_result_name(rc));
        csb_hint_oracle_htc_real_cache_free(&cache);
        return 0;
    }
    saw_real_load = 1;

    printf("matched_md5=%s\n", cache.matched_md5);
    printf("matched_label=%s\n", cache.matched_label);
    printf("file_size=%zu (observed) "
           "location_count=%zu hint_count=%zu page_count=%zu\n",
           cache.file_size,
           cache.htc.location_count,
           cache.htc.hint_count,
           cache.htc.page_count);

    variant = csb_hint_oracle_htc_variant_from_cache(&cache);
    printf("variant=%s (%s)\n",
           csb_hint_oracle_htc_variant_name(variant),
           csb_hint_oracle_htc_variant_release_name(variant));
    CHECK(variant != CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "real-asset cache classifies to a non-UNKNOWN variant");

    /* Expected mapping for the two locally-staged variants. */
    if (strcmp(cache.matched_md5,
               "8ce69b54cf255a15e98e909bb45b9742") == 0) {
        CHECK(variant == CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
              "locally-staged Atari ST 2.x MD5 → RELEASE_1_EN");
        CHECK(strcmp(csb_hint_oracle_htc_variant_release_name(variant),
                     "R1") == 0,
              "Atari ST 2.x variant carries release_name='R1'");
    } else if (strcmp(cache.matched_md5,
                      "803ede61136ccfc2bff8e266d8dc3935") == 0) {
        CHECK(variant == CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR,
              "locally-staged Amiga FR MD5 → AMIGA_FR");
        CHECK(strcmp(csb_hint_oracle_htc_variant_language(variant),
                     "FR") == 0,
              "Amiga FR variant carries language='FR'");
    } else {
        printf("  NOTE: matched MD5 %s not in the variant catalog; "
               "falling back to parsed-content classification.\n",
               cache.matched_md5);
    }

    /* Drift detail: catalog vs observed. */
    rc = csb_hint_oracle_htc_variant_drift(
        variant, &cache.htc, cache.file_size, &drift);
    CHECK(rc == 1, "drift helper returns 1 for the matched variant");
    if (rc == 1) {
        printf("drift.observed: size=%zu loc=%zu hint=%zu page=%zu\n",
               drift.observed_size,
               drift.observed_location_count,
               drift.observed_hint_count,
               drift.observed_page_count);
        printf("drift.expected: size=%zu loc=%zu hint=%zu page=%zu\n",
               drift.expected_size,
               drift.expected_location_count,
               drift.expected_hint_count,
               drift.expected_page_count);
        CHECK(drift.matches == 1,
              "real-asset drift matches the catalog contract exactly");
    }

    /* Diagnostic-name stability. */
    CHECK(csb_hint_oracle_htc_variant_name(variant) != NULL &&
          csb_hint_oracle_htc_variant_name(variant)[0] != '\0',
          "matched variant has a non-empty diagnostic name");
    CHECK(csb_hint_oracle_htc_variant_language(variant) != NULL &&
          csb_hint_oracle_htc_variant_language(variant)[0] != '\0',
          "matched variant has a non-empty language tag");

    csb_hint_oracle_htc_real_cache_free(&cache);
    (void)saw_real_load;
    return g_failures == 0;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir;
    int catalog_ok;
    int md5_ok;
    int real_ok;

    printf("=== CSB V1 Hint Oracle HCSB.HTC variant probe ===\n\n");

    catalog_ok = probe_catalog_identity();
    md5_ok = probe_md5_classify();

    dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
    real_ok = probe_real_asset_variant(dir);

    printf("\nchecks=%d failures=%d catalog_ok=%d md5_ok=%d real_ok=%d\n",
           g_checks, g_failures, catalog_ok, md5_ok, real_ok);
    return g_failures == 0 ? 0 : 1;
}
