/* csb_v2_touch_controller_affordance.c - CSB V2 touch/controller affordances.
 *
 * pass603: CSB V2 Phase 6 - Touch/controller ergonomics
 *
 * Source-lock anchors (shared DM1/CSB engine via ReDMCSB Common Toolchains):
 * - ReDMCSB COMMAND.C:108-113  mouse movement zones C001-C006
 * - ReDMCSB COMMAND.C:254-291  keyboard tables for C001..C006
 * - ReDMCSB COMMAND.C:1379     F0358_COMMAND_GetCommandFromMouseInput_CPSC
 * - ReDMCSB COMMAND.C:1452     F0359_COMMAND_ProcessClick_CPSC
 * - ReDMCSB COMMAND.C:1641,1643 primary-to-secondary mouse queue dispatch
 * - ReDMCSB COMMAND.C:1693     F0360_COMMAND_ProcessPendingClick
 * - ReDMCSB CLIKMENU.C:142    F0365_COMMAND_ProcessTypes1To2_TurnParty
 * - ReDMCSB CLIKMENU.C:180    F0366_COMMAND_ProcessTypes3To6_MoveParty
 * - ReDMCSB GAMELOOP.C:164-219 V1 input wait/command queue loop
 * - CSBWin/Chaos.cpp:60-69   DSA _CALL0-_CALL9 dispatch
 * - CSBWin/DSA.cpp           DSA interpreter (chaos magic)
 * - csb_v1_chaos_magic_pc34_compat.c:50 csb_v1_chaos_trigger
 *
 * Architecture notes:
 * - V2 affordances (swipes, dpad, sticks) map to C001-C006 movement commands
 *   via the shared DM1 V2 movement command adapter.  CSB inherits the DM1
 *   movement engine verbatim; the mapping is identical.
 * - Shoulder buttons (left/right bumper) are CSB-only: left cycles champions,
 *   right triggers/cycles chaos magic.  These have no V1 equivalent.
 * - V2-only affordances are accepted only when v2PresentationEnabled is true.
 *   When false, all affordances return accepted=0 preserving V1 input paths.
 * - Pinch-to-zoom: NOT implemented for CSB (CSB minimap differs from DM1).
 * - V1 touch/click parity: preserved via touch_pointer_input_pc34_compat.c.
 *   V2 affordances are an additional input layer above V1, not a replacement.
 *
 * Differences from DM1 V2 Phase 6 (dm1_v2_touch_controller_affordance_pc34):
 * - No pinch-to-zoom (CSB has no zoomable minimap)
 * - Left bumper maps to champion cycle (DM1: unused)
 * - Right bumper maps to chaos magic activate (DM1: unused)
 * - Edge-strafe gestures present but behave same as DM1 */

#include "csb_v2_touch_controller_affordance.h"
#include <string.h>

/* Internal helpers */

static CSB_V2_TouchControllerAffordanceRoute
make_route(int accepted,
           CSB_V2_TouchControllerAffordance aff,
           DM1_V2_MovementCommand cmd,
           DM1_V2_MovementCommandRoute route) {
    CSB_V2_TouchControllerAffordanceRoute r;
    r.accepted = accepted;
    r.v2Only = 1;
    r.inputKind = csb_v2_touch_controller_affordance_input_kind(aff);
    r.affordance = aff;
    r.movementCommand = cmd;
    r.route = route;
    return r;
}

/* Affordance to movement command mapping
 *
 * ReDMCSB: COMMAND.C:108-113 defines C001-C006 as the movement/turn
 * commands that CLIKMENU.C:142 (F0365) and CLIKMENU.C:180 (F0366)
 * consume.  Touch swipes and controller inputs are synthesized into
 * these same command IDs via the V2 movement command adapter. */

DM1_V2_MovementCommand
csb_v2_touch_controller_affordance_movement_command(
    CSB_V2_TouchControllerAffordance aff) {
    switch (aff) {
        /* Swipe forward / D-pad up / left stick up: C003 MOVE FORWARD */
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP:
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD;

        /* Swipe backward / D-pad down / left stick down: C005 MOVE BACK */
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD;

        /* Swipe left / D-pad left / right stick left: C001 TURN LEFT */
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
        case CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
            return DM1_V2_MOVEMENT_COMMAND_TURN_LEFT;

        /* Swipe right / D-pad right / right stick right: C002 TURN RIGHT */
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
        case CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
            return DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT;

        /* Edge-strafe left / left stick left: C006 MOVE LEFT
         * ReDMCSB CLIKMENU.C:180 F0366 processes C006 MOVE LEFT */
        case CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT;

        /* Edge-strafe right / left stick right: C004 MOVE RIGHT
         * ReDMCSB CLIKMENU.C:180 F0366 processes C004 MOVE RIGHT */
        case CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT;

        /* Shoulder buttons are CSB actions and do not synthesize movement. */
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER:
        case CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER:
            return DM1_V2_MOVEMENT_COMMAND_NONE;

        case CSB_V2_AFFORDANCE_NONE:
        default:
            return DM1_V2_MOVEMENT_COMMAND_NONE;
    }
}

/* Input-kind classification */

CSB_V2_TouchControllerInputKind
csb_v2_touch_controller_affordance_input_kind(
    CSB_V2_TouchControllerAffordance aff) {
    switch (aff) {
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP:
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
        case CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
        case CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
            return CSB_V2_AFFORDANCE_INPUT_TOUCH;

        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
        case CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
        case CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER:
        case CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER:
            return CSB_V2_AFFORDANCE_INPUT_CONTROLLER;

        case CSB_V2_AFFORDANCE_NONE:
        default:
            return CSB_V2_AFFORDANCE_INPUT_NONE;
    }
}

/* Route dispatch
 *
 * ReDMCSB GAMELOOP.C:164-219 is the V1 input wait/command queue loop.
 * V2 affordances are accepted only when V2 presentation is explicitly
 * enabled; otherwise all affordances return accepted=0 so the V1 input
 * path (mouse click matrix via F0358/F0359) remains authoritative. */

CSB_V2_TouchControllerAffordanceRoute
csb_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    CSB_V2_TouchControllerAffordance aff) {
    DM1_V2_MovementCommand cmd =
        csb_v2_touch_controller_affordance_movement_command(aff);
    DM1_V2_MovementCommandRoute mvRoute;

    /* Reject all affordances when V2 presentation is disabled.
     * This preserves V1 touch/click matrix as the sole input path. */
    if (!v2PresentationEnabled) {
        mvRoute = dm1_v2_movement_command_route_for_presentation(
            0, DM1_V2_MOVEMENT_COMMAND_NONE);
        return make_route(0, aff, DM1_V2_MOVEMENT_COMMAND_NONE, mvRoute);
    }

    /* Shoulder buttons: left bumper = champion cycle, right bumper = chaos.
     * These are CSB-specific one-shot actions with no movement command
     * equivalent; they are accepted at the affordance layer and then
     * dispatched to the appropriate CSB-specific handler (not the movement
     * adapter).  Return accepted=1 with NONE command so callers can
     * distinguish CSB-specific actions from movement affordances. */
    if (aff == CSB_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER ||
        aff == CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER) {
        /* Route NONE through V2 presentation pipeline for the flag flip */
        mvRoute = dm1_v2_movement_command_route_for_presentation(
            1, DM1_V2_MOVEMENT_COMMAND_NONE);
        return make_route(1, aff, DM1_V2_MOVEMENT_COMMAND_NONE, mvRoute);
    }

    /* Normal movement affordances: route through the DM1 V2 movement
     * adapter, which delegates to CLIKMENU.C:142/180 for turn/move. */
    if (cmd == DM1_V2_MOVEMENT_COMMAND_NONE) {
        mvRoute = dm1_v2_movement_command_route_for_presentation(
            1, DM1_V2_MOVEMENT_COMMAND_NONE);
        return make_route(0, aff, cmd, mvRoute);
    }

    mvRoute = dm1_v2_movement_command_route_for_presentation(1, cmd);
    return make_route(1, aff, cmd, mvRoute);
}

/* Debug name lookup */

const char*
csb_v2_touch_controller_affordance_name(
    CSB_V2_TouchControllerAffordance aff) {
    switch (aff) {
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP:               return "touch_swipe_up";
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:             return "touch_swipe_down";
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:             return "touch_swipe_left";
        case CSB_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:            return "touch_swipe_right";
        case CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:      return "touch_edge_strafe_left";
        case CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:     return "touch_edge_strafe_right";
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_UP:          return "controller_dpad_up";
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:        return "controller_dpad_down";
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:        return "controller_dpad_left";
        case CSB_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:       return "controller_dpad_right";
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:    return "controller_left_stick_up";
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:  return "controller_left_stick_down";
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:  return "controller_left_stick_left";
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT: return "controller_left_stick_right";
        case CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT: return "controller_right_stick_left";
        case CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:return "controller_right_stick_right";
        case CSB_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER:      return "controller_left_bumper";
        case CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER:     return "controller_right_bumper";
        case CSB_V2_AFFORDANCE_NONE:
        default:                                             return "none";
    }
}

/* Source-lock evidence */

const char*
csb_v2_touch_controller_affordance_source_evidence(void) {
    return "ReDMCSB COMMAND.C:108-113 C001-C006 movement zones; "
           "COMMAND.C:254-291 C001-C006 keyboard tables; "
           "COMMAND.C:1379 F0358_COMMAND_GetCommandFromMouseInput_CPSC; "
           "COMMAND.C:1452 F0359_COMMAND_ProcessClick_CPSC; "
           "COMMAND.C:1641,1643 primary-to-secondary mouse dispatch; "
           "COMMAND.C:1693 F0360_COMMAND_ProcessPendingClick; "
           "CLIKMENU.C:142 F0365_COMMAND_ProcessTypes1To2_TurnParty; "
           "CLIKMENU.C:180 F0366_COMMAND_ProcessTypes3To6_MoveParty; "
           "GAMELOOP.C:164-219 V1 input wait loop; "
           "CSBWin/Chaos.cpp:60-69 DSA _CALL0-_CALL9 dispatch; "
           "csb_v1_chaos_magic_pc34_compat.c:50 csb_v1_chaos_trigger; "
           "V2 affordances accepted only when V2 presentation enabled; "
           "V1 touch/click matrix (F0358/F0359) preserved as sole input path "
           "when V2 disabled; "
           "CSB left bumper maps to champion cycle, right bumper maps to chaos magic activate; "
           "CSB inherits DM1 movement engine (C001-C006) unchanged.";
}
