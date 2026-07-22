#include "dm1_v1_hoc_candidate_runtime_frame_lifecycle_pc34_compat.h"

#include <string.h>

static int valid_admission(const DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34 *admission,
                           uint64_t runtimeTick)
{
    return admission && admission->valid && admission->sourceOwned &&
           admission->admitRuntimeFrame && admission->commandCount == 2 &&
           admission->runtimeTick == runtimeTick &&
           admission->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           admission->commands[0].material.sourceOwned &&
           admission->commands[0].material.graphicIndex == 40 &&
           admission->commands[0].material.sourceHash == admission->c040SourceHash &&
           admission->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           admission->commands[1].material.sourceOwned &&
           admission->commands[1].material.graphicIndex == 26 &&
           admission->commands[1].material.sourceHash == admission->c026SourceHash;
}

static int valid_active_prior(const DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 *prior)
{
    return prior && prior->valid && prior->sourceOwned && prior->active &&
           prior->commandCount == 2 && prior->admittedTick != 0u &&
           prior->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           prior->commands[0].material.sourceOwned &&
           prior->commands[0].material.graphicIndex == 40 &&
           prior->commands[0].material.sourceHash == prior->c040SourceHash &&
           prior->commands[1].material.sourceOwned &&
           prior->commands[1].material.graphicIndex == 26 &&
           prior->commands[1].material.sourceHash == prior->c026SourceHash;
}

int DM1_V1_HocCandidateRuntimeFrameLifecycle_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeFrameLifecycleInputPc34 *input,
    DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 receipt;
    const DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34 *current;
    const DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 *prior;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352/DUNVIEW.C C026 runtime frames publish only new "
        "source state; stale C026 is revoked through its original material";
    current = input->currentAdmission;
    prior = input->prior;
    if (valid_admission(current, input->runtimeTick) &&
        (!prior || (valid_active_prior(prior) && input->runtimeTick > prior->admittedTick))) {
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

    if (!valid_active_prior(prior) || input->runtimeTick < prior->admittedTick) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.revokeStale = 1;
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

const char *DM1_V1_HocCandidateRuntimeFrameLifecycle_SourceEvidencePc34(void)
{
    return "ReDMCSB source-owned C040/C026 runtime state is replaced only by a later "
           "receipt; stale portrait material is cleared rather than replayed.";
}
