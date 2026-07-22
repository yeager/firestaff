#include "dm1_v1_hoc_candidate_panel_receipt_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int value, const char *message)
{
    if (!value) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
    ChampionPortraitClickInput_Compat click;
    CandidatePanelState_Compat panel;
    HocMirrorCandidateRuntimeInput_Compat runtimeInput;
    HocMirrorCandidateRuntimeReceipt_Compat selected;
    HocMirrorCandidateRuntimeReceipt_Compat cancelled;
    DM1_V1_HocCandidatePanelReceiptInputPc34 input;
    DM1_V1_HocCandidatePanelReceiptPc34 receipt;
    int ok = 1;

    memset(&click, 0, sizeof(click));
    click.command = DM1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    click.leaderEmptyHanded = 1;
    click.frontWallOrnamentHit = 1;
    click.frontSquareInBounds = 1;
    click.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    click.sensorData = 0; /* C127 ordinal zero is a valid first atlas row. */
    click.sensorCell = 2;
    click.clickedWallCell = 2;
    click.partyChampionCount = 1;
    memset(&runtimeInput, 0, sizeof(runtimeInput));
    runtimeInput.click = &click;
    runtimeInput.originalChampionRecordAvailable = 1;
    runtimeInput.c026PortraitAtlasAvailable = 1;
    runtimeInput.c040PanelGraphicAvailable = 1;
    selected = F0873_RESURRECTION_BuildHocMirrorCandidateRuntimeReceipt_Compat(
        &runtimeInput, DM1_COMMAND_CLICK_IN_DUNGEON_VIEW);
    panel.partyChampionCount = selected.nextPartyChampionCount;
    panel.candidateChampionOrdinal = selected.nextCandidateChampionOrdinal;
    runtimeInput.click = NULL;
    runtimeInput.panelState = panel;
    runtimeInput.activeMirrorOrdinal = click.sensorData;
    runtimeInput.activeMirrorSourceBound = 1;
    runtimeInput.c026PortraitAtlasAvailable = 1;
    runtimeInput.c040PanelGraphicAvailable = 1;
    cancelled = F0873_RESURRECTION_BuildHocMirrorCandidateRuntimeReceipt_Compat(
        &runtimeInput, DM1_COMMAND_CANCEL);

    memset(&input, 0, sizeof(input));
    input.activeReceipt = &selected;
    input.activeMirrorOrdinal = click.sensorData;
    input.activeMirrorSourceBound = 1;
    input.c127SensorOrdinal = click.sensorData;
    input.c127SensorEnabled = 1;
    input.c026PortraitAtlasAvailable = 1;
    input.c040PanelGraphicAvailable = 1;
    input.activePanelGeneration = 7;
    input.presentedPanelGeneration = 7;
    input.transition = DM1_V1_HOC_CANDIDATE_PANEL_REDRAW_PC34;
    ok &= check(DM1_V1_HocCandidatePanel_BuildReceiptPc34(&input, &receipt) &&
                receipt.valid && receipt.sourceOwned && receipt.redrawC026Portrait &&
                receipt.redrawC040Panel && receipt.keepsSensorEnabled &&
                receipt.mirrorOrdinal == 0,
                "redraw owns valid ordinal-zero C127/C026/C040 state");

    input.activeReceipt = &cancelled;
    input.transition = DM1_V1_HOC_CANDIDATE_PANEL_CLOSE_PC34;
    ok &= check(DM1_V1_HocCandidatePanel_BuildReceiptPc34(&input, &receipt) &&
                receipt.valid && receipt.clearC040Panel &&
                receipt.restoreC127Portrait && receipt.redrawC026Portrait &&
                receipt.keepsSensorEnabled && !receipt.disablesSensor &&
                receipt.nextPanelGeneration == 8,
                "C162 closes C040 and restores only the live C127 portrait");

    input.activeReceipt = &selected;
    input.reopenReceipt = &selected;
    input.transition = DM1_V1_HOC_CANDIDATE_PANEL_REOPEN_PC34;
    ok &= check(DM1_V1_HocCandidatePanel_BuildReceiptPc34(&input, &receipt) &&
                receipt.valid && receipt.opensCandidatePanel &&
                receipt.redrawC040Panel && receipt.nextPanelGeneration == 8,
                "reopen accepts the same source-owned C127 selection");

    input.c127SensorEnabled = 0;
    ok &= check(DM1_V1_HocCandidatePanel_BuildReceiptPc34(&input, &receipt) &&
                !receipt.valid,
                "reopen rejects a disabled source mirror sensor");

    input.c127SensorEnabled = 1;
    input.presentedPanelGeneration = 8;
    ok &= check(DM1_V1_HocCandidatePanel_BuildReceiptPc34(&input, &receipt) &&
                !receipt.valid,
                "reopen rejects a stale C040 generation");

    if (!ok) {
        return 1;
    }
    puts("ok: DM1 HoC C127/C040 redraw-close-reopen receipt is source-owned");
    return 0;
}
