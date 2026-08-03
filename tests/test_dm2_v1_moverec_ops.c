/* Test DM2 V1 moverec operations (c_moverec.cpp). */

#include "dm2_v1_moverec_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Mock record chain: creature -> type-0xE record -> END */
static uint8_t g_creature_rec[16];
static uint8_t g_type_e_rec[8];

static uint8_t *mock_get_record(void *ctx, uint16_t rw)
{
    (void)ctx;
    uint16_t db = (rw >> 10) & 0xF;
    if (db == 4) return g_creature_rec;
    if (db == 0xE) return g_type_e_rec;
    return NULL;
}

static int16_t mock_get_next(void *ctx, uint16_t rw)
{
    (void)ctx;
    uint16_t db = (rw >> 10) & 0xF;
    if (db == 0xE) return (int16_t)0xFFFE;
    return (int16_t)0xFFFE;
}

static void test_set_minion_door(void)
{
    memset(g_creature_rec, 0, sizeof(g_creature_rec));
    memset(g_type_e_rec, 0, sizeof(g_type_e_rec));
    /* creature word+2 points to type-0xE record: db_type=0xE -> bits 10-13 = 0xE */
    uint16_t type_e_word = (0xE << 10) | 0x01;
    g_creature_rec[2] = (uint8_t)(type_e_word & 0xFF);
    g_creature_rec[3] = (uint8_t)(type_e_word >> 8);

    DM2_V1_MinionDoorCallbacks cb = { mock_get_record, mock_get_next };
    dm2_v1_set_minion_recent_open_door_location(
        (4 << 10) | 0x00, 10, 15, 3, 1, &cb, NULL);
    uint16_t w4 = (uint16_t)(g_type_e_rec[4] | (g_type_e_rec[5] << 8));
    assert((w4 & 0x1F) == 10);
    assert(((w4 >> 5) & 0x1F) == 15);
    assert(((w4 >> 10) & 0x3F) == 3);
    assert((g_type_e_rec[6] & 1) == 1);
    printf("  PASS: set_minion_door\n");
}

static int g_timer_queued;
static int g_minion_set;

static void mock_queue_timer(void *ctx, uint8_t type, uint8_t x, uint8_t y,
                             uint16_t rec, uint32_t tick)
{
    (void)ctx; (void)x; (void)y; (void)rec; (void)tick;
    g_timer_queued = type;
}

static void mock_set_minion(void *ctx, uint16_t rec, uint8_t x, uint8_t y,
                            uint8_t map, int flag)
{
    (void)ctx; (void)rec; (void)x; (void)y; (void)map;
    g_minion_set = flag;
}

static void test_moverec_timer(void)
{
    g_timer_queued = 0;
    g_minion_set = 0;
    DM2_V1_MoverecTimerCallbacks cb = { 100, mock_queue_timer, mock_set_minion };
    dm2_v1_moverec_2fcf_01c5(0x1234, 5, 6, 2, 1, &cb, NULL);
    assert(g_timer_queued == 0x3D);
    assert(g_minion_set == 1);

    dm2_v1_moverec_2fcf_01c5(0x1234, 5, 6, 2, 0, &cb, NULL);
    assert(g_timer_queued == 0x3C);
    printf("  PASS: moverec_timer\n");
}

int main(void)
{
    printf("test_dm2_v1_moverec_ops:\n");
    test_set_minion_door();
    test_moverec_timer();
    printf("All moverec_ops tests passed.\n");
    return 0;
}
