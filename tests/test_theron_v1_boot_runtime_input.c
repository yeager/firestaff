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
    ASSERT(receipt.active_champion_slot == -1,
           "init active champion should be unavailable");
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
 * Original turn and lateral movement commands
 * ══════════════════════════════════════════════════════════════════════ */

static int test_original_turn_and_strafe_commands(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("LEFT/RIGHT turn, STRAFE_LEFT/RIGHT use original commands");
    setup_open_room(&world);
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_LEFT, &receipt) == 1,
           "LEFT should fill receipt");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW,
           "LEFT should turn");
    ASSERT(receipt.turned == 1, "LEFT should set turned");
    ASSERT(receipt.party_dir == THERON_DIR_WEST,
           "original command $01 should turn north to west");

    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_RIGHT, &receipt) == 1,
           "RIGHT should fill receipt");
    ASSERT(receipt.party_dir == THERON_DIR_NORTH,
           "original command $02 should turn west to north");

    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_STRAFE_LEFT, &receipt) == 1,
           "STRAFE_LEFT should fill receipt");
    ASSERT(receipt.moved == 1 && receipt.party_x == 2 &&
           receipt.party_y == 3 && receipt.party_dir == THERON_DIR_NORTH,
           "original command $06 should step west without turning");
    ASSERT(theron_v1_boot_runtime_handle_m12_input(&world, NULL,
            M12_MENU_INPUT_STRAFE_RIGHT, &receipt) == 1,
           "STRAFE_RIGHT should fill receipt");
    ASSERT(receipt.moved == 1 && receipt.party_x == 3 &&
           receipt.party_y == 3 && receipt.party_dir == THERON_DIR_NORTH,
           "original command $04 should step east without turning");
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
    ASSERT(theron_v1_move_party_original_command(
               &world, THERON_ORIGINAL_COMMAND_MOVE_FORWARD) ==
               THERON_MOVE_OK,
           "original command $03 should move in the current direction");
    ASSERT(world.party.leader_y == 1,
           "a second original $03 should continue north");
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

static int test_pickup_routes_to_front_cell(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;
    Theron_V1_Object object;

    TEST("PICKUP_ITEM takes the object in the facing cell");
    setup_open_room(&world);
    memset(&object, 0, sizeof(object));
    object.type = THERON_OBJTYPE_POTION;
    object.item_index = 2;
    object.dungeon_id = 1;
    object.level = 0;
    object.x = 3;
    object.y = 2;
    ASSERT(theron_v1_object_place(&world, &object) == 0,
           "front-cell object should be placed");
    ASSERT(theron_v1_boot_runtime_handle_m12_input(
               &world, NULL, M12_MENU_INPUT_PICKUP_ITEM, &receipt) == 1,
           "pickup input should produce a receipt");
    ASSERT(receipt.handled == 1 && receipt.picked_up == 1,
           "pickup receipt should report the mutation");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW,
           "successful pickup should redraw");
    ASSERT(strcmp(receipt.status, "ITEM PICKED UP") == 0,
           "pickup status should identify the source-item route");
    ASSERT(world.objects[0].flags & THERON_OBJ_F_PICKED_UP,
           "front-cell object should be marked picked up");
    ASSERT(theron_v1_boot_runtime_handle_m12_input(
               &world, NULL, M12_MENU_INPUT_DROP_ITEM, &receipt) == 1 &&
           receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED,
           "drop stays closed without a source-owned slot selection");
    ASSERT(theron_v1_boot_runtime_handle_m12_input(
               &world, NULL, M12_MENU_INPUT_USE_ITEM, &receipt) == 1 &&
           receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED,
           "use stays closed without the T900 consumer");
    PASS();
    return 1;
}

static int test_use_routes_to_front_door(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;
    Theron_V1_Object door;

    TEST("USE_ITEM operates the door in the facing cell");
    setup_open_room(&world);
    memset(&door, 0, sizeof(door));
    door.type = THERON_OBJTYPE_DOOR;
    door.state = THERON_DOOR_STATE_CLOSED;
    door.dungeon_id = 1;
    door.level = 0;
    door.x = 3;
    door.y = 2;
    ASSERT(theron_v1_object_place(&world, &door) == 0,
           "front-cell door should be placed");
    ASSERT(theron_v1_boot_runtime_handle_m12_input(
               &world, NULL, M12_MENU_INPUT_USE_ITEM, &receipt) == 1,
           "use input should produce a receipt");
    ASSERT(receipt.handled == 1 && receipt.used_front_object == 1,
           "use receipt should report the front-object mutation");
    ASSERT(receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW,
           "successful door use should redraw");
    ASSERT(world.objects[0].state == THERON_DOOR_STATE_OPEN,
           "front-cell door should be open");
    ASSERT(strcmp(receipt.status, "FRONT OBJECT USED") == 0,
           "use status should identify the source interaction route");
    door.state = THERON_DOOR_STATE_LOCKED;
    door.flags = THERON_DOOR_F_LOCKED;
    door.x = 4;
    ASSERT(theron_v1_object_place(&world, &door) == 0,
           "locked source door should be placed separately");
    world.objects[0].x = 4;
    world.objects[1].x = 3;
    world.levels[0][0].source_header_verified = 1;
    ASSERT(theron_v1_boot_runtime_handle_m12_input(
               &world, NULL, M12_MENU_INPUT_USE_ITEM, &receipt) == 1 &&
           receipt.used_front_object == 0 &&
           receipt.result == THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED,
           "source-level locked door must reject the host key fallback");
    ASSERT(world.objects[1].state == THERON_DOOR_STATE_LOCKED,
           "rejected locked door must remain locked");
    PASS();
    return 1;
}

static int test_source_inventory_selection_cycles_verified_slots(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("INVENTORY_TOGGLE selects only source-backed Theron slots");
    setup_open_room(&world);
    world.party.champions[0].inventory[3] = 7;
    world.inventory_source[0][3].valid = 1;
    world.inventory_source[0][3].item_type = 7;
    world.party.champions[0].inventory[8] = 11;
    world.inventory_source[0][8].valid = 1;
    world.inventory_source[0][8].item_type = 11;
    world.party.champions[0].inventory[5] = 9;
    ASSERT(theron_v1_boot_runtime_handle_m12_input_with_inventory_slot(
               &world, NULL, M12_MENU_INPUT_INVENTORY_TOGGLE, -1,
               &receipt) == 1,
           "inventory selection should produce a receipt");
    ASSERT(receipt.inventory_selected == 1 && receipt.inventory_slot == 3,
           "first source-backed slot should be selected");
    ASSERT(theron_v1_boot_runtime_handle_m12_input_with_inventory_slot(
               &world, NULL, M12_MENU_INPUT_INVENTORY_TOGGLE,
               receipt.inventory_slot, &receipt) == 1,
           "second inventory selection should produce a receipt");
    ASSERT(receipt.inventory_selected == 1 && receipt.inventory_slot == 8,
           "selection should skip compact IDs without source provenance");
    ASSERT(theron_v1_boot_runtime_handle_m12_input_with_inventory_slot(
               &world, NULL, M12_MENU_INPUT_INVENTORY_TOGGLE,
               receipt.inventory_slot, &receipt) == 1 &&
           receipt.inventory_slot == 3,
           "selection should wrap through authenticated slots");
    PASS();
    return 1;
}

static int test_cycle_selected_living_champions(void)
{
    Theron_V1_World world;
    Theron_V1_BootRuntimeInputReceipt receipt;

    TEST("CYCLE_CHAMPION stays inside the selected living party");
    setup_open_room(&world);
    world.party.champion_count = 2;
    world.party.active_slot = 0;
    world.party.champions[0].alive = 1;
    world.party.champions[1].alive = 1;
    ASSERT(theron_v1_boot_runtime_handle_m12_input(
               &world, NULL, M12_MENU_INPUT_CYCLE_CHAMPION, &receipt) == 1,
           "cycle input should produce a receipt");
    ASSERT(receipt.champion_cycled == 1 &&
           receipt.active_champion_slot == 1 &&
           world.party.active_slot == 1,
           "next selected living champion should become active");
    world.party.champions[0].alive = 0;
    ASSERT(theron_v1_boot_runtime_handle_m12_input(
               &world, NULL, M12_MENU_INPUT_CYCLE_CHAMPION, &receipt) == 1,
           "single-living cycle should still produce a receipt");
    ASSERT(receipt.champion_cycled == 0 && world.party.active_slot == 1,
           "dead or unselected slots must not become active");
    PASS();
    return 1;
}

static int test_source_move_advances_tick_without_fixture_stats(void)
{
    Theron_V1_World world;
    int result;

    TEST("Source level move advances T700 clock without guessed stats");
    setup_open_room(&world);
    world.levels[0][0].source_header_verified = 1;
    world.party.champions[0].food = 7;
    world.party.champions[0].water = 8;
    world.party.champions[0].stamina = 9;
    result = theron_v1_move_party(&world, THERON_DIR_NORTH);
    ASSERT(result == THERON_MOVE_OK, "source move should succeed");
    ASSERT(world.world_tick == 11, "source move should advance world tick");
    ASSERT(world.party.champions[0].food == 7 &&
           world.party.champions[0].water == 8 &&
           world.party.champions[0].stamina == 9,
           "unresolved T700 stat consumer must not use fixture drains");
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
    test_original_turn_and_strafe_commands();
    test_move_forward();
    test_move_backward();
    test_move_blocked();
    test_exit_dungeon();
    test_wait_tick();
    test_idle_tick();
    test_pickup_routes_to_front_cell();
    test_source_inventory_selection_cycles_verified_slots();
    test_use_routes_to_front_door();
    test_cycle_selected_living_champions();
    test_source_move_advances_tick_without_fixture_stats();

    printf("\n=====================================================\n");
    printf("Results: %d/%d passed  (%s)\n",
           g_tests_passed, g_tests_run,
           g_failures == 0 ? "all passed" : "FAILURES");
    printf("=====================================================\n");
    return g_failures == 0 ? 0 : 1;
}
