#include "dm2_v1_combat_helpers.h"
#include <assert.h>
#include <stdio.h>

static void test_stun_basic(void)
{
    DM2_V1_HandCooldownPair cur = {100, 50};
    DM2_V1_StunReceipt r;
    assert(dm2_v1_stun_champion_compute(&cur, 30, &r));
    assert(r.valid);
    assert(r.result.left_cooldown == 130);
    assert(r.result.right_cooldown == 80);
    assert(!r.clamped);
}

static void test_stun_clamp(void)
{
    DM2_V1_HandCooldownPair cur = {200, 250};
    DM2_V1_StunReceipt r;
    assert(dm2_v1_stun_champion_compute(&cur, 100, &r));
    assert(r.valid);
    assert(r.result.left_cooldown == 255);
    assert(r.result.right_cooldown == 255);
    assert(r.clamped);
}

static void test_stun_zero(void)
{
    DM2_V1_HandCooldownPair cur = {10, 20};
    DM2_V1_StunReceipt r;
    assert(dm2_v1_stun_champion_compute(&cur, 0, &r));
    assert(r.result.left_cooldown == 10);
    assert(r.result.right_cooldown == 20);
    assert(!r.clamped);
}

static void test_stun_null(void)
{
    DM2_V1_StunReceipt r;
    assert(!dm2_v1_stun_champion_compute(NULL, 10, &r));
    assert(!r.valid);
}

static void test_bones_item_id(void)
{
    assert(dm2_v1_get_champion_bones_item_id(0) == 0);
    assert(dm2_v1_get_champion_bones_item_id(1) == 5);
}

int main(void)
{
    test_stun_basic();
    test_stun_clamp();
    test_stun_zero();
    test_stun_null();
    test_bones_item_id();
    assert(dm2_v1_combat_helpers_source_evidence() != NULL);
    printf("All dm2_v1_combat_helpers tests passed.\n");
    return 0;
}
