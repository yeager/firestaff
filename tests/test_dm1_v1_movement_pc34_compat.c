#include "dm1_v1_movement_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void)
{
    DM1_V1_MovementState state;
    memset(&state, 0xFF, sizeof(state));
    dm1v1_movement_init(&state, 5, 10, DM1_V1_DIR_NORTH);
    assert(state.pos_x == 5);
    assert(state.pos_y == 10);
    assert(state.facing == DM1_V1_DIR_NORTH);
    assert(state.phase == DM1_V1_PHASE_IDLE);
    assert(state.step_timer == 0);
}

static void test_set_step_cost(void)
{
    DM1_V1_MovementState state;
    dm1v1_movement_init(&state, 0, 0, 0);
    dm1v1_movement_set_step_cost(&state, 3);
    assert(state.step_cost == 3);
}

static void test_command_queue_init(void)
{
    DM1_V1_CommandQueue queue;
    memset(&queue, 0xFF, sizeof(queue));
    dm1v1_command_queue_init(&queue);
    assert(queue.first_index == 0);
    assert(queue.last_index == DM1_V1_COMMAND_QUEUE_CAPACITY);
}

static void test_poll_input_enqueue(void)
{
    DM1_V1_CommandQueue queue;
    dm1v1_command_queue_init(&queue);
    int rc = dm1v1_movement_poll_input(&queue, 3);
    (void)rc;
    assert(rc == 1);
}

static void test_execute_step_empty_queue(void)
{
    DM1_V1_MovementState state;
    DM1_V1_CommandQueue queue;
    dm1v1_movement_init(&state, 5, 5, DM1_V1_DIR_NORTH);
    dm1v1_command_queue_init(&queue);
    int16_t cmd = dm1v1_movement_execute_step(&state, &queue);
    (void)cmd;
    assert(cmd == 0);
}

static void test_is_in_movement_idle(void)
{
    DM1_V1_MovementState state;
    dm1v1_movement_init(&state, 0, 0, 0);
    int rc = dm1v1_movement_is_in_movement(&state);
    (void)rc;
    assert(rc == 0);
}

static void test_process_vblank_idle(void)
{
    DM1_V1_MovementState state;
    dm1v1_movement_init(&state, 0, 0, 0);
    dm1v1_movement_process_vertical_blank(&state);
    assert(state.phase == DM1_V1_PHASE_IDLE);
}

static void test_command_for_menu_code(void)
{
    int cmd = DM1_V1_Movement_CommandForFirestaffMenuCodePc34Compat(0);
    (void)cmd;
    assert(cmd >= 0);
}

static void test_orchestrator_route_plan_null(void)
{
    int rc = DM1_V1_Movement_OrchestratorRoutePlanPc34Compat(
        3, 0, 0, 0, 0, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_orchestrator_route_plan_turn(void)
{
    DM1_V1_MovementOrchestratorRoutePlanPc34Compat plan;
    memset(&plan, 0, sizeof(plan));
    int rc = DM1_V1_Movement_OrchestratorRoutePlanPc34Compat(
        1, 0, 0, 0, 0, &plan);
    (void)rc;
    assert(rc == 1);
    assert(plan.valid == 1);
}

int main(void)
{
    test_init();
    test_set_step_cost();
    test_command_queue_init();
    test_poll_input_enqueue();
    test_execute_step_empty_queue();
    test_is_in_movement_idle();
    test_process_vblank_idle();
    test_command_for_menu_code();
    test_orchestrator_route_plan_null();
    test_orchestrator_route_plan_turn();

    puts("ok: DM1 movement (Q-DM1-08) 10 tests passed");
    return 0;
}
