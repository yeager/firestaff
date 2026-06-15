/**
 * firestaff_nexus_v2_smooth_movement_probe.c
 *
 * Nexus V2 Phase 5 — Smooth Movement and Viewport Interpolation Probe
 *
 * Headless C probe exercising the Nexus V2 smooth movement runtime binding:
 *   - Lifecycle: init, V1 tick, render frame
 *   - Smooth walk animation for N/S/E/W directions
 *   - Smooth turn animation for all 8 cardinal+diagonal directions
 *   - Smooth stairs animation with vertical offset
 *   - Deterministic input coverage: tick start, mid-tick, tick-end
 *   - Auto-detection of position/angle delta via nexus_v2_smooth_tick
 *
 * Compile (from repo root):
 *   cmake -B build -DCMAKE_BUILD_TYPE=Debug
 *   cmake --build build --target firestaff_nexus_v2_smooth_movement_probe
 *
 * Run (no game data needed):
 *   ./build/firestaff_nexus_v2_smooth_movement_probe
 *
 * Exit codes: 0 = PASS, 1 = FAIL
 *
 * Schema: firestaff.nexus_v2.smooth_movement_probe.v1
 *
 * Source: DMDF spec — camera/party world model
 *         nexus_v1_viewport.c — camera setup (nexus_camera_init)
 *         nexus_v1_movement.c — party position update
 *         ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)
 * Reference: dm2_v2_smooth_movement.c, csb_v2_smooth_movement.c
 *            dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 */

#include "nexus_v2_smooth_movement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ── Test framework ────────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        g_fail++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        g_pass++; \
    } \
} while (0)

#define PROBE_ASSERT_FLOAT_EQ(actual, expected, tolerance, fmt, ...) do { \
    float _a = (actual); float _e = (expected); \
    if (_a < _e - (tolerance) || _a > _e + (tolerance)) { \
        fprintf(stderr, "FAIL: " fmt " (got %.4f, expected %.4f +/-%.4f)\n", \
                ##__VA_ARGS__, _a, _e, (tolerance)); \
        g_fail++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        g_pass++; \
    } \
} while (0)

/* ── Lifecycle ────────────────────────────────────────────────────── */

static void test_smooth_init(void) {
    printf("--- Smooth init ---\n");

    Nexus_V2_SmoothState s;
    nexus_v2_smooth_init(&s);
    PROBE_ASSERT(!nexus_v2_smooth_is_active(&s),
                 "init: smooth inactive");
    PROBE_ASSERT(s.has_prev == 0,
                 "init: has_prev cleared (first-tick baseline unset)");

    /* Tick the first time — no animation should start (baseline record). */
    nexus_v2_smooth_tick(&s, 5.0f, 3.0f, 180.0f);
    PROBE_ASSERT(!nexus_v2_smooth_is_active(&s),
                 "first tick: no auto-animation (just records baseline)");
    PROBE_ASSERT(s.has_prev == 1, "first tick: has_prev now set");
    PROBE_ASSERT(s.prev_x == 5.0f && s.prev_y == 3.0f && s.prev_angle == 180.0f,
                 "first tick: prev_x/y/angle recorded");
}

/* ── Walk N/S/E/W coverage ────────────────────────────────────────── */

static void test_smooth_walk_nsew(void) {
    printf("--- Smooth walk N/S/E/W ---\n");

    struct {
        const char *name;
        float fx, fy, tx, ty;
    } cases[] = {
        {"North", 10.0f, 10.0f, 10.0f,  9.0f},
        {"East",  10.0f, 10.0f, 11.0f, 10.0f},
        {"South", 10.0f, 10.0f, 10.0f, 11.0f},
        {"West",  10.0f, 10.0f,  9.0f, 10.0f},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        fprintf(stderr, "  Walk %s: (%.1f,%.1f) -> (%.1f,%.1f)\n",
                cases[i].name,
                cases[i].fx, cases[i].fy,
                cases[i].tx, cases[i].ty);

        Nexus_V2_SmoothState s;
        nexus_v2_smooth_init(&s);
        /* Set a known baseline so the next move has a from-position. */
        nexus_v2_smooth_tick(&s, cases[i].fx, cases[i].fy, 0.0f);

        /* Walking 1 step should auto-start a walk from baseline to (tx,ty). */
        nexus_v2_smooth_tick(&s, cases[i].tx, cases[i].ty, 0.0f);
        PROBE_ASSERT(s.walk.active,
                     "walk %s: tick auto-starts walk from prev->current",
                     cases[i].name);

        /* Drive the animation halfway (27.5ms into a 55ms tick). */
        nexus_v2_smooth_update(&s, 27.5f);
        PROBE_ASSERT(s.walk.active,
                     "walk %s: still active at mid-tick (t=0.5)",
                     cases[i].name);

        float px = 0.0f, py = 0.0f;
        nexus_v2_smooth_get_position(&s, &px, &py);
        /* At t=0.5 with ease-out cubic: 1 - (1-0.5)^3 = 1 - 0.125 = 0.875 */
        float expected_t = 0.875f;
        float fx_step = cases[i].tx - cases[i].fx;
        float fy_step = cases[i].ty - cases[i].fy;
        PROBE_ASSERT_FLOAT_EQ(px, cases[i].fx + fx_step * expected_t, 0.01f,
                              "walk %s: X at t=0.5 ease-out-cubic", cases[i].name);
        PROBE_ASSERT_FLOAT_EQ(py, cases[i].fy + fy_step * expected_t, 0.01f,
                              "walk %s: Y at t=0.5 ease-out-cubic", cases[i].name);

        /* Drive the rest of the tick — animation should complete. */
        nexus_v2_smooth_update(&s, 27.5f);
        PROBE_ASSERT(!s.walk.active,
                     "walk %s: inactive after full V1 tick", cases[i].name);
    }
}

/* ── Turn 8-direction coverage ────────────────────────────────────── */

static void test_smooth_turn_8dir(void) {
    printf("--- Smooth turn 8 directions ---\n");

    struct {
        const char *name;
        float from, to;
    } cases[] = {
        {"NE",  0.0f,  45.0f},
        {"E",   0.0f,  90.0f},
        {"SE",  0.0f, 135.0f},
        {"S",   0.0f, 180.0f},
        {"SW",  0.0f, 225.0f},
        {"W",   0.0f, 270.0f},
        {"NW",  0.0f, 315.0f},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        fprintf(stderr, "  Turn %s: %.0f -> %.0f\n",
                cases[i].name, cases[i].from, cases[i].to);

        Nexus_V2_SmoothState s;
        nexus_v2_smooth_init(&s);
        nexus_v2_smooth_tick(&s, 0.0f, 0.0f, cases[i].from);
        nexus_v2_smooth_tick(&s, 0.0f, 0.0f, cases[i].to);
        PROBE_ASSERT(s.turn.active,
                     "turn %s: tick auto-starts turn on angle delta",
                     cases[i].name);

        /* Mid-tick angle: apply same shortest-path math the impl uses
         * (so we test that the easing lands at the expected fraction of
         * the shortest-path delta, not the raw linear midpoint). */
        float fa = cases[i].from;
        float ta = cases[i].to;
        while (fa < 0.0f)    fa += 360.0f;
        while (fa >= 360.0f) fa -= 360.0f;
        while (ta < 0.0f)    ta += 360.0f;
        while (ta >= 360.0f) ta -= 360.0f;
        float diff = ta - fa;
        if (diff >  180.0f) diff -= 360.0f;
        if (diff < -180.0f) diff += 360.0f;
        /* Ease-out quad at t=0.5: 1 - (1-0.5)^2 = 0.75 */
        float expected_mid = fa + diff * 0.75f;

        nexus_v2_smooth_update(&s, 27.5f);
        float mid = nexus_v2_smooth_get_angle(&s);
        PROBE_ASSERT_FLOAT_EQ(mid, expected_mid, 1.0f,
                              "turn %s: angle at t=0.5 ease-out-quad", cases[i].name);

        /* Full tick — animation completes. */
        nexus_v2_smooth_update(&s, 27.5f);
        PROBE_ASSERT(!s.turn.active,
                     "turn %s: inactive after full V1 tick", cases[i].name);
    }
}

/* ── Stairs vertical offset ───────────────────────────────────────── */

static void test_smooth_stairs(void) {
    printf("--- Smooth stairs with vertical offset ---\n");

    Nexus_V2_SmoothState s;
    nexus_v2_smooth_init(&s);
    nexus_v2_smooth_start_stairs(&s, 5.0f, 5.0f, 6.0f, 5.0f, 0.0f, 0.5f);

    PROBE_ASSERT(s.stairs.active, "stairs: active after start");
    PROBE_ASSERT_FLOAT_EQ(nexus_v2_smooth_get_vertical(&s), 0.0f, 0.001f,
                          "stairs: vertical at start (0.0)");

    /* Mid-tick (t=0.5): ease-in-out cubic, x=0.5, vert=0.25 */
    nexus_v2_smooth_update(&s, 27.5f);
    PROBE_ASSERT(s.stairs.active, "stairs: still active at mid-tick");

    float px = 0.0f, py = 0.0f;
    nexus_v2_smooth_get_position(&s, &px, &py);
    PROBE_ASSERT_FLOAT_EQ(px, 5.5f, 0.01f,
                          "stairs: X at t=0.5 ease-in-out-cubic");
    PROBE_ASSERT_FLOAT_EQ(py, 5.0f, 0.01f,
                          "stairs: Y at t=0.5 ease-in-out-cubic");
    PROBE_ASSERT_FLOAT_EQ(nexus_v2_smooth_get_vertical(&s), 0.25f, 0.01f,
                          "stairs: vertical at t=0.5");

    /* Full tick — animation completes. */
    nexus_v2_smooth_update(&s, 27.5f);
    PROBE_ASSERT(!s.stairs.active, "stairs: inactive after full V1 tick");
    PROBE_ASSERT_FLOAT_EQ(nexus_v2_smooth_get_vertical(&s), 0.0f, 0.001f,
                          "stairs: vertical 0 after completion");
}

/* ── Auto-detection: tick-driven walks/turns ──────────────────────── */

static void test_smooth_auto_detect(void) {
    printf("--- Auto-detection via tick ---\n");

    Nexus_V2_SmoothState s;
    nexus_v2_smooth_init(&s);

    /* Baseline. */
    nexus_v2_smooth_tick(&s, 10.0f, 10.0f, 0.0f);
    PROBE_ASSERT(!s.walk.active && !s.turn.active,
                 "auto: baseline tick starts no animation");

    /* Same position + same angle — no animation. */
    nexus_v2_smooth_tick(&s, 10.0f, 10.0f, 0.0f);
    PROBE_ASSERT(!s.walk.active && !s.turn.active,
                 "auto: no-op tick starts no animation");

    /* Position delta only — walk should start, no turn. */
    nexus_v2_smooth_tick(&s, 11.0f, 10.0f, 0.0f);
    PROBE_ASSERT(s.walk.active,
                 "auto: position delta starts walk");
    PROBE_ASSERT(!s.turn.active,
                 "auto: position-only delta does NOT start turn");

    /* Drain the walk so the next delta can be observed cleanly. */
    nexus_v2_smooth_update(&s, 55.0f);
    PROBE_ASSERT(!s.walk.active, "auto: walk drained after full tick");

    /* Angle delta only — turn should start, no walk. */
    nexus_v2_smooth_tick(&s, 11.0f, 10.0f, 90.0f);
    PROBE_ASSERT(s.turn.active,
                 "auto: angle delta starts turn");
    PROBE_ASSERT(!s.walk.active,
                 "auto: angle-only delta does NOT start walk");
}

/* ── Shortest-path turn via tick (wrap-around) ────────────────────── */

static void test_smooth_turn_shortest_path(void) {
    printf("--- Tick-driven turn shortest path ---\n");

    Nexus_V2_SmoothState s;
    nexus_v2_smooth_init(&s);
    /* Start at 350 deg, want 10 deg — shortest path is +20 deg. */
    nexus_v2_smooth_tick(&s, 0.0f, 0.0f, 350.0f);
    nexus_v2_smooth_tick(&s, 0.0f, 0.0f, 10.0f);
    PROBE_ASSERT(s.turn.active, "wrap turn: angle delta auto-starts turn");

    /* Full tick — final angle should land at the shortest-path destination.
     * Implementation returns the raw animation value (which may be 10 or
     * 10+360=370 depending on whether the shortest path was +20 or -340).
     * Either 10 or 370 is acceptable; -340 or any other value is not. */
    nexus_v2_smooth_update(&s, 55.0f);
    float final_angle = nexus_v2_smooth_get_angle(&s);
    float dist_to_10_pos = fabsf(final_angle - 10.0f);
    float dist_to_10_neg = fabsf(final_angle - 370.0f);
    PROBE_ASSERT(dist_to_10_pos < 1.0f || dist_to_10_neg < 1.0f,
                 "wrap turn: shortest-path landed near 10 deg, got %.2f",
                 final_angle);
}

/* ── Null safety ──────────────────────────────────────────────────── */

static void test_smooth_null_safety(void) {
    printf("--- Null safety ---\n");

    nexus_v2_smooth_init(NULL);
    nexus_v2_smooth_start_walk(NULL, 0, 0, 1, 1);
    nexus_v2_smooth_start_turn(NULL, 0, 90);
    nexus_v2_smooth_start_stairs(NULL, 0, 0, 1, 1, 0, 0.5f);
    nexus_v2_smooth_update(NULL, 16.0f);
    nexus_v2_smooth_tick(NULL, 1, 1, 0);

    float px = 0.0f, py = 0.0f;
    nexus_v2_smooth_get_position(NULL, &px, &py);
    (void)nexus_v2_smooth_get_vertical(NULL);
    (void)nexus_v2_smooth_get_angle(NULL);
    (void)nexus_v2_smooth_is_active(NULL);
    PROBE_ASSERT(1, "null: no crash on NULL state");
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Firestaff Nexus V2 Phase 5 Smooth Movement probe ===\n\n");

    test_smooth_init();
    test_smooth_walk_nsew();
    test_smooth_turn_8dir();
    test_smooth_stairs();
    test_smooth_auto_detect();
    test_smooth_turn_shortest_path();
    test_smooth_null_safety();

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
