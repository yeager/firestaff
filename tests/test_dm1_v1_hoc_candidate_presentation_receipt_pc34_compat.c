#include "dm1_v1_hoc_candidate_presentation_receipt_pc34_compat.h"

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

static DM1_V1_HocCandidateApplyReceiptPc34 apply_plan(int disableSensor)
{
    DM1_V1_HocCandidateApplyReceiptPc34 plan;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.sourceOwned = 1;
    plan.atomic = 1;
    plan.clearG0299CandidateOrdinal = 1;
    plan.writePartyChampionCount = 1;
    plan.nextPartyChampionCount = disableSensor ? 2 : 1;
    plan.closeC040Panel = 1;
    plan.clearStalePanelPayload = 1;
    plan.writeMirrorSensorEnabled = disableSensor;
    plan.nextMirrorSensorEnabled = 0;
    plan.clearStaleC026Portrait = disableSensor;
    plan.drawC026Portrait = !disableSensor;
    plan.mirrorOrdinal = 0;
    plan.nextPresentedPanelGeneration = 12;
    return plan;
}

int main(void)
{
    DM1_V1_HocCandidateApplyReceiptPc34 c160 = apply_plan(1);
    DM1_V1_HocCandidateApplyReceiptPc34 c162 = apply_plan(0);
    DM1_V1_HocCandidatePresentationInputPc34 input;
    DM1_V1_HocCandidatePresentationReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.state.mirrorSourceBound = 1;
    input.state.c040PanelGraphicAvailable = 1;
    input.state.c026PortraitAtlasAvailable = 1;
    input.state.presentedPanelGeneration = 12;
    input.state.previousSensorGeneration = 30;
    input.state.sensorGeneration = 31;
    input.state.partyChampionCount = 2;

    input.apply = &c160;
    ok &= expect(DM1_V1_HocCandidatePresentation_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.clearC040Panel &&
                 receipt.clearStaleC026Portrait && !receipt.drawC026Portrait &&
                 !receipt.sensorEnabled && receipt.sensorGeneration == 31,
                 "C160 post-state clears C040/C026 after sensor generation advances");

    input.apply = &c162;
    input.state.partyChampionCount = 1;
    input.state.c127SensorEnabled = 1;
    input.state.previousSensorGeneration = 31;
    input.state.sensorGeneration = 31;
    ok &= expect(DM1_V1_HocCandidatePresentation_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.clearC040Panel && receipt.drawC026Portrait &&
                 receipt.sensorEnabled && !receipt.clearStaleC026Portrait,
                 "C162 post-state restores source C127/C026 after C040 clear");

    input.state.c026PortraitAtlasAvailable = 0;
    ok &= expect(DM1_V1_HocCandidatePresentation_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "C162 post-state fails closed without C026 source bytes");

    input.state.c026PortraitAtlasAvailable = 1;
    input.state.sensorGeneration = 32;
    ok &= expect(DM1_V1_HocCandidatePresentation_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "C162 post-state rejects an unexpected sensor generation");

    if (!ok) return 1;
    puts("ok: DM1 HoC post-state presentation remains source-owned and generation-bound");
    return 0;
}
