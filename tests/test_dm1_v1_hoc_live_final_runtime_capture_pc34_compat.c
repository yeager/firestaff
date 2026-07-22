#include "dm1_v1_hoc_live_final_runtime_capture_pc34_compat.h"

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

static DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 lifecycle(int revoke)
{
    DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.active = !revoke;
    result.currentC127Proof = !revoke; result.publishCurrent = !revoke;
    result.clearC040 = 1; result.drawC026Portrait = !revoke;
    result.clearC026Portrait = revoke; result.clearRevoke = revoke;
    result.mirrorOrdinal = 3; result.candidateChampionOrdinal = 4;
    result.sensorGeneration = 21; result.panelGeneration = 18; result.admittedTick = 102;
    result.c040.sourceOwned = 1; result.c040.graphicIndex = 40;
    result.c040.width = 144; result.c040.height = 73; result.c040.sourceHash = 0x4040u;
    result.c026.sourceOwned = 1; result.c026.graphicIndex = 26;
    result.c026.width = 32; result.c026.height = 29; result.c026.sourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 current = lifecycle(0);
    DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 stale = lifecycle(1);
    DM1_V1_HocLiveFinalRuntimeCaptureInputPc34 input;
    DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.lifecycle = &current; input.runtimeTick = 102;
    ok &= expect(DM1_V1_HocLiveFinalRuntimeCapture_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.captureCurrentFrame && receipt.captureC127Proof &&
                 receipt.captureC040Panel && receipt.captureC026Portrait && !receipt.captureClearRevoke,
                 "current live lifecycle yields real C127/C040/C026 capture evidence");

    input.lifecycle = &stale;
    ok &= expect(DM1_V1_HocLiveFinalRuntimeCapture_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && !receipt.captureCurrentFrame && !receipt.captureC127Proof &&
                 receipt.captureC040Panel && !receipt.captureC026Portrait && receipt.captureClearRevoke,
                 "stale lifecycle yields only source clear/revoke capture evidence");

    if (!ok) return 1;
    puts("ok: DM1 HoC final capture evidence fences current and stale source state");
    return 0;
}
