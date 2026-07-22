#include "dm1_v1_hoc_candidate_click_presentation_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

static DM1_V1_HocCandidateClickPresentationInputPc34 base_input(void)
{
    DM1_V1_HocCandidateClickPresentationInputPc34 input;
    memset(&input, 0, sizeof(input));
    input.click.command = DM1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    input.click.leaderEmptyHanded = 1;
    input.click.frontWallOrnamentHit = 1;
    input.click.frontSquareInBounds = 1;
    input.click.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    input.click.sensorData = 0;
    input.click.sensorCell = 2;
    input.click.clickedWallCell = 2;
    input.click.partyChampionCount = 1;
    input.viewportX = 96;
    input.viewportY = 35;
    input.c127SensorOrdinal = 0;
    input.c127SensorEnabled = 1;
    input.originalChampionRecordAvailable = 1;
    input.c026PortraitAtlasAvailable = 1;
    input.c040PanelGraphicAvailable = 1;
    input.priorPanelGeneration = 9;
    input.presentedPanelGeneration = 9;
    return input;
}

int main(void)
{
    DM1_V1_HocCandidateClickPresentationInputPc34 input = base_input();
    DM1_V1_HocCandidateClickPresentationReceiptPc34 receipt;
    int ok = 1;

    ok &= expect(DM1_V1_HocCandidateClickPresentation_BuildReceiptPc34(
                     &input, &receipt) && receipt.valid && receipt.sourceOwned &&
                 receipt.portraitHit && receipt.consumedC127Sensor &&
                 receipt.opensCandidatePanel && receipt.drawC026Portrait &&
                 receipt.drawC040Panel && receipt.mirrorOrdinal == 0 &&
                 receipt.candidateChampionOrdinal == 2 &&
                 receipt.panelGeneration == 10,
                 "C127 ordinal-zero portrait click creates source-owned C026/C040 presentation");

    input = base_input();
    input.viewportX = 95;
    ok &= expect(DM1_V1_HocCandidateClickPresentation_BuildReceiptPc34(
                     &input, &receipt) && !receipt.valid,
                 "click outside source C026 rectangle cannot open C040");

    input = base_input();
    input.click.clickedWallCell = 3;
    ok &= expect(DM1_V1_HocCandidateClickPresentation_BuildReceiptPc34(
                     &input, &receipt) && !receipt.valid,
                 "wrong C127 wall cell cannot open C040");

    input = base_input();
    input.c040PanelGraphicAvailable = 0;
    ok &= expect(DM1_V1_HocCandidateClickPresentation_BuildReceiptPc34(
                     &input, &receipt) && !receipt.valid,
                 "missing source C040 graphic fails closed");

    input = base_input();
    input.presentedPanelGeneration = 10;
    ok &= expect(DM1_V1_HocCandidateClickPresentation_BuildReceiptPc34(
                     &input, &receipt) && !receipt.valid,
                 "stale panel generation cannot present a portrait click");

    if (!ok) return 1;
    puts("ok: DM1 HoC portrait click to C127/C026/C040 presentation is source-owned");
    return 0;
}
