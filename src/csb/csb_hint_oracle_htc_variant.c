/*
 * csb_hint_oracle_htc_variant.c
 *
 * Variant catalog + classification for the CSB Utility Disk
 * HCSB.HTC Hint Oracle content file.
 *
 * See include/csb_hint_oracle_htc_variant.h for scope and
 * source references.
 *
 * The catalog below is intentionally narrow: only the five
 * variants the dmweb Hint Oracle Files page documents with a
 * known MD5 are listed. The two variants we locally stage
 * (Atari ST 2.x PP 2009-02-22 hard-disk and Amiga 3.3 FR
 * Meynaf hard-disk utility) carry an MD5; the remaining three
 * have a documented MD5 + size + counts but no locally
 * staged copy yet. As additional variants are classified and
 * locally verified, the catalog grows; the matching surface
 * stays MD5-first with parsed-content fallback.
 */

#include "csb_hint_oracle_htc_variant.h"

#include <stddef.h>
#include <string.h>

/* ── Catalog ────────────────────────────────────────────────────── */

/* All numeric fields below are sourced verbatim from the dmweb
 * Hint Oracle Files page "Hints File" table; do not adjust
 * without updating the gap-list row + VERIFIED_HASHES.md
 * reference in the same commit.
 *
 *   - Atari ST 2.x PP 2009-02-22 hard-disk variant (R1 EN):
 *     locally staged at
 *     ~/.firestaff/data/csb-atari-st-2x/HCSB.HTC and at
 *     ~/.firestaff/data/csb-extras/legacy-atari-st/HardDisk/
 *       2009-02-22 PP/HCSB.HTC
 *     (both sha256 1b2fbff81a11928afd153f46c117355cce1f9a93
 *      482d14d58e35a115d9cde38).
 *     4,663 location records, 210 hints, 492 pages, 66,172
 *     bytes. Atari ST Hint Oracle masks 0x80..0xFF as spaces.
 *
 *   - Amiga 3.3 FR Meynaf hard-disk utility variant
 *     (HCSBF.HTC): locally staged at
 *     ~/.firestaff/data/csb-extras/legacy-amiga-dms/HardDisk/
 *       Chaos Strikes Back for Amiga v3.3 (French) Hacked by
 *       Meynaf/FTL_CSB_Utility/HCSBF.HTC
 *     (sha256 79fd268631d3518462058c9e855b7b8a89c0ef8a0938
 *      adf9ed8ea17a55719ea7).
 *     5,036 location records, 219 hints, 512 pages, 75,424
 *     bytes. Uses 8-bit accented glyphs.
 *
 *   - Amiga 3.x EN Release 2 (R2 EN): documented but not
 *     locally staged. dmweb describes this as a "Test file
 *     for translations mistakenly released".
 *     5,036 location records, 219 hints, 512 pages, 68,912
 *     bytes.
 *
 *   - Amiga 3.x EN Release 3 (R3 EN): documented but not
 *     locally staged. The "correct" replacement FTL released
 *     after R2; renders 0x80..0xFF as multilingual glyphs.
 *     5,044 location records, 219 hints, 513 pages, 69,963
 *     bytes.
 *
 *   - Amiga 3.x German (HCSBG.HTC): documented but not locally
 *     staged. 5,036 location records, 219 hints, 512 pages,
 *     75,504 bytes.
 *
 * Two variants share R2's counts (R2 EN, FR, GE) — the catalog
 * disambiguates by file size when classifying by content. */
static const CSB_HintOracleHTC_VariantCatalog g_catalog[] = {
    {
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN,
        "csb-atari-st-2x/2009-02-22-PP-hard-disk",
        "R1", "EN",
        "8ce69b54cf255a15e98e909bb45b9742",
        66172u, 4663u, 210u, 492u,
        0
    },
    {
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_2_EN,
        "csb-amiga-3.x-en-R2-util-disk",
        "R2", "EN",
        "334fc18cb98d1280a4c55a16566d5ef9",
        68912u, 5036u, 219u, 512u,
        0
    },
    {
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN,
        "csb-amiga-3.x-en-R3-util-disk",
        "R3", "EN",
        "c06862298f193b1fe479eaeff6acd57e",
        69963u, 5044u, 219u, 513u,
        1
    },
    {
        CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR,
        "csb-extras/legacy-amiga-dms/Meynaf-FR-v3.3-hard-disk",
        "-", "FR",
        "803ede61136ccfc2bff8e266d8dc3935",
        75424u, 5036u, 219u, 512u,
        1
    },
    {
        CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE,
        "csb-amiga-3.x-ge-util-disk",
        "-", "GE",
        "5a7ab2c8387215c7b2abe772e2ddc689",
        75504u, 5036u, 219u, 512u,
        1
    }
};

const CSB_HintOracleHTC_VariantCatalog *
csb_hint_oracle_htc_variant_catalog(size_t *out_count)
{
    if (out_count) {
        *out_count = sizeof(g_catalog) / sizeof(g_catalog[0]);
    }
    return g_catalog;
}

/* ── Helpers ────────────────────────────────────────────────────── */

/* Compare a 32-char hex MD5 to a catalog row's MD5, ignoring
 * ASCII case. Returns 1 on match, 0 otherwise. */
static int md5_eq_ci(const char *a, const char *b)
{
    size_t i;
    if (!a || !b) {
        return 0;
    }
    for (i = 0u; i < 32u; ++i) {
        char ca = a[i];
        char cb = b[i];
        if (ca == '\0' || cb == '\0') {
            return 0;
        }
        if (ca >= 'A' && ca <= 'Z') {
            ca = (char)(ca - 'A' + 'a');
        }
        if (cb >= 'A' && cb <= 'Z') {
            cb = (char)(cb - 'A' + 'a');
        }
        if (ca != cb) {
            return 0;
        }
    }
    return 1;
}

/* ── Classify by MD5 ────────────────────────────────────────────── */

CSB_HintOracleHTC_Variant
csb_hint_oracle_htc_variant_from_md5(const char *hex_md5)
{
    size_t count = 0u;
    size_t i;

    if (!hex_md5) {
        return CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN;
    }
    /* Cheap length check: every catalog MD5 is exactly 32 hex
     * chars. Bail early on too-short / too-long input. */
    if (strlen(hex_md5) < 32u) {
        return CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN;
    }
    for (i = 0u; i < sizeof(g_catalog) / sizeof(g_catalog[0]); ++i) {
        if (md5_eq_ci(hex_md5, g_catalog[i].md5)) {
            return g_catalog[i].variant;
        }
    }
    (void)count;
    return CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN;
}

/* ── Classify by parsed content ─────────────────────────────────── */

CSB_HintOracleHTC_Variant
csb_hint_oracle_htc_variant_from_parsed(const CSB_HintOracleHTC *htc,
                                       size_t observed_file_size)
{
    size_t i;
    size_t n;

    if (!htc) {
        return CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN;
    }
    /* The format-2 / dungeon-13 header is already enforced by
     * csb_hint_oracle_htc_parse; double-check defensively so a
     * malformed input never falls through to a content match. */
    if (htc->format_word != CSB_HINT_ORACLE_HTC_FORMAT_WORD ||
        htc->dungeon_id  != CSB_HINT_ORACLE_HTC_DUNGEON_ID) {
        return CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN;
    }
    n = sizeof(g_catalog) / sizeof(g_catalog[0]);
    /* First pass: enforce counts AND size when the caller
     * supplies a non-zero observed_file_size. A size-gated
     * match is the preferred disambiguation path because
     * the size gate breaks the R2 EN / FR / GE count tie. */
    if (observed_file_size != 0u) {
        for (i = 0u; i < n; ++i) {
            if (htc->location_count == g_catalog[i].expected_location_count &&
                htc->hint_count     == g_catalog[i].expected_hint_count &&
                htc->page_count     == g_catalog[i].expected_page_count &&
                observed_file_size  == g_catalog[i].expected_size) {
                return g_catalog[i].variant;
            }
        }
    }
    /* Second pass: when observed_file_size is 0 (unknown), fall
     * back to counts only. R1 EN (4663/210/492) and R3 EN
     * (5044/219/513) are unique by counts alone; R2 EN, FR,
     * and GE all share (5036/219/512) and cannot be told apart
     * without a size, so the catalog returns UNKNOWN for the
     * tied group. */
    {
        int match_count = 0;
        size_t first_idx = 0u;
        for (i = 0u; i < n; ++i) {
            if (htc->location_count == g_catalog[i].expected_location_count &&
                htc->hint_count     == g_catalog[i].expected_hint_count &&
                htc->page_count     == g_catalog[i].expected_page_count) {
                if (match_count == 0) {
                    first_idx = i;
                }
                ++match_count;
            }
        }
        if (match_count == 1) {
            return g_catalog[first_idx].variant;
        }
    }
    return CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN;
}

/* ── Classify a loaded cache (real-asset surface) ──────────────── */

CSB_HintOracleHTC_Variant
csb_hint_oracle_htc_variant_from_cache(
    const CSB_HintOracleHTC_RealCache *cache)
{
    if (!cache || !cache->loaded) {
        return CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN;
    }
    /* MD5 first (preferred). The real-scan populates
     * cache->matched_md5 even when the catalog has no entry
     * for it, so MD5 failure cleanly falls through. */
    if (cache->matched_md5[0] != '\0') {
        CSB_HintOracleHTC_Variant v =
            csb_hint_oracle_htc_variant_from_md5(cache->matched_md5);
        if (v != CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN) {
            return v;
        }
    }
    /* Fallback: classify by parsed content + observed file
     * size. This lets an in-progress or off-list staging of a
     * variant still get a variant tag as long as the size +
     * counts match a documented catalog row. */
    return csb_hint_oracle_htc_variant_from_parsed(
        &cache->htc, cache->file_size);
}

/* ── Drift detail ───────────────────────────────────────────────── */

int csb_hint_oracle_htc_variant_drift(
    CSB_HintOracleHTC_Variant variant,
    const CSB_HintOracleHTC *htc,
    size_t observed_file_size,
    CSB_HintOracleHTC_VariantDrift *out_drift)
{
    size_t i;
    size_t n;
    const CSB_HintOracleHTC_VariantCatalog *row = NULL;

    if (!out_drift) {
        return 0;
    }
    memset(out_drift, 0, sizeof(*out_drift));
    out_drift->variant = CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN;
    if (variant == CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN || !htc) {
        return 0;
    }
    n = sizeof(g_catalog) / sizeof(g_catalog[0]);
    for (i = 0u; i < n; ++i) {
        if (g_catalog[i].variant == variant) {
            row = &g_catalog[i];
            break;
        }
    }
    if (!row) {
        return 0;
    }
    out_drift->variant = row->variant;
    out_drift->expected_size = row->expected_size;
    out_drift->expected_location_count = row->expected_location_count;
    out_drift->expected_hint_count = row->expected_hint_count;
    out_drift->expected_page_count = row->expected_page_count;
    out_drift->observed_size = observed_file_size;
    out_drift->observed_location_count = htc->location_count;
    out_drift->observed_hint_count = htc->hint_count;
    out_drift->observed_page_count = htc->page_count;
    out_drift->matches =
        (out_drift->observed_size == out_drift->expected_size) &&
        (out_drift->observed_location_count ==
         out_drift->expected_location_count) &&
        (out_drift->observed_hint_count ==
         out_drift->expected_hint_count) &&
        (out_drift->observed_page_count ==
         out_drift->expected_page_count);
    return 1;
}

/* ── Diagnostics ────────────────────────────────────────────────── */

const char *csb_hint_oracle_htc_variant_name(
    CSB_HintOracleHTC_Variant variant)
{
    switch (variant) {
    case CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN: return "release-1-en";
    case CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_2_EN: return "release-2-en";
    case CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN: return "release-3-en";
    case CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR:     return "amiga-fr";
    case CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE:     return "amiga-ge";
    case CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN:
    default:                                        return "unknown";
    }
}

const char *csb_hint_oracle_htc_variant_release_name(
    CSB_HintOracleHTC_Variant variant)
{
    size_t i;
    size_t n;
    n = sizeof(g_catalog) / sizeof(g_catalog[0]);
    for (i = 0u; i < n; ++i) {
        if (g_catalog[i].variant == variant) {
            return g_catalog[i].release_name;
        }
    }
    return "?";
}

const char *csb_hint_oracle_htc_variant_language(
    CSB_HintOracleHTC_Variant variant)
{
    size_t i;
    size_t n;
    n = sizeof(g_catalog) / sizeof(g_catalog[0]);
    for (i = 0u; i < n; ++i) {
        if (g_catalog[i].variant == variant) {
            return g_catalog[i].language;
        }
    }
    return "?";
}
