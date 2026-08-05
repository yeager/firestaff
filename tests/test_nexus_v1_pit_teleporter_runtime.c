/*
 * tests/test_nexus_v1_pit_teleporter_runtime.c
 *
 * Nexus V1 pit/chute and teleporter runtime regression.
 * Verifies that stepping on chute/pit squares and teleporters produces
 * the correct pending level change / teleport targets, including
 * cross-level teleporters and stairs with explicit target coordinates.
 *
 * Source: DM1 MOVESENS.C F0267/F0268 (pit/chute/teleporter sensors);
 *         DM1 DUNGEON.C square type dispatch;
 *         ReDMCSB CLIKMENU.C:264-276 (level-transition special cases).
 */

#include "nexus_v1_engine.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_squares.h"
#include "nexus_v1_movement.h"
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

static void reset_engine_for_square_tests(Nexus_V1_Engine *engine,
                                          Nexus_MechanicsState *st,
                                          int start_x, int start_y, int start_dir,
                                          int map_index) {
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

    nexus_mechanics_init(st, start_x, start_y, start_dir);
    st->map_index = map_index;

    nexus_teleporters_init();
    nexus_stairs_init();
    nexus_doors_init();
}

static void test_chute_step_pending_level_change(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_CHUTE;
    engine.current_level.collision_refs[9][10] = 0;

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    int redraw = nexus_mechanics_tick(&st, &engine);

    CHECK(redraw == 1, "chute step requests redraw");
    CHECK(st.party_x == 10 && st.party_y == 9,
          "party moved onto chute square");
    CHECK(st.pending_level_change == 4,
          "chute sets pending_level_change to current level + 1");
}

static void test_chute_at_max_level_clamps(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 15);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_CHUTE;
    engine.current_level.collision_refs[9][10] = 0;

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    nexus_mechanics_tick(&st, &engine);

    CHECK(st.pending_level_change == 15,
          "chute at level 15 clamps pending_level_change to 15");
}

static void test_teleporter_same_level(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_TELEPORT;
    engine.current_level.collision_refs[9][10] = 0;
    nexus_teleporters_register(10, 9, 20, 20, 3);

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    /* First tick sets pending_teleport; second tick applies it. */
    nexus_mechanics_tick(&st, &engine);
    CHECK(st.pending_teleport == 1,
          "teleporter square sets pending_teleport");
    nexus_mechanics_tick(&st, &engine);

    CHECK(st.party_x == 20 && st.party_y == 20,
          "same-level teleport moves party to target square");
    CHECK(st.pending_level_change == -1,
          "same-level teleport does not set pending_level_change");
    CHECK(st.pending_teleport == 0,
          "pending_teleport cleared after application");
}

static void test_teleporter_cross_level(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_TELEPORT2;
    engine.current_level.collision_refs[9][10] = 0;
    nexus_teleporters_register(10, 9, 5, 5, 7);

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    nexus_mechanics_tick(&st, &engine);
    nexus_mechanics_tick(&st, &engine);

    CHECK(st.party_x == 5 && st.party_y == 5,
          "cross-level teleport moves party to target square");
    CHECK(st.pending_level_change == 7,
          "cross-level teleport sets pending_level_change to target level");
}

static void test_teleporter_unregistered_no_effect(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_TELEPORT;
    engine.current_level.collision_refs[9][10] = 0;
    /* No teleporter registered. */

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    nexus_mechanics_tick(&st, &engine);

    CHECK(st.party_x == 10 && st.party_y == 10,
          "unregistered teleporter blocks before party position changes");
    CHECK(st.pending_teleport == 0,
          "unregistered teleporter does not set pending_teleport");
    CHECK(st.pending_level_change == -1,
          "unregistered teleporter leaves pending_level_change clear");
}

static void test_stairs_down_with_target(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_STAIRS_DN;
    engine.current_level.collision_refs[9][10] = 0;
    nexus_stairs_register(10, 9, 5, 12, 14, NEXUS_DIR_SOUTH);

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    nexus_mechanics_tick(&st, &engine);

    CHECK(st.party_x == 12 && st.party_y == 14,
          "stairs down move party to registered target coordinates");
    CHECK(st.pending_level_change == 5,
          "stairs down set pending_level_change to registered target level");
    CHECK(st.party_dir == NEXUS_DIR_SOUTH,
          "stairs down apply registered facing");
}

static void test_stairs_down_unregistered_blocks(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_STAIRS_DN;
    engine.current_level.collision_refs[9][10] = 0;
    /* No source-owned stairs destination: movement must remain blocked. */

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    nexus_mechanics_tick(&st, &engine);

    CHECK(st.party_x == 10 && st.party_y == 10,
          "unregistered stairs down does not move party");
    CHECK(st.pending_level_change == -1,
          "unregistered stairs down leaves pending_level_change clear");
}

static void test_stairs_up_unregistered_blocks(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_STAIRS_UP;
    engine.current_level.collision_refs[9][10] = 0;
    /* No source-owned stairs destination: movement must remain blocked. */

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    nexus_mechanics_tick(&st, &engine);

    CHECK(st.party_x == 10 && st.party_y == 10,
          "unregistered stairs up does not move party");
    CHECK(st.pending_level_change == -1,
          "unregistered stairs up leaves pending_level_change clear");
}

static void test_square_event_unregistered_stairs_blocks(void) {
    int tx = 10;
    int ty = 9;
    int tl = 3;
    int td = NEXUS_DIR_NORTH;
    Nexus_SquareEvent event;

    nexus_stairs_init();
    event = nexus_process_square_event(NEXUS_SQUARE_STAIRS_DN, 10, 9,
                                        &tx, &ty, &tl, &td);
    CHECK(event == NEXUS_EVENT_BLOCKED,
          "square-event route blocks an unregistered stair");
    CHECK(tx == 10 && ty == 9 && tl == -1 && td == -1,
          "blocked square-event route exposes no transition target");
}

static void test_stairs_up_with_target(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_STAIRS_UP;
    engine.current_level.collision_refs[9][10] = 0;
    nexus_stairs_register(10, 9, 1, 12, 14, NEXUS_DIR_EAST);

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    nexus_mechanics_tick(&st, &engine);

    CHECK(st.party_x == 12 && st.party_y == 14,
          "stairs up move party to registered target coordinates");
    CHECK(st.pending_level_change == 1,
          "stairs up set pending_level_change to registered target level");
    CHECK(st.party_dir == NEXUS_DIR_EAST,
          "stairs up apply registered facing");
}

static void test_automap_write_remains_capture_gated(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    nexus_mechanics_tick(&st, &engine);

    CHECK(nexus_v1_automap_explored_count(&engine.automap, 3) == 0,
          "retail automap does not synthesize explored cells without Saturn evidence");
}

static void test_exit_final_level_game_over(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 15);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_EXIT;
    engine.current_level.collision_refs[9][10] = 0;

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    nexus_mechanics_tick(&st, &engine);

    CHECK(st.party_x == 10 && st.party_y == 9,
          "party moved onto exit square");
    CHECK(st.game_over == 1,
          "final-level exit sets game_over");
    CHECK(st.game_over_reason == 1,
          "final-level exit sets game_over_reason to exit_reached");
}

static void test_exit_non_final_level_no_game_over(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 5);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_EXIT;
    engine.current_level.collision_refs[9][10] = 0;

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    nexus_mechanics_tick(&st, &engine);

    CHECK(st.party_x == 10 && st.party_y == 9,
          "party moved onto non-final exit square");
    CHECK(st.game_over == 0,
          "non-final exit does not set game_over");
}

static void test_square_event_chute_returns_chute_fall(void) {
    int tx = -1, ty = -1, tl = -2, td = -2;
    Nexus_SquareEvent ev = nexus_process_square_event(
        NEXUS_SQUARE_CHUTE, 10, 9, &tx, &ty, &tl, &td);

    CHECK(ev == NEXUS_EVENT_CHUTE_FALL,
          "chute square returns NEXUS_EVENT_CHUTE_FALL");
    CHECK(tx == 10 && ty == 9,
          "chute event keeps coordinates");
    CHECK(tl == -1,
          "chute event signals default level transition (-1)");
}

static void test_square_event_teleport_returns_teleport(void) {
    int tx = -1, ty = -1, tl = -2;
    nexus_teleporters_register(10, 9, 21, 22, 6);
    Nexus_SquareEvent ev = nexus_process_square_event(
        NEXUS_SQUARE_TELEPORT, 10, 9, &tx, &ty, &tl, NULL);

    CHECK(ev == NEXUS_EVENT_TELEPORT,
          "registered teleporter returns NEXUS_EVENT_TELEPORT");
    CHECK(tx == 21 && ty == 22,
          "teleport event reports target coordinates");
    CHECK(tl == 6,
          "teleport event reports target level");
}

static void give_leader_item(Nexus_V1_Engine *engine, int item_id) {
    int leader_idx = -1;
    if (engine->champions.party_count > 0 &&
        engine->champions.leader_index >= 0 &&
        engine->champions.leader_index < engine->champions.party_count) {
        leader_idx = engine->champions.party[engine->champions.leader_index];
    }
    if (leader_idx >= 0 && leader_idx < engine->champions.champion_count) {
        engine->champions.champions[leader_idx].inventory[0] = (uint8_t)item_id;
    }
}

static void test_water_blocked_without_rope(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_WATER;
    engine.current_level.collision_refs[9][10] = 0;

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    int redraw = nexus_mechanics_tick(&st, &engine);

    CHECK(redraw == 1, "water square is always passable (no gate in DM.BIN)");
    CHECK(st.party_x == 10 && st.party_y == 9,
          "water square allows movement (DM.BIN: no CMP/EQ #21 in movement path)");
}

static void test_water_crossed_with_rope(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_WATER;
    engine.current_level.collision_refs[9][10] = 0;
    give_leader_item(&engine, 65);

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    int redraw = nexus_mechanics_tick(&st, &engine);

    CHECK(redraw == 1, "water square passable regardless of inventory");
    CHECK(st.party_x == 10 && st.party_y == 9,
          "water square allows movement (no crossing gate in DM.BIN)");
}

static void test_fire_blocked_without_rune(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_FIRE;
    engine.current_level.collision_refs[9][10] = 0;

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    int redraw = nexus_mechanics_tick(&st, &engine);

    CHECK(redraw == 0, "fire without rune does not request redraw");
    CHECK(st.party_x == 10 && st.party_y == 10,
          "fire without rune keeps party on starting square");
}

static void test_fire_crossed_with_rune(void) {
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;

    reset_engine_for_square_tests(&engine, &st, 10, 10, NEXUS_DIR_NORTH, 3);
    engine.current_level.squares[9][10] = NEXUS_SQUARE_FIRE;
    engine.current_level.collision_refs[9][10] = 0;
    st.fire_shield_ticks = 100;

    nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
    int redraw = nexus_mechanics_tick(&st, &engine);

    CHECK(redraw == 1, "fire with fire_shield allows crossing");
    CHECK(st.party_x == 10 && st.party_y == 9,
          "fire with fire_shield moves party (DM.BIN 0x0603C386 bit 0 gate)");
}

static void test_square_event_water_returns_cross_water(void) {
    Nexus_SquareEvent ev = nexus_process_square_event(
        NEXUS_SQUARE_WATER, 10, 9, NULL, NULL, NULL, NULL);

    CHECK(ev == NEXUS_EVENT_CROSS_WATER,
          "water square returns NEXUS_EVENT_CROSS_WATER");
}

static void test_square_event_fire_returns_cross_fire(void) {
    Nexus_SquareEvent ev = nexus_process_square_event(
        NEXUS_SQUARE_FIRE, 10, 9, NULL, NULL, NULL, NULL);

    CHECK(ev == NEXUS_EVENT_CROSS_FIRE,
          "fire square returns NEXUS_EVENT_CROSS_FIRE");
}

int main(void) {
    printf("Nexus V1 pit/chute, teleporter, stairs, exit, water and fire runtime test\n");

    test_chute_step_pending_level_change();
    test_chute_at_max_level_clamps();
    test_teleporter_same_level();
    test_teleporter_cross_level();
    test_teleporter_unregistered_no_effect();
    test_stairs_down_with_target();
    test_stairs_down_unregistered_blocks();
    test_stairs_up_with_target();
    test_automap_write_remains_capture_gated();
    test_stairs_up_unregistered_blocks();
    test_square_event_unregistered_stairs_blocks();
    test_exit_final_level_game_over();
    test_exit_non_final_level_no_game_over();
    test_square_event_chute_returns_chute_fall();
    test_square_event_teleport_returns_teleport();
    test_water_blocked_without_rope();
    test_water_crossed_with_rope();
    test_fire_blocked_without_rune();
    test_fire_crossed_with_rune();
    test_square_event_water_returns_cross_water();
    test_square_event_fire_returns_cross_fire();

    printf("Results: %d PASS, %d FAIL\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
