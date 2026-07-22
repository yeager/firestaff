#include "dm1_v1_hoc_configured_capture_m11_gate_pc34_compat.h"

#include <string.h>

static int material_matches(const DM1_V1_HocLiveMaterialEvidencePc34 *material,
                            int graphicIndex, uint32_t sourceHash)
{
    return material && material->sourceOwned && material->graphicIndex == graphicIndex &&
           material->width > 0 && material->height > 0 && sourceHash != 0u &&
           material->sourceHash == sourceHash;
}

int DM1_V1_HocConfiguredCaptureM11Gate_BuildReceiptPc34(
    const DM1_V1_HocConfiguredCaptureM11GateInputPc34 *input,
    DM1_V1_HocConfiguredCaptureM11GateReceiptPc34 *outReceipt)
{
    DM1_V1_HocConfiguredCaptureM11GateReceiptPc34 receipt;
    const DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 *lifecycle;
    const DM1_V1_HocConfiguredOriginalEvidencePc34 *configuration;
    int current;
    int revoke;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB configured C127 with decoded C040/C026 source pages gates "
        "capture at the M11 boundary; no unconfigured visual is admitted";
    lifecycle = input->lifecycle;
    configuration = input->configuration;
    if (!lifecycle || !configuration || !lifecycle->valid || !lifecycle->sourceOwned ||
        !configuration->sourceOwned || lifecycle->captureTick != input->runtimeTick ||
        lifecycle->mirrorOrdinal != configuration->c127MirrorOrdinal ||
        !material_matches(&lifecycle->c040, 40, configuration->c040SourceHash) ||
        !material_matches(&lifecycle->c026, 26, configuration->c026SourceHash)) {
        *outReceipt = receipt;
        return 1;
    }
    current = lifecycle->active && lifecycle->publishCurrentCapture &&
              lifecycle->captureC127Proof && lifecycle->captureC040Panel &&
              lifecycle->captureC026Portrait && !lifecycle->clearRevoke;
    revoke = !lifecycle->active && !lifecycle->publishCurrentCapture &&
             !lifecycle->captureC127Proof && lifecycle->captureC040Panel &&
             !lifecycle->captureC026Portrait && lifecycle->clearRevoke;
    if (!current && !revoke) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.admitCaptureToM11 = 1;
    receipt.captureCurrentC127 = current;
    receipt.captureC040Panel = 1;
    receipt.captureC026Portrait = current;
    receipt.clearRevoke = revoke;
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

const char *DM1_V1_HocConfiguredCaptureM11Gate_SourceEvidencePc34(void)
{
    return "ReDMCSB original C127/C040/C026 configuration is the sole capture "
           "admission proof at the M11 boundary; stale state is clear/revoke only.";
}
