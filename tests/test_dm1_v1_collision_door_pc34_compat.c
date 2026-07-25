#include "dm1_v1_collision_door_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_check_step_null(void)
{
    struct Dm1V1CollisionResult result;
    memset(&result, 0, sizeof(result));
    int rc = DM1_V1_Collision_CheckStep(NULL, NULL, NULL, 3, &result);
    (void)rc;
    assert(rc == 0);
}

static void test_check_step_null_result(void)
{
    int rc = DM1_V1_Collision_CheckStep(NULL, NULL, NULL, 3, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_query_square_null(void)
{
    struct Dm1V1CollisionResult result;
    memset(&result, 0, sizeof(result));
    int rc = DM1_V1_Collision_QuerySquare(NULL, 0, 0, 0, &result);
    (void)rc;
    assert(rc == 0);
}

static void test_detect_pit_null(void)
{
    struct Dm1V1PitResult result;
    memset(&result, 0, sizeof(result));
    int rc = DM1_V1_Collision_DetectPit(NULL, 0, 0, 0, &result);
    (void)rc;
    assert(rc == 0);
}

static void test_door_process_click_null(void)
{
    struct Dm1V1DoorInteractionResult result;
    memset(&result, 0, sizeof(result));
    int rc = DM1_V1_Door_ProcessClick(NULL, NULL, NULL, &result);
    (void)rc;
    assert(rc == 0);
}

static void test_door_apply_toggle_null(void)
{
    int rc = DM1_V1_Door_ApplyToggle(NULL, 0, 0, 0, DM1_DOOR_STATE_OPEN);
    (void)rc;
    assert(rc == 0);
}

static void test_door_is_passable_null(void)
{
    int rc = DM1_V1_Door_IsPassable(NULL, 0, 0, 0);
    (void)rc;
    assert(rc == -1);
}

static void test_door_state_constants(void)
{
    assert(DM1_DOOR_STATE_OPEN == 0);
    assert(DM1_DOOR_STATE_CLOSED == 4);
    assert(DM1_DOOR_STATE_DESTROYED == 5);
    assert(DM1_DOOR_STATE_CLOSED_ONE_FOURTH == 1);
}

static void test_collision_result_codes(void)
{
    assert(DM1_COLLISION_PASSABLE == 0);
    assert(DM1_COLLISION_BLOCKED_WALL == 1);
    assert(DM1_COLLISION_BLOCKED_DOOR == 2);
    assert(DM1_COLLISION_BLOCKED_FAKEWALL == 3);
    assert(DM1_COLLISION_BLOCKED_BOUNDS == 4);
    assert(DM1_COLLISION_BLOCKED_GROUP == 5);
}

static void test_pit_constants(void)
{
    assert(DM1_PIT_MASK_OPEN == 0x08);
    assert(DM1_PIT_MASK_IMAGINARY == 0x01);
    assert(DM1_PIT_NONE == 0);
    assert(DM1_PIT_OPEN_REAL == 1);
    assert(DM1_PIT_OPEN_IMAGINARY == 2);
    assert(DM1_PIT_CLOSED == 3);
}

static void test_source_evidence(void)
{
    const char *ev = DM1_V1_CollisionDoor_SourceEvidence();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_tile_index(void)
{
    int idx = dm1_tile_index(3, 5, 16);
    (void)idx;
    assert(idx == 3 * 16 + 5);
}

int main(void)
{
    test_check_step_null();
    test_check_step_null_result();
    test_query_square_null();
    test_detect_pit_null();
    test_door_process_click_null();
    test_door_apply_toggle_null();
    test_door_is_passable_null();
    test_door_state_constants();
    test_collision_result_codes();
    test_pit_constants();
    test_source_evidence();
    test_tile_index();

    puts("ok: DM1 collision/door (Q-DM1-04) 12 tests passed");
    return 0;
}
