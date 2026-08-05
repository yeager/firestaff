
#include "nexus_v1_hud_layout.h"

#include <string.h>

/* HUD element layout table from DM.BIN yam\menuctrl.c at 0x0376D0.
 * Extracted from the real Saturn binary — 80 entries with sentinel groups.
 * Sentinel entries use element_id 0xFFFF to separate layout groups. */

static const Nexus_HudElement g_hud_layout[] = {
    /* Group 0: main game screen */
    {0x0040, 256, 125},   /*  0: viewport */
    {0x0098, 273,  90},   /*  1: status area */
    {0x0094,  14,   7},   /*  2: compass */
    {0x0044, 271,  50},   /*  3: party frame */
    {0x0001, 272,   9},   /*  4: champion 1 portrait */
    {0x0002, 288,   9},   /*  5: champion 2 portrait */
    {0x0003, 272,  25},   /*  6: champion 3 portrait */
    {0x0004, 288,  25},   /*  7: champion 4 portrait */
    {0x0045, 272,  10},   /*  8: champion 1 name */
    {0x0046, 288,  10},   /*  9: champion 2 name */
    {0x0047, 272,  26},   /* 10: champion 3 name */
    {0x0048, 288,  26},   /* 11: champion 4 name */
    {0x0090,  64,   4},   /* 12: HP bar champion 0 */
    {0x0090, 128,   4},   /* 13: HP bar champion 1 */
    {0x0090, 192,   4},   /* 14: HP bar champion 2 */
    {0x0090, 256,   4},   /* 15: HP bar champion 3 */
    {0x002E, 288,  48},   /* 16: spell panel */
    {0x0095, 274,  92},   /* 17: action button 1 */
    {0x0097, 275,  92},   /* 18: action button 2 */
    {0xFFFF,   0,   0},   /* 19: sentinel */

    /* Group 1: movement / navigation */
    {0x0008, 134, 140},   /* 20: movement pad */
    {0x000A,   7, 137},   /* 21: turn left */
    {0x0092,  67, 143},   /* 22: minimap */
    {0x003E,  15,   7},   /* 23: compass overlay */
    {0xFFFF,   0,   0},   /* 24: sentinel */

    /* Group 2: champion 1 sub-panel */
    {0x000B,   7, 137},   /* 25: champion 1 action */
    {0xFFFF,   0,   0},   /* 26: sentinel */

    /* Group 3: champion 2 sub-panel */
    {0x000C,   7, 137},   /* 27: champion 2 action */
    {0xFFFF,   0,   0},   /* 28: sentinel */

    /* Group 4: champion 3 sub-panel */
    {0x000D,   7, 137},   /* 29: champion 3 action */
    {0xFFFF,   0,   0},   /* 30: sentinel */

    /* Group 5: inventory grid (4 champion panels) */
    {0x0010, 142,  86},   /* 31: inv slot top-left */
    {0x0010, 200,  86},   /* 32: inv slot top-right */
    {0x0010, 142, 150},   /* 33: inv slot bottom-left */
    {0x0010, 200, 150},   /* 34: inv slot bottom-right */
    {0x0093, 176,  73},   /* 35: champion panel top-left */
    {0x0093, 234,  73},   /* 36: champion panel top-right */
    {0x0093, 176, 137},   /* 37: champion panel bottom-left */
    {0x0093, 234, 137},   /* 38: champion panel bottom-right */
    {0x0096, 143,  74},   /* 39: champion frame top-left */
    {0x0096, 201,  74},   /* 40: champion frame top-right */
    {0x0096, 143, 138},   /* 41: champion frame bottom-left */
    {0x0096, 201, 138},   /* 42: champion frame bottom-right */
    {0x008F, 142,  72},   /* 43: equip slot top-left */
    {0x008F, 200,  72},   /* 44: equip slot top-right */
    {0x008F, 142, 136},   /* 45: equip slot bottom-left */
    {0x008F, 200, 136},   /* 46: equip slot bottom-right */
    {0x0058, 260, 176},   /* 47: action button A */
    {0x004E, 292, 176},   /* 48: action button B */
    {0x005A, 276, 192},   /* 49: action button C */
    {0x0035,   8,  48},   /* 50: text area */
    {0xFFFF,   0,   0},   /* 51: sentinel */

    /* Group 6: spell casting UI */
    {0x0052, 276, 160},   /* 52: spell icon */
    {0x0059, 288, 163},   /* 53: spell button 1 */
    {0x008B, 260, 176},   /* 54: spell button 2 */
    {0x0057, 292, 176},   /* 55: spell button 3 */
    {0x005A, 276, 192},   /* 56: spell confirm */
    {0x0006, 247,  66},   /* 57: spell list */
    {0x0005, 140,  72},   /* 58: spell grid 1 */
    {0x0005, 196,  72},   /* 59: spell grid 2 */
    {0x0005, 140, 104},   /* 60: spell grid 3 */
    {0x0005, 196, 104},   /* 61: spell grid 4 */
    {0x0007, 144, 136},   /* 62: spell result */
    {0xFFFF,   0,   0},   /* 63: sentinel */

    /* Group 7: combat action panel */
    {0x0006, 247,  66},   /* 64: combat actions */
    {0x007F, 276, 160},   /* 65: combat icon */
    {0x007E, 264, 163},   /* 66: combat button 1 */
    {0x0080, 260, 176},   /* 67: combat button 2 */
    {0x0082, 292, 176},   /* 68: combat button 3 */
    {0x0083, 288, 190},   /* 69: combat button 4 */
    {0x0084, 264, 190},   /* 70: combat button 5 */
    {0x005A, 276, 192},   /* 71: combat confirm */
    {0x0081, 288, 163},   /* 72: combat button 6 */
    {0xFFFF,   0,   0},   /* 73: sentinel */
    {0xFFFF,   0,   0},   /* 74: sentinel (double) */

    /* Group 8: map/menu buttons */
    {0x005B, 276, 160},   /* 75: menu button 1 */
    {0x005C, 288, 163},   /* 76: menu button 2 */
    {0x005D, 264, 163},   /* 77: menu button 3 */
    {0x008E, 292, 176},   /* 78: menu button 4 */
    {0x008B, 260, 176},   /* 79: menu button 5 */
};

#define HUD_LAYOUT_COUNT (sizeof(g_hud_layout) / sizeof(g_hud_layout[0]))

static const uint16_t g_hp_bar_x[4] = {64, 128, 192, 256};

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

int nexus_v1_hud_layout(const Nexus_HudElement **out) {
    if (out) *out = g_hud_layout;
    return (int)HUD_LAYOUT_COUNT;
}

int nexus_v1_hud_layout_parse_dm_bin(
    const uint8_t *data,
    size_t data_size,
    Nexus_HudElement *out,
    size_t out_capacity,
    size_t *out_count)
{
    size_t i;
    size_t required = (size_t)NEXUS_HUD_LAYOUT_ENTRY_COUNT *
                      NEXUS_HUD_LAYOUT_ENTRY_BYTES;

    if (out_count) *out_count = 0U;
    if (!data || !out || out_capacity < NEXUS_HUD_LAYOUT_ENTRY_COUNT ||
        data_size < (size_t)NEXUS_HUD_LAYOUT_DM_BIN_OFFSET + required) {
        return -1;
    }

    for (i = 0U; i < NEXUS_HUD_LAYOUT_ENTRY_COUNT; ++i) {
        const uint8_t *entry = data + NEXUS_HUD_LAYOUT_DM_BIN_OFFSET +
                               i * NEXUS_HUD_LAYOUT_ENTRY_BYTES;
        /* DM.BIN yam\\menuctrl.c stores (element, 0, x, y) as BE16. */
        if (read_be16(entry + 2U) != 0U) {
            return -2;
        }
        out[i].element_id = read_be16(entry + 0U);
        out[i].x = read_be16(entry + 4U);
        out[i].y = read_be16(entry + 6U);
    }
    if (out_count) *out_count = NEXUS_HUD_LAYOUT_ENTRY_COUNT;
    return 0;
}

const uint16_t *nexus_v1_hud_hp_bar_positions(void) {
    return g_hp_bar_x;
}
