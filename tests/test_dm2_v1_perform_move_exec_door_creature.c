/*
 * test_dm2_v1_perform_move_exec_door_creature.c
 *
 * Tests for the door attack and creature encounter execution paths
 * in dm2_v1_perform_move_exec (skmove.cpp:428-543).
 */

#include "dm2_v1_perform_move_exec_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static DM2_V1_PerformMoveExecRequest make_door_request(void)
{
    DM2_V1_PerformMoveExecRequest req;
    memset(&req, 0, sizeof(req));
    req.plan.valid = 1;
    req.plan.accepted = 1;
    req.plan.blocked = 0;
    req.plan.from_x = 5;
    req.plan.from_y = 5;
    req.plan.to_x = 6;
    req.plan.to_y = 5;
    req.plan.to_dir = 1;
    req.plan.current_level = 0;
    req.plan.target_is_door = 1;
    req.plan.target_door_state = 1;
    req.plan.target_square_type = 4;

    req.hero_count = 1;
    req.heroes[0].alive = 1;
    req.heroes[0].max_load = 100;
    req.heroes[0].player_weight = 50;

    req.door_tile_type = 4;
    req.door_record_byte2 = 0x80;
    req.door_record_byte3 = 0x01;
    req.door_required_power = 10;
    req.door_use_byte2_gate = 1;
    req.door_rebirth_altar = 0;
    req.door_timer_delay = 0;
    req.party_attack_power = 20;

    return req;
}

int main(void)
{
    DM2_V1_PerformMoveExecReceipt receipt;

    printf("=== DM2 V1 PERFORM_MOVE Exec: Door Attack + Creature ===\n");

    /* Door attack: sufficient power, gate open */
    {
        DM2_V1_PerformMoveExecRequest req = make_door_request();
        dm2_v1_perform_move_exec(NULL, NULL, NULL, &req, &receipt);
        CHECK(receipt.valid && receipt.classification == DM2_MOVE_CLASS_DOOR_ATTACK,
              "door attack classified correctly");
        CHECK(receipt.door_attacked == 1,
              "door_attacked flag set");
        CHECK(receipt.door_attack_power == 20,
              "door_attack_power receipted");
        CHECK(!receipt.fail_closed,
              "door attack with sufficient power does not fail closed");
    }

    /* Door attack: insufficient power */
    {
        DM2_V1_PerformMoveExecRequest req = make_door_request();
        req.party_attack_power = 5;
        dm2_v1_perform_move_exec(NULL, NULL, NULL, &req, &receipt);
        CHECK(receipt.door_attacked == 1,
              "door_attacked set even when power insufficient");
        CHECK(receipt.door_attack_power == 5,
              "insufficient power receipted");
    }

    /* Door attack: gate flag closed (byte2 & 0x80 == 0) */
    {
        DM2_V1_PerformMoveExecRequest req = make_door_request();
        req.door_record_byte2 = 0x00;
        dm2_v1_perform_move_exec(NULL, NULL, NULL, &req, &receipt);
        CHECK(receipt.door_attacked == 1,
              "door_attacked set even with gate closed");
    }

    /* Creature push: weight below threshold (force-pushable) */
    {
        DM2_V1_SkprojectCreaturePushReceipt push;
        int pushed = dm2_v1_skproject_move_12b4_0d75(
            6, 5, 1, 1, 50, 100, 0, &push);
        CHECK(pushed && push.lifted_by_force,
              "creature push succeeds when weight < threshold");
    }

    /* Creature push: not movable */
    {
        DM2_V1_SkprojectCreaturePushReceipt push;
        int pushed = dm2_v1_skproject_move_12b4_0d75(
            6, 5, 1, 0, 50, 100, 0, &push);
        CHECK(!pushed && push.blocked_unmovable,
              "creature push blocked when not movable");
    }

    /* Creature push: heavy with nonzero random */
    {
        DM2_V1_SkprojectCreaturePushReceipt push;
        int pushed = dm2_v1_skproject_move_12b4_0d75(
            6, 5, 1, 1, 200, 100, 5, &push);
        CHECK(!pushed,
              "heavy creature with nonzero random not pushed");
    }

    /* Creature push: heavy with zero random */
    {
        DM2_V1_SkprojectCreaturePushReceipt push;
        int pushed = dm2_v1_skproject_move_12b4_0d75(
            6, 5, 1, 1, 200, 100, 0, &push);
        CHECK(pushed && push.lifted_by_random_zero,
              "heavy creature pushed when random == 0");
    }

    printf("\n%d passed, %d failed (of %d)\n",
           passed, failed, passed + failed);
    return failed ? 1 : 0;
}
