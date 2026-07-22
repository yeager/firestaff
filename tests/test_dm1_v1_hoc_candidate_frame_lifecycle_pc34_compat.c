#include "dm1_v1_hoc_candidate_frame_lifecycle_pc34_compat.h"

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

static DM1_V1_HocCandidateFrameCommandReceiptPc34 frame(
    DM1_V1_HocCandidateRenderCommandKindPc34 c026Kind)
{
    DM1_V1_HocCandidateFrameCommandReceiptPc34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1; receipt.sourceOwned = 1; receipt.commandCount = 2;
    receipt.commands[0].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34;
    receipt.commands[0].material.sourceOwned = 1;
    receipt.commands[0].material.graphicIndex = 40;
    receipt.commands[0].material.sourceHash = 0x4040u;
    receipt.commands[1].kind = c026Kind;
    receipt.commands[1].material.sourceOwned = 1;
    receipt.commands[1].material.graphicIndex = 26;
    receipt.commands[1].material.sourceHash = 0x2626u;
    receipt.mirrorOrdinal = 3; receipt.sensorGeneration = 17;
    receipt.presentedPanelGeneration = 29;
    return receipt;
}

int main(void)
{
    DM1_V1_HocCandidateFrameCommandReceiptPc34 clear =
        frame(DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34);
    DM1_V1_HocCandidateFrameCommandReceiptPc34 portrait =
        frame(DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34);
    DM1_V1_HocCandidateFrameLifecycleInputPc34 input;
    DM1_V1_HocCandidateFrameLifecycleReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.priorClearFrame = &clear; input.nextPortraitFrame = &portrait;
    input.priorClearTick = 101; input.nextPortraitTick = 102;
    ok &= expect(DM1_V1_HocCandidateFrameLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.clearAccepted && receipt.portraitAdmitted &&
                 receipt.clearTick == 101 && receipt.portraitTick == 102 &&
                 receipt.c040SourceHash == 0x4040u && receipt.c026SourceHash == 0x2626u,
                 "later portrait command follows matching source clear");

    input.nextPortraitTick = 101;
    ok &= expect(DM1_V1_HocCandidateFrameLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "same tick cannot consume prior clear");

    input.nextPortraitTick = 102;
    portrait.sensorGeneration++;
    ok &= expect(DM1_V1_HocCandidateFrameLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "stale sensor generation cannot restore portrait");

    if (!ok) return 1;
    puts("ok: DM1 HoC frame lifecycle fences stale portrait restore");
    return 0;
}
