#include "dm1_v1_hoc_candidate_apply_receipt_pc34_compat.h"

#include <string.h>

int DM1_V1_HocCandidateApply_BuildReceiptPc34(
    const DM1_V1_HocCandidateApplyInputPc34 *input,
    DM1_V1_HocCandidateApplyReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateApplyReceiptPc34 receipt;
    const DM1_V1_HocCandidateHandoffReceiptPc34 *handoff;
    const DM1_V1_HocCandidateApplyStatePc34 *state;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB REVIVE.C F0282:744-806; PANEL.C F0352; "
        "DUNGEON.C F0172/F0174; DUNVIEW.C:3913-3928";
    handoff = input->handoff;
    state = &input->state;

    if (!handoff || !handoff->valid || !handoff->sourceOwned ||
        !handoff->candidateStateCleared || !handoff->c040PanelRetired ||
        !handoff->clearStalePanelPayload ||
        handoff->nextCandidateChampionOrdinal != 0u ||
        !state->mirrorSourceBound || state->mirrorOrdinal != handoff->mirrorOrdinal ||
        state->candidateChampionOrdinal == 0u ||
        state->candidateChampionOrdinal != state->partyChampionCount ||
        !state->c040PanelOpen || state->closePanelGeneration == 0u ||
        state->presentedPanelGeneration >= state->closePanelGeneration ||
        state->partyChampionCount < handoff->nextPartyChampionCount) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.atomic = 1;
    receipt.clearG0299CandidateOrdinal = 1;
    receipt.writePartyChampionCount = 1;
    receipt.nextPartyChampionCount = handoff->nextPartyChampionCount;
    receipt.closeC040Panel = 1;
    receipt.clearStalePanelPayload = 1;
    receipt.mirrorOrdinal = handoff->mirrorOrdinal;
    receipt.nextPresentedPanelGeneration = state->closePanelGeneration;

    if (handoff->disablesMirrorSensor) {
        if (!state->c127SensorEnabled || handoff->keepsMirrorSensorEnabled ||
            handoff->restoreC127Portrait || handoff->drawC026Portrait) {
            memset(&receipt, 0, sizeof(receipt));
            receipt.sourceAnchor =
                "ReDMCSB REVIVE.C F0282 C160/C161 atomic sensor retirement";
            *outReceipt = receipt;
            return 1;
        }
        receipt.writeMirrorSensorEnabled = 1;
        receipt.nextMirrorSensorEnabled = 0;
        receipt.clearStaleC026Portrait = 1;
        *outReceipt = receipt;
        return 1;
    }

    if (!handoff->keepsMirrorSensorEnabled || !handoff->restoreC127Portrait ||
        !handoff->drawC026Portrait || !handoff->reopenEligible ||
        !state->c127SensorEnabled || !state->c026PortraitAtlasAvailable) {
        memset(&receipt, 0, sizeof(receipt));
        receipt.sourceAnchor =
            "ReDMCSB REVIVE.C F0282 C162 live C127/C026 restore";
        *outReceipt = receipt;
        return 1;
    }
    /* REVIVE.C F0282 C162 keeps the current C127 sensor live so the exact
     * C026 portrait can be drawn again and selected again.  The apply
     * receipt is also consumed by M11's panel-close gate; leaving this
     * field at its zero-initialized value makes a successful cancel look
     * like a sensor retirement and traps later HoC mirror clicks behind
     * the stale C040 panel. */
    receipt.nextMirrorSensorEnabled = 1;
    receipt.drawC026Portrait = 1;
    return (*outReceipt = receipt), 1;
}

const char *DM1_V1_HocCandidateApply_SourceEvidencePc34(void)
{
    return "ReDMCSB REVIVE.C F0282 atomically clears G0299 and either "
           "decrements G0305 for C162 or disables the first mirror sensor "
           "for C160/C161; PANEL.C F0352 clears C040; DUNGEON.C F0172/F0174 "
           "and DUNVIEW.C:3913-3928 redraw C026 only for a still-enabled "
           "matching C127 sensor.";
}
