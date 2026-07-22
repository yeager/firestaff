#include "dm1_v1_hoc_candidate_host_bridge_consumption_pc34_compat.h"

#include <string.h>

static int active_source_route(
    const DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 *bridge,
    uint64_t hostTick)
{
    return bridge && bridge->valid && bridge->sourceOwned &&
           bridge->admitActiveHostState && bridge->commandCount == 2 &&
           bridge->hostTick != 0u && bridge->hostTick == hostTick &&
           bridge->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           bridge->commands[0].material.sourceOwned &&
           bridge->commands[0].material.graphicIndex == 40 &&
           bridge->commands[0].material.sourceHash != 0u &&
           bridge->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           bridge->commands[1].material.sourceOwned &&
           bridge->commands[1].material.graphicIndex == 26 &&
           bridge->commands[1].material.sourceHash != 0u;
}

int DM1_V1_HocCandidateHostBridgeConsumption_BuildReceiptPc34(
    const DM1_V1_HocCandidateHostBridgeConsumptionInputPc34 *input,
    DM1_V1_HocCandidateHostBridgeConsumptionReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateHostBridgeConsumptionReceiptPc34 receipt;
    const DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 *bridge;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352/DUNVIEW.C C026 source commands are consumed "
        "only from an active host route immediately before M11 rendering";
    bridge = input->hostBridge;
    if (!active_source_route(bridge, input->hostTick)) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.consumeBeforeM11 = 1;
    receipt.commandCount = 2;
    receipt.commands[0] = bridge->commands[0];
    receipt.commands[1] = bridge->commands[1];
    receipt.mirrorOrdinal = bridge->mirrorOrdinal;
    receipt.sensorGeneration = bridge->sensorGeneration;
    receipt.presentedPanelGeneration = bridge->presentedPanelGeneration;
    receipt.hostTick = input->hostTick;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateHostBridgeConsumption_SourceEvidencePc34(void)
{
    return "ReDMCSB source panel and portrait commands are admitted before M11 "
           "only while the host route is active at the matching tick.";
}
