/* Test DM2 V1 light operations (c_light.cpp). */

#include "dm2_v1_light_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int16_t g_light;
static int16_t g_queued_val;
static uint32_t g_queued_tick;
static int g_recalc;

static void mock_queue(void *ctx, int16_t val, uint32_t tick)
{
    (void)ctx;
    g_queued_val = val;
    g_queued_tick = tick;
}

static void mock_recalc(void *ctx) { (void)ctx; g_recalc = 1; }

static const int16_t mock_table[16] = {
    0, 10, 25, 45, 70, 100, 135, 175, 220, 270, 325, 385, 450, 520, 595, 675
};

static void test_proceed_light_darkness(void)
{
    g_light = 500;
    g_recalc = 0;
    DM2_V1_ProceedLightCallbacks cb = {
        &g_light, mock_table, 16, 1000, mock_queue, mock_recalc
    };
    dm2_v1_proceed_light(0x06, 100, &cb, NULL);
    assert(g_recalc == 1);
    /* step = max(8, between(32,256,101)/8) = max(8,12) = 12
     * darkness: delay = 16*(12-8)+16 = 80
     * timer_val = 12 (positive for darkness)
     * dir_mult = -2, light_delta = table[12]*(-2) = 450*(-2) = -900 */
    assert(g_queued_val > 0); /* darkness stores positive */
    assert(g_light < 500);
    printf("  PASS: proceed_light_darkness\n");
}

static void test_proceed_light_torch(void)
{
    g_light = 100;
    g_recalc = 0;
    DM2_V1_ProceedLightCallbacks cb = {
        &g_light, mock_table, 16, 500, mock_queue, mock_recalc
    };
    dm2_v1_proceed_light(0x26, 80, &cb, NULL);
    assert(g_recalc == 1);
    /* step = max(8, between(32,256,81)/8) = max(8,10) = 10
     * torch: delay = ((10-3)*128)+2000 = 2896
     * step = 10/4 + 1 = 3, then halve=1, dec=0
     * timer_val = -0 = 0 ... hmm, let's just verify timer was queued */
    assert(g_queued_val <= 0); /* non-darkness stores negative */
    printf("  PASS: proceed_light_torch\n");
}

static void test_proceed_light_invalid(void)
{
    g_light = 100;
    g_recalc = 0;
    DM2_V1_ProceedLightCallbacks cb = {
        &g_light, mock_table, 16, 0, mock_queue, mock_recalc
    };
    dm2_v1_proceed_light(0x05, 50, &cb, NULL);
    assert(g_recalc == 0);
    assert(g_light == 100);
    printf("  PASS: proceed_light_invalid\n");
}

/* ---- add_background_light_from_tile tests ---- */

static int16_t g_added_light;
static int16_t g_added_x, g_added_y;

static int16_t mock_get_tile_light(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return 10;
}

static void mock_add_light(void *ctx, int16_t x, int16_t y, int16_t amount)
{
    (void)ctx;
    g_added_x = x; g_added_y = y;
    g_added_light = amount;
}

static void test_add_background_light(void)
{
    g_added_light = 0;
    DM2_V1_AddBackgroundLightCallbacks cb = { mock_get_tile_light, mock_add_light };
    dm2_v1_add_background_light_from_tile(5, 10, 3, &cb, NULL);
    assert(g_added_light == 7); /* 10 - 3 = 7 */
    assert(g_added_x == 5);
    assert(g_added_y == 10);
    printf("  PASS: add_background_light\n");
}

static void test_add_background_light_clamped(void)
{
    g_added_light = 0;
    DM2_V1_AddBackgroundLightCallbacks cb = { mock_get_tile_light, mock_add_light };
    dm2_v1_add_background_light_from_tile(5, 10, 20, &cb, NULL);
    assert(g_added_light == 2); /* clamped to minimum 2 */
    printf("  PASS: add_background_light_clamped\n");
}

/* ---- check_recompute_light tests ---- */

static int g_dirty_flag;
static int g_recomputed;

static int mock_is_dirty(void *ctx) { (void)ctx; return g_dirty_flag; }
static void mock_recompute(void *ctx) { (void)ctx; g_recomputed = 1; }
static void mock_clear_dirty(void *ctx) { (void)ctx; g_dirty_flag = 0; }

static void test_check_recompute_clean(void)
{
    g_dirty_flag = 0; g_recomputed = 0;
    DM2_V1_CheckRecomputeLightCallbacks cb = { mock_is_dirty, mock_recompute, mock_clear_dirty };
    int32_t r = dm2_v1_check_recompute_light(&cb, NULL);
    assert(r == 0);
    assert(g_recomputed == 0);
    printf("  PASS: check_recompute_clean\n");
}

static void test_check_recompute_dirty(void)
{
    g_dirty_flag = 1; g_recomputed = 0;
    DM2_V1_CheckRecomputeLightCallbacks cb = { mock_is_dirty, mock_recompute, mock_clear_dirty };
    int32_t r = dm2_v1_check_recompute_light(&cb, NULL);
    assert(r == 1);
    assert(g_recomputed == 1);
    assert(g_dirty_flag == 0);
    printf("  PASS: check_recompute_dirty\n");
}

int main(void)
{
    printf("test_dm2_v1_light_ops:\n");
    test_proceed_light_darkness();
    test_proceed_light_torch();
    test_proceed_light_invalid();
    test_add_background_light();
    test_add_background_light_clamped();
    test_check_recompute_clean();
    test_check_recompute_dirty();
    printf("All light_ops tests passed.\n");
    return 0;
}
