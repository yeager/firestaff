#include "dm1_v1_hoc_candidate_lifecycle_frame_bridge_pc34_compat.h"

#include <string.h>

static int is_clear_frame(const DM1_V1_HocCandidateFrameCommandReceiptPc34 *frame)
{
    return frame && frame->valid && frame->sourceOwned && frame->commandCount == 2 &&
           frame->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           frame->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34 &&
           frame->commands[0].material.sourceOwned &&
           frame->commands[0].material.graphicIndex == 40 &&
           frame->commands[0].material.sourceHash != 0u;
}

static int is_portrait_frame(const DM1_V1_HocCandidateFrameCommandReceiptPc34 *frame)
{
    return frame && frame->valid && frame->sourceOwned && frame->commandCount == 2 &&
           frame->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           frame->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           frame->commands[1].material.sourceOwned &&
           frame->commands[1].material.graphicIndex == 26 &&
           frame->commands[1].material.sourceHash != 0u;
}

int DM1_V1_HocCandidateLifecycleFrameBridge_BuildReceiptPc34(
    const DM1_V1_HocCandidateLifecycleFrameBridgeInputPc34 *input,
    DM1_V1_HocCandidateLifecycleFrameBridgeReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateLifecycleFrameBridgeReceiptPc34 receipt;
    const DM1_V1_HocCandidateFrameLifecycleReceiptPc34 *lifecycle;
    const DM1_V1_HocCandidateFrameCommandReceiptPc34 *clearFrame;
    const DM1_V1_HocCandidateFrameCommandReceiptPc34 *portraitFrame;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352 clear completion precedes DUNVIEW.C C026 "
        "portrait publish; both source frame receipts must match lifecycle state";
    lifecycle = input->lifecycle;
    clearFrame = input->priorClearFrame;
    portraitFrame = input->portraitFrame;
    if (!lifecycle || !lifecycle->valid || !lifecycle->sourceOwned ||
        !lifecycle->clearAccepted || !lifecycle->portraitAdmitted ||
        !is_clear_frame(clearFrame) || !is_portrait_frame(portraitFrame) ||
        lifecycle->portraitTick <= lifecycle->clearTick ||
        clearFrame->mirrorOrdinal != lifecycle->mirrorOrdinal ||
        portraitFrame->mirrorOrdinal != lifecycle->mirrorOrdinal ||
        clearFrame->sensorGeneration != lifecycle->sensorGeneration ||
        portraitFrame->sensorGeneration != lifecycle->sensorGeneration ||
        clearFrame->presentedPanelGeneration != lifecycle->presentedPanelGeneration ||
        portraitFrame->presentedPanelGeneration != lifecycle->presentedPanelGeneration ||
        clearFrame->commands[0].material.sourceHash != lifecycle->c040SourceHash ||
        portraitFrame->commands[1].material.sourceHash != lifecycle->c026SourceHash) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.publishC026Portrait = 1;
    receipt.mirrorOrdinal = lifecycle->mirrorOrdinal;
    receipt.sensorGeneration = lifecycle->sensorGeneration;
    receipt.presentedPanelGeneration = lifecycle->presentedPanelGeneration;
    receipt.clearTick = lifecycle->clearTick;
    receipt.portraitTick = lifecycle->portraitTick;
    receipt.c040SourceHash = clearFrame->commands[0].material.sourceHash;
    receipt.portraitCommand = portraitFrame->commands[1];
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateLifecycleFrameBridge_SourceEvidencePc34(void)
{
    return "ReDMCSB PANEL.C F0352 clear ownership is consumed before DUNVIEW.C "
           "C026 portrait publish; mismatched generation or source material fails closed.";
}
