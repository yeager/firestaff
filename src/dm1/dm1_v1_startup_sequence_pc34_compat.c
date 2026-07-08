#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "title_frontend_v1.h"
#include <stdio.h>
#include <string.h>

#define DM1_V1_STARTUP_TITLE_ZOOM_STEPS_PC34 18u
#define DM1_V1_STARTUP_TITLE_SOURCE_ANIMATION_STEPS_PC34 23u
#define DM1_V1_STARTUP_TITLE_FRAME_BANK_EQUIVALENT_STEPS_PC34 53u
#define DM1_V1_STARTUP_TITLE_POST_ZOOM_VBLANKS_PC34 2u
#define DM1_V1_STARTUP_TITLE_FINAL_GUARD_VBLANKS_PC34 1u
#define DM1_V1_STARTUP_TITLE_VBLANK_TICK_MS_PC34 55u

const char* dm1_v1_startup_stage_name_pc34(DM1_V1_StartupStage_PC34 stage) {
    switch (stage) {
        case DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34:
            return "SWSH_LOGO";
        case DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34:
            return "SWSH_RUN_START";
        case DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34:
            return "TITLE_BEGIN";
        case DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34:
            return "TITLE_LAST_FRAME";
        case DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34:
            return "MENU_ELIGIBLE";
        case DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34:
            return "ENTRANCE_WAIT";
    }
    return "UNKNOWN";
}

int dm1_v1_startup_stage_after_pc34(DM1_V1_StartupStage_PC34 later,
                                    DM1_V1_StartupStage_PC34 earlier) {
    return (unsigned int)later > (unsigned int)earlier;
}

int dm1_v1_startup_launch_path_bypasses_intro_pc34(
    DM1_V1_StartupLaunchPath_PC34 path) {
    switch (path) {
        case DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34:
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_CLI_PC34:
            /* Firestaff --game dm1 bypasses only the M12 launcher surface.
             * main_loop_m11.c still routes through the source-visible
             * SWSH -> TITLE -> ENTRANCE startup sequence before gameplay. */
            return 0;
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34:
            /* M11_GameView_StartDm1() is a focused test/dev entry point.
             * It intentionally starts the DM1 game view directly and must
             * not be confused with the ReDMCSB SWSH -> TITLE -> ENTRANCE
             * launcher handoff used by normal DM1 startup. */
            return 1;
    }
    return 1;
}

int dm1_v1_startup_intro_bypass_applies_to_source_pc34(const char* sourceId,
                                                       int bypassed) {
    return sourceId && strcmp(sourceId, "dm1") == 0 && bypassed ? 1 : 0;
}

int dm1_v1_startup_receipt_phase_pc34(int level_loaded,
                                      int intro_bypassed,
                                      char* out_phase,
                                      int out_phase_size) {
    const char* phase;

    if (!out_phase || out_phase_size <= 0) {
        return 0;
    }
    if (!level_loaded) {
        phase = "dm1-loading";
    } else {
        phase = intro_bypassed
            ? "dm1-runtime-direct"
            : "dm1-runtime";
    }
    snprintf(out_phase, (size_t)out_phase_size, "%s", phase);
    return 1;
}

int dm1_v1_startup_boot_probe_receipt_pc34(int level_loaded,
                                           int intro_bypassed,
                                           char* out_phase,
                                           int out_phase_size,
                                           int* out_startup_active,
                                           char* out_animation,
                                           int out_animation_size,
                                           int* out_animation_active,
                                           int* out_title_frame,
                                           int* out_title_frame_max,
                                           int* out_title_ready) {
    const char* animation;

    if (!out_phase || out_phase_size <= 0 ||
        !out_startup_active ||
        !out_animation || out_animation_size <= 0 ||
        !out_animation_active ||
        !out_title_frame ||
        !out_title_frame_max ||
        !out_title_ready) {
        return 0;
    }
    if (!dm1_v1_startup_receipt_phase_pc34(level_loaded,
                                           intro_bypassed,
                                           out_phase,
                                           out_phase_size)) {
        return 0;
    }
    /* ReDMCSB: SWSH.C -> STARTUP1.C -> TITLE.C F0437 -> ENTRANCE.C.
     * M11 reaches this receipt after the source-visible launcher/title
     * path has completed or through the explicit direct-view bypass. */
    animation = intro_bypassed ? "dm1-title-bypassed" : "dm1-title";
    snprintf(out_animation, (size_t)out_animation_size, "%s", animation);
    *out_startup_active = 0;
    *out_animation_active = 0;
    *out_title_frame = V1_TITLE_DAT_FRAME_MAX;
    *out_title_frame_max = V1_TITLE_DAT_FRAME_MAX;
    *out_title_ready = 1;
    return 1;
}

int dm1_v1_startup_sequence_source_order_valid_pc34(void) {
    /* ReDMCSB startup source order:
     * SWSH.C:39-47 runs START.PRG after the FTL palette program;
     * STARTUP1.C:143 calls TITLE.C F0437_STARTEND_DrawTitle();
     * TITLE.C:319-409 draws PRESENTS, title zoom, STRIKES BACK and final
     * guard; ENTRANCE.C:850-883 then enters the entrance wait loop.
     */
    return dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34,
                                           DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34,
                                           DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34,
                                           DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34,
                                           DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34,
                                           DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34);
}

const char* dm1_v1_startup_sequence_source_evidence_pc34(void) {
    return "ReDMCSB SWSH.C:39-47 -> STARTUP1.C:143 -> TITLE.C:319-409 -> ENTRANCE.C:850-883";
}

unsigned int dm1_v1_startup_title_zoom_steps_pc34(void) {
    /* ReDMCSB: TITLE.C F0437 lines 340-360 prepares 18 shrinked title
     * bitmaps, then lines 385-387 blit them in reverse order. */
    return DM1_V1_STARTUP_TITLE_ZOOM_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_source_animation_steps_pc34(void) {
    /* PRESENTS + 18 zoom blits + 2 post-zoom waits + STRIKES BACK + final
     * guard. This is the source event count before Firestaff's frame-bank
     * cadence padding. */
    return DM1_V1_STARTUP_TITLE_SOURCE_ANIMATION_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_frame_bank_equivalent_steps_pc34(void) {
    return DM1_V1_STARTUP_TITLE_FRAME_BANK_EQUIVALENT_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_presents_hold_vblanks_pc34(void) {
    return DM1_V1_STARTUP_TITLE_FRAME_BANK_EQUIVALENT_STEPS_PC34 -
           DM1_V1_STARTUP_TITLE_SOURCE_ANIMATION_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_vblank_tick_ms_pc34(void) {
    return DM1_V1_STARTUP_TITLE_VBLANK_TICK_MS_PC34;
}

unsigned int dm1_v1_startup_title_presents_hold_ms_pc34(void) {
    return dm1_v1_startup_title_presents_hold_vblanks_pc34() *
           DM1_V1_STARTUP_TITLE_VBLANK_TICK_MS_PC34;
}

unsigned int dm1_v1_startup_title_post_zoom_vblanks_pc34(void) {
    return DM1_V1_STARTUP_TITLE_POST_ZOOM_VBLANKS_PC34;
}

unsigned int dm1_v1_startup_title_final_guard_vblanks_pc34(void) {
    return DM1_V1_STARTUP_TITLE_FINAL_GUARD_VBLANKS_PC34;
}
