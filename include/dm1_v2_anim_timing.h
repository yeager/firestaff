
#ifndef FIRESTAFF_DM1_V2_ANIM_TIMING_H
#define FIRESTAFF_DM1_V2_ANIM_TIMING_H

#include <stdint.h>

/* ══════════════════════════════════════════════════════════════════════
 * V2 Animation Timing — smooth visuals with exact V1 timing
 *
 * V1 game logic runs at the original tick rate (VBLANK-locked).
 * V2 renderer interpolates between V1 states at display refresh rate.
 *
 * Key invariant: game state ONLY advances on V1 ticks.
 * Visual state interpolates between previous and current V1 state.
 *
 * Source: ReDMCSB VBLANK.C M526_WaitVerticalBlank
 *         ReDMCSB GAMELOOP.C:47-50 input wait = 12 VBlanks
 * ══════════════════════════════════════════════════════════════════════ */

/* V1 tick rate: 18.2 Hz on Amiga (55ms), PC varies by VBlank.
 * We use 55ms as the canonical tick duration. */
#define V1_TICK_MS 55
#define V1_TICKS_PER_SECOND 18

/* Easing functions — all take t in [0,1], return [0,1] */
typedef enum {
    V2_EASE_LINEAR = 0,
    V2_EASE_IN_QUAD,
    V2_EASE_OUT_QUAD,
    V2_EASE_IN_OUT_QUAD,
    V2_EASE_OUT_CUBIC,
    V2_EASE_IN_OUT_CUBIC,
    V2_EASE_OUT_BACK,    /* slight overshoot for snappy feel */
    V2_EASE_COUNT
} V2_EaseType;

/* Animation state for any interpolated value */
typedef struct {
    float from;          /* value at start of transition */
    float to;            /* value at end of transition */
    float current;       /* interpolated value */
    float duration_ms;   /* transition time in ms (= V1_TICK_MS for 1-tick) */
    float elapsed_ms;    /* time elapsed */
    V2_EaseType easing;
    int active;
    int loops;           /* 0 = one-shot, -1 = infinite, N = count */
} V2_Anim;

/* Global sub-tick interpolation factor */
typedef struct {
    uint32_t last_v1_tick_ms;   /* timestamp of last V1 tick */
    uint32_t last_render_ms;    /* timestamp of last render frame */
    float sub_tick;             /* 0.0 to 1.0 within current V1 tick */
    float dt_ms;                /* ms since last render frame */
    int v1_tick_pending;        /* V1 state just advanced */
} V2_AnimClock;

/* Clock */
void v2_anim_clock_init(V2_AnimClock *clock);
void v2_anim_clock_v1_tick(V2_AnimClock *clock, uint32_t now_ms);
void v2_anim_clock_render_frame(V2_AnimClock *clock, uint32_t now_ms);
float v2_anim_clock_sub_tick(const V2_AnimClock *clock);

/* Easing */
float v2_ease(V2_EaseType type, float t);

/* Animation control */
void v2_anim_start(V2_Anim *a, float from, float to, float duration_ms, V2_EaseType ease);
void v2_anim_start_v1_tick(V2_Anim *a, float from, float to, V2_EaseType ease);
void v2_anim_update(V2_Anim *a, float dt_ms);
float v2_anim_value(const V2_Anim *a);
int v2_anim_is_done(const V2_Anim *a);

/* Convenience: interpolate between two states using sub-tick */
float v2_lerp(float a, float b, float t);
float v2_lerp_angle(float a, float b, float t); /* handles 360 wrap */

#endif
