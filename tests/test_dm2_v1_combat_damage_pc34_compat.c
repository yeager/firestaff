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
    int r = dm2_v1_calc_player_attack_damage_receipt(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_calc_player_attack_damage_receipt(NULL, NULL);
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
    int r = dm2_v1_calc_player_attack_damage_receipt(&req, &receipt);
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
    int r = dm2_v1_calc_player_attack_damage_receipt(&req, &receipt);
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
    req.power_base = 64;
    req.hero_ability = 40;
    req.hero_max_load = 200;
    req.item_weight = 20;
    req.hero_skill_level = 5;
    req.skill_type = 0;
    req.stamina_adj = 60;
    req.creature_armor = 5;
    req.rand_hit = 8;
    req.rand_defense = 0;
    req.rand_armor = 0;
    req.party_level = 3;
    req.creature_armor_mult = 4;
    int r = dm2_v1_calc_player_attack_damage_receipt(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 0);
    assert(receipt.hit == 1);
    assert(receipt.final_damage > 0);

    printf("  PASS: calc_damage_valid\n");
}

static void test_calc_damage_miss_by_defense(void)
{
    DM2_V1_CalcAttackDamageReceipt receipt;
    DM2_V1_CalcAttackDamageRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 0;
    req.hero_hp = 100;
    req.hero_dexterity = 5;
    req.creature_record = 0x1000;
    req.creature_defense = 80;
    req.rand_hit = 0;
    req.rand_defense = 0;
    req.party_level = 10;
    int r = dm2_v1_calc_player_attack_damage_receipt(&req, &receipt);
    assert(r == 1);
    assert(receipt.miss == 1);
    assert(receipt.hit == 0);

    printf("  PASS: calc_damage_miss_by_defense\n");
}

static void test_calc_damage_poison(void)
{
    DM2_V1_CalcAttackDamageReceipt receipt;
    DM2_V1_CalcAttackDamageRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 0;
    req.hero_hp = 100;
    req.hero_dexterity = 50;
    req.creature_record = 0x1000;
    req.creature_defense = 5;
    req.power_base = 64;
    req.hero_ability = 40;
    req.hero_max_load = 200;
    req.item_weight = 20;
    req.stamina_adj = 60;
    req.creature_armor = 0;
    req.creature_poison_resist = 5;
    req.rand_poison = 3;
    req.rand_hit = 8;
    req.party_level = 3;
    int r = dm2_v1_calc_player_attack_damage_receipt(&req, &receipt);
    assert(r == 1);
    assert(receipt.hit == 1);
    assert(receipt.poison_applied == 1);

    printf("  PASS: calc_damage_poison\n");
}

static void test_calc_damage_skill_exp(void)
{
    DM2_V1_CalcAttackDamageReceipt receipt;
    DM2_V1_CalcAttackDamageRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 0;
    req.hero_hp = 100;
    req.hero_dexterity = 50;
    req.creature_record = 0x1000;
    req.creature_defense = 5;
    req.power_base = 64;
    req.hero_ability = 40;
    req.hero_max_load = 200;
    req.item_weight = 20;
    req.stamina_adj = 60;
    req.creature_armor = 0;
    req.creature_armor_mult = 8;
    req.rand_hit = 8;
    req.party_level = 3;
    int r = dm2_v1_calc_player_attack_damage_receipt(&req, &receipt);
    assert(r == 1);
    assert(receipt.hit == 1);
    assert(receipt.skill_exp_awarded >= 3);

    printf("  PASS: calc_damage_skill_exp\n");
}

static void test_wound_null_safety(void)
{
    DM2_V1_WoundPlayerReceipt receipt;
    int r = dm2_v1_wound_player_receipt(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_wound_player_receipt(NULL, NULL);
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
    int r = dm2_v1_wound_player_receipt(&req, &receipt);
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
    int r = dm2_v1_wound_player_receipt(&req, &receipt);
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
    int r = dm2_v1_wound_player_receipt(&req, &receipt);
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
    int r = dm2_v1_wound_player_receipt(&req, &receipt);
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
    int r = dm2_v1_wound_player_receipt(&req, &receipt);
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
    req.heroes_in_party = 2;
    req.hero_wound[0].hero_hp = 100;
    req.hero_wound[0].hero_index = 0;
    req.hero_wound[1].hero_hp = 80;
    req.hero_wound[1].hero_index = 1;
    req.random_values[0] = 3;
    req.random_values[1] = 7;
    int r = dm2_v1_attack_party(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 0);
    assert(receipt.heroes_wounded == 2);
    assert(receipt.per_hero_damage[0] > 0);
    assert(receipt.per_hero_damage[1] > 0);

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
    test_calc_damage_miss_by_defense();
    test_calc_damage_poison();
    test_calc_damage_skill_exp();
    printf("All combat damage tests passed.\n");
    return 0;
}
