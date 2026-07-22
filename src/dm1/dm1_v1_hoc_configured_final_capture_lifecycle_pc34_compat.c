#include "dm1_v1_hoc_configured_final_capture_lifecycle_pc34_compat.h"

#include <string.h>

static int material_valid(const DM1_V1_HocLiveMaterialEvidencePc34 *material,
                          int graphicIndex)
{
    return material && material->sourceOwned && material->graphicIndex == graphicIndex &&
           material->width > 0 && material->height > 0 && material->sourceHash != 0u;
}

static int configured_current(
    const DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 *capture,
    const DM1_V1_HocConfiguredOriginalEvidencePc34 *configuration,
    uint64_t runtimeTick)
{
    return capture && configuration && capture->valid && capture->sourceOwned &&
           capture->captureCurrentFrame && capture->captureC127Proof &&
           capture->captureC040Panel && capture->captureC026Portrait &&
           !capture->captureClearRevoke && capture->runtimeTick == runtimeTick &&
           configuration->sourceOwned &&
           capture->mirrorOrdinal == configuration->c127MirrorOrdinal &&
           material_valid(&capture->c040, 40) && material_valid(&capture->c026, 26) &&
           capture->c040.sourceHash == configuration->c040SourceHash &&
           capture->c026.sourceHash == configuration->c026SourceHash;
}

static int valid_prior(const DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 *prior)
{
    return prior && prior->valid && prior->sourceOwned && prior->active &&
           prior->publishCurrentCapture && prior->captureC127Proof &&
           prior->captureC040Panel && prior->captureC026Portrait &&
           !prior->clearRevoke && prior->captureTick != 0u &&
           material_valid(&prior->c040, 40) && material_valid(&prior->c026, 26);
}

int DM1_V1_HocConfiguredFinalCaptureLifecycle_BuildReceiptPc34(
    const DM1_V1_HocConfiguredFinalCaptureLifecycleInputPc34 *input,
    DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 *outReceipt)
{
    DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 receipt;
    const DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 *capture;
    const DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 *prior;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB C127 configuration and decoded C040/C026 source pages fence "
        "final runtime capture; unconfigured or stale material is revoked";
    capture = input->currentCapture;
    prior = input->prior;
    if (configured_current(capture, input->configuration, input->runtimeTick) &&
        (!prior || (valid_prior(prior) && input->runtimeTick > prior->captureTick))) {
        receipt.valid = 1;
        receipt.sourceOwned = 1;
        receipt.active = 1;
        receipt.publishCurrentCapture = 1;
        receipt.captureC127Proof = 1;
        receipt.captureC040Panel = 1;
        receipt.captureC026Portrait = 1;
        receipt.mirrorOrdinal = capture->mirrorOrdinal;
        receipt.candidateChampionOrdinal = capture->candidateChampionOrdinal;
        receipt.sensorGeneration = capture->sensorGeneration;
        receipt.panelGeneration = capture->panelGeneration;
        receipt.captureTick = input->runtimeTick;
        receipt.c040 = capture->c040;
        receipt.c026 = capture->c026;
        *outReceipt = receipt;
        return 1;
    }

    if (!valid_prior(prior) || input->runtimeTick < prior->captureTick) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.captureC040Panel = 1;
    receipt.clearRevoke = 1;
    receipt.mirrorOrdinal = prior->mirrorOrdinal;
    receipt.candidateChampionOrdinal = prior->candidateChampionOrdinal;
    receipt.sensorGeneration = prior->sensorGeneration;
    receipt.panelGeneration = prior->panelGeneration;
    receipt.captureTick = input->runtimeTick;
    receipt.c040 = prior->c040;
    receipt.c026 = prior->c026;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocConfiguredFinalCaptureLifecycle_SourceEvidencePc34(void)
{
    return "ReDMCSB original C127 configuration and C040/C026 source hashes must "
           "match final capture evidence; otherwise the prior portrait is revoked.";
}
