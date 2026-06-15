/*
 * CTest gate: DM1 V1 Champion Panel & Inventory HUD source-lock.
 *
 * Validates that all source-locked constants, bar-graph computations,
 * slot box graphics, portrait positions, bar positions, inventory grid,
 * hand slot positions, name colors, and dead-status predicates match
 * the ReDMCSB source.
 */

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    int failures;

    printf("== DM1 V1 Champion Panel HUD source-lock self-test ==\n");
    failures = DM1_ChampionPanel_SelfTest();

    /* Additional structural assertions */

    /* Constants cross-check */
    if (DM1_BAR_GRAPH_WIDTH != 4) {
        fprintf(stderr, "FAIL: BAR_GRAPH_WIDTH != 4\n");
        failures++;
    }
    if (DM1_BAR_GRAPH_MAX_HEIGHT != 25) {
        fprintf(stderr, "FAIL: BAR_GRAPH_MAX_HEIGHT != 25\n");
        failures++;
    }
    if (DM1_STATUS_BOX_SPACING != 69) {
        fprintf(stderr, "FAIL: STATUS_BOX_SPACING != 69\n");
        failures++;
    }
    if (DM1_PORTRAIT_WIDTH != 32 || DM1_PORTRAIT_HEIGHT != 29) {
        fprintf(stderr, "FAIL: PORTRAIT_WIDTH/HEIGHT\n");
        failures++;
    }
    if (DM1_CHAMPION_ICON_WIDTH != 19 || DM1_CHAMPION_ICON_HEIGHT != 14) {
        fprintf(stderr, "FAIL: CHAMPION_ICON_WIDTH/HEIGHT\n");
        failures++;
    }
    if (DM1_SLOT_BOX_SIZE != 18) {
        fprintf(stderr, "FAIL: SLOT_BOX_SIZE != 18\n");
        failures++;
    }
    if (DM1_VIEWPORT_X != 0 || DM1_VIEWPORT_Y != 33 ||
        DM1_VIEWPORT_W != 224 || DM1_VIEWPORT_H != 136) {
        fprintf(stderr, "FAIL: VIEWPORT geometry\n");
        failures++;
    }

    /* Attribute flags cross-check */
    if (DM1_ATTR_STATUS_BOX != 0x1000) {
        fprintf(stderr, "FAIL: ATTR_STATUS_BOX\n");
        failures++;
    }
    if (DM1_ATTR_ACTION_HAND != 0x8000) {
        fprintf(stderr, "FAIL: ATTR_ACTION_HAND\n");
        failures++;
    }

    /* Graphic IDs */
    if (DM1_GFX_SLOT_NORMAL != 33 || DM1_GFX_SLOT_WOUNDED != 34 ||
        DM1_GFX_SLOT_ACTING != 35) {
        fprintf(stderr, "FAIL: GFX_SLOT IDs\n");
        failures++;
    }
    if (DM1_GFX_PORTRAITS != 26 || DM1_GFX_CHAMPION_ICONS != 28) {
        fprintf(stderr, "FAIL: GFX_PORTRAIT/ICON IDs\n");
        failures++;
    }

    /*
     * ReDMCSB: CHAMDRAW.C F0622 line ~41:
     *   M100/M101 set champion icon dimensions to G2080_C19/G2081_C14.
     * ReDMCSB: CHAMDRAW.C F0622 lines ~59-65:
     *   PC34 fills the bitmap with C01 while Event71Count_Invisibility is
     *   nonzero, otherwise the champion color; blits C028 from
     *   M026_CHAMPION_ICON_INDEX(Direction,PartyDirection) * 19 with C12
     *   transparency; then applies G2362 invisibility palette changes.
     */
    {
        DM1_ChampionPanel_IconBitmapModel icon;
        if (!DM1_ChampionPanel_BuildIconBitmapModel(2, 1, 3, 0, &icon) ||
            icon.width != DM1_CHAMPION_ICON_WIDTH ||
            icon.height != DM1_CHAMPION_ICON_HEIGHT ||
            icon.fillColor != DM1_COLOR_RED ||
            icon.graphicId != DM1_GFX_CHAMPION_ICONS ||
            icon.sourceX != 38 ||
            icon.sourceY != 0 ||
            icon.transparentColor != DM1_COLOR_DARKEST_GRAY ||
            icon.applyInvisibilityPalette != 0) {
            fprintf(stderr,
                    "FAIL: F0622 visible champion icon model fill=%d gfx=%d src=(%d,%d) transparent=%d palette=%d size=%dx%d\n",
                    icon.fillColor,
                    icon.graphicId,
                    icon.sourceX,
                    icon.sourceY,
                    icon.transparentColor,
                    icon.applyInvisibilityPalette,
                    icon.width,
                    icon.height);
            failures++;
        }
        if (!DM1_ChampionPanel_BuildIconBitmapModel(0, 0, 1, 7, &icon) ||
            icon.fillColor != DM1_COLOR_DARK_GRAY ||
            icon.sourceX != 57 ||
            icon.applyInvisibilityPalette != 1) {
            fprintf(stderr,
                    "FAIL: F0622 invisible champion icon model fill=%d srcX=%d palette=%d\n",
                    icon.fillColor,
                    icon.sourceX,
                    icon.applyInvisibilityPalette);
            failures++;
        }
        if (DM1_ChampionPanel_BuildIconBitmapModel(4, 0, 0, 0, &icon) ||
            DM1_ChampionPanel_BuildIconBitmapModel(0, 4, 0, 0, &icon) ||
            DM1_ChampionPanel_BuildIconBitmapModel(0, 0, -1, 0, &icon)) {
            fprintf(stderr, "FAIL: F0622 champion icon model accepts invalid input\n");
            failures++;
        }
    }

    /*
     * CHAMDRAW.C:F0292 champion-icon redraw source lock:
     * - 1019: icon box index is M026_CHAMPION_ICON_INDEX(Cell, partyDir).
     * - 1020: redraw is suppressed when the mouse pointer uses ordinal
     *   M000_INDEX_TO_ORDINAL(iconBoxIndex).
     * - 1022/1046-1051: redraw fills/blits box zone
     *   iconBoxIndex + C113_ZONE_CHAMPION_ICON_TOP_LEFT.
     * - 1025: sprite X is M026_CHAMPION_ICON_INDEX(Direction, partyDir) * 19.
     * DEFS.H anchors:
     * - 718: M026_CHAMPION_ICON_INDEX(value, direction).
     * - 2188: C028_GRAPHIC_CHAMPION_ICONS.
     * - 3779: C113_ZONE_CHAMPION_ICON_TOP_LEFT.
     * COMPILE.H:1038 anchors M000_INDEX_TO_ORDINAL(value) as value + 1.
     */
    {
        enum {
            DM1_ZONE_CHAMPION_ICON_TOP_LEFT = 113
        };
        int championIndex = 2;
        int championCell = 2;
        int championDirection = 1;
        int partyDirection = 3;
        int iconBoxIndex = (championCell + 4 - partyDirection) & 0x0003;
        int iconSpriteIndex = (championDirection + 4 - partyDirection) & 0x0003;
        int iconBoxOrdinal = iconBoxIndex + 1;
        int iconZone = iconBoxIndex + DM1_ZONE_CHAMPION_ICON_TOP_LEFT;
        int iconSpriteX = iconSpriteIndex * DM1_CHAMPION_ICON_WIDTH;
        int arrowPointerOrdinal = 0;
        int sameIconPointerOrdinal = iconBoxOrdinal;
        int redrawWhenPointerIsArrow = (arrowPointerOrdinal != iconBoxOrdinal);
        int redrawWhenPointerIsSameIcon = (sameIconPointerOrdinal != iconBoxOrdinal);

        if (iconBoxIndex != 3 ||
            iconBoxOrdinal != 4 ||
            iconZone != 116 ||
            iconSpriteIndex != 2 ||
            iconSpriteX != 38 ||
            DM1_ChampionColor[championIndex] != DM1_COLOR_RED ||
            redrawWhenPointerIsArrow != 1 ||
            redrawWhenPointerIsSameIcon != 0) {
            fprintf(stderr,
                    "FAIL: F0292 champion icon route box=%d ordinal=%d zone=%d sprite=%d spriteX=%d color=%d arrowRedraw=%d sameRedraw=%d\n",
                    iconBoxIndex,
                    iconBoxOrdinal,
                    iconZone,
                    iconSpriteIndex,
                    iconSpriteX,
                    DM1_ChampionColor[championIndex],
                    redrawWhenPointerIsArrow,
                    redrawWhenPointerIsSameIcon);
            failures++;
        }
    }

    /*
     * CHAMDRAW.C:F0291 source lock:
     * - 632-646: occupied body slots 0..5 choose wounded/normal borders.
     * - 648-651: the acting champion override is gated by
     *   P0614_ui_SlotIndex == C01_SLOT_ACTION_HAND, so it must not recolor
     *   the ready hand or other body slots.
     */
    if (DM1_ChampionPanel_SlotBoxGraphic(DM1_SLOT_READY_HAND, 0x0000, 1) !=
        DM1_GFX_SLOT_NORMAL) {
        fprintf(stderr, "FAIL: F0291 acting champion recolored ready hand\n");
        failures++;
    }
    if (DM1_ChampionPanel_SlotBoxGraphic(DM1_SLOT_HEAD,
            (uint16_t)(1u << DM1_SLOT_HEAD), 1) != DM1_GFX_SLOT_WOUNDED) {
        fprintf(stderr, "FAIL: F0291 acting champion overrode head wound\n");
        failures++;
    }

    /*
     * CHAMDRAW.C:F0292 inventory mouth/eye warning-border source lock:
     * - 908-918: food < 0, water < 0, or PoisonEventCount selects
     *   C034_GRAPHIC_SLOT_BOX_WOUNDED for C545_ZONE_MOUTH; otherwise C033.
     * - 920-932: any current statistic below maximum selects C034 for
     *   C546_ZONE_EYE; otherwise C033.
     * DEFS.H anchors:
     * - 2193-2195: C033/C034/C035 slot-box graphics.
     * - 3914-3915: C545_ZONE_MOUTH and C546_ZONE_EYE.
     *
     * This is a Firestaff-side structural gate for the inventory panel
     * warning route only. It does not claim original DOS pixel parity.
     */
    {
        enum {
            DM1_ZONE_MOUTH = 545,
            DM1_ZONE_EYE = 546
        };
        static const struct {
            int food;
            int water;
            int poisonEventCount;
            int expectedGraphic;
            const char *label;
        } mouthCases[] = {
            {  0,  0, 0, DM1_GFX_SLOT_NORMAL,  "normal" },
            { -1,  0, 0, DM1_GFX_SLOT_WOUNDED, "food" },
            {  0, -1, 0, DM1_GFX_SLOT_WOUNDED, "water" },
            {  0,  0, 1, DM1_GFX_SLOT_WOUNDED, "poison" },
        };
        static const struct {
            int current[DM1_STATISTIC_ROW_COUNT];
            int maximum[DM1_STATISTIC_ROW_COUNT];
            int expectedGraphic;
            const char *label;
        } eyeCases[] = {
            { { 50, 50, 50, 50, 50, 50 },
              { 50, 50, 50, 50, 50, 50 },
              DM1_GFX_SLOT_NORMAL, "all_equal" },
            { { 50, 50, 49, 50, 50, 50 },
              { 50, 50, 50, 50, 50, 50 },
              DM1_GFX_SLOT_WOUNDED, "stat_below_max" },
            { { 51, 51, 51, 51, 51, 51 },
              { 50, 50, 50, 50, 50, 50 },
              DM1_GFX_SLOT_NORMAL, "all_above_max" },
        };
        size_t i;

        if (DM1_ZONE_MOUTH != 545 || DM1_ZONE_EYE != 546) {
            fprintf(stderr, "FAIL: F0292 mouth/eye zone IDs\n");
            failures++;
        }

        for (i = 0; i < sizeof(mouthCases) / sizeof(mouthCases[0]); ++i) {
            int selectedGraphic =
                (mouthCases[i].food < 0 ||
                 mouthCases[i].water < 0 ||
                 mouthCases[i].poisonEventCount)
                    ? DM1_GFX_SLOT_WOUNDED
                    : DM1_GFX_SLOT_NORMAL;
            if (selectedGraphic != mouthCases[i].expectedGraphic) {
                fprintf(stderr,
                        "FAIL: F0292 mouth warning border %s got %d want %d\n",
                        mouthCases[i].label,
                        selectedGraphic,
                        mouthCases[i].expectedGraphic);
                failures++;
            }
        }

        for (i = 0; i < sizeof(eyeCases) / sizeof(eyeCases[0]); ++i) {
            int selectedGraphic = DM1_GFX_SLOT_NORMAL;
            int statIndex;
            for (statIndex = 0; statIndex < DM1_STATISTIC_ROW_COUNT; ++statIndex) {
                if (eyeCases[i].current[statIndex] < eyeCases[i].maximum[statIndex]) {
                    selectedGraphic = DM1_GFX_SLOT_WOUNDED;
                    break;
                }
            }
            if (selectedGraphic != eyeCases[i].expectedGraphic) {
                fprintf(stderr,
                        "FAIL: F0292 eye warning border %s got %d want %d\n",
                        eyeCases[i].label,
                        selectedGraphic,
                        eyeCases[i].expectedGraphic);
                failures++;
            }
        }
    }

    if (DM1_COLOR_LIGHT_GREEN != 7 || DM1_COLOR_RED != 8 ||
        DM1_COLOR_LIGHTEST_GRAY != 13) {
        fprintf(stderr, "FAIL: statistic color constants\n");
        failures++;
    }

    if (DM1_STATUS_BOX_DRAW_ALIVE != 1 || DM1_STATUS_BOX_DRAW_DEAD != 2) {
        fprintf(stderr, "FAIL: status-box draw kind constants\n");
        failures++;
    }

    /* All four champions' bar graph positions should be at y=2 */
    {
        int ci;
        for (ci = 0; ci < DM1_CHAMPION_COUNT; ci++) {
            int bx, by;
            DM1_ChampionPanel_BarGraphScreenXY(ci, 0, &bx, &by);
            if (by != 2) {
                fprintf(stderr, "FAIL: champion %d bar Y != 2 (got %d)\n", ci, by);
                failures++;
            }
            /* HP bar X should be champIdx * 69 + 46 */
            if (bx != ci * 69 + 46) {
                fprintf(stderr, "FAIL: champion %d HP bar X != %d (got %d)\n",
                        ci, ci * 69 + 46, bx);
                failures++;
            }
        }
    }

    /* Inventory slot 9 = action hand at (24, 10) */
    {
        int sx, sy;
        if (!DM1_ChampionPanel_InventorySlotXY(9, &sx, &sy) ||
            sx != 24 || sy != 10) {
            fprintf(stderr, "FAIL: inv slot 9 (ActionHand) XY\n");
            failures++;
        }
    }

    /*
     * CHAMDRAW.C:F0292 source lock:
     * - 750: status box origin is championIndex * C69.
     * - 1080-1088: ACTION_HAND redraw calls F0291 with
     *   C01_SLOT_ACTION_HAND before drawing the action icon.
     * CHAMDRAW.C:F0291 source lock:
     * - 536-548: non-inventory champion hand slot box index is
     *   (championIndex << 1) + slotIndex.
     */
    {
        int readyX, readyY;
        int actionX, actionY;
        DM1_ChampionPanel_StatusHandSlotXY(3, DM1_SLOT_READY_HAND,
                                           &readyX, &readyY);
        DM1_ChampionPanel_StatusHandSlotXY(3, DM1_SLOT_ACTION_HAND,
                                           &actionX, &actionY);
        if (readyX != 211 || readyY != 10 ||
            actionX != 231 || actionY != 10 ||
            actionX - readyX != 20) {
            fprintf(stderr,
                    "FAIL: F0292/F0291 status hand route ready=(%d,%d) action=(%d,%d)\n",
                    readyX, readyY, actionX, actionY);
            failures++;
        }
    }

    /*
     * CHAMDRAW.C:F0291 non-inventory hand-box source lock:
     * - 536-549: status-box hands use slotBoxIndex =
     *   (championIndex << 1) + slotIndex, while the inventory champion uses
     *   C08_SLOT_BOX_INVENTORY_FIRST_SLOT + slotIndex.
     * - 542-545: non-inventory slots return early when slotIndex > C01 or
     *   when G0299_ui_CandidateChampionOrdinal owns that champion ordinal.
     * DEFS.H anchors:
     * - 780-781: C00/C01 ready/action hand slot IDs.
     * - 1874: C08_SLOT_BOX_INVENTORY_FIRST_SLOT.
     * - 3800-3807: C211..C218 status-box hand zones.
     *
     * This keeps the C040 candidate panel from repainting its champion's
     * status hands while still allowing other champions' hand boxes through.
     */
    {
        enum {
            DM1_ZONE_STATUS_HAND_FIRST = 211
        };
        int championIndex = 2;
        int championOrdinal = championIndex + 1;
        int matchingCandidateOrdinal = championOrdinal;
        int otherCandidateOrdinal = 1;
        int noCandidateOrdinal = 0;
        int readySlot = DM1_SLOT_READY_HAND;
        int actionSlot = DM1_SLOT_ACTION_HAND;
        int headSlot = DM1_SLOT_HEAD;
        int readySlotBox = (championIndex << 1) + DM1_SLOT_READY_HAND;
        int actionSlotBox = (championIndex << 1) + DM1_SLOT_ACTION_HAND;
        int readyZone = DM1_ZONE_STATUS_HAND_FIRST + readySlotBox;
        int actionZone = DM1_ZONE_STATUS_HAND_FIRST + actionSlotBox;
        int inventoryReadySlotBox =
            DM1_SLOTBOX_FIRST_INVENTORY + DM1_SLOT_READY_HAND;
        int drawReadyWhenCandidateMatches =
            !((readySlot > DM1_SLOT_ACTION_HAND) ||
              (matchingCandidateOrdinal == championOrdinal));
        int drawActionWhenOtherCandidate =
            !((actionSlot > DM1_SLOT_ACTION_HAND) ||
              (otherCandidateOrdinal == championOrdinal));
        int drawHeadWhenNoCandidate =
            !((headSlot > DM1_SLOT_ACTION_HAND) ||
              (noCandidateOrdinal == championOrdinal));

        if (readySlotBox != 4 ||
            actionSlotBox != 5 ||
            readyZone != 215 ||
            actionZone != 216 ||
            inventoryReadySlotBox != 8 ||
            drawReadyWhenCandidateMatches != 0 ||
            drawActionWhenOtherCandidate != 1 ||
            drawHeadWhenNoCandidate != 0) {
            fprintf(stderr,
                    "FAIL: F0291 candidate/status hand route readyBox=%d actionBox=%d readyZone=%d actionZone=%d invReady=%d drawMatch=%d drawOther=%d drawHead=%d\n",
                    readySlotBox,
                    actionSlotBox,
                    readyZone,
                    actionZone,
                    inventoryReadySlotBox,
                    drawReadyWhenCandidateMatches,
                    drawActionWhenOtherCandidate,
                    drawHeadWhenNoCandidate);
            failures++;
        }
    }

    /*
     * CHAMDRAW.C:F0292 source lock:
     * - 750: L0868_i_ChampionStatusBoxX = championIndex * C69.
     * - 880-884: non-inventory NAME_TITLE clears only the name strip,
     *   left = statusBoxX, right = statusBoxX + 42, top = 0, bottom = 6,
     *   then prints at x = statusBoxX + 1, y = 5.
     * This fourth-slot edge gate keeps the live name strip from drifting
     * into the slot-3 bars/hands or past the 320-wide screen edge.
     */
    {
        int slot3NameLeft = DM1_ChampionPanel_NameZoneX(3);
        int slot3NameRight = slot3NameLeft + 42;
        int slot3NamePrintX = slot3NameLeft + 1;
        if (slot3NameLeft != 207 ||
            slot3NameRight != 249 ||
            slot3NamePrintX != 208 ||
            slot3NameRight - slot3NameLeft + 1 != 43) {
            fprintf(stderr,
                    "FAIL: F0292 slot3 NAME_TITLE strip left=%d right=%d printX=%d\n",
                    slot3NameLeft, slot3NameRight, slot3NamePrintX);
            failures++;
        }
    }

    /* CHAMDRAW.C F0289/F0290 inventory champion numeric values */
    if (DM1_ZONE_HEALTH_VALUE != 550 ||
        DM1_ZONE_MANA_VALUE != 551 ||
        DM1_ZONE_STAMINA_VALUE != 552) {
        fprintf(stderr, "FAIL: HP/stamina/mana value zone IDs\n");
        failures++;
    }
    if (DM1_ChampionPanel_StatusValueZone(DM1_STATUS_VALUE_HEALTH) !=
        DM1_ZONE_HEALTH_VALUE ||
        DM1_ChampionPanel_StatusValueZone(DM1_STATUS_VALUE_STAMINA) !=
        DM1_ZONE_MANA_VALUE ||
        DM1_ChampionPanel_StatusValueZone(DM1_STATUS_VALUE_MANA) !=
        DM1_ZONE_STAMINA_VALUE) {
        fprintf(stderr, "FAIL: F0290 value-zone routing\n");
        failures++;
    }
    {
        char value[8];
        if (!DM1_ChampionPanel_FormatStatusValue(
                DM1_STATUS_VALUE_HEALTH, 77, 100, 666, 999, 12, 33,
                value, sizeof(value)) || strcmp(value, " 77/100") != 0) {
            fprintf(stderr, "FAIL: F0289 health value format got %s\n", value);
            failures++;
        }
        if (!DM1_ChampionPanel_FormatStatusValue(
                DM1_STATUS_VALUE_STAMINA, 77, 100, 666, 999, 12, 33,
                value, sizeof(value)) || strcmp(value, " 66/ 99") != 0) {
            fprintf(stderr, "FAIL: F0290 stamina /10 format got %s\n", value);
            failures++;
        }
        if (!DM1_ChampionPanel_FormatStatusValue(
                DM1_STATUS_VALUE_MANA, 77, 100, 666, 999, 12, 33,
                value, sizeof(value)) || strcmp(value, " 12/ 33") != 0) {
            fprintf(stderr, "FAIL: F0290 mana value format got %s\n", value);
            failures++;
        }
    }

    /*
     * PANEL.C:F0351 source lock:
     * - 2081-2091: compare current statistic row against maximum row;
     *   below max red, above max light green, equal lightest gray.
     * - 2093-2105: current value and /maximum suffix are printed
     *   separately; only the current value uses the computed color.
     */
    if (DM1_ChampionPanel_StatisticCurrentColor(49, 50) != DM1_COLOR_RED) {
        fprintf(stderr, "FAIL: F0351 stat below max current color\n");
        failures++;
    }
    if (DM1_ChampionPanel_StatisticCurrentColor(50, 50) != DM1_COLOR_LIGHTEST_GRAY) {
        fprintf(stderr, "FAIL: F0351 stat equal max current color\n");
        failures++;
    }
    if (DM1_ChampionPanel_StatisticCurrentColor(51, 50) != DM1_COLOR_LIGHT_GREEN) {
        fprintf(stderr, "FAIL: F0351 stat above max current color\n");
        failures++;
    }
    if (DM1_ChampionPanel_StatisticMaximumColor() != DM1_COLOR_LIGHTEST_GRAY) {
        fprintf(stderr, "FAIL: F0351 max suffix color\n");
        failures++;
    }
    {
        char currentValue[4];
        char maximumValue[5];
        if (!DM1_ChampionPanel_FormatStatisticValue(49, 50,
                currentValue, sizeof(currentValue),
                maximumValue, sizeof(maximumValue)) ||
            strcmp(currentValue, " 49") != 0 ||
            strcmp(maximumValue, "/ 50") != 0) {
            fprintf(stderr, "FAIL: F0351 stat value format got %s %s\n",
                    currentValue, maximumValue);
            failures++;
        }
    }
    {
        DM1_ChampionPanel_StatisticTextRunModel run;
        if (!DM1_ChampionPanel_BuildStatisticTextRunModel(1, 51, 50, &run) ||
            run.nameZone != DM1_ZONE_SKILL_VALUE ||
            run.valueZone != DM1_ZONE_STATISTIC_VALUE ||
            run.nameX != DM1_STATISTIC_NAME_REL_X ||
            run.currentX != DM1_STATISTIC_CURRENT_REL_X ||
            run.maximumX != DM1_STATISTIC_CURRENT_REL_X + DM1_PANEL_TEXT_CHAR_WIDTH * 3 ||
            run.y != DM1_STATISTIC_FIRST_REL_Y + DM1_PANEL_TEXT_LINE_HEIGHT ||
            run.nameColor != DM1_COLOR_LIGHTEST_GRAY ||
            run.currentColor != DM1_COLOR_LIGHT_GREEN ||
            run.maximumColor != DM1_COLOR_LIGHTEST_GRAY ||
            strcmp(run.currentText, " 51") != 0 ||
            strcmp(run.maximumText, "/ 50") != 0) {
            fprintf(stderr, "FAIL: F0351 stat text run layout/color\n");
            failures++;
        }
    }

    /*
     * CHAMDRAW.C:F0292 source lock:
     * - 958-967: load label/value color is red only above max load; yellow
     *   starts at strict (load << 3) > maxLoad * 5.
     * - 986-1017: value format is current deci-kg as %3d.%d, /, rounded
     *   max kg ((maxLoad + 5) / 10), then " KG", printed in zone C555.
     * - 349-388: F0288 pads requested integer fields with spaces.
     */
    if (DM1_ZONE_CHAMPION_LOAD_LABEL != 554 ||
        DM1_ZONE_CHAMPION_LOAD_VALUE != 555 ||
        DM1_ChampionPanel_LoadValueZone() != DM1_ZONE_CHAMPION_LOAD_VALUE) {
        fprintf(stderr, "FAIL: load label/value zone IDs\n");
        failures++;
    }
    if (DM1_ChampionPanel_LoadColor(187, 300) != DM1_COLOR_LIGHTEST_GRAY) {
        fprintf(stderr, "FAIL: load color below strict 5/8 threshold\n");
        failures++;
    }
    if (DM1_ChampionPanel_LoadColor(188, 300) != DM1_COLOR_YELLOW) {
        fprintf(stderr, "FAIL: load color above strict 5/8 threshold\n");
        failures++;
    }
    if (DM1_ChampionPanel_LoadColor(300, 300) != DM1_COLOR_YELLOW) {
        fprintf(stderr, "FAIL: load color at exact maximum should be yellow\n");
        failures++;
    }
    if (DM1_ChampionPanel_LoadColor(301, 300) != DM1_COLOR_RED) {
        fprintf(stderr, "FAIL: load color above maximum should be red\n");
        failures++;
    }
    {
        char loadValue[16];
        if (!DM1_ChampionPanel_FormatLoadValue(0, 300, loadValue, sizeof(loadValue)) ||
            strcmp(loadValue, "  0.0/ 30 KG") != 0) {
            fprintf(stderr, "FAIL: load zero value format got %s\n", loadValue);
            failures++;
        }
        if (!DM1_ChampionPanel_FormatLoadValue(251, 300, loadValue, sizeof(loadValue)) ||
            strcmp(loadValue, " 25.1/ 30 KG") != 0) {
            fprintf(stderr, "FAIL: load value format got %s\n", loadValue);
            failures++;
        }
        if (!DM1_ChampionPanel_FormatLoadValue(999, 254, loadValue, sizeof(loadValue)) ||
            strcmp(loadValue, " 99.9/ 25 KG") != 0) {
            fprintf(stderr, "FAIL: load rounded max format got %s\n", loadValue);
            failures++;
        }
    }

    /*
     * CHAMDRAW.C:F0292 source lock:
     * - 771-789: STATUS_BOX fills live champion status box with C12.
     * - 810-815: inventory champion draws portrait and schedules only
     *   STATISTICS; other live champions schedule NAME_TITLE, STATISTICS,
     *   WOUNDS, and ACTION_HAND in the local redraw mask.
     * - 816-838: dead champion draws C008, prints name in C13/C01,
     *   draws action icon, then jumps to the end of F0292.
     */
    {
        DM1_ChampionPanel_StatusBoxModel model;
        if (!DM1_ChampionPanel_BuildStatusBoxModel(0, 0, 1, 100, &model) ||
            model.drawKind != DM1_STATUS_BOX_DRAW_ALIVE ||
            model.fillColor != DM1_COLOR_DARKEST_GRAY ||
            model.drawPortrait != 1 ||
            model.propagatedAttributes != DM1_ATTR_STATISTICS) {
            fprintf(stderr, "FAIL: F0292 inventory champion status-box propagation\n");
            failures++;
        }
        if (!DM1_ChampionPanel_BuildStatusBoxModel(3, 0, 0, 100, &model) ||
            model.drawKind != DM1_STATUS_BOX_DRAW_ALIVE ||
            model.drawPortrait != 0 ||
            model.propagatedAttributes !=
                (DM1_ATTR_NAME_TITLE | DM1_ATTR_STATISTICS |
                 DM1_ATTR_WOUNDS | DM1_ATTR_ACTION_HAND)) {
            fprintf(stderr, "FAIL: F0292 non-inventory champion status-box propagation\n");
            failures++;
        }
        if (!DM1_ChampionPanel_BuildStatusBoxModel(1, 0, 0, 0, &model) ||
            model.drawKind != DM1_STATUS_BOX_DRAW_DEAD ||
            model.graphicId != DM1_GFX_DEAD_CHAMPION ||
            model.nameColor != DM1_COLOR_LIGHTEST_GRAY ||
            model.nameBackgroundColor != DM1_COLOR_DARK_GRAY ||
            model.propagatedAttributes != 0 ||
            model.drawActionIcon != 1 ||
            model.stopAfterDead != 1) {
            fprintf(stderr, "FAIL: F0292 dead champion status-box route\n");
            failures++;
        }
        if (DM1_ChampionPanel_BuildStatusBoxModel(4, 0, 0, 100, &model)) {
            fprintf(stderr, "FAIL: F0292 status-box model accepts invalid champion\n");
            failures++;
        }
    }

    /*
     * ReDMCSB: CHAMDRAW.C F0292 line ~791:
     * - 791-801 clears a three-entry border list, then appends
     *   C038 fire shield, C039 spell shield, and C037 party/champion
     *   shield in that order for a live STATUS_BOX redraw.
     * - 802-808 blits by decrementing the count, so the visible draw
     *   order is the reverse of the append order.
     * ReDMCSB: DEFS.H line ~2197 anchors C037/C038/C039 graphic IDs.
     * ReDMCSB: DATA.C line ~207 keeps all three shield-border graphics
     * mandatory, matching the HUD route's assumption that they are loaded.
     */
    {
        enum {
            DM1_GFX_BORDER_PARTY_SHIELD = 37,
            DM1_GFX_BORDER_PARTY_FIRESHIELD = 38,
            DM1_GFX_BORDER_PARTY_SPELLSHIELD = 39
        };
        static const struct {
            int fireShieldDefense;
            int spellShieldDefense;
            int partyShieldDefense;
            int championShieldDefense;
            int expectedCount;
            int expectedAppend[3];
            int expectedDraw[3];
            const char *label;
        } borderCases[] = {
            { 0, 0, 0, 0, 0, { 0, 0, 0 }, { 0, 0, 0 }, "none" },
            { 0, 0, 1, 0, 1,
              { DM1_GFX_BORDER_PARTY_SHIELD, 0, 0 },
              { DM1_GFX_BORDER_PARTY_SHIELD, 0, 0 },
              "party_shield" },
            { 0, 0, 0, 1, 1,
              { DM1_GFX_BORDER_PARTY_SHIELD, 0, 0 },
              { DM1_GFX_BORDER_PARTY_SHIELD, 0, 0 },
              "champion_shield" },
            { 1, 1, 1, 0, 3,
              { DM1_GFX_BORDER_PARTY_FIRESHIELD,
                DM1_GFX_BORDER_PARTY_SPELLSHIELD,
                DM1_GFX_BORDER_PARTY_SHIELD },
              { DM1_GFX_BORDER_PARTY_SHIELD,
                DM1_GFX_BORDER_PARTY_SPELLSHIELD,
                DM1_GFX_BORDER_PARTY_FIRESHIELD },
              "all_party" },
            { 1, 0, 0, 1, 2,
              { DM1_GFX_BORDER_PARTY_FIRESHIELD,
                DM1_GFX_BORDER_PARTY_SHIELD, 0 },
              { DM1_GFX_BORDER_PARTY_SHIELD,
                DM1_GFX_BORDER_PARTY_FIRESHIELD, 0 },
              "fire_champion" },
        };
        size_t i;

        if (DM1_GFX_BORDER_PARTY_SHIELD != 37 ||
            DM1_GFX_BORDER_PARTY_FIRESHIELD != 38 ||
            DM1_GFX_BORDER_PARTY_SPELLSHIELD != 39) {
            fprintf(stderr, "FAIL: F0292 shield-border graphic IDs\n");
            failures++;
        }

        for (i = 0; i < sizeof(borderCases) / sizeof(borderCases[0]); ++i) {
            int appended[3] = { 0, 0, 0 };
            int drawn[3] = { 0, 0, 0 };
            int appendCount = 0;
            int drawCount = 0;
            int cursor;

            if (borderCases[i].fireShieldDefense > 0) {
                appended[appendCount++] = DM1_GFX_BORDER_PARTY_FIRESHIELD;
            }
            if (borderCases[i].spellShieldDefense > 0) {
                appended[appendCount++] = DM1_GFX_BORDER_PARTY_SPELLSHIELD;
            }
            if (borderCases[i].partyShieldDefense > 0 ||
                borderCases[i].championShieldDefense > 0) {
                appended[appendCount++] = DM1_GFX_BORDER_PARTY_SHIELD;
            }

            cursor = appendCount;
            while (cursor > 0) {
                drawn[drawCount++] = appended[--cursor];
            }

            if (appendCount != borderCases[i].expectedCount ||
                drawCount != borderCases[i].expectedCount ||
                memcmp(appended, borderCases[i].expectedAppend, sizeof(appended)) != 0 ||
                memcmp(drawn, borderCases[i].expectedDraw, sizeof(drawn)) != 0) {
                fprintf(stderr,
                        "FAIL: F0292 shield-border stack %s count=%d drawCount=%d append={%d,%d,%d} draw={%d,%d,%d}\n",
                        borderCases[i].label,
                        appendCount,
                        drawCount,
                        appended[0], appended[1], appended[2],
                        drawn[0], drawn[1], drawn[2]);
                failures++;
            }
        }
    }

    /*
     * ReDMCSB: PANEL.C:1563-1606 F0345_INVENTORY_DrawPanel_FoodWaterPoisoned
     * source lock:
     * - 1598: F0658(C030_GRAPHIC_FOOD_LABEL, C500_ZONE_FOOD, C12).
     * - 1599: F0658(C031_GRAPHIC_WATER_LABEL, C501_ZONE_WATER, C12).
     * - 1601-1606: only PoisonEventCount emits
     *   F0658(C032_GRAPHIC_POISONED_LABEL, C502_ZONE_POISONED, C12).
     * ReDMCSB: CHAMDRAW.C:1060-1063 F0292 calls F0345 from the mouth panel.
     * ReDMCSB: BASE.C:1341-1361 F0658 does the zone-index transparent blit.
     */
    {
        const DM1_ChampionPanel_F0658FoodWaterPoisonedBlitSpec *spec =
            DM1_ChampionPanel_F0658FoodWaterPoisonedBlitSpec_SourceLocked();
        const char *evidence =
            DM1_ChampionPanel_F0658PoisonedBlitSourceEvidence();
        int emittedWithoutPoison = 0;
        int emittedWithPoison = 0;
        int i;

        if (spec == NULL) {
            fprintf(stderr, "FAIL: F0658 food/water/poison spec is NULL\n");
            failures++;
        } else {
            static const int expectedBitmap[DM1_CHAMPION_PANEL_F0658_POISONED_BLIT_COUNT] = {
                DM1_GFX_FOOD_LABEL,
                DM1_GFX_WATER_LABEL,
                DM1_GFX_POISONED_LABEL
            };
            static const int expectedZone[DM1_CHAMPION_PANEL_F0658_POISONED_BLIT_COUNT] = {
                DM1_ZONE_FOOD,
                DM1_ZONE_WATER,
                DM1_ZONE_POISONED
            };
            static const int expectedLine[DM1_CHAMPION_PANEL_F0658_POISONED_BLIT_COUNT] = {
                1598,
                1599,
                1606
            };
            static const int expectedRequiresPoisoned[DM1_CHAMPION_PANEL_F0658_POISONED_BLIT_COUNT] = {
                0,
                0,
                1
            };

            if (spec->blitCount != DM1_CHAMPION_PANEL_F0658_POISONED_BLIT_COUNT) {
                fprintf(stderr, "FAIL: F0658 food/water/poison spec count %d\n",
                        spec->blitCount);
                failures++;
            }
            if (spec->sourceStartLine != 1598 ||
                spec->sourceEndLine != 1606 ||
                spec->conditionalLine != 1601) {
                fprintf(stderr,
                        "FAIL: F0658 line anchors start=%d end=%d conditional=%d\n",
                        spec->sourceStartLine,
                        spec->sourceEndLine,
                        spec->conditionalLine);
                failures++;
            }
            if (spec->sourceEvidence == NULL ||
                strstr(spec->sourceEvidence, "PANEL.C:1598-1606") == NULL ||
                strstr(spec->sourceEvidence, "F0345_INVENTORY_DrawPanel_FoodWaterPoisoned") == NULL ||
                strstr(spec->sourceEvidence, "CHAMDRAW.C:1060-1063 F0292") == NULL ||
                strstr(spec->sourceEvidence, "BASE.C:1341-1361 F0658") == NULL ||
                strstr(spec->sourceEvidence, "DEFS.H:2090 C12") == NULL ||
                strstr(spec->sourceEvidence, "3869-3871 C500/C501/C502") == NULL) {
                fprintf(stderr, "FAIL: F0658 source_evidence anchors missing\n");
                failures++;
            }
            if (evidence == NULL ||
                strcmp(evidence, spec->sourceEvidence) != 0) {
                fprintf(stderr, "FAIL: F0658 source_evidence entry mismatch\n");
                failures++;
            }

            for (i = 0; i < spec->blitCount; ++i) {
                const DM1_ChampionPanel_F0658BlitStepSpec *step =
                    &spec->blits[i];
                if (step->bitmapId != expectedBitmap[i] ||
                    step->zoneId != expectedZone[i] ||
                    step->transparentColor != DM1_COLOR_DARKEST_GRAY ||
                    step->sourceLine != expectedLine[i] ||
                    step->requiresPoisoned != expectedRequiresPoisoned[i]) {
                    fprintf(stderr,
                            "FAIL: F0658 blit[%d] bitmap=%d zone=%d transparent=%d line=%d requiresPoisoned=%d\n",
                            i,
                            step->bitmapId,
                            step->zoneId,
                            step->transparentColor,
                            step->sourceLine,
                            step->requiresPoisoned);
                    failures++;
                }
                if (!step->requiresPoisoned) {
                    emittedWithoutPoison++;
                }
                emittedWithPoison++;
            }

            if (emittedWithoutPoison != 2 || emittedWithPoison != 3) {
                fprintf(stderr,
                        "FAIL: F0658 conditional shape without=%d with=%d\n",
                        emittedWithoutPoison,
                        emittedWithPoison);
                failures++;
            }
        }
    }

    if (failures == 0) {
        printf("PASS: all champion panel HUD source-lock assertions passed.\n");
    } else {
        printf("FAIL: %d assertion(s) failed.\n", failures);
    }
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
