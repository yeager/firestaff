#include "dm1_v1_hoc_candidate_runtime_render_admission_pc34_compat.h"

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

static DM1_V1_HocCandidateFrameCommandReceiptPc34 clear_frame(void)
{
    DM1_V1_HocCandidateFrameCommandReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.commandCount = 2;
    result.mirrorOrdinal = 3; result.sensorGeneration = 17;
    result.presentedPanelGeneration = 29;
    result.commands[0].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34;
    result.commands[0].material.sourceOwned = 1;
    result.commands[0].material.graphicIndex = 40;
    result.commands[0].material.sourceHash = 0x4040u;
    result.commands[1].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34;
    return result;
}

static DM1_V1_HocCandidateLifecycleFrameBridgeReceiptPc34 bridge(void)
{
    DM1_V1_HocCandidateLifecycleFrameBridgeReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.publishC026Portrait = 1;
    result.mirrorOrdinal = 3; result.sensorGeneration = 17;
    result.presentedPanelGeneration = 29;
    result.clearTick = 101; result.portraitTick = 102;
    result.c040SourceHash = 0x4040u;
    result.portraitCommand.kind = DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34;
    result.portraitCommand.material.sourceOwned = 1;
    result.portraitCommand.material.graphicIndex = 26;
    result.portraitCommand.material.sourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocCandidateFrameCommandReceiptPc34 clear = clear_frame();
    DM1_V1_HocCandidateLifecycleFrameBridgeReceiptPc34 sourceBridge = bridge();
    DM1_V1_HocCandidateRuntimeRenderAdmissionInputPc34 input;
    DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.bridge = &sourceBridge; input.priorClearFrame = &clear;
    ok &= expect(DM1_V1_HocCandidateRuntimeRenderAdmission_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.admitHostRoute && receipt.commandCount == 2 &&
                 receipt.commands[0].material.graphicIndex == 40 &&
                 receipt.commands[1].material.graphicIndex == 26 &&
                 receipt.renderTick == 102,
                 "matching original C040/C026 enter the host route");

    clear.sensorGeneration++;
    ok &= expect(DM1_V1_HocCandidateRuntimeRenderAdmission_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "stale clear generation cannot enter the host route");

    if (!ok) return 1;
    puts("ok: DM1 HoC runtime admission requires original C040/C026 route");
    return 0;
}
