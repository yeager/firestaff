/* Test DM2 V1 creature operations — extended (c_creature.cpp). */

#include "dm2_v1_creature_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int g_deleted;
static int g_flag;

static void mock_delete(void *ctx, uint16_t x, uint16_t y, int p1, int p2)
{
    (void)ctx; (void)p1; (void)p2;
    g_deleted = (int)((x << 8) | y);
}

static void test_creature_kill_on_timer_position(void)
{
    g_deleted = 0;
    g_flag = 0;
    DM2_V1_CreatureKillCallbacks cb = { mock_delete, &g_flag };
    dm2_v1_creature_kill_on_timer_position(5, 7, &cb, NULL);
    assert(g_deleted == ((5 << 8) | 7));
    assert(g_flag == 1);
    printf("  PASS: creature_kill_on_timer_position\n");
}

static int mock_rand(void *ctx) { (void)ctx; return 2; }

static void test_creature_ccm06(void)
{
    DM2_V1_CreatureCCM06Callbacks cb = { 0, 2, mock_rand };
    /* Behind: delta=2, randomize */
    uint8_t result = dm2_v1_creature_ccm06(&cb, NULL);
    assert(result <= 3);

    /* Not behind: delta=1, face target */
    cb.creature_facing = 1;
    cb.target_dir = 2;
    result = dm2_v1_creature_ccm06(&cb, NULL);
    assert(result == 2);
    printf("  PASS: creature_ccm06\n");
}

static int16_t mock_rand16(void *ctx, int16_t max)
{
    (void)ctx;
    return (int16_t)(max / 2);
}

static int g_cloud_created;
static int mock_create_cloud(void *ctx, int spell_id, uint16_t strength,
                             uint16_t x, uint16_t y, int param)
{
    (void)ctx; (void)spell_id; (void)param;
    g_cloud_created = (int)((x << 16) | (y << 8) | strength);
    return 1;
}

static void test_creature_create_cloud(void)
{
    g_cloud_created = 0;
    DM2_V1_CreateCloudCallbacks cb = { mock_rand16, mock_create_cloud };
    int r = dm2_v1_creature_create_cloud(3, 4, &cb, NULL);
    assert(r == 1);
    assert(g_cloud_created != 0);
    printf("  PASS: creature_create_cloud\n");
}

static int g_wounded_mask;
static void mock_wound(void *ctx, int hero_idx, int16_t dmg, int bp, int wt)
{
    (void)ctx; (void)dmg; (void)bp; (void)wt;
    g_wounded_mask |= (1 << hero_idx);
}

static int mock_alive(void *ctx, int idx)
{
    (void)ctx;
    return idx < 3 ? 1 : 0;
}

static void test_attack_party(void)
{
    g_wounded_mask = 0;
    DM2_V1_AttackPartyCallbacks cb = { 4, mock_rand16, mock_alive, mock_wound };
    uint8_t result = dm2_v1_attack_party(100, 0x1F, 0, &cb, NULL);
    assert((result & 0x07) == 0x07);
    assert((result & 0x08) == 0);
    printf("  PASS: attack_party\n");
}

int main(void)
{
    printf("test_dm2_v1_creature_ops_ext:\n");
    test_creature_kill_on_timer_position();
    test_creature_ccm06();
    test_creature_create_cloud();
    test_attack_party();
    printf("All creature_ops_ext tests passed.\n");
    return 0;
}
