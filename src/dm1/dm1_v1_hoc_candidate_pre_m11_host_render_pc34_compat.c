#include "dm1_v1_hoc_candidate_pre_m11_host_render_pc34_compat.h"

#include <string.h>

static int exact_source_route(
    const DM1_V1_HocCandidatePreM11LifecycleBridgeReceiptPc34 *bridge,
    const DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 *route,
    uint64_t hostTick)
{
    return bridge && route && bridge->valid && bridge->sourceOwned &&
           bridge->admitToM11 && bridge->hostTick == hostTick &&
           route->valid && route->sourceOwned && route->admitActiveHostState &&
           route->commandCount == 2 && route->hostTick == hostTick &&
           bridge->mirrorOrdinal == route->mirrorOrdinal &&
           bridge->sensorGeneration == route->sensorGeneration &&
           bridge->presentedPanelGeneration == route->presentedPanelGeneration &&
           route->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           route->commands[0].material.sourceOwned &&
           route->commands[0].material.graphicIndex == 40 &&
           route->commands[0].material.sourceHash != 0u &&
           route->commands[0].material.sourceHash == bridge->c040SourceHash &&
           route->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           route->commands[1].material.sourceOwned &&
           route->commands[1].material.graphicIndex == 26 &&
           route->commands[1].material.sourceHash != 0u &&
           route->commands[1].material.sourceHash == bridge->c026SourceHash;
}

int DM1_V1_HocCandidatePreM11HostRender_BuildReceiptPc34(
    const DM1_V1_HocCandidatePreM11HostRenderInputPc34 *input,
    DM1_V1_HocCandidatePreM11HostRenderReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidatePreM11HostRenderReceiptPc34 receipt;
    const DM1_V1_HocCandidatePreM11LifecycleBridgeReceiptPc34 *bridge;
    const DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 *route;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352 C040 and DUNVIEW.C C026 retain exact source "
        "material proof at the host-render boundary before M11";
    bridge = input->bridge;
    route = input->activeHostRoute;
    if (!exact_source_route(bridge, route, input->hostTick)) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.renderAtM11Boundary = 1;
    receipt.commandCount = 2;
    receipt.commands[0] = route->commands[0];
    receipt.commands[1] = route->commands[1];
    receipt.mirrorOrdinal = bridge->mirrorOrdinal;
    receipt.sensorGeneration = bridge->sensorGeneration;
    receipt.presentedPanelGeneration = bridge->presentedPanelGeneration;
    receipt.hostTick = input->hostTick;
    receipt.c040SourceHash = bridge->c040SourceHash;
    receipt.c026SourceHash = bridge->c026SourceHash;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidatePreM11HostRender_SourceEvidencePc34(void)
{
    return "ReDMCSB C040 panel and C026 portrait materials retain their exact source "
           "hash proof through the pre-M11 host-render boundary.";
}
