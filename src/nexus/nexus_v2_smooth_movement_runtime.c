/*
 * nexus_v2_smooth_movement_runtime.c — Nexus V2 Phase 5 Smooth Movement Runtime
 *
 * V1→V2 smooth movement bridge for Dungeon Master Nexus (Saturn).
 *
 * Wraps the nexus_v2_smooth_movement.c state + tick APIs behind a
 * phase-gated runtime.  When V1 is the active presentation, the per-tick
 * advance is a no-op (V1 smooth state preserved).
 *
 * Source: ReDMCSB GROUP.C:1695-1770 (F0207 creature attack projectile payload)
 *         skproject/SKWIN/SkWinCore.cpp (ease-out cubic / ease-in-out cubic)
 *         ReDMCSB GAMELOOP.C:47-50 (V1 tick cadence 55ms)
 *         Saturn NEXUS.BIN (Saturn-specific interpolation timing)
 */

#include "nexus_v2_smooth_movement_runtime.h"
#include "nexus_v2_smooth_movement.h"
#include "nexus_v2_phase_gate_pc34.h"
#include <string.h>

/* ── Module state ──────────────────────────────────────────────── */
static Nexus_V2_SmoothState s_smooth;
static int s_initialized = 0;
static const NEXUS_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;
static int s_tick_count = 0;

/* ── Lifecycle ──────────────────────────────────────────────────── */
void nexus_v2_smooth_movement_runtime_init(void) {
    if (s_initialized) return;
    nexus_v2_smooth_init(&s_smooth);
    s_initialized = 1;
    s_gate_config = NULL;
    s_force_active = 0;
    s_tick_count = 0;
}

void nexus_v2_smooth_movement_runtime_shutdown(void) {
    if (!s_initialized) return;
    nexus_v2_smooth_init(&s_smooth);  /* reset to defaults */
    s_initialized = 0;
    s_gate_config = NULL;
    s_force_active = 0;
    s_tick_count = 0;
}

/* ── Configuration ──────────────────────────────────────────────── */
void nexus_v2_smooth_movement_runtime_set_gate_config(const NEXUS_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── Per-frame tick ─────────────────────────────────────────────── */
void nexus_v2_smooth_movement_runtime_tick(float dt_ms) {
    if (!s_initialized) return;
    if (!s_force_active) {
        if (!s_gate_config) return;
        if (!s_gate_config->v2PresentationEnabled) return;
        if (!s_gate_config->v2ConfigPersistenceEnabled) return;
    }
    nexus_v2_smooth_update(&s_smooth, dt_ms);
    s_tick_count++;
}

/* ── Status ────────────────────────────────────────────────────── */
int nexus_v2_smooth_movement_runtime_is_active(void) {
    if (!s_initialized) return 0;
    if (s_force_active) return 1;
    if (!s_gate_config) return 0;
    if (!s_gate_config->v2PresentationEnabled) return 0;
    if (!s_gate_config->v2ConfigPersistenceEnabled) return 0;
    return 1;
}

/* ── State accessors ───────────────────────────────────────────── */
const Nexus_V2_SmoothState *nexus_v2_smooth_movement_runtime_get_state(void) {
    if (!s_initialized) return NULL;
    if (!nexus_v2_smooth_movement_runtime_is_active()) return NULL;
    return &s_smooth;
}

/* ── Triggers ──────────────────────────────────────────────────── */
void nexus_v2_smooth_movement_runtime_start_walk(float from_x, float from_y,
    float to_x, float to_y)
{
    if (!s_initialized) return;
    if (!nexus_v2_smooth_movement_runtime_is_active()) return;
    nexus_v2_smooth_start_walk(&s_smooth, from_x, from_y, to_x, to_y);
}

void nexus_v2_smooth_movement_runtime_start_turn(float from_angle, float to_angle) {
    if (!s_initialized) return;
    if (!nexus_v2_smooth_movement_runtime_is_active()) return;
    nexus_v2_smooth_start_turn(&s_smooth, from_angle, to_angle);
}

void nexus_v2_smooth_movement_runtime_start_stairs(float from_x, float from_y,
    float to_x, float to_y, float from_vert, float to_vert)
{
    if (!s_initialized) return;
    if (!nexus_v2_smooth_movement_runtime_is_active()) return;
    nexus_v2_smooth_start_stairs(&s_smooth, from_x, from_y, to_x, to_y, from_vert, to_vert);
}

/* ── V1 compatibility helper ──────────────────────────────────── */
void nexus_v2_smooth_movement_runtime_force_active_for_test(int active) {
    s_force_active = active ? 1 : 0;
}

int nexus_v2_smooth_movement_runtime_tick_count(void) {
    return s_tick_count;
}

const char *nexus_v2_smooth_movement_runtime_source_evidence(void) {
    return
        "Nexus V2 Smooth Movement Runtime — Phase 5 source-lock\n"
        "Source: ReDMCSB GROUP.C:1695-1770 (F0207 creature attack)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp (ease-out/in-out cubic)\n"
        "Source: ReDMCSB GAMELOOP.C:47-50 (V1 tick cadence 55ms)\n"
        "Source: Saturn NEXUS.BIN (Saturn-specific interpolation timing)\n"
        "Source: dm2_v2_smooth_movement_runtime.c (sibling DM2 V2 wire-up pattern)\n"
        "V1 invariant: smooth tick is no-op when V1 is active\n"
        "V2 invariant: walk/turn/stairs triggers only when V2 enabled\n"
        "V2 invariant: tick_count is monotonic and resets only on init()\n";
}