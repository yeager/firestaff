#ifndef FIRESTAFF_DM2_V2_VIEWPORT_RENDERER_H
#define FIRESTAFF_DM2_V2_VIEWPORT_RENDERER_H

/*
 * DM2 V2 Viewport Renderer — Phase 5
 *
 * Indoor: EPX upscale from V1 320×200 → scaled output.
 * Outdoor: sky gradient, tree parallax, building perspective.
 * V2.2: smooth camera transitions between indoor/outdoor.
 *
 * Phase 5 adds smooth movement interpolation using the V2_AnimClock
 * tick: v2_anim_clock_v1_tick (V1 state snap) → render_frame (sub-tick
 * interpolation) → smooth movement state updated each frame.
 *
 * Source: SKULL.ASM T560  — dungeon viewport rendering
 *         SKULL.ASM T600  — outdoor viewport rendering
 *         SKULL.ASM T520  — party/movement tick
 */

#include <stdint.h>
#include "dm1_v2_anim_timing.h"
#include "dm2_v2_smooth_movement.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── DM2 V2 Viewport State ─────────────────────────────────────────── */

typedef struct {
    /* Rendering config */
    int is_outdoor;
    int scale_factor;   /* 2 = V2.0/V2.1 (EPX), 4 = V2.2 */
    int epx_enabled;

    /* Outdoor parallax */
    float sky_scroll_offset;
    float parallax_offset;

    /* V2 animation clock — drives all V2 interpolation */
    V2_AnimClock clock;

    /* V2.2 smooth indoor/outdoor blend */
    V2_Anim indoor_outdoor_blend;

    /* Phase 5: smooth movement interpolation */
    DM2_V2_SmoothState smooth;

} DM2_V2_ViewportState;

/* ── Lifecycle ─────────────────────────────────────────────────────── */

/* dm2_v2_viewport_init — reset viewport and smooth movement state */
void dm2_v2_viewport_init(DM2_V2_ViewportState *s, int scale);

/* dm2_v2_viewport_set_outdoor — switch indoor/outdoor mode.
 * Triggers smooth indoor_outdoor_blend animation. */
void dm2_v2_viewport_set_outdoor(DM2_V2_ViewportState *s, int outdoor);

/* dm2_v2_viewport_v1_tick — called at V1 tick rate (18.2 Hz, ~55ms).
 * Advances the animation clock and smooth movement state.
 * This is where game state snaps (V1 invariant). */
void dm2_v2_viewport_v1_tick(DM2_V2_ViewportState *s, uint32_t now_ms);

/* dm2_v2_viewport_render_frame — called at display refresh rate.
 * Updates the animation clock sub-tick and smooth movement dt.
 * Renders the viewport at display rate with interpolated visuals. */
void dm2_v2_viewport_render_frame(DM2_V2_ViewportState *s, uint32_t now_ms);

/* ── Phase 5: smooth movement triggers ───────────────────────────── */

/* dm2_v2_viewport_smooth_walk — begin smooth walk animation.
 * Call on V1 tick when party moves to (to_x, to_y) from (from_x, from_y). */
void dm2_v2_viewport_smooth_walk(DM2_V2_ViewportState *s,
                                  float from_x, float from_y,
                                  float to_x,   float to_y);

/* dm2_v2_viewport_smooth_turn — begin smooth turn animation.
 * Call on V1 tick when party turns from from_angle to to_angle. */
void dm2_v2_viewport_smooth_turn(DM2_V2_ViewportState *s,
                                 float from_angle, float to_angle);

/* dm2_v2_viewport_smooth_stairs — begin smooth stairs animation.
 * Call on V1 tick when party uses stairs with vertical camera offset. */
void dm2_v2_viewport_smooth_stairs(DM2_V2_ViewportState *s,
                                   float from_x, float from_y,
                                   float to_x,   float to_y,
                                   float vert_offset);

/* dm2_v2_viewport_smooth_query — returns interpolated visual position.
 * Use in the V1 viewport render call to offset camera during animation. */
void dm2_v2_viewport_smooth_query(const DM2_V2_ViewportState *s,
                                   float *out_x, float *out_y,
                                   float *out_vertical,
                                   float *out_angle);

/* ── Source evidence ───────────────────────────────────────────────── */
const char *dm2_v2_viewport_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_VIEWPORT_RENDERER_H */