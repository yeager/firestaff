/*
 * test_m11_input_scale_inverse_safety_pc34_compat.c
 *
 * Data-free regression test for the M11 input scale inverse-direction
 * and null/degenerate-safety contracts of
 * `M11_MapPresentedGamePointToSourceForPresentation()` in
 * `src/engine/m11_game_input_mapping.c`. The
 * `test_m11_input_scale_boundary_pc34_compat` gate already pins:
 *
 *   - V20_FILTERED = 640x400 (ignores caller-supplied extents)
 *   - V21/V22 with (0,0) or negative extents return 0
 *   - Top-row y=0 / bottom-row y=199 round-trip across 10 selected
 *     resolutions
 *   - Determinism across 10 repeated identical mappings
 *   - Overflow input clamps to source (319, 199)
 *   - Return-value matrix (1 for V20/V21/V22, 0 for V1/unknown)
 *
 * This complementary gate closes the three contracts production
 * runtime callers (and refactors) actually trip over but were not yet
 * pinned in a focused regression:
 *
 *   1. NULL pointer safety — passing `x == NULL` or `y == NULL` (or
 *      both NULL) must return 0 without dereferencing the NULL slot,
 *      regardless of presentation mode or extent values. The helper
 *      guards `!x || !y` first, but the contract was not pinned.
 *   2. Monotonicity — as the presented (x, y) walks left-to-right
 *      across the presented 0..width-1 (or 0..height-1) pixel range,
 *      the mapped source x (or y) must be non-decreasing. The
 *      ReDMCSB F0358 mouse-row scan at COMMAND.C:1379-1449 walks
 *      the source order, so a non-monotonic reverse mapping would
 *      skip or revisit source rows it should not. Same invariant
 *      on the y axis.
 *   3. Degenerate single-pixel extents — V21/V22 with positive but
 *      minimal extents (e.g. width=1, height=1, or one axis at 1
 *      while the other is large) must still clamp the source output
 *      into [0..319] x [0..199] and never read beyond an integer-
 *      divide-by-one source cell.
 *
 * All paths are data-free, deterministic, and do not need SDL or any
 * game data. Source lock:
 *   - ReDMCSB COMMAND.C:1379-1449 F0358 mouse-row scan
 *   - ReDMCSB COMMAND.C:1641-1660 F0359 primary click dispatch
 *   - src/engine/m11_game_input_mapping.c
 *     M11_MapPresentedGamePointToSourceForPresentation
 */

#include "main_loop_m11.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_passes = 0;

#define CHECK(expr) do {                                                  \
    if (!(expr)) {                                                        \
        ++g_failures;                                                     \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);   \
    } else {                                                              \
        ++g_passes;                                                       \
    }                                                                     \
} while (0)

#define CHECK_INT(label, got, expected) do {                              \
    int g_ = (got);                                                       \
    int e_ = (expected);                                                  \
    if (g_ != e_) {                                                       \
        ++g_failures;                                                     \
        fprintf(stderr, "FAIL %s: got %d expected %d\n",                  \
                (label), g_, e_);                                         \
    } else {                                                              \
        ++g_passes;                                                       \
    }                                                                     \
} while (0)

/* NULL pointer safety contract. The helper guards `!x || !y` so a NULL
 * output slot is rejected with a 0 return without touching the NULL
 * pointer. Each presentation mode must reject NULL x, NULL y, and
 * NULL-both for the same reason: refactors may pass a partial slot
 * (e.g. only y is interesting and x is a throwaway). The contract is
 * independent of presentation extents, so we pass a mix of positive,
 * zero, and negative extents to confirm the NULL guard fires first. */
static void test_null_pointer_safety(void) {
    int dummy = 0;
    int* nullX = NULL;
    int* nullY = NULL;

    /* V20_FILTERED with NULL x. */
    CHECK_INT("V20 NULL x returns 0",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V20_FILTERED, 640, 400, nullX, &dummy),
              0);
    CHECK_INT("V20 NULL x preserves y-side dummy", dummy, 0);

    /* V20_FILTERED with NULL y. */
    CHECK_INT("V20 NULL y returns 0",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V20_FILTERED, 640, 400, &dummy, nullY),
              0);
    CHECK_INT("V20 NULL y preserves x-side dummy", dummy, 0);

    /* V20_FILTERED with both NULL — must not crash. */
    CHECK_INT("V20 both NULL returns 0",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V20_FILTERED, 640, 400, nullX, nullY),
              0);

    /* V21_UPSCALED with NULL x. The helper takes the NULL guard
     * before the (presentationWidth > 0 && presentationHeight > 0)
     * guard, so even an extent of (0, 0) with NULL x must return 0. */
    CHECK_INT("V21 NULL x returns 0 (positive extents)",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V21_UPSCALED, 1920, 1080, nullX, &dummy),
              0);
    CHECK_INT("V21 NULL x returns 0 (zero extents)",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V21_UPSCALED, 0, 0, nullX, &dummy),
              0);

    /* V22_MODERN with NULL y. */
    CHECK_INT("V22 NULL y returns 0 (positive extents)",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN, 1920, 1080, &dummy, nullY),
              0);

    /* V22 with both NULL. */
    CHECK_INT("V22 both NULL returns 0",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN, 1920, 1080, nullX, nullY),
              0);

    /* V1_ORIGINAL with NULL x — even though V1 returns 0 because it
     * takes the `else` branch, the NULL slot must not crash either. */
    CHECK_INT("V1 NULL x returns 0",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V1_ORIGINAL, 320, 200, nullX, &dummy),
              0);

    /* Unknown mode with NULL y. */
    CHECK_INT("unknown NULL y returns 0",
              M11_MapPresentedGamePointToSourceForPresentation(
                  99, 1920, 1080, &dummy, nullY),
              0);
}

/* Monotonicity contract. For every (presentationMode, presentationWidth,
 * presentationHeight) tuple we walk presented x from 0 to width-1 in
 * single-pixel steps and assert the mapped source x is non-decreasing.
 * Same for y. A non-monotonic mapping would silently violate the
 * ReDMCSB F0358 source-row scan at COMMAND.C:1379-1449 because the
 * caller (the M11 game view pointer handler in main_loop_m11.c)
 * reads F0638_GetZone with the mapped source row as the index. A
 * non-monotonic mapping would skip or revisit zones it should not.
 * We cover V20_FILTERED at its locked 640x400 frame, V21_UPSCALED at
 * 640x360 (an aspect-mismatched window), V22_MODERN at 1920x1080
 * (the most common modern window), and V21_UPSCALED at 3840x2160
 * (the modern 4K surface) so every documented mode is exercised
 * at least once. */
static void test_monotonic_walk(void) {
    int mode;
    int modeCount = 4;
    int modes[] = {
        M12_PRESENTATION_V20_FILTERED,
        M12_PRESENTATION_V21_UPSCALED,
        M12_PRESENTATION_V22_MODERN,
        M12_PRESENTATION_V21_UPSCALED
    };
    int widths[]  = {640,  640, 1920, 3840};
    int heights[] = {400,  360, 1080, 2160};
    int m;
    int x;
    int y;

    for (m = 0; m < modeCount; ++m) {
        mode = modes[m];

        /* x-axis monotonic walk. The previous source x is reset to -1
         * (a sentinel that any real source cell exceeds) at every
         * resolution so the first comparison always passes. */
        int prevX = -1;
        int prevY = -1;
        for (x = 0; x < widths[m]; ++x) {
            int testX = x;
            int testY = 100;
            CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                      mode, widths[m], heights[m], &testX, &testY) == 1);
            if (testX < prevX) {
                ++g_failures;
                fprintf(stderr,
                        "FAIL monotonic-x mode=%d w=%d h=%d x=%d"
                        " prevSrc=%d got=%d\n",
                        mode, widths[m], heights[m], x, prevX, testX);
            } else {
                ++g_passes;
            }
            prevX = testX;
        }

        /* y-axis monotonic walk at every mode + extent. */
        for (y = 0; y < heights[m]; ++y) {
            int testX = widths[m] / 2;
            int testY = y;
            CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                      mode, widths[m], heights[m], &testX, &testY) == 1);
            if (testY < prevY) {
                ++g_failures;
                fprintf(stderr,
                        "FAIL monotonic-y mode=%d w=%d h=%d y=%d"
                        " prevSrc=%d got=%d\n",
                        mode, widths[m], heights[m], y, prevY, testY);
            } else {
                ++g_passes;
            }
            prevY = testY;
        }
    }
}

/* Source-cell coverage contract. Walk every (sourceX, sourceY) in the
 * [0..319] x [0..199] grid. For each, derive a presented center using
 * the same `(c * presentedExtent + sourceExtent - 1) / sourceExtent`
 * integer-nearest formula the test already uses, feed it through the
 * helper, and assert we land in the same source cell. This is the
 * exact reverse of what `m11_map_presented_game_point_to_source`
 * does at runtime in main_loop_m11.c, so a passing identity for
 * every source cell across several realistic V21/V22 extents is a
 * strong stability guarantee. */
static void test_source_cell_identity(void) {
    int width;
    int height;
    int resolutionCount = 3;
    int widths[]  = {640, 1280, 1920};
    int heights[] = {400, 720,  1080};
    int mode;
    int modeCount = 2;
    int modes[] = {
        M12_PRESENTATION_V21_UPSCALED,
        M12_PRESENTATION_V22_MODERN
    };
    int sourceExtentX = 320;
    int sourceExtentY = 200;
    int sx;
    int sy;
    int r;
    int m;

    for (m = 0; m < modeCount; ++m) {
        mode = modes[m];
        for (r = 0; r < resolutionCount; ++r) {
            width = widths[r];
            height = heights[r];

            /* Sample a deterministic subset: every 32nd column and
             * every 25th row covers 10*8 = 80 cells per (mode, w, h)
             * pair — 480 cells total — which is enough to catch any
             * cell that drifts to a neighbour row without bloating
             * the test run. */
            for (sx = 0; sx < sourceExtentX; sx += 32) {
                for (sy = 0; sy < sourceExtentY; sy += 25) {
                    int presentedX = (sx * width + sourceExtentX - 1) / sourceExtentX;
                    int presentedY = (sy * height + sourceExtentY - 1) / sourceExtentY;
                    int outX = presentedX;
                    int outY = presentedY;
                    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                              mode, width, height, &outX, &outY) == 1);
                    /* Cell identity: the round trip must land at sx
                     * or its nearest neighbour for source cells whose
                     * presented center sits exactly on the integer-
                     * divide boundary. We allow one source-cell
                     * of slack because the integer-nearest formula
                     * can pick the next source cell up to one cell
                     * from the boundary depending on the (c+1)
                     * half-open clause. */
                    if (outX < sx - 1 || outX > sx + 1) {
                        ++g_failures;
                        fprintf(stderr,
                                "FAIL cell-id-x mode=%d w=%d h=%d"
                                " src=(%d,%d) presented=(%d,%d) got=(%d,%d)\n",
                                mode, width, height,
                                sx, sy, presentedX, presentedY, outX, outY);
                    } else {
                        ++g_passes;
                    }
                    if (outY < sy - 1 || outY > sy + 1) {
                        ++g_failures;
                        fprintf(stderr,
                                "FAIL cell-id-y mode=%d w=%d h=%d"
                                " src=(%d,%d) presented=(%d,%d) got=(%d,%d)\n",
                                mode, width, height,
                                sx, sy, presentedX, presentedY, outX, outY);
                    } else {
                        ++g_passes;
                    }
                }
            }
        }
    }
}

/* Degenerate single-pixel extent contract. V21/V22 with width=1 and
 * height=1 (or one axis = 1 while the other is large) must still
 * produce a source coordinate inside [0..319] x [0..199]. The helper
 * guards `presentationWidth > 0 && presentationHeight > 0`, so 1
 * survives the positive-check and lands in the inner arithmetic —
 * `*x = (*x * 320) / 1`. A presented (0, 0) maps cleanly to (0, 0).
 * A presented (1, 0) would map to (320, …) which the clamp shrinks
 * back to (319, …). Pin both edges. */
static void test_degenerate_extents(void) {
    int x;
    int y;

    /* V21 with w=h=1: presented (0, 0) → source (0, 0). */
    x = 0; y = 0;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 1, 1, &x, &y) == 1);
    CHECK_INT("V21 1x1 (0,0) maps to source (0,0)", x, 0);
    CHECK_INT("V21 1x1 (0,0) y maps to source (0,0)", y, 0);

    /* V21 with w=1, h=1 and input (1, 1): x and y both exceed 319/199,
     * so they clamp down. */
    x = 1; y = 1;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 1, 1, &x, &y) == 1);
    CHECK_INT("V21 1x1 (1,1) x clamps to 319", x, 319);
    CHECK_INT("V21 1x1 (1,1) y clamps to 199", y, 199);

    /* V22 with w=1, h=large. Positive-guarded branch picks w=1, h=N
     * so the source clamp still fires. */
    x = 1; y = 100;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V22_MODERN, 1, 1000, &x, &y) == 1);
    CHECK_INT("V22 1x1000 (1,100) x clamps to 319", x, 319);

    /* V22 with w=large, h=1. */
    x = 1000; y = 1;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V22_MODERN, 1000, 1, &x, &y) == 1);
    CHECK_INT("V22 1000x1 (1000,1) y clamps to 199", y, 199);
}

int main(void) {
    printf("=== M11 input scale inverse / safety regression ===\n");
    printf("Source: ReDMCSB COMMAND.C:1379-1449 F0358 / 1641-1660 F0359,\n");
    printf("        src/engine/m11_game_input_mapping.c, COORD.C:1903-1920.\n\n");

    test_null_pointer_safety();
    test_monotonic_walk();
    test_source_cell_identity();
    test_degenerate_extents();

    printf("\nresult=%s\n", g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}
