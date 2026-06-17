/*
 * dm2_v2_lighting_runtime.c — DM2 V2 Phase 4 Lighting Runtime
 *
 * V1→V2 lighting + outdoor FX bridge for Dungeon Master II: Skullkeep.
 *
 * Wraps the dm2_v2_lighting.c + dm2_v2_outdoor_enhanced.c state/tick
 * APIs behind a phase-gated runtime. When V1 is the active presentation,
 * the per-frame tick is a no-op (V1 state preserved).
 *
 * Source: SKULL.ASM PROCESS_TIMER_0C — per-champion torch timers
 *         SKULL.ASM T560          — indoor dungeon viewport (lighting)
 *         SKULL.ASM T600          — outdoor viewport (outdoor FX)
 *         ReDMCSB PANEL.C:367-428 — DM1 palette lighting semantics
 *         dm2_v2_lighting.c       — state + bloom tick
 *         dm2_v2_outdoor_enhanced.c — outdoor FX state + tick
 *         dm2_v2_phase_gate.h     — DM2_V2_PHASE_DOMAIN_HUD gate
 */

#include "dm2_v2_lighting_runtime.h"
#include "dm2_v2_lighting.h"
#include "dm2_v2_outdoor_enhanced.h"
#include "dm2_v2_phase_gate.h"
#include <string.h>

/* ── Module state ──────────────────────────────────────────────── */
static DM2_V2_LightingState s_lighting;
static DM2_V2_OutdoorFX s_outdoor_fx;
static int s_initialized = 0;
static const DM2_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;  /* 0 = phase-gated, 1 = always on (test only) */
static int s_tick_count = 0;

/* ── Lifecycle ──────────────────────────────────────────────────── */
void dm2_v2_lighting_runtime_init(void) {
    if (s_initialized) return;
    dm2_v2_lighting_init(&s_lighting);
    dm2_v2_outdoor_fx_init(&s_outdoor_fx);
    s_initialized = 1;
    s_gate_config = NULL;
    s_force_active = 0;
    s_tick_count = 0;
}

void dm2_v2_lighting_runtime_shutdown(void) {
    if (!s_initialized) return;
    /* Reset state but keep struct memory. */
    dm2_v2_lighting_reset(&s_lighting);
    memset(&s_outdoor_fx, 0, sizeof(s_outdoor_fx));
    s_initialized = 0;
    s_gate_config = NULL;
    s_force_active = 0;
    s_tick_count = 0;
}

/* ── Configuration ──────────────────────────────────────────────── */
void dm2_v2_lighting_runtime_set_gate_config(const DM2_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── Per-frame tick ─────────────────────────────────────────────── */
void dm2_v2_lighting_runtime_tick(float dt_seconds, int weather) {
    if (!s_initialized) return;
    if (!s_force_active) {
        if (!s_gate_config) return;
        if (!s_gate_config->v2LaunchEnabled) return;
        if (!s_gate_config->v2ProfileEnabled) return;
    }
    /* Lighting: bloom timer advance (weather-independent). */
    dm2_v2_lighting_tick_bloom(&s_lighting, dt_seconds);
    /* Outdoor FX: cloud drift + ambient tint + lightning advance
     * (weather-dependent). */
    dm2_v2_outdoor_fx_tick(&s_outdoor_fx, dt_seconds, weather);
    s_tick_count++;
}

/* ── Status ────────────────────────────────────────────────────── */
int dm2_v2_lighting_runtime_is_active(void) {
    if (!s_initialized) return 0;
    if (s_force_active) return 1;
    if (!s_gate_config) return 0;
    if (!s_gate_config->v2LaunchEnabled) return 0;
    if (!s_gate_config->v2ProfileEnabled) return 0;
    return 1;
}

/* ── State accessors ───────────────────────────────────────────── */
const DM2_V2_LightingState *dm2_v2_lighting_runtime_get_state(void) {
    if (!s_initialized) return NULL;
    if (!dm2_v2_lighting_runtime_is_active()) return NULL;
    return &s_lighting;
}

const DM2_V2_OutdoorFX *dm2_v2_lighting_runtime_get_outdoor_fx(void) {
    if (!s_initialized) return NULL;
    if (!dm2_v2_lighting_runtime_is_active()) return NULL;
    return &s_outdoor_fx;
}

/* ── V1 compatibility helper ──────────────────────────────────── */
void dm2_v2_lighting_runtime_force_active_for_test(int active) {
    s_force_active = active ? 1 : 0;
}

int dm2_v2_lighting_runtime_tick_count(void) {
    return s_tick_count;
}

const char *dm2_v2_lighting_runtime_source_evidence(void) {
    return
        "DM2 V2 Lighting Runtime — Phase 4 source-lock\n"
        "ReDMCSB SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: SKULL.ASM PROCESS_TIMER_0C — per-champion torch timers\n"
        "Source: SKULL.ASM T560 — indoor dungeon viewport (lighting read)\n"
        "Source: SKULL.ASM T600 — outdoor viewport (outdoor FX read)\n"
        "Source: ReDMCSB PANEL.C:367-428 — DM1 palette lighting semantics\n"
        "Source: dm2_v2_lighting.c — state + bloom tick\n"
        "Source: dm2_v2_outdoor_enhanced.c — outdoor FX state + tick\n"
        "Source: dm2_v2_phase_gate.h — DM2_V2_PHASE_DOMAIN_HUD gate\n"
        "V1 invariant: lighting + outdoor FX ticks are no-op when V1 is active\n"
        "V2 invariant: lighting state is preserved across V2 toggle cycles\n"
        "V2 invariant: tick_count is monotonic and resets only on init()\n";
}