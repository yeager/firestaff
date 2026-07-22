#include "dm1_v1_hoc_candidate_apply_receipt_pc34_compat.h"

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

static DM1_V1_HocCandidateHandoffReceiptPc34 handoff(int disables)
{
    DM1_V1_HocCandidateHandoffReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.candidateStateCleared = 1;
    receipt.c040PanelRetired = 1;
    receipt.clearStalePanelPayload = 1;
    receipt.disablesMirrorSensor = disables;
    receipt.keepsMirrorSensorEnabled = !disables;
    receipt.clearStalePortrait = disables;
    receipt.restoreC127Portrait = !disables;
    receipt.drawC026Portrait = !disables;
    receipt.reopenEligible = !disables;
    receipt.mirrorOrdinal = 0;
    receipt.nextPartyChampionCount = disables ? 2 : 1;
    return receipt;
}

int main(void)
{
    DM1_V1_HocCandidateHandoffReceiptPc34 c160 = handoff(1);
    DM1_V1_HocCandidateHandoffReceiptPc34 c162 = handoff(0);
    DM1_V1_HocCandidateApplyInputPc34 input;
    DM1_V1_HocCandidateApplyReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.state.partyChampionCount = 2;
    input.state.candidateChampionOrdinal = 2;
    input.state.mirrorSourceBound = 1;
    input.state.c040PanelOpen = 1;
    input.state.c127SensorEnabled = 1;
    input.state.c026PortraitAtlasAvailable = 1;
    input.state.closePanelGeneration = 9;
    input.state.presentedPanelGeneration = 8;

    input.handoff = &c160;
    ok &= expect(DM1_V1_HocCandidateApply_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.atomic && receipt.clearG0299CandidateOrdinal &&
                 receipt.writePartyChampionCount && receipt.nextPartyChampionCount == 2 &&
                 receipt.closeC040Panel && receipt.writeMirrorSensorEnabled &&
                 !receipt.nextMirrorSensorEnabled && receipt.clearStaleC026Portrait &&
                 !receipt.drawC026Portrait && receipt.nextPresentedPanelGeneration == 9,
                 "C160 emits one atomic source-owned retirement plan");

    input.handoff = &c162;
    ok &= expect(DM1_V1_HocCandidateApply_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.atomic && receipt.nextPartyChampionCount == 1 &&
                 !receipt.writeMirrorSensorEnabled && receipt.drawC026Portrait &&
                 !receipt.clearStaleC026Portrait,
                 "C162 emits one atomic source-owned C127/C026 restore plan");

    input.state.c026PortraitAtlasAvailable = 0;
    ok &= expect(DM1_V1_HocCandidateApply_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "C162 fails closed without the real C026 atlas");

    input.state.c026PortraitAtlasAvailable = 1;
    input.state.presentedPanelGeneration = 9;
    ok &= expect(DM1_V1_HocCandidateApply_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "stale generation cannot apply a C040 close plan");

    if (!ok) return 1;
    puts("ok: DM1 HoC final handoff has an atomic source-owned mutation plan");
    return 0;
}
