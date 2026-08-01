/*
 * test_dm2_v1_combat_damage_pc34_compat.c — unit tests for the
 * DM2 combat damage pipeline.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_combat_damage_pc34_compat.h"

static void test_calc_damage_null_safety(void)
{
    DM2_V1_CalcAttackDamageReceipt receipt;
    int r = dm2_v1_calc_player_attack_damage(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_calc_player_attack_damage(NULL, NULL);
    assert(r == 0);

    printf("  PASS: calc_damage_null_safety\n");
}

static void test_calc_damage_dead_hero(void)
{
    DM2_V1_CalcAttackDamageReceipt receipt;
    DM2_V1_CalcAttackDamageRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_hp = 0;
    req.creature_record = 100;
    int r = dm2_v1_calc_player_attack_damage(&req, &receipt);
    assert(r == 0);
    assert(receipt.miss == 1);

    printf("  PASS: calc_damage_dead_hero\n");
}

static void test_calc_damage_no_creature(void)
{
    DM2_V1_CalcAttackDamageReceipt receipt;
    DM2_V1_CalcAttackDamageRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_hp = 100;
    req.creature_record = -1;
    int r = dm2_v1_calc_player_attack_damage(&req, &receipt);
    assert(r == 0);
    assert(receipt.miss == 1);

    printf("  PASS: calc_damage_no_creature\n");
}

static void test_calc_damage_valid(void)
{
    DM2_V1_CalcAttackDamageReceipt receipt;
    DM2_V1_CalcAttackDamageRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 0;
    req.hero_hp = 100;
    req.hero_dexterity = 30;
    req.hero_strength = 40;
    req.creature_record = 0x1000;
    req.creature_defense = 10;
    req.power_base = 20;
    int r = dm2_v1_calc_player_attack_damage(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    assert(receipt.hit == 1);

    printf("  PASS: calc_damage_valid\n");
}

static void test_wound_null_safety(void)
{
    DM2_V1_WoundPlayerReceipt receipt;
    int r = dm2_v1_wound_player(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_wound_player(NULL, NULL);
    assert(r == 0);

    printf("  PASS: wound_null_safety\n");
}

static void test_wound_zero_damage(void)
{
    DM2_V1_WoundPlayerReceipt receipt;
    DM2_V1_WoundPlayerRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 0;
    req.wound_amount = 0;
    req.hero_hp = 100;
    int r = dm2_v1_wound_player(&req, &receipt);
    assert(r == 0);
    assert(receipt.hero_wounded == 0);

    printf("  PASS: wound_zero_damage\n");
}

static void test_wound_lethal(void)
{
    DM2_V1_WoundPlayerReceipt receipt;
    DM2_V1_WoundPlayerRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 0;
    req.wound_amount = 200;
    req.hero_hp = 50;
    int r = dm2_v1_wound_player(&req, &receipt);
    assert(r == 1);
    assert(receipt.hero_wounded == 1);
    assert(receipt.hero_killed == 1);
    assert(receipt.hp_remaining == 0);

    printf("  PASS: wound_lethal\n");
}

static void test_wound_survivable(void)
{
    DM2_V1_WoundPlayerReceipt receipt;
    DM2_V1_WoundPlayerRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 1;
    req.wound_amount = 20;
    req.hero_hp = 100;
    int r = dm2_v1_wound_player(&req, &receipt);
    assert(r == 1);
    assert(receipt.hero_wounded == 1);
    assert(receipt.hero_killed == 0);
    assert(receipt.hp_remaining == 80);
    assert(receipt.damage_dealt == 20);

    printf("  PASS: wound_survivable\n");
}

static void test_wound_dead_hero(void)
{
    DM2_V1_WoundPlayerReceipt receipt;
    DM2_V1_WoundPlayerRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 0;
    req.wound_amount = 10;
    req.hero_hp = 0;
    int r = dm2_v1_wound_player(&req, &receipt);
    assert(r == 0);

    printf("  PASS: wound_dead_hero\n");
}

static void test_wound_invalid_index(void)
{
    DM2_V1_WoundPlayerReceipt receipt;
    DM2_V1_WoundPlayerRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 5;
    req.wound_amount = 10;
    req.hero_hp = 100;
    int r = dm2_v1_wound_player(&req, &receipt);
    assert(r == 0);

    printf("  PASS: wound_invalid_index\n");
}

static void test_attack_party_null_safety(void)
{
    DM2_V1_AttackPartyReceipt receipt;
    int r = dm2_v1_attack_party(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_attack_party(NULL, NULL);
    assert(r == 0);

    printf("  PASS: attack_party_null_safety\n");
}

static void test_attack_party_zero_damage(void)
{
    DM2_V1_AttackPartyReceipt receipt;
    DM2_V1_AttackPartyRequest req;
    memset(&req, 0, sizeof(req));
    req.base_damage = 0;
    req.heroes_in_party = 4;
    int r = dm2_v1_attack_party(&req, &receipt);
    assert(r == 0);

    printf("  PASS: attack_party_zero_damage\n");
}

static void test_attack_party_valid(void)
{
    DM2_V1_AttackPartyReceipt receipt;
    DM2_V1_AttackPartyRequest req;
    memset(&req, 0, sizeof(req));
    req.base_damage = 50;
    req.damage_type = DM2_DMG_PHYSICAL;
    req.heroes_in_party = 4;
    int r = dm2_v1_attack_party(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);

    printf("  PASS: attack_party_valid\n");
}

int main(void)
{
    printf("test_dm2_v1_combat_damage_pc34_compat:\n");
    test_calc_damage_null_safety();
    test_calc_damage_dead_hero();
    test_calc_damage_no_creature();
    test_calc_damage_valid();
    test_wound_null_safety();
    test_wound_zero_damage();
    test_wound_lethal();
    test_wound_survivable();
    test_wound_dead_hero();
    test_wound_invalid_index();
    test_attack_party_null_safety();
    test_attack_party_zero_damage();
    test_attack_party_valid();
    printf("All combat damage tests passed.\n");
    return 0;
}
