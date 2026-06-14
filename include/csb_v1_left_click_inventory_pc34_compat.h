/*
 * csb_v1_left_click_inventory_pc34_compat.h
 *
 * CSB V1 Left-Click Inventory Access (Champions GAP 4,
 * CHANGE7_28).  Source-locked per ReDMCSB DEFS.H:327-330
 * (C125..C128 commands for each of the 4 champion icon
 * corners), COMMAND.C CHANGE7_28 (left-click on the
 * champion portrait/stat bars opens inventory), and
 * DEFS.H:226 (C33_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION).
 *
 * DM1 PC 3.4 requires right-click or menu navigation to
 * open the champion inventory.  CSB PC 3.4 added left-click
 * dispatch.  v1 implements a bounded version that maps
 * C113..C116 (champion icon click zones) to C125..C128
 * (champion icon click commands) which then dispatch
 * through the existing F0378 panel route to open
 * inventory.
 *
 * The helper returns the command number (125..128) for a
 * given champion slot, or 0 if the click is not on a
 * champion icon.  M11 calls this from the M569_PANEL_CHEST
 * dispatch path.
 */
#ifndef REDMCSB_CSB_V1_LEFT_CLICK_INVENTORY_PC34_COMPAT_H
#define REDMCSB_CSB_V1_LEFT_CLICK_INVENTORY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* CSB V1 left-click inventory mode toggle.  Default 0
 * (DM1 behaviour: right-click required).  When 1, the
 * M11 dispatch emits C125..C128 on left-click of the
 * corresponding champion icon zone. */
int  csb_v1_left_click_inventory_get(void);
void csb_v1_left_click_inventory_set(int enabled);

/* Source-locked command mapping per ReDMCSB DEFS.H:327-330.
 * Returns the C125..C128 command number for a given
 * champion slot 0..3 (0 = top-left, 1 = top-right, 2 =
 * bottom-right, 3 = bottom-left).  Returns 0 if the
 * slot is out of range or left-click mode is disabled. */
int  csb_v1_champion_icon_left_click_command(int championSlot);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_LEFT_CLICK_INVENTORY_PC34_COMPAT_H */
