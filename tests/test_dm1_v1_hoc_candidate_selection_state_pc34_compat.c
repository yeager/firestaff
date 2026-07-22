#include "dm1_v1_hoc_candidate_selection_state_pc34_compat.h"

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

static DM1_V1_HocCandidateClickPresentationReceiptPc34 presentation(void)
{
    DM1_V1_HocCandidateClickPresentationReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.portraitHit = 1;
    receipt.consumedC127Sensor = 1;
    receipt.opensCandidatePanel = 1;
    receipt.drawC026Portrait = 1;
    receipt.drawC040Panel = 1;
    receipt.mirrorOrdinal = 0;
    receipt.candidateChampionIndex = 1;
    receipt.candidateChampionOrdinal = 2;
    receipt.panelGeneration = 15;
    return receipt;
}

int main(void)
{
    DM1_V1_HocCandidateClickPresentationReceiptPc34 click = presentation();
    DM1_V1_HocCandidateSelectionStateInputPc34 input;
    DM1_V1_HocCandidateSelectionStateReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.presentation = &click;
    input.state.partyChampionCount = 1;
    input.state.c127SensorEnabled = 1;
    input.state.c026PortraitAtlasAvailable = 1;
    input.state.c040PanelGraphicAvailable = 1;
    input.state.sensorGeneration = 42;
    input.state.panelGeneration = 14;

    ok &= expect(DM1_V1_HocCandidateSelectionState_BuildReceiptPc34(
                     &input, &receipt) && receipt.valid && receipt.sourceOwned &&
                 receipt.atomic && receipt.nextCandidateChampionOrdinal == 2 &&
                 receipt.nextPartyChampionCount == 2 &&
                 receipt.nextActiveMirrorOrdinal == 0 &&
                 receipt.nextActiveMirrorSourceBound && receipt.openC040Panel &&
                 receipt.drawC026Portrait && receipt.drawC040Panel &&
                 receipt.keepsC127SensorEnabled && receipt.sensorGeneration == 42 &&
                 receipt.panelGeneration == 15,
                 "selection publishes ordinal-zero C127 candidate state atomically");

    input.state.candidateChampionOrdinal = 2;
    ok &= expect(DM1_V1_HocCandidateSelectionState_BuildReceiptPc34(
                     &input, &receipt) && !receipt.valid,
                 "an active candidate panel blocks a second C127 selection");

    input.state.candidateChampionOrdinal = 0;
    input.state.c127SensorEnabled = 0;
    ok &= expect(DM1_V1_HocCandidateSelectionState_BuildReceiptPc34(
                     &input, &receipt) && !receipt.valid,
                 "disabled C127 sensor cannot publish a candidate");

    input.state.c127SensorEnabled = 1;
    input.state.panelGeneration = 15;
    ok &= expect(DM1_V1_HocCandidateSelectionState_BuildReceiptPc34(
                     &input, &receipt) && !receipt.valid,
                 "wrong panel generation cannot publish a candidate");

    if (!ok) return 1;
    puts("ok: DM1 HoC candidate selection state is source-owned and generation-bound");
    return 0;
}
