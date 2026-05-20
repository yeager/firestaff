#include "dm1_v2_touch_controller_affordance_pc34.h"

/* DM1 V2 touch/controller affordance metadata.
 *
 * Source-lock anchors:
 * - ReDMCSB DEFS.H:197-211 defines input records as command-bearing keyboard
 *   and mouse zones, and DEFS.H:238-243 owns movement command ids C001..C006.
 * - ReDMCSB COMMAND.C:2045-2155 F0380_COMMAND_ProcessQueue_CPSC consumes the
 *   queued source command and dispatches C001/C002 to turns and C003..C006 to
 *   movement without knowing whether the command came from keyboard or mouse.
 * - ReDMCSB CLIKMENU.C:142-174 and CLIKMENU.C:180-390 own turn/move behavior;
 *   this file only labels V2 presentation affordances with those source ids.
 * - ReDMCSB GAMELOOP.C:164-219 owns the V1 input wait/command queue loop.
 *
 * V2 gesture/controller affordances are metadata only.  The V1/off route
 * deliberately rejects them so the existing V1 mouse/touch/click route matrix
 * remains the only V1 input path.  When V2 presentation is explicitly enabled,
 * each affordance maps to a source-locked movement command and then reuses the
 * existing movement command adapter route. */

static DM1_V2_TouchControllerAffordanceRoute dm1_v2_affordance_result(
    int accepted,
    DM1_V2_TouchControllerAffordance affordance,
    DM1_V2_MovementCommand movementCommand,
    DM1_V2_MovementCommandRoute route) {
    DM1_V2_TouchControllerAffordanceRoute result;
    result.accepted = accepted;
    result.v2Only = 1;
    result.inputKind = dm1_v2_touch_controller_affordance_input_kind(affordance);
    result.affordance = affordance;
    result.movementCommand = movementCommand;
    result.route = route;
    return result;
}

DM1_V2_MovementCommand dm1_v2_touch_controller_affordance_movement_command(
    DM1_V2_TouchControllerAffordance affordance) {
    switch (affordance) {
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_UP:
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD;
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD;
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
        case DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
            return DM1_V2_MOVEMENT_COMMAND_TURN_LEFT;
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
        case DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
            return DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT;
        case DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT;
        case DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
        case DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT;
        case DM1_V2_AFFORDANCE_NONE:
        default:
            return DM1_V2_MOVEMENT_COMMAND_NONE;
    }
}

DM1_V2_TouchControllerInputKind dm1_v2_touch_controller_affordance_input_kind(
    DM1_V2_TouchControllerAffordance affordance) {
    switch (affordance) {
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_UP:
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
        case DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
        case DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
            return DM1_V2_AFFORDANCE_INPUT_TOUCH;
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
        case DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
        case DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER:
        case DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER:
            return DM1_V2_AFFORDANCE_INPUT_CONTROLLER;
        case DM1_V2_AFFORDANCE_NONE:
        default:
            return DM1_V2_AFFORDANCE_INPUT_NONE;
    }
}

DM1_V2_TouchControllerAffordanceRoute dm1_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    DM1_V2_TouchControllerAffordance affordance) {
    DM1_V2_MovementCommand command =
        dm1_v2_touch_controller_affordance_movement_command(affordance);
    DM1_V2_MovementCommandRoute route;

    if (!v2PresentationEnabled || command == DM1_V2_MOVEMENT_COMMAND_NONE) {
        route = dm1_v2_movement_command_route_for_presentation(
            0,
            DM1_V2_MOVEMENT_COMMAND_NONE);
        return dm1_v2_affordance_result(0, affordance, command, route);
    }

    route = dm1_v2_movement_command_route_for_presentation(1, command);
    return dm1_v2_affordance_result(1, affordance, command, route);
}

const char* dm1_v2_touch_controller_affordance_name(
    DM1_V2_TouchControllerAffordance affordance) {
    switch (affordance) {
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_UP: return "touch_swipe_up";
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_DOWN: return "touch_swipe_down";
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_LEFT: return "touch_swipe_left";
        case DM1_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT: return "touch_swipe_right";
        case DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT: return "touch_edge_strafe_left";
        case DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT: return "touch_edge_strafe_right";
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_UP: return "controller_dpad_up";
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN: return "controller_dpad_down";
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT: return "controller_dpad_left";
        case DM1_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT: return "controller_dpad_right";
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP: return "controller_left_stick_up";
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN: return "controller_left_stick_down";
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT: return "controller_left_stick_left";
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT: return "controller_left_stick_right";
        case DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT: return "controller_right_stick_left";
        case DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT: return "controller_right_stick_right";
        case DM1_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER: return "controller_left_bumper";
        case DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER: return "controller_right_bumper";
        case DM1_V2_AFFORDANCE_NONE:
        default: return "none";
    }
}

const char* dm1_v2_touch_controller_affordance_source_lock_evidence(void) {
    return "ReDMCSB DEFS.H:197-211,238-243 input records and C001..C006 movement commands; COMMAND.C:2045-2155 queue dispatch; CLIKMENU.C:142-174,180-390 movement/turn owners; GAMELOOP.C:164-219 V1 input wait loop. V2 touch/controller affordances are accepted only when V2 presentation is enabled and route through dm1_v2_movement_command_route_for_presentation.";
}
