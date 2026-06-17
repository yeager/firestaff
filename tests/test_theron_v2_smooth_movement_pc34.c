/*
 * test_theron_v2_smooth_movement_pc34.c
 *
 * Theron V2.2 smooth movement unit test. Mirrors test_csb_v2_smooth_movement
 * with the theron_ prefix and Theron-specific behaviour (4-direction
 * compass, single-axis walk, teleporter fade replaces stairs).
 *
 * Build: see CMakeLists.txt `test_theron_v2_smooth_movement_pc34` target.
 */
#include "theron_v2_smooth_movement.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

static int g_failed = 0;
static int g_total  = 0;

#define CHECK(cond) do { \
    g_total++; \
    if (!(cond)) { \
        g_failed++; \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

/* Approximate float equality within a tolerance. */
static int near(float a, float b, float eps) {
    float d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

static V2_AnimClock make_clock(uint32_t now_ms, float dt_ms) {
    V2_AnimClock c;
    memset(&c, 0, sizeof(c));
    c.dt_ms = dt_ms;
    c.last_v1_tick_ms = now_ms;
    c.sub_tick = 1.0f;
    return c;
}

static void t_init_defaults(void) {
    theron_v2_smooth_init();
    CHECK(theron_v2_smooth_is_moving() == 0);
    CHECK(theron_v2_smooth_fade_active() == 0);
    /* Idle x/y/angle/fade are all 0. */
    CHECK(near(theron_v2_smooth_get_x(),     0.0f, 1e-6f));
    CHECK(near(theron_v2_smooth_get_y(),     0.0f, 1e-6f));
    CHECK(near(theron_v2_smooth_get_angle(), 0.0f, 1e-6f));
    CHECK(near(theron_v2_smooth_get_fade(),  0.0f, 1e-6f));
}

static void t_dir_to_angle(void) {
    /* 4-direction compass mapping. */
    CHECK(near(theron_v2_dir_to_angle(THERON_V2_DIR_N),   0.0f,   1e-6f));
    CHECK(near(theron_v2_dir_to_angle(THERON_V2_DIR_E),   90.0f,  1e-6f));
    CHECK(near(theron_v2_dir_to_angle(THERON_V2_DIR_S), 180.0f,  1e-6f));
    CHECK(near(theron_v2_dir_to_angle(THERON_V2_DIR_W), 270.0f,  1e-6f));
    /* Out-of-range input is masked to 0..3. */
    CHECK(near(theron_v2_dir_to_angle(4),   0.0f, 1e-6f));
    CHECK(near(theron_v2_dir_to_angle(5),  90.0f, 1e-6f));
    CHECK(near(theron_v2_dir_to_angle(-1), 270.0f, 1e-6f));
}

static void t_walk_north(void) {
    /* North = +Y in Theron world, but this module treats (fx,tx,fy,ty)
     * as raw values; the caller picks the axis.  Walk from (0,0) to (0,1). */
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 0.0f, 0.0f, 1.0f);
    CHECK(theron_v2_smooth_is_moving() == 1);
    /* At t=0 the value is the from-value. */
    CHECK(near(theron_v2_smooth_get_y(), 0.0f, 1e-6f));
    /* Advance past the full V1 tick (55ms). */
    V2_AnimClock c = make_clock(0u, 60.0f);
    theron_v2_smooth_update_from_clock(&c);
    /* After 1 V1 tick the walk has fully resolved to the target. */
    CHECK(near(theron_v2_smooth_get_y(), 1.0f, 1e-3f));
    CHECK(theron_v2_smooth_is_moving() == 0);
}

static void t_walk_east_axis(void) {
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(5.0f, 6.0f, 0.0f, 0.0f);
    CHECK(near(theron_v2_smooth_get_x(), 5.0f, 1e-6f));
    V2_AnimClock c = make_clock(0u, 60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_x(), 6.0f, 1e-3f));
}

static void t_walk_no_diagonal(void) {
    /* Confirm that walk uses a single axis: setting both axes to non-zero
     * values is allowed, but each axis animates independently.  The Theron
     * caller is responsible for using one axis at a time. */
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 3.0f, 0.0f, 0.0f);
    V2_AnimClock c = make_clock(0u, 60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_x(), 3.0f, 1e-3f));
    CHECK(near(theron_v2_smooth_get_y(), 0.0f, 1e-3f));
}

static void t_turn_short_way(void) {
    /* N (0) -> E (90): +90° (short way, no wrap). */
    theron_v2_smooth_init();
    theron_v2_smooth_start_turn(0.0f, 90.0f);
    CHECK(near(theron_v2_smooth_get_angle(), 0.0f, 1e-6f));
    V2_AnimClock c = make_clock(0u, 60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_angle(), 90.0f, 1e-3f));
}

static void t_turn_wrap_short_way(void) {
    /* W (270) -> N (0): the short way is +90 (via 360), not -270.
     * The anim stores from=270, to=360 (wrapped) so that v2_anim_value
     * reads in the wrapped range; V2 presentation can modulo 360 if
     * it wants a [0, 360) reading.  End value after 1 V1 tick: 360. */
    theron_v2_smooth_init();
    theron_v2_smooth_start_turn(270.0f, 0.0f);
    CHECK(near(theron_v2_smooth_get_angle(), 270.0f, 1e-6f));
    V2_AnimClock c = make_clock(0u, 60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_angle(), 360.0f, 1e-3f));
    /* At the half-way tick (27.5ms), the angle should be mid-way
     * between 270 and 360 (i.e. near 315) since the short way goes
     * via 360, not via 180. */
    theron_v2_smooth_init();
    theron_v2_smooth_start_turn(270.0f, 0.0f);
    V2_AnimClock c_mid = make_clock(0u, 27.5f);
    theron_v2_smooth_update_from_clock(&c_mid);
    float mid = theron_v2_smooth_get_angle();
    CHECK(mid > 270.0f);
    CHECK(mid < 360.0f);
}

static void t_turn_reverse_long_way(void) {
    /* N (0) -> W (270): the short way is -90 (via 360), not +270.
     * After 1 V1 tick the angle should be at -90. */
    theron_v2_smooth_init();
    theron_v2_smooth_start_turn(0.0f, 270.0f);
    CHECK(near(theron_v2_smooth_get_angle(), 0.0f, 1e-6f));
    V2_AnimClock c = make_clock(0u, 60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_angle(), -90.0f, 1e-3f));
}

static void t_turn_negative_inputs(void) {
    /* Negative angles normalise to [0, 360) for the from-value, and
     * the shortest-path wrap logic still picks the short way.  Here
     * -90 → 0 normalises to 270 → 0, which is the wrap-short-way case
     * (via 360).  End value is the wrapped target 360. */
    theron_v2_smooth_init();
    theron_v2_smooth_start_turn(-90.0f, 0.0f);
    V2_AnimClock c = make_clock(0u, 60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_angle(), 360.0f, 1e-3f));

    /* Symmetric case: -90 -> -180 normalises to 270 -> 180, which is
     * a -90 short-way turn with no wrap.  End value = 180 (normalised). */
    theron_v2_smooth_init();
    theron_v2_smooth_start_turn(-90.0f, -180.0f);
    V2_AnimClock c2 = make_clock(0u, 60.0f);
    theron_v2_smooth_update_from_clock(&c2);
    CHECK(near(theron_v2_smooth_get_angle(), 180.0f, 1e-3f));
}

static void t_fade_in_out(void) {
    theron_v2_smooth_init();
    CHECK(theron_v2_smooth_fade_active() == 0);
    /* Fade in: 0 (visible) -> 1 (faded).  Used by teleporter chain. */
    theron_v2_smooth_start_fade(0.0f, 1.0f);
    CHECK(theron_v2_smooth_fade_active() == 1);
    CHECK(near(theron_v2_smooth_get_fade(), 0.0f, 1e-6f));
    V2_AnimClock c = make_clock(0u, 60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_fade(), 1.0f, 1e-3f));
    CHECK(theron_v2_smooth_fade_active() == 0);
}

static void t_is_moving_combined(void) {
    /* Walk + fade simultaneously both contribute to is_moving. */
    theron_v2_smooth_init();
    CHECK(theron_v2_smooth_is_moving() == 0);
    theron_v2_smooth_start_walk(0.0f, 1.0f, 0.0f, 0.0f);
    CHECK(theron_v2_smooth_is_moving() == 1);
    V2_AnimClock c = make_clock(0u, 60.0f);
    theron_v2_smooth_update_from_clock(&c);
    /* Walk is done. */
    CHECK(theron_v2_smooth_is_moving() == 0);
    /* Now start a fade. */
    theron_v2_smooth_start_fade(0.0f, 0.5f);
    CHECK(theron_v2_smooth_is_moving() == 1);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(theron_v2_smooth_is_moving() == 0);
}

static void t_update_null_clock(void) {
    /* NULL clock is a no-op (defensive guard). */
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 1.0f, 0.0f, 0.0f);
    theron_v2_smooth_update_from_clock(NULL);
    /* Value should still be at the from (no update happened). */
    CHECK(near(theron_v2_smooth_get_x(), 0.0f, 1e-6f));
}

static void t_idempotent_init(void) {
    /* Calling init while an animation is in flight resets the state. */
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 1.0f, 0.0f, 0.0f);
    CHECK(theron_v2_smooth_is_moving() == 1);
    theron_v2_smooth_init();
    CHECK(theron_v2_smooth_is_moving() == 0);
    CHECK(near(theron_v2_smooth_get_x(), 0.0f, 1e-6f));
}

static void t_source_evidence_non_null(void) {
    const char *ev = theron_v2_smooth_source_evidence();
    CHECK(ev != NULL);
    /* Must mention Theron-specific anchors. */
    if (ev) {
        CHECK(strstr(ev, "THQUEST.ASM") != NULL);
        CHECK(strstr(ev, "T520") != NULL);
        CHECK(strstr(ev, "F0365") != NULL);
        CHECK(strstr(ev, "F0366") != NULL);
        CHECK(strstr(ev, "HuC6260") != NULL);
        CHECK(strstr(ev, "F0380") != NULL);
    }
}

int main(void) {
    t_init_defaults();
    t_dir_to_angle();
    t_walk_north();
    t_walk_east_axis();
    t_walk_no_diagonal();
    t_turn_short_way();
    t_turn_wrap_short_way();
    t_turn_reverse_long_way();
    t_turn_negative_inputs();
    t_fade_in_out();
    t_is_moving_combined();
    t_update_null_clock();
    t_idempotent_init();
    t_source_evidence_non_null();

    if (g_failed) {
        fprintf(stderr, "\n%d/%d FAILED\n", g_failed, g_total);
        return 1;
    }
    printf("\ntest_theron_v2_smooth_movement_pc34: %d/%d ok\n",
           g_total, g_total);
    return 0;
}
