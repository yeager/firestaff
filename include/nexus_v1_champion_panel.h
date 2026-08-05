
#ifndef NEXUS_V1_CHAMPION_PANEL_H
#define NEXUS_V1_CHAMPION_PANEL_H

#include <stddef.h>
#include <stdint.h>

/* Nexus champion panel UI click rectangles from the retail DM.BIN.
 * Three groups of screen-space rectangles for the champion/inventory UI:
 *   Group 1: champion stat bar selectors (action 0x2D)
 *   Group 2: inventory grid 2x2 (action 0x29)
 *   Group 3: equipment body slots (action 0x2C)
 * Each record is six big-endian uint16 values: x1,y1,x2,y2,action,param.
 * Source: DM.BIN yam\\inventry.c / yam\\menuctrl.c tables. */

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

#define NEXUS_PANEL_RECT_BYTES 12U
#define NEXUS_PANEL_STAT_OFFSET 0x38428U
#define NEXUS_PANEL_INV_OFFSET  0x384D0U
/* DM.BIN stores a four-byte 0xFFFF/zero table separator before equipment. */
#define NEXUS_PANEL_EQUIP_OFFSET 0x3850CU

/* Parse all three retail tables. No static or inferred geometry is returned.
 * Returns 0 on success and -1 on missing/truncated/malformed input. */
int nexus_v1_champion_panel_parse_dm_bin(
    const uint8_t *data, size_t data_size,
    Nexus_PanelRect stat_bars[NEXUS_STAT_BAR_RECT_COUNT],
    Nexus_PanelRect inv_slots[NEXUS_INV_SLOT_RECT_COUNT],
    Nexus_PanelRect equip_slots[NEXUS_EQUIP_SLOT_RECT_COUNT]);

/* Hit test: returns the matching rect, or NULL if no hit. */
const Nexus_PanelRect* nexus_v1_panel_hit_test(
    const Nexus_PanelRect* rects, int count, int x, int y);

#endif
