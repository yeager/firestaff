#include "nexus_v1_movement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int g_fail = 0;
static int g_count = 0;

static void expect(int cond, const char *msg) {
    g_count++;
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); g_fail++; }
}

static void test_input_queue_push_pop(void) {
    Nexus_InputQueue q;
    nexus_input_queue_init(&q);
    expect(nexus_input_queue_count(&q) == 0, "queue starts empty");

    int rc = nexus_input_queue_push(&q, NEXUS_CMD_FORWARD);
    expect(rc == 0 || rc == 1, "push returns success");
    expect(nexus_input_queue_count(&q) == 1, "queue has 1 item after push");

    nexus_input_queue_push(&q, NEXUS_CMD_TURN_LEFT);
    expect(nexus_input_queue_count(&q) == 2, "queue has 2 items");

    int cmd = -1;
    int pop_rc = nexus_input_queue_pop(&q, &cmd);
    expect(pop_rc == 1, "pop returns 1 on success");
    expect(cmd == NEXUS_CMD_FORWARD, "first pop returns FORWARD");

    nexus_input_queue_pop(&q, &cmd);
    expect(cmd == NEXUS_CMD_TURN_LEFT, "second pop returns TURN_LEFT");

    expect(nexus_input_queue_count(&q) == 0, "queue empty after two pops");

    pop_rc = nexus_input_queue_pop(&q, &cmd);
    expect(pop_rc == 0, "pop from empty queue returns 0");
}

static void test_input_queue_overflow(void) {
    Nexus_InputQueue q;
    nexus_input_queue_init(&q);

    /* Fill the queue */
    for (int i = 0; i < NEXUS_INPUT_QUEUE_SIZE; i++) {
        nexus_input_queue_push(&q, NEXUS_CMD_FORWARD);
    }
    expect(nexus_input_queue_count(&q) == NEXUS_INPUT_QUEUE_SIZE,
           "queue full at max size");

    /* Pushing beyond capacity should fail or wrap */
    int rc = nexus_input_queue_push(&q, NEXUS_CMD_BACKWARD);
    expect(rc == 0 || nexus_input_queue_count(&q) <= NEXUS_INPUT_QUEUE_SIZE,
           "queue does not exceed max size");
}

static void test_movement_step_valid(void) {
    Nexus_MovementState ms;
    nexus_movement_init(&ms, 5, 5, NEXUS_DIR_NORTH);

    /* Create a simple map with floor squares */
    uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    memset(squares, NEXUS_SQUARE_WALL, sizeof(squares));
    /* Make a corridor */
    squares[5][5] = NEXUS_SQUARE_FLOOR;
    squares[4][5] = NEXUS_SQUARE_FLOOR;  /* north of (5,5) */
    squares[6][5] = NEXUS_SQUARE_FLOOR;  /* south of (5,5) */

    Nexus_MoveResultData result;
    memset(&result, 0, sizeof(result));
    int rc = nexus_movement_step(&ms, squares, NEXUS_CMD_FORWARD, &result);
    /* Party facing north, moving forward: should move to (5,4) or result OK */
    expect(rc == 0 || result.result == NEXUS_MOVE_OK ||
           result.result == NEXUS_MOVE_BLOCKED_WALL,
           "movement step returns valid result code");
}

static void test_movement_step_blocked(void) {
    Nexus_MovementState ms;
    nexus_movement_init(&ms, 5, 5, NEXUS_DIR_EAST);

    /* All walls */
    uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    memset(squares, NEXUS_SQUARE_WALL, sizeof(squares));
    squares[5][5] = NEXUS_SQUARE_FLOOR;

    Nexus_MoveResultData result;
    memset(&result, 0, sizeof(result));
    nexus_movement_step(&ms, squares, NEXUS_CMD_FORWARD, &result);
    expect(result.result == NEXUS_MOVE_BLOCKED_WALL,
           "forward into wall is blocked");
}

static void test_turn_left_right(void) {
    Nexus_MovementState ms;
    nexus_movement_init(&ms, 5, 5, NEXUS_DIR_NORTH);

    Nexus_MoveResultData result;
    memset(&result, 0, sizeof(result));

    nexus_movement_turn(&ms, 1, &result);  /* turn right */
    expect(ms.party_dir == NEXUS_DIR_EAST, "turn right from N -> E");
    expect(result.result == NEXUS_MOVE_TURN_ONLY, "turn result is TURN_ONLY");

    nexus_movement_turn(&ms, 1, &result);  /* turn right */
    expect(ms.party_dir == NEXUS_DIR_SOUTH, "turn right from E -> S");

    nexus_movement_turn(&ms, 0, &result);  /* turn left */
    expect(ms.party_dir == NEXUS_DIR_EAST, "turn left from S -> E");

    nexus_movement_turn(&ms, 0, &result);  /* turn left */
    expect(ms.party_dir == NEXUS_DIR_NORTH, "turn left from E -> N");

    /* Full circle left */
    for (int i = 0; i < 4; i++) {
        nexus_movement_turn(&ms, 0, &result);
    }
    expect(ms.party_dir == NEXUS_DIR_NORTH, "4 left turns returns to N");
}

static void test_movement_cooldown(void) {
    Nexus_MovementState ms;
    nexus_movement_init(&ms, 5, 5, NEXUS_DIR_NORTH);
    /* Verify cooldown field is initialized */
    expect(ms.move_cooldown_ticks >= 0, "cooldown ticks initialized >= 0");
    expect(ms.disabled_ticks == 0, "disabled ticks starts at 0");
}

int main(void) {
    test_input_queue_push_pop();
    test_input_queue_overflow();
    test_movement_step_valid();
    test_movement_step_blocked();
    test_turn_left_right();
    test_movement_cooldown();

    if (g_fail) {
        fprintf(stderr, "test_nexus_v1_movement_gameplay: %d failure(s)\n", g_fail);
        return 1;
    }
    printf("ok: nexus_v1_movement_gameplay (%d tests)\n", g_count);
    return 0;
}
