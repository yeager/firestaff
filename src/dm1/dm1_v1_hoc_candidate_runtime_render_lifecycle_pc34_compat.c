#include "dm1_v1_hoc_candidate_runtime_render_lifecycle_pc34_compat.h"

#include <string.h>

static int is_current_source_admission(
    const DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 *admission,
    uint64_t hostTick)
{
    return admission && admission->valid && admission->sourceOwned &&
           admission->admitHostRoute && admission->commandCount == 2 &&
           admission->renderTick != 0u && admission->renderTick == hostTick &&
           admission->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           admission->commands[0].material.sourceOwned &&
           admission->commands[0].material.graphicIndex == 40 &&
           admission->commands[0].material.sourceHash != 0u &&
           admission->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           admission->commands[1].material.sourceOwned &&
           admission->commands[1].material.graphicIndex == 26 &&
           admission->commands[1].material.sourceHash != 0u;
}

int DM1_V1_HocCandidateRuntimeRenderLifecycle_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeRenderLifecycleInputPc34 *input,
    DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 receipt;
    const DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 *admission;
    const DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 *prior;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352 C040 and DUNVIEW.C C026 host consumption "
        "is fenced by the source render frame; stale ticks fail closed";
    admission = input->admission;
    prior = input->prior;
    if (!is_current_source_admission(admission, input->hostTick) ||
        (prior && (!prior->valid || !prior->sourceOwned ||
                   prior->consumedTick >= input->hostTick))) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.consumeC040 = 1;
    receipt.consumeC026 = 1;
    receipt.mirrorOrdinal = admission->mirrorOrdinal;
    receipt.sensorGeneration = admission->sensorGeneration;
    receipt.presentedPanelGeneration = admission->presentedPanelGeneration;
    receipt.consumedTick = input->hostTick;
    receipt.c040SourceHash = admission->commands[0].material.sourceHash;
    receipt.c026SourceHash = admission->commands[1].material.sourceHash;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateRuntimeRenderLifecycle_SourceEvidencePc34(void)
{
    return "ReDMCSB C040 panel clear and C026 portrait draw are consumed in frame "
           "order; source receipt ticks reject stale host replay.";
}
