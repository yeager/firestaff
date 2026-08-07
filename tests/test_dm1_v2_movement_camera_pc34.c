/* Source movement remains discrete and the V2 camera only mirrors the
 * accepted PC34 tuple. */
#include "dm1_v2_movement_command_adapter_pc34.h"

#include <stdio.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { ++failures; fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); } } while (0)

static void init_running(DM1_V2_RuntimeState* rt, DM1_V2_CameraController* camera) {
    dm1_v2_runtime_init(rt);
    dm1_v2_runtime_start(rt, 1000U);
    dm1_v2_camera_init(camera, &rt->player);
}

static void test_cardinal_commands_mirror_immediately(void) {
    static const DM1_V2_MovementCommand commands[] = {
        DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD,
        DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD,
        DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT,
        DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT
    };
    size_t i;
    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        DM1_V2_RuntimeState rt;
        DM1_V2_CameraController camera;
        DM1_V2_MovementCommandResult result;
        init_running(&rt, &camera);
        result = dm1_v2_movement_command_apply(&rt, &camera, commands[i], 1000U, 96);
        CHECK(result.accepted == 1);
        CHECK(camera.logicalX == dm1_v2_get_x(&rt.player));
        CHECK(camera.logicalY == dm1_v2_get_y(&rt.player));
        CHECK(camera.visualX == camera.logicalX);
        CHECK(camera.visualY == camera.logicalY);
        CHECK(!dm1_v2_camera_is_active(&camera));
        CHECK(dm1_v2_camera_offset_x(&camera) == 0);
        CHECK(dm1_v2_camera_offset_y(&camera) == 0);
    }
}

static void test_turn_mirrors_accepted_direction(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;
    init_running(&rt, &camera);
    result = dm1_v2_movement_command_apply(&rt, &camera,
                                            DM1_V2_MOVEMENT_COMMAND_TURN_LEFT,
                                            1000U, 96);
    CHECK(result.accepted == 1);
    CHECK(camera.facingDir == rt.player.facingDir);
    CHECK(camera.fromFacingDir == rt.player.facingDir);
    CHECK(!camera.turning);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);
}

int main(void) {
    test_cardinal_commands_mirror_immediately();
    test_turn_mirrors_accepted_direction();
    if (failures) return 1;
    puts("dm1_v2_movement_camera_pc34: ok");
    return 0;
}
