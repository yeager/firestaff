#include "dm1_v2_movement_command_adapter_pc34.h"

/* DM1 V2 movement command adapter.
 *
 * Source-lock anchors:
 * - ReDMCSB DEFS.H:238-243 assigns movement command ids: turn-left=1,
 *   turn-right=2, move-forward=3, move-right=4, move-backward=5,
 *   move-left=6.
 * - ReDMCSB COMMAND.C:2045-2155 F0380_COMMAND_ProcessQueue_CPSC pops one
 *   queued command, dispatches C001/C002 to F0365_COMMAND_ProcessTypes1To2_TurnParty
 *   and C003..C006 to F0366_COMMAND_ProcessTypes3To6_MoveParty.
 * - ReDMCSB CLIKMENU.C:142-174 F0365_COMMAND_ProcessTypes1To2_TurnParty
 *   sets StopWaitingForPlayerInput and updates party direction by +1 for
 *   turn-right or +3 for turn-left in the four-direction source model.
 * - ReDMCSB CLIKMENU.C:180-390 F0366_COMMAND_ProcessTypes3To6_MoveParty maps
 *   movement arrows to forward/right step counts before calling
 *   F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement.
 * - ReDMCSB DUNGEON.C:35-44 and DUNGEON.C:1371-1391 define the direction step
 *   tables and apply forward plus right/strafe counts to map coordinates.
 * - ReDMCSB GAMELOOP.C:164-219 clears StopWaitingForPlayerInput, processes
 *   F0380_COMMAND_ProcessQueue_CPSC, redraws/highlight-disables while waiting,
 *   and only advances after a command stops waiting and game time is ticking.
 *
 * V2 keeps gameplay movement discrete/source-like.  With the V2 presentation
 * route disabled, source command ids remain untouched for the V1 command queue.
 * With the explicit V2 presentation route enabled, this adapter translates
 * source-faithful semantic command ids onto the current V2 runtime shell command
 * ids, then starts the camera controller for presentation-only interpolation. */

#define DM1_V2_RUNTIME_COMMAND_MOVE_FORWARD 1
#define DM1_V2_RUNTIME_COMMAND_MOVE_BACKWARD 2
#define DM1_V2_RUNTIME_COMMAND_TURN_LEFT 3
#define DM1_V2_RUNTIME_COMMAND_TURN_RIGHT 4
#define DM1_V2_RUNTIME_COMMAND_MOVE_RIGHT 5
#define DM1_V2_RUNTIME_COMMAND_MOVE_LEFT 6

static DM1_V2_MovementCommandResult dm1_v2_result(int sourceCommand, int runtimeCommand) {
    DM1_V2_MovementCommandResult result;
    result.accepted = 0;
    result.sourceCommand = sourceCommand;
    result.runtimeCommand = runtimeCommand;
    result.fromFacingDir = 0;
    result.targetFacingDir = 0;
    return result;
}

static int dm1_v2_translate_runtime_command(DM1_V2_MovementCommand command) {
    switch (command) {
        case DM1_V2_MOVEMENT_COMMAND_TURN_LEFT:
            return DM1_V2_RUNTIME_COMMAND_TURN_LEFT;
        case DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT:
            return DM1_V2_RUNTIME_COMMAND_TURN_RIGHT;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD:
            return DM1_V2_RUNTIME_COMMAND_MOVE_FORWARD;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD:
            return DM1_V2_RUNTIME_COMMAND_MOVE_BACKWARD;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT:
            return DM1_V2_RUNTIME_COMMAND_MOVE_RIGHT;
        case DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT:
            return DM1_V2_RUNTIME_COMMAND_MOVE_LEFT;
        case DM1_V2_MOVEMENT_COMMAND_NONE:
        default:
            return 0;
    }
}

DM1_V2_MovementCommandRoute dm1_v2_movement_command_route_for_presentation(
    int v2PresentationEnabled,
    DM1_V2_MovementCommand command) {
    DM1_V2_MovementCommandRoute route;
    route.routeKind = v2PresentationEnabled
        ? DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION
        : DM1_V2_MOVEMENT_ROUTE_V1_SOURCE;
    route.v2PresentationEnabled = v2PresentationEnabled ? 1 : 0;
    route.sourceCommand = (int)command;
    route.runtimeCommand = v2PresentationEnabled
        ? dm1_v2_translate_runtime_command(command)
        : (int)command;

    if (command == DM1_V2_MOVEMENT_COMMAND_NONE) {
        route.runtimeCommand = 0;
    }
    return route;
}

DM1_V2_MovementCommandResult dm1_v2_movement_command_apply(
    DM1_V2_RuntimeState* runtime,
    DM1_V2_CameraController* camera,
    DM1_V2_MovementCommand command,
    uint32_t nowMs,
    int32_t cameraDurationMs) {
    int runtimeCommand = dm1_v2_movement_command_route_for_presentation(
        1,
        command).runtimeCommand;
    DM1_V2_MovementCommandResult result = dm1_v2_result((int)command, runtimeCommand);
    int16_t beforeFacing;

    if (!runtime || runtimeCommand == 0) {
        return result;
    }

    beforeFacing = runtime->player.facingDir;
    result.fromFacingDir = beforeFacing;
    if (!dm1_v2_runtime_apply_command(runtime, runtimeCommand, nowMs)) {
        result.targetFacingDir = runtime->player.facingDir;
        return result;
    }

    result.accepted = 1;
    result.targetFacingDir = runtime->player.facingDir;
    if (camera) {
        if ((command == DM1_V2_MOVEMENT_COMMAND_TURN_LEFT) ||
            (command == DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT)) {
            dm1_v2_camera_begin_turn(camera, beforeFacing, runtime->player.facingDir, cameraDurationMs);
        } else {
            dm1_v2_camera_begin_move(camera, &runtime->player, cameraDurationMs);
        }
    }
    return result;
}
