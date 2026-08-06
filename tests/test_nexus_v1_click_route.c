/*
 * test_nexus_v1_click_route.c
 *
 * Nexus V1 mouse click-route dispatch regression.
 * Verifies that inventory, world-square, door, and floor-item clicks are
 * translated into the same command queue consumed by nexus_mechanics_tick().
 * Inventory-use execution remains explicitly no-op until Saturn ITEM.IBS
 * action semantics are source-bound.
 *
 * Source: DM1 COMMAND.C mouse/click dispatch; CLIKMENU.C F0366 command queue;
 *         CHAMPION.C F0309 equipment slot layout.
 */

#include "nexus_v1_click_route.h"
#include "nexus_v1_inventory.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_squares.h"
#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { \
        g_pass++; \
    } else { \
        g_fail++; \
        fprintf(stderr, "FAIL: %s\n", (msg)); \
    } \
} while (0)

static void reset_engine_for_click_tests(Nexus_V1_Engine *engine,
                                          Nexus_MechanicsState *st) {
    int x, y;
    memset(engine, 0, sizeof(*engine));
    engine->level_loaded = 1;
    engine->audio_enabled = 1;
    engine->audio.initialized = 1;
    engine->audio.sfx_enabled = 1;

    engine->current_level.width = NEXUS_MAX_MAP_SIZE;
    engine->current_level.height = NEXUS_MAX_MAP_SIZE;
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; y++)
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; x++)
            engine->current_level.squares[y][x] = NEXUS_SQUARE_FLOOR;
    for (x = 0; x < NEXUS_MAX_MAP_SIZE; x++) {
        engine->current_level.squares[0][x] = NEXUS_SQUARE_WALL;
        engine->current_level.squares[NEXUS_MAX_MAP_SIZE - 1][x] = NEXUS_SQUARE_WALL;
        engine->current_level.squares[x][0] = NEXUS_SQUARE_WALL;
        engine->current_level.squares[x][NEXUS_MAX_MAP_SIZE - 1] = NEXUS_SQUARE_WALL;
    }

    engine->champions.champion_count = 1;
    engine->champions.party[0] = 0;
    engine->champions.party_count = 1;
    engine->champions.leader_index = 0;
    engine->champions.champions[0].alive = 1;
    engine->champions.champions[0].stamina = 100;
    engine->champions.champions[0].max_stamina = 100;
    memset(engine->champions.champions[0].inventory, 0xFF,
           sizeof(engine->champions.champions[0].inventory));
    memset(engine->champions.champions[0].slots, 0xFF,
           sizeof(engine->champions.champions[0].slots));

    nexus_mechanics_init(st, 10, 10, NEXUS_DIR_NORTH);
}

static void test_inventory_slot_click_use_item(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target;

    reset_engine_for_click_tests(&engine, &st);
    engine.champions.champions[0].inventory[2] = 30; /* Health Potion */
    engine.champions.champions[0].health = 10;
    engine.champions.champions[0].max_health = 100;

    target = nexus_click_target_inventory_slot(0, 2);
    CHECK(nexus_click_route_dispatch(&st, &engine, &target) == NEXUS_CLICK_RESULT_NO_ACTION,
          "inventory slot click is blocked without Saturn item semantics");
    CHECK(nexus_mechanics_tick(&st, &engine) == 0,
          "use-item command is consumed without synthetic redraw");
    CHECK(engine.champions.champions[0].health == 10,
          "health potion has no unverified Saturn use effect");
    CHECK(engine.champions.champions[0].inventory[2] == 30,
          "unverified item remains in inventory");
}

static void test_inventory_slot_click_empty_noop(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target;

    reset_engine_for_click_tests(&engine, &st);
    target = nexus_click_target_inventory_slot(0, 2);
    CHECK(nexus_click_route_dispatch(&st, &engine, &target) == NEXUS_CLICK_RESULT_NO_ACTION,
          "click on empty inventory slot returns NO_ACTION");
}

static void test_equipment_slot_click_unequip(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target;

    reset_engine_for_click_tests(&engine, &st);
    engine.champions.champions[0].slots[NEXUS_SLOT_WEAPON - 1] = 5; /* Sword */

    target = nexus_click_target_equipment_slot(0, NEXUS_SLOT_WEAPON);
    CHECK(nexus_click_route_dispatch(&st, &engine, &target) == NEXUS_CLICK_RESULT_NO_ACTION,
          "equipment slot click is blocked without Saturn item semantics");
    CHECK(engine.champions.champions[0].slots[NEXUS_SLOT_WEAPON - 1] == 5,
          "unverified sword remains equipped");
    CHECK(engine.champions.champions[0].inventory[0] == 0xFFU,
          "unverified sword is not copied into inventory");
}

static void test_equipment_slot_click_empty(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target;

    reset_engine_for_click_tests(&engine, &st);
    target = nexus_click_target_equipment_slot(0, NEXUS_SLOT_WEAPON);
    CHECK(nexus_click_route_dispatch(&st, &engine, &target) == NEXUS_CLICK_RESULT_NO_ACTION,
          "click on empty equipment slot returns NO_ACTION");
}

static void test_world_square_forward(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target;
    int cmd;

    reset_engine_for_click_tests(&engine, &st);
    target = nexus_click_target_world_square(10, 9); /* directly north */
    CHECK(nexus_click_route_dispatch(&st, &engine, &target) == NEXUS_CLICK_RESULT_OK,
          "world square north dispatches OK");
    CHECK(nexus_mechanics_pop_command(&st, &cmd) == 1,
          "a command was queued");
    CHECK(cmd == NEXUS_CMD_FORWARD,
          "north square maps to FORWARD");
}

static void test_world_square_turn_then_forward(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target;
    int cmd1, cmd2;

    reset_engine_for_click_tests(&engine, &st);
    target = nexus_click_target_world_square(11, 10); /* east */
    CHECK(nexus_click_route_dispatch(&st, &engine, &target) == NEXUS_CLICK_RESULT_OK,
          "world square east dispatches OK");
    CHECK(nexus_mechanics_pop_command(&st, &cmd1) == 1,
          "first command queued");
    CHECK(cmd1 == NEXUS_CMD_TURN_RIGHT,
          "east square first turns right");
    /* No forward queued yet; UI re-clicks after the turn tick. */
    CHECK(nexus_mechanics_pop_command(&st, &cmd2) == 0,
          "no second command queued for diagonal/far target");
}

static void test_world_square_wall_no_path(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target;

    reset_engine_for_click_tests(&engine, &st);
    target = nexus_click_target_world_square(0, 10); /* wall column */
    CHECK(nexus_click_route_dispatch(&st, &engine, &target) == NEXUS_CLICK_RESULT_NO_PATH,
          "wall square in front returns NO_PATH");
}

static void test_door_square_click_forces_forward(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target;
    int cmd;

    reset_engine_for_click_tests(&engine, &st);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_DOOR;
    nexus_doors_register(10, 9);

    target = nexus_click_target_door_square(10, 9);
    CHECK(nexus_click_route_dispatch(&st, &engine, &target) == NEXUS_CLICK_RESULT_OK,
          "door square click dispatches OK");
    CHECK(nexus_mechanics_pop_command(&st, &cmd) == 1,
          "door click queues command");
    CHECK(cmd == NEXUS_CMD_FORWARD,
          "door in front maps to FORWARD");
}

static void test_floor_item_pickup_at_current_square(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target;

    nexus_floor_init();
    reset_engine_for_click_tests(&engine, &st);
    nexus_floor_drop_source(10, 10, 63, 1, 0U, 0U, -1);
    /* Corn at party position: explicit fixture/source-admission lane. */

    target = nexus_click_target_floor_item(10, 10, 0);
    CHECK(nexus_click_route_dispatch(&st, &engine, &target) == NEXUS_CLICK_RESULT_OK,
          "floor item at current square dispatches OK");
    CHECK(st.input_count == 1,
          "floor item click queues one command");
    CHECK(st.input_queue[st.input_head] == NEXUS_CMD_INTERACT,
          "floor item at feet maps to INTERACT");

    (void)nexus_mechanics_tick(&st, &engine);
    CHECK(nexus_floor_count_at(10, 10) == 1,
          "unproven Saturn pickup leaves floor item in place");
    CHECK(engine.champions.champions[0].inventory[0] == 0xFFU,
          "unproven Saturn pickup leaves inventory unchanged");
}

static void test_floor_item_far_away_moves_toward(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target;
    int cmd;
    (void)cmd;

    nexus_floor_init();
    reset_engine_for_click_tests(&engine, &st);
    nexus_floor_drop_source(10, 8, 63, 1, 0U, 0U, -1);
    /* Corn north of party: explicit fixture/source-admission lane. */

    target = nexus_click_target_floor_item(10, 8, 0);
    CHECK(nexus_click_route_dispatch(&st, &engine, &target) == NEXUS_CLICK_RESULT_OK,
          "distant floor item dispatches OK");
    CHECK(st.input_count == 1,
          "distant floor item queues one command");
    CHECK(st.input_queue[st.input_head] == NEXUS_CMD_FORWARD,
          "distant floor item north maps to FORWARD");
}

static void test_invalid_arguments(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    Nexus_ClickTarget target = nexus_click_target_inventory_slot(0, 2);

    reset_engine_for_click_tests(&engine, &st);
    CHECK(nexus_click_route_dispatch(NULL, &engine, &target) == NEXUS_CLICK_RESULT_INVALID,
          "NULL mechanics state returns INVALID");
    CHECK(nexus_click_route_dispatch(&st, NULL, &target) == NEXUS_CLICK_RESULT_INVALID,
          "NULL engine returns INVALID");
    CHECK(nexus_click_route_dispatch(&st, &engine, NULL) == NEXUS_CLICK_RESULT_INVALID,
          "NULL target returns INVALID");
}

int main(void) {
    printf("Nexus V1 click-route dispatch test\n");

    test_inventory_slot_click_use_item();
    test_inventory_slot_click_empty_noop();
    test_equipment_slot_click_unequip();
    test_equipment_slot_click_empty();
    test_world_square_forward();
    test_world_square_turn_then_forward();
    test_world_square_wall_no_path();
    test_door_square_click_forces_forward();
    test_floor_item_pickup_at_current_square();
    test_floor_item_far_away_moves_toward();
    test_invalid_arguments();

    printf("Results: %d PASS, %d FAIL\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
