/* Test DM2 V1 hero/champion operations (c_hero.cpp + SkWinCore2.cpp). */

#include "dm2_v1_hero_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM2_V1_HeroState g_heroes[4];
static int g_wound_hero, g_wound_dmg;
static int g_stam_hero;
static int16_t g_stam_amt;
static int g_poison_hero;
static int16_t g_poison_counters;
static uint32_t g_poison_delay;

static DM2_V1_HeroState *mock_get_hero(void *ctx, int idx)
{
    (void)ctx;
    if (idx < 0 || idx >= 4) return NULL;
    return &g_heroes[idx];
}

static void mock_wound(void *ctx, int hero, int dmg, int t, int f)
{
    (void)ctx; (void)t; (void)f;
    g_wound_hero = hero;
    g_wound_dmg = dmg;
}

static void mock_adjust_stam(void *ctx, int hero, int16_t amt)
{
    (void)ctx;
    g_stam_hero = hero;
    g_stam_amt = amt;
}

static void mock_queue_poison(void *ctx, int hero, int16_t cnt, uint32_t delay)
{
    (void)ctx;
    g_poison_hero = hero;
    g_poison_counters = cnt;
    g_poison_delay = delay;
}

static void reset_heroes(void)
{
    memset(g_heroes, 0, sizeof(g_heroes));
    for (int i = 0; i < 4; i++) {
        g_heroes[i].cur_hp = 100;
        g_heroes[i].max_hp = 200;
        g_heroes[i].cur_stamina = 100;
        g_heroes[i].max_stamina = 200;
        g_heroes[i].cur_water = 1000;
    }
}

static void test_adjust_stamina_normal(void)
{
    reset_heroes();
    DM2_V1_StaminaCallbacks cb = { mock_get_hero, mock_wound };
    int16_t r = dm2_v1_adjust_stamina(0, 30, &cb, NULL);
    assert(r == 30);
    assert(g_heroes[0].cur_stamina == 70);
    assert((g_heroes[0].hero_flag & DM2_V1_HERO_FLAG_0800) != 0);
    printf("  PASS: adjust_stamina_normal\n");
}

static void test_adjust_stamina_wounds(void)
{
    reset_heroes();
    g_heroes[1].cur_stamina = 10;
    g_wound_hero = -1;
    DM2_V1_StaminaCallbacks cb = { mock_get_hero, mock_wound };
    dm2_v1_adjust_stamina(1, 50, &cb, NULL);
    assert(g_heroes[1].cur_stamina == 0);
    assert(g_wound_hero == 1);
    assert(g_wound_dmg == 20); /* -(-40)/2 = 20 */
    printf("  PASS: adjust_stamina_wounds\n");
}

static void test_adjust_stamina_invalid(void)
{
    DM2_V1_StaminaCallbacks cb = { mock_get_hero, mock_wound };
    assert(dm2_v1_adjust_stamina(-1, 10, &cb, NULL) == -1);
    printf("  PASS: adjust_stamina_invalid\n");
}

/* ---- Cure poison ---- */
static uint8_t g_timer_types[8];
static uint8_t g_timer_actors[8];
static int g_timer_count;
static int g_deleted_timers[8];
static int g_delete_count;

static int mock_timer_count(void *ctx) { (void)ctx; return g_timer_count; }
static uint8_t mock_timer_type(void *ctx, int i) { (void)ctx; return g_timer_types[i]; }
static uint8_t mock_timer_actor(void *ctx, int i) { (void)ctx; return g_timer_actors[i]; }
static void mock_delete_timer(void *ctx, int i)
{
    (void)ctx;
    g_deleted_timers[g_delete_count++] = i;
}

static void test_cure_poison(void)
{
    reset_heroes();
    g_heroes[2].poison_value = 5;
    g_timer_types[0] = 0x46; g_timer_actors[0] = 2;
    g_timer_types[1] = 0x4B; g_timer_actors[1] = 2;
    g_timer_types[2] = 0x4B; g_timer_actors[2] = 1;
    g_timer_types[3] = 0x4B; g_timer_actors[3] = 2;
    g_timer_count = 4;
    g_delete_count = 0;

    DM2_V1_CurePoisonCallbacks cb = {
        mock_timer_count, mock_timer_type, mock_timer_actor,
        mock_delete_timer, mock_get_hero
    };
    assert(dm2_v1_cure_poison(2, &cb, NULL) == 1);
    assert(g_heroes[2].poison_value == 0);
    assert(g_delete_count == 2);
    printf("  PASS: cure_poison\n");
}

static void test_process_poison(void)
{
    reset_heroes();
    g_wound_hero = -1;
    g_poison_hero = -1;
    DM2_V1_ProcessPoisonCallbacks cb = {
        mock_get_hero, mock_wound, mock_adjust_stam, mock_queue_poison
    };
    assert(dm2_v1_process_poison(0, 3, &cb, NULL) == 1);
    assert(g_wound_hero == 0);
    assert(g_stam_hero == 0);
    assert(g_stam_amt == 48); /* 3 << 4 */
    assert(g_heroes[0].cur_water == 900);
    assert(g_poison_hero == 0);
    assert(g_poison_counters == 2);
    assert(g_poison_delay == 0x24);
    printf("  PASS: process_poison\n");
}

static void test_process_poison_last_tick(void)
{
    reset_heroes();
    DM2_V1_ProcessPoisonCallbacks cb = {
        mock_get_hero, mock_wound, mock_adjust_stam, mock_queue_poison
    };
    g_poison_hero = -1;
    assert(dm2_v1_process_poison(0, 1, &cb, NULL) == 1);
    assert(g_poison_hero == -1); /* no timer queued */
    printf("  PASS: process_poison_last_tick\n");
}

static void test_coin_wallet(void)
{
    int16_t wallet[4] = {0, 0, 0, 0};
    assert(dm2_v1_add_coin_to_wallet(wallet, 4, 0) == 1);
    assert(wallet[0] == 1);
    assert(dm2_v1_add_coin_to_wallet(wallet, 4, 2) == 1);
    assert(wallet[2] == 1);
    assert(dm2_v1_take_coin_from_wallet(wallet, 4, 0) == 1);
    assert(wallet[0] == 0);
    assert(dm2_v1_take_coin_from_wallet(wallet, 4, 0) == 0);
    assert(dm2_v1_add_coin_to_wallet(wallet, 4, 5) == 0); /* out of range */
    printf("  PASS: coin_wallet\n");
}

static void test_perform_turn_squad(void)
{
    reset_heroes();
    g_heroes[0].party_pos = 0;
    g_heroes[1].party_pos = 1;
    g_heroes[2].party_pos = 2;
    g_heroes[3].party_pos = 3;
    DM2_V1_SquadCallbacks cb = { 4, mock_get_hero };
    dm2_v1_perform_turn_squad(1, &cb, NULL);
    assert(g_heroes[0].party_pos == 1);
    assert(g_heroes[1].party_pos == 2);
    assert(g_heroes[2].party_pos == 3);
    assert(g_heroes[3].party_pos == 0);
    printf("  PASS: perform_turn_squad\n");
}

static void test_set_spelling_champion(void)
{
    int16_t spelling = -1;
    DM2_V1_SpellingState state = { &spelling };
    dm2_v1_set_spelling_champion(&state, 2);
    assert(spelling == 2);
    dm2_v1_set_spelling_champion(&state, -1);
    assert(spelling == -1);
    printf("  PASS: set_spelling_champion\n");
}

int main(void)
{
    printf("test_dm2_v1_hero_ops:\n");
    test_adjust_stamina_normal();
    test_adjust_stamina_wounds();
    test_adjust_stamina_invalid();
    test_cure_poison();
    test_process_poison();
    test_process_poison_last_tick();
    test_coin_wallet();
    test_perform_turn_squad();
    test_set_spelling_champion();
    printf("All hero_ops tests passed.\n");
    return 0;
}
