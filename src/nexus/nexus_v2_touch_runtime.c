/*
 * nexus_v2_touch_runtime.c — Nexus V2 Phase 6 Touch/Controller Runtime
 *
 * V1→V2 input bridge for Dungeon Master Nexus (Saturn).
 *
 * Translates V2 touch/controller affordances into V1 command-queue
 * entries (Dm1V1QueuedCommandPc34Compat) so the existing V1 input
 * pipeline handles the actual command dispatch.
 *
 * Source: Saturn NEXUS.BIN touch/joypad input layer
 *         Saturn SDK JOYPAD API (SMP-SONY Japan)
 *         ReDMCSB COMMAND.C:108-113 (mouse movement zones C001-C006)
 *         ReDMCSB COMMAND.C:254-291 (keyboard tables for C001..C006)
 *         ReDMCSB CLIKMENU.C:142 (F0365_COMMAND_ProcessTypes1To2_TurnParty)
 *         ReDMCSB CLIKMENU.C:180 (F0366_COMMAND_ProcessTypes3To6_MoveParty)
 *         ReDMCSB GAMELOOP.C:164-219 (V1 input wait/command queue loop)
 */

#include "nexus_v2_touch_runtime.h"
#include "nexus_v2_touch_controller_affordance.h"
#include "nexus_v2_phase_gate_pc34.h"
#include "nexus_v1_movement.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v2_movement_command_adapter_pc34.h"
#include <string.h>

/* ── Module state ──────────────────────────────────────────────── */
static int s_initialized = 0;
static const NEXUS_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;
static int s_translation_count = 0;

/* ── Lifecycle ──────────────────────────────────────────────────── */
void nexus_v2_touch_runtime_init(void) {
    if (s_initialized) return;
    s_initialized = 1;
    s_gate_config = NULL;
    s_force_active = 0;
    s_translation_count = 0;
}

void nexus_v2_touch_runtime_shutdown(void) {
    if (!s_initialized) return;
    s_initialized = 0;
    s_gate_config = NULL;
    s_force_active = 0;
    s_translation_count = 0;
}

/* ── Configuration ──────────────────────────────────────────────── */
void nexus_v2_touch_runtime_set_gate_config(const NEXUS_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── Translation: affordance → V1 command-queue entry ─────────── */
int nexus_v2_touch_runtime_translate_affordance(
    Nexus_V2_TouchControllerAffordance aff,
    int x, int y,
    struct Dm1V1QueuedCommandPc34Compat *out)
{
    if (out) {
        out->command = DM1_V1_COMMAND_NONE;
        out->x = x;
        out->y = y;
    }
    if (!s_initialized) return 0;
    if (aff == NEXUS_V2_AFFORDANCE_NONE) return 0;
    if (!s_force_active) {
        if (!s_gate_config) return 0;
        if (!s_gate_config->v2PresentationEnabled) return 0;
        if (!s_gate_config->v2ConfigPersistenceEnabled) return 0;
    }
    if (!out) return 0;

    /* Hand off to the affordance router. */
    Nexus_V2_TouchControllerAffordanceRoute r =
        nexus_v2_touch_controller_affordance_route(1 /* v2 enabled */, aff);

    if (!r.accepted) return 0;

    /* Convert NEXUS_CMD_* (1-6) → DM1_V2_MOVEMENT_COMMAND_* (0-6)
     * The Nexus affordance router returns NEXUS_CMD_* values which
     * mirror the V1 Nexus movement engine; the V1 command queue the
     * runtime feeds expects DM1_V2_MOVEMENT_COMMAND_* values.  This
     * translation preserves the contract:
     *   NEXUS_CMD_FORWARD (1)      → MOVE_FORWARD (3)
     *   NEXUS_CMD_BACKWARD (2)     → MOVE_BACKWARD (5)
     *   NEXUS_CMD_TURN_LEFT (3)    → TURN_LEFT (1)
     *   NEXUS_CMD_TURN_RIGHT (4)   → TURN_RIGHT (2)
     *   NEXUS_CMD_STRAFE_LEFT (5)  → MOVE_LEFT (6)
     *   NEXUS_CMD_STRAFE_RIGHT (6) → MOVE_RIGHT (4) */
    int mv_cmd = (int)r.movementCommand;
    int v2_cmd = DM1_V2_MOVEMENT_COMMAND_NONE;
    if      (mv_cmd == (int)NEXUS_CMD_FORWARD)     v2_cmd = (int)DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD;
    else if (mv_cmd == (int)NEXUS_CMD_BACKWARD)    v2_cmd = (int)DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD;
    else if (mv_cmd == (int)NEXUS_CMD_TURN_LEFT)   v2_cmd = (int)DM1_V2_MOVEMENT_COMMAND_TURN_LEFT;
    else if (mv_cmd == (int)NEXUS_CMD_TURN_RIGHT)  v2_cmd = (int)DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT;
    else if (mv_cmd == (int)NEXUS_CMD_STRAFE_LEFT) v2_cmd = (int)DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT;
    else if (mv_cmd == (int)NEXUS_CMD_STRAFE_RIGHT)v2_cmd = (int)DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT;
    else return 0;

    /* Convert DM1_V2_MovementCommand → DM1_V1_COMMAND_* */
    int v1_cmd = DM1_V1_COMMAND_NONE;
    switch (v2_cmd) {
        case DM1_V2_MOVEMENT_COMMAND_TURN_LEFT:    v1_cmd = DM1_V1_COMMAND_TURN_LEFT;    break;
        case DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT:   v1_cmd = DM1_V1_COMMAND_TURN_RIGHT;   break;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD: v1_cmd = DM1_V1_COMMAND_MOVE_FORWARD; break;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT:   v1_cmd = DM1_V1_COMMAND_MOVE_RIGHT;   break;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD:v1_cmd = DM1_V1_COMMAND_MOVE_BACKWARD;break;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT:    v1_cmd = DM1_V1_COMMAND_MOVE_LEFT;    break;
        default: return 0;
    }

    out->command = v1_cmd;
    out->x = x;
    out->y = y;
    s_translation_count++;
    return 1;
}

/* ── Status ────────────────────────────────────────────────────── */
int nexus_v2_touch_runtime_is_active(void) {
    if (!s_initialized) return 0;
    if (s_force_active) return 1;
    if (!s_gate_config) return 0;
    if (!s_gate_config->v2PresentationEnabled) return 0;
    if (!s_gate_config->v2ConfigPersistenceEnabled) return 0;
    return 1;
}

/* ── V1 compatibility helper ──────────────────────────────────── */
void nexus_v2_touch_runtime_force_active_for_test(int active) {
    s_force_active = active ? 1 : 0;
}

int nexus_v2_touch_runtime_translation_count(void) {
    return s_translation_count;
}

const char *nexus_v2_touch_runtime_source_evidence(void) {
    return
        "Nexus V2 Touch Runtime — Phase 6 source-lock\n"
        "Source: Saturn NEXUS.BIN touch/joypad input layer\n"
        "Source: Saturn SDK JOYPAD API (SMP-SONY Japan)\n"
        "Source: ReDMCSB COMMAND.C:108-113 (mouse movement zones C001-C006)\n"
        "Source: ReDMCSB COMMAND.C:254-291 (keyboard tables for C001..C006)\n"
        "Source: ReDMCSB CLIKMENU.C:142 (F0365_COMMAND_ProcessTypes1To2_TurnParty)\n"
        "Source: ReDMCSB CLIKMENU.C:180 (F0366_COMMAND_ProcessTypes3To6_MoveParty)\n"
        "Source: ReDMCSB GAMELOOP.C:164-219 (V1 input wait/command queue loop)\n"
        "Source: dm2_v2_touch_runtime.c (sibling DM2 V2 wire-up pattern)\n"
        "V1 invariant: V1 mouse/touch/click route matrix is the sole input path\n"
        "V2 invariant: touch/controller affordances are V2-only; V1 path takes V1-source routes\n"
        "V2 invariant: V2 affordances are NEVER injected when V1 is the active presentation\n";
}