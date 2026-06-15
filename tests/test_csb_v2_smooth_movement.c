/* CSB V2 Phase 5 Smooth Movement — smoke test
 *
 * Tests that the smooth movement implementation initialises,
 * drives animations, and produces deterministic interpolated
 * positions without any game data or SDL.
 *
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *            dm2_v2_smooth_movement.c (DM2 V2 Phase 5)
 *            csb_v2_smooth_movement.c
 * Key invariant: game state ONLY advances on V1 ticks.
 *
 * CSB V2 smooth is global state (no struct), driven by a
 * V2_AnimClock. This test creates a local clock and uses it
 * to advance the animations deterministically.
 */

#include "csb_v2_smooth_movement.h"
#include "dm1_v2_anim_timing.h"
#include <stdio.h>
#include <math.h>
#include <string.h>


static int s_tests_passed = 0;
static int s_tests_failed = 0;

static void check(const char *name, int cond) {
    if (cond) {
        printf("  PASS: %s\n", name);
        s_tests_passed++;
    } else {
        printf("  FAIL: %s\n", name);
        s_tests_failed++;
    }
}

static void checkf(const char *name, float got, float expected, float tol) {
    int ok = fabsf(got - expected) <= tol;
    if (ok) {
        printf("  PASS: %s (%.4f)\n", name, got);
        s_tests_passed++;
    } else {
        printf("  FAIL: %s — got %.4f expected %.4f (±%.4f)\n", name, got, expected, tol);
        s_tests_failed++;
    }
}

/* Helper: run a single sub-frame of dt_ms through the global smooth
 * state by mutating the local clock's dt_ms and feeding it to
 * csb_v2_smooth_update_from_clock.
 *
 * Sub_tick is held at 0.0; the test does not depend on sub-tick
 * interpolation — only on the per-frame dt_ms advancement. */
static void drive_csb(float dt_ms) {
    static V2_AnimClock clock;
    static int initialised = 0;
    if (!initialised) {
        v2_anim_clock_init(&clock);
        initialised = 1;
    }
    /* Inject the desired dt_ms by directly advancing all anims via
     * the global API. csb_v2_smooth_update_from_clock reads
     * clock->dt_ms, so we set it manually. */
    clock.dt_ms = dt_ms;
    csb_v2_smooth_update_from_clock(&clock);
}

int main(void) {
    printf("=== CSB V2 Phase 5 Smooth Movement smoke test ===\n\n");

    /* ── Init ───────────────────────────────────────────────────── */
    csb_v2_smooth_init();
    check("init: smooth inactive", !csb_v2_smooth_is_moving());

    /* ── Walk animation N/E/S/W ─────────────────────────────────── */
    /* ease-out-cubic at t=0.5: progress = 1 - (1-0.5)^3 = 0.875 */
    struct {
        const char *name;
        float fx, fy, tx, ty;
        float expected_t;
    } walk_cases[] = {
        {"North", 10.0f, 10.0f, 10.0f,  9.0f, 0.875f},
        {"East",  10.0f, 10.0f, 11.0f, 10.0f, 0.875f},
        {"South", 10.0f, 10.0f, 10.0f, 11.0f, 0.875f},
        {"West",  10.0f, 10.0f,  9.0f, 10.0f, 0.875f},
    };
    for (size_t i = 0; i < sizeof(walk_cases) / sizeof(walk_cases[0]); i++) {
        csb_v2_smooth_init();
        csb_v2_smooth_start_walk(walk_cases[i].fx, walk_cases[i].fy,
                                  walk_cases[i].tx, walk_cases[i].ty);
        check("walk: active after start", csb_v2_smooth_is_moving());
        drive_csb(55.0f * 0.5f);
        float px = csb_v2_smooth_get_x();
        float py = csb_v2_smooth_get_y();
        float fx_step = walk_cases[i].tx - walk_cases[i].fx;
        float fy_step = walk_cases[i].ty - walk_cases[i].fy;
        checkf("walk X at t=0.5 ease-out-cubic",
               px, walk_cases[i].fx + fx_step * walk_cases[i].expected_t, 0.01f);
        checkf("walk Y at t=0.5 ease-out-cubic",
               py, walk_cases[i].fy + fy_step * walk_cases[i].expected_t, 0.01f);
        /* Drive the rest of the tick */
        drive_csb(55.0f * 0.6f);
        check("walk: not moving after full tick", !csb_v2_smooth_is_moving());
    }

    /* ── Turn animation 8 directions ───────────────────────────── */
    /* ease-out-quad at t=0.5: progress = 1 - (1-0.5)^2 = 0.75 */
    struct {
        const char *name;
        float from, to;
    } turn_cases[] = {
        {"N→N",  0.0f,   0.0f},
        {"N→NE", 0.0f,  45.0f},
        {"N→E",  0.0f,  90.0f},
        {"N→SE", 0.0f, 135.0f},
        {"N→S",  0.0f, 180.0f},
        {"N→SW", 0.0f, 225.0f},
        {"N→W",  0.0f, 270.0f},
        {"N→NW", 0.0f, 315.0f},
    };
    for (size_t i = 0; i < sizeof(turn_cases) / sizeof(turn_cases[0]); i++) {
        csb_v2_smooth_init();
        csb_v2_smooth_start_turn(turn_cases[i].from, turn_cases[i].to);
        check("turn: active after start", csb_v2_smooth_is_moving());
        drive_csb(55.0f * 0.5f);
        float angle = csb_v2_smooth_get_angle();
        /* ease-out-quad at t=0.5: from + (to-from)*0.75 */
        float expected = turn_cases[i].from + (turn_cases[i].to - turn_cases[i].from) * 0.75f;
        checkf("turn angle at t=0.5 ease-out-quad", angle, expected, 1.0f);
        drive_csb(55.0f * 0.6f);
        check("turn: not moving after full tick", !csb_v2_smooth_is_moving());
    }

    /* ── Stairs animation with vertical offset ─────────────────── */
    csb_v2_smooth_init();
    csb_v2_smooth_start_stairs(5.0f, 5.0f, 6.0f, 5.0f, 0.5f);
    check("stairs: active after start", csb_v2_smooth_is_moving());
    checkf("stairs vertical at start (0.0)", csb_v2_smooth_get_vertical(), 0.0f, 0.001f);

    /* ease-in-out-cubic at t=0.5: position progress = 0.5, vert progress = 0.25
     * (vert anims from 0 to 0.5, mid-progress = 0.5 * 0.5 ease = 0.25) */
    drive_csb(55.0f * 0.5f);
    checkf("stairs X at t=0.5 (≈5.5)", csb_v2_smooth_get_x(), 5.5f, 0.01f);
    checkf("stairs Y at t=0.5 (≈5.0)", csb_v2_smooth_get_y(), 5.0f, 0.01f);
    checkf("stairs vert at t=0.5 (≈0.25)", csb_v2_smooth_get_vertical(), 0.25f, 0.01f);

    drive_csb(55.0f * 0.6f);
    check("stairs: not moving after full tick", !csb_v2_smooth_is_moving());
    /* After complete, vert anims hold at final 0.5 */
    checkf("stairs vert at end (≈0.5)", csb_v2_smooth_get_vertical(), 0.5f, 0.01f);

    /* ── Source evidence ────────────────────────────────────────── */
    const char *ev = csb_v2_smooth_source_evidence();
    check("source evidence: non-empty", ev != NULL && strlen(ev) > 10);

    /* ── Null safety ─────────────────────────────────────────────── */
    csb_v2_smooth_update_from_clock(NULL);
    check("null clock: no crash", 1);

    /* ── Result ─────────────────────────────────────────────────── */
    printf("\n=== Results: %d passed, %d failed ===\n",
        s_tests_passed, s_tests_failed);
    return s_tests_failed > 0 ? 1 : 0;
}
