/**
 * firestaff_csb_v2_smooth_movement_probe.c
 *
 * CSB V2 Phase 5 — Smooth Movement and Viewport Interpolation Probe
 *
 * Headless C probe exercising CSB V2 smooth movement runtime binding:
 *   - Lifecycle: init, V1 tick, render frame
 *   - Walk animation: N/S/E/W directions
 *   - Turn animation: all 8 cardinal+diagonal directions
 *   - Stairs animation with vertical offset
 *   - Deterministic input coverage: tick-start, mid-tick, tick-end
 *   - Source evidence strings
 *
 * Compile (from repo root):
 *   cmake -B build -DCMAKE_BUILD_TYPE=Debug
 *   cmake --build build --target firestaff_csb_v2_smooth_movement_probe
 *
 * Run (no game data needed):
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_csb_v2_smooth_movement_probe
 *
 * Exit codes: 0 = PASS, 1 = FAIL
 *
 * Schema: firestaff.csb_v2.smooth_movement_probe.v1
 *
 * Source: ReDMCSB COMMAND.C F0380, CLIKMENU.C F0365/F0366, GAMELOOP.C
 *         CSBWin/Viewport.cpp: base rendering
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *   v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync
 *   v22_smooth_update_from_clock / v22_smooth_get_x/y/angle
 */

#include "csb_v2_smooth_movement.h"
#include "csb_v2_viewport_renderer.h"
#include "dm1_v2_anim_timing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Test framework ─────────────────────────────────────────────── */

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

#define PROBE_ASSERT_FLOAT_EQ(actual, expected, tol, fmt, ...) do { \
    float _a = (actual), _e = (expected); \
    if (_a < _e - (tol) || _a > _e + (tol)) { \
        fprintf(stderr, "FAIL: " fmt " (got %.4f, expected %.4f ±%.4f)\n", \
                ##__VA_ARGS__, _a, _e, (float)(tol)); \
        g_fail++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        g_pass++; \
    } \
} while (0)

/* ── Lifecycle tests ─────────────────────────────────────────────── */

static void test_smooth_init(void) {
    printf("--- Smooth init ---\n");

    CSB_V2_ViewportState vp;
    csb_v2_viewport_init(&vp, 2);

    /* V1 tick sequence does not crash */
    csb_v2_viewport_v1_tick(&vp, 0);
    csb_v2_viewport_v1_tick(&vp, 55000);
    PROBE_ASSERT(1, "V1 tick sequence does not crash");

    /* Render frame does not crash */
    csb_v2_viewport_render_frame(&vp, 55000);
    PROBE_ASSERT(1, "render_frame does not crash");

    /* Source evidence strings are non-trivial */
    const char *ev = csb_v2_smooth_source_evidence();
    PROBE_ASSERT(ev != NULL && strlen(ev) > 10,
                 "csb_v2_smooth_source_evidence non-empty (len=%zu)", strlen(ev));

    ev = csb_v2_viewport_source_evidence();
    PROBE_ASSERT(ev != NULL && strlen(ev) > 10,
                 "csb_v2_viewport_source_evidence non-empty (len=%zu)", strlen(ev));
}

/* ── Walk N/S/E/W ────────────────────────────────────────────────── */

/*
 * Game loop timing:
 *   v1_tick(T) fires every ~55ms (V1 cadence).
 *   render_frame(now_ms) fires every ~16ms (display rate).
 *   After v1_tick(55000), four render frames at 55000/55016/55032/55048
 *   give dt = 0/16/32/48ms total.  Sum = 96ms > 55ms → animation completes.
 *
 * Test pattern:
 *   v1_tick(0)          → start animation
 *   v1_tick(55000)      → V1 tick boundary
 *   render_frame(55000) → dt=0 (same timestamp), animation still active
 *   render_frame(55016) → dt=16ms, animation 16ms toward completion
 *   render_frame(55032) → dt=16ms, animation 32ms total
 *   render_frame(55048) → dt=16ms, animation 48ms total
 *   render_frame(55064) → dt=16ms, animation 64ms total > 55ms → completes
 */
static void test_smooth_walk_nsew(void) {
    printf("--- Smooth walk N/S/E/W ---\n");

    struct {
        const char *name;
        float fx, fy, tx, ty;
    } cases[] = {
        {"North", 10.0f, 10.0f, 10.0f, 9.0f},
        {"East",  10.0f, 10.0f, 11.0f, 10.0f},
        {"South", 10.0f, 10.0f, 10.0f, 11.0f},
        {"West",  10.0f, 10.0f,  9.0f, 10.0f},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        fprintf(stderr, "  Walk %s: (%.1f,%.1f) → (%.1f,%.1f)\n",
                cases[i].name,
                cases[i].fx, cases[i].fy,
                cases[i].tx, cases[i].ty);

        CSB_V2_ViewportState vp;
        csb_v2_viewport_init(&vp, 2);

        /* Start animation at T=0 */
        csb_v2_viewport_v1_tick(&vp, 0);
        csb_v2_smooth_start_walk(cases[i].fx, cases[i].fy,
                                 cases[i].tx, cases[i].ty);

        /* Animation active at tick start */
        PROBE_ASSERT(csb_v2_smooth_is_moving(),
                     "walk %s: animation active at start", cases[i].name);

        /* v1_tick(55000) — 4 render frames = 64ms > 55ms → animation completes */
        csb_v2_viewport_v1_tick(&vp, 55000);
        csb_v2_viewport_render_frame(&vp, 55000);
        csb_v2_viewport_render_frame(&vp, 55016);
        csb_v2_viewport_render_frame(&vp, 55032);
        csb_v2_viewport_render_frame(&vp, 55048);
        csb_v2_viewport_render_frame(&vp, 55064);

        /* After tick-end: animation complete, position at target */
        PROBE_ASSERT(!csb_v2_smooth_is_moving(),
                     "walk %s: done at tick-end", cases[i].name);
        PROBE_ASSERT_FLOAT_EQ(csb_v2_smooth_get_x(), cases[i].tx, 0.01f,
                              "walk %s: x at tick-end", cases[i].name);
        PROBE_ASSERT_FLOAT_EQ(csb_v2_smooth_get_y(), cases[i].ty, 0.01f,
                              "walk %s: y at tick-end", cases[i].name);
    }
}

/* ── Turn N/S/E/W ────────────────────────────────────────────────── */

static void test_smooth_turn_nsew(void) {
    printf("--- Smooth turn N/S/E/W ---\n");

    /* CSB uses same direction system as DM1: 0=N 90=E 180=S 270=W */
    struct {
        const char *name;
        float from, to;
    } cases[] = {
        {"N→E",   0.0f,  90.0f},
        {"E→S",  90.0f, 180.0f},
        {"S→W", 180.0f, 270.0f},
        {"W→N", 270.0f, 360.0f},
        {"E→N",  90.0f,   0.0f},
        {"S→E", 180.0f,  90.0f},
        {"W→S", 270.0f, 180.0f},
        {"N→W",   0.0f, 270.0f},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        fprintf(stderr, "  Turn %s: %.0f° → %.0f°\n",
                cases[i].name, cases[i].from, cases[i].to);

        CSB_V2_ViewportState vp;
        csb_v2_viewport_init(&vp, 2);

        csb_v2_viewport_v1_tick(&vp, 0);
        csb_v2_smooth_start_turn(cases[i].from, cases[i].to);

        PROBE_ASSERT(csb_v2_smooth_is_moving(),
                     "turn %s: animation active at start", cases[i].name);

        csb_v2_viewport_v1_tick(&vp, 55000);
        /* 5 renders = 64ms > 55ms → completes */
        csb_v2_viewport_render_frame(&vp, 55000);
        csb_v2_viewport_render_frame(&vp, 55016);
        csb_v2_viewport_render_frame(&vp, 55032);
        csb_v2_viewport_render_frame(&vp, 55048);
        csb_v2_viewport_render_frame(&vp, 55064);

        PROBE_ASSERT(!csb_v2_smooth_is_moving(),
                     "turn %s: done at tick-end", cases[i].name);

        /* Angle at tick-end should be at target (normalised to [0,360)) */
        float angle = csb_v2_smooth_get_angle();
        float exp = cases[i].to;
        while (exp < 0.0f)   exp += 360.0f;
        while (exp >= 360.0f) exp -= 360.0f;
        /* W→N: 360° normalises to 0°, but angle may be 360.0f — normalise */
        float angle_norm = angle;
        while (angle_norm < 0.0f)   angle_norm += 360.0f;
        while (angle_norm >= 360.0f) angle_norm -= 360.0f;
        PROBE_ASSERT_FLOAT_EQ(angle_norm, exp, 0.5f,
                              "turn %s: angle at tick-end", cases[i].name);
    }
}

/* ── Stairs ─────────────────────────────────────────────────────── */

static void test_smooth_stairs(void) {
    printf("--- Smooth stairs ---\n");

    CSB_V2_ViewportState vp;
    csb_v2_viewport_init(&vp, 2);

    csb_v2_viewport_v1_tick(&vp, 0);
    csb_v2_smooth_start_stairs(10.0f, 10.0f, 10.0f, 10.0f, 4.0f);

    /* At start, stairs active with vertical offset = 0 */
    PROBE_ASSERT(csb_v2_smooth_is_moving(), "stairs: active at start");
    PROBE_ASSERT_FLOAT_EQ(csb_v2_smooth_get_vertical(), 0.0f, 0.01f,
                           "stairs: vertical offset 0 at start");

    csb_v2_viewport_v1_tick(&vp, 55000);
    /* 5 renders = 64ms > 55ms → completes */
    csb_v2_viewport_render_frame(&vp, 55000);
    csb_v2_viewport_render_frame(&vp, 55016);
    csb_v2_viewport_render_frame(&vp, 55032);
    csb_v2_viewport_render_frame(&vp, 55048);
    csb_v2_viewport_render_frame(&vp, 55064);

    PROBE_ASSERT(!csb_v2_smooth_is_moving(),
                 "stairs: done at tick-end");
    PROBE_ASSERT_FLOAT_EQ(csb_v2_smooth_get_vertical(), 4.0f, 0.01f,
                           "stairs: vertical offset 4.0 at tick-end");
}

/* ── Deterministic coverage ─────────────────────────────────────── */

/*
 * Walk/Turn/Stairs: tick-start → mid-tick (dt=0) → tick-end (5 renders)
 */
static void test_deterministic_coverage(void) {
    printf("--- Deterministic input coverage ---\n");

    /* Walk: tick-start → mid-tick (first render, dt=0) → tick-end (5 renders) */
    {
        CSB_V2_ViewportState vp;
        csb_v2_viewport_init(&vp, 2);
        csb_v2_viewport_v1_tick(&vp, 0);
        csb_v2_smooth_start_walk(10.0f, 10.0f, 11.0f, 10.0f);

        PROBE_ASSERT(csb_v2_smooth_is_moving(),
                     "walk: active at tick-start");

        /* Mid-tick: after v1_tick(55000), first render (dt=0), animation active */
        csb_v2_viewport_v1_tick(&vp, 55000);
        csb_v2_viewport_render_frame(&vp, 55000);
        PROBE_ASSERT(csb_v2_smooth_is_moving(),
                     "walk: active at mid-tick (first render, dt=0)");

        /* Tick-end: 5 renders (64ms > 55ms), animation completes */
        csb_v2_viewport_render_frame(&vp, 55016);
        csb_v2_viewport_render_frame(&vp, 55032);
        csb_v2_viewport_render_frame(&vp, 55048);
        csb_v2_viewport_render_frame(&vp, 55064);
        PROBE_ASSERT(!csb_v2_smooth_is_moving(),
                     "walk: inactive at tick-end");
    }

    /* Turn: tick-start → mid-tick → tick-end */
    {
        CSB_V2_ViewportState vp;
        csb_v2_viewport_init(&vp, 2);
        csb_v2_viewport_v1_tick(&vp, 0);
        csb_v2_smooth_start_turn(0.0f, 90.0f);

        PROBE_ASSERT(csb_v2_smooth_is_moving(),
                     "turn: active at tick-start");

        csb_v2_viewport_v1_tick(&vp, 55000);
        csb_v2_viewport_render_frame(&vp, 55000);
        PROBE_ASSERT(csb_v2_smooth_is_moving(),
                     "turn: active at mid-tick (first render, dt=0)");

        csb_v2_viewport_render_frame(&vp, 55016);
        csb_v2_viewport_render_frame(&vp, 55032);
        csb_v2_viewport_render_frame(&vp, 55048);
        csb_v2_viewport_render_frame(&vp, 55064);
        PROBE_ASSERT(!csb_v2_smooth_is_moving(),
                     "turn: inactive at tick-end");
    }

    /* Stairs: tick-start → mid-tick → tick-end */
    {
        CSB_V2_ViewportState vp;
        csb_v2_viewport_init(&vp, 2);
        csb_v2_viewport_v1_tick(&vp, 0);
        csb_v2_smooth_start_stairs(10.0f, 10.0f, 10.0f, 10.0f, 4.0f);

        PROBE_ASSERT(csb_v2_smooth_is_moving(),
                     "stairs: active at tick-start");

        csb_v2_viewport_v1_tick(&vp, 55000);
        csb_v2_viewport_render_frame(&vp, 55000);
        PROBE_ASSERT(csb_v2_smooth_is_moving(),
                     "stairs: active at mid-tick");

        csb_v2_viewport_render_frame(&vp, 55016);
        csb_v2_viewport_render_frame(&vp, 55032);
        csb_v2_viewport_render_frame(&vp, 55048);
        csb_v2_viewport_render_frame(&vp, 55064);
        PROBE_ASSERT(!csb_v2_smooth_is_moving(),
                     "stairs: inactive at tick-end");
    }
}

/* ── Sub-tick interpolation ─────────────────────────────────────── */

static void test_sub_tick(void) {
    printf("--- Sub-tick interpolation ---\n");

    CSB_V2_ViewportState vp;
    csb_v2_viewport_init(&vp, 2);

    /* Start animation at T=0, v1_tick(55000).
     * Render frames at 55000/55016 give dt=0/16ms.
     * At 55016: 16ms ≈ 29% through the 55ms tick.
     *   ease-out cubic: 1-(1-0.29)^3 ≈ 0.64
     *   x = 0 + 0.64*(10-0) ≈ 6.4 */
    csb_v2_viewport_v1_tick(&vp, 0);
    csb_v2_smooth_start_walk(0.0f, 0.0f, 10.0f, 0.0f);
    csb_v2_viewport_v1_tick(&vp, 55000);
    csb_v2_viewport_render_frame(&vp, 55000);
    csb_v2_viewport_render_frame(&vp, 55016);

    float x = csb_v2_smooth_get_x();
    PROBE_ASSERT(x > 5.0f && x < 10.0f,
                  "sub-tick: x in range (%.2f, expected between 5.0 and 10.0)",
                  x);
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(void) {
    printf("CSB V2 Smooth Movement Probe — Phase 5\n");
    printf("========================================\n");

    test_smooth_init();
    test_smooth_walk_nsew();
    test_smooth_turn_nsew();
    test_smooth_stairs();
    test_deterministic_coverage();
    test_sub_tick();

    printf("========================================\n");
    printf("Results: %d passed, %d errors\n", g_pass, g_fail);
    if (g_fail > 0) {
        printf("STATUS: FAILED\n");
        return 1;
    }
    printf("STATUS: PASSED\n");
    return 0;
}
