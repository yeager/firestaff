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

/* ---- try_push_object_to tests ---- */

static int g_tile_free_mask;
static int g_tile_check_count;

static int mock_is_tile_free(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    int dir = g_tile_check_count++;
    return (g_tile_free_mask >> dir) & 1;
}

static void mock_move_to(void *ctx, int32_t rec, int16_t x, int16_t y)
{
    (void)ctx; (void)rec; (void)x; (void)y;
}

static void test_try_push_all_blocked(void)
{
    g_tile_free_mask = 0;
    g_tile_check_count = 0;
    int16_t ox, oy;
    DM2_V1_TryPushObjectToCallbacks cb = { mock_is_tile_free, mock_move_to };
    int32_t r = dm2_v1_try_push_object_to(0x1000, 5, 5, &ox, &oy, &cb, NULL);
    assert(r == 0);
    printf("  PASS: try_push_all_blocked\n");
}

static void test_try_push_south_free(void)
{
    g_tile_free_mask = 4; /* bit 2 = south */
    g_tile_check_count = 0;
    int16_t ox, oy;
    DM2_V1_TryPushObjectToCallbacks cb = { mock_is_tile_free, mock_move_to };
    int32_t r = dm2_v1_try_push_object_to(0x1000, 5, 5, &ox, &oy, &cb, NULL);
    assert(r == 1);
    printf("  PASS: try_push_south_free\n");
}

/* ---- moverec_2fcf_0234 tests ---- */

static int g_unlinked, g_linked;

static void mock_unlink(void *ctx, int32_t rec, int16_t x, int16_t y)
{
    (void)ctx; (void)rec; (void)x; (void)y;
    g_unlinked = 1;
}

static void mock_link(void *ctx, int32_t rec, int16_t x, int16_t y)
{
    (void)ctx; (void)rec; (void)x; (void)y;
    g_linked = 1;
}

static void test_moverec_relink(void)
{
    g_unlinked = 0;
    g_linked = 0;
    DM2_V1_Moverec2fcf0234Callbacks cb = { mock_unlink, mock_link };
    dm2_v1_moverec_2fcf_0234(0x2000, 3, 4, 7, 8, &cb, NULL);
    assert(g_unlinked == 1);
    assert(g_linked == 1);
    printf("  PASS: moverec_relink\n");
}

/* ---- moverec_3ce7d tests ---- */

static int32_t g_dispatched;

static int32_t mock_dispatch(void *ctx, int32_t rec, int16_t x, int16_t y,
                             int32_t kind, int32_t flags)
{
    (void)ctx; (void)x; (void)y; (void)kind; (void)flags;
    g_dispatched = rec;
    return 1;
}

static void test_moverec_3ce7d(void)
{
    g_dispatched = 0;
    DM2_V1_Moverec3ce7dCallbacks cb = { mock_dispatch };
    dm2_v1_moverec_3ce7d(0x3000, 10, 20, 5, 0xFF, &cb, NULL);
    assert(g_dispatched == 0x3000);
    printf("  PASS: moverec_3ce7d\n");
}

int main(void)
{
    printf("test_dm2_v1_moverec_ops:\n");
    test_set_minion_door();
    test_moverec_timer();
    test_try_push_all_blocked();
    test_try_push_south_free();
    test_moverec_relink();
    test_moverec_3ce7d();
    printf("All moverec_ops tests passed.\n");
    return 0;
}
