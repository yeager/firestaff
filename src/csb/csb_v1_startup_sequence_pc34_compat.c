#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "title_frontend_v1.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>

enum {
    CSB_V1_TITLE_TOTAL_TICKS_PC34 = 53,
    CSB_V1_TITLE_PRESENTS_TICKS_PC34 = 30,
    CSB_V1_ENTRANCE_WAIT_SOURCE_STEP_PC34 = 4,
    CSB_V1_ENTRANCE_PRE_OPEN_DELAY_TICKS_PC34 = 20,
    CSB_V1_ENTRANCE_CREDITS_TICKS_PC34 = 1800,
    CSB_V1_ENTRANCE_DOOR_SCREEN_Y_PC34 = 28,
    CSB_V1_GRAPHIC_TITLE_PC34 = 1,
    CSB_V1_GRAPHIC_ENTRANCE_LEFT_DOOR_PC34 = 2,
    CSB_V1_GRAPHIC_ENTRANCE_RIGHT_DOOR_PC34 = 3,
    CSB_V1_GRAPHIC_ENTRANCE_SCREEN_PC34 = 4,
    CSB_V1_GRAPHIC_ENTRANCE_CREDITS_PC34 = 5,
    CSB_V1_RENDER_TEXT_STYLE_SMALL_PC34 = 1,
    CSB_V1_RENDER_TEXT_STYLE_TITLE_PC34 = 2,
    CSB_V1_RENDER_TEXT_STYLE_SHADOW_PC34 = 3
};

const char* csb_v1_startup_stage_name_pc34(CSB_V1_StartupStage_PC34 stage)
{
    switch (stage) {
        case CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34:
            return "TITLE_PRESENTS";
        case CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34:
            return "TITLE_CHAOS_ZOOM";
        case CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34:
            return "TITLE_STRIKES_BACK";
        case CSB_V1_STARTUP_STAGE_ENTRANCE_LOAD_BLACK_PC34:
            return "ENTRANCE_LOAD_BLACK";
        case CSB_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34:
            return "ENTRANCE_WAIT";
        case CSB_V1_STARTUP_STAGE_ENTRANCE_PRE_OPEN_DELAY_PC34:
            return "ENTRANCE_PRE_OPEN_DELAY";
        case CSB_V1_STARTUP_STAGE_ENTRANCE_DOOR_OPENING_PC34:
            return "ENTRANCE_DOOR_OPENING";
        case CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34:
            return "DUNGEON_RUNTIME";
    }
    return "UNKNOWN";
}

int csb_v1_startup_stage_after_pc34(CSB_V1_StartupStage_PC34 later,
                                    CSB_V1_StartupStage_PC34 earlier)
{
    return (unsigned int)later > (unsigned int)earlier;
}

int csb_v1_startup_title_total_ticks_pc34(void)
{
    return CSB_V1_TITLE_TOTAL_TICKS_PC34;
}

int csb_v1_startup_title_presents_ticks_pc34(void)
{
    return CSB_V1_TITLE_PRESENTS_TICKS_PC34;
}

int csb_v1_startup_title_stage_for_frame_pc34(int frame)
{
    V1_TitleFrontendSourceAnimationStep step;
    unsigned int sourceStep;

    if (frame < CSB_V1_TITLE_PRESENTS_TICKS_PC34) {
        return CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34;
    }
    sourceStep = csb_v1_startup_title_source_step_for_frame_pc34(frame);
    if (!V1_TitleFrontend_GetSourceAnimationStep(sourceStep, &step)) {
        return CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34;
    }
    switch (step.kind) {
        case V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS:
            return CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34;
        case V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT:
            return CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34;
        case V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT:
        case V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK:
        case V1_TITLE_FRONTEND_SOURCE_EVENT_FINAL_GUARD_VBLANK:
        case V1_TITLE_FRONTEND_SOURCE_EVENT_MENU_ELIGIBLE:
        default:
            return CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34;
    }
}

unsigned int csb_v1_startup_title_source_step_for_frame_pc34(int frame)
{
    if (frame < CSB_V1_TITLE_PRESENTS_TICKS_PC34) {
        return 1u;
    }
    return (unsigned int)(frame - CSB_V1_TITLE_PRESENTS_TICKS_PC34 + 1);
}

static void csb_v1_startup_clear_title_rect_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    plan->title_source_x = 0;
    plan->title_source_y = 0;
    plan->title_source_w = 0;
    plan->title_source_h = 0;
    plan->title_blit_kind = CSB_V1_STARTUP_TITLE_BLIT_NONE_PC34;
    plan->title_transparent_color = -1;
    plan->title_empty_fallback_x = 0;
    plan->title_empty_fallback_y = 0;
    plan->title_empty_fallback_style = 0;
    plan->title_empty_fallback_text = NULL;
    plan->title_dest_x = 0;
    plan->title_dest_y = 0;
    plan->title_dest_w = 0;
    plan->title_dest_h = 0;
    plan->title_special_palette = -1;
    plan->special_palette = -1;
}

static void csb_v1_startup_clear_door_rects_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    plan->closed_left_source_x = 0;
    plan->closed_left_source_y = 0;
    plan->closed_left_dest_x = 0;
    plan->closed_left_dest_y = 0;
    plan->closed_left_w = 0;
    plan->closed_left_h = 0;
    plan->closed_right_source_x = 0;
    plan->closed_right_source_y = 0;
    plan->closed_right_dest_x = 0;
    plan->closed_right_dest_y = 0;
    plan->closed_right_w = 0;
    plan->closed_right_h = 0;
    plan->opening_door_valid = 0;
    plan->opening_door_step = 0;
    plan->opening_left_source_x = 0;
    plan->opening_left_source_y = 0;
    plan->opening_left_dest_x = 0;
    plan->opening_left_dest_y = 0;
    plan->opening_left_w = 0;
    plan->opening_left_h = 0;
    plan->opening_right_source_x = 0;
    plan->opening_right_source_y = 0;
    plan->opening_right_dest_x = 0;
    plan->opening_right_dest_y = 0;
    plan->opening_right_w = 0;
    plan->opening_right_h = 0;
}

static void csb_v1_startup_clear_fallback_text_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    plan->fallback_title_x = 0;
    plan->fallback_title_y = 0;
    plan->fallback_title_style = 0;
    plan->fallback_title_text = NULL;
    plan->fallback_subtitle_x = 0;
    plan->fallback_subtitle_y = 0;
    plan->fallback_subtitle_style = 0;
    plan->fallback_subtitle_text = NULL;
    plan->fallback_status_x = 0;
    plan->fallback_status_y = 0;
    plan->fallback_status_style = 0;
    plan->fallback_status_text = NULL;
    plan->fallback_detail_x = 0;
    plan->fallback_detail_y = 0;
    plan->fallback_detail_style = 0;
    plan->fallback_detail_text = NULL;
    plan->fallback_prompt_x = 0;
    plan->fallback_prompt_y = 0;
    plan->fallback_prompt_style = 0;
    plan->fallback_prompt_text = NULL;
}

static void csb_v1_startup_set_credits_fallback_text_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    /* ReDMCSB ENTRANCE.C F0442/F0806 draws the C005 credits page after the
     * C203 entrance command. These text rows are Firestaff's no-asset
     * fallback, so keep their semantic placement in the CSB startup plan. */
    plan->fallback_title_x = 38;
    plan->fallback_title_y = 42;
    plan->fallback_title_style = CSB_V1_RENDER_TEXT_STYLE_TITLE_PC34;
    plan->fallback_title_text = "CHAOS STRIKES BACK";
    plan->fallback_subtitle_x = 38;
    plan->fallback_subtitle_y = 68;
    plan->fallback_subtitle_style = CSB_V1_RENDER_TEXT_STYLE_SHADOW_PC34;
    plan->fallback_subtitle_text = "CREDITS";
    plan->fallback_prompt_x = 38;
    plan->fallback_prompt_y = 154;
    plan->fallback_prompt_style = CSB_V1_RENDER_TEXT_STYLE_SMALL_PC34;
    plan->fallback_prompt_text = "PRESS ENTER";
}

static void csb_v1_startup_set_entrance_fallback_text_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    /* ReDMCSB ENTRANCE.C F0806/F0441 owns the C004 entrance wait surface and
     * input prompt.  These rows are Firestaff's no-asset fallback composition,
     * so keep their positions/styles in the CSB startup render plan. */
    plan->fallback_title_x = 38;
    plan->fallback_title_y = 42;
    plan->fallback_title_style = CSB_V1_RENDER_TEXT_STYLE_TITLE_PC34;
    plan->fallback_title_text = "CHAOS STRIKES BACK";
    plan->fallback_subtitle_x = 38;
    plan->fallback_subtitle_y = 64;
    plan->fallback_subtitle_style = CSB_V1_RENDER_TEXT_STYLE_SHADOW_PC34;
    plan->fallback_subtitle_text = "ENTRANCE";
    plan->fallback_status_x = 38;
    plan->fallback_status_y = 84;
    plan->fallback_status_style = CSB_V1_RENDER_TEXT_STYLE_SMALL_PC34;
    plan->fallback_status_text = "CSB RUNTIME READY";
    plan->fallback_detail_x = 38;
    plan->fallback_detail_y = 96;
    plan->fallback_detail_style = CSB_V1_RENDER_TEXT_STYLE_SMALL_PC34;
    plan->fallback_detail_text = "START";
    plan->fallback_prompt_x = 38;
    plan->fallback_prompt_y = 154;
    plan->fallback_prompt_style = CSB_V1_RENDER_TEXT_STYLE_SHADOW_PC34;
    plan->fallback_prompt_text = "PRESS ENTER";
}

static void csb_v1_startup_set_closed_door_rects_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    /* ReDMCSB ENTRANCE.C F0806 lines 721-778 loads C002/C003 door
     * bitmaps before F0439/F0441 presents C004 at the entrance wait loop.
     * DATA.C places the closed left and right doors at screen y=28. */
    plan->closed_left_source_x = 0;
    plan->closed_left_source_y = 0;
    plan->closed_left_dest_x = 0;
    plan->closed_left_dest_y = CSB_V1_ENTRANCE_DOOR_SCREEN_Y_PC34;
    plan->closed_left_w = 105;
    plan->closed_left_h = 161;
    plan->closed_right_source_x = 0;
    plan->closed_right_source_y = 0;
    plan->closed_right_dest_x = 105;
    plan->closed_right_dest_y = CSB_V1_ENTRANCE_DOOR_SCREEN_Y_PC34;
    plan->closed_right_w = 127;
    plan->closed_right_h = 161;
}

static void csb_v1_startup_set_opening_door_rects_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    int animationStep;
    int leftRight;
    int rightLeft;

    if (!plan || plan->opening_step <= 0) {
        return;
    }
    /* ReDMCSB ENTRANCE.C F0438/F0807 lines 142-304 run 31 source
     * animation steps, with moving C002/C003 strips copied at screen y=28. */
    if (plan->opening_step < 1 || plan->opening_step >= 32) {
        return;
    }
    animationStep = plan->opening_step;
    leftRight = 100 - 4 * (animationStep - 1);
    rightLeft = 109 + 4 * (animationStep - 1);
    plan->opening_door_valid = 1;
    plan->opening_door_step = animationStep;
    if (leftRight >= 0) {
        plan->opening_left_source_x = (animationStep & 0x00fc) << 2;
        plan->opening_left_source_y = 0;
        plan->opening_left_dest_x = 0;
        plan->opening_left_dest_y = CSB_V1_ENTRANCE_DOOR_SCREEN_Y_PC34;
        plan->opening_left_w = leftRight + 1;
        plan->opening_left_h = 161;
    }
    if (rightLeft <= 231) {
        plan->opening_right_source_x = (animationStep & 0x0003) << 2;
        plan->opening_right_source_y = 0;
        plan->opening_right_dest_x = rightLeft;
        plan->opening_right_dest_y = CSB_V1_ENTRANCE_DOOR_SCREEN_Y_PC34;
        plan->opening_right_w = 231 - rightLeft + 1;
        plan->opening_right_h = 161;
    }
}

static void csb_v1_startup_set_title_rect_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    V1_TitleFrontendSourceAnimationStep step;

    if (!plan) {
        return;
    }
    plan->source_asset_id = CSB_V1_GRAPHIC_TITLE_PC34;
    csb_v1_startup_clear_title_rect_pc34(plan);
    plan->fallback_title_x = 38;
    plan->fallback_title_y = 52;
    plan->fallback_title_style = CSB_V1_RENDER_TEXT_STYLE_SMALL_PC34;
    plan->fallback_title_text = "FTL PRESENTS";
    plan->fallback_subtitle_x = 38;
    plan->fallback_subtitle_y = 86;
    plan->fallback_subtitle_style = CSB_V1_RENDER_TEXT_STYLE_TITLE_PC34;
    plan->fallback_subtitle_text = "CHAOS";
    plan->fallback_prompt_x = 38;
    plan->fallback_prompt_y = 112;
    plan->fallback_prompt_style = CSB_V1_RENDER_TEXT_STYLE_SHADOW_PC34;
    plan->fallback_prompt_text = "STRIKES BACK";
    /* ReDMCSB TITLE.C F0437 lines 430, 433-457, and 461 draw the title
     * through C424 PRESENTS, C425 CHAOS, and C426 STRIKES BACK zones. */
    if (plan->title_stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34) {
        (void)V1_TitleFrontend_GetStepPalette(
            V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS,
            &plan->title_special_palette);
        plan->special_palette = plan->title_special_palette;
        plan->title_source_x = 0;
        plan->title_source_y = 137;
        plan->title_source_w = 320;
        plan->title_source_h = 16;
        plan->title_dest_x = 0;
        plan->title_dest_y = 90;
        plan->title_dest_w = 320;
        plan->title_dest_h = 16;
        plan->title_blit_kind = CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34;
        plan->title_transparent_color = -1;
        plan->title_empty_fallback_x = 38;
        plan->title_empty_fallback_y = 90;
        plan->title_empty_fallback_style = CSB_V1_RENDER_TEXT_STYLE_SMALL_PC34;
        plan->title_empty_fallback_text = "FTL PRESENTS";
        return;
    }
    if (plan->title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
        V1_TitleFrontend_GetSourceAnimationStep(
            (unsigned int)plan->title_source_step,
            &step) &&
        step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT) {
        (void)V1_TitleFrontend_GetStepPalette(
            step.kind,
            &plan->title_special_palette);
        plan->special_palette = plan->title_special_palette;
        plan->title_source_x = (int)step.x;
        plan->title_source_y = (int)step.y;
        plan->title_source_w = (int)step.width;
        plan->title_source_h = (int)step.height;
        plan->title_dest_x = 0;
        plan->title_dest_y = 0;
        plan->title_dest_w = 320;
        plan->title_dest_h = 80;
        plan->title_blit_kind =
            CSB_V1_STARTUP_TITLE_BLIT_SCALED_REGION_PC34;
        plan->title_transparent_color = -1;
        return;
    }
    if (plan->title_stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34) {
        (void)V1_TitleFrontend_GetStepPalette(
            V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT,
            &plan->title_special_palette);
        plan->special_palette = plan->title_special_palette;
        plan->title_source_x = 0;
        plan->title_source_y = 80;
        plan->title_source_w = 320;
        plan->title_source_h = 57;
        plan->title_dest_x = 0;
        plan->title_dest_y = 118;
        plan->title_dest_w = 320;
        plan->title_dest_h = 57;
        plan->title_blit_kind = CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34;
        plan->title_transparent_color = 0;
    }
}

int csb_v1_startup_entrance_wait_stage_pc34(void)
{
    return CSB_V1_ENTRANCE_WAIT_SOURCE_STEP_PC34;
}

int csb_v1_startup_entrance_pre_open_delay_ticks_pc34(void)
{
    return CSB_V1_ENTRANCE_PRE_OPEN_DELAY_TICKS_PC34;
}

int csb_v1_startup_entrance_credits_ticks_pc34(void)
{
    return CSB_V1_ENTRANCE_CREDITS_TICKS_PC34;
}

int csb_v1_startup_entrance_action_for_input_pc34(
    int credits_active,
    CSB_V1_StartupInput_PC34 input)
{
    /* ReDMCSB ENTRANCE.C F0441 lines ~857-883 owns the interactive entrance
     * loop: Return/Enter sets C001_MODE_LOAD_DUNGEON, credits redraw loops
     * back to the entrance, and other queued commands can leave via the saved
     * game/menu paths. This maps Firestaff's bounded startup input tokens onto
     * those source-owned entrance outcomes. */
    if (credits_active) {
        return CSB_V1_STARTUP_ENTRANCE_ACTION_NONE_PC34;
    }
    switch (input) {
        case CSB_V1_STARTUP_INPUT_ACCEPT_PC34:
        case CSB_V1_STARTUP_INPUT_ACTION_PC34:
            return CSB_V1_STARTUP_ENTRANCE_ACTION_ENTER_DUNGEON_PC34;
        case CSB_V1_STARTUP_INPUT_BACK_PC34:
            return CSB_V1_STARTUP_ENTRANCE_ACTION_QUIT_PC34;
        case CSB_V1_STARTUP_INPUT_DISK_MENU_PC34:
            return CSB_V1_STARTUP_ENTRANCE_ACTION_RESUME_PC34;
        case CSB_V1_STARTUP_INPUT_NONE_PC34:
        default:
            return CSB_V1_STARTUP_ENTRANCE_ACTION_NONE_PC34;
    }
}

int csb_v1_startup_entrance_command_for_action_pc34(
    CSB_V1_StartupEntranceAction_PC34 action)
{
    /* ReDMCSB ENTRANCE.C F0441/F0806 uses the source entrance command ids
     * C001/C200..C216 to leave the startup wait loop.  Keep that mapping in
     * CSB startup code so M11 only adapts and executes the resolved command. */
    switch (action) {
        case CSB_V1_STARTUP_ENTRANCE_ACTION_ENTER_DUNGEON_PC34:
            return CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;
        case CSB_V1_STARTUP_ENTRANCE_ACTION_RESUME_PC34:
            return CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34;
        case CSB_V1_STARTUP_ENTRANCE_ACTION_QUIT_PC34:
            return CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34;
        case CSB_V1_STARTUP_ENTRANCE_ACTION_NONE_PC34:
        default:
            return CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    }
}

int csb_v1_startup_advance_tick_pc34(
    CSB_V1_StartupTickState_PC34 *state,
    CSB_V1_StartupTickResult_PC34 *out_result)
{
    if (out_result) {
        out_result->redraw = 0;
        out_result->title_finished = 0;
        out_result->reached_entrance_wait = 0;
        out_result->credits_finished = 0;
        out_result->door_opening_finished = 0;
    }
    if (!state) {
        return 0;
    }
    state->entrance_frame++;
    if (out_result) {
        out_result->redraw = 1;
    }
    if (state->title_active) {
        state->title_frame++;
        state->title_source_step =
            csb_v1_startup_title_stage_for_frame_pc34(state->title_frame);
        if (state->title_frame >= csb_v1_startup_title_total_ticks_pc34()) {
            state->title_active = 0;
            state->title_source_step = 0;
            state->entrance_source_step = 1;
            state->entrance_frame = 0;
            if (out_result) {
                out_result->title_finished = 1;
            }
        }
        return 1;
    }
    if (!state->credits_active &&
        !state->opening_active &&
        state->entrance_source_step > 0 &&
        state->entrance_source_step < csb_v1_startup_entrance_wait_stage_pc34()) {
        state->entrance_source_step++;
        if (state->entrance_source_step ==
            csb_v1_startup_entrance_wait_stage_pc34() &&
            out_result) {
            out_result->reached_entrance_wait = 1;
        }
    }
    if (state->credits_active && state->credits_remaining_ticks > 0) {
        state->credits_remaining_ticks--;
        if (state->credits_remaining_ticks == 0) {
            state->credits_active = 0;
            if (out_result) {
                out_result->credits_finished = 1;
            }
        }
    }
    if (state->opening_active) {
        int door_step_count = state->door_step_count > 0
            ? state->door_step_count
            : 0;
        if (state->opening_delay_ticks > 0) {
            state->opening_delay_ticks--;
        } else if (state->opening_step < door_step_count) {
            state->opening_step++;
        } else if (out_result) {
            out_result->door_opening_finished = 1;
        }
    }
    return 1;
}

int csb_v1_startup_build_render_plan_pc34(
    const CSB_V1_StartupRenderState_PC34 *state,
    CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    CSB_V1_StartupRenderPlan_PC34 plan;

    if (!out_plan) {
        return 0;
    }
    plan.surface = CSB_V1_STARTUP_RENDER_NONE_PC34;
    plan.waiting_for_input = 0;
    plan.source_asset_id = 0;
    plan.title_source_step = 0;
    plan.title_stage = 0;
    csb_v1_startup_clear_title_rect_pc34(&plan);
    csb_v1_startup_clear_door_rects_pc34(&plan);
    csb_v1_startup_clear_fallback_text_pc34(&plan);
    plan.blink_prompt_visible = 0;
    plan.opening_step = 0;
    if (!state || !state->entrance_active) {
        *out_plan = plan;
        return 0;
    }
    plan.waiting_for_input =
        state->entrance_source_step >=
        csb_v1_startup_entrance_wait_stage_pc34();
    if (state->title_active) {
        plan.surface = CSB_V1_STARTUP_RENDER_TITLE_PC34;
        /* ReDMCSB TITLE.C F0437 lines 424-463 draws title startup as
         * PRESENTS, CHAOS zoom, then STRIKES BACK. Keep that stage visible
         * to render consumers separately from the source blit step. */
        plan.title_stage =
            csb_v1_startup_title_stage_for_frame_pc34(state->title_frame);
        plan.title_source_step =
            (int)csb_v1_startup_title_source_step_for_frame_pc34(
                state->title_frame);
        csb_v1_startup_set_title_rect_pc34(&plan);
        *out_plan = plan;
        return 1;
    }
    if (state->credits_active) {
        plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34;
        plan.source_asset_id = CSB_V1_GRAPHIC_ENTRANCE_CREDITS_PC34;
        plan.special_palette = VGA_PALETTE_PC34_SPECIAL_CREDITS;
        csb_v1_startup_set_credits_fallback_text_pc34(&plan);
        *out_plan = plan;
        return 1;
    }
    if (!plan.waiting_for_input && state->entrance_source_step == 2) {
        /* ReDMCSB ENTRANCE.C F0441 lines 426-443 fades/curtains to black
         * before C004/C002/C003 are redrawn. */
        plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34;
        plan.special_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
        *out_plan = plan;
        return 1;
    }
    if (state->opening_active) {
        plan.surface = state->opening_delay_ticks > 0
            ? CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34
            : CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34;
        plan.opening_step = state->opening_step;
        plan.source_asset_id = CSB_V1_GRAPHIC_ENTRANCE_SCREEN_PC34;
        plan.special_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
        csb_v1_startup_set_closed_door_rects_pc34(&plan);
        if (plan.surface ==
            CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34) {
            csb_v1_startup_set_opening_door_rects_pc34(&plan);
        }
        *out_plan = plan;
        return 1;
    }
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.source_asset_id = CSB_V1_GRAPHIC_ENTRANCE_SCREEN_PC34;
    plan.special_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
    csb_v1_startup_set_closed_door_rects_pc34(&plan);
    csb_v1_startup_set_entrance_fallback_text_pc34(&plan);
    plan.blink_prompt_visible =
        ((state->entrance_frame / 12) & 1) == 0;
    *out_plan = plan;
    return 1;
}

int csb_v1_startup_init_command_state_pc34(
    CSB_V1_StartupCommandState_PC34 *state,
    int skip_startup)
{
    if (!state) {
        return 0;
    }
    state->title_active = skip_startup ? 0 : 1;
    state->title_frame = 0;
    state->title_source_step = state->title_active
        ? CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34
        : 0;
    state->entrance_active = skip_startup ? 0 : 1;
    state->entrance_source_step =
        state->entrance_active && !state->title_active ? 1 : 0;
    state->entrance_dismissed = state->entrance_active ? 0 : 1;
    state->credits_active = 0;
    state->credits_remaining_ticks = 0;
    state->opening_active = 0;
    state->opening_delay_ticks = 0;
    state->opening_step = 0;
    state->pending_command = 0;
    return 1;
}

int csb_v1_startup_entrance_accepts_input_pc34(
    const CSB_V1_StartupCommandState_PC34 *state)
{
    if (!state || !state->entrance_active || state->title_active) {
        return 0;
    }
    if (state->credits_active) {
        return 1;
    }
    if (state->opening_active) {
        return 0;
    }
    return state->entrance_source_step >=
        csb_v1_startup_entrance_wait_stage_pc34();
}

int csb_v1_startup_resolve_entrance_command_pc34(
    const CSB_V1_StartupCommandState_PC34 *state,
    int command_id,
    CSB_V1_StartupEntranceDecision_PC34 *out_decision)
{
    CSB_V1_StartupEntranceDecision_PC34 decision =
        CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34;

    if (!out_decision) {
        return 0;
    }
    if (!state || !state->entrance_active) {
        *out_decision = decision;
        return 1;
    }
    if (state->credits_active) {
        *out_decision =
            CSB_V1_STARTUP_ENTRANCE_DECISION_DISMISS_CREDITS_PC34;
        return 1;
    }
    if (!csb_v1_startup_entrance_accepts_input_pc34(state)) {
        *out_decision = decision;
        return 1;
    }

    /* ReDMCSB ENTRANCE.C F0441 only dispatches these command ids after
     * F0441 reaches its interactive wait loop. Door-opening and title/prewait
     * phases ignore queued startup commands until the source loop is ready. */
    switch (command_id) {
        case CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34:
            decision = CSB_V1_STARTUP_ENTRANCE_DECISION_ENTER_DUNGEON_PC34;
            break;
        case CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_BONUS_DUNGEON_PC34:
            decision =
                CSB_V1_STARTUP_ENTRANCE_DECISION_ENTER_BONUS_DUNGEON_PC34;
            break;
        case CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34:
            decision = CSB_V1_STARTUP_ENTRANCE_DECISION_RESUME_PC34;
            break;
        case CSB_V1_STARTUP_ENTRANCE_COMMAND_DRAW_CREDITS_PC34:
            decision =
                CSB_V1_STARTUP_ENTRANCE_DECISION_BEGIN_CREDITS_PC34;
            break;
        case CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34:
            decision = CSB_V1_STARTUP_ENTRANCE_DECISION_QUIT_PC34;
            break;
        case CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34:
        default:
            decision = CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34;
            break;
    }
    *out_decision = decision;
    return 1;
}

void csb_v1_startup_entrance_command_plan_init_pc34(
    CSB_V1_StartupEntranceCommandPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    plan->kind = CSB_V1_STARTUP_ENTRANCE_PLAN_IGNORE_PC34;
    plan->command_id = CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    plan->status_scope = NULL;
    plan->status = NULL;
    plan->failure_status = NULL;
    plan->unavailable_status = NULL;
}

int csb_v1_startup_plan_for_entrance_command_pc34(
    const CSB_V1_StartupCommandState_PC34 *state,
    int command_id,
    CSB_V1_StartupEntranceCommandPlan_PC34 *out_plan)
{
    CSB_V1_StartupEntranceDecision_PC34 decision =
        CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34;

    if (!out_plan) {
        return 0;
    }
    csb_v1_startup_entrance_command_plan_init_pc34(out_plan);
    out_plan->command_id = command_id;
    if (!csb_v1_startup_resolve_entrance_command_pc34(
            state,
            command_id,
            &decision)) {
        return 0;
    }
    switch (decision) {
        case CSB_V1_STARTUP_ENTRANCE_DECISION_DISMISS_CREDITS_PC34:
            out_plan->kind =
                CSB_V1_STARTUP_ENTRANCE_PLAN_DISMISS_CREDITS_PC34;
            out_plan->status_scope = "BOOT";
            out_plan->status = "CSB ENTRANCE";
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_DECISION_ENTER_DUNGEON_PC34:
            out_plan->kind =
                CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_DUNGEON_PC34;
            out_plan->status_scope = "BOOT";
            out_plan->status = "CSB DOORS";
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_DECISION_ENTER_BONUS_DUNGEON_PC34:
            out_plan->kind =
                CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_BONUS_DUNGEON_PC34;
            out_plan->status_scope = "BOOT";
            out_plan->status = "CSB DOORS";
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_DECISION_RESUME_PC34:
            out_plan->kind = CSB_V1_STARTUP_ENTRANCE_PLAN_RESUME_PC34;
            out_plan->status_scope = "BOOT";
            out_plan->status = "CSB DOORS";
            out_plan->failure_status = "CSB RESUME FAILED";
            out_plan->unavailable_status = "CSB RESUME UNAVAILABLE";
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_DECISION_BEGIN_CREDITS_PC34:
            out_plan->kind =
                CSB_V1_STARTUP_ENTRANCE_PLAN_BEGIN_CREDITS_PC34;
            out_plan->status_scope = "BOOT";
            out_plan->status = "CSB CREDITS";
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_DECISION_QUIT_PC34:
            out_plan->kind = CSB_V1_STARTUP_ENTRANCE_PLAN_QUIT_PC34;
            out_plan->status_scope = "RETURN";
            out_plan->status = "BACK TO LAUNCHER";
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34:
        default:
            out_plan->kind = CSB_V1_STARTUP_ENTRANCE_PLAN_IGNORE_PC34;
            return 1;
    }
}

int csb_v1_startup_begin_door_opening_pc34(
    CSB_V1_StartupCommandState_PC34 *state,
    int pending_command)
{
    if (!state || !state->entrance_active || state->opening_active) {
        return 0;
    }
    /* ReDMCSB ENTRANCE.C F0441/F0806 starts dungeon entry by leaving the
     * interactive wait loop, cancelling transient credits, waiting 20 vblanks,
     * then running F0438/F0807 door animation before runtime handoff. */
    state->credits_active = 0;
    state->credits_remaining_ticks = 0;
    state->opening_active = 1;
    state->entrance_source_step =
        CSB_V1_STARTUP_STAGE_ENTRANCE_PRE_OPEN_DELAY_PC34;
    state->opening_delay_ticks =
        csb_v1_startup_entrance_pre_open_delay_ticks_pc34();
    state->opening_step = 1;
    state->pending_command = pending_command;
    return 1;
}

int csb_v1_startup_finish_door_opening_pc34(
    CSB_V1_StartupCommandState_PC34 *state)
{
    if (!state || !state->opening_active) {
        return 0;
    }
    state->opening_active = 0;
    state->opening_delay_ticks = 0;
    state->opening_step = 0;
    state->entrance_active = 0;
    state->entrance_dismissed = 1;
    state->entrance_source_step = 0;
    state->credits_active = 0;
    state->credits_remaining_ticks = 0;
    state->pending_command = 0;
    return 1;
}

int csb_v1_startup_begin_credits_pc34(
    CSB_V1_StartupCommandState_PC34 *state)
{
    if (!state || !state->entrance_active) {
        return 0;
    }
    /* ReDMCSB ENTRANCE.C F0442 lines ~966-1091 shows C005 credits for
     * 1800 vertical blanks, then returns to the entrance loop. */
    state->credits_active = 1;
    state->credits_remaining_ticks =
        csb_v1_startup_entrance_credits_ticks_pc34();
    state->opening_active = 0;
    state->opening_delay_ticks = 0;
    state->opening_step = 0;
    state->pending_command = 0;
    return 1;
}

int csb_v1_startup_dismiss_credits_pc34(
    CSB_V1_StartupCommandState_PC34 *state)
{
    if (!state || !state->credits_active) {
        return 0;
    }
    state->credits_active = 0;
    state->credits_remaining_ticks = 0;
    return 1;
}

int csb_v1_startup_quit_to_launcher_pc34(
    CSB_V1_StartupCommandState_PC34 *state)
{
    if (!state) {
        return 0;
    }
    state->title_active = 0;
    state->title_frame = 0;
    state->title_source_step = 0;
    state->entrance_active = 0;
    state->entrance_source_step = 0;
    state->entrance_dismissed = 1;
    state->credits_active = 0;
    state->credits_remaining_ticks = 0;
    state->opening_active = 0;
    state->opening_delay_ticks = 0;
    state->opening_step = 0;
    state->pending_command = 0;
    return 1;
}

int csb_v1_startup_receipt_phase_pc34(
    const CSB_V1_StartupCommandState_PC34 *state,
    int entrance_frame,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame)
{
    int startup_active = 0;
    int startup_frame = entrance_frame;
    const char *literal_phase = "csb-runtime";

    if (!out_phase || out_phase_size <= 0) {
        return 0;
    }
    out_phase[0] = '\0';
    if (state) {
        startup_active = state->entrance_active ? 1 : 0;
        startup_frame = state->title_active
            ? state->title_frame
            : entrance_frame;
        if (state->title_active) {
            snprintf(out_phase,
                     (size_t)out_phase_size,
                     "csb-title-%d",
                     state->title_source_step);
        } else if (state->opening_active) {
            snprintf(out_phase,
                     (size_t)out_phase_size,
                     "csb-entrance-opening-%d",
                     state->opening_step);
        } else if (state->credits_active) {
            literal_phase = "csb-credits";
            snprintf(out_phase, (size_t)out_phase_size, "%s", literal_phase);
        } else if (state->entrance_active) {
            snprintf(out_phase,
                     (size_t)out_phase_size,
                     "csb-entrance-%d",
                     state->entrance_source_step);
        } else {
            snprintf(out_phase, (size_t)out_phase_size, "%s", literal_phase);
        }
    } else {
        snprintf(out_phase, (size_t)out_phase_size, "%s", literal_phase);
    }
    if (out_startup_active) {
        *out_startup_active = startup_active;
    }
    if (out_startup_frame) {
        *out_startup_frame = startup_frame;
    }
    return 1;
}

int csb_v1_startup_sequence_source_order_valid_pc34(void)
{
    /* ReDMCSB startup source order:
     * TITLE.C F0437 draws PRESENTS, CHAOS zoom, then STRIKES BACK.
     * ENTRANCE.C F0441/F0806 then loads the entrance screen, waits for
     * input, applies the 20-vblank pre-open delay, runs F0438/F0807 door
     * animation, and only then enters dungeon runtime.
     */
    return csb_v1_startup_stage_after_pc34(
               CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34,
               CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34) &&
           csb_v1_startup_stage_after_pc34(
               CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34,
               CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34) &&
           csb_v1_startup_stage_after_pc34(
               CSB_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34,
               CSB_V1_STARTUP_STAGE_ENTRANCE_LOAD_BLACK_PC34) &&
           csb_v1_startup_stage_after_pc34(
               CSB_V1_STARTUP_STAGE_ENTRANCE_PRE_OPEN_DELAY_PC34,
               CSB_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34) &&
           csb_v1_startup_stage_after_pc34(
               CSB_V1_STARTUP_STAGE_ENTRANCE_DOOR_OPENING_PC34,
               CSB_V1_STARTUP_STAGE_ENTRANCE_PRE_OPEN_DELAY_PC34) &&
           csb_v1_startup_stage_after_pc34(
               CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34,
               CSB_V1_STARTUP_STAGE_ENTRANCE_DOOR_OPENING_PC34);
}

const char* csb_v1_startup_sequence_source_evidence_pc34(void)
{
    return "ReDMCSB TITLE.C F0437 -> ENTRANCE.C F0441/F0806 -> ENTRANCE.C F0438/F0807";
}
