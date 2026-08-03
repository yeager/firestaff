/* Test DM2 V1 timer handler operations (c_tim_proc.cpp). */

#include "dm2_v1_timer_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- PROCESS_TIMER_LIGHT ---- */
static int16_t g_light_val;
static int16_t g_queued_intensity;
static uint32_t g_queued_delay;
static int g_light_queued;

static void mock_queue_light(void *ctx, int16_t intensity, uint32_t delay)
{
    (void)ctx;
    g_queued_intensity = intensity;
    g_queued_delay = delay;
    g_light_queued = 1;
}

static const int16_t mock_light_table[16] = {
    0, 10, 25, 45, 70, 100, 135, 175, 220, 270, 325, 385, 450, 520, 595, 675
};

static void test_process_timer_light_darkness(void)
{
    g_light_val = 500;
    g_light_queued = 0;
    DM2_V1_LightTimerCallbacks cb = {
        &g_light_val, mock_queue_light, mock_light_table, 16
    };
    /* Positive = darkness spell, intensity 3 */
    dm2_v1_process_timer_light(3, &cb, NULL);
    /* delta = (table[3] - table[2]) * 2 = (45-25)*2 = 40 */
    assert(g_light_val == 540);
    assert(g_light_queued == 1);
    assert(g_queued_intensity == 2);
    assert(g_queued_delay == 8);
    printf("  PASS: process_timer_light_darkness\n");
}

static void test_process_timer_light_source(void)
{
    g_light_val = 100;
    g_light_queued = 0;
    DM2_V1_LightTimerCallbacks cb = {
        &g_light_val, mock_queue_light, mock_light_table, 16
    };
    /* Negative = light source, intensity -2 */
    dm2_v1_process_timer_light(-2, &cb, NULL);
    /* abs=2, dec=1. delta = table[2]-table[1] = 25-10 = 15. Negated = -15 */
    assert(g_light_val == 85);
    assert(g_light_queued == 1);
    assert(g_queued_intensity == -1);
    printf("  PASS: process_timer_light_source\n");
}

static void test_process_timer_light_zero(void)
{
    g_light_val = 100;
    DM2_V1_LightTimerCallbacks cb = {
        &g_light_val, mock_queue_light, mock_light_table, 16
    };
    assert(dm2_v1_process_timer_light(0, &cb, NULL) == 0);
    assert(g_light_val == 100);
    printf("  PASS: process_timer_light_zero\n");
}

/* ---- RELEASE_DOOR_BUTTON ---- */
static uint8_t g_door_rec[8];
static uint8_t *mock_get_rec(void *ctx, uint16_t rw)
{
    (void)ctx; (void)rw;
    return g_door_rec;
}

static void test_release_door_button(void)
{
    DM2_V1_RecordAddressCallbacks cb = { mock_get_rec };
    memset(g_door_rec, 0, sizeof(g_door_rec));
    g_door_rec[3] = 0x0F;
    dm2_v1_process_timer_release_door_button(0x0C00, &cb, NULL);
    assert(g_door_rec[3] == 0x07); /* bit 3 cleared */
    dm2_v1_process_timer_release_door_button(0, NULL, NULL);
    printf("  PASS: release_door_button\n");
}

/* ---- DESTROY_DOOR ---- */
static uint8_t g_tile_byte;
static int g_redraw;
static uint8_t *mock_get_tile(void *ctx, uint8_t x, uint8_t y)
{
    (void)ctx; (void)x; (void)y;
    return &g_tile_byte;
}

static void test_destroy_door(void)
{
    g_redraw = 0;
    DM2_V1_DestroyDoorCallbacks cb = { mock_get_tile, 2, 2, &g_redraw };
    g_tile_byte = 0xF0;
    dm2_v1_process_timer_destroy_door(3, 5, &cb, NULL);
    assert(g_tile_byte == 0xF5); /* low 3 bits = 5 */
    assert(g_redraw == 3);

    /* Different map — no redraw */
    g_redraw = 0;
    cb.current_map = 1;
    cb.party_map = 2;
    g_tile_byte = 0x08;
    dm2_v1_process_timer_destroy_door(0, 0, &cb, NULL);
    assert(g_tile_byte == 0x0D);
    assert(g_redraw == 0);
    printf("  PASS: destroy_door\n");
}

/* ---- TIMER_3D ---- */
static int g_moved, g_noise;
static int mock_move_record(void *ctx, uint16_t rec, int16_t lev,
                            int16_t unused, int16_t x, int16_t y)
{
    (void)ctx; (void)rec; (void)lev; (void)unused; (void)x; (void)y;
    g_moved = 1;
    return 0;
}
static void mock_noise(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    g_noise = 1;
}

static void test_timer_3d(void)
{
    DM2_V1_Timer3DCallbacks cb = { mock_move_record, mock_noise };

    /* Move succeeds (returns 0), type != 0x3D — no noise */
    g_moved = g_noise = 0;
    dm2_v1_process_timer_3d(0x1400, 5, 3, 0x3C, &cb, NULL);
    assert(g_moved == 1);
    assert(g_noise == 0);

    /* Type 0x3D — always noise */
    g_moved = g_noise = 0;
    dm2_v1_process_timer_3d(0x1400, 5, 3, 0x3D, &cb, NULL);
    assert(g_noise == 1);

    printf("  PASS: timer_3d\n");
}

int main(void)
{
    printf("test_dm2_v1_timer_ops:\n");
    test_process_timer_light_darkness();
    test_process_timer_light_source();
    test_process_timer_light_zero();
    test_release_door_button();
    test_destroy_door();
    test_timer_3d();
    printf("All timer_ops tests passed.\n");
    return 0;
}
