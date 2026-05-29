#ifndef FIRESTAFF_DM2_V2_RUNTIME_H
#define FIRESTAFF_DM2_V2_RUNTIME_H

/*
 * DM2 V2 Runtime — Phase 5: Smooth Movement Integration
 *
 * Wires DM2_V2_ViewportState (smooth movement + animation clock) into
 * the Firestaff game loop.  Provides:
 *
 *   1. Global DM2 V2 viewport state (smooth movement + V2_AnimClock)
 *   2. V2 render entry point (called from game loop per render frame)
 *   3. V2 V1-tick entry point (called from game loop per V1 tick)
 *
 * V1 invariant preserved: game state snaps on V1 ticks (18.2 Hz).
 * V2 visual interpolation: camera smoothly transitions between V1
 *   snaps over 1 V1 tick using V2_AnimClock sub-tick + V2_Anim easing.
 *
 * Smooth movement triggers:
 *   - Walk: ease-out cubic  — snappy, natural deceleration
 *   - Turn: ease-out quad   — quick rotation snap (shortest path)
 *   - Stairs: ease-in-out cubic + vertical offset
 *
 * Source: SKULL.ASM T520  — party/movement tick
 *         SKULL.ASM T560  — dungeon viewport rendering
 *         SKULL.ASM T600  — outdoor viewport rendering
 *         ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *         ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *   v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync
 *   v22_smooth_update_from_clock / v22_smooth_get_x/y/angle
 */

#include <stdint.h>
#include "dm2_v2_viewport_renderer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Global DM2 V2 Viewport State ─────────────────────────────────── */

/* dm2_v2_runtime_get_viewport — returns the global DM2 V2 viewport state.
 * Initialised by dm2_v2_runtime_init(). */
DM2_V2_ViewportState *dm2_v2_runtime_get_viewport(void);

/* ── Lifecycle ─────────────────────────────────────────────────────── */

/* dm2_v2_runtime_init — initialise the DM2 V2 runtime.
 * Must be called once at game startup before any other dm2_v2_runtime_*
 * functions.  scale: 2 = V2.0/V2.1 EPX, 4 = V2.2 high-res. */
void dm2_v2_runtime_init(int scale);

/* ── V1 Tick ─────────────────────────────────────────────────────── */

/* dm2_v2_runtime_v1_tick — advance V2 animation clock on V1 tick.
 * Called from game loop every ~55ms alongside dm2_v1_runtime_tick().
 * Pass current timestamp in ms (e.g. SDL_GetTicks()). */
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

/* ── Source evidence ─────────────────────────────────────────────── */
const char *dm2_v2_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_RUNTIME_H */
