/*
 * dm2_v1_clickrect_pc34_compat.c — DM2 click rectangle zones.
 *
 * Source: skproject c_clickrect.cpp
 * Reference: c_clickrect.h (c_clickrectnode, c_clickrectdata)
 *
 * Functions:
 *   init_clickrect_table      — initialize 18-entry click rect table
 *   set_clickrect_datas       — compute click rect coordinates
 *   refresh_clickrectlink_1   — unlink node (clear 0x80)
 *   refresh_clickrectlink_2   — insert node sorted by priority (set 0x80)
 *   alloc_clickrect_data      — allocate node data (set 0x40)
 */

#include "dm2_v1_clickrect_pc34_compat.h"
#include <stdlib.h>
#include <string.h>

/* Initial table from skproject c_clickrect.cpp */
static const struct {
    int16_t w_00;
    int8_t  b_02;
    int8_t  b_03;
    int8_t  buttons;
    int8_t  b_05;
} clickrect_init_data[DM2_V1_CLICKRECT_TABLE_SIZE] = {
    { 0x000b, 0x00, 0x01, 0x00, 0x00 },
    { 0x005c, 0x01, 0x01, 0x00, 0x00 },
    { 0x00a1, 0x01, 0x01, 0x00, 0x00 },
    { 0x00a2, 0x01, 0x01, 0x00, 0x00 },
    { 0x00a3, 0x01, 0x01, 0x00, 0x00 },
    { 0x00a4, 0x01, 0x01, 0x00, 0x00 },
    { 0x0009, 0x00, 0x01, 0x00, 0x00 },
    { 0x0007, 0x01, 0x0f, 0x00, 0x00 },
    { 0x0007, 0x01, 0x0f, 0x00, 0x60 },
    { (int16_t)0x823b, 0x00, 0x0f, 0x00, 0x00 },
    { (int16_t)0x81ee, 0x00, 0x0f, 0x00, 0x00 },
    { 0x002f, 0x02, 0x0f, 0x00, 0x40 },
    { 0x000b, 0x00, 0x01, 0x00, 0x00 },
    { 0x009c, 0x00, 0x02, 0x00, 0x00 },
    { 0x009d, 0x00, 0x02, 0x00, 0x00 },
    { 0x009e, 0x00, 0x02, 0x00, 0x00 },
    { 0x009f, 0x00, 0x02, 0x00, 0x00 },
    { (int16_t)0x8267, 0x00, 0x0f, 0x00, 0x00 },
};

void dm2_v1_init_clickrect_table(DM2_V1_ClickRectNode table[DM2_V1_CLICKRECT_TABLE_SIZE])
{
    if (!table) return;
    for (int i = 0; i < DM2_V1_CLICKRECT_TABLE_SIZE; i++) {
        table[i].w_00    = clickrect_init_data[i].w_00;
        table[i].b_02    = clickrect_init_data[i].b_02;
        table[i].b_03    = clickrect_init_data[i].b_03;
        table[i].buttons = clickrect_init_data[i].buttons;
        table[i].b_05    = clickrect_init_data[i].b_05;
        table[i].node    = NULL;
    }
}

DM2_V1_SetClickRectReceipt dm2_v1_set_clickrect_datas(
    int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    DM2_V1_SetClickRectReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    receipt.set = true;
    receipt.rx0 = x0;
    receipt.ry0 = y0;
    receipt.rx1 = x1;
    receipt.ry1 = y1;
    receipt.rect_w = x1 - x0 + 1;
    receipt.rect_h = y1 - y0 + 1;
    return receipt;
}

DM2_V1_RefreshClickRectReceipt dm2_v1_refresh_clickrectlink_1(
    DM2_V1_ClickRectNode *node,
    DM2_V1_ClickRectNode **list_head)
{
    DM2_V1_RefreshClickRectReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    if (!node || !list_head) return receipt;

    /* If not linked (0x80 not set), nothing to do */
    if ((node->b_03 & 0x80) == 0) {
        receipt.was_linked = false;
        return receipt;
    }

    receipt.was_linked = true;
    node->b_03 &= 0x7f;  /* Clear linked flag */

    /* Walk list to find and unlink this node */
    DM2_V1_ClickRectNode **pp = list_head;
    DM2_V1_ClickRectNode *cur;
    for (cur = *list_head; cur != NULL && cur != node; ) {
        if (cur->node)
            pp = (DM2_V1_ClickRectNode **)&cur->node->next;
        cur = cur->node ? cur->node->next : NULL;
    }

    if (cur == node && node->node) {
        *pp = node->node->next;
    }

    receipt.refreshed = true;
    return receipt;
}

DM2_V1_RefreshClickRectReceipt dm2_v1_refresh_clickrectlink_2(
    DM2_V1_ClickRectNode *node,
    DM2_V1_ClickRectNode **list_head)
{
    DM2_V1_RefreshClickRectReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    if (!node || !list_head) return receipt;

    /* If already linked, nothing to do */
    if ((node->b_03 & 0x80) != 0) {
        receipt.was_linked = true;
        return receipt;
    }

    node->b_03 |= (int8_t)0x80;  /* Set linked flag */

    /* Walk list to find insertion point (sorted by priority, low nibble) */
    DM2_V1_ClickRectNode **pp = list_head;
    DM2_V1_ClickRectNode *cur = *list_head;

    while (cur != NULL && ((cur->b_03 & 0xf) > (node->b_03 & 0xf))) {
        if (cur->node)
            pp = (DM2_V1_ClickRectNode **)&cur->node->next;
        cur = cur->node ? cur->node->next : NULL;
    }

    /* Insert node here */
    if (node->node) {
        node->node->next = cur;
    }
    *pp = node;

    receipt.refreshed = true;
    return receipt;
}

DM2_V1_AllocClickRectReceipt dm2_v1_alloc_clickrect_data(
    DM2_V1_ClickRectNode *node)
{
    DM2_V1_AllocClickRectReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    if (!node) return receipt;

    if ((node->b_03 & 0x40) != 0) {
        receipt.was_already_allocated = true;
        return receipt;
    }

    node->b_03 |= 0x40;
    node->node = (DM2_V1_ClickRectData *)calloc(1, sizeof(DM2_V1_ClickRectData));
    receipt.allocated = (node->node != NULL);
    return receipt;
}
