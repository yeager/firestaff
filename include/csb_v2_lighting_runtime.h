#ifndef FIRESTAFF_CSB_V2_LIGHTING_RUNTIME_H
#define FIRESTAFF_CSB_V2_LIGHTING_RUNTIME_H
#include "csb_v2_lighting_dynamic.h"
#include "csb_v2_phase_gate_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * csb_v2_lighting_runtime.h — CSB V2 Phase 4 Lighting Runtime
 *
 * V1→V2 lighting bridge for Chaos Strikes Back.
 * Mirrors nexus_v2_lighting_runtime.c pattern (sibling wire-up).
 *
 * Phase-gated tick wrapper around the CSB V2 lighting module's global
 * state. When V1 is the active presentation, the per-frame tick is a
 * no-op (V1 light state preserved).
 *
 * Source-lock anchors:
 *   ReDMCSB LIGHT.C F0380 (light radius + flicker timing)
 *   ReDMCSB LIGHT.C F0381 (torch source)
 *   ReDMCSB LIGHT.C F0382 (ambient per dungeon level)
 *   CSBWin/resurrect/CsbV2FieldLight.cpp (CSBWin reimpl)
 *   dm2_v2_lighting_runtime.c (sibling DM2 V2 wire-up pattern)
 *   nexus_v2_lighting_runtime.c (sibling Nexus V2 wire-up pattern)
 * ================================================================ */

/* ── Lifecycle ─────────────────────────────────────────────────── */
void csb_v2_lighting_runtime_init(void);
void csb_v2_lighting_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
void csb_v2_lighting_runtime_set_gate_config(const CSB_V2_PhaseGateConfig *config);

/* ── Per-frame tick ───────────────────────────────────────────── */
void csb_v2_lighting_runtime_tick(float dt_seconds);

/* ── Status ────────────────────────────────────────────────────── */
int csb_v2_lighting_runtime_is_active(void);

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
void csb_v2_lighting_runtime_force_active_for_test(int active);

/* Observability: total ticks executed (monotonic, reset by init). */
int csb_v2_lighting_runtime_tick_count(void);

/* Source evidence citation */
const char *csb_v2_lighting_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_LIGHTING_RUNTIME_H */