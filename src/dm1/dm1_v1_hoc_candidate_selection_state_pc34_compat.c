#include "dm1_v1_hoc_candidate_selection_state_pc34_compat.h"

#include <string.h>

int DM1_V1_HocCandidateSelectionState_BuildReceiptPc34(
    const DM1_V1_HocCandidateSelectionStateInputPc34 *input,
    DM1_V1_HocCandidateSelectionStateReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateSelectionStateReceiptPc34 receipt;
    const DM1_V1_HocCandidateClickPresentationReceiptPc34 *presentation;
    const DM1_V1_HocCandidateSelectionStatePc34 *state;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB MOVESENS.C C127/F0280; REVIVE.C F0280:124-132/272-276; "
        "PANEL.C F0344-F0347; DUNVIEW.C:3913-3928";
    presentation = input->presentation;
    state = &input->state;

    if (!presentation || !presentation->valid || !presentation->sourceOwned ||
        !presentation->portraitHit || !presentation->consumedC127Sensor ||
        !presentation->opensCandidatePanel || !presentation->drawC026Portrait ||
        !presentation->drawC040Panel ||
        state->candidateChampionOrdinal != 0u || state->c040PanelOpen ||
        state->activeMirrorSourceBound || !state->c127SensorEnabled ||
        state->c127SensorOrdinal != presentation->mirrorOrdinal ||
        !state->c026PortraitAtlasAvailable || !state->c040PanelGraphicAvailable ||
        state->panelGeneration == UINT32_MAX ||
        presentation->panelGeneration != state->panelGeneration + 1u ||
        presentation->candidateChampionOrdinal != state->partyChampionCount + 1u ||
        presentation->candidateChampionIndex != state->partyChampionCount) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.atomic = 1;
    receipt.publishCandidateChampionOrdinal = 1;
    receipt.nextCandidateChampionOrdinal = presentation->candidateChampionOrdinal;
    receipt.publishPartyChampionCount = 1;
    receipt.nextPartyChampionCount = state->partyChampionCount + 1u;
    receipt.publishActiveMirror = 1;
    receipt.nextActiveMirrorOrdinal = presentation->mirrorOrdinal;
    receipt.nextActiveMirrorSourceBound = 1;
    receipt.openC040Panel = 1;
    receipt.drawC026Portrait = 1;
    receipt.drawC040Panel = 1;
    receipt.keepsC127SensorEnabled = 1;
    receipt.sensorGeneration = state->sensorGeneration;
    receipt.panelGeneration = presentation->panelGeneration;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateSelectionState_SourceEvidencePc34(void)
{
    return "ReDMCSB MOVESENS.C invokes F0280 for matching C127; REVIVE.C "
           "F0280 appends G0305 and publishes G0299; PANEL.C draws C040; "
           "DUNVIEW.C draws C026.  The C127 sensor remains enabled until "
           "REVIVE.C F0282 receives C160 or C161.";
}
