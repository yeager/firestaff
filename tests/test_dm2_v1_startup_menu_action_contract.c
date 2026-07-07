#include "dm2_v1_startup_menu.h"

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
    DM2_V1_StartupMenu menu;
    DM2_V1_StartupAction action;
    DM2_V1_StartupActionPlan plan;
    DM2_V1_StartupExecution execution;
    DM2_V1_StartupHit hit;
    DM2_V1_StartupMenuSnapshot snapshot;
    DM2_V1_StartupRowKind row_kind;
    DM2_V1_StartupRenderRow rows[4];
    DM2_V1_SessionState direct_session;
    DM2_V1_StartupSavePathResult save_path_result;
    char phase[64];
    char save_root[128];
    uint8_t save_slot;
    int last_session;
    int row_slot;
    int startup_active;
    int row_count;

    check(dm2_v1_startup_input_from_firestaff_menu_code(0) ==
              DM2_V1_STARTUP_INPUT_NONE &&
              dm2_v1_startup_input_from_firestaff_menu_code(1) ==
                  DM2_V1_STARTUP_INPUT_UP &&
              dm2_v1_startup_input_from_firestaff_menu_code(2) ==
                  DM2_V1_STARTUP_INPUT_DOWN &&
              dm2_v1_startup_input_from_firestaff_menu_code(9) ==
                  DM2_V1_STARTUP_INPUT_ACCEPT &&
              dm2_v1_startup_input_from_firestaff_menu_code(10) ==
                  DM2_V1_STARTUP_INPUT_BACK &&
              dm2_v1_startup_input_from_firestaff_menu_code(11) ==
                  DM2_V1_STARTUP_INPUT_ACTION,
          "Firestaff menu input codes map through DM2 startup input adapter");
    check(dm2_v1_startup_input_from_firestaff_menu_code(999) ==
              DM2_V1_STARTUP_INPUT_NONE,
          "unknown Firestaff menu input maps to DM2 startup idle input");

    save_path_result = dm2_v1_startup_load_session_from_save_path(
        "/tmp/firestaff-dm2-startup-missing/Other.dat",
        save_root,
        (int)sizeof(save_root),
        &direct_session,
        &save_slot,
        &last_session);
    check(save_path_result == DM2_V1_STARTUP_SAVE_PATH_INVALID,
          "direct resume session loader rejects non-SKSave paths");
    save_path_result = dm2_v1_startup_load_session_from_save_path(
        "/tmp/firestaff-dm2-startup-missing/SKSave03.dat",
        save_root,
        (int)sizeof(save_root),
        &direct_session,
        &save_slot,
        &last_session);
    check(save_path_result == DM2_V1_STARTUP_SAVE_PATH_LOAD_FAILED &&
              strcmp(save_root, "/tmp/firestaff-dm2-startup-missing") == 0 &&
              save_slot == 3u &&
              last_session == 0,
          "direct resume session loader reports missing slot after parsing");
    save_path_result = dm2_v1_startup_load_session_from_save_path(
        "/tmp/firestaff-dm2-startup-missing/SKSave.dat",
        save_root,
        (int)sizeof(save_root),
        &direct_session,
        &save_slot,
        &last_session);
    check(save_path_result == DM2_V1_STARTUP_SAVE_PATH_LOAD_FAILED &&
              strcmp(save_root, "/tmp/firestaff-dm2-startup-missing") == 0 &&
              save_slot == 0u &&
              last_session == 1,
          "direct resume session loader reports missing last-session after parsing");

    dm2_v1_startup_menu_init(&menu, "/tmp/firestaff-dm2-startup");
    check(dm2_v1_startup_menu_refresh(&menu, 1, (1u << 2)) &&
              menu.resume_available == 1 &&
              menu.slot_mask == (1u << 2) &&
              menu.row_count == 3 &&
              menu.selected_row == 0,
          "refresh exposes Continue, slot, and New Game rows");
    check(dm2_v1_startup_menu_scan_saves(&menu) &&
              menu.resume_available == 0 &&
              menu.slot_mask == 0u &&
              menu.row_count == 1 &&
              menu.selected_row == 0,
          "scan saves exposes New Game only for empty startup root");
    check(dm2_v1_startup_menu_scan_saves(NULL) == 0,
          "scan saves rejects NULL startup menu");
    check(dm2_v1_startup_menu_refresh(&menu, 1, (1u << 2)) &&
              menu.resume_available == 1 &&
              menu.slot_mask == (1u << 2) &&
              menu.row_count == 3 &&
              menu.selected_row == 0,
          "refresh restores Continue, slot, and New Game rows");
    check(dm2_v1_startup_menu_snapshot_from_menu(&snapshot, &menu) &&
              dm2_v1_startup_menu_snapshot_row_at(
                  &snapshot, 1, &row_kind, &row_slot) &&
              row_kind == DM2_V1_STARTUP_ROW_SLOT &&
              row_slot == 2,
          "snapshot row lookup resolves slot without M11 row adapter");
    row_count = dm2_v1_startup_menu_build_render_rows(
        &menu,
        rows,
        (int)(sizeof(rows) / sizeof(rows[0])));
    check(row_count == 3 &&
              rows[0].kind == DM2_V1_STARTUP_ROW_CONTINUE &&
              rows[0].selected == 1 &&
              rows[0].rect.x == 92 &&
              rows[0].rect.y == 76 &&
              rows[0].highlight_rect.w == 140 &&
              rows[0].text_x == DM2_V1_STARTUP_ROW_TEXT_X &&
              rows[0].text_y == 78 &&
              rows[0].label[0] == 'C',
          "render rows expose selected Continue presentation");
    check(rows[1].kind == DM2_V1_STARTUP_ROW_SLOT &&
              rows[1].slot == 2 &&
              rows[1].rect.y == 90 &&
              rows[1].selected == 0 &&
              rows[1].label[0] == 'L',
          "render rows expose slot presentation");
    check(rows[2].kind == DM2_V1_STARTUP_ROW_NEW_GAME &&
              rows[2].slot == -1 &&
              rows[2].rect.y == 104 &&
              rows[2].label[0] == 'N',
          "render rows expose New Game presentation");
    check(dm2_v1_startup_menu_handle_input(
              &menu, DM2_V1_STARTUP_INPUT_ACCEPT, &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_CONTINUE &&
              action.row == 0 &&
              action.slot == -1,
          "keyboard Accept returns Continue action directly");
    check(dm2_v1_startup_plan_for_action(&action, &plan) &&
              plan.kind == DM2_V1_STARTUP_PLAN_CONTINUE &&
              plan.slot == -1 &&
              plan.rescan_saves_on_failure == 1 &&
              strcmp(plan.success_status, "DM2 CONTINUED") == 0 &&
              strcmp(plan.failure_status, "DM2 CONTINUE FAILED") == 0,
          "Continue action resolves to DM2-owned startup load plan");
    check(dm2_v1_startup_execute_plan(
              &plan, "/tmp/firestaff-dm2-startup-missing", &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_STATUS_REDRAW &&
              execution.rescan_saves == 1 &&
              strcmp(execution.status, "DM2 CONTINUE FAILED") == 0,
          "Continue plan execution reports failed load and rescan");
    check(dm2_v1_startup_execute_action(
              &action, "/tmp/firestaff-dm2-startup-missing", &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_STATUS_REDRAW &&
              execution.rescan_saves == 1 &&
              strcmp(execution.status, "DM2 CONTINUE FAILED") == 0,
          "Continue action executes through DM2-owned startup wrapper");
    check(dm2_v1_startup_menu_handle_input(
              &menu, DM2_V1_STARTUP_INPUT_DOWN, &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE &&
              menu.selected_row == 1,
          "keyboard Down returns navigation action directly");
    check(dm2_v1_startup_plan_for_action(&action, &plan) &&
              plan.kind == DM2_V1_STARTUP_PLAN_IGNORE &&
              plan.slot == -1 &&
              strcmp(plan.success_status, "DM2 START SELECT") == 0,
          "navigation action resolves to DM2-owned redraw plan");
    check(dm2_v1_startup_menu_handle_input(
              &menu, DM2_V1_STARTUP_INPUT_ACTION, &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_LOAD_SLOT &&
              action.row == 1 &&
              action.slot == 2,
          "keyboard Action returns Load Slot action directly");
    check(dm2_v1_startup_plan_for_action(&action, &plan) &&
              plan.kind == DM2_V1_STARTUP_PLAN_LOAD_SLOT &&
              plan.slot == 2 &&
              plan.rescan_saves_on_failure == 1 &&
              strcmp(plan.success_status, "DM2 SLOT LOADED") == 0 &&
              strcmp(plan.failure_status, "DM2 SLOT LOAD FAILED") == 0,
          "Load Slot action resolves to DM2-owned startup load plan");
    check(dm2_v1_startup_execute_plan(
              &plan, "/tmp/firestaff-dm2-startup-missing", &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_STATUS_REDRAW &&
              execution.rescan_saves == 1 &&
              strcmp(execution.status, "DM2 SLOT LOAD FAILED") == 0,
          "Load Slot plan execution reports failed load and rescan");
    check(dm2_v1_startup_execute_action(
              &action, "/tmp/firestaff-dm2-startup-missing", &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_STATUS_REDRAW &&
              execution.rescan_saves == 1 &&
              strcmp(execution.status, "DM2 SLOT LOAD FAILED") == 0,
          "Load Slot action executes through DM2-owned startup wrapper");
    check(dm2_v1_startup_menu_handle_input(
              &menu, DM2_V1_STARTUP_INPUT_DOWN, &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE &&
              menu.selected_row == 2,
          "keyboard Down reaches New Game row");
    check(dm2_v1_startup_menu_handle_input(
              &menu, DM2_V1_STARTUP_INPUT_ACCEPT, &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NEW_GAME &&
              action.row == 2 &&
              action.slot == -1,
          "keyboard Accept returns New Game action directly");
    check(dm2_v1_startup_plan_for_action(&action, &plan) &&
              plan.kind == DM2_V1_STARTUP_PLAN_NEW_GAME &&
              plan.slot == -1 &&
              plan.rescan_saves_on_failure == 0 &&
              strcmp(plan.success_status, "DM2 NEW GAME") == 0,
          "New Game action resolves to DM2-owned startup session plan");
    check(dm2_v1_startup_execute_plan(
              &plan, "/tmp/firestaff-dm2-startup-missing", &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_SESSION_READY &&
              execution.rescan_saves == 0 &&
              execution.session.champion_count == 4 &&
              execution.session.party_x == 15 &&
              execution.session.party_y == 15 &&
              strcmp(execution.status, "DM2 NEW GAME") == 0,
          "New Game plan execution creates a DM2 startup session");
    check(dm2_v1_startup_execute_action(
              &action, "/tmp/firestaff-dm2-startup-missing", &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_SESSION_READY &&
              execution.rescan_saves == 0 &&
              execution.session.champion_count == 4 &&
              execution.session.party_x == 15 &&
              execution.session.party_y == 15 &&
              strcmp(execution.status, "DM2 NEW GAME") == 0,
          "New Game action executes through DM2-owned startup wrapper");
    check(dm2_v1_startup_menu_handle_input(
              &menu, DM2_V1_STARTUP_INPUT_BACK, &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER &&
              action.row == 2 &&
              action.slot == -1,
          "keyboard Back returns launcher action directly");
    check(dm2_v1_startup_plan_for_action(&action, &plan) &&
              plan.kind == DM2_V1_STARTUP_PLAN_RETURN_TO_LAUNCHER &&
              strcmp(plan.success_status, "BACK TO LAUNCHER") == 0,
          "Back action resolves to DM2-owned launcher-return plan");
    check(dm2_v1_startup_execute_plan(
              &plan, "/tmp/firestaff-dm2-startup-missing", &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_RETURN_TO_LAUNCHER &&
              strcmp(execution.status, "BACK TO LAUNCHER") == 0,
          "Back plan execution returns launcher command");
    check(dm2_v1_startup_execute_action(
              &action, "/tmp/firestaff-dm2-startup-missing", &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_RETURN_TO_LAUNCHER &&
              strcmp(execution.status, "BACK TO LAUNCHER") == 0,
          "Back action executes through DM2-owned startup wrapper");

    hit.kind = DM2_V1_STARTUP_HIT_PANEL;
    hit.row = -1;
    check(dm2_v1_startup_menu_handle_hit(&menu, &hit, &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE &&
              action.row == menu.selected_row,
          "pointer panel hit is consumed without activation");
    hit.kind = DM2_V1_STARTUP_HIT_ROW;
    hit.row = 1;
    check(dm2_v1_startup_menu_handle_hit(&menu, &hit, &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_LOAD_SLOT &&
              action.row == 1 &&
              action.slot == 2 &&
              menu.selected_row == 1,
          "pointer row hit returns Load Slot action directly");
    check(!dm2_v1_startup_menu_handle_input(
              &menu, DM2_V1_STARTUP_INPUT_NONE, &action),
          "idle input is ignored");
    check(dm2_v1_startup_receipt_phase(
              1, phase, sizeof(phase), &startup_active) &&
              strcmp(phase, "dm2-startup-menu") == 0 &&
              startup_active == 1,
          "receipt phase reports DM2 startup menu");
    check(dm2_v1_startup_receipt_phase(
              0, phase, sizeof(phase), &startup_active) &&
              strcmp(phase, "dm2-runtime") == 0 &&
              startup_active == 0,
          "receipt phase reports DM2 runtime");

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed ? 1 : 0;
}
