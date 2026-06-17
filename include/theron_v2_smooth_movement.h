/*
 * theron_v2_smooth_movement.h
 *
 * Theron's Quest V2.2 Smooth Movement — visual interpolation of party
 * tile transitions and 4-direction compass turns.
 *
 * Phase 5: presentation-only.  V1 game state (party position + direction
 * in the Theron V1 world) is mutated only by Theron V1 code (THQUEST.ASM
 * T520/T560/T600 + theron_v1_dungeon_progression.c).  V2 only observes
 * the change and starts a smooth visual transition that completes in
 * exactly one V1 tick (~55ms, V1_TICK_MS).
 *
 * Theron-specific:
 *   - 4-direction compass (N=0, E=1, S=2, W=3).  Diagonals are not used.
 *   - Walk is single-axis (party moves one tile on X or Y per step).
 *   - "Stairs" do not exist in Theron; teleporter chains replace them
 *     and are handled by theron_v1_dungeon_progression (F0364 in
 *     ReDMCSB CLIKMENU.C).  This module exposes a fade animation for
 *     the V2 presenter to use, but it never touches V1 state.
 *
 * Easing:
 *   - Walk: ease-out cubic  — snappy but not jarring
 *   - Turn: ease-out quad   — quick rotation snap
 *   - Fade (teleporter): ease-in-out cubic — deliberate dissolve
 *
 * Phase gate: this module is presentation-only and is gated by the
 * THERON_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION domain in
 * theron_v2_phase_gate_pc34.h.  V1 cooldowns, collision, sensor timing,
 * and dungeon progression are unaffected.
 *
 * Source: THQUEST.ASM T520 (party/movement tick), T560 (dungeon viewport),
 *         T600 (UI overlay zones), T700 (timers / world tick).
 *         ReDMCSB CLIKMENU.C F0365 (turn), F0366 (move), F0364 (stairs
 *         reference for ease choice), COMMAND.C F0380 (queue dispatch).
 *         HuC6260/HuC6270 VDC/VCE (PC Engine video chip datasheet).
 *         HuC6280 CPU + ADPCM (PC Engine CPU/audio datasheets).
 * Reference: dm1_v2_smooth_movement_pc34.c, csb_v2_smooth_movement.c,
 *            dm2_v2_smooth_movement.c, nexus_v2_smooth_movement.c
 *            (cross-game V2 smooth-movement pattern).
 */

#ifndef FIRESTAFF_THERON_V2_SMOOTH_MOVEMENT_H
#define FIRESTAFF_THERON_V2_SMOOTH_MOVEMENT_H

#include "dm1_v2_anim_timing.h"
#include "theron_v2_phase_gate_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Theron V2 compass → angle mapping (degrees, 0=N, 90=E, 180=S, 270=W).
 * Matches theron_v1_viewport.c:391 — `world->party.leader_dir & 3`. */
#define THERON_V2_DIR_N 0
#define THERON_V2_DIR_E 1
#define THERON_V2_DIR_S 2
#define THERON_V2_DIR_W 3

/* Compass → angle in degrees, used for smooth turn interpolation. */
float theron_v2_dir_to_angle(int dir);

/* Lifecycle: initialise the module-level smooth-movement state.  Idempotent. */
void theron_v2_smooth_init(void);

/* Start a smooth walk animation along a single axis.  Theron walk is
 * one-axis-at-a-time (no diagonal), so callers should pass (from, to)
 * on either X or Y and leave the other as 0/0. */
void theron_v2_smooth_start_walk(float fx, float tx, float fy, float ty);

/* Start a smooth turn animation.  fa/ta in degrees (0..359).
 * Shortest-path wrap is handled internally (e.g. 270→90 takes the
 * +180 path through 0, not the -180 path through 180). */
void theron_v2_smooth_start_turn(float fa, float ta);

/* Start a smooth fade animation.  fade_to in [0.0, 1.0]: 0 = fully
 * visible, 1 = fully faded.  Used by V2 presenter for teleporter
 * chain transitions.  Duration: 1 V1 tick (ease-in-out cubic). */
void theron_v2_smooth_start_fade(float from, float to);

/* Per-frame update.  Drives all anims forward by clock->dt_ms.  No-op
 * if clock is NULL. */
void theron_v2_smooth_update_from_clock(const V2_AnimClock *clock);

/* Read-side accessors.  Used by the M11 viewport to draw the
 * interpolated tile, the V2 HUD to draw the smoothed compass needle,
 * and the V2 presenter to read the fade alpha. */
float theron_v2_smooth_get_x(void);
float theron_v2_smooth_get_y(void);
float theron_v2_smooth_get_angle(void);
float theron_v2_smooth_get_fade(void);

/* 1 if any smooth animation is still in flight, 0 if all are settled. */
int   theron_v2_smooth_is_moving(void);

/* 1 if the fade animation is currently active, 0 otherwise.
 * Used by the V2 presenter to choose between "draw scene normally"
 * and "draw scene with fade overlay". */
int   theron_v2_smooth_fade_active(void);

/* Source-evidence string.  Cited by probes and audit gates. */
const char *theron_v2_smooth_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_SMOOTH_MOVEMENT_H */
