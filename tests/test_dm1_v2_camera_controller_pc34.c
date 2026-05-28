#include "dm1_v2_camera_controller_pc34.h"
#include <stdio.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); failures++; } } while (0)

int main(void) {
    DM1_V2_PlayerPos player;
    DM1_V2_MoveParams params = {256, 1, 256, 0, 0};
    DM1_V2_CameraController camera;

    dm1_v2_pos_init(&player, 1, 2, 0);
    dm1_v2_camera_init(&camera, &player);
    CHECK(camera.logicalX == 256);
    CHECK(camera.logicalY == 512);
    CHECK(!dm1_v2_camera_is_active(&camera));

    /* The camera controller follows an already accepted logical tile move.
     * Keep this test focused on presentation interpolation rather than the
     * lower-level movement engine's subpixel stepping. */
    (void)params;
    player.xPixel = 2 * DM1_V2_SUBPIXEL_SCALE;
    player.yPixel = 2 * DM1_V2_SUBPIXEL_SCALE;
    dm1_v2_camera_begin_move(&camera, &player, 1000);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(camera.targetX == 512);
    CHECK(dm1_v2_camera_offset_x(&camera) == -256);
    dm1_v2_camera_tick(&camera, 500);
    CHECK(camera.visualX == 384);
    CHECK(dm1_v2_camera_offset_x(&camera) == -128);
    dm1_v2_camera_tick(&camera, 500);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.visualX == camera.logicalX);
    CHECK(dm1_v2_camera_offset_x(&camera) == 0);

    dm1_v2_camera_begin_turn(&camera, 0, 7, 1000);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(dm1_v2_camera_interpolated_facing(&camera) == 0);
    dm1_v2_camera_tick(&camera, 500);
    CHECK(dm1_v2_camera_interpolated_facing(&camera) == 7);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);
    dm1_v2_camera_tick(&camera, 500);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.facingDir == 7);

    dm1_v2_camera_begin_turn_pan(&camera, 0, 1, 1000);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);
    dm1_v2_camera_tick(&camera, 250);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 128);
    dm1_v2_camera_tick(&camera, 250);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 256);
    dm1_v2_camera_tick(&camera, 250);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == -128);
    dm1_v2_camera_tick(&camera, 250);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_camera_controller_pc34: ok");
    return 0;
}
