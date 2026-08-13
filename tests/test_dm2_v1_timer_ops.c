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
    dm2_v1_process_timer_light_tile(3, &cb, NULL);
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
    dm2_v1_process_timer_light_tile(-2, &cb, NULL);
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
    assert(dm2_v1_process_timer_light_tile(0, &cb, NULL) == 0);
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
    dm2_v1_process_timer_destroy_door_tile(3, 5, &cb, NULL);
    assert(g_tile_byte == 0xF5); /* low 3 bits = 5 */
    assert(g_redraw == 3);

    /* Different map — no redraw */
    g_redraw = 0;
    cb.current_map = 1;
    cb.party_map = 2;
    g_tile_byte = 0x08;
    dm2_v1_process_timer_destroy_door_tile(0, 0, &cb, NULL);
    assert(g_tile_byte == 0x0D);
    assert(g_redraw == 0);
    printf("  PASS: destroy_door\n");
}

/* ---- TIMER_3D / MOVE_RECORD_ROTATE ---- */
static int g_moved, g_noise;
static int16_t g_move_x, g_move_y, g_rotate_dir;
static int mock_move_record(void *ctx, uint16_t rec, int16_t lev,
                            int16_t unused, int16_t x, int16_t y)
{
    (void)ctx; (void)rec; (void)lev; (void)unused;
    g_moved = 1;
    g_move_x = x;
    g_move_y = y;
    return 0;
}
static void mock_party_rotate(void *ctx, int16_t direction)
{
    (void)ctx;
    g_rotate_dir = direction;
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
    dm2_v1_process_timer_3d_tile(0x1400, 5, 3, 0x3C, &cb, NULL);
    assert(g_moved == 1);
    assert(g_noise == 0);

    /* Type 0x3D — always noise */
    g_moved = g_noise = 0;
    dm2_v1_process_timer_3d_tile(0x1400, 5, 3, 0x3D, &cb, NULL);
    assert(g_noise == 1);

    printf("  PASS: timer_3d\n");
}

static void test_move_record_rotate_payload(void)
{
    DM2_V1_MoveRecordRotateCallbacks cb = {
        mock_move_record, mock_party_rotate, 4
    };
    const uint16_t value_a = (uint16_t)(7u | (9u << 5) | (2u << 10));

    g_moved = 0;
    g_move_x = g_move_y = g_rotate_dir = -1;
    dm2_v1_process_timer_move_record_rotate(value_a, 4, 12, 13, &cb, NULL);
    assert(g_moved == 1);
    assert(g_move_x == 7);
    assert(g_move_y == (int16_t)((value_a << 6) >> 11));
    assert(g_rotate_dir == (int16_t)((value_a << 4) >> 14));

    g_moved = 0;
    dm2_v1_process_timer_move_record_rotate(value_a, 3, 12, 13, &cb, NULL);
    assert(g_moved == 0);
    printf("  PASS: move_record_rotate_payload\n");
}

/* ---- RESURRECTION source payload/phase ---- */
static int16_t g_res_record;
static int g_res_cut, g_res_dealloc, g_res_add_mode;
static uint8_t g_res_next_yb;
static int16_t g_cloud_type, g_cloud_param, g_cloud_x, g_cloud_y, g_cloud_cls;

static int16_t mock_res_tile_link(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return g_res_record;
}
static int16_t mock_res_next_link(void *ctx, uint16_t record)
{
    (void)ctx; (void)record;
    return -2; /* source OBJECT_END */
}
static int16_t mock_res_cls1(void *ctx, int32_t record)
{
    (void)ctx; (void)record;
    return 0x15;
}
static int16_t mock_res_cls2(void *ctx, int32_t record)
{
    (void)ctx; (void)record;
    return 0;
}
static int16_t mock_res_add_charge(void *ctx, int32_t record, int mode)
{
    (void)ctx; (void)record;
    g_res_add_mode = mode;
    return 0;
}
static void mock_res_cut(void *ctx, uint16_t record, int16_t x, int16_t y)
{
    (void)ctx; (void)record; (void)x; (void)y;
    g_res_cut = 1;
}
static void mock_res_dealloc(void *ctx, uint16_t record)
{
    (void)ctx; (void)record;
    g_res_dealloc = 1;
}
static int16_t mock_res_cloud(void *ctx, int16_t type, int16_t param,
                              int16_t x, int16_t y, int16_t cls)
{
    (void)ctx;
    g_cloud_type = type;
    g_cloud_param = param;
    g_cloud_x = x;
    g_cloud_y = y;
    g_cloud_cls = cls;
    return 1;
}
static void mock_res_queue(void *ctx, uint8_t next_yb)
{
    (void)ctx;
    g_res_next_yb = next_yb;
}

static void test_resurrection_source_payload(void)
{
    DM2_V1_ResurrectionCallbacks cb = {
        NULL, mock_res_tile_link, mock_res_next_link,
        mock_res_cls1, mock_res_cls2, mock_res_add_charge,
        mock_res_cut, mock_res_dealloc, mock_res_cloud, mock_res_queue
    };

    g_res_record = (int16_t)0x8001; /* DB2, matching timer.xB=2 */
    g_res_cut = g_res_dealloc = 0;
    g_res_add_mode = -1;
    g_res_next_yb = 0xff;
    assert(dm2_v1_process_timer_resurrection_tile(
        3, 4, 2, 1, 0, &cb, NULL) == 1);
    assert(g_res_add_mode == 0);
    assert(g_res_cut == 1 && g_res_dealloc == 1);
    assert(g_res_next_yb == 0);

    g_cloud_type = g_cloud_param = g_cloud_x = g_cloud_y = g_cloud_cls = -1;
    g_res_next_yb = 0xff;
    assert(dm2_v1_process_timer_resurrection_tile(
        3, 4, 6, 2, 0, &cb, NULL) == 1);
    assert(g_cloud_type == (int16_t)0xffe4 && g_cloud_param == 0);
    assert(g_cloud_x == 3 && g_cloud_y == 4 && g_cloud_cls == 6);
    assert(g_res_next_yb == 1);
    printf("  PASS: resurrection_source_payload\n");
}

/* ---- PROCESS_CLOUD source word@2 decay/requeue ---- */
static uint8_t g_cloud_data[8];
static int g_cloud_queued, g_cloud_cut, g_cloud_dealloc, g_cloud_noise;
static uint8_t g_cloud_noise_cat, g_cloud_noise_idx, g_cloud_noise_vol, g_cloud_noise_pan;
static uint8_t g_cloud_noise_p1, g_cloud_noise_p2;

static uint8_t *mock_process_cloud_record(void *ctx, uint16_t rw)
{
    (void)ctx; (void)rw;
    return g_cloud_data;
}
static uint8_t mock_process_cloud_tile(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return 0x40;
}
static void mock_process_cloud_queue(void *ctx)
{
    (void)ctx;
    g_cloud_queued = 1;
}
static void mock_process_cloud_cut(void *ctx, uint16_t rw, int16_t x, int16_t y)
{
    (void)ctx; (void)rw; (void)x; (void)y;
    g_cloud_cut = 1;
}
static void mock_process_cloud_dealloc(void *ctx, uint16_t rw)
{
    (void)ctx; (void)rw;
    g_cloud_dealloc = 1;
}
static void mock_process_cloud_noise(void *ctx, uint8_t cat, uint8_t idx,
                                     uint8_t vol, uint8_t pan, int16_t x,
                                     int16_t y, int repeat, uint8_t p1,
                                     uint8_t p2)
{
    (void)ctx; (void)x; (void)y; (void)repeat;
    g_cloud_noise = 1;
    g_cloud_noise_cat = cat;
    g_cloud_noise_idx = idx;
    g_cloud_noise_vol = vol;
    g_cloud_noise_pan = pan;
    g_cloud_noise_p1 = p1;
    g_cloud_noise_p2 = p2;
}

static void test_process_cloud_source_decay(void)
{
    DM2_V1_ProcessCloudCallbacks cb = {
        .get_record_address = mock_process_cloud_record,
        .get_tile_value = mock_process_cloud_tile,
        .queue_timer = mock_process_cloud_queue,
        .cut_record_from = mock_process_cloud_cut,
        .dealloc_record = mock_process_cloud_dealloc,
        .queue_noise_gen2 = mock_process_cloud_noise,
        .current_map = 2,
        .party_map = 2
    };

    memset(g_cloud_data, 0, sizeof(g_cloud_data));
    g_cloud_data[2] = 0x07;
    g_cloud_data[3] = 0x06; /* source type 7, high-byte lifetime 6 */
    g_cloud_queued = g_cloud_cut = g_cloud_dealloc = 0;
    assert(dm2_v1_process_cloud_tile(0x1200, 3, 4, &cb, NULL) == 1);
    assert((uint16_t)(g_cloud_data[2] | (g_cloud_data[3] << 8)) == 0x0307);
    assert(g_cloud_queued == 1 && g_cloud_cut == 0 && g_cloud_dealloc == 0);

    g_cloud_data[2] = 0x64;
    g_cloud_data[3] = 0x01; /* source type 0x64 */
    g_cloud_queued = g_cloud_cut = g_cloud_dealloc = g_cloud_noise = 0;
    assert(dm2_v1_process_cloud_tile(0x1200, 3, 4, &cb, NULL) == 1);
    assert((g_cloud_data[2] & 0x7f) == 0x65);
    assert(g_cloud_queued == 1 && g_cloud_cut == 0 && g_cloud_dealloc == 0);
    assert(g_cloud_noise == 1 && g_cloud_noise_cat == 0x0d &&
           g_cloud_noise_idx == 0x64 && g_cloud_noise_vol == 0x81 &&
           g_cloud_noise_pan == 0xfe && g_cloud_noise_p1 == 0x6c &&
           g_cloud_noise_p2 == 0xc8);
    printf("  PASS: process_cloud_source_decay\n");
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
    test_move_record_rotate_payload();
    test_resurrection_source_payload();
    test_process_cloud_source_decay();
    printf("All timer_ops tests passed.\n");
    return 0;
}
