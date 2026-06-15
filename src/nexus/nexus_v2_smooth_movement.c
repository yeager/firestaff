/*
 * nexus_v2_smooth_movement.c — Nexus V2 Smooth Movement
 *
 * Phase 5: Smooth movement and viewport interpolation for Nexus V2.
 *
 * V1 movement is instant (one tick = new position).
 * V2 interpolates between positions over exactly 1 V1 tick (55ms):
 *   - Walk: ease-out cubic (X and Y independently)
 *   - Turn: ease-out quad  (angle, shortest path)
 *   - Stairs: ease-in-out cubic + vertical camera offset
 *
 * The interpolation is purely visual — game state updates instantly
 * as in V1, but the camera smoothly transitions.
 *
 * Uses V2_Anim / V2_AnimClock from dm1_v2_anim_timing.h (shared,
 * game-agnostic).  The DM2 V2 smooth movement implementation in
 * dm2_v2_smooth_movement.c is the reference pattern.
 *
 * Key invariant: game state ONLY advances on V1 ticks.
 * Visual state interpolates between previous and current V1 state.
 *
 * Source: DMDF spec — camera/party world model
 *         nexus_v1_viewport.c — camera setup (nexus_camera_init)
 *         nexus_v1_movement.c — party position update
 *         DM2 V2 smooth: dm2_v2_smooth_movement.c (pass ad2dc2c8)
 */

#include "nexus_v2_smooth_movement.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ── Lifecycle ─────────────────────────────────────────────────────── */

void nexus_v2_smooth_init(Nexus_V2_SmoothState *s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
}

/* ── Walk ──────────────────────────────────────────────────────────── */

void nexus_v2_smooth_start_walk(Nexus_V2_SmoothState *s,
                                float from_x, float from_y,
                                float to_x,   float to_y) {
    if (!s) return;
    /* Ease-out cubic: snappy start, gentle land */
    v2_anim_start_v1_tick(&s->walk.anim_x, from_x, to_x, V2_EASE_OUT_CUBIC);
    v2_anim_start_v1_tick(&s->walk.anim_y, from_y, to_y, V2_EASE_OUT_CUBIC);
    s->walk.active = 1;
}

/* ── Turn ─────────────────────────────────────────────────────────── */

void nexus_v2_smooth_start_turn(Nexus_V2_SmoothState *s,
                               float from_angle, float to_angle) {
    if (!s) return;
    /* Normalise to [0, 360) before storing */
    float fa = fmodf(from_angle, 360.0f);
    float ta = fmodf(to_angle,   360.0f);
    if (fa < 0.0f) fa += 360.0f;
    if (ta < 0.0f) ta += 360.0f;

    /* Find shortest path (0-360, wrap-around) */
    float diff = ta - fa;
    if (diff >  180.0f) diff -= 360.0f;
    if (diff < -180.0f) diff += 360.0f;

    /* ease-out quad for turns — snappy but not abrupt */
    v2_anim_start_v1_tick(&s->turn.anim_angle, fa, fa + diff, V2_EASE_OUT_QUAD);
    s->turn.active = 1;
}

/* ── Stairs ───────────────────────────────────────────────────────── */

void nexus_v2_smooth_start_stairs(Nexus_V2_SmoothState *s,
                                  float from_x, float from_y,
                                  float to_x,   float to_y,
                                  float from_vert, float to_vert) {
    if (!s) return;
    v2_anim_start_v1_tick(&s->stairs.anim_x,    from_x,    to_x,    V2_EASE_IN_OUT_CUBIC);
    v2_anim_start_v1_tick(&s->stairs.anim_y,    from_y,    to_y,    V2_EASE_IN_OUT_CUBIC);
    v2_anim_start_v1_tick(&s->stairs.anim_vert, from_vert, to_vert, V2_EASE_IN_OUT_CUBIC);
    s->stairs.active = 1;
}

/* ── Update ─────────────────────────────────────────────────────────── */

void nexus_v2_smooth_update(Nexus_V2_SmoothState *s, float dt_ms) {
    if (!s) return;

    if (s->walk.active) {
        v2_anim_update(&s->walk.anim_x, dt_ms);
        v2_anim_update(&s->walk.anim_y, dt_ms);
        if (v2_anim_is_done(&s->walk.anim_x) &&
            v2_anim_is_done(&s->walk.anim_y))
            s->walk.active = 0;
    }

    if (s->turn.active) {
        v2_anim_update(&s->turn.anim_angle, dt_ms);
        if (v2_anim_is_done(&s->turn.anim_angle))
            s->turn.active = 0;
    }

    if (s->stairs.active) {
        v2_anim_update(&s->stairs.anim_x,     dt_ms);
        v2_anim_update(&s->stairs.anim_y,     dt_ms);
        v2_anim_update(&s->stairs.anim_vert,  dt_ms);
        if (v2_anim_is_done(&s->stairs.anim_x) &&
            v2_anim_is_done(&s->stairs.anim_y) &&
            v2_anim_is_done(&s->stairs.anim_vert))
            s->stairs.active = 0;
    }
}

/* ── V1 tick record ─────────────────────────────────────────────────── */

void nexus_v2_smooth_tick(Nexus_V2_SmoothState *s,
                         float game_x, float game_y, float game_angle) {
    if (!s) return;

    /* First call after init: just record the baseline state, no animation.
     * We can't tell "started at origin" apart from "delta is 0" without
     * a has_prev flag, so we keep one explicitly. */
    if (!s->has_prev) {
        s->prev_x = game_x;
        s->prev_y = game_y;
        s->prev_angle = game_angle;
        s->has_prev = 1;
        return;
    }

    /* Detect position delta → start a walk animation from previous
     * game state to current game state. Stairs animation, if active,
     * is the higher-priority visual transition; the caller is expected
     * to invoke nexus_v2_smooth_start_stairs() instead of letting the
     * tick auto-start a walk in that case. */
    if (game_x != s->prev_x || game_y != s->prev_y) {
        v2_anim_start_v1_tick(&s->walk.anim_x, s->prev_x, game_x, V2_EASE_OUT_CUBIC);
        v2_anim_start_v1_tick(&s->walk.anim_y, s->prev_y, game_y, V2_EASE_OUT_CUBIC);
        s->walk.active = 1;
    }

    /* Detect angle delta → start a turn animation. Normalize to
     * [0, 360) and pick the shortest path (the same shortest-path
     * logic used by nexus_v2_smooth_start_turn, kept in sync). */
    if (game_angle != s->prev_angle) {
        float fa = s->prev_angle;
        float ta = game_angle;
        while (fa < 0.0f)    fa += 360.0f;
        while (fa >= 360.0f) fa -= 360.0f;
        while (ta < 0.0f)    ta += 360.0f;
        while (ta >= 360.0f) ta -= 360.0f;
        float diff = ta - fa;
        if (diff >  180.0f) diff -= 360.0f;
        if (diff < -180.0f) diff += 360.0f;
        v2_anim_start_v1_tick(&s->turn.anim_angle, fa, fa + diff, V2_EASE_OUT_QUAD);
        s->turn.active = 1;
    }

    /* Record the new baseline. */
    s->prev_x = game_x;
    s->prev_y = game_y;
    s->prev_angle = game_angle;
}

/* ── Query ─────────────────────────────────────────────────────────── */

void nexus_v2_smooth_get_position(const Nexus_V2_SmoothState *s,
                                 float *out_x, float *out_y) {
    if (!s || !out_x || !out_y) return;

    if (s->stairs.active) {
        *out_x = v2_anim_value(&s->stairs.anim_x);
        *out_y = v2_anim_value(&s->stairs.anim_y);
    } else if (s->walk.active) {
        *out_x = v2_anim_value(&s->walk.anim_x);
        *out_y = v2_anim_value(&s->walk.anim_y);
    } else {
        /* No active animation — return snapped position (walk.to or stairs.to) */
        *out_x = v2_anim_value(&s->walk.anim_x);
        *out_y = v2_anim_value(&s->walk.anim_y);
    }
}

float nexus_v2_smooth_get_vertical(const Nexus_V2_SmoothState *s) {
    if (!s) return 0.0f;
    if (!s->stairs.active) return 0.0f;
    return v2_anim_value(&s->stairs.anim_vert);
}

float nexus_v2_smooth_get_angle(const Nexus_V2_SmoothState *s) {
    if (!s) return 0.0f;
    return v2_anim_value(&s->turn.anim_angle);
}

int nexus_v2_smooth_is_active(const Nexus_V2_SmoothState *s) {
    if (!s) return 0;
    return s->walk.active || s->turn.active || s->stairs.active;
}