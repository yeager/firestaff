#include "dm1_v1_hoc_candidate_handoff_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int require(int value, const char *label)
{
    if (!value) {
        fprintf(stderr, "FAIL: %s\n", label);
        return 0;
    }
    return 1;
}

static HocMirrorCandidateRuntimeReceipt_Compat make_transition(
    int disablesSensor,
    int restorePortrait,
    int awaitsRename)
{
    HocMirrorCandidateRuntimeReceipt_Compat receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.clearsCandidatePanel = awaitsRename ? 0 : 1;
    receipt.awaitsRename = awaitsRename;
    receipt.disablesMirrorSensor = disablesSensor;
    receipt.restoreC127MirrorPortrait = restorePortrait;
    receipt.mirrorOrdinal = 0;
    receipt.nextPartyChampionCount = restorePortrait ? 1 : 2;
    return receipt;
}

static DM1_V1_HocCandidatePanelReceiptPc34 make_close(int disablesSensor,
                                                       int restorePortrait)
{
    DM1_V1_HocCandidatePanelReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.clearC040Panel = 1;
    receipt.clearStalePanelPayload = 1;
    receipt.disablesSensor = disablesSensor;
    receipt.keepsSensorEnabled = !disablesSensor;
    receipt.restoreC127Portrait = restorePortrait;
    receipt.redrawC026Portrait = restorePortrait;
    receipt.mirrorOrdinal = 0;
    return receipt;
}

int main(void)
{
    HocMirrorCandidateRuntimeReceipt_Compat c160 = make_transition(1, 0, 0);
    HocMirrorCandidateRuntimeReceipt_Compat c162 = make_transition(0, 1, 0);
    HocMirrorCandidateRuntimeReceipt_Compat c161Wait = make_transition(0, 0, 1);
    DM1_V1_HocCandidatePanelReceiptPc34 closeDisable = make_close(1, 0);
    DM1_V1_HocCandidatePanelReceiptPc34 closeCancel = make_close(0, 1);
    DM1_V1_HocCandidateHandoffInputPc34 input;
    DM1_V1_HocCandidateHandoffReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.activeMirrorSourceBound = 1;
    input.activeMirrorOrdinal = 0;
    input.c127SensorOrdinal = 0;
    input.c127SensorEnabled = 1;
    input.c026PortraitAtlasAvailable = 1;

    input.transition = &c160;
    input.panelClose = &closeDisable;
    ok &= require(DM1_V1_HocCandidateHandoff_BuildReceiptPc34(&input, &receipt) &&
                  receipt.valid && receipt.candidateStateCleared &&
                  receipt.c040PanelRetired && receipt.disablesMirrorSensor &&
                  receipt.clearStalePortrait && !receipt.restoreC127Portrait,
                  "C160 closes C040 and retires C127 portrait ownership");

    input.transition = &c162;
    input.panelClose = &closeCancel;
    ok &= require(DM1_V1_HocCandidateHandoff_BuildReceiptPc34(&input, &receipt) &&
                  receipt.valid && receipt.keepsMirrorSensorEnabled &&
                  receipt.restoreC127Portrait && receipt.drawC026Portrait &&
                  receipt.reopenEligible && !receipt.disablesMirrorSensor,
                  "C162 restores only the matching live C127 portrait");

    input.c127SensorEnabled = 0;
    ok &= require(DM1_V1_HocCandidateHandoff_BuildReceiptPc34(&input, &receipt) &&
                  !receipt.valid,
                  "C162 refuses portrait restore after sensor retirement");

    input.c127SensorEnabled = 1;
    input.transition = &c161Wait;
    input.panelClose = &closeDisable;
    ok &= require(DM1_V1_HocCandidateHandoff_BuildReceiptPc34(&input, &receipt) &&
                  !receipt.valid,
                  "C161 rename wait cannot leak a premature close handoff");

    if (!ok) return 1;
    puts("ok: DM1 HoC C160/C161/C162 handoff keeps C127 portrait ownership source-bound");
    return 0;
}
