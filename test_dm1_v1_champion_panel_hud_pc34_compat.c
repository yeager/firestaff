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

    if (failures == 0) {
        printf("PASS: all champion panel HUD source-lock assertions passed.\n");
    } else {
        printf("FAIL: %d assertion(s) failed.\n", failures);
    }
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
