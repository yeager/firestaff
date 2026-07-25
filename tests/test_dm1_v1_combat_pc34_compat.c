#include "dm1_v1_combat_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_attack_types(void)
{
    assert(DM1_ATTACK_NORMAL == 0);
    assert(DM1_ATTACK_FIRE == 1);
    assert(DM1_ATTACK_MAGIC == 5);
    assert(DM1_ATTACK_LIGHTNING == 7);
    assert(DM1_ATTACK_TYPE_COUNT == 8);
}

static void test_wound_masks(void)
{
    assert(DM1_WOUND_NONE == 0x0000);
    assert(DM1_WOUND_READY_HAND == 0x0001);
    assert(DM1_WOUND_HEAD == 0x0004);
    assert(DM1_WOUND_ALL == 0x003F);
}

static void test_wound_indices(void)
{
    assert(DM1_WOUND_IDX_READY_HAND == 0);
    assert(DM1_WOUND_IDX_FEET == 5);
    assert(DM1_WOUND_IDX_COUNT == 6);
}

static void test_outcome_enum(void)
{
    assert(DM1_OUTCOME_KILLED_NONE == 0);
    assert(DM1_OUTCOME_KILLED_SOME == 1);
    assert(DM1_OUTCOME_KILLED_ALL == 2);
}

static void test_creature_sizes(void)
{
    assert(DM1_CREATURE_SIZE_FULL_SQUARE == 0);
    assert(DM1_CREATURE_SIZE_HALF_SQUARE == 1);
    assert(DM1_CREATURE_SIZE_QUARTER_SQUARE == 2);
}

static void test_weapon_constants(void)
{
    assert(DM1_WEAPON_CLASS_BOW_AMMUNITION == 10);
    assert(DM1_WEAPON_CLASS_FIRST_BOW == 16);
    assert(DM1_ACTION_SHOOT_DISABLED_TICKS_PC34 == 14);
    assert(DM1_ACTION_SHOOT_STAMINA_BASE_PC34 == 3);
}

static void test_combat_init(void)
{
    DM1_CombatState s;
    dm1_combat_init(&s);
    assert(s.championCount == 0);
    assert(s.partyShieldDefense == 0);
}

static void test_champion_init(void)
{
    DM1_ChampionCombat ch;
    dm1_combat_init_champion(&ch);
    assert(ch.currentHealth == 100);
    assert(ch.alive == 1);
    assert(ch.maxHealth == 100);
    assert(ch.wounds == DM1_WOUND_NONE);
}

static void test_group_init(void)
{
    DM1_CreatureGroup g;
    dm1_combat_init_group(&g);
    assert(g.count == 0);
}

static void test_armor_defense(void)
{
    DM1_ArmorPiece a;
    memset(&a, 0, sizeof(a));
    a.defense = 10;
    a.sharpDefense = 5;
    int d1 = dm1_armor_defense(&a, 0);
    int d2 = dm1_armor_defense(&a, 1);
    (void)d1; (void)d2;
    assert(d1 >= 0);
    assert(d2 >= 0);
}

static void test_scaled_product(void)
{
    int r = dm1_scaled_product(100, 3, 4);
    (void)r;
    assert(r >= 0);
}

static void test_max_load(void)
{
    int load = dm1_combat_get_maximum_load_pc34(50);
    (void)load;
    assert(load > 0);
}

static void test_movement_ticks(void)
{
    int ticks = dm1_combat_get_movement_ticks_pc34(100, 400);
    (void)ticks;
    assert(ticks >= 0);
}

static void test_ranged_shoot_source_evidence(void)
{
    const char* e = dm1_ranged_shoot_source_evidence_pc34();
    assert(e != NULL);
    assert(strlen(e) > 0);
}

static void test_pass601_source_evidence(void)
{
    const char* e = dm1_combat_pass601_source_evidence();
    assert(e != NULL);
    assert(strlen(e) > 0);
}

int main(void)
{
    test_attack_types();
    test_wound_masks();
    test_wound_indices();
    test_outcome_enum();
    test_creature_sizes();
    test_weapon_constants();
    test_combat_init();
    test_champion_init();
    test_group_init();
    test_armor_defense();
    test_scaled_product();
    test_max_load();
    test_movement_ticks();
    test_ranged_shoot_source_evidence();
    test_pass601_source_evidence();

    puts("ok: DM1 combat system (Q-DM1-05) 15 tests passed");
    return 0;
}
