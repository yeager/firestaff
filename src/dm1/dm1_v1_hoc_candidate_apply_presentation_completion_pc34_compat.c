#include "dm1_v1_hoc_candidate_apply_presentation_completion_pc34_compat.h"

#include <string.h>

int DM1_V1_HocCandidateCompletion_BuildReceiptPc34(
    const DM1_V1_HocCandidateCompletionInputPc34 *input,
    DM1_V1_HocCandidateCompletionReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateCompletionReceiptPc34 receipt;
    const DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34 *bridge;
    const DM1_V1_HocCandidateApplyReceiptPc34 *apply;
    const DM1_V1_HocCandidatePresentationReceiptPc34 *presentation;
    const DM1_V1_HocCandidateCompletionStatePc34 *state;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB REVIVE.C F0282; PANEL.C F0352; DUNGEON.C F0172/F0174; "
        "DUNVIEW.C:3913-3928";
    bridge = input->bridge;
    apply = input->apply;
    presentation = input->presentation;
    state = &input->state;

    if (!bridge || !apply || !presentation || !bridge->valid ||
        !bridge->sourceOwned || !bridge->atomic || !apply->valid ||
        !apply->sourceOwned || !apply->atomic || !presentation->valid ||
        !presentation->sourceOwned || !bridge->handoff.valid ||
        !bridge->handoff.sourceOwned ||
        !apply->clearG0299CandidateOrdinal || !apply->writePartyChampionCount ||
        !apply->closeC040Panel || !apply->clearStalePanelPayload ||
        !presentation->clearC040Panel || !presentation->clearStalePanelPayload ||
        !state->mirrorSourceBound ||
        state->mirrorOrdinal != bridge->handoff.mirrorOrdinal ||
        state->candidateChampionOrdinal != 0u || state->c040PanelOpen ||
        state->partyChampionCount != bridge->handoff.nextPartyChampionCount ||
        apply->nextPartyChampionCount != bridge->handoff.nextPartyChampionCount ||
        apply->mirrorOrdinal != bridge->handoff.mirrorOrdinal ||
        state->presentedPanelGeneration != apply->nextPresentedPanelGeneration ||
        state->presentedPanelGeneration != presentation->presentedPanelGeneration ||
        state->sensorGeneration != presentation->sensorGeneration) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.atomic = 1;
    receipt.publishC040Cleared = 1;
    receipt.publishCandidateCleared = 1;
    receipt.mirrorOrdinal = state->mirrorOrdinal;
    receipt.sensorGeneration = state->sensorGeneration;
    receipt.presentedPanelGeneration = state->presentedPanelGeneration;

    if (bridge->handoff.disablesMirrorSensor) {
        if (!apply->writeMirrorSensorEnabled || apply->nextMirrorSensorEnabled ||
            !apply->clearStaleC026Portrait || apply->drawC026Portrait ||
            presentation->sensorEnabled || !presentation->clearStaleC026Portrait ||
            presentation->drawC026Portrait || state->c127SensorEnabled ||
            state->c026PortraitVisible) {
            memset(&receipt, 0, sizeof(receipt));
            receipt.sourceAnchor =
                "ReDMCSB F0282 C160/C161 final sensor/portrait retirement";
            *outReceipt = receipt;
            return 1;
        }
        receipt.publishMirrorSensorEnabled = 0;
        receipt.clearStaleC026Portrait = 1;
        *outReceipt = receipt;
        return 1;
    }

    if (apply->writeMirrorSensorEnabled || !bridge->handoff.keepsMirrorSensorEnabled ||
        !bridge->handoff.restoreC127Portrait || !apply->drawC026Portrait ||
        apply->clearStaleC026Portrait || !presentation->sensorEnabled ||
        !presentation->drawC026Portrait || presentation->clearStaleC026Portrait ||
        !state->c127SensorEnabled || !state->c026PortraitVisible) {
        memset(&receipt, 0, sizeof(receipt));
        receipt.sourceAnchor =
            "ReDMCSB F0282 C162 matching C127/C026 final publication";
        *outReceipt = receipt;
        return 1;
    }
    receipt.publishMirrorSensorEnabled = 1;
    receipt.publishC026Portrait = 1;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateCompletion_SourceEvidencePc34(void)
{
    return "ReDMCSB REVIVE.C F0282 commits C160/C161/C162, PANEL.C F0352 "
           "clears C040, and DUNGEON.C F0172/F0174 with DUNVIEW.C:3913-3928 "
           "may restore C026 only when C162 left the matching C127 sensor live.";
}
