#ifndef FIRESTAFF_DM1_V2_CAMERA_CONTROLLER_PC34_H
#define FIRESTAFF_DM1_V2_CAMERA_CONTROLLER_PC34_H

#include <stdint.h>
#include "dm1_v2_movement_engine_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t logicalX;
    int32_t logicalY;
    int32_t visualX;
    int32_t visualY;
    int32_t fromX;
    int32_t fromY;
    int32_t targetX;
    int32_t targetY;
    int16_t facingDir;
    int16_t fromFacingDir;
    int16_t targetFacingDir;
    int32_t elapsedMs;
    int32_t durationMs;
    int32_t turnPanOffsetX;
    int active;
    int turning;
    int turnPanEnabled;
} DM1_V2_CameraController;

void dm1_v2_camera_init(DM1_V2_CameraController* camera, const DM1_V2_PlayerPos* player);
void dm1_v2_camera_begin_move(DM1_V2_CameraController* camera, const DM1_V2_PlayerPos* player, int32_t durationMs);
void dm1_v2_camera_begin_turn(DM1_V2_CameraController* camera, int16_t fromFacingDir, int16_t targetFacingDir, int32_t durationMs);
void dm1_v2_camera_begin_turn_pan(DM1_V2_CameraController* camera, int16_t fromFacingDir, int16_t targetFacingDir, int32_t durationMs);
void dm1_v2_camera_tick(DM1_V2_CameraController* camera, int32_t dtMs);
int dm1_v2_camera_is_active(const DM1_V2_CameraController* camera);
int16_t dm1_v2_camera_interpolated_facing(const DM1_V2_CameraController* camera);
int32_t dm1_v2_camera_offset_x(const DM1_V2_CameraController* camera);
int32_t dm1_v2_camera_offset_y(const DM1_V2_CameraController* camera);
int32_t dm1_v2_camera_turn_pan_offset_x(const DM1_V2_CameraController* camera);

/* pass601a: vertical turn-pan for diagonal/lateral facing transitions.
 * Source-lock: ReDMCSB DUNVIEW.C:8318 depth-perspective draw order. */
int32_t dm1_v2_camera_turn_pan_offset_y(const DM1_V2_CameraController* camera);

/* pass601a: combined lateral pan for strafe/multi-step movement.
 * Source-lock: ReDMCSB DUNVIEW.C:8318.
 * Currently returns 0 as a safe stub until horizontal pan fields are added
 * to DM1_V2_CameraController. */
int32_t dm1_v2_camera_horizontal_pan_offset(const DM1_V2_CameraController* camera);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_CAMERA_CONTROLLER_PC34_H */
