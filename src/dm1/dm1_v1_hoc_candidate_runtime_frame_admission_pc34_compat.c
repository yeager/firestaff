#include "dm1_v1_hoc_candidate_runtime_frame_admission_pc34_compat.h"

#include <string.h>

static int exact_pre_m11_proof(
    const DM1_V1_HocCandidatePreM11LifecycleBridgeReceiptPc34 *bridge,
    const DM1_V1_HocCandidatePreM11HostRenderReceiptPc34 *render,
    uint64_t runtimeTick)
{
    return bridge && render && bridge->valid && bridge->sourceOwned &&
           bridge->admitToM11 && bridge->hostTick == runtimeTick &&
           render->valid && render->sourceOwned && render->renderAtM11Boundary &&
           render->commandCount == 2 && render->hostTick == runtimeTick &&
           bridge->mirrorOrdinal == render->mirrorOrdinal &&
           bridge->sensorGeneration == render->sensorGeneration &&
           bridge->presentedPanelGeneration == render->presentedPanelGeneration &&
           bridge->c040SourceHash == render->c040SourceHash &&
           bridge->c026SourceHash == render->c026SourceHash &&
           render->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           render->commands[0].material.sourceOwned &&
           render->commands[0].material.graphicIndex == 40 &&
           render->commands[0].material.sourceHash == render->c040SourceHash &&
           render->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           render->commands[1].material.sourceOwned &&
           render->commands[1].material.graphicIndex == 26 &&
           render->commands[1].material.sourceHash == render->c026SourceHash;
}

int DM1_V1_HocCandidateRuntimeFrameAdmission_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeFrameAdmissionInputPc34 *input,
    DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34 receipt;
    const DM1_V1_HocCandidatePreM11LifecycleBridgeReceiptPc34 *bridge;
    const DM1_V1_HocCandidatePreM11HostRenderReceiptPc34 *render;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352 and DUNVIEW.C C026 pre-M11 bridge/render proof "
        "combine only when source C040/C026 tick, generation and hash match";
    bridge = input->preM11Bridge;
    render = input->hostRender;
    if (!exact_pre_m11_proof(bridge, render, input->runtimeTick)) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.admitRuntimeFrame = 1;
    receipt.commandCount = 2;
    receipt.commands[0] = render->commands[0];
    receipt.commands[1] = render->commands[1];
    receipt.mirrorOrdinal = render->mirrorOrdinal;
    receipt.sensorGeneration = render->sensorGeneration;
    receipt.presentedPanelGeneration = render->presentedPanelGeneration;
    receipt.runtimeTick = input->runtimeTick;
    receipt.c040SourceHash = render->c040SourceHash;
    receipt.c026SourceHash = render->c026SourceHash;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateRuntimeFrameAdmission_SourceEvidencePc34(void)
{
    return "ReDMCSB C040/C026 source proof is combined into a runtime frame only "
           "when bridge and host-render receipts agree exactly.";
}
