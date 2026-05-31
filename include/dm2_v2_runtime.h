/*
 * dm2_v2_runtime.h — DM2 V2 Runtime Integration
 *
 * Phase 5: Smooth movement and viewport interpolation.
 * Phase 4: Enhanced lighting and outdoor effects.
 *
 * Wires DM2_V2_ViewportState (smooth movement + animation clock) into
 * the Firestaff game loop.  Provides:
 *
 *   1. Global DM2 V2 viewport state (smooth movement + V2_AnimClock)
 *   2. Global DM2 V2 lighting state (Phase 4: fog, torch, bloom)
 *   3. Global DM2 V2 outdoor FX state (Phase 4: cloud, sway, lightning)
 *   4. V2 render entry point (called from game loop per render frame)
 *   5. V2 V1-tick entry point (called from game loop per V1 tick)
 *
 * V1 invariant preserved: game state snaps on V1 ticks (18.2 Hz).
 * V2 visual interpolation: camera smoothly transitions between V1
 *   snaps over 1 V1 tick using V2_AnimClock sub-tick + V2_Anim easing.
 *
 * Phase 4 is entirely presentation.  V1 game state (torch charges,
 * weather, time_of_day, triggers, combat) is unchanged.
 * Deterministic fallback: all Phase 4 functions are no-ops when
 * s_enhanced_outdoor=0 (V1 source-locked).
 *
 * Smooth movement triggers:
 *   - Walk: ease-out cubic  — snappy, natural deceleration
 *   - Turn: ease-out quad   — quick rotation snap (shortest path)
 *   - Stairs: ease-in-out cubic + vertical offset
 *
 * Source: SKULL.ASM T520  — party/movement tick
 *         SKULL.ASM T560  — dungeon viewport rendering
 *         SKULL.ASM T600  — outdoor viewport rendering
 *         SKULL.ASM PROCESS_TIMER_0C — per-champion torch flicker
 *         ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *         ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *   v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync
 *   v22_smooth_update_from_clock / v22_smooth_get_x/y/angle
 */

#ifndef FIRESTAFF_DM2_V2_RUNTIME_H
#define FIRESTAFF_DM2_V2_RUNTIME_H

#include <stdint.h>
#include "dm2_v2_viewport_renderer.h"
#include "dm2_v2_lighting.h"
#include "dm2_v2_outdoor_enhanced.h"
#include "dm2_v2_hud_overlay.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Global DM2 V2 State Accessors ──────────────────────────────── */

/* dm2_v2_runtime_get_viewport — returns the global DM2 V2 viewport state.
 * Initialised by dm2_v2_runtime_init().  Contains smooth movement + clock. */
DM2_V2_ViewportState *dm2_v2_runtime_get_viewport(void);

/* dm2_v2_runtime_get_lighting — returns the global DM2 V2 lighting state.
 * Contains fog map, ambient overlay, torch flicker, lightning bloom.
 * Phase 4 enhanced lighting — presentation-only; V1 state unchanged. */
DM2_V2_LightingState *dm2_v2_runtime_get_lighting(void);

/* dm2_v2_runtime_get_outdoor_fx — returns the global DM2 V2 outdoor FX state.
 * Contains cloud drift, tree sway, per-weather ambient tint, lightning.
 * Phase 4 outdoor effects — presentation-only; V1 state unchanged. */
DM2_V2_OutdoorFX *dm2_v2_runtime_get_outdoor_fx(void);

/* ── Lifecycle ─────────────────────────────────────────────────────── */

/* dm2_v2_runtime_init — initialise the DM2 V2 runtime.
 * Must be called once at game startup before any other dm2_v2_runtime_*
 * functions.  scale: 2 = V2.0/V2.1 EPX, 4 = V2.2 high-res.
 *
 * Phase 4: also initialises s_lighting and s_outdoor_fx.
 * Phase 4 effects default to disabled (V1 fallback) until
 * dm2_v2_runtime_set_enhanced_outdoor(1) is called. */
void dm2_v2_runtime_init(int scale);

/* ── V1 Tick ─────────────────────────────────────────────────────── */

/* dm2_v2_runtime_v1_tick — advance V2 animation clock on V1 tick.
 * Called from game loop every ~55ms alongside dm2_v1_runtime_tick().
 * Pass current timestamp in ms (e.g. SDL_GetTicks()).
 *
 * Phase 4: also advances torch flicker (per-champion sine composite)
 * and lightning bloom fade timer.
 *
 * Source: SKULL.ASM PROCESS_TIMER_0C — torch timers
 *         dm2_v2_lighting.c dm2_v2_lighting_tick_bloom */
void dm2_v2_runtime_v1_tick(uint32_t now_ms);

/* ── Render Frame ────────────────────────────────────────────────── */

/* dm2_v2_runtime_render_frame — render one DM2 V2 viewport frame.
 *
 * party_dir:  facing direction 0=N 1=E 2=S 3=W
 * party_x, party_y: V1-snapped party grid position
 * framebuffer: target pixel buffer (FS_FB_W × FS_FB_H, indexed VGA)
 * fb_stride: bytes per row in framebuffer
 * view_w, view_h: viewport dimensions (≤ FS_FB_W × FS_FB_H)
 *
 * Renders at display rate using smooth-interpolated camera offset.
 * When smooth animation is active the camera offset is interpolated
 * between the previous and current V1-snapped position, giving
 * smooth visual motion that preserves exact V1 game-state semantics.
 *
 * Returns 0 on success. */
int dm2_v2_runtime_render_frame(int party_dir,
                                int party_x, int party_y,
                                uint8_t *framebuffer,
                                int fb_stride,
                                int view_w, int view_h);

/* ── Smooth Movement Triggers ────────────────────────────────────── */

/* dm2_v2_runtime_smooth_walk — begin smooth walk animation.
 * Call from game tick when party has moved from (fx,fy) to (tx,ty).
 * Duration: exactly 1 V1 tick (V1_TICK_MS = 55ms). */
void dm2_v2_runtime_smooth_walk(float fx, float fy, float tx, float ty);

/* dm2_v2_runtime_smooth_turn — begin smooth turn animation.
 * Call from game tick when party turns from fa to ta degrees.
 * Uses shortest-path rotation.  Duration: 1 V1 tick. */
void dm2_v2_runtime_smooth_turn(float fa, float ta);

/* dm2_v2_runtime_smooth_stairs — begin smooth stairs animation.
 * Call from game tick when party uses stairs.
 * vert_offset: camera vertical displacement (DM2 stairs raise/lower view).
 * Duration: 1 V1 tick with ease-in-out cubic. */
void dm2_v2_runtime_smooth_stairs(float fx, float fy,
                                  float tx, float ty,
                                  float vert_offset);

/* ── Phase 4: Enhanced Lighting and Outdoor FX ─────────────────────── */

/* dm2_v2_runtime_lighting_tick — advance enhanced lighting state.
 * Call from render loop at display rate (not V1 tick rate).
 * dt: seconds since last render frame (e.g. 0.016s for 60fps).
 * weather: DM2_WEATHER_CLEAR/RAIN/FOG/STORM from dm2_v1_weather.h.
 *
 * Advances outdoor cloud drift, tree sway, lightning sequence,
 * per-weather ambient tint, fog overlay animation.
 *
 * Deterministic fallback: no-op when enhanced outdoor is disabled.
 *
 * Source: dm2_v2_outdoor_enhanced.c dm2_v2_outdoor_fx_tick
 *         dm2_v2_lighting.c dm2_v2_fog_tick
 *         SKULL.ASM T600 — outdoor viewport rendering */
void dm2_v2_runtime_lighting_tick(float dt, int weather);

/* dm2_v2_runtime_outdoor_fx_trigger_lightning — trigger one-shot flash.
 * Call from dungeon event system when a spell/effect triggers lightning.
 *
 * Deterministic fallback: no-op when enhanced outdoor is disabled.
 * Source: dm2_v2_outdoor_enhanced.c dm2_v2_outdoor_fx_trigger_lightning */
void dm2_v2_runtime_outdoor_fx_trigger_lightning(void);

/* dm2_v2_runtime_fog_rebuild — rebuild fog density map from sources.
 * Call when dungeon geometry changes (e.g. door opened/closed).
 * Source: dm2_v2_lighting.c dm2_v2_fog_rebuild */
void dm2_v2_runtime_fog_rebuild(void);

/* dm2_v2_runtime_fog_set_weather — update weather-driven fog overlay.
 * Call when weather changes (dm2_v1_weather state transitions).
 *
 * Deterministic fallback: no-op when enhanced outdoor is disabled.
 * Source: dm2_v2_lighting.c dm2_v2_fog_set_weather
 *         docs/dm2_time.md §Weather §Fog */
void dm2_v2_runtime_fog_set_weather(int weather);

/* dm2_v2_runtime_set_enhanced_outdoor — enable/disable Phase 4 effects.
 * Call from phase gate bind when PROFILE domain is enabled.
 * enhanced: 1 = Phase 4 outdoor effects active; 0 = V1 fallback.
 *
 * When disabled, all Phase 4 functions (lighting_tick,
 * outdoor_fx_trigger_lightning, fog_rebuild, fog_set_weather)
 * are no-ops.  V1 game state is never affected. */
void dm2_v2_runtime_set_enhanced_outdoor(int enhanced);

/* ── Phase 3: Enhanced HUD Overlay ──────────────────────────────── */

/* dm2_v2_runtime_set_hud_enabled — enable/disable Phase 3 HUD overlay.
 * Called from dm2_v2_phase_gate_bind when HUD domain is enabled
 * (LAUNCH + PROFILE both active).  When disabled, dm2_v2_runtime_hud_render
 * is a no-op and the V1 source-locked HUD chrome is used instead.
 *
 * This function does NOT mutate V1 game state.  It only controls
 * whether the V2 presentation overlay is rendered on top of the V1 HUD.
 *
 * Source: SKULL.ASM T560 (HUD rendering)
 *         SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *         ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *         ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing) */
void dm2_v2_runtime_set_hud_enabled(int enhanced);

/* dm2_v2_runtime_get_hud — returns the global DM2 V2 HUD overlay state.
 * Allows callers to set compass direction, level depth, party gold,
 * champion bar stats, and action strip state from game runtime.
 * Phase 3 HUD — presentation-only; V1 game state unchanged. */
DM2_V2_HudOverlay *dm2_v2_runtime_get_hud(void);

/* dm2_v2_runtime_hud_render — render Phase 3 HUD overlay into framebuffer.
 * Called from the render pipeline after V1 viewport has been drawn.
 * When s_enhanced_hud=0, this is a no-op (V1 source-locked HUD intact).
 *
 * fb:       320×200 VGA framebuffer (indexed colour, 1 byte/pixel)
 * stride:   bytes per row (typically 320 for VGA mode 13h)
 * h_res:   horizontal resolution (typically 320)
 *
 * Source: SKULL.ASM T560 (HUD rendering)
 *         SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout) */
void dm2_v2_runtime_hud_render(uint8_t *fb, int stride, int h_res);

/* ── Source evidence ─────────────────────────────────────────────── */
const char *dm2_v2_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_RUNTIME_H */