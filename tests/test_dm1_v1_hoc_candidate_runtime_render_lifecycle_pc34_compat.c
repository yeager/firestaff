#include "dm1_v1_hoc_candidate_runtime_render_lifecycle_pc34_compat.h"

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

static DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 admission(uint64_t tick)
{
    DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.admitHostRoute = 1;
    result.commandCount = 2; result.mirrorOrdinal = 3;
    result.sensorGeneration = 17; result.presentedPanelGeneration = 29;
    result.renderTick = tick;
    result.commands[0].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34;
    result.commands[0].material.sourceOwned = 1;
    result.commands[0].material.graphicIndex = 40;
    result.commands[0].material.sourceHash = 0x4040u;
    result.commands[1].kind = DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34;
    result.commands[1].material.sourceOwned = 1;
    result.commands[1].material.graphicIndex = 26;
    result.commands[1].material.sourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 current = admission(102);
    DM1_V1_HocCandidateRuntimeRenderLifecycleInputPc34 input;
    DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 receipt;
    DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 previous;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.admission = &current; input.hostTick = 102;
    ok &= expect(DM1_V1_HocCandidateRuntimeRenderLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.consumeC040 && receipt.consumeC026 &&
                 receipt.consumedTick == 102 && receipt.c040SourceHash == 0x4040u &&
                 receipt.c026SourceHash == 0x2626u,
                 "current source C040/C026 admission is consumed once");

    previous = receipt;
    input.prior = &previous;
    ok &= expect(DM1_V1_HocCandidateRuntimeRenderLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "same tick cannot replay host material");

    input.prior = NULL; input.hostTick = 103;
    ok &= expect(DM1_V1_HocCandidateRuntimeRenderLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "admission cannot be consumed on a later host tick");

    if (!ok) return 1;
    puts("ok: DM1 HoC runtime lifecycle fences stale host C040/C026 consumption");
    return 0;
}
