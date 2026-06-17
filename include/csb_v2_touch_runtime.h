#ifndef FIRESTAFF_CSB_V2_TOUCH_RUNTIME_H
#define FIRESTAFF_CSB_V2_TOUCH_RUNTIME_H
#include "csb_v2_touch_controller_affordance.h"
#include "csb_v2_phase_gate_pc34.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * csb_v2_touch_runtime.h — CSB V2 Phase 6 Touch/Controller Runtime
 *
 * V1→V2 input bridge for Chaos Strikes Back.
 * Mirrors dm2_v2_touch_runtime / nexus_v2_touch_runtime pattern.
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
 *   ReDMCSB COMMAND.C:108-113 (mouse movement zones C001-C006)
 *   ReDMCSB COMMAND.C:254-291 (keyboard tables for C001..C006)
 *   ReDMCSB CLIKMENU.C:142 (F0365 turn)
 *   ReDMCSB CLIKMENU.C:180 (F0366 move)
 *   ReDMCSB GAMELOOP.C:164-219 (V1 input wait loop)
 *   CSBWin/resurrect/CsbV2InputBridge.cpp (CSBWin reimpl)
 *   dm2_v2_touch_runtime.c (sibling DM2 V2 wire-up pattern)
 *   nexus_v2_touch_runtime.c (sibling Nexus V2 wire-up pattern)
 * ================================================================ */

/* ── Lifecycle ─────────────────────────────────────────────────── */
void csb_v2_touch_runtime_init(void);
void csb_v2_touch_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
void csb_v2_touch_runtime_set_gate_config(const CSB_V2_PhaseGateConfig *config);

/* ── Translation: affordance → V1 command-queue entry ─────────── */
int csb_v2_touch_runtime_translate_affordance(
    CSB_V2_TouchControllerAffordance aff,
    int x, int y,
    struct Dm1V1QueuedCommandPc34Compat *out);

/* ── Status ────────────────────────────────────────────────────── */
int csb_v2_touch_runtime_is_active(void);

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
void csb_v2_touch_runtime_force_active_for_test(int active);

/* Observability: total translations accepted (monotonic). */
int csb_v2_touch_runtime_translation_count(void);

/* Source evidence citation */
const char *csb_v2_touch_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_TOUCH_RUNTIME_H */