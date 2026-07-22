#include "dm1_v1_hoc_live_material_runtime_m11_bridge_pc34_compat.h"

#include <string.h>

static int material_valid(const DM1_V1_HocLiveMaterialEvidencePc34 *material,
                          int graphicIndex)
{
    return material && material->sourceOwned && material->graphicIndex == graphicIndex &&
           material->width > 0 && material->height > 0 && material->sourceHash != 0u;
}

int DM1_V1_HocLiveMaterialRuntimeM11Bridge_BuildReceiptPc34(
    const DM1_V1_HocLiveMaterialRuntimeM11BridgeInputPc34 *input,
    DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 *outReceipt)
{
    DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 receipt;
    const DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 *frame;
    int current;
    int revoke;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB C127 F0172/F0174, PANEL.C C040 and DUNVIEW.C C026 live "
        "source material crosses toward M11 only as current proof or clear/revoke";
    frame = input->runtimeFrame;
    if (!frame || !frame->valid || !frame->sourceOwned ||
        frame->runtimeTick != input->runtimeTick || !frame->clearC040 ||
        !material_valid(&frame->c040, 40) || !material_valid(&frame->c026, 26)) {
        *outReceipt = receipt;
        return 1;
    }
    current = frame->active && frame->publishCurrent && frame->consumedC127 &&
              frame->drawC026Portrait && !frame->clearC026Portrait && !frame->revokeStale;
    revoke = !frame->active && !frame->publishCurrent && !frame->consumedC127 &&
             !frame->drawC026Portrait && frame->clearC026Portrait && frame->revokeStale;
    if (!current && !revoke) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.admitToM11 = 1;
    receipt.currentC127Proof = current;
    receipt.publishCurrent = current;
    receipt.clearC040 = 1;
    receipt.drawC026Portrait = current;
    receipt.clearC026Portrait = revoke;
    receipt.clearRevoke = revoke;
    receipt.mirrorOrdinal = frame->mirrorOrdinal;
    receipt.candidateChampionOrdinal = frame->candidateChampionOrdinal;
    receipt.sensorGeneration = frame->sensorGeneration;
    receipt.panelGeneration = frame->panelGeneration;
    receipt.runtimeTick = input->runtimeTick;
    receipt.c040 = frame->c040;
    receipt.c026 = frame->c026;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocLiveMaterialRuntimeM11Bridge_SourceEvidencePc34(void)
{
    return "ReDMCSB live C127/C040/C026 state reaches the M11 boundary only "
           "with current sensor proof; stale material is explicitly cleared.";
}
