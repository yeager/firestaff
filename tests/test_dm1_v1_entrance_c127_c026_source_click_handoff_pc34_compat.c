#include "dm1_v1_entrance_champion_select_pc34_compat.h"

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

static DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34
build_source_receipt(DM1V1D1LD1RF0115RuntimeThingReceiptPc34 *thing)
{
    DM1_V1_HocMirrorCandidateClickAdmissionInputPc34 input;
    DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34 receipt;

    memset(&input, 0, sizeof(input));
    memset(&receipt, 0, sizeof(receipt));
    memset(thing, 0, sizeof(*thing));
    thing->valid = 1;
    thing->thing_cell = 7;
    thing->view_cell = 7;
    input.mirrorRuntime.wallSquareVisible = 1;
    input.mirrorRuntime.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    input.mirrorRuntime.sensorData = 3;
    input.mirrorRuntime.ornamentOrdinal = 43;
    input.mirrorRuntime.thingCell = 7;
    input.mirrorRuntime.visibleWallCell = 7;
    input.mirrorRuntime.backingAssetAvailable = 1;
    input.mirrorRuntime.runtimeThingReceipt = thing;
    input.clickPresentation.click.command = DM1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    input.clickPresentation.click.leaderEmptyHanded = 1;
    input.clickPresentation.click.frontWallOrnamentHit = 1;
    input.clickPresentation.click.frontSquareInBounds = 1;
    input.clickPresentation.click.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    input.clickPresentation.click.sensorData = 3;
    input.clickPresentation.click.sensorCell = 7;
    input.clickPresentation.click.clickedWallCell = 7;
    input.clickPresentation.click.partyChampionCount = 1;
    input.clickPresentation.viewportX = 96;
    input.clickPresentation.viewportY = 35;
    input.clickPresentation.c127SensorOrdinal = 3;
    input.clickPresentation.c127SensorEnabled = 1;
    input.clickPresentation.originalChampionRecordAvailable = 1;
    input.clickPresentation.c026PortraitAtlasAvailable = 1;
    input.clickPresentation.c040PanelGraphicAvailable = 1;
    input.clickPresentation.priorPanelGeneration = 1;
    input.clickPresentation.presentedPanelGeneration = 1;
    input.c026.sourceOwned = 1;
    input.c026.graphicIndex = DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_GRAPHIC_PC34;
    input.c026.sourceX = 96;
    input.c026.sourceY = 0;
    input.c026.width = 32;
    input.c026.height = 29;
    input.c026.dstX = 96;
    input.c026.dstY = 35;
    input.c026.sourceHash = 0x2626u;
    input.c040.sourceOwned = 1;
    input.c040.graphicIndex = DM1_V1_ENTRANCE_RESURRECT_PANEL_GRAPHIC_PC34;
    input.c040.width = 144;
    input.c040.height = 73;
    input.c040.sourceHash = 0x4040u;
    (void)DM1_V1_HocMirrorCandidateClickAdmission_BuildReceiptPc34(&input,
                                                                     &receipt);
    return receipt;
}

int main(void)
{
    DM1_V1_EntranceCtxPc34 ctx;
    DM1_V1_EntranceTickResultPc34 result;
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 thing;
    DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34 receipt;
    int ok = 1;

    receipt = build_source_receipt(&thing);
    DM1_V1_Entrance_InitPc34Compat(&ctx);
    ctx.state = DM1_ENTRANCE_VIEWING;
    ok &= expect(DM1_V1_Entrance_AddMirrorPc34Compat(&ctx, 3, 2, 3, 2, 0) == 0,
                 "entrance mirror is sourced from ordinal 3");
    memset(&result, 0, sizeof(result));
    ok &= expect(DM1_V1_Entrance_ClickMirrorFromSourceReceiptPc34Compat(
                     &ctx, 0, 100u, &receipt, &result) && result.mirrorSelected &&
                     ctx.state == DM1_ENTRANCE_SELECTING,
                 "live C127 and exact C026/C040 material select the mirror");

    DM1_V1_Entrance_InitPc34Compat(&ctx);
    ctx.state = DM1_ENTRANCE_VIEWING;
    (void)DM1_V1_Entrance_AddMirrorPc34Compat(&ctx, 3, 2, 3, 2, 0);
    receipt.c026.sourceX++;
    memset(&result, 0, sizeof(result));
    ok &= expect(!DM1_V1_Entrance_ClickMirrorFromSourceReceiptPc34Compat(
                     &ctx, 0, 100u, &receipt, &result) && !result.mirrorSelected &&
                     ctx.state == DM1_ENTRANCE_VIEWING,
                 "shifted C026 material cannot select a source mirror");

    receipt = build_source_receipt(&thing);
    receipt.mirrorDecision.consumedF0115ThingReceipt = 0;
    memset(&result, 0, sizeof(result));
    ok &= expect(!DM1_V1_Entrance_ClickMirrorFromSourceReceiptPc34Compat(
                     &ctx, 0, 100u, &receipt, &result),
                 "missing DUNGEON.DAT C127 receipt fails closed");

    if (!ok) return 1;
    puts("ok: DM1 Entrance C127/C026 source click handoff is fail-closed");
    return 0;
}
