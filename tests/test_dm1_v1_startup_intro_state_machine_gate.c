#include "main_loop_m11.h"
#include "swsh_frontend_pc34_compat.h"
#include "title_frontend_v1.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* IMG3 globals are required by firestaff_m10 when this focused gate links
 * the full M11 runtime libraries through main_loop_m11.c. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

typedef enum DM1V1StartupStage {
    DM1_V1_STARTUP_STAGE_SWSH_LOGO = 1,
    DM1_V1_STARTUP_STAGE_SWSH_RUN_START = 2,
    DM1_V1_STARTUP_STAGE_TITLE_BEGIN = 3,
    DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME = 4,
    DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE = 5,
    DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT = 6
} DM1V1StartupStage;

static int g_failures = 0;

static void expect_u(const char* label, unsigned int got, unsigned int want) {
    if (got != want) {
        printf("FAIL %s: got %u want %u\n", label, got, want);
        g_failures++;
    }
}

static void expect_i(const char* label, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d want %d\n", label, got, want);
        g_failures++;
    }
}

static void expect_truth(const char* label, int got) {
    if (!got) {
        printf("FAIL %s\n", label);
        g_failures++;
    }
}

static void expect_stage_after(const char* label,
                               DM1V1StartupStage later,
                               DM1V1StartupStage earlier) {
    if ((unsigned int)later <= (unsigned int)earlier) {
        printf("FAIL %s: later %u earlier %u\n",
               label,
               (unsigned int)later,
               (unsigned int)earlier);
        g_failures++;
    }
}

static void check_swsh_to_title_boundary(void) {
    SWSH_CompatSourceTiming swshTiming = SWSH_Compat_GetSourceTimingEvidence();
    SWSH_CompatSourceAnimationStep first;
    SWSH_CompatSourceAnimationStep last;

    memset(&first, 0, sizeof(first));
    memset(&last, 0, sizeof(last));

    /* ReDMCSB PC startup chain:
     *   APPA.C:51 runs FTL_SWSH, then APPA.C:52-53 passes FTL_TITL to
     *   FTL_ANIM; SWSH.C:39-47 runs START.PRG only after the palette
     *   command terminator, and STARTUP1.C:143 then calls
     *   F0437_STARTEND_DrawTitle().
     */
    expect_u("SWSH source file is available", swshTiming.sourceFile != 0, 1u);
    expect_u("SWSH source function is available", swshTiming.sourceFunction != 0, 1u);
    expect_u("SWSH source step count", SWSH_Compat_GetSourceAnimationStepCount(), 29u);
    expect_i("SWSH step zero rejected", SWSH_Compat_GetSourceAnimationStep(0u, &first), 0);
    expect_i("SWSH first step exists", SWSH_Compat_GetSourceAnimationStep(1u, &first), 1);
    expect_i("SWSH last step exists",
             SWSH_Compat_GetSourceAnimationStep(SWSH_Compat_GetSourceAnimationStepCount(), &last),
             1);
    expect_u("SWSH first step loads logo",
             (unsigned int)first.kind,
             (unsigned int)SWSH_COMPAT_SOURCE_EVENT_LOAD_LOGO_BITMAP);
    expect_u("SWSH last step runs START",
             (unsigned int)last.kind,
             (unsigned int)SWSH_COMPAT_SOURCE_EVENT_RUN_START_PROGRAM);
    expect_stage_after("TITLE begins only after SWSH run-start",
                       DM1_V1_STARTUP_STAGE_TITLE_BEGIN,
                       DM1_V1_STARTUP_STAGE_SWSH_RUN_START);
}

static void check_title_to_menu_boundary(void) {
    V1_TitleFrontendSourceAnimationStep sourceStep;
    V1_TitleFrontendSourceTiming titleTiming = V1_TitleFrontend_GetSourceTimingEvidence();
    V1_TitleFrontendHandoffDecision firstTitle =
        V1_TitleFrontend_DecideTitleMenuHandoffStep(1u, 1);
    V1_TitleFrontendHandoffDecision lastTitle =
        V1_TitleFrontend_DecideTitleMenuHandoffStep(V1_TITLE_DAT_FRAME_MAX, 1);
    V1_TitleFrontendHandoffDecision menu =
        V1_TitleFrontend_DecideTitleMenuHandoffStep(V1_TITLE_DAT_FRAME_MAX + 1u, 1);
    V1_TitleFrontendHandoffDecision held =
        V1_TitleFrontend_DecideTitleMenuHandoffStep(V1_TITLE_DAT_FRAME_MAX + 1u, 0);

    memset(&sourceStep, 0, sizeof(sourceStep));

    /* ReDMCSB TITLE.C F0437 lines 319-324 draw PRESENTS, lines 385-387
     * run the title zoom blits, lines 395-409 complete the post-zoom,
     * STRIKES BACK, and final VBlank guard. Firestaff's decoded TITLE
     * frame bank must not make the menu eligible before frame 53 is held.
     */
    expect_u("TITLE source step count", titleTiming.sourceAnimationStepCount, 23u);
    expect_i("TITLE source step 1 exists",
             V1_TitleFrontend_GetSourceAnimationStep(1u, &sourceStep),
             1);
    expect_u("TITLE source step 1 is PRESENTS",
             (unsigned int)sourceStep.kind,
             (unsigned int)V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS);

    expect_u("TITLE first frame ordinal", firstTitle.title.renderFrameOrdinal, 1u);
    expect_u("TITLE first surface is title",
             (unsigned int)firstTitle.surface,
             (unsigned int)V1_TITLE_FRONTEND_SURFACE_TITLE);
    expect_i("TITLE first frame not menu", firstTitle.enteredMenuAfterHandoff, 0);

    expect_u("TITLE last frame ordinal", lastTitle.title.renderFrameOrdinal, V1_TITLE_DAT_FRAME_MAX);
    expect_i("TITLE last frame is handoff-ready", lastTitle.title.handoffReady, 1);
    expect_u("TITLE last frame still title surface",
             (unsigned int)lastTitle.surface,
             (unsigned int)V1_TITLE_FRONTEND_SURFACE_TITLE);
    expect_i("TITLE last frame has not entered menu", lastTitle.enteredMenuAfterHandoff, 0);

    expect_u("TITLE post-boundary frame holds last frame",
             menu.title.renderFrameOrdinal,
             V1_TITLE_DAT_FRAME_MAX);
    expect_u("TITLE post-boundary action holds",
             (unsigned int)menu.title.action,
             (unsigned int)V1_TITLE_FRONTEND_SEQUENCE_HOLD_LAST_FRAME);
    expect_u("TITLE post-boundary surface is menu",
             (unsigned int)menu.surface,
             (unsigned int)V1_TITLE_FRONTEND_SURFACE_MENU);
    expect_i("TITLE post-boundary entered menu", menu.enteredMenuAfterHandoff, 1);

    expect_u("TITLE explicit hold stays title surface",
             (unsigned int)held.surface,
             (unsigned int)V1_TITLE_FRONTEND_SURFACE_TITLE);
    expect_i("TITLE explicit hold does not enter menu", held.enteredMenuAfterHandoff, 0);

    expect_stage_after("last TITLE frame follows first TITLE frame",
                       DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME,
                       DM1_V1_STARTUP_STAGE_TITLE_BEGIN);
    expect_stage_after("menu follows last TITLE frame",
                       DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE,
                       DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME);
}

static void check_menu_to_entrance_wait_boundary(void) {
    /* ReDMCSB ENTRANCE.C:850-883 discards prior input, sets
     * C099_MODE_WAITING_ON_ENTRANCE, then waits for a fresh command.
     * The launcher/title key or click that reaches menu eligibility must not
     * be recycled as the first ENTER_DUNGEON command.
     */
    expect_i("interactive entrance does not auto-enter after title/menu handoff",
             M11_Entrance_ShouldAutoEnterForTimeout(0, 1200, 6000u),
             0);
    expect_i("headless entrance may auto-enter after explicit timeout",
             M11_Entrance_ShouldAutoEnterForTimeout(1, 1200, 1201u),
             1);
    expect_stage_after("entrance wait follows menu eligibility",
                       DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT,
                       DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE);
}

int main(void) {
    check_swsh_to_title_boundary();
    check_title_to_menu_boundary();
    check_menu_to_entrance_wait_boundary();

    expect_truth("startup stage order is monotonic",
                 DM1_V1_STARTUP_STAGE_SWSH_LOGO <
                 DM1_V1_STARTUP_STAGE_SWSH_RUN_START &&
                 DM1_V1_STARTUP_STAGE_SWSH_RUN_START <
                 DM1_V1_STARTUP_STAGE_TITLE_BEGIN &&
                 DM1_V1_STARTUP_STAGE_TITLE_BEGIN <
                 DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME &&
                 DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME <
                 DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE &&
                 DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE <
                 DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT);

    if (g_failures) {
        printf("dm1_v1_startup_intro_state_machine_gate failures=%d\n", g_failures);
        return 1;
    }
    printf("ok: DM1 V1 startup intro state-machine ordering is data-free and deterministic\n");
    return 0;
}
