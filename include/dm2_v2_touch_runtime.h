#ifndef FIRESTAFF_DM2_V2_TOUCH_RUNTIME_H
#define FIRESTAFF_DM2_V2_TOUCH_RUNTIME_H
#include "dm2_v2_touch_controller_affordance.h"
#include "dm2_v2_phase_gate.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * dm2_v2_touch_runtime.h — DM2 V2 Phase 6 Touch/Controller Runtime
 *
 * V1→V2 input bridge for Dungeon Master II: Skullkeep.
 *
 * Translates V2 touch/controller affordances into V1 command-queue
 * entries (Dm1V1QueuedCommandPc34Compat) so the existing V1 input
 * pipeline handles the actual command dispatch. This is the same
 * architectural pattern as csb_v2_touch_runtime (sibling) and
 * nexus_v2_touch_runtime (sibling): V2-only input layer, V1 engine
 * routes the command.
 *
 * DM2 inherits the DM1 movement engine verbatim (C001-C006 →
 * DM1_V1_COMMAND_TURN_LEFT/RIGHT/MOVE_*). The mapping is via the
 * shared DM1_V2_MovementCommand adapter (dm1_v2_movement_command_route_for_presentation).
 *
 * Phase 6 rule: touch/controller affordances are V2-only. When
 * v2PresentationEnabled is false (V1 active), all affordances are
 * rejected so the V1 mouse/touch/click route matrix is the sole
 * input path. V1 parity guard.
 *
 * Architecture:
 *   SDL event → dm2_v2_touch_runtime_translate_affordance()
 *     → dm2_v2_touch_controller_affordance_route()
 *       → DM2_V2_TouchControllerAffordanceRoute {accepted, command, routeKind}
 *     → Dm1V1QueuedCommandPc34Compat {command, x, y}
 *     → dm1_v1_input_command_queue_push (existing V1 input pipeline)
 *
 * Source-lock anchors:
 *   SKULL.ASM T520  — party/movement tick (consumer of input queue)
 *   SKULL.ASM T048  — input dispatch (where the queue feeds)
 *   SKULL.ASM T560  — dungeon viewport rendering (presentation)
 *   ReDMCSB COMMAND.C:108-113  mouse movement zones C001-C006
 *   ReDMCSB COMMAND.C:254-291  keyboard tables for C001..C006
 *   ReDMCSB CLIKMENU.C:142    F0365_COMMAND_ProcessTypes1To2_TurnParty
 *   ReDMCSB CLIKMENU.C:180    F0366_COMMAND_ProcessTypes3To6_MoveParty
 *   ReDMCSB GAMELOOP.C:164-219 V1 input wait/command queue loop
 *   dm2_v2_touch_controller_affordance.c (affordance classification)
 *   dm1_v1_input_command_queue_pc34_compat.c (V1 queue sink)
 * ================================================================ */

#define DM2_V2_TOUCH_FRAMEBUFFER_W 320
#define DM2_V2_TOUCH_FRAMEBUFFER_H 200
#define DM2_V2_TOUCH_TOP_HUD_SAFE_H 32

/* ── Lifecycle ─────────────────────────────────────────────────── */
void dm2_v2_touch_runtime_init(void);
void dm2_v2_touch_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
void dm2_v2_touch_runtime_set_gate_config(const DM2_V2_PhaseGateConfig *config);

/* ── Translation: affordance → V1 command-queue entry ─────────── */
/* Translate one touch/controller affordance into a V1 command-queue
 * entry.  Returns 1 if accepted (the out entry is populated), 0 if
 * rejected (V1 parity guard or invalid affordance).
 *
 * When v2PresentationEnabled is false, returns 0 and out is zeroed.
 * When the affordance is NONE, returns 0 and out is zeroed.
 * When the affordance is V1-source-routed, returns 0 (V1 path takes it).
 * When V2-accepted, the resulting out.command is one of:
 *   DM1_V1_COMMAND_TURN_LEFT (1), DM1_V1_COMMAND_TURN_RIGHT (2),
 *   DM1_V1_COMMAND_MOVE_FORWARD (3), DM1_V1_COMMAND_MOVE_RIGHT (4),
 *   DM1_V1_COMMAND_MOVE_BACKWARD (5), DM1_V1_COMMAND_MOVE_LEFT (6)
 *
 * Accepted coordinates (x, y) are passed through verbatim from the caller.
 * Touch-origin affordances that begin on V2 HUD chrome are rejected before
 * movement translation so HUD/status/action-strip hits stay overlay-local.
 * Controller affordances do not use the framebuffer coordinate gate. */
int dm2_v2_touch_runtime_translate_affordance(
    DM2_V2_TouchControllerAffordance aff,
    int x, int y,
    struct Dm1V1QueuedCommandPc34Compat *out);

/* Presentation-only safety gate for touch gestures.  Returns 1 when a
 * 320x200 framebuffer point belongs to V2 HUD chrome instead of the
 * dungeon gesture surface.  Controller affordances intentionally do not
 * use this coordinate gate. */
int dm2_v2_touch_runtime_point_in_hud_chrome(int x, int y);

/* ── Status ────────────────────────────────────────────────────── */
/* Returns 1 if the touch runtime is active (V2 enabled).  Returns
 * 0 if V1 is the active presentation. */
int dm2_v2_touch_runtime_is_active(void);

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
/* Force-activates the touch runtime regardless of phase gate
 * (used by wire-up probes to verify the input flow).  Not called
 * by production code. */
void dm2_v2_touch_runtime_force_active_for_test(int active);

/* Observability helper: returns the count of accepted translations
 * since the last init.  Used by the wire-up probe to verify that
 * V2-on translations actually reach the V1 input queue. */
int dm2_v2_touch_runtime_translation_count(void);

/* Source evidence citation */
const char *dm2_v2_touch_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_TOUCH_RUNTIME_H */
