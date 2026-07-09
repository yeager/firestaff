#include "main_loop_m11.h"
#include "entrance_frontend_pc34_compat.h"
#include "swsh_frontend_pc34_compat.h"
#include "title_frontend_v1.h"
#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* IMG3 globals are required by firestaff_m10 when this focused gate links
 * the full M11 runtime libraries through main_loop_m11.c. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

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
                               DM1_V1_StartupStage_PC34 later,
                               DM1_V1_StartupStage_PC34 earlier) {
    if (!dm1_v1_startup_stage_after_pc34(later, earlier)) {
        printf("FAIL %s: later %s earlier %s\n",
               label,
               dm1_v1_startup_stage_name_pc34(later),
               dm1_v1_startup_stage_name_pc34(earlier));
        g_failures++;
    }
}

typedef struct FakeDm1StartupCallbacks {
    char order[128];
    unsigned int order_len;
    int title_played;
    int entrance_command;
    int entrance_timeout_ms;
    int active;
    int resolve_ok;
    int load_ok;
    int used_backup;
    int log_loaded;
    int log_missing;
    int log_skipped;
    int open_ok;
    int after_open;
    int draw_opened;
    int mark_failed;
    int prelude_begin_count;
    int prelude_end_count;
    int post_begin_count;
    int post_end_count;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 prelude_media;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 post_media;
    DM1_V1_EntranceFullStartRenderReceiptPc34 post_entrance;
    char opened_source_id[64];
    char resolved_path[512];
} FakeDm1StartupCallbacks;

static void fake_append(FakeDm1StartupCallbacks* fake, char token) {
    if (!fake || fake->order_len + 1U >= sizeof(fake->order)) {
        return;
    }
    fake->order[fake->order_len++] = token;
    fake->order[fake->order_len] = '\0';
}

static int fake_report_source_order_failure(void* user, const char* evidence) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    (void)evidence;
    fake_append(fake, 'F');
    return 1;
}

static int fake_raise_window(void* user) {
    fake_append((FakeDm1StartupCallbacks*)user, 'R');
    return 1;
}

static int fake_play_swsh(void* user, const char* game_id, int preserve_audio) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    (void)game_id;
    (void)preserve_audio;
    fake_append(fake, 'S');
    return 1;
}

static int fake_discard_presentation(void* user) {
    fake_append((FakeDm1StartupCallbacks*)user, 'D');
    return 1;
}

static int fake_begin_prelude_plan(
    void* user,
    const DM1_V1_StartupHandoffPreludePlan_PC34* plan) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    if (!fake || !plan) {
        return 0;
    }
    fake->prelude_begin_count++;
    fake->prelude_media = plan->media_receipt;
    return 1;
}

static int fake_end_prelude_plan(void* user) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    if (!fake) {
        return 0;
    }
    fake->prelude_end_count++;
    return 1;
}

static int fake_begin_post_launch_plan(
    void* user,
    const DM1_V1_StartupHandoffPostLaunchPlan_PC34* plan) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    if (!fake || !plan) {
        return 0;
    }
    fake->post_begin_count++;
    fake->post_media = plan->media_receipt;
    fake->post_entrance = plan->entrance_full_start_receipt;
    return 1;
}

static int fake_end_post_launch_plan(void* user) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    if (!fake) {
        return 0;
    }
    fake->post_end_count++;
    return 1;
}

static int fake_play_title(void* user,
                           const char* source_id,
                           int* out_played_any_frame) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    (void)source_id;
    fake_append(fake, 'T');
    fake->title_played = 1;
    if (out_played_any_frame) {
        *out_played_any_frame = 1;
    }
    return 1;
}

static int fake_play_entrance(void* user,
                              const char* source_id,
                              int auto_enter_after_ms,
                              int* out_entrance_command) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    (void)source_id;
    fake_append(fake, 'E');
    fake->entrance_timeout_ms = auto_enter_after_ms;
    if (out_entrance_command) {
        *out_entrance_command = fake->entrance_command;
    }
    return 1;
}

static DM1_V1_StartupHandoffCallbacks_PC34 fake_callbacks(
    FakeDm1StartupCallbacks* fake) {
    DM1_V1_StartupHandoffCallbacks_PC34 callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.user = fake;
    callbacks.begin_prelude_plan = fake_begin_prelude_plan;
    callbacks.end_prelude_plan = fake_end_prelude_plan;
    callbacks.begin_post_launch_plan = fake_begin_post_launch_plan;
    callbacks.end_post_launch_plan = fake_end_post_launch_plan;
    callbacks.report_source_order_failure = fake_report_source_order_failure;
    callbacks.raise_window = fake_raise_window;
    callbacks.play_swsh = fake_play_swsh;
    callbacks.discard_presentation_texture = fake_discard_presentation;
    callbacks.play_title = fake_play_title;
    callbacks.play_entrance = fake_play_entrance;
    return callbacks;
}

static int fake_set_game_active(void* user, int active) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    fake->active = active;
    return 1;
}

static int fake_resolve_resume(void* user,
                               const char* source_id,
                               char* out_path,
                               int out_path_size) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    (void)source_id;
    if (!fake->resolve_ok || !out_path || out_path_size <= 0) {
        return 0;
    }
    snprintf(out_path, (size_t)out_path_size, "%s", fake->resolved_path);
    return 1;
}

static int fake_load_resume(void* user,
                            const char* save_path,
                            int* out_used_backup) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    (void)save_path;
    if (!fake->load_ok) {
        return 0;
    }
    if (out_used_backup) {
        *out_used_backup = fake->used_backup;
    }
    return 1;
}

static int fake_log_resume_loaded(void* user,
                                  const char* save_path,
                                  int used_backup) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    (void)save_path;
    (void)used_backup;
    fake->log_loaded = 1;
    return 1;
}

static int fake_log_resume_missing(void* user, const char* save_path) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    (void)save_path;
    fake->log_missing = 1;
    return 1;
}

static int fake_log_entrance_skipped(void* user) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    fake->log_skipped = 1;
    return 1;
}

static DM1_V1_StartupHostCallbacks_PC34 fake_host_callbacks(
    FakeDm1StartupCallbacks* fake) {
    DM1_V1_StartupHostCallbacks_PC34 callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.user = fake;
    callbacks.set_game_active = fake_set_game_active;
    callbacks.resolve_resume_save_path = fake_resolve_resume;
    callbacks.load_resume_save_path = fake_load_resume;
    callbacks.log_resume_loaded = fake_log_resume_loaded;
    callbacks.log_resume_missing = fake_log_resume_missing;
    callbacks.log_entrance_skipped = fake_log_entrance_skipped;
    return callbacks;
}

static int fake_open_selected_entry(void* user,
                                    char* out_source_id,
                                    int out_source_id_size) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    fake_append(fake, 'O');
    if (!fake->open_ok) {
        return 0;
    }
    if (out_source_id && out_source_id_size > 0) {
        snprintf(out_source_id,
                 (size_t)out_source_id_size,
                 "%s",
                 fake->opened_source_id[0] ? fake->opened_source_id : "dm1");
    }
    return 1;
}

static int fake_after_open(void* user) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    fake_append(fake, 'A');
    fake->after_open = 1;
    return 1;
}

static int fake_draw_opened(void* user) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    fake_append(fake, 'G');
    fake->draw_opened = 1;
    return 1;
}

static int fake_mark_launch_failed(void* user) {
    FakeDm1StartupCallbacks* fake = (FakeDm1StartupCallbacks*)user;
    fake_append(fake, 'M');
    fake->mark_failed = 1;
    return 1;
}

static DM1_V1_StartupSelectedLaunchCallbacks_PC34 fake_selected_launch_callbacks(
    FakeDm1StartupCallbacks* fake,
    const DM1_V1_StartupHandoffCallbacks_PC34* handoff_callbacks,
    const DM1_V1_StartupHostCallbacks_PC34* host_callbacks) {
    DM1_V1_StartupSelectedLaunchCallbacks_PC34 callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.user = fake;
    callbacks.handoff_callbacks = handoff_callbacks;
    callbacks.host_callbacks = host_callbacks;
    callbacks.open_selected_entry = fake_open_selected_entry;
    callbacks.after_open = fake_after_open;
    callbacks.draw_opened = fake_draw_opened;
    callbacks.mark_launch_failed = fake_mark_launch_failed;
    return callbacks;
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
    expect_u("SWSH palette command count",
             swshTiming.paletteCommandCount,
             SWSH_COMPAT_SOURCE_PALETTE_COMMAND_COUNT);
    expect_u("SWSH palette color-set count",
             swshTiming.paletteColorSetCount,
             SWSH_COMPAT_SOURCE_PALETTE_COLOR_SET_COUNT);
    expect_u("SWSH palette wait command count",
             swshTiming.paletteWaitCommandCount,
             SWSH_COMPAT_SOURCE_PALETTE_WAIT_COMMAND_COUNT);
    expect_u("SWSH palette wait vblank count",
             swshTiming.paletteWaitVblankCount,
             SWSH_COMPAT_SOURCE_PALETTE_WAIT_VBLANK_COUNT);
    expect_u("SWSH sound register write count",
             swshTiming.soundRegisterWriteCount,
             SWSH_COMPAT_SOURCE_SOUND_REGISTER_WRITE_COUNT);
    expect_u("SWSH sound wait command count",
             swshTiming.soundWaitCommandCount,
             SWSH_COMPAT_SOURCE_SOUND_WAIT_COMMAND_COUNT);
    expect_u("SWSH sound wait vblank count",
             swshTiming.soundWaitVblankCount,
             SWSH_COMPAT_SOURCE_SOUND_WAIT_VBLANK_COUNT);
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
                       DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34,
                       DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34);
}

static void check_title_to_menu_boundary(void) {
    V1_TitleFrontendSourceAnimationStep sourceStep;
    V1_TitleFrontendSourceAnimationStep finalGuard;
    DM1_V1_StartupTitleMenuEligibilityFacts_PC34 facts;
    DM1_V1_StartupTitleMenuEligibilityReceipt_PC34 receipt;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 media;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 badMedia;
    DM1_V1_StartupEntranceRenderAudioCommand_PC34 entranceCommand;
    EntranceCompatSourceAnimationStep entranceStep;
    EntranceCompatSourceAnimationStep doorStep;
    EntranceCompatSourceAnimationStep closedStep;
    V1_TitleFrontendSourceTiming titleTiming = V1_TitleFrontend_GetSourceTimingEvidence();
    int expected_presents_palette = 0;
    int expected_title_palette = 0;
    V1_TitleFrontendHandoffDecision firstTitle =
        V1_TitleFrontend_DecideTitleMenuHandoffStep(1u, 1);
    V1_TitleFrontendHandoffDecision lastTitle =
        V1_TitleFrontend_DecideTitleMenuHandoffStep(V1_TITLE_DAT_FRAME_MAX, 1);
    V1_TitleFrontendHandoffDecision menu =
        V1_TitleFrontend_DecideTitleMenuHandoffStep(V1_TITLE_DAT_FRAME_MAX + 1u, 1);
    V1_TitleFrontendHandoffDecision held =
        V1_TitleFrontend_DecideTitleMenuHandoffStep(V1_TITLE_DAT_FRAME_MAX + 1u, 0);

    memset(&sourceStep, 0, sizeof(sourceStep));
    memset(&finalGuard, 0, sizeof(finalGuard));
    memset(&entranceStep, 0, sizeof(entranceStep));
    memset(&doorStep, 0, sizeof(doorStep));
    memset(&closedStep, 0, sizeof(closedStep));
    memset(&entranceCommand, 0, sizeof(entranceCommand));
    memset(&media, 0, sizeof(media));
    memset(&badMedia, 0, sizeof(badMedia));
    (void)V1_TitleFrontend_GetStepPalette(
        V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS,
        &expected_presents_palette);
    (void)V1_TitleFrontend_GetStepPalette(
        V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT,
        &expected_title_palette);

    /* ReDMCSB TITLE.C F0437 lines 319-324 draw PRESENTS, lines 385-387
     * run the title zoom blits, lines 395-409 complete the post-zoom,
     * STRIKES BACK, and final VBlank guard. Firestaff's decoded TITLE
     * frame bank must not make the menu eligible before frame 53 is held.
     */
    expect_u("TITLE source step count",
             titleTiming.sourceAnimationStepCount,
             dm1_v1_startup_title_source_animation_steps_pc34());
    expect_u("TITLE PRESENTS hold uses C001 preparation budget",
             titleTiming.presentsHoldVblankCount,
             dm1_v1_startup_title_presents_hold_vblanks_pc34());
    expect_u("TITLE first menu-eligible step",
             titleTiming.firstMenuEligibleStep,
             V1_TITLE_DAT_FRAME_MAX + 1u);
    expect_i("TITLE source step 1 exists",
             V1_TitleFrontend_GetSourceAnimationStep(1u, &sourceStep),
             1);
    expect_u("TITLE source step 1 is PRESENTS",
             (unsigned int)sourceStep.kind,
             (unsigned int)V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS);
    expect_i("TITLE final source guard exists",
             V1_TitleFrontend_GetSourceAnimationStep(titleTiming.sourceAnimationStepCount,
                                                     &finalGuard),
             1);
    expect_u("TITLE final source step is guard vblank",
             (unsigned int)finalGuard.kind,
             (unsigned int)V1_TITLE_FRONTEND_SOURCE_EVENT_FINAL_GUARD_VBLANK);
    expect_u("TITLE final source guard waits one vblank",
             finalGuard.vblankBeforeEvent,
             1u);
    expect_i("TITLE source step after guard is rejected",
             V1_TitleFrontend_GetSourceAnimationStep(titleTiming.sourceAnimationStepCount + 1u,
                                                     &sourceStep),
             0);
    expect_i("DM1 full graphics media receipt builds",
             dm1_v1_startup_full_graphics_media_receipt_pc34("dm1", &media),
             1);
    expect_i("DM1 full graphics media receipt handled",
             media.handled,
             1);
    expect_i("DM1 full graphics media receipt plays swsh/title/entrance",
             media.play_swsh && media.play_title && media.play_entrance,
             1);
    expect_u("DM1 full graphics media receipt keeps SWSH logo hold",
             media.swsh_initial_logo_hold_ms,
             SWSH_Compat_GetRuntimeInitialLogoHoldMs());
    expect_u("DM1 full graphics media receipt exposes SWSH vblank",
             media.swsh_vblank_ms,
             SWSH_COMPAT_RUNTIME_VBLANK_MS);
    expect_u("DM1 full graphics media receipt keeps SWSH palette waits",
             media.swsh_palette_wait_ms,
             SWSH_Compat_GetRuntimeDelayMsForVblankCount(
                 SWSH_COMPAT_SOURCE_PALETTE_WAIT_VBLANK_COUNT));
    expect_u("DM1 full graphics media receipt keeps TITLE PRESENTS hold",
             media.title_presents_hold_ms,
             V1_TitleFrontend_GetRuntimePresentsHoldDelayMs(&titleTiming));
    expect_u("DM1 full graphics media receipt keeps TITLE zoom delay",
             media.title_zoom_frame_delay_ms,
             V1_TitleFrontend_GetRuntimeFrameDelayMs(&titleTiming));
    expect_u("DM1 full graphics media receipt keeps TITLE C001 pad",
             media.title_c001_cadence_pad_ms,
             V1_TitleFrontend_GetRuntimeC001CadencePadDelayMs(&titleTiming));
    expect_u("DM1 full graphics media receipt menu boundary",
             media.title_menu_boundary_frame,
             V1_TITLE_DAT_FRAME_MAX + 1u);
    expect_i("DM1 full graphics media receipt PRESENTS palette",
             media.title_presents_palette,
             expected_presents_palette);
    expect_i("DM1 full graphics media receipt title palette",
             media.title_zoom_palette,
             expected_title_palette);
    expect_u("DM1 full graphics media receipt entrance source steps",
             media.entrance_source_animation_steps,
             ENTRANCE_Compat_GetSourceAnimationStepCount());
    expect_u("DM1 full graphics media receipt entrance door steps",
             media.entrance_door_step_count,
             ENTRANCE_Compat_GetDoorAnimationStepCount());
    expect_u("DM1 full graphics media receipt entrance vblank",
             media.entrance_vblank_ms,
             ENTRANCE_Compat_GetVblankDelayMs());
    expect_i("DM1 full graphics media receipt entrance pre-open step exists",
             ENTRANCE_Compat_GetSourceAnimationStep(6u, &entranceStep) &&
                 entranceStep.kind == ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY,
             1);
    expect_u("DM1 full graphics media receipt entrance pre-open delay",
             media.entrance_pre_open_delay_ms,
             ENTRANCE_Compat_GetRuntimeDelayMs(&entranceStep));
    expect_i("DM1 full graphics media receipt entrance timing valid",
             dm1_v1_startup_entrance_timing_receipt_valid_pc34(&media),
             1);
    expect_i("DM1 full graphics media receipt entrance door step exists",
             ENTRANCE_Compat_GetSourceAnimationStep(7u, &doorStep) &&
                 doorStep.kind == ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP,
             1);
    expect_i("DM1 full graphics media receipt closed-door step exists",
             ENTRANCE_Compat_GetSourceAnimationStep(3u, &closedStep) &&
                 closedStep.kind ==
                     ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_ENTRANCE_SCREEN,
             1);
    expect_u("DM1 entrance receipt delay helper uses vblank timing",
             dm1_v1_startup_entrance_step_delay_ms_pc34(
                 &media,
                 (int)doorStep.kind,
                 doorStep.delayTicks,
                 doorStep.vblankLoopCount),
             media.entrance_vblank_ms);
    expect_u("DM1 entrance receipt delay helper uses pre-open timing",
             dm1_v1_startup_entrance_step_delay_ms_pc34(
                 &media,
                 (int)entranceStep.kind,
                 entranceStep.delayTicks,
                 entranceStep.vblankLoopCount),
             media.entrance_pre_open_delay_ms);
    badMedia = media;
    badMedia.entrance_vblank_ms = 1u;
    expect_i("DM1 entrance timing validator rejects too-fast vblank",
             dm1_v1_startup_entrance_timing_receipt_valid_pc34(&badMedia),
             0);
    badMedia = media;
    badMedia.entrance_pre_open_delay_ms = 1u;
    expect_i("DM1 entrance timing validator rejects too-fast pre-open",
             dm1_v1_startup_entrance_timing_receipt_valid_pc34(&badMedia),
             0);
    expect_i("DM1 entrance command builds closed-door render",
             dm1_v1_startup_entrance_render_audio_command_pc34(
                 &media,
                 closedStep.sourceStepOrdinal,
                 (int)closedStep.kind,
                 closedStep.delayTicks,
                 closedStep.vblankLoopCount,
                 &entranceCommand) &&
                 entranceCommand.render_kind ==
                     DM1_V1_STARTUP_ENTRANCE_RENDER_CLOSED_DOORS_PC34 &&
                 entranceCommand.present_entrance_palette,
             1);
    expect_i("DM1 entrance command builds rattle door render",
             dm1_v1_startup_entrance_render_audio_command_pc34(
                 &media,
                 doorStep.sourceStepOrdinal,
                 (int)doorStep.kind,
                 doorStep.delayTicks,
                 doorStep.vblankLoopCount,
                 &entranceCommand) &&
                 entranceCommand.render_kind ==
                     DM1_V1_STARTUP_ENTRANCE_RENDER_OPENING_DOOR_PC34 &&
                 entranceCommand.door_animation_step == 1u &&
                 entranceCommand.play_door_rattle_sound &&
                 entranceCommand.delay_ms == media.entrance_vblank_ms,
             1);
    badMedia = media;
    badMedia.entrance_vblank_ms = 1u;
    expect_i("DM1 entrance command rejects bad timing receipt",
             dm1_v1_startup_entrance_render_audio_command_pc34(
                 &badMedia,
                 doorStep.sourceStepOrdinal,
                 (int)doorStep.kind,
                 doorStep.delayTicks,
                 doorStep.vblankLoopCount,
                 &entranceCommand),
             0);
    expect_i("DM1 full graphics media receipt blocks non-DM1",
             dm1_v1_startup_full_graphics_media_receipt_pc34("csb", &media) &&
                 media.handled == 0,
             1);

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

    memset(&facts, 0, sizeof(facts));
    memset(&receipt, 0, sizeof(receipt));
    facts.title_frame = V1_TITLE_DAT_FRAME_MAX;
    facts.title_frame_max = V1_TITLE_DAT_FRAME_MAX;
    facts.title_handoff_ready = 1;
    facts.advance_requested = 1;
    expect_i("DM1 title menu receipt handles last title frame",
             dm1_v1_startup_title_menu_eligibility_receipt_pc34(&facts,
                                                                &receipt),
             1);
    expect_i("DM1 title menu receipt keeps last title surface",
             receipt.keep_title_surface, 1);
    expect_i("DM1 title menu receipt does not enter early",
             receipt.menu_eligible, 0);

    facts.title_frame = V1_TITLE_DAT_FRAME_MAX + 1u;
    facts.advance_requested = 0;
    expect_i("DM1 title menu receipt handles held post-boundary frame",
             dm1_v1_startup_title_menu_eligibility_receipt_pc34(&facts,
                                                                &receipt),
             1);
    expect_i("DM1 title menu receipt holds without input",
             receipt.keep_title_surface, 1);
    expect_i("DM1 title menu receipt still blocks menu without input",
             receipt.menu_eligible, 0);

    facts.advance_requested = 1;
    expect_i("DM1 title menu receipt handles menu boundary",
             dm1_v1_startup_title_menu_eligibility_receipt_pc34(&facts,
                                                                &receipt),
             1);
    expect_i("DM1 title menu receipt enters menu at post-boundary",
             receipt.menu_eligible, 1);
    expect_i("DM1 title menu receipt consumes boundary input",
             receipt.consume_pending_input, 1);
    expect_u("DM1 title menu receipt advances stage",
             (unsigned int)receipt.next_stage,
             (unsigned int)DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34);

    expect_stage_after("last TITLE frame follows first TITLE frame",
                       DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34,
                       DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34);
    expect_stage_after("menu follows last TITLE frame",
                       DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34,
                       DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34);
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
    expect_i("headless entrance waits until explicit timeout",
             M11_Entrance_ShouldAutoEnterForTimeout(1, 1200, 1199u),
             0);
    expect_i("headless entrance may auto-enter after explicit timeout",
             M11_Entrance_ShouldAutoEnterForTimeout(1, 1200, 1201u),
             1);
    expect_stage_after("entrance wait follows menu eligibility",
                       DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34,
                       DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34);
}

static void check_dm1_launch_path_bypass_contract(void) {
    char phase[64];
    char animation[64];
    DM1_V1_StartupHandoffPreludePlan_PC34 prelude;
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 post;
    DM1_V1_StartupHandoffOutcome_PC34 outcome;
    DM1_V1_StartupHostApplyResult_PC34 apply_result;
    DM1_V1_StartupSelectedLaunchCallbacks_PC34 launch_callbacks;
    DM1_V1_StartupSelectedLaunchResult_PC34 launch_result;
    DM1_V1_StartupSelectedLaunchRouteFacts_PC34 route_facts;
    DM1_V1_StartupSelectedLaunchRouteReceipt_PC34 route_receipt;
    DM1_V1_StartupLaunchPathFacts_PC34 launch_facts;
    DM1_V1_StartupLaunchPathReceipt_PC34 launch_receipt;
    DM1_V1_StartupRuntimeStartFacts_PC34 runtime_facts;
    DM1_V1_StartupRuntimeStartReceipt_PC34 runtime_receipt;
    DM1_V1_StartupDungeonPathFacts_PC34 dungeon_facts;
    DM1_V1_StartupDungeonPathReceipt_PC34 dungeon_receipt;
    DM1_V1_StartupGraphicsBindFacts_PC34 graphics_facts;
    DM1_V1_StartupGraphicsBindReceipt_PC34 graphics_receipt;
    DM1_V1_StartupDungeonLoadFacts_PC34 load_facts;
    DM1_V1_StartupDungeonLoadReceipt_PC34 load_receipt;
    DM1_V1_StartupRuntimeReadyFacts_PC34 ready_facts;
    DM1_V1_StartupRuntimeReadyReceipt_PC34 ready_receipt;
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34 runtime_handoff;
    DM1_V1_StartupHoCFirstFrameReceipt_PC34 hoc_first_frame;
    DM1_V1_StartupHoCHostRenderPlan_PC34 hoc_host_plan;
    DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34 hoc_full_graphics_proof;
    DM1_V1_StartupHoCProductionFullStartHook_PC34 hoc_production_hook;
    DM1_V1_StartupHoCFullStartProductionReceipt_PC34 hoc_production_receipt;
    DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34 hoc_capture_artifact;
    DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34 hoc_capture_facts;
    DM1_V1_StartupHoCFullGraphicsCaptureProofReceipt_PC34 hoc_capture_proof;
    DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34 hoc_runtime_apply;
    DM1_V1_StartupHoCFullGraphicsThingSuppressionFacts_PC34
        hoc_suppression_facts;
    DM1_V1_StartupHoCFullGraphicsThingSuppressionReceipt_PC34
        hoc_suppression_receipt;
    DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34
        hoc_production_consumer;
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 mirror_front_wall;
    DM1_V1_ChampionMirrorRenderReceiptPc34 mirror_render;
    DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34 mirror_boundary;
    DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34 mirror_thing_consumer;
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 hoc_floor_thing;
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 hoc_projectile_thing;
    const DM1V1D1LD1RF0115LanePc34Data* hoc_lane;
    DM1_V1_StartupHoCRenderConsumerReceipt_PC34 hoc_render_consumer;
    DM1_V1_StartupHoCFirstFrameReceipt_PC34 bad_hoc_first_frame;
    FakeDm1StartupCallbacks fake;
    DM1_V1_StartupHandoffCallbacks_PC34 callbacks;
    DM1_V1_StartupHostCallbacks_PC34 host_callbacks;
    int title_played = 0;
    int entrance_command = 0;
    int startup_active = -1;
    int animation_active = -1;
    int title_frame = -1;
    int title_frame_max = -1;
    int title_ready = -1;
    DM1_V1_StartupBootProbeFacts_PC34 boot_facts;
    DM1_V1_StartupBootProbeReceipt_PC34 boot_receipt;
    DM1_V1_StartupSelectedBootProbeFacts_PC34 selected_boot_facts;
    DM1_V1_StartupSelectedBootProbeReceipt_PC34 selected_boot_receipt;
    DM1_V1_StartupSelectedBootProbeSourceKindFacts_PC34 selected_kind_facts;
    DM1_V1_StartupSelectedBootProbeSourceKindReceipt_PC34 selected_kind_receipt;

    expect_i("launcher launch path does not bypass intro",
             dm1_v1_startup_launch_path_bypasses_intro_pc34(
                 DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34),
             0);
    expect_i("direct CLI launch path does not bypass source-visible intro",
             dm1_v1_startup_launch_path_bypasses_intro_pc34(
                 DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_CLI_PC34),
             0);
    expect_i("direct game-view path bypasses intro",
             dm1_v1_startup_launch_path_bypasses_intro_pc34(
                 DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34),
             1);
    expect_i("unknown launch path is treated as bypass",
             dm1_v1_startup_launch_path_bypasses_intro_pc34(
                 (DM1_V1_StartupLaunchPath_PC34)99),
             1);

    expect_i("empty source reports no DM1 intro bypass",
             dm1_v1_startup_intro_bypass_applies_to_source_pc34(NULL, 1),
             0);
    expect_i("DM1 source reports direct intro bypass",
             dm1_v1_startup_intro_bypass_applies_to_source_pc34("dm1", 1),
             1);
    expect_i("DM1 source without bypass reports no bypass",
             dm1_v1_startup_intro_bypass_applies_to_source_pc34("dm1", 0),
             0);
    expect_i("non-DM1 source ignores intro bypass flag",
             dm1_v1_startup_intro_bypass_applies_to_source_pc34("csb", 1),
             0);
    expect_i("DM1 handoff requires source-visible intro",
             dm1_v1_startup_source_visible_handoff_required_pc34("dm1"),
             1);
    expect_i("CSB handoff does not use DM1 source-visible intro policy",
             dm1_v1_startup_source_visible_handoff_required_pc34("csb"),
             0);
    expect_i("NULL handoff does not use DM1 source-visible intro policy",
             dm1_v1_startup_source_visible_handoff_required_pc34(NULL),
             0);
    expect_i("DM1 selected-entry receipt rejects intro bypass",
             dm1_v1_startup_selected_entry_receipt_valid_pc34("dm1", 1),
             0);
    expect_i("DM1 selected-entry receipt accepts source-visible intro",
             dm1_v1_startup_selected_entry_receipt_valid_pc34("dm1", 0),
             1);
    expect_i("CSB selected-entry receipt ignores DM1 intro bypass policy",
             dm1_v1_startup_selected_entry_receipt_valid_pc34("csb", 1),
             1);

    memset(&route_facts, 0, sizeof(route_facts));
    memset(&route_receipt, 0, sizeof(route_receipt));
    route_facts.selected_game_id = "dm1";
    expect_i("DM1 selected launch route receipt builds",
             dm1_v1_startup_selected_launch_route_receipt_pc34(&route_facts,
                                                               &route_receipt),
             1);
    expect_i("DM1 selected launch route uses DM1 transaction",
             route_receipt.handled == 1 &&
                 route_receipt.use_dm1_transaction == 1 &&
                 route_receipt.use_generic_launch == 0 &&
                 route_receipt.requires_source_visible_intro == 1,
             1);
    route_facts.selected_game_id = "csb";
    expect_i("CSB selected launch route stays generic",
             dm1_v1_startup_selected_launch_route_receipt_pc34(&route_facts,
                                                               &route_receipt) &&
                 route_receipt.handled == 1 &&
                 route_receipt.use_dm1_transaction == 0 &&
                 route_receipt.use_generic_launch == 1,
             1);
    route_facts.selected_game_id = NULL;
    expect_i("NULL selected launch route stays generic",
             dm1_v1_startup_selected_launch_route_receipt_pc34(&route_facts,
                                                               &route_receipt) &&
                 route_receipt.use_dm1_transaction == 0 &&
                 route_receipt.use_generic_launch == 1,
             1);
    expect_i("NULL selected launch route facts rejected",
             dm1_v1_startup_selected_launch_route_receipt_pc34(NULL,
                                                               &route_receipt),
             0);

    memset(&selected_boot_facts, 0, sizeof(selected_boot_facts));
    memset(&selected_boot_receipt, 0, sizeof(selected_boot_receipt));
    selected_boot_facts.expected_game_id = "dm1";
    selected_boot_facts.actual_source_id = "dm1";
    selected_boot_facts.active = 1;
    selected_boot_facts.started_from_launcher = 1;
    selected_boot_facts.intro_bypassed = 0;
    expect_i("DM1 selected boot-probe receipt builds",
             dm1_v1_startup_selected_boot_probe_receipt_pc34(
                 &selected_boot_facts,
                 &selected_boot_receipt),
             1);
    expect_i("DM1 selected boot-probe receipt accepts source-visible launch",
             selected_boot_receipt.valid == 1 &&
                 selected_boot_receipt.source_matches == 1 &&
                 selected_boot_receipt.selected_entry_receipt_valid == 1,
             1);
    selected_boot_facts.intro_bypassed = 1;
    expect_i("DM1 selected boot-probe receipt rejects bypassed intro",
             dm1_v1_startup_selected_boot_probe_receipt_pc34(
                 &selected_boot_facts,
                 &selected_boot_receipt) &&
                 selected_boot_receipt.valid == 0 &&
                 selected_boot_receipt.selected_entry_receipt_valid == 0,
             1);
    selected_boot_facts.expected_game_id = "csb";
    selected_boot_facts.actual_source_id = "csb";
    expect_i("CSB selected boot-probe receipt stays generic-valid",
             dm1_v1_startup_selected_boot_probe_receipt_pc34(
                 &selected_boot_facts,
                 &selected_boot_receipt) &&
                 selected_boot_receipt.selected_entry_receipt_valid == 1,
             1);
    expect_i("NULL selected boot-probe facts rejected",
             dm1_v1_startup_selected_boot_probe_receipt_pc34(
                 NULL,
                 &selected_boot_receipt),
             0);

    memset(&selected_kind_facts, 0, sizeof(selected_kind_facts));
    memset(&selected_kind_receipt, 0, sizeof(selected_kind_receipt));
    selected_kind_facts.expected_game_id = "dm1";
    selected_kind_facts.actual_source_kind = 0;
    selected_kind_facts.dm1_builtin_source_kind = 0;
    expect_i("DM1 selected boot-probe source kind accepts builtin catalog",
             dm1_v1_startup_selected_boot_probe_source_kind_receipt_pc34(
                 &selected_kind_facts,
                 &selected_kind_receipt) &&
                 selected_kind_receipt.handled == 1 &&
                 selected_kind_receipt.valid == 1 &&
                 selected_kind_receipt.expected_source_kind == 0,
             1);
    selected_kind_facts.actual_source_kind = 2;
    expect_i("DM1 selected boot-probe source kind rejects non-builtin",
             dm1_v1_startup_selected_boot_probe_source_kind_receipt_pc34(
                 &selected_kind_facts,
                 &selected_kind_receipt) &&
                 selected_kind_receipt.handled == 1 &&
                 selected_kind_receipt.valid == 0 &&
                 selected_kind_receipt.expected_source_kind == 0 &&
                 selected_kind_receipt.actual_source_kind == 2,
             1);
    selected_kind_facts.expected_game_id = "csb";
    expect_i("CSB selected boot-probe source kind stays generic-valid",
             dm1_v1_startup_selected_boot_probe_source_kind_receipt_pc34(
                 &selected_kind_facts,
                 &selected_kind_receipt) &&
                 selected_kind_receipt.handled == 0 &&
                 selected_kind_receipt.valid == 1,
             1);
    expect_i("NULL selected boot-probe source kind facts rejected",
             dm1_v1_startup_selected_boot_probe_source_kind_receipt_pc34(
                 NULL,
                 &selected_kind_receipt),
             0);

    memset(&launch_facts, 0, sizeof(launch_facts));
    memset(&launch_receipt, 0, sizeof(launch_receipt));
    launch_facts.source_id = "dm1";
    launch_facts.launch_path = DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34;
    expect_i("DM1 launcher launch-path receipt succeeds",
             dm1_v1_startup_launch_path_receipt_pc34(&launch_facts,
                                                     &launch_receipt),
             1);
    expect_i("DM1 launcher launch-path receipt handled",
             launch_receipt.handled,
             1);
    expect_i("DM1 launcher launch-path receipt does not bypass",
             launch_receipt.intro_bypassed,
             0);
    expect_i("DM1 launcher launch-path marks launcher start",
             launch_receipt.started_from_launcher,
             1);
    expect_i("DM1 launcher launch-path selected receipt valid",
             launch_receipt.selected_entry_receipt_valid,
             1);
    launch_facts.launch_path =
        DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34;
    expect_i("DM1 direct-view launch-path receipt bypasses",
             dm1_v1_startup_launch_path_receipt_pc34(&launch_facts,
                                                     &launch_receipt) &&
                 launch_receipt.intro_bypassed == 1 &&
                 launch_receipt.started_from_launcher == 0 &&
                 launch_receipt.selected_entry_receipt_valid == 0,
             1);
    launch_facts.source_id = "csb";
    expect_i("non-DM1 launch-path receipt no-op",
             dm1_v1_startup_launch_path_receipt_pc34(&launch_facts,
                                                     &launch_receipt) &&
                 launch_receipt.handled == 0,
             1);
    expect_i("NULL launch-path facts rejected",
             dm1_v1_startup_launch_path_receipt_pc34(NULL,
                                                     &launch_receipt),
             0);

    memset(&runtime_facts, 0, sizeof(runtime_facts));
    memset(&runtime_receipt, 0, sizeof(runtime_receipt));
    runtime_facts.game_id = "dm1";
    runtime_facts.source_id = NULL;
    runtime_facts.title = "Dungeon Master";
    runtime_facts.verified_asset_md5 = "0123456789abcdef0123456789abcdef";
    runtime_facts.dungeon_path = "/tmp/DUNGEON.DAT";
    runtime_facts.source_kind = 2;
    runtime_facts.presentation_mode = 1;
    runtime_facts.presentation_width = 320;
    runtime_facts.presentation_height = 200;
    runtime_facts.font_scale = 4;
    runtime_facts.launch_path = DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34;
    expect_i("DM1 runtime start receipt builds",
             dm1_v1_startup_runtime_start_receipt_pc34(&runtime_facts,
                                                       &runtime_receipt),
             1);
    expect_i("DM1 runtime start receipt handled",
             runtime_receipt.handled,
             1);
    expect_i("DM1 runtime start receipt activates gameplay",
             runtime_receipt.active,
             1);
    expect_i("DM1 runtime direct start receipt is not launcher-started",
             runtime_receipt.started_from_launcher,
             0);
    expect_i("DM1 runtime start defaults source id",
             strcmp(runtime_receipt.source_id, "launcher") == 0,
             1);
    expect_i("DM1 runtime start clamps invalid font scale",
             runtime_receipt.font_scale,
             0);
    expect_i("DM1 runtime start preserves boot md5",
             strcmp(runtime_receipt.boot_asset_md5,
                    "0123456789abcdef0123456789abcdef") == 0,
             1);
    expect_i("DM1 runtime start direct view bypasses intro",
             runtime_receipt.launch_path_receipt.intro_bypassed,
             1);
    expect_i("DM1 runtime start has boot ready status",
             strcmp(runtime_receipt.status_title, "BOOT") == 0 &&
                 strcmp(runtime_receipt.status_detail,
                        "GAME DATA LOADED") == 0 &&
                 strcmp(runtime_receipt.inspect_title, "READY") == 0,
             1);
    runtime_facts.font_scale = 2;
    runtime_facts.source_id = "dm1";
    runtime_facts.launch_path = DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34;
    expect_i("DM1 runtime start accepts valid font scale",
             dm1_v1_startup_runtime_start_receipt_pc34(&runtime_facts,
                                                       &runtime_receipt) &&
                 runtime_receipt.font_scale == 2 &&
                 runtime_receipt.started_from_launcher == 1 &&
                 runtime_receipt.launch_path_receipt.intro_bypassed == 0,
             1);
    runtime_facts.game_id = "csb";
    expect_i("non-DM1 runtime start receipt no-op",
             dm1_v1_startup_runtime_start_receipt_pc34(&runtime_facts,
                                                       &runtime_receipt) &&
                 runtime_receipt.handled == 0,
             1);
    expect_i("NULL runtime start facts rejected",
             dm1_v1_startup_runtime_start_receipt_pc34(NULL,
                                                       &runtime_receipt),
             0);

    memset(&dungeon_facts, 0, sizeof(dungeon_facts));
    memset(&dungeon_receipt, 0, sizeof(dungeon_receipt));
    dungeon_facts.game_id = "dm1";
    dungeon_facts.data_dir = "/tmp/dm1";
    dungeon_facts.source_kind =
        DM1_V1_STARTUP_SOURCE_KIND_BUILTIN_CATALOG_PC34;
    expect_i("DM1 builtin dungeon path receipt builds",
             dm1_v1_startup_dungeon_path_receipt_pc34(&dungeon_facts,
                                                      &dungeon_receipt),
             1);
    expect_i("DM1 builtin dungeon path receipt handled",
             dungeon_receipt.handled,
             1);
    expect_i("DM1 builtin dungeon path asks host to resolve",
             dungeon_receipt.resolve_builtin_path == 1 &&
                 dungeon_receipt.use_explicit_path == 0,
             1);
    dungeon_facts.source_kind =
        DM1_V1_STARTUP_SOURCE_KIND_DIRECT_DUNGEON_PC34;
    expect_i("DM1 direct dungeon requires explicit path",
             dm1_v1_startup_dungeon_path_receipt_pc34(&dungeon_facts,
                                                      &dungeon_receipt) &&
                 dungeon_receipt.explicit_path_required == 1 &&
                 dungeon_receipt.use_explicit_path == 0 &&
                 dungeon_receipt.resolve_builtin_path == 0,
             1);
    dungeon_facts.explicit_dungeon_path = "/tmp/DUNGEON.DAT";
    expect_i("DM1 direct dungeon accepts explicit path",
             dm1_v1_startup_dungeon_path_receipt_pc34(&dungeon_facts,
                                                      &dungeon_receipt) &&
                 dungeon_receipt.use_explicit_path == 1 &&
                 strcmp(dungeon_receipt.explicit_dungeon_path,
                        "/tmp/DUNGEON.DAT") == 0,
             1);
    dungeon_facts.source_kind =
        DM1_V1_STARTUP_SOURCE_KIND_CUSTOM_DUNGEON_PC34;
    dungeon_facts.explicit_dungeon_path = "/tmp/CUSTOM.DAT";
    expect_i("DM1 custom dungeon accepts explicit path",
             dm1_v1_startup_dungeon_path_receipt_pc34(&dungeon_facts,
                                                      &dungeon_receipt) &&
                 dungeon_receipt.use_explicit_path == 1 &&
                 strcmp(dungeon_receipt.explicit_dungeon_path,
                        "/tmp/CUSTOM.DAT") == 0,
             1);
    dungeon_facts.game_id = "csb";
    expect_i("non-DM1 dungeon path receipt no-op",
             dm1_v1_startup_dungeon_path_receipt_pc34(&dungeon_facts,
                                                      &dungeon_receipt) &&
                 dungeon_receipt.handled == 0,
             1);
    expect_i("NULL dungeon path facts rejected",
             dm1_v1_startup_dungeon_path_receipt_pc34(NULL,
                                                      &dungeon_receipt),
             0);

    memset(&graphics_facts, 0, sizeof(graphics_facts));
    memset(&graphics_receipt, 0, sizeof(graphics_receipt));
    graphics_facts.game_id = "dm1";
    graphics_facts.dungeon_path = "/tmp/firestaff/DATA/DUNGEON.DAT";
    expect_i("DM1 graphics bind receipt builds",
             dm1_v1_startup_graphics_bind_receipt_pc34(&graphics_facts,
                                                       &graphics_receipt),
             1);
    expect_i("DM1 graphics bind receipt handled",
             graphics_receipt.handled,
             1);
    expect_i("DM1 graphics bind points at sibling GRAPHICS.DAT",
             graphics_receipt.bind_graphics_dat == 1 &&
                 strcmp(graphics_receipt.graphics_dat_path,
                        "/tmp/firestaff/DATA/GRAPHICS.DAT") == 0,
             1);
    graphics_facts.dungeon_path = "DUNGEON.DAT";
    expect_i("DM1 graphics bind needs a parent path",
             dm1_v1_startup_graphics_bind_receipt_pc34(&graphics_facts,
                                                       &graphics_receipt) &&
                 graphics_receipt.handled == 1 &&
                 graphics_receipt.bind_graphics_dat == 0,
             1);
    graphics_facts.game_id = "csb";
    graphics_facts.dungeon_path = "/tmp/csb/DUNGEON.DAT";
    expect_i("non-DM1 graphics bind receipt no-op",
             dm1_v1_startup_graphics_bind_receipt_pc34(&graphics_facts,
                                                       &graphics_receipt) &&
                 graphics_receipt.handled == 0,
             1);
    expect_i("NULL graphics bind facts rejected",
             dm1_v1_startup_graphics_bind_receipt_pc34(NULL,
                                                       &graphics_receipt),
             0);

    memset(&load_facts, 0, sizeof(load_facts));
    memset(&load_receipt, 0, sizeof(load_receipt));
    load_facts.game_id = "dm1";
    load_facts.load_succeeded = 0;
    expect_i("DM1 dungeon-load failure receipt builds",
             dm1_v1_startup_dungeon_load_receipt_pc34(&load_facts,
                                                      &load_receipt),
             1);
    expect_i("DM1 dungeon-load failure receipt handled",
             load_receipt.handled,
             1);
    expect_i("DM1 dungeon-load failure receipt reports boot failure",
             load_receipt.load_succeeded == 0 &&
                 strcmp(load_receipt.status_title, "BOOT") == 0 &&
                 strcmp(load_receipt.status_detail,
                        "FAILED TO LOAD DUNGEON.DAT") == 0,
             1);
    load_facts.load_succeeded = 1;
    expect_i("DM1 dungeon-load success receipt reports boot loaded",
             dm1_v1_startup_dungeon_load_receipt_pc34(&load_facts,
                                                      &load_receipt) &&
                 load_receipt.load_succeeded == 1 &&
                 strcmp(load_receipt.status_detail,
                        "GAME DATA LOADED") == 0,
             1);
    load_facts.game_id = "csb";
    expect_i("non-DM1 dungeon-load receipt no-op",
             dm1_v1_startup_dungeon_load_receipt_pc34(&load_facts,
                                                      &load_receipt) &&
                 load_receipt.handled == 0,
             1);
    expect_i("NULL dungeon-load facts rejected",
             dm1_v1_startup_dungeon_load_receipt_pc34(NULL,
                                                      &load_receipt),
             0);

    memset(&ready_facts, 0, sizeof(ready_facts));
    memset(&ready_receipt, 0, sizeof(ready_receipt));
    memset(&runtime_handoff, 0, sizeof(runtime_handoff));
    ready_facts.runtime_start.game_id = "dm1";
    ready_facts.runtime_start.source_id = "dm1";
    ready_facts.runtime_start.title = "Dungeon Master";
    ready_facts.runtime_start.verified_asset_md5 =
        "0123456789abcdef0123456789abcdef";
    ready_facts.runtime_start.dungeon_path =
        "/tmp/firestaff/DATA/DUNGEON.DAT";
    ready_facts.runtime_start.source_kind = 2;
    ready_facts.runtime_start.presentation_mode = 1;
    ready_facts.runtime_start.presentation_width = 320;
    ready_facts.runtime_start.presentation_height = 200;
    ready_facts.runtime_start.font_scale = 2;
    ready_facts.runtime_start.launch_path =
        DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34;
    ready_facts.load_succeeded = 1;
    expect_i("DM1 runtime-ready receipt builds",
             dm1_v1_startup_runtime_ready_receipt_pc34(&ready_facts,
                                                       &ready_receipt),
             1);
    expect_i("DM1 runtime-ready receipt combines load runtime graphics",
             ready_receipt.handled == 1 &&
                 ready_receipt.load_receipt.load_succeeded == 1 &&
                 ready_receipt.runtime_start_receipt.active == 1 &&
                 ready_receipt.graphics_bind_receipt.bind_graphics_dat == 1 &&
                 strcmp(ready_receipt.graphics_bind_receipt.graphics_dat_path,
                        "/tmp/firestaff/DATA/GRAPHICS.DAT") == 0,
             1);
    ready_facts.load_succeeded = 0;
    expect_i("DM1 runtime-ready rejects failed dungeon load",
             dm1_v1_startup_runtime_ready_receipt_pc34(&ready_facts,
                                                       &ready_receipt),
             0);
    ready_facts.load_succeeded = 1;
    ready_facts.runtime_start.game_id = "csb";
    expect_i("non-DM1 runtime-ready receipt no-op",
             dm1_v1_startup_runtime_ready_receipt_pc34(&ready_facts,
                                                       &ready_receipt) &&
                 ready_receipt.handled == 0,
             1);
    expect_i("NULL runtime-ready facts rejected",
             dm1_v1_startup_runtime_ready_receipt_pc34(NULL,
                                                       &ready_receipt),
             0);
    expect_i("DM1 handoff prelude plan builds",
             dm1_v1_startup_handoff_prelude_plan_pc34("dm1", &prelude),
             1);
    expect_i("DM1 handoff prelude is required",
             prelude.required,
             1);
    expect_i("DM1 handoff prelude plays SWSH",
             prelude.play_swsh,
             1);
    expect_i("DM1 handoff prelude carries startup media receipt",
             prelude.media_receipt.handled &&
                 prelude.media_receipt.play_swsh,
             1);
    expect_i("DM1 handoff prelude discards post-SWSH texture",
             prelude.discard_presentation_after_swsh,
             1);
    expect_i("DM1 handoff prelude source order valid",
             prelude.source_order_valid,
             1);
    expect_i("CSB handoff prelude plan builds",
             dm1_v1_startup_handoff_prelude_plan_pc34("csb", &prelude),
             1);
    expect_i("CSB handoff prelude is not DM1-required",
             prelude.required,
             0);
    expect_i("NULL handoff prelude rejects missing output",
             dm1_v1_startup_handoff_prelude_plan_pc34("dm1", NULL),
             0);
    expect_i("DM1 post-launch plan builds",
             dm1_v1_startup_handoff_post_launch_plan_pc34("dm1", &post),
             1);
    expect_i("DM1 post-launch plan plays title",
             post.play_title,
             1);
    expect_i("DM1 post-launch plan plays entrance",
             post.play_entrance,
             1);
    expect_i("DM1 post-launch plan carries media title receipt",
             post.media_receipt.handled &&
                 post.media_receipt.play_title &&
                 post.media_receipt.title_menu_boundary_frame ==
                     (unsigned int)post.title_menu_boundary_frame,
             1);
    expect_i("DM1 post-launch plan carries entrance timing receipt",
             post.media_receipt.entrance_source_animation_steps ==
                 ENTRANCE_Compat_GetSourceAnimationStepCount() &&
                 post.media_receipt.entrance_door_step_count ==
                     ENTRANCE_Compat_GetDoorAnimationStepCount() &&
                 post.media_receipt.entrance_vblank_ms ==
                     ENTRANCE_Compat_GetVblankDelayMs(),
             1);
    expect_i("DM1 post-launch plan keeps entrance timeout",
             post.entrance_auto_enter_ms,
             1200);
    expect_i("DM1 post-launch plan carries title boundary frame",
             post.title_menu_boundary_frame,
             (int)dm1_v1_startup_title_frame_bank_equivalent_steps_pc34() + 1);
    expect_i("DM1 post-launch plan marks title menu eligible",
             post.title_menu_eligible,
             1);
    expect_i("DM1 post-launch plan consumes title boundary input",
             post.title_consume_pending_input,
             1);
    expect_u("DM1 post-launch plan enters entrance wait after menu",
             (unsigned int)post.entrance_wait_stage,
             (unsigned int)DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34);
    expect_i("DM1 post-launch plan carries entrance C255 map",
             post.entrance_full_start_receipt.mapIndex,
             DM1_V1_ENTRANCE_MAP_INDEX_PC34);
    expect_i("DM1 post-launch plan carries 5x5 entrance width",
             post.entrance_full_start_receipt.width,
             DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34);
    expect_i("DM1 post-launch plan carries 5x5 entrance height",
             post.entrance_full_start_receipt.height,
             DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34);
    expect_i("DM1 post-launch plan carries entrance corridor count",
             post.entrance_full_start_receipt.corridorCount,
             6);
    expect_i("DM1 post-launch plan starts facing south at 2,0",
             post.entrance_full_start_receipt.partyDirection,
             DM1_V1_ENTRANCE_DIRECTION_SOUTH_PC34);
    memset(&outcome, 0, sizeof(outcome));
    expect_i("DM1 enter outcome builds for HoC first frame",
             dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
                 ENTRANCE_COMPAT_COMMAND_PATH_ENTER,
                 &outcome),
             1);
    memset(&hoc_first_frame, 0, sizeof(hoc_first_frame));
    expect_i("DM1 HoC first-frame receipt builds",
             dm1_v1_startup_hoc_first_frame_receipt_pc34(
                 "dm1",
                 &post,
                 &outcome,
                 &hoc_first_frame),
             1);
    expect_i("DM1 HoC first-frame releases title surface",
             hoc_first_frame.title_surface_released,
             1);
    expect_i("DM1 HoC first-frame consumes entrance wait",
             hoc_first_frame.entrance_wait_consumed,
             1);
    expect_i("DM1 HoC first-frame has C255 entrance map",
             hoc_first_frame.entrance_map_ready &&
                 hoc_first_frame.entrance_full_start_receipt.mapIndex ==
                     DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
                 hoc_first_frame.entrance_full_start_receipt.partyDirection ==
                     DM1_V1_ENTRANCE_DIRECTION_SOUTH_PC34,
             1);
    expect_i("DM1 HoC first-frame has opened entrance door",
             hoc_first_frame.entrance_door_open_frame_ready &&
                 hoc_first_frame.entrance_full_start_receipt.doorFrameIndex == 9,
             1);
    expect_i("DM1 HoC first-frame routes champion-select hall",
             hoc_first_frame.hoc_menu_route_ready &&
                 hoc_first_frame.champion_select_route.route ==
                     DM1_V1_ENTRANCE_MENU_ROUTE_HALL_PC34 &&
                 hoc_first_frame.champion_select_route.state ==
                     DM1_ENTRANCE_VIEWING,
             1);
    expect_i("DM1 HoC first-frame renders hall mirrors",
             hoc_first_frame.render_hall_mirrors &&
                 hoc_first_frame.render_overlay_commands_ready &&
                 hoc_first_frame.render_overlay_command_count == 1 &&
                 hoc_first_frame.render_overlay_commands[0].valid &&
                 hoc_first_frame.render_overlay_commands[0].kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 hoc_first_frame.render_overlay_commands[0]
                     .clearStalePanelFirst &&
                 hoc_first_frame.render_overlay_commands[0]
                     .suppressThingPayloads &&
                 hoc_first_frame.render_overlay_commands[0]
                     .blockEnterUntilChampionSelected &&
                 hoc_first_frame.champion_select_route.renderOverlayCommandCount == 1 &&
                 hoc_first_frame.champion_select_route.renderOverlayCommands[0].kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34,
             1);
    expect_i("DM1 HoC first-frame clears stale champion panel",
             hoc_first_frame.clear_stale_champion_panel,
             1);
    expect_i("DM1 HoC first-frame blocks enter before champion",
             hoc_first_frame.block_enter_until_champion_selected,
             1);
    expect_i("DM1 HoC first-frame suppresses host fallback visuals",
             hoc_first_frame.runtime_first_frame_ready &&
                 hoc_first_frame.suppress_host_fallback_visuals,
             1);
    expect_i("DM1 HoC first-frame emits render command plan",
             hoc_first_frame.hoc_render_command_count == 3 &&
                 hoc_first_frame.hoc_render_commands[0].valid &&
                 hoc_first_frame.hoc_render_commands[0].kind ==
                     DM1_V1_STARTUP_HOC_RENDER_COMMAND_ENTRANCE_OPEN_FRAME_PC34 &&
                 hoc_first_frame.hoc_render_commands[0].map_index ==
                     DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
                 hoc_first_frame.hoc_render_commands[0].door_frame_index == 9,
             1);
    expect_i("DM1 HoC first-frame clears panel before mirrors",
             hoc_first_frame.hoc_render_commands[1].valid &&
                 hoc_first_frame.hoc_render_commands[1].kind ==
                     DM1_V1_STARTUP_HOC_RENDER_COMMAND_CLEAR_CHAMPION_PANEL_PC34 &&
                 hoc_first_frame.hoc_render_commands[1].clear_stale_panel_first &&
                 hoc_first_frame.hoc_render_commands[1]
                     .suppress_host_fallback_visuals,
             1);
    expect_i("DM1 HoC first-frame renders hall mirrors after clear",
             hoc_first_frame.hoc_render_commands[2].valid &&
                 hoc_first_frame.hoc_render_commands[2].kind ==
                     DM1_V1_STARTUP_HOC_RENDER_COMMAND_HALL_MIRRORS_PC34 &&
                 hoc_first_frame.hoc_render_commands[2].overlay_kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 hoc_first_frame.hoc_render_commands[2]
                     .block_enter_until_champion_selected &&
                 hoc_first_frame.hoc_render_commands[2]
                     .suppress_host_fallback_visuals,
             1);
    memset(&hoc_host_plan, 0, sizeof(hoc_host_plan));
    expect_i("DM1 HoC host render plan builds",
             dm1_v1_startup_hoc_host_render_plan_from_first_frame_pc34(
                 &hoc_first_frame,
                 &hoc_host_plan),
             1);
    expect_i("DM1 HoC host render plan is ready",
             hoc_host_plan.handled &&
                 hoc_host_plan.ready &&
                 hoc_host_plan.consume_dm1_receipt_only &&
                 hoc_host_plan.command_count == 3,
             1);
    expect_i("DM1 HoC host render plan carries opened entrance frame",
             hoc_host_plan.draw_opened_entrance_frame &&
                 hoc_host_plan.entrance_map_index ==
                     DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
                 hoc_host_plan.entrance_door_frame_index == 9,
             1);
    expect_i("DM1 HoC host render plan carries Hall overlay",
             hoc_host_plan.clear_champion_panel &&
                 hoc_host_plan.render_hall_mirror_overlay &&
                 hoc_host_plan.hall_mirror_overlay_kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 hoc_host_plan.suppress_host_fallback_visuals &&
                 hoc_host_plan.block_enter_until_champion_selected,
             1);
    memset(&hoc_full_graphics_proof, 0, sizeof(hoc_full_graphics_proof));
    expect_i("DM1 HoC packaged full-graphics proof builds",
             dm1_v1_startup_hoc_packaged_full_graphics_proof_from_host_plan_pc34(
                 &hoc_host_plan,
                 &hoc_full_graphics_proof),
             1);
    expect_i("DM1 HoC packaged proof consumes host plan",
             hoc_full_graphics_proof.handled &&
                 hoc_full_graphics_proof.ready &&
                 hoc_full_graphics_proof.consume_host_render_plan_only &&
                 hoc_full_graphics_proof.capture_required &&
                 hoc_full_graphics_proof.packaged_full_graphics_proof_ready,
             1);
    expect_i("DM1 HoC packaged proof carries first-frame capture fields",
             hoc_full_graphics_proof.expected_map_index ==
                     DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
                 hoc_full_graphics_proof.expected_map_width ==
                     DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 &&
                 hoc_full_graphics_proof.expected_map_height ==
                     DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 &&
                 hoc_full_graphics_proof.expected_entrance_door_frame_index ==
                     9 &&
                 hoc_full_graphics_proof.expected_hall_overlay_kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34,
             1);
    expect_i("DM1 HoC packaged proof forbids stale host visuals",
             hoc_full_graphics_proof.require_opened_entrance_frame &&
                 hoc_full_graphics_proof.require_clear_champion_panel &&
                 hoc_full_graphics_proof.require_hall_mirror_overlay &&
                 hoc_full_graphics_proof.require_no_title_surface &&
                 hoc_full_graphics_proof.require_no_closed_door_frame &&
                 hoc_full_graphics_proof.require_no_host_fallback_visuals &&
                 hoc_full_graphics_proof.block_enter_until_champion_selected &&
                 hoc_full_graphics_proof.command_count == 3,
             1);
    memset(&hoc_production_hook, 0, sizeof(hoc_production_hook));
    expect_i("DM1 HoC production full-start hook builds",
             dm1_v1_startup_hoc_production_full_start_hook_from_proof_pc34(
                 &hoc_full_graphics_proof,
                 &hoc_production_hook),
             1);
    expect_i("DM1 HoC production hook consumes startup receipts",
             hoc_production_hook.handled &&
                 hoc_production_hook.ready &&
                 hoc_production_hook.consume_dm1_startup_receipts_only &&
                 hoc_production_hook.run_before_hoc_input &&
                 hoc_production_hook.capture_after_first_frame_render &&
                 hoc_production_hook.publish_packaged_full_graphics_proof,
             1);
    expect_i("DM1 HoC production hook carries render plan",
             hoc_production_hook.draw_opened_entrance_frame &&
                 hoc_production_hook.clear_champion_panel &&
                 hoc_production_hook.render_hall_mirror_overlay &&
                 hoc_production_hook.suppress_host_fallback_visuals &&
                 hoc_production_hook.expected_map_index ==
                     DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
                 hoc_production_hook.expected_map_width ==
                     DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 &&
                 hoc_production_hook.expected_map_height ==
                     DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 &&
                 hoc_production_hook.expected_entrance_door_frame_index == 9 &&
                 hoc_production_hook.expected_hall_overlay_kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 hoc_production_hook.block_enter_until_champion_selected,
             1);
    memset(&hoc_production_receipt, 0, sizeof(hoc_production_receipt));
    expect_i("DM1 HoC full-start production receipt builds",
             dm1_v1_startup_hoc_full_start_production_receipt_pc34(
                 "dm1",
                 &post,
                 &outcome,
                 &hoc_production_receipt),
             1);
    expect_i("DM1 HoC full-start production receipt is ready",
             hoc_production_receipt.handled &&
                 hoc_production_receipt.ready &&
                 hoc_production_receipt.consumed_post_launch_plan &&
                 hoc_production_receipt.consumed_handoff_outcome &&
                 hoc_production_receipt.first_frame_ready &&
                 hoc_production_receipt.host_render_plan_ready &&
                 hoc_production_receipt.packaged_full_graphics_proof_ready &&
                 hoc_production_receipt.production_hook_ready,
             1);
    expect_i("DM1 HoC production receipt owns title/entrance/Hall handoff",
             hoc_production_receipt.title_surface_released &&
                 hoc_production_receipt.entrance_wait_consumed &&
                 hoc_production_receipt.production_hook
                     .consume_dm1_startup_receipts_only &&
                 hoc_production_receipt.production_hook
                     .capture_after_first_frame_render &&
                 hoc_production_receipt.production_hook
                     .publish_packaged_full_graphics_proof &&
                 hoc_production_receipt.host_render_plan
                         .entrance_door_frame_index == 9 &&
                 hoc_production_receipt.packaged_proof
                         .expected_hall_overlay_kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34,
             1);
    memset(&hoc_capture_artifact, 0, sizeof(hoc_capture_artifact));
    expect_i("DM1 HoC capture artifact builds",
             dm1_v1_startup_hoc_full_graphics_capture_artifact_from_production_pc34(
                 &hoc_production_receipt,
                 &hoc_capture_artifact),
             1);
    expect_i("DM1 HoC capture artifact is ready",
             hoc_capture_artifact.handled &&
                 hoc_capture_artifact.ready &&
                 hoc_capture_artifact.consume_full_start_production_receipt_only &&
                 hoc_capture_artifact.capture_manifest_ready &&
                 hoc_capture_artifact.capture_after_first_frame_render &&
                 hoc_capture_artifact.publish_packaged_full_graphics_proof,
             1);
    expect_i("DM1 HoC capture artifact forbids stale surfaces",
             hoc_capture_artifact.title_surface_forbidden &&
                 hoc_capture_artifact.closed_door_frame_forbidden &&
                 hoc_capture_artifact.host_fallback_visuals_forbidden &&
                 hoc_capture_artifact.opened_entrance_frame_required &&
                 hoc_capture_artifact.hall_mirror_overlay_required &&
                 hoc_capture_artifact.clear_champion_panel_required,
             1);
    expect_i("DM1 HoC capture artifact carries manifest fields",
             hoc_capture_artifact.expected_map_index ==
                     DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
                 hoc_capture_artifact.expected_map_width ==
                     DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 &&
                 hoc_capture_artifact.expected_map_height ==
                     DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 &&
                 hoc_capture_artifact.expected_entrance_door_frame_index == 9 &&
                 hoc_capture_artifact.expected_hall_overlay_kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 hoc_capture_artifact.expected_hoc_render_command_count == 3 &&
                 hoc_capture_artifact.block_enter_until_champion_selected,
             1);
    memset(&hoc_capture_facts, 0, sizeof(hoc_capture_facts));
    hoc_capture_facts.captured_after_first_frame_render = 1;
    hoc_capture_facts.captured_map_index = DM1_V1_ENTRANCE_MAP_INDEX_PC34;
    hoc_capture_facts.captured_map_width =
        DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34;
    hoc_capture_facts.captured_map_height =
        DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34;
    hoc_capture_facts.captured_entrance_door_frame_index = 9;
    hoc_capture_facts.captured_hall_overlay_kind =
        DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34;
    hoc_capture_facts.captured_hoc_render_command_count = 3;
    hoc_capture_facts.saw_opened_entrance_frame = 1;
    hoc_capture_facts.saw_hall_mirror_overlay = 1;
    hoc_capture_facts.cleared_champion_panel = 1;
    hoc_capture_facts.blocked_enter_until_champion_selected = 1;
    memset(&hoc_capture_proof, 0, sizeof(hoc_capture_proof));
    expect_i("DM1 HoC capture proof receipt builds",
             dm1_v1_startup_hoc_full_graphics_capture_proof_receipt_pc34(
                 &hoc_capture_artifact,
                 &hoc_capture_facts,
                 &hoc_capture_proof),
             1);
    expect_i("DM1 HoC capture proof passes exact first frame",
             hoc_capture_proof.handled &&
                 hoc_capture_proof.ready &&
                 hoc_capture_proof.proof_passed &&
                 hoc_capture_proof.geometry_matches &&
                 hoc_capture_proof.entrance_frame_matches &&
                 hoc_capture_proof.hall_overlay_matches &&
                 hoc_capture_proof.command_count_matches &&
                 hoc_capture_proof.required_layers_present &&
                 hoc_capture_proof.input_block_matches,
             1);
    memset(&hoc_runtime_apply, 0, sizeof(hoc_runtime_apply));
    expect_i("DM1 HoC runtime apply receipt builds",
             dm1_v1_startup_hoc_full_graphics_runtime_apply_receipt_pc34(
                 &hoc_capture_artifact,
                 &hoc_capture_proof,
                 &hoc_runtime_apply),
             1);
    expect_i("DM1 HoC runtime apply receipt is ready",
             hoc_runtime_apply.handled &&
                 hoc_runtime_apply.ready &&
                 hoc_runtime_apply.consumed_capture_artifact &&
                 hoc_runtime_apply.consumed_capture_proof &&
                 hoc_runtime_apply.require_proof_passed &&
                 hoc_runtime_apply.apply_before_hoc_input,
             1);
    expect_i("DM1 HoC runtime apply receipt carries commands",
             hoc_runtime_apply.apply_opened_entrance_frame &&
                 hoc_runtime_apply.apply_clear_champion_panel &&
                 hoc_runtime_apply.apply_hall_mirror_overlay &&
                 hoc_runtime_apply.suppress_title_surface &&
                 hoc_runtime_apply.suppress_closed_door_frame &&
                 hoc_runtime_apply.suppress_host_fallback_visuals &&
                 hoc_runtime_apply.publish_packaged_full_graphics_proof &&
                 hoc_runtime_apply.block_enter_until_champion_selected,
             1);
    expect_i("DM1 HoC runtime apply receipt carries geometry",
             hoc_runtime_apply.map_index == DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
                 hoc_runtime_apply.map_width ==
                     DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 &&
                 hoc_runtime_apply.map_height ==
                     DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 &&
                 hoc_runtime_apply.entrance_door_frame_index == 9 &&
                 hoc_runtime_apply.hall_overlay_kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 hoc_runtime_apply.render_command_count == 3,
             1);
    memset(&hoc_suppression_facts, 0, sizeof(hoc_suppression_facts));
    hoc_suppression_facts.observed_hall_mirror_overlay = 1;
    hoc_suppression_facts.observed_enter_blocked_until_champion_selected = 1;
    hoc_suppression_facts.observed_hoc_render_command_count =
        hoc_runtime_apply.render_command_count;
    memset(&hoc_suppression_receipt, 0, sizeof(hoc_suppression_receipt));
    expect_i("DM1 HoC thing suppression receipt builds",
             dm1_v1_startup_hoc_full_graphics_thing_suppression_receipt_pc34(
                 &hoc_runtime_apply,
                 &hoc_suppression_facts,
                 &hoc_suppression_receipt),
             1);
    expect_i("DM1 HoC thing suppression proof passes clean Hall frame",
             hoc_suppression_receipt.handled &&
                 hoc_suppression_receipt.ready &&
                 hoc_suppression_receipt.proof_passed &&
                 hoc_suppression_receipt.walk_capture_safe &&
                 hoc_suppression_receipt.consumed_runtime_apply_receipt &&
                 hoc_suppression_receipt.consumed_suppression_facts,
             1);
    expect_i("DM1 HoC thing suppression forbids false payload layers",
             hoc_suppression_receipt.champion_mirror_overlay_present &&
                 hoc_suppression_receipt.false_item_payloads_absent &&
                 hoc_suppression_receipt.projectile_payloads_absent &&
                 hoc_suppression_receipt.spell_effect_payloads_absent &&
                 hoc_suppression_receipt.mirror_payload_thing_absent &&
                 hoc_suppression_receipt.fallback_visuals_absent &&
                 hoc_suppression_receipt.stale_title_absent &&
                 hoc_suppression_receipt.stale_door_absent,
             1);
    expect_i("DM1 HoC thing suppression matches commands/input",
             hoc_suppression_receipt.command_count_matches &&
                 hoc_suppression_receipt.enter_block_matches,
             1);
    memset(&hoc_production_consumer, 0, sizeof(hoc_production_consumer));
    expect_i("DM1 HoC production consumer receipt builds",
             dm1_v1_startup_hoc_full_graphics_production_consumer_receipt_pc34(
                 &hoc_runtime_apply,
                 &hoc_suppression_receipt,
                 &hoc_production_consumer),
             1);
    expect_i("DM1 HoC production consumer is ready",
             hoc_production_consumer.handled &&
                 hoc_production_consumer.ready &&
                 hoc_production_consumer.consumed_runtime_apply_receipt &&
                 hoc_production_consumer.consumed_thing_suppression_receipt &&
                 hoc_production_consumer.consume_dm1_receipts_only &&
                 hoc_production_consumer.execute_before_hoc_input,
             1);
    expect_i("DM1 HoC production consumer carries render commands",
             hoc_production_consumer.draw_opened_entrance_frame &&
                 hoc_production_consumer.clear_champion_panel &&
                 hoc_production_consumer.render_hall_mirror_overlay &&
                 hoc_production_consumer.suppress_title_surface &&
                 hoc_production_consumer.suppress_closed_door_frame &&
                 hoc_production_consumer.suppress_host_fallback_visuals &&
                 hoc_production_consumer.publish_packaged_full_graphics_proof &&
                 hoc_production_consumer.block_enter_until_champion_selected,
             1);
    expect_i("DM1 HoC production consumer suppresses false payloads",
             hoc_production_consumer.suppress_false_item_payloads &&
                 hoc_production_consumer.suppress_projectile_payloads &&
                 hoc_production_consumer.suppress_spell_effect_payloads &&
                 hoc_production_consumer.suppress_mirror_payload_things &&
                 hoc_production_consumer.walk_capture_safe,
             1);
    expect_i("DM1 HoC production consumer carries geometry",
             hoc_production_consumer.map_index ==
                     DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
                 hoc_production_consumer.map_width ==
                     DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 &&
                 hoc_production_consumer.map_height ==
                     DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 &&
                 hoc_production_consumer.entrance_door_frame_index == 9 &&
                 hoc_production_consumer.hall_overlay_kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 hoc_production_consumer.render_command_count == 3,
             1);
    hoc_lane = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    expect_i("DM1 HoC render consumer prepares mirror wall receipt",
             hoc_lane != NULL &&
                 DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
                     127, 13, 4, 2, 2, &mirror_front_wall) &&
                 DM1_V1_ChampionMirror_BuildRenderReceiptPc34(
                     &mirror_front_wall, &mirror_render) &&
                 DM1_V1_ChampionMirror_BuildThingLayerBoundaryReceiptPc34(
                     &mirror_render, &mirror_boundary),
             1);
    expect_i("DM1 HoC render consumer prepares floor thing receipt",
             dm1_v1_viewport_d1l_d1r_f0115_runtime_thing_receipt_pc34(
                 hoc_lane, 5, 1, 1, 0, &hoc_floor_thing) &&
                 hoc_floor_thing.valid &&
                 hoc_floor_thing.draw_item &&
                 !hoc_floor_thing.suppress_item,
             1);
    expect_i("DM1 HoC render consumer consumes floor mirror receipt",
             DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
                 &mirror_boundary,
                 &hoc_floor_thing,
                 &mirror_thing_consumer) &&
                 mirror_thing_consumer.valid &&
                 mirror_thing_consumer.drawChampionPortraitAsWallOverlay &&
                 mirror_thing_consumer.drawFloorObject &&
                 !mirror_thing_consumer.drawRuntimeProjectile,
             1);
    memset(&hoc_render_consumer, 0, sizeof(hoc_render_consumer));
    expect_i("DM1 HoC render consumer builds floor production hook",
             dm1_v1_startup_hoc_render_consumer_from_first_frame_and_thing_pc34(
                 &hoc_first_frame,
                 &mirror_thing_consumer,
                 &hoc_render_consumer),
             1);
    expect_i("DM1 HoC render consumer is M11-callsite ready for floor item",
             hoc_render_consumer.handled &&
                 hoc_render_consumer.ready &&
                 hoc_render_consumer.consumed_hoc_first_frame_receipt &&
                 hoc_render_consumer.consumed_mirror_thing_layer_consumer &&
                 hoc_render_consumer.consume_dm1_receipts_only &&
                 hoc_render_consumer.no_m11_fallback_scan &&
                 hoc_render_consumer.execute_before_hoc_input,
             1);
    expect_i("DM1 HoC render consumer returns wall plus floor decisions",
             hoc_render_consumer.draw_opened_entrance_frame &&
                 hoc_render_consumer.clear_champion_panel &&
                 hoc_render_consumer.render_hall_mirror_overlay &&
                 hoc_render_consumer.draw_champion_mirror_wall_overlay &&
                 hoc_render_consumer.draw_real_floor_object &&
                 !hoc_render_consumer.draw_real_projectile,
             1);
    expect_i("DM1 HoC render consumer suppresses mirror spell fallback",
             hoc_render_consumer.require_runtime_spell_effect_receipt &&
                 hoc_render_consumer.suppress_mirror_floor_item_payload &&
                 hoc_render_consumer.suppress_mirror_projectile_payload &&
                 hoc_render_consumer.suppress_mirror_spell_effect_payload &&
                 hoc_render_consumer.suppress_materialized_item_payload &&
                 hoc_render_consumer.suppress_host_fallback_visuals,
             1);
    expect_i("DM1 HoC render consumer carries floor geometry",
             hoc_render_consumer.map_index == DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
                 hoc_render_consumer.entrance_door_frame_index == 9 &&
                 hoc_render_consumer.hall_overlay_kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 hoc_render_consumer.render_command_count == 3 &&
                 hoc_render_consumer.zone == hoc_floor_thing.zone &&
                 hoc_render_consumer.row == hoc_floor_thing.row &&
                 hoc_render_consumer.view_cell == hoc_floor_thing.view_cell,
             1);
    expect_i("DM1 HoC render consumer prepares projectile receipt",
             dm1_v1_viewport_d1l_d1r_f0115_runtime_thing_receipt_pc34(
                 hoc_lane, 14, 1, 1, 1, &hoc_projectile_thing) &&
                 hoc_projectile_thing.valid &&
                 hoc_projectile_thing.draw_projectile &&
                 !hoc_projectile_thing.suppress_projectile &&
                 DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
                     &mirror_boundary,
                     &hoc_projectile_thing,
                     &mirror_thing_consumer),
             1);
    memset(&hoc_render_consumer, 0, sizeof(hoc_render_consumer));
    expect_i("DM1 HoC render consumer returns projectile decisions",
             dm1_v1_startup_hoc_render_consumer_from_first_frame_and_thing_pc34(
                 &hoc_first_frame,
                 &mirror_thing_consumer,
                 &hoc_render_consumer) &&
                 hoc_render_consumer.ready &&
                 hoc_render_consumer.no_m11_fallback_scan &&
                 hoc_render_consumer.draw_champion_mirror_wall_overlay &&
                 !hoc_render_consumer.draw_real_floor_object &&
                 hoc_render_consumer.draw_real_projectile &&
                 hoc_render_consumer.suppress_mirror_projectile_payload &&
                 hoc_render_consumer.suppress_mirror_spell_effect_payload,
             1);
    bad_hoc_first_frame = hoc_first_frame;
    bad_hoc_first_frame.suppress_host_fallback_visuals = 0;
    expect_i("DM1 HoC render consumer rejects fallback-scan frame",
             dm1_v1_startup_hoc_render_consumer_from_first_frame_and_thing_pc34(
                 &bad_hoc_first_frame,
                 &mirror_thing_consumer,
                 &hoc_render_consumer) &&
                 hoc_render_consumer.handled &&
                 !hoc_render_consumer.ready &&
                 !hoc_render_consumer.no_m11_fallback_scan,
             1);
    expect_i("DM1 HoC thing suppression rejects projectile leak",
             (hoc_suppression_facts.observed_projectile_payload_count = 1,
              dm1_v1_startup_hoc_full_graphics_thing_suppression_receipt_pc34(
                  &hoc_runtime_apply,
                  &hoc_suppression_facts,
                  &hoc_suppression_receipt) &&
                  hoc_suppression_receipt.handled &&
                  hoc_suppression_receipt.ready &&
                  !hoc_suppression_receipt.proof_passed &&
                  !hoc_suppression_receipt.projectile_payloads_absent &&
                  !hoc_suppression_receipt.walk_capture_safe),
             1);
    expect_i("DM1 HoC production consumer rejects failed suppression",
             dm1_v1_startup_hoc_full_graphics_production_consumer_receipt_pc34(
                 &hoc_runtime_apply,
                 &hoc_suppression_receipt,
                 &hoc_production_consumer) &&
                 hoc_production_consumer.handled &&
                 !hoc_production_consumer.ready,
             1);
    hoc_suppression_facts.observed_projectile_payload_count = 0;
    expect_i("DM1 HoC thing suppression rejects false floor item leak",
             (hoc_suppression_facts.observed_false_floor_item_payload_count = 1,
              dm1_v1_startup_hoc_full_graphics_thing_suppression_receipt_pc34(
                  &hoc_runtime_apply,
                  &hoc_suppression_facts,
                  &hoc_suppression_receipt) &&
                  hoc_suppression_receipt.handled &&
                  hoc_suppression_receipt.ready &&
                  !hoc_suppression_receipt.proof_passed &&
                  !hoc_suppression_receipt.false_item_payloads_absent),
             1);
    hoc_suppression_facts.observed_false_floor_item_payload_count = 0;
    expect_i("DM1 HoC thing suppression rejects mirror thing leak",
             (hoc_suppression_facts.observed_mirror_payload_as_thing_count = 1,
              dm1_v1_startup_hoc_full_graphics_thing_suppression_receipt_pc34(
                  &hoc_runtime_apply,
                  &hoc_suppression_facts,
                  &hoc_suppression_receipt) &&
                  hoc_suppression_receipt.handled &&
                  hoc_suppression_receipt.ready &&
                  !hoc_suppression_receipt.proof_passed &&
                  !hoc_suppression_receipt.mirror_payload_thing_absent),
             1);
    hoc_suppression_facts.observed_mirror_payload_as_thing_count = 0;
    expect_i("DM1 HoC thing suppression rejects NULL input",
             dm1_v1_startup_hoc_full_graphics_thing_suppression_receipt_pc34(
                 NULL,
                 &hoc_suppression_facts,
                 &hoc_suppression_receipt),
             0);
    expect_i("DM1 HoC production consumer rejects NULL input",
             dm1_v1_startup_hoc_full_graphics_production_consumer_receipt_pc34(
                 NULL,
                 &hoc_suppression_receipt,
                 &hoc_production_consumer),
             0);
    expect_i("DM1 HoC capture proof rejects stale title surface",
             (hoc_capture_facts.saw_title_surface = 1,
              dm1_v1_startup_hoc_full_graphics_capture_proof_receipt_pc34(
                  &hoc_capture_artifact,
                  &hoc_capture_facts,
                  &hoc_capture_proof) &&
                  hoc_capture_proof.handled &&
                  hoc_capture_proof.ready &&
                  !hoc_capture_proof.proof_passed &&
                  !hoc_capture_proof.stale_title_absent),
             1);
    expect_i("DM1 HoC runtime apply rejects failed proof",
             dm1_v1_startup_hoc_full_graphics_runtime_apply_receipt_pc34(
                 &hoc_capture_artifact,
                 &hoc_capture_proof,
                 &hoc_runtime_apply) &&
                 hoc_runtime_apply.handled &&
                 !hoc_runtime_apply.ready &&
                 hoc_runtime_apply.require_proof_passed,
             1);
    hoc_capture_facts.saw_title_surface = 0;
    expect_i("DM1 HoC capture proof rejects NULL input",
             dm1_v1_startup_hoc_full_graphics_capture_proof_receipt_pc34(
                 NULL,
                 &hoc_capture_facts,
                 &hoc_capture_proof),
             0);
    hoc_production_receipt.packaged_proof.require_no_title_surface = 0;
    expect_i("DM1 HoC capture artifact rejects corrupt production receipt",
             dm1_v1_startup_hoc_full_graphics_capture_artifact_from_production_pc34(
                 &hoc_production_receipt,
                 &hoc_capture_artifact) &&
                 hoc_capture_artifact.handled &&
                 !hoc_capture_artifact.ready,
             1);
    expect_i("DM1 HoC capture artifact rejects NULL input",
             dm1_v1_startup_hoc_full_graphics_capture_artifact_from_production_pc34(
                 NULL,
                 &hoc_capture_artifact),
             0);
    expect_i("DM1 HoC full-start production receipt no-ops for CSB",
             dm1_v1_startup_hoc_full_start_production_receipt_pc34(
                 "csb",
                 &post,
                 &outcome,
                 &hoc_production_receipt) &&
                 !hoc_production_receipt.handled &&
                 !hoc_production_receipt.ready,
             1);
    expect_i("DM1 HoC full-start production receipt rejects NULL output",
             dm1_v1_startup_hoc_full_start_production_receipt_pc34(
                 "dm1",
                 &post,
                 &outcome,
                 NULL),
             0);
    hoc_full_graphics_proof.expected_map_width = 4;
    expect_i("DM1 HoC production hook rejects corrupt proof metadata",
             dm1_v1_startup_hoc_production_full_start_hook_from_proof_pc34(
                 &hoc_full_graphics_proof,
                 &hoc_production_hook) &&
                 hoc_production_hook.handled &&
                 !hoc_production_hook.ready,
             1);
    expect_i("DM1 HoC production hook rejects NULL input",
             dm1_v1_startup_hoc_production_full_start_hook_from_proof_pc34(
                 NULL,
                 &hoc_production_hook),
             0);
    hoc_host_plan.ready = 0;
    expect_i("DM1 HoC packaged proof rejects unready host plan",
             dm1_v1_startup_hoc_packaged_full_graphics_proof_from_host_plan_pc34(
                 &hoc_host_plan,
                 &hoc_full_graphics_proof) &&
                 hoc_full_graphics_proof.handled &&
                 !hoc_full_graphics_proof.ready &&
                 hoc_full_graphics_proof.capture_required,
             1);
    expect_i("DM1 HoC packaged proof rejects NULL input",
             dm1_v1_startup_hoc_packaged_full_graphics_proof_from_host_plan_pc34(
                 NULL,
                 &hoc_full_graphics_proof),
             0);
    hoc_first_frame.hoc_render_commands[0].kind =
        DM1_V1_STARTUP_HOC_RENDER_COMMAND_HALL_MIRRORS_PC34;
    expect_i("DM1 HoC host render plan rejects wrong command order",
             dm1_v1_startup_hoc_host_render_plan_from_first_frame_pc34(
                 &hoc_first_frame,
                 &hoc_host_plan) &&
                 hoc_host_plan.handled &&
                 !hoc_host_plan.ready,
             1);
    expect_i("DM1 HoC host render plan rejects NULL input",
             dm1_v1_startup_hoc_host_render_plan_from_first_frame_pc34(
                 NULL,
                 &hoc_host_plan),
             0);
    expect_i("DM1 HoC first-frame no-op for CSB",
             dm1_v1_startup_hoc_first_frame_receipt_pc34(
                 "csb",
                 &post,
                 &outcome,
                 &hoc_first_frame) &&
                 hoc_first_frame.handled == 0,
             1);
    expect_i("DM1 HoC first-frame rejects missing plan",
             dm1_v1_startup_hoc_first_frame_receipt_pc34(
                 "dm1",
                 NULL,
                 &outcome,
                 &hoc_first_frame),
             0);
    expect_i("CSB post-launch plan does not use DM1 title",
             dm1_v1_startup_handoff_post_launch_plan_pc34("csb", &post) &&
                 post.play_title == 0 &&
                 post.play_entrance == 0,
             1);
    expect_i("NULL post-launch plan rejects missing output",
             dm1_v1_startup_handoff_post_launch_plan_pc34("dm1", NULL),
             0);

    memset(&fake, 0, sizeof(fake));
    fake.entrance_command = 2;
    callbacks = fake_callbacks(&fake);
    expect_i("DM1 prelude executor succeeds",
             dm1_v1_startup_execute_handoff_prelude_pc34("dm1", &callbacks),
             1);
    expect_i("DM1 prelude executor owns SWSH order",
             strcmp(fake.order, "RSD"),
             0);
    expect_i("DM1 prelude executor brackets media receipt",
             fake.prelude_begin_count == 1 &&
                 fake.prelude_end_count == 1 &&
                 fake.prelude_media.handled &&
                 fake.prelude_media.play_swsh,
             1);

    memset(&fake, 0, sizeof(fake));
    fake.entrance_command = 2;
    callbacks = fake_callbacks(&fake);
    expect_i("DM1 post-launch executor succeeds",
             dm1_v1_startup_execute_handoff_post_launch_pc34(
                 "dm1",
                 &callbacks,
                 &title_played,
                 &entrance_command),
             1);
    expect_i("DM1 post-launch executor owns TITLE then ENTRANCE order",
             strcmp(fake.order, "RTE"),
             0);
    expect_i("DM1 post-launch executor brackets media receipt",
             fake.post_begin_count == 1 &&
                 fake.post_end_count == 1 &&
                 fake.post_media.handled &&
                 fake.post_media.play_title,
             1);
    expect_i("DM1 post-launch executor brackets entrance receipt",
             fake.post_entrance.valid &&
                 fake.post_entrance.mapIndex == DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
                 fake.post_entrance.partyDirection ==
                     DM1_V1_ENTRANCE_DIRECTION_SOUTH_PC34,
             1);
    expect_i("DM1 post-launch executor brackets entrance timing",
             fake.post_media.entrance_source_animation_steps ==
                 ENTRANCE_Compat_GetSourceAnimationStepCount() &&
                 fake.post_media.entrance_door_step_count ==
                     ENTRANCE_Compat_GetDoorAnimationStepCount(),
             1);
    expect_i("DM1 post-launch executor reports title played",
             title_played,
             1);
    expect_i("DM1 post-launch executor returns entrance command",
             entrance_command,
             2);
    expect_i("DM1 post-launch executor keeps entrance timeout",
             fake.entrance_timeout_ms,
             1200);
    memset(&outcome, 0, sizeof(outcome));
    expect_i("DM1 post-launch outcome executor succeeds",
             dm1_v1_startup_execute_handoff_post_launch_outcome_pc34(
                 "dm1",
                 &callbacks,
                 &outcome),
             1);
    expect_i("DM1 post-launch outcome reports title",
             outcome.title_played,
             1);
    expect_i("DM1 post-launch outcome maps resume action",
             (int)outcome.action,
             (int)DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34);

    memset(&fake, 0, sizeof(fake));
    callbacks = fake_callbacks(&fake);
    expect_i("CSB prelude executor is a no-op for DM1 facade",
             dm1_v1_startup_execute_handoff_prelude_pc34("csb", &callbacks) &&
                 fake.order[0] == '\0',
             1);
    expect_i("CSB post-launch executor is a no-op for DM1 facade",
             dm1_v1_startup_execute_handoff_post_launch_pc34(
                 "csb",
                 &callbacks,
                 &title_played,
                 &entrance_command) &&
                 fake.order[0] == '\0' &&
                 title_played == 0 &&
                 entrance_command == 0,
             1);
    expect_i("CSB post-launch outcome executor remains no-op",
             dm1_v1_startup_execute_handoff_post_launch_outcome_pc34(
                 "csb",
                 &callbacks,
                 &outcome) &&
                 outcome.action == DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34 &&
                 outcome.title_played == 0 &&
                 outcome.entrance_command == 0,
             1);
    expect_i("NULL prelude executor callbacks reject",
             dm1_v1_startup_execute_handoff_prelude_pc34("dm1", NULL),
             0);
    expect_i("NULL post-launch executor callbacks reject",
             dm1_v1_startup_execute_handoff_post_launch_pc34(
                 "dm1",
                 NULL,
                 &title_played,
                 &entrance_command),
             0);
    callbacks = fake_callbacks(&fake);
    callbacks.play_swsh = NULL;
    expect_i("DM1 prelude executor rejects missing required SWSH callback",
             dm1_v1_startup_execute_handoff_prelude_pc34("dm1", &callbacks),
             0);
    callbacks = fake_callbacks(&fake);
    callbacks.play_entrance = NULL;
    expect_i("DM1 post-launch executor rejects missing required entrance callback",
             dm1_v1_startup_execute_handoff_post_launch_pc34(
                 "dm1",
                 &callbacks,
                 &title_played,
                 &entrance_command),
             0);
    expect_i("DM1 outcome maps enter command",
             dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(1,
                                                                       &outcome) &&
                 outcome.action == DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34,
             1);
    expect_i("DM1 outcome maps resume command",
             dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(2,
                                                                       &outcome) &&
                 outcome.action == DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34,
             1);
    expect_i("DM1 outcome maps quit command",
             dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(-1,
                                                                       &outcome) &&
                 outcome.action == DM1_V1_STARTUP_HANDOFF_ACTION_QUIT_PC34,
             1);
    expect_i("DM1 outcome maps skipped entrance as nonfatal",
             dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(0,
                                                                       &outcome) &&
                 outcome.action ==
                     DM1_V1_STARTUP_HANDOFF_ACTION_SKIPPED_NONFATAL_PC34,
             1);
    expect_i("NULL outcome rejects missing output",
             dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(1, NULL),
             0);

    memset(&fake, 0, sizeof(fake));
    fake.resolve_ok = 1;
    fake.load_ok = 1;
    fake.used_backup = 1;
    snprintf(fake.resolved_path, sizeof(fake.resolved_path), "%s", "/tmp/dm1.sav");
    host_callbacks = fake_host_callbacks(&fake);
    (void)dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(2, &outcome);
    expect_i("DM1 host apply resume succeeds",
             dm1_v1_startup_apply_handoff_outcome_pc34(&outcome,
                                                       "dm1",
                                                       &host_callbacks,
                                                       &apply_result),
             1);
    expect_i("DM1 host apply resume handled",
             apply_result.handled,
             1);
    expect_i("DM1 host apply resume loaded",
             apply_result.resume_loaded,
             1);
    expect_i("DM1 host apply resume keeps backup flag",
             apply_result.resume_used_backup,
             1);
    expect_i("DM1 host apply resume activates game",
             fake.active,
             1);
    expect_i("DM1 host apply resume logs loaded",
             fake.log_loaded,
             1);
    expect_i("DM1 host apply resume path copied",
             strcmp(apply_result.resume_path, "/tmp/dm1.sav"),
             0);

    memset(&fake, 0, sizeof(fake));
    fake.resolve_ok = 1;
    fake.load_ok = 0;
    snprintf(fake.resolved_path, sizeof(fake.resolved_path), "%s", "/tmp/missing.sav");
    host_callbacks = fake_host_callbacks(&fake);
    expect_i("DM1 host apply missing resume remains nonfatal",
             dm1_v1_startup_apply_handoff_outcome_pc34(&outcome,
                                                       "dm1",
                                                       &host_callbacks,
                                                       &apply_result),
             1);
    expect_i("DM1 host apply missing resume logs missing",
             fake.log_missing,
             1);
    expect_i("DM1 host apply missing resume not loaded",
             apply_result.resume_loaded,
             0);

    memset(&fake, 0, sizeof(fake));
    host_callbacks = fake_host_callbacks(&fake);
    (void)dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(-1, &outcome);
    expect_i("DM1 host apply quit succeeds",
             dm1_v1_startup_apply_handoff_outcome_pc34(&outcome,
                                                       "dm1",
                                                       &host_callbacks,
                                                       &apply_result),
             1);
    expect_i("DM1 host apply quit deactivates game",
             fake.active,
             0);
    expect_i("DM1 host apply quit requested",
             apply_result.quit_requested,
             1);

    memset(&fake, 0, sizeof(fake));
    host_callbacks = fake_host_callbacks(&fake);
    (void)dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(0, &outcome);
    expect_i("DM1 host apply skipped entrance succeeds",
             dm1_v1_startup_apply_handoff_outcome_pc34(&outcome,
                                                       "dm1",
                                                       &host_callbacks,
                                                       &apply_result),
             1);
    expect_i("DM1 host apply skipped entrance logs",
             fake.log_skipped,
             1);
    expect_i("DM1 host apply enter/no-op succeeds",
             dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(1,
                                                                       &outcome) &&
                 dm1_v1_startup_apply_handoff_outcome_pc34(&outcome,
                                                           "dm1",
                                                           &host_callbacks,
                                                           &apply_result) &&
                 apply_result.handled == 0,
             1);
    outcome.title_played = 1;
    expect_i("DM1 full graphics handoff enters HoC runtime",
             dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
                 "dm1",
                 "dm1",
                 &outcome,
                 &apply_result,
                 &runtime_handoff) &&
                 runtime_handoff.handled &&
                 runtime_handoff.full_graphics_consumed &&
                 runtime_handoff.hoc_runtime_ready &&
                 runtime_handoff.champion_mirror_startup_handoff_ready &&
                 runtime_handoff.champion_mirror_startup_route.handled &&
                 runtime_handoff.champion_mirror_startup_route.route ==
                     DM1_V1_ENTRANCE_MENU_ROUTE_HALL_PC34 &&
                 runtime_handoff.champion_mirror_startup_route.showHall &&
                 runtime_handoff.champion_mirror_startup_route.needsRedraw &&
                 runtime_handoff.champion_mirror_startup_input_ready &&
                 runtime_handoff.champion_mirror_startup_panel_clear &&
                 runtime_handoff.champion_mirror_startup_blocks_enter &&
                 runtime_handoff.champion_mirror_startup_overlay_commands_ready &&
                 runtime_handoff.champion_mirror_startup_overlay_command_count == 1 &&
                 runtime_handoff.champion_mirror_startup_overlay_commands[0]
                     .valid &&
                 runtime_handoff.champion_mirror_startup_overlay_commands[0]
                     .kind == DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 runtime_handoff.champion_mirror_startup_overlay_commands[0]
                     .suppressThingPayloads &&
                 runtime_handoff.champion_mirror_startup_route
                         .selectedMirrorIndex < 0 &&
                 !runtime_handoff.champion_mirror_startup_route
                      .showChampionPanel &&
                 !runtime_handoff.champion_mirror_startup_route.canEnterDungeon &&
                 runtime_handoff.hoc_first_frame_ready &&
                 runtime_handoff.runtime_first_frame_ready &&
                 runtime_handoff.draw_opened_runtime &&
                 !runtime_handoff.suppress_draw_opened,
             1);

    memset(&fake, 0, sizeof(fake));
    fake.entrance_command = 2;
    fake.resolve_ok = 1;
    fake.load_ok = 1;
    snprintf(fake.resolved_path, sizeof(fake.resolved_path), "%s", "/tmp/combined.sav");
    callbacks = fake_callbacks(&fake);
    host_callbacks = fake_host_callbacks(&fake);
    expect_i("DM1 combined post-launch apply succeeds",
             dm1_v1_startup_execute_handoff_post_launch_and_apply_pc34(
                 "dm1",
                 &callbacks,
                 &host_callbacks,
                 &outcome,
                 &apply_result),
             1);
    expect_i("DM1 combined post-launch apply owns render order",
             strcmp(fake.order, "RTE"),
             0);
    expect_i("DM1 combined post-launch apply loads resume",
             apply_result.resume_loaded,
             1);
    expect_i("DM1 combined post-launch apply maps resume action",
             outcome.action == DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34,
             1);
    expect_i("DM1 full graphics handoff draws loaded resume runtime",
             dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
                 "dm1",
                 "dm1",
                 &outcome,
                 &apply_result,
                 &runtime_handoff) &&
                 runtime_handoff.full_graphics_consumed &&
                 runtime_handoff.resumed_runtime_ready &&
                 !runtime_handoff.champion_mirror_startup_handoff_ready &&
                 !runtime_handoff.champion_mirror_startup_input_ready &&
                 !runtime_handoff.champion_mirror_startup_panel_clear &&
                 !runtime_handoff.champion_mirror_startup_blocks_enter &&
                 !runtime_handoff.hoc_first_frame_ready &&
                 runtime_handoff.runtime_first_frame_ready &&
                 runtime_handoff.draw_opened_runtime &&
                 !runtime_handoff.hoc_runtime_ready,
             1);

    memset(&fake, 0, sizeof(fake));
    callbacks = fake_callbacks(&fake);
    host_callbacks = fake_host_callbacks(&fake);
    expect_i("CSB combined post-launch apply stays no-op",
             dm1_v1_startup_execute_handoff_post_launch_and_apply_pc34(
                 "csb",
                 &callbacks,
                 &host_callbacks,
                 &outcome,
                 &apply_result) &&
                 fake.order[0] == '\0' &&
                 outcome.action == DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34 &&
                 apply_result.handled == 0,
             1);
    expect_i("CSB full graphics handoff no-ops",
             dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
                 "csb",
                 "csb",
                 &outcome,
                 &apply_result,
                 &runtime_handoff) &&
                 runtime_handoff.handled == 0,
             1);
    expect_i("NULL combined post-launch apply rejects missing outcome",
             dm1_v1_startup_execute_handoff_post_launch_and_apply_pc34(
                 "dm1",
                 &callbacks,
                 &host_callbacks,
                 NULL,
                 &apply_result),
             0);
    expect_i("NULL combined post-launch apply rejects missing result",
             dm1_v1_startup_execute_handoff_post_launch_and_apply_pc34(
                 "dm1",
                 &callbacks,
                 &host_callbacks,
                 &outcome,
                 NULL),
             0);

    memset(&fake, 0, sizeof(fake));
    fake.open_ok = 1;
    fake.entrance_command = 1;
    callbacks = fake_callbacks(&fake);
    host_callbacks = fake_host_callbacks(&fake);
    launch_callbacks = fake_selected_launch_callbacks(&fake,
                                                      &callbacks,
                                                      &host_callbacks);
    expect_i("DM1 selected launch transaction succeeds",
             dm1_v1_startup_execute_selected_launch_transaction_pc34(
                 "dm1",
                 &launch_callbacks,
                 &launch_result),
             1);
    expect_i("DM1 selected launch transaction is handled",
             launch_result.handled,
             1);
    expect_i("DM1 selected launch transaction opens",
             launch_result.opened,
             1);
    expect_i("DM1 selected launch transaction owns full order",
             strcmp(fake.order, "RSDOARTEG"),
             0);
    expect_i("DM1 selected launch transaction brackets startup media",
             fake.prelude_begin_count == 1 &&
                 fake.prelude_end_count == 1 &&
                 fake.post_begin_count == 1 &&
                 fake.post_end_count == 1 &&
                 fake.prelude_media.handled &&
                 fake.post_media.handled,
             1);
    expect_i("DM1 selected launch transaction brackets entrance receipt",
             fake.post_entrance.valid &&
                 fake.post_entrance.corridorCount == 6,
             1);
    expect_i("DM1 selected launch transaction draws enter path",
             fake.draw_opened,
             1);
    expect_i("DM1 selected launch transaction exposes HoC handoff receipt",
                 launch_result.runtime_handoff_receipt.handled &&
                 launch_result.runtime_handoff_receipt.full_graphics_consumed &&
                 launch_result.runtime_handoff_receipt.hoc_runtime_ready &&
                 launch_result.runtime_handoff_receipt
                     .champion_mirror_startup_handoff_ready &&
                 launch_result.runtime_handoff_receipt
                     .champion_mirror_startup_route.route ==
                     DM1_V1_ENTRANCE_MENU_ROUTE_HALL_PC34 &&
                 launch_result.runtime_handoff_receipt
                     .champion_mirror_startup_input_ready &&
                 launch_result.runtime_handoff_receipt
                     .champion_mirror_startup_panel_clear &&
                 launch_result.runtime_handoff_receipt
                     .champion_mirror_startup_blocks_enter &&
                 launch_result.runtime_handoff_receipt.hoc_first_frame_ready &&
                 launch_result.runtime_handoff_receipt.runtime_first_frame_ready &&
                 launch_result.runtime_handoff_receipt.draw_opened_runtime,
             1);

    memset(&fake, 0, sizeof(fake));
    fake.open_ok = 1;
    fake.entrance_command = -1;
    callbacks = fake_callbacks(&fake);
    host_callbacks = fake_host_callbacks(&fake);
    launch_callbacks = fake_selected_launch_callbacks(&fake,
                                                      &callbacks,
                                                      &host_callbacks);
    expect_i("DM1 selected launch transaction handles quit",
             dm1_v1_startup_execute_selected_launch_transaction_pc34(
                 "dm1",
                 &launch_callbacks,
                 &launch_result) &&
                 launch_result.host_apply_result.quit_requested &&
                 !launch_result.runtime_handoff_receipt
                      .champion_mirror_startup_handoff_ready &&
                 !launch_result.runtime_handoff_receipt.runtime_first_frame_ready &&
                 launch_result.runtime_handoff_receipt.suppress_draw_opened &&
                 fake.draw_opened == 0,
             1);

    memset(&fake, 0, sizeof(fake));
    fake.open_ok = 0;
    callbacks = fake_callbacks(&fake);
    host_callbacks = fake_host_callbacks(&fake);
    launch_callbacks = fake_selected_launch_callbacks(&fake,
                                                      &callbacks,
                                                      &host_callbacks);
    expect_i("DM1 selected launch transaction reports open failure",
             dm1_v1_startup_execute_selected_launch_transaction_pc34(
                 "dm1",
                 &launch_callbacks,
                 &launch_result) &&
                 launch_result.handled &&
                 launch_result.launch_failed &&
                 fake.mark_failed,
             1);
    expect_i("DM1 selected launch open failure order",
             strcmp(fake.order, "RSDOM"),
             0);

    memset(&fake, 0, sizeof(fake));
    callbacks = fake_callbacks(&fake);
    host_callbacks = fake_host_callbacks(&fake);
    launch_callbacks = fake_selected_launch_callbacks(&fake,
                                                      &callbacks,
                                                      &host_callbacks);
    expect_i("CSB selected launch transaction is no-op",
             dm1_v1_startup_execute_selected_launch_transaction_pc34(
                 "csb",
                 &launch_callbacks,
                 &launch_result) &&
                 launch_result.handled == 0 &&
                 fake.order[0] == '\0',
             1);
    expect_i("NULL selected launch transaction rejects result",
             dm1_v1_startup_execute_selected_launch_transaction_pc34(
                 "dm1",
                 &launch_callbacks,
                 NULL),
             0);
    launch_callbacks.open_selected_entry = NULL;
    expect_i("DM1 selected launch transaction rejects missing open callback",
             dm1_v1_startup_execute_selected_launch_transaction_pc34(
                 "dm1",
                 &launch_callbacks,
                 &launch_result),
             0);
    launch_callbacks = fake_selected_launch_callbacks(&fake,
                                                      NULL,
                                                      &host_callbacks);
    expect_i("DM1 selected launch transaction rejects missing handoff callbacks",
             dm1_v1_startup_execute_selected_launch_transaction_pc34(
                 "dm1",
                 &launch_callbacks,
                 &launch_result),
             0);
    launch_callbacks = fake_selected_launch_callbacks(&fake,
                                                      &callbacks,
                                                      NULL);
    expect_i("DM1 selected launch transaction rejects missing host callbacks",
             dm1_v1_startup_execute_selected_launch_transaction_pc34(
                 "dm1",
                 &launch_callbacks,
                 &launch_result),
             0);
    expect_i("NULL host apply rejects missing outcome",
             dm1_v1_startup_apply_handoff_outcome_pc34(NULL,
                                                       "dm1",
                                                       &host_callbacks,
                                                       &apply_result),
             0);
    expect_i("NULL host apply rejects missing result",
             dm1_v1_startup_apply_handoff_outcome_pc34(&outcome,
                                                       "dm1",
                                                       &host_callbacks,
                                                       NULL),
             0);

    expect_i("receipt unloaded rc",
             dm1_v1_startup_receipt_phase_pc34(0, 0, phase, sizeof(phase)),
             1);
    expect_i("receipt unloaded phase",
             strcmp(phase, "dm1-loading"),
             0);
    expect_i("receipt launcher runtime rc",
             dm1_v1_startup_receipt_phase_pc34(1, 0, phase, sizeof(phase)),
             1);
    expect_i("receipt launcher runtime phase",
             strcmp(phase, "dm1-runtime"),
             0);
    expect_i("receipt direct runtime rc",
             dm1_v1_startup_receipt_phase_pc34(1, 1, phase, sizeof(phase)),
             1);
    expect_i("receipt direct runtime phase",
             strcmp(phase, "dm1-runtime-direct"),
             0);
    expect_i("receipt null phase rejected",
             dm1_v1_startup_receipt_phase_pc34(1, 0, NULL, 0),
             0);

    expect_i("boot probe receipt launcher rc",
             dm1_v1_startup_boot_probe_receipt_pc34(
                 1, 0, phase, sizeof(phase),
                 &startup_active,
                 animation, sizeof(animation),
                 &animation_active,
                 &title_frame,
                 &title_frame_max,
                 &title_ready),
             1);
    expect_i("boot probe receipt launcher phase",
             strcmp(phase, "dm1-runtime"),
             0);
    expect_i("boot probe receipt launcher animation",
             strcmp(animation, "dm1-title"),
             0);
    expect_i("boot probe receipt launcher startup inactive",
             startup_active,
             0);
    expect_i("boot probe receipt launcher animation inactive",
             animation_active,
             0);
    expect_i("boot probe receipt launcher title frame",
             title_frame,
             (int)V1_TITLE_DAT_FRAME_MAX);
    expect_i("boot probe receipt launcher title frame max",
             title_frame_max,
             (int)V1_TITLE_DAT_FRAME_MAX);
    expect_i("boot probe receipt launcher title ready",
             title_ready,
             1);

    expect_i("boot probe receipt direct rc",
             dm1_v1_startup_boot_probe_receipt_pc34(
                 1, 1, phase, sizeof(phase),
                 &startup_active,
                 animation, sizeof(animation),
                 &animation_active,
                 &title_frame,
                 &title_frame_max,
                 &title_ready),
             1);
    expect_i("boot probe receipt direct phase",
             strcmp(phase, "dm1-runtime-direct"),
             0);
    expect_i("boot probe receipt direct animation",
             strcmp(animation, "dm1-title-bypassed"),
             0);
    expect_i("boot probe receipt rejects incomplete output",
             dm1_v1_startup_boot_probe_receipt_pc34(
                 1, 0, phase, sizeof(phase),
                 NULL,
                 animation, sizeof(animation),
                 &animation_active,
                 &title_frame,
                 &title_frame_max,
                 &title_ready),
             0);

    memset(&boot_facts, 0, sizeof(boot_facts));
    memset(&boot_receipt, 0, sizeof(boot_receipt));
    boot_facts.source_id = "dm1";
    boot_facts.level_loaded = 1;
    boot_facts.intro_bypassed = 0;
    boot_facts.map_index = 2;
    boot_facts.party_x = 3;
    boot_facts.party_y = 4;
    boot_facts.party_dir = 1;
    boot_facts.champion_count = 4;
    boot_facts.runtime_tick = 55;
    boot_facts.world_tick = 77u;
    expect_i("DM1 boot probe facts receipt succeeds",
             dm1_v1_startup_boot_probe_receipt_from_facts_pc34(
                 &boot_facts,
                 &boot_receipt),
             1);
    expect_i("DM1 boot probe facts receipt handled",
             boot_receipt.handled,
             1);
    expect_i("DM1 boot probe facts receipt phase",
             strcmp(boot_receipt.startup_phase, "dm1-runtime"),
             0);
    expect_i("DM1 boot probe facts receipt animation",
             strcmp(boot_receipt.startup_animation, "dm1-title"),
             0);
    expect_i("DM1 boot probe facts receipt keeps party x",
             boot_receipt.party_x,
             3);
    expect_i("DM1 boot probe facts receipt keeps world tick",
             (int)boot_receipt.world_tick,
             77);

    boot_facts.intro_bypassed = 1;
    expect_i("DM1 boot probe facts receipt direct phase",
             dm1_v1_startup_boot_probe_receipt_from_facts_pc34(
                 &boot_facts,
                 &boot_receipt) &&
                 strcmp(boot_receipt.startup_phase,
                        "dm1-runtime-direct") == 0 &&
                 boot_receipt.dm1_startup_intro_bypassed == 1,
             1);
    boot_facts.source_id = "csb";
    expect_i("non-DM1 boot probe facts receipt no-op",
             dm1_v1_startup_boot_probe_receipt_from_facts_pc34(
                 &boot_facts,
                 &boot_receipt) &&
                 boot_receipt.handled == 0,
             1);
    expect_i("NULL DM1 boot probe facts receipt rejects",
             dm1_v1_startup_boot_probe_receipt_from_facts_pc34(
                 NULL,
                 &boot_receipt),
             0);
}

int main(void) {
    check_swsh_to_title_boundary();
    check_title_to_menu_boundary();
    check_menu_to_entrance_wait_boundary();
    check_dm1_launch_path_bypass_contract();

    expect_truth("startup stage order is source-valid",
                 dm1_v1_startup_sequence_source_order_valid_pc34());
    expect_truth("startup source evidence is present",
                 dm1_v1_startup_sequence_source_evidence_pc34() != 0 &&
                 strstr(dm1_v1_startup_sequence_source_evidence_pc34(),
                        "TITLE.C") != 0 &&
                 strstr(dm1_v1_startup_sequence_source_evidence_pc34(),
                        "ENTRANCE.C") != 0);

    if (g_failures) {
        printf("dm1_v1_startup_intro_state_machine_gate failures=%d\n", g_failures);
        return 1;
    }
    printf("ok: DM1 V1 startup intro state-machine ordering is data-free and deterministic\n");
    return 0;
}
