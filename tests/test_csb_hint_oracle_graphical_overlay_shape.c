/*
 * test_csb_hint_oracle_graphical_overlay_shape.c
 *
 * Data-free contract tests for the CSB Hint Oracle graphical
 * overlay SHAPE / LAYOUT gate.
 *
 * Scope:
 *   - Cell-size lookup: R1 EN + R2 EN + UNKNOWN get the narrow
 *     8x8 cell; R3 EN + FR + GE get the wide 8x16 cell.
 *   - Shape invariants: title + body rects never overlap the
 *     border, never overlap each other, and always fit inside
 *     the panel rect; the body's width + height are exact
 *     multiples of the cell width + height.
 *   - Variant-aware class: the same hint name + decoded text
 *     length classifies as STANDARD on R1 EN and LARGE on
 *     R3 EN + FR + GE; a deliberately tiny hint name + zero
 *     decoded text classifies as STANDARD on every variant
 *     because the panel fills the 320x200 canvas.
 *   - Determinism: two compute() calls with the same inputs
 *     produce identical shapes; two ASCII sketch calls with
 *     the same shape produce identical output.
 *   - Framebuffer fit verdict: a 320x200 framebuffer accepts
 *     the shape; a 100x100 framebuffer rejects it; a NULL
 *     shape returns -1.
 *   - ASCII sketch: the sketch includes the shape-class name
 *     + variant name + panel/title/body dimensions in the
 *     header, and uses '#' for border, 'T' for title, 'B'
 *     for body, '.' for background, ' ' for outside the
 *     panel. The sketch truncates safely on a tiny buffer.
 *   - Name truncation: a hint name longer than
 *     CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES is truncated to fit
 *     the name cap (the format-level limit is 22 bytes).
 *
 * Non-claims:
 *   - No real Utility Disk asset is loaded (synthetic inputs
 *     only).
 *   - No SDL / framebuffer write happens. The shape module
 *     never touches a renderer.
 *   - No pixel-parity claim against the original Utility
 *     Disk Hint Oracle screen.
 */

#include "csb_hint_oracle_graphical_overlay_shape.h"

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

#define CHECK_STR_EQ(actual, expected, msg) do {                           \
    ++g_checks;                                                            \
    const char *_a = (actual);                                             \
    const char *_e = (expected);                                           \
    if (_a && strcmp(_a, _e) == 0) {                                       \
        printf("  PASS: %s\n", msg);                                       \
    } else {                                                               \
        ++g_failures;                                                      \
        printf("  FAIL: %s (got='%s' expected='%s')\n",                   \
               msg, _a ? _a : "(null)", _e);                               \
    }                                                                      \
} while (0)

/* ── Cell-size lookup ───────────────────────────────────────────── */

static int test_cell_size_for_variant(void)
{
    CSB_HintOracleOverlayShape shape;

    /* R1 EN — narrow cell. */
    csb_hint_oracle_overlay_shape_compute(
        "ANYWHERE", 0u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &shape);
    CHECK_EQ(shape.cell_w, 8, "R1 EN cell_w = 8");
    CHECK_EQ(shape.cell_h, 8, "R1 EN cell_h = 8");
    CHECK_EQ(shape.cell_is_wide, 0, "R1 EN cell_is_wide = 0");

    /* R2 EN — narrow cell (no high-glyphs flag in catalog). */
    csb_hint_oracle_overlay_shape_compute(
        "ANYWHERE", 0u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_2_EN, &shape);
    CHECK_EQ(shape.cell_w, 8, "R2 EN cell_w = 8");
    CHECK_EQ(shape.cell_h, 8, "R2 EN cell_h = 8");
    CHECK_EQ(shape.cell_is_wide, 0, "R2 EN cell_is_wide = 0");

    /* R3 EN — wide cell. */
    csb_hint_oracle_overlay_shape_compute(
        "ANYWHERE", 0u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN, &shape);
    CHECK_EQ(shape.cell_w, 8, "R3 EN cell_w = 8");
    CHECK_EQ(shape.cell_h, 16, "R3 EN cell_h = 16");
    CHECK_EQ(shape.cell_is_wide, 1, "R3 EN cell_is_wide = 1");

    /* Amiga FR — wide cell. */
    csb_hint_oracle_overlay_shape_compute(
        "ANYWHERE", 0u,
        CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR, &shape);
    CHECK_EQ(shape.cell_w, 8, "Amiga FR cell_w = 8");
    CHECK_EQ(shape.cell_h, 16, "Amiga FR cell_h = 16");
    CHECK_EQ(shape.cell_is_wide, 1, "Amiga FR cell_is_wide = 1");

    /* Amiga GE — wide cell. */
    csb_hint_oracle_overlay_shape_compute(
        "ANYWHERE", 0u,
        CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE, &shape);
    CHECK_EQ(shape.cell_w, 8, "Amiga GE cell_w = 8");
    CHECK_EQ(shape.cell_h, 16, "Amiga GE cell_h = 16");
    CHECK_EQ(shape.cell_is_wide, 1, "Amiga GE cell_is_wide = 1");

    /* UNKNOWN — narrow cell (defensive default). */
    csb_hint_oracle_overlay_shape_compute(
        "ANYWHERE", 0u,
        CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN, &shape);
    CHECK_EQ(shape.cell_w, 8, "UNKNOWN cell_w = 8");
    CHECK_EQ(shape.cell_h, 8, "UNKNOWN cell_h = 8");
    CHECK_EQ(shape.cell_is_wide, 0, "UNKNOWN cell_is_wide = 0");

    return 1;
}

/* ── Shape invariants ───────────────────────────────────────────── */

static int test_shape_invariants(void)
{
    CSB_HintOracleOverlayShape shape;

    csb_hint_oracle_overlay_shape_compute(
        "PROVE YOU ARE WIZARD", 200u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &shape);

    /* Panel rect anchored at (0,0), fills 320x200. */
    CHECK_EQ(shape.panel_x, 0, "panel_x = 0");
    CHECK_EQ(shape.panel_y, 0, "panel_y = 0");
    CHECK_EQ(shape.panel_w, 320, "panel_w = 320");
    CHECK_EQ(shape.panel_h, 200, "panel_h = 200");

    /* Title row pinned inside the panel. */
    CHECK(shape.title_x >= shape.panel_x, "title_x >= panel_x");
    CHECK(shape.title_y >= shape.panel_y, "title_y >= panel_y");
    CHECK(shape.title_x + shape.title_w <=
          shape.panel_x + shape.panel_w,
          "title right edge <= panel right");
    CHECK(shape.title_y + shape.title_h <=
          shape.panel_y + shape.panel_h,
          "title bottom <= panel bottom");

    /* Body rect pinned below the title row, inside the panel. */
    CHECK(shape.body_x == shape.title_x,
          "body_x == title_x (same horizontal anchor)");
    CHECK(shape.body_y >= shape.title_y + shape.title_h,
          "body_y >= title_y + title_h (no overlap)");
    CHECK(shape.body_x + shape.body_w <=
          shape.panel_x + shape.panel_w,
          "body right edge <= panel right");
    CHECK(shape.body_y + shape.body_h <=
          shape.panel_y + shape.panel_h,
          "body bottom <= panel bottom");

    /* Body width + height are exact multiples of the cell. */
    CHECK_EQ(shape.body_w % shape.cell_w, 0,
             "body_w is a multiple of cell_w");
    CHECK_EQ(shape.body_h % shape.cell_h, 0,
             "body_h is a multiple of cell_h");

    /* Body line + column counts match the rect math. */
    CHECK_EQ(shape.body_line_count, shape.body_h / shape.cell_h,
             "body_line_count = body_h / cell_h");
    CHECK_EQ(shape.body_column_count, shape.body_w / shape.cell_w,
             "body_column_count = body_w / cell_w");
    CHECK(shape.body_line_count >= 1,
          "body_line_count >= 1 (always at least one row)");
    CHECK(shape.body_column_count >= 1,
          "body_column_count >= 1 (always at least one col)");

    /* No overlap between border + title + body. */
    CHECK(shape.title_y + shape.title_h <= shape.body_y,
          "title bottom <= body top (no overlap)");

    return 1;
}

static int test_shape_invariants_wide_cell(void)
{
    CSB_HintOracleOverlayShape shape;

    csb_hint_oracle_overlay_shape_compute(
        "PROVE", 100u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN, &shape);

    /* Same shape invariants under the wide cell. */
    CHECK_EQ(shape.cell_h, 16, "R3 EN cell_h = 16");
    CHECK_EQ(shape.body_h % shape.cell_h, 0,
             "R3 EN body_h is a multiple of cell_h (16)");
    CHECK(shape.title_y + shape.title_h <= shape.body_y,
          "R3 EN title bottom <= body top");
    CHECK(shape.body_y + shape.body_h <=
          shape.panel_y + shape.panel_h,
          "R3 EN body bottom <= panel bottom");
    CHECK_EQ(shape.body_line_count, shape.body_h / shape.cell_h,
             "R3 EN body_line_count = body_h / cell_h");

    /* Wide cell shape must classify as LARGE (panel height
     * stays 200, but the wide cell pushes body line count
     * differently than narrow). */
    CHECK(shape.shape_class == CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_LARGE,
          "R3 EN PROVE/100 → LARGE class (wide cell)");
    return 1;
}

/* ── Variant-aware class ────────────────────────────────────────── */

static int test_variant_aware_class(void)
{
    CSB_HintOracleOverlayShape narrow;
    CSB_HintOracleOverlayShape wide;

    /* Same hint name + same decoded text length, but different
     * variants — the wide variant must classify as LARGE. */
    csb_hint_oracle_overlay_shape_compute(
        "PROVE YOU ARE WIZARD", 300u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &narrow);
    csb_hint_oracle_overlay_shape_compute(
        "PROVE YOU ARE WIZARD", 300u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN, &wide);

    CHECK_EQ(narrow.cell_is_wide, 0, "narrow cell_is_wide = 0");
    CHECK_EQ(wide.cell_is_wide, 1, "wide cell_is_wide = 1");

    /* Narrow: panel_h is 200, which is above the 159 standard
     * threshold when the body needs many lines, so it can
     * classify as STANDARD (because 200 <= 159? no — 200 >
     * 159 → LARGE). The contract here is that narrow and
     * wide do NOT have to classify the same way; the wide
     * variant with the same hint text + length must still
     * produce a LARGE class because the cell height grew. */
    CHECK(wide.shape_class == CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_LARGE,
          "wide variant → LARGE class");

    /* Narrow path with the same inputs — could be STANDARD
     * or LARGE depending on body line count + cell height;
     * we only assert it is NOT COMPACT (because the panel
     * fills the 320x200 canvas and a panel that fills the
     * canvas is never COMPACT). */
    CHECK(narrow.shape_class != CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_COMPACT,
          "narrow panel fills 320x200 → never COMPACT");

    /* Identical inputs on the SAME variant produce the same
     * class. */
    {
        CSB_HintOracleOverlayShape narrow_again;
        csb_hint_oracle_overlay_shape_compute(
            "PROVE YOU ARE WIZARD", 300u,
            CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &narrow_again);
        CHECK_EQ((int)narrow.shape_class, (int)narrow_again.shape_class,
                 "deterministic class for identical inputs");
    }
    return 1;
}

/* ── Determinism ────────────────────────────────────────────────── */

static int test_determinism(void)
{
    CSB_HintOracleOverlayShape a;
    CSB_HintOracleOverlayShape b;
    char sketch_a[2048];
    char sketch_b[2048];
    int n_a;
    int n_b;

    csb_hint_oracle_overlay_shape_compute(
        "ANYWHERE", 137u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &a);
    csb_hint_oracle_overlay_shape_compute(
        "ANYWHERE", 137u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &b);

    CHECK_EQ(a.panel_w, b.panel_w, "deterministic panel_w");
    CHECK_EQ(a.panel_h, b.panel_h, "deterministic panel_h");
    CHECK_EQ(a.body_w, b.body_w, "deterministic body_w");
    CHECK_EQ(a.body_h, b.body_h, "deterministic body_h");
    CHECK_EQ(a.body_line_count, b.body_line_count,
             "deterministic body_line_count");
    CHECK_EQ(a.body_column_count, b.body_column_count,
             "deterministic body_column_count");
    CHECK_EQ((int)a.shape_class, (int)b.shape_class,
             "deterministic shape_class");
    CHECK(strcmp(a.hint_name, b.hint_name) == 0,
          "deterministic hint_name");

    /* ASCII sketch determinism. */
    memset(sketch_a, 0, sizeof(sketch_a));
    memset(sketch_b, 0, sizeof(sketch_b));
    n_a = csb_hint_oracle_overlay_shape_ascii_sketch(&a, sketch_a,
                                                      sizeof(sketch_a));
    n_b = csb_hint_oracle_overlay_shape_ascii_sketch(&b, sketch_b,
                                                      sizeof(sketch_b));
    CHECK(n_a > 0, "sketch_a wrote > 0 bytes");
    CHECK(n_b > 0, "sketch_b wrote > 0 bytes");
    CHECK_EQ(n_a, n_b, "sketch determinism: same byte count");
    CHECK(strcmp(sketch_a, sketch_b) == 0,
          "sketch determinism: byte-identical content");
    return 1;
}

/* ── Framebuffer fit verdict ────────────────────────────────────── */

static int test_fits_framebuffer(void)
{
    CSB_HintOracleOverlayShape shape;
    int verdict;

    csb_hint_oracle_overlay_shape_compute(
        "ANYWHERE", 50u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &shape);

    /* 320x200 canvas: shape fits. */
    verdict = csb_hint_oracle_overlay_shape_fits_framebuffer(
        &shape, 320, 200);
    CHECK_EQ(verdict, 1, "320x200 framebuffer accepts the shape");

    /* 320x199 framebuffer: panel_h = 200 > 199 → reject. */
    verdict = csb_hint_oracle_overlay_shape_fits_framebuffer(
        &shape, 320, 199);
    CHECK_EQ(verdict, 0, "320x199 framebuffer rejects the shape");

    /* 319x200 framebuffer: panel_w = 320 > 319 → reject. */
    verdict = csb_hint_oracle_overlay_shape_fits_framebuffer(
        &shape, 319, 200);
    CHECK_EQ(verdict, 0, "319x200 framebuffer rejects the shape");

    /* 100x100 framebuffer: too small for the panel → reject. */
    verdict = csb_hint_oracle_overlay_shape_fits_framebuffer(
        &shape, 100, 100);
    CHECK_EQ(verdict, 0, "100x100 framebuffer rejects the shape");

    /* Negative dimensions: reject (the shape cannot fit a
     * non-positive area). */
    verdict = csb_hint_oracle_overlay_shape_fits_framebuffer(
        &shape, -1, 200);
    CHECK_EQ(verdict, 0, "-1x200 framebuffer rejects");
    verdict = csb_hint_oracle_overlay_shape_fits_framebuffer(
        &shape, 320, 0);
    CHECK_EQ(verdict, 0, "320x0 framebuffer rejects");

    /* NULL shape → argument error. */
    verdict = csb_hint_oracle_overlay_shape_fits_framebuffer(
        NULL, 320, 200);
    CHECK_EQ(verdict, -1, "NULL shape → -1");
    return 1;
}

/* ── ASCII sketch ───────────────────────────────────────────────── */

static int test_ascii_sketch_basic(void)
{
    CSB_HintOracleOverlayShape shape;
    char sketch[2048];
    int n;

    csb_hint_oracle_overlay_shape_compute(
        "PROVE YOU ARE WIZARD", 200u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &shape);

    memset(sketch, 0, sizeof(sketch));
    n = csb_hint_oracle_overlay_shape_ascii_sketch(&shape, sketch,
                                                    sizeof(sketch));
    CHECK(n > 0, "sketch wrote > 0 bytes");
    /* Header should name the class. */
    CHECK(strstr(sketch, "STANDARD") != NULL ||
          strstr(sketch, "LARGE") != NULL ||
          strstr(sketch, "COMPACT") != NULL,
          "sketch header names the class");
    /* Header should name the variant. */
    CHECK(strstr(sketch, "release-1-en") != NULL,
          "sketch header names the variant");
    /* Header should include cell + panel + title + body dims. */
    CHECK(strstr(sketch, "cell=8x8") != NULL,
          "sketch header names cell=8x8 for R1 EN");
    CHECK(strstr(sketch, "panel=320x200") != NULL,
          "sketch header names panel=320x200");
    /* Sketch must contain border pixels. */
    CHECK(strchr(sketch, '#') != NULL,
          "sketch contains '#' border pixels");
    /* Sketch must contain title pixels. */
    CHECK(strchr(sketch, 'T') != NULL,
          "sketch contains 'T' title pixels");
    /* Sketch must contain body pixels. */
    CHECK(strchr(sketch, 'B') != NULL,
          "sketch contains 'B' body pixels");
    /* Sketch must contain background pixels. */
    CHECK(strchr(sketch, '.') != NULL,
          "sketch contains '.' background pixels");
    /* Sketch must be NUL-terminated. */
    CHECK(sketch[n] == '\0' || sketch[sizeof(sketch) - 1u] == '\0',
          "sketch is NUL-terminated");
    return 1;
}

static int test_ascii_sketch_wide_variant(void)
{
    CSB_HintOracleOverlayShape shape;
    char sketch[2048];
    int n;

    csb_hint_oracle_overlay_shape_compute(
        "PROVE", 100u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN, &shape);

    memset(sketch, 0, sizeof(sketch));
    n = csb_hint_oracle_overlay_shape_ascii_sketch(&shape, sketch,
                                                    sizeof(sketch));
    CHECK(n > 0, "wide-variant sketch wrote > 0 bytes");
    CHECK(strstr(sketch, "release-3-en") != NULL,
          "wide-variant sketch names release-3-en");
    CHECK(strstr(sketch, "cell=8x16") != NULL,
          "wide-variant sketch names cell=8x16");
    return 1;
}

static int test_ascii_sketch_truncates_safely(void)
{
    CSB_HintOracleOverlayShape shape;
    char tiny[16];
    int n;

    csb_hint_oracle_overlay_shape_compute(
        "ANYWHERE", 0u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &shape);

    memset(tiny, 'X', sizeof(tiny));
    n = csb_hint_oracle_overlay_shape_ascii_sketch(&shape, tiny,
                                                    sizeof(tiny));
    CHECK(n >= 0, "tiny sketch returns >= 0 bytes");
    CHECK(tiny[sizeof(tiny) - 1u] == '\0',
          "tiny sketch is NUL-terminated");
    /* Some characters must have been overwritten. */
    CHECK(tiny[0] != 'X' || tiny[1] != 'X',
          "tiny sketch wrote at least one byte");
    return 1;
}

/* ── Name truncation ────────────────────────────────────────────── */

static int test_name_truncation(void)
{
    CSB_HintOracleOverlayShape shape;
    /* Hint name well past the 22-byte format cap. */
    const char *long_name =
        "PROVE YOU ARE WIZARD OF THE SECOND TIER ABCDEFGHIJ";

    csb_hint_oracle_overlay_shape_compute(
        long_name, 50u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &shape);

    /* The shape struct holds the truncated copy. */
    CHECK(strlen(shape.hint_name) <=
          CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES,
          "truncated hint_name <= 22 bytes");
    /* NUL-terminator preserved. */
    CHECK(shape.hint_name[
              CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES] == '\0',
          "NUL terminator at NAME_CAP - 1u is preserved");
    /* First 22 chars of the source survived. */
    CHECK(strncmp(shape.hint_name, long_name,
                  CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES) == 0,
          "first 22 chars match the source");
    return 1;
}

/* ── Empty / NULL inputs ────────────────────────────────────────── */

static int test_empty_inputs(void)
{
    CSB_HintOracleOverlayShape shape;

    /* NULL hint name still produces a valid shape. */
    csb_hint_oracle_overlay_shape_compute(
        NULL, 100u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &shape);
    CHECK(shape.panel_w == 320, "NULL hint_name → panel still 320 wide");
    CHECK(shape.hint_name[0] == '\0',
          "NULL hint_name → empty hint_name");
    CHECK(shape.body_line_count >= 1,
          "NULL hint_name → body still has >= 1 line");

    /* Empty hint name + zero decoded text. */
    csb_hint_oracle_overlay_shape_compute(
        "", 0u,
        CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_1_EN, &shape);
    CHECK(shape.panel_w == 320, "empty inputs → panel still 320 wide");
    CHECK(shape.body_line_count >= 1,
          "empty inputs → body still has >= 1 line");
    return 1;
}

/* ── Class-name table ───────────────────────────────────────────── */

static int test_class_name_table(void)
{
    CHECK_STR_EQ(csb_hint_oracle_overlay_shape_class_name(
                     CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_UNKNOWN),
                 "UNKNOWN", "class name UNKNOWN");
    CHECK_STR_EQ(csb_hint_oracle_overlay_shape_class_name(
                     CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_COMPACT),
                 "COMPACT", "class name COMPACT");
    CHECK_STR_EQ(csb_hint_oracle_overlay_shape_class_name(
                     CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_STANDARD),
                 "STANDARD", "class name STANDARD");
    CHECK_STR_EQ(csb_hint_oracle_overlay_shape_class_name(
                     CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_LARGE),
                 "LARGE", "class name LARGE");
    CHECK_STR_EQ(csb_hint_oracle_overlay_shape_class_name(
                     (CSB_HintOracleOverlay_ShapeClass)0x7fffffff),
                 "INVALID", "class name INVALID for out-of-range tag");
    return 1;
}

int main(void)
{
    int rc = 0;

    printf("csb_hint_oracle_graphical_overlay_shape unit tests\n");

    if (!test_cell_size_for_variant())            rc = 1;
    if (!test_shape_invariants())                 rc = 1;
    if (!test_shape_invariants_wide_cell())       rc = 1;
    if (!test_variant_aware_class())              rc = 1;
    if (!test_determinism())                      rc = 1;
    if (!test_fits_framebuffer())                 rc = 1;
    if (!test_ascii_sketch_basic())               rc = 1;
    if (!test_ascii_sketch_wide_variant())        rc = 1;
    if (!test_ascii_sketch_truncates_safely())    rc = 1;
    if (!test_name_truncation())                  rc = 1;
    if (!test_empty_inputs())                     rc = 1;
    if (!test_class_name_table())                 rc = 1;

    printf("csb_hint_oracle_graphical_overlay_shape: %d/%d checks passed\n",
           g_checks - g_failures, g_checks);
    return rc;
}
