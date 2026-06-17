#ifndef FIRESTAFF_NEXUS_V2_LIGHTING_RUNTIME_H
#define FIRESTAFF_NEXUS_V2_LIGHTING_RUNTIME_H
#include "nexus_v2_lighting.h"
#include "nexus_v2_phase_gate_pc34.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * nexus_v2_lighting_runtime.h — Nexus V2 Phase 4 Lighting Runtime
 *
 * V1→V2 lighting bridge for Dungeon Master Nexus (Saturn).
 * Mirrors dm2_v2_lighting_runtime.c pattern (sibling DM2 wire-up).
 *
 * Wraps the nexus_v2_lighting.c state + tick APIs behind a phase-gated
 * runtime. When V1 is the active presentation, the per-frame tick is
 * a no-op (V1 light state preserved).
 *
 * Phase 4 rule: lighting + ambient + torch flicker are V2-only. When
 * v2PresentationEnabled + v2ConfigPersistenceEnabled are both enabled
 * (or force_active_for_test=1), the runtime advances torch_flicker_phase
 * + ambient state per V1 tick cadence.  V1 invariant: lighting tick
 * must not run when V1 is active.
 *
 * Source-lock anchors:
 *   Saturn NEXUS.BIN VDP1 polygon lighting
 *   Saturn NEXUS.BIN VDP2 shadow layer
 *   DMDF level data (per-tile light emission values, DGN format)
 *   ReDMCSB LIGHT.C F0380 (light radius + flicker timing)
 *   ReDMCSB COMMAND.C F0209 (spell-light colour binding)
 *   ReDMCSB DUNGEON.C (torch position tracking in party state)
 * ================================================================ */

/* ── Lifecycle ─────────────────────────────────────────────────── */
void nexus_v2_lighting_runtime_init(void);
void nexus_v2_lighting_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
void nexus_v2_lighting_runtime_set_gate_config(const NEXUS_V2_PhaseGateConfig *config);

/* ── Per-frame tick ───────────────────────────────────────────── */
void nexus_v2_lighting_runtime_tick(float dt_seconds);

/* ── Status ────────────────────────────────────────────────────── */
int nexus_v2_lighting_runtime_is_active(void);

/* ── State accessors (read-only) ───────────────────────────────── */
const Nexus_V2_LightingState *nexus_v2_lighting_runtime_get_state(void);

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
void nexus_v2_lighting_runtime_force_active_for_test(int active);

/* Observability: total ticks executed (monotonic, reset by init). */
int nexus_v2_lighting_runtime_tick_count(void);

/* Source evidence citation */
const char *nexus_v2_lighting_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V2_LIGHTING_RUNTIME_H */