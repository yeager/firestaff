#include "dm1_v1_hoc_candidate_frame_lifecycle_pc34_compat.h"

#include <string.h>

static int is_source_frame(const DM1_V1_HocCandidateFrameCommandReceiptPc34 *frame,
                           DM1_V1_HocCandidateRenderCommandKindPc34 c026Kind)
{
    return frame && frame->valid && frame->sourceOwned && frame->commandCount == 2 &&
           frame->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           frame->commands[1].kind == c026Kind &&
           frame->commands[0].material.sourceOwned &&
           frame->commands[0].material.graphicIndex == 40 &&
           frame->commands[0].material.sourceHash != 0u &&
           frame->commands[1].material.sourceOwned &&
           frame->commands[1].material.graphicIndex == 26 &&
           frame->commands[1].material.sourceHash != 0u;
}

int DM1_V1_HocCandidateFrameLifecycle_BuildReceiptPc34(
    const DM1_V1_HocCandidateFrameLifecycleInputPc34 *input,
    DM1_V1_HocCandidateFrameLifecycleReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateFrameLifecycleReceiptPc34 receipt;
    const DM1_V1_HocCandidateFrameCommandReceiptPc34 *clearFrame;
    const DM1_V1_HocCandidateFrameCommandReceiptPc34 *portraitFrame;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352 C040 clear followed by DUNVIEW.C C026 "
        "portrait presentation; host frame ticks fence stale panel output";
    clearFrame = input->priorClearFrame;
    portraitFrame = input->nextPortraitFrame;
    if (!is_source_frame(clearFrame, DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34) ||
        !is_source_frame(portraitFrame, DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34) ||
        input->priorClearTick == 0u || input->nextPortraitTick <= input->priorClearTick ||
        clearFrame->mirrorOrdinal != portraitFrame->mirrorOrdinal ||
        clearFrame->sensorGeneration != portraitFrame->sensorGeneration ||
        clearFrame->presentedPanelGeneration != portraitFrame->presentedPanelGeneration) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.clearAccepted = 1;
    receipt.portraitAdmitted = 1;
    receipt.mirrorOrdinal = clearFrame->mirrorOrdinal;
    receipt.sensorGeneration = clearFrame->sensorGeneration;
    receipt.presentedPanelGeneration = clearFrame->presentedPanelGeneration;
    receipt.clearTick = input->priorClearTick;
    receipt.portraitTick = input->nextPortraitTick;
    receipt.c040SourceHash = portraitFrame->commands[0].material.sourceHash;
    receipt.c026SourceHash = portraitFrame->commands[1].material.sourceHash;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateFrameLifecycle_SourceEvidencePc34(void)
{
    return "ReDMCSB PANEL.C F0352 clears C040 before DUNVIEW.C consumes C026; "
           "a later host frame may restore only matching source-owned state.";
}
