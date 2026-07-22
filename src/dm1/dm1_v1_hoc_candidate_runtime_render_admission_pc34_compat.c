#include "dm1_v1_hoc_candidate_runtime_render_admission_pc34_compat.h"

#include <string.h>

static int original_c040_clear(const DM1_V1_HocCandidateFrameCommandReceiptPc34 *frame)
{
    return frame && frame->valid && frame->sourceOwned && frame->commandCount == 2 &&
           frame->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           frame->commands[0].material.sourceOwned &&
           frame->commands[0].material.graphicIndex == 40 &&
           frame->commands[0].material.sourceHash != 0u;
}

static int original_c026_portrait(const DM1_V1_HocCandidateFrameCommandPc34 *command)
{
    return command && command->kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           command->material.sourceOwned && command->material.graphicIndex == 26 &&
           command->material.sourceHash != 0u;
}

int DM1_V1_HocCandidateRuntimeRenderAdmission_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeRenderAdmissionInputPc34 *input,
    DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 receipt;
    const DM1_V1_HocCandidateLifecycleFrameBridgeReceiptPc34 *bridge;
    const DM1_V1_HocCandidateFrameCommandReceiptPc34 *clearFrame;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352 C040 clear and DUNVIEW.C C026 portrait "
        "must remain original materials before host render-route admission";
    bridge = input->bridge;
    clearFrame = input->priorClearFrame;
    if (!bridge || !bridge->valid || !bridge->sourceOwned ||
        !bridge->publishC026Portrait || !original_c040_clear(clearFrame) ||
        !original_c026_portrait(&bridge->portraitCommand) ||
        bridge->portraitTick <= bridge->clearTick ||
        clearFrame->mirrorOrdinal != bridge->mirrorOrdinal ||
        clearFrame->sensorGeneration != bridge->sensorGeneration ||
        clearFrame->presentedPanelGeneration != bridge->presentedPanelGeneration ||
        clearFrame->commands[0].material.sourceHash != bridge->c040SourceHash) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.admitHostRoute = 1;
    receipt.commandCount = 2;
    receipt.commands[0] = clearFrame->commands[0];
    receipt.commands[1] = bridge->portraitCommand;
    receipt.mirrorOrdinal = bridge->mirrorOrdinal;
    receipt.sensorGeneration = bridge->sensorGeneration;
    receipt.presentedPanelGeneration = bridge->presentedPanelGeneration;
    receipt.renderTick = bridge->portraitTick;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateRuntimeRenderAdmission_SourceEvidencePc34(void)
{
    return "ReDMCSB PANEL.C F0352 and DUNVIEW.C C026 enter rendering as original "
           "materials; a stale receipt must not enter the host route.";
}
