/* Test simple DM2 runtime timer handlers. */

#include "dm2_v1_timer_handlers_simple_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TIMER_SIZE 12

static int16_t g_light_delta;
static int g_ench_flag_hero;
static int g_ench_power_hero;
static int16_t g_ench_power_amount;
static int g_poison_hero;
static int16_t g_poison_amount;
static uint16_t g_clear_link;
static uint16_t g_set_link;
static int g_alive[4];

static void mock_adjust_light(void *ctx, int16_t delta)
{
    (void)ctx;
    g_light_delta = (int16_t)(g_light_delta + delta);
}

static int mock_decrement_ench_flag(void *ctx, int hero_idx)
{
    (void)ctx;
    g_ench_flag_hero = hero_idx;
    return 0;
}

static void mock_decrement_ench_power(void *ctx, int hero_idx, int16_t amount)
{
    (void)ctx;
    g_ench_power_hero = hero_idx;
    g_ench_power_amount = amount;
}

static void mock_apply_poison(void *ctx, int hero_idx, int16_t amount)
{
    (void)ctx;
    g_poison_hero = hero_idx;
    g_poison_amount = amount;
}

static int mock_alive(void *ctx, int hero_idx)
{
    (void)ctx;
    return (hero_idx >= 0 && hero_idx < 4) ? g_alive[hero_idx] : 0;
}

static void mock_record_clear(void *ctx, uint16_t link)
{
    (void)ctx;
    g_clear_link = link;
}

static void mock_record_set(void *ctx, uint16_t link)
{
    (void)ctx;
    g_set_link = link;
}

static int mock_hero_count(void *ctx)
{
    (void)ctx;
    return 4;
}

static void set_timer(uint8_t *t, uint8_t type, uint8_t actor, int16_t valueA)
{
    memset(t, 0, TIMER_SIZE);
    t[4] = type;
    t[5] = actor;
    t[6] = (uint8_t)(valueA & 0xFF);
    t[7] = (uint8_t)((valueA >> 8) & 0xFF);
}

static DM2_V1_SimpleTimerCallbacks make_cb(void)
{
    DM2_V1_SimpleTimerCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.adjust_light = mock_adjust_light;
    cb.decrement_hero_ench_flag = mock_decrement_ench_flag;
    cb.decrement_hero_ench_power = mock_decrement_ench_power;
    cb.apply_poison_tick = mock_apply_poison;
    cb.hero_is_alive = mock_alive;
    cb.record_clear_bit = mock_record_clear;
    cb.record_set_bit = mock_record_set;
    cb.get_hero_count = mock_hero_count;
    return cb;
}

static void test_null_safety(void)
{
    assert(dm2_v1_handle_timer_light(NULL, NULL) == 0);
    assert(dm2_v1_handle_timer_record_clear(NULL, NULL) == 0);
    printf("  PASS: null_safety\n");
}

static void test_light_positive(void)
{
    g_light_delta = 0;
    uint8_t t[TIMER_SIZE];
    set_timer(t, 0x46, 0, 5);
    DM2_V1_SimpleTimerCallbacks cb = make_cb();
    assert(dm2_v1_handle_timer_light(t, &cb) == 1);
    assert(g_light_delta == 248);
    printf("  PASS: light_positive\n");
}

static void test_light_negative(void)
{
    g_light_delta = 0;
    uint8_t t[TIMER_SIZE];
    set_timer(t, 0x46, 0, -1);
    DM2_V1_SimpleTimerCallbacks cb = make_cb();
    assert(dm2_v1_handle_timer_light(t, &cb) == 1);
    assert(g_light_delta == -8);
    printf("  PASS: light_negative\n");
}

static void test_ench_flag(void)
{
    g_ench_flag_hero = -1;
    uint8_t t[TIMER_SIZE];
    set_timer(t, 0x47, 2, 0);
    DM2_V1_SimpleTimerCallbacks cb = make_cb();
    assert(dm2_v1_handle_timer_hero_ench_flag(t, &cb) == 1);
    assert(g_ench_flag_hero == 2);
    printf("  PASS: ench_flag\n");
}

static void test_ench_power(void)
{
    memset(g_alive, 0, sizeof(g_alive));
    g_alive[0] = 1; g_alive[2] = 1;
    g_ench_power_hero = -1;
    uint8_t t[TIMER_SIZE];
    set_timer(t, 0x48, 0x05, 7);
    DM2_V1_SimpleTimerCallbacks cb = make_cb();
    assert(dm2_v1_handle_timer_ench_power(t, &cb) == 1);
    assert(g_ench_power_hero == 2);
    assert(g_ench_power_amount == 7);
    printf("  PASS: ench_power\n");
}

static void test_poison(void)
{
    memset(g_alive, 0, sizeof(g_alive));
    g_alive[1] = 1;
    g_poison_hero = -1;
    uint8_t t[TIMER_SIZE];
    set_timer(t, 0x4B, 1, 3);
    DM2_V1_SimpleTimerCallbacks cb = make_cb();
    assert(dm2_v1_handle_timer_poison(t, &cb) == 1);
    assert(g_poison_hero == 1);
    assert(g_poison_amount == 3);
    printf("  PASS: poison\n");
}

static void test_record_clear(void)
{
    g_clear_link = 0;
    uint8_t t[TIMER_SIZE];
    set_timer(t, 0x5B, 0, 0x1234);
    DM2_V1_SimpleTimerCallbacks cb = make_cb();
    assert(dm2_v1_handle_timer_record_clear(t, &cb) == 1);
    assert(g_clear_link == 0x1234);
    printf("  PASS: record_clear\n");
}

static void test_record_set(void)
{
    g_set_link = 0;
    uint8_t t[TIMER_SIZE];
    set_timer(t, 0x5C, 0, 0xABCD);
    DM2_V1_SimpleTimerCallbacks cb = make_cb();
    assert(dm2_v1_handle_timer_record_set(t, &cb) == 1);
    assert(g_set_link == 0xABCD);
    printf("  PASS: record_set\n");
}

int main(void)
{
    printf("test_dm2_v1_timer_handlers_simple:\n");
    test_null_safety();
    test_light_positive();
    test_light_negative();
    test_ench_flag();
    test_ench_power();
    test_poison();
    test_record_clear();
    test_record_set();
    printf("All timer_handlers_simple tests passed.\n");
    return 0;
}
