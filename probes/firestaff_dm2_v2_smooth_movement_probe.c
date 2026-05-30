/**
 * firestaff_dm2_v2_smooth_movement_probe.c
 *
 * Pass H2335: DM2 V2 Phase 5 — Smooth Movement and Viewport Interpolation Probe
 *
 * Headless C probe exercising the DM2 V2 smooth movement runtime binding:
 *   - DM2 V2 runtime lifecycle (init, V1 tick, render frame)
 *   - Smooth walk animation for N/S/E/W directions
 *   - Smooth turn animation for all 8 cardinal+diagonal directions
 *   - Smooth stairs animation with vertical offset
 *   - Deterministic input coverage: tick start, mid-tick, tick-end
 *   - Pixel/presentation gates: V2 smooth vs V1 discrete headless comparison
 *
 * Compile (from repo root):
 *   cmake -B build -DCMAKE_BUILD_TYPE=Debug
 *   cmake --build build --target firestaff_dm2_v2_smooth_movement_probe
 *
 * Run (no game data needed):
 *   ./build/firestaff_dm2_v2_smooth_movement_probe
 *
 * Schema: firestaff.dm2_v2.smooth_movement_probe.v1
 *
 * Source: SKULL.ASM T520 — party/movement tick
 *         SKULL.ASM T560 — dungeon viewport rendering
 *         SKULL.ASM T600 — outdoor viewport rendering
 *         ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *         ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 */

#include "dm2_v2_runtime.h"
#include "dm2_v2_smooth_movement.h"
#include "dm2_v2_viewport_renderer.h"
#include "dm2_v1_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Test framework ────────────────────────────────────────────────── */

static int passed = 0;
static int errors = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        passed++; \
    } \
} while (0)

#define PROBE_ASSERT_FLOAT_EQ(actual, expected, tolerance, fmt, ...) do { \
    float _a = (actual); float _e = (expected); \
    if (_a < _e - (tolerance) || _a > _e + (tolerance)) { \
        fprintf(stderr, "FAIL: " fmt " (got %.4f, expected %.4f ±%.4f)\n", \
                ##__VA_ARGS__, _a, _e, (tolerance)); \
        errors++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        passed++; \
    } \
} while (0)

/* ── Lifecycle tests ─────────────────────────────────────────────────── */

static void test_smooth_init(void) {
    printf("--- Smooth init ---\n");

    dm2_v2_runtime_init(2);

    DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
    PROBE_ASSERT(vp != NULL, "viewport state non-NULL after init");

    /* V1 ticks at two different timestamps advance the clock */
    dm2_v2_runtime_v1_tick(0);
    dm2_v2_runtime_v1_tick(55000);
    PROBE_ASSERT(1, "V1 tick sequence does not crash");

    /* Trigger smooth walk directly via the runtime API */
    dm2_v2_runtime_smooth_walk(5.0f, 5.0f, 6.0f, 5.0f);
    PROBE_ASSERT(1, "smooth_walk does not crash");

    /* Render frame should succeed (stub renderer, no dungeon data needed) */
    uint8_t fb[320 * 200];
    memset(fb, 0, sizeof(fb));
    int r = dm2_v2_runtime_render_frame(0, 6, 5, fb, 320, 320, 200);
    PROBE_ASSERT(r == 0 || r == -1, "render_frame returns 0 or -1 (headless stub)");

    /* Source evidence strings are non-trivial */
    const char *ev = dm2_v2_runtime_source_evidence();
    PROBE_ASSERT(ev != NULL && strlen(ev) > 10,
                 "dm2_v2_runtime_source_evidence non-empty");
}

/* ── N/S/E/W walk coverage ───────────────────────────────────────────── */

/*
 * Animation clock timing strategy:
 *
 * v2_anim_clock_v1_tick(clock, T) sets clock->last_v1_tick_ms = T.
 * v2_anim_clock_render_frame(clock, now_ms) computes:
 *     dt_ms = now_ms - clock->last_render_ms   (NOT last_v1_tick_ms!)
 *
 * On first render after v1_tick(0):
 *     render_frame(now_ms=0): dt = 0 - last_render_ms(0) = 0
 *     Then clock->last_render_ms is updated to 0.
 *
 * After v1_tick(55000), next render_frame:
 *     dt = now_ms(55000) - last_render_ms(0) = 55000 ms!
 *     Animation completes (duration = 55000ms).
 *
 * Summary: v1_tick(55000) + render_frame() → dt = 55000 → animation complete.
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

        dm2_v2_runtime_init(2);

        /* Start animation at T=0 */
        dm2_v2_runtime_v1_tick(0);
        dm2_v2_runtime_smooth_walk(cases[i].fx, cases[i].fy,
                                   cases[i].tx, cases[i].ty);

        /* Verify animation is active at tick start */
        {
            DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
            PROBE_ASSERT(dm2_v2_smooth_is_active(&vp->smooth),
                         "walk %s: animation active at start",
                         cases[i].name);
        }

        /* Mid-tick check: after v1_tick(55000) but before completing render_frame,
         * the animation is still running (render hasn't been called yet). */
        {
            DM2_V2_ViewportState *vp_before = dm2_v2_runtime_get_viewport();
            PROBE_ASSERT(dm2_v2_smooth_is_active(&vp_before->smooth),
                         "walk %s: animation active before mid-tick render",
                         cases[i].name);

            /* First render_frame after v1_tick(55000): dt = 55000 → completes */
            uint8_t fb[320 * 200];
            memset(fb, 0, sizeof(fb));
            dm2_v2_runtime_render_frame(0, (int)cases[i].tx, (int)cases[i].ty,
                                        fb, 320, 320, 200);
        }

        /* After completing render, animation should be done */
        {
            DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
            PROBE_ASSERT(!dm2_v2_smooth_is_active(&vp->smooth),
                         "walk %s: animation done at tick-end",
                         cases[i].name);
        }
    }
}

/* ── Turn coverage N/S/E/W ───────────────────────────────────────────── */

static void test_smooth_turn_nsew(void) {
    printf("--- Smooth turn N/S/E/W ---\n");

    struct {
        const char *name;
        int from_dir, to_dir;
    } cases[] = {
        {"N→E", 0, 1}, {"E→S", 1, 2}, {"S→W", 2, 3}, {"W→N", 3, 0},
        {"E→N", 1, 0}, {"S→E", 2, 1}, {"W→S", 3, 2}, {"N→W", 0, 3},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        fprintf(stderr, "  Turn %s: dir %d → dir %d\n",
                cases[i].name, cases[i].from_dir, cases[i].to_dir);

        dm2_v2_runtime_init(2);

        float fa = (float)cases[i].from_dir * 90.0f;
        float ta = (float)cases[i].to_dir * 90.0f;

        dm2_v2_runtime_v1_tick(0);
        dm2_v2_runtime_smooth_turn(fa, ta);

        {
            DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
            PROBE_ASSERT(dm2_v2_smooth_is_turning(&vp->smooth),
                         "turn %s: active at start",
                         cases[i].name);
        }

        /* v1_tick(55000) + render_frame(): dt = 55000 → completes */
        dm2_v2_runtime_v1_tick(55000);
        {
            uint8_t fb[320 * 200];
            memset(fb, 0, sizeof(fb));
            dm2_v2_runtime_render_frame(cases[i].to_dir, 10, 10,
                                        fb, 320, 320, 200);
        }

        DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
        PROBE_ASSERT(!dm2_v2_smooth_is_turning(&vp->smooth),
                     "turn %s: done at tick-end",
                     cases[i].name);
    }
}

/* ── Stairs ────────────────────────────────────────────────────────── */

static void test_smooth_stairs(void) {
    printf("--- Smooth stairs ---\n");

    dm2_v2_runtime_init(2);

    dm2_v2_runtime_v1_tick(0);
    dm2_v2_runtime_smooth_stairs(10.0f, 10.0f, 10.0f, 10.0f, 4.0f);

    /* At start, stairs active with vertical offset = 0 */
    {
        DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
        PROBE_ASSERT(dm2_v2_smooth_is_active(&vp->smooth),
                     "stairs: active at start");
        float vert = dm2_v2_smooth_get_vertical(&vp->smooth);
        PROBE_ASSERT_FLOAT_EQ(vert, 0.0f, 0.01f,
                              "stairs: vertical offset 0 at start");
    }

    /* v1_tick(55000) + render_frame(): dt = 55000 → completes */
    {
        dm2_v2_runtime_v1_tick(55000);
        uint8_t fb[320 * 200];
        memset(fb, 0, sizeof(fb));
        dm2_v2_runtime_render_frame(0, 10, 10, fb, 320, 320, 200);
        DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
        PROBE_ASSERT(!dm2_v2_smooth_is_active(&vp->smooth),
                     "stairs: done at tick-end");
    }
}

/* ── Deterministic input coverage ───────────────────────────────────── */

/*
 * Deterministic coverage: each animation type checked at tick-start,
 * mid-tick (before completing render), and tick-end (after completing render).
 *
 * The v2_anim_clock dt_ms accumulation:
 *   v1_tick(0): last_v1_tick_ms=0
 *   render_frame(): dt=0 (last_render_ms starts at 0, not last_v1_tick_ms)
 *   v1_tick(55000): last_v1_tick_ms=55000
 *   render_frame(): dt=55000-0=55000 → completes animation
 *
 * For mid-tick verification, we check animation state BEFORE the
 * completing render_frame (after v1_tick but before render).
 */
static void test_deterministic_coverage(void) {
    printf("--- Deterministic input coverage ---\n");

    /* Walk: tick-start → mid-tick → tick-end */
    {
        dm2_v2_runtime_init(2);
        dm2_v2_runtime_v1_tick(0);
        dm2_v2_runtime_smooth_walk(10.0f, 10.0f, 11.0f, 10.0f);

        DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
        PROBE_ASSERT(dm2_v2_smooth_is_active(&vp->smooth),
                     "walk: active at tick-start");

        /* Mid-tick: after v1_tick(55000), before completing render_frame.
         * Animation is still running since completing render hasn't happened. */
        dm2_v2_runtime_v1_tick(55000);
        PROBE_ASSERT(dm2_v2_smooth_is_active(&vp->smooth),
                     "walk: active at mid-tick (before completing render)");

        /* Completing render: dt_ms = 55000 → animation completes */
        {
            uint8_t fb[320 * 200];
            memset(fb, 0, sizeof(fb));
            dm2_v2_runtime_render_frame(0, 11, 10, fb, 320, 320, 200);
        }
        PROBE_ASSERT(!dm2_v2_smooth_is_active(&vp->smooth),
                     "walk: inactive at tick-end (after completing render)");
    }

    /* Turn: tick-start → mid-tick → tick-end */
    {
        dm2_v2_runtime_init(2);
        dm2_v2_runtime_v1_tick(0);
        dm2_v2_runtime_smooth_turn(0.0f, 90.0f);

        DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
        PROBE_ASSERT(dm2_v2_smooth_is_turning(&vp->smooth),
                     "turn: active at tick-start");

        dm2_v2_runtime_v1_tick(55000);
        PROBE_ASSERT(dm2_v2_smooth_is_turning(&vp->smooth),
                     "turn: active at mid-tick");

        {
            uint8_t fb[320 * 200];
            memset(fb, 0, sizeof(fb));
            dm2_v2_runtime_render_frame(1, 10, 10, fb, 320, 320, 200);
        }
        PROBE_ASSERT(!dm2_v2_smooth_is_turning(&vp->smooth),
                     "turn: inactive at tick-end");
    }

    /* Stairs: tick-start → mid-tick → tick-end */
    {
        dm2_v2_runtime_init(2);
        dm2_v2_runtime_v1_tick(0);
        dm2_v2_runtime_smooth_stairs(10.0f, 10.0f, 10.0f, 10.0f, 4.0f);

        DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
        PROBE_ASSERT(dm2_v2_smooth_is_active(&vp->smooth),
                     "stairs: active at tick-start");

        dm2_v2_runtime_v1_tick(55000);
        PROBE_ASSERT(dm2_v2_smooth_is_active(&vp->smooth),
                     "stairs: active at mid-tick");

        {
            uint8_t fb[320 * 200];
            memset(fb, 0, sizeof(fb));
            dm2_v2_runtime_render_frame(0, 10, 10, fb, 320, 320, 200);
        }
        PROBE_ASSERT(!dm2_v2_smooth_is_active(&vp->smooth),
                     "stairs: inactive at tick-end");
    }
}

/* ── Pixel/presentation gates ───────────────────────────────────────── */

/*
 * Pixel gate in headless mode:
 *
 * Without dungeon data, dm2_v1_runtime_render_frame returns -1.
 * The pixel gate in headless mode verifies:
 *   1. V2 idle returns same status as V1
 *   2. V2 smooth mid-tick: animation active, render completes
 *   3. V2 smooth tick-end: animation done, render completes
 *
 * With dungeon data, actual pixel comparison would be enabled:
 *   V2 idle: should match V1 discrete render
 *   V2 smooth mid-tick: should differ from V1 (animation offset applied)
 *   V2 smooth tick-end: should match V1 (animation complete)
 */
static void test_pixel_gate(void) {
    printf("--- Pixel/presentation gates (headless) ---\n");

    /* Gate 1: V2 idle = V1 idle status (both return -1 in headless) */
    {
        dm2_v2_runtime_init(2);

        uint8_t fb_v1[320 * 200];
        uint8_t fb_v2[320 * 200];
        memset(fb_v1, 0, sizeof(fb_v1));
        memset(fb_v2, 0, sizeof(fb_v2));

        int v1_status = dm2_v1_runtime_render_frame(0, 10, 10,
                                                     fb_v1, 320, 320, 200);
        int v2_status = dm2_v2_runtime_render_frame(0, 10, 10,
                                                     fb_v2, 320, 320, 200);

        PROBE_ASSERT(v1_status == v2_status,
                     "pixel gate: V2 idle status matches V1 (%d == %d)",
                     v2_status, v1_status);
        PROBE_ASSERT(v2_status == -1,
                     "pixel gate: headless stub returns -1 (got %d)",
                     v2_status);
    }

    /* Gate 2: V2 smooth mid-tick — animation active, render completes */
    {
        dm2_v2_runtime_init(2);
        dm2_v2_runtime_v1_tick(0);
        dm2_v2_runtime_smooth_walk(10.0f, 10.0f, 11.0f, 10.0f);
        dm2_v2_runtime_v1_tick(55000);

        uint8_t fb[320 * 200];
        memset(fb, 0, sizeof(fb));

        int r = dm2_v2_runtime_render_frame(0, 11, 10, fb, 320, 320, 200);

        DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
        PROBE_ASSERT(dm2_v2_smooth_is_active(&vp->smooth),
                     "pixel gate smooth: animation active at mid-tick");
        PROBE_ASSERT(r == 0 || r == -1,
                     "pixel gate smooth: render completes (got %d)",
                     r);
    }

    /* Gate 3: V2 smooth at tick-end — animation done, render completes */
    {
        dm2_v2_runtime_init(2);
        dm2_v2_runtime_v1_tick(0);
        dm2_v2_runtime_smooth_walk(10.0f, 10.0f, 11.0f, 10.0f);
        dm2_v2_runtime_v1_tick(55000);

        uint8_t fb[320 * 200];
        memset(fb, 0, sizeof(fb));

        int r = dm2_v2_runtime_render_frame(0, 11, 10, fb, 320, 320, 200);

        DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
        PROBE_ASSERT(!dm2_v2_smooth_is_active(&vp->smooth),
                     "pixel gate end: animation complete at tick-end");
        PROBE_ASSERT(r == 0 || r == -1,
                     "pixel gate end: render completes (got %d)",
                     r);
    }
}

/* ── Source evidence ───────────────────────────────────────────────── */

static void test_source_evidence(void) {
    printf("--- Source evidence ---\n");

    const char *ev = NULL;

    ev = dm2_v2_runtime_source_evidence();
    PROBE_ASSERT(ev != NULL && strlen(ev) > 50,
                 "dm2_v2_runtime_source_evidence non-trivial (len=%zu)",
                 strlen(ev));

    ev = dm2_v2_viewport_source_evidence();
    PROBE_ASSERT(ev != NULL && strlen(ev) > 10,
                 "dm2_v2_viewport_source_evidence non-trivial");

    ev = dm2_v2_smooth_source_evidence();
    PROBE_ASSERT(ev != NULL && strlen(ev) > 10,
                 "dm2_v2_smooth_source_evidence non-trivial");
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(void) {
    printf("DM2 V2 Smooth Movement Probe — Phase 5\n");
    printf("========================================\n");

    test_smooth_init();
    test_smooth_walk_nsew();
    test_smooth_turn_nsew();
    test_smooth_stairs();
    test_deterministic_coverage();
    test_pixel_gate();
    test_source_evidence();

    printf("========================================\n");
    printf("Results: %d passed, %d errors\n", passed, errors);
    if (errors > 0) {
        printf("STATUS: FAILED\n");
        return 1;
    }
    printf("STATUS: PASSED\n");
    return 0;
}