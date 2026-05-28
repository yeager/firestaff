/*
 * dm2_v2_viewport_renderer.c — DM2 V2 Viewport Renderer
 *
 * Phase 5: Smooth movement and viewport interpolation.
 *
 * Indoor: EPX upscale from V1 320×200 to scale_factor×320×200.
 * Outdoor: sky gradient, tree parallax, building perspective.
 * V2.2: smooth camera transitions between indoor/outdoor.
 *
 * Smooth movement (Phase 5):
 *   V1 tick: game state snaps to new position (V1 invariant preserved).
 *   Render frames between ticks: visual camera interpolates from
 *   old position to new over exactly 1 V1 tick using V2_AnimClock
 *   sub-tick + DM2_V2_SmoothState animations.
 *
 * Reference: dm1_v2_smooth_movement_pc34.c — DM1 V2.2 smooth movement
 *   v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync
 *   v22_smooth_update_from_clock / v22_smooth_get_x/y/angle
 *
 * Source: SKULL.ASM T520  — party/movement tick
 *         SKULL.ASM T560  — dungeon viewport rendering
 *         SKULL.ASM T600  — outdoor viewport rendering
 */

#include "dm2_v2_viewport_renderer.h"
#include <string.h>

/* ── Lifecycle ─────────────────────────────────────────────────────── */

void dm2_v2_viewport_init(DM2_V2_ViewportState *s, int scale) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->scale_factor = (scale == 4) ? 4 : 2;
    s->epx_enabled  = 1;
    v2_anim_clock_init(&s->clock);
    dm2_v2_smooth_init(&s->smooth);
}

void dm2_v2_viewport_set_outdoor(DM2_V2_ViewportState *s, int outdoor) {
    if (!s) return;
    if (outdoor != s->is_outdoor) {
        /* V2.2: smooth blend between indoor/outdoor rendering */
        v2_anim_start(&s->indoor_outdoor_blend,
            (float)s->is_outdoor, (float)outdoor,
            (float)V1_TICK_MS * 3, V2_EASE_IN_OUT_QUAD);
        s->is_outdoor = outdoor;
    }
}

/* ── V1 Tick — game state snaps (V1 invariant) ─────────────────────── */

void dm2_v2_viewport_v1_tick(DM2_V2_ViewportState *s, uint32_t now_ms) {
    if (!s) return;
    v2_anim_clock_v1_tick(&s->clock, now_ms);

    /* Outdoor parallax: scroll sky slowly */
    if (s->is_outdoor) {
        s->sky_scroll_offset += 0.1f;
        if (s->sky_scroll_offset >= 32.0f) s->sky_scroll_offset -= 32.0f;
    }
}

/* ── Render Frame — display rate, interpolation happens here ───────── */

void dm2_v2_viewport_render_frame(DM2_V2_ViewportState *s, uint32_t now_ms) {
    if (!s) return;

    /* Update clock sub-tick (0.0–1.0 within current V1 tick) */
    v2_anim_clock_render_frame(&s->clock, now_ms);

    /* Advance smooth movement state by dt since last frame */
    dm2_v2_smooth_tick(&s->smooth, s->clock.dt_ms);

    /* Advance indoor/outdoor blend */
    v2_anim_update(&s->indoor_outdoor_blend, s->clock.dt_ms);
}

/* ── Phase 5: smooth movement triggers ───────────────────────────── */

void dm2_v2_viewport_smooth_walk(DM2_V2_ViewportState *s,
                                  float from_x, float from_y,
                                  float to_x,   float to_y) {
    if (!s) return;
    dm2_v2_smooth_start_walk(&s->smooth, from_x, from_y, to_x, to_y);
}

void dm2_v2_viewport_smooth_turn(DM2_V2_ViewportState *s,
                                 float from_angle, float to_angle) {
    if (!s) return;
    dm2_v2_smooth_start_turn(&s->smooth, from_angle, to_angle);
}

void dm2_v2_viewport_smooth_stairs(DM2_V2_ViewportState *s,
                                   float from_x, float from_y,
                                   float to_x,   float to_y,
                                   float vert_offset) {
    if (!s) return;
    dm2_v2_smooth_start_stairs(&s->smooth, from_x, from_y, to_x, to_y, vert_offset);
}

void dm2_v2_viewport_smooth_query(const DM2_V2_ViewportState *s,
                                  float *out_x, float *out_y,
                                  float *out_vertical,
                                  float *out_angle) {
    if (!s) {
        if (out_x)       *out_x = 0.0f;
        if (out_y)       *out_y = 0.0f;
        if (out_vertical) *out_vertical = 0.0f;
        if (out_angle)   *out_angle = 0.0f;
        return;
    }

    float vx = 0.0f, vy = 0.0f;
    dm2_v2_smooth_get_position(&s->smooth, &vx, &vy);

    if (out_x)       *out_x       = vx;
    if (out_y)       *out_y       = vy;
    if (out_vertical) *out_vertical = dm2_v2_smooth_get_vertical(&s->smooth);
    if (out_angle)   *out_angle   = dm2_v2_smooth_get_angle(&s->smooth);
}

/* ── Source evidence ───────────────────────────────────────────────── */

const char *dm2_v2_viewport_source_evidence(void) {
    return
        "DM2 V2 Viewport Renderer — Phase 5\n"
        "Source: SKULL.ASM T520  — party/movement tick\n"
        "Source: SKULL.ASM T560  — dungeon viewport rendering\n"
        "Source: SKULL.ASM T600  — outdoor viewport rendering (sky, parallax)\n"
        "Source: ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution\n"
        "Source: ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)\n"
        "Source: ReDMCSB VBLANK.C M526_WaitVerticalBlank\n"
        "Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)\n"
        "  v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync\n"
        "  v22_smooth_update_from_clock / v22_smooth_get_x/y/angle\n"
        "DM2 V2.2: smooth indoor/outdoor blend (V2_EASE_IN_OUT_QUAD, 3 ticks)\n"
        "DM2 V2.2: smooth walk (V2_EASE_OUT_CUBIC, 1 tick)\n"
        "DM2 V2.2: smooth turn (V2_EASE_OUT_QUAD, 1 tick)\n"
        "DM2 V2.2: smooth stairs (V2_EASE_IN_OUT_CUBIC, 1 tick + vertical)\n";
}