#ifndef NEXUS_V2_SMOOTH_MOVEMENT_H
#define NEXUS_V2_SMOOTH_MOVEMENT_H

/*
 * Nexus V2 Smooth Movement — Phase 5
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
 *         DM2 V2 smooth: dm2_v2_smooth_movement.c
 */

#include <stdint.h>
#include "dm1_v2_anim_timing.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Smooth movement state ─────────────────────────────────────────── */

/* Walk animation: smooth X/Y transition over 1 V1 tick */
typedef struct {
    V2_Anim anim_x;
    V2_Anim anim_y;
    int active;
} Nexus_V2_SmoothWalk;

/* Turn animation: smooth angle transition over 1 V1 tick */
typedef struct {
    V2_Anim anim_angle;
    int active;
} Nexus_V2_SmoothTurn;

/* Stairs animation: smooth X/Y transition + vertical shift */
typedef struct {
    V2_Anim anim_x;
    V2_Anim anim_y;
    V2_Anim anim_vert;  /* vertical camera offset */
    int active;
} Nexus_V2_SmoothStairs;

/* Global smooth movement state */
typedef struct {
    Nexus_V2_SmoothWalk   walk;
    Nexus_V2_SmoothTurn   turn;
    Nexus_V2_SmoothStairs stairs;
    float prev_x;       /* party X before last V1 tick */
    float prev_y;       /* party Y before last V1 tick */
    float prev_angle;   /* party angle before last V1 tick */
    int   has_prev;     /* 0 until first nexus_v2_smooth_tick has recorded */
} Nexus_V2_SmoothState;

/* ── Lifecycle ─────────────────────────────────────────────────────── */

/* nexus_v2_smooth_init — reset all smooth state */
void nexus_v2_smooth_init(Nexus_V2_SmoothState *s);

/* nexus_v2_smooth_start_walk — begin visual walk transition.
 * from_x/y: party position before move
 * to_x/y:   party position after move (snapped game state)
 * Uses ease-out cubic over 1 V1 tick (V1_TICK_MS). */
void nexus_v2_smooth_start_walk(Nexus_V2_SmoothState *s,
                                float from_x, float from_y,
                                float to_x,   float to_y);

/* nexus_v2_smooth_start_turn — begin visual turn transition.
 * from_angle: facing before turn (0-359)
 * to_angle:  facing after turn (0-359, shortest path) */
void nexus_v2_smooth_start_turn(Nexus_V2_SmoothState *s,
                               float from_angle, float to_angle);

/* nexus_v2_smooth_start_stairs — begin visual stairs transition.
 * from_x/y: party position before stairs
 * to_x/y:   party position after stairs
 * from_vert: camera height before, to_vert: after */
void nexus_v2_smooth_start_stairs(Nexus_V2_SmoothState *s,
                                  float from_x, float from_y,
                                  float to_x,   float to_y,
                                  float from_vert, float to_vert);

/* nexus_v2_smooth_update — advance animations by dt_ms.
 * Call once per render frame (may be called multiple times per V1 tick). */
void nexus_v2_smooth_update(Nexus_V2_SmoothState *s, float dt_ms);

/* nexus_v2_smooth_tick — called once per V1 tick (55ms).
 * Records pre-tick position for next smooth transition. */
void nexus_v2_smooth_tick(Nexus_V2_SmoothState *s,
                         float game_x, float game_y, float game_angle);

/* ── Query ─────────────────────────────────────────────────────────── */

/* nexus_v2_smooth_get_position — returns interpolated camera X/Y
 * when animation is active, or the game-state position when idle. */
void nexus_v2_smooth_get_position(const Nexus_V2_SmoothState *s,
                                 float *out_x, float *out_y);

/* nexus_v2_smooth_get_vertical — returns vertical camera offset
 * (only non-zero during stairs animation). */
float nexus_v2_smooth_get_vertical(const Nexus_V2_SmoothState *s);

/* nexus_v2_smooth_get_angle — returns interpolated camera angle. */
float nexus_v2_smooth_get_angle(const Nexus_V2_SmoothState *s);

/* nexus_v2_smooth_is_active — returns 1 if any animation is running. */
int nexus_v2_smooth_is_active(const Nexus_V2_SmoothState *s);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V2_SMOOTH_MOVEMENT_H */