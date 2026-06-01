/* test_dm1_v2_anim_timing_pc34.c
 * Unit tests for dm1_v2_anim_timing_pc34 — V1-synchronous interpolation
 * and easing functions used by the V2 smooth movement layer.
 *
 * Source-lock: ReDMCSB VBLANK.C M526_WaitVerticalBlank,
 * GAMELOOP.C:47-50 input wait = 12 VBlanks (55ms/tick). */

#include "dm1_v2_anim_timing.h"
#include <stdio.h>

static int failures = 0;
#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

int main(void) {
    /* ── V2_Anim one-shot (55ms duration) ─────────────────────────── */
    V2_Anim a;
    v2_anim_start(&a, 0.0f, 1.0f, 55.0f, V2_EASE_LINEAR);
    CHECK(v2_anim_is_done(&a) == 0);
    v2_anim_update(&a, 27.5f);
    CHECK(v2_anim_value(&a) > 0.49f && v2_anim_value(&a) < 0.51f);
    v2_anim_update(&a, 27.5f);
    CHECK(v2_anim_is_done(&a) == 1);
    CHECK(v2_anim_value(&a) == 1.0f);

    /* Clamp: overshooting snaps to end */
    V2_Anim b;
    v2_anim_start(&b, 0.0f, 1.0f, 55.0f, V2_EASE_LINEAR);
    v2_anim_update(&b, 100.0f);
    CHECK(v2_anim_is_done(&b) == 1);
    CHECK(v2_anim_value(&b) == 1.0f);

    /* v2_anim_start_v1_tick uses exactly V1_TICK_MS */
    V2_Anim c;
    v2_anim_start_v1_tick(&c, 0.0f, 1.0f, V2_EASE_OUT_CUBIC);
    CHECK(c.duration_ms == (float)V1_TICK_MS);
    CHECK(c.active == 1);
    v2_anim_update(&c, 27.5f);
    CHECK(v2_anim_value(&c) > 0.0f && v2_anim_value(&c) < 1.0f);
    v2_anim_update(&c, 27.5f);
    CHECK(v2_anim_is_done(&c) == 1);

    /* ── V2_Anim loops ────────────────────────────────────────────── */
    V2_Anim d;
    v2_anim_start(&d, 0.0f, 1.0f, 10.0f, V2_EASE_LINEAR);
    d.loops = 2;
    v2_anim_update(&d, 100.0f);
    CHECK(d.active == 1);
    CHECK(d.loops == 1);
    CHECK(d.from == 1.0f && d.to == 0.0f);

    /* ── Easing edge cases ────────────────────────────────────────── */
    CHECK(v2_ease(V2_EASE_LINEAR, 0.0f) == 0.0f);
    CHECK(v2_ease(V2_EASE_LINEAR, 1.0f) == 1.0f);
    CHECK(v2_ease(V2_EASE_LINEAR, 0.5f) == 0.5f);
    CHECK(v2_ease(V2_EASE_OUT_CUBIC, 1.0f) == 1.0f);
    CHECK(v2_ease(V2_EASE_IN_QUAD, 0.0f) == 0.0f);
    CHECK(v2_ease(V2_EASE_OUT_QUAD, 1.0f) == 1.0f);
    CHECK(v2_ease(V2_EASE_IN_OUT_QUAD, 1.0f) == 1.0f);
    CHECK(v2_ease(V2_EASE_IN_OUT_CUBIC, 1.0f) == 1.0f);

    /* ── V2_AnimClock (V1_TICK_MS=55) ───────────────────────────────
     * v2_anim_clock_render_frame measures dt from last render frame.
     * After v1_tick(1000): last_render_ms synced to 1000 (pass602a fix).
     * render(1016): elapsed=16ms, sub_tick=16/55≈0.291
     * render(1033): elapsed=17ms, sub_tick=17/55≈0.309
     * render(1070): elapsed=37ms, sub_tick=37/55≈0.673
     * render(1172): elapsed=56ms→clamp to 1.0 */
    V2_AnimClock clock;
    v2_anim_clock_init(&clock);
    CHECK(clock.sub_tick == 0.0f);
    CHECK(clock.v1_tick_pending == 0);

    v2_anim_clock_v1_tick(&clock, 1000);
    CHECK(clock.last_v1_tick_ms == 1000);
    CHECK(clock.v1_tick_pending == 1);

    v2_anim_clock_render_frame(&clock, 1016);
    CHECK(clock.v1_tick_pending == 0);
    CHECK(clock.sub_tick > 0.28f && clock.sub_tick < 0.32f);

    v2_anim_clock_render_frame(&clock, 1033);
    /* 17ms since last render → 17/55 ≈ 0.309 */
    CHECK(clock.sub_tick > 0.29f && clock.sub_tick < 0.33f);

    v2_anim_clock_render_frame(&clock, 1070);
    /* 37ms since last render → 37/55 ≈ 0.673 */
    CHECK(clock.sub_tick > 0.65f && clock.sub_tick < 0.70f);

    v2_anim_clock_render_frame(&clock, 1172);
    /* 56ms since last render → clamp to 1.0 */
    CHECK(clock.sub_tick >= 1.0f);

    /* ── v2_lerp / v2_lerp_angle ─────────────────────────────────── */
    CHECK(v2_lerp(0.0f, 100.0f, 0.5f) == 50.0f);
    CHECK(v2_lerp_angle(0.0f, 90.0f, 0.5f) == 45.0f);
    /* pass602a: result normalized to [0,360); short path 350→10 via +20 */
    CHECK(v2_lerp_angle(350.0f, 10.0f, 0.5f) == 0.0f);
    CHECK(v2_lerp_angle(10.0f, 350.0f, 0.5f) == 0.0f);
    /* No wrap needed: straight paths */
    CHECK(v2_lerp_angle(0.0f, 180.0f, 0.5f) == 90.0f);
    CHECK(v2_lerp_angle(0.0f, 270.0f, 0.5f) == 315.0f);  /* via -90° short path */

    /* ── Easing monotonicity ─────────────────────────────────────── */
    {
        float prev = 0.0f;
        int monotonic_ok = 1;
        for (int i = 0; i <= 20; i++) {
            float t = (float)i / 20.0f;
            float e = v2_ease(V2_EASE_OUT_CUBIC, t);
            if (e < prev) monotonic_ok = 0;
            prev = e;
        }
        CHECK(monotonic_ok == 1);
    }

    if (failures == 0) {
        printf("dm1_v2_anim_timing_pc34: ok\n");
    }
    return failures;
}
