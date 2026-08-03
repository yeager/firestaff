/* Test DM2 post-load global effect timer accumulator.
 * Source: sksvgame.cpp:1041-1106 (DM2_PROCEED_GLOBAL_EFFECT_TIMERS). */

#include "dm2_v1_save_post_load_global_effects_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TIMER_SIZE 12

static int16_t g_ench_power[4];
static int16_t g_poison[4];
static int g_alive[4];
static int g_0e_count;

static void mock_process_0e(void *ctx, const uint8_t *timer)
{
    (void)ctx; (void)timer;
    g_0e_count++;
}

static void mock_add_ench(void *ctx, int hero_idx, int16_t power)
{
    (void)ctx;
    if (hero_idx >= 0 && hero_idx < 4)
        g_ench_power[hero_idx] = (int16_t)(g_ench_power[hero_idx] + power);
}

static void mock_add_poison(void *ctx, int hero_idx, int16_t amount)
{
    (void)ctx;
    if (hero_idx >= 0 && hero_idx < 4)
        g_poison[hero_idx] = (int16_t)(g_poison[hero_idx] + amount);
}

static int mock_hero_alive(void *ctx, int hero_idx)
{
    (void)ctx;
    if (hero_idx >= 0 && hero_idx < 4) return g_alive[hero_idx];
    return 0;
}

static void reset_mocks(void)
{
    memset(g_ench_power, 0, sizeof(g_ench_power));
    memset(g_poison, 0, sizeof(g_poison));
    memset(g_alive, 0, sizeof(g_alive));
    g_0e_count = 0;
}

static DM2_V1_GlobalEffectCallbacks make_cb(void)
{
    DM2_V1_GlobalEffectCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.process_timer_0e = mock_process_0e;
    cb.add_hero_ench_power = mock_add_ench;
    cb.add_hero_poison = mock_add_poison;
    cb.hero_is_alive = mock_hero_alive;
    return cb;
}

static void set_timer(uint8_t *t, uint8_t type, uint8_t actor, int16_t valueA)
{
    memset(t, 0, TIMER_SIZE);
    t[4] = type;
    t[5] = actor;
    t[6] = (uint8_t)(valueA & 0xFF);
    t[7] = (uint8_t)((valueA >> 8) & 0xFF);
}

static void test_null_safety(void)
{
    DM2_V1_GlobalEffectReceipt r;
    assert(dm2_v1_post_load_global_effects(NULL, 0, 0, NULL, &r) == -1);
    printf("  PASS: null_safety\n");
}

static void test_no_timers(void)
{
    DM2_V1_GlobalEffectCallbacks cb = make_cb();
    DM2_V1_GlobalEffectReceipt r;
    assert(dm2_v1_post_load_global_effects(NULL, 0, 2, &cb, &r) == 0);
    assert(r.valid == 1);
    assert(r.timers_scanned == 0);
    printf("  PASS: no_timers\n");
}

static void test_light_positive(void)
{
    reset_mocks();
    uint8_t timers[TIMER_SIZE];
    set_timer(timers, 0x46, 0, 3);
    DM2_V1_GlobalEffectCallbacks cb = make_cb();
    DM2_V1_GlobalEffectReceipt r;
    assert(dm2_v1_post_load_global_effects(timers, 1, 0, &cb, &r) == 0);
    assert(r.light_accumulator == 56);
    printf("  PASS: light_positive\n");
}

static void test_light_negative(void)
{
    reset_mocks();
    uint8_t timers[TIMER_SIZE];
    set_timer(timers, 0x46, 0, -2);
    DM2_V1_GlobalEffectCallbacks cb = make_cb();
    DM2_V1_GlobalEffectReceipt r;
    assert(dm2_v1_post_load_global_effects(timers, 1, 0, &cb, &r) == 0);
    assert(r.light_accumulator == -24);
    printf("  PASS: light_negative\n");
}

static void test_attack_count(void)
{
    reset_mocks();
    uint8_t timers[3 * TIMER_SIZE];
    set_timer(timers + 0 * TIMER_SIZE, 0x47, 0, 0);
    set_timer(timers + 1 * TIMER_SIZE, 0x47, 0, 0);
    set_timer(timers + 2 * TIMER_SIZE, 0x00, 0, 0);
    DM2_V1_GlobalEffectCallbacks cb = make_cb();
    DM2_V1_GlobalEffectReceipt r;
    assert(dm2_v1_post_load_global_effects(timers, 3, 0, &cb, &r) == 0);
    assert(r.attack_count == 2);
    printf("  PASS: attack_count\n");
}

static void test_ench_power_bitmask(void)
{
    reset_mocks();
    g_alive[0] = 1; g_alive[1] = 1; g_alive[2] = 0; g_alive[3] = 1;
    uint8_t timers[TIMER_SIZE];
    set_timer(timers, 0x48, 0x0B, 5);
    DM2_V1_GlobalEffectCallbacks cb = make_cb();
    DM2_V1_GlobalEffectReceipt r;
    assert(dm2_v1_post_load_global_effects(timers, 1, 4, &cb, &r) == 0);
    assert(g_ench_power[0] == 5);
    assert(g_ench_power[1] == 5);
    assert(g_ench_power[2] == 0);
    assert(g_ench_power[3] == 5);
    assert(r.ench_power_applied == 3);
    printf("  PASS: ench_power_bitmask\n");
}

static void test_poison(void)
{
    reset_mocks();
    g_alive[2] = 1;
    uint8_t timers[TIMER_SIZE];
    set_timer(timers, 0x4B, 2, 10);
    DM2_V1_GlobalEffectCallbacks cb = make_cb();
    DM2_V1_GlobalEffectReceipt r;
    assert(dm2_v1_post_load_global_effects(timers, 1, 4, &cb, &r) == 0);
    assert(g_poison[2] == 10);
    assert(r.poison_applied == 1);
    printf("  PASS: poison\n");
}

static void test_timer_0e(void)
{
    reset_mocks();
    uint8_t timers[TIMER_SIZE];
    set_timer(timers, 0x0E, 0, 0);
    DM2_V1_GlobalEffectCallbacks cb = make_cb();
    DM2_V1_GlobalEffectReceipt r;
    assert(dm2_v1_post_load_global_effects(timers, 1, 0, &cb, &r) == 0);
    assert(r.timer_0e_processed == 1);
    assert(g_0e_count == 1);
    printf("  PASS: timer_0e\n");
}

static void test_dead_hero_skipped(void)
{
    reset_mocks();
    g_alive[1] = 0;
    uint8_t timers[TIMER_SIZE];
    set_timer(timers, 0x4B, 1, 5);
    DM2_V1_GlobalEffectCallbacks cb = make_cb();
    DM2_V1_GlobalEffectReceipt r;
    assert(dm2_v1_post_load_global_effects(timers, 1, 4, &cb, &r) == 0);
    assert(g_poison[1] == 0);
    assert(r.poison_applied == 0);
    printf("  PASS: dead_hero_skipped\n");
}

int main(void)
{
    printf("test_dm2_v1_save_post_load_global_effects:\n");
    test_null_safety();
    test_no_timers();
    test_light_positive();
    test_light_negative();
    test_attack_count();
    test_ench_power_bitmask();
    test_poison();
    test_timer_0e();
    test_dead_hero_skipped();
    printf("All post_load_global_effects tests passed.\n");
    return 0;
}
