#ifndef FIRESTAFF_DM1_V2_MOVEMENT_COMMAND_ADAPTER_PC34_H
#define FIRESTAFF_DM1_V2_MOVEMENT_COMMAND_ADAPTER_PC34_H

#include <stdint.h>
#include "dm1_v2_camera_controller_pc34.h"
#include "dm1_v2_runtime_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V2_MOVEMENT_COMMAND_NONE = 0,
    DM1_V2_MOVEMENT_COMMAND_TURN_LEFT = 1,
    DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT = 2,
    DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD = 3,
    DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT = 4,
    DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD = 5,
    DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT = 6
} DM1_V2_MovementCommand;

typedef struct {
    int accepted;
    int sourceCommand;
    int runtimeCommand;
    int16_t fromFacingDir;
    int16_t targetFacingDir;
} DM1_V2_MovementCommandResult;

DM1_V2_MovementCommandResult dm1_v2_movement_command_apply(
    DM1_V2_RuntimeState* runtime,
    DM1_V2_CameraController* camera,
    DM1_V2_MovementCommand command,
    uint32_t nowMs,
    int32_t cameraDurationMs);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_MOVEMENT_COMMAND_ADAPTER_PC34_H */
