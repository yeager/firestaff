/* Test DM2 V1 record operations (c_record.cpp + SkWinCore2.cpp). */

#include "dm2_v1_record_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Mock callbacks for GDAT dbspec ---- */
static uint8_t mock_cls1(void *ctx, uint16_t rw) { (void)ctx; return (uint8_t)((rw >> 10) & 0xF); }
static uint8_t mock_cls2(void *ctx, uint16_t rw) { (void)ctx; return (uint8_t)(rw & 0x3F); }
static int16_t mock_gdat_idx(void *ctx, uint8_t c1, uint8_t c2, uint8_t et, uint8_t di)
{
    (void)ctx;
    if (c1 == 5 && c2 == 3 && et == 11 && di == 7) return 42;
    return 0;
}

static void test_query_gdat_dbspec_word_value(void)
{
    DM2_V1_GdatDbspecCallbacks cb = { mock_cls1, mock_cls2, mock_gdat_idx };
    assert(dm2_v1_query_gdat_dbspec_word_value(0xFFFF, 7, &cb, NULL) == 0);
    uint16_t rw = (5 << 10) | 3;
    assert(dm2_v1_query_gdat_dbspec_word_value(rw, 7, &cb, NULL) == 42);
    assert(dm2_v1_query_gdat_dbspec_word_value(0, 0, NULL, NULL) == 0);
    printf("  PASS: query_gdat_dbspec_word_value\n");
}

/* ---- Mock tile record walk ---- */
static int16_t g_chain[8];
static int g_chain_len;

static int16_t mock_tile_link(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return g_chain_len > 0 ? g_chain[0] : (int16_t)0xFFFE;
}

static int16_t mock_next_link(void *ctx, uint16_t rw)
{
    (void)ctx;
    for (int i = 0; i < g_chain_len - 1; i++) {
        if ((uint16_t)g_chain[i] == rw)
            return g_chain[i + 1];
    }
    return (int16_t)0xFFFE;
}

static void test_get_wall_tile_anyitem_record(void)
{
    DM2_V1_TileRecordWalkCallbacks cb = { mock_tile_link, mock_next_link };
    /* Chain: type 1, type 2, type 5 (first item) */
    g_chain[0] = (int16_t)(1 << 10 | 0); /* type 1 */
    g_chain[1] = (int16_t)(2 << 10 | 1); /* type 2 */
    g_chain[2] = (int16_t)(5 << 10 | 2); /* type 5 = weapon */
    g_chain_len = 3;
    int16_t r = dm2_v1_get_wall_tile_anyitem_record(0, 0, &cb, NULL);
    assert((uint16_t)r == (uint16_t)g_chain[2]);

    /* Empty chain */
    g_chain_len = 0;
    r = dm2_v1_get_wall_tile_anyitem_record(0, 0, &cb, NULL);
    assert((uint16_t)r == 0xFFFE);
    printf("  PASS: get_wall_tile_anyitem_record\n");
}

static void test_get_wall_tile_any_takeable_item(void)
{
    DM2_V1_TileRecordWalkCallbacks cb = { mock_tile_link, mock_next_link };
    /* dir=2, type=6 (clothing) */
    g_chain[0] = (int16_t)((2u << 14) | (6 << 10) | 0);
    g_chain_len = 1;
    int16_t r = dm2_v1_get_wall_tile_any_takeable_item_record(0, 0, 2, &cb, NULL);
    assert(r == g_chain[0]);
    /* Wrong direction */
    r = dm2_v1_get_wall_tile_any_takeable_item_record(0, 0, 1, &cb, NULL);
    assert((uint16_t)r == 0xFFFF);
    printf("  PASS: get_wall_tile_any_takeable_item_record\n");
}

static void test_wall_decoration(void)
{
    uint8_t act[8] = {0};
    act[4] = 0x30; /* deco_count = 3 */
    uint8_t table[4] = {10, 20, 30, 40};
    assert(dm2_v1_get_wall_decoration_of_actuator(act, table, 4) == 30);
    act[4] = 0x00; /* no decoration */
    assert(dm2_v1_get_wall_decoration_of_actuator(act, table, 4) == 0xFF);
    assert(dm2_v1_get_wall_decoration_of_actuator(NULL, table, 4) == 0xFF);
    printf("  PASS: wall_decoration\n");
}

static void test_floor_decoration(void)
{
    uint8_t act[8] = {0};
    act[4] = 0x10; /* deco_count = 1 */
    uint8_t table[2] = {55, 66};
    assert(dm2_v1_get_floor_decoration_of_actuator(act, table, 2) == 55);
    printf("  PASS: floor_decoration\n");
}

static int g_deleted_idx;
static const uint8_t *mock_rec_addr(void *ctx, uint16_t rw)
{
    (void)ctx; (void)rw;
    static uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0x07, 0x00};
    return data;
}
static void mock_del_timer(void *ctx, int16_t idx)
{
    (void)ctx;
    g_deleted_idx = idx;
}

static void test_missile_timer_cleanup(void)
{
    DM2_V1_MissileCleanupCallbacks cb = { mock_rec_addr, mock_del_timer };
    g_deleted_idx = -1;
    uint16_t missile_rw = (0xE << 10) | 5;
    assert(dm2_v1_missile_timer_cleanup(missile_rw, &cb, NULL) == 1);
    assert(g_deleted_idx == 7);
    /* Non-missile */
    uint16_t item_rw = (5 << 10) | 3;
    assert(dm2_v1_missile_timer_cleanup(item_rw, &cb, NULL) == 0);
    printf("  PASS: missile_timer_cleanup\n");
}

static void test_rotate_record_by_teleporter(void)
{
    uint8_t out_dir = 0;
    /* Absolute mode (bit 4 clear) */
    uint16_t rw = (2u << 14) | 0x100;
    uint16_t result = dm2_v1_rotate_record_by_teleporter(0x00, 3, rw, &out_dir);
    assert(out_dir == 3);
    /* record word unchanged in absolute mode */
    assert(result == rw);

    /* Relative mode (bit 4 set) */
    rw = (1u << 14) | 0x200;
    result = dm2_v1_rotate_record_by_teleporter(0x10, 3, rw, &out_dir);
    assert(out_dir == 3);
    /* direction should be rotated by delta (3 - 1) = 2, so 1+2=3 */
    assert(((result >> 14) & 0x3) == 3);
    printf("  PASS: rotate_record_by_teleporter\n");
}

static int mock_text_db(void *ctx, uint16_t rw) { (void)ctx; (void)rw; return 0x02; }
static const uint8_t *mock_text_addr(void *ctx, uint16_t rw)
{
    (void)ctx; (void)rw;
    static uint8_t data[4] = {0, 0, 0x01, 0};
    return data;
}

static void test_is_object_visible_text(void)
{
    DM2_V1_ObjectTextCallbacks cb = { mock_text_db, mock_text_addr };
    assert(dm2_v1_is_object_visible_text(0x100, &cb, NULL) == 1);
    assert(dm2_v1_is_object_visible_text(0xFFFF, &cb, NULL) == 0);
    printf("  PASS: is_object_visible_text\n");
}

int main(void)
{
    printf("test_dm2_v1_record_ops:\n");
    test_query_gdat_dbspec_word_value();
    test_get_wall_tile_anyitem_record();
    test_get_wall_tile_any_takeable_item();
    test_wall_decoration();
    test_floor_decoration();
    test_missile_timer_cleanup();
    test_rotate_record_by_teleporter();
    test_is_object_visible_text();
    printf("All record_ops tests passed.\n");
    return 0;
}
