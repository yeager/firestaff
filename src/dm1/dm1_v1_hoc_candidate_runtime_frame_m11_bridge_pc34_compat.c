#include "dm1_v1_hoc_candidate_runtime_frame_m11_bridge_pc34_compat.h"

#include <string.h>

static int source_commands_match(
    const DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 *lifecycle)
{
    return lifecycle && lifecycle->commandCount == 2 &&
           lifecycle->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           lifecycle->commands[0].material.sourceOwned &&
           lifecycle->commands[0].material.graphicIndex == 40 &&
           lifecycle->commands[0].material.sourceHash == lifecycle->c040SourceHash &&
           lifecycle->commands[1].material.sourceOwned &&
           lifecycle->commands[1].material.graphicIndex == 26 &&
           lifecycle->commands[1].material.sourceHash == lifecycle->c026SourceHash;
}

int DM1_V1_HocCandidateRuntimeFrameM11Bridge_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeFrameM11BridgeInputPc34 *input,
    DM1_V1_HocCandidateRuntimeFrameM11BridgeReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateRuntimeFrameM11BridgeReceiptPc34 receipt;
    const DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 *lifecycle;
    int current;
    int revoke;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352/DUNVIEW.C C026 runtime state reaches M11 only "
        "as current C040/C026 or source-owned clear/revoke commands";
    lifecycle = input->lifecycle;
    if (!lifecycle || !lifecycle->valid || !lifecycle->sourceOwned ||
        lifecycle->admittedTick != input->runtimeTick || !source_commands_match(lifecycle)) {
        *outReceipt = receipt;
        return 1;
    }
    current = lifecycle->active && lifecycle->publishCurrent && !lifecycle->revokeStale &&
              lifecycle->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34;
    revoke = !lifecycle->active && !lifecycle->publishCurrent && lifecycle->revokeStale &&
             lifecycle->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34;
    if (!current && !revoke) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.admitToM11 = 1;
    receipt.publishCurrent = current;
    receipt.clearRevoke = revoke;
    receipt.commandCount = 2;
    receipt.commands[0] = lifecycle->commands[0];
    receipt.commands[1] = lifecycle->commands[1];
    receipt.mirrorOrdinal = lifecycle->mirrorOrdinal;
    receipt.sensorGeneration = lifecycle->sensorGeneration;
    receipt.presentedPanelGeneration = lifecycle->presentedPanelGeneration;
    receipt.runtimeTick = input->runtimeTick;
    receipt.c040SourceHash = lifecycle->c040SourceHash;
    receipt.c026SourceHash = lifecycle->c026SourceHash;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateRuntimeFrameM11Bridge_SourceEvidencePc34(void)
{
    return "ReDMCSB C040/C026 runtime state is handed toward M11 only as current "
           "source material or a source-owned stale-clear revoke.";
}
