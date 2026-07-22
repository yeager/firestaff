#include "dm1_v1_hoc_candidate_runtime_m11_bridge_lifecycle_pc34_compat.h"

#include <string.h>

static int valid_current(const DM1_V1_HocCandidateRuntimeFrameM11BridgeReceiptPc34 *bridge,
                         uint64_t runtimeTick)
{
    return bridge && bridge->valid && bridge->sourceOwned && bridge->admitToM11 &&
           bridge->publishCurrent && !bridge->clearRevoke && bridge->commandCount == 2 &&
           bridge->runtimeTick == runtimeTick &&
           bridge->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           bridge->commands[0].material.sourceOwned &&
           bridge->commands[0].material.graphicIndex == 40 &&
           bridge->commands[0].material.sourceHash == bridge->c040SourceHash &&
           bridge->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           bridge->commands[1].material.sourceOwned &&
           bridge->commands[1].material.graphicIndex == 26 &&
           bridge->commands[1].material.sourceHash == bridge->c026SourceHash;
}

static int valid_prior(const DM1_V1_HocCandidateRuntimeM11BridgeLifecycleReceiptPc34 *prior)
{
    return prior && prior->valid && prior->sourceOwned && prior->active &&
           prior->commandCount == 2 && prior->admittedTick != 0u &&
           prior->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           prior->commands[0].material.sourceOwned &&
           prior->commands[0].material.graphicIndex == 40 &&
           prior->commands[0].material.sourceHash == prior->c040SourceHash &&
           prior->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           prior->commands[1].material.sourceOwned &&
           prior->commands[1].material.graphicIndex == 26 &&
           prior->commands[1].material.sourceHash == prior->c026SourceHash;
}

int DM1_V1_HocCandidateRuntimeM11BridgeLifecycle_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeM11BridgeLifecycleInputPc34 *input,
    DM1_V1_HocCandidateRuntimeM11BridgeLifecycleReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateRuntimeM11BridgeLifecycleReceiptPc34 receipt;
    const DM1_V1_HocCandidateRuntimeFrameM11BridgeReceiptPc34 *current;
    const DM1_V1_HocCandidateRuntimeM11BridgeLifecycleReceiptPc34 *prior;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB C040/C026 M11-bound state is current only on a newer bridge tick; "
        "stale bridge output clears the existing source portrait";
    current = input->currentBridge;
    prior = input->prior;
    if (valid_current(current, input->runtimeTick) &&
        (!prior || (valid_prior(prior) && input->runtimeTick > prior->admittedTick))) {
        receipt.valid = 1;
        receipt.sourceOwned = 1;
        receipt.active = 1;
        receipt.publishCurrent = 1;
        receipt.commandCount = 2;
        receipt.commands[0] = current->commands[0];
        receipt.commands[1] = current->commands[1];
        receipt.mirrorOrdinal = current->mirrorOrdinal;
        receipt.sensorGeneration = current->sensorGeneration;
        receipt.presentedPanelGeneration = current->presentedPanelGeneration;
        receipt.admittedTick = input->runtimeTick;
        receipt.c040SourceHash = current->c040SourceHash;
        receipt.c026SourceHash = current->c026SourceHash;
        *outReceipt = receipt;
        return 1;
    }

    if (!valid_prior(prior) || input->runtimeTick < prior->admittedTick) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.clearRevoke = 1;
    receipt.commandCount = 2;
    receipt.commands[0] = prior->commands[0];
    receipt.commands[1] = prior->commands[1];
    receipt.commands[1].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34;
    receipt.mirrorOrdinal = prior->mirrorOrdinal;
    receipt.sensorGeneration = prior->sensorGeneration;
    receipt.presentedPanelGeneration = prior->presentedPanelGeneration;
    receipt.admittedTick = input->runtimeTick;
    receipt.c040SourceHash = prior->c040SourceHash;
    receipt.c026SourceHash = prior->c026SourceHash;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateRuntimeM11BridgeLifecycle_SourceEvidencePc34(void)
{
    return "ReDMCSB source C026 portrait state enters the M11 boundary only from a "
           "newer tick; stale state becomes a source-owned clear command.";
}
