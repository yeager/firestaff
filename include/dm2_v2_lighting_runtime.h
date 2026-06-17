#ifndef FIRESTAFF_DM2_V2_LIGHTING_RUNTIME_H
#define FIRESTAFF_DM2_V2_LIGHTING_RUNTIME_H
#include "dm2_v2_lighting.h"
#include "dm2_v2_outdoor_enhanced.h"
#include "dm2_v2_phase_gate.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * dm2_v2_lighting_runtime.h — DM2 V2 Phase 4 Lighting Runtime
 *
 * V1→V2 lighting + outdoor FX bridge for Dungeon Master II: Skullkeep.
 *
 * Wraps the dm2_v2_lighting.c + dm2_v2_outdoor_enhanced.c state/tick
 * APIs behind a phase-gated runtime. When V1 is the active presentation,
 * the per-frame tick is a no-op (V1 state preserved).
 *
 * Phase 4 rule: lighting + outdoor FX are V2-only. When V2 launch+profile
 * are both enabled (or force_active_for_test=1), the runtime ticks
 * lighting.bloom_timer + outdoor_fx state per V1 tick cadence (55ms).
 * V1 invariants: dm2_v2_lighting_init must not be called when V2 is off,
 * and dm2_v2_lighting_tick_bloom + dm2_v2_outdoor_fx_tick must not run.
 *
 * Source-lock anchors:
 *   SKULL.ASM PROCESS_TIMER_0C — per-champion torch timers
 *   SKULL.ASM T560              — indoor dungeon viewport (lighting read)
 *   SKULL.ASM T600              — outdoor viewport (outdoor FX read)
 *   ReDMCSB PANEL.C:367-428     — DM1 palette lighting semantics
 *   ReDMCSB PANEL.C:367-428     — per-champion torch flicker timing
 *   dm2_v1_weather.c            — weather state source (sun position,
 *                                  rain intensity, ambient)
 *   dm2_v2_lighting.c           — state + bloom tick
 *   dm2_v2_outdoor_enhanced.c   — outdoor FX state + tick
 *   dm2_v2_phase_gate.h         — DM2_V2_PHASE_DOMAIN_HUD gate
 *                                 (Phase 4 lighting shares the HUD
 *                                  domain gate since both are
 *                                  presentation-only overlays)
 * ================================================================ */

/* ── Lifecycle ─────────────────────────────────────────────────── */
void dm2_v2_lighting_runtime_init(void);
void dm2_v2_lighting_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
void dm2_v2_lighting_runtime_set_gate_config(const DM2_V2_PhaseGateConfig *config);

/* ── Per-frame tick ───────────────────────────────────────────── */
/* Tick lighting.bloom_timer + outdoor FX state.  Phase-gated: when V2
 * is disabled (and force_active_for_test=0), this is a no-op so the
 * V1 weather state is preserved.
 *
 * dt_seconds is the elapsed time since last tick (typically 0.055 = 55ms
 * for one V1 tick).
 *
 * weather is one of the DM2_WEATHER_* constants from dm2_v1_weather.h
 * (0=CLEAR, 1=CLOUDY, 2=RAIN, 3=STORM, 4=FOG, 5=SNOW, etc.).  Only used
 * for outdoor FX tick — lighting is weather-independent (per-champion
 * torch timers + bloom only). */
void dm2_v2_lighting_runtime_tick(float dt_seconds, int weather);

/* ── Status ────────────────────────────────────────────────────── */
/* Returns 1 if the lighting runtime is active (V2 enabled).  Returns
 * 0 if V1 is the active presentation. */
int dm2_v2_lighting_runtime_is_active(void);

/* ── State accessors (read-only) ───────────────────────────────── */
/* Read-only access to the lighting state for the V2 renderer.  When
 * the runtime is inactive, returns NULL (renderer should fall back to
 * V1 lighting palette). */
const DM2_V2_LightingState *dm2_v2_lighting_runtime_get_state(void);
const DM2_V2_OutdoorFX *dm2_v2_lighting_runtime_get_outdoor_fx(void);

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
/* Force-activates the lighting runtime regardless of phase gate
 * (used by wire-up probes to verify the tick path).  Not called by
 * production code. */
void dm2_v2_lighting_runtime_force_active_for_test(int active);

/* Observability: total ticks executed (monotonic, reset by init). */
int dm2_v2_lighting_runtime_tick_count(void);

/* Source evidence citation */
const char *dm2_v2_lighting_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_LIGHTING_RUNTIME_H */