#include "dm1_v1_hoc_candidate_confirmation_pc34_compat.h"

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

static DM1_V1_HocCandidateSelectionStateReceiptPc34 selection(void)
{
    DM1_V1_HocCandidateSelectionStateReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.atomic = 1;
    receipt.publishCandidateChampionOrdinal = 1;
    receipt.nextCandidateChampionOrdinal = 2;
    receipt.publishPartyChampionCount = 1;
    receipt.nextPartyChampionCount = 2;
    receipt.publishActiveMirror = 1;
    receipt.nextActiveMirrorOrdinal = 0;
    receipt.nextActiveMirrorSourceBound = 1;
    receipt.openC040Panel = 1;
    receipt.keepsC127SensorEnabled = 1;
    receipt.sensorGeneration = 40;
    receipt.panelGeneration = 14;
    return receipt;
}

int main(void)
{
    DM1_V1_HocCandidateSelectionStateReceiptPc34 selected = selection();
    DM1_V1_HocCandidateConfirmationInputPc34 input;
    DM1_V1_HocCandidateConfirmationReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.selection = &selected;
    input.state.partyChampionCount = 2;
    input.state.candidateChampionOrdinal = 2;
    input.state.activeMirrorSourceBound = 1;
    input.state.c040PanelOpen = 1;
    input.state.c127SensorEnabled = 1;
    input.state.c026PortraitAtlasAvailable = 1;
    input.state.c040PanelGraphicAvailable = 1;
    input.state.sensorGeneration = 40;
    input.state.panelGeneration = 14;

    input.command = DM1_COMMAND_RESURRECT;
    ok &= expect(DM1_V1_HocCandidateConfirmation_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.commitReady && receipt.clearsCandidatePanel &&
                 receipt.disablesMirrorSensor && receipt.requiresFirstMirrorSensorOwner &&
                 !receipt.restoreC127Portrait && receipt.mirrorOrdinal == 0 &&
                 receipt.expectedSensorGeneration == 41 && receipt.expectedPanelGeneration == 15,
                 "C160 confirmation binds selection state before sensor retirement");

    input.command = DM1_COMMAND_REINCARNATE;
    input.renameAccepted = 0;
    ok &= expect(DM1_V1_HocCandidateConfirmation_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.awaitsRename && !receipt.commitReady &&
                 receipt.expectedSensorGeneration == 40 && receipt.expectedPanelGeneration == 14,
                 "C161 rename wait preserves selection generations");

    input.command = DM1_COMMAND_CANCEL;
    input.renameAccepted = 0;
    ok &= expect(DM1_V1_HocCandidateConfirmation_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.commitReady && !receipt.disablesMirrorSensor &&
                 receipt.restoreC127Portrait && receipt.nextPartyChampionCount == 1 &&
                 receipt.expectedSensorGeneration == 40 && receipt.expectedPanelGeneration == 15,
                 "C162 confirmation retains the matching C127 owner");

    input.state.panelGeneration = 15;
    ok &= expect(DM1_V1_HocCandidateConfirmation_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "confirmation rejects wrong C040 generation");

    if (!ok) return 1;
    puts("ok: DM1 HoC confirmation is source-owned before atomic apply");
    return 0;
}
