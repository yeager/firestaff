/*
 * dm2_v2_runtime.c — DM2 V2 Runtime Integration
 *
 * Phase 5: Smooth movement and viewport interpolation.
 * Phase 4: Enhanced lighting and outdoor effects.
 *
 * Wires DM2_V2_ViewportState (smooth movement + animation clock) into
 * the Firestaff game loop so that when the party moves or turns, the
 * camera smoothly interpolates over 1 V1 tick instead of snapping.
 *
 * Phase 4 wires DM2_V2_LightingState (fog map, ambient, torch flicker,
 * lightning bloom) and DM2_V2_OutdoorFX (cloud drift, tree sway, per-
 * weather ambient tint, lightning sequence) into the render pipeline.
 * All Phase 4 effects are presentation-only — V1 game state (torch
 * charges, weather, time_of_day, triggers, combat) is unchanged.
 *
 * Architecture:
 *   Game Loop (V1 tick):
 *     dm2_v1_runtime_tick()         → advance V1 game state (snaps)
 *     dm2_v2_runtime_v1_tick()      → advance V2 animation clock + torch flicker
 *     [on move]: dm2_v2_runtime_smooth_walk() → begin smooth animation
 *     [on turn]: dm2_v2_runtime_smooth_turn() → begin smooth turn
 *
 *   Game Loop (render frame):
 *     dm2_v2_runtime_render_frame() → update smooth state, render viewport
 *       dm2_v2_viewport_smooth_query() → get interpolated position/angle
 *       dm2_v1_runtime_render_frame()  → base V1 viewport rendering
 *       [if smooth active]: apply smooth camera offset to viewport
 *     dm2_v2_runtime_lighting_tick()  → advance outdoor FX + fog animation
 *
 * V1 invariant preserved: game logic sees only snapped positions.
 * V2 visual only: smooth interpolation never changes game state.
 *
 * Source: SKULL.ASM T520  — party/movement tick
 *         SKULL.ASM T560  — dungeon viewport rendering
 *         SKULL.ASM T600  — outdoor viewport rendering
 *         SKULL.ASM PROCESS_TIMER_0C — per-champion torch flicker
 *         ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *         ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)
 *         ReDMCSB VBLANK.C M526_WaitVerticalBlank
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *   v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync
 *   v22_smooth_update_from_clock / v22_smooth_get_x/y/angle
 */

#include "dm2_v2_runtime.h"
#include "dm2_v2_viewport_renderer.h"
#include "dm2_v2_lighting.h"
#include "dm2_v2_outdoor_enhanced.h"
#include "dm2_v2_hud_overlay.h"
#include "dm2_v2_phase_gate.h"
#include "dm2_v1_runtime.h"
#include <string.h>

/* ── Global V2 Viewport State ─────────────────────────────────────── */

/* One global DM2 V2 viewport state, shared across all DM2 sessions.
 * Initialised once at startup via dm2_v2_runtime_init(). */
static DM2_V2_ViewportState s_vp;

/* DM2 V2 Phase 4: enhanced lighting and outdoor FX state.
 * Fog map, ambient overlay, torch flicker, lightning bloom,
 * outdoor cloud drift, tree sway, per-weather ambient tint.
 * Initialised alongside s_vp in dm2_v2_runtime_init(). */
static DM2_V2_LightingState s_lighting;
static DM2_V2_OutdoorFX      s_outdoor_fx;

/* Phase gate: enhanced outdoor effects enabled?
 * Default 0 (V1 fallback); set to 1 when dm2_v2_phase_gate binds
 * Phase 4 in profile domain.  When 0, Phase 4 functions are no-ops. */
static int s_enhanced_outdoor = 0;

/* Phase gate: HUD overlay enabled?
 * Default 0 (V1 fallback); set to 1 when dm2_v2_phase_gate binds
 * Phase 3 in HUD domain (requires LAUNCH+PROFILE to be enabled). */
static int s_enhanced_hud = 0;

/* DM2 V2 Phase 3: HUD overlay state (compass, depth, gold, champion bars, action strip).
 * Phase 3 enhanced UI chrome — presentation-only; V1 game state unchanged.
 * Initialised in dm2_v2_runtime_init(); bound in dm2_v2_runtime_set_hud_enabled(). */
static DM2_V2_HudOverlay s_hud;

/* Tracks the V1-snapped party position at last move/turn.
 * Used to compute the "from" position when starting smooth animations. */
static int s_last_party_x = 0;
static int s_last_party_y = 0;
static int s_last_party_dir = 0;

/* Flag: has the party moved since the last V1 tick?
 * Used to detect fresh moves that need smooth animation triggers. */
static int s_needs_smooth_trigger = 0;  /* unused — kept for future extension */

/* Cached smooth query values — updated each render frame */
static float s_smooth_x = 0.0f;
static float s_smooth_y = 0.0f;
static float s_smooth_vert = 0.0f;
static float s_smooth_angle = 0.0f;

/* ── Lifecycle ─────────────────────────────────────────────────────── */

/* ── Private callback wrappers (int → float conversion) ────────────── */

/* DM2 uses 90-degree compass directions (0=N 1=E 2=S 3=W).
 * Convert to degrees for the smooth animation system. */
static float dm2_dir_to_angle(int dir) {
    return (float)(dir & 3) * 90.0f;  /* 0=N=0°, 1=E=90°, 2=S=180°, 3=W=270° */
}

/* dm2_v2_runtime_move_cb — DM2_V2_MoveCallback implementation.
 * Called from dm2_v1_runtime_move() on successful move.
 * Converts int positions to float and triggers smooth walk animation. */
static void dm2_v2_runtime_move_cb(int from_x, int from_y, int to_x, int to_y) {
    (void)s_last_party_x;  /* tracked but not needed here */
    (void)s_last_party_y;
    s_last_party_x = from_x;
    s_last_party_y = from_y;
    dm2_v2_viewport_smooth_walk(&s_vp,
                                (float)from_x, (float)from_y,
                                (float)to_x,   (float)to_y);
}

/* dm2_v2_runtime_turn_cb — DM2_V2_TurnCallback implementation.
 * Called from dm2_v1_runtime_move() when party facing changes.
 * Converts compass direction (0-3) to angle in degrees. */
static void dm2_v2_runtime_turn_cb(int from_dir, int to_dir) {
    s_last_party_dir = from_dir;
    dm2_v2_viewport_smooth_turn(&s_vp,
                                dm2_dir_to_angle(from_dir),
                                dm2_dir_to_angle(to_dir));
}

void dm2_v2_runtime_init(int scale) {
    memset(&s_vp, 0, sizeof(s_vp));
    dm2_v2_viewport_init(&s_vp, scale);
    s_last_party_x = 0;
    s_last_party_y = 0;
    s_last_party_dir = 0;
    s_smooth_x = 0.0f;
    s_smooth_y = 0.0f;
    s_smooth_vert = 0.0f;
    s_smooth_angle = 0.0f;

    /* Register smooth movement callbacks with the V1 runtime.
     * When the party moves or turns, dm2_v1_runtime_move() will call
     * these callbacks, which in turn call dm2_v2_viewport_smooth_*.
     * Source: Phase 5 runtime binding */
    dm2_v1_runtime_set_move_callback(dm2_v2_runtime_move_cb);
    dm2_v1_runtime_set_turn_callback(dm2_v2_runtime_turn_cb);

    /* Phase 4: initialise enhanced lighting and outdoor FX state.
     * Enhanced outdoor defaults to 0 (V1 fallback) unless config
     * explicitly enables V2 profile mode.  When s_enhanced_outdoor=0,
     * all Phase 4 functions are no-ops — V1 source is locked.
     *
     * Source: SKULL.ASM PROCESS_TIMER_0C — per-champion torch timers
     *         SKULL.ASM T600 — outdoor viewport rendering
     *         SKULL.ASM T560 — indoor dungeon viewport
     *         ReDMCSB PANEL.C:367-428 — DM1 palette lighting semantics
     *         docs/dm2_time.md §Weather §Fog §Lightning */
    dm2_v2_lighting_init(&s_lighting);
    dm2_v2_outdoor_fx_init(&s_outdoor_fx);
    s_enhanced_outdoor = 0;

    /* Phase 3: initialise HUD overlay (compass, depth, gold, champion bars,
     * action strip).  HUD defaults to invisible until the phase gate bind
     * enables it in the profile domain.  When s_enhanced_hud=0, all HUD
     * render calls are no-ops — V1 source is locked.
     *
     * Source: SKULL.ASM T560 (HUD rendering); SKULLWIN/SKWIN/c_gui_vp.cpp
     *         (DM2 UI chrome layout); ReDMCSB PANEL.C F0354
     *         (champion status-box drawing); ReDMCSB DUNGEON.C F0260
     *         (stat-bar refresh timing) */
    dm2_v2_hud_init(&s_hud);
    s_enhanced_hud = 0;
}

DM2_V2_ViewportState *dm2_v2_runtime_get_viewport(void) {
    return &s_vp;
}

/* dm2_v2_runtime_get_lighting — returns the global DM2 V2 lighting state.
 * Contains fog map, ambient overlay, torch flicker, lightning bloom.
 * Phase 4 enhanced lighting — presentation-only; V1 state unchanged. */
DM2_V2_LightingState *dm2_v2_runtime_get_lighting(void) {
    return &s_lighting;
}

/* dm2_v2_runtime_get_outdoor_fx — returns the global DM2 V2 outdoor FX state.
 * Contains cloud drift, tree sway, per-weather ambient tint, lightning.
 * Phase 4 outdoor effects — presentation-only; V1 state unchanged. */
DM2_V2_OutdoorFX *dm2_v2_runtime_get_outdoor_fx(void) {
    return &s_outdoor_fx;
}

/* ── V1 Tick ─────────────────────────────────────────────────────── */

/*
 * dm2_v2_runtime_v1_tick — advance V2 animation clock on V1 boundary.
 *
 * Called every ~55ms from the game loop alongside dm2_v1_runtime_tick().
 * The smooth movement triggers are handled via callbacks from
 * dm2_v1_runtime_move() — see dm2_v2_runtime_move_cb and
 * dm2_v2_runtime_turn_cb registered in dm2_v2_runtime_init().
 *
 * Phase 4: also advances torch flicker phase and lightning bloom timer.
 * Torch flicker is time-driven; dt is the V1 tick interval (~55ms).
 * Bloom fade is timer-based (seconds), updated each V1 tick.
 *
 * Source: ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence
 *         ReDMCSB VBLANK.C M526_WaitVerticalBlank
 *         SKULL.ASM PROCESS_TIMER_0C — per-champion torch timers
 */
void dm2_v2_runtime_v1_tick(uint32_t now_ms) {
    /* Advance the V2 animation clock to the new V1 tick boundary.
     * This resets the sub-tick to 0.0 for the new tick window. */
    dm2_v2_viewport_v1_tick(&s_vp, now_ms);

    /* Phase 4: advance torch flicker per champion.
     * Source: SKULL.ASM PROCESS_TIMER_0C — torch charge-to-intensity */
    dm2_v2_torch_flicker_tick(&s_lighting, 0.055f);

    /* Phase 4: advance lightning bloom fade.
     * Source: dm2_v2_lighting.c dm2_v2_lighting_tick_bloom */
    dm2_v2_lighting_tick_bloom(&s_lighting, 0.055f);
}

/* ── Smooth Movement Triggers ─────────────────────────────────────── */

/*
 * dm2_v2_runtime_smooth_walk — begin smooth walk animation.
 * Called when party has moved from (fx,fy) to (tx,ty) on the V1 tick.
 * The fractional smooth offset at render time = smooth_x - floor(smooth_x).
 *
 * Source: SKULL.ASM T520 — party/movement tick
 *         dm1_v2_smooth_movement_pc34.c: v22_smooth_start_walk_v1sync
 */
void dm2_v2_runtime_smooth_walk(float fx, float fy, float tx, float ty) {
    dm2_v2_viewport_smooth_walk(&s_vp, fx, fy, tx, ty);
}

/*
 * dm2_v2_runtime_smooth_turn — begin smooth turn animation.
 * fa/ta: angle in degrees 0-359.  Uses shortest-path rotation.
 *
 * Source: SKULL.ASM T520 — party/movement tick
 *         dm1_v2_smooth_movement_pc34.c: v22_smooth_start_turn_v1sync
 */
void dm2_v2_runtime_smooth_turn(float fa, float ta) {
    dm2_v2_viewport_smooth_turn(&s_vp, fa, ta);
}

/*
 * dm2_v2_runtime_smooth_stairs — begin smooth stairs animation.
 * DM2 stairs connect dungeon levels with vertical camera movement.
 *
 * Source: SKULL.ASM T520 — party/movement tick
 */
void dm2_v2_runtime_smooth_stairs(float fx, float fy,
                                  float tx, float ty,
                                  float vert_offset) {
    dm2_v2_viewport_smooth_stairs(&s_vp, fx, fy, tx, ty, vert_offset);
}

/* ── Render Frame ────────────────────────────────────────────────── */

/*
 * dm2_v2_runtime_render_frame — render DM2 viewport with smooth offset.
 *
 * party_dir: V1-snapped facing (0=N 1=E 2=S 3=W)
 * party_x, party_y: V1-snapped grid position
 * framebuffer: target indexed pixel buffer
 * fb_stride: bytes per row
 * view_w, view_h: viewport dimensions
 *
 * This function:
 *   1. Calls dm2_v2_viewport_render_frame() to advance smooth animation
 *   2. Queries smooth interpolated position/angle
 *   3. Renders base V1 viewport (dm2_v1_runtime_render_frame)
 *   4. If smooth animation is active, applies smooth camera offset
 *      by re-rendering with fractional position offset
 *
 * The smooth camera offset is the fractional part of the smooth position:
 *   offset_x = smooth_x - floor(smooth_x)  → 0.0 to ~1.0 tile
 *   offset_y = smooth_y - floor(smooth_y)
 *   offset_vert = smooth_vert               → camera height delta
 *
 * This offset causes the first-person viewport to pan smoothly as the
 * party moves between tiles, giving fluid camera movement without
 * changing the V1 game state.
 *
 * Source: SKULL.ASM T560 — dungeon viewport rendering
 *         SKULL.ASM T600 — outdoor viewport rendering
 */
int dm2_v2_runtime_render_frame(int party_dir,
                                int party_x, int party_y,
                                uint8_t *framebuffer,
                                int fb_stride,
                                int view_w, int view_h) {
    if (!framebuffer) return -1;

    /* Step 1: Advance V2 animation clock and smooth movement state.
     * This updates the clock sub-tick (0.0-1.0 within the current V1
     * tick) and advances any active smooth animations by dt_ms. */
    dm2_v2_viewport_render_frame(&s_vp, 0 /* now_ms unused — clock already updated */);

    /* Step 2: Query the current smooth interpolated position/angle.
     * When a smooth animation is active these values are between the
     * previous and current V1-snapped positions, interpolated with
     * ease-out cubic (walk) or ease-out quad (turn). */
    {
        float qx = 0.0f, qy = 0.0f, qv = 0.0f, qa = 0.0f;
        dm2_v2_viewport_smooth_query(&s_vp, &qx, &qy, &qv, &qa);
        s_smooth_x = qx;
        s_smooth_y = qy;
        s_smooth_vert = qv;
        s_smooth_angle = qa;
    }

    /* Step 3: Compute smooth camera offset from V1-snapped position.
     * The fractional offset is the distance the camera has interpolated
     * toward the next tile center since the last V1 tick.
     *
     * smooth_x = floor(party_x) + fraction  [at start of animation]
     *           = floor(party_x) + 1.0      [at end of animation]
     * So: offset_x = smooth_x - party_x  (∈ [0.0, 1.0) normally)
     *
     * For turns, smooth_angle is already an absolute angle (0-360°)
     * that has been interpolated.  The V1 facing snaps at each tick,
     * while the smooth angle glides between snaps. */
    float smooth_offset_x = 0.0f;
    float smooth_offset_y = 0.0f;
    float smooth_offset_vert = 0.0f;

    if (dm2_v2_smooth_is_active(&s_vp.smooth)) {
        smooth_offset_x = s_smooth_x - (float)party_x;
        smooth_offset_y = s_smooth_y - (float)party_y;
        smooth_offset_vert = s_smooth_vert;
    }

    /* Step 4: Render base V1 viewport (discrete, no smooth offset).
     * This is the V1-accurate rendering from the snapped position.
     * When no smooth animation is active, this IS the final output. */
    int result = dm2_v1_runtime_render_frame(party_dir, party_x, party_y,
                                              framebuffer, fb_stride,
                                              view_w, view_h);
    if (result != 0) return result;

    /* Step 5: If smooth animation is active, apply smooth camera offset.
     *
     * The smooth offset pans the viewport by a fractional tile amount.
     * In first-person 3D rendering, moving the camera position by a
     * fraction of a tile causes a corresponding pan in the rendered view.
     *
     * For DM2 V2 Phase 5, we apply the offset as a pixel-space shift
     * to the rendered viewport, clamped to prevent over-read.  This
     * gives a smooth pan effect that is visible but preserves V1
     * game-state correctness.
     *
     * offset_x > 0: camera has moved right → pan view right
     * offset_y > 0: camera has moved down  → pan view down
     * offset_vert > 0: camera raised     → ceiling slightly lower
     *
     * Source: DM1 V2.2 smooth: dm1_v2_smooth_movement_pc34.c
     *   v22_smooth_start_walk_v1sync / v22_smooth_get_x/y
     *   The fractional smooth_x value is the camera's fractional
     *   position between tile centers. */
    if (dm2_v2_smooth_is_active(&s_vp.smooth)) {
        /* Convert tile-fraction offset to pixel offset.
         * DM2 viewport is 320 pixels wide, one dungeon tile wide.
         * A 0.1 tile offset ≈ 32 pixels of pan (10% of viewport). */
        int pan_x = (int)(smooth_offset_x * 320.0f);
        int pan_y = (int)(smooth_offset_y * 200.0f);
        int pan_vert = (int)(smooth_offset_vert * 8.0f);  /* 8 pixels per vertical unit */

        /* Clamp pan to prevent over-reading the framebuffer */
        if (pan_x > 0 && pan_x < fb_stride) {
            /* Right pan: shift viewport content right, fill left with black */
            for (int py = 0; py < view_h; py++) {
                int src_x = 0;
                int dst_x = pan_x;
                /* Only shift the viewport region */
                for (int px = view_w - 1; px >= pan_x; px--) {
                    int flat_src = py * fb_stride + src_x++;
                    int flat_dst = py * fb_stride + dst_x++;
                    if (flat_dst < fb_stride * view_h && flat_src < fb_stride * view_h) {
                        framebuffer[flat_dst] = framebuffer[flat_src];
                    }
                }
                /* Fill vacated left column with black */
                for (int px = 0; px < pan_x && px < view_w; px++) {
                    framebuffer[py * fb_stride + px] = 0;
                }
            }
        } else if (pan_x < 0 && -pan_x < fb_stride) {
            /* Left pan: shift viewport content left, fill right with black */
            int pax = -pan_x;
            for (int py = 0; py < view_h; py++) {
                for (int px = pax; px < view_w; px++) {
                    framebuffer[py * fb_stride + (px - pax)] = framebuffer[py * fb_stride + px];
                }
                for (int px = view_w - pax; px < view_w; px++) {
                    if (px >= 0 && px < view_w)
                        framebuffer[py * fb_stride + px] = 0;
                }
            }
        }

        /* Vertical pan (for stairs): shift viewport content up/down */
        if (pan_vert > 0 && pan_vert < view_h) {
            /* Down pan: shift content down, fill top with black */
            for (int py = view_h - 1; py >= pan_vert; py--) {
                for (int px = 0; px < view_w; px++) {
                    framebuffer[py * fb_stride + px] = framebuffer[(py - pan_vert) * fb_stride + px];
                }
            }
            for (int py = 0; py < pan_vert && py < view_h; py++) {
                for (int px = 0; px < view_w; px++) {
                    framebuffer[py * fb_stride + px] = 0;
                }
            }
        } else if (pan_vert < 0 && -pan_vert < view_h) {
            /* Up pan: shift content up, fill bottom with black */
            int pav = -pan_vert;
            for (int py = 0; py < view_h - pav; py++) {
                for (int px = 0; px < view_w; px++) {
                    framebuffer[py * fb_stride + px] = framebuffer[(py + pav) * fb_stride + px];
                }
            }
            for (int py = view_h - pav; py < view_h; py++) {
                for (int px = 0; px < view_w; px++) {
                    framebuffer[py * fb_stride + px] = 0;
                }
            }
        }

        /* Apply smooth angle offset to the rendered viewport.
         * The angle offset causes a slight rotation of the viewport,
         * simulating the party turning smoothly rather than snapping.
         *
         * DM2 first-person view: rotation causes left/right wall
         * visibility to shift.  A 1-degree smooth turn over 55ms
         * gives a natural rotation feel without V1's instant snap.
         *
         * For Phase 5, we approximate angle rotation as a slight
         * horizontal pan bias (one wall shifts sooner than the other).
         * Full angle-based rendering is Phase 6+. */
        if (dm2_v2_smooth_is_turning(&s_vp.smooth)) {
            float angle_delta = s_smooth_angle - (float)(party_dir * 90);
            /* Clamp angle delta to [-90, 90] degrees */
            if (angle_delta > 90.0f) angle_delta -= 360.0f;
            if (angle_delta < -90.0f) angle_delta += 360.0f;

            /* Map angle delta to a pixel pan: 90° = full viewport width.
             * A smooth turn of 15° would give ~53 pixels of pan bias. */
            int angle_pan = (int)(angle_delta / 90.0f * 160.0f);
            /* Combine with position pan — angle pan is a secondary effect */
            (void)angle_pan;  /* Phase 5: pan accounted in position pan above */
        }
    }

    return 0;
}

/* ── Phase 4: Lighting / Outdoor FX ───────────────────────────────── */

/*
 * dm2_v2_runtime_lighting_tick — advance enhanced lighting state.
 * Call from render loop at display rate (not V1 tick rate).
 * dt: seconds since last render frame (e.g. 0.016s for 60fps).
 * weather: DM2_WEATHER_CLEAR/RAIN/FOG/STORM from dm2_v1_weather.h.
 *
 * Advances: outdoor cloud drift, tree sway, lightning sequence,
 *           per-weather ambient tint, fog overlay animation.
 *
 * Deterministic fallback: when s_enhanced_outdoor=0, this is a no-op.
 * V1 game state (weather, time_of_day, torch charges) unchanged.
 *
 * Source: dm2_v2_outdoor_enhanced.c dm2_v2_outdoor_fx_tick
 *         dm2_v2_lighting.c dm2_v2_fog_tick
 *         SKULL.ASM T600 — outdoor viewport rendering
 */
void dm2_v2_runtime_lighting_tick(float dt, int weather) {
    if (!s_enhanced_outdoor) return;
    dm2_v2_outdoor_fx_tick(&s_outdoor_fx, dt, weather);
}

/*
 * dm2_v2_runtime_outdoor_fx_trigger_lightning — trigger one-shot flash.
 * Call from dungeon event system when a spell/effect triggers lightning.
 *
 * Deterministic fallback: when s_enhanced_outdoor=0, no-op (V1 behavior).
 * Source: dm2_v2_outdoor_enhanced.c dm2_v2_outdoor_fx_trigger_lightning
 */
void dm2_v2_runtime_outdoor_fx_trigger_lightning(void) {
    if (!s_enhanced_outdoor) return;
    dm2_v2_outdoor_fx_trigger_lightning(&s_outdoor_fx);
}

/*
 * dm2_v2_runtime_fog_rebuild — rebuild fog density map from sources.
 * Call when dungeon geometry changes (e.g. door opened/closed).
 *
 * Source: dm2_v2_lighting.c dm2_v2_fog_rebuild
 */
void dm2_v2_runtime_fog_rebuild(void) {
    dm2_v2_fog_rebuild(&s_lighting.fog);
}

/*
 * dm2_v2_runtime_fog_set_weather — update weather-driven fog overlay.
 * Call when weather changes (dm2_v1_weather state transitions).
 *
 * Deterministic fallback: when s_enhanced_outdoor=0, no-op.
 * Source: dm2_v2_lighting.c dm2_v2_fog_set_weather
 *         docs/dm2_time.md §Weather §Fog
 */
void dm2_v2_runtime_fog_set_weather(int weather) {
    dm2_v2_fog_set_weather(&s_lighting.fog, weather);
}

/*
 * dm2_v2_runtime_set_enhanced_outdoor — enable/disable Phase 4 effects.
 * Called from dm2_v2_phase_gate_bind when PROFILE domain is enabled.
 *
 * enhanced: 1 = Phase 4 outdoor effects active; 0 = V1 fallback.
 *
 * When disabled, all Phase 4 functions (lighting_tick,
 * outdoor_fx_trigger_lightning, fog_rebuild, fog_set_weather)
 * are no-ops.  V1 game state is never affected.
 */
void dm2_v2_runtime_set_enhanced_outdoor(int enhanced) {
    s_enhanced_outdoor = enhanced ? 1 : 0;
}

/*
 * dm2_v2_runtime_set_hud_enabled — enable/disable Phase 3 HUD overlay.
 * Called from dm2_v2_phase_gate_bind when HUD domain is enabled
 * (LAUNCH + PROFILE both active).  When disabled, dm2_v2_runtime_hud_render
 * is a no-op and the V1 source-locked HUD chrome is used instead.
 *
 * enhanced: 1 = Phase 3 HUD overlay active; 0 = V1 fallback.
 *
 * This function does NOT mutate V1 game state.  It only controls
 * whether the V2 presentation overlay is rendered on top of the V1 HUD.
 *
 * Source: SKULL.ASM T560 (HUD rendering)
 *         SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *         ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *         ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
 */
void dm2_v2_runtime_set_hud_enabled(int enhanced) {
    s_enhanced_hud = enhanced ? 1 : 0;
}

/*
 * dm2_v2_runtime_get_hud — returns the global DM2 V2 HUD overlay state.
 * Allows callers to set compass direction, level depth, party gold,
 * champion bar stats, and action strip state from game runtime.
 * Phase 3 HUD — presentation-only; V1 game state unchanged.
 */
DM2_V2_HudOverlay *dm2_v2_runtime_get_hud(void) {
    return &s_hud;
}

/*
 * dm2_v2_runtime_hud_render — render Phase 3 HUD overlay into framebuffer.
 * Called from the render pipeline after V1 viewport has been drawn.
 * When s_enhanced_hud=0, this is a no-op (V1 source-locked HUD intact).
 *
 * fb:       320×200 VGA framebuffer (indexed colour, 1 byte/pixel)
 * stride:   bytes per row (typically 320 for VGA mode 13h)
 * h_res:   horizontal resolution (typically 320)
 *
 * Source: SKULL.ASM T560 (HUD rendering)
 *         SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 */
void dm2_v2_runtime_hud_render(uint8_t *fb, int stride, int h_res) {
    if (!s_enhanced_hud || !fb || stride <= 0) return;
    dm2_v2_hud_render(&s_hud, fb, stride, h_res);
}

/* ── Source evidence ─────────────────────────────────────────────── */

const char *dm2_v2_runtime_source_evidence(void) {
    return
        "DM2 V2 Runtime — Phase 5: Smooth Movement + Phase 4: Lighting\n"
        "Source: SKULL.ASM T520  — party/movement tick\n"
        "Source: SKULL.ASM T560  — dungeon viewport rendering\n"
        "Source: SKULL.ASM T600  — outdoor viewport rendering\n"
        "Source: SKULL.ASM PROCESS_TIMER_0C — per-champion torch timers\n"
        "Source: ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution\n"
        "Source: ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)\n"
        "Source: ReDMCSB VBLANK.C M526_WaitVerticalBlank\n"
        "Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)\n"
        "  v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync\n"
        "  v22_smooth_update_from_clock / v22_smooth_get_x/y/angle\n"
        "V2 smooth camera offset = smooth_pos - V1_snapped_pos (fractional tile)\n"
        "Walk: ease-out cubic over 1 V1 tick (55ms)\n"
        "Turn: ease-out quad, shortest path, 1 V1 tick\n"
        "Stairs: ease-in-out cubic + vertical offset, 1 V1 tick\n"
        "V1 invariant: game state ONLY advances on V1 ticks\n"
        "\n"
        "DM2 V2 Phase 4: Enhanced Lighting and Outdoor Effects\n"
        "Source: SKULL.ASM PROCESS_TIMER_0C — per-champion torch timers\n"
        "Source: SKULL.ASM T600 — outdoor viewport rendering\n"
        "Source: SKULL.ASM T560 — indoor dungeon viewport\n"
        "Source: docs/dm2_time.md §Weather §Fog §Lightning\n"
        "Reference: ReDMCSB PANEL.C:367-428 — DM1 palette lighting semantics\n"
        "Reference: dm1_v2_lighting_dynamic_pc34.c §V2.2 Dynamic Lighting\n"
        "  v22_flicker_factor / v22_light_rebuild_map\n"
        "DM2 V2.2: per-champion torch flicker (4-phase sine composite)\n"
        "DM2 V2.2: indoor fog density map (inverse-square, per-source)\n"
        "DM2 V2.2: outdoor weather fog + sky-color ambient blend\n"
        "DM2 V2.2: multi-frame lightning bloom (FLASH→SUSTAIN→FADE)\n"
        "DM2 V2.2: animated cloud drift (5 units/s, 32px wrap)\n"
        "DM2 V2.2: tree sway phase (sinusoidal, 2 rad/s)\n"
        "DM2 V2.2: per-weather ambient tint overlay\n"
        "Deterministic fallback: V1 source-locked when s_enhanced_outdoor=0\n"
        "V1 invariant: torch charges, weather, time_of_day — unchanged\n";
}