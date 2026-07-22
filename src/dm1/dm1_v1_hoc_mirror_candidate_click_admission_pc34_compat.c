#include "dm1_v1_hoc_mirror_candidate_click_admission_pc34_compat.h"

#include <string.h>

static int material_valid(const DM1_V1_HocMirrorCandidateSourceMaterialPc34 *material,
                          int graphicIndex)
{
    return material && material->sourceOwned && material->graphicIndex == graphicIndex &&
           material->width > 0 && material->height > 0 && material->sourceHash != 0u;
}

static int raw_inputs_match(const DM1_V1_HocMirrorCandidateClickAdmissionInputPc34 *input)
{
    const DM1_V1_ChampionMirrorRuntimeRenderInputPc34 *mirror = &input->mirrorRuntime;
    const DM1_V1_HocCandidateClickPresentationInputPc34 *click =
        &input->clickPresentation;

    return mirror->wallSquareVisible &&
           mirror->sensorType == DM1_SENSOR_WALL_CHAMPION_PORTRAIT &&
           mirror->thingCell == mirror->visibleWallCell &&
           !mirror->candidatePanelActive && mirror->backingAssetAvailable &&
           mirror->runtimeThingReceipt && mirror->runtimeThingReceipt->valid &&
           click->click.command == DM1_COMMAND_CLICK_IN_DUNGEON_VIEW &&
           click->click.sensorType == mirror->sensorType &&
           click->click.sensorData == mirror->sensorData &&
           click->click.sensorCell == mirror->thingCell &&
           click->click.clickedWallCell == mirror->visibleWallCell &&
           click->c127SensorEnabled &&
           click->c127SensorOrdinal == (uint16_t)mirror->sensorData &&
           !click->candidatePanelAlreadyActive &&
           click->originalChampionRecordAvailable &&
           click->c026PortraitAtlasAvailable && click->c040PanelGraphicAvailable;
}

int DM1_V1_HocMirrorCandidateClickAdmission_BuildReceiptPc34(
    const DM1_V1_HocMirrorCandidateClickAdmissionInputPc34 *input,
    DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34 *outReceipt)
{
    DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34 receipt;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB DUNGEON.C F0172/F0174 C127; MOVESENS.C F0280 click; "
        "PANEL.C C040; DUNVIEW.C:3913-3928 C026 source admission";
    if (!raw_inputs_match(input) ||
        !DM1_V1_ChampionMirror_BuildRuntimeRenderDecisionPc34(
            &input->mirrorRuntime, &receipt.mirrorDecision) ||
        !DM1_V1_HocCandidateClickPresentation_BuildReceiptPc34(
            &input->clickPresentation, &receipt.clickReceipt) ||
        !receipt.mirrorDecision.valid || !receipt.mirrorDecision.consumedF0172Sensor ||
        !receipt.mirrorDecision.drawFrontWallOverlay ||
        !receipt.mirrorDecision.drawChampionPortraitAsWallOverlay ||
        !receipt.mirrorDecision.suppressHostFallbackVisuals ||
        !receipt.mirrorDecision.hostDraw.sourceAssetsBound ||
        !receipt.mirrorDecision.hostDraw.sourceOwnedExecution ||
        receipt.mirrorDecision.hostDraw.drawMirrorBackingFallbackRect ||
        !receipt.clickReceipt.valid || !receipt.clickReceipt.sourceOwned ||
        !receipt.clickReceipt.portraitHit || !receipt.clickReceipt.consumedC127Sensor ||
        !receipt.clickReceipt.opensCandidatePanel || !receipt.clickReceipt.drawC026Portrait ||
        !receipt.clickReceipt.drawC040Panel ||
        receipt.mirrorDecision.frontWall.championPortraitRenderIndex !=
            (int)receipt.clickReceipt.mirrorOrdinal ||
        receipt.mirrorDecision.render.renderIndex != (int)receipt.clickReceipt.mirrorOrdinal ||
        !material_valid(&input->c026, DM1_V1_CHAMPION_MIRROR_PORTRAIT_GRAPHIC_PC34_COMPAT) ||
        input->c026.sourceX != receipt.mirrorDecision.render.sourceX ||
        input->c026.sourceY != receipt.mirrorDecision.render.sourceY ||
        input->c026.width != receipt.mirrorDecision.render.width ||
        input->c026.height != receipt.mirrorDecision.render.height ||
        input->c026.dstX != receipt.mirrorDecision.render.dstX ||
        input->c026.dstY != receipt.mirrorDecision.render.dstY ||
        !material_valid(&input->c040, 40)) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.admitCandidateClick = 1;
    receipt.consumedC127 = 1;
    receipt.admitC026Portrait = 1;
    receipt.admitC040Panel = 1;
    receipt.suppressFallbackVisuals = 1;
    receipt.mirrorOrdinal = receipt.clickReceipt.mirrorOrdinal;
    receipt.panelGeneration = receipt.clickReceipt.panelGeneration;
    receipt.c026 = input->c026;
    receipt.c040 = input->c040;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocMirrorCandidateClickAdmission_SourceEvidencePc34(void)
{
    return "ReDMCSB F0172/F0174 supplies the real C127 front-wall ordinal; "
           "F0280 accepts its matching click and admits only the decoded C026/C040 "
           "source material, never a fallback visual.";
}
