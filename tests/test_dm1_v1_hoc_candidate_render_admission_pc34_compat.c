#include "dm1_v1_hoc_candidate_render_admission_pc34_compat.h"

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

static DM1_V1_HocCandidateCompletionReceiptPc34 completion(int clearPortrait)
{
    DM1_V1_HocCandidateCompletionReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.atomic = 1;
    receipt.publishC040Cleared = 1;
    receipt.publishCandidateCleared = 1;
    receipt.publishMirrorSensorEnabled = !clearPortrait;
    receipt.publishC026Portrait = !clearPortrait;
    receipt.clearStaleC026Portrait = clearPortrait;
    receipt.mirrorOrdinal = 0;
    receipt.sensorGeneration = clearPortrait ? 9 : 8;
    receipt.presentedPanelGeneration = 17;
    return receipt;
}

int main(void)
{
    DM1_V1_HocCandidateCompletionReceiptPc34 c160 = completion(1);
    DM1_V1_HocCandidateCompletionReceiptPc34 c162 = completion(0);
    DM1_V1_HocCandidateRenderAdmissionInputPc34 input;
    DM1_V1_HocCandidateRenderAdmissionReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.mirrorSourceBound = 1;
    input.c040PanelGraphicAvailable = 1;
    input.presentedPanelGeneration = 17;

    input.completion = &c160;
    input.sensorGeneration = 9;
    ok &= expect(DM1_V1_HocCandidateRenderAdmission_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.commandCount == 2 &&
                 receipt.commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
                 receipt.commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34,
                 "C160 admits only source C040/C026 clear commands");

    input.completion = &c162;
    input.sensorGeneration = 8;
    input.c127SensorEnabled = 1;
    input.c026PortraitVisible = 1;
    input.c026PortraitAtlasAvailable = 1;
    ok &= expect(DM1_V1_HocCandidateRenderAdmission_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.commandCount == 2 &&
                 receipt.commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
                 receipt.commands[1].left == 96 && receipt.commands[1].bottom == 63,
                 "C162 admits only the matching source C026 portrait command");

    input.c026PortraitAtlasAvailable = 0;
    ok &= expect(DM1_V1_HocCandidateRenderAdmission_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "C162 draw admission fails closed without the C026 atlas");

    if (!ok) return 1;
    puts("ok: DM1 HoC final render commands are source-owned");
    return 0;
}
