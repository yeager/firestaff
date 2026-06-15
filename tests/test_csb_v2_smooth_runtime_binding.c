/* CSB V2 Phase 5 — Runtime Binding Integration Test
 *
 * Verifies the Phase 5 binding seam: csb_v2_runtime_bind_to_v1(profile)
 * observes the V1 party state at each v1_tick and triggers the
 * appropriate smooth animation (walk / turn / stairs) when the
 * party position changes.
 *
 * Test strategy:
 *   - Mock CSB_V1_RuntimeProfile (in-memory; no game assets required)
 *   - Drive csb_v2_runtime_v1_tick() at fake timestamps
 *   - Mutate the profile's party_x/y/z/dir between ticks
 *   - Assert csb_v2_smooth_is_moving() and the cached smooth values
 *
 * Source-locked references:
 *   - ReDMCSB COMMAND.C F0380 (queue dispatch)
 *   - ReDMCSB CLIKMENU.C F0365 (turn), F0366 (move)
 *   - ReDMCSB CLIKMENU.C F0364 (stairs — party_z delta)
 *   - ReDMCSB GAMELOOP.C (V1 tick cadence, VBLANK-locked 55ms)
 *
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *            dm2_v2_runtime.c (DM2 V2 runtime integration)
 *            csb_v2_smooth_movement.c (CSB V2.2 smooth movement)
 */

#include "csb_v2_runtime.h"
#include "csb_v2_smooth_movement.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v2_anim_timing.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

/* ── Test framework ──────────────────────────────────────────────── */

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

/* ── Test helpers ────────────────────────────────────────────────── */

/* Reset the V2 runtime + V1 profile, and bind them.  Returns the
 * profile pointer for further mutation. */
static CSB_V1_RuntimeProfile *fresh_bind(int start_x, int start_y,
                                          int start_z, int start_dir) {
    static CSB_V1_RuntimeProfile profile;
    memset(&profile, 0, sizeof(profile));
    profile.party_x = start_x;
    profile.party_y = start_y;
    profile.party_z = start_z;
    profile.party_dir = start_dir;

    /* Cleanup + re-init runtime so each test starts clean. */
    csb_v2_runtime_cleanup();
    csb_v2_runtime_init(2);
    csb_v2_runtime_bind_to_v1(&profile);
    return &profile;
}

/* Drive a single V1 tick + one render frame at a given wall-clock
 * timestamp.  This is what the game loop does:
 *   csb_v1_runtime_tick()         → V1 game state advance
 *   csb_v2_runtime_v1_tick(now)   → V2 clock + binding detect
 *   csb_v2_runtime_render_frame(now) → smooth advance by dt */
static void drive_tick(uint32_t now_ms) {
    csb_v2_runtime_v1_tick(now_ms);
    csb_v2_runtime_render_frame(now_ms);
}

/* ── Tests ────────────────────────────────────────────────────────── */

static void test_lifecycle_and_bound_flag(void) {
    printf("\n--- test_lifecycle_and_bound_flag ---\n");
    csb_v2_runtime_cleanup();
    csb_v2_runtime_init(2);
    check("init: not bound", !csb_v2_runtime_is_bound());
    check("init: viewport non-NULL", csb_v2_runtime_get_viewport() != NULL);

    /* bind/unbind */
    CSB_V1_RuntimeProfile p;
    memset(&p, 0, sizeof(p));
    csb_v2_runtime_bind_to_v1(&p);
    check("bind: is bound", csb_v2_runtime_is_bound());

    csb_v2_runtime_bind_to_v1(NULL);
    check("unbind (NULL): not bound", !csb_v2_runtime_is_bound());
}

static void test_walk_trigger_on_xy_delta(void) {
    printf("\n--- test_walk_trigger_on_xy_delta ---\n");
    CSB_V1_RuntimeProfile *p = fresh_bind(10, 10, 0, 0);
    check("walk: bound", csb_v2_runtime_is_bound());
    check("walk: not moving at start", !csb_v2_smooth_is_moving());

    /* First tick at t=0: anchors "from" — no animation. */
    drive_tick(0);
    check("walk: not moving after anchor tick", !csb_v2_smooth_is_moving());

    /* Move 1 step North: y -= 1. */
    p->party_y = 9;
    drive_tick(55);
    check("walk: moving after xy delta", csb_v2_smooth_is_moving());

    /* Walk easing: from=10.0 to=9.0; after 0 ms of animation the
     * value is "from".  The animation will start but the value read
     * here is the cached pre-render value. */
    checkf("walk: x stays at from (10.0)",
           csb_v2_smooth_get_x(), 10.0f, 0.01f);
}

static void test_turn_trigger_on_dir_delta(void) {
    printf("\n--- test_turn_trigger_on_dir_delta ---\n");
    CSB_V1_RuntimeProfile *p = fresh_bind(5, 5, 0, 0);
    drive_tick(0);
    check("turn: not moving after anchor tick", !csb_v2_smooth_is_moving());

    /* Turn 90° clockwise: dir 0→1 (N→E). */
    p->party_dir = 1;
    drive_tick(55);
    check("turn: moving after dir delta", csb_v2_smooth_is_moving());
}

static void test_no_trigger_on_no_change(void) {
    printf("\n--- test_no_trigger_on_no_change ---\n");
    (void)fresh_bind(7, 7, 0, 2);
    drive_tick(0);

    /* No change at all across multiple ticks → no animation. */
    drive_tick(55);
    check("noop: not moving when no V1 delta", !csb_v2_smooth_is_moving());
    drive_tick(110);
    check("noop: still not moving on second tick", !csb_v2_smooth_is_moving());
    drive_tick(165);
    check("noop: still not moving on third tick", !csb_v2_smooth_is_moving());
}

static void test_stairs_trigger_on_z_delta(void) {
    printf("\n--- test_stairs_trigger_on_z_delta ---\n");
    CSB_V1_RuntimeProfile *p = fresh_bind(3, 3, 0, 0);
    drive_tick(0);
    check("stairs: not moving after anchor tick", !csb_v2_smooth_is_moving());

    /* party_z change (level) is the F0364 stairs event. */
    p->party_z = -1;
    drive_tick(55);
    check("stairs: moving after party_z delta", csb_v2_smooth_is_moving());
    checkf("stairs: vertical at from (0.0)",
           csb_v2_smooth_get_vertical(), 0.0f, 0.01f);
}

static void test_stairs_priority_over_walk(void) {
    printf("\n--- test_stairs_priority_over_walk ---\n");
    CSB_V1_RuntimeProfile *p = fresh_bind(3, 3, 0, 0);
    drive_tick(0);

    /* Both xy AND z change on the same tick — stairs takes priority
     * (CLIKMENU.C F0364 happens before F0366 in the source-lock). */
    p->party_y = 4;
    p->party_z = -1;
    drive_tick(55);
    check("stairs-priority: moving", csb_v2_smooth_is_moving());
    /* Stairs path is active, so get_x/y read stairs anim values. */
    /* The stairs animation eases from (3, 3) to (3, 4) with vert=−1.
     * At t=0, value is "from".  But we triggered the stairs path,
     * so get_x() / get_y() return stairs_x / stairs_y (not walk). */
    checkf("stairs-priority: x at from (3.0)",
           csb_v2_smooth_get_x(), 3.0f, 0.01f);
    checkf("stairs-priority: y at from (3.0)",
           csb_v2_smooth_get_y(), 3.0f, 0.01f);
}

static void test_force_sync_resets_anchor(void) {
    printf("\n--- test_force_sync_resets_anchor ---\n");
    CSB_V1_RuntimeProfile *p = fresh_bind(8, 8, 0, 0);
    drive_tick(0);

    /* Simulate a saved-game load: party jumped from (8,8) to (20,20)
     * but we do NOT want a phantom walk from (8,8) to (20,20). */
    p->party_x = 20;
    p->party_y = 20;
    csb_v2_runtime_force_sync();
    check("force_sync: not moving immediately", !csb_v2_smooth_is_moving());

    /* Subsequent tick with no V1 change must not start a phantom walk. */
    drive_tick(55);
    check("force_sync: no phantom walk on next tick",
          !csb_v2_smooth_is_moving());

    /* But a fresh move should still trigger normally.  Use a
     * 1-ms render frame delta so the animation is still active when
     * we check is_moving() — a full 55-ms render frame would
     * complete the 1-V1-tick animation before we get to assert. */
    p->party_x = 21;
    csb_v2_runtime_v1_tick(60);
    check("force_sync: fresh move triggers walk",
          csb_v2_smooth_is_moving());
    (void)p; /* suppress unused-variable warning under -Wextra */
}

static void test_unbind_stops_triggers(void) {
    printf("\n--- test_unbind_stops_triggers ---\n");
    CSB_V1_RuntimeProfile *p = fresh_bind(1, 1, 0, 0);
    drive_tick(0);
    check("unbind: bound", csb_v2_runtime_is_bound());

    csb_v2_runtime_bind_to_v1(NULL);
    check("unbind: not bound", !csb_v2_runtime_is_bound());

    /* Mutate profile — but the binding is gone, so no trigger. */
    p->party_y = 2;
    drive_tick(55);
    check("unbind: no trigger after unbind",
          !csb_v2_smooth_is_moving());
}

static void test_manual_api_works_independently(void) {
    printf("\n--- test_manual_api_works_independently ---\n");
    csb_v2_runtime_cleanup();
    csb_v2_runtime_init(2);

    /* No V1 profile bound.  Manual API still works. */
    check("manual: not moving at start", !csb_v2_smooth_is_moving());

    csb_v2_runtime_smooth_walk(0.0f, 0.0f, 1.0f, 0.0f);
    check("manual: moving after smooth_walk", csb_v2_smooth_is_moving());

    csb_v2_runtime_smooth_turn(0.0f, 90.0f);
    check("manual: still moving after smooth_turn", csb_v2_smooth_is_moving());

    csb_v2_runtime_smooth_stairs(0.0f, 0.0f, 1.0f, 0.0f, 0.5f);
    check("manual: still moving after smooth_stairs", csb_v2_smooth_is_moving());
}

static void test_source_evidence(void) {
    printf("\n--- test_source_evidence ---\n");
    const char *ev = csb_v2_runtime_source_evidence();
    check("source evidence: non-empty", ev != NULL && strlen(ev) > 10);
    check("source evidence: mentions F0380", strstr(ev, "F0380") != NULL);
    check("source evidence: mentions F0365", strstr(ev, "F0365") != NULL);
    check("source evidence: mentions F0366", strstr(ev, "F0366") != NULL);
    check("source evidence: mentions F0364", strstr(ev, "F0364") != NULL);
    check("source evidence: mentions GAMELOOP.C",
          strstr(ev, "GAMELOOP.C") != NULL);
    check("source evidence: mentions Phase 5",
          strstr(ev, "Phase 5") != NULL);
}

static void test_idempotent_rebind(void) {
    printf("\n--- test_idempotent_rebind ---\n");
    csb_v2_runtime_cleanup();
    csb_v2_runtime_init(2);

    static CSB_V1_RuntimeProfile p1, p2;
    memset(&p1, 0, sizeof(p1));
    memset(&p2, 0, sizeof(p2));
    p1.party_x = 5; p1.party_y = 5;
    p2.party_x = 100; p2.party_y = 100;

    csb_v2_runtime_bind_to_v1(&p1);
    check("rebind: bound to p1", csb_v2_runtime_is_bound());

    csb_v2_runtime_bind_to_v1(&p2);
    check("rebind: still bound to p2", csb_v2_runtime_is_bound());

    /* After rebinding to p2, the anchor should be p2's (100, 100),
     * not p1's (5, 5).  Drive a tick — p1 changes should NOT trigger. */
    p1.party_x = 6;
    drive_tick(0);
    check("rebind: p1 mutation after rebind does not trigger",
          !csb_v2_smooth_is_moving());

    /* p2 mutations should now trigger. */
    p2.party_y = 99;
    drive_tick(55);
    check("rebind: p2 mutation after rebind does trigger",
          csb_v2_smooth_is_moving());
}

static void test_walk_completes_after_full_v1_tick(void) {
    printf("\n--- test_walk_completes_after_full_v1_tick ---\n");
    CSB_V1_RuntimeProfile *p = fresh_bind(0, 0, 0, 0);
    drive_tick(0);

    p->party_x = 1;
    drive_tick(55);
    check("walk-complete: moving right after trigger",
          csb_v2_smooth_is_moving());

    /* Advance animation past completion: total elapsed since the
     * trigger tick (55ms) — drive another full tick to push the
     * smooth animation well past 1 V1 tick (55ms). */
    drive_tick(110);
    drive_tick(165);
    /* After ~110ms of animation (55 + 55), the 55ms-duration walk
     * should be complete.  Allow one more frame to be safe. */
    drive_tick(220);
    check("walk-complete: not moving after full tick elapsed",
          !csb_v2_smooth_is_moving());
}

/* ── Main ────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== CSB V2 Phase 5 Runtime Binding integration test ===\n");

    test_lifecycle_and_bound_flag();
    test_walk_trigger_on_xy_delta();
    test_turn_trigger_on_dir_delta();
    test_no_trigger_on_no_change();
    test_stairs_trigger_on_z_delta();
    test_stairs_priority_over_walk();
    test_force_sync_resets_anchor();
    test_unbind_stops_triggers();
    test_manual_api_works_independently();
    test_source_evidence();
    test_idempotent_rebind();
    test_walk_completes_after_full_v1_tick();

    printf("\n=== Results: %d passed, %d failed ===\n",
        s_tests_passed, s_tests_failed);
    return s_tests_failed > 0 ? 1 : 0;
}
