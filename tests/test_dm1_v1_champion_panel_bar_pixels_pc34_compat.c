/*
 * CTest gate: DM1 V1 Champion Panel PC34 bar pixel bands.
 *
 * Source-locks the CHAMDRAW.C F0287 PC34 bar split, which is not covered by
 * the existing champion-panel runtime/status probes or the old fixed-point
 * bar-height helper. Synthetic current/max fixtures only.
 */

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>

static int expect_int(const char *label, int actual, int expected)
{
    if (actual == expected) {
        return 0;
    }
    fprintf(stderr, "FAIL: %s got %d expected %d\n",
            label, actual, expected);
    return 1;
}

static int expect_bar(const char *label,
                      int championIndex, int statIndex,
                      int current, int maximum,
                      int expectedZone,
                      int expectedX,
                      int expectedBlankHeight,
                      int expectedFillY,
                      int expectedFillHeight,
                      int expectedEmitsBlank,
                      int expectedEmitsFill)
{
    DM1_ChampionPanel_BarFillModel model;
    int failures = 0;

    if (!DM1_ChampionPanel_BuildPc34BarFillModel(
            championIndex, statIndex, current, maximum, &model)) {
        fprintf(stderr, "FAIL: %s model build rejected valid fixture\n", label);
        return 1;
    }

    /* ReDMCSB CHAMDRAW.C:307-309: zone is C195 + champion + stat*4,
     * then F0638_GetZone supplies the 4x25 full bar rectangle. */
    failures += expect_int(label, model.zoneId, expectedZone);
    failures += expect_int("bar x", model.x, expectedX);
    failures += expect_int("bar y", model.y, 2);
    failures += expect_int("bar width", model.width, DM1_BAR_GRAPH_WIDTH);
    failures += expect_int("bar height", model.height, DM1_BAR_GRAPH_MAX_HEIGHT);

    /* ReDMCSB CHAMDRAW.C:310-327: when current < maximum, the top blank band
     * is filled with C12 after subtracting max(1, height*current/maximum). */
    failures += expect_int("blank x", model.blankX, expectedX);
    failures += expect_int("blank y", model.blankY, 2);
    failures += expect_int("blank width", model.blankWidth, DM1_BAR_GRAPH_WIDTH);
    failures += expect_int("blank height", model.blankHeight, expectedBlankHeight);
    failures += expect_int("blank color", model.blankColor, DM1_COLOR_DARKEST_GRAY);
    failures += expect_int("blank emitted", model.emitsBlank, expectedEmitsBlank);

    /* ReDMCSB CHAMDRAW.C:335-342: nonzero current moves the colored band below
     * the blank band and fills it with G0046 champion color. */
    failures += expect_int("fill x", model.fillX, expectedX);
    failures += expect_int("fill y", model.fillY, expectedFillY);
    failures += expect_int("fill width", model.fillWidth, DM1_BAR_GRAPH_WIDTH);
    failures += expect_int("fill height", model.fillHeight, expectedFillHeight);
    failures += expect_int("fill color", model.fillColor,
                           DM1_ChampionColor[championIndex]);
    failures += expect_int("fill emitted", model.emitsFill, expectedEmitsFill);

    return failures;
}

int main(void)
{
    DM1_ChampionPanel_BarFillModel model;
    int failures = 0;

    printf("== DM1 V1 Champion Panel PC34 bar pixel-band source-lock ==\n");

    /*
     * ReDMCSB CHAMDRAW.C:307-342 anchors every HP tick below:
     * C195 + champion + stat*4 selects the HP zone, current < maximum emits
     * C12 blank pixels, current != 0 emits the champion-color bottom band.
     */
    failures += expect_bar("hp 0/25", 0, DM1_STATUS_VALUE_HEALTH, 0, 25,
                           195, 46, 25, 27, 0, 1, 0);
    failures += expect_bar("hp 1/25", 0, DM1_STATUS_VALUE_HEALTH, 1, 25,
                           195, 46, 24, 26, 1, 1, 1);
    failures += expect_bar("hp 12/25", 0, DM1_STATUS_VALUE_HEALTH, 12, 25,
                           195, 46, 13, 15, 12, 1, 1);
    failures += expect_bar("hp 13/25", 0, DM1_STATUS_VALUE_HEALTH, 13, 25,
                           195, 46, 12, 14, 13, 1, 1);
    failures += expect_bar("hp 24/25", 0, DM1_STATUS_VALUE_HEALTH, 24, 25,
                           195, 46, 1, 3, 24, 1, 1);
    failures += expect_bar("hp 25/25", 0, DM1_STATUS_VALUE_HEALTH, 25, 25,
                           195, 46, 0, 2, 25, 0, 1);
    failures += expect_bar("hp 30/25", 0, DM1_STATUS_VALUE_HEALTH, 30, 25,
                           195, 46, 0, 2, 25, 0, 1);

    /*
     * ReDMCSB CHAMDRAW.C:320-321: PC34 uses integer floor for
     * height*current/maximum after enforcing a one-pixel nonzero minimum.
     * This locks the visible 50/100 split to 13 blank pixels + 12 filled
     * pixels, distinct from the older fixed-point ceil helper.
     */
    failures += expect_bar("hp 50/100", 1, DM1_STATUS_VALUE_HEALTH, 50, 100,
                           196, 115, 13, 15, 12, 1, 1);
    failures += expect_bar("hp 1/100", 1, DM1_STATUS_VALUE_HEALTH, 1, 100,
                           196, 115, 24, 26, 1, 1, 1);

    /*
     * ReDMCSB CHAMDRAW.C:157-163 loads HP, stamina, mana as rows 0, 1, 2;
     * CHAMDRAW.C:307-308 advances zone indexes by +4, matching
     * C195/C199/C203 plus the champion ordinal.
     */
    failures += expect_bar("champion2 stamina 25/100", 2, DM1_STATUS_VALUE_STAMINA,
                           25, 100, 201, 191, 19, 21, 6, 1, 1);
    failures += expect_bar("champion2 mana 100/100", 2, DM1_STATUS_VALUE_MANA,
                           100, 100, 205, 198, 0, 2, 25, 0, 1);

    /*
     * ReDMCSB CHAMDRAW.C:307-342 only operates on four champions, three bar
     * counters, and valid positive maxima; invalid synthetic fixtures must
     * not produce a model.
     */
    if (DM1_ChampionPanel_BuildPc34BarFillModel(4, 0, 1, 25, &model) ||
        DM1_ChampionPanel_BuildPc34BarFillModel(0, 3, 1, 25, &model) ||
        DM1_ChampionPanel_BuildPc34BarFillModel(0, 0, 1, 0, &model)) {
        fprintf(stderr, "FAIL: F0287 PC34 bar model accepted invalid input\n");
        failures++;
    }

    if (failures == 0) {
        printf("PASS: PC34 champion-panel bar pixel-band assertions passed.\n");
    } else {
        printf("FAIL: %d assertion(s) failed.\n", failures);
    }

    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
