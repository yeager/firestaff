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
static int16_t g_timer_values[8];

static int16_t mock_timer_value(void *ctx, int i) { (void)ctx; return g_timer_values[i]; }
static void mock_set_timer_value(void *ctx, int i, int16_t v) { (void)ctx; g_timer_values[i] = v; }
static void mock_delete_timer(void *ctx, int i)
{
    (void)ctx;
    g_deleted_timers[g_delete_count++] = i;
}

static void test_cure_poison(void)
{
    reset_heroes();
    g_heroes[2].poison_value = 8; /* sum of matching timer values: 5+3 */
    g_heroes[2].poisoned_count = 2; /* two 0x4B timers for hero 2 */
    g_timer_types[0] = 0x46; g_timer_actors[0] = 2;
    g_timer_types[1] = 0x4B; g_timer_actors[1] = 2;
    g_timer_types[2] = 0x4B; g_timer_actors[2] = 1;
    g_timer_types[3] = 0x4B; g_timer_actors[3] = 2;
    g_timer_count = 4;
    g_delete_count = 0;

    g_timer_values[1] = 5;
    g_timer_values[3] = 3;
    DM2_V1_CurePoisonCallbacks cb = {
        mock_timer_count, mock_timer_type, mock_timer_actor,
        mock_timer_value, mock_set_timer_value,
        mock_delete_timer, mock_get_hero
    };
    assert(dm2_v1_cure_poison(2, 10, &cb, NULL) == 1);
    assert(g_heroes[2].poison_value == 0);
    assert(g_delete_count == 2);
    printf("  PASS: cure_poison\n");
}

static int mock_is_last_hero(void *ctx, int hero_idx)
{ (void)ctx; (void)hero_idx; return 0; }

static void test_process_poison(void)
{
    reset_heroes();
    g_wound_hero = -1;
    g_poison_hero = -1;
    DM2_V1_ProcessPoisonCallbacks cb = {
        mock_get_hero, mock_is_last_hero, mock_wound, mock_queue_poison
    };
    assert(dm2_v1_process_poison(0, 3, &cb, NULL) == 1);
    assert(g_wound_hero == 0);
    assert(g_poison_hero == 0);
    assert(g_poison_counters == 2);
    assert(g_poison_delay == 0x24);
    printf("  PASS: process_poison\n");
}

static void test_process_poison_last_tick(void)
{
    reset_heroes();
    DM2_V1_ProcessPoisonCallbacks cb = {
        mock_get_hero, mock_is_last_hero, mock_wound, mock_queue_poison
    };
    g_poison_hero = -1;
    assert(dm2_v1_process_poison(0, 1, &cb, NULL) == 1);
    assert(g_poison_hero == -1); /* no timer queued */
    printf("  PASS: process_poison_last_tick\n");
}

/* ---- Coin wallet: mock container item-record store (NEXT-linked). ---- */
#define MOCK_MAX_ITEMS 32
static uint16_t g_item_db_type[MOCK_MAX_ITEMS];
static uint16_t g_item_denom[MOCK_MAX_ITEMS];
static uint8_t g_item_charge[MOCK_MAX_ITEMS];
static uint16_t g_item_next[MOCK_MAX_ITEMS];
static uint16_t g_container_head[MOCK_MAX_ITEMS];
static uint16_t g_next_alloc_ref;
static int g_dealloc_calls;
static uint16_t g_last_dealloc;

static void reset_wallet_mock(void)
{
    for (int i = 0; i < MOCK_MAX_ITEMS; i++) {
        g_item_db_type[i] = 0;
        g_item_denom[i] = 0;
        g_item_charge[i] = 0;
        g_item_next[i] = DM2_V1_OBJECT_END;
        g_container_head[i] = DM2_V1_OBJECT_END;
    }
    g_next_alloc_ref = 20;
    g_dealloc_calls = 0;
    g_last_dealloc = DM2_V1_OBJECT_NULL;
}

static int mock_is_moneybox(void *ctx, uint16_t ref) { (void)ctx; (void)ref; return 1; }
static int mock_is_currency(void *ctx, uint16_t ref)
{ (void)ctx; return g_item_db_type[ref] == DM2_V1_ITEM_DB_TYPE_MISC_CURRENCY; }
static uint16_t mock_get_db_type(void *ctx, uint16_t ref)
{ (void)ctx; return g_item_db_type[ref]; }
static uint16_t mock_get_denom(void *ctx, uint16_t ref)
{ (void)ctx; return g_item_denom[ref]; }
static uint16_t mock_get_head(void *ctx, uint16_t container)
{ (void)ctx; return g_container_head[container]; }
static uint16_t mock_get_next(void *ctx, uint16_t ref)
{ (void)ctx; return g_item_next[ref]; }
static uint8_t mock_get_charge(void *ctx, uint16_t ref)
{ (void)ctx; return g_item_charge[ref]; }
static void mock_set_charge(void *ctx, uint16_t ref, uint8_t c)
{ (void)ctx; g_item_charge[ref] = c; }

static void mock_append(void *ctx, uint16_t ref, uint16_t container)
{
    (void)ctx;
    g_item_next[ref] = DM2_V1_OBJECT_END;
    uint16_t head = g_container_head[container];
    if (head == DM2_V1_OBJECT_END) {
        g_container_head[container] = ref;
        return;
    }
    uint16_t cur = head;
    while (g_item_next[cur] != DM2_V1_OBJECT_END)
        cur = g_item_next[cur];
    g_item_next[cur] = ref;
}

static void mock_cut(void *ctx, uint16_t ref, uint16_t container)
{
    (void)ctx;
    uint16_t head = g_container_head[container];
    if (head == ref) {
        g_container_head[container] = g_item_next[ref];
        return;
    }
    uint16_t cur = head;
    while (cur != DM2_V1_OBJECT_END && g_item_next[cur] != ref)
        cur = g_item_next[cur];
    if (cur != DM2_V1_OBJECT_END)
        g_item_next[cur] = g_item_next[ref];
}

static void mock_dealloc(void *ctx, uint16_t ref)
{
    (void)ctx;
    g_dealloc_calls++;
    g_last_dealloc = ref;
}

static uint16_t mock_alloc(void *ctx, uint16_t denom)
{
    (void)ctx;
    uint16_t ref = g_next_alloc_ref++;
    g_item_db_type[ref] = DM2_V1_ITEM_DB_TYPE_MISC_CURRENCY;
    g_item_denom[ref] = denom;
    g_item_charge[ref] = 0;
    g_item_next[ref] = DM2_V1_OBJECT_END;
    return ref;
}

static void test_coin_wallet(void)
{
    reset_wallet_mock();
    static const uint16_t coin_types[4] = {10, 20, 30, 40};
    DM2_V1_WalletCallbacks cb = {
        mock_is_moneybox, mock_is_currency, mock_get_db_type, mock_get_denom,
        mock_get_head, mock_get_next, mock_get_charge, mock_set_charge,
        mock_append, mock_cut, mock_dealloc, mock_alloc,
        coin_types, 4
    };
    const uint16_t container = 1;

    /* Two coin item records of denomination 10 to add. */
    uint16_t item5 = 5, item6 = 6;
    g_item_db_type[item5] = DM2_V1_ITEM_DB_TYPE_MISC_CURRENCY;
    g_item_denom[item5] = 10;
    g_item_db_type[item6] = DM2_V1_ITEM_DB_TYPE_MISC_CURRENCY;
    g_item_denom[item6] = 10;

    assert(dm2_v1_add_coin_to_wallet(container, item5, &cb, NULL) == 1);
    assert(g_container_head[container] == item5);

    /* Second coin of same denomination collapses into item5's charge
     * and deallocates its own record. */
    assert(dm2_v1_add_coin_to_wallet(container, item6, &cb, NULL) == 1);
    assert(g_item_charge[item5] == 1);
    assert(g_dealloc_calls == 1);
    assert(g_last_dealloc == item6);

    /* Take: stack has charge > 0, decrement and allocate a fresh
     * single-coin record for the caller. */
    uint16_t taken = dm2_v1_take_coin_from_wallet(container, 0, &cb, NULL);
    assert(taken != DM2_V1_OBJECT_NULL);
    assert(taken != item5);
    assert(g_item_charge[item5] == 0);

    /* Take again: stack is down to its last coin (charge == 0), so the
     * record itself is cut from the container and returned. */
    taken = dm2_v1_take_coin_from_wallet(container, 0, &cb, NULL);
    assert(taken == item5);
    assert(g_container_head[container] == DM2_V1_OBJECT_END);

    /* Wallet now empty for that denomination. */
    taken = dm2_v1_take_coin_from_wallet(container, 0, &cb, NULL);
    assert(taken == DM2_V1_OBJECT_NULL);

    /* Out-of-range coin type index. */
    taken = dm2_v1_take_coin_from_wallet(container, 99, &cb, NULL);
    assert(taken == DM2_V1_OBJECT_NULL);

    printf("  PASS: coin_wallet\n");
}

static int g_squad_dir_reset;
static void mock_reset_squad_dir(void *ctx) { (void)ctx; g_squad_dir_reset = 1; }
static uint8_t mock_get_tile_value(void *ctx, int16_t x, int16_t y)
{ (void)ctx; (void)x; (void)y; return 0; /* corridor tile */ }
static int mock_get_teleporter_detail(void *ctx, int16_t x, int16_t y,
    uint8_t *b1, uint8_t *b2, uint8_t *b3, uint8_t *b4)
{ (void)ctx; (void)x; (void)y; (void)b1; (void)b2; (void)b3; (void)b4; return 0; }
static int g_moverec_calls;
static void mock_moverec(void *ctx, int16_t x, int16_t y, int16_t d, int a, int b, int c)
{ (void)ctx; (void)x; (void)y; (void)d; (void)a; (void)b; (void)c; g_moverec_calls++; }
static int16_t g_rotated_dir;
static void mock_rotate_party(void *ctx, int16_t nd)
{ (void)ctx; g_rotated_dir = nd; }

static void test_perform_turn_squad(void)
{
    g_squad_dir_reset = 0;
    g_moverec_calls = 0;
    g_rotated_dir = -1;
    DM2_V1_SquadCallbacks cb = {
        .hero_count = 4, .get_hero = mock_get_hero,
        .reset_squad_dir = mock_reset_squad_dir,
        .get_tile_value = mock_get_tile_value,
        .get_teleporter_detail = mock_get_teleporter_detail,
        .move_stairs = NULL, .map_teleport = NULL,
        .moverec = mock_moverec,
        .rotate_party = mock_rotate_party,
        .party_x = 5, .party_y = 3, .party_dir = 0
    };
    dm2_v1_perform_turn_squad(1, &cb, NULL);
    assert(g_squad_dir_reset == 1);
    assert(g_moverec_calls == 2);
    assert(g_rotated_dir == 3); /* delta=1 → rotation=3, (3+0)&3=3 */
    printf("  PASS: perform_turn_squad\n");
}

static void test_set_spelling_champion(void)
{
    reset_heroes();
    g_heroes[2].cur_hp = 10;
    int16_t spelling = -1;
    DM2_V1_SpellingState state = { &spelling, 0, 0, 0, NULL, NULL };
    DM2_V1_SquadCallbacks scb = {
        .hero_count = 4, .get_hero = mock_get_hero,
        .reset_squad_dir = mock_reset_squad_dir,
        .get_tile_value = mock_get_tile_value,
        .get_teleporter_detail = mock_get_teleporter_detail,
        .moverec = mock_moverec,
        .rotate_party = mock_rotate_party,
    };
    dm2_v1_set_spelling_champion(&state, 2, &scb, NULL);
    assert(spelling == 3); /* hero_idx+1 per skproject */
    /* hero_idx=-1 → get_hero returns NULL → no change */
    dm2_v1_set_spelling_champion(&state, -1, &scb, NULL);
    assert(spelling == 3); /* unchanged */
    printf("  PASS: set_spelling_champion\n");
}

/* ---- Enchantment self ---- */
static uint8_t g_ench_queued_actor;
static int16_t g_ench_queued_power;
static uint16_t g_ench_queued_duration;
static uint8_t g_ench_timer_actors[4];
static uint8_t g_ench_timer_types[4];
static int g_ench_timer_count;
static int g_ench_deleted[4];
static int g_ench_delete_count;

static int mock_ench_timer_count(void *ctx) { (void)ctx; return g_ench_timer_count; }
static uint8_t mock_ench_timer_type(void *ctx, int i) { (void)ctx; return g_ench_timer_types[i]; }
static uint8_t mock_ench_timer_actor(void *ctx, int i) { (void)ctx; return g_ench_timer_actors[i]; }
static void mock_ench_set_timer_actor(void *ctx, int i, uint8_t a) { (void)ctx; g_ench_timer_actors[i] = a; }
static void mock_ench_delete_timer(void *ctx, int i) { (void)ctx; g_ench_deleted[g_ench_delete_count++] = i; }
static void mock_ench_queue(void *ctx, uint8_t actor, int16_t power, uint16_t dur)
{
    (void)ctx;
    g_ench_queued_actor = actor;
    g_ench_queued_power = power;
    g_ench_queued_duration = dur;
}

static void test_proceed_enchantment_self(void)
{
    reset_heroes();
    g_ench_timer_count = 0;
    g_ench_delete_count = 0;
    DM2_V1_EnchantmentCallbacks cb = {
        4, mock_get_hero,
        mock_ench_timer_count, mock_ench_timer_type, mock_ench_timer_actor,
        mock_ench_set_timer_actor, mock_ench_delete_timer, mock_ench_queue
    };
    /* Apply aura 5 power 20 to heroes 0 and 2 (mask = 0x05) */
    dm2_v1_proceed_enchantment_self(0x05, 5, 20, 100, &cb, NULL);
    assert(g_heroes[0].ench_aura == 5);
    assert(g_heroes[0].ench_power == 20);
    assert(g_heroes[2].ench_aura == 5);
    assert(g_heroes[2].ench_power == 20);
    assert(g_heroes[1].ench_aura == 0);
    assert(g_ench_queued_actor == 0x05);
    assert(g_ench_queued_duration == 100);

    /* Dead hero should be excluded */
    reset_heroes();
    g_heroes[1].cur_hp = 0;
    dm2_v1_proceed_enchantment_self(0x03, 2, 10, 50, &cb, NULL);
    assert(g_heroes[0].ench_aura == 2);
    assert(g_heroes[1].ench_aura == 0);
    assert((g_ench_queued_actor & 0x02) == 0);
    printf("  PASS: proceed_enchantment_self\n");
}

/* ---- Global effect timers ---- */
static uint8_t g_ge_types[8];
static uint16_t g_ge_actors[8];
static int16_t g_ge_values[8];
static int g_ge_count;
static int g_ge_0e_called;

static uint8_t mock_ge_type(void *ctx, int i) { (void)ctx; return g_ge_types[i]; }
static uint16_t mock_ge_actor(void *ctx, int i) { (void)ctx; return g_ge_actors[i]; }
static int16_t mock_ge_value(void *ctx, int i) { (void)ctx; return g_ge_values[i]; }
static void mock_ge_0e(void *ctx, int i) { (void)ctx; (void)i; g_ge_0e_called++; }

static const int16_t mock_light_table[16] = {
    0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150
};

static void test_proceed_global_effect_timers(void)
{
    reset_heroes();
    DM2_V1_GlobalSpellEffects effects;
    DM2_V1_GlobalEffectCallbacks cb = {
        0, mock_ge_type, mock_ge_actor, mock_ge_value, mock_ge_0e,
        4, mock_get_hero, mock_light_table, 16
    };

    /* Light timer: negative value adds light */
    g_ge_types[0] = 0x46; g_ge_actors[0] = 0; g_ge_values[0] = -3;
    /* Invisibility */
    g_ge_types[1] = 0x47; g_ge_actors[1] = 0; g_ge_values[1] = 0;
    /* Enchantment for hero 0 */
    g_ge_types[2] = 0x48; g_ge_actors[2] = 0x01; g_ge_values[2] = 5;
    /* Poison for hero 2 */
    g_ge_types[3] = 0x4B; g_ge_actors[3] = 2; g_ge_values[3] = 0;
    /* Item bonus */
    g_ge_types[4] = 0x0E; g_ge_actors[4] = 0; g_ge_values[4] = 0;
    cb.timer_count = 5;
    g_ge_0e_called = 0;

    dm2_v1_proceed_global_effect_timers(&effects, &cb, NULL);
    assert(effects.light == 30); /* light_table[3] = 30 */
    assert(effects.invisibility == 1);
    assert(g_heroes[0].ench_power == 5);
    assert(g_heroes[2].poison_value == 1);
    assert(g_ge_0e_called == 1);
    printf("  PASS: proceed_global_effect_timers\n");
}

static void test_process_timer_0c(void)
{
    reset_heroes();
    g_heroes[0].timer_idx = 5;
    g_heroes[0].cur_hp = 50;
    assert(dm2_v1_process_timer_0c(&g_heroes[0]) == 1);
    assert(g_heroes[0].timer_idx == -1);
    assert((g_heroes[0].hero_flag & DM2_V1_HERO_FLAG_0800) != 0);

    /* Dead hero: timer_idx cleared but no flag set */
    reset_heroes();
    g_heroes[1].timer_idx = 3;
    g_heroes[1].cur_hp = 0;
    assert(dm2_v1_process_timer_0c(&g_heroes[1]) == 1);
    assert(g_heroes[1].timer_idx == -1);
    assert((g_heroes[1].hero_flag & DM2_V1_HERO_FLAG_0800) == 0);

    assert(dm2_v1_process_timer_0c(NULL) == 0);
    printf("  PASS: process_timer_0c\n");
}

static int g_backbuff_called;
static int g_display_mode_arg;
static void mock_init_backbuff(void *ctx) { (void)ctx; g_backbuff_called = 1; }
static int mock_display_mode(void *ctx, int mode) { (void)ctx; g_display_mode_arg = mode; return mode; }

static void test_resume_from_wake(void)
{
    int wake = 0, sleep = 1, tick = 0;
    g_backbuff_called = 0;
    g_display_mode_arg = -1;
    DM2_V1_WakeCallbacks cb = { &wake, &sleep, &tick, mock_init_backbuff, mock_display_mode };
    int r = dm2_v1_resume_from_wake(&cb, NULL);
    assert(wake == 1);
    assert(sleep == 0);
    assert(tick == 0x8);
    assert(g_backbuff_called == 1);
    assert(g_display_mode_arg == 5);
    assert(r == 5);
    printf("  PASS: resume_from_wake\n");
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
    test_proceed_enchantment_self();
    test_proceed_global_effect_timers();
    test_process_timer_0c();
    test_resume_from_wake();
    printf("All hero_ops tests passed.\n");
    return 0;
}
