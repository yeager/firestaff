/* test_dm1_v2_movement_camera_pc34.c — V2 movement command -> camera gate.
 *
 * Source-lock intent:
 * - ReDMCSB COMMAND.C:2095-2100 rejects movement while source cooldowns
 *   G0310/G0311 are active; COMMAND.C:2150-2156 dispatches accepted turns
 *   and moves to the source handlers only.
 * - ReDMCSB CLIKMENU.C:278-329 owns collision decisions and F0267 move-result
 *   dispatch before CLIKMENU.C:330-346 writes the source movement cooldown.
 * - ReDMCSB MOVESENS.C:799-819 owns source sensor removal/addition dispatch.
 * - ReDMCSB GAMELOOP.C:69,90,124,150-155,215-219 owns timeline, redraw,
 *   game-time, cooldown decrement, and command-loop cadence.
 *
 * Firestaff V2 keeps that split: command adapter mutates logical runtime state,
 * then camera controller interpolates presentation only.  This gate verifies that
 * movement/turning starts camera animation without changing source-owned timing,
 * collision, sensor, creature, or redraw-cadence state. */

#include "dm1_v2_movement_command_adapter_pc34.h"

#include <stdio.h>

static int failures = 0;
#define CHECK(expr) do { \
    if (!(expr)) { \
        failures++; \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
    } \
} while (0)

static void init_running(DM1_V2_RuntimeState* rt, DM1_V2_CameraController* camera) {
    dm1_v2_runtime_init(rt);
    dm1_v2_runtime_start(rt, 1000U);
    dm1_v2_camera_init(camera, &rt->player);
}

static void test_forward_command_starts_move_camera(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;
    int32_t startX;
    int32_t startY;

    init_running(&rt, &camera);
    startX = camera.visualX;
    startY = camera.visualY;

    result = dm1_v2_movement_command_apply(&rt,
                                           &camera,
                                           DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD,
                                           1000U,
                                           64);

    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 3);
    CHECK(result.runtimeCommand == 1);
    CHECK(rt.lastCommand == 1);
    CHECK(camera.turning == 0);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(camera.fromX == startX);
    CHECK(camera.fromY == startY);
    CHECK(camera.targetX == dm1_v2_get_x(&rt.player));
    CHECK(camera.targetY == dm1_v2_get_y(&rt.player));
    CHECK(camera.visualX == startX);
    CHECK(camera.visualY == startY);

    dm1_v2_camera_tick(&camera, 32);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(camera.visualX >= camera.fromX && camera.visualX <= camera.targetX);
    CHECK(camera.visualY >= camera.fromY && camera.visualY <= camera.targetY);

    dm1_v2_camera_tick(&camera, 32);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.visualX == dm1_v2_get_x(&rt.player));
    CHECK(camera.visualY == dm1_v2_get_y(&rt.player));
    CHECK(dm1_v2_camera_offset_x(&camera) == 0);
    CHECK(dm1_v2_camera_offset_y(&camera) == 0);
}

static void test_turn_left_command_starts_turn_camera(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;
    int32_t startX;
    int32_t startY;

    init_running(&rt, &camera);
    startX = camera.visualX;
    startY = camera.visualY;

    result = dm1_v2_movement_command_apply(&rt,
                                           &camera,
                                           DM1_V2_MOVEMENT_COMMAND_TURN_LEFT,
                                           1100U,
                                           80);

    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 1);
    CHECK(result.runtimeCommand == 3);
    CHECK(result.fromFacingDir == 0);
    CHECK(result.targetFacingDir == 7);
    CHECK(rt.player.facingDir == 7);
    CHECK(camera.turning == 1);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(camera.fromFacingDir == 0);
    CHECK(camera.targetFacingDir == 7);
    CHECK(camera.visualX == startX);
    CHECK(camera.visualY == startY);

    dm1_v2_camera_tick(&camera, 39);
    CHECK(camera.facingDir == 0);
    CHECK(dm1_v2_camera_is_active(&camera));

    dm1_v2_camera_tick(&camera, 1);
    CHECK(camera.facingDir == 7);
    CHECK(dm1_v2_camera_is_active(&camera));

    dm1_v2_camera_tick(&camera, 40);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.facingDir == 7);
    CHECK(camera.visualX == startX);
    CHECK(camera.visualY == startY);
}

static void test_backward_command_tracks_new_logical_position(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;

    init_running(&rt, &camera);

    result = dm1_v2_movement_command_apply(&rt,
                                           &camera,
                                           DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD,
                                           1000U,
                                           32);

    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 5);
    CHECK(result.runtimeCommand == 2);
    CHECK(camera.logicalX == dm1_v2_get_x(&rt.player));
    CHECK(camera.logicalY == dm1_v2_get_y(&rt.player));
    CHECK(camera.targetX == dm1_v2_get_x(&rt.player));
    CHECK(camera.targetY == dm1_v2_get_y(&rt.player));

    dm1_v2_camera_tick(&camera, 32);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.visualX == camera.logicalX);
    CHECK(camera.visualY == camera.logicalY);
}

static void test_none_command_does_not_start_camera(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;

    init_running(&rt, &camera);

    result = dm1_v2_movement_command_apply(&rt,
                                           &camera,
                                           DM1_V2_MOVEMENT_COMMAND_NONE,
                                           1200U,
                                           64);

    CHECK(result.accepted == 0);
    CHECK(result.sourceCommand == 0);
    CHECK(result.runtimeCommand == 0);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.logicalX == dm1_v2_get_x(&rt.player));
    CHECK(camera.logicalY == dm1_v2_get_y(&rt.player));
}

static void test_camera_ticks_are_presentation_only(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;
    uint32_t tickCount;
    uint32_t lastTickMs;
    int lastCommand;
    int16_t x;
    int16_t y;
    int16_t facingDir;
    int viewportDirty;
    DM1_V2_ScrollState viewportScroll;

    init_running(&rt, &camera);

    result = dm1_v2_movement_command_apply(&rt,
                                           &camera,
                                           DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD,
                                           1300U,
                                           96);

    CHECK(result.accepted == 1);
    tickCount = rt.tickCount;
    lastTickMs = rt.lastTickMs;
    lastCommand = rt.lastCommand;
    x = dm1_v2_get_x(&rt.player);
    y = dm1_v2_get_y(&rt.player);
    facingDir = rt.player.facingDir;
    viewportDirty = rt.viewport.dirty;
    viewportScroll = rt.viewport.scroll;

    dm1_v2_camera_tick(&camera, 24);
    dm1_v2_camera_tick(&camera, 24);
    dm1_v2_camera_tick(&camera, 48);

    CHECK(rt.tickCount == tickCount);
    CHECK(rt.lastTickMs == lastTickMs);
    CHECK(rt.lastCommand == lastCommand);
    CHECK(dm1_v2_get_x(&rt.player) == x);
    CHECK(dm1_v2_get_y(&rt.player) == y);
    CHECK(rt.player.facingDir == facingDir);
    CHECK(rt.viewport.dirty == viewportDirty);
    CHECK(rt.viewport.scroll.scrollOffX == viewportScroll.scrollOffX);
    CHECK(rt.viewport.scroll.scrollOffY == viewportScroll.scrollOffY);
    CHECK(rt.viewport.scroll.scrollTargetX == viewportScroll.scrollTargetX);
    CHECK(rt.viewport.scroll.scrollTargetY == viewportScroll.scrollTargetY);
    CHECK(rt.viewport.scroll.scrollSpeed == viewportScroll.scrollSpeed);
    CHECK(rt.viewport.scroll.scrollProgress == viewportScroll.scrollProgress);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.visualX == camera.logicalX);
    CHECK(camera.visualY == camera.logicalY);
}

int main(void) {
    test_forward_command_starts_move_camera();
    test_turn_left_command_starts_turn_camera();
    test_backward_command_tracks_new_logical_position();
    test_none_command_does_not_start_camera();
    test_camera_ticks_are_presentation_only();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_movement_camera_pc34: ok");
    return 0;
}
