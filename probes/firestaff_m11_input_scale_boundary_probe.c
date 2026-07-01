/*
 * firestaff_m11_input_scale_boundary_probe.c
 *
 * Headless probe that complements
 * tests/test_m11_input_scale_boundary_pc34_compat.c with the
 * four production-runtime scenarios that the CTest unit test
 * deliberately leaves out so it stays focused on the
 * return-value + boundary-clamp matrix:
 *
 *   1. V21 / V22 identity mapping at the source resolution
 *      (presentationWidth == 320, presentationHeight == 200).
 *      This is the case the M11 launcher→game handoff hits when
 *      the user picks the smallest user-selectable resolution in
 *      V2.2 modern mode. The helper must short-circuit to the
 *      input (no rounding error), and the source must NOT shift
 *      a click on (160, 100) anywhere off-center.
 *
 *   2. V21 exact-2x equivalence: with (640, 400) extents V21 must
 *      produce the same source (x, y) as V20 (which is locked to
 *      640x400). Without this guarantee a launcher passing the
 *      same 640x400 extent for either mode would silently
 *      re-route the same pointer click to a different dungeon
 *      sensor, since F0358 / F0359 dispatch reads the source
 *      point twice per click.
 *
 *   3. 8K saturation: V22 with (7680, 4320) — the upper end of
 *      the user-selectable V2.2 modern resolution matrix.
 *      Integer-truncation at the 24x / 21.6x scale must still
 *      land the geometric center (3840, 2160) on the source
 *      center (160, 100) ± 1. Without this guarantee a user
 *      running at 8K would have the dungeon viewport offset
 *      from the cursor by a full source column.
 *
 *   4. Sequential re-entry contract: three different mapping
 *      calls in a row (V22 4K → V20 640x400 → V21 1080p) must
 *      each leave the (x, y) the previous call wrote. The
 *      helper is stateless, but a future refactor that
 *      introduces an internal scratch buffer would silently
 *      corrupt callers that pass the same pointer twice. This
 *      probe catches that regression before it lands.
 *
 * Source lock:
 *   - ReDMCSB COMMAND.C:1379-1449 F0358 mouse-row scan
 *   - ReDMCSB COMMAND.C:1641-1660 F0359 primary click dispatch
 *   - ReDMCSB COORD.C:1903-1920 inclusive source zone expansion
 *   - src/engine/m11_game_input_mapping.c
 *     M11_MapPresentedGamePointToSourceForPresentation
 *   - include/menu_startup_m12.h M12_PresentationMode enum
 *
 * Data-free: no game data, no SDL window, no game assets.
 * Skip-safe: produces a stable PASS / FAIL summary on every host.
 */

#include "main_loop_m11.h"

#include <stdio.h>

static int g_total = 0;
static int g_failed = 0;

static void check_int(const char* label, int got, int expected) {
    ++g_total;
    if (got != expected) {
        ++g_failed;
        fprintf(stderr, "FAIL: %s got=%d expected=%d\n", label, got, expected);
    } else {
        printf("PASS: %s = %d\n", label, got);
    }
}

static void check_true(const char* label, int condition) {
    ++g_total;
    if (!condition) {
        ++g_failed;
        fprintf(stderr, "FAIL: %s\n", label);
    } else {
        printf("PASS: %s\n", label);
    }
}

/* V21 / V22 at source resolution: a click at (160, 100) on a
 * 320x200 presented surface must round-trip to source (160, 100)
 * with no rounding error. The launcher→M11 handoff passes
 * (320, 200) as the fallback when the user keeps the V2.2 modern
 * mode at its smallest user-selectable resolution. */
static void probe_v22_identity_mapping_at_source_resolution(void) {
    int x, y;

    /* V21: (160, 100) at (320, 200) → (160, 100). */
    x = 160; y = 100;
    check_int("V21 identity return value",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V21_UPSCALED, 320, 200, &x, &y),
              1);
    check_int("V21 identity x at center", x, 160);
    check_int("V21 identity y at center", y, 100);

    /* V22: (160, 100) at (320, 200) → (160, 100). */
    x = 160; y = 100;
    check_int("V22 identity return value",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN, 320, 200, &x, &y),
              1);
    check_int("V22 identity x at center", x, 160);
    check_int("V22 identity y at center", y, 100);

    /* V22: top-left (0, 0) → (0, 0). */
    x = 0; y = 0;
    check_int("V22 identity top-left x",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN, 320, 200, &x, &y) == 1 ? x : -1,
              0);
    check_int("V22 identity top-left y",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN, 320, 200, &x, &y) == 1 ? y : -1,
              0);

    /* V22: bottom-right (319, 199) → (319, 199). */
    x = 319; y = 199;
    (void)M11_MapPresentedGamePointToSourceForPresentation(
        M12_PRESENTATION_V22_MODERN, 320, 200, &x, &y);
    check_int("V22 identity bottom-right x", x, 319);
    check_int("V22 identity bottom-right y", y, 199);
}

/* V21 at the exact 2x extent (640, 400) must produce the same
 * source (x, y) as V20 (which is locked to 640x400). The CTest
 * unit test covers each mode in isolation; this probe covers
 * the cross-mode equivalence the runtime depends on when the
 * launcher passes the same 640x400 extent for either mode. */
static void probe_v21_v20_exact_2x_equivalence(void) {
    int xV20, yV20;
    int xV21, yV21;
    int xV22, yV22;

    /* Sample 9 representative points across the 640x400 surface
     * (corners, edge midpoints, and the geometric center). The
     * helper uses independent integer-division on x and y, so
     * these 9 points exercise every per-axis rounding corner. */
    struct { int px, py; } points[] = {
        {  0,   0},
        {319,   0},
        {639,   0},
        {  0, 199},
        {320, 200},
        {639, 199},
        {  0, 399},
        {320, 399},
        {639, 399},
    };
    int i;
    int n = (int)(sizeof(points) / sizeof(points[0]));

    for (i = 0; i < n; ++i) {
        xV20 = points[i].px; yV20 = points[i].py;
        xV21 = points[i].px; yV21 = points[i].py;
        xV22 = points[i].px; yV22 = points[i].py;

        (void)M11_MapPresentedGamePointToSourceForPresentation(
            M12_PRESENTATION_V20_FILTERED, 0, 0, &xV20, &yV20);
        (void)M11_MapPresentedGamePointToSourceForPresentation(
            M12_PRESENTATION_V21_UPSCALED, 640, 400, &xV21, &yV21);
        (void)M11_MapPresentedGamePointToSourceForPresentation(
            M12_PRESENTATION_V22_MODERN, 640, 400, &xV22, &yV22);

        check_true("V21 == V20 x at (px, py) point", xV21 == xV20);
        check_true("V21 == V20 y at (px, py) point", yV21 == yV20);
        check_true("V22 == V20 x at (px, py) point", xV22 == xV20);
        check_true("V22 == V20 y at (px, py) point", yV22 == yV20);
    }

    /* The geometric center (320, 200) is the canonical
     * per-axis-rounding boundary: 320/640 = 0.5 → 160,
     * 200/400 = 0.5 → 100. Pin the absolute values explicitly
     * so a future integer-math refactor (truncation vs
     * round-to-nearest) cannot drift without notice. */
    xV20 = 320; yV20 = 200;
    xV21 = 320; yV21 = 200;
    (void)M11_MapPresentedGamePointToSourceForPresentation(
        M12_PRESENTATION_V20_FILTERED, 0, 0, &xV20, &yV20);
    (void)M11_MapPresentedGamePointToSourceForPresentation(
        M12_PRESENTATION_V21_UPSCALED, 640, 400, &xV21, &yV21);
    check_int("V20 center x at exact 2x", xV20, 160);
    check_int("V20 center y at exact 2x", yV20, 100);
    check_int("V21 center x at exact 2x", xV21, 160);
    check_int("V21 center y at exact 2x", yV21, 100);
}

/* 8K saturation: V22 with (7680, 4320) — the upper end of the
 * user-selectable V2.2 modern resolution matrix. Integer
 * truncation at the 24x / 21.6x scale must still land the
 * geometric center on the source center. */
static void probe_v22_8k_saturation(void) {
    int x, y;

    /* Center (3840, 2160) at (7680, 4320). With integer
     * division: 3840 * 320 / 7680 = 160, 2160 * 200 / 4320 =
     * 100. The runtime path uses this mapping when the user
     * runs at 8K and clicks the window center, so a ±1 source
     * offset would silently mis-route F0358 / F0359 dispatches. */
    x = 3840; y = 2160;
    check_int("V22 8K center return value",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V22_MODERN, 7680, 4320, &x, &y),
              1);
    check_int("V22 8K center x", x, 160);
    check_int("V22 8K center y", y, 100);

    /* Top-left (0, 0) at 8K. */
    x = 0; y = 0;
    (void)M11_MapPresentedGamePointToSourceForPresentation(
        M12_PRESENTATION_V22_MODERN, 7680, 4320, &x, &y);
    check_int("V22 8K top-left x", x, 0);
    check_int("V22 8K top-left y", y, 0);

    /* Just past the bottom-right (7680, 4320) at 8K — must
     * clamp to (319, 199), not wrap or overflow. */
    x = 8000; y = 4500;
    (void)M11_MapPresentedGamePointToSourceForPresentation(
        M12_PRESENTATION_V22_MODERN, 7680, 4320, &x, &y);
    check_int("V22 8K overflow x clamps to 319", x, 319);
    check_int("V22 8K overflow y clamps to 199", y, 199);

    /* Quarter-point (1920, 1080) at 8K — exercises 1/4 of the
     * presented extent on each axis. With integer division:
     * 1920 * 320 / 7680 = 80, 1080 * 200 / 4320 = 50. */
    x = 1920; y = 1080;
    (void)M11_MapPresentedGamePointToSourceForPresentation(
        M12_PRESENTATION_V22_MODERN, 7680, 4320, &x, &y);
    check_int("V22 8K quarter-x", x, 80);
    check_int("V22 8K quarter-y", y, 50);
}

/* Sequential re-entry contract: three different mapping calls
 * in a row (V22 4K → V20 640x400 → V21 1080p) must each leave
 * the (x, y) the previous call wrote, and must each report the
 * correct source point. The helper is documented stateless, but
 * a future refactor that introduces an internal scratch buffer
 * would silently corrupt callers that pass the same pointer
 * twice. This probe catches that regression before it lands. */
static void probe_sequential_reentry(void) {
    int x, y;

    /* First call: V22 4K, geometric center → (160, 100). */
    x = 1920; y = 1080;
    (void)M11_MapPresentedGamePointToSourceForPresentation(
        M12_PRESENTATION_V22_MODERN, 3840, 2160, &x, &y);
    check_int("sequential V22 4K x", x, 160);
    check_int("sequential V22 4K y", y, 100);

    /* Second call: same pointer, V20 640x400, top-left →
     * (0, 0). The previous (160, 100) must NOT survive. */
    x = 0; y = 0;
    (void)M11_MapPresentedGamePointToSourceForPresentation(
        M12_PRESENTATION_V20_FILTERED, 640, 400, &x, &y);
    check_int("sequential V20 640x400 x", x, 0);
    check_int("sequential V20 640x400 y", y, 0);

    /* Third call: same pointer, V21 1080p, bottom-right →
     * (319, 199). */
    x = 1919; y = 1079;
    (void)M11_MapPresentedGamePointToSourceForPresentation(
        M12_PRESENTATION_V21_UPSCALED, 1920, 1080, &x, &y);
    check_int("sequential V21 1080p x", x, 319);
    check_int("sequential V21 1080p y", y, 199);

    /* Fourth call: V1 short-circuit. The helper returns 0
     * and must NOT touch (x, y). Caller semantics depend on
     * x / y surviving untouched. */
    x = 42; y = 7;
    check_int("sequential V1 return value",
              M11_MapPresentedGamePointToSourceForPresentation(
                  M12_PRESENTATION_V1_ORIGINAL, 320, 200, &x, &y),
              0);
    check_int("sequential V1 x preserved", x, 42);
    check_int("sequential V1 y preserved", y, 7);

    /* Fifth call: same pointer, V22 4K negative input →
     * clamps to (0, 0). Previous V1 values must NOT survive. */
    x = -50; y = -50;
    (void)M11_MapPresentedGamePointToSourceForPresentation(
        M12_PRESENTATION_V22_MODERN, 3840, 2160, &x, &y);
    check_int("sequential V22 4K negative x clamps", x, 0);
    check_int("sequential V22 4K negative y clamps", y, 0);
}

int main(void) {
    printf("=== M11 input scale boundary probe (headless) ===\n");
    printf("Source: ReDMCSB COMMAND.C:1379-1449 F0358 / 1641-1660 F0359,\n");
    printf("        src/engine/m11_game_input_mapping.c, COORD.C:1903-1920.\n\n");
    printf("Companion to tests/test_m11_input_scale_boundary_pc34_compat.c.\n");
    printf("Pins identity mapping at source resolution, V21 <-> V20\n");
    printf("exact-2x equivalence, V22 8K saturation, and the\n");
    printf("sequential re-entry contract.\n\n");

    probe_v22_identity_mapping_at_source_resolution();
    probe_v21_v20_exact_2x_equivalence();
    probe_v22_8k_saturation();
    probe_sequential_reentry();

    printf("\nsummary: total=%d failed=%d\n", g_total, g_failed);
    if (g_failed) {
        printf("result: FAIL\n");
        return 1;
    }
    printf("result: PASS\n");
    return 0;
}
