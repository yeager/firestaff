#include "dm1_v2_camera_controller_pc34.h"

#include <stdio.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { \
    failures++; \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
} } while (0)

/* 1. duration clamps to minimum of 1ms */
static void test_duration_clamp(void) {
    DM1_V2_CameraController camera;
    dm1_v2_camera_init(&camera, NULL);
    CHECK(camera.facingDir == 0);
    CHECK(camera.elapsedMs == 0);
    CHECK(camera.durationMs == 0);

    /* zero duration becomes 1 */
    dm1_v2_camera_begin_turn(&camera, 0, 1, 0);
    CHECK(camera.durationMs == 1);
    CHECK(camera.facingDir == 0);
    CHECK(camera.fromFacingDir == 0);
    CHECK(camera.targetFacingDir == 1);

    /* incomplete: in first half, offset is 0 */
    CHECK(dm1_v2_camera_offset_x(&camera) == 0);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);

    /* tick once -> complete */
    dm1_v2_camera_tick(&camera, 1);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.facingDir == 1);
}

/* 2. dt=0 does not advance elapsed */
static void test_zero_dt_does_not_advance(void) {
    DM1_V2_CameraController camera;
    dm1_v2_camera_init(&camera, NULL);
    dm1_v2_camera_begin_turn(&camera, 0, 3, 10);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(camera.elapsedMs == 0);

    dm1_v2_camera_tick(&camera, 0);
    CHECK(camera.elapsedMs == 0);
    CHECK(dm1_v2_camera_is_active(&camera));
}

/* 3. negative dt clamped to 0 */
static void test_negative_dt_clamped(void) {
    DM1_V2_CameraController camera;
    dm1_v2_camera_init(&camera, NULL);
    dm1_v2_camera_begin_turn(&camera, 2, 3, 50);
    CHECK(camera.elapsedMs == 0);

    dm1_v2_camera_tick(&camera, -100);
    CHECK(camera.elapsedMs == 0);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(camera.facingDir == 2);
}

/* 4. midpoint threshold: elapsedMs * 2 < duration is the first half */
static void test_midpoint_threshold(void) {
    DM1_V2_CameraController camera;
    dm1_v2_camera_init(&camera, NULL);
    dm1_v2_camera_begin_turn(&camera, 0, 3, 10);
    CHECK(dm1_v2_camera_is_active(&camera));

    /* t=0: first half (0*2=0 < 10) -> fromFacingDir */
    CHECK(camera.facingDir == 0);

    /* t=4: 4*2=8 < 10, first half */
    dm1_v2_camera_tick(&camera, 4);
    CHECK(camera.facingDir == 0);

    /* t=5: 5*2=10, NOT < 10, second half -> lerp/facingDir */
    dm1_v2_camera_tick(&camera, 1);
    /* lerp: from=0, to=3, elapsed=5.
     * delta = 3-0 = 3. 0 + (3*5)/10 = 0 + 1 = 1.
     * But wait, dm1_v2_camera_interpolated_facing has its own logic:
     *   if (elapsed * 2 < duration) return from... 
     *   return (from + delta) & 7
     * So it's not lerp, it's a step at the midpoint!
     * (0 + 3) & 7 = 3
     * Let me just check what it returns. */
    CHECK(dm1_v2_camera_interpolated_facing(&camera) == 3);

    /* tick again to complete */
    dm1_v2_camera_tick(&camera, 100);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.facingDir == 3);
}

/* 5. opposite direction: 0->4 shortest path is +4 (not -4) */
static void test_opposite_delta_4(void) {
    DM1_V2_CameraController camera;
    dm1_v2_camera_init(&camera, NULL);
    dm1_v2_camera_begin_turn(&camera, 0, 4, 8);
    CHECK(dm1_v2_camera_is_active(&camera));

    /* half=4: 4*2=8 NOT < 8, second half. delta=4, 0+4=4 */
    dm1_v2_camera_tick(&camera, 4);
    CHECK(camera.facingDir == 4);

    dm1_v2_camera_tick(&camera, 4);  /* complete */
    CHECK(camera.facingDir == 4);
}

/* 6. shortest path 7->0 is +1 (not -7) */
static void test_wraparound_7_to_0(void) {
    DM1_V2_CameraController camera;
    dm1_v2_camera_init(&camera, NULL);
    dm1_v2_camera_begin_turn(&camera, 7, 0, 100);
    CHECK(camera.facingDir == 7);

    dm1_v2_camera_tick(&camera, 50);  /* 50*2 = 100 -> second half */
    /* delta = 0-7 = -7. -7 < -4 -> delta = 1. (7+1) & 7 = 0 */
    CHECK(camera.facingDir == 0);
}

/* 7. begin_move with unchanged position is inactive */
static void test_begin_move_same_position(void) {
    DM1_V2_CameraController camera;
    DM1_V2_PlayerPos player;
    dm1_v2_pos_init(&player, 50, 60, 2);
    dm1_v2_camera_init(&camera, &player);

    dm1_v2_camera_begin_move(&camera, &player, 500);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.visualX == camera.targetX);
    /* lerp from == to means delta = 0, so offset stays 0 */
}

/* 8. inactive tick is a no-op */
static void test_early_tick_rejects_inactive(void) {
    DM1_V2_CameraController camera;
    dm1_v2_camera_init(&camera, NULL);
    CHECK(camera.elapsedMs == 0);

    /* inactive tick silently returns */
    dm1_v2_camera_tick(&camera, 500);
    CHECK(camera.elapsedMs == 0);
}

/* 9. 2ms duration: t=0 first half, t=1 second half, t=2 complete */
static void test_2ms_duration_steps(void) {
    DM1_V2_CameraController camera;
    dm1_v2_camera_init(&camera, NULL);
    dm1_v2_camera_begin_turn(&camera, 0, 1, 2);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(camera.durationMs == 2);

    /* first tick: elapsed=1. 1*2=2 NOT < 2. second half */
    /* delta=1. (0+1)&7=1 */
    dm1_v2_camera_tick(&camera, 1);
    CHECK(camera.facingDir == 1);

    dm1_v2_camera_tick(&camera, 99);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.facingDir == 1);
}

/* 10. negative duration clamped to 1 */
static void test_negative_duration(void) {
    DM1_V2_CameraController camera;
    dm1_v2_camera_init(&camera, NULL);
    dm1_v2_camera_begin_turn(&camera, 0, 3, -50);
    CHECK(camera.durationMs == 1);
    CHECK(dm1_v2_camera_is_active(&camera));

    dm1_v2_camera_tick(&camera, 1);
    CHECK(!dm1_v2_camera_is_active(&camera));
}

/* 11. optional turn pan is presentation-only and follows source cardinal turn side */
static void test_optional_turn_pan_direction(void) {
    DM1_V2_CameraController camera;
    dm1_v2_camera_init(&camera, NULL);

    dm1_v2_camera_begin_turn_pan(&camera, 0, 3, 100);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);

    dm1_v2_camera_tick(&camera, 25);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == -128);
    CHECK(camera.logicalX == 0);
    CHECK(camera.logicalY == 0);

    dm1_v2_camera_tick(&camera, 25);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == -256);
    CHECK(camera.facingDir == 3);

    dm1_v2_camera_tick(&camera, 25);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 128);

    dm1_v2_camera_tick(&camera, 25);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);
}

int main(void) {
    test_duration_clamp();
    test_zero_dt_does_not_advance();
    test_negative_dt_clamped();
    test_midpoint_threshold();
    test_opposite_delta_4();
    test_wraparound_7_to_0();
    test_begin_move_same_position();
    test_early_tick_rejects_inactive();
    test_2ms_duration_steps();
    test_negative_duration();
    test_optional_turn_pan_direction();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_camera_turn_edge_cases_pc34: ok");
    return 0;
}
