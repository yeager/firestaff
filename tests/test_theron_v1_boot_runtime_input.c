/*
 * test_theron_v1_boot_runtime_input.c — Theron V1 runtime input/idle facade
 *
 * Regression coverage for theron_v1_boot_runtime_handle_m12_input() and
 * theron_v1_boot_runtime_handle_idle_tick().  These facades own the M12
 * token-to-Theron-action mapping so M11 no longer calls turn/move/tick
 * directly in the Track 02 runtime path.
 *
 * Source references:
 *   THQUEST.ASM T520 — party placement / start position
 *   THQUEST.ASM T560 — dungeon loading
 *   THQUEST.ASM T600 — map transitions
 *   THQUEST.ASM T700 — tick world / per-tick updates
 *   ReDMCSB COMMAND.C F7015 — input dispatch
 *   ReDMCSB MOVESENS.C F0267/F0268 — square interaction
 */

#include "theron_v1_boot.h"
#include "theron_v1_world.h"
#include "menu_input_m12.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ── Test counters ─────────────────────────────────────────────────── */

static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_failures     = 0;

#define TEST(name) do {                                             \
    printf("  %-55s ", name);                                      \
    fflush(stdout);                                                 \
    g_tests_run++;                                                  \
} while (0)

#define PASS() do {                                                 \
    printf("PASS\n");                                               \
    g_tests_passed++;                                               \
} while (0)

#define FAIL(msg) do {                                              \
    printf("FAIL: %s\n", msg);                                      \
    g_failures++;                                                   \
} while (0)

#define ASSERT(cond, msg) do {                                      \
    if (!(cond)) { FAIL(msg); return 0; }                           \
} while (0)

/* ══════════════════════════════════════════════════════════════════════
 * Test helpers
 * ══════════════════════════════════════════════════════════════════════ */

static void copy_map_row_by_row(uint8_t dst[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE],
                                const uint8_t *src,
                                int width,
                                int height)
{
    int y;
    for (y = 0; y < height; ++y) {
        memcpy(dst[y], &src[y * width], (size_t)width);
    }
}

static void setup_open_room(Theron_V1_World *world)
{
    static const uint8_t map[8 * 8] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    };
    theron_v1_world_init(world);
    world->current_dungeon = 1;
    world->current_level   = 0;
    world->level_loaded[0][0] = 1;
    world->levels[0][0].width  = 8;
    world->levels[0][0].height = 8;
    world->levels[0][0].start_x = 3;
    world->levels[0][0].start_y = 3;
    world->levels[0][0].start_dir = THERON_DIR_NORTH;
    copy_map_row_by_row(world->levels[0][0].squares, map, 8, 8);
    world->party.leader_x = 3;
    world->party.leader_y = 3;
    world->party.leader_dir = THERON_DIR_NORTH;
    world->world_tick = 10;
}

static void setup_room_with_exit(Theron_V1_World *world)
{
    static uint8_t map[8 * 8] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    };
    /* Exit square one step north of start. */
    map[2 * 8 + 3] = THERON_SQUARE_EXIT;
    theron_v1_world_init(world);
    world->current_dungeon = 1;
    world->current_level   = 0;
    world->level_loaded[0][0] = 1;
    world->levels[0][0].width  = 8;
    world->levels[0][0].height = 8;
    world->levels[0][0].start_x = 3;
    world->levels[0][0].start_y = 3;
    world->levels[0][0].start_dir = THERON_DIR_NORTH;
    copy_map_row_by_row(world->levels[0][0].squares, map, 8, 8);
    world->party.leader_x = 3;
    world->party.leader_y = 3;
    world->party.leader_dir = THERON_DIR_NORTH;
    world->world_tick = 10;
    world->dungeon_complete = 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Receipt init
 * ══════════════════════════════════════════════════════════════════════ */

static int test_receipt_init(void)
{
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("Receipt init zeros result and sets defaults");
    theron_v1_boot_runtime_input_receipt_init(&receipt);
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED,
           "init result should be IGNORED");
    ASSERT(receipt.handled == 0, "init handled should be 0");
    ASSERT(receipt.party_x == -1, "init party_x should be -1");
    ASSERT(receipt.party_y == -1, "init party_y should be -1");
    ASSERT(receipt.party_dir == -1, "init party_dir should be -1");
    ASSERT(receipt.tick_count == -1, "init tick_count should be -1");
    ASSERT(receipt.status_scope != NULL, "init scope should be set");
    ASSERT(receipt.status != NULL, "init status should be set");
    PASS();
    return 1;
}

static int test_handle_null(void)
{
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("Null world returns 0");
    theron_v1_boot_runtime_input_receipt_init(&receipt);
    ASSERT(theron_v1_boot_runtime_handle_m12_input(NULL, NULL,
            M12_MENU_INPUT_UP, &receipt) == 0,
           "null world should return 0");
    ASSERT(theron_v1_boot_runtime_handle_m12_input(NULL, NULL,
            M12_MENU_INPUT_UP, NULL) == 0,
           "null receipt should return 0");
    PASS();
    return 1;
}

static int test_unknown_input(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("Unknown input token is ignored and preserves pose");
    setup_open_room(&world);
    theron_v1_boot_runtime_input_receipt_init(&receipt);
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            9999, &receipt) == 1,
           "unknown input should fill receipt");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED,
           "unknown input should be ignored");
    ASSERT(receipt.party_x == 3, "x should be preserved");
    ASSERT(receipt.party_y == 3, "y should be preserved");
    ASSERT(receipt.party_dir == THERON_DIR_NORTH, "dir should be preserved");
    ASSERT(receipt.tick_count == 10, "tick should be preserved");
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Turn inputs
 * ══════════════════════════════════════════════════════════════════════ */

static int test_turn_left(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("TURN_LEFT rotates party left and requests redraw");
    setup_open_room(&world);
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_TURN_LEFT, &receipt) == 1,
           "turn left should succeed");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW,
           "turn left should redraw");
    ASSERT(receipt.turned == 1, "turned flag should be set");
    ASSERT(receipt.moved == 0, "moved flag should be clear");
    ASSERT(receipt.party_dir == THERON_DIR_WEST,
           "should now face west");
    ASSERT(strcmp(receipt.status_scope, "TURN") == 0, "scope should be TURN");
    ASSERT(strcmp(receipt.status, "LEFT") == 0, "status should be LEFT");
    PASS();
    return 1;
}

static int test_turn_right(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("TURN_RIGHT rotates party right and requests redraw");
    setup_open_room(&world);
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_TURN_RIGHT, &receipt) == 1,
           "turn right should succeed");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW,
           "turn right should redraw");
    ASSERT(receipt.turned == 1, "turned flag should be set");
    ASSERT(receipt.party_dir == THERON_DIR_EAST,
           "should now face east");
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Strafe rejection
 * ══════════════════════════════════════════════════════════════════════ */

static int test_strafe_rejected(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("Strafe and legacy LEFT/RIGHT tokens are ignored");
    setup_open_room(&world);
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_LEFT, &receipt) == 1,
           "LEFT should fill receipt");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED,
           "LEFT should be ignored");
    ASSERT(strcmp(receipt.status, "THERON HAS NO STRAFE") == 0,
           "status should note no strafe");

    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_STRAFE_LEFT, &receipt) == 1,
           "STRAFE_LEFT should fill receipt");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED,
           "STRAFE_LEFT should be ignored");
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Movement
 * ══════════════════════════════════════════════════════════════════════ */

static int test_move_forward(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("UP moves party forward one square");
    setup_open_room(&world);
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_UP, &receipt) == 1,
           "UP should succeed");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW,
           "UP should redraw");
    ASSERT(receipt.moved == 1, "moved flag should be set");
    ASSERT(receipt.party_y == 2, "should step north to y=2");
    ASSERT(receipt.party_x == 3, "x should stay 3");
    ASSERT(strcmp(receipt.status, "THERON ADVANCED") == 0,
           "status should be advanced");
    PASS();
    return 1;
}

static int test_move_backward(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("DOWN moves party backward one square");
    setup_open_room(&world);
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_DOWN, &receipt) == 1,
           "DOWN should succeed");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW,
           "DOWN should redraw");
    ASSERT(receipt.moved == 1, "moved flag should be set");
    ASSERT(receipt.party_y == 4, "should step south to y=4");
    ASSERT(receipt.party_x == 3, "x should stay 3");
    ASSERT(receipt.party_dir == THERON_DIR_NORTH,
           "facing should be restored to north");
    ASSERT(strcmp(receipt.status, "THERON STEPPED BACK") == 0,
           "status should be stepped back");
    PASS();
    return 1;
}

static int test_move_blocked(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("UP into a wall is ignored with BLOCKED status");
    setup_open_room(&world);
    /* Put party one step south of the north wall, facing north. */
    world.party.leader_y = 1;
    world.party.leader_dir = THERON_DIR_NORTH;
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_UP, &receipt) == 1,
           "blocked UP should fill receipt");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED,
           "blocked move should be ignored");
    ASSERT(receipt.blocked == 1, "blocked flag should be set");
    ASSERT(receipt.moved == 0, "moved flag should be clear");
    ASSERT(strcmp(receipt.status, "BLOCKED") == 0,
           "status should be BLOCKED");
    PASS();
    return 1;
}

static int test_exit_dungeon(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("UP onto exit square emits EXIT_DUNGEON receipt");
    setup_room_with_exit(&world);
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_UP, &receipt) == 1,
           "exit UP should succeed");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_EXIT_DUNGEON,
           "result should be EXIT_DUNGEON");
    ASSERT(receipt.exited == 1, "exited flag should be set");
    ASSERT(receipt.exit_receipt.result == THERON_STARTUP_OK,
           "exit receipt result should be OK");
    ASSERT(receipt.exit_receipt.host_receipt.input_result ==
               THERON_STARTUP_INPUT_RESULT_REDRAW,
           "exit receipt host input result should be REDRAW");
    ASSERT(strcmp(receipt.status_scope, "MOVE") == 0,
           "scope should be MOVE");
    ASSERT(strcmp(receipt.status, "EXIT DUNGEON") == 0,
           "status should be EXIT DUNGEON");
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Wait / idle tick
 * ══════════════════════════════════════════════════════════════════════ */

static int test_wait_tick(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("ACCEPT and ACTION both tick world and request redraw");
    setup_open_room(&world);
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_ACCEPT, &receipt) == 1,
           "ACCEPT should succeed");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW,
           "ACCEPT should redraw");
    ASSERT(receipt.waited == 1, "waited flag should be set");
    ASSERT(receipt.tick_count == 11, "tick should increment");

    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_ACTION, &receipt) == 1,
           "ACTION should succeed");
    ASSERT(receipt.tick_count == 12, "tick should increment again");
    PASS();
    return 1;
}

static int test_idle_tick(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("Idle tick facade increments tick and requests redraw");
    setup_open_room(&world);
    ASSERT(theron_v1_boot_runtime_handle_idle_tick(&world, &receipt) == 1,
           "idle tick should succeed");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW,
           "idle tick should redraw");
    ASSERT(receipt.waited == 1, "waited flag should be set");
    ASSERT(receipt.tick_count == 11, "tick should increment");
    ASSERT(strcmp(receipt.status, "THERON TICK") == 0,
           "status should be THERON TICK");
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Main
 * ══════════════════════════════════════════════════════════════════════ */

int main(void)
{
    printf("=== Theron V1 Boot Runtime Input Facade Tests ===\n\n");

    test_receipt_init();
    test_handle_null();
    test_unknown_input();
    test_turn_left();
    test_turn_right();
    test_strafe_rejected();
    test_move_forward();
    test_move_backward();
    test_move_blocked();
    test_exit_dungeon();
    test_wait_tick();
    test_idle_tick();

    printf("\n=====================================================\n");
    printf("Results: %d/%d passed  (%s)\n",
           g_tests_passed, g_tests_run,
           g_failures == 0 ? "all passed" : "FAILURES");
    printf("=====================================================\n");
    return g_failures == 0 ? 0 : 1;
}
