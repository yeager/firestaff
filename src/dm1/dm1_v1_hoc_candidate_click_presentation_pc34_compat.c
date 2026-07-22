#include "dm1_v1_hoc_candidate_click_presentation_pc34_compat.h"

#include <string.h>

static int in_c026_click_rect(int x, int y)
{
    return x >= DM1_V1_HOC_C026_CLICK_LEFT_PC34 &&
           x <= DM1_V1_HOC_C026_CLICK_RIGHT_PC34 &&
           y >= DM1_V1_HOC_C026_CLICK_TOP_PC34 &&
           y <= DM1_V1_HOC_C026_CLICK_BOTTOM_PC34;
}

int DM1_V1_HocCandidateClickPresentation_BuildReceiptPc34(
    const DM1_V1_HocCandidateClickPresentationInputPc34 *input,
    DM1_V1_HocCandidateClickPresentationReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateClickPresentationReceiptPc34 receipt;
    HocMirrorCandidateRuntimeInput_Compat runtimeInput;
    DM1_V1_HocCandidatePanelReceiptInputPc34 panelInput;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB COMMAND.C C080; MOVESENS.C C127/F0280; REVIVE.C F0280; "
        "PANEL.C F0344-F0347; DUNVIEW.C:3913-3928 C026";

    if (!in_c026_click_rect(input->viewportX, input->viewportY) ||
        !input->c127SensorEnabled ||
        input->click.sensorType != DM1_SENSOR_WALL_CHAMPION_PORTRAIT ||
        input->click.sensorData != input->c127SensorOrdinal ||
        input->click.sensorCell != input->click.clickedWallCell ||
        input->c127SensorOrdinal >= DM1_V1_CHAMPION_MIRROR_PORTRAIT_ATLAS_COUNT_PC34_COMPAT ||
        input->priorPanelGeneration == UINT32_MAX ||
        input->presentedPanelGeneration != input->priorPanelGeneration) {
        *outReceipt = receipt;
        return 1;
    }

    memset(&runtimeInput, 0, sizeof(runtimeInput));
    runtimeInput.click = &input->click;
    runtimeInput.originalChampionRecordAvailable =
        input->originalChampionRecordAvailable;
    runtimeInput.c026PortraitAtlasAvailable = input->c026PortraitAtlasAvailable;
    runtimeInput.c040PanelGraphicAvailable = input->c040PanelGraphicAvailable;
    runtimeInput.candidatePanelAlreadyActive = input->candidatePanelAlreadyActive;
    receipt.runtime = F0873_RESURRECTION_BuildHocMirrorCandidateRuntimeReceipt_Compat(
        &runtimeInput, DM1_COMMAND_CLICK_IN_DUNGEON_VIEW);
    if (!receipt.runtime.valid || !receipt.runtime.sourceOwned ||
        !receipt.runtime.opensCandidatePanel ||
        receipt.runtime.mirrorOrdinal != input->c127SensorOrdinal) {
        *outReceipt = receipt;
        return 1;
    }

    memset(&panelInput, 0, sizeof(panelInput));
    panelInput.transition = DM1_V1_HOC_CANDIDATE_PANEL_REDRAW_PC34;
    panelInput.activeReceipt = &receipt.runtime;
    panelInput.activeMirrorOrdinal = receipt.runtime.mirrorOrdinal;
    panelInput.activeMirrorSourceBound = 1;
    panelInput.c127SensorOrdinal = input->c127SensorOrdinal;
    panelInput.c127SensorEnabled = 1;
    panelInput.c026PortraitAtlasAvailable = input->c026PortraitAtlasAvailable;
    panelInput.c040PanelGraphicAvailable = input->c040PanelGraphicAvailable;
    panelInput.activePanelGeneration = input->priorPanelGeneration + 1u;
    panelInput.presentedPanelGeneration = panelInput.activePanelGeneration;
    if (!DM1_V1_HocCandidatePanel_BuildReceiptPc34(&panelInput,
                                                    &receipt.panel) ||
        !receipt.panel.valid || !receipt.panel.sourceOwned ||
        !receipt.panel.redrawC026Portrait || !receipt.panel.redrawC040Panel) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.portraitHit = 1;
    receipt.consumedC127Sensor = 1;
    receipt.opensCandidatePanel = 1;
    receipt.drawC026Portrait = 1;
    receipt.drawC040Panel = 1;
    receipt.mirrorOrdinal = receipt.runtime.mirrorOrdinal;
    receipt.candidateChampionIndex = receipt.runtime.candidateChampionIndex;
    receipt.candidateChampionOrdinal = receipt.runtime.nextCandidateChampionOrdinal;
    receipt.panelGeneration = panelInput.activePanelGeneration;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateClickPresentation_SourceEvidencePc34(void)
{
    return "ReDMCSB COMMAND.C C080 routes the front wall click; MOVESENS.C "
           "checks C127's matching wall cell and calls F0280; REVIVE.C F0280 "
           "publishes G0299; PANEL.C F0344-F0347 redraws C040; DUNVIEW.C "
           "3913-3928 draws C026 at G0109 {96,127,35,63}.";
}
