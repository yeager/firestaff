#ifndef REDMCSB_CSB_TOUCH_CLICK_ZONE_MATRIX_PC34_COMPAT_H
#define REDMCSB_CSB_TOUCH_CLICK_ZONE_MATRIX_PC34_COMPAT_H

/*
 * CSB (Chaos Strikes Back) per-view click/touch zone inventory.
 *
 * Source lock: ReDMCSB WIP20210206 Toolchains/Common/Source/COMMAND.C
 * PC-media (MEDIA529/I34E-family) MOUSE_INPUT route tables G0447/G0448/
 * G0452/G0453/G0454/G0455 — the same engine source the Firestaff CSB
 * lane executes (src/engine/redmcsb_f*.c modules, src/csb/).  Each route is
 * { command, coord-mode, zone-index | literal box, button-mask }.
 *
 * Coordinate modes and button masks are shared with the DM1 matrix
 * (touch_click_zone_zone_matrix_pc34_compat.h) because both games are
 * built from the same COMMAND.C with identical CM1/CM2 semantics
 * (F0358_COMMAND_GetCommandFromMouseInput_CPSC, COMMAND.C:1379-1449).
 *
 * This inventory covers the complete CSB route-table set: the dungeon
 * game view (movement arrows, primary interface, action-area subviews,
 * spell-area subview, champion names/hands), the champion inventory,
 * the chest panel, the resurrect/reincarnate/cancel panel, the
 * entrance and restart screens, and the champion rename panel.
 */

#include "touch_click_zone_matrix_pc34_compat.h"

typedef enum CsbTouchClickViewPc34Compat {
    CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT = 1,     /* G0448 secondary */
    CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT,        /* G0447 primary   */
    CSB_TOUCH_CLICK_VIEW_ACTION_AREA_NAMES_PC34_COMPAT,/* G0452           */
    CSB_TOUCH_CLICK_VIEW_ACTION_AREA_ICONS_PC34_COMPAT,/* G0453           */
    CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT,       /* G0454           */
    CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, /* G0455       */
    CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT,   /* G0449       */
    CSB_TOUCH_CLICK_VIEW_PANEL_CHEST_PC34_COMPAT,          /* G0456       */
    CSB_TOUCH_CLICK_VIEW_PANEL_RESURRECT_PC34_COMPAT,      /* G0457       */
    CSB_TOUCH_CLICK_VIEW_ENTRANCE_PC34_COMPAT,             /* G0445       */
    CSB_TOUCH_CLICK_VIEW_RESTART_GAME_PC34_COMPAT,         /* G0446       */
    CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT /* G2045       */
} CsbTouchClickViewPc34Compat;

/* CSB-specific button masks beyond the shared left/right pair: the
 * entrance bonus-dungeon route uses MASK0x0010_MOUSE_BONUS_DUNGEON
 * (COMMAND.C G0445, DEFS.H). */
#define CSB_TOUCH_CLICK_BUTTON_BONUS_DUNGEON_PC34_COMPAT 0x0010u

typedef struct CsbTouchClickZonePc34Compat {
    unsigned int commandId;
    unsigned int zoneIndex;
    CsbTouchClickViewPc34Compat view;
    TouchClickCoordModePc34Compat coordMode;
    unsigned int buttonMask;
    int x;
    int y;
    int w;
    int h;
    const char* groupName;
    const char* sourceEvidence;
} CsbTouchClickZonePc34Compat;

unsigned int CSB_TOUCHCLICK_Compat_GetZoneCount(void);
int CSB_TOUCHCLICK_Compat_GetZone(unsigned int ordinal,
                                  CsbTouchClickZonePc34Compat* outZone);
unsigned int CSB_TOUCHCLICK_Compat_GetViewZoneCount(
    CsbTouchClickViewPc34Compat view);
int CSB_TOUCHCLICK_Compat_GetViewZone(CsbTouchClickViewPc34Compat view,
                                      unsigned int viewOrdinal,
                                      CsbTouchClickZonePc34Compat* outZone);
/* Source-ordered hit-test inside one view: first matching zone whose
 * box contains the point and whose button mask intersects wins. */
int CSB_TOUCHCLICK_Compat_HitTestInView(CsbTouchClickViewPc34Compat view,
                                        int screenX, int screenY,
                                        unsigned int buttonMask,
                                        CsbTouchClickZonePc34Compat* outZone);
const char* CSB_TOUCHCLICK_Compat_GetViewName(
    CsbTouchClickViewPc34Compat view);
const char* CSB_TOUCHCLICK_Compat_GetSourceEvidence(void);

#endif
