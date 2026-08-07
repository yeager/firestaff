#include "dm1_v2_camera_controller_pc34.h"

#include <stdio.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); failures++; } } while (0)

int main(void) {
    DM1_V2_PlayerPos player;
    DM1_V2_CameraController camera;

    dm1_v2_pos_init(&player, 1, 2, 0);
    dm1_v2_camera_init(&camera, &player);
    CHECK(camera.logicalX == 256);
    CHECK(camera.logicalY == 512);
    CHECK(!dm1_v2_camera_is_active(&camera));

    player.xPixel = 2 * DM1_V2_SUBPIXEL_SCALE;
    dm1_v2_camera_begin_move(&camera, &player, 1000);
    CHECK(camera.logicalX == 512);
    CHECK(camera.visualX == 512);
    CHECK(camera.targetX == 512);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(dm1_v2_camera_offset_x(&camera) == 0);
    CHECK(dm1_v2_camera_offset_y(&camera) == 0);
    dm1_v2_camera_tick(&camera, 500);
    CHECK(camera.visualX == camera.logicalX);

    dm1_v2_camera_begin_turn(&camera, 0, 7, 1000);
    CHECK(camera.facingDir == 7);
    CHECK(camera.fromFacingDir == 7);
    CHECK(camera.targetFacingDir == 7);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(dm1_v2_camera_interpolated_facing(&camera) == 7);

    dm1_v2_camera_begin_turn_pan(&camera, 0, 1, 1000);
    CHECK(camera.facingDir == 1);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);
    CHECK(dm1_v2_camera_turn_pan_offset_y(&camera) == 0);
    CHECK(dm1_v2_camera_horizontal_pan_offset(&camera) == 0);

    if (failures) return 1;
    puts("dm1_v2_camera_controller_pc34: ok");
    return 0;
}
