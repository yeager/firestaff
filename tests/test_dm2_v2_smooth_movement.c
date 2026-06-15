/* DM2 V2 Phase 5 Smooth Movement — smoke test
 *
 * Tests that the smooth movement implementation initialises,
 * drives animations, and produces deterministic interpolated
 * positions without any game data or SDL.
 *
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *            dm2_v2_smooth_movement.c (DM2 V2 Phase 5)
 * Key invariant: game state ONLY advances on V1 ticks.
 *
 * DM2 V2 smooth is struct-based (DM2_V2_SmoothState), like Nexus V2
 * but without tick-driven auto-detection — animations are explicitly
 * started with from/to positions, then advanced via dm2_v2_smooth_tick.
 */

#include "dm2_v2_smooth_movement.h"
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

/* sp_mid — mirror dm2_v2_smooth_start_turn's shortest-path
 * normalization to compute the expected mid-tick angle. */
static float sp_mid(float from, float to, float t) {
    float fa = from, ta = to;
    while (fa < 0.0f)    fa += 360.0f;
    while (fa >= 360.0f) fa -= 360.0f;
    while (ta < 0.0f)    ta += 360.0f;
    while (ta >= 360.0f) ta -= 360.0f;
    float diff = ta - fa;
    if (diff >  180.0f) ta -= 360.0f;
    if (diff < -180.0f) ta += 360.0f;
    return fa + (ta - fa) * t;
}

int main(void) {
    printf("=== DM2 V2 Phase 5 Smooth Movement smoke test ===\n\n");

    /* ── Init ───────────────────────────────────────────────────── */
    DM2_V2_SmoothState state;
    dm2_v2_smooth_init(&state);
    check("init: smooth inactive", !dm2_v2_smooth_is_active(&state));

    /* ── Walk animation N/E/S/W ─────────────────────────────────── */
    /* ease-out-cubic at t=0.5: progress = 1 - (1-0.5)^3 = 0.875 */
    struct {
        const char *name;
        float fx, fy, tx, ty;
    } walk_cases[] = {
        {"North", 10.0f, 10.0f, 10.0f,  9.0f},
        {"East",  10.0f, 10.0f, 11.0f, 10.0f},
        {"South", 10.0f, 10.0f, 10.0f, 11.0f},
        {"West",  10.0f, 10.0f,  9.0f, 10.0f},
    };
    for (size_t i = 0; i < sizeof(walk_cases) / sizeof(walk_cases[0]); i++) {
        dm2_v2_smooth_init(&state);
        dm2_v2_smooth_start_walk(&state,
            walk_cases[i].fx, walk_cases[i].fy,
            walk_cases[i].tx, walk_cases[i].ty);
        check("walk: active after start", state.walk.active);
        check("walk: is_active true", dm2_v2_smooth_is_active(&state));

        /* Advance half a V1 tick — ease-out-cubic at t=0.5 → 0.875 */
        dm2_v2_smooth_tick(&state, 55.0f * 0.5f);
        float px, py;
        dm2_v2_smooth_get_position(&state, &px, &py);
        float fx_step = walk_cases[i].tx - walk_cases[i].fx;
        float fy_step = walk_cases[i].ty - walk_cases[i].fy;
        checkf("walk X at t=0.5 ease-out-cubic",
               px, walk_cases[i].fx + fx_step * 0.875f, 0.01f);
        checkf("walk Y at t=0.5 ease-out-cubic",
               py, walk_cases[i].fy + fy_step * 0.875f, 0.01f);

        /* Advance full V1 tick → animation done */
        dm2_v2_smooth_tick(&state, 55.0f * 0.6f);
        check("walk: inactive after full tick", !state.walk.active);
        check("walk: is_active false after complete",
              !dm2_v2_smooth_is_active(&state));
    }

    /* ── Turn animation 8 directions ───────────────────────────── */
    /* ease-out-quad at t=0.5: progress = 1 - (1-0.5)^2 = 0.75.
     * DM2 V2 normalises via shortest-path (sp_mid declared above). */
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
        dm2_v2_smooth_init(&state);
        dm2_v2_smooth_start_turn(&state, turn_cases[i].from, turn_cases[i].to);
        check("turn: active after start", state.turn.active);
        check("turn: is_turning true", dm2_v2_smooth_is_turning(&state));

        dm2_v2_smooth_tick(&state, 55.0f * 0.5f);
        float angle = dm2_v2_smooth_get_angle(&state);
        float expected = sp_mid(turn_cases[i].from, turn_cases[i].to, 0.75f);
        checkf("turn angle at t=0.5 ease-out-quad", angle, expected, 0.5f);

        dm2_v2_smooth_tick(&state, 55.0f * 0.6f);
        check("turn: inactive after full tick", !state.turn.active);
        check("turn: is_turning false", !dm2_v2_smooth_is_turning(&state));
    }

    /* ── Shortest path turn (wrap-around) ──────────────────────── */
    /* 350° → 10°: shortest-path normalises to from=350, to=10+360=370,
     * mid (t=0.75) = 350 + 20*0.75 = 365 → 5° mod 360. */
    dm2_v2_smooth_init(&state);
    dm2_v2_smooth_start_turn(&state, 350.0f, 10.0f);
    dm2_v2_smooth_tick(&state, 55.0f * 0.5f);
    float angle = dm2_v2_smooth_get_angle(&state);
    float wrap_expected = sp_mid(350.0f, 10.0f, 0.75f);
    checkf("wrap turn mid (shortest-path forward)",
           angle, wrap_expected, 0.5f);

    /* ── Stairs animation with vertical offset ─────────────────── */
    dm2_v2_smooth_init(&state);
    dm2_v2_smooth_start_stairs(&state, 5.0f, 5.0f, 6.0f, 5.0f, 0.5f);
    check("stairs: active after start", state.stairs.active);
    checkf("stairs vertical at start (0.0)", dm2_v2_smooth_get_vertical(&state), 0.0f, 0.001f);

    /* ease-in-out-cubic at t=0.5: position progress = 0.5, vert progress = 0.25 */
    dm2_v2_smooth_tick(&state, 55.0f * 0.5f);
    float px, py;
    dm2_v2_smooth_get_position(&state, &px, &py);
    checkf("stairs X at t=0.5 (≈5.5)", px, 5.5f, 0.01f);
    checkf("stairs Y at t=0.5 (≈5.0)", py, 5.0f, 0.01f);
    checkf("stairs vert at t=0.5 (≈0.25)", dm2_v2_smooth_get_vertical(&state), 0.25f, 0.01f);

    dm2_v2_smooth_tick(&state, 55.0f * 0.6f);
    check("stairs: inactive after full tick", !state.stairs.active);
    /* DM2 V2 returns 0.0 from get_vertical once stairs.active=0
     * (caller should hold the camera at the final offset via state
     * outside smooth). The internal anim_vert current is 0.5 but
     * it is masked by the !active guard. */
    checkf("stairs vert after full tick (returns 0.0 when inactive)",
           dm2_v2_smooth_get_vertical(&state), 0.0f, 0.01f);

    /* ── Source evidence ────────────────────────────────────────── */
    const char *ev = dm2_v2_smooth_source_evidence();
    check("source evidence: non-empty", ev != NULL && strlen(ev) > 10);

    /* ── Pipeline integration (inline struct check) ─────────────── */
    DM2_V2_SmoothState ps;
    memset(&ps, 0, sizeof(ps));
    check("pipeline smooth: zero-init inactive", !dm2_v2_smooth_is_active(&ps));

    /* Walk sub-struct accessible */
    ps.walk.active = 1;
    check("pipeline smooth: walk.active writable", ps.walk.active == 1);
    ps.walk.active = 0;

    /* Turn sub-struct accessible */
    ps.turn.active = 1;
    check("pipeline smooth: turn.active writable", ps.turn.active == 1);
    ps.turn.active = 0;

    /* Stairs sub-struct accessible */
    ps.stairs.active = 1;
    check("pipeline smooth: stairs.active writable", ps.stairs.active == 1);
    ps.stairs.active = 0;

    /* ── Null safety ─────────────────────────────────────────────── */
    dm2_v2_smooth_init(NULL);
    dm2_v2_smooth_start_walk(NULL, 0, 0, 1, 1);
    dm2_v2_smooth_start_turn(NULL, 0, 90);
    dm2_v2_smooth_start_stairs(NULL, 0, 0, 1, 1, 0.5f);
    dm2_v2_smooth_tick(NULL, 16.0f);
    dm2_v2_smooth_get_position(NULL, &px, &py);
    (void)dm2_v2_smooth_get_vertical(NULL);
    (void)dm2_v2_smooth_get_angle(NULL);
    (void)dm2_v2_smooth_is_active(NULL);
    (void)dm2_v2_smooth_is_turning(NULL);
    check("null pointers: no crash", 1);

    /* ── Result ─────────────────────────────────────────────────── */
    printf("\n=== Results: %d passed, %d failed ===\n",
        s_tests_passed, s_tests_failed);
    return s_tests_failed > 0 ? 1 : 0;
}
