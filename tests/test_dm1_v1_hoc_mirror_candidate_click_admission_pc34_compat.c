#include "dm1_v1_hoc_mirror_candidate_click_admission_pc34_compat.h"

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

static DM1_V1_HocMirrorCandidateClickAdmissionInputPc34 base_input(
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 *thing)
{
    DM1_V1_HocMirrorCandidateClickAdmissionInputPc34 input;
    memset(&input, 0, sizeof(input));
    memset(thing, 0, sizeof(*thing));
    thing->valid = 1; thing->thing_cell = 7; thing->view_cell = 7;
    input.mirrorRuntime.wallSquareVisible = 1;
    input.mirrorRuntime.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    input.mirrorRuntime.sensorData = 3; input.mirrorRuntime.ornamentOrdinal = 43;
    input.mirrorRuntime.thingCell = 7; input.mirrorRuntime.visibleWallCell = 7;
    input.mirrorRuntime.backingAssetAvailable = 1; input.mirrorRuntime.runtimeThingReceipt = thing;
    input.clickPresentation.click.command = DM1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    input.clickPresentation.click.leaderEmptyHanded = 1;
    input.clickPresentation.click.frontWallOrnamentHit = 1;
    input.clickPresentation.click.frontSquareInBounds = 1;
    input.clickPresentation.click.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    input.clickPresentation.click.sensorData = 3;
    input.clickPresentation.click.sensorCell = 7; input.clickPresentation.click.clickedWallCell = 7;
    input.clickPresentation.click.partyChampionCount = 1;
    input.clickPresentation.viewportX = 96; input.clickPresentation.viewportY = 35;
    input.clickPresentation.c127SensorOrdinal = 3; input.clickPresentation.c127SensorEnabled = 1;
    input.clickPresentation.originalChampionRecordAvailable = 1;
    input.clickPresentation.c026PortraitAtlasAvailable = 1;
    input.clickPresentation.c040PanelGraphicAvailable = 1;
    input.clickPresentation.priorPanelGeneration = 9;
    input.clickPresentation.presentedPanelGeneration = 9;
    input.c026.sourceOwned = 1; input.c026.graphicIndex = 26;
    input.c026.sourceX = 96; input.c026.sourceY = 0; input.c026.width = 32;
    input.c026.height = 29; input.c026.dstX = 96; input.c026.dstY = 35;
    input.c026.sourceHash = 0x2626u;
    input.c040.sourceOwned = 1; input.c040.graphicIndex = 40;
    input.c040.width = 144; input.c040.height = 73; input.c040.sourceHash = 0x4040u;
    return input;
}

int main(void)
{
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 thing;
    DM1_V1_HocMirrorCandidateClickAdmissionInputPc34 input = base_input(&thing);
    DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34 receipt;
    int ok = 1;

    ok &= expect(DM1_V1_HocMirrorCandidateClickAdmission_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.sourceOwned && receipt.admitCandidateClick &&
                 receipt.consumedC127 && receipt.admitC026Portrait && receipt.admitC040Panel &&
                 receipt.suppressFallbackVisuals && receipt.mirrorOrdinal == 3 &&
                 receipt.panelGeneration == 10,
                 "real C127 front wall and matching click admit C026/C040 candidate route");

    input.clickPresentation.click.sensorData = 2;
    ok &= expect(DM1_V1_HocMirrorCandidateClickAdmission_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "mismatched C127 click data fails closed without fallback");

    if (!ok) return 1;
    puts("ok: DM1 HoC mirror candidate click admission is source-owned");
    return 0;
}
