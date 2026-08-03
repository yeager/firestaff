/* Test DM2 V1 world/tile operations. */

#include "dm2_v1_world_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- SET_TILE_ATTRIBUTE_02 ---- */
static uint8_t g_tile_attr;
static uint8_t *mock_get_tile_attr(void *ctx, int16_t x, int16_t y, int16_t map)
{
    (void)ctx; (void)x; (void)y; (void)map;
    return &g_tile_attr;
}

static void test_set_tile_attribute_02(void)
{
    DM2_V1_TileAttrCallbacks cb = { mock_get_tile_attr };
    g_tile_attr = 0x00;
    dm2_v1_set_tile_attribute_02(3, 5, 0, &cb, NULL);
    assert(g_tile_attr == 0x02);
    g_tile_attr = 0x01;
    dm2_v1_set_tile_attribute_02(3, 5, 0, &cb, NULL);
    assert(g_tile_attr == 0x03);
    dm2_v1_set_tile_attribute_02(0, 0, 0, NULL, NULL);
    printf("  PASS: set_tile_attribute_02\n");
}

/* ---- __CHECK_ROOM_FOR_CONTAINER ---- */
static uint16_t g_container_items[4];
static int g_container_idx;

static uint16_t mock_get_contained(void *ctx, uint16_t rw)
{
    (void)ctx; (void)rw;
    if (g_container_idx >= 4 || g_container_items[g_container_idx] == 0xFFFF)
        return 0xFFFE;
    return g_container_items[g_container_idx];
}

static int g_cut_count;
static void mock_cut_record(void *ctx, uint16_t container, uint16_t item)
{
    (void)ctx; (void)container; (void)item;
    g_container_idx++;
    g_cut_count++;
}

static void test_check_room_for_container(void)
{
    DM2_V1_ContainerCallbacks cb = { mock_get_contained, mock_cut_record };
    uint16_t items[8];
    uint16_t current = 0xFFFF;

    g_container_items[0] = 0x1400;
    g_container_items[1] = 0x1800;
    g_container_items[2] = 0xFFFF;
    g_container_idx = 0;
    g_cut_count = 0;

    int n = dm2_v1_check_room_for_container(0x100, &current, items, 8, &cb, NULL);
    assert(n == 2);
    assert(items[0] == 0x1400);
    assert(items[1] == 0x1800);
    assert(items[2] == 0xFFFF);
    assert(current == 0x100);
    assert(g_cut_count == 2);

    /* Already loaded — should return 0 */
    n = dm2_v1_check_room_for_container(0x100, &current, items, 8, &cb, NULL);
    assert(n == 0);

    printf("  PASS: check_room_for_container\n");
}

/* ---- CREATURE_ROTATES_TARGET_CREATURE ---- */
static uint16_t g_rotate_creature_rw;
static int g_rotate_dir;

static uint16_t mock_get_creature_at(void *ctx, int16_t x, int16_t y)
{
    (void)ctx;
    if (x == 5 && y == 3) return 0x3C00;
    return 0xFFFF;
}

static void mock_rotate_creature(void *ctx, uint16_t rw, int mode, int dir)
{
    (void)ctx;
    g_rotate_creature_rw = rw;
    g_rotate_dir = dir;
}

static void test_creature_rotates_target(void)
{
    DM2_V1_CreatureRotateTargetCallbacks cb = {
        mock_get_creature_at, mock_rotate_creature
    };
    g_rotate_creature_rw = 0;
    assert(dm2_v1_creature_rotates_target_creature(5, 3, 2, &cb, NULL) == 0);
    assert(g_rotate_creature_rw == 0x3C00);
    assert(g_rotate_dir == 2);

    /* No creature at position */
    assert(dm2_v1_creature_rotates_target_creature(0, 0, 1, &cb, NULL) == 1);
    printf("  PASS: creature_rotates_target\n");
}

int main(void)
{
    printf("test_dm2_v1_world_ops:\n");
    test_set_tile_attribute_02();
    test_check_room_for_container();
    test_creature_rotates_target();
    printf("All world_ops tests passed.\n");
    return 0;
}
