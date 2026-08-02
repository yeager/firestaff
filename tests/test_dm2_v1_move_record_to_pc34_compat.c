/*
 * test_dm2_v1_move_record_to_pc34_compat.c — unit tests for record
 * relocation and movement execution.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_move_record_to_pc34_compat.h"
#include "dm2_v1_perform_move_exec_pc34_compat.h"
#include "dm2_v1_perform_move.h"

/* ── MOVE_RECORD_TO tests ─────────────────────────────────────────── */

static void test_move_record_to_null_safety(void)
{
    DM2_V1_MoveRecordToReceipt receipt;
    int r = dm2_v1_move_record_to(NULL, NULL, NULL,
                                   0, 0, 0, 0, 0, 0, 0, 0, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_move_record_to(NULL, NULL, NULL,
                               0, 0, 0, 0, 0, 0, 0, 0, NULL);
    assert(r == 0);

    printf("  PASS: move_record_to_null_safety\n");
}

static void test_move_record_to_party_move(void)
{
    DM2_V1_MoveRecordToReceipt receipt;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_DungeonData dungeon;
    DM2_V1_SourceTimerQueue queue;

    memset(&pools, 0, sizeof(pools));
    memset(&dungeon, 0, sizeof(dungeon));
    dm2_v1_source_timer_queue_init(&queue);

    int r = dm2_v1_move_record_to(&pools, &dungeon, &queue,
                                   (int16_t)0xFFFF, 5, 3, 6, 3, 0,
                                   0, 100, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.is_party_move == 1);
    assert(receipt.is_creature_move == 0);
    assert(receipt.from_x == 5);
    assert(receipt.from_y == 3);
    assert(receipt.dest_x == 6);
    assert(receipt.dest_y == 3);

    printf("  PASS: move_record_to_party_move\n");
}

static void test_move_record_to_same_tile(void)
{
    DM2_V1_MoveRecordToReceipt receipt;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_DungeonData dungeon;

    memset(&pools, 0, sizeof(pools));
    memset(&dungeon, 0, sizeof(dungeon));

    int r = dm2_v1_move_record_to(&pools, &dungeon, NULL,
                                   (int16_t)0xFFFF, 5, 3, 5, 3, 0,
                                   0, 0, &receipt);
    assert(r == 1);
    assert(receipt.is_same_tile == 1);

    printf("  PASS: move_record_to_same_tile\n");
}

static void test_move_record_to_creature(void)
{
    DM2_V1_MoveRecordToReceipt receipt;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_DungeonData dungeon;

    memset(&pools, 0, sizeof(pools));
    memset(&dungeon, 0, sizeof(dungeon));

    /* Creature handle: DB type 4 in bits 10-13 */
    int16_t creature_handle = (int16_t)((4 << 10) | 7);
    int r = dm2_v1_move_record_to(&pools, &dungeon, NULL,
                                   creature_handle, 2, 3, 4, 5, 1,
                                   0, 0, &receipt);
    assert(r == 1);
    assert(receipt.is_creature_move == 1);
    assert(receipt.is_party_move == 0);

    printf("  PASS: move_record_to_creature\n");
}

static void test_move_record_to_from_nowhere(void)
{
    DM2_V1_MoveRecordToReceipt receipt;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_DungeonData dungeon;

    memset(&pools, 0, sizeof(pools));
    memset(&dungeon, 0, sizeof(dungeon));

    int r = dm2_v1_move_record_to(&pools, &dungeon, NULL,
                                   (int16_t)0xFFFF, -1, -1, 5, 3, 0,
                                   0, 0, &receipt);
    assert(r == 1);
    assert(receipt.is_from_nowhere == 1);
    assert(receipt.is_to_nowhere == 0);

    printf("  PASS: move_record_to_from_nowhere\n");
}

/* ── Walk delay tests ─────────────────────────────────────────────── */

static void test_calc_party_walk_delay(void)
{
    int d = dm2_v1_calc_party_walk_delay(NULL, 0);
    assert(d == 1);

    DM2_V1_HeroMoveState heroes[2];
    memset(heroes, 0, sizeof(heroes));
    heroes[0].alive = 1;
    heroes[0].max_load = 100;
    heroes[0].player_weight = 50;
    heroes[1].alive = 0;

    d = dm2_v1_calc_party_walk_delay(heroes, 2);
    assert(d >= 1);

    printf("  PASS: calc_party_walk_delay\n");
}

static void test_drain_party_stamina(void)
{
    int d = dm2_v1_drain_party_stamina(NULL, 0);
    assert(d == 0);

    DM2_V1_HeroMoveState heroes[2];
    memset(heroes, 0, sizeof(heroes));
    heroes[0].alive = 1;
    heroes[0].max_load = 100;
    heroes[0].player_weight = 50;
    heroes[1].alive = 1;
    heroes[1].max_load = 80;
    heroes[1].player_weight = 60;

    d = dm2_v1_drain_party_stamina(heroes, 2);
    assert(d == 2);

    printf("  PASS: drain_party_stamina\n");
}

/* ── Perform move exec tests ─────────────────────────────────────── */

static void test_perform_move_exec_null(void)
{
    DM2_V1_PerformMoveExecReceipt receipt;
    int r = dm2_v1_perform_move_exec(NULL, NULL, NULL, NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_perform_move_exec(NULL, NULL, NULL, NULL, NULL);
    assert(r == 0);

    printf("  PASS: perform_move_exec_null\n");
}

static void test_perform_move_exec_accepted(void)
{
    DM2_V1_PerformMoveExecReceipt exec_receipt;
    DM2_V1_PerformMoveExecRequest req;
    DM2_V1_PerformMoveRequest plan_req;
    DM2_V1_PerformMoveReceipt plan;

    memset(&plan_req, 0, sizeof(plan_req));
    plan_req.runtime_ready = 1;
    plan_req.can_move = 1;
    plan_req.current_level = 0;
    plan_req.from_x = 5;
    plan_req.from_y = 3;
    plan_req.from_dir = 0;
    plan_req.direction = 1; /* east */
    plan_req.target_raw_valid = 1;
    plan_req.target_square_type = 0; /* DM2_SQUARE_FLOOR */
    dm2_v1_DM2_PERFORM_MOVE_plan(&plan_req, &plan);
    assert(plan.accepted == 1);

    memset(&req, 0, sizeof(req));
    req.plan = plan;
    req.hero_count = 1;
    req.heroes[0].alive = 1;
    req.heroes[0].max_load = 100;
    req.heroes[0].player_weight = 50;
    req.game_tick = 500;

    int r = dm2_v1_perform_move_exec(NULL, NULL, NULL, &req, &exec_receipt);
    assert(r == 1);
    assert(exec_receipt.valid == 1);
    assert(exec_receipt.position_updated == 1);
    assert(exec_receipt.final_x == plan.to_x);
    assert(exec_receipt.final_y == plan.to_y);
    assert(exec_receipt.classification == DM2_MOVE_CLASS_OPEN_TILE);
    assert(exec_receipt.squad_dir_reset == 1);
    assert(exec_receipt.stamina_drained == 1);

    printf("  PASS: perform_move_exec_accepted\n");
}

static void test_perform_move_exec_blocked(void)
{
    DM2_V1_PerformMoveExecReceipt exec_receipt;
    DM2_V1_PerformMoveExecRequest req;
    DM2_V1_PerformMoveRequest plan_req;
    DM2_V1_PerformMoveReceipt plan;

    memset(&plan_req, 0, sizeof(plan_req));
    plan_req.runtime_ready = 1;
    plan_req.can_move = 1;
    plan_req.current_level = 0;
    plan_req.from_x = 5;
    plan_req.from_y = 3;
    plan_req.from_dir = 0;
    plan_req.direction = 0;
    plan_req.target_raw_valid = 1;
    plan_req.target_square_type = 1; /* DM2_SQUARE_WALL */
    dm2_v1_DM2_PERFORM_MOVE_plan(&plan_req, &plan);
    assert(plan.blocked == 1);

    memset(&req, 0, sizeof(req));
    req.plan = plan;

    int r = dm2_v1_perform_move_exec(NULL, NULL, NULL, &req, &exec_receipt);
    assert(r == 0);
    assert(exec_receipt.valid == 1);
    assert(exec_receipt.position_updated == 0);

    printf("  PASS: perform_move_exec_blocked\n");
}

static void test_perform_move_exec_teleporter(void)
{
    DM2_V1_PerformMoveExecReceipt exec_receipt;
    DM2_V1_PerformMoveExecRequest req;
    DM2_V1_PerformMoveReceipt plan;

    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.accepted = 1;
    plan.current_level = 0;
    plan.from_x = 5;
    plan.from_y = 3;
    plan.to_x = 6;
    plan.to_y = 3;
    plan.to_dir = 1;
    plan.target_raw_valid = 1;
    plan.target_square_type = 8; /* teleporter */

    memset(&req, 0, sizeof(req));
    req.plan = plan;
    req.hero_count = 1;
    req.heroes[0].alive = 1;
    req.heroes[0].max_load = 100;
    req.heroes[0].player_weight = 50;

    int r = dm2_v1_perform_move_exec(NULL, NULL, NULL, &req, &exec_receipt);
    assert(r == 1);
    assert(exec_receipt.classification == DM2_MOVE_CLASS_TELEPORTER);
    assert(exec_receipt.position_updated == 1);
    assert(exec_receipt.fail_closed == 1);

    printf("  PASS: perform_move_exec_teleporter\n");
}

static void test_perform_move_exec_stair_up(void)
{
    DM2_V1_PerformMoveExecRequest req;
    DM2_V1_PerformMoveExecReceipt exec_receipt;
    DM2_V1_PerformMoveReceipt plan;

    memset(&req, 0, sizeof(req));
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.accepted = 1;
    plan.current_level = 2;
    plan.from_x = 5; plan.from_y = 5;
    plan.to_x = 5; plan.to_y = 4;
    plan.to_dir = 0;
    plan.target_square_type = 6; /* DM2_SQUARE_STAIRS_UP */
    req.plan = plan;

    /* Without map descriptors: fail_closed */
    int r = dm2_v1_perform_move_exec(NULL, NULL, NULL, &req, &exec_receipt);
    assert(exec_receipt.classification == DM2_MOVE_CLASS_STAIR_UP);
    assert(exec_receipt.fail_closed == 1);
    assert(exec_receipt.stair_level_changed == 0);

    /* With map descriptors: resolve stair destination.
     * All maps share same world origin so coordinates overlap. */
    DM2_V1_SkprojectMapDescriptor maps[3];
    memset(maps, 0, sizeof(maps));
    maps[0].map_id = 0; maps[0].world_x = 0; maps[0].world_y = 0;
    maps[0].width = 10; maps[0].height = 10;
    maps[1].map_id = 1; maps[1].world_x = 0; maps[1].world_y = 0;
    maps[1].width = 10; maps[1].height = 10;
    maps[2].map_id = 2; maps[2].world_x = 0; maps[2].world_y = 0;
    maps[2].width = 10; maps[2].height = 10;
    req.map_descriptors = maps;
    req.map_descriptor_count = 3;

    r = dm2_v1_perform_move_exec(NULL, NULL, NULL, &req, &exec_receipt);
    assert(exec_receipt.classification == DM2_MOVE_CLASS_STAIR_UP);
    assert(exec_receipt.stair_level_changed == 1);
    /* locate_other_level finds first candidate that contains the world
     * coords — with all maps at same origin, that's map 0 or 1 (any
     * map except the source map 2). */
    assert(exec_receipt.stair_dest_level >= 0);
    assert(exec_receipt.stair_dest_level != 2);
    assert(exec_receipt.final_level == exec_receipt.stair_dest_level);
    (void)r;
    printf("  PASS: perform_move_exec_stair_up\n");
}

static void test_perform_move_exec_stair_down(void)
{
    DM2_V1_PerformMoveExecRequest req;
    DM2_V1_PerformMoveExecReceipt exec_receipt;
    DM2_V1_PerformMoveReceipt plan;

    memset(&req, 0, sizeof(req));
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.accepted = 1;
    plan.current_level = 1;
    plan.from_x = 3; plan.from_y = 3;
    plan.to_x = 3; plan.to_y = 4;
    plan.to_dir = 2;
    plan.target_square_type = 7; /* DM2_SQUARE_STAIRS_DOWN */
    req.plan = plan;

    DM2_V1_SkprojectMapDescriptor maps[3];
    memset(maps, 0, sizeof(maps));
    maps[0].map_id = 0; maps[0].world_x = 0; maps[0].world_y = 0;
    maps[0].width = 10; maps[0].height = 10;
    maps[1].map_id = 1; maps[1].world_x = 0; maps[1].world_y = 0;
    maps[1].width = 10; maps[1].height = 10;
    maps[2].map_id = 2; maps[2].world_x = 0; maps[2].world_y = 0;
    maps[2].width = 10; maps[2].height = 10;
    req.map_descriptors = maps;
    req.map_descriptor_count = 3;

    int r = dm2_v1_perform_move_exec(NULL, NULL, NULL, &req, &exec_receipt);
    assert(exec_receipt.classification == DM2_MOVE_CLASS_STAIR_DOWN);
    assert(exec_receipt.stair_level_changed == 1);
    assert(exec_receipt.stair_dest_level >= 0);
    assert(exec_receipt.stair_dest_level != 1);
    assert(exec_receipt.final_level == exec_receipt.stair_dest_level);
    (void)r;
    printf("  PASS: perform_move_exec_stair_down\n");
}

int main(void)
{
    printf("test_dm2_v1_move_record_to_pc34_compat:\n");
    test_move_record_to_null_safety();
    test_move_record_to_party_move();
    test_move_record_to_same_tile();
    test_move_record_to_creature();
    test_move_record_to_from_nowhere();
    test_calc_party_walk_delay();
    test_drain_party_stamina();
    test_perform_move_exec_null();
    test_perform_move_exec_accepted();
    test_perform_move_exec_blocked();
    test_perform_move_exec_teleporter();
    test_perform_move_exec_stair_up();
    test_perform_move_exec_stair_down();
    printf("All move record/exec tests passed.\n");
    return 0;
}
