#include "dm1_v1_hoc_candidate_pre_m11_lifecycle_bridge_pc34_compat.h"

#include <string.h>

static int route_proves_lifecycle(
    const DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 *lifecycle,
    const DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 *route,
    uint64_t hostTick)
{
    return lifecycle && route && lifecycle->valid && lifecycle->sourceOwned &&
           lifecycle->consumeBeforeM11 && lifecycle->activeHostRouteProven &&
           lifecycle->consumedTick != 0u && lifecycle->consumedTick == hostTick &&
           route->valid && route->sourceOwned && route->admitActiveHostState &&
           route->commandCount == 2 && route->hostTick == hostTick &&
           lifecycle->mirrorOrdinal == route->mirrorOrdinal &&
           lifecycle->sensorGeneration == route->sensorGeneration &&
           lifecycle->presentedPanelGeneration == route->presentedPanelGeneration &&
           route->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           route->commands[0].material.sourceOwned &&
           route->commands[0].material.graphicIndex == 40 &&
           route->commands[0].material.sourceHash == lifecycle->c040SourceHash &&
           route->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           route->commands[1].material.sourceOwned &&
           route->commands[1].material.graphicIndex == 26 &&
           route->commands[1].material.sourceHash == lifecycle->c026SourceHash;
}

int DM1_V1_HocCandidatePreM11LifecycleBridge_BuildReceiptPc34(
    const DM1_V1_HocCandidatePreM11LifecycleBridgeInputPc34 *input,
    DM1_V1_HocCandidatePreM11LifecycleBridgeReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidatePreM11LifecycleBridgeReceiptPc34 receipt;
    const DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 *lifecycle;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352/DUNVIEW.C C026 source state reaches the M11 "
        "boundary only through the current active host-route proof";
    lifecycle = input->lifecycle;
    if (!route_proves_lifecycle(lifecycle, input->activeHostRoute, input->hostTick)) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.admitToM11 = 1;
    receipt.mirrorOrdinal = lifecycle->mirrorOrdinal;
    receipt.sensorGeneration = lifecycle->sensorGeneration;
    receipt.presentedPanelGeneration = lifecycle->presentedPanelGeneration;
    receipt.hostTick = input->hostTick;
    receipt.c040SourceHash = lifecycle->c040SourceHash;
    receipt.c026SourceHash = lifecycle->c026SourceHash;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidatePreM11LifecycleBridge_SourceEvidencePc34(void)
{
    return "ReDMCSB C040/C026 source state crosses the pre-M11 boundary only "
           "with a current active host-route proof.";
}
