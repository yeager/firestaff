#include "dm1_v1_hoc_live_m11_bridge_lifecycle_pc34_compat.h"

#include <string.h>

static int material_valid(const DM1_V1_HocLiveMaterialEvidencePc34 *material,
                          int graphicIndex)
{
    return material && material->sourceOwned && material->graphicIndex == graphicIndex &&
           material->width > 0 && material->height > 0 && material->sourceHash != 0u;
}

static int valid_current(const DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 *bridge,
                         uint64_t runtimeTick)
{
    return bridge && bridge->valid && bridge->sourceOwned && bridge->admitToM11 &&
           bridge->currentC127Proof && bridge->publishCurrent && !bridge->clearRevoke &&
           bridge->clearC040 && bridge->drawC026Portrait && !bridge->clearC026Portrait &&
           bridge->runtimeTick == runtimeTick && material_valid(&bridge->c040, 40) &&
           material_valid(&bridge->c026, 26);
}

static int valid_prior(const DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 *prior)
{
    return prior && prior->valid && prior->sourceOwned && prior->active &&
           prior->currentC127Proof && prior->publishCurrent && prior->clearC040 &&
           prior->drawC026Portrait && !prior->clearC026Portrait && !prior->clearRevoke &&
           prior->admittedTick != 0u && material_valid(&prior->c040, 40) &&
           material_valid(&prior->c026, 26);
}

int DM1_V1_HocLiveM11BridgeLifecycle_BuildReceiptPc34(
    const DM1_V1_HocLiveM11BridgeLifecycleInputPc34 *input,
    DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 *outReceipt)
{
    DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 receipt;
    const DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 *current;
    const DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 *prior;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB live C127/C040/C026 output remains active only on a newer "
        "M11-bound bridge tick; stale source material is cleared and revoked";
    current = input->currentBridge;
    prior = input->prior;
    if (valid_current(current, input->runtimeTick) &&
        (!prior || (valid_prior(prior) && input->runtimeTick > prior->admittedTick))) {
        receipt.valid = 1;
        receipt.sourceOwned = 1;
        receipt.active = 1;
        receipt.currentC127Proof = 1;
        receipt.publishCurrent = 1;
        receipt.clearC040 = 1;
        receipt.drawC026Portrait = 1;
        receipt.mirrorOrdinal = current->mirrorOrdinal;
        receipt.candidateChampionOrdinal = current->candidateChampionOrdinal;
        receipt.sensorGeneration = current->sensorGeneration;
        receipt.panelGeneration = current->panelGeneration;
        receipt.admittedTick = input->runtimeTick;
        receipt.c040 = current->c040;
        receipt.c026 = current->c026;
        *outReceipt = receipt;
        return 1;
    }

    if (!valid_prior(prior) || input->runtimeTick < prior->admittedTick) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.clearC040 = 1;
    receipt.clearC026Portrait = 1;
    receipt.clearRevoke = 1;
    receipt.mirrorOrdinal = prior->mirrorOrdinal;
    receipt.candidateChampionOrdinal = prior->candidateChampionOrdinal;
    receipt.sensorGeneration = prior->sensorGeneration;
    receipt.panelGeneration = prior->panelGeneration;
    receipt.admittedTick = input->runtimeTick;
    receipt.c040 = prior->c040;
    receipt.c026 = prior->c026;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocLiveM11BridgeLifecycle_SourceEvidencePc34(void)
{
    return "ReDMCSB live C127/C040/C026 state is retained only for a newer bridge "
           "tick; stale portrait evidence becomes an explicit source clear.";
}
