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
    CSB_V1_UtilRenderTextRow preview_rows[CSB_V1_UTIL_PREVIEW_MAX_RENDER_ROWS];
    int row_count;

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
              rows[0].text_x == rows[0].x &&
              rows[0].text_y == rows[0].y + 2 &&
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
              action_plan.kind == CSB_V1_UTIL_ACTION_PLAN_IMPORT_READY &&
              action_plan.preview_active == 0 &&
              strcmp(action_plan.status_scope, "BOOT") == 0 &&
              strcmp(action_plan.status, "CSB IMPORT READY") == 0,
          "utility action plan resolves Import status");
    check(csb_v1_util_flow_plan_for_action(
              CSB_V1_UTIL_ACTION_VIEW,
              &action_plan) &&
              action_plan.kind == CSB_V1_UTIL_ACTION_PLAN_VIEW_READY &&
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
              panel.preview_max_rows == 4,
          "utility panel geometry remains source-shaped");
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
