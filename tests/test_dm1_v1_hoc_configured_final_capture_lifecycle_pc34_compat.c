#include "dm1_v1_hoc_configured_final_capture_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

static DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 capture(uint64_t tick)
{
    DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.captureCurrentFrame = 1;
    result.captureC127Proof = 1; result.captureC040Panel = 1; result.captureC026Portrait = 1;
    result.mirrorOrdinal = 3; result.candidateChampionOrdinal = 4;
    result.sensorGeneration = 21; result.panelGeneration = 18; result.runtimeTick = tick;
    result.c040.sourceOwned = 1; result.c040.graphicIndex = 40;
    result.c040.width = 144; result.c040.height = 73; result.c040.sourceHash = 0x4040u;
    result.c026.sourceOwned = 1; result.c026.graphicIndex = 26;
    result.c026.width = 32; result.c026.height = 29; result.c026.sourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 first = capture(102);
    DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 next = capture(103);
    DM1_V1_HocConfiguredOriginalEvidencePc34 configuration;
    DM1_V1_HocConfiguredFinalCaptureLifecycleInputPc34 input;
    DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 receipt;
    DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 previous;
    int ok = 1;

    memset(&configuration, 0, sizeof(configuration));
    configuration.sourceOwned = 1; configuration.c127MirrorOrdinal = 3;
    configuration.c040SourceHash = 0x4040u; configuration.c026SourceHash = 0x2626u;
    memset(&input, 0, sizeof(input));
    input.currentCapture = &first; input.configuration = &configuration; input.runtimeTick = 102;
    ok &= expect(DM1_V1_HocConfiguredFinalCaptureLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.active && receipt.publishCurrentCapture && receipt.captureC127Proof,
                 "configured original C127/C040/C026 starts final capture lifecycle");

    previous = receipt;
    input.prior = &previous; input.currentCapture = &next; input.runtimeTick = 103;
    ok &= expect(DM1_V1_HocConfiguredFinalCaptureLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.active && receipt.captureTick == 103,
                 "newer configured original capture remains active");

    previous = receipt;
    configuration.c026SourceHash = 0x2627u;
    input.prior = &previous; input.currentCapture = &next; input.runtimeTick = 103;
    ok &= expect(DM1_V1_HocConfiguredFinalCaptureLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && !receipt.active && receipt.clearRevoke && !receipt.captureC026Portrait,
                 "unconfigured source hash revokes stale capture instead of falling back");

    if (!ok) return 1;
    puts("ok: DM1 HoC configured final capture lifecycle fences source evidence");
    return 0;
}
