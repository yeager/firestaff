#include "dm1_v2_camera_controller_pc34.h"

#include <stdio.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { failures++; fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); } } while (0)

static void test_turns_are_immediate_for_all_durations(void) {
    static const int durations[] = { -50, 0, 1, 2, 100 };
    size_t i;
    for (i = 0; i < sizeof(durations) / sizeof(durations[0]); ++i) {
        DM1_V2_CameraController camera;
        dm1_v2_camera_init(&camera, NULL);
        dm1_v2_camera_begin_turn(&camera, 0, 3, durations[i]);
        CHECK(camera.facingDir == 3);
        CHECK(camera.fromFacingDir == 3);
        CHECK(camera.targetFacingDir == 3);
        CHECK(camera.durationMs == 0);
        CHECK(!dm1_v2_camera_is_active(&camera));
        dm1_v2_camera_tick(&camera, 100);
        CHECK(camera.facingDir == 3);
        CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);
        CHECK(dm1_v2_camera_turn_pan_offset_y(&camera) == 0);
    }
}

static void test_move_and_pan_remain_source_tuple_only(void) {
    DM1_V2_PlayerPos player;
    DM1_V2_CameraController camera;
    dm1_v2_pos_init(&player, 50, 60, 2);
    dm1_v2_camera_init(&camera, &player);
    player.xPixel = 51 * DM1_V2_SUBPIXEL_SCALE;
    dm1_v2_camera_begin_move(&camera, &player, 96);
    CHECK(camera.visualX == camera.logicalX);
    CHECK(camera.visualY == camera.logicalY);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(dm1_v2_camera_offset_x(&camera) == 0);
    CHECK(dm1_v2_camera_offset_y(&camera) == 0);
    CHECK(dm1_v2_camera_horizontal_pan_offset(&camera) == 0);
}

int main(void) {
    test_turns_are_immediate_for_all_durations();
    test_move_and_pan_remain_source_tuple_only();
    if (failures) return 1;
    puts("dm1_v2_camera_turn_edge_cases_pc34: ok");
    return 0;
}
