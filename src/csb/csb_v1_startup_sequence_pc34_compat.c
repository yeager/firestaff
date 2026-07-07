#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "title_frontend_v1.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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
    CSB_V1_RENDER_TEXT_STYLE_SHADOW_PC34 = 3,
    CSB_V1_FALLBACK_BLACK_PC34 = 0,
    CSB_V1_FALLBACK_YELLOW_PC34 = 14,
    CSB_V1_FALLBACK_LIGHT_GRAY_PC34 = 2,
    CSB_V1_FALLBACK_DARK_GRAY_PC34 = 12
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

static void csb_v1_startup_clear_surface_blit_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    plan->surface_dest_x = 0;
    plan->surface_dest_y = 0;
    plan->surface_w = 0;
    plan->surface_h = 0;
    plan->surface_transparent_color = -1;
}

static void csb_v1_startup_set_full_surface_blit_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    plan->surface_dest_x = 0;
    plan->surface_dest_y = 0;
    plan->surface_w = 320;
    plan->surface_h = 200;
    plan->surface_transparent_color = -1;
}

static void csb_v1_startup_clear_door_rects_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    plan->closed_left_source_x = 0;
    plan->closed_left_source_y = 0;
    plan->closed_left_asset_id = 0;
    plan->closed_left_dest_x = 0;
    plan->closed_left_dest_y = 0;
    plan->closed_left_w = 0;
    plan->closed_left_h = 0;
    plan->closed_left_fallback_fill_color = 0;
    plan->closed_left_fallback_light_edge_color = 0;
    plan->closed_left_fallback_dark_edge_color = 0;
    plan->closed_right_source_x = 0;
    plan->closed_right_source_y = 0;
    plan->closed_right_asset_id = 0;
    plan->closed_right_dest_x = 0;
    plan->closed_right_dest_y = 0;
    plan->closed_right_w = 0;
    plan->closed_right_h = 0;
    plan->closed_right_fallback_fill_color = 0;
    plan->closed_right_fallback_light_edge_color = 0;
    plan->closed_right_fallback_dark_edge_color = 0;
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
    plan->opening_composite_valid = 0;
    plan->opening_composite_screen_asset_id = 0;
    plan->opening_composite_left_asset_id = 0;
    plan->opening_composite_right_asset_id = 0;
    plan->opening_composite_animation_step = 0;
    plan->opening_composite_left_box_x = 0;
    plan->opening_composite_left_box_y = 0;
    plan->opening_composite_left_box_w = 0;
    plan->opening_composite_left_box_h = 0;
    plan->opening_composite_right_box_x = 0;
    plan->opening_composite_right_box_y = 0;
    plan->opening_composite_right_box_w = 0;
    plan->opening_composite_right_box_h = 0;
    plan->opening_composite_left_source_x = 0;
    plan->opening_composite_right_source_x = 0;
}

static void csb_v1_startup_clear_fallback_text_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    int i;
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
    plan->fallback_status_visible = 0;
    plan->fallback_frame_valid = 0;
    plan->fallback_frame_x = 0;
    plan->fallback_frame_y = 0;
    plan->fallback_frame_w = 0;
    plan->fallback_frame_h = 0;
    plan->fallback_frame_color = 0;
    plan->fallback_detail_x = 0;
    plan->fallback_detail_y = 0;
    plan->fallback_detail_style = 0;
    plan->fallback_detail_text = NULL;
    plan->fallback_detail_visible = 0;
    plan->fallback_runtime_detail_visible = 0;
    plan->fallback_runtime_detail_text[0] = '\0';
    plan->fallback_prompt_x = 0;
    plan->fallback_prompt_y = 0;
    plan->fallback_prompt_style = 0;
    plan->fallback_prompt_text = NULL;
    plan->fallback_text_row_count = 0;
    for (i = 0; i < CSB_V1_STARTUP_FALLBACK_TEXT_ROW_CAP_PC34; ++i) {
        plan->fallback_text_rows[i].x = 0;
        plan->fallback_text_rows[i].y = 0;
        plan->fallback_text_rows[i].style = 0;
        plan->fallback_text_rows[i].visible = 0;
        plan->fallback_text_rows[i].text = NULL;
    }
}

static void csb_v1_startup_clear_primitive_commands_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    int i;
    if (!plan) {
        return;
    }
    plan->primitive_command_count = 0;
    for (i = 0; i < CSB_V1_STARTUP_PRIMITIVE_COMMAND_CAP_PC34; ++i) {
        plan->primitive_commands[i].kind =
            CSB_V1_STARTUP_PRIMITIVE_NONE_PC34;
        plan->primitive_commands[i].x = 0;
        plan->primitive_commands[i].y = 0;
        plan->primitive_commands[i].w = 0;
        plan->primitive_commands[i].h = 0;
        plan->primitive_commands[i].color = 0;
        plan->primitive_commands[i].light_edge_color = 0;
        plan->primitive_commands[i].dark_edge_color = 0;
        plan->primitive_commands[i].visible = 0;
    }
}

static void csb_v1_startup_clear_asset_commands_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    int i;
    if (!plan) {
        return;
    }
    plan->asset_command_count = 0;
    for (i = 0; i < CSB_V1_STARTUP_ASSET_COMMAND_CAP_PC34; ++i) {
        plan->asset_commands[i].kind = CSB_V1_STARTUP_ASSET_NONE_PC34;
        plan->asset_commands[i].asset_id = 0;
        plan->asset_commands[i].source_x = 0;
        plan->asset_commands[i].source_y = 0;
        plan->asset_commands[i].source_w = 0;
        plan->asset_commands[i].source_h = 0;
        plan->asset_commands[i].dest_x = 0;
        plan->asset_commands[i].dest_y = 0;
        plan->asset_commands[i].dest_w = 0;
        plan->asset_commands[i].dest_h = 0;
        plan->asset_commands[i].transparent_color = -1;
        plan->asset_commands[i].visible = 0;
    }
}

static void csb_v1_startup_add_asset_command_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupAssetCommandKind_PC34 kind,
    int asset_id,
    int source_x,
    int source_y,
    int source_w,
    int source_h,
    int dest_x,
    int dest_y,
    int dest_w,
    int dest_h,
    int transparent_color,
    int visible)
{
    CSB_V1_StartupAssetCommand_PC34 *cmd;
    if (!plan || kind == CSB_V1_STARTUP_ASSET_NONE_PC34 ||
        asset_id <= 0 || source_w <= 0 || source_h <= 0 ||
        dest_w <= 0 || dest_h <= 0 ||
        plan->asset_command_count >=
            CSB_V1_STARTUP_ASSET_COMMAND_CAP_PC34) {
        return;
    }
    cmd = &plan->asset_commands[plan->asset_command_count++];
    cmd->kind = kind;
    cmd->asset_id = asset_id;
    cmd->source_x = source_x;
    cmd->source_y = source_y;
    cmd->source_w = source_w;
    cmd->source_h = source_h;
    cmd->dest_x = dest_x;
    cmd->dest_y = dest_y;
    cmd->dest_w = dest_w;
    cmd->dest_h = dest_h;
    cmd->transparent_color = transparent_color;
    cmd->visible = visible ? 1 : 0;
}

static void csb_v1_startup_rebuild_asset_commands_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    csb_v1_startup_clear_asset_commands_pc34(plan);
    if (plan->surface == CSB_V1_STARTUP_RENDER_NONE_PC34 ||
        plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34) {
        return;
    }
    if (plan->surface == CSB_V1_STARTUP_RENDER_TITLE_PC34) {
        if (plan->title_blit_kind ==
            CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34) {
            csb_v1_startup_add_asset_command_pc34(
                plan,
                CSB_V1_STARTUP_ASSET_TITLE_REGION_PC34,
                plan->source_asset_id,
                plan->title_source_x,
                plan->title_source_y,
                plan->title_source_w,
                plan->title_source_h,
                plan->title_dest_x,
                plan->title_dest_y,
                plan->title_dest_w,
                plan->title_dest_h,
                plan->title_transparent_color,
                1);
        } else if (plan->title_blit_kind ==
                   CSB_V1_STARTUP_TITLE_BLIT_SCALED_REGION_PC34) {
            csb_v1_startup_add_asset_command_pc34(
                plan,
                CSB_V1_STARTUP_ASSET_TITLE_SCALED_REGION_PC34,
                plan->source_asset_id,
                plan->title_source_x,
                plan->title_source_y,
                plan->title_source_w,
                plan->title_source_h,
                plan->title_dest_x,
                plan->title_dest_y,
                plan->title_dest_w,
                plan->title_dest_h,
                plan->title_transparent_color,
                1);
        }
        return;
    }
    if (plan->source_asset_id > 0 && plan->surface_w > 0 &&
        plan->surface_h > 0) {
        csb_v1_startup_add_asset_command_pc34(
            plan,
            CSB_V1_STARTUP_ASSET_FULL_SURFACE_PC34,
            plan->source_asset_id,
            0,
            0,
            plan->surface_w,
            plan->surface_h,
            plan->surface_dest_x,
            plan->surface_dest_y,
            plan->surface_w,
            plan->surface_h,
            plan->surface_transparent_color,
            1);
    }
    if (plan->closed_left_asset_id > 0 && plan->closed_left_w > 0 &&
        plan->closed_left_h > 0) {
        csb_v1_startup_add_asset_command_pc34(
            plan,
            CSB_V1_STARTUP_ASSET_CLOSED_LEFT_DOOR_PC34,
            plan->closed_left_asset_id,
            plan->closed_left_source_x,
            plan->closed_left_source_y,
            plan->closed_left_w,
            plan->closed_left_h,
            plan->closed_left_dest_x,
            plan->closed_left_dest_y,
            plan->closed_left_w,
            plan->closed_left_h,
            -1,
            1);
    }
    if (plan->closed_right_asset_id > 0 && plan->closed_right_w > 0 &&
        plan->closed_right_h > 0) {
        csb_v1_startup_add_asset_command_pc34(
            plan,
            CSB_V1_STARTUP_ASSET_CLOSED_RIGHT_DOOR_PC34,
            plan->closed_right_asset_id,
            plan->closed_right_source_x,
            plan->closed_right_source_y,
            plan->closed_right_w,
            plan->closed_right_h,
            plan->closed_right_dest_x,
            plan->closed_right_dest_y,
            plan->closed_right_w,
            plan->closed_right_h,
            -1,
            1);
    }
}

static void csb_v1_startup_add_primitive_command_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupPrimitiveCommandKind_PC34 kind,
    int x,
    int y,
    int w,
    int h,
    int color,
    int light_edge_color,
    int dark_edge_color,
    int visible)
{
    CSB_V1_StartupPrimitiveCommand_PC34 *cmd;
    if (!plan || kind == CSB_V1_STARTUP_PRIMITIVE_NONE_PC34 ||
        w <= 0 || h <= 0 ||
        plan->primitive_command_count >=
            CSB_V1_STARTUP_PRIMITIVE_COMMAND_CAP_PC34) {
        return;
    }
    cmd = &plan->primitive_commands[plan->primitive_command_count++];
    cmd->kind = kind;
    cmd->x = x;
    cmd->y = y;
    cmd->w = w;
    cmd->h = h;
    cmd->color = color;
    cmd->light_edge_color = light_edge_color;
    cmd->dark_edge_color = dark_edge_color;
    cmd->visible = visible ? 1 : 0;
}

static void csb_v1_startup_fill_rect_pc34(
    unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int x,
    int y,
    int w,
    int h,
    unsigned char color)
{
    int yy;
    if (!framebuffer || framebuffer_width <= 0 || framebuffer_height <= 0 ||
        w <= 0 || h <= 0) {
        return;
    }
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > framebuffer_width) {
        w = framebuffer_width - x;
    }
    if (y + h > framebuffer_height) {
        h = framebuffer_height - y;
    }
    if (w <= 0 || h <= 0) {
        return;
    }
    for (yy = y; yy < y + h; ++yy) {
        unsigned char *row = framebuffer + yy * framebuffer_width;
        int xx;
        for (xx = x; xx < x + w; ++xx) {
            row[xx] = color;
        }
    }
}

static void csb_v1_startup_draw_rect_pc34(
    unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int x,
    int y,
    int w,
    int h,
    unsigned char color)
{
    if (!framebuffer || framebuffer_width <= 0 || framebuffer_height <= 0 ||
        w <= 0 || h <= 0) {
        return;
    }
    csb_v1_startup_fill_rect_pc34(framebuffer, framebuffer_width,
                                  framebuffer_height, x, y, w, 1, color);
    csb_v1_startup_fill_rect_pc34(framebuffer, framebuffer_width,
                                  framebuffer_height, x, y + h - 1, w, 1,
                                  color);
    csb_v1_startup_fill_rect_pc34(framebuffer, framebuffer_width,
                                  framebuffer_height, x, y, 1, h, color);
    csb_v1_startup_fill_rect_pc34(framebuffer, framebuffer_width,
                                  framebuffer_height, x + w - 1, y, 1, h,
                                  color);
}

static void csb_v1_startup_draw_door_panel_pc34(
    unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    const CSB_V1_StartupPrimitiveCommand_PC34 *cmd)
{
    if (!cmd) {
        return;
    }
    csb_v1_startup_fill_rect_pc34(framebuffer, framebuffer_width,
                                  framebuffer_height, cmd->x, cmd->y,
                                  cmd->w, cmd->h,
                                  (unsigned char)cmd->color);
    csb_v1_startup_fill_rect_pc34(framebuffer, framebuffer_width,
                                  framebuffer_height, cmd->x, cmd->y,
                                  cmd->w, 1,
                                  (unsigned char)cmd->light_edge_color);
    csb_v1_startup_fill_rect_pc34(framebuffer, framebuffer_width,
                                  framebuffer_height, cmd->x,
                                  cmd->y + cmd->h - 1, cmd->w, 1,
                                  (unsigned char)cmd->dark_edge_color);
    csb_v1_startup_fill_rect_pc34(framebuffer, framebuffer_width,
                                  framebuffer_height, cmd->x, cmd->y,
                                  1, cmd->h,
                                  (unsigned char)cmd->light_edge_color);
    csb_v1_startup_fill_rect_pc34(framebuffer, framebuffer_width,
                                  framebuffer_height,
                                  cmd->x + cmd->w - 1, cmd->y, 1, cmd->h,
                                  (unsigned char)cmd->dark_edge_color);
}

int csb_v1_startup_execute_primitive_commands_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height)
{
    int i;
    int executed = 0;
    if (!plan || !framebuffer || framebuffer_width <= 0 ||
        framebuffer_height <= 0) {
        return 0;
    }
    for (i = 0; i < plan->primitive_command_count &&
                i < CSB_V1_STARTUP_PRIMITIVE_COMMAND_CAP_PC34; ++i) {
        const CSB_V1_StartupPrimitiveCommand_PC34 *cmd =
            &plan->primitive_commands[i];
        if (!cmd->visible || cmd->w <= 0 || cmd->h <= 0) {
            continue;
        }
        switch (cmd->kind) {
            case CSB_V1_STARTUP_PRIMITIVE_FILL_RECT_PC34:
                csb_v1_startup_fill_rect_pc34(
                    framebuffer, framebuffer_width, framebuffer_height,
                    cmd->x, cmd->y, cmd->w, cmd->h,
                    (unsigned char)cmd->color);
                ++executed;
                break;
            case CSB_V1_STARTUP_PRIMITIVE_DRAW_RECT_PC34:
                csb_v1_startup_draw_rect_pc34(
                    framebuffer, framebuffer_width, framebuffer_height,
                    cmd->x, cmd->y, cmd->w, cmd->h,
                    (unsigned char)cmd->color);
                ++executed;
                break;
            case CSB_V1_STARTUP_PRIMITIVE_DOOR_PANEL_PC34:
                csb_v1_startup_draw_door_panel_pc34(
                    framebuffer, framebuffer_width, framebuffer_height, cmd);
                ++executed;
                break;
            case CSB_V1_STARTUP_PRIMITIVE_NONE_PC34:
            default:
                break;
        }
    }
    return executed;
}

int csb_v1_startup_execute_asset_commands_kind_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupAssetCommandKind_PC34 kind,
    CSB_V1_StartupAssetExecutor_PC34 executor,
    void *user)
{
    int i;
    int executed = 0;
    if (!plan || !executor || kind == CSB_V1_STARTUP_ASSET_NONE_PC34) {
        return 0;
    }
    for (i = 0; i < plan->asset_command_count &&
                i < CSB_V1_STARTUP_ASSET_COMMAND_CAP_PC34; ++i) {
        const CSB_V1_StartupAssetCommand_PC34 *cmd =
            &plan->asset_commands[i];
        if (cmd->kind != kind || !cmd->visible || cmd->asset_id <= 0 ||
            cmd->source_w <= 0 || cmd->source_h <= 0 ||
            cmd->dest_w <= 0 || cmd->dest_h <= 0) {
            continue;
        }
        if (executor(user, cmd)) {
            ++executed;
        }
    }
    return executed;
}

int csb_v1_startup_execute_closed_door_asset_commands_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupAssetExecutor_PC34 executor,
    void *user)
{
    int left;
    int right;
    left = csb_v1_startup_execute_asset_commands_kind_pc34(
        plan,
        CSB_V1_STARTUP_ASSET_CLOSED_LEFT_DOOR_PC34,
        executor,
        user);
    right = csb_v1_startup_execute_asset_commands_kind_pc34(
        plan,
        CSB_V1_STARTUP_ASSET_CLOSED_RIGHT_DOOR_PC34,
        executor,
        user);
    return left > 0 && right > 0;
}

static int csb_v1_startup_count_nonzero_region_pc34(
    const unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int x,
    int y,
    int w,
    int h)
{
    int yy;
    int xx;
    int count = 0;
    if (!framebuffer || framebuffer_width <= 0 || framebuffer_height <= 0 ||
        w <= 0 || h <= 0) {
        return 0;
    }
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > framebuffer_width) {
        w = framebuffer_width - x;
    }
    if (y + h > framebuffer_height) {
        h = framebuffer_height - y;
    }
    if (w <= 0 || h <= 0) {
        return 0;
    }
    for (yy = y; yy < y + h; ++yy) {
        const unsigned char *row = framebuffer + yy * framebuffer_width;
        for (xx = x; xx < x + w; ++xx) {
            if (row[xx] != 0u) {
                ++count;
            }
        }
    }
    return count;
}

int csb_v1_startup_title_empty_fallback_needed_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height)
{
    if (!plan || !framebuffer || !plan->title_empty_fallback_text ||
        plan->title_empty_fallback_text[0] == '\0' ||
        plan->title_dest_w <= 0 || plan->title_dest_h <= 0) {
        return 0;
    }
    return csb_v1_startup_count_nonzero_region_pc34(
               framebuffer,
               framebuffer_width,
               framebuffer_height,
               plan->title_dest_x,
               plan->title_dest_y,
               plan->title_dest_w,
               plan->title_dest_h) == 0;
}

int csb_v1_startup_execute_opening_composite_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupOpeningCompositeExecutor_PC34 executor,
    void *user)
{
    CSB_V1_StartupOpeningComposite_PC34 composite;
    if (!plan || !executor || !plan->opening_composite_valid ||
        plan->opening_composite_screen_asset_id <= 0 ||
        plan->opening_composite_left_asset_id <= 0 ||
        plan->opening_composite_right_asset_id <= 0 ||
        plan->opening_composite_left_box_w <= 0 ||
        plan->opening_composite_left_box_h <= 0 ||
        plan->opening_composite_right_box_w <= 0 ||
        plan->opening_composite_right_box_h <= 0) {
        return 0;
    }
    composite.screen_asset_id = plan->opening_composite_screen_asset_id;
    composite.left_door_asset_id = plan->opening_composite_left_asset_id;
    composite.right_door_asset_id = plan->opening_composite_right_asset_id;
    composite.animation_step = plan->opening_composite_animation_step;
    composite.left_box_x = plan->opening_composite_left_box_x;
    composite.left_box_y = plan->opening_composite_left_box_y;
    composite.left_box_w = plan->opening_composite_left_box_w;
    composite.left_box_h = plan->opening_composite_left_box_h;
    composite.right_box_x = plan->opening_composite_right_box_x;
    composite.right_box_y = plan->opening_composite_right_box_y;
    composite.right_box_w = plan->opening_composite_right_box_w;
    composite.right_box_h = plan->opening_composite_right_box_h;
    composite.left_source_x = plan->opening_composite_left_source_x;
    composite.right_source_x = plan->opening_composite_right_source_x;
    return executor(user, &composite) ? 1 : 0;
}

int csb_v1_startup_execute_render_plan_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const CSB_V1_StartupRenderExecutor_PC34 *executor)
{
    int i;
    int drew_asset = 0;
    if (!plan || !executor) {
        return 0;
    }
    for (i = 0; i < plan->render_command_count &&
                i < CSB_V1_STARTUP_RENDER_COMMAND_CAP_PC34; ++i) {
        const CSB_V1_StartupRenderCommand_PC34 *command =
            &plan->render_commands[i];
        switch (command->kind) {
            case CSB_V1_STARTUP_RENDER_COMMAND_TITLE_PC34:
                if (executor->draw_title) {
                    (void)executor->draw_title(executor->user, plan);
                }
                return 1;
            case CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34:
                if (executor->clear_black) {
                    executor->clear_black(executor->user, plan);
                }
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_OR_TEXT_PC34:
                if (executor->draw_full_surface &&
                    executor->draw_full_surface(executor->user, plan)) {
                    drew_asset = 1;
                } else if (executor->draw_fallback_text) {
                    executor->draw_fallback_text(executor->user, plan);
                }
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34:
                if (executor->draw_full_surface &&
                    executor->draw_full_surface(executor->user, plan)) {
                    drew_asset = 1;
                }
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_OPENING_FRAME_IF_SURFACE_PC34:
                if (drew_asset && executor->draw_opening_frame &&
                    executor->draw_opening_frame(executor->user, plan)) {
                    return 1;
                }
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_ELSE_FALLBACK_PC34:
                if (drew_asset) {
                    if (executor->draw_closed_doors) {
                        executor->draw_closed_doors(executor->user, plan);
                    }
                } else if (executor->draw_door_fallback) {
                    executor->draw_door_fallback(executor->user, plan);
                }
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_FALLBACK_IF_NO_SURFACE_PC34:
                if (!drew_asset && executor->draw_fallback_text) {
                    executor->draw_fallback_text(executor->user, plan);
                }
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_UTILITY_PANEL_IF_WAITING_PC34:
                if (plan->waiting_for_input && executor->draw_utility_panel) {
                    executor->draw_utility_panel(executor->user, plan);
                }
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_NONE_PC34:
            default:
                break;
        }
    }
    return 1;
}

static void csb_v1_startup_rebuild_primitive_commands_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    csb_v1_startup_clear_primitive_commands_pc34(plan);
    if (plan->surface == CSB_V1_STARTUP_RENDER_NONE_PC34) {
        return;
    }
    csb_v1_startup_add_primitive_command_pc34(
        plan,
        CSB_V1_STARTUP_PRIMITIVE_FILL_RECT_PC34,
        0,
        0,
        320,
        200,
        CSB_V1_FALLBACK_BLACK_PC34,
        0,
        0,
        1);
    if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
        plan->fallback_frame_valid) {
        csb_v1_startup_add_primitive_command_pc34(
            plan,
            CSB_V1_STARTUP_PRIMITIVE_DRAW_RECT_PC34,
            plan->fallback_frame_x,
            plan->fallback_frame_y,
            plan->fallback_frame_w,
            plan->fallback_frame_h,
            plan->fallback_frame_color,
            0,
            0,
            1);
    }
    if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34 ||
        plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34) {
        csb_v1_startup_add_primitive_command_pc34(
            plan,
            CSB_V1_STARTUP_PRIMITIVE_DOOR_PANEL_PC34,
            plan->closed_left_dest_x,
            plan->closed_left_dest_y,
            plan->closed_left_w,
            plan->closed_left_h,
            plan->closed_left_fallback_fill_color,
            plan->closed_left_fallback_light_edge_color,
            plan->closed_left_fallback_dark_edge_color,
            1);
        csb_v1_startup_add_primitive_command_pc34(
            plan,
            CSB_V1_STARTUP_PRIMITIVE_DOOR_PANEL_PC34,
            plan->closed_right_dest_x,
            plan->closed_right_dest_y,
            plan->closed_right_w,
            plan->closed_right_h,
            plan->closed_right_fallback_fill_color,
            plan->closed_right_fallback_light_edge_color,
            plan->closed_right_fallback_dark_edge_color,
            1);
    }
}

static void csb_v1_startup_clear_render_commands_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    int i;
    if (!plan) {
        return;
    }
    plan->render_command_count = 0;
    for (i = 0; i < CSB_V1_STARTUP_RENDER_COMMAND_CAP_PC34; ++i) {
        plan->render_commands[i].kind =
            CSB_V1_STARTUP_RENDER_COMMAND_NONE_PC34;
    }
}

static void csb_v1_startup_add_render_command_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupRenderCommandKind_PC34 kind)
{
    CSB_V1_StartupRenderCommand_PC34 *command;
    if (!plan || kind == CSB_V1_STARTUP_RENDER_COMMAND_NONE_PC34 ||
        plan->render_command_count >=
            CSB_V1_STARTUP_RENDER_COMMAND_CAP_PC34) {
        return;
    }
    command = &plan->render_commands[plan->render_command_count++];
    command->kind = kind;
}

static void csb_v1_startup_rebuild_render_commands_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    csb_v1_startup_clear_render_commands_pc34(plan);
    if (plan->surface == CSB_V1_STARTUP_RENDER_TITLE_PC34) {
        csb_v1_startup_add_render_command_pc34(
            plan, CSB_V1_STARTUP_RENDER_COMMAND_TITLE_PC34);
        return;
    }
    csb_v1_startup_add_render_command_pc34(
        plan, CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34);
    if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34) {
        return;
    }
    if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34) {
        csb_v1_startup_add_render_command_pc34(
            plan, CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_OR_TEXT_PC34);
        return;
    }
    csb_v1_startup_add_render_command_pc34(
        plan, CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34);
    if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34) {
        csb_v1_startup_add_render_command_pc34(
            plan,
            CSB_V1_STARTUP_RENDER_COMMAND_OPENING_FRAME_IF_SURFACE_PC34);
    }
    if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34 ||
        plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34) {
        csb_v1_startup_add_render_command_pc34(
            plan,
            CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_ELSE_FALLBACK_PC34);
        return;
    }
    csb_v1_startup_add_render_command_pc34(
        plan,
        CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_ELSE_FALLBACK_PC34);
    csb_v1_startup_add_render_command_pc34(
        plan,
        CSB_V1_STARTUP_RENDER_COMMAND_FALLBACK_IF_NO_SURFACE_PC34);
    csb_v1_startup_add_render_command_pc34(
        plan,
        CSB_V1_STARTUP_RENDER_COMMAND_UTILITY_PANEL_IF_WAITING_PC34);
}

static void csb_v1_startup_add_fallback_text_row_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan,
    int x,
    int y,
    int style,
    const char *text,
    int visible)
{
    CSB_V1_StartupFallbackTextRow_PC34 *row;
    if (!plan || !text || text[0] == '\0' ||
        plan->fallback_text_row_count >=
            CSB_V1_STARTUP_FALLBACK_TEXT_ROW_CAP_PC34) {
        return;
    }
    row = &plan->fallback_text_rows[plan->fallback_text_row_count++];
    row->x = x;
    row->y = y;
    row->style = style;
    row->text = text;
    row->visible = visible ? 1 : 0;
}

static void csb_v1_startup_rebuild_fallback_text_rows_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan) {
        return;
    }
    plan->fallback_text_row_count = 0;
    csb_v1_startup_add_fallback_text_row_pc34(
        plan,
        plan->fallback_title_x,
        plan->fallback_title_y,
        plan->fallback_title_style,
        plan->fallback_title_text,
        1);
    csb_v1_startup_add_fallback_text_row_pc34(
        plan,
        plan->fallback_subtitle_x,
        plan->fallback_subtitle_y,
        plan->fallback_subtitle_style,
        plan->fallback_subtitle_text,
        1);
    csb_v1_startup_add_fallback_text_row_pc34(
        plan,
        plan->fallback_status_x,
        plan->fallback_status_y,
        plan->fallback_status_style,
        plan->fallback_status_text,
        plan->fallback_status_visible);
    csb_v1_startup_add_fallback_text_row_pc34(
        plan,
        plan->fallback_detail_x,
        plan->fallback_detail_y,
        plan->fallback_detail_style,
        plan->fallback_runtime_detail_visible
            ? plan->fallback_runtime_detail_text
            : plan->fallback_detail_text,
        plan->fallback_detail_visible);
    csb_v1_startup_add_fallback_text_row_pc34(
        plan,
        plan->fallback_prompt_x,
        plan->fallback_prompt_y,
        plan->fallback_prompt_style,
        plan->fallback_prompt_text,
        plan->blink_prompt_visible);
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
    plan->blink_prompt_visible = 1;
    csb_v1_startup_rebuild_fallback_text_rows_pc34(plan);
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
    plan->fallback_frame_valid = 1;
    plan->fallback_frame_x = 18;
    plan->fallback_frame_y = 18;
    plan->fallback_frame_w = 284;
    plan->fallback_frame_h = 164;
    plan->fallback_frame_color = CSB_V1_FALLBACK_YELLOW_PC34;
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
    plan->closed_left_asset_id = CSB_V1_GRAPHIC_ENTRANCE_LEFT_DOOR_PC34;
    plan->closed_left_dest_x = 0;
    plan->closed_left_dest_y = CSB_V1_ENTRANCE_DOOR_SCREEN_Y_PC34;
    plan->closed_left_w = 105;
    plan->closed_left_h = 161;
    plan->closed_left_fallback_fill_color = CSB_V1_FALLBACK_DARK_GRAY_PC34;
    plan->closed_left_fallback_light_edge_color =
        CSB_V1_FALLBACK_LIGHT_GRAY_PC34;
    plan->closed_left_fallback_dark_edge_color =
        CSB_V1_FALLBACK_BLACK_PC34;
    plan->closed_right_source_x = 0;
    plan->closed_right_source_y = 0;
    plan->closed_right_asset_id = CSB_V1_GRAPHIC_ENTRANCE_RIGHT_DOOR_PC34;
    plan->closed_right_dest_x = 105;
    plan->closed_right_dest_y = CSB_V1_ENTRANCE_DOOR_SCREEN_Y_PC34;
    plan->closed_right_w = 127;
    plan->closed_right_h = 161;
    plan->closed_right_fallback_fill_color = CSB_V1_FALLBACK_DARK_GRAY_PC34;
    plan->closed_right_fallback_light_edge_color =
        CSB_V1_FALLBACK_LIGHT_GRAY_PC34;
    plan->closed_right_fallback_dark_edge_color =
        CSB_V1_FALLBACK_BLACK_PC34;
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

static void csb_v1_startup_set_opening_composite_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    if (!plan || !plan->opening_door_valid ||
        plan->source_asset_id <= 0 ||
        plan->closed_left_asset_id <= 0 ||
        plan->closed_right_asset_id <= 0) {
        return;
    }
    /* ReDMCSB ENTRANCE.C F0438/F0807 composites C004 with the live dungeon
     * viewport and moving C002/C003 door strips.  M11 supplies pixels, but
     * CSB owns the source asset ids and door-step rectangle contract. */
    plan->opening_composite_valid = 1;
    plan->opening_composite_screen_asset_id = plan->source_asset_id;
    plan->opening_composite_left_asset_id = plan->closed_left_asset_id;
    plan->opening_composite_right_asset_id = plan->closed_right_asset_id;
    plan->opening_composite_animation_step = plan->opening_door_step;
    plan->opening_composite_left_box_x = plan->opening_left_dest_x;
    plan->opening_composite_left_box_y = plan->opening_left_source_y;
    plan->opening_composite_left_box_w = plan->opening_left_w;
    plan->opening_composite_left_box_h = plan->opening_left_h;
    plan->opening_composite_right_box_x = plan->opening_right_dest_x;
    plan->opening_composite_right_box_y = plan->opening_right_source_y;
    plan->opening_composite_right_box_w = plan->opening_right_w;
    plan->opening_composite_right_box_h = plan->opening_right_h;
    plan->opening_composite_left_source_x = plan->opening_left_source_x;
    plan->opening_composite_right_source_x = plan->opening_right_source_x;
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
    plan->blink_prompt_visible = 1;
    csb_v1_startup_rebuild_fallback_text_rows_pc34(plan);
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

CSB_V1_StartupInput_PC34 csb_v1_startup_input_from_firestaff_menu_code_pc34(
    int menu_input)
{
    enum {
        FIRESTAFF_MENU_INPUT_NONE = 0,
        FIRESTAFF_MENU_INPUT_ACCEPT = 9,
        FIRESTAFF_MENU_INPUT_BACK = 10,
        FIRESTAFF_MENU_INPUT_ACTION = 11,
        FIRESTAFF_MENU_INPUT_DISK_MENU = 32
    };

    switch (menu_input) {
        case FIRESTAFF_MENU_INPUT_ACCEPT:
            return CSB_V1_STARTUP_INPUT_ACCEPT_PC34;
        case FIRESTAFF_MENU_INPUT_ACTION:
            return CSB_V1_STARTUP_INPUT_ACTION_PC34;
        case FIRESTAFF_MENU_INPUT_BACK:
            return CSB_V1_STARTUP_INPUT_BACK_PC34;
        case FIRESTAFF_MENU_INPUT_DISK_MENU:
            return CSB_V1_STARTUP_INPUT_DISK_MENU_PC34;
        case FIRESTAFF_MENU_INPUT_NONE:
        default:
            return CSB_V1_STARTUP_INPUT_NONE_PC34;
    }
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

int csb_v1_startup_entrance_command_for_input_pc34(
    int credits_active,
    CSB_V1_StartupInput_PC34 input)
{
    return csb_v1_startup_entrance_command_for_action_pc34(
        (CSB_V1_StartupEntranceAction_PC34)
            csb_v1_startup_entrance_action_for_input_pc34(credits_active,
                                                          input));
}

int csb_v1_startup_entrance_command_for_firestaff_input_pc34(
    int credits_active,
    int menu_input,
    int *out_command)
{
    CSB_V1_StartupInput_PC34 input =
        csb_v1_startup_input_from_firestaff_menu_code_pc34(menu_input);
    int command = csb_v1_startup_entrance_command_for_input_pc34(
        credits_active,
        input);

    if (out_command) {
        *out_command = command;
    }
    return command != CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34 ||
           credits_active;
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

int csb_v1_startup_render_state_from_command_state_pc34(
    const CSB_V1_StartupCommandState_PC34 *command_state,
    int entrance_frame,
    int utility_overlay_active,
    int runtime_start_valid,
    int runtime_start_x,
    int runtime_start_y,
    int runtime_start_dir,
    CSB_V1_StartupRenderState_PC34 *out_state)
{
    if (!command_state || !out_state) {
        return 0;
    }
    memset(out_state, 0, sizeof(*out_state));
    out_state->entrance_active = command_state->entrance_active;
    out_state->entrance_frame = entrance_frame;
    out_state->title_active = command_state->title_active;
    out_state->title_frame = command_state->title_frame;
    out_state->entrance_source_step =
        command_state->entrance_source_step;
    out_state->credits_active = command_state->credits_active;
    out_state->opening_active = command_state->opening_active;
    out_state->opening_delay_ticks = command_state->opening_delay_ticks;
    out_state->opening_step = command_state->opening_step;
    out_state->utility_overlay_active = utility_overlay_active ? 1 : 0;
    out_state->runtime_start_valid = runtime_start_valid ? 1 : 0;
    out_state->runtime_start_x = runtime_start_x;
    out_state->runtime_start_y = runtime_start_y;
    out_state->runtime_start_dir = runtime_start_dir;
    return 1;
}

int csb_v1_startup_build_render_plan_from_request_pc34(
    const CSB_V1_StartupRenderPlanRequest_PC34 *request,
    CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    CSB_V1_StartupCommandStateRequest_PC34 command_request;
    CSB_V1_StartupCommandState_PC34 command_state;
    CSB_V1_StartupRenderState_PC34 render_state;

    if (!out_plan) {
        return 0;
    }
    if (!request) {
        return csb_v1_startup_build_render_plan_pc34(NULL, out_plan);
    }

    memset(&command_request, 0, sizeof(command_request));
    command_request.title_active = request->title_active;
    command_request.title_frame = request->title_frame;
    command_request.title_source_step = request->title_source_step;
    command_request.entrance_active = request->entrance_active;
    command_request.entrance_source_step = request->entrance_source_step;
    command_request.entrance_dismissed = request->entrance_dismissed;
    command_request.credits_active = request->credits_active;
    command_request.credits_remaining_ticks = request->credits_remaining_ticks;
    command_request.opening_active = request->opening_active;
    command_request.opening_delay_ticks = request->opening_delay_ticks;
    command_request.opening_step = request->opening_step;
    command_request.pending_command = request->pending_command;
    if (!csb_v1_startup_command_state_from_request_pc34(&command_request,
                                                        &command_state)) {
        return 0;
    }

    if (!csb_v1_startup_render_state_from_command_state_pc34(
            &command_state,
            request->entrance_frame,
            request->utility_overlay_active,
            request->runtime_start_valid,
            request->runtime_start_x,
            request->runtime_start_y,
            request->runtime_start_dir,
            &render_state)) {
        return 0;
    }
    return csb_v1_startup_build_render_plan_pc34(&render_state, out_plan);
}

int csb_v1_startup_command_state_from_request_pc34(
    const CSB_V1_StartupCommandStateRequest_PC34 *request,
    CSB_V1_StartupCommandState_PC34 *out_state)
{
    if (!out_state) {
        return 0;
    }
    memset(out_state, 0, sizeof(*out_state));
    if (!request) {
        return 1;
    }
    out_state->title_active = request->title_active ? 1 : 0;
    out_state->title_frame = request->title_frame;
    out_state->title_source_step = request->title_source_step;
    out_state->entrance_active = request->entrance_active ? 1 : 0;
    out_state->entrance_source_step = request->entrance_source_step;
    out_state->entrance_dismissed = request->entrance_dismissed ? 1 : 0;
    out_state->credits_active = request->credits_active ? 1 : 0;
    out_state->credits_remaining_ticks = request->credits_remaining_ticks;
    out_state->opening_active = request->opening_active ? 1 : 0;
    out_state->opening_delay_ticks = request->opening_delay_ticks;
    out_state->opening_step = request->opening_step;
    out_state->pending_command = request->pending_command;
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
    csb_v1_startup_clear_surface_blit_pc34(&plan);
    csb_v1_startup_clear_title_rect_pc34(&plan);
    csb_v1_startup_clear_door_rects_pc34(&plan);
    csb_v1_startup_clear_fallback_text_pc34(&plan);
    csb_v1_startup_clear_asset_commands_pc34(&plan);
    csb_v1_startup_clear_primitive_commands_pc34(&plan);
    csb_v1_startup_clear_render_commands_pc34(&plan);
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
        csb_v1_startup_rebuild_asset_commands_pc34(&plan);
        csb_v1_startup_rebuild_primitive_commands_pc34(&plan);
        csb_v1_startup_rebuild_render_commands_pc34(&plan);
        *out_plan = plan;
        return 1;
    }
    if (state->credits_active) {
        plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34;
        plan.source_asset_id = CSB_V1_GRAPHIC_ENTRANCE_CREDITS_PC34;
        csb_v1_startup_set_full_surface_blit_pc34(&plan);
        plan.special_palette = VGA_PALETTE_PC34_SPECIAL_CREDITS;
        csb_v1_startup_set_credits_fallback_text_pc34(&plan);
        csb_v1_startup_rebuild_asset_commands_pc34(&plan);
        csb_v1_startup_rebuild_primitive_commands_pc34(&plan);
        csb_v1_startup_rebuild_render_commands_pc34(&plan);
        *out_plan = plan;
        return 1;
    }
    if (!plan.waiting_for_input && state->entrance_source_step == 2) {
        /* ReDMCSB ENTRANCE.C F0441 lines 426-443 fades/curtains to black
         * before C004/C002/C003 are redrawn. */
        plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34;
        plan.special_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
        csb_v1_startup_rebuild_asset_commands_pc34(&plan);
        csb_v1_startup_rebuild_primitive_commands_pc34(&plan);
        csb_v1_startup_rebuild_render_commands_pc34(&plan);
        *out_plan = plan;
        return 1;
    }
    if (state->opening_active) {
        plan.surface = state->opening_delay_ticks > 0
            ? CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34
            : CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34;
        plan.opening_step = state->opening_step;
        plan.source_asset_id = CSB_V1_GRAPHIC_ENTRANCE_SCREEN_PC34;
        csb_v1_startup_set_full_surface_blit_pc34(&plan);
        plan.special_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
        csb_v1_startup_set_closed_door_rects_pc34(&plan);
        if (plan.surface ==
            CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34) {
            csb_v1_startup_set_opening_door_rects_pc34(&plan);
            csb_v1_startup_set_opening_composite_pc34(&plan);
        }
        csb_v1_startup_rebuild_asset_commands_pc34(&plan);
        csb_v1_startup_rebuild_primitive_commands_pc34(&plan);
        csb_v1_startup_rebuild_render_commands_pc34(&plan);
        *out_plan = plan;
        return 1;
    }
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.source_asset_id = CSB_V1_GRAPHIC_ENTRANCE_SCREEN_PC34;
    csb_v1_startup_set_full_surface_blit_pc34(&plan);
    plan.special_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
    csb_v1_startup_set_closed_door_rects_pc34(&plan);
    csb_v1_startup_set_entrance_fallback_text_pc34(&plan);
    plan.fallback_status_visible = !state->utility_overlay_active;
    if (state->runtime_start_valid) {
        plan.fallback_detail_visible = !state->utility_overlay_active;
        plan.fallback_runtime_detail_visible = 1;
        snprintf(plan.fallback_runtime_detail_text,
                 sizeof(plan.fallback_runtime_detail_text),
                 "START %d,%d DIR %d",
                 state->runtime_start_x,
                 state->runtime_start_y,
                 state->runtime_start_dir);
    }
    plan.blink_prompt_visible =
        ((state->entrance_frame / 12) & 1) == 0;
    csb_v1_startup_rebuild_fallback_text_rows_pc34(&plan);
    csb_v1_startup_rebuild_asset_commands_pc34(&plan);
    csb_v1_startup_rebuild_primitive_commands_pc34(&plan);
    csb_v1_startup_rebuild_render_commands_pc34(&plan);
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

void csb_v1_startup_session_options_init_pc34(
    CSB_V1_StartupSessionOptions_PC34 *options)
{
    if (!options) {
        return;
    }
    memset(options, 0, sizeof(*options));
    options->import_utility_state = (int)CSB_V1_UTIL_FLOW_INIT;
}

static int csb_v1_startup_copy_bounded_string_pc34(char *dst,
                                                   size_t dst_size,
                                                   const char *src)
{
    int rc;
    if (!dst || dst_size == 0u) {
        return 0;
    }
    dst[0] = '\0';
    if (!src || src[0] == '\0') {
        return 0;
    }
    rc = snprintf(dst, dst_size, "%s", src);
    if (rc <= 0 || (size_t)rc >= dst_size) {
        dst[0] = '\0';
        return 0;
    }
    return 1;
}

int csb_v1_startup_build_session_options_pc34(
    const CSB_V1_StartupSessionOptionsInput_PC34 *input,
    CSB_V1_StartupSessionOptions_PC34 *out_options)
{
    if (!out_options) {
        return 0;
    }
    csb_v1_startup_session_options_init_pc34(out_options);
    if (!input || input->direct_resume_loaded) {
        return 1;
    }

    if (input->import_party_loaded &&
        input->import_champion_count > 0 &&
        csb_v1_startup_copy_bounded_string_pc34(
            out_options->import_dm1_save_path,
            sizeof(out_options->import_dm1_save_path),
            input->import_dm1_save_path)) {
        out_options->import_available = 1;
        out_options->import_champion_count = input->import_champion_count;
        out_options->import_selected_action_index = 0;
        out_options->import_preview_active = 0;
        out_options->import_utility_state = input->import_utility_state;
        (void)csb_v1_startup_copy_bounded_string_pc34(
            out_options->import_utility_prompt,
            sizeof(out_options->import_utility_prompt),
            input->import_utility_prompt);
    }

    if (input->entrance_resume_can_load &&
        csb_v1_startup_copy_bounded_string_pc34(
            out_options->entrance_resume_path,
            sizeof(out_options->entrance_resume_path),
            input->entrance_resume_save_path)) {
        out_options->entrance_resume_available = 1;
    }
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

int csb_v1_startup_entrance_input_outcome_pc34(
    const CSB_V1_StartupEntranceCommandPlan_PC34 *plan,
    int resume_available,
    int resume_loaded,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome)
{
    if (!out_outcome) {
        return 0;
    }
    memset(out_outcome, 0, sizeof(*out_outcome));
    out_outcome->result = CSB_V1_STARTUP_ENTRANCE_INPUT_IGNORE_PC34;
    if (!plan) {
        return 0;
    }

    switch (plan->kind) {
        case CSB_V1_STARTUP_ENTRANCE_PLAN_DISMISS_CREDITS_PC34:
        case CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_DUNGEON_PC34:
        case CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_BONUS_DUNGEON_PC34:
        case CSB_V1_STARTUP_ENTRANCE_PLAN_BEGIN_CREDITS_PC34:
            out_outcome->result = CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34;
            out_outcome->status_scope =
                plan->status_scope ? plan->status_scope : "BOOT";
            out_outcome->status = plan->status ? plan->status : "CSB DOORS";
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_PLAN_RESUME_PC34:
            out_outcome->result = CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34;
            out_outcome->status_scope =
                plan->status_scope ? plan->status_scope : "BOOT";
            if (resume_loaded) {
                out_outcome->status =
                    plan->status ? plan->status : "CSB DOORS";
            } else if (resume_available) {
                out_outcome->status = plan->failure_status
                    ? plan->failure_status
                    : "CSB RESUME FAILED";
            } else {
                out_outcome->status = plan->unavailable_status
                    ? plan->unavailable_status
                    : "CSB RESUME UNAVAILABLE";
            }
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_PLAN_QUIT_PC34:
            out_outcome->result =
                CSB_V1_STARTUP_ENTRANCE_INPUT_RETURN_TO_LAUNCHER_PC34;
            out_outcome->status_scope =
                plan->status_scope ? plan->status_scope : "RETURN";
            out_outcome->status =
                plan->status ? plan->status : "BACK TO LAUNCHER";
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_PLAN_IGNORE_PC34:
        default:
            return 1;
    }
}

CSB_V1_StartupEntranceApplyResult_PC34
csb_v1_startup_apply_pure_entrance_plan_pc34(
    CSB_V1_StartupCommandState_PC34 *state,
    const CSB_V1_StartupEntranceCommandPlan_PC34 *plan,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome)
{
    if (out_outcome) {
        memset(out_outcome, 0, sizeof(*out_outcome));
        out_outcome->result = CSB_V1_STARTUP_ENTRANCE_INPUT_IGNORE_PC34;
    }
    if (!state || !plan) {
        return CSB_V1_STARTUP_ENTRANCE_APPLY_NOT_HANDLED_PC34;
    }

    switch (plan->kind) {
        case CSB_V1_STARTUP_ENTRANCE_PLAN_DISMISS_CREDITS_PC34:
            (void)csb_v1_startup_dismiss_credits_pc34(state);
            (void)csb_v1_startup_entrance_input_outcome_pc34(
                plan,
                0,
                0,
                out_outcome);
            return CSB_V1_STARTUP_ENTRANCE_APPLY_REDRAW_PC34;
        case CSB_V1_STARTUP_ENTRANCE_PLAN_BEGIN_CREDITS_PC34:
            (void)csb_v1_startup_begin_credits_pc34(state);
            (void)csb_v1_startup_entrance_input_outcome_pc34(
                plan,
                0,
                0,
                out_outcome);
            return CSB_V1_STARTUP_ENTRANCE_APPLY_REDRAW_PC34;
        case CSB_V1_STARTUP_ENTRANCE_PLAN_QUIT_PC34:
            (void)csb_v1_startup_quit_to_launcher_pc34(state);
            (void)csb_v1_startup_entrance_input_outcome_pc34(
                plan,
                0,
                0,
                out_outcome);
            return CSB_V1_STARTUP_ENTRANCE_APPLY_RETURN_TO_LAUNCHER_PC34;
        case CSB_V1_STARTUP_ENTRANCE_PLAN_IGNORE_PC34:
            (void)csb_v1_startup_entrance_input_outcome_pc34(
                plan,
                0,
                0,
                out_outcome);
            return CSB_V1_STARTUP_ENTRANCE_APPLY_IGNORED_PC34;
        case CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_DUNGEON_PC34:
        case CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_BONUS_DUNGEON_PC34:
        case CSB_V1_STARTUP_ENTRANCE_PLAN_RESUME_PC34:
        default:
            return CSB_V1_STARTUP_ENTRANCE_APPLY_NOT_HANDLED_PC34;
    }
}

void csb_v1_startup_runtime_plan_init_pc34(
    CSB_V1_StartupRuntimePlan_PC34 *runtime_plan)
{
    if (!runtime_plan) {
        return;
    }
    memset(runtime_plan, 0, sizeof(*runtime_plan));
    runtime_plan->kind = CSB_V1_STARTUP_RUNTIME_PLAN_NONE_PC34;
    runtime_plan->command_id = CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
}

int csb_v1_startup_runtime_plan_for_entrance_plan_pc34(
    const CSB_V1_StartupEntranceCommandPlan_PC34 *plan,
    CSB_V1_StartupRuntimePlan_PC34 *out_runtime_plan)
{
    if (!out_runtime_plan) {
        return 0;
    }
    csb_v1_startup_runtime_plan_init_pc34(out_runtime_plan);
    if (!plan) {
        return 0;
    }
    out_runtime_plan->command_id = plan->command_id;
    out_runtime_plan->status_scope = plan->status_scope;
    out_runtime_plan->status = plan->status;
    out_runtime_plan->failure_status = plan->failure_status;
    out_runtime_plan->unavailable_status = plan->unavailable_status;

    switch (plan->kind) {
        case CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_DUNGEON_PC34:
            out_runtime_plan->kind =
                CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_DUNGEON_PC34;
            out_runtime_plan->set_bonus_dungeon = 1;
            out_runtime_plan->bonus_dungeon = 0;
            out_runtime_plan->begin_door_opening = 1;
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_BONUS_DUNGEON_PC34:
            out_runtime_plan->kind =
                CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_BONUS_DUNGEON_PC34;
            out_runtime_plan->set_bonus_dungeon = 1;
            out_runtime_plan->bonus_dungeon = 1;
            out_runtime_plan->begin_door_opening = 1;
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_PLAN_RESUME_PC34:
            out_runtime_plan->kind = CSB_V1_STARTUP_RUNTIME_PLAN_RESUME_PC34;
            out_runtime_plan->requires_resume_load = 1;
            out_runtime_plan->begin_door_opening = 1;
            return 1;
        case CSB_V1_STARTUP_ENTRANCE_PLAN_DISMISS_CREDITS_PC34:
        case CSB_V1_STARTUP_ENTRANCE_PLAN_BEGIN_CREDITS_PC34:
        case CSB_V1_STARTUP_ENTRANCE_PLAN_QUIT_PC34:
        case CSB_V1_STARTUP_ENTRANCE_PLAN_IGNORE_PC34:
        default:
            return 0;
    }
}

int csb_v1_startup_apply_runtime_plan_pc34(
    CSB_V1_StartupCommandState_PC34 *state,
    const CSB_V1_StartupRuntimePlan_PC34 *runtime_plan,
    int resume_available,
    int resume_loaded,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome)
{
    CSB_V1_StartupEntranceCommandPlan_PC34 outcome_plan;

    if (out_outcome) {
        memset(out_outcome, 0, sizeof(*out_outcome));
        out_outcome->result = CSB_V1_STARTUP_ENTRANCE_INPUT_IGNORE_PC34;
    }
    if (!state || !runtime_plan) {
        return 0;
    }
    if (runtime_plan->kind == CSB_V1_STARTUP_RUNTIME_PLAN_NONE_PC34) {
        return 0;
    }

    csb_v1_startup_entrance_command_plan_init_pc34(&outcome_plan);
    outcome_plan.command_id = runtime_plan->command_id;
    outcome_plan.status_scope = runtime_plan->status_scope;
    outcome_plan.status = runtime_plan->status;
    outcome_plan.failure_status = runtime_plan->failure_status;
    outcome_plan.unavailable_status = runtime_plan->unavailable_status;

    switch (runtime_plan->kind) {
        case CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_DUNGEON_PC34:
            outcome_plan.kind =
                CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_DUNGEON_PC34;
            break;
        case CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_BONUS_DUNGEON_PC34:
            outcome_plan.kind =
                CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_BONUS_DUNGEON_PC34;
            break;
        case CSB_V1_STARTUP_RUNTIME_PLAN_RESUME_PC34:
            outcome_plan.kind = CSB_V1_STARTUP_ENTRANCE_PLAN_RESUME_PC34;
            if (!resume_available || !resume_loaded) {
                (void)csb_v1_startup_entrance_input_outcome_pc34(
                    &outcome_plan,
                    resume_available,
                    resume_loaded,
                    out_outcome);
                return 1;
            }
            break;
        case CSB_V1_STARTUP_RUNTIME_PLAN_NONE_PC34:
        default:
            return 0;
    }

    if (runtime_plan->begin_door_opening) {
        (void)csb_v1_startup_begin_door_opening_pc34(
            state,
            runtime_plan->command_id);
    }
    (void)csb_v1_startup_entrance_input_outcome_pc34(
        &outcome_plan,
        resume_available,
        runtime_plan->kind == CSB_V1_STARTUP_RUNTIME_PLAN_RESUME_PC34
            ? resume_loaded
            : 0,
        out_outcome);
    return 1;
}

void csb_v1_startup_runtime_apply_receipt_init_pc34(
    CSB_V1_StartupRuntimeApplyReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->result = CSB_V1_STARTUP_RUNTIME_APPLY_NOT_HANDLED_PC34;
}

int csb_v1_startup_apply_runtime_plan_with_receipt_pc34(
    CSB_V1_StartupCommandState_PC34 *state,
    const CSB_V1_StartupRuntimePlan_PC34 *runtime_plan,
    int resume_available,
    int resume_loaded,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome,
    CSB_V1_StartupRuntimeApplyReceipt_PC34 *out_receipt)
{
    int applied;

    if (out_receipt) {
        csb_v1_startup_runtime_apply_receipt_init_pc34(out_receipt);
    }
    if (!state || !runtime_plan ||
        runtime_plan->kind == CSB_V1_STARTUP_RUNTIME_PLAN_NONE_PC34) {
        return 0;
    }

    applied = csb_v1_startup_apply_runtime_plan_pc34(state,
                                                     runtime_plan,
                                                     resume_available,
                                                     resume_loaded,
                                                     out_outcome);
    if (!applied) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->clear_import_preview = 1;
        switch (runtime_plan->kind) {
            case CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_DUNGEON_PC34:
                out_receipt->result =
                    CSB_V1_STARTUP_RUNTIME_APPLY_REDRAW_PC34;
                out_receipt->bonus_requested_changed = 1;
                out_receipt->bonus_requested = 0;
                break;
            case CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_BONUS_DUNGEON_PC34:
                out_receipt->result =
                    CSB_V1_STARTUP_RUNTIME_APPLY_REDRAW_PC34;
                out_receipt->bonus_requested_changed = 1;
                out_receipt->bonus_requested = 1;
                break;
            case CSB_V1_STARTUP_RUNTIME_PLAN_RESUME_PC34:
                out_receipt->result =
                    CSB_V1_STARTUP_RUNTIME_APPLY_REDRAW_PC34;
                break;
            case CSB_V1_STARTUP_RUNTIME_PLAN_NONE_PC34:
            default:
                out_receipt->result =
                    CSB_V1_STARTUP_RUNTIME_APPLY_IGNORED_PC34;
                break;
        }
    }
    return 1;
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
