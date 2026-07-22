#include "dm1_v1_hoc_candidate_pre_m11_consumption_lifecycle_pc34_compat.h"

#include <string.h>

static int active_route_matches(
    const DM1_V1_HocCandidateHostBridgeConsumptionReceiptPc34 *consumption,
    const DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 *route,
    uint64_t hostTick)
{
    return consumption && route && consumption->valid && consumption->sourceOwned &&
           consumption->consumeBeforeM11 && consumption->commandCount == 2 &&
           route->valid && route->sourceOwned && route->admitActiveHostState &&
           route->commandCount == 2 && consumption->hostTick == hostTick &&
           route->hostTick == hostTick &&
           consumption->commands[0].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 &&
           consumption->commands[0].material.sourceOwned &&
           consumption->commands[0].material.graphicIndex == 40 &&
           consumption->commands[0].material.sourceHash != 0u &&
           consumption->commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 &&
           consumption->commands[1].material.sourceOwned &&
           consumption->commands[1].material.graphicIndex == 26 &&
           consumption->commands[1].material.sourceHash != 0u &&
           consumption->mirrorOrdinal == route->mirrorOrdinal &&
           consumption->sensorGeneration == route->sensorGeneration &&
           consumption->presentedPanelGeneration == route->presentedPanelGeneration &&
           consumption->commands[0].material.sourceHash == route->commands[0].material.sourceHash &&
           consumption->commands[1].material.sourceHash == route->commands[1].material.sourceHash;
}

int DM1_V1_HocCandidatePreM11ConsumptionLifecycle_BuildReceiptPc34(
    const DM1_V1_HocCandidatePreM11ConsumptionLifecycleInputPc34 *input,
    DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 receipt;
    const DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 *prior;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352/DUNVIEW.C C026 pre-M11 consumption remains "
        "source-owned only while the matching host route is active";
    prior = input->prior;
    if (!active_route_matches(input->consumption, input->activeHostRoute, input->hostTick) ||
        (prior && (!prior->valid || !prior->sourceOwned ||
                   !prior->activeHostRouteProven || prior->consumedTick >= input->hostTick))) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.consumeBeforeM11 = 1;
    receipt.activeHostRouteProven = 1;
    receipt.mirrorOrdinal = input->consumption->mirrorOrdinal;
    receipt.sensorGeneration = input->consumption->sensorGeneration;
    receipt.presentedPanelGeneration = input->consumption->presentedPanelGeneration;
    receipt.consumedTick = input->hostTick;
    receipt.c040SourceHash = input->consumption->commands[0].material.sourceHash;
    receipt.c026SourceHash = input->consumption->commands[1].material.sourceHash;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidatePreM11ConsumptionLifecycle_SourceEvidencePc34(void)
{
    return "ReDMCSB source-owned C040/C026 pre-M11 consumption follows the active "
           "host route and cannot be replayed on a stale tick.";
}
