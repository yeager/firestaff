/* DM2 V2 Phase 5 — Runtime Binding Integration Test
 *
 * Verifies the Phase 5 binding seam: dm2_v2_runtime_bind_to_v1(profile)
 * observes the V1 party state at each v1_tick and triggers the
 * appropriate smooth animation (walk / turn / stairs) when the
 * party position changes.
 *
 * Test strategy:
 *   - Mock DM2_V1_RuntimeProfile (in-memory; no game assets required)
 *   - Drive dm2_v2_runtime_v1_tick() at fake timestamps
 *   - Mutate the profile's party_x/y/dir/current_level between ticks
 *   - Assert dm2_v2_smooth_is_active() and the cached smooth values
 *
 * Source-locked references:
 *   - SKULL.ASM T520  — party/movement tick
 *   - SKULL.ASM T048  — input dispatch / tick update
 *   - ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)
 *   - ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *            dm2_v2_smooth_movement.c (DM2 V2 Phase 5 smooth)
 *            dm2_v2_runtime.c (DM2 V2 runtime integration)
 *            csb_v2_runtime.c (CSB V2 binding seam — same shape)
 */

#include "dm2_v2_runtime.h"
#include "dm2_v2_smooth_movement.h"
#include "dm2_v2_viewport_renderer.h"
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
static DM2_V1_RuntimeProfile *fresh_bind(int start_x, int start_y,
                                          int start_z, int start_dir) {
    static DM2_V1_RuntimeProfile profile;
    memset(&profile, 0, sizeof(profile));
    profile.party_x = start_x;
    profile.party_y = start_y;
    profile.party_dir = start_dir;
    profile.current_level = start_z;

    /* Re-init the V2 runtime.  This resets the global V2 viewport,
     * smooth state, profile-binding state, and re-registers the
     * default move/turn callbacks on the V1 runtime. */
    dm2_v2_runtime_init(2);
    dm2_v2_runtime_bind_to_v1(&profile);
    return &profile;
}

/* Drive a single V1 tick + one render frame at a given wall-clock
 * timestamp.  This is what the game loop does:
 *   dm2_v1_runtime_tick()         → V1 game state advance
 *   dm2_v2_runtime_v1_tick(now)   → V2 clock + binding detect
 *   dm2_v2_viewport_render_frame(vp, now) → smooth advance by dt
 *
 * In the headless binding test, dm2_v1_runtime_tick() is omitted
 * because the V1 move callbacks aren't exercised here — the binding
 * seam observes the profile at v1_tick and triggers smooth
 * animations.  We call dm2_v2_viewport_render_frame() directly
 * (vs dm2_v2_runtime_render_frame which requires a framebuffer)
 * to keep the test headless.
 *
 * NB: dm2_v2_viewport_render_frame computes dt_ms from the V1 tick
 * boundary (v2_anim_clock_render_frame uses last_v1_tick_ms, set
 * by v1_tick).  Calling v1_tick(N) followed by render_frame(N)
 * gives dt_ms=0 — the smooth state is read at "from" because no
 * time has passed.  This matches the CSB V2 binding test pattern
 * and lets us assert that the animation is at "from" right after
 * the trigger.  Use drive_tick_advance() to push the animation
 * forward by a real wall-clock delta. */
static void drive_tick(uint32_t now_ms) {
    dm2_v2_runtime_v1_tick(now_ms);
    DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
    if (vp) dm2_v2_viewport_render_frame(vp, now_ms);
}

/* drive_tick_advance — like drive_tick but offsets the render_frame
 * timestamp by `dt_ms` so the smooth animation advances by dt_ms.
 * Used to push past the 1-V1-tick animation duration. */
static void drive_tick_advance(uint32_t now_ms, uint32_t dt_ms) {
    dm2_v2_runtime_v1_tick(now_ms);
    DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
    if (vp) dm2_v2_viewport_render_frame(vp, now_ms + dt_ms);
}

/* Get the smooth sub-state from the global viewport.  Used to check
 * whether a walk/turn/stairs animation is in flight. */
static DM2_V2_SmoothState *get_smooth_state(void) {
    DM2_V2_ViewportState *vp = dm2_v2_runtime_get_viewport();
    return vp ? &vp->smooth : NULL;
}

static int smooth_is_active(void) {
    DM2_V2_SmoothState *s = get_smooth_state();
    return s ? dm2_v2_smooth_is_active(s) : 0;
}

/* ── Tests ────────────────────────────────────────────────────────── */

static void test_lifecycle_and_bound_flag(void) {
    printf("\n--- test_lifecycle_and_bound_flag ---\n");
    dm2_v2_runtime_init(2);
    check("init: not bound", !dm2_v2_runtime_is_bound());
    check("init: viewport non-NULL", dm2_v2_runtime_get_viewport() != NULL);

    /* bind/unbind */
    DM2_V1_RuntimeProfile p;
    memset(&p, 0, sizeof(p));
    dm2_v2_runtime_bind_to_v1(&p);
    check("bind: is bound", dm2_v2_runtime_is_bound());

    dm2_v2_runtime_bind_to_v1(NULL);
    check("unbind (NULL): not bound", !dm2_v2_runtime_is_bound());
}

static void test_walk_trigger_on_xy_delta(void) {
    printf("\n--- test_walk_trigger_on_xy_delta ---\n");
    DM2_V1_RuntimeProfile *p = fresh_bind(10, 10, 0, 0);
    check("walk: bound", dm2_v2_runtime_is_bound());
    check("walk: not moving at start", !smooth_is_active());

    /* First tick at t=0: anchors "from" — no animation. */
    drive_tick(0);
    check("walk: not moving after anchor tick", !smooth_is_active());

    /* Move 1 step North: y -= 1. */
    p->party_y = 9;
    drive_tick(55);
    check("walk: moving after xy delta", smooth_is_active());

    /* Walk easing: from=10.0 to=9.0; after 0 ms of animation the
     * value is "from".  The animation will start but the value read
     * here is the cached pre-render value. */
    DM2_V2_SmoothState *s = get_smooth_state();
    checkf("walk: x stays at from (10.0)",
           v2_anim_value(&s->walk.anim_x), 10.0f, 0.01f);
}

static void test_turn_trigger_on_dir_delta(void) {
    printf("\n--- test_turn_trigger_on_dir_delta ---\n");
    DM2_V1_RuntimeProfile *p = fresh_bind(5, 5, 0, 0);
    drive_tick(0);
    check("turn: not moving after anchor tick", !smooth_is_active());

    /* Turn 90° clockwise: dir 0→1 (N→E). */
    p->party_dir = 1;
    drive_tick(55);
    check("turn: moving after dir delta", smooth_is_active());
}

static void test_no_trigger_on_no_change(void) {
    printf("\n--- test_no_trigger_on_no_change ---\n");
    (void)fresh_bind(7, 7, 0, 2);
    drive_tick(0);

    /* No change at all across multiple ticks → no animation. */
    drive_tick(55);
    check("noop: not moving when no V1 delta", !smooth_is_active());
    drive_tick(110);
    check("noop: still not moving on second tick", !smooth_is_active());
    drive_tick(165);
    check("noop: still not moving on third tick", !smooth_is_active());
}

static void test_stairs_trigger_on_z_delta(void) {
    printf("\n--- test_stairs_trigger_on_z_delta ---\n");
    DM2_V1_RuntimeProfile *p = fresh_bind(3, 3, 0, 0);
    drive_tick(0);
    check("stairs: not moving after anchor tick", !smooth_is_active());

    /* current_level change is the stairs event.  DM2 stairs connect
     * dungeon levels (SKULL.ASM T520 stairs handling). */
    p->current_level = -1;
    drive_tick(55);
    check("stairs: moving after current_level delta", smooth_is_active());
    checkf("stairs: vertical at from (0.0)",
           dm2_v2_smooth_get_vertical(get_smooth_state()), 0.0f, 0.01f);
}

static void test_stairs_priority_over_walk(void) {
    printf("\n--- test_stairs_priority_over_walk ---\n");
    DM2_V1_RuntimeProfile *p = fresh_bind(3, 3, 0, 0);
    drive_tick(0);

    /* Both xy AND z change on the same tick — stairs takes priority
     * (SKULL.ASM T520 stairs handling happens before walk on the
     * same tick). */
    p->party_y = 4;
    p->current_level = -1;
    drive_tick(55);
    check("stairs-priority: moving", smooth_is_active());
    /* Stairs path is active, so position reads return stairs anim
     * values (not walk).  At t=0, value is "from". */
    DM2_V2_SmoothState *s = get_smooth_state();
    checkf("stairs-priority: x at from (3.0)",
           v2_anim_value(&s->stairs.anim_x), 3.0f, 0.01f);
    checkf("stairs-priority: y at from (3.0)",
           v2_anim_value(&s->stairs.anim_y), 3.0f, 0.01f);
}

static void test_force_sync_resets_anchor(void) {
    printf("\n--- test_force_sync_resets_anchor ---\n");
    DM2_V1_RuntimeProfile *p = fresh_bind(8, 8, 0, 0);
    drive_tick(0);

    /* Simulate a saved-game load: party jumped from (8,8) to (20,20)
     * but we do NOT want a phantom walk from (8,8) to (20,20). */
    p->party_x = 20;
    p->party_y = 20;
    dm2_v2_runtime_force_sync();
    check("force_sync: not moving immediately", !smooth_is_active());

    /* Subsequent tick with no V1 change must not start a phantom walk. */
    drive_tick(55);
    check("force_sync: no phantom walk on next tick",
          !smooth_is_active());

    /* But a fresh move should still trigger normally. */
    p->party_x = 21;
    dm2_v2_runtime_v1_tick(60);
    check("force_sync: fresh move triggers walk", smooth_is_active());
    (void)p; /* suppress unused-variable warning under -Wextra */
}

static void test_unbind_stops_triggers(void) {
    printf("\n--- test_unbind_stops_triggers ---\n");
    DM2_V1_RuntimeProfile *p = fresh_bind(1, 1, 0, 0);
    drive_tick(0);
    check("unbind: bound", dm2_v2_runtime_is_bound());

    dm2_v2_runtime_bind_to_v1(NULL);
    check("unbind: not bound", !dm2_v2_runtime_is_bound());

    /* Mutate profile — but the binding is gone, so no trigger. */
    p->party_y = 2;
    drive_tick(55);
    check("unbind: no trigger after unbind",
          !smooth_is_active());
}

static void test_manual_api_works_independently(void) {
    printf("\n--- test_manual_api_works_independently ---\n");
    dm2_v2_runtime_init(2);

    /* No V1 profile bound.  Manual API still works. */
    check("manual: not moving at start", !smooth_is_active());

    dm2_v2_runtime_smooth_walk(0.0f, 0.0f, 1.0f, 0.0f);
    check("manual: moving after smooth_walk", smooth_is_active());

    dm2_v2_runtime_smooth_turn(0.0f, 90.0f);
    check("manual: still moving after smooth_turn", smooth_is_active());

    dm2_v2_runtime_smooth_stairs(0.0f, 0.0f, 1.0f, 0.0f, 0.5f);
    check("manual: still moving after smooth_stairs", smooth_is_active());
}

static void test_source_evidence(void) {
    printf("\n--- test_source_evidence ---\n");
    const char *ev = dm2_v2_runtime_source_evidence();
    check("source evidence: non-empty", ev != NULL && strlen(ev) > 10);
    check("source evidence: mentions SKULL.ASM T520",
          strstr(ev, "SKULL.ASM T520") != NULL);
    check("source evidence: mentions SKULL.ASM T048",
          strstr(ev, "SKULL.ASM T048") != NULL);
    check("source evidence: mentions GAMELOOP.C",
          strstr(ev, "GAMELOOP.C") != NULL);
    check("source evidence: mentions DUNGEON.C",
          strstr(ev, "DUNGEON.C") != NULL);
    check("source evidence: mentions binding seam",
          strstr(ev, "binding seam") != NULL);
    check("source evidence: mentions Phase 5",
          strstr(ev, "Phase 5") != NULL);
}

static void test_idempotent_rebind(void) {
    printf("\n--- test_idempotent_rebind ---\n");
    dm2_v2_runtime_init(2);

    static DM2_V1_RuntimeProfile p1, p2;
    memset(&p1, 0, sizeof(p1));
    memset(&p2, 0, sizeof(p2));
    p1.party_x = 5; p1.party_y = 5;
    p2.party_x = 100; p2.party_y = 100;

    dm2_v2_runtime_bind_to_v1(&p1);
    check("rebind: bound to p1", dm2_v2_runtime_is_bound());

    dm2_v2_runtime_bind_to_v1(&p2);
    check("rebind: still bound to p2", dm2_v2_runtime_is_bound());

    /* After rebinding to p2, the anchor should be p2's (100, 100),
     * not p1's (5, 5).  Drive a tick — p1 changes should NOT trigger. */
    p1.party_x = 6;
    drive_tick(0);
    check("rebind: p1 mutation after rebind does not trigger",
          !smooth_is_active());

    /* p2 mutations should now trigger. */
    p2.party_y = 99;
    drive_tick(55);
    check("rebind: p2 mutation after rebind does trigger",
          smooth_is_active());
}

static void test_walk_completes_after_full_v1_tick(void) {
    printf("\n--- test_walk_completes_after_full_v1_tick ---\n");
    DM2_V1_RuntimeProfile *p = fresh_bind(0, 0, 0, 0);
    drive_tick(0);

    p->party_x = 1;
    drive_tick(55);
    check("walk-complete: moving right after trigger",
          smooth_is_active());

    /* Advance animation past completion.  Each drive_tick_advance(N, 16)
     * pushes the smooth animation forward by 16ms (one 60fps frame).
     * After 4 such frames (64ms of animation), the 55ms-duration walk
     * should be complete. */
    drive_tick_advance(110, 16);
    drive_tick_advance(165, 16);
    drive_tick_advance(220, 16);
    drive_tick_advance(275, 16);
    check("walk-complete: not moving after full tick elapsed",
          !smooth_is_active());
}

/* ── Main ────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== DM2 V2 Phase 5 Runtime Binding integration test ===\n");

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
