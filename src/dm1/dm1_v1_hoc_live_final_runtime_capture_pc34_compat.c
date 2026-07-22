#include "dm1_v1_hoc_live_final_runtime_capture_pc34_compat.h"

#include <string.h>

static int material_valid(const DM1_V1_HocLiveMaterialEvidencePc34 *material,
                          int graphicIndex)
{
    return material && material->sourceOwned && material->graphicIndex == graphicIndex &&
           material->width > 0 && material->height > 0 && material->sourceHash != 0u;
}

int DM1_V1_HocLiveFinalRuntimeCapture_BuildReceiptPc34(
    const DM1_V1_HocLiveFinalRuntimeCaptureInputPc34 *input,
    DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 *outReceipt)
{
    DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 receipt;
    const DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 *lifecycle;
    int current;
    int revoke;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB C127 F0172/F0174 with PANEL.C C040 and DUNVIEW.C C026 "
        "is final runtime-capture evidence only while source lifecycle is current";
    lifecycle = input->lifecycle;
    if (!lifecycle || !lifecycle->valid || !lifecycle->sourceOwned ||
        lifecycle->admittedTick != input->runtimeTick || !lifecycle->clearC040 ||
        !material_valid(&lifecycle->c040, 40) || !material_valid(&lifecycle->c026, 26)) {
        *outReceipt = receipt;
        return 1;
    }
    current = lifecycle->active && lifecycle->currentC127Proof && lifecycle->publishCurrent &&
              lifecycle->drawC026Portrait && !lifecycle->clearC026Portrait &&
              !lifecycle->clearRevoke;
    revoke = !lifecycle->active && !lifecycle->currentC127Proof &&
             !lifecycle->publishCurrent && !lifecycle->drawC026Portrait &&
             lifecycle->clearC026Portrait && lifecycle->clearRevoke;
    if (!current && !revoke) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.captureCurrentFrame = current;
    receipt.captureC127Proof = current;
    receipt.captureC040Panel = 1;
    receipt.captureC026Portrait = current;
    receipt.captureClearRevoke = revoke;
    receipt.mirrorOrdinal = lifecycle->mirrorOrdinal;
    receipt.candidateChampionOrdinal = lifecycle->candidateChampionOrdinal;
    receipt.sensorGeneration = lifecycle->sensorGeneration;
    receipt.panelGeneration = lifecycle->panelGeneration;
    receipt.runtimeTick = input->runtimeTick;
    receipt.c040 = lifecycle->c040;
    receipt.c026 = lifecycle->c026;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocLiveFinalRuntimeCapture_SourceEvidencePc34(void)
{
    return "ReDMCSB C127/C040/C026 final capture evidence is real-source only; "
           "a stale source lifecycle is represented solely by its clear/revoke.";
}
