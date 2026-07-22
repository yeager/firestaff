#include "dm1_v1_hoc_candidate_runtime_lifecycle_host_bridge_pc34_compat.h"

#include <string.h>

static int is_active_admission(
    const DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 *admission,
    const DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 *lifecycle,
    uint64_t hostTick)
{
    return admission && admission->valid && admission->sourceOwned &&
           admission->admitHostRoute && admission->commandCount == 2 &&
           admission->renderTick == hostTick &&
           admission->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           admission->commands[0].material.sourceOwned &&
           admission->commands[0].material.graphicIndex == 40 &&
           admission->commands[0].material.sourceHash == lifecycle->c040SourceHash &&
           admission->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           admission->commands[1].material.sourceOwned &&
           admission->commands[1].material.graphicIndex == 26 &&
           admission->commands[1].material.sourceHash == lifecycle->c026SourceHash;
}

int DM1_V1_HocCandidateRuntimeLifecycleHostBridge_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeLifecycleHostBridgeInputPc34 *input,
    DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 receipt;
    const DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 *lifecycle;
    const DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 *admission;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352 C040 and DUNVIEW.C C026 become host-visible "
        "only through the active, tick-fenced source lifecycle";
    lifecycle = input->lifecycle;
    admission = input->admission;
    if (!lifecycle || !lifecycle->valid || !lifecycle->sourceOwned ||
        !lifecycle->consumeC040 || !lifecycle->consumeC026 ||
        lifecycle->consumedTick == 0u || lifecycle->consumedTick != input->hostTick ||
        !is_active_admission(admission, lifecycle, input->hostTick) ||
        admission->mirrorOrdinal != lifecycle->mirrorOrdinal ||
        admission->sensorGeneration != lifecycle->sensorGeneration ||
        admission->presentedPanelGeneration != lifecycle->presentedPanelGeneration) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.admitActiveHostState = 1;
    receipt.commandCount = 2;
    receipt.commands[0] = admission->commands[0];
    receipt.commands[1] = admission->commands[1];
    receipt.mirrorOrdinal = lifecycle->mirrorOrdinal;
    receipt.sensorGeneration = lifecycle->sensorGeneration;
    receipt.presentedPanelGeneration = lifecycle->presentedPanelGeneration;
    receipt.hostTick = input->hostTick;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateRuntimeLifecycleHostBridge_SourceEvidencePc34(void)
{
    return "ReDMCSB C040/C026 frame state enters the host only while its source "
           "lifecycle is active at the matching render tick.";
}
