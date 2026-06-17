#include <stddef.h>

/*
 * csb_v2_touch_runtime.c — CSB V2 Phase 6 Touch/Controller Runtime
 *
 * V1→V2 input bridge for Chaos Strikes Back.
 *
 * Translates V2 touch/controller affordances into V1 command-queue
 * entries (Dm1V1QueuedCommandPc34Compat) so the existing V1 input
 * pipeline handles the actual command dispatch.
 *
 * Source: ReDMCSB COMMAND.C:108-113 (mouse movement zones C001-C006)
 *         ReDMCSB COMMAND.C:254-291 (keyboard tables for C001..C006)
 *         ReDMCSB CLIKMENU.C:142 (F0365 turn)
 *         ReDMCSB CLIKMENU.C:180 (F0366 move)
 *         ReDMCSB GAMELOOP.C:164-219 (V1 input wait loop)
 *         CSBWin/resurrect/CsbV2InputBridge.cpp (CSBWin reimpl)
 */

#include "csb_v2_touch_runtime.h"
#include "csb_v2_touch_controller_affordance.h"
#include "csb_v2_phase_gate_pc34.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v2_movement_command_adapter_pc34.h"

/* ── Module state ──────────────────────────────────────────────── */
static int s_initialized = 0;
static const CSB_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;
static int s_translation_count = 0;

/* ── Lifecycle ──────────────────────────────────────────────────── */
void csb_v2_touch_runtime_init(void) {
    if (s_initialized) return;
    s_initialized = 1;
    s_gate_config = NULL;
    s_force_active = 0;
    s_translation_count = 0;
}

void csb_v2_touch_runtime_shutdown(void) {
    if (!s_initialized) return;
    s_initialized = 0;
    s_gate_config = NULL;
    s_force_active = 0;
    s_translation_count = 0;
}

/* ── Configuration ──────────────────────────────────────────────── */
void csb_v2_touch_runtime_set_gate_config(const CSB_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── Translation: affordance → V1 command-queue entry ─────────── */
int csb_v2_touch_runtime_translate_affordance(
    CSB_V2_TouchControllerAffordance aff,
    int x, int y,
    struct Dm1V1QueuedCommandPc34Compat *out)
{
    if (out) {
        out->command = DM1_V1_COMMAND_NONE;
        out->x = x;
        out->y = y;
    }
    if (!s_initialized) return 0;
    if (aff == CSB_V2_AFFORDANCE_NONE) return 0;
    int v2_enabled = s_force_active ? 1 :
        (s_gate_config
         && s_gate_config->v2PresentationEnabled
         && s_gate_config->v2ConfigPersistenceEnabled);
    if (!v2_enabled) return 0;
    if (!out) return 0;

    /* Hand off to the affordance router.  CSB's router takes v2 flag
     * as the first parameter (unlike DM2/Nexus). */
    CSB_V2_TouchControllerAffordanceRoute r =
        csb_v2_touch_controller_affordance_route(v2_enabled, aff);

    if (!r.accepted) return 0;

    /* Convert DM1_V2_MovementCommand → DM1_V1_COMMAND_* */
    int v1_cmd = DM1_V1_COMMAND_NONE;
    switch (r.movementCommand) {
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
int csb_v2_touch_runtime_is_active(void) {
    if (!s_initialized) return 0;
    if (s_force_active) return 1;
    if (!s_gate_config) return 0;
    if (!s_gate_config->v2PresentationEnabled) return 0;
    if (!s_gate_config->v2ConfigPersistenceEnabled) return 0;
    return 1;
}

/* ── V1 compatibility helper ──────────────────────────────────── */
void csb_v2_touch_runtime_force_active_for_test(int active) {
    s_force_active = active ? 1 : 0;
}

int csb_v2_touch_runtime_translation_count(void) {
    return s_translation_count;
}

const char *csb_v2_touch_runtime_source_evidence(void) {
    return
        "CSB V2 Touch Runtime — Phase 6 source-lock\n"
        "Source: ReDMCSB COMMAND.C:108-113 (mouse movement zones C001-C006)\n"
        "Source: ReDMCSB COMMAND.C:254-291 (keyboard tables for C001..C006)\n"
        "Source: ReDMCSB CLIKMENU.C:142 (F0365 turn)\n"
        "Source: ReDMCSB CLIKMENU.C:180 (F0366 move)\n"
        "Source: ReDMCSB GAMELOOP.C:164-219 (V1 input wait loop)\n"
        "Source: CSBWin/resurrect/CsbV2InputBridge.cpp (CSBWin reimpl)\n"
        "Source: dm2_v2_touch_runtime.c (sibling DM2 V2 wire-up pattern)\n"
        "Source: nexus_v2_touch_runtime.c (sibling Nexus V2 wire-up pattern)\n"
        "V1 invariant: V1 mouse/touch/click route matrix is the sole input path\n"
        "V2 invariant: touch/controller affordances are V2-only; V1 path takes V1-source routes\n"
        "V2 invariant: V2 affordances are NEVER injected when V1 is the active presentation\n";
}