/*
 * test_csb_hint_oracle_htc_variant.c
 *
 * Data-free contract tests for the CSB Hint Oracle HCSB.HTC
 * variant catalog + classifier.
 *
 * Scope:
 *   - Catalog identity: the documented dmweb variant set is
 *     present with the expected MD5s, sizes, and
 *     location/hint/page counts. Each catalog row maps 1:1
 *     to its enum tag and a stable, non-empty label +
 *     release + language.
 *   - MD5 classification: every catalog MD5 round-trips to
 *     the right enum tag; case-insensitive match works;
 *     short / NULL / off-list MD5s return UNKNOWN.
 *   - Content classification: a parsed HCSB.HTC view with
 *     the R1 EN counts (4,663 / 210 / 492) and the R1 EN
 *     size (66,172) classifies as RELEASE_1_EN. Counts alone
 *     resolve the uniquely-counted rows (R1 EN, R3 EN); the
 *     size gate disambiguates the three-way R2 EN / FR / GE
 *     tie. Counts that match no catalog row return UNKNOWN;
 *     a non-format-2 / non-dungeon-13 view also returns
 *     UNKNOWN even if the counts coincidentally match.
 *   - Cache classification: a fake "loaded" cache with the
 *     R1 EN matched MD5 + R1 EN counts classifies as
 *     RELEASE_1_EN; an empty cache returns UNKNOWN.
 *   - Drift detail: a parsed view that exactly matches the
 *     catalog contract reports matches=1; a parsed view
 *     with a single-byte drift in any of size / location /
 *     hint / page reports matches=0 with the expected vs.
 *     observed fields exposed.
 *   - Diagnostic names: variant_name / release_name /
 *     language round-trip and stay stable so callers can
 *     rely on them as string keys.
 *   - Scanner hash slots: every catalog MD5 is also present
 *     in csb_hint_oracle_htc_real_scan so operator-staged
 *     English R1/R2/R3, French R1, and German R1/R2 HTC files
 *     are accepted by the skip-safe scanner.
 *
 * Non-claims:
 *   - No real Utility Disk asset is loaded (synthetic
 *     fixture only).
 *   - The variant catalog is intentionally narrow: only the
 *     five variants the dmweb Hint Oracle Files page
 *     documents with a known MD5 are listed. Variants with
 *     no locally-staged copy carry an MD5 anyway so the
 *     MD5 classification works as soon as an operator
 *     stages the file.
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

#define CHECK_STR_EQ(actual, expected, msg) do {                           \
    ++g_checks;                                                            \
    if ((actual) && strcmp((actual), (expected)) == 0) {                    \
        printf("  PASS: %s\n", msg);                                       \
    } else {                                                               \
        ++g_failures;                                                      \
        printf("  FAIL: %s (got='%s' expected='%s')\n",                   \
               msg,                                                        \
               (actual) ? (actual) : "(null)",                             \
               (expected));                                                \
    }                                                                      \
} while (0)

/* Populate a CSB_HintOracleHTC view from explicit counts +
 * format word + dungeon id. The pointer fields point into
 * the supplied scratch buffer so the view is "loaded" enough
 * for the variant classifier to consult. The classifier only
 * reads format_word, dungeon_id, location_count, hint_count,
 * and page_count — the pointer fields can be NULL for these
 * tests because none of the variant helpers dereferences
 * them. */
static void make_view(CSB_HintOracleHTC *htc,
                      uint16_t format_word,
                      uint16_t dungeon_id,
                      size_t location_count,
                      size_t hint_count,
                      size_t page_count)
{
    memset(htc, 0, sizeof(*htc));
    htc->format_word = format_word;
    htc->dungeon_id = dungeon_id;
    htc->location_count = location_count;
    htc->hint_count = hint_count;
    htc->page_count = page_count;
}

/* Populate a fake loaded cache with the given matched MD5,
 * label, file size, and counts. The parser view inside the
 * cache is initialized with the format-2 / dungeon-13 header
 * the classifier expects; the data/locations/hints/contents
 * pointers stay NULL because the variant helper never
 * dereferences them. */
static void make_cache(CSB_HintOracleHTC_RealCache *cache,
                       const char *md5,
                       const char *label,
                       size_t file_size,
                       size_t location_count,
                       size_t hint_count,
                       size_t page_count)
{
    memset(cache, 0, sizeof(*cache));
    if (md5) {
        size_t i;
        for (i = 0u; i < 32u && md5[i] != '\0'; ++i) {
            cache->matched_md5[i] = md5[i];
        }
        cache->matched_md5[32] = '\0';
    }
    if (label) {
        snprintf(cache->matched_label, sizeof(cache->matched_label),
                 "%s", label);
    }
    cache->file_size = file_size;
    cache->loaded = 1;
    cache->htc.format_word = CSB_HINT_ORACLE_HTC_FORMAT_WORD;
    cache->htc.dungeon_id = CSB_HINT_ORACLE_HTC_DUNGEON_ID;
    cache->htc.location_count = location_count;
    cache->htc.hint_count = hint_count;
    cache->htc.page_count = page_count;
}

/* ── Catalog identity ───────────────────────────────────────────── */

static int test_catalog_identity(void)
{
    size_t count = 0u;
    const CSB_HintOracleHTC_VariantCatalog *cat;
    int saw_r1 = 0, saw_r2 = 0, saw_r3 = 0, saw_fr = 0, saw_ge = 0;
    size_t i;

    cat = csb_hint_oracle_htc_variant_catalog(&count);
    printf("=== catalog_identity ===\n");
    printf("  catalog_count=%zu\n", count);
    CHECK(count == 5u,
          "catalog carries exactly the 5 documented dmweb variants");

    for (i = 0u; i < count; ++i) {
        printf("  [%zu] variant=%d label='%s' release='%s' lang='%s' "
               "md5=%s size=%zu loc=%zu hint=%zu page=%zu high_glyphs=%d\n",
               i, (int)cat[i].variant,
               cat[i].label ? cat[i].label : "(null)",
               cat[i].release_name ? cat[i].release_name : "(null)",
               cat[i].language ? cat[i].language : "(null)",
               cat[i].md5 ? cat[i].md5 : "(null)",
               cat[i].expected_size,
               cat[i].expected_location_count,
               cat[i].expected_hint_count,
               cat[i].expected_page_count,
               cat[i].uses_high_glyphs);
        CHECK(cat[i].label && cat[i].label[0] != '\0',
              "every catalog row has a non-empty label");
        CHECK(cat[i].md5 && strlen(cat[i].md5) == 32u,
              "every catalog row has a 32-char hex MD5");
        CHECK(cat[i].expected_size > 0u,
              "every catalog row has a positive expected size");
        CHECK(cat[i].expected_location_count > 0u &&
              cat[i].expected_hint_count > 0u &&
              cat[i].expected_page_count > 0u,
              "every catalog row has positive expected counts");
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

    /* Spot-check a few documented exact numbers. */
    CHECK(cat[0].variant == CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN &&
          cat[0].expected_size == 66172u &&
          cat[0].expected_location_count == 4663u &&
          cat[0].expected_hint_count == 210u &&
          cat[0].expected_page_count == 492u,
          "R1 EN catalog row matches the dmweb-documented numbers");
    CHECK(cat[3].variant == CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR &&
          cat[3].expected_size == 75424u &&
          cat[3].expected_location_count == 5036u &&
          cat[3].expected_hint_count == 219u &&
          cat[3].expected_page_count == 512u,
          "Amiga FR catalog row matches the dmweb-documented numbers");

    /* Use-high-glyphs gating matches the dmweb note that R3
     * EN, FR, and GE render 0x80..0xFF as multilingual glyphs
     * while R1 EN (Atari ST 2.x / Amiga R1 EN) renders them
     * as spaces. */
    CHECK(cat[0].uses_high_glyphs == 0, "R1 EN masks 0x80..0xFF");
    CHECK(cat[2].uses_high_glyphs == 1, "R3 EN renders 0x80..0xFF");
    CHECK(cat[3].uses_high_glyphs == 1, "Amiga FR renders 0x80..0xFF");
    CHECK(cat[4].uses_high_glyphs == 1, "Amiga GE renders 0x80..0xFF");
    return 1;
}

/* ── MD5 classification ────────────────────────────────────────── */

static int test_md5_classify(void)
{
    printf("=== md5_classify ===\n");

    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "8ce69b54cf255a15e98e909bb45b9742") ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
          "lowercase R1 EN MD5 classifies as RELEASE_1_EN");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "8CE69B54CF255A15E98E909BB45B9742") ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
          "uppercase R1 EN MD5 classifies as RELEASE_1_EN (case-insensitive)");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "334fc18cb98d1280a4c55a16566d5ef9") ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_2_EN,
          "R2 EN MD5 classifies as RELEASE_2_EN");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "c06862298f193b1fe479eaeff6acd57e") ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN,
          "R3 EN MD5 classifies as RELEASE_3_EN");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "803ede61136ccfc2bff8e266d8dc3935") ==
              CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR,
          "Amiga FR MD5 classifies as AMIGA_FR");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "5a7ab2c8387215c7b2abe772e2ddc689") ==
              CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE,
          "Amiga GE MD5 classifies as AMIGA_GE");

    /* Negative: NULL, short, off-list. */
    CHECK(csb_hint_oracle_htc_variant_from_md5(NULL) ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "NULL MD5 returns UNKNOWN");
    CHECK(csb_hint_oracle_htc_variant_from_md5("") ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "empty MD5 returns UNKNOWN");
    CHECK(csb_hint_oracle_htc_variant_from_md5("deadbeef") ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "short MD5 returns UNKNOWN");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "00000000000000000000000000000000") ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "32-char off-list MD5 returns UNKNOWN");
    return 1;
}

/* ── Real-scan hash slot coverage ───────────────────────────────── */

static int test_real_scan_hash_slots(void)
{
    size_t cat_count = 0u;
    size_t known_count = 0u;
    const CSB_HintOracleHTC_VariantCatalog *cat;
    const CSB_HintOracleHTC_RealKnownHash *known;
    size_t i;
    size_t j;

    printf("=== real_scan_hash_slots ===\n");

    cat = csb_hint_oracle_htc_variant_catalog(&cat_count);
    known = csb_hint_oracle_htc_real_known_hashes(&known_count);
    printf("  catalog_count=%zu known_hash_slots=%zu\n",
           cat_count, known_count);
    CHECK(known_count == cat_count,
          "real scanner carries one MD5 slot per documented catalog variant");

    for (i = 0u; i < cat_count; ++i) {
        int found = 0;
        int duplicate = 0;
        size_t found_index = 0u;
        for (j = 0u; j < known_count; ++j) {
            if (known[j].md5 && cat[i].md5 &&
                strcmp(known[j].md5, cat[i].md5) == 0) {
                if (found) {
                    duplicate = 1;
                }
                found = 1;
                found_index = j;
            }
        }
        CHECK(found,
              "catalog MD5 is registered in the real-scan known-hash table");
        CHECK(!duplicate,
              "catalog MD5 appears once in the real-scan known-hash table");
        if (found) {
            printf("  slot[%zu] variant=%s label='%s' md5=%s size=%zu\n",
                   found_index,
                   csb_hint_oracle_htc_variant_name(cat[i].variant),
                   known[found_index].label ? known[found_index].label : "",
                   known[found_index].md5 ? known[found_index].md5 : "",
                   known[found_index].size_bytes);
            CHECK(known[found_index].size_bytes == cat[i].expected_size,
                  "real-scan hash slot size matches the catalog contract");
            CHECK(known[found_index].label &&
                  known[found_index].label[0] != '\0',
                  "real-scan hash slot has a non-empty label");
            CHECK(csb_hint_oracle_htc_variant_from_md5(
                      known[found_index].md5) == cat[i].variant,
                  "real-scan hash slot MD5 round-trips through variant lookup");
        }
    }

    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "334fc18cb98d1280a4c55a16566d5ef9") ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_2_EN,
          "English R2 scanner slot resolves through the catalog");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "c06862298f193b1fe479eaeff6acd57e") ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN,
          "English R3 scanner slot resolves through the catalog");
    CHECK(csb_hint_oracle_htc_variant_from_md5(
              "5a7ab2c8387215c7b2abe772e2ddc689") ==
              CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE,
          "German R1/R2 scanner slot resolves through the catalog");
    return 1;
}

/* ── Content classification ─────────────────────────────────────── */

static int test_parsed_classify(void)
{
    CSB_HintOracleHTC view;
    CSB_HintOracleHTC bad_view;

    printf("=== parsed_classify ===\n");

    /* R1 EN: counts (4663/210/492) are unique by-counts, so
     * the classifier resolves them without a size gate. */
    make_view(&view,
              CSB_HINT_ORACLE_HTC_FORMAT_WORD,
              CSB_HINT_ORACLE_HTC_DUNGEON_ID,
              4663u, 210u, 492u);
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&view, 0u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
          "R1 EN counts-only classification resolves RELEASE_1_EN");
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&view, 66172u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
          "R1 EN counts+size classification resolves RELEASE_1_EN");

    /* R3 EN: counts (5044/219/513) are unique by-counts. */
    make_view(&view,
              CSB_HINT_ORACLE_HTC_FORMAT_WORD,
              CSB_HINT_ORACLE_HTC_DUNGEON_ID,
              5044u, 219u, 513u);
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&view, 0u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN,
          "R3 EN counts-only classification resolves RELEASE_3_EN");
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&view, 69963u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN,
          "R3 EN counts+size classification resolves RELEASE_3_EN");

    /* R2 EN / FR / GE all share (5036/219/512). The size gate
     * is what disambiguates. */
    make_view(&view,
              CSB_HINT_ORACLE_HTC_FORMAT_WORD,
              CSB_HINT_ORACLE_HTC_DUNGEON_ID,
              5036u, 219u, 512u);
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&view, 0u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "tied counts without size are ambiguous → UNKNOWN");
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&view, 68912u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_2_EN,
          "R2 EN size disambiguates the tied-count group");
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&view, 75424u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR,
          "FR size disambiguates the tied-count group");
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&view, 75504u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE,
          "GE size disambiguates the tied-count group");
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&view, 99999u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "off-list size returns UNKNOWN even when counts match");

    /* Counts that match no catalog row. */
    make_view(&view,
              CSB_HINT_ORACLE_HTC_FORMAT_WORD,
              CSB_HINT_ORACLE_HTC_DUNGEON_ID,
              1u, 1u, 1u);
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&view, 1024u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "off-list counts return UNKNOWN");

    /* Wrong format / dungeon id: UNKNOWN even with catalog counts. */
    make_view(&bad_view, 99u, 99u, 4663u, 210u, 492u);
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&bad_view, 66172u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "non-format-2/dungeon-13 header returns UNKNOWN");
    make_view(&bad_view, CSB_HINT_ORACLE_HTC_FORMAT_WORD, 99u,
              4663u, 210u, 492u);
    CHECK(csb_hint_oracle_htc_variant_from_parsed(&bad_view, 66172u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "format-2 with wrong dungeon id returns UNKNOWN");

    /* NULL view: UNKNOWN. */
    CHECK(csb_hint_oracle_htc_variant_from_parsed(NULL, 66172u) ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "NULL view returns UNKNOWN");
    return 1;
}

/* ── Cache classification ──────────────────────────────────────── */

static int test_cache_classify(void)
{
    CSB_HintOracleHTC_RealCache cache;
    CSB_HintOracleHTC_RealCache empty;

    printf("=== cache_classify ===\n");

    csb_hint_oracle_htc_real_cache_init(&empty);
    CHECK(csb_hint_oracle_htc_variant_from_cache(&empty) ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "empty cache returns UNKNOWN");
    CHECK(csb_hint_oracle_htc_variant_from_cache(NULL) ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "NULL cache returns UNKNOWN");

    /* Cache with the locally-staged Atari ST 2.x MD5. */
    make_cache(&cache,
               "8ce69b54cf255a15e98e909bb45b9742",
               "csb-atari-st-2x/2009-02-22-PP-hard-disk",
               66172u, 4663u, 210u, 492u);
    CHECK(csb_hint_oracle_htc_variant_from_cache(&cache) ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
          "loaded cache with R1 EN MD5 classifies as RELEASE_1_EN");
    csb_hint_oracle_htc_real_cache_free(&cache);

    /* Cache with the locally-staged Amiga FR Meynaf MD5. */
    make_cache(&cache,
               "803ede61136ccfc2bff8e266d8dc3935",
               "csb-extras/legacy-amiga-dms/Meynaf-FR-v3.3-hard-disk",
               75424u, 5036u, 219u, 512u);
    CHECK(csb_hint_oracle_htc_variant_from_cache(&cache) ==
              CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR,
          "loaded cache with Amiga FR MD5 classifies as AMIGA_FR");
    csb_hint_oracle_htc_real_cache_free(&cache);

    /* Cache with an off-list MD5 but catalog-matching counts +
     * size: fallback classification should still resolve the
     * variant. (e.g. an in-progress R3 EN staging before the
     * known-hash list is updated.) */
    make_cache(&cache,
               "00000000000000000000000000000000",
               "synthetic-r3-en",
               69963u, 5044u, 219u, 513u);
    CHECK(csb_hint_oracle_htc_variant_from_cache(&cache) ==
              CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN,
          "off-list MD5 falls back to parsed-content classification");
    csb_hint_oracle_htc_real_cache_free(&cache);

    /* Cache with off-list MD5 + off-list counts: UNKNOWN. */
    make_cache(&cache,
               "00000000000000000000000000000000",
               "synthetic-unknown",
               12345u, 1u, 1u, 1u);
    CHECK(csb_hint_oracle_htc_variant_from_cache(&cache) ==
              CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "off-list MD5 + off-list counts return UNKNOWN");
    csb_hint_oracle_htc_real_cache_free(&cache);
    return 1;
}

/* ── Drift detail ───────────────────────────────────────────────── */

static int test_drift(void)
{
    CSB_HintOracleHTC_RealCache cache;
    CSB_HintOracleHTC_VariantDrift drift;
    int rc;

    printf("=== drift ===\n");

    /* Build a "loaded" cache that exactly matches the R1 EN
     * catalog contract. */
    make_cache(&cache,
               "8ce69b54cf255a15e98e909bb45b9742",
               "csb-atari-st-2x/2009-02-22-PP-hard-disk",
               66172u, 4663u, 210u, 492u);
    rc = csb_hint_oracle_htc_variant_drift(
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
        &cache.htc, cache.file_size, &drift);
    CHECK(rc == 1, "drift helper returns 1 on valid input");
    CHECK(drift.variant == CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
          "drift struct reports the queried variant");
    CHECK(drift.matches == 1,
          "exact-match cache reports matches=1");
    CHECK(drift.observed_size == drift.expected_size &&
          drift.observed_location_count == drift.expected_location_count &&
          drift.observed_hint_count == drift.expected_hint_count &&
          drift.observed_page_count == drift.expected_page_count,
          "observed vs expected fields agree on exact match");

    /* Drift the size: reports matches=0 with the diff visible. */
    rc = csb_hint_oracle_htc_variant_drift(
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
        &cache.htc, 99999u, &drift);
    CHECK(rc == 1 && drift.matches == 0,
          "size drift → matches=0");
    CHECK(drift.observed_size == 99999u &&
          drift.expected_size == 66172u,
          "size drift exposes both observed and expected sizes");

    /* Drift the location count. */
    cache.htc.location_count = 1000u;
    rc = csb_hint_oracle_htc_variant_drift(
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
        &cache.htc, cache.file_size, &drift);
    CHECK(rc == 1 && drift.matches == 0,
          "location-count drift → matches=0");
    CHECK(drift.observed_location_count == 1000u &&
          drift.expected_location_count == 4663u,
          "location-count drift exposes both numbers");
    cache.htc.location_count = 4663u;

    /* Drift the hint count. */
    cache.htc.hint_count = 200u;
    rc = csb_hint_oracle_htc_variant_drift(
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
        &cache.htc, cache.file_size, &drift);
    CHECK(rc == 1 && drift.matches == 0,
          "hint-count drift → matches=0");
    cache.htc.hint_count = 210u;

    /* Drift the page count. */
    cache.htc.page_count = 100u;
    rc = csb_hint_oracle_htc_variant_drift(
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
        &cache.htc, cache.file_size, &drift);
    CHECK(rc == 1 && drift.matches == 0,
          "page-count drift → matches=0");

    /* UNKNOWN variant + NULL helpers. */
    rc = csb_hint_oracle_htc_variant_drift(
        CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN, &cache.htc,
        cache.file_size, &drift);
    CHECK(rc == 0, "UNKNOWN variant → drift helper returns 0");
    rc = csb_hint_oracle_htc_variant_drift(
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, NULL,
        cache.file_size, &drift);
    CHECK(rc == 0, "NULL htc → drift helper returns 0");
    rc = csb_hint_oracle_htc_variant_drift(
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &cache.htc,
        cache.file_size, NULL);
    CHECK(rc == 0, "NULL out → drift helper returns 0");

    csb_hint_oracle_htc_real_cache_free(&cache);
    return 1;
}

/* ── Diagnostic names ───────────────────────────────────────────── */

static int test_diagnostic_names(void)
{
    printf("=== diagnostic_names ===\n");

    CHECK_STR_EQ(csb_hint_oracle_htc_variant_name(
                     CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN),
                 "release-1-en", "variant_name RELEASE_1_EN");
    CHECK_STR_EQ(csb_hint_oracle_htc_variant_name(
                     CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_2_EN),
                 "release-2-en", "variant_name RELEASE_2_EN");
    CHECK_STR_EQ(csb_hint_oracle_htc_variant_name(
                     CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN),
                 "release-3-en", "variant_name RELEASE_3_EN");
    CHECK_STR_EQ(csb_hint_oracle_htc_variant_name(
                     CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR),
                 "amiga-fr", "variant_name AMIGA_FR");
    CHECK_STR_EQ(csb_hint_oracle_htc_variant_name(
                     CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE),
                 "amiga-ge", "variant_name AMIGA_GE");
    CHECK_STR_EQ(csb_hint_oracle_htc_variant_name(
                     CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN),
                 "unknown", "variant_name UNKNOWN");

    CHECK_STR_EQ(csb_hint_oracle_htc_variant_release_name(
                     CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN),
                 "R1", "release_name R1 EN");
    CHECK_STR_EQ(csb_hint_oracle_htc_variant_release_name(
                     CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN),
                 "R3", "release_name R3 EN");
    CHECK_STR_EQ(csb_hint_oracle_htc_variant_release_name(
                     CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR),
                 "-", "release_name FR (no release number)");

    CHECK_STR_EQ(csb_hint_oracle_htc_variant_language(
                     CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN),
                 "EN", "language R1 EN");
    CHECK_STR_EQ(csb_hint_oracle_htc_variant_language(
                     CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR),
                 "FR", "language FR");
    CHECK_STR_EQ(csb_hint_oracle_htc_variant_language(
                     CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE),
                 "GE", "language GE");
    return 1;
}

/* ── main ───────────────────────────────────────────────────────── */

int main(void)
{
    printf("=== CSB Hint Oracle HTC variant catalog tests ===\n\n");

    (void)test_catalog_identity();
    (void)test_md5_classify();
    (void)test_real_scan_hash_slots();
    (void)test_parsed_classify();
    (void)test_cache_classify();
    (void)test_drift();
    (void)test_diagnostic_names();

    printf("\nchecks=%d failures=%d\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
