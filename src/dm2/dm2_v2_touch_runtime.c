/*
 * dm2_v2_touch_runtime.c — DM2 V2 Phase 6 Touch/Controller Runtime
 *
 * V1→V2 input bridge for Dungeon Master II: Skullkeep.
 *
 * Translates V2 touch/controller affordances into V1 command-queue
 * entries (Dm1V1QueuedCommandPc34Compat) so the existing V1 input
 * pipeline handles the actual command dispatch.
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
 */

#include "dm2_v2_touch_runtime.h"
#include "dm2_v2_touch_controller_affordance.h"
#include "dm2_v2_phase_gate.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v2_movement_command_adapter_pc34.h"
#include <string.h>

/* ── Module state ──────────────────────────────────────────────── */
static int s_initialized = 0;
static const DM2_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;  /* 0 = phase-gated, 1 = always on (test only) */
static int s_translation_count = 0;  /* observability counter for the wire-up probe */

/* ── Lifecycle ──────────────────────────────────────────────────── */
void dm2_v2_touch_runtime_init(void) {
    if (s_initialized) return;
    s_initialized = 1;
    s_gate_config = NULL;
    s_force_active = 0;
    s_translation_count = 0;
}

void dm2_v2_touch_runtime_shutdown(void) {
    if (!s_initialized) return;
    s_initialized = 0;
    s_gate_config = NULL;
    s_force_active = 0;
    s_translation_count = 0;
}

/* ── Configuration ──────────────────────────────────────────────── */
void dm2_v2_touch_runtime_set_gate_config(const DM2_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── Translation: affordance → V1 command-queue entry ─────────── */
int dm2_v2_touch_runtime_translate_affordance(
    DM2_V2_TouchControllerAffordance aff,
    int x, int y,
    struct Dm1V1QueuedCommandPc34Compat *out)
{
    if (out) {
        out->command = DM1_V1_COMMAND_NONE;
        out->x = x;
        out->y = y;
    }
    if (!s_initialized) return 0;
    if (aff == DM2_V2_AFFORDANCE_NONE) return 0;
    if (!s_force_active) {
        if (!s_gate_config) return 0;
        if (!s_gate_config->v2LaunchEnabled) return 0;
        if (!s_gate_config->v2ProfileEnabled) return 0;
    }
    if (!out) return 0;

    /* Hand off to the affordance router.  The router maps the affordance
     * to a DM1_V2_MovementCommand and a route kind.  When the route is
     * V1_SOURCE, we still want to convert to a V1 command because the
     * V2 affordance WAS accepted — V2 touch is just a presentation-only
     * input path.  When the route is V2_PRESENTATION, the existing DM1
     * adapter handles presentation routing internally.  In both cases,
     * the resulting command becomes the V1 queue entry. */
    DM2_V2_TouchControllerAffordanceRoute r =
        dm2_v2_touch_controller_affordance_route(1 /* v2 enabled */, aff);

    if (!r.accepted) return 0;

    /* Convert DM1_V2_MovementCommand → DM1_V1_COMMAND_* (use if-else
     * for explicit integer comparison rather than switch on enum to
     * avoid any compiler-specific switch-on-enum folding) */
    int v1_cmd = DM1_V1_COMMAND_NONE;
    int mv_cmd = (int)r.movementCommand;
    if (mv_cmd == (int)DM1_V2_MOVEMENT_COMMAND_TURN_LEFT)         v1_cmd = (int)DM1_V1_COMMAND_TURN_LEFT;
    else if (mv_cmd == (int)DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT)    v1_cmd = (int)DM1_V1_COMMAND_TURN_RIGHT;
    else if (mv_cmd == (int)DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD)  v1_cmd = (int)DM1_V1_COMMAND_MOVE_FORWARD;
    else if (mv_cmd == (int)DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT)    v1_cmd = (int)DM1_V1_COMMAND_MOVE_RIGHT;
    else if (mv_cmd == (int)DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD) v1_cmd = (int)DM1_V1_COMMAND_MOVE_BACKWARD;
    else if (mv_cmd == (int)DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT)     v1_cmd = (int)DM1_V1_COMMAND_MOVE_LEFT;
    else return 0;

    out->command = v1_cmd;
    out->x = x;
    out->y = y;
    s_translation_count++;
    return 1;
}

/* ── Status ────────────────────────────────────────────────────── */
int dm2_v2_touch_runtime_is_active(void) {
    if (!s_initialized) return 0;
    if (s_force_active) return 1;
    if (!s_gate_config) return 0;
    if (!s_gate_config->v2LaunchEnabled) return 0;
    if (!s_gate_config->v2ProfileEnabled) return 0;
    return 1;
}

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
void dm2_v2_touch_runtime_force_active_for_test(int active) {
    s_force_active = active ? 1 : 0;
}

/* Observability helper for the wire-up probe */
int dm2_v2_touch_runtime_translation_count(void) {
    return s_translation_count;
}

const char *dm2_v2_touch_runtime_source_evidence(void) {
    return
        "DM2 V2 Touch Runtime — Phase 6 source-lock\n"
        "ReDMCSB SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: SKULL.ASM T520              (party/movement tick, consumer of input queue)\n"
        "Source: SKULL.ASM T048              (input dispatch, where the queue feeds)\n"
        "Source: SKULL.ASM T560              (dungeon viewport rendering, presentation)\n"
        "Source: ReDMCSB COMMAND.C:108-113   (mouse movement zones C001-C006)\n"
        "Source: ReDMCSB COMMAND.C:254-291   (keyboard tables for C001..C006)\n"
        "Source: ReDMCSB CLIKMENU.C:142      (F0365_COMMAND_ProcessTypes1To2_TurnParty)\n"
        "Source: ReDMCSB CLIKMENU.C:180      (F0366_COMMAND_ProcessTypes3To6_MoveParty)\n"
        "Source: ReDMCSB GAMELOOP.C:164-219  (V1 input wait/command queue loop)\n"
        "Source: dm2_v2_touch_controller_affordance.c (affordance classification)\n"
        "Source: dm1_v1_input_command_queue_pc34_compat.c (V1 queue sink)\n"
        "Source: dm2_v2_phase_gate.h         (DM2_V2_PHASE_DOMAIN_PROFILE gate)\n"
        "V1 invariant: V1 mouse/touch/click route matrix is the sole input path\n"
        "V2 invariant: touch/controller affordances are V2-only; V1 path takes V1-source routes\n"
        "V2 invariant: V2 affordances are NEVER injected when V1 is the active presentation\n";
}