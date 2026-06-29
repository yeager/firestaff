/*
 * theron_v2_touch_runtime.c
 *
 * Theron V2 Phase 6 touch/controller runtime bridge.
 *
 * Source-lock anchors:
 *   THQUEST.ASM T520/T560/T600  Theron party placement, viewport, UI zones
 *   ReDMCSB DEFS.H:238-243      C001-C006 movement command ids
 *   ReDMCSB COMMAND.C:2045-2155 F0380 command queue dispatch
 *   ReDMCSB CLIKMENU.C:142      F0365_COMMAND_ProcessTypes1To2_TurnParty
 *   ReDMCSB CLIKMENU.C:180      F0366_COMMAND_ProcessTypes3To6_MoveParty
 *   ReDMCSB GAMELOOP.C:164-219  V1 input wait/command queue loop
 */

#include "theron_v2_touch_runtime.h"
#include "theron_v2_hud_overlay_pc34.h"
#include <string.h>

static int s_initialized = 0;
static const THERON_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;
static int s_translation_count = 0;

static int movement_to_v1_command(DM1_V2_MovementCommand command)
{
    switch (command) {
        case DM1_V2_MOVEMENT_COMMAND_TURN_LEFT:
            return DM1_V1_COMMAND_TURN_LEFT;
        case DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT:
            return DM1_V1_COMMAND_TURN_RIGHT;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD:
            return DM1_V1_COMMAND_MOVE_FORWARD;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT:
            return DM1_V1_COMMAND_MOVE_RIGHT;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD:
            return DM1_V1_COMMAND_MOVE_BACKWARD;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT:
            return DM1_V1_COMMAND_MOVE_LEFT;
        case DM1_V2_MOVEMENT_COMMAND_NONE:
        default:
            return DM1_V1_COMMAND_NONE;
    }
}

int theron_v2_touch_runtime_point_in_hud_chrome(int x, int y)
{
    if (x < 0 || y < 0 ||
        x >= THERON_V2_TOUCH_FRAMEBUFFER_W ||
        y >= THERON_V2_TOUCH_FRAMEBUFFER_H) {
        return 1;
    }
    if (y < THERON_V2_HUD_TOPBAR_H) {
        return 1;
    }
    if (y >= THERON_V2_CHAMP_BAR_Y &&
        y < THERON_V2_CHAMP_BAR_Y + THERON_V2_CHAMP_BAR_H) {
        return 1;
    }
    if (y >= THERON_V2_ACTION_STRIP_Y &&
        y < THERON_V2_ACTION_STRIP_Y + THERON_V2_ACTION_STRIP_H) {
        return 1;
    }
    return 0;
}

void theron_v2_touch_runtime_init(void)
{
    if (s_initialized) {
        return;
    }
    s_initialized = 1;
    s_gate_config = NULL;
    s_force_active = 0;
    s_translation_count = 0;
}

void theron_v2_touch_runtime_shutdown(void)
{
    if (!s_initialized) {
        return;
    }
    s_initialized = 0;
    s_gate_config = NULL;
    s_force_active = 0;
    s_translation_count = 0;
}

void theron_v2_touch_runtime_set_gate_config(const THERON_V2_PhaseGateConfig *config)
{
    s_gate_config = config;
}

int theron_v2_touch_runtime_translate_affordance(
    Theron_V2_TouchControllerAffordance affordance,
    int x,
    int y,
    struct Dm1V1QueuedCommandPc34Compat *out)
{
    Theron_V2_TouchControllerAffordanceRoute route;
    int command;

    if (out) {
        out->command = DM1_V1_COMMAND_NONE;
        out->x = x;
        out->y = y;
    }

    if (!s_initialized || affordance == THERON_V2_AFFORDANCE_NONE) {
        return 0;
    }
    if (!s_force_active) {
        if (!s_gate_config || !theron_v2_phase_gate_v2_active(s_gate_config)) {
            return 0;
        }
    }
    if (!out) {
        return 0;
    }

    /* THQUEST.ASM T600 owns Theron's V1/V2 UI zones. Touch gestures that
     * start on V2 chrome stay overlay-local; controller input has no
     * framebuffer origin and bypasses this spatial guard. */
    if (theron_v2_touch_controller_affordance_input_kind(affordance) ==
            THERON_V2_AFFORDANCE_INPUT_TOUCH &&
        theron_v2_touch_runtime_point_in_hud_chrome(x, y)) {
        return 0;
    }

    route = theron_v2_touch_controller_affordance_route(1, affordance);
    if (!route.accepted) {
        return 0;
    }

    command = movement_to_v1_command(route.movementCommand);
    if (command == DM1_V1_COMMAND_NONE) {
        return 0;
    }

    out->command = command;
    out->x = x;
    out->y = y;
    s_translation_count++;
    return 1;
}

int theron_v2_touch_runtime_is_active(void)
{
    if (!s_initialized) {
        return 0;
    }
    if (s_force_active) {
        return 1;
    }
    return s_gate_config ? theron_v2_phase_gate_v2_active(s_gate_config) : 0;
}

void theron_v2_touch_runtime_force_active_for_test(int active)
{
    s_force_active = active ? 1 : 0;
}

int theron_v2_touch_runtime_translation_count(void)
{
    return s_translation_count;
}

const char *theron_v2_touch_runtime_source_evidence(void)
{
    return
        "Theron V2 Touch Runtime - Phase 6\n"
        "Source: THQUEST.ASM T520/T560/T600 party placement, viewport, UI zones\n"
        "Source: ReDMCSB DEFS.H:238-243 C001-C006 movement command ids\n"
        "Source: ReDMCSB COMMAND.C:2045-2155 F0380 command queue dispatch\n"
        "Source: ReDMCSB CLIKMENU.C:142 F0365_COMMAND_ProcessTypes1To2_TurnParty\n"
        "Source: ReDMCSB CLIKMENU.C:180 F0366_COMMAND_ProcessTypes3To6_MoveParty\n"
        "Source: ReDMCSB GAMELOOP.C:164-219 V1 input wait/command queue loop\n"
        "Source: theron_v2_hud_overlay_pc34.h top-bar/champion/action-strip geometry\n"
        "V1 invariant: V2 affordances are rejected when V2 presentation is off.\n"
        "V2 invariant: touch starts on HUD chrome are not injected as movement.\n"
        "V2 invariant: controller affordances bypass framebuffer coordinate gates.\n";
}
