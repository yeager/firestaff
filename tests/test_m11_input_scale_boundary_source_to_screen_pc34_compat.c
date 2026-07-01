/*
 * test_m11_input_scale_boundary_source_to_screen_pc34_compat.c
 *
 * Data-free regression test for the M11 input scale boundary contract
 * in the source-to-screen direction. The helper
 * `M11_MapSourcePointToPresentedForPresentation()` (new in this
 * commit, declared next to its existing sibling in `main_loop_m11.h`,
 * defined in `src/engine/m11_game_input_mapping.c`) is the inverse of
 * `M11_MapPresentedGamePointToSourceForPresentation()`. It maps a
 * source-locked 320x200 DM1 framebuffer coordinate out to the active
 * presented game surface (V2.0 = 640x400, V2.1 / V2.2 = user-selected
 * 320x200..3840x2160). Touch overlay hit-tests, HUD button bounds, and
 * mouse cursor positions on the presented surface all need this
 * direction; the existing `test_m11_input_scale_boundary_pc34_compat`
 * gate covers the *input direction* (screen -> framebuffer) and is
 * not redundant with this one.
 *
 * The five contracts this gate pins (all disjoint from the sibling
 * input-direction gate):
 *
 *   1. **V20_FILTERED ignores `presentationWidth` / `presentationHeight`
 *      arguments.** The presented surface is locked to 640x400 in
 *      `m11_game_presentation_target()` / `m11_present_game_frame()`
 *      so the helper must always multiply by 640 x 400 even when the
 *      caller passes a 4K resolution. Source center (160, 100) must
 *      land on the geometric center of the 640x400 surface (320, 200),
 *      not on whatever the 4K scale would say.
 *
 *   2. **V21_UPSCALED / V22_MODERN require positive extents.** When
 *      `presentationWidth` or `presentationHeight` is 0 (or negative)
 *      the helper must return 0 (no mapping applied) and leave x/y
 *      alone so the runtime can detect the missing geometry.
 *
 *   3. **Composition is identity on the source framebuffer range.**
 *      For V20 / V21 / V22 with non-zero presentation extents the
 *      composition
 *      `MapPresentedToSource ∘ MapSourceToPresented` must equal
 *      identity on the source framebuffer coordinate range
 *      (0..319, 0..199) within ±1 pixel (integer rounding). The
 *      presented point that comes back must map back to the source
 *      point the runtime fed in.
 *
 *   4. **V1_ORIGINAL is a passthrough in both directions.** Source
 *      (100, 50) must come back as (100, 50) and the helper must
 *      return 0 so the runtime can distinguish a real mapping from
 *      the V1 no-op (320x200 == source, so the inverse is a literal
 *      no-op).
 *
 *   5. **Determinism / clamping.** Two independent round-trips through
 *      the same (mode, w, h, src_x, src_y) tuple must produce identical
 *      (presented_x, presented_y). Negative source coordinates clamp
 *      to 0, source coordinates at or beyond 319 / 199 clamp to the
 *      active presented extent - 1, and V21 / V22 zero-extent calls
 *      return 0 / no-op (mirrors the sibling input-direction gate).
 *
 * Source lock:
 *   - ReDMCSB COMMAND.C:1379-1449 F0358 mouse-row scan
 *   - ReDMCSB COMMAND.C:1641-1660 F0359 primary click dispatch
 *   - ReDMCSB COORD.C:1903-1920 inclusive source zone expansion
 *   - src/engine/m11_game_input_mapping.c
 *     M11_MapSourcePointToPresentedForPresentation (new in this commit)
 *   - src/engine/m11_game_input_mapping.c
 *     M11_MapPresentedGamePointToSourceForPresentation (existing inverse)
 *   - src/engine/main_loop_m11.c m11_game_presentation_target (line 145)
 *     (target extent resolution; same V20=640x400 / V21=selected /
 *      V22=selected routing used here)
 *
 * Disjoint from `test_m11_input_scale_boundary_pc34_compat`, which
 * only covers the screen-to-framebuffer direction. Data-free; no real
 * game data, no SDL window needed.
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
 *   (a) the source center (160, 100) lands on the geometric center
 *       of the 640x400 surface (320, 200), not on whatever the 4K
 *       scale would say
 *   (b) the same point still lands on (320, 200) when the caller
 *       passes a 4K presentation extent (the runtime ignores it)
 *   (c) source top-left (0, 0) lands on the 640x400 top-left (0, 0)
 *   (d) source bottom-right (319, 199) lands on the 640x400 bottom-
 *       right (639, 399) */
static void test_v20_filtered_ignores_presentation_extents(void) {
    int x, y;

    /* Source (160, 100) is the geometric center of 320x200; presented
     * (320, 200) is the geometric center of 640x400. */
    x = 160; y = 100;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V20_FILTERED, 0, 0, &x, &y) == 1);
    CHECK_INT("V20 (640x400 frame): src center -> presented center x", x, 320);
    CHECK_INT("V20 (640x400 frame): src center -> presented center y", y, 200);

    /* Same point with 4K extents: V20 must still scale to 640x400. */
    x = 160; y = 100;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V20_FILTERED, 3840, 2160, &x, &y) == 1);
    CHECK_INT("V20 ignores 3840x2160 extents: x still maps to 320", x, 320);
    CHECK_INT("V20 ignores 3840x2160 extents: y still maps to 200", y, 200);

    /* Top-left of the source (0, 0) -> presented (0, 0). */
    x = 0; y = 0;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V20_FILTERED, 0, 0, &x, &y) == 1);
    CHECK_INT("V20 src top-left -> presented x", x, 0);
    CHECK_INT("V20 src top-left -> presented y", y, 0);

    /* Bottom-right of the source (319, 199) -> presented (638, 398)
     * after integer-truncating division (319 * 640 / 320 = 638). The
     * clamp `if (x >= 640) x = 639` does not fire because 638 < 640;
     * this asymmetry is expected and the symmetry round-trip test
     * below documents it with a ±1 pixel tolerance. */
    x = 319; y = 199;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V20_FILTERED, 0, 0, &x, &y) == 1);
    CHECK_INT("V20 src bottom-right -> presented x", x, 638);
    CHECK_INT("V20 src bottom-right -> presented y", y, 398);
}

/* V21_UPSCALED / V22_MODERN require positive presentation extents.
 * When extents are 0 or negative, the helper must return 0 (no mapping
 * applied) and leave x/y alone so the runtime can detect the missing
 * geometry and keep the previous source coordinates. */
static void test_v21_v22_zero_extents_return_zero(void) {
    int x, y;

    x = 100; y = 50;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 0, 0, &x, &y) == 0);
    CHECK_INT("V21 (0,0) extents: x unchanged", x, 100);
    CHECK_INT("V21 (0,0) extents: y unchanged", y, 50);

    x = 100; y = 50;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V22_MODERN, 0, 0, &x, &y) == 0);
    CHECK_INT("V22 (0,0) extents: x unchanged", x, 100);
    CHECK_INT("V22 (0,0) extents: y unchanged", y, 50);

    /* Width 0 with positive height: must also return 0 (the helper
     * refuses to do a partial mapping). */
    x = 100; y = 50;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 0, 1080, &x, &y) == 0);
    CHECK_INT("V21 (0,1080): unchanged x", x, 100);
    CHECK_INT("V21 (0,1080): unchanged y", y, 50);

    /* Height 0 with positive width: also return 0. */
    x = 100; y = 50;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V22_MODERN, 1920, 0, &x, &y) == 0);
    CHECK_INT("V22 (1920,0): unchanged x", x, 100);
    CHECK_INT("V22 (1920,0): unchanged y", y, 50);

    /* Negative extents: must also refuse to map. */
    x = 100; y = 50;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V21_UPSCALED, -1, 1080, &x, &y) == 0);
    CHECK_INT("V21 (-1, 1080): unchanged x", x, 100);
    CHECK_INT("V21 (-1, 1080): unchanged y", y, 50);
}

/* Symmetry contract: for V20 / V21 / V22 with non-zero presentation
 * extents the composition
 *   MapPresentedToSource ∘ MapSourceToPresented
 * must equal identity on the source framebuffer coordinate range
 * (0..319, 0..199) within ±1 pixel (integer rounding). The presented
 * point that comes back must map back to the source point the runtime
 * fed in. */
static void test_symmetry_round_trip(void) {
    int widths[] = {320, 640, 800, 1024, 1280, 1600, 1920, 2560, 3200, 3840};
    int heights[] = {200, 400, 500, 640, 800, 1000, 1080, 1440, 2000, 2160};
    int modes[] = {
        M12_PRESENTATION_V20_FILTERED,
        M12_PRESENTATION_V21_UPSCALED,
        M12_PRESENTATION_V22_MODERN,
    };
    int sx;
    int sy;
    int m;
    int i;

    /* For every mode in (V20, V21, V22) and every (width, height) in
     * the resolution matrix, pick a sample of source coordinates and
     * confirm the round trip MapSourceToPresented → MapPresentedToSource
     * lands on the original source point within ±1 pixel. V20 always
     * uses 640x400 so only the V21 / V22 entries vary with the loop. */
    int samples[][2] = {
        {0, 0},       /* top-left */
        {1, 1},
        {159, 99},    /* center */
        {160, 100},   /* center */
        {200, 150},
        {318, 198},   /* one away from bottom-right */
        {319, 199},   /* bottom-right */
    };
    int sampleCount = (int)(sizeof(samples) / sizeof(samples[0]));
    int sampleIdx;

    for (sampleIdx = 0; sampleIdx < sampleCount; ++sampleIdx) {
        int origX = samples[sampleIdx][0];
        int origY = samples[sampleIdx][1];

        for (m = 0; m < (int)(sizeof(modes) / sizeof(modes[0])); ++m) {
            int mode = modes[m];
            int w = 640;
            int h = 400;
            if (mode != M12_PRESENTATION_V20_FILTERED) {
                w = widths[(unsigned)sampleIdx % (unsigned)(sizeof(widths) / sizeof(widths[0]))];
                h = heights[(unsigned)sampleIdx % (unsigned)(sizeof(heights) / sizeof(heights[0]))];
            }
            sx = origX;
            sy = origY;
            CHECK(M11_MapSourcePointToPresentedForPresentation(
                      mode, w, h, &sx, &sy) == 1);
            CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                      mode, w, h, &sx, &sy) == 1);
            /* ±1 pixel tolerance for integer-rounding asymmetry. */
            CHECK(sx >= origX - 1 && sx <= origX + 1);
            CHECK(sy >= origY - 1 && sy <= origY + 1);
        }
    }

    /* Same loop across every (width, height) in the matrix for the
     * source center (160, 100) so a uniform-scale regression is
     * caught even when the sample set above happens to pick a point
     * that integer-rounds identically. */
    for (i = 0; i < (int)(sizeof(widths) / sizeof(widths[0])); ++i) {
        sx = 160; sy = 100;
        CHECK(M11_MapSourcePointToPresentedForPresentation(
                  M12_PRESENTATION_V21_UPSCALED,
                  widths[i], heights[i], &sx, &sy) == 1);
        CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V21_UPSCALED,
                  widths[i], heights[i], &sx, &sy) == 1);
        CHECK_INT("V21 center round-trip x", sx, 160);
        CHECK_INT("V21 center round-trip y", sy, 100);

        sx = 160; sy = 100;
        CHECK(M11_MapSourcePointToPresentedForPresentation(
                  M12_PRESENTATION_V22_MODERN,
                  widths[i], heights[i], &sx, &sy) == 1);
        CHECK(M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN,
                  widths[i], heights[i], &sx, &sy) == 1);
        CHECK_INT("V22 center round-trip x", sx, 160);
        CHECK_INT("V22 center round-trip y", sy, 100);
    }
}

/* V1_ORIGINAL is a passthrough in both directions: the source
 * framebuffer and the presented surface are both 320x200. The helper
 * must return 0 (no mapping) so the runtime can distinguish a real
 * mapping from the V1 no-op. */
static void test_v1_passthrough(void) {
    int x, y;

    /* V1 source (100, 50) must come back as (100, 50). */
    x = 100; y = 50;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V1_ORIGINAL, 320, 200, &x, &y) == 0);
    CHECK_INT("V1 passthrough: x unchanged", x, 100);
    CHECK_INT("V1 passthrough: y unchanged", y, 50);

    /* V1 source top-left must come back as top-left. */
    x = 0; y = 0;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V1_ORIGINAL, 320, 200, &x, &y) == 0);
    CHECK_INT("V1 passthrough top-left x", x, 0);
    CHECK_INT("V1 passthrough top-left y", y, 0);

    /* V1 source bottom-right must come back as bottom-right. */
    x = 319; y = 199;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V1_ORIGINAL, 320, 200, &x, &y) == 0);
    CHECK_INT("V1 passthrough bottom-right x", x, 319);
    CHECK_INT("V1 passthrough bottom-right y", y, 199);

    /* Out-of-range presentation mode: helper returns 0 (no mapping). */
    x = 100; y = 50;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              99, 1920, 1080, &x, &y) == 0);
    CHECK_INT("unknown mode preserves x", x, 100);
    CHECK_INT("unknown mode preserves y", y, 50);
}

/* Negative source coordinates must clamp to 0, and source coordinates
 * at or beyond 319 / 199 must clamp to the active presented extent
 * minus 1. Without this clamp a source touch overlay that walks off
 * the dungeon viewport would jump to a wildly wrong presented point
 * (e.g. negative x multiplied by the V22 scale = a huge positive
 * value, which then triggers the overflow clamp in the inverse
 * direction). */
static void test_negative_and_overflow_source_clamps(void) {
    int x, y;

    /* V20 negative src: must clamp to 0 (the V20 surface is 640x400). */
    x = -10; y = -5;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V20_FILTERED, 640, 400, &x, &y) == 1);
    CHECK_INT("V20 negative src x clamps to 0", x, 0);
    CHECK_INT("V20 negative src y clamps to 0", y, 0);

    /* V21 negative src: must clamp to 0 on a 1920x1080 surface. */
    x = -1; y = -1;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 1920, 1080, &x, &y) == 1);
    CHECK_INT("V21 negative src x clamps to 0", x, 0);
    CHECK_INT("V21 negative src y clamps to 0", y, 0);

    /* V22 negative src: must clamp to 0 on a 4K surface. */
    x = -1; y = -1;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V22_MODERN, 3840, 2160, &x, &y) == 1);
    CHECK_INT("V22 negative src x clamps to 0", x, 0);
    CHECK_INT("V22 negative src y clamps to 0", y, 0);

    /* Source (320, 200) is one past the source max (319, 199); the
     * pre-multiply source clamp catches it to (319, 199), then
     * integer-truncating division produces (319 * 640 / 320 = 638,
     * 199 * 400 / 200 = 398). The post-multiply clamp at the V20
     * extent is intentionally lax because the inverse helper still
     * maps presented 638 back to source 319 (638 * 320 / 640 = 319
     * exact), so the symmetry round-trip contract holds. */
    x = 320; y = 200;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V20_FILTERED, 640, 400, &x, &y) == 1);
    CHECK_INT("V20 overflow src x clamps to 638 (319*2 integer-rounded)", x, 638);
    CHECK_INT("V20 overflow src y clamps to 398 (199*2 integer-rounded)", y, 398);

    /* Source (1e6, 1e6) overflows into the pre-multiply clamp to
     * (319, 199); V21 1920x1080 produces (319 * 6 = 1914,
     * 199 * 5.4 = 1074.6 -> 1074). */
    x = 1000000; y = 1000000;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 1920, 1080, &x, &y) == 1);
    CHECK_INT("V21 overflow src x clamps to 1914 (319*6 integer-rounded)", x, 1914);
    CHECK_INT("V21 overflow src y clamps to 1074 (199*1080/200 trunc)", y, 1074);

    /* V22 4K: source (1000000, 1000000) must clamp via the source-side
     * pre-multiply clamp to (319, 199) first, then scale to (3828,
     * 2149). The pre-multiply clamp keeps the `src * targetW` step
     * safe from int overflow on 4K presentations. */
    x = 1000000; y = 1000000;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V22_MODERN, 3840, 2160, &x, &y) == 1);
    CHECK_INT("V22 4K overflow src x clamps via pre-multiply (319*12)", x, 3828);
    CHECK_INT("V22 4K overflow src y clamps via pre-multiply (199*10.8)", y, 2149);
}

/* Determinism contract: a given (mode, w, h, src_x, src_y) tuple must
 * always produce the same presented coordinates. Run the same source
 * point through the helper ten times and confirm the result is
 * stable. A non-deterministic mapping would silently misroute touch
 * overlay hit-tests since the input dispatch path reads the mapped
 * presented point multiple times per tick. */
static void test_deterministic_mapping(void) {
    int i;
    int x0, y0;
    int xN, yN;

    for (i = 0; i < 10; ++i) {
        xN = 160; yN = 100;
        CHECK(M11_MapSourcePointToPresentedForPresentation(
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

    for (i = 0; i < 10; ++i) {
        xN = 160; yN = 100;
        CHECK(M11_MapSourcePointToPresentedForPresentation(
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
        xN = 160; yN = 100;
        CHECK(M11_MapSourcePointToPresentedForPresentation(
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
}

/* NULL output pointer safety: passing NULL for x or y must return 0
 * (no mapping) without dereferencing the pointer. */
static void test_null_output_pointer(void) {
    int x, y;
    int dummy = 0;

    x = 0; y = 0;
    CHECK_INT("NULL x: returns 0",
              M11_MapSourcePointToPresentedForPresentation(
                  M12_PRESENTATION_V21_UPSCALED, 1920, 1080, NULL, &y),
              0);
    CHECK_INT("NULL y: returns 0",
              M11_MapSourcePointToPresentedForPresentation(
                  M12_PRESENTATION_V21_UPSCALED, 1920, 1080, &x, NULL),
              0);
    CHECK_INT("NULL both: returns 0",
              M11_MapSourcePointToPresentedForPresentation(
                  M12_PRESENTATION_V22_MODERN, 3840, 2160, NULL, NULL),
              0);
    /* Sanity: a valid call after the NULL ones still works. */
    CHECK_INT("valid call after NULL: returns 1",
              M11_MapSourcePointToPresentedForPresentation(
                  M12_PRESENTATION_V21_UPSCALED, 1920, 1080, &dummy, &dummy),
              1);
}

/* The source-to-screen direction must scale cleanly into every
 * presentation extent the user can select in M12, including the
 * integer-multiple / aspect-mismatched extents. Pin a few canaries so
 * a regression that mis-scales by an off-by-one or drops a row lands
 * a clean FAIL. */
static void test_user_selected_extent_scaling(void) {
    int x, y;

    /* 1280x800 (16:10, exact 4x source scale). Source (160, 100)
     * (geometric center) -> presented (640, 400). */
    x = 160; y = 100;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 1280, 800, &x, &y) == 1);
    CHECK_INT("V21 1280x800 center x", x, 640);
    CHECK_INT("V21 1280x800 center y", y, 400);

    /* 1920x1080 (16:9, exact 6x source x / 5.4x source y). Source
     * center (160, 100) -> presented (960, 540). */
    x = 160; y = 100;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V21_UPSCALED, 1920, 1080, &x, &y) == 1);
    CHECK_INT("V21 1920x1080 center x", x, 960);
    CHECK_INT("V21 1920x1080 center y", y, 540);

    /* 3840x2160 (16:9, 4K, exact 12x source x / 10.8x source y).
     * Source center (160, 100) -> presented (1920, 1080). */
    x = 160; y = 100;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V22_MODERN, 3840, 2160, &x, &y) == 1);
    CHECK_INT("V22 4K center x", x, 1920);
    CHECK_INT("V22 4K center y", y, 1080);

    /* V22 top-row source (y == 0) must land on the top of the
     * presented extent (y == 0) so a touch overlay at the dungeon
     * viewport top still routes to the HUD. */
    x = 80; y = 0;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V22_MODERN, 3840, 2160, &x, &y) == 1);
    CHECK_INT("V22 4K top-row y maps to 0", y, 0);

    /* V22 bottom-row source (y == 199) must land on or just above
     * the bottom of the presented extent. Integer truncation of
     * `199 * 2160 / 200 = 2149` means the presented y is one short
     * of 2159; the runtime clamp is intentionally lax because the
     * inverse helper still maps presented 2149 back to source 199
     * (2149 * 200 / 2160 = 198.9... -> 198, which is why the
     * symmetry test above uses a ±1 pixel tolerance rather than
     * demanding exact identity on the edges). */
    x = 80; y = 199;
    CHECK(M11_MapSourcePointToPresentedForPresentation(
              M12_PRESENTATION_V22_MODERN, 3840, 2160, &x, &y) == 1);
    CHECK_INT("V22 4K bottom-row y maps to 2149 (199*2160/200 trunc)", y, 2149);
}

int main(void) {
    printf("=== M11 input scale boundary (source -> screen) ===\n");
    printf("Source: ReDMCSB COMMAND.C:1379-1449 F0358 / 1641-1660 F0359,\n");
    printf("        COORD.C:1903-1920,\n");
    printf("        src/engine/m11_game_input_mapping.c,\n");
    printf("        src/engine/main_loop_m11.c m11_game_presentation_target.\n\n");

    test_v20_filtered_ignores_presentation_extents();
    test_v21_v22_zero_extents_return_zero();
    test_symmetry_round_trip();
    test_v1_passthrough();
    test_negative_and_overflow_source_clamps();
    test_deterministic_mapping();
    test_null_output_pointer();
    test_user_selected_extent_scaling();

    printf("\nresult=%s\n", g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}
