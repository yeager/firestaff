
#include "nexus_v1_champion_panel.h"
#include <stddef.h>

/* Champion stat bar selectors — DM.BIN 0x038428.
 * 4 pairs across the screen, each pair has two 16x16 stat bars
 * plus a group selector rect. Param 0-7 = champion index,
 * negative params (0xFFFF-0xFFFC as signed) = pair group selectors. */
static const Nexus_PanelRect g_stat_bars[NEXUS_STAT_BAR_RECT_COUNT] = {
    { 32,  25,  48,  41, 0x2D,  0},
    { 48,  25,  64,  41, 0x2D,  1},
    { 17,  18,  31,  42, 0x2D, -1},
    { 96,  25, 112,  41, 0x2D,  2},
    {112,  25, 128,  41, 0x2D,  3},
    { 81,  18,  95,  42, 0x2D, -2},
    {160,  25, 176,  41, 0x2D,  4},
    {176,  25, 192,  41, 0x2D,  5},
    {145,  18, 159,  42, 0x2D, -3},
    {224,  25, 240,  41, 0x2D,  6},
    {240,  25, 256,  41, 0x2D,  7},
    {209,  18, 223,  42, 0x2D, -4},
};

/* Inventory grid — DM.BIN 0x0384D0. 2x2 slots at bottom-right. */
static const Nexus_PanelRect g_inv_slots[NEXUS_INV_SLOT_RECT_COUNT] = {
    {192, 152, 208, 168, 0x29, 0},
    {208, 152, 224, 168, 0x29, 1},
    {192, 168, 208, 184, 0x29, 2},
    {208, 168, 224, 184, 0x29, 3},
};

/* Equipment body slots — DM.BIN 0x038508. 8 slots around champion portrait. */
static const Nexus_PanelRect g_equip_slots[NEXUS_EQUIP_SLOT_RECT_COUNT] = {
    {257, 104, 273, 120, 0x2C, 0},
    {288, 104, 304, 120, 0x2C, 1},
    {272,  72, 288,  88, 0x2C, 2},
    {272,  91, 288, 107, 0x2C, 3},
    {273, 115, 289, 131, 0x2C, 4},
    {272, 136, 288, 152, 0x2C, 5},
    {248,  80, 264,  96, 0x2C, 6},
    {252, 136, 268, 152, 0x2C, 7},
};

const Nexus_PanelRect* nexus_v1_stat_bar_rects(void) {
    return g_stat_bars;
}

const Nexus_PanelRect* nexus_v1_inv_slot_rects(void) {
    return g_inv_slots;
}

const Nexus_PanelRect* nexus_v1_equip_slot_rects(void) {
    return g_equip_slots;
}

const Nexus_PanelRect* nexus_v1_panel_hit_test(
    const Nexus_PanelRect* rects, int count, int x, int y)
{
    if (!rects) return NULL;
    for (int i = 0; i < count; i++) {
        if (x >= rects[i].x1 && x < rects[i].x2 &&
            y >= rects[i].y1 && y < rects[i].y2)
            return &rects[i];
    }
    return NULL;
}
