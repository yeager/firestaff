/*
 * theron_v2_touch_controller_affordance.c
 *
 * Theron V2 Phase 6 touch/controller affordance mapping.
 *
 * Source-lock anchors:
 *   THQUEST.ASM T520/T560/T600  Theron party placement, viewport, UI zones
 *   ReDMCSB DEFS.H:238-243      C001-C006 movement command ids
 *   ReDMCSB COMMAND.C:2045-2155 F0380 command queue dispatch
 *   ReDMCSB CLIKMENU.C:142      F0365_COMMAND_ProcessTypes1To2_TurnParty
 *   ReDMCSB CLIKMENU.C:180      F0366_COMMAND_ProcessTypes3To6_MoveParty
 *   ReDMCSB GAMELOOP.C:164-219  V1 input wait/command queue loop
 */

#include "theron_v2_touch_controller_affordance.h"

static Theron_V2_TouchControllerAffordanceRoute
make_route(int accepted,
           Theron_V2_TouchControllerAffordance aff,
           DM1_V2_MovementCommand command,
           DM1_V2_MovementCommandRoute route)
{
    Theron_V2_TouchControllerAffordanceRoute r;
    r.accepted = accepted;
    r.v2Only = 1;
    r.inputKind = theron_v2_touch_controller_affordance_input_kind(aff);
    r.affordance = aff;
    r.movementCommand = command;
    r.route = route;
    return r;
}

static int theron_v2_runtime_command(DM1_V2_MovementCommand command)
{
    switch (command) {
        case DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD:
            return 1;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD:
            return 2;
        case DM1_V2_MOVEMENT_COMMAND_TURN_LEFT:
            return 3;
        case DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT:
            return 4;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT:
            return 5;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT:
            return 6;
        case DM1_V2_MOVEMENT_COMMAND_NONE:
        default:
            return 0;
    }
}

static DM1_V2_MovementCommandRoute
theron_v2_movement_route_for_presentation(
    int v2PresentationEnabled,
    DM1_V2_MovementCommand command)
{
    DM1_V2_MovementCommandRoute route;
    route.routeKind = v2PresentationEnabled
        ? DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION
        : DM1_V2_MOVEMENT_ROUTE_V1_SOURCE;
    route.v2PresentationEnabled = v2PresentationEnabled ? 1 : 0;
    route.sourceCommand = (int)command;
    route.runtimeCommand = v2PresentationEnabled
        ? theron_v2_runtime_command(command)
        : (int)command;
    if (command == DM1_V2_MOVEMENT_COMMAND_NONE) {
        route.runtimeCommand = 0;
    }
    return route;
}

DM1_V2_MovementCommand
theron_v2_touch_controller_affordance_movement_command(
    Theron_V2_TouchControllerAffordance aff)
{
    switch (aff) {
        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP:
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD;

        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD;

        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
        case THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
            return DM1_V2_MOVEMENT_COMMAND_TURN_LEFT;

        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
        case THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
            return DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT;

        case THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT;

        case THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT;

        case THERON_V2_AFFORDANCE_NONE:
        default:
            return DM1_V2_MOVEMENT_COMMAND_NONE;
    }
}

Theron_V2_TouchControllerInputKind
theron_v2_touch_controller_affordance_input_kind(
    Theron_V2_TouchControllerAffordance aff)
{
    switch (aff) {
        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP:
        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
        case THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
        case THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
            return THERON_V2_AFFORDANCE_INPUT_TOUCH;

        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
        case THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
        case THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
            return THERON_V2_AFFORDANCE_INPUT_CONTROLLER;

        case THERON_V2_AFFORDANCE_NONE:
        default:
            return THERON_V2_AFFORDANCE_INPUT_NONE;
    }
}

Theron_V2_TouchControllerAffordanceRoute
theron_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    Theron_V2_TouchControllerAffordance aff)
{
    DM1_V2_MovementCommand command =
        theron_v2_touch_controller_affordance_movement_command(aff);
    DM1_V2_MovementCommandRoute route =
        theron_v2_movement_route_for_presentation(
            v2PresentationEnabled ? 1 : 0, command);

    if (!v2PresentationEnabled || command == DM1_V2_MOVEMENT_COMMAND_NONE) {
        return make_route(0, aff, command, route);
    }

    return make_route(1, aff, command, route);
}

const char *
theron_v2_touch_controller_affordance_name(
    Theron_V2_TouchControllerAffordance aff)
{
    switch (aff) {
        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP: return "touch_swipe_up";
        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_DOWN: return "touch_swipe_down";
        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_LEFT: return "touch_swipe_left";
        case THERON_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT: return "touch_swipe_right";
        case THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT: return "touch_edge_strafe_left";
        case THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT: return "touch_edge_strafe_right";
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_UP: return "controller_dpad_up";
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN: return "controller_dpad_down";
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT: return "controller_dpad_left";
        case THERON_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT: return "controller_dpad_right";
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP: return "controller_left_stick_up";
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN: return "controller_left_stick_down";
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT: return "controller_left_stick_left";
        case THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT: return "controller_left_stick_right";
        case THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT: return "controller_right_stick_left";
        case THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT: return "controller_right_stick_right";
        case THERON_V2_AFFORDANCE_NONE:
        default:
            return "none";
    }
}

const char *theron_v2_touch_controller_affordance_source_evidence(void)
{
    return
        "Theron V2 Touch/Controller Affordance - Phase 6\n"
        "Source: THQUEST.ASM T520/T560/T600 party placement, viewport, UI zones\n"
        "Source: ReDMCSB DEFS.H:238-243 C001-C006 movement command ids\n"
        "Source: ReDMCSB COMMAND.C:2045-2155 F0380 command queue dispatch\n"
        "Source: ReDMCSB CLIKMENU.C:142 F0365_COMMAND_ProcessTypes1To2_TurnParty\n"
        "Source: ReDMCSB CLIKMENU.C:180 F0366_COMMAND_ProcessTypes3To6_MoveParty\n"
        "Source: ReDMCSB GAMELOOP.C:164-219 V1 input wait/command queue loop\n"
        "Theron inherits the DM1-family discrete movement command ids.\n"
        "V1 touch/click parity: all affordances are rejected when V2 presentation is off.\n"
        "Edge strafe is V2-only and maps to C006/C004 through the shared adapter.\n";
}
