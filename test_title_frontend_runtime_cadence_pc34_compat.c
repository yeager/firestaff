#include "title_frontend_v1.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void expect_u(const char* label, unsigned int got, unsigned int want) {
    if (got != want) {
        printf("FAIL %s: got %u want %u\n", label, got, want);
        failures++;
    }
}

int main(void) {
    V1_TitleFrontendSourceTiming timing = V1_TitleFrontend_GetSourceTimingEvidence();
    V1_TitleFrontendSourceTiming zero;

    memset(&zero, 0, sizeof(zero));

    expect_u("source zoom step count", timing.zoomStepCount, 18u);
    expect_u("source animation step count", timing.sourceAnimationStepCount, 23u);
    expect_u("runtime frame delay from source vblank cadence",
             V1_TitleFrontend_GetRuntimeFrameDelayMs(&timing),
             20u);
    expect_u("runtime final guard delay from source post/final vblanks",
             V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&timing),
             60u);
    expect_u("runtime fallback frame delay stays deliberate, not zero-speed",
             V1_TitleFrontend_GetRuntimeFrameDelayMs(&zero),
             50u);
    expect_u("runtime null final guard delay is safe",
             V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(NULL),
             0u);

    if (failures) {
        return 1;
    }
    printf("ok: TITLE runtime cadence uses ReDMCSB timing evidence\n");
    return 0;
}
