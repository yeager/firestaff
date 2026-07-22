#include "dm1_v1_hoc_candidate_confirmation_pc34_compat.h"

#include <string.h>

int DM1_V1_HocCandidateConfirmation_BuildReceiptPc34(
    const DM1_V1_HocCandidateConfirmationInputPc34 *input,
    DM1_V1_HocCandidateConfirmationReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateConfirmationReceiptPc34 receipt;
    const DM1_V1_HocCandidateSelectionStateReceiptPc34 *selection;
    const DM1_V1_HocCandidateConfirmationStatePc34 *state;
    CandidatePanelState_Compat panelState;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB REVIVE.C F0280:272-276 and F0282:744-806; "
        "PANEL.C F0344-F0347; DUNGEON.C F0172/F0174";
    selection = input->selection;
    state = &input->state;

    if (!selection || !selection->valid || !selection->sourceOwned ||
        !selection->atomic || !selection->publishCandidateChampionOrdinal ||
        !selection->publishPartyChampionCount || !selection->publishActiveMirror ||
        !selection->openC040Panel || !selection->keepsC127SensorEnabled ||
        state->partyChampionCount != selection->nextPartyChampionCount ||
        state->candidateChampionOrdinal != selection->nextCandidateChampionOrdinal ||
        state->activeMirrorOrdinal != selection->nextActiveMirrorOrdinal ||
        !state->activeMirrorSourceBound || !state->c040PanelOpen ||
        !state->c127SensorEnabled ||
        state->c127SensorOrdinal != selection->nextActiveMirrorOrdinal ||
        !state->c026PortraitAtlasAvailable || !state->c040PanelGraphicAvailable ||
        state->sensorGeneration != selection->sensorGeneration ||
        state->panelGeneration != selection->panelGeneration ||
        (input->command != DM1_COMMAND_RESURRECT &&
         input->command != DM1_COMMAND_REINCARNATE &&
         input->command != DM1_COMMAND_CANCEL)) {
        *outReceipt = receipt;
        return 1;
    }

    panelState.partyChampionCount = state->partyChampionCount;
    panelState.candidateChampionOrdinal = state->candidateChampionOrdinal;
    receipt.finalization =
        F0872_RESURRECTION_BuildHocMirrorCandidateFinalizeReceipt_Compat(
            panelState, input->command, input->renameAccepted);
    receipt.mirrorOrdinal = state->activeMirrorOrdinal;

    if (receipt.finalization.renameRequiredBeforeFinalize) {
        receipt.valid = 1;
        receipt.sourceOwned = 1;
        receipt.awaitsRename = 1;
        receipt.nextPartyChampionCount = state->partyChampionCount;
        receipt.nextCandidateChampionOrdinal = state->candidateChampionOrdinal;
        receipt.expectedSensorGeneration = state->sensorGeneration;
        receipt.expectedPanelGeneration = state->panelGeneration;
        *outReceipt = receipt;
        return 1;
    }
    if (!receipt.finalization.valid) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.commitReady = 1;
    receipt.clearsCandidatePanel = 1;
    receipt.disablesMirrorSensor = receipt.finalization.disablesMirrorSensor;
    receipt.restoreC127Portrait = receipt.finalization.cancelled;
    receipt.requiresFirstMirrorSensorOwner =
        receipt.finalization.requiresFirstMirrorSensorOwner;
    receipt.nextPartyChampionCount = receipt.finalization.nextPartyChampionCount;
    receipt.nextCandidateChampionOrdinal = 0;
    receipt.expectedSensorGeneration = state->sensorGeneration +
        (receipt.disablesMirrorSensor ? 1u : 0u);
    receipt.expectedPanelGeneration = state->panelGeneration + 1u;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateConfirmation_SourceEvidencePc34(void)
{
    return "ReDMCSB REVIVE.C F0280 publishes the appended G0305/G0299 state; "
           "F0282 C160/C161/C162 consumes exactly that last candidate.  C161 "
           "must complete F0281 rename before committing; C160/C161 disable the "
           "mirror sensor while C162 leaves C127 available for C026 redraw.";
}
