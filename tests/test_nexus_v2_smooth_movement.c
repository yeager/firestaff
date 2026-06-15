/* Nexus V2 Phase 5 Smooth Movement — smoke test
 *
 * Tests that the smooth movement implementation initialises,
 * drives animations, and produces deterministic interpolated
 * positions without any game data or SDL.
 *
 * Reference: dm2_v2_smooth_movement.c (pass ad2dc2c8)
 * Key invariant: game state ONLY advances on V1 ticks.
 */

#include "nexus_v2_smooth_movement.h"
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

int main(void) {
    printf("=== Nexus V2 Phase 5 Smooth Movement smoke test ===\n\n");

    /* ── Init ───────────────────────────────────────────────────── */
    Nexus_V2_SmoothState state;
    nexus_v2_smooth_init(&state);
    check("init: smooth inactive", !nexus_v2_smooth_is_active(&state));

    /* ── Walk animation ─────────────────────────────────────────── */
    nexus_v2_smooth_start_walk(&state, 0.0f, 0.0f, 1.0f, 0.0f);
    check("walk: active after start", state.walk.active);
    check("walk: is_active true", nexus_v2_smooth_is_active(&state));

    /* Advance half a V1 tick — ease-out-cubic at t=0.5 gives ~0.875 progress */
    nexus_v2_smooth_update(&state, 55.0f * 0.5f);
    float px, py;
    nexus_v2_smooth_get_position(&state, &px, &py);
    /* ease-out cubic: t=0.5 → (1 - (1-0.5)^3) = 1 - 0.125 = 0.875 */
    checkf("walk X at t=0.5 (≈0.875)", px, 0.875f, 0.01f);
    checkf("walk Y at t=0.5 (≈0.0)", py, 0.0f, 0.01f);

    /* Advance full V1 tick → animation done */
    nexus_v2_smooth_update(&state, 55.0f * 0.6f);
    check("walk: inactive after full tick", !state.walk.active);
    check("walk: is_active false after complete",
        !nexus_v2_smooth_is_active(&state));

    /* ── Turn animation ─────────────────────────────────────────── */
    /* Turn north (0°) → east (90°) */
    nexus_v2_smooth_start_turn(&state, 0.0f, 90.0f);
    check("turn: active after start", state.turn.active);

    /* At t=0.5 ease-out-quad: progress = 1 - (1-0.5)^2 = 0.75
     * angle = 0 + 90 * 0.75 = 67.5 */
    nexus_v2_smooth_update(&state, 55.0f * 0.5f);
    float angle = nexus_v2_smooth_get_angle(&state);
    checkf("turn angle at t=0.5 (≈67.5°)", angle, 67.5f, 0.5f);

    /* Advance full tick → done */
    nexus_v2_smooth_update(&state, 55.0f * 0.6f);
    check("turn: inactive after full tick", !state.turn.active);

    /* ── Shortest path turn (wrap-around) ──────────────────────── */
    /* 350° → 10° (should go forward, not backward 340°) */
    nexus_v2_smooth_start_turn(&state, 350.0f, 10.0f);
    nexus_v2_smooth_update(&state, 55.0f * 0.5f);
    angle = nexus_v2_smooth_get_angle(&state);
    checkf("wrap turn mid (~5°)", fmodf(angle + 360.0f, 360.0f), 5.0f, 5.0f);

    /* ── Stairs animation ────────────────────────────────────────── */
    nexus_v2_smooth_start_stairs(&state,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f);
    check("stairs: active after start", state.stairs.active);
    checkf("stairs vertical at start", nexus_v2_smooth_get_vertical(&state), 0.0f, 0.01f);

    nexus_v2_smooth_update(&state, 55.0f * 0.5f);
    nexus_v2_smooth_get_position(&state, &px, &py);
    float vert = nexus_v2_smooth_get_vertical(&state);
    /* ease-in-out-cubic at t=0.5 = 0.5 (symmetric) */
    checkf("stairs X at t=0.5 (≈0.5)", px, 0.5f, 0.01f);
    checkf("stairs Y at t=0.5 (≈0.5)", py, 0.5f, 0.01f);
    checkf("stairs vert at t=0.5 (≈0.25)", vert, 0.25f, 0.01f);

    nexus_v2_smooth_update(&state, 55.0f * 0.6f);
    check("stairs: inactive after full tick", !state.stairs.active);

    /* ── tick / prev_x record ───────────────────────────────────── */
    nexus_v2_smooth_tick(&state, 5.0f, 3.0f, 180.0f);
    checkf("prev_x recorded after tick", state.prev_x, 5.0f, 0.001f);
    checkf("prev_y recorded after tick", state.prev_y, 3.0f, 0.001f);
    checkf("prev_angle recorded after tick", state.prev_angle, 180.0f, 0.001f);

    /* Moving to new game position triggers new walk animation */
    nexus_v2_smooth_tick(&state, 6.0f, 3.0f, 180.0f);
    check("walk: starts after position delta", state.walk.active);

    /* ── Pipeline integration (inline struct check) ───────────────── */
    /* Verify Nexus_V2_SmoothState is directly accessible and zeroable */
    Nexus_V2_SmoothState ps;
    memset(&ps, 0, sizeof(ps));
    check("pipeline smooth: zero-init inactive", !nexus_v2_smooth_is_active(&ps));
    check("pipeline smooth: prev_x/y/angle zeroed", ps.prev_x == 0.0f && ps.prev_y == 0.0f && ps.prev_angle == 0.0f);

    /* Walk state sub-struct accessible */
    ps.walk.active = 1;
    check("pipeline smooth: walk.active writable", ps.walk.active == 1);
    ps.walk.active = 0;

    /* Turn state sub-struct accessible */
    ps.turn.active = 1;
    check("pipeline smooth: turn.active writable", ps.turn.active == 1);
    ps.turn.active = 0;

    /* Stairs state sub-struct accessible */
    ps.stairs.active = 1;
    check("pipeline smooth: stairs.active writable", ps.stairs.active == 1);
    ps.stairs.active = 0;

    /* ── Null safety ─────────────────────────────────────────────── */
    nexus_v2_smooth_init(NULL);
    nexus_v2_smooth_start_walk(NULL, 0, 0, 1, 1);
    nexus_v2_smooth_start_turn(NULL, 0, 90);
    nexus_v2_smooth_start_stairs(NULL, 0, 0, 1, 1, 0, 0.5f);
    nexus_v2_smooth_update(NULL, 16.0f);
    nexus_v2_smooth_tick(NULL, 1, 1, 0);
    nexus_v2_smooth_get_position(NULL, &px, &py);
    (void)nexus_v2_smooth_get_vertical(NULL);
    (void)nexus_v2_smooth_get_angle(NULL);
    nexus_v2_smooth_is_active(NULL);
    check("null pointers: no crash", 1);

    /* ── Result ─────────────────────────────────────────────────── */
    printf("\n=== Results: %d passed, %d failed ===\n",
        s_tests_passed, s_tests_failed);
    return s_tests_failed > 0 ? 1 : 0;
}