/*
 * test_dm2_v1_clickrect_pc34_compat.c — unit tests for DM2 click rectangle zones.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_clickrect_pc34_compat.h"

static int passed = 0, failed = 0;
#define RUN(fn) do { fn(); passed++; } while(0)

static void test_init_table(void)
{
    DM2_V1_ClickRectNode table[DM2_V1_CLICKRECT_TABLE_SIZE];
    memset(table, 0xFF, sizeof(table));
    dm2_v1_init_clickrect_table(table);

    assert(table[0].w_00 == 0x000b);
    assert(table[0].b_02 == 0x00);
    assert(table[0].b_03 == 0x01);
    assert(table[0].node == NULL);

    assert(table[1].w_00 == 0x005c);
    assert(table[1].b_02 == 0x01);

    assert(table[7].w_00 == 0x0007);
    assert(table[7].b_03 == 0x0f);

    assert(table[8].b_05 == 0x60);
    assert(table[11].b_05 == 0x40);
    assert(table[11].b_02 == 0x02);

    assert(table[17].w_00 == (int16_t)0x8267);
    assert(table[17].b_03 == 0x0f);

    printf("  PASS: init_table\n");
}

static void test_set_clickrect_datas(void)
{
    DM2_V1_SetClickRectReceipt r = dm2_v1_set_clickrect_datas(10, 20, 50, 80);
    assert(r.set);
    assert(r.rx0 == 10);
    assert(r.ry0 == 20);
    assert(r.rx1 == 50);
    assert(r.ry1 == 80);
    assert(r.rect_w == 41);
    assert(r.rect_h == 61);
    printf("  PASS: set_clickrect_datas\n");
}

static void test_refresh_link1_not_linked(void)
{
    DM2_V1_ClickRectNode node;
    memset(&node, 0, sizeof(node));
    node.b_03 = 0x01; /* no 0x80 bit */

    DM2_V1_ClickRectNode *head = NULL;
    DM2_V1_RefreshClickRectReceipt r = dm2_v1_refresh_clickrectlink_1(&node, &head);
    assert(!r.was_linked);
    assert(!r.refreshed);
    printf("  PASS: refresh_link1_not_linked\n");
}

static void test_refresh_link1_unlink(void)
{
    DM2_V1_ClickRectNode node;
    DM2_V1_ClickRectData data;
    memset(&node, 0, sizeof(node));
    memset(&data, 0, sizeof(data));
    node.b_03 = (int8_t)0x81; /* linked */
    node.node = &data;
    data.next = NULL;

    DM2_V1_ClickRectNode *head = &node;
    DM2_V1_RefreshClickRectReceipt r = dm2_v1_refresh_clickrectlink_1(&node, &head);
    assert(r.was_linked);
    assert(r.refreshed);
    assert((node.b_03 & 0x80) == 0);
    printf("  PASS: refresh_link1_unlink\n");
}

static void test_refresh_link2_insert(void)
{
    DM2_V1_ClickRectNode node;
    DM2_V1_ClickRectData data;
    memset(&node, 0, sizeof(node));
    memset(&data, 0, sizeof(data));
    node.b_03 = 0x05; /* not linked, priority 5 */
    node.node = &data;

    DM2_V1_ClickRectNode *head = NULL;
    DM2_V1_RefreshClickRectReceipt r = dm2_v1_refresh_clickrectlink_2(&node, &head);
    assert(r.refreshed);
    assert(!r.was_linked);
    assert(head == &node);
    assert((node.b_03 & 0x80) != 0);
    printf("  PASS: refresh_link2_insert\n");
}

static void test_alloc_not_allocated(void)
{
    DM2_V1_ClickRectNode node;
    memset(&node, 0, sizeof(node));
    node.b_03 = 0x01; /* no 0x40 bit */

    DM2_V1_AllocClickRectReceipt r = dm2_v1_alloc_clickrect_data(&node);
    assert(r.allocated);
    assert(!r.was_already_allocated);
    assert(node.node != NULL);
    assert((node.b_03 & 0x40) != 0);
    free(node.node);
    printf("  PASS: alloc_not_allocated\n");
}

static void test_alloc_already_allocated(void)
{
    DM2_V1_ClickRectNode node;
    memset(&node, 0, sizeof(node));
    node.b_03 = 0x41; /* already has 0x40 */

    DM2_V1_AllocClickRectReceipt r = dm2_v1_alloc_clickrect_data(&node);
    assert(!r.allocated);
    assert(r.was_already_allocated);
    printf("  PASS: alloc_already_allocated\n");
}

static void test_null_safety(void)
{
    dm2_v1_init_clickrect_table(NULL);
    dm2_v1_refresh_clickrectlink_1(NULL, NULL);
    dm2_v1_refresh_clickrectlink_2(NULL, NULL);
    dm2_v1_alloc_clickrect_data(NULL);
    printf("  PASS: null_safety\n");
}

int main(void)
{
    printf("test_dm2_v1_clickrect_pc34_compat:\n");
    RUN(test_init_table);
    RUN(test_set_clickrect_datas);
    RUN(test_refresh_link1_not_linked);
    RUN(test_refresh_link1_unlink);
    RUN(test_refresh_link2_insert);
    RUN(test_alloc_not_allocated);
    RUN(test_alloc_already_allocated);
    RUN(test_null_safety);
    printf("  %d passed, %d failed\n", passed, failed);
    return failed;
}
