#include "dm1_v1_hoc_candidate_lifecycle_frame_bridge_pc34_compat.h"

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
    DM1_V1_HocCandidateFrameCommandReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.commandCount = 2;
    result.mirrorOrdinal = 3; result.sensorGeneration = 17;
    result.presentedPanelGeneration = 29;
    result.commands[0].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34;
    result.commands[0].material.sourceOwned = 1;
    result.commands[0].material.graphicIndex = 40;
    result.commands[0].material.sourceHash = 0x4040u;
    result.commands[1].kind = c026Kind;
    result.commands[1].material.sourceOwned = 1;
    result.commands[1].material.graphicIndex = 26;
    result.commands[1].material.sourceHash = 0x2626u;
    return result;
}

static DM1_V1_HocCandidateFrameLifecycleReceiptPc34 lifecycle(void)
{
    DM1_V1_HocCandidateFrameLifecycleReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1;
    result.clearAccepted = 1; result.portraitAdmitted = 1;
    result.mirrorOrdinal = 3; result.sensorGeneration = 17;
    result.presentedPanelGeneration = 29;
    result.clearTick = 101; result.portraitTick = 102;
    result.c040SourceHash = 0x4040u; result.c026SourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocCandidateFrameCommandReceiptPc34 clear =
        frame(DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34);
    DM1_V1_HocCandidateFrameCommandReceiptPc34 portrait =
        frame(DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34);
    DM1_V1_HocCandidateFrameLifecycleReceiptPc34 life = lifecycle();
    DM1_V1_HocCandidateLifecycleFrameBridgeInputPc34 input;
    DM1_V1_HocCandidateLifecycleFrameBridgeReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.lifecycle = &life; input.priorClearFrame = &clear;
    input.portraitFrame = &portrait;
    ok &= expect(DM1_V1_HocCandidateLifecycleFrameBridge_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.publishC026Portrait &&
                 receipt.portraitCommand.kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
                 receipt.c040SourceHash == 0x4040u &&
                 receipt.portraitCommand.material.sourceHash == 0x2626u,
                 "matching clear lifecycle publishes source-owned C026 portrait");

    portrait.presentedPanelGeneration++;
    ok &= expect(DM1_V1_HocCandidateLifecycleFrameBridge_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "stale portrait frame cannot cross lifecycle bridge");

    if (!ok) return 1;
    puts("ok: DM1 HoC lifecycle bridge fences C026 portrait publish");
    return 0;
}
