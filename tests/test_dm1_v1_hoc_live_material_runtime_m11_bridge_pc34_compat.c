#include "dm1_v1_hoc_live_material_runtime_m11_bridge_pc34_compat.h"

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

static DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 frame(int revoke)
{
    DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.active = !revoke;
    result.publishCurrent = !revoke; result.clearC040 = 1;
    result.drawC026Portrait = !revoke; result.clearC026Portrait = revoke;
    result.revokeStale = revoke; result.consumedC127 = !revoke;
    result.mirrorOrdinal = 3; result.candidateChampionOrdinal = 4;
    result.sensorGeneration = 21; result.panelGeneration = 18; result.runtimeTick = 102;
    result.c040.sourceOwned = 1; result.c040.graphicIndex = 40;
    result.c040.width = 144; result.c040.height = 73; result.c040.sourceHash = 0x4040u;
    result.c026.sourceOwned = 1; result.c026.graphicIndex = 26;
    result.c026.width = 32; result.c026.height = 29; result.c026.sourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 current = frame(0);
    DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 stale = frame(1);
    DM1_V1_HocLiveMaterialRuntimeM11BridgeInputPc34 input;
    DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.runtimeFrame = &current; input.runtimeTick = 102;
    ok &= expect(DM1_V1_HocLiveMaterialRuntimeM11Bridge_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.currentC127Proof && receipt.publishCurrent &&
                 receipt.drawC026Portrait && !receipt.clearC026Portrait,
                 "current C127/C040/C026 proof admits source portrait toward M11");

    input.runtimeFrame = &stale;
    ok &= expect(DM1_V1_HocLiveMaterialRuntimeM11Bridge_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && !receipt.currentC127Proof && receipt.clearRevoke &&
                 receipt.clearC026Portrait && !receipt.drawC026Portrait,
                 "stale live material admits only C026 clear/revoke toward M11");

    if (!ok) return 1;
    puts("ok: DM1 HoC live material bridge fences current and stale M11 paths");
    return 0;
}
