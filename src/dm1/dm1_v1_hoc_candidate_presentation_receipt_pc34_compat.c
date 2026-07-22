#include "dm1_v1_hoc_candidate_presentation_receipt_pc34_compat.h"

#include <string.h>

int DM1_V1_HocCandidatePresentation_BuildReceiptPc34(
    const DM1_V1_HocCandidatePresentationInputPc34 *input,
    DM1_V1_HocCandidatePresentationReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidatePresentationReceiptPc34 receipt;
    const DM1_V1_HocCandidateApplyReceiptPc34 *apply;
    const DM1_V1_HocCandidatePresentationStatePc34 *state;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB REVIVE.C F0282; PANEL.C F0352; DUNGEON.C F0172/F0174; "
        "DUNVIEW.C:3913-3928";
    apply = input->apply;
    state = &input->state;

    if (!apply || !apply->valid || !apply->sourceOwned || !apply->atomic ||
        !apply->clearG0299CandidateOrdinal || !apply->writePartyChampionCount ||
        !apply->closeC040Panel || !apply->clearStalePanelPayload ||
        !state->mirrorSourceBound || state->mirrorOrdinal != apply->mirrorOrdinal ||
        state->candidateChampionOrdinal != 0u ||
        state->partyChampionCount != apply->nextPartyChampionCount ||
        state->c040PanelOpen ||
        state->presentedPanelGeneration != apply->nextPresentedPanelGeneration ||
        !state->c040PanelGraphicAvailable) {
        *outReceipt = receipt;
        return 1;
    }

    if (apply->writeMirrorSensorEnabled) {
        if (apply->nextMirrorSensorEnabled || state->c127SensorEnabled ||
            state->sensorGeneration != state->previousSensorGeneration + 1u ||
            apply->drawC026Portrait || !apply->clearStaleC026Portrait) {
            *outReceipt = receipt;
            return 1;
        }
        receipt.valid = 1;
        receipt.sourceOwned = 1;
        receipt.clearC040Panel = 1;
        receipt.clearStalePanelPayload = 1;
        receipt.clearStaleC026Portrait = 1;
        receipt.sensorEnabled = 0;
        receipt.mirrorOrdinal = apply->mirrorOrdinal;
        receipt.sensorGeneration = state->sensorGeneration;
        receipt.presentedPanelGeneration = state->presentedPanelGeneration;
        *outReceipt = receipt;
        return 1;
    }

    if (apply->nextMirrorSensorEnabled || !state->c127SensorEnabled ||
        state->sensorGeneration != state->previousSensorGeneration ||
        !apply->drawC026Portrait || !state->c026PortraitAtlasAvailable) {
        *outReceipt = receipt;
        return 1;
    }
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.clearC040Panel = 1;
    receipt.clearStalePanelPayload = 1;
    receipt.drawC026Portrait = 1;
    receipt.sensorEnabled = 1;
    receipt.mirrorOrdinal = apply->mirrorOrdinal;
    receipt.sensorGeneration = state->sensorGeneration;
    receipt.presentedPanelGeneration = state->presentedPanelGeneration;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidatePresentation_SourceEvidencePc34(void)
{
    return "ReDMCSB REVIVE.C F0282 clears G0299/C040 and retires the mirror "
           "sensor only for C160/C161; PANEL.C F0352 clears the panel; "
           "DUNGEON.C F0172/F0174 and DUNVIEW.C:3913-3928 redraw C026 only "
           "after C162 leaves the matching C127 sensor live.";
}
