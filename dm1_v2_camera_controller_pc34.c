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
 * - ReDMCSB GAMELOOP.C:215-219 processes one command queue pass, then waits
 *   for stop-waiting/game-time before the next loop.
 *
 * V2 keeps logical movement discrete and source-faithful.  This controller is
 * presentation-only: it interpolates from the old logical tile/facing state to
 * the new one after the source-style command has already been accepted. */

static int32_t dm1_v2_camera_clamp_duration(int32_t durationMs) {
    return durationMs <= 0 ? 1 : durationMs;
}

static int32_t dm1_v2_camera_lerp(int32_t from, int32_t to, int32_t elapsed, int32_t duration) {
    int64_t delta;
    if (elapsed >= duration) return to;
    if (elapsed <= 0) return from;
    delta = (int64_t)(to - from) * (int64_t)elapsed;
    return from + (int32_t)(delta / duration);
}

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
    camera->fromX = camera->visualX;
    camera->fromY = camera->visualY;
    camera->logicalX = dm1_v2_get_x(player);
    camera->logicalY = dm1_v2_get_y(player);
    camera->targetX = camera->logicalX;
    camera->targetY = camera->logicalY;
    camera->facingDir = player->facingDir;
    camera->fromFacingDir = camera->facingDir;
    camera->targetFacingDir = camera->facingDir;
    camera->elapsedMs = 0;
    camera->durationMs = dm1_v2_camera_clamp_duration(durationMs);
    camera->active = (camera->fromX != camera->targetX || camera->fromY != camera->targetY);
    camera->turning = 0;
    if (!camera->active) {
        camera->visualX = camera->targetX;
        camera->visualY = camera->targetY;
    }
}

void dm1_v2_camera_begin_turn(DM1_V2_CameraController* camera, int16_t fromFacingDir, int16_t targetFacingDir, int32_t durationMs) {
    if (!camera) return;
    camera->fromFacingDir = (int16_t)(fromFacingDir & 7);
    camera->targetFacingDir = (int16_t)(targetFacingDir & 7);
    camera->facingDir = camera->fromFacingDir;
    camera->elapsedMs = 0;
    camera->durationMs = dm1_v2_camera_clamp_duration(durationMs);
    camera->active = camera->fromFacingDir != camera->targetFacingDir;
    camera->turning = camera->active;
}

void dm1_v2_camera_tick(DM1_V2_CameraController* camera, int32_t dtMs) {
    if (!camera || !camera->active) return;
    if (dtMs < 0) dtMs = 0;
    camera->elapsedMs += dtMs;
    if (camera->elapsedMs >= camera->durationMs) {
        camera->elapsedMs = camera->durationMs;
        camera->visualX = camera->targetX;
        camera->visualY = camera->targetY;
        camera->facingDir = camera->targetFacingDir;
        camera->active = 0;
        camera->turning = 0;
        return;
    }
    if (camera->turning) {
        camera->facingDir = dm1_v2_camera_interpolated_facing(camera);
    } else {
        camera->visualX = dm1_v2_camera_lerp(camera->fromX, camera->targetX, camera->elapsedMs, camera->durationMs);
        camera->visualY = dm1_v2_camera_lerp(camera->fromY, camera->targetY, camera->elapsedMs, camera->durationMs);
    }
}

int dm1_v2_camera_is_active(const DM1_V2_CameraController* camera) {
    return camera && camera->active;
}

int16_t dm1_v2_camera_interpolated_facing(const DM1_V2_CameraController* camera) {
    int delta;
    if (!camera) return 0;
    if (!camera->turning || camera->elapsedMs >= camera->durationMs) {
        return camera->targetFacingDir;
    }
    delta = camera->targetFacingDir - camera->fromFacingDir;
    if (delta > 4) delta -= 8;
    if (delta < -4) delta += 8;
    if (camera->elapsedMs * 2 < camera->durationMs) {
        return camera->fromFacingDir;
    }
    return (int16_t)((camera->fromFacingDir + delta) & 7);
}

int32_t dm1_v2_camera_offset_x(const DM1_V2_CameraController* camera) {
    if (!camera) return 0;
    return camera->visualX - camera->logicalX;
}

int32_t dm1_v2_camera_offset_y(const DM1_V2_CameraController* camera) {
    if (!camera) return 0;
    return camera->visualY - camera->logicalY;
}
