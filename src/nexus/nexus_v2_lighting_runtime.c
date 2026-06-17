/*
 * nexus_v2_lighting_runtime.c — Nexus V2 Phase 4 Lighting Runtime
 *
 * V1→V2 lighting bridge for Dungeon Master Nexus (Saturn).
 *
 * Wraps the nexus_v2_lighting.c state + tick APIs behind a phase-gated
 * runtime. When V1 is the active presentation, the per-frame tick is
 * a no-op (V1 light state preserved).
 *
 * Source: Saturn NEXUS.BIN VDP1 polygon lighting / VDP2 shadow layer
 *         DMDF level data (per-tile light emission values)
 *         ReDMCSB LIGHT.C F0380 (light radius + flicker timing)
 *         ReDMCSB COMMAND.C F0209 (spell-light colour binding)
 *         ReDMCSB DUNGEON.C (torch position tracking in party state)
 */

#include "nexus_v2_lighting_runtime.h"
#include "nexus_v2_lighting.h"
#include "nexus_v2_phase_gate_pc34.h"
#include <string.h>

/* ── Module state ──────────────────────────────────────────────── */
static Nexus_V2_LightingState s_lighting;
static int s_initialized = 0;
static const NEXUS_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;
static int s_tick_count = 0;

/* ── Lifecycle ──────────────────────────────────────────────────── */
void nexus_v2_lighting_runtime_init(void) {
    if (s_initialized) return;
    nexus_v2_lighting_init(&s_lighting);
    s_initialized = 1;
    s_gate_config = NULL;
    s_force_active = 0;
    s_tick_count = 0;
}

void nexus_v2_lighting_runtime_shutdown(void) {
    if (!s_initialized) return;
    nexus_v2_lighting_init(&s_lighting);  /* reset to defaults */
    s_initialized = 0;
    s_gate_config = NULL;
    s_force_active = 0;
    s_tick_count = 0;
}

/* ── Configuration ──────────────────────────────────────────────── */
void nexus_v2_lighting_runtime_set_gate_config(const NEXUS_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── Per-frame tick ─────────────────────────────────────────────── */
void nexus_v2_lighting_runtime_tick(float dt_seconds) {
    if (!s_initialized) return;
    if (!s_force_active) {
        if (!s_gate_config) return;
        if (!s_gate_config->v2PresentationEnabled) return;
        if (!s_gate_config->v2ConfigPersistenceEnabled) return;
    }
    nexus_v2_lighting_tick(&s_lighting, dt_seconds);
    s_tick_count++;
}

/* ── Status ────────────────────────────────────────────────────── */
int nexus_v2_lighting_runtime_is_active(void) {
    if (!s_initialized) return 0;
    if (s_force_active) return 1;
    if (!s_gate_config) return 0;
    if (!s_gate_config->v2PresentationEnabled) return 0;
    if (!s_gate_config->v2ConfigPersistenceEnabled) return 0;
    return 1;
}

/* ── State accessors ───────────────────────────────────────────── */
const Nexus_V2_LightingState *nexus_v2_lighting_runtime_get_state(void) {
    if (!s_initialized) return NULL;
    if (!nexus_v2_lighting_runtime_is_active()) return NULL;
    return &s_lighting;
}

/* ── V1 compatibility helper ──────────────────────────────────── */
void nexus_v2_lighting_runtime_force_active_for_test(int active) {
    s_force_active = active ? 1 : 0;
}

int nexus_v2_lighting_runtime_tick_count(void) {
    return s_tick_count;
}

const char *nexus_v2_lighting_runtime_source_evidence(void) {
    return
        "Nexus V2 Lighting Runtime — Phase 4 source-lock\n"
        "Source: Saturn NEXUS.BIN VDP1 polygon lighting\n"
        "Source: Saturn NEXUS.BIN VDP2 shadow layer\n"
        "Source: DMDF level data (per-tile light emission values, DGN format)\n"
        "Source: ReDMCSB LIGHT.C F0380 (light radius + flicker timing)\n"
        "Source: ReDMCSB COMMAND.C F0209 (spell-light colour binding)\n"
        "Source: ReDMCSB DUNGEON.C (torch position tracking in party state)\n"
        "Source: dm2_v2_lighting_runtime.c (sibling DM2 V2 wire-up pattern)\n"
        "V1 invariant: lighting tick is no-op when V1 is active\n"
        "V2 invariant: torch_flicker_phase + ambient state advance only when V2 enabled\n"
        "V2 invariant: tick_count is monotonic and resets only on init()\n";
}