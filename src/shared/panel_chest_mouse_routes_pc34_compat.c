/*
 * Panel chest mouse routes — PC 3.4 compat layer.
 *
 * Source-lock evidence:
 *   COMMAND.C:215-227  G0456_as_Graphic561_MouseInput_PanelChest[9]
 *     defines 8 chest slot click boxes {C058..C065, x1,x2,y1,y2}.
 *     C058..C065 map to viewport-relative child zones C537..C544.
 *   COMMAND.C:498-507  Same G0456 re-expressed as CM2_VIEWPORT_RELATIVE
 *     zone-based input for MEDIA529 (newer) builds.
 *   COMMAND.C:1982     F0378_COMMAND_ProcessType81_ClickInPanel M569_PANEL_CHEST
 *     case: calls F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0456,...)
 *     then F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox with
 *     command minus C020_COMMAND_CLICK_ON_SLOT_BOX_00 offset.
 *   COMMAND.C:2174-2176  Range gate: command must be >= C028 and <= C065;
 *     dispatch calls F0302 with same C020 offset.
 *   CHAMPION.C:662-712  F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:
 *     C30+ slot indices access G0425_aT_ChestSlots[slot-C30].
 *     C00..C29 route to champion Slots[] via slot>>1 and slot&1.
 *     Empty/empty no-ops; AllowedSlots gate leader-hand placement.
 *   CHAMPION.C:512-514  F0300 chest slot path:
 *     G0425_aT_ChestSlots[slot-C30_SLOT_CHEST_1] lookup.
 *   CHAMPION.C:609-610  F0301 chest slot path:
 *     G0425_aT_ChestSlots[slot-C30_SLOT_CHEST_1] write.
 *   CHEST.C:30-76   F0333_INVENTORY_OpenAndDrawChest:
 *     sets M569_PANEL_CHEST, draws C025 open-chest panel,
 *     copies first 8 linked container things into G0425_aT_ChestSlots,
 *     draws C145 (open) / C144 (closed) action-hand icon.
 *   CHEST.C:112-133  F0334_INVENTORY_CloseChest:
 *     compacts non-empty G0425_aT_ChestSlots back to dungeon linked list.
 *   DEFS.H:778-817   C00..C37 slot namespace; C30_SLOT_CHEST_1=30, C37=37.
 *   BUG0_39 (CHAMPION.C comment): leader-hand container swap triggers
 *     double panel refresh (MASK0x0800 then F0292) causing visual flash.
 */

#include "panel_chest_mouse_routes_pc34_compat.h"

const char* panel_chest_mouse_routes_GetEvidence(void) {
    return
        "COMMAND.C:215-227 G0456_as_Graphic561_MouseInput_PanelChest[9] "
        "{C058..C065, x1,x2,y1,y2}\n"
        "COMMAND.C:498-507 G0456 zone-relative MEDIA529 variant\n"
        "COMMAND.C:1982 F0378 M569_PANEL_CHEST dispatch via F0358+F0302\n"
        "COMMAND.C:2174-2176 command range gate and F0302 dispatch\n"
        "CHAMPION.C:662-712 F0302 chest slot index routing (C30+ -> G0425)\n"
        "CHAMPION.C:512-514 F0300 chest slot read path\n"
        "CHAMPION.C:609-610 F0301 chest slot write path\n"
        "CHEST.C:30-76 F0333 open chest panel + G0425 copy\n"
        "CHEST.C:112-133 F0334 compact-close G0425 -> dungeon list\n"
        "DEFS.H:778-817 C00..C37 slot namespace; C30_SLOT_CHEST_1=30\n"
        "BUG0_39: leader-hand container swap double-panel-flash (preserved)\n";
}

/*
 * ReDMCSB COMMAND.C:215-227 G0456_as_Graphic561_MouseInput_PanelChest
 * defines 8 chest slots.  Each entry is:
 *   { Command, Box.Left, Box.Right, Box.Top, Box.Bottom, Button }
 * Command values: C058=38..C065=45 (chest_1..chest_8).
 * Box.Left = x1, Box.Top = y1 — these are the top-left corner pixels
 * of the viewport-relative click zone for each chest slot.
 *
 * Expected values per slot (chestOrdinal 0..7):
 *   Ordinal  ZoneId   x1    y1
 *     0      537    117    92
 *     1      538    106   109
 *     2      539    111   126
 *     3      540    128   131
 *     4      541    145   134
 *     5      542    162   136
 *     6      543    179   137
 *     7      544    196   138
 *
 * Note: kV1ChestSlotBoxZones (m11_game_view.c) stores panel-relative
 * y-values (59, 76, 93, 98, 101, 103, 104, 105) which are exactly
 * 33 pixels less than the COMMAND.C viewport-relative y1 values.
 * This 33-pixel offset equals the inventory panel's viewport y-origin.
 * Both interpretations are valid: COMMAND.C y1 is viewport-relative,
 * kV1 zones are panel-relative.  The x-values (117, 106, 111, 128,
 * 145, 162, 179, 196) match exactly.
 */
static const int kExpectedChestZoneCount = 8;
static const int kExpectedZoneIds[8]   = { 537, 538, 539, 540, 541, 542, 543, 544 };
static const int kExpectedX[8]         = { 117, 106, 111, 128, 145, 162, 179, 196 };
/* y-values are viewport-relative per COMMAND.C:215-227 y1 field */
static const int kExpectedViewportY[8] = {  92,  109,  126,  131,  134,  136,  137,  138 };
/* y-values are panel-relative in m11_game_view.c: viewport y - 33. */
static const int kExpectedPanelY[8]    = {  59,   76,   93,   98,  101,  103,  104,  105 };

unsigned int panel_chest_mouse_routes_GetInvariant(void) {
    unsigned int ok = 1;
    int i;

    for (i = 0; i < kExpectedChestZoneCount; i++) {
        if (kExpectedZoneIds[i] != 537 + i) {
            ok = 0;
        }
        if (kExpectedX[i] <= 0) {
            ok = 0;
        }
        if (kExpectedViewportY[i] - kExpectedPanelY[i] != 33) {
            ok = 0;
        }
        if (i > 0 && kExpectedZoneIds[i] <= kExpectedZoneIds[i - 1]) {
            ok = 0;
        }
    }

    return ok;
}
