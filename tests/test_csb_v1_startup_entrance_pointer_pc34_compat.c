#include "firestaff/csb/v1/startup_entrance_pointer_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_passed;
static int g_failed;

typedef struct AssetExecutorProbe {
    int call_count;
    CSB_V1_StartupAssetCommandKind_PC34 fail_kind;
    int seen_kind[CSB_V1_STARTUP_ASSET_COMMAND_CAP_PC34];
    int seen_asset_id[CSB_V1_STARTUP_ASSET_COMMAND_CAP_PC34];
} AssetExecutorProbe;

typedef struct OpeningCompositeProbe {
    int call_count;
    int fail;
    CSB_V1_StartupOpeningComposite_PC34 seen;
} OpeningCompositeProbe;

typedef struct RenderExecutorProbe {
    int draw_title_count;
    int clear_black_count;
    int draw_full_surface_count;
    int draw_full_surface_result;
    int draw_opening_frame_count;
    int draw_opening_frame_result;
    int draw_closed_doors_count;
    int draw_door_fallback_count;
    int draw_fallback_text_count;
    int draw_utility_panel_count;
} RenderExecutorProbe;

unsigned int ENTRANCE_Compat_GetMouseRouteCount(void)
{
    return 5u;
}

int ENTRANCE_Compat_GetMouseRoute(unsigned int ordinal,
                                  EntranceMouseRouteCompat *outRoute)
{
    (void)ordinal;
    if (outRoute) {
        memset(outRoute, 0, sizeof(*outRoute));
    }
    return 0;
}

int ENTRANCE_Compat_HitTestMouseRoute(int screenX,
                                      int screenY,
                                      unsigned int buttonMask,
                                      EntranceMouseRouteCompat *outRoute)
{
    if (!outRoute) {
        return 0;
    }
    memset(outRoute, 0, sizeof(*outRoute));
    if (screenX == 244 && screenY == 45 &&
        buttonMask == ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) {
        outRoute->commandId = 200u;
        return 1;
    }
    if (screenX == 244 && screenY == 45 &&
        buttonMask == ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT) {
        outRoute->commandId = 201u;
        return 1;
    }
    if (screenX == 244 && screenY == 76 &&
        buttonMask == ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) {
        outRoute->commandId = 202u;
        return 1;
    }
    if (screenX == 248 && screenY == 186 &&
        buttonMask == ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) {
        outRoute->commandId = 203u;
        return 1;
    }
    if (screenX == 243 && screenY == 110 &&
        buttonMask == ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) {
        outRoute->commandId = 216u;
        return 1;
    }
    return 0;
}

const char *ENTRANCE_Compat_GetMouseRouteEvidence(void)
{
    return "test stub";
}

static void check(int condition, const char *message)
{
    if (condition) {
        ++g_passed;
        printf("PASS %s\n", message);
    } else {
        ++g_failed;
        printf("FAIL %s\n", message);
    }
}

static int asset_executor_probe(void *user,
                                const CSB_V1_StartupAssetCommand_PC34 *cmd)
{
    AssetExecutorProbe *probe = (AssetExecutorProbe *)user;
    if (!probe || !cmd) {
        return 0;
    }
    if (probe->call_count < CSB_V1_STARTUP_ASSET_COMMAND_CAP_PC34) {
        probe->seen_kind[probe->call_count] = cmd->kind;
        probe->seen_asset_id[probe->call_count] = cmd->asset_id;
    }
    ++probe->call_count;
    return cmd->kind != probe->fail_kind;
}

static int opening_composite_probe(
    void *user,
    const CSB_V1_StartupOpeningComposite_PC34 *composite)
{
    OpeningCompositeProbe *probe = (OpeningCompositeProbe *)user;
    if (!probe || !composite) {
        return 0;
    }
    probe->seen = *composite;
    ++probe->call_count;
    return !probe->fail;
}

static int render_probe_draw_title(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    RenderExecutorProbe *probe = (RenderExecutorProbe *)user;
    (void)plan;
    if (!probe) {
        return 0;
    }
    ++probe->draw_title_count;
    return 1;
}

static void render_probe_clear_black(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    RenderExecutorProbe *probe = (RenderExecutorProbe *)user;
    (void)plan;
    if (probe) {
        ++probe->clear_black_count;
    }
}

static int render_probe_draw_full_surface(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    RenderExecutorProbe *probe = (RenderExecutorProbe *)user;
    (void)plan;
    if (!probe) {
        return 0;
    }
    ++probe->draw_full_surface_count;
    return probe->draw_full_surface_result;
}

static int render_probe_draw_opening_frame(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    RenderExecutorProbe *probe = (RenderExecutorProbe *)user;
    (void)plan;
    if (!probe) {
        return 0;
    }
    ++probe->draw_opening_frame_count;
    return probe->draw_opening_frame_result;
}

static void render_probe_draw_closed_doors(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    RenderExecutorProbe *probe = (RenderExecutorProbe *)user;
    (void)plan;
    if (probe) {
        ++probe->draw_closed_doors_count;
    }
}

static void render_probe_draw_door_fallback(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    RenderExecutorProbe *probe = (RenderExecutorProbe *)user;
    (void)plan;
    if (probe) {
        ++probe->draw_door_fallback_count;
    }
}

static void render_probe_draw_fallback_text(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    RenderExecutorProbe *probe = (RenderExecutorProbe *)user;
    (void)plan;
    if (probe) {
        ++probe->draw_fallback_text_count;
    }
}

static void render_probe_draw_utility_panel(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    RenderExecutorProbe *probe = (RenderExecutorProbe *)user;
    (void)plan;
    if (probe) {
        ++probe->draw_utility_panel_count;
    }
}

static void render_probe_executor_init(
    CSB_V1_StartupRenderExecutor_PC34 *executor,
    RenderExecutorProbe *probe)
{
    memset(executor, 0, sizeof(*executor));
    executor->user = probe;
    executor->draw_title = render_probe_draw_title;
    executor->clear_black = render_probe_clear_black;
    executor->draw_full_surface = render_probe_draw_full_surface;
    executor->draw_opening_frame = render_probe_draw_opening_frame;
    executor->draw_closed_doors = render_probe_draw_closed_doors;
    executor->draw_door_fallback = render_probe_draw_door_fallback;
    executor->draw_fallback_text = render_probe_draw_fallback_text;
    executor->draw_utility_panel = render_probe_draw_utility_panel;
}

static void expect_action(const char *message,
                          int x,
                          int y,
                          unsigned int mask,
                          CSB_V1_StartupEntrancePointerAction_PC34 expected,
                          int expectedCommand)
{
    CSB_V1_StartupEntrancePointerAction_PC34 action =
        CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34;
    int hit = csb_v1_startup_entrance_pointer_action_pc34(
        0, x, y, mask, &action);
    check(hit && action == expected &&
              csb_v1_startup_entrance_command_for_pointer_action_pc34(
                  action) == expectedCommand,
          message);
}

int main(void)
{
    CSB_V1_StartupEntrancePointerAction_PC34 action =
        CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_QUIT_PC34;
    CSB_V1_StartupTickState_PC34 tick;
    CSB_V1_StartupTickResult_PC34 result;
    CSB_V1_StartupRenderState_PC34 render_state;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupCommandState_PC34 command_state;
    CSB_V1_StartupEntranceCommandPlan_PC34 command_plan;
    CSB_V1_StartupRuntimePlan_PC34 runtime_plan;
    CSB_V1_StartupSessionOptionsInput_PC34 session_input;
    CSB_V1_StartupSessionOptions_PC34 session_options;
    CSB_V1_StartupEntranceInputOutcome_PC34 outcome;
    CSB_V1_StartupEntranceDecision_PC34 decision;
    CSB_V1_TextMaterial_PC34 material;
    AssetExecutorProbe probe;
    OpeningCompositeProbe composite_probe;
    RenderExecutorProbe render_probe;
    CSB_V1_StartupRenderExecutor_PC34 render_executor;
    char phase[64];
    int startup_active;
    int startup_frame;
    int command;
    int i;
    unsigned char fb[320 * 200];

    material = csb_v1_text_material_pc34(CSB_V1_TEXT_STYLE_TITLE_PC34);
    check(material.scale_x == 2 &&
              material.scale_y == 1 &&
              material.color == 11 &&
              material.shadow_dx == 1 &&
              material.shadow_dy == 1 &&
              material.shadow_color == 0,
          "startup title text material owns CSB title palette");
    material = csb_v1_text_material_pc34(CSB_V1_TEXT_STYLE_SHADOW_PC34);
    check(material.scale_x == 1 &&
              material.color == 15 &&
              material.shadow_dx == 1 &&
              material.shadow_dy == 1,
          "startup shadow text material owns CSB fallback shadow");

    expect_action("enter route becomes CSB startup Enter",
                  244, 45,
                  ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                  CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_ENTER_DUNGEON_PC34,
                  200);
    expect_action("bonus route becomes CSB startup Bonus",
                  244, 45,
                  ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT,
                  CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_ENTER_BONUS_DUNGEON_PC34,
                  201);
    expect_action("resume route becomes CSB startup Resume",
                  244, 76,
                  ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                  CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_RESUME_PC34,
                  202);
    expect_action("credits route becomes CSB startup Credits",
                  248, 186,
                  ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                  CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_DRAW_CREDITS_PC34,
                  203);
    expect_action("quit route becomes CSB startup Quit",
                  243, 110,
                  ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
                  CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_QUIT_PC34,
                  216);

    check(!csb_v1_startup_entrance_pointer_action_pc34(
              0, 1, 1, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, &action) &&
              action == CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34,
          "outside route is ignored");
    command = 0;
    check(csb_v1_startup_entrance_command_for_pointer_pc34(
              0,
              244,
              45,
              ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
              &command) &&
              command == 200,
          "pointer dispatch resolves source entrance command");
    command = 216;
    check(!csb_v1_startup_entrance_command_for_pointer_pc34(
              0,
              1,
              1,
              ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
              &command) &&
              command == 0,
          "pointer dispatch ignores outside route");
    action = CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_QUIT_PC34;
    check(csb_v1_startup_entrance_pointer_action_pc34(
              1, 244, 45, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, &action) &&
              action == CSB_V1_STARTUP_ENTRANCE_POINTER_ACTION_NONE_PC34,
          "credits-active click is consumed as dismiss");
    command = 216;
    check(csb_v1_startup_entrance_command_for_pointer_pc34(
              1,
              244,
              45,
              ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
              &command) &&
              command == 0,
          "pointer dispatch owns credits dismiss");
    check(csb_v1_startup_entrance_pointer_action_pc34(
              0, 244, 45, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, NULL) == 0,
          "NULL output action is rejected");

    memset(&tick, 0, sizeof(tick));
    tick.title_active = 1;
    tick.title_frame = csb_v1_startup_title_total_ticks_pc34() - 1;
    check(csb_v1_startup_advance_tick_pc34(&tick, &result) &&
              result.redraw &&
              result.title_finished &&
              !tick.title_active &&
              tick.title_frame == csb_v1_startup_title_total_ticks_pc34() &&
              tick.entrance_source_step == 1 &&
              tick.entrance_frame == 0,
          "startup tick hands title to entrance source step");

    memset(&tick, 0, sizeof(tick));
    tick.entrance_source_step = 1;
    for (i = 0; i < csb_v1_startup_entrance_wait_stage_pc34() - 1; ++i) {
        (void)csb_v1_startup_advance_tick_pc34(&tick, &result);
    }
    check(tick.entrance_source_step ==
              csb_v1_startup_entrance_wait_stage_pc34() &&
              result.reached_entrance_wait,
          "startup tick advances entrance prelude to wait stage");

    memset(&tick, 0, sizeof(tick));
    tick.credits_active = 1;
    tick.credits_remaining_ticks = 1;
    check(csb_v1_startup_advance_tick_pc34(&tick, &result) &&
              result.credits_finished &&
              !tick.credits_active &&
              tick.credits_remaining_ticks == 0,
          "startup tick owns credits timeout");

    memset(&tick, 0, sizeof(tick));
    tick.opening_active = 1;
    tick.opening_delay_ticks =
        csb_v1_startup_entrance_pre_open_delay_ticks_pc34();
    tick.opening_step = 1;
    tick.door_step_count = 3;
    for (i = 0; i < csb_v1_startup_entrance_pre_open_delay_ticks_pc34(); ++i) {
        (void)csb_v1_startup_advance_tick_pc34(&tick, &result);
    }
    check(tick.opening_delay_ticks == 0 &&
              tick.opening_step == 1 &&
              !result.door_opening_finished,
          "startup tick consumes pre-open delay before door steps");
    (void)csb_v1_startup_advance_tick_pc34(&tick, &result);
    check(tick.opening_step == 2 && !result.door_opening_finished,
          "startup tick advances door opening steps");
    (void)csb_v1_startup_advance_tick_pc34(&tick, &result);
    (void)csb_v1_startup_advance_tick_pc34(&tick, &result);
    check(result.door_opening_finished,
          "startup tick reports door-opening completion");

    check(csb_v1_startup_entrance_credits_ticks_pc34() == 1800,
          "startup sequence owns credits timeout");

    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.title_active = 1;
    render_state.title_frame = 0;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              plan.source_asset_id == 1 &&
              plan.title_source_step == 1 &&
              plan.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              plan.title_source_x == 0 &&
              plan.title_source_y == 137 &&
              plan.title_source_w == 320 &&
              plan.title_source_h == 16 &&
              plan.title_blit_kind ==
                  CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34 &&
              plan.title_transparent_color == -1 &&
              plan.title_dest_x == 0 &&
              plan.title_dest_y == 90 &&
              plan.title_dest_w == 320 &&
              plan.title_dest_h == 16 &&
              plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS &&
              plan.special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS &&
              plan.fallback_title_x == 38 &&
              plan.fallback_title_y == 52 &&
              plan.fallback_title_style == 1 &&
              strcmp(plan.fallback_title_text, "FTL PRESENTS") == 0 &&
              plan.title_empty_fallback_x == 38 &&
              plan.title_empty_fallback_y == 90 &&
              plan.title_empty_fallback_style == 1 &&
              strcmp(plan.title_empty_fallback_text, "FTL PRESENTS") == 0 &&
              plan.fallback_subtitle_x == 38 &&
              plan.fallback_subtitle_y == 86 &&
              plan.fallback_subtitle_style == 2 &&
              strcmp(plan.fallback_subtitle_text, "CHAOS") == 0 &&
              plan.fallback_prompt_x == 38 &&
              plan.fallback_prompt_y == 112 &&
              plan.fallback_prompt_style == 3 &&
              strcmp(plan.fallback_prompt_text, "STRIKES BACK") == 0 &&
              plan.fallback_text_row_count == 3 &&
              plan.fallback_text_rows[0].visible &&
              plan.fallback_text_rows[0].x == 38 &&
              plan.fallback_text_rows[0].y == 52 &&
              plan.fallback_text_rows[0].style == 1 &&
              strcmp(plan.fallback_text_rows[0].text, "FTL PRESENTS") == 0 &&
              plan.fallback_text_rows[1].visible &&
              strcmp(plan.fallback_text_rows[1].text, "CHAOS") == 0 &&
              plan.fallback_text_rows[2].visible &&
              strcmp(plan.fallback_text_rows[2].text, "STRIKES BACK") == 0 &&
              plan.primitive_command_count == 1 &&
              plan.primitive_commands[0].kind ==
                  CSB_V1_STARTUP_PRIMITIVE_FILL_RECT_PC34 &&
              plan.primitive_commands[0].x == 0 &&
              plan.primitive_commands[0].y == 0 &&
              plan.primitive_commands[0].w == 320 &&
              plan.primitive_commands[0].h == 200 &&
              plan.primitive_commands[0].color == 0 &&
              plan.asset_command_count == 1 &&
              plan.asset_commands[0].kind ==
                  CSB_V1_STARTUP_ASSET_TITLE_REGION_PC34 &&
              plan.asset_commands[0].asset_id == 1 &&
              plan.asset_commands[0].source_y == 137 &&
              plan.asset_commands[0].dest_y == 90 &&
              plan.asset_commands[0].transparent_color == -1 &&
              !plan.waiting_for_input,
          "startup render plan owns title PRESENTS surface, boxes, palette, fallback rows, asset blit, and primitive clear");
    check(plan.render_command_count == 1 &&
              plan.render_commands[0].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_TITLE_PC34,
          "startup render plan owns title draw command");
    memset(&render_probe, 0, sizeof(render_probe));
    render_probe_executor_init(&render_executor, &render_probe);
    check(csb_v1_startup_execute_render_plan_pc34(&plan, &render_executor) &&
              render_probe.draw_title_count == 1 &&
              render_probe.clear_black_count == 0 &&
              render_probe.draw_full_surface_count == 0 &&
              render_probe.draw_fallback_text_count == 0,
          "startup render executor dispatches title command and stops");
    memset(&probe, 0, sizeof(probe));
    check(csb_v1_startup_execute_asset_commands_kind_pc34(
              &plan,
              CSB_V1_STARTUP_ASSET_TITLE_REGION_PC34,
              asset_executor_probe,
              &probe) == 1 &&
              probe.call_count == 1 &&
              probe.seen_kind[0] ==
                  CSB_V1_STARTUP_ASSET_TITLE_REGION_PC34 &&
              probe.seen_asset_id[0] == 1,
          "startup asset executor dispatches PRESENTS title region");
    {
        unsigned char framebuffer[320 * 200];
        memset(framebuffer, 0, sizeof(framebuffer));
        check(csb_v1_startup_title_empty_fallback_needed_pc34(
                  &plan,
                  framebuffer,
                  320,
                  200),
              "startup title policy requests fallback when PRESENTS blit is empty");
        framebuffer[plan.title_dest_y * 320 + plan.title_dest_x] = 7u;
        check(!csb_v1_startup_title_empty_fallback_needed_pc34(
                  &plan,
                  framebuffer,
                  320,
                  200),
              "startup title policy suppresses fallback when PRESENTS pixels exist");
        check(!csb_v1_startup_title_empty_fallback_needed_pc34(
                  NULL,
                  framebuffer,
                  320,
                  200),
              "startup title policy ignores missing render plan");
    }

    render_state.title_frame =
        csb_v1_startup_title_presents_ticks_pc34() + 1;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              plan.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              plan.title_source_x == 136 &&
              plan.title_source_y == 74 &&
              plan.title_source_w == 48 &&
              plan.title_source_h == 12 &&
              plan.title_blit_kind ==
                  CSB_V1_STARTUP_TITLE_BLIT_SCALED_REGION_PC34 &&
              plan.title_transparent_color == -1 &&
              plan.title_empty_fallback_text == NULL &&
              plan.title_dest_x == 0 &&
              plan.title_dest_y == 0 &&
              plan.title_dest_w == 320 &&
              plan.title_dest_h == 80 &&
              plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_TITLE &&
              plan.special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_TITLE &&
              plan.asset_command_count == 1 &&
              plan.asset_commands[0].kind ==
                  CSB_V1_STARTUP_ASSET_TITLE_SCALED_REGION_PC34 &&
              plan.asset_commands[0].asset_id == 1 &&
              plan.asset_commands[0].dest_w == 320 &&
              plan.asset_commands[0].dest_h == 80,
          "startup render plan exposes title CHAOS zoom stage, boxes, palette, and asset blit");

    render_state.title_frame =
        csb_v1_startup_title_total_ticks_pc34() - 1;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              plan.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 &&
              plan.title_source_x == 0 &&
              plan.title_source_y == 80 &&
              plan.title_source_w == 320 &&
              plan.title_source_h == 57 &&
              plan.title_blit_kind ==
                  CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34 &&
              plan.title_transparent_color == 0 &&
              plan.title_empty_fallback_text == NULL &&
              plan.title_dest_x == 0 &&
              plan.title_dest_y == 118 &&
              plan.title_dest_w == 320 &&
              plan.title_dest_h == 57 &&
              plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_TITLE &&
              plan.special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_TITLE &&
              plan.asset_command_count == 1 &&
              plan.asset_commands[0].kind ==
                  CSB_V1_STARTUP_ASSET_TITLE_REGION_PC34 &&
              plan.asset_commands[0].asset_id == 1 &&
              plan.asset_commands[0].source_y == 80 &&
              plan.asset_commands[0].dest_y == 118 &&
              plan.asset_commands[0].transparent_color == 0,
          "startup render plan exposes title STRIKES BACK stage, boxes, palette, and asset blit");

    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.entrance_source_step = 2;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34 &&
              plan.title_special_palette == -1 &&
              plan.special_palette == VGA_PALETTE_PC34_SPECIAL_ENTRANCE &&
              plan.primitive_command_count == 1 &&
              plan.primitive_commands[0].kind ==
                  CSB_V1_STARTUP_PRIMITIVE_FILL_RECT_PC34 &&
              !plan.waiting_for_input,
          "startup render plan owns entrance blackout");
    check(plan.render_command_count == 1 &&
              plan.render_commands[0].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34,
          "startup render plan owns entrance blackout command");

    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.entrance_frame = 0;
    render_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    render_state.runtime_start_valid = 1;
    render_state.runtime_start_x = 5;
    render_state.runtime_start_y = 7;
    render_state.runtime_start_dir = 2;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              plan.source_asset_id == 4 &&
              plan.surface_dest_x == 0 &&
              plan.surface_dest_y == 0 &&
              plan.surface_w == 320 &&
              plan.surface_h == 200 &&
              plan.surface_transparent_color == -1 &&
              plan.closed_left_asset_id == 2 &&
              plan.closed_right_asset_id == 3 &&
              plan.special_palette == VGA_PALETTE_PC34_SPECIAL_ENTRANCE &&
              plan.waiting_for_input &&
              plan.blink_prompt_visible &&
              plan.closed_left_source_x == 0 &&
              plan.closed_left_source_y == 0 &&
              plan.closed_left_dest_x == 0 &&
              plan.closed_left_dest_y == 28 &&
              plan.closed_left_w == 105 &&
              plan.closed_left_h == 161 &&
              plan.closed_left_fallback_fill_color == 12 &&
              plan.closed_left_fallback_light_edge_color == 2 &&
              plan.closed_left_fallback_dark_edge_color == 0 &&
              plan.closed_right_source_x == 0 &&
              plan.closed_right_source_y == 0 &&
              plan.closed_right_dest_x == 105 &&
              plan.closed_right_dest_y == 28 &&
              plan.closed_right_w == 127 &&
              plan.closed_right_h == 161 &&
              plan.closed_right_fallback_fill_color == 12 &&
              plan.closed_right_fallback_light_edge_color == 2 &&
              plan.closed_right_fallback_dark_edge_color == 0 &&
              plan.fallback_title_x == 38 &&
              plan.fallback_title_y == 42 &&
              plan.fallback_title_style == 2 &&
              strcmp(plan.fallback_title_text, "CHAOS STRIKES BACK") == 0 &&
              plan.fallback_subtitle_x == 38 &&
              plan.fallback_subtitle_y == 64 &&
              plan.fallback_subtitle_style == 3 &&
              strcmp(plan.fallback_subtitle_text, "ENTRANCE") == 0 &&
              plan.fallback_status_x == 38 &&
              plan.fallback_status_y == 84 &&
              plan.fallback_status_style == 1 &&
              strcmp(plan.fallback_status_text, "CSB RUNTIME READY") == 0 &&
              plan.fallback_status_visible &&
              plan.fallback_frame_valid &&
              plan.fallback_frame_x == 18 &&
              plan.fallback_frame_y == 18 &&
              plan.fallback_frame_w == 284 &&
              plan.fallback_frame_h == 164 &&
              plan.fallback_frame_color == 14 &&
              plan.fallback_detail_x == 38 &&
              plan.fallback_detail_y == 96 &&
              plan.fallback_detail_style == 1 &&
              strcmp(plan.fallback_detail_text, "START") == 0 &&
              plan.fallback_detail_visible &&
              plan.fallback_runtime_detail_visible &&
              strcmp(plan.fallback_runtime_detail_text, "START 5,7 DIR 2") == 0 &&
              plan.fallback_prompt_x == 38 &&
              plan.fallback_prompt_y == 154 &&
              plan.fallback_prompt_style == 3 &&
              strcmp(plan.fallback_prompt_text, "PRESS ENTER") == 0 &&
              plan.fallback_text_row_count == 5 &&
              strcmp(plan.fallback_text_rows[0].text, "CHAOS STRIKES BACK") == 0 &&
              strcmp(plan.fallback_text_rows[1].text, "ENTRANCE") == 0 &&
              strcmp(plan.fallback_text_rows[2].text, "CSB RUNTIME READY") == 0 &&
              strcmp(plan.fallback_text_rows[3].text, "START 5,7 DIR 2") == 0 &&
              strcmp(plan.fallback_text_rows[4].text, "PRESS ENTER") == 0 &&
              plan.primitive_command_count == 2 &&
              plan.primitive_commands[0].kind ==
                  CSB_V1_STARTUP_PRIMITIVE_FILL_RECT_PC34 &&
              plan.primitive_commands[1].kind ==
                  CSB_V1_STARTUP_PRIMITIVE_DRAW_RECT_PC34 &&
              plan.primitive_commands[1].x == 18 &&
              plan.primitive_commands[1].y == 18 &&
              plan.primitive_commands[1].w == 284 &&
              plan.primitive_commands[1].h == 164 &&
              plan.primitive_commands[1].color == 14 &&
              plan.asset_command_count == 3 &&
              plan.asset_commands[0].kind ==
                  CSB_V1_STARTUP_ASSET_FULL_SURFACE_PC34 &&
              plan.asset_commands[0].asset_id == 4 &&
              plan.asset_commands[1].kind ==
                  CSB_V1_STARTUP_ASSET_CLOSED_LEFT_DOOR_PC34 &&
              plan.asset_commands[1].asset_id == 2 &&
              plan.asset_commands[2].kind ==
                  CSB_V1_STARTUP_ASSET_CLOSED_RIGHT_DOOR_PC34 &&
              plan.asset_commands[2].asset_id == 3,
          "startup render plan owns closed entrance prompt, fallback rows, door boxes, asset blits, and primitive frame");
    check(plan.render_command_count == 5 &&
              plan.render_commands[0].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34 &&
              plan.render_commands[1].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34 &&
              plan.render_commands[2].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_ELSE_FALLBACK_PC34 &&
              plan.render_commands[3].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_FALLBACK_IF_NO_SURFACE_PC34 &&
              plan.render_commands[4].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_UTILITY_PANEL_IF_WAITING_PC34,
          "startup render plan owns closed entrance command order");
    memset(&probe, 0, sizeof(probe));
    check(csb_v1_startup_execute_asset_commands_kind_pc34(
              &plan,
              CSB_V1_STARTUP_ASSET_FULL_SURFACE_PC34,
              asset_executor_probe,
              &probe) == 1 &&
              probe.call_count == 1 &&
              probe.seen_kind[0] ==
                  CSB_V1_STARTUP_ASSET_FULL_SURFACE_PC34 &&
              probe.seen_asset_id[0] == 4,
          "startup asset executor dispatches closed entrance surface");
    memset(&probe, 0, sizeof(probe));
    check(csb_v1_startup_execute_closed_door_asset_commands_pc34(
              &plan,
              asset_executor_probe,
              &probe) &&
              probe.call_count == 2 &&
              probe.seen_kind[0] ==
                  CSB_V1_STARTUP_ASSET_CLOSED_LEFT_DOOR_PC34 &&
              probe.seen_kind[1] ==
                  CSB_V1_STARTUP_ASSET_CLOSED_RIGHT_DOOR_PC34 &&
              probe.seen_asset_id[0] == 2 &&
              probe.seen_asset_id[1] == 3,
          "startup asset executor dispatches closed entrance door pair");
    memset(&probe, 0, sizeof(probe));
    probe.fail_kind = CSB_V1_STARTUP_ASSET_CLOSED_RIGHT_DOOR_PC34;
    check(!csb_v1_startup_execute_closed_door_asset_commands_pc34(
              &plan,
              asset_executor_probe,
              &probe) &&
              probe.call_count == 2,
          "startup asset executor requires both closed doors");

    memset(fb, 0xaa, sizeof(fb));
    check(csb_v1_startup_execute_primitive_commands_pc34(&plan,
                                                         fb,
                                                         320,
                                                         200) == 2 &&
              fb[0] == 0 &&
              fb[18 + 18 * 320] == 14 &&
              fb[301 + 18 * 320] == 14 &&
              fb[18 + 181 * 320] == 14 &&
              fb[301 + 181 * 320] == 14 &&
              fb[19 + 19 * 320] == 0,
          "startup primitive executor draws CSB-owned clear plus fallback entrance frame");

    render_state.utility_overlay_active = 1;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              !plan.fallback_status_visible &&
              !plan.fallback_detail_visible &&
              plan.fallback_runtime_detail_visible &&
              plan.fallback_text_row_count == 5 &&
              plan.fallback_text_rows[0].visible &&
              plan.fallback_text_rows[1].visible &&
              !plan.fallback_text_rows[2].visible &&
              !plan.fallback_text_rows[3].visible &&
              plan.fallback_text_rows[4].visible,
          "startup render plan suppresses fallback status rows behind utility overlay");

    render_state.utility_overlay_active = 0;
    render_state.entrance_frame = 12;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              !plan.blink_prompt_visible &&
              plan.fallback_text_row_count == 5 &&
              !plan.fallback_text_rows[4].visible,
          "startup render plan owns prompt blink cadence");

    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.credits_active = 1;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34 &&
              plan.source_asset_id == 5 &&
              plan.surface_dest_x == 0 &&
              plan.surface_dest_y == 0 &&
              plan.surface_w == 320 &&
              plan.surface_h == 200 &&
              plan.surface_transparent_color == -1 &&
              plan.special_palette == VGA_PALETTE_PC34_SPECIAL_CREDITS &&
              plan.fallback_title_x == 38 &&
              plan.fallback_title_y == 42 &&
              plan.fallback_title_style == 2 &&
              strcmp(plan.fallback_title_text, "CHAOS STRIKES BACK") == 0 &&
              plan.fallback_subtitle_x == 38 &&
              plan.fallback_subtitle_y == 68 &&
              plan.fallback_subtitle_style == 3 &&
              strcmp(plan.fallback_subtitle_text, "CREDITS") == 0 &&
              plan.fallback_prompt_x == 38 &&
              plan.fallback_prompt_y == 154 &&
              plan.fallback_prompt_style == 1 &&
              strcmp(plan.fallback_prompt_text, "PRESS ENTER") == 0 &&
              plan.fallback_text_row_count == 3 &&
              strcmp(plan.fallback_text_rows[0].text, "CHAOS STRIKES BACK") == 0 &&
              strcmp(plan.fallback_text_rows[1].text, "CREDITS") == 0 &&
              strcmp(plan.fallback_text_rows[2].text, "PRESS ENTER") == 0 &&
              plan.primitive_command_count == 1 &&
              plan.primitive_commands[0].kind ==
                  CSB_V1_STARTUP_PRIMITIVE_FILL_RECT_PC34 &&
              plan.asset_command_count == 1 &&
              plan.asset_commands[0].kind ==
                  CSB_V1_STARTUP_ASSET_FULL_SURFACE_PC34 &&
              plan.asset_commands[0].asset_id == 5 &&
              plan.asset_commands[0].dest_w == 320 &&
              plan.asset_commands[0].dest_h == 200,
          "startup render plan owns credits surface, asset blit, fallback rows, and primitive clear");
    check(plan.render_command_count == 2 &&
              plan.render_commands[0].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34 &&
              plan.render_commands[1].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_OR_TEXT_PC34,
          "startup render plan owns credits command order");

    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.opening_active = 1;
    render_state.opening_delay_ticks = 1;
    render_state.opening_step = 2;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34 &&
              plan.source_asset_id == 4 &&
              plan.surface_dest_x == 0 &&
              plan.surface_dest_y == 0 &&
              plan.surface_w == 320 &&
              plan.surface_h == 200 &&
              plan.surface_transparent_color == -1 &&
              plan.closed_left_asset_id == 2 &&
              plan.closed_right_asset_id == 3 &&
              plan.closed_left_fallback_fill_color == 12 &&
              plan.closed_right_fallback_fill_color == 12 &&
              plan.special_palette == VGA_PALETTE_PC34_SPECIAL_ENTRANCE &&
              plan.opening_step == 2 &&
              plan.primitive_command_count == 3 &&
              plan.primitive_commands[0].kind ==
                  CSB_V1_STARTUP_PRIMITIVE_FILL_RECT_PC34 &&
              plan.primitive_commands[1].kind ==
                  CSB_V1_STARTUP_PRIMITIVE_DOOR_PANEL_PC34 &&
              plan.primitive_commands[1].x == 0 &&
              plan.primitive_commands[1].y == 28 &&
              plan.primitive_commands[1].w == 105 &&
              plan.primitive_commands[1].h == 161 &&
              plan.primitive_commands[1].color == 12 &&
              plan.primitive_commands[1].light_edge_color == 2 &&
              plan.primitive_commands[1].dark_edge_color == 0 &&
              plan.primitive_commands[2].kind ==
                  CSB_V1_STARTUP_PRIMITIVE_DOOR_PANEL_PC34 &&
              plan.primitive_commands[2].x == 105 &&
              plan.primitive_commands[2].w == 127 &&
              plan.asset_command_count == 3 &&
              plan.asset_commands[0].kind ==
                  CSB_V1_STARTUP_ASSET_FULL_SURFACE_PC34 &&
              plan.asset_commands[1].kind ==
                  CSB_V1_STARTUP_ASSET_CLOSED_LEFT_DOOR_PC34 &&
              plan.asset_commands[2].kind ==
                  CSB_V1_STARTUP_ASSET_CLOSED_RIGHT_DOOR_PC34,
          "startup render plan owns door pre-open surface, asset blits, and fallback primitive panels");
    check(plan.render_command_count == 3 &&
              plan.render_commands[0].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34 &&
              plan.render_commands[1].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34 &&
              plan.render_commands[2].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_ELSE_FALLBACK_PC34,
          "startup render plan owns door pre-open command order");

    memset(fb, 0xaa, sizeof(fb));
    check(csb_v1_startup_execute_primitive_commands_pc34(&plan,
                                                         fb,
                                                         320,
                                                         200) == 3 &&
              fb[0] == 0 &&
              fb[0 + 28 * 320] == 2 &&
              fb[104 + 28 * 320] == 0 &&
              fb[0 + 188 * 320] == 2 &&
              fb[104 + 188 * 320] == 0 &&
              fb[1 + 29 * 320] == 12 &&
              fb[105 + 28 * 320] == 2 &&
              fb[231 + 188 * 320] == 0 &&
              fb[106 + 29 * 320] == 12,
          "startup primitive executor draws CSB-owned fallback door panels");

    render_state.opening_delay_ticks = 0;
    check(csb_v1_startup_build_render_plan_pc34(&render_state, &plan) &&
              plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34 &&
              plan.special_palette == VGA_PALETTE_PC34_SPECIAL_ENTRANCE &&
              plan.source_asset_id == 4 &&
              plan.surface_dest_x == 0 &&
              plan.surface_dest_y == 0 &&
              plan.surface_w == 320 &&
              plan.surface_h == 200 &&
              plan.surface_transparent_color == -1 &&
              plan.closed_left_asset_id == 2 &&
              plan.closed_right_asset_id == 3 &&
              plan.closed_left_fallback_light_edge_color == 2 &&
              plan.closed_right_fallback_dark_edge_color == 0 &&
              plan.opening_step == 2 &&
              plan.opening_door_valid &&
              plan.opening_door_step == 2 &&
              plan.opening_left_source_x == 0 &&
              plan.opening_left_dest_x == 0 &&
              plan.opening_left_dest_y == 28 &&
              plan.opening_left_w == 97 &&
              plan.opening_left_h == 161 &&
              plan.opening_right_source_x == 8 &&
              plan.opening_right_dest_x == 113 &&
              plan.opening_right_dest_y == 28 &&
              plan.opening_right_w == 119 &&
              plan.opening_right_h == 161 &&
              plan.opening_composite_valid &&
              plan.opening_composite_screen_asset_id == 4 &&
              plan.opening_composite_left_asset_id == 2 &&
              plan.opening_composite_right_asset_id == 3 &&
              plan.opening_composite_animation_step == 2 &&
              plan.opening_composite_left_box_x == 0 &&
              plan.opening_composite_left_box_y == 0 &&
              plan.opening_composite_left_box_w == 97 &&
              plan.opening_composite_left_box_h == 161 &&
              plan.opening_composite_right_box_x == 113 &&
              plan.opening_composite_right_box_y == 0 &&
              plan.opening_composite_right_box_w == 119 &&
              plan.opening_composite_right_box_h == 161 &&
              plan.opening_composite_left_source_x == 0 &&
              plan.opening_composite_right_source_x == 8 &&
              plan.primitive_command_count == 3 &&
              plan.primitive_commands[1].kind ==
                  CSB_V1_STARTUP_PRIMITIVE_DOOR_PANEL_PC34 &&
              plan.asset_command_count == 3 &&
              plan.asset_commands[0].kind ==
                  CSB_V1_STARTUP_ASSET_FULL_SURFACE_PC34 &&
              plan.asset_commands[1].kind ==
                  CSB_V1_STARTUP_ASSET_CLOSED_LEFT_DOOR_PC34 &&
              plan.asset_commands[2].kind ==
                  CSB_V1_STARTUP_ASSET_CLOSED_RIGHT_DOOR_PC34,
          "startup render plan owns door-opening frame surface, boxes, composite plan, asset blits, and fallback primitive panels");
    check(plan.render_command_count == 4 &&
              plan.render_commands[0].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34 &&
              plan.render_commands[1].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34 &&
              plan.render_commands[2].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_OPENING_FRAME_IF_SURFACE_PC34 &&
              plan.render_commands[3].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_ELSE_FALLBACK_PC34,
          "startup render plan owns door-opening command order");
    memset(&render_probe, 0, sizeof(render_probe));
    render_probe.draw_full_surface_result = 1;
    render_probe.draw_opening_frame_result = 1;
    render_probe_executor_init(&render_executor, &render_probe);
    check(csb_v1_startup_execute_render_plan_pc34(&plan, &render_executor) &&
              render_probe.clear_black_count == 1 &&
              render_probe.draw_full_surface_count == 1 &&
              render_probe.draw_opening_frame_count == 1 &&
              render_probe.draw_closed_doors_count == 0 &&
              render_probe.draw_door_fallback_count == 0,
          "startup render executor stops after successful opening composite");
    memset(&render_probe, 0, sizeof(render_probe));
    render_probe.draw_full_surface_result = 0;
    render_probe_executor_init(&render_executor, &render_probe);
    check(csb_v1_startup_execute_render_plan_pc34(&plan, &render_executor) &&
              render_probe.clear_black_count == 1 &&
              render_probe.draw_full_surface_count == 1 &&
              render_probe.draw_opening_frame_count == 0 &&
              render_probe.draw_closed_doors_count == 0 &&
              render_probe.draw_door_fallback_count == 1,
          "startup render executor routes missing door surface to fallback primitives");
    memset(&composite_probe, 0, sizeof(composite_probe));
    check(csb_v1_startup_execute_opening_composite_pc34(
              &plan,
              opening_composite_probe,
              &composite_probe) &&
              composite_probe.call_count == 1 &&
              composite_probe.seen.screen_asset_id == 4 &&
              composite_probe.seen.left_door_asset_id == 2 &&
              composite_probe.seen.right_door_asset_id == 3 &&
              composite_probe.seen.animation_step == 2 &&
              composite_probe.seen.left_box_x == 0 &&
              composite_probe.seen.left_box_w == 97 &&
              composite_probe.seen.right_box_x == 113 &&
              composite_probe.seen.right_box_w == 119 &&
              composite_probe.seen.left_source_x == 0 &&
              composite_probe.seen.right_source_x == 8,
          "startup opening composite executor dispatches source door frame");
    memset(&composite_probe, 0, sizeof(composite_probe));
    composite_probe.fail = 1;
    check(!csb_v1_startup_execute_opening_composite_pc34(
              &plan,
              opening_composite_probe,
              &composite_probe) &&
              composite_probe.call_count == 1,
          "startup opening composite executor reports callback failure");

    memset(&command_state, 0, sizeof(command_state));
    check(csb_v1_startup_init_command_state_pc34(&command_state, 0) &&
              command_state.title_active &&
              command_state.title_frame == 0 &&
              command_state.title_source_step ==
                  CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              command_state.entrance_active &&
              !command_state.entrance_dismissed &&
              command_state.entrance_source_step == 0 &&
              !command_state.credits_active &&
              !command_state.opening_active &&
              command_state.pending_command == 0,
          "startup command state initializes new-game title");

    memset(&command_state, 0xff, sizeof(command_state));
    check(csb_v1_startup_init_command_state_pc34(&command_state, 1) &&
              !command_state.title_active &&
              command_state.title_frame == 0 &&
              command_state.title_source_step == 0 &&
              !command_state.entrance_active &&
              command_state.entrance_dismissed &&
              command_state.entrance_source_step == 0 &&
              !command_state.credits_active &&
              !command_state.opening_active &&
              command_state.pending_command == 0,
          "startup command state initializes resume runtime");

    csb_v1_startup_session_options_init_pc34(&session_options);
    check(session_options.import_utility_state == (int)CSB_V1_UTIL_FLOW_INIT &&
              !session_options.import_available &&
              !session_options.entrance_resume_available,
          "startup session options initialize to clean utility state");
    memset(&session_input, 0, sizeof(session_input));
    session_input.direct_resume_loaded = 1;
    session_input.import_dm1_save_path = "/tmp/DM1SAVE.DAT";
    session_input.import_party_loaded = 1;
    session_input.import_champion_count = 2;
    session_input.import_utility_state = (int)CSB_V1_UTIL_FLOW_DONE;
    session_input.import_utility_prompt = "CHAOS STRIKES BACK READY";
    session_input.entrance_resume_save_path = "/tmp/CSBSAVE.DAT";
    session_input.entrance_resume_can_load = 1;
    check(csb_v1_startup_build_session_options_pc34(
              &session_input,
              &session_options) &&
              !session_options.import_available &&
              !session_options.entrance_resume_available &&
              session_options.import_dm1_save_path[0] == '\0' &&
              session_options.entrance_resume_path[0] == '\0',
          "startup session options suppress entrance choices after direct resume");
    memset(&session_input, 0, sizeof(session_input));
    session_input.import_dm1_save_path = "/tmp/DM1SAVE.DAT";
    session_input.import_party_loaded = 1;
    session_input.import_champion_count = 2;
    session_input.import_utility_state = (int)CSB_V1_UTIL_FLOW_DONE;
    session_input.import_utility_prompt = "CHAOS STRIKES BACK READY";
    session_input.entrance_resume_save_path = "/tmp/CSBSAVE.DAT";
    session_input.entrance_resume_can_load = 1;
    check(csb_v1_startup_build_session_options_pc34(
              &session_input,
              &session_options) &&
              session_options.import_available &&
              session_options.import_champion_count == 2 &&
              session_options.import_selected_action_index == 0 &&
              !session_options.import_preview_active &&
              session_options.import_utility_state ==
                  (int)CSB_V1_UTIL_FLOW_DONE &&
              strcmp(session_options.import_dm1_save_path,
                     "/tmp/DM1SAVE.DAT") == 0 &&
              strcmp(session_options.import_utility_prompt,
                     "CHAOS STRIKES BACK READY") == 0 &&
              session_options.entrance_resume_available &&
              strcmp(session_options.entrance_resume_path,
                     "/tmp/CSBSAVE.DAT") == 0,
          "startup session options publish import and validated resume choices");
    memset(&session_input, 0, sizeof(session_input));
    session_input.import_dm1_save_path = "/tmp/DM1SAVE.DAT";
    session_input.import_party_loaded = 1;
    session_input.import_champion_count = 0;
    session_input.entrance_resume_save_path = "/tmp/BAD.DAT";
    session_input.entrance_resume_can_load = 0;
    check(csb_v1_startup_build_session_options_pc34(
              &session_input,
              &session_options) &&
              !session_options.import_available &&
              !session_options.entrance_resume_available,
          "startup session options reject empty import and invalid resume");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.title_active = 1;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(!csb_v1_startup_entrance_accepts_input_pc34(&command_state),
          "startup entrance gate blocks title phase input");

    command_state.title_active = 0;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34() - 1;
    check(!csb_v1_startup_entrance_accepts_input_pc34(&command_state),
          "startup entrance gate blocks pre-wait input");

    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(csb_v1_startup_entrance_accepts_input_pc34(&command_state),
          "startup entrance gate accepts wait-loop input");

    command_state.opening_active = 1;
    check(!csb_v1_startup_entrance_accepts_input_pc34(&command_state),
          "startup entrance gate blocks door-opening input");

    command_state.opening_active = 0;
    command_state.credits_active = 1;
    command_state.entrance_source_step = 0;
    check(csb_v1_startup_entrance_accepts_input_pc34(&command_state),
          "startup entrance gate accepts credits-dismiss input");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.credits_active = 1;
    command_state.credits_remaining_ticks = 99;
    check(csb_v1_startup_begin_door_opening_pc34(&command_state, 200) &&
              command_state.pending_command == 200 &&
              command_state.opening_active &&
              command_state.entrance_source_step ==
                  CSB_V1_STARTUP_STAGE_ENTRANCE_PRE_OPEN_DELAY_PC34 &&
              command_state.opening_delay_ticks ==
                  csb_v1_startup_entrance_pre_open_delay_ticks_pc34() &&
              command_state.opening_step == 1 &&
              !command_state.credits_active &&
              command_state.credits_remaining_ticks == 0,
          "startup command state begins door opening");

    check(csb_v1_startup_finish_door_opening_pc34(&command_state) &&
              !command_state.opening_active &&
              !command_state.entrance_active &&
              command_state.entrance_dismissed &&
              command_state.pending_command == 0 &&
              command_state.entrance_source_step == 0,
          "startup command state finishes door opening");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.opening_active = 1;
    command_state.opening_step = 3;
    check(csb_v1_startup_begin_credits_pc34(&command_state) &&
              command_state.credits_active &&
              command_state.credits_remaining_ticks ==
                  csb_v1_startup_entrance_credits_ticks_pc34() &&
              !command_state.opening_active &&
              command_state.opening_step == 0,
          "startup command state begins credits");

    check(csb_v1_startup_dismiss_credits_pc34(&command_state) &&
              !command_state.credits_active &&
              command_state.credits_remaining_ticks == 0,
          "startup command state dismisses credits");

    check(csb_v1_startup_input_from_firestaff_menu_code_pc34(0) ==
              CSB_V1_STARTUP_INPUT_NONE_PC34 &&
              csb_v1_startup_input_from_firestaff_menu_code_pc34(9) ==
                  CSB_V1_STARTUP_INPUT_ACCEPT_PC34 &&
              csb_v1_startup_input_from_firestaff_menu_code_pc34(10) ==
                  CSB_V1_STARTUP_INPUT_BACK_PC34 &&
              csb_v1_startup_input_from_firestaff_menu_code_pc34(11) ==
                  CSB_V1_STARTUP_INPUT_ACTION_PC34 &&
              csb_v1_startup_input_from_firestaff_menu_code_pc34(32) ==
                  CSB_V1_STARTUP_INPUT_DISK_MENU_PC34,
          "Firestaff menu input codes map through CSB startup input adapter");
    check(csb_v1_startup_input_from_firestaff_menu_code_pc34(999) ==
              CSB_V1_STARTUP_INPUT_NONE_PC34,
          "unknown Firestaff menu input maps to CSB startup idle input");

    check(csb_v1_startup_entrance_command_for_action_pc34(
              CSB_V1_STARTUP_ENTRANCE_ACTION_ENTER_DUNGEON_PC34) ==
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              csb_v1_startup_entrance_command_for_action_pc34(
                  CSB_V1_STARTUP_ENTRANCE_ACTION_RESUME_PC34) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34 &&
              csb_v1_startup_entrance_command_for_action_pc34(
                  CSB_V1_STARTUP_ENTRANCE_ACTION_QUIT_PC34) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34 &&
              csb_v1_startup_entrance_command_for_action_pc34(
                  CSB_V1_STARTUP_ENTRANCE_ACTION_NONE_PC34) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
          "startup entrance actions resolve source command ids");
    check(csb_v1_startup_entrance_command_for_input_pc34(
              0,
              CSB_V1_STARTUP_INPUT_ACCEPT_PC34) ==
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              csb_v1_startup_entrance_command_for_input_pc34(
                  0,
                  CSB_V1_STARTUP_INPUT_DISK_MENU_PC34) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34 &&
              csb_v1_startup_entrance_command_for_input_pc34(
                  0,
                  CSB_V1_STARTUP_INPUT_BACK_PC34) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34 &&
              csb_v1_startup_entrance_command_for_input_pc34(
                  1,
              CSB_V1_STARTUP_INPUT_ACCEPT_PC34) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
          "startup entrance inputs resolve source command ids");
    command = CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    check(csb_v1_startup_entrance_command_for_firestaff_input_pc34(
              0,
              9,
              &command) &&
              command ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
          "startup Firestaff keyboard dispatch resolves Enter command");
    command = CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34;
    check(!csb_v1_startup_entrance_command_for_firestaff_input_pc34(
              0,
              999,
              &command) &&
              command == CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
          "startup Firestaff keyboard dispatch ignores idle input");
    command = CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34;
    check(csb_v1_startup_entrance_command_for_firestaff_input_pc34(
              1,
              999,
              &command) &&
              command == CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
          "startup Firestaff keyboard dispatch owns credits dismiss");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.credits_active = 1;
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &decision) &&
              decision ==
                  CSB_V1_STARTUP_ENTRANCE_DECISION_DISMISS_CREDITS_PC34,
          "startup command resolver dismisses credits before command dispatch");

    command_state.credits_active = 0;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34() - 1;
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &decision) &&
              decision == CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34,
          "startup command resolver blocks pre-wait commands");

    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    command_state.opening_active = 1;
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &decision) &&
              decision == CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34,
          "startup command resolver blocks opening commands");

    command_state.opening_active = 0;
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &decision) &&
              decision ==
                  CSB_V1_STARTUP_ENTRANCE_DECISION_ENTER_DUNGEON_PC34,
          "startup command resolver accepts dungeon entry command");
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_BONUS_DUNGEON_PC34,
              &decision) &&
              decision ==
                  CSB_V1_STARTUP_ENTRANCE_DECISION_ENTER_BONUS_DUNGEON_PC34,
          "startup command resolver accepts bonus dungeon command");
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34,
              &decision) &&
              decision == CSB_V1_STARTUP_ENTRANCE_DECISION_RESUME_PC34,
          "startup command resolver accepts resume command");
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_DRAW_CREDITS_PC34,
              &decision) &&
              decision ==
                  CSB_V1_STARTUP_ENTRANCE_DECISION_BEGIN_CREDITS_PC34,
          "startup command resolver accepts credits command");
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34,
              &decision) &&
              decision == CSB_V1_STARTUP_ENTRANCE_DECISION_QUIT_PC34,
          "startup command resolver accepts quit command");
    check(csb_v1_startup_resolve_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
              &decision) &&
              decision == CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34,
          "startup command resolver ignores empty command");
    command_state.credits_active = 1;
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &command_plan) &&
              command_plan.kind ==
                  CSB_V1_STARTUP_ENTRANCE_PLAN_DISMISS_CREDITS_PC34 &&
              strcmp(command_plan.status_scope, "BOOT") == 0 &&
              strcmp(command_plan.status, "CSB ENTRANCE") == 0,
          "startup command plan resolves credits dismissal status");
    command_state.credits_active = 0;
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &command_plan) &&
              command_plan.kind ==
                  CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_DUNGEON_PC34 &&
              strcmp(command_plan.status, "CSB DOORS") == 0,
          "startup command plan resolves dungeon entry handoff");
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_BONUS_DUNGEON_PC34,
              &command_plan) &&
              command_plan.kind ==
                  CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_BONUS_DUNGEON_PC34 &&
              strcmp(command_plan.status, "CSB DOORS") == 0,
          "startup command plan resolves bonus dungeon handoff");
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34,
              &command_plan) &&
              command_plan.kind ==
                  CSB_V1_STARTUP_ENTRANCE_PLAN_RESUME_PC34 &&
              strcmp(command_plan.status, "CSB DOORS") == 0 &&
              strcmp(command_plan.failure_status,
                     "CSB RESUME FAILED") == 0 &&
              strcmp(command_plan.unavailable_status,
                     "CSB RESUME UNAVAILABLE") == 0,
          "startup command plan resolves resume statuses");
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_DRAW_CREDITS_PC34,
              &command_plan) &&
              command_plan.kind ==
                  CSB_V1_STARTUP_ENTRANCE_PLAN_BEGIN_CREDITS_PC34 &&
              strcmp(command_plan.status, "CSB CREDITS") == 0,
          "startup command plan resolves credits status");
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34,
              &command_plan) &&
              command_plan.kind ==
                  CSB_V1_STARTUP_ENTRANCE_PLAN_QUIT_PC34 &&
              strcmp(command_plan.status_scope, "RETURN") == 0 &&
              strcmp(command_plan.status, "BACK TO LAUNCHER") == 0,
          "startup command plan resolves quit status");
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
              &command_plan) &&
              command_plan.kind ==
                  CSB_V1_STARTUP_ENTRANCE_PLAN_IGNORE_PC34,
          "startup command plan resolves ignored command");

    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &command_plan) &&
              csb_v1_startup_runtime_plan_for_entrance_plan_pc34(
                  &command_plan,
                  &runtime_plan) &&
              runtime_plan.kind ==
                  CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_DUNGEON_PC34 &&
              runtime_plan.command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              runtime_plan.set_bonus_dungeon &&
              runtime_plan.bonus_dungeon == 0 &&
              !runtime_plan.requires_resume_load &&
              runtime_plan.begin_door_opening,
          "startup runtime plan owns normal dungeon handoff");
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_BONUS_DUNGEON_PC34,
              &command_plan) &&
              csb_v1_startup_runtime_plan_for_entrance_plan_pc34(
                  &command_plan,
                  &runtime_plan) &&
              runtime_plan.kind ==
                  CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_BONUS_DUNGEON_PC34 &&
              runtime_plan.set_bonus_dungeon &&
              runtime_plan.bonus_dungeon == 1 &&
              runtime_plan.begin_door_opening,
          "startup runtime plan owns bonus dungeon handoff");
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34,
              &command_plan) &&
              csb_v1_startup_runtime_plan_for_entrance_plan_pc34(
                  &command_plan,
                  &runtime_plan) &&
              runtime_plan.kind == CSB_V1_STARTUP_RUNTIME_PLAN_RESUME_PC34 &&
              runtime_plan.requires_resume_load &&
              runtime_plan.begin_door_opening &&
              strcmp(runtime_plan.failure_status, "CSB RESUME FAILED") == 0,
          "startup runtime plan owns resume handoff");
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_DRAW_CREDITS_PC34,
              &command_plan) &&
              !csb_v1_startup_runtime_plan_for_entrance_plan_pc34(
                  &command_plan,
                  &runtime_plan),
          "startup runtime plan rejects pure credits command");

    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
              &command_plan),
          "startup command plan resets to ignored command");
    check(csb_v1_startup_entrance_input_outcome_pc34(
              &command_plan,
              0,
              0,
              &outcome) &&
              outcome.result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_IGNORE_PC34 &&
              outcome.status == NULL,
          "startup input outcome ignores ignored command plans");

    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34,
              &command_plan) &&
              csb_v1_startup_entrance_input_outcome_pc34(
                  &command_plan,
                  0,
                  0,
                  &outcome) &&
              outcome.result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 &&
              strcmp(outcome.status_scope, "BOOT") == 0 &&
              strcmp(outcome.status, "CSB RESUME UNAVAILABLE") == 0,
          "startup input outcome reports unavailable resume status");
    check(csb_v1_startup_entrance_input_outcome_pc34(
              &command_plan,
              1,
              0,
              &outcome) &&
              outcome.result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 &&
              strcmp(outcome.status, "CSB RESUME FAILED") == 0,
          "startup input outcome reports failed resume status");
    check(csb_v1_startup_entrance_input_outcome_pc34(
              &command_plan,
              1,
              1,
              &outcome) &&
              outcome.result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 &&
              strcmp(outcome.status, "CSB DOORS") == 0,
          "startup input outcome reports loaded resume status");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &command_plan) &&
              csb_v1_startup_runtime_plan_for_entrance_plan_pc34(
                  &command_plan,
                  &runtime_plan) &&
              csb_v1_startup_apply_runtime_plan_pc34(
                  &command_state,
                  &runtime_plan,
                  0,
                  0,
                  &outcome) &&
              command_state.opening_active &&
              command_state.pending_command ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              outcome.result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 &&
              strcmp(outcome.status, "CSB DOORS") == 0,
          "startup runtime apply begins normal dungeon door opening");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34,
              &command_plan) &&
              csb_v1_startup_runtime_plan_for_entrance_plan_pc34(
                  &command_plan,
                  &runtime_plan) &&
              csb_v1_startup_apply_runtime_plan_pc34(
                  &command_state,
                  &runtime_plan,
                  1,
                  0,
                  &outcome) &&
              !command_state.opening_active &&
              strcmp(outcome.status, "CSB RESUME FAILED") == 0,
          "startup runtime apply keeps door shut on failed resume");
    check(csb_v1_startup_apply_runtime_plan_pc34(
              &command_state,
              &runtime_plan,
              1,
              1,
              &outcome) &&
              command_state.opening_active &&
              command_state.pending_command ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34 &&
              strcmp(outcome.status, "CSB DOORS") == 0,
          "startup runtime apply begins door after loaded resume");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34,
              &command_plan) &&
              csb_v1_startup_entrance_input_outcome_pc34(
                  &command_plan,
                  0,
                  0,
                  &outcome) &&
              outcome.result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_RETURN_TO_LAUNCHER_PC34 &&
              strcmp(outcome.status_scope, "RETURN") == 0 &&
              strcmp(outcome.status, "BACK TO LAUNCHER") == 0,
          "startup input outcome reports quit-to-launcher result");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.credits_active = 1;
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &command_plan) &&
              csb_v1_startup_apply_pure_entrance_plan_pc34(
                  &command_state,
                  &command_plan,
                  &outcome) ==
                  CSB_V1_STARTUP_ENTRANCE_APPLY_REDRAW_PC34 &&
              !command_state.credits_active &&
              outcome.result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 &&
              strcmp(outcome.status, "CSB ENTRANCE") == 0,
          "startup pure command apply dismisses credits");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_DRAW_CREDITS_PC34,
              &command_plan) &&
              csb_v1_startup_apply_pure_entrance_plan_pc34(
                  &command_state,
                  &command_plan,
                  &outcome) ==
                  CSB_V1_STARTUP_ENTRANCE_APPLY_REDRAW_PC34 &&
              command_state.credits_active &&
              command_state.credits_remaining_ticks ==
                  csb_v1_startup_entrance_credits_ticks_pc34() &&
              strcmp(outcome.status, "CSB CREDITS") == 0,
          "startup pure command apply begins credits");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34,
              &command_plan) &&
              csb_v1_startup_apply_pure_entrance_plan_pc34(
                  &command_state,
                  &command_plan,
                  &outcome) ==
                  CSB_V1_STARTUP_ENTRANCE_APPLY_RETURN_TO_LAUNCHER_PC34 &&
              !command_state.entrance_active &&
              command_state.entrance_dismissed &&
              strcmp(outcome.status, "BACK TO LAUNCHER") == 0,
          "startup pure command apply quits to launcher");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    check(csb_v1_startup_plan_for_entrance_command_pc34(
              &command_state,
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
              &command_plan) &&
              csb_v1_startup_apply_pure_entrance_plan_pc34(
                  &command_state,
                  &command_plan,
                  &outcome) ==
                  CSB_V1_STARTUP_ENTRANCE_APPLY_NOT_HANDLED_PC34,
          "startup pure command apply leaves runtime dungeon entry to M11");

    command_state.title_active = 1;
    command_state.title_frame = 7;
    command_state.title_source_step = 2;
    command_state.entrance_active = 1;
    command_state.entrance_source_step = 4;
    command_state.credits_active = 1;
    command_state.credits_remaining_ticks = 1;
    command_state.opening_active = 1;
    command_state.opening_delay_ticks = 1;
    command_state.opening_step = 1;
    command_state.pending_command = 200;
    check(csb_v1_startup_quit_to_launcher_pc34(&command_state) &&
              !command_state.title_active &&
              command_state.title_frame == 0 &&
              command_state.title_source_step == 0 &&
              !command_state.entrance_active &&
              command_state.entrance_dismissed &&
              command_state.entrance_source_step == 0 &&
              !command_state.credits_active &&
              !command_state.opening_active &&
              command_state.pending_command == 0,
          "startup command state quits to launcher");

    memset(&command_state, 0, sizeof(command_state));
    command_state.entrance_active = 1;
    command_state.title_active = 1;
    command_state.title_frame = 7;
    command_state.title_source_step = 2;
    check(csb_v1_startup_receipt_phase_pc34(
              &command_state, 44, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "csb-title-2") == 0 &&
              startup_active == 1 &&
              startup_frame == 7,
          "startup receipt phase reports title source step");

    command_state.title_active = 0;
    command_state.opening_active = 1;
    command_state.opening_step = 5;
    check(csb_v1_startup_receipt_phase_pc34(
              &command_state, 44, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "csb-entrance-opening-5") == 0 &&
              startup_active == 1 &&
              startup_frame == 44,
          "startup receipt phase reports door opening");

    command_state.opening_active = 0;
    command_state.credits_active = 1;
    check(csb_v1_startup_receipt_phase_pc34(
              &command_state, 44, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "csb-credits") == 0 &&
              startup_active == 1,
          "startup receipt phase reports credits");

    command_state.credits_active = 0;
    command_state.entrance_source_step = 4;
    check(csb_v1_startup_receipt_phase_pc34(
              &command_state, 44, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "csb-entrance-4") == 0 &&
              startup_active == 1,
          "startup receipt phase reports entrance source step");

    command_state.entrance_active = 0;
    check(csb_v1_startup_receipt_phase_pc34(
              &command_state, 44, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "csb-runtime") == 0 &&
              startup_active == 0,
          "startup receipt phase reports runtime");

    check(strstr(csb_v1_startup_entrance_pointer_source_evidence_pc34(),
                 "ENTRANCE.C") != NULL,
          "source evidence cites ENTRANCE.C");

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed ? 1 : 0;
}
