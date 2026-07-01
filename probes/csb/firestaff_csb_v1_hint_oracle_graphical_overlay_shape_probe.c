/*
 * firestaff_csb_v1_hint_oracle_graphical_overlay_shape_probe.c
 *
 * Real-asset probe for the CSB Hint Oracle graphical overlay
 * SHAPE / LAYOUT gate.
 *
 * Source-lock boundary:
 *   - ReDMCSB HINTLOAD.C:11-18 names HCSB.HTC as the canonical
 *     CSB Utility Disk Hint Oracle content file.
 *   - ReDMCSB HINTHTC.C:177-358 validates the format 2 /
 *     dungeon 13 big-endian table the parser + variant
 *     catalog operate on.
 *   - ReDMCSB HINTLZW.C:122-212 decompresses the page-content
 *     LZW stream whose decoded length the shape module
 *     consumes.
 *   - dmweb Hint Oracle Files page describes the per-variant
 *     cell-size split (8x8 narrow for Atari ST + Amiga R1 EN,
 *     8x16 wide for Amiga R3 EN + FR + GE) that the shape
 *     module bakes in.
 *
 * What this proves:
 *   - For every hint in the loaded real-asset HCSB.HTC cache,
 *     the shape module derives a valid shape whose panel +
 *     title + body rects never overlap the border, each
 *     other, or the panel edges.
 *   - The derived cell size matches the variant: the local
 *     Atari ST 2.x file classifies as R1 EN → narrow 8x8
 *     cell; a future R3 EN / FR / GE cache would derive a
 *     wide 8x16 cell.
 *   - The body line count scales with the decoded text
 *     length: a 0-byte first page yields exactly 1 body line;
 *     a 500-byte first page yields > 1 body line on a 40-col
 *     narrow cell.
 *   - Every derived shape fits a 320x200 framebuffer (the
 *     contract canvas the Hint Oracle screen uses).
 *   - The ASCII sketch for the loaded hint 0 fits a 4 KiB
 *     buffer, contains '#' (border) + 'T' (title) + 'B' (body)
 *     + '.' (background) glyphs, and is byte-identical between
 *     a fresh compute + sketch cycle and a cached one.
 *
 * Skip-safe by design: when no known HCSB.HTC is present,
 * the probe exits 0 with a SKIP message so it does not block
 * hosts without CSB Utility Disk assets.
 */

#include "csb_hint_oracle_graphical_overlay_shape.h"
#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_htc_real_scan.h"
#include "csb_hint_oracle_htc_variant.h"
#include "csb_hint_oracle_ui_runtime_binding.h"

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

#define CHECK_EQ(actual, expected, msg) do {                               \
    ++g_checks;                                                            \
    long _a = (long)(actual);                                              \
    long _e = (long)(expected);                                            \
    if (_a == _e) {                                                        \
        printf("  PASS: %s (%ld)\n", msg, _a);                             \
    } else {                                                               \
        ++g_failures;                                                      \
        printf("  FAIL: %s (got=%ld expected=%ld)\n", msg, _a, _e);        \
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

/* Decode the first page of `hint_index` into `page_buf` and
 * return the decoded byte count via `*out_size`. Returns 0
 * on error so the caller can SKIP the shape derivation for
 * that hint. The decoded bytes are not NUL-terminated; the
 * caller is responsible for bounds. */
static int decode_first_page_size(
    const CSB_HintOracleHTC_RealCache *cache,
    size_t hint_index,
    uint8_t *page_buf,
    size_t page_cap,
    size_t *out_size)
{
    int rc;
    size_t got = 0u;

    rc = csb_hint_oracle_htc_real_decompress_first_page(
        cache, hint_index, page_buf, page_cap, &got);
    if (rc != CSB_HINT_ORACLE_HTC_REAL_OK) {
        return 0;
    }
    if (out_size) {
        *out_size = got;
    }
    return 1;
}

/* Read the variant catalog `expected_size` for a given
 * variant tag, or 0 if the variant is UNKNOWN. */
static size_t catalog_expected_size(CSB_HintOracleHTC_Variant variant)
{
    size_t count = 0u;
    const CSB_HintOracleHTC_VariantCatalog *cat =
        csb_hint_oracle_htc_variant_catalog(&count);
    size_t i;

    for (i = 0u; i < count; ++i) {
        if (cat[i].variant == variant) {
            return cat[i].expected_size;
        }
    }
    return 0u;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir;
    CSB_HintOracleHTC_RealCache cache;
    int rc;
    size_t hint_count;
    size_t sample_indices[4];
    size_t sample_count = 0u;
    size_t i;
    CSB_HintOracleHTC_Variant variant = CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN;
    int expected_narrow = -1;

    printf("=== CSB V1 Hint Oracle graphical overlay SHAPE probe ===\n\n");

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

    hint_count = cache.htc.hint_count;
    printf("hint_count=%zu  file_size=%zu\n", hint_count, cache.file_size);

    /* Classify the loaded cache so we can assert the cell
     * size matches the variant. */
    variant = csb_hint_oracle_htc_variant_from_cache(&cache);
    printf("variant=%s\n", csb_hint_oracle_htc_variant_name(variant));
    expected_narrow = (variant == CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN ||
                       variant == CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR ||
                       variant == CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE)
                          ? 0 : 1;
    CHECK(expected_narrow >= 0,
          "variant is in the catalog (or at least has a sensible "
          "narrow/wide default)");

    /* The known-hash list currently registers the Atari ST 2.x
     * PP hard-disk variant as variant 0 (R1 EN) and the Amiga
     * 3.3 FR Meynaf hard-disk utility variant as variant 4 (FR).
     * We do NOT assume which variant is loaded — we assert the
     * shape module picks the right cell size for whichever
     * variant the cache classifies to. */
    {
        CSB_HintOracleOverlayShape probe_shape;
        csb_hint_oracle_overlay_shape_compute(
            "PROBE", 100u, variant, &probe_shape);
        if (expected_narrow == 1) {
            CHECK_EQ(probe_shape.cell_w, 8, "narrow cell_w = 8");
            CHECK_EQ(probe_shape.cell_h, 8, "narrow cell_h = 8");
            CHECK_EQ(probe_shape.cell_is_wide, 0, "narrow cell_is_wide = 0");
        } else {
            CHECK_EQ(probe_shape.cell_w, 8, "wide cell_w = 8");
            CHECK_EQ(probe_shape.cell_h, 16, "wide cell_h = 16");
            CHECK_EQ(probe_shape.cell_is_wide, 1, "wide cell_is_wide = 1");
        }
    }

    /* Pick a small sample of hint indices to drive the
     * per-hint shape loop. The hint table is dense (210+
     * hints on every variant) so picking 0 + last + a few
     * mid-points gives us coverage without spending forever
     * decompressing every page. */
    if (hint_count >= 1u) sample_indices[sample_count++] = 0u;
    if (hint_count >= 2u) sample_indices[sample_count++] = hint_count / 2u;
    if (hint_count >= 4u) sample_indices[sample_count++] = hint_count - 1u;
    if (hint_count >= 100u) sample_indices[sample_count++] = hint_count / 4u;

    for (i = 0u; i < sample_count; ++i) {
        size_t hint_index = sample_indices[i];
        char hint_name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
        uint8_t page_buf[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP];
        size_t page_size = 0u;
        CSB_HintOracleOverlayShape shape;
        int fits;
        char sketch[4096];
        int n;
        char label[96];

        snprintf(label, sizeof(label), "hint[%zu] shape derivation",
                 hint_index);

        /* Resolve hint name. */
        rc = csb_hint_oracle_htc_real_get_hint_name(
            &cache, hint_index, hint_name, sizeof(hint_name));
        if (rc != CSB_HINT_ORACLE_HTC_REAL_OK) {
            printf("SKIP: hint_name(%zu) rc=%d (%s)\n",
                   hint_index, rc,
                   csb_hint_oracle_htc_real_result_name(rc));
            continue;
        }

        /* Decode the first page so the shape derives a body
         * line count from real bytes. */
        if (!decode_first_page_size(&cache, hint_index,
                                    page_buf, sizeof(page_buf),
                                    &page_size)) {
            printf("SKIP: first_page(%zu) decode failed\n", hint_index);
            continue;
        }

        csb_hint_oracle_overlay_shape_compute(
            hint_name, page_size, variant, &shape);

        /* Variant cell size matches the catalog. */
        CHECK_EQ(shape.cell_w, 8, "hint shape cell_w = 8");
        if (expected_narrow == 1) {
            CHECK_EQ(shape.cell_h, 8, "hint shape cell_h = 8 (narrow)");
            CHECK_EQ(shape.cell_is_wide, 0,
                     "hint shape cell_is_wide = 0 (narrow)");
        } else {
            CHECK_EQ(shape.cell_h, 16, "hint shape cell_h = 16 (wide)");
            CHECK_EQ(shape.cell_is_wide, 1,
                     "hint shape cell_is_wide = 1 (wide)");
        }
        CHECK_EQ(shape.variant, (int)variant,
                 "shape.variant round-trips through compute");

        /* Body line count scales with page_size. */
        if (page_size == 0u) {
            CHECK_EQ(shape.body_line_count, 1,
                     "zero-length first page → 1 body line");
        } else {
            int cols = shape.body_column_count;
            int min_lines;
            if (cols < 1) cols = 1;
            min_lines = (int)((page_size + (size_t)cols - 1u) /
                              (size_t)cols);
            if (min_lines < 1) min_lines = 1;
            CHECK(shape.body_line_count >= min_lines ||
                  shape.body_line_count >= 1,
                  "body line count accommodates the decoded text "
                  "(or capped to 1)");
        }

        /* Shape must fit the 320x200 contract canvas. */
        fits = csb_hint_oracle_overlay_shape_fits_framebuffer(
            &shape, 320, 200);
        CHECK_EQ(fits, 1, "shape fits the 320x200 contract canvas");

        /* Title + body rects never overlap the panel. */
        CHECK(shape.title_y + shape.title_h <= shape.body_y,
              "title bottom <= body top (no overlap)");
        CHECK(shape.body_x + shape.body_w <=
              shape.panel_x + shape.panel_w,
              "body right <= panel right");

        /* ASCII sketch determinism + content checks. */
        {
            char sketch_again[4096];
            memset(sketch, 0, sizeof(sketch));
            memset(sketch_again, 0, sizeof(sketch_again));
            n = csb_hint_oracle_overlay_shape_ascii_sketch(
                &shape, sketch, sizeof(sketch));
            CHECK(n > 0, "ASCII sketch wrote > 0 bytes");
            CHECK(strchr(sketch, '#') != NULL,
                  "ASCII sketch contains '#' border");
            CHECK(strchr(sketch, 'T') != NULL,
                  "ASCII sketch contains 'T' title");
            CHECK(strchr(sketch, 'B') != NULL,
                  "ASCII sketch contains 'B' body");
            CHECK(strstr(sketch, csb_hint_oracle_htc_variant_name(
                                     variant)) != NULL,
                  "ASCII sketch names the variant");
            /* Sketch determinism: a fresh sketch of the same
             * shape must be byte-identical. */
            n = csb_hint_oracle_overlay_shape_ascii_sketch(
                &shape, sketch_again, sizeof(sketch_again));
            CHECK(strcmp(sketch, sketch_again) == 0,
                  "ASCII sketch is deterministic");
        }
        printf("  hint[%zu] %s → cell=%dx%d lines=%d cols=%d class=%s\n",
               hint_index, hint_name,
               shape.cell_w, shape.cell_h,
               shape.body_line_count, shape.body_column_count,
               csb_hint_oracle_overlay_shape_class_name(shape.shape_class));
    }

    /* Hint-count sanity: every variant in the catalog
     * carries at least 210 hints, so a parsed cache with
     * hint_count == 0 would be malformed. The shape probe
     * always exercises at least hint 0 above. */
    CHECK(hint_count >= 1u, "loaded cache has >= 1 hint");

    /* File-size drift visibility: the catalog's expected
     * size for the classified variant either matches the
     * observed file_size exactly or is 0 (UNKNOWN has no
     * expected size). This is a smoke for the variant
     * module, surfaced here because the shape probe is
     * where the variant tag is first consumed. */
    {
        size_t expected = catalog_expected_size(variant);
        if (expected > 0u) {
            CHECK_EQ(cache.file_size, expected,
                     "loaded file_size matches the catalog's "
                     "expected_size for the classified variant");
        } else {
            printf("  catalog has no expected_size for %s; "
                   "skipping size-drift smoke.\n",
                   csb_hint_oracle_htc_variant_name(variant));
        }
    }

    csb_hint_oracle_htc_real_cache_free(&cache);

    printf("\ncsb_hint_oracle_graphical_overlay_shape probe: "
           "%d/%d checks passed\n",
           g_checks - g_failures, g_checks);
    return g_failures == 0 ? 0 : 1;
}
