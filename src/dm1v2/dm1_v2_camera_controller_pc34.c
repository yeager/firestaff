#include "dm1_v2_camera_controller_pc34.h"

#include <string.h>

/* DM1 V2 camera controller.
 *
 * Source-lock anchors:
 * - ReDMCSB DUNGEON.C:35-44 defines G0233/G0234 direction-to-step tables.
 * - ReDMCSB DUNGEON.C:1371-1391 applies logical map-coordinate movement from
 *   facing direction plus forward/right step counts.
 * - ReDMCSB GAMELOOP.C:90 redraws the dungeon view from the mutated party
 *   direction and map coordinates.
 * - ReDMCSB COMMAND.C:2150-2152 dispatches C001/C002 turns to
 *   F0365_COMMAND_ProcessTypes1To2_TurnParty.
 * - ReDMCSB CLIKMENU.C:142-173 sets StopWaitingForPlayerInput, highlights
 *   the source turn box, updates G0308_i_PartyDirection, and processes
 *   departure/arrival sensors on the same square.
 * - ReDMCSB GAMELOOP.C:215-219 processes one command queue pass, then waits
 *   for stop-waiting/game-time before the next loop.
 *
 * The former controller interpolated coordinates, facing direction and turn
 * pan after a command was accepted. PC34 instead redraws F0128 from the
 * already-mutated G0308/G0306/G0307 tuple. The controller therefore mirrors
 * that tuple immediately and exposes no transient camera state or offsets.
 * The camera is presentation-only: it never mutates the runtime tuple. */

void dm1_v2_camera_init(DM1_V2_CameraController* camera, const DM1_V2_PlayerPos* player) {
    if (!camera) return;
    memset(camera, 0, sizeof(*camera));
    if (player) {
        camera->logicalX = dm1_v2_get_x(player);
        camera->logicalY = dm1_v2_get_y(player);
        camera->visualX = camera->logicalX;
        camera->visualY = camera->logicalY;
        camera->fromX = camera->logicalX;
        camera->fromY = camera->logicalY;
        camera->targetX = camera->logicalX;
        camera->targetY = camera->logicalY;
        camera->facingDir = player->facingDir;
        camera->fromFacingDir = player->facingDir;
        camera->targetFacingDir = player->facingDir;
    }
}

void dm1_v2_camera_begin_move(DM1_V2_CameraController* camera, const DM1_V2_PlayerPos* player, int32_t durationMs) {
    if (!camera || !player) return;
    (void)durationMs;
    camera->logicalX = dm1_v2_get_x(player);
    camera->logicalY = dm1_v2_get_y(player);
    camera->fromX = camera->logicalX;
    camera->fromY = camera->logicalY;
    camera->targetX = camera->logicalX;
    camera->targetY = camera->logicalY;
    camera->visualX = camera->logicalX;
    camera->visualY = camera->logicalY;
    camera->facingDir = player->facingDir;
    camera->fromFacingDir = camera->facingDir;
    camera->targetFacingDir = camera->facingDir;
    camera->elapsedMs = 0;
    camera->durationMs = 0;
    camera->active = 0;
    camera->turning = 0;
    camera->turnPanEnabled = 0;
    camera->turnPanOffsetX = 0;
    camera->on_complete = 0;
    camera->on_complete_ctx = 0;
}

void dm1_v2_camera_begin_turn(DM1_V2_CameraController* camera, int16_t fromFacingDir, int16_t targetFacingDir, int32_t durationMs) {
    if (!camera) return;
    (void)fromFacingDir;
    (void)durationMs;
    camera->fromFacingDir = (int16_t)(targetFacingDir & 7);
    camera->targetFacingDir = (int16_t)(targetFacingDir & 7);
    camera->facingDir = camera->targetFacingDir;
    camera->elapsedMs = 0;
    camera->durationMs = 0;
    camera->active = 0;
    camera->turning = 0;
    camera->turnPanEnabled = 0;
    camera->turnPanOffsetX = 0;
    camera->on_complete = 0;
    camera->on_complete_ctx = 0;
}

void dm1_v2_camera_begin_turn_pan(DM1_V2_CameraController* camera, int16_t fromFacingDir, int16_t targetFacingDir, int32_t durationMs) {
    if (!camera) return;
    dm1_v2_camera_begin_turn(camera, fromFacingDir, targetFacingDir, durationMs);
}

void dm1_v2_camera_tick(DM1_V2_CameraController* camera, int32_t dtMs) {
    (void)camera;
    (void)dtMs;
}

int dm1_v2_camera_is_active(const DM1_V2_CameraController* camera) {
    return camera && camera->active;
}

int16_t dm1_v2_camera_interpolated_facing(const DM1_V2_CameraController* camera) {
    if (!camera) return 0;
    return camera->facingDir;
}

int32_t dm1_v2_camera_offset_x(const DM1_V2_CameraController* camera) {
    if (!camera) return 0;
    return camera->visualX - camera->logicalX;
}

int32_t dm1_v2_camera_offset_y(const DM1_V2_CameraController* camera) {
    if (!camera) return 0;
    return camera->visualY - camera->logicalY;
}

int32_t dm1_v2_camera_turn_pan_offset_x(const DM1_V2_CameraController* camera) {
    if (!camera || !camera->turnPanEnabled) return 0;
    return camera->turnPanOffsetX;
}

int32_t dm1_v2_camera_turn_pan_offset_y(const DM1_V2_CameraController* camera) {
    (void)camera;
    return 0;
}

int32_t dm1_v2_camera_horizontal_pan_offset(const DM1_V2_CameraController* camera) {
    (void)camera;
    return 0;
}
