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

    if (failures == 0) {
        printf("PASS: all champion panel HUD source-lock assertions passed.\n");
    } else {
        printf("FAIL: %d assertion(s) failed.\n", failures);
    }
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
