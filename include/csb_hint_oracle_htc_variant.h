/*
 * csb_hint_oracle_htc_variant.h
 *
 * Variant catalog + classification for the Chaos Strikes Back
 * Utility Disk HCSB.HTC Hint Oracle content file.
 *
 * Background:
 *   The CSB Hint Oracle ships in five documented Utility Disk
 *   variants (English Release 1 / Release 2 / Release 3, French,
 *   and German). Each variant keeps the same big-endian
 *   format-2 / dungeon-13 layout the parser already understands
 *   (ReDMCSB HINTHTC.C:177-358) and the same 9/10-bit LZW
 *   page-content compression (ReDMCSB HINTLZW.C:122-212). The
 *   differences between variants are:
 *
 *     - File size.
 *     - Number of location records (4,663 / 5,036 / 5,044).
 *     - Number of hint records (210 / 219 / 219).
 *     - Number of hint pages (492 / 512 / 513).
 *     - Hint name language (English / French / German).
 *     - Hint content language and character set (the Amiga
 *       Release 3 EN, Amiga FR, and Amiga GE variants use
 *       8-bit characters above 0x7F for accented/multilingual
 *       glyphs; Atari ST 2.x + Amiga Release 1 EN render
 *       characters above 0x7F as spaces).
 *
 *   The variant catalog below is source-cited against the
 *   dmweb "Hint Oracle Files" page (the canonical "which CSB
 *   Hint Oracle file is which" reference) and against the
 *   local Firestaff-data staging of the Atari ST 2.x PP
 *   2009-02-22 hard-disk variant + the Amiga 3.3 FR Meynaf
 *   hard-disk utility variant.
 *
 * This module is a thin, data-side metadata layer that sits on
 * top of:
 *
 *   - csb_hint_oracle_htc.h            (the format parser)
 *   - csb_hint_oracle_htc_real_scan.h  (the real-asset scan)
 *   - csb_hint_oracle_htc_real_known_hashes (the source-cited
 *     MD5 list, which already covers the two locally-verified
 *     variants)
 *
 * Scope:
 *   - Variant tag enumeration, by MD5 (preferred) and by parsed
 *     content (fallback).
 *   - Validation of a parsed HCSB.HTC against the documented
 *     location/hint/page count expectations.
 *   - Bounded diagnostic report describing the matched variant,
 *     expected counts, observed counts, and any drift between
 *     the two.
 *
 * What this module does NOT do:
 *   - It does not draw the Hint Oracle graphical overlay.
 *   - It does not claim parity for every Utility Disk release.
 *     The catalog only lists the five variants the dmweb
 *     Hint Oracle Files page documents with a known MD5; the
 *     known-hash list in csb_hint_oracle_htc_real_scan is the
 *     authority for which HCSB.HTCs we *accept* on disk.
 *   - It does not bind into the M11 game loop, M12 launch
 *     flow, or any savegame. It only enriches the existing
 *     text-side binding surface with variant metadata.
 *
 * Skip-safe by design: when the supplied cache is empty or the
 * loaded file does not match any documented variant, the
 * classifier reports CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN with
 * empty fields so callers can still produce a non-NULL output.
 *
 * Source references:
 *   - dmweb Hint Oracle Files page (the canonical "all known
 *     HCSB.HTC files" reference, including per-variant MD5s
 *     and per-variant location/hint/page counts).
 *   - ReDMCSB HINTLOAD.C:11-18 names HCSB.HTC as the
 *     canonical CSB Utility Disk Hint Oracle file.
 *   - ReDMCSB HINTHTC.C:177-358 validates the format 2 /
 *     dungeon 13 big-endian table all five variants share.
 *   - ReDMCSB HINTLZW.C:122-212 decompresses hint content
 *     the variants differ only in count + language of.
 */

#ifndef FIRESTAFF_CSB_HINT_ORACLE_HTC_VARIANT_H
#define FIRESTAFF_CSB_HINT_ORACLE_HTC_VARIANT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_htc_real_scan.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Variant tags ───────────────────────────────────────────────── */

/* The five documented CSB Utility Disk Hint Oracle variants
 * plus UNKNOWN for unclassified / off-list files. The tags
 * intentionally map 1:1 to the dmweb Hint Oracle Files page
 * "Hints File" table; do not renumber without updating the
 * `csb_hint_oracle_htc_variant_name()` switch. */
typedef enum {
    CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN = 0,

    /* Atari ST 2.0 / 2.1 EN + Amiga 3.x EN Release 1
     * ("R1"). 4,663 location records, 210 hints, 492 pages.
     * dmweb notes the Atari ST Utility Disk and Amiga
     * English Release 1 Utility Disk share the same file
     * bytes; this is also the variant csb-atari-st-2x/HCSB.HTC
     * locally stages. */
    CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN = 1,

    /* Amiga 3.x EN Release 2
     * (test-file-for-translations mistakenly released).
     * 5,036 location records, 219 hints, 512 pages. */
    CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_2_EN = 2,

    /* Amiga 3.x EN Release 3
     * (the correct replacement FTL released after R2).
     * 5,044 location records, 219 hints, 513 pages.
     * dmweb notes this variant displays characters above
     * 0x7F for multilingual support. */
    CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN = 3,

    /* Amiga 3.x French
     * (HCSBF.HTC, 5,036 location records, 219 hints, 512
     * pages). csb-extras/legacy-amiga-dms/.../HCSBF.HTC
     * locally stages this variant. */
    CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR = 4,

    /* Amiga 3.x German
     * (HCSBG.HTC, 5,036 location records, 219 hints, 512
     * pages). */
    CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE = 5
} CSB_HintOracleHTC_Variant;

/* ── Catalog row ────────────────────────────────────────────────── */

/* Per-variant metadata sourced from the dmweb Hint Oracle
 * Files page. The `expected_*` fields are the contract the
 * parser must hit for a content-based classification to
 * succeed (in addition to the format-2 / dungeon-13 header
 * every variant already shares). */
typedef struct {
    CSB_HintOracleHTC_Variant variant;
    const char *label;          /* short human-readable label */
    const char *release_name;   /* "R1" / "R2" / "R3" / "-" */
    const char *language;       /* "EN" / "FR" / "GE" / "EN/" */
    const char *md5;            /* 32-char hex MD5, NULL/empty
                                 * when the variant has no
                                 * locally-staged copy */
    size_t      expected_size;  /* documented file size */
    size_t      expected_location_count;
    size_t      expected_hint_count;
    size_t      expected_page_count;
    /* 1 when this variant uses 8-bit characters above 0x7F
     * (R3 EN + FR + GE); 0 for the Atari ST 2.x / Amiga R1 EN
     * variant whose Hint Oracle masks 0x80..0xFF as spaces. */
    int         uses_high_glyphs;
} CSB_HintOracleHTC_VariantCatalog;

const CSB_HintOracleHTC_VariantCatalog *
csb_hint_oracle_htc_variant_catalog(size_t *out_count);

/* ── Classify by MD5 ────────────────────────────────────────────── */

/* Returns the variant tag whose catalog `md5` matches
 * `hex_md5` exactly (case-insensitive), or
 * CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN when no variant
 * matches. `hex_md5` must be a 32-char lowercase hex string;
 * a NULL or short argument returns UNKNOWN. */
CSB_HintOracleHTC_Variant
csb_hint_oracle_htc_variant_from_md5(const char *hex_md5);

/* ── Classify by parsed content ─────────────────────────────────── */

/* Classify a parsed HCSB.HTC view by content only. The match
 * requires the format-2 / dungeon-13 header (already enforced
 * by csb_hint_oracle_htc_parse) AND an exact match on
 * location/hint/page counts AND an exact match on the
 * observed file size. The first catalog row that satisfies
 * all three constraints wins. Multiple variants share the
 * same counts (R2/R3 EN differ by 1 page; FR/GE share R2's
 * counts); in those cases the documented file size
 * disambiguates. Returns CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN
 * when no catalog row matches.
 *
 * `observed_file_size` may be 0 to skip the size gate (use
 * this only when the file is streamed and the size is not
 * known up front). */
CSB_HintOracleHTC_Variant
csb_hint_oracle_htc_variant_from_parsed(const CSB_HintOracleHTC *htc,
                                       size_t observed_file_size);

/* ── Classify a loaded cache (real-asset surface) ──────────────── */

/* Higher-level helper: take a loaded `CSB_HintOracleHTC_RealCache`
 * and return the variant tag. Tries MD5 first (preferred), and
 * falls back to parsed-content classification when the MD5 is
 * empty / not in the catalog. Returns
 * CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN when the cache is empty
 * (cache->loaded == 0) or when neither path matches. */
CSB_HintOracleHTC_Variant
csb_hint_oracle_htc_variant_from_cache(
    const CSB_HintOracleHTC_RealCache *cache);

/* ── Variant drift detail ───────────────────────────────────────── */

/* Detailed comparison between the catalog contract for
 * `variant` and a parsed HCSB.HTC view. `matches` is set to
 * 1 when the observed size + counts equal the catalog's
 * `expected_*` fields exactly. The struct is safe to
 * zero-initialize; the helper returns 0 on argument error
 * and leaves the struct unchanged. */
typedef struct {
    CSB_HintOracleHTC_Variant variant;
    int         matches;
    size_t      observed_size;
    size_t      expected_size;
    size_t      observed_location_count;
    size_t      expected_location_count;
    size_t      observed_hint_count;
    size_t      expected_hint_count;
    size_t      observed_page_count;
    size_t      expected_page_count;
} CSB_HintOracleHTC_VariantDrift;

int csb_hint_oracle_htc_variant_drift(
    CSB_HintOracleHTC_Variant variant,
    const CSB_HintOracleHTC *htc,
    size_t observed_file_size,
    CSB_HintOracleHTC_VariantDrift *out_drift);

/* ── Diagnostics ────────────────────────────────────────────────── */

const char *csb_hint_oracle_htc_variant_name(
    CSB_HintOracleHTC_Variant variant);

const char *csb_hint_oracle_htc_variant_release_name(
    CSB_HintOracleHTC_Variant variant);

const char *csb_hint_oracle_htc_variant_language(
    CSB_HintOracleHTC_Variant variant);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_HTC_VARIANT_H */
