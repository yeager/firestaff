#include "dm1_v2_movement_command_adapter_pc34.h"

#include <stdio.h>
#include <stdlib.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int test_source_ids_translate_to_runtime_ids(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;

    dm1_v2_runtime_init(&rt);
    dm1_v2_runtime_start(&rt, 1000);
    dm1_v2_camera_init(&camera, &rt.player);

    /* Source-lock: COMMAND.C dispatches C003 move-forward to movement and V2
     * maps that semantic command to the existing runtime forward command. */
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 1000, 64);
    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 3);
    CHECK(result.runtimeCommand == 1);
    CHECK(rt.lastCommand == 1);
    CHECK(dm1_v2_get_x(&rt.player) == 1);
    CHECK(dm1_v2_get_y(&rt.player) == 0);
    CHECK(camera.logicalX == dm1_v2_get_x(&rt.player));
    CHECK(camera.logicalY == dm1_v2_get_y(&rt.player));

    rt.player.moveState = 0;
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, 1000, 64);
    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 5);
    CHECK(result.runtimeCommand == 2);
    CHECK(rt.lastCommand == 2);
    CHECK(dm1_v2_get_x(&rt.player) == 0);
    CHECK(dm1_v2_get_y(&rt.player) == 0);
    CHECK(camera.logicalX == dm1_v2_get_x(&rt.player));
    CHECK(camera.logicalY == dm1_v2_get_y(&rt.player));

    return 0;
}

static int test_turns_start_presentation_only_camera_turns(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;

    dm1_v2_runtime_init(&rt);
    dm1_v2_runtime_start(&rt, 1000);
    dm1_v2_camera_init(&camera, &rt.player);

    /* Source-lock: COMMAND.C dispatches C001/C002 as turns; adapter preserves
     * those ids while runtime/camera use their own presentation split. */
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, 1100, 80);
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

    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, 1200, 80);
    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 2);
    CHECK(result.runtimeCommand == 4);
    CHECK(result.fromFacingDir == 7);
    CHECK(result.targetFacingDir == 0);
    CHECK(rt.player.facingDir == 0);
    CHECK(camera.turning == 1);
    CHECK(camera.fromFacingDir == 7);
    CHECK(camera.targetFacingDir == 0);

    return 0;
}

static int test_rejects_unsupported_or_stopped_commands(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;

    dm1_v2_runtime_init(&rt);
    dm1_v2_runtime_start(&rt, 1000);
    dm1_v2_camera_init(&camera, &rt.player);

    /* The ReDMCSB source has C004/C006 strafe commands, but the current V2
     * runtime shell has no discrete strafe primitive yet; do not fake it. */
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT, 1300, 40);
    CHECK(result.accepted == 0);
    CHECK(result.sourceCommand == 4);
    CHECK(result.runtimeCommand == 0);
    CHECK(rt.lastCommand == 0);

    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT, 1400, 40);
    CHECK(result.accepted == 0);
    CHECK(result.sourceCommand == 6);
    CHECK(result.runtimeCommand == 0);
    CHECK(rt.lastCommand == 0);

    dm1_v2_runtime_stop(&rt);
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 1500, 40);
    CHECK(result.accepted == 0);
    CHECK(result.runtimeCommand == 1);

    return 0;
}

int main(void) {
    if (test_source_ids_translate_to_runtime_ids()) return 1;
    if (test_turns_start_presentation_only_camera_turns()) return 1;
    if (test_rejects_unsupported_or_stopped_commands()) return 1;
    puts("dm1_v2_movement_command_adapter_pc34: ok");
    return 0;
}
