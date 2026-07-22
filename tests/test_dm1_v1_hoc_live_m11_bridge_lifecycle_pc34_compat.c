#include "dm1_v1_hoc_live_m11_bridge_lifecycle_pc34_compat.h"

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

static DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 bridge(uint64_t tick)
{
    DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.admitToM11 = 1;
    result.currentC127Proof = 1; result.publishCurrent = 1;
    result.clearC040 = 1; result.drawC026Portrait = 1;
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
    DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 first = bridge(102);
    DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 next = bridge(103);
    DM1_V1_HocLiveM11BridgeLifecycleInputPc34 input;
    DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 receipt;
    DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 previous;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.currentBridge = &first; input.runtimeTick = 102;
    ok &= expect(DM1_V1_HocLiveM11BridgeLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.active && receipt.currentC127Proof && receipt.drawC026Portrait,
                 "first live bridge publishes C127/C026/C040 source state");

    previous = receipt;
    input.prior = &previous; input.currentBridge = &next; input.runtimeTick = 103;
    ok &= expect(DM1_V1_HocLiveM11BridgeLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.active && receipt.publishCurrent && receipt.admittedTick == 103,
                 "newer live bridge preserves current portrait");

    previous = receipt;
    input.prior = &previous; input.currentBridge = &first; input.runtimeTick = 103;
    ok &= expect(DM1_V1_HocLiveM11BridgeLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && !receipt.active && receipt.clearRevoke &&
                 receipt.clearC026Portrait && !receipt.drawC026Portrait,
                 "stale bridge clears and revokes portrait state");

    if (!ok) return 1;
    puts("ok: DM1 HoC live M11 bridge lifecycle fences stale source output");
    return 0;
}
