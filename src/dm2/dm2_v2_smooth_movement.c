/*
 * dm2_v2_smooth_movement.c — DM2 V2 Smooth Movement
 *
 * Phase 5: Smooth movement and viewport interpolation for DM2 V2.
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
 * game-agnostic).  The DM1 V2.2 smooth movement implementation in
 * dm1_v2_smooth_movement_pc34.c is the reference pattern.
 *
 * Key invariant: game state ONLY advances on V1 ticks.
 * Visual state interpolates between previous and current V1 state.
 *
 * Source: SKULL.ASM T520  — party/movement tick
 *         SKULL.ASM T048  — input dispatch / tick update
 *         ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *         ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence
 *         DM1 V2.2 smooth: dm1_v2_smooth_movement_pc34.c
 *           v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync
 *           v22_smooth_update_from_clock / v22_smooth_get_x/y/angle
 */

#include "dm2_v2_smooth_movement.h"
#include <math.h>
#include <string.h>

/* ── Lifecycle ─────────────────────────────────────────────────────── */

void dm2_v2_smooth_init(DM2_V2_SmoothState *s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
}

/* ── Walk ──────────────────────────────────────────────────────────── */

void dm2_v2_smooth_start_walk(DM2_V2_SmoothState *s,
                               float from_x, float from_y,
                               float to_x,   float to_y) {
    if (!s) return;
    /* Ease-out cubic: snappy start, gentle land */
    v2_anim_start_v1_tick(&s->walk.anim_x, from_x, to_x, V2_EASE_OUT_CUBIC);
    v2_anim_start_v1_tick(&s->walk.anim_y, from_y, to_y, V2_EASE_OUT_CUBIC);
    s->walk.active = 1;
}

/* ── Turn ─────────────────────────────────────────────────────────── */

void dm2_v2_smooth_start_turn(DM2_V2_SmoothState *s,
                              float from_angle, float to_angle) {
    if (!s) return;
    /* Normalise to [0, 360) before storing */
    float fa = from_angle;
    float ta = to_angle;
    while (fa < 0.0f)   fa += 360.0f;
    while (fa >= 360.0f) fa -= 360.0f;
    while (ta < 0.0f)   ta += 360.0f;
    while (ta >= 360.0f) ta -= 360.0f;
    /* Shortest-path turn: if diff > 180, reverse direction */
    float diff = ta - fa;
    if (diff >  180.0f) ta -= 360.0f;
    if (diff < -180.0f) ta += 360.0f;

    v2_anim_start_v1_tick(&s->turn.anim_angle, fa, ta, V2_EASE_OUT_QUAD);
    s->turn.active = 1;
}

/* ── Stairs ────────────────────────────────────────────────────────── */

void dm2_v2_smooth_start_stairs(DM2_V2_SmoothState *s,
                                float from_x, float from_y,
                                float to_x,   float to_y,
                                float vert_offset) {
    if (!s) return;
    /* Ease-in-out cubic for smooth climb/descent */
    v2_anim_start_v1_tick(&s->stairs.anim_x,    from_x,     to_x,     V2_EASE_IN_OUT_CUBIC);
    v2_anim_start_v1_tick(&s->stairs.anim_y,    from_y,     to_y,     V2_EASE_IN_OUT_CUBIC);
    v2_anim_start_v1_tick(&s->stairs.anim_vert,  0.0f, vert_offset, V2_EASE_IN_OUT_CUBIC);
    s->stairs.active = 1;
}

/* ── Tick ─────────────────────────────────────────────────────────── */

void dm2_v2_smooth_tick(DM2_V2_SmoothState *s, float dt_ms) {
    if (!s) return;

    if (s->walk.active) {
        v2_anim_update(&s->walk.anim_x, dt_ms);
        v2_anim_update(&s->walk.anim_y, dt_ms);
        if (v2_anim_is_done(&s->walk.anim_x) && v2_anim_is_done(&s->walk.anim_y))
            s->walk.active = 0;
    }

    if (s->turn.active) {
        v2_anim_update(&s->turn.anim_angle, dt_ms);
        if (v2_anim_is_done(&s->turn.anim_angle))
            s->turn.active = 0;
    }

    if (s->stairs.active) {
        v2_anim_update(&s->stairs.anim_x,    dt_ms);
        v2_anim_update(&s->stairs.anim_y,    dt_ms);
        v2_anim_update(&s->stairs.anim_vert, dt_ms);
        if (v2_anim_is_done(&s->stairs.anim_x) &&
            v2_anim_is_done(&s->stairs.anim_y) &&
            v2_anim_is_done(&s->stairs.anim_vert))
            s->stairs.active = 0;
    }
}

/* ── Query ─────────────────────────────────────────────────────────── */

void dm2_v2_smooth_get_position(const DM2_V2_SmoothState *s, float *out_x, float *out_y) {
    if (!s || !out_x || !out_y) return;

    if (s->stairs.active) {
        *out_x = v2_anim_value(&s->stairs.anim_x);
        *out_y = v2_anim_value(&s->stairs.anim_y);
    } else if (s->walk.active) {
        *out_x = v2_anim_value(&s->walk.anim_x);
        *out_y = v2_anim_value(&s->walk.anim_y);
    } else {
        /* No active animation — return the "to" position (snapped) */
        *out_x = v2_anim_value(&s->walk.anim_x);
        *out_y = v2_anim_value(&s->walk.anim_y);
    }
}

float dm2_v2_smooth_get_vertical(const DM2_V2_SmoothState *s) {
    if (!s) return 0.0f;
    if (!s->stairs.active) return 0.0f;
    return v2_anim_value(&s->stairs.anim_vert);
}

float dm2_v2_smooth_get_angle(const DM2_V2_SmoothState *s) {
    if (!s) return 0.0f;
    return v2_anim_value(&s->turn.anim_angle);
}

int dm2_v2_smooth_is_active(const DM2_V2_SmoothState *s) {
    if (!s) return 0;
    return s->walk.active || s->turn.active || s->stairs.active;
}

int dm2_v2_smooth_is_turning(const DM2_V2_SmoothState *s) {
    if (!s) return 0;
    return s->turn.active;
}

/* ── Source evidence ───────────────────────────────────────────────── */

const char *dm2_v2_smooth_source_evidence(void) {
    return
        "DM2 V2 Smooth Movement — Phase 5\n"
        "Source: SKULL.ASM T520  — party/movement tick\n"
        "Source: SKULL.ASM T048  — input dispatch / tick update\n"
        "Source: ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution\n"
        "Source: ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)\n"
        "Source: ReDMCSB VBLANK.C M526_WaitVerticalBlank\n"
        "Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)\n"
        "  v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync\n"
        "  v22_smooth_update_from_clock / v22_smooth_get_x/y/angle\n"
        "DM1 V2.2: ease-out cubic for walk, ease-out quad for turn,\n"
        "          ease-in-out cubic for stairs + vertical offset\n";
}
