#ifndef FIRESTAFF_DM2_V2_SMOOTH_MOVEMENT_H
#define FIRESTAFF_DM2_V2_SMOOTH_MOVEMENT_H

/*
 * DM2 V2 Smooth Movement — Phase 5
 *
 * V1 movement is instant (one tick = new position).
 * V2 interpolates between positions over exactly 1 V1 tick (55ms):
 *   - Walk: ease-out cubic
 *   - Turn: ease-out quad
 *   - Stairs: ease-in-out cubic + vertical shift
 *
 * The interpolation is purely visual — game state updates instantly
 * as in V1, but the camera smoothly transitions.
 *
 * Uses the shared V2_Anim / V2_AnimClock infrastructure from
 * dm1_v2_anim_timing.h, which is game-agnostic.
 *
 * Key invariant: game state ONLY advances on V1 ticks.
 * Visual state interpolates between previous and current V1 state.
 *
 * Source: SKULL.ASM T520 — party/movement tick
 *         DM1 V2.2 smooth movement: dm1_v2_smooth_movement_pc34.c
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
} DM2_V2_SmoothWalk;

/* Turn animation: smooth angle transition over 1 V1 tick */
typedef struct {
    V2_Anim anim_angle;
    int active;
} DM2_V2_SmoothTurn;

/* Stairs animation: smooth X/Y transition + vertical shift over 1 V1 tick */
typedef struct {
    V2_Anim anim_x;
    V2_Anim anim_y;
    V2_Anim anim_vert;  /* vertical camera offset */
    int active;
} DM2_V2_SmoothStairs;

/* Global smooth movement state */
typedef struct {
    DM2_V2_SmoothWalk  walk;
    DM2_V2_SmoothTurn  turn;
    DM2_V2_SmoothStairs stairs;
} DM2_V2_SmoothState;

/* ── Lifecycle ─────────────────────────────────────────────────────── */

/* dm2_v2_smooth_init — reset all smooth state */
void dm2_v2_smooth_init(DM2_V2_SmoothState *s);

/* dm2_v2_smooth_start_walk — begin visual walk transition.
 * from_x/y: party position before move
 * to_x/y:   party position after move
 * Uses ease-out cubic over 1 V1 tick (V1_TICK_MS). */
void dm2_v2_smooth_start_walk(DM2_V2_SmoothState *s,
                               float from_x, float from_y,
                               float to_x,   float to_y);

/* dm2_v2_smooth_start_turn — begin visual turn transition.
 * from_angle: facing before turn (0-359)
 * to_angle:  facing after turn (0-359, shortest path) */
void dm2_v2_smooth_start_turn(DM2_V2_SmoothState *s,
                              float from_angle, float to_angle);

/* dm2_v2_smooth_start_stairs — begin visual stairs transition.
 * from_x/y, to_x/y: horizontal position
 * vert_offset: camera vertical offset (DM2 stairs go up/down) */
void dm2_v2_smooth_start_stairs(DM2_V2_SmoothState *s,
                                 float from_x, float from_y,
                                 float to_x,   float to_y,
                                 float vert_offset);

/* dm2_v2_smooth_tick — advance animation state by dt_ms */
void dm2_v2_smooth_tick(DM2_V2_SmoothState *s, float dt_ms);

/* ── Query ─────────────────────────────────────────────────────────── */

/* dm2_v2_smooth_get_position — returns interpolated X/Y.
 * Only valid when a walk or stairs animation is active. */
void dm2_v2_smooth_get_position(const DM2_V2_SmoothState *s,
                                 float *out_x, float *out_y);

/* dm2_v2_smooth_get_vertical — returns interpolated vertical offset.
 * Returns 0.0 when stairs animation is inactive. */
float dm2_v2_smooth_get_vertical(const DM2_V2_SmoothState *s);

/* dm2_v2_smooth_get_angle — returns interpolated facing.
 * Only valid when a turn animation is active. */
float dm2_v2_smooth_get_angle(const DM2_V2_SmoothState *s);

/* dm2_v2_smooth_is_active — true if any animation is in progress */
int  dm2_v2_smooth_is_active(const DM2_V2_SmoothState *s);

/* dm2_v2_smooth_is_turning — true if a turn animation is in progress */
int  dm2_v2_smooth_is_turning(const DM2_V2_SmoothState *s);

/* ── Source evidence ───────────────────────────────────────────────── */
const char *dm2_v2_smooth_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_SMOOTH_MOVEMENT_H */
