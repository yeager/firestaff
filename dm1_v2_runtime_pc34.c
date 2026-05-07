#include "dm1_v2_runtime_pc34.h"

#include <string.h>

/* DM1 V2 runtime shell.
 *
 * Source-lock anchors:
 * - ReDMCSB GAMELOOP.C:164 sets G0321_B_StopWaitingForPlayerInput before the
 *   input command loop.
 * - ReDMCSB GAMELOOP.C:215 dispatches F0380_COMMAND_ProcessQueue_CPSC each
 *   loop, then GAMELOOP.C:219 waits until a command stops input waiting and
 *   game time is ticking.
 * - ReDMCSB DUNGEON.C:1371-1391 updates map coordinates from facing direction,
 *   forward steps and right/strafe steps using direction-to-step tables.
 *
 * V2 keeps those semantics split into a logical runtime tick and a modern
 * viewport/camera presentation layer.  This file intentionally does not make
 * gameplay decisions; it owns the V2 shell state used by later lanes. */

#define DM1_V2_DEFAULT_MOVE_SPEED 256
#define DM1_V2_DEFAULT_TURN_SPEED 1
#define DM1_V2_DEFAULT_STEP_THRESHOLD 256
#define DM1_V2_DEFAULT_SUBPIXEL_ACCEL 0
#define DM1_V2_DEFAULT_COLLISION_PREDICT_FRAMES 0
#define DM1_V2_COMMAND_MOVE_FORWARD 1
#define DM1_V2_COMMAND_MOVE_BACKWARD 2
#define DM1_V2_COMMAND_TURN_LEFT 3
#define DM1_V2_COMMAND_TURN_RIGHT 4
#define DM1_V2_COMMAND_MOVE_RIGHT 5
#define DM1_V2_COMMAND_MOVE_LEFT 6

void dm1_v2_runtime_init(DM1_V2_RuntimeState* runtime) {
    if (!runtime) return;
    memset(runtime, 0, sizeof(*runtime));
    runtime->mode = DM1_V2_RUNTIME_STOPPED;
    dm1_v2_pos_init(&runtime->player, 0, 0, 0);
    runtime->movement.moveSpeed = DM1_V2_DEFAULT_MOVE_SPEED;
    runtime->movement.turnSpeed = DM1_V2_DEFAULT_TURN_SPEED;
    runtime->movement.stepThreshold = DM1_V2_DEFAULT_STEP_THRESHOLD;
    runtime->movement.subPixelAccel = DM1_V2_DEFAULT_SUBPIXEL_ACCEL;
    runtime->movement.collisionPredictFrames = DM1_V2_DEFAULT_COLLISION_PREDICT_FRAMES;
    dm1_v2_vp_init(&runtime->viewport);
    runtime->lastCommand = 0;
}

void dm1_v2_runtime_start(DM1_V2_RuntimeState* runtime, uint32_t nowMs) {
    if (!runtime) return;
    runtime->mode = DM1_V2_RUNTIME_RUNNING;
    runtime->tickCount = 0;
    runtime->lastTickMs = nowMs;
    runtime->lastCommand = 0;
    dm1_v2_vp_mark_dirty(&runtime->viewport);
}

void dm1_v2_runtime_stop(DM1_V2_RuntimeState* runtime) {
    if (!runtime) return;
    runtime->mode = DM1_V2_RUNTIME_STOPPED;
}

int dm1_v2_runtime_is_running(const DM1_V2_RuntimeState* runtime) {
    return runtime && runtime->mode == DM1_V2_RUNTIME_RUNNING;
}

void dm1_v2_runtime_tick(DM1_V2_RuntimeState* runtime, uint32_t nowMs) {
    uint32_t dt;
    if (!dm1_v2_runtime_is_running(runtime)) return;
    dt = nowMs >= runtime->lastTickMs ? nowMs - runtime->lastTickMs : 0U;
    runtime->tickCount++;
    runtime->lastTickMs = nowMs;
    dm1_v2_vp_tick_scroll(&runtime->viewport, (int)dt);
    if (!dm1_v2_vp_is_scrolling(&runtime->viewport)) {
        dm1_v2_vp_present(&runtime->viewport, (int32_t)nowMs);
    }
}

int dm1_v2_runtime_apply_command(DM1_V2_RuntimeState* runtime, int command, uint32_t nowMs) {
    if (!dm1_v2_runtime_is_running(runtime)) return 0;
    runtime->lastCommand = command;
    switch (command) {
        case DM1_V2_COMMAND_MOVE_FORWARD:
            dm1_v2_move_step(&runtime->player, &runtime->movement, runtime->player.facingDir, (int32_t)nowMs);
            dm1_v2_vp_begin_scroll(&runtime->viewport, 0, -8, 16);
            break;
        case DM1_V2_COMMAND_MOVE_BACKWARD:
            dm1_v2_move_step(&runtime->player, &runtime->movement, (runtime->player.facingDir + 4) & 7, (int32_t)nowMs);
            dm1_v2_vp_begin_scroll(&runtime->viewport, 0, 8, 16);
            break;
        case DM1_V2_COMMAND_TURN_LEFT:
            dm1_v2_turn(&runtime->player, -1);
            dm1_v2_vp_begin_scroll(&runtime->viewport, -8, 0, 16);
            break;
        case DM1_V2_COMMAND_TURN_RIGHT:
            dm1_v2_turn(&runtime->player, 1);
            dm1_v2_vp_begin_scroll(&runtime->viewport, 8, 0, 16);
            break;
        case DM1_V2_COMMAND_MOVE_RIGHT:
            dm1_v2_move_step(&runtime->player, &runtime->movement, (runtime->player.facingDir + 2) & 7, (int32_t)nowMs);
            dm1_v2_vp_begin_scroll(&runtime->viewport, 8, 0, 16);
            break;
        case DM1_V2_COMMAND_MOVE_LEFT:
            dm1_v2_move_step(&runtime->player, &runtime->movement, (runtime->player.facingDir + 6) & 7, (int32_t)nowMs);
            dm1_v2_vp_begin_scroll(&runtime->viewport, -8, 0, 16);
            break;
        default:
            return 0;
    }
    dm1_v2_vp_mark_dirty(&runtime->viewport);
    return 1;
}
