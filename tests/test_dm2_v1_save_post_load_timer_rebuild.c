/* Test DM2 post-load timer index rebuild.
 * Source: sksvgame.cpp:1351-1410 (DM2_3a15_020f). */

#include "dm2_v1_save_post_load_timer_rebuild_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Mock state */
static int16_t g_hero_timeridx[4];
static uint16_t g_backlink_link;
static int16_t g_backlink_timer_idx;
static int g_backlink_count;

static void mock_set_hero_timeridx(void *ctx, int hero_idx, int16_t timer_idx)
{
    (void)ctx;
    if (hero_idx >= 0 && hero_idx < 4)
        g_hero_timeridx[hero_idx] = timer_idx;
}

static void mock_set_backlink(void *ctx, uint16_t link, int16_t timer_idx)
{
    (void)ctx;
    g_backlink_link = link;
    g_backlink_timer_idx = timer_idx;
    g_backlink_count++;
}

static void test_null_safety(void)
{
    DM2_V1_TimerRebuildReceipt r;
    assert(dm2_v1_post_load_timer_rebuild(NULL, 0, 0, NULL, &r) == -1);
    printf("  PASS: null_safety\n");
}

static void test_no_timers(void)
{
    DM2_V1_TimerRebuildCallbacks cb;
    DM2_V1_TimerRebuildReceipt r;
    memset(&cb, 0, sizeof(cb));
    memset(g_hero_timeridx, 0, sizeof(g_hero_timeridx));
    cb.set_hero_timeridx = mock_set_hero_timeridx;

    assert(dm2_v1_post_load_timer_rebuild(NULL, 0, 2, &cb, &r) == 0);
    assert(r.valid == 1);
    assert(r.hero_timeridx_cleared == 2);
    assert(g_hero_timeridx[0] == -1);
    assert(g_hero_timeridx[1] == -1);
    printf("  PASS: no_timers\n");
}

static void test_type_0c_hero_timeridx(void)
{
    /* Timer at index 3, type 0x0C, actor 1 -> hero[1].timeridx = 3 */
    uint8_t timers[4 * 12];
    DM2_V1_TimerRebuildCallbacks cb;
    DM2_V1_TimerRebuildReceipt r;

    memset(timers, 0, sizeof(timers));
    memset(&cb, 0, sizeof(cb));
    memset(g_hero_timeridx, 0, sizeof(g_hero_timeridx));

    /* Timer 0: type 0x00 (no match) */
    timers[0 * 12 + 4] = 0x00;
    /* Timer 1: type 0x05 (no match) */
    timers[1 * 12 + 4] = 0x05;
    /* Timer 2: type 0x0C, actor 0 */
    timers[2 * 12 + 4] = 0x0C;
    timers[2 * 12 + 5] = 0x00;
    /* Timer 3: type 0x0C, actor 1 */
    timers[3 * 12 + 4] = 0x0C;
    timers[3 * 12 + 5] = 0x01;

    cb.set_hero_timeridx = mock_set_hero_timeridx;

    assert(dm2_v1_post_load_timer_rebuild(timers, 4, 4, &cb, &r) == 0);
    assert(r.valid == 1);
    assert(r.hero_timeridx_set == 2);
    assert(r.timers_scanned == 4);
    assert(g_hero_timeridx[0] == 2);
    assert(g_hero_timeridx[1] == 3);
    printf("  PASS: type_0c_hero_timeridx\n");
}

static void test_type_1d_backlink(void)
{
    /* Timer 0: type 0x1D, valueA = 0x0042 -> backlink */
    uint8_t timers[12];
    DM2_V1_TimerRebuildCallbacks cb;
    DM2_V1_TimerRebuildReceipt r;

    memset(timers, 0, sizeof(timers));
    memset(&cb, 0, sizeof(cb));
    g_backlink_count = 0;

    timers[4] = 0x1D;
    timers[6] = 0x42;
    timers[7] = 0x00;

    cb.set_hero_timeridx = mock_set_hero_timeridx;
    cb.set_record_timer_backlink = mock_set_backlink;

    assert(dm2_v1_post_load_timer_rebuild(timers, 1, 0, &cb, &r) == 0);
    assert(r.valid == 1);
    assert(r.ornate_backlinks_set == 1);
    assert(g_backlink_link == 0x0042);
    assert(g_backlink_timer_idx == 0);
    assert(g_backlink_count == 1);
    printf("  PASS: type_1d_backlink\n");
}

static void test_type_1e_backlink(void)
{
    uint8_t timers[12];
    DM2_V1_TimerRebuildCallbacks cb;
    DM2_V1_TimerRebuildReceipt r;

    memset(timers, 0, sizeof(timers));
    memset(&cb, 0, sizeof(cb));
    g_backlink_count = 0;

    timers[4] = 0x1E;
    timers[6] = 0x10;
    timers[7] = 0x02;

    cb.set_hero_timeridx = mock_set_hero_timeridx;
    cb.set_record_timer_backlink = mock_set_backlink;

    assert(dm2_v1_post_load_timer_rebuild(timers, 1, 0, &cb, &r) == 0);
    assert(r.ornate_backlinks_set == 1);
    assert(g_backlink_link == 0x0210);
    printf("  PASS: type_1e_backlink\n");
}

int main(void)
{
    printf("test_dm2_v1_save_post_load_timer_rebuild:\n");
    test_null_safety();
    test_no_timers();
    test_type_0c_hero_timeridx();
    test_type_1d_backlink();
    test_type_1e_backlink();
    printf("All post_load_timer_rebuild tests passed.\n");
    return 0;
}
