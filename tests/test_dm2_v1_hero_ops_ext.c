/* Test DM2 V1 hero operations — extended. */

#include "dm2_v1_hero_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM2_V1_HeroState g_heroes[4];

static DM2_V1_HeroState *mock_get_hero(void *ctx, int idx)
{
    (void)ctx;
    if (idx < 0 || idx >= 4) return NULL;
    return &g_heroes[idx];
}

static int g_enchant_called;
static void mock_proceed_ench(void *ctx, uint16_t mask, uint8_t aura,
                              int16_t power, uint16_t duration)
{
    (void)ctx; (void)mask; (void)aura; (void)power; (void)duration;
    g_enchant_called = 1;
}

static void test_hero_cast_enchantment(void)
{
    memset(g_heroes, 0, sizeof(g_heroes));
    g_heroes[0].cur_mp = 10;
    g_enchant_called = 0;
    DM2_V1_CastEnchantCallbacks cb = { mock_proceed_ench };
    int r = dm2_v1_hero_cast_enchantment(&g_heroes[0], 2, 64, 1, &cb, NULL);
    assert(r == 1);
    assert(g_heroes[0].cur_mp == 6);
    assert(g_enchant_called == 1);

    /* Not enough MP: halves power */
    g_heroes[0].cur_mp = 2;
    g_enchant_called = 0;
    r = dm2_v1_hero_cast_enchantment(&g_heroes[0], 2, 64, 1, &cb, NULL);
    assert(r == 0);
    assert(g_heroes[0].cur_mp == 0);
    assert(g_enchant_called == 1);

    /* No MP at all */
    r = dm2_v1_hero_cast_enchantment(&g_heroes[0], 2, 64, 1, &cb, NULL);
    assert(r == 0);
    printf("  PASS: hero_cast_enchantment\n");
}

static int g_shoot_called;
static void mock_shoot(void *ctx, uint16_t item, int16_t x, int16_t y,
                       int dir, int face, uint8_t dmg, uint8_t ke, uint8_t pw)
{
    (void)ctx; (void)item; (void)x; (void)y; (void)dir; (void)face;
    (void)dmg; (void)ke; (void)pw;
    g_shoot_called = 1;
}

static void test_shoot_champion_missile(void)
{
    g_shoot_called = 0;
    DM2_V1_ShootMissileState state = { 0, 0, 10, 20 };
    DM2_V1_ShootMissileCallbacks cb = { mock_shoot };
    dm2_v1_shoot_champion_missile(&state, 0x100, 50, 30, 10, &cb, NULL);
    assert(g_shoot_called == 1);
    printf("  PASS: shoot_champion_missile\n");
}

static void test_cast_champion_missile_spell(void)
{
    memset(g_heroes, 0, sizeof(g_heroes));
    g_heroes[0].cur_mp = 20;
    g_shoot_called = 0;
    DM2_V1_ShootMissileState state = { 1, 2, 5, 5 };
    DM2_V1_ShootMissileCallbacks cb = { mock_shoot };
    int r = dm2_v1_cast_champion_missile_spell(
        &g_heroes[0], &state, 0x200, 30, 10, &cb, NULL);
    assert(r == 1);
    assert(g_heroes[0].cur_mp == 10);
    assert(g_shoot_called == 1);

    /* Not enough MP */
    g_heroes[0].cur_mp = 5;
    r = dm2_v1_cast_champion_missile_spell(
        &g_heroes[0], &state, 0x200, 30, 10, &cb, NULL);
    assert(r == 0);
    printf("  PASS: cast_champion_missile_spell\n");
}

static int16_t mock_get_player_at(void *ctx, uint8_t pos)
{
    (void)ctx;
    if (pos == 0) return 0;
    return -1;
}

static int g_post_called;
static void mock_post(void *ctx, int idx) { (void)ctx; (void)idx; g_post_called = 1; }

static void test_bring_champion_to_life(void)
{
    memset(g_heroes, 0, sizeof(g_heroes));
    g_heroes[0].max_hp = 100;
    g_heroes[0].cur_hp = 0;
    g_heroes[0].party_pos = 0xFF;
    g_post_called = 0;
    DM2_V1_ResurrectCallbacks cb = {
        4, mock_get_hero, mock_get_player_at, 0, mock_post
    };
    dm2_v1_hero_bring_to_life(0, &cb, NULL);
    assert(g_heroes[0].cur_hp > 0);
    assert(g_heroes[0].max_hp < 100);
    assert(g_heroes[0].max_hp >= 25);
    assert((g_heroes[0].hero_flag & DM2_V1_HERO_FLAG_4000) != 0);
    assert(g_post_called == 1);
    printf("  PASS: bring_champion_to_life\n");
}

static int g_equipped_slot;
static int mock_is_fit(void *ctx, uint16_t item, int slot, int mode)
{
    (void)ctx; (void)item; (void)mode;
    return slot >= 2 ? 1 : 0;
}

static void mock_equip(void *ctx, int hero_idx, uint16_t item, int slot)
{
    (void)ctx; (void)hero_idx; (void)item;
    g_equipped_slot = slot;
}

static void test_add_item_to_player(void)
{
    int16_t items[30];
    memset(items, 0xFF, sizeof(items));
    items[0] = 0x10;
    items[1] = 0x20;
    /* Slot 2 is empty (-1) */

    DM2_V1_SlotGroup groups[] = {
        { 0, 1, -1 },
        { 2, 5, -1 },
    };
    g_equipped_slot = -1;
    DM2_V1_AddItemCallbacks cb = { mock_is_fit, mock_equip };
    int r = dm2_v1_add_item_to_player(0, items, 30, 0x100, 5, groups, 2, &cb, NULL);
    assert(r == 1);
    assert(g_equipped_slot == 2);
    printf("  PASS: add_item_to_player\n");
}

static int16_t mock_hand_item(void *ctx, int hero, int hand)
{
    (void)ctx;
    if (hero == 0 && hand == 0) return 0x50;
    return -1;
}

static uint16_t mock_item_flags(void *ctx, uint16_t item)
{
    (void)ctx; (void)item;
    return 0x10;
}

static int g_charge_calls;
static int16_t mock_add_charge(void *ctx, uint16_t item, int16_t delta)
{
    (void)ctx; (void)item;
    g_charge_calls++;
    if (delta == 0) return 5;
    return 4;
}

static void mock_set_imp(void *ctx, uint16_t item, int val)
{
    (void)ctx; (void)item; (void)val;
}

static int g_recalc_called;
static void mock_recalc(void *ctx) { (void)ctx; g_recalc_called = 1; }

static void test_burn_lighting_items(void)
{
    g_charge_calls = 0;
    g_recalc_called = 0;
    DM2_V1_BurnLightCallbacks cb = {
        1, NULL, mock_hand_item, mock_item_flags, mock_add_charge,
        mock_set_imp, mock_recalc
    };
    int r = dm2_v1_burn_player_lighting_items(&cb, NULL);
    assert(r == 1);
    assert(g_recalc_called == 1);
    printf("  PASS: burn_lighting_items\n");
}

static int mock_burn_is_last_hero(void *ctx, int hero_idx)
{ (void)ctx; return hero_idx == 1; }

static void test_burn_lighting_items_excludes_last_hero(void)
{
    g_charge_calls = 0;
    g_recalc_called = 0;
    DM2_V1_BurnLightCallbacks cb = {
        2, mock_burn_is_last_hero, mock_hand_item, mock_item_flags,
        mock_add_charge, mock_set_imp, mock_recalc
    };
    /* hero_count = 2; is_last_hero(1) == true excludes hero index 1,
     * so only hero 0's hand item (which mock_hand_item burns) is
     * processed and mock_add_charge should still be called for it. */
    int r = dm2_v1_burn_player_lighting_items(&cb, NULL);
    assert(r == 1);
    assert(g_charge_calls > 0);
    printf("  PASS: burn_lighting_items_excludes_last_hero\n");
}

static int g_drop_count;
static int16_t mock_remove_poss(void *ctx, int hero, int slot)
{
    (void)ctx; (void)hero;
    if (slot == 0 || slot == 2) return (int16_t)slot;
    return -1;
}

static void mock_move_rec(void *ctx, uint16_t rec, int16_t x, int16_t y, uint8_t pos)
{
    (void)ctx; (void)rec; (void)x; (void)y; (void)pos;
    g_drop_count++;
}

static void test_drop_player_items(void)
{
    g_drop_count = 0;
    uint8_t order[] = { 0, 1, 2, 3 };
    DM2_V1_DropItemsCallbacks cb = { mock_remove_poss, mock_move_rec };
    int r = dm2_v1_drop_player_items(0, 1, 5, 6, order, 4, &cb, NULL);
    assert(r == 2);
    assert(g_drop_count == 2);
    printf("  PASS: drop_player_items\n");
}

static int16_t mock_remove_hand(void *ctx) { (void)ctx; return 0x42; }

static void test_put_item_to_player(void)
{
    memset(g_heroes, 0, sizeof(g_heroes));
    g_heroes[0].cur_hp = 50;
    int16_t items[30];
    for (int i = 0; i < 30; i++) items[i] = (int16_t)(i < 15 ? i : -1);
    g_equipped_slot = -1;
    DM2_V1_PutItemCallbacks cb = { 0x42, mock_remove_hand, mock_equip };
    int r = dm2_v1_put_item_to_player(0, &g_heroes[0], items, 30, &cb, NULL);
    assert(r == 1);
    assert(g_equipped_slot == 15);
    printf("  PASS: put_item_to_player\n");
}

int main(void)
{
    printf("test_dm2_v1_hero_ops_ext:\n");
    test_hero_cast_enchantment();
    test_shoot_champion_missile();
    test_cast_champion_missile_spell();
    test_bring_champion_to_life();
    test_add_item_to_player();
    test_burn_lighting_items();
    test_burn_lighting_items_excludes_last_hero();
    test_drop_player_items();
    test_put_item_to_player();
    printf("All hero_ops_ext tests passed.\n");
    return 0;
}
