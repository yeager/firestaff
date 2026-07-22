#include "dm1_v1_hoc_candidate_confirmation_apply_bridge_pc34_compat.h"

#include <string.h>

int DM1_V1_HocCandidateConfirmationApplyBridge_BuildReceiptPc34(
    const DM1_V1_HocCandidateConfirmationApplyBridgeInputPc34 *input,
    DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34 receipt;
    const DM1_V1_HocCandidateConfirmationReceiptPc34 *confirmation;
    const DM1_V1_HocCandidateConfirmationApplyBridgeStatePc34 *state;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB REVIVE.C F0282:744-806; PANEL.C F0352; "
        "DUNGEON.C F0172/F0174; DUNVIEW.C:3913-3928";
    confirmation = input->confirmation;
    state = &input->state;

    if (!confirmation || !confirmation->valid || !confirmation->sourceOwned ||
        !confirmation->commitReady || confirmation->awaitsRename ||
        !confirmation->clearsCandidatePanel ||
        confirmation->nextCandidateChampionOrdinal != 0u ||
        !state->activeMirrorSourceBound || !state->c040PanelOpen ||
        !state->c127SensorEnabled ||
        state->activeMirrorOrdinal != confirmation->mirrorOrdinal ||
        state->c127SensorOrdinal != confirmation->mirrorOrdinal ||
        state->candidateChampionOrdinal != state->partyChampionCount ||
        state->partyChampionCount < confirmation->nextPartyChampionCount ||
        !state->c026PortraitAtlasAvailable || !state->c040PanelGraphicAvailable ||
        confirmation->expectedPanelGeneration != state->panelGeneration + 1u) {
        *outReceipt = receipt;
        return 1;
    }
    if (confirmation->disablesMirrorSensor) {
        if (!confirmation->requiresFirstMirrorSensorOwner ||
            !state->firstMirrorSensorOwnerAvailable ||
            confirmation->restoreC127Portrait ||
            confirmation->expectedSensorGeneration != state->sensorGeneration + 1u) {
            *outReceipt = receipt;
            return 1;
        }
    } else if (confirmation->requiresFirstMirrorSensorOwner ||
               !confirmation->restoreC127Portrait ||
               confirmation->expectedSensorGeneration != state->sensorGeneration) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.atomic = 1;
    receipt.handoff.valid = 1;
    receipt.handoff.sourceOwned = 1;
    receipt.handoff.candidateStateCleared = 1;
    receipt.handoff.c040PanelRetired = 1;
    receipt.handoff.clearStalePanelPayload = 1;
    receipt.handoff.disablesMirrorSensor = confirmation->disablesMirrorSensor;
    receipt.handoff.keepsMirrorSensorEnabled =
        confirmation->disablesMirrorSensor ? 0 : 1;
    receipt.handoff.clearStalePortrait =
        confirmation->disablesMirrorSensor ? 1 : 0;
    receipt.handoff.restoreC127Portrait = confirmation->restoreC127Portrait;
    receipt.handoff.drawC026Portrait = confirmation->restoreC127Portrait;
    receipt.handoff.reopenEligible = confirmation->restoreC127Portrait;
    receipt.handoff.mirrorOrdinal = confirmation->mirrorOrdinal;
    receipt.handoff.nextPartyChampionCount =
        confirmation->nextPartyChampionCount;
    receipt.handoff.nextCandidateChampionOrdinal = 0;

    receipt.applyState.partyChampionCount = state->partyChampionCount;
    receipt.applyState.candidateChampionOrdinal = state->candidateChampionOrdinal;
    receipt.applyState.mirrorOrdinal = state->activeMirrorOrdinal;
    receipt.applyState.mirrorSourceBound = 1;
    receipt.applyState.c040PanelOpen = 1;
    receipt.applyState.c127SensorEnabled = 1;
    receipt.applyState.c026PortraitAtlasAvailable = 1;
    receipt.applyState.presentedPanelGeneration = state->panelGeneration;
    receipt.applyState.closePanelGeneration = confirmation->expectedPanelGeneration;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateConfirmationApplyBridge_SourceEvidencePc34(void)
{
    return "ReDMCSB REVIVE.C F0282 commits C160/C161/C162 from the current "
           "G0299/G0305 candidate, PANEL.C F0352 closes C040, and C160/C161 "
           "require the first source mirror sensor while C162 leaves C127 "
           "available for C026 restoration.";
}
