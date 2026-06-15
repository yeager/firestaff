/*
 * csb_v2_runtime.h — CSB V2 Runtime Integration (Phase 5: smooth movement)
 *
 * Wires CSB_V2_ViewportState (smooth movement + V2_AnimClock) into the
 * CSB V1 game tick path.  Mirrors dm2_v2_runtime.h so CSB and DM2 share
 * the same V2 presentation contract for game-loop integration.
 *
 *   Game Loop (V1 tick):
 *     csb_v1_runtime_tick()        → advance V1 game state (snaps)
 *     csb_v2_runtime_v1_tick()     → advance V2 animation clock
 *     [on move]: csb_v2_runtime_bind_to_v1_tick(profile)
 *                                  → begin smooth walk animation
 *     [on turn]:                    → begin smooth turn animation
 *
 *     csb_v2_runtime_render_frame()→ update smooth state, render viewport
 *       csb_v2_viewport_render_frame() → drive V2_AnimClock
 *       csb_v2_smooth_get_x/y/angle()  → read interpolated position
 *
 * V2 visual only: smooth interpolation never changes game state.
 * The V1 party state (profile->party_x, profile->party_y, profile->party_dir)
 * is mutated only by V1 code; CSB V2 only observes the change and starts
 * a smooth visual transition.
 *
 * Phase 5 smooth movement triggers (manual, for tests / external callers):
 *   - Walk: ease-out cubic  — snappy but not jarring
 *   - Turn: ease-out quad   — quick rotation snap
 *   - Stairs: ease-in-out cubic — deliberate vertical feel
 *
 * Source: ReDMCSB COMMAND.C F0380 (queue dispatch)
 *         ReDMCSB CLIKMENU.C F0365 (turn), F0366 (move)
 *         ReDMCSB GAMELOOP.C (tick cadence, VBLANK-locked 55ms)
 *         ReDMCSB CLIKMENU.C F0364 (stairs)
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *   v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync
 *   v22_smooth_update_from_clock / v22_smooth_get_x/y/angle
 * Reference: dm2_v2_runtime.c (DM2 V2 runtime integration — same shape)
 *   dm2_v2_runtime_smooth_walk / dm2_v2_runtime_smooth_turn
 *   dm2_v2_runtime_bind_via_callbacks (move/turn callbacks from V1)
 */

#ifndef FIRESTAFF_CSB_V2_RUNTIME_H
#define FIRESTAFF_CSB_V2_RUNTIME_H

#include <stdint.h>
#include "csb_v2_viewport_renderer.h"
#include "csb_v1_runtime_pc34_compat.h" /* CSB_V1_RuntimeProfile (binding target) */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Global CSB V2 State Accessors ──────────────────────────────── */

/* csb_v2_runtime_get_viewport — returns the global CSB V2 viewport state.
 * Initialised by csb_v2_runtime_init().  Contains smooth movement
 * (csb_v2_smooth_*) and V2_AnimClock. */
CSB_V2_ViewportState *csb_v2_runtime_get_viewport(void);

/* ── Lifecycle ─────────────────────────────────────────────────────── */

/* csb_v2_runtime_init — initialise the CSB V2 runtime.
 * Must be called once at game startup before any other csb_v2_runtime_*
 * function.  scale: 2 = V2.0/V2.1 EPX, 4 = V2.2 high-res. */
void csb_v2_runtime_init(int scale);

/* csb_v2_runtime_cleanup — release runtime state.  Safe to call
 * multiple times.  After cleanup, csb_v2_runtime_init() must be
 * called again before any other API. */
void csb_v2_runtime_cleanup(void);

/* ── V1 Tick ─────────────────────────────────────────────────────── */

/* csb_v2_runtime_v1_tick — advance V2 animation clock on V1 tick.
 * Called from game loop every ~55ms alongside csb_v1_runtime_tick().
 * Pass current timestamp in ms (e.g. SDL_GetTicks()).
 *
 * Source: ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms) */
void csb_v2_runtime_v1_tick(uint32_t now_ms);

/* ── Render Frame ────────────────────────────────────────────────── */

/* csb_v2_runtime_render_frame — drive V2 smooth animation one render
 * frame.  Called from game loop at display rate (~16ms for 60fps).
 * now_ms: current wall-clock timestamp in ms.
 *
 * Advances the smooth movement animations by elapsed wall-clock time
 * since the previous render frame.  No-op if init was not called. */
void csb_v2_runtime_render_frame(uint32_t now_ms);

/* ── Smooth Movement Triggers (manual API) ──────────────────────── */

/* csb_v2_runtime_smooth_walk — begin smooth walk animation.
 * Call when party has moved from (fx,fy) to (tx,ty) on a V1 tick.
 * Duration: exactly 1 V1 tick (CSB_V2_WALK_EASE = ease-out cubic). */
void csb_v2_runtime_smooth_walk(float fx, float fy, float tx, float ty);

/* csb_v2_runtime_smooth_turn — begin smooth turn animation.
 * fa/ta: facing angle in degrees (0-359).  CSB uses 90-degree
 * compass directions (0=N, 90=E, 180=S, 270=W).  Duration: 1 V1 tick
 * (CSB_V2_TURN_EASE = ease-out quad). */
void csb_v2_runtime_smooth_turn(float fa, float ta);

/* csb_v2_runtime_smooth_stairs — begin smooth stairs animation.
 * Call when party takes stairs (F0364 in CLIKMENU.C).
 * vert_offset: camera vertical displacement.
 * Duration: 1 V1 tick (CSB_V2_STAIRS_EASE = ease-in-out cubic). */
void csb_v2_runtime_smooth_stairs(float fx, float fy,
                                  float tx, float ty,
                                  float vert_offset);

/* ── Runtime Binding (Phase 5 binding seam) ─────────────────────── */

/* csb_v2_runtime_bind_to_v1 — bind CSB V2 smooth movement to a CSB V1
 * runtime profile.  After binding, csb_v2_runtime_v1_tick() will track
 * the profile's party position and trigger smooth walk/turn animations
 * automatically when the V1 state changes (delta > 0 on the tile or
 * direction axis).
 *
 * profile: CSB V1 runtime profile (must outlive the binding).
 *
 * Idempotent: calling twice rebinds to the new profile.
 * Pass NULL to unbind.
 *
 * Source: ReDMCSB COMMAND.C F0380 (queue dispatch)
 *         ReDMCSB CLIKMENU.C F0365 (turn), F0366 (move)
 *         ReDMCSB GAMELOOP.C (V1 tick cadence)
 *
 * Phase 5 binding seam: V1 state writes are observed (read-only) at
 * each v1_tick; smooth animation starts are pure presentation. */
void csb_v2_runtime_bind_to_v1(CSB_V1_RuntimeProfile *profile);

/* csb_v2_runtime_is_bound — returns 1 if a profile is currently bound,
 * 0 otherwise. */
int csb_v2_runtime_is_bound(void);

/* csb_v2_runtime_force_sync — re-anchor the binding's "from" state to
 * the profile's current party state without triggering any animation.
 * Useful after loading a saved game (party state jumped; we don't want
 * a phantom walk animation to play). */
void csb_v2_runtime_force_sync(void);

/* ── Source evidence ─────────────────────────────────────────────── */
const char *csb_v2_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_RUNTIME_H */
