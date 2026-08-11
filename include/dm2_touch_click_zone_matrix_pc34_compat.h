#ifndef REDMCSB_DM2_TOUCH_CLICK_ZONE_MATRIX_PC34_COMPAT_H
#define REDMCSB_DM2_TOUCH_CLICK_ZONE_MATRIX_PC34_COMPAT_H

/*
 * DM2 (Dungeon Master II) per-view click/touch zone inventory.
 *
 * Source lock: SKWIN (Paul Stevens) DM2 data segment skval1.h
 * MOUSE_INPUT route tables — sk0d9e command list _4976_0d9e
 * (skval1.h:92, 257 entries), sk1891 node tree _4976_1574
 * (skval1.h:110, 74 nodes), packed child list _4976_169c
 * (skval1.h:109), sk16ed rect descriptors _4976_16ed
 * (skval1.h:114, 60 entries) and the 10 view roots
 * _4976_1891 (skval1.h:113) — the same tables SKULLWIN's
 * handwritten DM2_1031_030a/DM2_1031_027e/DM2_1031_0a88
 * (c_1031.cpp) consume, and the same engine the Firestaff
 * DM2 lane targets.  The startup relocation pass
 * _1031_07d6 (SkWinCore.cpp:55037-55120) rewrites the packed
 * w2/w4 list ordinals to real offsets; this inventory is
 * decoded in relocated form, exactly as the running game
 * hit-tests it (IBMIO_USER_INPUT_CHECK, SkWinCore.cpp:15239,
 * dispatch _1031_030a SkWinCore.cpp:12191-12242 ->
 * _1031_0a88 SkWinCore.cpp:12131-12190, first matching
 * non-disabled command whose button mask intersects and
 * whose rect contains the point wins).
 *
 * Rectangle space: GDAT rect pool (GRAPHICS.DAT DM2 PC
 * English, md5 25247ede4dabb6a71e5dabdfbcd5907d, raw 201
 * T1/I0/D4/S0, magic 0xFC0D, 30 groups, 2183 rects),
 * resolved through QUERY_BLIT_RECT anchor semantics
 * (SkWinCore.cpp:34078-34619) and _1031_01d5 origin
 * translation (SkWinCore.cpp:12095-12130; 0x8000 ->
 * topleft(7)=(0,40), 0x4000 -> topleft(18)=(48,32)).  All
 * zones are stored screen-relative in the 320x200 source
 * screen with origins baked in — the DM2 hit-test performs
 * no further coordinate translation (unlike CSB's CM2).
 *
 * The inventory covers the complete 10-view route set: main
 * menu, credits, sleep, pause, dialog, the dungeon game
 * view (movement arrows, viewport, champion hands, spell
 * runes, party positions, magic map, moneybox, stats
 * bars), the champion ribbon, the champion inventory, the
 * full inventory view and the savegame slot list.
 */

#include "touch_click_zone_matrix_pc34_compat.h"

typedef enum Dm2TouchClickViewPc34Compat {
    DM2_TOUCH_CLICK_VIEW_MAIN_MENU_PC34_COMPAT = 1, /* view 0: main_menu */
    DM2_TOUCH_CLICK_VIEW_CREDITS_PC34_COMPAT, /* view 1: credits */
    DM2_TOUCH_CLICK_VIEW_SLEEP_PC34_COMPAT, /* view 2: sleep */
    DM2_TOUCH_CLICK_VIEW_PAUSE_PC34_COMPAT, /* view 3: pause */
    DM2_TOUCH_CLICK_VIEW_DIALOG_PC34_COMPAT, /* view 4: dialog */
    DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, /* view 5: dungeon */
    DM2_TOUCH_CLICK_VIEW_CHAMPION_RIBBON_PC34_COMPAT, /* view 6: champion_ribbon */
    DM2_TOUCH_CLICK_VIEW_CHAMPION_PC34_COMPAT, /* view 7: champion */
    DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT, /* view 8: inventory */
    DM2_TOUCH_CLICK_VIEW_SAVEGAME_SLOTS_PC34_COMPAT /* view 9: savegame_slots */
} Dm2TouchClickViewPc34Compat;

typedef struct Dm2TouchClickZonePc34Compat {
    unsigned int commandId;
    unsigned int zoneIndex; /* sk0d9e command-list ordinal */
    Dm2TouchClickViewPc34Compat view;
    TouchClickCoordModePc34Compat coordMode;
    unsigned int buttonMask;
    int x;
    int y;
    int w;
    int h;
    const char* groupName;
    const char* sourceEvidence;
} Dm2TouchClickZonePc34Compat;

unsigned int DM2_TOUCHCLICK_Compat_GetZoneCount(void);
int DM2_TOUCHCLICK_Compat_GetZone(unsigned int ordinal,
                                  Dm2TouchClickZonePc34Compat* outZone);
/* Look up the source route ordinal used by _4976_0d9e.  This returns source
 * context metadata only; its PC geometry remains reference data and must not
 * be used by the FM Towns pointer owner. */
int DM2_TOUCHCLICK_Compat_GetZoneByIndex(
    unsigned int zoneIndex, Dm2TouchClickZonePc34Compat* outZone);
int DM2_TOUCHCLICK_Compat_GetZoneBySourceRecord(
    unsigned int eventIndex, unsigned int rectId, unsigned int buttonMask,
    unsigned int occurrence,
    Dm2TouchClickZonePc34Compat* outZone);
unsigned int DM2_TOUCHCLICK_Compat_GetSourceRecordContextCount(
    unsigned int eventIndex, unsigned int rectId, unsigned int buttonMask);
int DM2_TOUCHCLICK_Compat_GetSourceRecordContextAt(
    unsigned int eventIndex, unsigned int rectId, unsigned int buttonMask,
    unsigned int occurrence, Dm2TouchClickZonePc34Compat* outZone);
unsigned int DM2_TOUCHCLICK_Compat_GetViewZoneCount(
    Dm2TouchClickViewPc34Compat view);
int DM2_TOUCHCLICK_Compat_GetViewZone(Dm2TouchClickViewPc34Compat view,
                                      unsigned int viewOrdinal,
                                      Dm2TouchClickZonePc34Compat* outZone);
/* Source-ordered hit-test inside one view: first matching zone whose
 * box contains the point and whose button mask intersects wins. */
int DM2_TOUCHCLICK_Compat_HitTestInView(Dm2TouchClickViewPc34Compat view,
                                        int screenX, int screenY,
                                        unsigned int buttonMask,
                                        Dm2TouchClickZonePc34Compat* outZone);
const char* DM2_TOUCHCLICK_Compat_GetViewName(
    Dm2TouchClickViewPc34Compat view);
const char* DM2_TOUCHCLICK_Compat_GetSourceEvidence(void);

#endif
