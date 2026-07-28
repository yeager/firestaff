#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "csb_v1_boot.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    CSB_V1_TITLE_PRESENTS_TICKS_PC34 = 60,
    /* ReDMCSB TITLE.C F0437 presents the PC-path CHAOS raster sequence
     * through source steps 2..21, keeps the full CHAOS image through the
     * post-zoom hold, then blits STRIKES BACK before ENTRANCE.C. */
    CSB_V1_TITLE_CHAOS_ZOOM_TICKS_PC34 = 20,
    /* TITLE.C:460 keeps the full C425 CHAOS raster for Delay(20) after its
     * 20 shrink frames.  C426 STRIKES BACK is then held by Delay(2). */
    CSB_V1_TITLE_CHAOS_HOLD_TICKS_PC34 = 20,
    /* TITLE.C:461-463 draws CM60 then calls F0022_MAIN_Delay(2).
     * Keep the STRIKES BACK raster on screen for both source VBlanks before
     * ENTRANCE.C gets ownership; the old one-tick route cut that final title
     * image in half on a 50 Hz host. */
    CSB_V1_TITLE_STRIKES_BACK_TICKS_PC34 = 2,
    CSB_V1_TITLE_TOTAL_TICKS_PC34 = 102,
    CSB_V1_ENTRANCE_WAIT_SOURCE_STEP_PC34 = 4,
    CSB_V1_ENTRANCE_PRE_OPEN_DELAY_TICKS_PC34 = 20,
    CSB_V1_ENTRANCE_CREDITS_TICKS_PC34 = 1800,
    CSB_V1_ENTRANCE_DOOR_SCREEN_Y_PC34 = 30,
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
    CSB_V1_FALLBACK_DARK_GRAY_PC34 = 12,
    CSB_V1_ENTRANCE_DOOR_STEP_COUNT_PC34 = 31
};

typedef struct CSB_V1_StartupCommandStateRequest_PC34 {
    int title_active;
    int title_frame;
    int title_source_step;
    int entrance_active;
    int entrance_source_step;
    int entrance_dismissed;
    int credits_active;
    int credits_remaining_ticks;
    int opening_active;
    int opening_delay_ticks;
    int opening_step;
    int pending_command;
} CSB_V1_StartupCommandStateRequest_PC34;

typedef struct CSB_V1_StartupRenderPlanRequest_PC34 {
    int title_active;
    int title_frame;
    int title_source_step;
    int entrance_active;
    int entrance_source_step;
    int entrance_dismissed;
    int credits_active;
    int credits_remaining_ticks;
    int opening_active;
    int opening_delay_ticks;
    int opening_step;
    int pending_command;
    int entrance_frame;
    int utility_overlay_active;
    int resume_available;
    int runtime_start_valid;
    int runtime_start_x;
    int runtime_start_y;
    int runtime_start_dir;
} CSB_V1_StartupRenderPlanRequest_PC34;

static int csb_v1_startup_command_state_from_request_pc34(
    const CSB_V1_StartupCommandStateRequest_PC34 *request,
    CSB_V1_StartupCommandState_PC34 *out_state);
static int csb_v1_startup_entrance_accepts_input_from_request_pc34(
    const CSB_V1_StartupCommandStateRequest_PC34 *request);

void csb_v1_startup_host_facts_init_pc34(
    CSB_V1_StartupHostFacts_PC34 *facts)
{
    if (!facts) {
        return;
    }
    memset(facts, 0, sizeof(*facts));
}

int csb_v1_startup_host_facts_from_runtime_state_pc34(
    CSB_V1_StartupHostFacts_PC34 *facts,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const void *boot_profile)
{
    if (!facts) {
        return 0;
    }
    csb_v1_startup_host_facts_init_pc34(facts);
    facts->title_active = title_active;
    facts->title_frame = title_frame;
    facts->title_source_step = title_source_step;
    facts->entrance_active = entrance_active;
    facts->entrance_source_step = entrance_source_step;
    facts->entrance_dismissed = entrance_dismissed;
    facts->credits_active = credits_active;
    facts->credits_remaining_ticks = credits_remaining_ticks;
    facts->opening_active = opening_active;
    facts->opening_delay_ticks = opening_delay_ticks;
    facts->opening_step = opening_step;
    facts->pending_command = pending_command;
    facts->entrance_frame = entrance_frame;
    facts->utility_overlay_active = utility_overlay_active;
    facts->utility_selected_action_index = utility_selected_action_index;
    facts->utility_imported_champion_count = utility_imported_champion_count;
    facts->utility_preview_active = utility_preview_active;
    facts->utility_prompt = utility_prompt;
    facts->door_step_count = CSB_V1_ENTRANCE_DOOR_STEP_COUNT_PC34;
    facts->resume_available = resume_available ? 1 : 0;
    facts->resume_path = resume_path;
    facts->boot_profile = boot_profile;
    return 1;
}

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

int csb_v1_startup_title_chaos_zoom_ticks_pc34(void)
{
    return CSB_V1_TITLE_CHAOS_ZOOM_TICKS_PC34;
}

int csb_v1_startup_title_chaos_hold_ticks_pc34(void)
{
    return CSB_V1_TITLE_CHAOS_HOLD_TICKS_PC34;
}

int csb_v1_startup_title_strikes_back_ticks_pc34(void)
{
    return CSB_V1_TITLE_STRIKES_BACK_TICKS_PC34;
}

int csb_v1_startup_title_stage_for_frame_pc34(int frame)
{
    if (frame < CSB_V1_TITLE_PRESENTS_TICKS_PC34) {
        return CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34;
    }
    if (frame >= CSB_V1_TITLE_PRESENTS_TICKS_PC34 +
                     CSB_V1_TITLE_CHAOS_ZOOM_TICKS_PC34 +
                     CSB_V1_TITLE_CHAOS_HOLD_TICKS_PC34) {
        return CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34;
    }
    return CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34;
}

unsigned int csb_v1_startup_title_source_step_for_frame_pc34(int frame)
{
    /* ReDMCSB: TITLE.C F0437 lines 425-463 uses the CSB title path:
     * CM58 PRESENTS is blitted, then TITLE.C waits until
     * G0317_i_WaitForInputVerticalBlankCount + 60. The PC path reaches the
     * full CHAOS bitmap at source step 21, holds it through Delay(20),
     * then draws STRIKES BACK at source step 22 for Delay(2). */
    if (frame < CSB_V1_TITLE_PRESENTS_TICKS_PC34) {
        return 1u;
    }
    if (frame < CSB_V1_TITLE_PRESENTS_TICKS_PC34 +
                    CSB_V1_TITLE_CHAOS_ZOOM_TICKS_PC34) {
        return (unsigned int)(frame - CSB_V1_TITLE_PRESENTS_TICKS_PC34 + 2);
    }
    if (frame < CSB_V1_TITLE_PRESENTS_TICKS_PC34 +
                    CSB_V1_TITLE_CHAOS_ZOOM_TICKS_PC34 +
                    CSB_V1_TITLE_CHAOS_HOLD_TICKS_PC34) return 21u;
    return 22u;
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

static void csb_v1_startup_clear_menu_options_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    int i;
    if (!plan) {
        return;
    }
    plan->menu_option_count = 0;
    for (i = 0; i < CSB_V1_STARTUP_MENU_OPTION_CAP_PC34; ++i) {
        plan->menu_options[i].command_id =
            CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
        plan->menu_options[i].enabled = 0;
        plan->menu_options[i].selected = 0;
        plan->menu_options[i].x = 0;
        plan->menu_options[i].y = 0;
        plan->menu_options[i].label = NULL;
        plan->menu_options[i].unavailable_label = NULL;
    }
}

static void csb_v1_startup_add_menu_option_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan,
    int command_id,
    int enabled,
    int selected,
    int x,
    int y,
    const char *label,
    const char *unavailable_label)
{
    CSB_V1_StartupMenuOption_PC34 *option;
    if (!plan || !label || label[0] == '\0' ||
        plan->menu_option_count >= CSB_V1_STARTUP_MENU_OPTION_CAP_PC34) {
        return;
    }
    option = &plan->menu_options[plan->menu_option_count++];
    option->command_id = command_id;
    option->enabled = enabled ? 1 : 0;
    option->selected = selected ? 1 : 0;
    option->x = x;
    option->y = y;
    option->label = label;
    option->unavailable_label = unavailable_label;
}

static void csb_v1_startup_set_entrance_menu_options_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan,
    int resume_available)
{
    if (!plan) {
        return;
    }
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 waits on the entrance
     * command loop, with C001/C000/C203/C216-style command exits.  This
     * table exposes that menu contract to M11/HUD consumers without moving
     * command execution out of the CSB startup module. */
    csb_v1_startup_clear_menu_options_pc34(plan);
    csb_v1_startup_add_menu_option_pc34(
        plan,
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
        1,
        1,
        38,
        108,
        "ENTER DUNGEON",
        NULL);
    csb_v1_startup_add_menu_option_pc34(
        plan,
        CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34,
        resume_available,
        0,
        38,
        120,
        "RESUME",
        "RESUME UNAVAILABLE");
    csb_v1_startup_add_menu_option_pc34(
        plan,
        CSB_V1_STARTUP_ENTRANCE_COMMAND_DRAW_CREDITS_PC34,
        1,
        0,
        38,
        132,
        "CREDITS",
        NULL);
    csb_v1_startup_add_menu_option_pc34(
        plan,
        CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34,
        1,
        0,
        38,
        144,
        "QUIT",
        NULL);
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
            case CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34:
                if (!executor->draw_full_surface ||
                    !executor->draw_full_surface(executor->user, plan)) {
                    /* The packaged route has attempted its real surface.
                     * Do not manufacture an entrance/title fallback after a
                     * source-asset failure. */
                    return 1;
                }
                drew_asset = 1;
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_OPENING_FRAME_IF_SURFACE_PC34:
                if (!drew_asset) {
                    return 1;
                }
                if (!executor->draw_opening_frame ||
                    !executor->draw_opening_frame(executor->user, plan)) {
                    return 0;
                }
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_ELSE_FALLBACK_PC34:
            case CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_PC34:
                if (!drew_asset) {
                    return 1;
                }
                if (!executor->draw_closed_doors) {
                    return 0;
                }
                executor->draw_closed_doors(executor->user, plan);
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_FALLBACK_IF_NO_SURFACE_PC34:
                /* Retained for old serialized plans. A CSB package failure
                 * must stay visible as a failed draw, never synthesized UI. */
                if (!drew_asset) return 0;
                break;
            case CSB_V1_STARTUP_RENDER_COMMAND_UTILITY_PANEL_IF_WAITING_PC34:
                if (plan->waiting_for_input && executor->draw_utility_panel) {
                    executor->draw_utility_panel(executor->user, plan, NULL);
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
    /* ReDMCSB ENTRANCE.C owns C004/C002/C003.  In particular, do not add
     * synthetic door panels or a decorative text frame after those package
     * assets were scheduled: a missing asset is a failed entrance draw. */
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
        /* ReDMCSB TITLE.C F0437 lines 429, 453 clears the screen before
         * PRESENTS and before the CHAOS zoom/hold transaction. Keep clear
         * as an explicit CSB render command so host title drawing does not
         * depend on stale FTL/previous-title framebuffer contents. */
        csb_v1_startup_add_render_command_pc34(
            plan, CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34);
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
            plan, CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34);
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
            CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_PC34);
        return;
    }
    csb_v1_startup_add_render_command_pc34(
        plan,
        CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_PC34);
    csb_v1_startup_add_render_command_pc34(
        plan,
        CSB_V1_STARTUP_RENDER_COMMAND_UTILITY_PANEL_IF_WAITING_PC34);
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
    /* ReDMCSB ENTRANCE.C F0438 lines 172-239 composites C004 with the live
     * dungeon viewport and moving C002/C003 door strips at screen y=28.
     * M11 supplies pixels, but CSB owns the source asset ids and destination
     * rectangle contract. */
    plan->opening_composite_valid = 1;
    plan->opening_composite_screen_asset_id = plan->source_asset_id;
    plan->opening_composite_left_asset_id = plan->closed_left_asset_id;
    plan->opening_composite_right_asset_id = plan->closed_right_asset_id;
    plan->opening_composite_animation_step = plan->opening_door_step;
    plan->opening_composite_left_box_x = plan->opening_left_dest_x;
    plan->opening_composite_left_box_y = plan->opening_left_dest_y;
    plan->opening_composite_left_box_w = plan->opening_left_w;
    plan->opening_composite_left_box_h = plan->opening_left_h;
    plan->opening_composite_right_box_x = plan->opening_right_dest_x;
    plan->opening_composite_right_box_y = plan->opening_right_dest_y;
    plan->opening_composite_right_box_w = plan->opening_right_w;
    plan->opening_composite_right_box_h = plan->opening_right_h;
    plan->opening_composite_left_source_x = plan->opening_left_source_x;
    plan->opening_composite_left_source_y = plan->opening_left_source_y;
    plan->opening_composite_right_source_x = plan->opening_right_source_x;
    plan->opening_composite_right_source_y = plan->opening_right_source_y;
}

static void csb_v1_startup_set_title_rect_pc34(
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    int zoom_index;
    int zoom_w;
    int zoom_h;

    if (!plan) {
        return;
    }
    plan->source_asset_id = CSB_V1_GRAPHIC_TITLE_PC34;
    csb_v1_startup_clear_title_rect_pc34(plan);
    /* ReDMCSB TITLE.C F0437 lines 430, 433-457, and 461 draw the title
     * through C424 PRESENTS, C425 CHAOS, and C426 STRIKES BACK zones. The
     * source plan must not also manufacture text to replace a missing C001
     * region: a real-asset session either presents that region or fails
     * closed at the host-surface boundary. */
    if (plan->title_stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34) {
        /* ReDMCSB STARTND2.C F0437 / CSBWin _DisplayChaosStrikesBack:
         * C424 starts with CSB's dark-blue palette and white index 15. */
        plan->title_special_palette =
            VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS;
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
        return;
    }
    if (plan->title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
        plan->title_source_step >= 2 &&
        plan->title_source_step <= 21) {
        /* ReDMCSB TITLE.C F0437:190-199 applies C425's CSB-only gold and
         * dark-blue slots before the source-ordered zoom raster. */
        plan->title_special_palette = VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS;
        plan->special_palette = plan->title_special_palette;
        /* ReDMCSB PC TITLE.C F0437 lines 340-360 creates shrinked
         * bitmaps from the full 320x80 title source, and lines 385-387
         * blit those bitmaps centered on the screen.  Do not crop the
         * source: the animation grows the full CHAOS image from 16x4 to
         * 320x80. */
        zoom_index = plan->title_source_step <= 19
            ? 19 - plan->title_source_step
            : 0;
        zoom_w = 320 - 16 * zoom_index;
        zoom_h = 80 - 4 * zoom_index;
        plan->title_source_x = 0;
        plan->title_source_y = 0;
        plan->title_source_w = 320;
        plan->title_source_h = 80;
        plan->title_dest_x = (320 - zoom_w) >> 1;
        plan->title_dest_y = (160 - zoom_h) >> 1;
        plan->title_dest_w = zoom_w;
        plan->title_dest_h = zoom_h;
        plan->title_blit_kind =
            CSB_V1_STARTUP_TITLE_BLIT_SCALED_REGION_PC34;
        plan->title_transparent_color = -1;
        return;
    }
    if (plan->title_stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34) {
        /* F0437 changes slots 10 and 12 after C426.  This is a distinct
         * CSB transaction, not the DM PC/F20 DUNGEON+MASTER palette. */
        plan->title_special_palette = VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES;
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
        /* ReDMCSB TITLE.C F0437 keeps the title frame and source blit step
         * in lockstep at the host receipt boundary; otherwise the CHAOS zoom
         * frame that starts after PRESENTS fails source-coherence validation. */
        state->title_source_step =
            (int)csb_v1_startup_title_source_step_for_frame_pc34(
                state->title_frame);
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

void csb_v1_startup_tick_receipt_init_pc34(
    CSB_V1_StartupTickReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_startup_command_state_receipt_init_pc34(&receipt->state);
}

int csb_v1_startup_advance_tick_from_facts_with_receipt_pc34(
    int entrance_frame,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_source_step,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int door_step_count,
    CSB_V1_StartupTickReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupTickState_PC34 state;

    if (out_receipt) {
        csb_v1_startup_tick_receipt_init_pc34(out_receipt);
    }
    if (!out_receipt) {
        return 0;
    }

    memset(&state, 0, sizeof(state));
    state.entrance_frame = entrance_frame;
    state.title_active = title_active;
    state.title_frame = title_frame;
    state.title_source_step = title_source_step;
    state.entrance_source_step = entrance_source_step;
    state.credits_active = credits_active;
    state.credits_remaining_ticks = credits_remaining_ticks;
    state.opening_active = opening_active;
    state.opening_delay_ticks = opening_delay_ticks;
    state.opening_step = opening_step;
    state.door_step_count = door_step_count;

    if (!csb_v1_startup_advance_tick_pc34(
            &state,
            &out_receipt->tick_result)) {
        csb_v1_startup_tick_receipt_init_pc34(out_receipt);
        return 0;
    }
    out_receipt->entrance_frame = state.entrance_frame;
    return csb_v1_startup_command_state_receipt_from_facts_pc34(
        state.title_active,
        state.title_frame,
        state.title_source_step,
        1,
        state.entrance_source_step,
        0,
        state.credits_active,
        state.credits_remaining_ticks,
        state.opening_active,
        state.opening_delay_ticks,
        state.opening_step,
        pending_command,
        &out_receipt->state);
}

int csb_v1_startup_advance_tick_from_host_facts_with_receipt_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts,
    CSB_V1_StartupTickReceipt_PC34 *out_receipt)
{
    if (!facts) {
        if (out_receipt) {
            csb_v1_startup_tick_receipt_init_pc34(out_receipt);
        }
        return 0;
    }
    return csb_v1_startup_advance_tick_from_facts_with_receipt_pc34(
        facts->entrance_frame,
        facts->title_active,
        facts->title_frame,
        facts->title_source_step,
        facts->entrance_source_step,
        facts->credits_active,
        facts->credits_remaining_ticks,
        facts->opening_active,
        facts->opening_delay_ticks,
        facts->opening_step,
        facts->pending_command,
        facts->door_step_count,
        out_receipt);
}

void csb_v1_startup_idle_receipt_init_pc34(
    CSB_V1_StartupIdleReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_startup_tick_receipt_init_pc34(&receipt->tick_receipt);
    csb_v1_startup_command_state_receipt_init_pc34(
        &receipt->finish_receipt);
    csb_v1_startup_host_receipt_init_pc34(&receipt->host_receipt);
}

int csb_v1_startup_advance_idle_from_host_facts_with_receipt_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts,
    CSB_V1_StartupIdleReceipt_PC34 *out_receipt)
{
    int pending_command;

    if (out_receipt) {
        csb_v1_startup_idle_receipt_init_pc34(out_receipt);
    }
    if (!facts || !out_receipt) {
        return 0;
    }
    if (!csb_v1_startup_advance_tick_from_host_facts_with_receipt_pc34(
            facts,
            &out_receipt->tick_receipt)) {
        return 0;
    }
    if (out_receipt->tick_receipt.tick_result.title_finished ||
        out_receipt->tick_receipt.tick_result.reached_entrance_wait ||
        out_receipt->tick_receipt.tick_result.credits_finished) {
        out_receipt->host_receipt.input_result =
            CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34;
        out_receipt->host_receipt.status_scope = "BOOT";
        out_receipt->host_receipt.status = "CSB ENTRANCE";
    }
    if (!out_receipt->tick_receipt.tick_result.door_opening_finished) {
        return 1;
    }
    pending_command = out_receipt->tick_receipt.state.pending_command;
    if (!csb_v1_startup_finish_door_opening_from_facts_with_receipt_pc34(
            out_receipt->tick_receipt.state.title_active,
            out_receipt->tick_receipt.state.title_frame,
            out_receipt->tick_receipt.state.title_source_step,
            out_receipt->tick_receipt.state.entrance_active,
            out_receipt->tick_receipt.state.entrance_source_step,
            out_receipt->tick_receipt.state.entrance_dismissed,
            out_receipt->tick_receipt.state.credits_active,
            out_receipt->tick_receipt.state.credits_remaining_ticks,
            out_receipt->tick_receipt.state.opening_active,
            out_receipt->tick_receipt.state.opening_delay_ticks,
            out_receipt->tick_receipt.state.opening_step,
            out_receipt->tick_receipt.state.pending_command,
            &out_receipt->finish_receipt)) {
        return 1;
    }
    out_receipt->finish_receipt_valid = 1;
    out_receipt->host_receipt.input_result =
        CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34;
    out_receipt->host_receipt.status_scope = "BOOT";
    if (pending_command ==
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_BONUS_DUNGEON_PC34) {
        out_receipt->host_receipt.status = "CSB BONUS";
    } else if (pending_command ==
               CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34) {
        out_receipt->host_receipt.status = "CSB RESUMED";
    } else {
        out_receipt->host_receipt.status = "CSB READY";
    }
    return 1;
}

static int csb_v1_startup_render_state_from_command_state_pc34(
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
    out_state->resume_available = 0;
    out_state->runtime_start_valid = runtime_start_valid ? 1 : 0;
    out_state->runtime_start_x = runtime_start_x;
    out_state->runtime_start_y = runtime_start_y;
    out_state->runtime_start_dir = runtime_start_dir;
    return 1;
}

static int csb_v1_startup_build_render_plan_from_state_pc34(
    const CSB_V1_StartupRenderState_PC34 *state,
    CSB_V1_StartupRenderPlan_PC34 *out_plan);

int csb_v1_startup_source_render_plan_from_state_pc34(
    const CSB_V1_StartupRenderState_PC34 *state,
    CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    return csb_v1_startup_build_render_plan_from_state_pc34(state, out_plan);
}

static int csb_v1_startup_build_render_plan_from_request_pc34(
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
        return csb_v1_startup_build_render_plan_from_state_pc34(NULL,
                                                                 out_plan);
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
    render_state.resume_available = request->resume_available ? 1 : 0;
    return csb_v1_startup_build_render_plan_from_state_pc34(&render_state,
                                                             out_plan);
}

static int csb_v1_startup_build_render_plan_from_host_facts_struct_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts,
    CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    CSB_V1_StartupRenderPlanRequest_PC34 request;

    if (!facts) {
        return csb_v1_startup_build_render_plan_from_request_pc34(NULL,
                                                                  out_plan);
    }
    memset(&request, 0, sizeof(request));
    request.title_active = facts->title_active;
    request.title_frame = facts->title_frame;
    request.title_source_step = facts->title_source_step;
    request.entrance_active = facts->entrance_active;
    request.entrance_source_step = facts->entrance_source_step;
    request.entrance_dismissed = facts->entrance_dismissed;
    request.credits_active = facts->credits_active;
    request.credits_remaining_ticks = facts->credits_remaining_ticks;
    request.opening_active = facts->opening_active;
    request.opening_delay_ticks = facts->opening_delay_ticks;
    request.opening_step = facts->opening_step;
    request.pending_command = facts->pending_command;
    request.entrance_frame = facts->entrance_frame;
    request.utility_overlay_active = facts->utility_overlay_active;
    request.resume_available = facts->resume_available;
    if (facts->boot_profile) {
        const CSB_V1_BootProfile *profile =
            (const CSB_V1_BootProfile *)facts->boot_profile;
        request.runtime_start_valid = 1;
        request.runtime_start_x = profile->runtime.party_x;
        request.runtime_start_y = profile->runtime.party_y;
        request.runtime_start_dir = profile->runtime.party_dir;
    }
    return csb_v1_startup_build_render_plan_from_request_pc34(&request,
                                                              out_plan);
}

static int csb_v1_startup_command_state_from_request_pc34(
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

int csb_v1_startup_command_state_from_facts_pc34(
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    CSB_V1_StartupCommandState_PC34 *out_state)
{
    CSB_V1_StartupCommandStateRequest_PC34 request;

    memset(&request, 0, sizeof(request));
    request.title_active = title_active;
    request.title_frame = title_frame;
    request.title_source_step = title_source_step;
    request.entrance_active = entrance_active;
    request.entrance_source_step = entrance_source_step;
    request.entrance_dismissed = entrance_dismissed;
    request.credits_active = credits_active;
    request.credits_remaining_ticks = credits_remaining_ticks;
    request.opening_active = opening_active;
    request.opening_delay_ticks = opening_delay_ticks;
    request.opening_step = opening_step;
    request.pending_command = pending_command;
    return csb_v1_startup_command_state_from_request_pc34(&request,
                                                          out_state);
}

void csb_v1_startup_command_state_receipt_init_pc34(
    CSB_V1_StartupCommandStateReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int csb_v1_startup_command_state_receipt_from_state_pc34(
    const CSB_V1_StartupCommandState_PC34 *state,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_receipt)
{
    if (!state || !out_receipt) {
        return 0;
    }
    csb_v1_startup_command_state_receipt_init_pc34(out_receipt);
    out_receipt->title_active = state->title_active;
    out_receipt->title_frame = state->title_frame;
    out_receipt->title_source_step = state->title_source_step;
    out_receipt->entrance_active = state->entrance_active;
    out_receipt->entrance_source_step = state->entrance_source_step;
    out_receipt->entrance_dismissed = state->entrance_dismissed;
    out_receipt->credits_active = state->credits_active;
    out_receipt->credits_remaining_ticks = state->credits_remaining_ticks;
    out_receipt->opening_active = state->opening_active;
    out_receipt->opening_delay_ticks = state->opening_delay_ticks;
    out_receipt->opening_step = state->opening_step;
    out_receipt->pending_command = state->pending_command;
    return 1;
}

int csb_v1_startup_command_state_receipt_from_facts_pc34(
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupCommandState_PC34 state;

    if (!csb_v1_startup_command_state_from_facts_pc34(
            title_active,
            title_frame,
            title_source_step,
            entrance_active,
            entrance_source_step,
            entrance_dismissed,
            credits_active,
            credits_remaining_ticks,
            opening_active,
            opening_delay_ticks,
            opening_step,
            pending_command,
            &state)) {
        if (out_receipt) {
            csb_v1_startup_command_state_receipt_init_pc34(out_receipt);
        }
        return 0;
    }
    return csb_v1_startup_command_state_receipt_from_state_pc34(
        &state,
        out_receipt);
}

int csb_v1_startup_init_command_state_receipt_pc34(
    int skip_startup,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupCommandState_PC34 state;

    if (out_receipt) {
        csb_v1_startup_command_state_receipt_init_pc34(out_receipt);
    }
    if (!csb_v1_startup_init_command_state_pc34(&state, skip_startup)) {
        return 0;
    }
    return csb_v1_startup_command_state_receipt_from_state_pc34(
        &state,
        out_receipt);
}

void csb_v1_startup_init_state_receipt_init_pc34(
    CSB_V1_StartupInitStateReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int csb_v1_startup_init_state_receipt_pc34(
    int skip_startup,
    CSB_V1_StartupInitStateReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_startup_init_state_receipt_init_pc34(out_receipt);
    if (!csb_v1_startup_init_command_state_receipt_pc34(
            skip_startup,
            &out_receipt->command_state)) {
        return 0;
    }
    out_receipt->entrance_frame = 0;
    out_receipt->entrance_last_command = 0;
    out_receipt->entrance_bonus_requested = 0;
    return 1;
}

int csb_v1_startup_entrance_accepts_input_from_facts_pc34(
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command)
{
    CSB_V1_StartupCommandStateRequest_PC34 request;

    memset(&request, 0, sizeof(request));
    request.title_active = title_active;
    request.title_frame = title_frame;
    request.title_source_step = title_source_step;
    request.entrance_active = entrance_active;
    request.entrance_source_step = entrance_source_step;
    request.entrance_dismissed = entrance_dismissed;
    request.credits_active = credits_active;
    request.credits_remaining_ticks = credits_remaining_ticks;
    request.opening_active = opening_active;
    request.opening_delay_ticks = opening_delay_ticks;
    request.opening_step = opening_step;
    request.pending_command = pending_command;
    return csb_v1_startup_entrance_accepts_input_from_request_pc34(
        &request);
}

static int csb_v1_startup_entrance_accepts_input_from_request_pc34(
    const CSB_V1_StartupCommandStateRequest_PC34 *request)
{
    CSB_V1_StartupCommandState_PC34 state;

    if (!csb_v1_startup_command_state_from_request_pc34(request, &state)) {
        return 0;
    }
    return csb_v1_startup_entrance_accepts_input_pc34(&state);
}

int csb_v1_startup_entrance_accepts_input_from_host_facts_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts)
{
    if (!facts) {
        return 0;
    }
    return csb_v1_startup_entrance_accepts_input_from_facts_pc34(
        facts->title_active,
        facts->title_frame,
        facts->title_source_step,
        facts->entrance_active,
        facts->entrance_source_step,
        facts->entrance_dismissed,
        facts->credits_active,
        facts->credits_remaining_ticks,
        facts->opening_active,
        facts->opening_delay_ticks,
        facts->opening_step,
        facts->pending_command);
}

/* Receipt construction is the public startup presentation boundary. Keep the
 * state-to-plan adapter private so hosts cannot bypass verified host facts.
 * ReDMCSB: TITLE.C F0437; ENTRANCE.C F0441/F0806. */
static int csb_v1_startup_build_render_plan_from_state_pc34(
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
    csb_v1_startup_clear_menu_options_pc34(&plan);
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
        /* ENTRANCE.C F0442 expands C005 directly to the logical screen.
         * C005 is required for this route; never replace it with generated
         * text when the source surface cannot be presented. */
        csb_v1_startup_rebuild_asset_commands_pc34(&plan);
        csb_v1_startup_rebuild_primitive_commands_pc34(&plan);
        csb_v1_startup_rebuild_render_commands_pc34(&plan);
        *out_plan = plan;
        return 1;
    }
    if (!plan.waiting_for_input && state->entrance_source_step > 0) {
        /* ReDMCSB ENTRANCE.C F0441 lines 426-443 fades/curtains to black
         * before C004/C002/C003 are redrawn. */
        plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34;
        plan.special_palette = VGA_PALETTE_PC34_SPECIAL_CSB_ENTRANCE;
        csb_v1_startup_rebuild_asset_commands_pc34(&plan);
        csb_v1_startup_rebuild_primitive_commands_pc34(&plan);
        csb_v1_startup_rebuild_render_commands_pc34(&plan);
        *out_plan = plan;
        return 1;
    }
    if (state->opening_active) {
        if (state->opening_delay_ticks > 0) {
            if (state->opening_step != 0) {
                *out_plan = plan;
                return 0;
            }
        } else if (state->opening_step < 1 ||
                   state->opening_step > CSB_V1_ENTRANCE_DOOR_STEP_COUNT_PC34) {
            *out_plan = plan;
            return 0;
        }
        plan.surface = state->opening_delay_ticks > 0
            ? CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34
            : CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34;
        plan.opening_step = state->opening_step;
        plan.source_asset_id = CSB_V1_GRAPHIC_ENTRANCE_SCREEN_PC34;
        csb_v1_startup_set_full_surface_blit_pc34(&plan);
        plan.special_palette = VGA_PALETTE_PC34_SPECIAL_CSB_ENTRANCE;
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
    plan.special_palette = VGA_PALETTE_PC34_SPECIAL_CSB_ENTRANCE;
    csb_v1_startup_set_closed_door_rects_pc34(&plan);
    csb_v1_startup_set_entrance_menu_options_pc34(
        &plan,
        state->resume_available);
    /* ENTRANCE.C F0439/F0441 presents C004 with C002/C003 and its original
     * input tables.  No Firestaff text, frame, pose, or prompt may be drawn
     * over that real-data composition. */
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

int csb_v1_startup_apply_entrance_command_with_receipt_pc34(
    CSB_V1_StartupCommandState_PC34 *state,
    int command_id,
    CSB_V1_StartupEntranceCommandReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupEntranceCommandPlan_PC34 plan;
    CSB_V1_StartupRuntimePlan_PC34 runtime_plan;
    CSB_V1_StartupEntranceApplyResult_PC34 pure_result;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->command_id = command_id;
        out_receipt->pure_apply_result =
            CSB_V1_STARTUP_ENTRANCE_APPLY_NOT_HANDLED_PC34;
        csb_v1_startup_runtime_plan_init_pc34(&out_receipt->runtime_plan);
    }
    if (!state || !out_receipt) {
        return 0;
    }
    if (!csb_v1_startup_plan_for_entrance_command_pc34(
            state,
            command_id,
            &plan)) {
        return 0;
    }

    out_receipt->handled = 1;
    pure_result = csb_v1_startup_apply_pure_entrance_plan_pc34(
        state,
        &plan,
        &out_receipt->outcome);
    out_receipt->pure_apply_result = pure_result;
    if (pure_result !=
        CSB_V1_STARTUP_ENTRANCE_APPLY_NOT_HANDLED_PC34) {
        return 1;
    }

    if (!csb_v1_startup_runtime_plan_for_entrance_plan_pc34(
            &plan,
            &runtime_plan)) {
        out_receipt->handled = 0;
        return 0;
    }
    out_receipt->requires_runtime_plan = 1;
    out_receipt->runtime_plan = runtime_plan;
    return 1;
}

int csb_v1_startup_apply_entrance_command_from_facts_with_receipts_pc34(
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int command_id,
    CSB_V1_StartupEntranceCommandReceipt_PC34 *out_command_receipt,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_state_receipt)
{
    CSB_V1_StartupCommandState_PC34 state;

    if (out_state_receipt) {
        csb_v1_startup_command_state_receipt_init_pc34(out_state_receipt);
    }
    if (!csb_v1_startup_command_state_from_facts_pc34(
            title_active,
            title_frame,
            title_source_step,
            entrance_active,
            entrance_source_step,
            entrance_dismissed,
            credits_active,
            credits_remaining_ticks,
            opening_active,
            opening_delay_ticks,
            opening_step,
            pending_command,
            &state)) {
        return 0;
    }
    if (!csb_v1_startup_apply_entrance_command_with_receipt_pc34(
            &state,
            command_id,
            out_command_receipt)) {
        return 0;
    }
    return csb_v1_startup_command_state_receipt_from_state_pc34(
        &state,
        out_state_receipt);
}

int csb_v1_startup_apply_entrance_command_from_host_facts_with_receipts_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts,
    int command_id,
    CSB_V1_StartupEntranceCommandReceipt_PC34 *out_command_receipt,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_state_receipt)
{
    if (!facts) {
        if (out_state_receipt) {
            csb_v1_startup_command_state_receipt_init_pc34(
                out_state_receipt);
        }
        return 0;
    }
    return csb_v1_startup_apply_entrance_command_from_facts_with_receipts_pc34(
        facts->title_active,
        facts->title_frame,
        facts->title_source_step,
        facts->entrance_active,
        facts->entrance_source_step,
        facts->entrance_dismissed,
        facts->credits_active,
        facts->credits_remaining_ticks,
        facts->opening_active,
        facts->opening_delay_ticks,
        facts->opening_step,
        facts->pending_command,
        command_id,
        out_command_receipt,
        out_state_receipt);
}

CSB_V1_StartupEntranceInputResult_PC34
csb_v1_startup_input_result_for_entrance_apply_pc34(
    CSB_V1_StartupEntranceApplyResult_PC34 apply_result)
{
    switch (apply_result) {
        case CSB_V1_STARTUP_ENTRANCE_APPLY_REDRAW_PC34:
            return CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34;
        case CSB_V1_STARTUP_ENTRANCE_APPLY_RETURN_TO_LAUNCHER_PC34:
            return CSB_V1_STARTUP_ENTRANCE_INPUT_RETURN_TO_LAUNCHER_PC34;
        case CSB_V1_STARTUP_ENTRANCE_APPLY_IGNORED_PC34:
        case CSB_V1_STARTUP_ENTRANCE_APPLY_NOT_HANDLED_PC34:
        default:
            return CSB_V1_STARTUP_ENTRANCE_INPUT_IGNORE_PC34;
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

void csb_v1_startup_host_receipt_init_pc34(
    CSB_V1_StartupHostReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->input_result = CSB_V1_STARTUP_ENTRANCE_INPUT_IGNORE_PC34;
}

void csb_v1_startup_entrance_host_action_receipt_init_pc34(
    CSB_V1_StartupEntranceHostActionReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->command_receipt.pure_apply_result =
        CSB_V1_STARTUP_ENTRANCE_APPLY_NOT_HANDLED_PC34;
    csb_v1_startup_command_state_receipt_init_pc34(
        &receipt->state_receipt);
    csb_v1_startup_host_receipt_init_pc34(&receipt->host_receipt);
}

int csb_v1_startup_host_receipt_from_pure_entrance_pc34(
    const CSB_V1_StartupEntranceCommandReceipt_PC34 *command_receipt,
    CSB_V1_StartupHostReceipt_PC34 *out_receipt)
{
    if (!command_receipt || !out_receipt ||
        command_receipt->pure_apply_result ==
            CSB_V1_STARTUP_ENTRANCE_APPLY_NOT_HANDLED_PC34) {
        return 0;
    }
    csb_v1_startup_host_receipt_init_pc34(out_receipt);
    out_receipt->input_result =
        csb_v1_startup_input_result_for_entrance_apply_pc34(
            command_receipt->pure_apply_result);
    out_receipt->status_scope = command_receipt->outcome.status_scope;
    out_receipt->status = command_receipt->outcome.status;
    return 1;
}

int csb_v1_startup_execute_entrance_command_from_host_facts_with_receipts_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts,
    int command_id,
    CSB_V1_StartupEntranceHostActionReceipt_PC34 *out_receipt)
{
    if (out_receipt) {
        csb_v1_startup_entrance_host_action_receipt_init_pc34(
            out_receipt);
    }
    if (!facts || !out_receipt) {
        return 0;
    }
    if (!csb_v1_startup_apply_entrance_command_from_host_facts_with_receipts_pc34(
            facts,
            command_id,
            &out_receipt->command_receipt,
            &out_receipt->state_receipt) ||
        !out_receipt->command_receipt.handled) {
        return 1;
    }

    out_receipt->handled = 1;
    if (out_receipt->command_receipt.pure_apply_result !=
        CSB_V1_STARTUP_ENTRANCE_APPLY_NOT_HANDLED_PC34) {
        return csb_v1_startup_host_receipt_from_pure_entrance_pc34(
            &out_receipt->command_receipt,
            &out_receipt->host_receipt);
    }
    return 1;
}

int csb_v1_startup_host_receipt_from_runtime_apply_pc34(
    const CSB_V1_StartupRuntimeApplyReceipt_PC34 *runtime_receipt,
    const CSB_V1_StartupEntranceInputOutcome_PC34 *outcome,
    CSB_V1_StartupHostReceipt_PC34 *out_receipt)
{
    if (!runtime_receipt || !out_receipt ||
        runtime_receipt->result ==
            CSB_V1_STARTUP_RUNTIME_APPLY_NOT_HANDLED_PC34) {
        return 0;
    }
    csb_v1_startup_host_receipt_init_pc34(out_receipt);
    out_receipt->status_scope = outcome ? outcome->status_scope : NULL;
    out_receipt->status = outcome ? outcome->status : NULL;
    out_receipt->clear_import_preview =
        runtime_receipt->clear_import_preview ? 1 : 0;
    out_receipt->bonus_requested_changed =
        runtime_receipt->bonus_requested_changed ? 1 : 0;
    out_receipt->bonus_requested = runtime_receipt->bonus_requested ? 1 : 0;
    switch (runtime_receipt->result) {
        case CSB_V1_STARTUP_RUNTIME_APPLY_REDRAW_PC34:
            out_receipt->input_result =
                CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34;
            break;
        case CSB_V1_STARTUP_RUNTIME_APPLY_IGNORED_PC34:
        case CSB_V1_STARTUP_RUNTIME_APPLY_NOT_HANDLED_PC34:
        default:
            out_receipt->input_result =
                CSB_V1_STARTUP_ENTRANCE_INPUT_IGNORE_PC34;
            break;
    }
    return 1;
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

int csb_v1_startup_apply_runtime_plan_from_facts_with_receipts_pc34(
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    const CSB_V1_StartupRuntimePlan_PC34 *runtime_plan,
    int resume_available,
    int resume_loaded,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome,
    CSB_V1_StartupRuntimeApplyReceipt_PC34 *out_runtime_receipt,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_state_receipt)
{
    CSB_V1_StartupCommandState_PC34 state;

    if (out_state_receipt) {
        csb_v1_startup_command_state_receipt_init_pc34(out_state_receipt);
    }
    if (!csb_v1_startup_command_state_from_facts_pc34(
            title_active,
            title_frame,
            title_source_step,
            entrance_active,
            entrance_source_step,
            entrance_dismissed,
            credits_active,
            credits_remaining_ticks,
            opening_active,
            opening_delay_ticks,
            opening_step,
            pending_command,
            &state)) {
        return 0;
    }
    if (!csb_v1_startup_apply_runtime_plan_with_receipt_pc34(
            &state,
            runtime_plan,
            resume_available,
            resume_loaded,
            out_outcome,
            out_runtime_receipt)) {
        return 0;
    }
    return csb_v1_startup_command_state_receipt_from_state_pc34(
        &state,
        out_state_receipt);
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
    state->opening_step = 0;
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

int csb_v1_startup_finish_door_opening_from_facts_with_receipt_pc34(
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupCommandState_PC34 state;

    if (out_receipt) {
        csb_v1_startup_command_state_receipt_init_pc34(out_receipt);
    }
    if (!csb_v1_startup_command_state_from_facts_pc34(
            title_active,
            title_frame,
            title_source_step,
            entrance_active,
            entrance_source_step,
            entrance_dismissed,
            credits_active,
            credits_remaining_ticks,
            opening_active,
            opening_delay_ticks,
            opening_step,
            pending_command,
            &state)) {
        return 0;
    }
    if (!csb_v1_startup_finish_door_opening_pc34(&state)) {
        return 0;
    }
    return csb_v1_startup_command_state_receipt_from_state_pc34(
        &state,
        out_receipt);
}

int csb_v1_startup_finish_door_opening_from_host_facts_with_receipt_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_receipt)
{
    if (!facts) {
        if (out_receipt) {
            csb_v1_startup_command_state_receipt_init_pc34(out_receipt);
        }
        return 0;
    }
    return csb_v1_startup_finish_door_opening_from_facts_with_receipt_pc34(
        facts->title_active,
        facts->title_frame,
        facts->title_source_step,
        facts->entrance_active,
        facts->entrance_source_step,
        facts->entrance_dismissed,
        facts->credits_active,
        facts->credits_remaining_ticks,
        facts->opening_active,
        facts->opening_delay_ticks,
        facts->opening_step,
        facts->pending_command,
        out_receipt);
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

int csb_v1_startup_receipt_phase_from_facts_pc34(
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame)
{
    CSB_V1_StartupCommandState_PC34 state;

    if (!csb_v1_startup_command_state_from_facts_pc34(
            title_active,
            title_frame,
            title_source_step,
            entrance_active,
            entrance_source_step,
            entrance_dismissed,
            credits_active,
            credits_remaining_ticks,
            opening_active,
            opening_delay_ticks,
            opening_step,
            pending_command,
            &state)) {
        return 0;
    }
    return csb_v1_startup_receipt_phase_pc34(
        &state,
        entrance_frame,
        out_phase,
        out_phase_size,
        out_startup_active,
        out_startup_frame);
}

int csb_v1_startup_receipt_presentation_from_facts_pc34(
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    int title_max;

    if (!csb_v1_startup_receipt_phase_from_facts_pc34(
            title_active,
            title_frame,
            title_source_step,
            entrance_active,
            entrance_source_step,
            entrance_dismissed,
            credits_active,
            credits_remaining_ticks,
            opening_active,
            opening_delay_ticks,
            opening_step,
            pending_command,
            entrance_frame,
            out_phase,
            out_phase_size,
            out_startup_active,
            out_startup_frame)) {
        return 0;
    }

    title_max = csb_v1_startup_title_total_ticks_pc34();
    if (out_title_frame_max) {
        *out_title_frame_max = title_max;
    }
    if (out_animation_active) {
        *out_animation_active = 0;
    }
    if (out_title_frame) {
        *out_title_frame = title_max;
    }
    if (out_title_ready) {
        *out_title_ready = 1;
    }
    if (!out_animation || out_animation_size <= 0) {
        return 1;
    }

    if (title_active) {
        snprintf(out_animation, (size_t)out_animation_size, "%s",
                 "csb-title");
        if (out_animation_active) {
            *out_animation_active = 1;
        }
        if (out_title_frame) {
            *out_title_frame = title_frame;
        }
        if (out_title_ready) {
            *out_title_ready = 0;
        }
    } else if (opening_active) {
        snprintf(out_animation, (size_t)out_animation_size, "%s",
                 "csb-entrance-opening");
        if (out_animation_active) {
            *out_animation_active = 1;
        }
    } else if (entrance_active) {
        snprintf(out_animation, (size_t)out_animation_size, "%s",
                 "csb-entrance");
    } else {
        snprintf(out_animation, (size_t)out_animation_size, "%s",
                 "csb-runtime");
    }
    return 1;
}

int csb_v1_startup_receipt_presentation_from_host_facts_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    int title_max;
    if (!facts) {
        title_max = csb_v1_startup_title_total_ticks_pc34();
        if (out_phase && out_phase_size > 0) {
            out_phase[0] = '\0';
        }
        if (out_animation && out_animation_size > 0) {
            out_animation[0] = '\0';
        }
        if (out_startup_active) {
            *out_startup_active = 0;
        }
        if (out_startup_frame) {
            *out_startup_frame = 0;
        }
        if (out_animation_active) {
            *out_animation_active = 0;
        }
        if (out_title_frame) {
            *out_title_frame = title_max;
        }
        if (out_title_frame_max) {
            *out_title_frame_max = title_max;
        }
        if (out_title_ready) {
            *out_title_ready = 1;
        }
        return 0;
    }
    return csb_v1_startup_receipt_presentation_from_facts_pc34(
        facts->title_active,
        facts->title_frame,
        facts->title_source_step,
        facts->entrance_active,
        facts->entrance_source_step,
        facts->entrance_dismissed,
        facts->credits_active,
        facts->credits_remaining_ticks,
        facts->opening_active,
        facts->opening_delay_ticks,
        facts->opening_step,
        facts->pending_command,
        facts->entrance_frame,
        out_phase,
        out_phase_size,
        out_startup_active,
        out_startup_frame,
        out_animation,
        out_animation_size,
        out_animation_active,
        out_title_frame,
        out_title_frame_max,
        out_title_ready);
}

void csb_v1_startup_presentation_receipt_init_pc34(
    CSB_V1_StartupPresentationReceipt_PC34 *receipt)
{
    int title_max;
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    title_max = csb_v1_startup_title_total_ticks_pc34();
    receipt->title_frame = title_max;
    receipt->title_frame_max = title_max;
    receipt->title_ready = 1;
    receipt->redmcsb_source_locked = 1;
    receipt->redmcsb_title_source =
        "ReDMCSB TITLE.C F0437 lines 424-463";
    receipt->redmcsb_entrance_source =
        "ReDMCSB ENTRANCE.C F0441/F0806 lines 409-447,850-883";
    receipt->redmcsb_title_graphic_id = CSB_V1_GRAPHIC_TITLE_PC34;
    receipt->redmcsb_presents_ticks =
        CSB_V1_TITLE_PRESENTS_TICKS_PC34;
    receipt->redmcsb_chaos_zoom_ticks =
        CSB_V1_TITLE_CHAOS_ZOOM_TICKS_PC34;
    receipt->redmcsb_chaos_hold_ticks =
        CSB_V1_TITLE_CHAOS_HOLD_TICKS_PC34;
    receipt->redmcsb_strikes_back_ticks =
        CSB_V1_TITLE_STRIKES_BACK_TICKS_PC34;
    receipt->redmcsb_entrance_screen_graphic_id =
        CSB_V1_GRAPHIC_ENTRANCE_SCREEN_PC34;
    receipt->redmcsb_credits_graphic_id =
        CSB_V1_GRAPHIC_ENTRANCE_CREDITS_PC34;
    receipt->redmcsb_left_door_graphic_id =
        CSB_V1_GRAPHIC_ENTRANCE_LEFT_DOOR_PC34;
    receipt->redmcsb_right_door_graphic_id =
        CSB_V1_GRAPHIC_ENTRANCE_RIGHT_DOOR_PC34;
    receipt->redmcsb_pre_open_delay_ticks =
        CSB_V1_ENTRANCE_PRE_OPEN_DELAY_TICKS_PC34;
    receipt->redmcsb_door_step_count =
        CSB_V1_ENTRANCE_DOOR_STEP_COUNT_PC34;
    receipt->redmcsb_closed_door_left_x = 0;
    receipt->redmcsb_closed_door_right_x = 128;
    receipt->redmcsb_closed_door_y = CSB_V1_ENTRANCE_DOOR_SCREEN_Y_PC34;
    receipt->redmcsb_closed_door_w = 128;
    receipt->redmcsb_closed_door_h = 161;
}

static int csb_v1_startup_presentation_facts_are_source_coherent_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts)
{
    int title_active;
    int entrance_active;
    int credits_active;
    int opening_active;

    if (!facts || facts->entrance_frame < 0) {
        return 0;
    }
    title_active = facts->title_active ? 1 : 0;
    entrance_active = facts->entrance_active ? 1 : 0;
    credits_active = facts->credits_active ? 1 : 0;
    opening_active = facts->opening_active ? 1 : 0;

    /* ReDMCSB TITLE.C F0437 runs within the ENTRANCE.C F0806 session. The
     * title step is therefore a source-derived fact, while credits and the
     * door animation are mutually exclusive later entrance states. */
    if (title_active) {
        return entrance_active && !credits_active && !opening_active &&
               facts->title_frame >= 0 &&
               facts->title_frame < csb_v1_startup_title_total_ticks_pc34() &&
               facts->title_source_step ==
                   (int)csb_v1_startup_title_source_step_for_frame_pc34(
                       facts->title_frame);
    }
    if (credits_active) {
        return entrance_active && !opening_active &&
               facts->credits_remaining_ticks > 0;
    }
    if (opening_active) {
        return entrance_active && !facts->entrance_dismissed &&
               facts->opening_delay_ticks >= 0 &&
               (facts->opening_delay_ticks > 0
                    ? facts->opening_step == 0
                    : facts->opening_step >= 1 &&
                          facts->opening_step <=
                              CSB_V1_ENTRANCE_DOOR_STEP_COUNT_PC34);
    }
    return !facts->entrance_dismissed || !entrance_active;
}

int csb_v1_startup_presentation_receipt_from_host_facts_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts,
    CSB_V1_StartupPresentationReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_startup_presentation_receipt_init_pc34(out_receipt);
    if (!facts) {
        return 0;
    }
    if (!csb_v1_startup_presentation_facts_are_source_coherent_pc34(facts)) {
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0441/F0806 own the CSB
     * startup presentation order.  Keep phase, animation, render plan, and
     * entrance menu/input state in one CSB receipt so host HUD/menu code
     * does not infer source-stage semantics itself. */
    if (!csb_v1_startup_receipt_presentation_from_host_facts_pc34(
            facts,
            out_receipt->phase,
            (int)sizeof(out_receipt->phase),
            &out_receipt->startup_active,
            &out_receipt->startup_frame,
            out_receipt->animation,
            (int)sizeof(out_receipt->animation),
            &out_receipt->animation_active,
            &out_receipt->title_frame,
            &out_receipt->title_frame_max,
            &out_receipt->title_ready)) {
        csb_v1_startup_presentation_receipt_init_pc34(out_receipt);
        return 0;
    }
    if (!csb_v1_startup_build_render_plan_from_host_facts_struct_pc34(
            facts,
            &out_receipt->render_plan)) {
        csb_v1_startup_presentation_receipt_init_pc34(out_receipt);
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->accepts_input =
        csb_v1_startup_entrance_accepts_input_from_host_facts_pc34(facts);
    out_receipt->waiting_for_input =
        out_receipt->render_plan.waiting_for_input;
    out_receipt->menu_option_count =
        out_receipt->render_plan.menu_option_count;
    out_receipt->render_command_count =
        out_receipt->render_plan.render_command_count;
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
