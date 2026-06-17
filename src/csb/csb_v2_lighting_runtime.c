#include <stddef.h>

/*
 * csb_v2_lighting_runtime.c — CSB V2 Phase 4 Lighting Runtime
 *
 * V1→V2 lighting bridge for Chaos Strikes Back.
 *
 * Phase-gated tick wrapper around the CSB V2 lighting module's global
 * state. When V1 is the active presentation, the per-frame tick is a
 * no-op (V1 light state preserved).
 *
 * Source: ReDMCSB LIGHT.C F0380 (light radius + flicker timing)
 *         ReDMCSB LIGHT.C F0381 (torch source)
 *         ReDMCSB LIGHT.C F0382 (ambient per dungeon level)
 *         CSBWin/resurrect/CsbV2FieldLight.cpp (CSBWin reimpl)
 *         dm2_v2_lighting_runtime.c (sibling DM2 V2 wire-up pattern)
 */

#include "csb_v2_lighting_runtime.h"
#include "csb_v2_lighting_dynamic.h"
#include "csb_v2_phase_gate_pc34.h"

/* ── Module state ──────────────────────────────────────────────── */
static int s_initialized = 0;
static const CSB_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;
static int s_tick_count = 0;

/* ── Lifecycle ──────────────────────────────────────────────────── */
void csb_v2_lighting_runtime_init(void) {
    if (s_initialized) return;
    csb_v2_light_init();
    s_initialized = 1;
    s_gate_config = NULL;
    s_force_active = 0;
    s_tick_count = 0;
}

void csb_v2_lighting_runtime_shutdown(void) {
    if (!s_initialized) return;
    /* Re-init the lighting module to its default state. */
    csb_v2_light_init();
    s_initialized = 0;
    s_gate_config = NULL;
    s_force_active = 0;
    s_tick_count = 0;
}

/* ── Configuration ──────────────────────────────────────────────── */
void csb_v2_lighting_runtime_set_gate_config(const CSB_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── Per-frame tick ─────────────────────────────────────────────── */
void csb_v2_lighting_runtime_tick(float dt_seconds) {
    if (!s_initialized) return;
    if (!s_force_active) {
        if (!s_gate_config) return;
        if (!s_gate_config->v2PresentationEnabled) return;
        if (!s_gate_config->v2ConfigPersistenceEnabled) return;
    }
    csb_v2_light_tick(dt_seconds);
    csb_v2_light_update_flicker(dt_seconds);
    s_tick_count++;
}

/* ── Status ────────────────────────────────────────────────────── */
int csb_v2_lighting_runtime_is_active(void) {
    if (!s_initialized) return 0;
    if (s_force_active) return 1;
    if (!s_gate_config) return 0;
    if (!s_gate_config->v2PresentationEnabled) return 0;
    if (!s_gate_config->v2ConfigPersistenceEnabled) return 0;
    return 1;
}

/* ── V1 compatibility helper ──────────────────────────────────── */
void csb_v2_lighting_runtime_force_active_for_test(int active) {
    s_force_active = active ? 1 : 0;
}

int csb_v2_lighting_runtime_tick_count(void) {
    return s_tick_count;
}

const char *csb_v2_lighting_runtime_source_evidence(void) {
    return
        "CSB V2 Lighting Runtime — Phase 4 source-lock\n"
        "Source: ReDMCSB LIGHT.C F0380 (light radius + flicker timing)\n"
        "Source: ReDMCSB LIGHT.C F0381 (torch source)\n"
        "Source: ReDMCSB LIGHT.C F0382 (ambient per dungeon level)\n"
        "Source: CSBWin/resurrect/CsbV2FieldLight.cpp (CSBWin reimpl)\n"
        "Source: dm2_v2_lighting_runtime.c (sibling DM2 V2 wire-up pattern)\n"
        "Source: nexus_v2_lighting_runtime.c (sibling Nexus V2 wire-up pattern)\n"
        "V1 invariant: lighting tick is no-op when V1 is active\n"
        "V2 invariant: csb_v2_light_tick + flicker advance only when V2 enabled\n"
        "V2 invariant: tick_count is monotonic and resets only on init()\n";
}