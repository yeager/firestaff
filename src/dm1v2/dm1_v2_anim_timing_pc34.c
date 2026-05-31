
/* V2_ANIM_TIMING implementation */
#include "dm1_v2_anim_timing.h"
#include <math.h>

/* ── Clock ─────────────────────────────────────────────────────────── */

void v2_anim_clock_init(V2_AnimClock *clock) {
    if (!clock) return;
    clock->last_v1_tick_ms = 0;
    clock->last_render_ms = 0;
    clock->sub_tick = 0.0f;
    clock->dt_ms = 0.0f;
    clock->v1_tick_pending = 0;
}

void v2_anim_clock_v1_tick(V2_AnimClock *clock, uint32_t now_ms) {
    if (!clock) return;
    clock->last_v1_tick_ms = now_ms;
    clock->sub_tick = 0.0f;
    clock->v1_tick_pending = 1;
}

void v2_anim_clock_render_frame(V2_AnimClock *clock, uint32_t now_ms) {
    if (!clock) return;
    /* dt = wall-clock delta since last render frame.
     * Uses last_render_ms (not last_v1_tick_ms) so that multiple
     * render frames within one V1 tick window each advance the
     * animation by their real frame delta (~16ms at 60fps). */
    float elapsed = (float)(now_ms - clock->last_render_ms);
    clock->sub_tick = elapsed / (float)V1_TICK_MS;
    if (clock->sub_tick > 1.0f) clock->sub_tick = 1.0f;
    clock->dt_ms = elapsed;
    clock->v1_tick_pending = 0;
    clock->last_render_ms = now_ms;
}

float v2_anim_clock_sub_tick(const V2_AnimClock *clock) {
    return clock ? clock->sub_tick : 0.0f;
}

/* ── Easing functions ──────────────────────────────────────────────── */

float v2_ease(V2_EaseType type, float t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;

    switch (type) {
        case V2_EASE_LINEAR:
            return t;

        case V2_EASE_IN_QUAD:
            return t * t;

        case V2_EASE_OUT_QUAD:
            return t * (2.0f - t);

        case V2_EASE_IN_OUT_QUAD:
            if (t < 0.5f) return 2.0f * t * t;
            return -1.0f + (4.0f - 2.0f * t) * t;

        case V2_EASE_OUT_CUBIC: {
            float u = 1.0f - t;
            return 1.0f - u * u * u;
        }

        case V2_EASE_IN_OUT_CUBIC:
            if (t < 0.5f) return 4.0f * t * t * t;
            return 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;

        case V2_EASE_OUT_BACK: {
            /* Slight overshoot for snappy movement feel */
            float c1 = 1.70158f;
            float c3 = c1 + 1.0f;
            float u = t - 1.0f;
            return 1.0f + c3 * u * u * u + c1 * u * u;
        }

        default:
            return t;
    }
}

/* ── Animation control ─────────────────────────────────────────────── */

void v2_anim_start(V2_Anim *a, float from, float to,
    float duration_ms, V2_EaseType ease)
{
    if (!a) return;
    a->from = from;
    a->to = to;
    a->current = from;
    a->duration_ms = duration_ms > 0.0f ? duration_ms : 1.0f;
    a->elapsed_ms = 0.0f;
    a->easing = ease;
    a->active = 1;
    a->loops = 0;
}

void v2_anim_start_v1_tick(V2_Anim *a, float from, float to,
    V2_EaseType ease)
{
    /* Duration = exactly one V1 tick — animation completes in sync */
    v2_anim_start(a, from, to, (float)V1_TICK_MS, ease);
}

void v2_anim_update(V2_Anim *a, float dt_ms) {
    float t;
    if (!a || !a->active) return;
    a->elapsed_ms += dt_ms;
    t = a->elapsed_ms / a->duration_ms;
    if (t >= 1.0f) {
        if (a->loops == 0) {
            a->current = a->to;
            a->active = 0;
            return;
        }
        if (a->loops > 0) a->loops--;
        a->elapsed_ms -= a->duration_ms;
        t = a->elapsed_ms / a->duration_ms;
        /* Swap direction for ping-pong */
        float tmp = a->from;
        a->from = a->to;
        a->to = tmp;
    }
    a->current = a->from + (a->to - a->from) * v2_ease(a->easing, t);
}

float v2_anim_value(const V2_Anim *a) {
    return a ? a->current : 0.0f;
}

int v2_anim_is_done(const V2_Anim *a) {
    return a ? !a->active : 1;
}

/* ── Interpolation helpers ─────────────────────────────────────────── */

float v2_lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float v2_lerp_angle(float a, float b, float t) {
    /* Shortest path around 360 degrees */
    float diff = b - a;
    while (diff > 180.0f) diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;
    return a + diff * t;
}
