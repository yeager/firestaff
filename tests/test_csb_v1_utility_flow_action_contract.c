#include "csb_v1_utility_flow_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_passed;
static int g_failed;

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

int main(void)
{
    CSB_V1_UtilFlowContext flow;
    CSB_V1_UtilInputResult result;
    CSB_V1_UtilActionPlan action_plan;
    CSB_V1_UtilPanelLayout panel;
    CSB_V1_UtilRuntimeSnapshot snapshot;
    CSB_V1_UtilRenderRow rows[CSB_V1_UTIL_MENU_ROW_COUNT];
    CSB_V1_UtilRenderTextRow status_row;
    CSB_V1_UtilRenderTextRow prompt_row;
    CSB_V1_UtilRenderTextRow preview_rows[CSB_V1_UTIL_PREVIEW_MAX_RENDER_ROWS];
    CSB_V1_UtilRenderPlan render_plan;
    CSB_V1_TextMaterial_PC34 material;
    int row_count;

    check(csb_v1_util_input_from_firestaff_menu_code(0) ==
              CSB_V1_UTIL_INPUT_NONE &&
              csb_v1_util_input_from_firestaff_menu_code(1) ==
                  CSB_V1_UTIL_INPUT_UP &&
              csb_v1_util_input_from_firestaff_menu_code(2) ==
                  CSB_V1_UTIL_INPUT_DOWN &&
              csb_v1_util_input_from_firestaff_menu_code(9) ==
                  CSB_V1_UTIL_INPUT_ACCEPT &&
              csb_v1_util_input_from_firestaff_menu_code(10) ==
                  CSB_V1_UTIL_INPUT_BACK &&
              csb_v1_util_input_from_firestaff_menu_code(11) ==
                  CSB_V1_UTIL_INPUT_ACTION,
          "Firestaff menu input codes map through CSB utility input adapter");
    check(csb_v1_util_input_from_firestaff_menu_code(999) ==
              CSB_V1_UTIL_INPUT_NONE,
          "unknown Firestaff menu input maps to CSB utility idle input");

    material = csb_v1_text_material_pc34(CSB_V1_TEXT_STYLE_SMALL_PC34);
    check(material.scale_x == 1 &&
              material.scale_y == 1 &&
              material.color == 15 &&
              material.shadow_dx == 0 &&
              material.shadow_dy == 0 &&
              material.shadow_color == 0,
          "CSB small text material owns utility row palette");

    csb_v1_util_flow_init(&flow);
    flow.state = CSB_V1_UTIL_FLOW_SELECT_ACTION;
    flow.selected_action_index = 0;

    row_count = csb_v1_util_flow_menu_render_rows(
        &flow,
        rows,
        (int)(sizeof(rows) / sizeof(rows[0])));
    check(row_count == CSB_V1_UTIL_MENU_ROW_COUNT,
          "render rows expose all utility menu actions");
    check(rows[0].action == CSB_V1_UTIL_ACTION_IMPORT &&
              rows[0].selected == 1 &&
              rows[0].highlight_x == rows[0].x - 2 &&
              rows[0].highlight_y == rows[0].y &&
              rows[0].highlight_color == 12 &&
              rows[0].text_x == rows[0].x &&
              rows[0].text_y == rows[0].y + 2 &&
              rows[0].text_style == 1 &&
              rows[0].label[0] == '>' &&
              strstr(rows[0].label, "IMPORT") != NULL,
          "selected Import render row owns marker, highlight, and label");
    check(rows[1].action == CSB_V1_UTIL_ACTION_LOAD &&
              rows[1].selected == 0 &&
              rows[1].label[0] == ' ' &&
              strstr(rows[1].label, "LOAD") != NULL,
          "unselected Load render row owns visible label");

    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.selected_action_index = -1;
    snapshot.imported_champion_count = 1;
    check(csb_v1_util_flow_build_from_runtime_snapshot(&snapshot, &flow) &&
              flow.state == CSB_V1_UTIL_FLOW_SELECT_ACTION &&
              flow.selected_action_index == 3 &&
              flow.action == CSB_V1_UTIL_ACTION_VIEW &&
              flow.imported_champion_count == 1,
          "utility flow builds from runtime snapshot and wraps cursor");
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.selected_action_index = 5;
    snapshot.imported_champion_count = 1;
    snapshot.imported_party_available = 1;
    snapshot.imported_party.ImportedFromDM1 = 1;
    snapshot.imported_party.ChampionCount = 3;
    snprintf(snapshot.imported_party.Champions[0].Name,
             sizeof(snapshot.imported_party.Champions[0].Name),
             "%s",
             "GAMMA   ");
    snapshot.imported_party.Champions[0].CurrentHealth = 17;
    snapshot.imported_party.Champions[0].MaximumHealth = 29;
    check(csb_v1_util_flow_build_from_runtime_snapshot(&snapshot, &flow) &&
              flow.selected_action_index == 1 &&
              flow.action == CSB_V1_UTIL_ACTION_LOAD &&
              flow.imported_champion_count == 3 &&
              flow.imported_party.Champions[0].CurrentHealth == 17,
          "utility flow snapshot imports party preview state");
    flow.state = CSB_V1_UTIL_FLOW_SELECT_ACTION;
    flow.selected_action_index = 2;
    flow.action = CSB_V1_UTIL_ACTION_NEW;
    check(csb_v1_util_flow_accept_import_action(&flow) &&
              flow.selected_action_index == 0 &&
              flow.action == CSB_V1_UTIL_ACTION_IMPORT,
          "utility flow owns forced Import action acceptance");
    flow.state = CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS;
    check(!csb_v1_util_flow_accept_import_action(&flow),
          "utility flow rejects forced Import outside the action menu");
    check(!csb_v1_util_flow_accept_import_action(NULL),
          "utility flow rejects NULL forced Import");
    check(csb_v1_util_flow_entrance_command_for_action(
              CSB_V1_UTIL_ACTION_NEW) ==
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              csb_v1_util_flow_entrance_command_for_action(
                  CSB_V1_UTIL_ACTION_LOAD) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34 &&
              csb_v1_util_flow_entrance_command_for_action(
                  CSB_V1_UTIL_ACTION_IMPORT) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34 &&
              csb_v1_util_flow_entrance_command_for_action(
                  CSB_V1_UTIL_ACTION_VIEW) ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
          "utility flow actions resolve entrance source commands");
    check(csb_v1_util_flow_plan_for_action(
              CSB_V1_UTIL_ACTION_LOAD,
              &action_plan) &&
              action_plan.kind ==
                  CSB_V1_UTIL_ACTION_PLAN_ENTRANCE_COMMAND &&
              action_plan.entrance_command ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34 &&
              action_plan.preview_active == 0,
          "utility action plan resolves Load to entrance resume");
    check(csb_v1_util_flow_plan_for_action(
              CSB_V1_UTIL_ACTION_NEW,
              &action_plan) &&
              action_plan.kind ==
                  CSB_V1_UTIL_ACTION_PLAN_ENTRANCE_COMMAND &&
              action_plan.entrance_command ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
          "utility action plan resolves New to entrance dungeon");
    check(csb_v1_util_flow_plan_for_action(
              CSB_V1_UTIL_ACTION_IMPORT,
              &action_plan) &&
              action_plan.kind == CSB_V1_UTIL_ACTION_PLAN_STATUS_REDRAW &&
              action_plan.preview_active == 0 &&
              strcmp(action_plan.status_scope, "BOOT") == 0 &&
              strcmp(action_plan.status, "CSB IMPORT READY") == 0,
          "utility action plan resolves Import status");
    check(csb_v1_util_flow_plan_for_action(
              CSB_V1_UTIL_ACTION_VIEW,
              &action_plan) &&
              action_plan.kind == CSB_V1_UTIL_ACTION_PLAN_STATUS_REDRAW &&
              action_plan.preview_active == 1 &&
              strcmp(action_plan.status_scope, "BOOT") == 0 &&
              strcmp(action_plan.status, "CSB PARTY READY") == 0,
          "utility action plan resolves View status");
    check(csb_v1_util_flow_plan_for_action(
              CSB_V1_UTIL_ACTION_EXIT,
              &action_plan) &&
              action_plan.kind == CSB_V1_UTIL_ACTION_PLAN_IGNORE,
          "utility action plan resolves Exit as ignored");

    csb_v1_util_flow_init(&flow);
    flow.state = CSB_V1_UTIL_FLOW_SELECT_ACTION;
    flow.selected_action_index = 0;
    check(csb_v1_util_flow_handle_input(
              &flow,
              CSB_V1_UTIL_INPUT_DOWN,
              1,
              &result) &&
              result.kind == CSB_V1_UTIL_INPUT_RESULT_CURSOR_MOVED &&
              result.selected_action_index == 1 &&
              result.action == CSB_V1_UTIL_ACTION_LOAD &&
              result.preview_active == 0,
          "keyboard Down moves cursor and closes preview");
    check(csb_v1_util_flow_handle_input(
              &flow,
              CSB_V1_UTIL_INPUT_ACTION,
              0,
              &result) &&
              result.kind == CSB_V1_UTIL_INPUT_RESULT_ACTIVATE &&
              result.selected_action_index == 1 &&
              result.action == CSB_V1_UTIL_ACTION_LOAD,
          "keyboard Action activates selected Load row");
    check(csb_v1_util_flow_handle_input(
              &flow,
              CSB_V1_UTIL_INPUT_BACK,
              1,
              &result) &&
              result.kind == CSB_V1_UTIL_INPUT_RESULT_CLOSE_PREVIEW &&
              result.preview_active == 0 &&
              strcmp(result.status_scope, "BOOT") == 0 &&
              strcmp(result.status, "CSB IMPORT READY") == 0,
          "keyboard Back close-preview result owns status");
    check(csb_v1_util_flow_handle_firestaff_input_if_active(
              &flow,
              2,
              1,
              0,
              0,
              1,
              &result) &&
              result.kind == CSB_V1_UTIL_INPUT_RESULT_CURSOR_MOVED &&
              result.selected_action_index == 2 &&
              result.action == CSB_V1_UTIL_ACTION_NEW &&
              result.preview_active == 0,
          "Firestaff keyboard helper owns overlay gate and input mapping");
    check(!csb_v1_util_flow_handle_firestaff_input_if_active(
              &flow,
              2,
              0,
              0,
              0,
              1,
              &result) &&
              result.kind == CSB_V1_UTIL_INPUT_RESULT_NONE &&
              result.preview_active == 1,
          "Firestaff keyboard helper rejects inactive overlay");
    check(csb_v1_util_flow_overlay_accepts_input(1, 0, 0) &&
              !csb_v1_util_flow_overlay_accepts_input(0, 0, 0) &&
              !csb_v1_util_flow_overlay_accepts_input(1, 1, 0) &&
              !csb_v1_util_flow_overlay_accepts_input(1, 0, 1),
          "utility overlay input gate belongs to CSB flow");

    flow.selected_action_index = 0;
    check(csb_v1_util_flow_handle_point_if_active(
              &flow,
              40,
              116,
              1,
              0,
              0,
              1,
              &result) &&
              result.kind == CSB_V1_UTIL_INPUT_RESULT_ACTIVATE &&
              result.action == CSB_V1_UTIL_ACTION_LOAD &&
              result.selected_action_index == 1 &&
              result.preview_active == 0 &&
              flow.selected_action_index == 1,
          "pointer helper owns overlay gate and row activation");
    check(!csb_v1_util_flow_handle_point_if_active(
              &flow,
              40,
              116,
              0,
              0,
              0,
              1,
              &result) &&
              result.kind == CSB_V1_UTIL_INPUT_RESULT_NONE &&
              result.selected_action_index == 1 &&
              result.preview_active == 1,
          "pointer helper rejects inactive overlay");

    flow.selected_action_index = 0;
    check(csb_v1_util_flow_handle_point(&flow, 40, 116, 1, &result) &&
              result.kind == CSB_V1_UTIL_INPUT_RESULT_ACTIVATE &&
              result.action == CSB_V1_UTIL_ACTION_LOAD &&
              result.selected_action_index == 1 &&
              result.preview_active == 0 &&
              flow.selected_action_index == 1,
          "pointer row activates through CSB-owned result");
    check(csb_v1_util_flow_handle_point(&flow, 40, 92, 1, &result) &&
              result.kind == CSB_V1_UTIL_INPUT_RESULT_NONE &&
              result.selected_action_index == 1 &&
              result.preview_active == 1,
          "pointer panel whitespace is consumed");
    check(!csb_v1_util_flow_handle_point(&flow, 2, 2, 0, &result),
          "pointer outside panel is ignored");

    check(csb_v1_util_flow_panel_layout(&flow, 1, &panel) &&
              panel.x == 38 &&
              panel.y == 80 &&
              panel.fill_visible == 1 &&
              panel.fill_color == 0 &&
              panel.border_visible == 1 &&
              panel.border_color == 14 &&
              panel.prompt_text_style == 1 &&
              panel.preview_max_rows == 4,
          "utility panel geometry and material remain source-shaped");
    flow.imported_champion_count = 2;
    flow.imported_party.ChampionCount = 2;
    snprintf(flow.imported_party.Champions[0].Name,
             sizeof(flow.imported_party.Champions[0].Name),
             "%s",
             "ALPHA   ");
    flow.imported_party.Champions[0].CurrentHealth = 31;
    flow.imported_party.Champions[0].MaximumHealth = 44;
    snprintf(flow.imported_party.Champions[1].Name,
             sizeof(flow.imported_party.Champions[1].Name),
             "%s",
             "BETA    ");
    flow.imported_party.Champions[1].CurrentHealth = 22;
    flow.imported_party.Champions[1].MaximumHealth = 33;
    check(csb_v1_util_flow_import_status_render_row(&flow, &status_row) &&
              status_row.x == 38 &&
              status_row.y == 80 &&
              status_row.text_style == 1 &&
              strcmp(status_row.text,
                     "DM1 IMPORT READY: 2 CHAMPIONS") == 0,
          "utility flow owns import status render row");
    check(!csb_v1_util_flow_prompt_render_row(&flow, "", &prompt_row),
          "utility flow skips empty prompt render row");
    check(csb_v1_util_flow_prompt_render_row(
              &flow,
              "CHAOS STRIKES BACK READY",
              &prompt_row) &&
              prompt_row.x == 38 &&
              prompt_row.y == 92 &&
              prompt_row.text_style == 1 &&
              strcmp(prompt_row.text, "CHAOS STRIKES BACK READY") == 0,
          "utility flow owns prompt render row");
    row_count = csb_v1_util_flow_preview_render_rows(
        &flow,
        preview_rows,
        (int)(sizeof(preview_rows) / sizeof(preview_rows[0])));
    check(row_count == 2 &&
              preview_rows[0].x == 48 &&
              preview_rows[0].y == 154 &&
              strcmp(preview_rows[0].text, "1 ALPHA  HP 31/44") == 0 &&
              preview_rows[1].x == 48 &&
              preview_rows[1].y == 164 &&
              strcmp(preview_rows[1].text, "2 BETA  HP 22/33") == 0,
          "utility flow owns imported champion preview rows");
    check(csb_v1_util_flow_render_plan(
              &flow,
              "CHAOS STRIKES BACK READY",
              1,
              &render_plan) &&
              render_plan.panel.x == 38 &&
              render_plan.panel.fill_visible == 1 &&
              render_plan.panel.border_visible == 1 &&
              render_plan.has_status_row == 1 &&
              render_plan.has_prompt_row == 1 &&
              render_plan.menu_row_count == CSB_V1_UTIL_MENU_ROW_COUNT &&
              render_plan.preview_active == 1 &&
              render_plan.preview_row_count == 2 &&
              strcmp(render_plan.status_row.text,
                     "DM1 IMPORT READY: 2 CHAMPIONS") == 0 &&
              strcmp(render_plan.prompt_row.text,
                     "CHAOS STRIKES BACK READY") == 0 &&
              strcmp(render_plan.preview_rows[0].text,
                     "1 ALPHA  HP 31/44") == 0,
          "utility flow owns complete startup render plan");
    check(csb_v1_util_flow_render_plan(
              &flow,
              "",
              0,
              &render_plan) &&
              render_plan.has_status_row == 1 &&
              render_plan.has_prompt_row == 0 &&
              render_plan.preview_active == 0 &&
              render_plan.preview_row_count == 0,
          "utility render plan gates optional prompt and preview rows");
    check(csb_v1_util_flow_action_at_point(&flow, 40, 116) ==
              CSB_V1_UTIL_ACTION_LOAD,
          "legacy point lookup still resolves Load row");
    check(csb_v1_util_flow_entrance_command_for_action(
              CSB_V1_UTIL_ACTION_LOAD) ==
              CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34,
          "Load utility action resolves to CSB startup resume command");
    check(csb_v1_util_flow_entrance_command_for_action(
              CSB_V1_UTIL_ACTION_NEW) ==
              CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
          "New Game utility action resolves to CSB startup enter command");
    check(csb_v1_util_flow_entrance_command_for_action(
              CSB_V1_UTIL_ACTION_IMPORT) == 0 &&
              csb_v1_util_flow_entrance_command_for_action(
                  CSB_V1_UTIL_ACTION_VIEW) == 0 &&
              csb_v1_util_flow_entrance_command_for_action(
                  CSB_V1_UTIL_ACTION_EXIT) == 0,
          "non-entrance utility actions do not synthesize startup commands");
    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed ? 1 : 0;
}
