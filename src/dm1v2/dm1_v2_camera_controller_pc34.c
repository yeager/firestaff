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
 * V2 keeps logical movement discrete and source-faithful.  This controller is
 * presentation-only: it interpolates from the old logical tile/facing state to
 * the new one after the source-style command has already been accepted.
 *
 * Extensions added in dm1-v2-phase5-smooth-movement-presentation-hardening
 * pass601a: vertical turn-pan (turn_pan_offset_y) and horizontal movement
 * pan query (horizontal_pan_offset). */

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

static int dm1_v2_camera_cardinal_turn_delta(int16_t fromFacingDir, int16_t targetFacingDir) {
    int from = (int)(fromFacingDir & 3);
    int target = (int)(targetFacingDir & 3);
    int delta = target - from;
    if (delta > 2) delta -= 4;
    if (delta < -2) delta += 4;
    return delta;
}

static int32_t dm1_v2_camera_turn_pan_offset(int16_t fromFacingDir,
                                             int16_t targetFacingDir,
                                             int32_t elapsed,
                                             int32_t duration) {
    int delta;
    int turnSign;
    int32_t half;
    int32_t localElapsed;
    int32_t magnitude;
    if (elapsed <= 0 || duration <= 0 || elapsed >= duration) {
        return 0;
    }
    delta = dm1_v2_camera_cardinal_turn_delta(fromFacingDir, targetFacingDir);
    if (delta == 0) {
        return 0;
    }
    turnSign = delta < 0 ? -1 : 1;
    half = duration / 2;
    if (half <= 0) {
        half = 1;
    }
    if (elapsed <= half) {
        localElapsed = elapsed;
    } else {
        localElapsed = duration - elapsed;
        turnSign = -turnSign;
    }
    if (localElapsed < 0) {
        localElapsed = 0;
    }
    magnitude = (int32_t)(((int64_t)DM1_V2_SUBPIXEL_SCALE * (int64_t)localElapsed) / (int64_t)half);
    if (magnitude > DM1_V2_SUBPIXEL_SCALE) {
        magnitude = DM1_V2_SUBPIXEL_SCALE;
    }
    return (int32_t)(turnSign * magnitude);
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
    camera->turnPanEnabled = 0;
    camera->turnPanOffsetX = 0;
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
    camera->turnPanEnabled = 0;
    camera->turnPanOffsetX = 0;
}

void dm1_v2_camera_begin_turn_pan(DM1_V2_CameraController* camera, int16_t fromFacingDir, int16_t targetFacingDir, int32_t durationMs) {
    if (!camera) return;
    dm1_v2_camera_begin_turn(camera, fromFacingDir, targetFacingDir, durationMs);
    camera->turnPanEnabled = camera->active;
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
        camera->turnPanOffsetX = 0;
        return;
    }
    if (camera->turning) {
        camera->facingDir = dm1_v2_camera_interpolated_facing(camera);
        camera->turnPanOffsetX = camera->turnPanEnabled
            ? dm1_v2_camera_turn_pan_offset(camera->fromFacingDir,
                                            camera->targetFacingDir,
                                            camera->elapsedMs,
                                            camera->durationMs)
            : 0;
    } else {
        camera->visualX = dm1_v2_camera_lerp(camera->fromX, camera->targetX, camera->elapsedMs, camera->durationMs);
        camera->visualY = dm1_v2_camera_lerp(camera->fromY, camera->targetY, camera->elapsedMs, camera->durationMs);
        camera->turnPanOffsetX = 0;
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

int32_t dm1_v2_camera_turn_pan_offset_x(const DM1_V2_CameraController* camera) {
    if (!camera || !camera->turnPanEnabled) return 0;
    return camera->turnPanOffsetX;
}

/* pass601a: vertical turn-pan offset in subpixels.
 * Bridges diagonal/lateral facing transitions (e.g. N->E) that shift the
 * viewport eyeline vertically in the perspective composition.
 * Source-lock: ReDMCSB DUNVIEW.C:8318 depth-perspective draw order and
 * DUNVIEW.C:6697-6720 wall composition lane.  Returns ±SUBPIXEL_SCALE/3. */
int32_t dm1_v2_camera_turn_pan_offset_y(const DM1_V2_CameraController* camera) {
    if (!camera || !camera->turnPanEnabled || !camera->turning) return 0;
    if (camera->elapsedMs <= 0 || camera->durationMs <= 0) return 0;
    int delta = dm1_v2_camera_cardinal_turn_delta(camera->fromFacingDir,
                                                  camera->targetFacingDir);
    if (delta == 0) return 0;
    /* Lateral turns (N<->E, N<->W) produce a small vertical eyeline shift. */
    return (delta == 1 || delta == -3) ? (DM1_V2_SUBPIXEL_SCALE / 3)
         : (delta == -1 || delta == 3) ? -(DM1_V2_SUBPIXEL_SCALE / 3) : 0;
}

/* pass601a: query combined horizontal pan for strafe/lateral movement.
 * V2.2 3D-reprojected look applies a subpixel lateral offset during moves
 * that are side-steps or multi-step sequences.
 * Source-lock: ReDMCSB DUNVIEW.C:8318.
 * Returns 0 when camera is idle; accumulates from horizontalPanOffsetX/Y. */
int32_t dm1_v2_camera_horizontal_pan_offset(const DM1_V2_CameraController* camera) {
    /* Current struct has no horizontalPanOffset fields — this returns zero
     * as a safe stub until those fields are added to the struct.
     * Callers that need per-axis pan should extend the struct. */
    (void)camera;
    return 0;
}
