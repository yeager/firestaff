#include "title_frontend_v1.h"
#include "dm1_v2_anim_timing.h"
#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"

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
    unsigned char packed[4];
    unsigned char indexed[12];

    memset(&zero, 0, sizeof(zero));
    memset(packed, 0, sizeof(packed));
    memset(indexed, 0xee, sizeof(indexed));

    expect_u("source zoom step count",
             timing.zoomStepCount,
             dm1_v1_startup_title_zoom_steps_pc34());
    expect_u("source animation step count",
             timing.sourceAnimationStepCount,
             dm1_v1_startup_title_source_animation_steps_pc34());
    expect_u("DM1 startup contract matches TITLE source animation count",
             dm1_v1_startup_title_source_animation_steps_pc34(),
             V1_TitleFrontend_GetSourceAnimationStepCount());
    expect_u("source PRESENTS hold uses hidden C001 build/pad budget",
             timing.presentsHoldVblankCount,
             dm1_v1_startup_title_presents_hold_vblanks_pc34());
    expect_u("C001 cadence pad target matches TITLE frame-bank cadence",
             timing.frameBankEquivalentStepCount,
             dm1_v1_startup_title_frame_bank_equivalent_steps_pc34());
    expect_u("runtime frame delay from source vblank cadence",
             V1_TitleFrontend_GetRuntimeFrameDelayMs(&timing),
             dm1_v1_startup_title_vblank_tick_ms_pc34());
    expect_u("runtime PRESENTS hold prevents one-tick flash",
             V1_TitleFrontend_GetRuntimePresentsHoldDelayMs(&timing),
             dm1_v1_startup_title_presents_hold_ms_pc34());
    expect_u("runtime final guard delay from source post/final vblanks",
             V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&timing),
             (dm1_v1_startup_title_post_zoom_vblanks_pc34() +
              dm1_v1_startup_title_final_guard_vblanks_pc34()) *
                 dm1_v1_startup_title_vblank_tick_ms_pc34());
    expect_u("runtime C001 cadence pad moved before zoom",
             V1_TitleFrontend_GetRuntimeC001CadencePadDelayMs(&timing),
             0u);
    /* ReDMCSB TITLE.C F0437 waits through the zoom/post-zoom/final-guard
     * VBlank path, and Firestaff pads the shorter GRAPHICS.DAT C001 step
     * schedule to the same finite cadence as the existing 53-frame TITLE
     * bank path so fast machines cannot collapse the intro into a burst. */
    expect_u("runtime fallback frame delay stays deliberate, not zero-speed",
             V1_TitleFrontend_GetRuntimeFrameDelayMs(&zero),
             50u);
    expect_u("runtime null final guard delay is safe",
             V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(NULL),
             0u);
    expect_u("runtime null PRESENTS hold delay is safe",
             V1_TitleFrontend_GetRuntimePresentsHoldDelayMs(NULL),
             0u);
    expect_u("runtime null C001 cadence pad is safe",
             V1_TitleFrontend_GetRuntimeC001CadencePadDelayMs(NULL),
             0u);

    /* ReDMCSB TITLE.C PC/F20 source audit:
     *   lines 340-360 build shrink bitmaps from 320x80 down to 48x12,
     *   lines 385-387 present them in reverse order with one VBlank each,
     *   lines 395-402 wait two VBlanks and blit Master/Strikes Back,
     *   line 409 adds the BUG0_71 final guard before handoff.
     * Firestaff maps those VBlanks onto the same 55 ms V1 tick used by the game
     * clock; 20 ms made the TITLE zoom visibly race on modern displays. Lock
     * representative geometry so the runtime evidence cannot degrade into a
     * count-only cadence probe.
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

    packed[0] = 0x12u;
    packed[1] = 0x34u;
    packed[2] = 0xabu;
    packed[3] = 0xcdu;
    expect_u("TITLE 4bpp unpack accepts valid even-width surface",
             (unsigned int)V1_TitleFrontend_Unpack4bppScreenToIndexed(
                 packed, 4u, 2u, indexed, 6u),
             1u);
    expect_u("TITLE 4bpp row0 pixel0", indexed[0], 1u);
    expect_u("TITLE 4bpp row0 pixel1", indexed[1], 2u);
    expect_u("TITLE 4bpp row0 pixel2", indexed[2], 3u);
    expect_u("TITLE 4bpp row0 pixel3", indexed[3], 4u);
    expect_u("TITLE 4bpp stride gap preserved", indexed[4], 0xeeu);
    expect_u("TITLE 4bpp row1 pixel0", indexed[6], 0x0au);
    expect_u("TITLE 4bpp row1 pixel1", indexed[7], 0x0bu);
    expect_u("TITLE 4bpp row1 pixel2", indexed[8], 0x0cu);
    expect_u("TITLE 4bpp row1 pixel3", indexed[9], 0x0du);
    expect_u("TITLE 4bpp rejects odd width",
             (unsigned int)V1_TitleFrontend_Unpack4bppScreenToIndexed(
                 packed, 3u, 2u, indexed, 6u),
             0u);
    expect_u("TITLE 4bpp rejects short stride",
             (unsigned int)V1_TitleFrontend_Unpack4bppScreenToIndexed(
                 packed, 4u, 2u, indexed, 3u),
             0u);

    if (failures) {
        return 1;
    }
    printf("ok: TITLE runtime cadence uses ReDMCSB timing and geometry evidence\n");
    return 0;
}
