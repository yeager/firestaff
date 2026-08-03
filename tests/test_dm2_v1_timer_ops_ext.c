/* Test DM2 V1 timer operations — extended (ornate, tick gen, etc). */

#include "dm2_v1_timer_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t g_rec[8];
static int g_timer_queued;

static uint8_t *mock_get_rec(void *ctx, uint16_t rw)
{
    (void)ctx; (void)rw;
    return g_rec;
}

static int16_t mock_anim_len(void *ctx, uint8_t *rec, int mode)
{
    (void)ctx; (void)rec; (void)mode;
    return 4;
}

static void mock_queue(void *ctx) { (void)ctx; g_timer_queued = 1; }

static void test_continue_ornate_animator(void)
{
    memset(g_rec, 0, sizeof(g_rec));
    g_timer_queued = 0;
    /* frame=0 in bits 7-15 of word+2 */
    g_rec[2] = 0x00;
    g_rec[3] = 0x00;
    g_rec[4] = 0x01; /* active bit set */
    DM2_V1_OrnateAnimCallbacks cb = { mock_get_rec, mock_anim_len, mock_queue };
    int r = dm2_v1_continue_ornate_animator(0x1234, 0, &cb, NULL);
    assert(r == 1); /* frame 1, not at cycle end */
    assert(g_timer_queued == 1);

    /* Advance to frame 3 (anim_len=4, so frame 4 mod 4 = 0 => stop) */
    uint16_t w2 = (uint16_t)(g_rec[2] | (g_rec[3] << 8));
    uint16_t frame = (w2 >> 7) & 0x1FF;
    assert(frame == 1);

    /* Set frame to 3, next will be 4 mod 4 = 0 => clear active */
    g_rec[2] = (uint8_t)((3 << 7) & 0xFF);
    g_rec[3] = (uint8_t)((3 << 7) >> 8);
    g_timer_queued = 0;
    r = dm2_v1_continue_ornate_animator(0x1234, 0, &cb, NULL);
    assert(r == 0);
    assert((g_rec[4] & 0x01) == 0);
    printf("  PASS: continue_ornate_animator\n");
}

static int g_invoked;
static void mock_invoke(void *ctx, uint8_t *rec, uint16_t act, uint16_t param)
{
    (void)ctx; (void)rec; (void)act; (void)param;
    g_invoked = 1;
}

static void mock_requeue(void *ctx, uint16_t delay, uint8_t mult)
{
    (void)ctx; (void)delay; (void)mult;
    g_timer_queued = 1;
}

static void test_continue_tick_generator(void)
{
    memset(g_rec, 0, sizeof(g_rec));
    g_invoked = 0;
    g_timer_queued = 0;
    /* word+4 normal mode: bits 3-4 not both set, bit 2 (action count) = 1 */
    g_rec[4] = 0x04; /* bit 2 set -> action bits = 1 */
    /* word+2 bits 7+ = delay */
    g_rec[2] = 0x80; /* delay_base = 1 */
    DM2_V1_TickGenCallbacks cb = { mock_get_rec, mock_invoke, mock_requeue };
    DM2_V1_TickGenTimerState ts = { 0, 0 };
    int r = dm2_v1_continue_tick_generator(0x1234, &ts, &cb, NULL);
    assert(g_invoked == 1);
    printf("  PASS: continue_tick_generator\n");
}

static uint32_t g_fire_tick;
static uint8_t g_tick_mult;

static void mock_queue_tick(void *ctx, uint16_t idx, uint8_t mult, uint32_t tick)
{
    (void)ctx; (void)idx;
    g_tick_mult = mult;
    g_fire_tick = tick;
}

static void test_activate_tick_generator(void)
{
    uint8_t act_rec[8];
    memset(act_rec, 0, sizeof(act_rec));
    /* subtype 0x1E (30) -> multiplier 1, period=2 in bits 7+ of word+2 */
    uint16_t aw2 = 0x1E | (2 << 7); /* subtype=0x1E, period=2 */
    act_rec[2] = (uint8_t)(aw2 & 0xFF);
    act_rec[3] = (uint8_t)(aw2 >> 8);
    DM2_V1_ActivateTickGenCallbacks cb = { 100, 0, mock_queue_tick };
    int r = dm2_v1_activate_tick_generator(act_rec, 0x10, &cb, NULL);
    assert(r == 1);
    assert(g_tick_mult == 1);
    assert((act_rec[4] & 0x01) == 1);
    printf("  PASS: activate_tick_generator\n");
}

static uint16_t g_rotated_creature;
static int g_rotate_mode;

static uint16_t mock_get_creature_at(void *ctx, uint16_t x, uint16_t y)
{
    (void)ctx;
    if (x == 3 && y == 5) return 0x0042;
    return 0xFFFF;
}

static void mock_rotate(void *ctx, uint16_t cw, int mode, int dir)
{
    (void)ctx;
    g_rotated_creature = cw;
    g_rotate_mode = mode;
}

static void test_skw_3a15_0d5c(void)
{
    uint8_t act[8];
    memset(act, 0, sizeof(act));
    /* No bit5 set, timer_yb=0 -> should proceed */
    /* word+6: target_x=3 (bits 6-10), target_y=5 (bits 11-15) */
    uint16_t w6 = (uint16_t)((5 << 11) | (3 << 6));
    act[6] = (uint8_t)(w6 & 0xFF);
    act[7] = (uint8_t)(w6 >> 8);
    /* word+2 bits 7-8 = mode = 2 */
    act[2] = (uint8_t)(2 << 7);
    act[3] = 0;

    g_rotated_creature = 0;
    DM2_V1_RotateCreatureActCallbacks cb = { mock_get_creature_at, mock_rotate };
    int r = dm2_v1_skw_3a15_0d5c(act, 0, &cb, NULL);
    assert(r == 1);
    assert(g_rotated_creature == 0x0042);
    assert(g_rotate_mode == 1);

    /* bit5 set + yb=0 => must fail */
    act[4] = 0x20;
    r = dm2_v1_skw_3a15_0d5c(act, 0, &cb, NULL);
    assert(r == 0);

    /* bit5 set + yb=1 => proceed */
    r = dm2_v1_skw_3a15_0d5c(act, 1, &cb, NULL);
    assert(r == 1);
    printf("  PASS: skw_3a15_0d5c\n");
}

static uint8_t g_0e_rec[16];
static uint8_t g_0e_backup[16];
static int g_0e_bonus_called;

static uint8_t *mock_0e_get_rec(void *ctx, uint16_t rw)
{
    (void)ctx; (void)rw;
    return g_0e_rec;
}

static void *mock_alloc(void *ctx, int32_t size)
{
    (void)ctx; (void)size;
    return g_0e_backup;
}

static void mock_dealloc(void *ctx, void *ptr, int32_t size)
{
    (void)ctx; (void)ptr; (void)size;
}

static void mock_set_itemtype(void *ctx, uint16_t rec, uint16_t type)
{
    (void)ctx; (void)rec; (void)type;
}

static void mock_process_bonus(void *ctx, uint8_t actor, uint16_t rec,
                               int mode, uint16_t val)
{
    (void)ctx; (void)actor; (void)rec; (void)mode; (void)val;
    g_0e_bonus_called = 1;
}

static void mock_copy(void *dst, const void *src, int32_t size)
{
    memcpy(dst, src, (size_t)size);
}

static int32_t mock_item_size(uint16_t db) { (void)db; return 8; }

static void test_process_timer_0e(void)
{
    memset(g_0e_rec, 0xAA, sizeof(g_0e_rec));
    g_0e_bonus_called = 0;
    DM2_V1_Timer0ECallbacks cb = {
        mock_0e_get_rec, mock_alloc, mock_dealloc,
        mock_set_itemtype, mock_process_bonus, mock_copy, mock_item_size
    };
    dm2_v1_process_timer_0e(5, 0x10, 0, 1, &cb, NULL);
    assert(g_0e_bonus_called == 1);
    /* Record should be restored to original */
    assert(g_0e_rec[0] == 0xAA);
    printf("  PASS: process_timer_0e\n");
}

int main(void)
{
    printf("test_dm2_v1_timer_ops_ext:\n");
    test_continue_ornate_animator();
    test_continue_tick_generator();
    test_activate_tick_generator();
    test_skw_3a15_0d5c();
    test_process_timer_0e();
    printf("All timer_ops_ext tests passed.\n");
    return 0;
}
