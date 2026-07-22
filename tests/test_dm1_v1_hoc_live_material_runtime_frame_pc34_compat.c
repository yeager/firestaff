#include "dm1_v1_hoc_live_material_runtime_frame_pc34_compat.h"

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

static DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34 route(void)
{
    DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.consumedC127 = 1;
    result.admitC026Portrait = 1; result.admitC040Panel = 1;
    result.suppressFallbackVisuals = 1; result.mirrorOrdinal = 3;
    result.candidateChampionOrdinal = 4; result.sensorGeneration = 21; result.panelGeneration = 18;
    result.c040.sourceOwned = 1; result.c040.graphicIndex = 40;
    result.c040.width = 144; result.c040.height = 73; result.c040.sourceHash = 0x4040u;
    result.c026.sourceOwned = 1; result.c026.graphicIndex = 26;
    result.c026.width = 32; result.c026.height = 29; result.c026.sourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34 live = route();
    DM1_V1_HocLiveMaterialRuntimeFrameInputPc34 input;
    DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 receipt;
    DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 previous;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.currentRoute = &live; input.runtimeTick = 102;
    ok &= expect(DM1_V1_HocLiveMaterialRuntimeFrame_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.active && receipt.publishCurrent && receipt.consumedC127 &&
                 receipt.clearC040 && receipt.drawC026Portrait && !receipt.clearC026Portrait,
                 "new live C127/C040/C026 route publishes runtime frame");

    previous = receipt;
    input.prior = &previous; input.runtimeTick = 103;
    ok &= expect(DM1_V1_HocLiveMaterialRuntimeFrame_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.active && receipt.publishCurrent && receipt.runtimeTick == 103,
                 "newer tick preserves live source portrait");

    previous = receipt;
    input.prior = &previous; input.runtimeTick = 103;
    ok &= expect(DM1_V1_HocLiveMaterialRuntimeFrame_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && !receipt.active && receipt.revokeStale && receipt.clearC040 &&
                 receipt.clearC026Portrait && !receipt.drawC026Portrait,
                 "same-tick route is stale and clears source portrait");

    if (!ok) return 1;
    puts("ok: DM1 HoC live material runtime frame fences stale C026 output");
    return 0;
}
