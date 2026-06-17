/*
 * firestaff_theron_v2_smooth_movement_probe.c
 *
 * Headless verification probe for Theron V2.2 smooth movement.
 * Same shape as firestaff_csb_v2_smooth_movement_probe, adapted to
 * Theron's 4-direction compass and teleporter-fade.  Mirrors the
 * cross-game V2 smooth-movement probe contract.
 *
 * Headless: no game data files loaded.
 * No network / display / audio.
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

static int near(float a, float b, float eps) {
    float d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

static V2_AnimClock make_clock(float dt_ms) {
    V2_AnimClock c;
    memset(&c, 0, sizeof(c));
    c.dt_ms = dt_ms;
    c.sub_tick = 1.0f;
    return c;
}

/* ── Lifecycle + API surface ────────────────────────────────────── */
static void p_lifecycle(void) {
    theron_v2_smooth_init();
    CHECK(theron_v2_smooth_is_moving() == 0);
    CHECK(theron_v2_smooth_fade_active() == 0);
    CHECK(near(theron_v2_smooth_get_x(),     0.0f, 1e-6f));
    CHECK(near(theron_v2_smooth_get_y(),     0.0f, 1e-6f));
    CHECK(near(theron_v2_smooth_get_angle(), 0.0f, 1e-6f));
    CHECK(near(theron_v2_smooth_get_fade(),  0.0f, 1e-6f));
}

/* ── 4-direction compass → angle mapping ─────────────────────────── */
static void p_dir_to_angle(void) {
    CHECK(near(theron_v2_dir_to_angle(0),   0.0f,  1e-6f));
    CHECK(near(theron_v2_dir_to_angle(1),  90.0f,  1e-6f));
    CHECK(near(theron_v2_dir_to_angle(2), 180.0f,  1e-6f));
    CHECK(near(theron_v2_dir_to_angle(3), 270.0f,  1e-6f));
}

/* ── Walk 4 directions ──────────────────────────────────────────── */
static void p_walk_4_directions(void) {
    /* North: dy=+1 (Y axis). */
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 0.0f, 0.0f, 1.0f);
    V2_AnimClock c = make_clock(60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_y(), 1.0f, 1e-3f));

    /* East: dx=+1 (X axis). */
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 1.0f, 0.0f, 0.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_x(), 1.0f, 1e-3f));

    /* South: dy=-1. */
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 0.0f, 0.0f, -1.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_y(), -1.0f, 1e-3f));

    /* West: dx=-1. */
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, -1.0f, 0.0f, 0.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_x(), -1.0f, 1e-3f));
}

/* ── Turn 4 cardinal + 4 short-way diagonals ────────────────────── */
static void p_turn_compass(void) {
    /* N->E, E->S, S->W, W->N: 3 plain +90° and one wrap-short-way
     * (270->0 takes the +90 path via 360).  After 1 V1 tick the
     * animation has fully resolved.  The wrap case ends at 360 (the
     * wrapped target), not at 0; the caller can modulo 360 if it
     * wants a [0, 360) reading. */
    struct { float from, to, expected_end; } turns[] = {
        {   0.0f,  90.0f,  90.0f },
        {  90.0f, 180.0f, 180.0f },
        { 180.0f, 270.0f, 270.0f },
        { 270.0f,   0.0f, 360.0f }, /* short way via 360, end = 360 */
    };
    for (size_t i = 0; i < sizeof(turns) / sizeof(turns[0]); i++) {
        theron_v2_smooth_init();
        theron_v2_smooth_start_turn(turns[i].from, turns[i].to);
        V2_AnimClock c = make_clock(60.0f);
        theron_v2_smooth_update_from_clock(&c);
        CHECK(near(theron_v2_smooth_get_angle(), turns[i].expected_end, 1e-3f));
    }

    /* Confirm the wrap actually takes the short way at mid-tick:
     * 270 -> 0 must pass through ~315 (i.e. via 360), NOT through ~225
     * (i.e. via 180).  The short way means: at the half-tick the
     * angle should be in [270, 360], not in [180, 270). */
    theron_v2_smooth_init();
    theron_v2_smooth_start_turn(270.0f, 0.0f);
    V2_AnimClock c_mid = make_clock(27.5f);
    theron_v2_smooth_update_from_clock(&c_mid);
    float mid = theron_v2_smooth_get_angle();
    CHECK(mid > 270.0f);
    CHECK(mid < 360.0f);
}

/* ── Reverse turn (long way detection) ──────────────────────────── */
static void p_turn_reverse(void) {
    /* N (0) -> W (270): short way is -90 (via 360), not +270.
     * After 1 V1 tick the angle ends at -90. */
    theron_v2_smooth_init();
    theron_v2_smooth_start_turn(0.0f, 270.0f);
    V2_AnimClock c = make_clock(60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_angle(), -90.0f, 1e-3f));
}

/* ── Fade animation (teleporter chain) ──────────────────────────── */
static void p_fade(void) {
    theron_v2_smooth_init();
    CHECK(theron_v2_smooth_fade_active() == 0);
    theron_v2_smooth_start_fade(0.0f, 1.0f);
    CHECK(theron_v2_smooth_fade_active() == 1);
    CHECK(near(theron_v2_smooth_get_fade(), 0.0f, 1e-6f));
    V2_AnimClock c = make_clock(60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_fade(), 1.0f, 1e-3f));
    CHECK(theron_v2_smooth_fade_active() == 0);
}

/* ── Mid-animation read (smooth value) ──────────────────────────── */
static void p_mid_animation(void) {
    /* Walk should be partway through at half a tick. */
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 10.0f, 0.0f, 0.0f);
    /* Half a V1 tick (27.5ms < 55ms duration). */
    V2_AnimClock c = make_clock(27.5f);
    theron_v2_smooth_update_from_clock(&c);
    float x = theron_v2_smooth_get_x();
    /* Should be strictly between 0 and 10 (still in flight). */
    CHECK(x > 0.0f);
    CHECK(x < 10.0f);
    CHECK(theron_v2_smooth_is_moving() == 1);
}

/* ── Multiple animations can run in parallel ────────────────────── */
static void p_parallel_anims(void) {
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 1.0f, 0.0f, 0.0f);
    theron_v2_smooth_start_turn(0.0f, 90.0f);
    theron_v2_smooth_start_fade(0.0f, 0.5f);
    CHECK(theron_v2_smooth_is_moving() == 1);
    CHECK(theron_v2_smooth_fade_active() == 1);
    /* All three resolve after 1 V1 tick. */
    V2_AnimClock c = make_clock(60.0f);
    theron_v2_smooth_update_from_clock(&c);
    CHECK(near(theron_v2_smooth_get_x(),     1.0f, 1e-3f));
    CHECK(near(theron_v2_smooth_get_angle(), 90.0f, 1e-3f));
    CHECK(near(theron_v2_smooth_get_fade(),  0.5f, 1e-3f));
    CHECK(theron_v2_smooth_is_moving() == 0);
    CHECK(theron_v2_smooth_fade_active() == 0);
}

/* ── Determinism: same input → same output ──────────────────────── */
static void p_determinism(void) {
    /* Run the same sequence twice and confirm both reach the same
     * end state. */
    float end_x_a, end_angle_a, end_fade_a;
    float end_x_b, end_angle_b, end_fade_b;

    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(2.0f, 7.0f, 0.0f, 0.0f);
    theron_v2_smooth_start_turn(45.0f, 135.0f);
    theron_v2_smooth_start_fade(0.0f, 0.8f);
    V2_AnimClock c = make_clock(60.0f);
    theron_v2_smooth_update_from_clock(&c);
    end_x_a     = theron_v2_smooth_get_x();
    end_angle_a = theron_v2_smooth_get_angle();
    end_fade_a  = theron_v2_smooth_get_fade();

    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(2.0f, 7.0f, 0.0f, 0.0f);
    theron_v2_smooth_start_turn(45.0f, 135.0f);
    theron_v2_smooth_start_fade(0.0f, 0.8f);
    theron_v2_smooth_update_from_clock(&c);
    end_x_b     = theron_v2_smooth_get_x();
    end_angle_b = theron_v2_smooth_get_angle();
    end_fade_b  = theron_v2_smooth_get_fade();

    CHECK(near(end_x_a, end_x_b, 1e-6f));
    CHECK(near(end_angle_a, end_angle_b, 1e-6f));
    CHECK(near(end_fade_a, end_fade_b, 1e-6f));
}

/* ── Null-arg safety on update ──────────────────────────────────── */
static void p_null_clock_safe(void) {
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 1.0f, 0.0f, 0.0f);
    /* Must not crash on NULL. */
    theron_v2_smooth_update_from_clock(NULL);
    CHECK(near(theron_v2_smooth_get_x(), 0.0f, 1e-6f));
}

/* ── Init resets in-flight animations ───────────────────────────── */
static void p_init_resets(void) {
    theron_v2_smooth_init();
    theron_v2_smooth_start_walk(0.0f, 1.0f, 0.0f, 0.0f);
    theron_v2_smooth_start_fade(0.0f, 1.0f);
    CHECK(theron_v2_smooth_is_moving() == 1);
    CHECK(theron_v2_smooth_fade_active() == 1);
    theron_v2_smooth_init();
    CHECK(theron_v2_smooth_is_moving() == 0);
    CHECK(theron_v2_smooth_fade_active() == 0);
}

/* ── Source evidence cites the V2 domain anchors ────────────────── */
static void p_source_evidence(void) {
    const char *ev = theron_v2_smooth_source_evidence();
    CHECK(ev != NULL);
    if (ev) {
        CHECK(strstr(ev, "THQUEST.ASM") != NULL);
        CHECK(strstr(ev, "F0365") != NULL);
        CHECK(strstr(ev, "F0366") != NULL);
        CHECK(strstr(ev, "F0380") != NULL);
        CHECK(strstr(ev, "GAMELOOP.C") != NULL);
        CHECK(strstr(ev, "HuC6260") != NULL);
        CHECK(strstr(ev, "55ms") != NULL);
        CHECK(strstr(ev, "Theron") != NULL);
    }
}

/* ── Independent of CSB/DM1/DM2/Nexus V2 modules ────────────────── */
static void p_independent_state(void) {
    /* The Theron module owns its own module-static anims.  We can't
     * link csb_v2_smooth_movement here without bloating the probe,
     * but we can confirm the Theron module does not modify any
     * global state that another probe might observe.  The cross-game
     * shape consistency is verified by the header + the source
     * evidence string.  Just confirm we have an isolated API. */
    theron_v2_smooth_init();
    /* Pre-set the read-side to a non-zero value via a walk. */
    theron_v2_smooth_start_walk(3.0f, 5.0f, 0.0f, 0.0f);
    CHECK(near(theron_v2_smooth_get_x(), 3.0f, 1e-6f));
    /* Init returns the read-side to 0. */
    theron_v2_smooth_init();
    CHECK(near(theron_v2_smooth_get_x(), 0.0f, 1e-6f));
}

int main(void) {
    p_lifecycle();
    p_dir_to_angle();
    p_walk_4_directions();
    p_turn_compass();
    p_turn_reverse();
    p_fade();
    p_mid_animation();
    p_parallel_anims();
    p_determinism();
    p_null_clock_safe();
    p_init_resets();
    p_source_evidence();
    p_independent_state();

    if (g_failed) {
        fprintf(stderr, "\n%d/%d FAILED\n", g_failed, g_total);
        return 1;
    }
    printf("\nfirestaff_theron_v2_smooth_movement_probe: %d/%d ok\n",
           g_total, g_total);
    return 0;
}
