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

static void expect_step(unsigned int ordinal,
                        V1_TitleFrontendSourceEventKind kind,
                        unsigned int vblank,
                        unsigned int zoomIndex,
                        unsigned int x,
                        unsigned int y,
                        unsigned int width,
                        unsigned int height) {
    V1_TitleFrontendSourceAnimationStep step;
    char label[96];

    memset(&step, 0, sizeof(step));
    snprintf(label, sizeof(label), "source step %u exists", ordinal);
    expect_u(label, (unsigned int)V1_TitleFrontend_GetSourceAnimationStep(ordinal, &step), 1u);

    snprintf(label, sizeof(label), "source step %u kind", ordinal);
    expect_u(label, (unsigned int)step.kind, (unsigned int)kind);
    snprintf(label, sizeof(label), "source step %u vblank", ordinal);
    expect_u(label, step.vblankBeforeEvent, vblank);
    snprintf(label, sizeof(label), "source step %u zoom index", ordinal);
    expect_u(label, step.zoomSourceIndex, zoomIndex);
    snprintf(label, sizeof(label), "source step %u x", ordinal);
    expect_u(label, step.x, x);
    snprintf(label, sizeof(label), "source step %u y", ordinal);
    expect_u(label, step.y, y);
    snprintf(label, sizeof(label), "source step %u width", ordinal);
    expect_u(label, step.width, width);
    snprintf(label, sizeof(label), "source step %u height", ordinal);
    expect_u(label, step.height, height);
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

    /* ReDMCSB TITLE.C PC/F20 source audit:
     *   lines 340-360 build shrink bitmaps from 320x80 down to 48x12,
     *   lines 385-387 present them in reverse order with one VBlank each,
     *   lines 395-402 wait two VBlanks and blit Master/Strikes Back,
     *   line 409 adds the BUG0_71 final guard before handoff.
     * Lock representative geometry so the runtime evidence cannot degrade into
     * a count-only cadence probe.
     */
    expect_step(1u, V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS, 0u, 0u, 0u, 90u, 320u, 16u);
    expect_step(2u, V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT, 1u, 17u, 136u, 74u, 48u, 12u);
    expect_step(10u, V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT, 1u, 9u, 72u, 58u, 176u, 44u);
    expect_step(19u, V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT, 1u, 0u, 0u, 40u, 320u, 80u);
    expect_step(20u, V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK, 1u, 0u, 0u, 0u, 0u, 0u);
    expect_step(21u, V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK, 1u, 0u, 0u, 0u, 0u, 0u);
    expect_step(22u, V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT, 0u, 0u, 0u, 118u, 320u, 57u);
    expect_step(23u, V1_TITLE_FRONTEND_SOURCE_EVENT_FINAL_GUARD_VBLANK, 1u, 0u, 0u, 0u, 0u, 0u);
    expect_u("source step 24 is rejected",
             (unsigned int)V1_TitleFrontend_GetSourceAnimationStep(24u, NULL),
             0u);

    if (failures) {
        return 1;
    }
    printf("ok: TITLE runtime cadence uses ReDMCSB timing and geometry evidence\n");
    return 0;
}
