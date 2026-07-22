#include "dm1_v1_hoc_candidate_runtime_frame_lifecycle_pc34_compat.h"

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

static DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34 admission(uint64_t tick)
{
    DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.admitRuntimeFrame = 1;
    result.commandCount = 2; result.mirrorOrdinal = 3;
    result.sensorGeneration = 17; result.presentedPanelGeneration = 29;
    result.runtimeTick = tick; result.c040SourceHash = 0x4040u;
    result.c026SourceHash = 0x2626u;
    result.commands[0].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34;
    result.commands[0].material.sourceOwned = 1; result.commands[0].material.graphicIndex = 40;
    result.commands[0].material.sourceHash = 0x4040u;
    result.commands[1].kind = DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34;
    result.commands[1].material.sourceOwned = 1; result.commands[1].material.graphicIndex = 26;
    result.commands[1].material.sourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34 first = admission(102);
    DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34 next = admission(103);
    DM1_V1_HocCandidateRuntimeFrameLifecycleInputPc34 input;
    DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 receipt;
    DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 previous;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.currentAdmission = &first; input.runtimeTick = 102;
    ok &= expect(DM1_V1_HocCandidateRuntimeFrameLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.active && receipt.publishCurrent && !receipt.revokeStale,
                 "first source admission publishes runtime frame");

    previous = receipt;
    input.prior = &previous; input.currentAdmission = &next; input.runtimeTick = 103;
    ok &= expect(DM1_V1_HocCandidateRuntimeFrameLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.active && receipt.publishCurrent && receipt.admittedTick == 103,
                 "strictly newer source admission replaces active frame");

    previous = receipt;
    input.prior = &previous; input.currentAdmission = &first; input.runtimeTick = 103;
    ok &= expect(DM1_V1_HocCandidateRuntimeFrameLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && !receipt.active && receipt.revokeStale &&
                 receipt.commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34,
                 "stale admission clears and revokes prior portrait");

    if (!ok) return 1;
    puts("ok: DM1 HoC runtime frame lifecycle publishes or revokes source state");
    return 0;
}
