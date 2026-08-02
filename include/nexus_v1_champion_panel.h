
#ifndef NEXUS_V1_CHAMPION_PANEL_H
#define NEXUS_V1_CHAMPION_PANEL_H

/* Nexus champion panel UI click rectangles — DM.BIN 0x038428.
 * Three groups of screen-space rectangles for the champion/inventory UI:
 *   Group 1: champion stat bar selectors (action 0x2D)
 *   Group 2: inventory grid 2x2 (action 0x29)
 *   Group 3: equipment body slots (action 0x2C)
 * Source: yam\inventry.c, yam\menuctrl.c debug strings nearby. */

typedef struct {
    int x1, y1, x2, y2;
    int action;
    int param;
} Nexus_PanelRect;

#define NEXUS_PANEL_ACTION_STAT_BAR   0x2D
#define NEXUS_PANEL_ACTION_INV_SLOT   0x29
#define NEXUS_PANEL_ACTION_EQUIP_SLOT 0x2C
#define NEXUS_PANEL_ACTION_FULLSCREEN 0x12

#define NEXUS_STAT_BAR_RECT_COUNT  12
#define NEXUS_INV_SLOT_RECT_COUNT   4
#define NEXUS_EQUIP_SLOT_RECT_COUNT 8

const Nexus_PanelRect* nexus_v1_stat_bar_rects(void);
const Nexus_PanelRect* nexus_v1_inv_slot_rects(void);
const Nexus_PanelRect* nexus_v1_equip_slot_rects(void);

/* Hit test: returns the matching rect, or NULL if no hit. */
const Nexus_PanelRect* nexus_v1_panel_hit_test(
    const Nexus_PanelRect* rects, int count, int x, int y);

#endif
