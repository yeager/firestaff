#include "dm1_v1_hoc_candidate_confirmation_apply_bridge_pc34_compat.h"

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

static DM1_V1_HocCandidateConfirmationReceiptPc34 confirmation(int disable)
{
    DM1_V1_HocCandidateConfirmationReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.commitReady = 1;
    receipt.clearsCandidatePanel = 1;
    receipt.disablesMirrorSensor = disable;
    receipt.restoreC127Portrait = !disable;
    receipt.requiresFirstMirrorSensorOwner = disable;
    receipt.mirrorOrdinal = 0;
    receipt.nextPartyChampionCount = disable ? 2 : 1;
    receipt.expectedSensorGeneration = disable ? 51 : 50;
    receipt.expectedPanelGeneration = 23;
    return receipt;
}

int main(void)
{
    DM1_V1_HocCandidateConfirmationReceiptPc34 c160 = confirmation(1);
    DM1_V1_HocCandidateConfirmationReceiptPc34 c162 = confirmation(0);
    DM1_V1_HocCandidateConfirmationApplyBridgeInputPc34 input;
    DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.state.partyChampionCount = 2;
    input.state.candidateChampionOrdinal = 2;
    input.state.activeMirrorSourceBound = 1;
    input.state.c040PanelOpen = 1;
    input.state.c127SensorEnabled = 1;
    input.state.c026PortraitAtlasAvailable = 1;
    input.state.c040PanelGraphicAvailable = 1;
    input.state.firstMirrorSensorOwnerAvailable = 1;
    input.state.sensorGeneration = 50;
    input.state.panelGeneration = 22;

    input.confirmation = &c160;
    ok &= expect(DM1_V1_HocCandidateConfirmationApplyBridge_BuildReceiptPc34(
                     &input, &receipt) && receipt.valid && receipt.sourceOwned &&
                 receipt.atomic && receipt.handoff.disablesMirrorSensor &&
                 receipt.handoff.clearStalePortrait &&
                 receipt.applyState.closePanelGeneration == 23 &&
                 receipt.applyState.presentedPanelGeneration == 22,
                 "C160 bridge exports atomic source-owned apply input");

    input.confirmation = &c162;
    ok &= expect(DM1_V1_HocCandidateConfirmationApplyBridge_BuildReceiptPc34(
                     &input, &receipt) && receipt.valid &&
                 receipt.handoff.keepsMirrorSensorEnabled &&
                 receipt.handoff.restoreC127Portrait && receipt.handoff.drawC026Portrait &&
                 receipt.applyState.closePanelGeneration == 23,
                 "C162 bridge preserves source C127/C026 for apply");

    input.confirmation = &c160;
    input.state.firstMirrorSensorOwnerAvailable = 0;
    ok &= expect(DM1_V1_HocCandidateConfirmationApplyBridge_BuildReceiptPc34(
                     &input, &receipt) && !receipt.valid,
                 "C160 bridge rejects an unowned mirror sensor");

    input.state.firstMirrorSensorOwnerAvailable = 1;
    input.state.panelGeneration = 23;
    ok &= expect(DM1_V1_HocCandidateConfirmationApplyBridge_BuildReceiptPc34(
                     &input, &receipt) && !receipt.valid,
                 "bridge rejects stale C040 generation");

    if (!ok) return 1;
    puts("ok: DM1 HoC confirmation-to-apply bridge is source-owned and atomic");
    return 0;
}
