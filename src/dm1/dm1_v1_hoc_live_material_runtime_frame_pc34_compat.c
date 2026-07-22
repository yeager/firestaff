#include "dm1_v1_hoc_live_material_runtime_frame_pc34_compat.h"

#include <string.h>

static int material_valid(const DM1_V1_HocLiveMaterialEvidencePc34 *material,
                          int graphicIndex)
{
    return material && material->sourceOwned && material->graphicIndex == graphicIndex &&
           material->width > 0 && material->height > 0 && material->sourceHash != 0u;
}

static int valid_live_route(const DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34 *route)
{
    return route && route->valid && route->sourceOwned && route->consumedC127 &&
           route->admitC026Portrait && route->admitC040Panel &&
           route->suppressFallbackVisuals && material_valid(&route->c040, 40) &&
           material_valid(&route->c026, 26);
}

static int valid_active_prior(const DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 *prior)
{
    return prior && prior->valid && prior->sourceOwned && prior->active &&
           prior->publishCurrent && prior->clearC040 && prior->drawC026Portrait &&
           !prior->clearC026Portrait && !prior->revokeStale && prior->runtimeTick != 0u &&
           material_valid(&prior->c040, 40) && material_valid(&prior->c026, 26);
}

int DM1_V1_HocLiveMaterialRuntimeFrame_BuildReceiptPc34(
    const DM1_V1_HocLiveMaterialRuntimeFrameInputPc34 *input,
    DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 *outReceipt)
{
    DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 receipt;
    const DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34 *route;
    const DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 *prior;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB C127 F0172/F0174, C040 PANEL.C and C026 DUNVIEW.C source "
        "materials form a tick-fenced runtime frame; stale portraits are cleared";
    route = input->currentRoute;
    prior = input->prior;
    if (valid_live_route(route) &&
        (!prior || (valid_active_prior(prior) && input->runtimeTick > prior->runtimeTick))) {
        receipt.valid = 1;
        receipt.sourceOwned = 1;
        receipt.active = 1;
        receipt.publishCurrent = 1;
        receipt.clearC040 = 1;
        receipt.drawC026Portrait = 1;
        receipt.consumedC127 = 1;
        receipt.mirrorOrdinal = route->mirrorOrdinal;
        receipt.candidateChampionOrdinal = route->candidateChampionOrdinal;
        receipt.sensorGeneration = route->sensorGeneration;
        receipt.panelGeneration = route->panelGeneration;
        receipt.runtimeTick = input->runtimeTick;
        receipt.c040 = route->c040;
        receipt.c026 = route->c026;
        *outReceipt = receipt;
        return 1;
    }

    if (!valid_active_prior(prior) || input->runtimeTick < prior->runtimeTick) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.clearC040 = 1;
    receipt.clearC026Portrait = 1;
    receipt.revokeStale = 1;
    receipt.mirrorOrdinal = prior->mirrorOrdinal;
    receipt.candidateChampionOrdinal = prior->candidateChampionOrdinal;
    receipt.sensorGeneration = prior->sensorGeneration;
    receipt.panelGeneration = prior->panelGeneration;
    receipt.runtimeTick = input->runtimeTick;
    receipt.c040 = prior->c040;
    receipt.c026 = prior->c026;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocLiveMaterialRuntimeFrame_SourceEvidencePc34(void)
{
    return "ReDMCSB C127/C040/C026 live material is publishable only on a newer "
           "runtime tick; stale C026 material is revoked with a clear.";
}
