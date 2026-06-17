#ifndef FIRESTAFF_NEXUS_V2_TOUCH_RUNTIME_H
#define FIRESTAFF_NEXUS_V2_TOUCH_RUNTIME_H
#include "nexus_v2_touch_controller_affordance.h"
#include "nexus_v2_phase_gate_pc34.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * nexus_v2_touch_runtime.h — Nexus V2 Phase 6 Touch/Controller Runtime
 *
 * V1→V2 input bridge for Dungeon Master Nexus (Saturn).
 * Mirrors dm2_v2_touch_runtime.c pattern (sibling DM2 wire-up).
 *
 * Translates V2 touch/controller affordances into V1 command-queue
 * entries (Dm1V1QueuedCommandPc34Compat) so the existing V1 input
 * pipeline handles the actual command dispatch.
 *
 * Phase 6 rule: touch/controller affordances are V2-only. When V1 is
 * the active presentation, all affordances are rejected so the V1
 * mouse/touch/click route matrix is the sole input path.
 *
 * Source-lock anchors:
 *   Saturn NEXUS.BIN touch/joypad input layer
 *   Saturn SDK JOYPAD API (SMP-SONY Japan)
 *   ReDMCSB COMMAND.C:108-113 (mouse movement zones C001-C006)
 *   ReDMCSB COMMAND.C:254-291 (keyboard tables for C001..C006)
 *   ReDMCSB CLIKMENU.C:142 (F0365_COMMAND_ProcessTypes1To2_TurnParty)
 *   ReDMCSB CLIKMENU.C:180 (F0366_COMMAND_ProcessTypes3To6_MoveParty)
 *   ReDMCSB GAMELOOP.C:164-219 (V1 input wait/command queue loop)
 *   dm2_v2_touch_runtime.c (sibling DM2 V2 wire-up pattern)
 * ================================================================ */

/* ── Lifecycle ─────────────────────────────────────────────────── */
void nexus_v2_touch_runtime_init(void);
void nexus_v2_touch_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
void nexus_v2_touch_runtime_set_gate_config(const NEXUS_V2_PhaseGateConfig *config);

/* ── Translation: affordance → V1 command-queue entry ─────────── */
int nexus_v2_touch_runtime_translate_affordance(
    Nexus_V2_TouchControllerAffordance aff,
    int x, int y,
    struct Dm1V1QueuedCommandPc34Compat *out);

/* ── Status ────────────────────────────────────────────────────── */
int nexus_v2_touch_runtime_is_active(void);

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
void nexus_v2_touch_runtime_force_active_for_test(int active);

/* Observability: total translations accepted (monotonic). */
int nexus_v2_touch_runtime_translation_count(void);

/* Source evidence citation */
const char *nexus_v2_touch_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V2_TOUCH_RUNTIME_H */