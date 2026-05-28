/*
 * Headless probe: DM1 V1 chest slot zone validation against ReDMCSB.
 *
 * Validates kV1ChestSlotBoxZones (m11_game_view.c) against
 * ReDMCSB COMMAND.C:215-227 G0456_as_Graphic561_MouseInput_PanelChest
 * zone definitions.  No game data required.
 *
 * ReDMCSB COMMAND.C:215-227 defines the chest slot mouse input array:
 *   G0456_as_Graphic561_MouseInput_PanelChest[9] = {
 *     entry fields: { Command, Box.Left, Box.Right, Box.Top, Box.Bottom, Button }
 *     { C058_COMMAND_CLICK_ON_SLOT_BOX_38_CHEST_1, 117, 132,  92, 107, ... },
 *     { C059_COMMAND_CLICK_ON_SLOT_BOX_39_CHEST_2, 106, 121, 109, 124, ... },
 *     { C060_COMMAND_CLICK_ON_SLOT_BOX_40_CHEST_3, 111, 126, 126, 141, ... },
 *     { C061_COMMAND_CLICK_ON_SLOT_BOX_41_CHEST_4, 128, 143, 131, 146, ... },
 *     { C062_COMMAND_CLICK_ON_SLOT_BOX_42_CHEST_5, 145, 160, 134, 149, ... },
 *     { C063_COMMAND_CLICK_ON_SLOT_BOX_43_CHEST_6, 162, 177, 136, 151, ... },
 *     { C064_COMMAND_CLICK_ON_SLOT_BOX_44_CHEST_7, 179, 194, 137, 152, ... },
 *     { C065_COMMAND_CLICK_ON_SLOT_BOX_45_CHEST_8, 196, 211, 138, 153, ... },
 *     { 0, 0, 0, 0, 0, 0 }
 *   };
 *
 * Each entry's Box.Left (=x1) and Box.Top (=y1) define the viewport-relative
 * top-left corner of the click zone for each chest slot (C537..C544).
 *
 * Key findings (ReDMCSB vs kV1ChestSlotBoxZones):
 *   - Zone IDs C537..C544: MATCH
 *   - x-values (117,106,111,128,145,162,179,196): MATCH exactly
 *   - y-values: kV1 = (59,76,93,98,101,103,104,105);
 *     COMMAND.C y1 = (92,109,126,131,134,136,137,138)
 *     Difference: kV1_y = COMMAND.C_y1 - 33 (panel viewport y-origin)
 *     The 33-pixel offset equals the inventory panel's viewport y-base.
 *     kV1 stores panel-relative y-values; COMMAND.C stores viewport-relative.
 *     Both are correct within their respective coordinate systems.
 *     For hit-testing, M11_GameView_GetV1ChestSlotBoxZone returns kV1 values
 *     (panel-relative), which are correct for panel-relative hit detection.
 *
 * Source citations:
 *   COMMAND.C:215-227  G0456 chest slot click box definitions
 *   COMMAND.C:498-507  G0456 zone-relative MEDIA529 variant
 *   COMMAND.C:1982     F0378 M569_PANEL_CHEST dispatch
 *   CHAMPION.C:662-712 F0302 chest slot routing via G0425_aT_ChestSlots
 *   CHEST.C:30-76      F0333 open chest panel + slot population
 */

#include <stdio.h>
#include <string.h>
#include "m11_game_view.h"

/* image_backend_pc34_compat.c references these globals (from test files) */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

static void rec(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

/* Expected zone IDs per COMMAND.C:215-227 zone indices C537..C544 */
static const int kExpId[8]    = { 537, 538, 539, 540, 541, 542, 543, 544 };
/* Expected x (viewport-relative Box.Left) per COMMAND.C */
static const int kExpVx[8]    = { 117, 106, 111, 128, 145, 162, 179, 196 };
/* COMMAND.C viewport-relative y1 values (Box.Top) */
static const int kExpVy1[8]    = {  92,  109,  126,  131,  134,  136,  137,  138 };
/* kV1ChestSlotBoxZones stores panel-relative y (= viewport_y - 33) */
static const int kExpPanelY[8] = {  59,   76,   93,   98,  101,  103,  104,  105 };
/* Slot box dimensions: 16x16 per DEFS.H slot standard */
static const int kSlotDim = 16;

int main(void) {
    int i;
    int count;

    printf("probe=firestaff_m11_chest_slot_zones\n");
    printf("sourceEvidence=%s\n",
           "COMMAND.C:215-227 G0456 chest slot click boxes; "
           "COMMAND.C:1982 F0378 dispatch; "
           "CHAMPION.C:662-712 F0302 chest slot routing; "
           "CHEST.C:30-76 F0333 open chest panel");

    count = M11_GameView_GetV1ChestSlotBoxZoneCount();
    rec("zoneCount", count == 8,
        "COMMAND.C:215-227: G0456[9] has 8 chest slots + sentinel");

    for (i = 0; i < 8; i++) {
        int zoneId, x, y, w, h;
        char msg[128];

        zoneId = M11_GameView_GetV1ChestSlotBoxZoneId(i);
        snprintf(msg, sizeof(msg), "chest[%d] zoneId=C537+%d", i, i);
        rec(msg, zoneId == kExpId[i],
            "COMMAND.C:215-227 zone index C537+i");

        if (!M11_GameView_GetV1ChestSlotBoxZone(i, &x, &y, &w, &h)) {
            snprintf(msg, sizeof(msg), "chest[%d] zoneExists", i);
            rec(msg, 0, "M11_GameView_GetV1ChestSlotBoxZone returned false");
            continue;
        }

        snprintf(msg, sizeof(msg), "chest[%d] x=Box.Left=%d", i, kExpVx[i]);
        rec(msg, x == kExpVx[i],
            "COMMAND.C:215-227 Box.Left (viewport x) matches");

        snprintf(msg, sizeof(msg), "chest[%d] panelY=viewportY-33", i);
        rec(msg, y == kExpPanelY[i],
            "kV1ChestSlotBoxZones y = COMMAND.C y1 - 33 (panel-relative)");

        snprintf(msg, sizeof(msg), "chest[%d] viewportY=Box.Top=%d", i, kExpVy1[i]);
        rec(msg, (33 + y) == kExpVy1[i],
            "viewport-relative y1 = panel_y + 33 per COMMAND.C");

        snprintf(msg, sizeof(msg), "chest[%d] w>=0", i);
        rec(msg, w > 0, "slot box width positive");

        snprintf(msg, sizeof(msg), "chest[%d] h>=0", i);
        rec(msg, h > 0, "slot box height positive");

        /* Width/height should be 16x16 per DEFS.H C033 standard slot box */
        snprintf(msg, sizeof(msg), "chest[%d] w>=kSlotDim", i);
        rec(msg, w >= kSlotDim,
            "slot box width >= 16px (DEFS.H C033 standard)");

        snprintf(msg, sizeof(msg), "chest[%d] h>=kSlotDim", i);
        rec(msg, h >= kSlotDim,
            "slot box height >= 16px (DEFS.H C033 standard)");
    }

    printf("sourceLockEvidence=%s\n",
           "COMMAND.C:215-227: G0456[9] Box.Left/Top = viewport coords for "
           "C537..C544; kV1ChestSlotBoxZones stores panel-relative y; "
           "COMMAND.C:498-507: G0456 zone-relative MEDIA529 variant; "
           "COMMAND.C:1982: F0378 M569_PANEL_CHEST dispatch via "
           "F0358_COMMAND_GetCommandFromMouseInput_CPSC then "
           "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox; "
           "CHAMPION.C:662-712 F0302: C30+ routes to "
           "G0425_aT_ChestSlots[slot-30]; "
           "CHEST.C:30-76 F0333: copies first 8 container items into G0425, "
           "draws C025 open-chest panel, uses C145 (open) / C144 (closed) "
           "action-hand icon");
    printf("result=%s\n", g_fail == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_pass, g_fail);

    return g_fail == 0 ? 0 : 1;
}
