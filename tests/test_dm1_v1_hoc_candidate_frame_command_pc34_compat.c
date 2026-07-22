#include "dm1_v1_hoc_candidate_frame_command_pc34_compat.h"

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

static DM1_V1_HocCandidateCompletionReceiptPc34 completion(int clear)
{
    DM1_V1_HocCandidateCompletionReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1; receipt.sourceOwned = 1; receipt.atomic = 1;
    receipt.clearStaleC026Portrait = clear;
    receipt.publishC026Portrait = !clear;
    receipt.mirrorOrdinal = 0; receipt.sensorGeneration = 4;
    receipt.presentedPanelGeneration = 8;
    return receipt;
}

static DM1_V1_HocCandidateRenderAdmissionReceiptPc34 admission(int clear)
{
    DM1_V1_HocCandidateRenderAdmissionReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1; receipt.sourceOwned = 1; receipt.commandCount = 2;
    receipt.commands[0].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34;
    receipt.commands[1].kind = clear ? DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34
                                     : DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34;
    receipt.mirrorOrdinal = 0; receipt.sensorGeneration = 4;
    receipt.presentedPanelGeneration = 8;
    return receipt;
}

int main(void)
{
    DM1_V1_HocCandidateCompletionReceiptPc34 clear = completion(1);
    DM1_V1_HocCandidateCompletionReceiptPc34 draw = completion(0);
    DM1_V1_HocCandidateRenderAdmissionReceiptPc34 clearAdmission = admission(1);
    DM1_V1_HocCandidateRenderAdmissionReceiptPc34 drawAdmission = admission(0);
    DM1_V1_HocCandidateFrameCommandInputPc34 input;
    DM1_V1_HocCandidateFrameCommandReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.c040.sourceOwned = 1; input.c040.graphicIndex = 40;
    input.c040.width = 144; input.c040.height = 73; input.c040.dstLeft = 80;
    input.c040.dstTop = 20; input.c040.sourceHash = 0x4040u;
    input.c026.sourceOwned = 1; input.c026.graphicIndex = 26;
    input.c026.width = 32; input.c026.height = 29; input.c026.dstLeft = 96;
    input.c026.dstTop = 35; input.c026.sourceHash = 0x2626u;

    input.completion = &clear; input.admission = &clearAdmission;
    ok &= expect(DM1_V1_HocCandidateFrameCommand_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.commandCount == 2 &&
                 receipt.commands[0].material.width == 144 &&
                 receipt.commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34 &&
                 receipt.commands[1].material.dstLeft == 96,
                 "clear frame preserves source C040/C026 material rectangles");

    input.completion = &draw; input.admission = &drawAdmission;
    ok &= expect(DM1_V1_HocCandidateFrameCommand_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
                 receipt.commands[1].material.sourceHash == 0x2626u,
                 "restore frame preserves verified C026 source material");

    drawAdmission.presentedPanelGeneration = 9;
    ok &= expect(DM1_V1_HocCandidateFrameCommand_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "stale completion/admission cannot emit frame commands");

    if (!ok) return 1;
    puts("ok: DM1 HoC frame command receipt preserves source C026/C040 material");
    return 0;
}
