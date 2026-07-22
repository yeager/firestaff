#include "dm1_v1_hoc_candidate_apply_presentation_completion_pc34_compat.h"

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

static DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34 bridge(int disable)
{
    DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.atomic = 1;
    receipt.handoff.valid = 1;
    receipt.handoff.sourceOwned = 1;
    receipt.handoff.candidateStateCleared = 1;
    receipt.handoff.c040PanelRetired = 1;
    receipt.handoff.clearStalePanelPayload = 1;
    receipt.handoff.disablesMirrorSensor = disable;
    receipt.handoff.keepsMirrorSensorEnabled = !disable;
    receipt.handoff.restoreC127Portrait = !disable;
    receipt.handoff.drawC026Portrait = !disable;
    receipt.handoff.mirrorOrdinal = 0;
    receipt.handoff.nextPartyChampionCount = disable ? 2 : 1;
    return receipt;
}

static DM1_V1_HocCandidateApplyReceiptPc34 apply(int disable)
{
    DM1_V1_HocCandidateApplyReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.atomic = 1;
    receipt.clearG0299CandidateOrdinal = 1;
    receipt.writePartyChampionCount = 1;
    receipt.nextPartyChampionCount = disable ? 2 : 1;
    receipt.closeC040Panel = 1;
    receipt.clearStalePanelPayload = 1;
    receipt.writeMirrorSensorEnabled = disable;
    receipt.nextMirrorSensorEnabled = 0;
    receipt.clearStaleC026Portrait = disable;
    receipt.drawC026Portrait = !disable;
    receipt.mirrorOrdinal = 0;
    receipt.nextPresentedPanelGeneration = 7;
    return receipt;
}

static DM1_V1_HocCandidatePresentationReceiptPc34 presentation(int disable)
{
    DM1_V1_HocCandidatePresentationReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.clearC040Panel = 1;
    receipt.clearStalePanelPayload = 1;
    receipt.clearStaleC026Portrait = disable;
    receipt.drawC026Portrait = !disable;
    receipt.sensorEnabled = !disable;
    receipt.mirrorOrdinal = 0;
    receipt.sensorGeneration = disable ? 18 : 17;
    receipt.presentedPanelGeneration = 7;
    return receipt;
}

int main(void)
{
    DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34 c160Bridge = bridge(1);
    DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34 c162Bridge = bridge(0);
    DM1_V1_HocCandidateApplyReceiptPc34 c160Apply = apply(1);
    DM1_V1_HocCandidateApplyReceiptPc34 c162Apply = apply(0);
    DM1_V1_HocCandidatePresentationReceiptPc34 c160Presentation = presentation(1);
    DM1_V1_HocCandidatePresentationReceiptPc34 c162Presentation = presentation(0);
    DM1_V1_HocCandidateCompletionInputPc34 input;
    DM1_V1_HocCandidateCompletionReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.state.mirrorSourceBound = 1;
    input.state.presentedPanelGeneration = 7;

    input.bridge = &c160Bridge;
    input.apply = &c160Apply;
    input.presentation = &c160Presentation;
    input.state.partyChampionCount = 2;
    input.state.sensorGeneration = 18;
    ok &= expect(DM1_V1_HocCandidateCompletion_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.atomic && receipt.publishC040Cleared &&
                 receipt.publishCandidateCleared && !receipt.publishMirrorSensorEnabled &&
                 receipt.clearStaleC026Portrait && !receipt.publishC026Portrait,
                 "C160 completion publishes only final sensor and portrait retirement");

    input.bridge = &c162Bridge;
    input.apply = &c162Apply;
    input.presentation = &c162Presentation;
    input.state.partyChampionCount = 1;
    input.state.c127SensorEnabled = 1;
    input.state.c026PortraitVisible = 1;
    input.state.sensorGeneration = 17;
    ok &= expect(DM1_V1_HocCandidateCompletion_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.publishMirrorSensorEnabled &&
                 receipt.publishC026Portrait && !receipt.clearStaleC026Portrait,
                 "C162 completion publishes only matching C127/C026 restore");

    input.state.c026PortraitVisible = 0;
    ok &= expect(DM1_V1_HocCandidateCompletion_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "C162 completion rejects a missing final source portrait");

    if (!ok) return 1;
    puts("ok: DM1 HoC apply-to-presentation completion is source-owned");
    return 0;
}
