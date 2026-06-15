/*
 * csb_v1_left_click_inventory_pc34_compat.c
 *
 * Source-locked per ReDMCSB DEFS.H:327-330 + COMMAND.C
 * CHANGE7_28.  C125..C128 map to champion icon click
 * commands; the C33_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION
 * sentinel drives F0378's panel-dispatch.
 */
#include "csb_v1_left_click_inventory_pc34_compat.h"

static int g_csb_v1_left_click_inventory_enabled = 0;

int csb_v1_left_click_inventory_get(void) {
    return g_csb_v1_left_click_inventory_enabled;
}

void csb_v1_left_click_inventory_set(int enabled) {
    g_csb_v1_left_click_inventory_enabled = enabled ? 1 : 0;
}

int csb_v1_champion_icon_left_click_command(int championSlot) {
    /* ReDMCSB DEFS.H:327-330: C125..C128. */
    if (!g_csb_v1_left_click_inventory_enabled) return 0;
    if (championSlot < 0 || championSlot > 3) return 0;
    return 125 + championSlot; /* C125..C128 */
}
