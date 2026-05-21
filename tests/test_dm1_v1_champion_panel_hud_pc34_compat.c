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
