#include "dm1_v1_hoc_configured_capture_m11_gate_pc34_compat.h"

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

static DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 lifecycle(int revoke)
{
    DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.active = !revoke;
    result.publishCurrentCapture = !revoke; result.captureC127Proof = !revoke;
    result.captureC040Panel = 1; result.captureC026Portrait = !revoke;
    result.clearRevoke = revoke; result.mirrorOrdinal = 3; result.candidateChampionOrdinal = 4;
    result.sensorGeneration = 21; result.panelGeneration = 18; result.captureTick = 102;
    result.c040.sourceOwned = 1; result.c040.graphicIndex = 40;
    result.c040.width = 144; result.c040.height = 73; result.c040.sourceHash = 0x4040u;
    result.c026.sourceOwned = 1; result.c026.graphicIndex = 26;
    result.c026.width = 32; result.c026.height = 29; result.c026.sourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 current = lifecycle(0);
    DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 stale = lifecycle(1);
    DM1_V1_HocConfiguredOriginalEvidencePc34 configuration;
    DM1_V1_HocConfiguredCaptureM11GateInputPc34 input;
    DM1_V1_HocConfiguredCaptureM11GateReceiptPc34 receipt;
    int ok = 1;

    memset(&configuration, 0, sizeof(configuration));
    configuration.sourceOwned = 1; configuration.c127MirrorOrdinal = 3;
    configuration.c040SourceHash = 0x4040u; configuration.c026SourceHash = 0x2626u;
    memset(&input, 0, sizeof(input));
    input.lifecycle = &current; input.configuration = &configuration; input.runtimeTick = 102;
    ok &= expect(DM1_V1_HocConfiguredCaptureM11Gate_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.admitCaptureToM11 && receipt.captureCurrentC127 &&
                 receipt.captureC040Panel && receipt.captureC026Portrait && !receipt.clearRevoke,
                 "configured current C127/C040/C026 enters M11 capture gate");

    input.lifecycle = &stale;
    ok &= expect(DM1_V1_HocConfiguredCaptureM11Gate_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && !receipt.captureCurrentC127 && receipt.captureC040Panel &&
                 !receipt.captureC026Portrait && receipt.clearRevoke,
                 "configured stale state enters gate only as clear/revoke");

    if (!ok) return 1;
    puts("ok: DM1 HoC configured capture gate admits only original source state");
    return 0;
}
