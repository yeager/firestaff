/*
 * test_m11_input_scale_boundary_pc34_compat.c
 *
 * Data-free regression test for the M11 input scale boundary contract
 * in `src/engine/m11_game_input_mapping.c`. The helper
 * `M11_MapPresentedGamePointToSourceForPresentation()` is the single
 * source of truth that converts an SDL mouse/window point on the
 * *presented* game surface back to the source-locked 320x200 DM1
 * framebuffer. The ReDMCSB layout-696 / COMMAND.C:1379-1449 / 1641-1660
 * route requires the source coordinates to be in [0..319] x [0..199]
 * regardless of the active presentation mode (V1 original 320x200,
 * V2.0 filtered 640x400, V2.1 upscaled, V2.2 modern at any user-
 * selected 320x200..3840x2160 resolution).
 *
 * The existing `test_dm1_v2_selected_resolution_input_mapping_pc34`
 * covers the V21/V22 happy-path round-trip and the V1 mode no-op
 * behavior. This focused gate closes the boundary cases that
 * production runtime callers actually hit but were not yet pinned:
 *
 *   1. V20_FILTERED ignores `presentationWidth` / `presentationHeight`
 *      arguments (the presentation target is locked to 640x400 in
 *      `m11_game_presentation_target()` and `m11_present_game_frame()`),
 *      so the helper must always divide by 640 x 400 even when the
 *      caller passes a 4K resolution.
 *   2. V21_UPSCALED / V22_MODERN require `presentationWidth > 0` AND
 *      `presentationHeight > 0`; passing 0 (or a negative value)
 *      must return 0 (no mapping applied) so the runtime can keep
 *      the previous source coordinates instead of jumping to (0,0).
 *   3. Negative input coordinates must clamp to 0, not wrap to a
 *      high source coordinate.
 *   4. Input coordinates at or beyond the presented extent must clamp
 *      to the source extent - 1 (319, 199).
 *   5. The first row (y == 0) of every presented extent must round-
 *      trip back to y == 0, and the last row back to y == 199 — so a
 *      user clicking at the window top / bottom still routes to the
 *      correct top/bottom edge of the dungeon viewport.
 *   6. Aspect-mismatched resolutions (e.g. 1920x1080 for a 320x200
 *      source) must NOT map the same presented point to a different
 *      source row each time the mode changes; the result depends
 *      only on the (presentationMode, presentationWidth, presentationHeight,
 *      x, y) tuple.
 *   7. The helper must report `1` (success) for every V20_FILTERED /
 *      V21_UPSCALED / V22_MODERN call with non-zero extents so the
 *      caller can distinguish a real mapping from a no-op pass-through
 *      that V1_ORIGINAL returns.
 *
 * Source lock:
 *   - ReDMCSB COMMAND.C:1379-1449 F0358 mouse-row scan
 *   - ReDMCSB COMMAND.C:1641-1660 F0359 primary click dispatch
 *   - ReDMCSB COORD.C:1903-1920 inclusive source zone expansion
 *   - src/engine/m11_game_input_mapping.c M11_MapPresentedGamePointToSourceForPresentation
 *   - src/engine/main_loop_m11.c m11_game_presentation_target (line 145)
 *   - src/engine/main_loop_m11.c m11_present_game_frame (line 195)
 */

#include "main_loop_m11.h"

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

/* V20_FILTERED is locked to 640x400 regardless of caller-supplied
 * presentationWidth / presentationHeight. Verify that:
 *   (a) a V20 call still succeeds when the caller passes a 4K
 *       resolution (the runtime ignores it)
 *   (b) the mapping divides by 640 x 400 specifically, so a window
 *       click at (320, 200) — the geometric center of the 640x400
 *       surface — rounds to source (160, 100), not to whatever the
 *       4K scale would say. */
static void test_v20_filtered_ignores_presentation_extents(void) {
    int x, y;

    /* Geometric center of 640x400 → source (160, 100). */
    x = 320; y = 200;
    CHECK_INT("V20 (640x400 frame): center x", x, 320);
    CHECK_INT("V20 (640x400 frame): center y", y, 200);
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V20_FILTERED, 0, 0, &x, &y) == 1);
    CHECK_INT("V20 ignores 0,0 extents: x maps to 160", x, 160);
    CHECK_INT("V20 ignores 0,0 extents: y maps to 100", y, 100);

    /* Same point with 4K extents: V20 must still divide by 640 x 400. */
    x = 320; y = 200;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V20_FILTERED, 3840, 2160, &x, &y) == 1);
    CHECK_INT("V20 ignores 3840x2160 extents: x still maps to 160", x, 160);
    CHECK_INT("V20 ignores 3840x2160 extents: y still maps to 100", y, 100);

    /* Lower-right of the 640x400 surface (639, 399) → source (319, 199). */
    x = 639; y = 399;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V20_FILTERED, 3840, 2160, &x, &y) == 1);
    CHECK_INT("V20 lower-right x", x, 319);
    CHECK_INT("V20 lower-right y", y, 199);

    /* Top-left of 640x400 (0, 0) → source (0, 0). */
    x = 0; y = 0;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V20_FILTERED, 0, 0, &x, &y) == 1);
    CHECK_INT("V20 top-left x", x, 0);
    CHECK_INT("V20 top-left y", y, 0);
}

/* V21_UPSCALED / V22_MODERN require positive presentation extents.
 * When extents are 0 or negative, the helper must return 0 (no mapping
 * applied) and leave x/y alone so the runtime can detect the missing
 * geometry and keep the previous source coordinates. */
static void test_v21_v22_zero_extents_return_zero(void) {
    int x, y;

    x = 100; y = 50;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 0, 0, &x, &y) == 0);
    CHECK_INT("V21 (0,0) extents: x unchanged", x, 100);
    CHECK_INT("V21 (0,0) extents: y unchanged", y, 50);

    x = 100; y = 50;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V22_MODERN, 0, 0, &x, &y) == 0);
    CHECK_INT("V22 (0,0) extents: x unchanged", x, 100);
    CHECK_INT("V22 (0,0) extents: y unchanged", y, 50);

    /* Width 0 with positive height: must also return 0 (the helper
     * refuses to do a partial mapping). */
    x = 100; y = 50;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 0, 1080, &x, &y) == 0);
    CHECK_INT("V21 (0,1080): unchanged x", x, 100);
    CHECK_INT("V21 (0,1080): unchanged y", y, 50);

    /* Height 0 with positive width: also return 0. */
    x = 100; y = 50;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V22_MODERN, 1920, 0, &x, &y) == 0);
    CHECK_INT("V22 (1920,0): unchanged x", x, 100);
    CHECK_INT("V22 (1920,0): unchanged y", y, 50);

    /* Negative extents: must also refuse to map. The helper does not
     * claim to sanitize bad inputs — it surfaces the missing geometry
     * to the caller via a 0 return value. */
    x = 100; y = 50;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED, -1, 1080, &x, &y) == 0);
    CHECK_INT("V21 (-1, 1080): unchanged x", x, 100);
    CHECK_INT("V21 (-1, 1080): unchanged y", y, 50);
}

/* Negative input coordinates must clamp to 0, not wrap to a high
 * source coordinate. A negative pointer delta in SDL can land here
 * if the window is resized while the mouse button is held down. */
static void test_negative_input_clamps_to_zero(void) {
    int x, y;

    /* V20 with negative (x, y): must clamp to (0, 0). */
    x = -10; y = -5;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V20_FILTERED, 640, 400, &x, &y) == 1);
    CHECK_INT("V20 negative x clamps to 0", x, 0);
    CHECK_INT("V20 negative y clamps to 0", y, 0);

    /* V21 / V22 with negative (x, y): same clamp. */
    x = -1; y = -1;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 1920, 1080, &x, &y) == 1);
    CHECK_INT("V21 negative x clamps to 0", x, 0);
    CHECK_INT("V21 negative y clamps to 0", y, 0);

    x = -1; y = -1;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V22_MODERN, 1920, 1080, &x, &y) == 1);
    CHECK_INT("V22 negative x clamps to 0", x, 0);
    CHECK_INT("V22 negative y clamps to 0", y, 0);
}

/* Input coordinates at or beyond the presented extent must clamp
 * to source (319, 199). Without this clamp a window-edge click
 * would overshoot the dungeon viewport and F0638_GetZone would
 * return a sentinel row, dropping the command. */
static void test_overflow_input_clamps_to_source_max(void) {
    int x, y;

    /* V20 surface is 640x400: (640, 400) is one past the last pixel. */
    x = 640; y = 400;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V20_FILTERED, 640, 400, &x, &y) == 1);
    CHECK_INT("V20 x at 640 clamps to 319", x, 319);
    CHECK_INT("V20 y at 400 clamps to 199", y, 199);

    /* Massive overflow on V21: (1e6, 1e6) → still 319, 199. */
    x = 1000000; y = 1000000;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 1920, 1080, &x, &y) == 1);
    CHECK_INT("V21 overflow x clamps to 319", x, 319);
    CHECK_INT("V21 overflow y clamps to 199", y, 199);

    /* V22 4K surface: (3840, 2160) is one past the last pixel. */
    x = 3840; y = 2160;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V22_MODERN, 3840, 2160, &x, &y) == 1);
    CHECK_INT("V22 4K x at 3840 clamps to 319", x, 319);
    CHECK_INT("V22 4K y at 2160 clamps to 199", y, 199);
}

/* Top-row (y == 0) of every presented extent must round-trip back to
 * y == 0; bottom-row (y == presentedH - 1) back to y == 199. This
 * pins the V20_FILTERED = 640x400 / V21 = user-selected / V22 = user-
 * selected extents, so a mouse at the window top/bottom is always
 * mapped to the dungeon viewport top/bottom regardless of mode. */
static void test_top_bottom_round_trip(void) {
    int x, y;
    int widths[] = {320, 640, 800, 1024, 1280, 1600, 1920, 2560, 3200, 3840};
    int heights[] = {200, 400, 500, 640, 800, 1000, 1080, 1440, 2000, 2160};
    int i;

    /* Top row maps to 0 in every (width, height) pair, every mode. */
    for (i = 0; i < (int)(sizeof(widths) / sizeof(widths[0])); ++i) {
        x = widths[i] / 2;
        y = 0;
        CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V21_UPSCALED,
                  widths[i], heights[i], &x, &y) == 1);
        CHECK_INT("V21 top-row y maps to 0 (mode-resolved)", y, 0);
        x = widths[i] / 2;
        y = 0;
        CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN,
                  widths[i], heights[i], &x, &y) == 1);
        CHECK_INT("V22 top-row y maps to 0 (mode-resolved)", y, 0);

        /* Bottom row maps to 199. */
        x = widths[i] / 2;
        y = heights[i] - 1;
        CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V21_UPSCALED,
                  widths[i], heights[i], &x, &y) == 1);
        CHECK_INT("V21 bottom-row y maps to 199", y, 199);
        x = widths[i] / 2;
        y = heights[i] - 1;
        CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN,
                  widths[i], heights[i], &x, &y) == 1);
        CHECK_INT("V22 bottom-row y maps to 199", y, 199);
    }

    /* V20 always uses 640 x 400 regardless of the caller-supplied
     * extents, so its top/bottom row is independent of the loop. */
    x = 320; y = 0;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V20_FILTERED, 1920, 1080, &x, &y) == 1);
    CHECK_INT("V20 top-row y maps to 0 (640x400 frame)", y, 0);
    x = 320; y = 399;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V20_FILTERED, 1920, 1080, &x, &y) == 1);
    CHECK_INT("V20 bottom-row y maps to 199 (640x400 frame)", y, 199);
}

/* Determinism contract: a given (mode, w, h, x, y) tuple must always
 * produce the same source coordinates. Run the same point through the
 * helper ten times and confirm the result is stable. A non-deterministic
 * mapping would silently misroute command queues, since the input
 * dispatch path reads the mapped source point multiple times per tick. */
static void test_deterministic_mapping(void) {
    int i;
    int x0, y0;
    int xN, yN;

    for (i = 0; i < 10; ++i) {
        xN = 1920; yN = 1080;
        CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V21_UPSCALED,
                  1920, 1080, &xN, &yN) == 1);
        if (i == 0) {
            x0 = xN;
            y0 = yN;
        } else {
            CHECK_INT("V21 deterministic x", xN, x0);
            CHECK_INT("V21 deterministic y", yN, y0);
        }
    }

    for (i = 0; i < 10; ++i) {
        xN = 3840; yN = 2160;
        CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN,
                  3840, 2160, &xN, &yN) == 1);
        if (i == 0) {
            x0 = xN;
            y0 = yN;
        } else {
            CHECK_INT("V22 deterministic x", xN, x0);
            CHECK_INT("V22 deterministic y", yN, y0);
        }
    }

    for (i = 0; i < 10; ++i) {
        xN = 640; yN = 400;
        CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V20_FILTERED,
                  640, 400, &xN, &yN) == 1);
        if (i == 0) {
            x0 = xN;
            y0 = yN;
        } else {
            CHECK_INT("V20 deterministic x", xN, x0);
            CHECK_INT("V20 deterministic y", yN, y0);
        }
    }
}

/* Return-value matrix: the helper must return 1 for every
 * V20_FILTERED / V21_UPSCALED / V22_MODERN call with positive extents,
 * and 0 for V1_ORIGINAL. Without this distinction the runtime cannot
 * tell a real mapping from a no-op pass-through. */
static void test_return_value_matrix(void) {
    int x, y;

    x = 0; y = 0;
    CHECK_INT("V20 returns 1",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V20_FILTERED, 640, 400, &x, &y),
              1);

    x = 0; y = 0;
    CHECK_INT("V21 returns 1",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V21_UPSCALED, 1920, 1080, &x, &y),
              1);

    x = 0; y = 0;
    CHECK_INT("V22 returns 1",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN, 1920, 1080, &x, &y),
              1);

    x = 0; y = 0;
    CHECK_INT("V1 returns 0",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V1_ORIGINAL, 320, 200, &x, &y),
              0);

    /* Out-of-range presentation mode: helper returns 0 (no mapping). */
    x = 100; y = 50;
    CHECK_INT("unknown mode returns 0",
              M11_MapPresentedGamePointToSourceForPresentation(
                  99, 1920, 1080, &x, &y),
              0);
    CHECK_INT("unknown mode preserves x", x, 100);
    CHECK_INT("unknown mode preserves y", y, 50);
}

int main(void) {
    printf("=== M11 input scale boundary regression ===\n");
    printf("Source: ReDMCSB COMMAND.C:1379-1449 F0358 / 1641-1660 F0359,\n");
    printf("        src/engine/m11_game_input_mapping.c, COORD.C:1903-1920.\n\n");

    test_v20_filtered_ignores_presentation_extents();
    test_v21_v22_zero_extents_return_zero();
    test_negative_input_clamps_to_zero();
    test_overflow_input_clamps_to_source_max();
    test_top_bottom_round_trip();
    test_deterministic_mapping();
    test_return_value_matrix();

    printf("\nresult=%s\n", g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}
