#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_creature_ai_pc34_compat.h"

static void test_rotate_null_safety(void)
{
    DM2_V1_RotateCreatureReceipt receipt;
    int r = dm2_v1_rotate_creature(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    r = dm2_v1_rotate_creature(NULL, NULL);
    assert(r == 0);
    printf("  PASS: rotate_null_safety\n");
}

static void test_rotate_invalid_handle(void)
{
    DM2_V1_RotateCreatureReceipt receipt;
    DM2_V1_RotateCreatureRequest req;
    memset(&req, 0, sizeof(req));
    req.creature_handle = -1;
    int r = dm2_v1_rotate_creature(&req, &receipt);
    assert(r == 0);
    printf("  PASS: rotate_invalid_handle\n");
}

static void test_rotate_valid(void)
{
    DM2_V1_RotateCreatureReceipt receipt;
    DM2_V1_RotateCreatureRequest req;
    memset(&req, 0, sizeof(req));
    req.creature_handle = 0x1000;
    req.new_direction = 2;
    int r = dm2_v1_rotate_creature(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    assert(receipt.new_direction == 2);
    printf("  PASS: rotate_valid\n");
}

static void test_rotate_direction_wrap(void)
{
    DM2_V1_RotateCreatureReceipt receipt;
    DM2_V1_RotateCreatureRequest req;
    memset(&req, 0, sizeof(req));
    req.creature_handle = 0x1000;
    req.new_direction = 7;
    int r = dm2_v1_rotate_creature(&req, &receipt);
    assert(r == 1);
    assert(receipt.new_direction == 3);
    printf("  PASS: rotate_direction_wrap\n");
}

static void test_think_null_safety(void)
{
    DM2_V1_ThinkCreatureReceipt receipt;
    int r = dm2_v1_think_creature(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    r = dm2_v1_think_creature(NULL, NULL);
    assert(r == 0);
    printf("  PASS: think_null_safety\n");
}

static void test_think_invalid_tile(void)
{
    DM2_V1_ThinkCreatureReceipt receipt;
    DM2_V1_ThinkCreatureRequest req;
    memset(&req, 0, sizeof(req));
    req.tile_x = -1;
    req.tile_y = 5;
    int r = dm2_v1_think_creature(&req, &receipt);
    assert(r == 0);
    printf("  PASS: think_invalid_tile\n");
}

static void test_think_valid(void)
{
    DM2_V1_ThinkCreatureReceipt receipt;
    DM2_V1_ThinkCreatureRequest req;
    memset(&req, 0, sizeof(req));
    req.tile_x = 10;
    req.tile_y = 15;
    req.map_level = 3;
    req.game_tick = 1000;
    int r = dm2_v1_think_creature(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    printf("  PASS: think_valid\n");
}

int main(void)
{
    printf("test_dm2_v1_creature_ai_pc34_compat:\n");
    test_rotate_null_safety();
    test_rotate_invalid_handle();
    test_rotate_valid();
    test_rotate_direction_wrap();
    test_think_null_safety();
    test_think_invalid_tile();
    test_think_valid();
    printf("All creature AI tests passed.\n");
    return 0;
}
