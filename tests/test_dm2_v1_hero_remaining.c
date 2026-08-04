/* Test DM2 V1 hero remaining functions (pass1107). */

#include "dm2_v1_hero_ops_pc34_compat.h"
#include "dm2_v1_hero_stats_pc34_compat.h"
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

static void reset_heroes(void)
{
    memset(g_heroes, 0, sizeof(g_heroes));
    for (int i = 0; i < 4; i++) {
        g_heroes[i].cur_hp = 100;
        g_heroes[i].max_hp = 200;
        g_heroes[i].cur_stamina = 100;
        g_heroes[i].max_stamina = 200;
        g_heroes[i].party_pos = (uint8_t)i;
    }
}

/* ---- hero_init ---- */
static void test_hero_init(void)
{
    g_heroes[0].cur_hp = 50;
    g_heroes[0].hero_flag = 0x1234;
    dm2_v1_hero_init(&g_heroes[0]);
    assert(g_heroes[0].cur_hp == 0);
    assert(g_heroes[0].hero_flag == 0);
    assert(g_heroes[0].max_hp == 0);
    printf("  PASS: hero_init\n");
}

/* ---- party_init ---- */
static void test_party_init(void)
{
    reset_heroes();
    int hero_count = 4, mode = 2, acthero = 3;
    DM2_V1_PartyInitCallbacks cb = { 4, mock_get_hero };
    dm2_v1_party_init(&hero_count, &mode, &acthero, &cb, NULL);
    assert(hero_count == 0);
    assert(mode == 0);
    assert(acthero == 0);
    assert(g_heroes[0].cur_hp == 0);
    printf("  PASS: party_init\n");
}

/* ---- party_rotate ---- */
static void test_party_rotate(void)
{
    reset_heroes();
    g_heroes[0].party_pos = 0; g_heroes[0].absdir = 0;
    g_heroes[1].party_pos = 1; g_heroes[1].absdir = 0;
    int16_t party_dir = 0;
    int16_t party_absdir = 0;
    DM2_V1_RotateCallbacks cb = {
        2, mock_get_hero, &party_dir, &party_absdir, 0, 0
    };
    dm2_v1_party_rotate(1, &cb, NULL);
    assert(party_dir == 1);
    assert(g_heroes[0].party_pos == 1);
    assert(g_heroes[0].absdir == 1);
    assert(g_heroes[1].party_pos == 2);
    /* No-op if same dir */
    dm2_v1_party_rotate(1, &cb, NULL);
    assert(g_heroes[0].party_pos == 1); /* unchanged */
    printf("  PASS: party_rotate\n");
}

/* ---- set_hero_flags ---- */
static void test_set_hero_flags(void)
{
    reset_heroes();
    dm2_v1_party_set_hero_flags(4, mock_get_hero, NULL);
    for (int i = 0; i < 4; i++)
        assert((g_heroes[i].hero_flag & DM2_V1_HERO_FLAG_4000) != 0);
    printf("  PASS: set_hero_flags\n");
}

/* ---- reset_squad_dir ---- */
static void test_reset_squad_dir(void)
{
    reset_heroes();
    g_heroes[0].absdir = 3;
    g_heroes[1].absdir = 1;
    dm2_v1_reset_squad_dir(4, 2, mock_get_hero, NULL);
    for (int i = 0; i < 4; i++)
        assert(g_heroes[i].absdir == 2);
    printf("  PASS: reset_squad_dir\n");
}

/* ---- select_champion_leader ---- */
static void test_select_champion_leader(void)
{
    reset_heroes();
    int16_t event_hero = 0;
    DM2_V1_SelectLeaderCallbacks cb = { 4, mock_get_hero, &event_hero, 0 };
    dm2_v1_select_champion_leader(2, &cb, NULL);
    assert(event_hero == 2);
    assert((g_heroes[0].hero_flag & 0x1400) != 0); /* old leader flagged */
    assert((g_heroes[2].hero_flag & 0x1400) != 0); /* new leader flagged */

    /* Same leader: no-op */
    g_heroes[2].hero_flag = 0;
    dm2_v1_select_champion_leader(2, &cb, NULL);
    assert(g_heroes[2].hero_flag == 0);

    /* Dead hero: rejected */
    g_heroes[3].cur_hp = 0;
    dm2_v1_select_champion_leader(3, &cb, NULL);
    assert(event_hero == 2); /* unchanged */
    printf("  PASS: select_champion_leader\n");
}

/* ---- equip_item_to_hand ---- */
static void test_equip_item_to_hand(void)
{
    uint16_t items[30];
    memset(items, 0xFF, sizeof(items));
    DM2_V1_EquipCallbacks cb = { items, 30, NULL, 0, NULL };
    dm2_v1_equip_item_to_hand(0, 0x1234, 5, &cb, NULL);
    assert(items[5] == (0x1234 & 0x3FFF));
    /* -1 item: no-op */
    dm2_v1_equip_item_to_hand(0, 0xFFFF, 3, &cb, NULL);
    assert(items[3] == 0xFFFF); /* unchanged */
    printf("  PASS: equip_item_to_hand\n");
}

/* ---- remove_possession ---- */
static void test_remove_possession(void)
{
    uint16_t items[30];
    for (int i = 0; i < 30; i++) items[i] = 0xFFFF;
    items[5] = 0x42;
    DM2_V1_RemovePossessionCallbacks cb = {
        items, 30, NULL, 0, NULL, NULL, 0, 0
    };
    int16_t r = dm2_v1_remove_possession(0, 5, &cb, NULL);
    assert(r == 0x42);
    assert(items[5] == 0xFFFF);
    /* Empty slot returns -1 */
    r = dm2_v1_remove_possession(0, 5, &cb, NULL);
    assert(r == -1);
    printf("  PASS: remove_possession\n");
}

/* ---- adjust_hand_cooldown ---- */
static void test_adjust_hand_cooldown(void)
{
    DM2_V1_HeroCooldownState state;
    memset(&state, 0, sizeof(state));
    /* Single hand */
    dm2_v1_adjust_hand_cooldown(&state, 20, 0, 0);
    assert(state.hand_cooldown[0] > 0);
    assert(state.hand_cooldown[1] == 0);
    /* All hands */
    memset(&state, 0, sizeof(state));
    dm2_v1_adjust_hand_cooldown(&state, 40, -1, 0);
    assert(state.hand_cooldown[0] > 0);
    assert(state.hand_cooldown[1] > 0);
    assert(state.hand_cooldown[2] > 0);
    /* Easy mode halves */
    DM2_V1_HeroCooldownState easy;
    memset(&easy, 0, sizeof(easy));
    dm2_v1_adjust_hand_cooldown(&easy, 40, 0, 1);
    assert(easy.hand_cooldown[0] < state.hand_cooldown[0]);
    printf("  PASS: adjust_hand_cooldown\n");
}

/* ---- get_party_special_force ---- */
static int16_t mock_weight(void *ctx, int idx)
{
    (void)ctx; (void)idx;
    return 50; /* weight > 0 → returns 0x32 */
}

static void test_get_party_special_force(void)
{
    reset_heroes();
    DM2_V1_SpecialForceCallbacks cb = { 4, mock_get_hero, mock_weight };
    int force = dm2_v1_get_party_special_force(&cb, NULL);
    /* 4 heroes alive with weight > 0 → 4 * 0x32 = 200 */
    assert(force == 4 * 0x32);
    /* Dead hero contributes 0 */
    g_heroes[3].cur_hp = 0;
    force = dm2_v1_get_party_special_force(&cb, NULL);
    assert(force == 3 * 0x32);
    printf("  PASS: get_party_special_force\n");
}

/* ---- attack_party ---- */
static int16_t g_wound_amounts[4];
static int g_wound_count;

static int16_t mock_wound_for_attack(void *ctx, int hero_idx, int16_t damage,
                                      int body_parts, int damage_type)
{
    (void)ctx; (void)body_parts; (void)damage_type;
    if (hero_idx >= 0 && hero_idx < 4)
        g_wound_amounts[hero_idx] = damage;
    g_wound_count++;
    return damage;
}

static int16_t mock_rand16(void *ctx, int16_t max)
{
    (void)ctx;
    return max > 0 ? (int16_t)(max / 2) : 0;
}

static int16_t mock_max_fn(void *ctx, int16_t a, int16_t b)
{
    (void)ctx;
    return a > b ? a : b;
}

static void test_attack_party(void)
{
    reset_heroes();
    g_wound_count = 0;
    memset(g_wound_amounts, 0, sizeof(g_wound_amounts));
    DM2_V1_HeroAttackPartyCallbacks cb = {
        4, mock_wound_for_attack, mock_rand16, mock_max_fn
    };
    int mask = dm2_v1_hero_attack_party(100, 0x3F, 1, &cb, NULL);
    assert(g_wound_count == 4);
    assert(mask == 0xF); /* all 4 wounded */
    /* Zero damage: no-op */
    assert(dm2_v1_hero_attack_party(0, 0, 0, &cb, NULL) == 0);
    printf("  PASS: attack_party\n");
}

/* ---- remove_object_from_hand ---- */
static void test_remove_object_from_hand(void)
{
    int16_t cursor = 0x55;
    int16_t extra = 10;
    int16_t weight = 5;
    int8_t ctype = 3;
    DM2_V1_RemoveFromHandCallbacks cb = {
        &cursor, &extra, &weight, &ctype, NULL, NULL, NULL, 0
    };
    int16_t r = dm2_v1_remove_object_from_hand(&cb, NULL);
    assert(r == 0x55);
    assert(cursor == -1);
    assert(extra == 0);
    assert(weight == 0);
    assert(ctype == -1);
    /* Empty hand */
    r = dm2_v1_remove_object_from_hand(&cb, NULL);
    assert(r == -1);
    printf("  PASS: remove_object_from_hand\n");
}

/* ---- process_players_damage ---- */
static int g_defeated_hero;
static void mock_defeated(void *ctx, int idx) { (void)ctx; g_defeated_hero = idx; }
static void mock_damage_timer(void *ctx, int idx, int16_t dmg) { (void)ctx; (void)idx; (void)dmg; }

static void test_process_players_damage(void)
{
    reset_heroes();
    int16_t pending[4] = {0, 50, 0, 300};
    uint16_t wounds[4] = {0, 0, 0, 0};
    g_defeated_hero = -1;
    DM2_V1_ProcessDamageCallbacks cb = {
        4, mock_get_hero, pending, wounds, mock_defeated, mock_damage_timer
    };
    dm2_v1_process_players_damage(&cb, NULL);
    /* Hero 1: 100-50=50 → survives */
    assert(g_heroes[1].cur_hp == 50);
    /* Hero 3: 100-300<0 → defeated */
    assert(g_defeated_hero == 3);
    /* Pending cleared */
    assert(pending[1] == 0);
    assert(pending[3] == 0);
    printf("  PASS: process_players_damage\n");
}

/* ---- player_defeated ---- */
static int g_drop_called, g_bones_called, g_cure_called;
static int g_leader_selected;
static int g_all_dead;

static void mock_drop(void *ctx, int idx) { (void)ctx; (void)idx; g_drop_called = 1; }
static void mock_bones(void *ctx, int idx, uint8_t pos) { (void)ctx; (void)idx; (void)pos; g_bones_called = 1; }
static void mock_cure(void *ctx, int idx) { (void)ctx; (void)idx; g_cure_called = 1; }
static void mock_select(void *ctx, int idx) { (void)ctx; g_leader_selected = idx; }
static void mock_all_dead(void *ctx) { (void)ctx; g_all_dead = 1; }

static void test_player_defeated(void)
{
    reset_heroes();
    g_heroes[0].poisoned_count = 2;
    g_drop_called = g_bones_called = g_cure_called = 0;
    g_leader_selected = -1;
    g_all_dead = 0;
    DM2_V1_PlayerDefeatedCallbacks cb = {
        4, mock_get_hero, mock_drop, mock_bones, mock_cure,
        mock_select, NULL, mock_all_dead, NULL, 0, 0, 0
    };
    dm2_v1_player_defeated(0, &cb, NULL);
    assert(g_heroes[0].cur_hp == 0);
    assert(g_drop_called == 1);
    assert(g_bones_called == 1);
    assert(g_cure_called == 1); /* was poisoned */
    assert(g_leader_selected == 1); /* next alive hero */
    assert(g_all_dead == 0);

    /* All dead */
    for (int i = 0; i < 4; i++) g_heroes[i].cur_hp = 0;
    g_heroes[2].cur_hp = 50;
    g_all_dead = 0;
    dm2_v1_player_defeated(2, &cb, NULL);
    assert(g_all_dead == 1);
    printf("  PASS: player_defeated\n");
}

/* ---- use_dexterity_attribute ---- */
static void test_use_dexterity_attribute(void)
{
    DM2_V1_UseDexterityResult out;
    /* Normal case: adj_dex=80, no weight, not sleeping */
    dm2_v1_use_dexterity_attribute(80, 0, 100, 0, 3, 2, 1, &out);
    assert(out.valid == 1);
    assert(out.value > 0);
    assert(out.value <= 100);
    /* Sleeping halves */
    DM2_V1_UseDexterityResult out2;
    dm2_v1_use_dexterity_attribute(80, 0, 100, 1, 3, 2, 1, &out2);
    assert(out2.value <= out.value);
    printf("  PASS: use_dexterity_attribute\n");
}

/* ---- use_luck ---- */
static void test_use_luck(void)
{
    DM2_V1_HeroStats stats;
    memset(&stats, 0, sizeof(stats));
    stats.ability[DM2_V1_HERO_ABILITY_LUCK].current = 100;
    stats.ability[DM2_V1_HERO_ABILITY_LUCK].maximum = 120;
    DM2_V1_HeroUseLuckResult out;
    /* Randbit=1, rand100=80 vs threshold=50 → success (80>50) */
    dm2_v1_hero_use_luck(&stats, 50, 1, 80, 0, &out);
    assert(out.valid == 1);
    assert(out.result == 1);
    assert(out.new_luck == 98); /* success: luck -= 2 */
    /* Failure case: randbit=0, rand_ability=30 vs threshold=50 */
    stats.ability[DM2_V1_HERO_ABILITY_LUCK].current = 100;
    dm2_v1_hero_use_luck(&stats, 50, 0, 0, 30, &out);
    assert(out.result == 0);
    assert(out.new_luck == 102); /* failure: luck += 2 */
    printf("  PASS: use_luck\n");
}

/* ---- adjust_ability ---- */
static void test_adjust_ability(void)
{
    DM2_V1_HeroStats stats;
    memset(&stats, 0, sizeof(stats));
    stats.ability[0].current = 50;
    stats.ability[0].maximum = 50;
    /* Small delta: no diminishing returns */
    dm2_v1_hero_adjust_ability(&stats, 0, 10);
    assert(stats.ability[0].current == 60);
    /* Clamp to 220 */
    stats.ability[0].current = 215;
    stats.ability[0].maximum = 100;
    dm2_v1_hero_adjust_ability(&stats, 0, 30);
    assert(stats.ability[0].current <= 220);
    /* Clamp to 10 */
    stats.ability[0].current = 15;
    stats.ability[0].maximum = 100;
    dm2_v1_hero_adjust_ability(&stats, 0, -50);
    assert(stats.ability[0].current >= 10);
    printf("  PASS: adjust_ability\n");
}

/* ---- calc_player_weight ---- */
static int16_t mock_item_weight(void *ctx, uint16_t item)
{
    (void)ctx;
    if (item == 0xFFFF) return 0;
    return 5;
}

static int mock_is_chest(void *ctx, uint16_t item)
{
    (void)ctx; (void)item;
    return 0;
}

static void test_calc_player_weight(void)
{
    reset_heroes();
    uint16_t items[30];
    for (int i = 0; i < 30; i++) items[i] = 0xFFFF;
    items[0] = 10;
    items[1] = 20;
    DM2_V1_CalcWeightCallbacks cb = {
        mock_item_weight, mock_is_chest,
        items, 30, NULL, 0, 0, 0
    };
    int16_t w = dm2_v1_calc_player_weight(&g_heroes[0], &cb, NULL);
    assert(w == 10); /* 2 items * 5 weight each */
    assert((g_heroes[0].hero_flag & 0x1000) != 0);
    printf("  PASS: calc_player_weight\n");
}

int main(void)
{
    printf("test_dm2_v1_hero_remaining:\n");
    test_hero_init();
    test_party_init();
    test_party_rotate();
    test_set_hero_flags();
    test_reset_squad_dir();
    test_select_champion_leader();
    test_equip_item_to_hand();
    test_remove_possession();
    test_adjust_hand_cooldown();
    test_get_party_special_force();
    test_attack_party();
    test_remove_object_from_hand();
    test_process_players_damage();
    test_player_defeated();
    test_use_dexterity_attribute();
    test_use_luck();
    test_adjust_ability();
    test_calc_player_weight();
    printf("All hero_remaining tests passed.\n");
    return 0;
}
