#ifndef FIRESTAFF_NEXUS_V2_SMOOTH_MOVEMENT_RUNTIME_H
#define FIRESTAFF_NEXUS_V2_SMOOTH_MOVEMENT_RUNTIME_H
#include "nexus_v2_smooth_movement.h"
#include "nexus_v2_phase_gate_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * nexus_v2_smooth_movement_runtime.h — Nexus V2 Phase 5 Smooth Movement Runtime
 *
 * V1→V2 smooth movement bridge for Dungeon Master Nexus (Saturn).
 * Mirrors dm2_v2_smooth_movement_runtime pattern (sibling DM2 wire-up).
 *
 * Wraps the nexus_v2_smooth_movement.c state + tick APIs behind a
 * phase-gated runtime.  When V1 is the active presentation, the per-tick
 * advance is a no-op (V1 smooth state preserved).
 *
 * Phase 5 rule: smooth movement interpolation is V2-only. When
 * v2PresentationEnabled + v2ConfigPersistenceEnabled are both enabled
 * (or force_active_for_test=1), the runtime advances the smooth state
 * per V1 tick cadence (55ms). V1 invariant: tick must not run when V1
 * is active.
 *
 * Source-lock anchors:
 *   ReDMCSB GROUP.C:1695-1770 (F0207 creature attack projectile payload)
 *   skproject/SKULLWIN/c_creature.cpp (DM2 smooth-movement sibling)
 *   skproject/SKWIN/SkWinCore.cpp (ease-out cubic / ease-in-out cubic)
 *   ReDMCSB GAMELOOP.C:47-50 (V1 tick cadence 55ms)
 *   Saturn NEXUS.BIN (Saturn-specific interpolation timing)
 * ================================================================ */

/* ── Lifecycle ─────────────────────────────────────────────────── */
void nexus_v2_smooth_movement_runtime_init(void);
void nexus_v2_smooth_movement_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
void nexus_v2_smooth_movement_runtime_set_gate_config(const NEXUS_V2_PhaseGateConfig *config);

/* ── Per-frame tick ───────────────────────────────────────────── */
void nexus_v2_smooth_movement_runtime_tick(float dt_ms);

/* ── Status ────────────────────────────────────────────────────── */
int nexus_v2_smooth_movement_runtime_is_active(void);

/* ── State accessors (read-only) ───────────────────────────────── */
const Nexus_V2_SmoothState *nexus_v2_smooth_movement_runtime_get_state(void);

/* ── Triggers (called by V1 game-loop on party-state delta) ── */
void nexus_v2_smooth_movement_runtime_start_walk(float from_x, float from_y,
    float to_x, float to_y);
void nexus_v2_smooth_movement_runtime_start_turn(float from_angle, float to_angle);
void nexus_v2_smooth_movement_runtime_start_stairs(float from_x, float from_y,
    float to_x, float to_y, float from_vert, float to_vert);

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
void nexus_v2_smooth_movement_runtime_force_active_for_test(int active);

/* Observability: total ticks executed (monotonic, reset by init). */
int nexus_v2_smooth_movement_runtime_tick_count(void);

/* Source evidence citation */
const char *nexus_v2_smooth_movement_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V2_SMOOTH_MOVEMENT_RUNTIME_H */