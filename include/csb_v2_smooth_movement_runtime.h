#ifndef FIRESTAFF_CSB_V2_SMOOTH_MOVEMENT_RUNTIME_H
#define FIRESTAFF_CSB_V2_SMOOTH_MOVEMENT_RUNTIME_H
#include "csb_v2_smooth_movement.h"
#include "csb_v2_phase_gate_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * csb_v2_smooth_movement_runtime.h — CSB V2 Phase 5 Smooth Movement Runtime
 *
 * V1→V2 smooth movement bridge for Chaos Strikes Back.
 * Mirrors nexus_v2_smooth_movement_runtime.c pattern (sibling wire-up).
 *
 * Phase-gated tick wrapper around the CSB V2 smooth movement module's
 * global state. When V1 is the active presentation, the per-frame tick
 * is a no-op (V1 smooth state preserved).
 *
 * Source-lock anchors:
 *   ReDMCSB GROUP.C:1695-1770 (F0207 creature attack)
 *   skproject/SKWIN/SkWinCore.cpp (ease-out cubic / ease-in-out cubic)
 *   CSBWin/resurrect/CsbV2FieldSmooth.cpp (CSBWin reimpl)
 *   ReDMCSB GAMELOOP.C:47-50 (V1 tick cadence 55ms)
 *   dm2_v2_smooth_movement_runtime.c (sibling DM2 V2 wire-up pattern)
 *   nexus_v2_smooth_movement_runtime.c (sibling Nexus V2 wire-up pattern)
 * ================================================================ */

/* ── Lifecycle ─────────────────────────────────────────────────── */
void csb_v2_smooth_movement_runtime_init(void);
void csb_v2_smooth_movement_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
void csb_v2_smooth_movement_runtime_set_gate_config(const CSB_V2_PhaseGateConfig *config);

/* ── Per-frame tick ───────────────────────────────────────────── */
void csb_v2_smooth_movement_runtime_tick(float dt_seconds);

/* ── Status ────────────────────────────────────────────────────── */
int csb_v2_smooth_movement_runtime_is_active(void);

/* ── Triggers (called by V1 game-loop on party-state delta) ── */
void csb_v2_smooth_movement_runtime_start_walk(float from_x, float from_y,
    float to_x, float to_y);
void csb_v2_smooth_movement_runtime_start_turn(float from_angle, float to_angle);
void csb_v2_smooth_movement_runtime_start_stairs(float from_x, float from_y,
    float to_x, float to_y, float vert_offset);

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
void csb_v2_smooth_movement_runtime_force_active_for_test(int active);

/* Observability: total ticks executed (monotonic, reset by init). */
int csb_v2_smooth_movement_runtime_tick_count(void);

/* Source evidence citation */
const char *csb_v2_smooth_movement_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_SMOOTH_MOVEMENT_RUNTIME_H */