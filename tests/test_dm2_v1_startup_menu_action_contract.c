#include "dm2_v1_startup_menu.h"

#include <stdio.h>
#include <string.h>

static int g_passed;
static int g_failed;
static int g_apply_calls;
static int g_apply_result;
static DM2_V1_SessionState g_applied_session;

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

static int test_apply_session(void *userdata,
                              const DM2_V1_SessionState *session)
{
    (void)userdata;
    ++g_apply_calls;
    if (session) {
        g_applied_session = *session;
    }
    return g_apply_result;
}

int main(void)
{
    DM2_V1_StartupMenu menu;
    DM2_V1_StartupAction action;
    DM2_V1_StartupActionPlan plan;
    DM2_V1_StartupExecution execution;
    DM2_V1_StartupModeUpdate mode_update;
    DM2_V1_StartupInputOutcome outcome;
    DM2_V1_StartupApplyReceipt receipt;
    DM2_V1_StartupHostReceipt host_receipt;
    DM2_V1_StartupHostActionReceipt host_action_receipt;
    DM2_V1_StartupIdleReceipt idle_receipt;
    DM2_V1_StartupLaunchReceipt launch_receipt;
    DM2_V1_StartupHostFacts host_facts;
    DM2_V1_StartupMenuStateReceipt state_receipt;
    DM2_V1_StartupHit hit;
    DM2_V1_StartupRect panel_rect;
    DM2_V1_StartupRect row_rect;
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
    memset(save_root, 0, sizeof(save_root));
    check(dm2_v1_startup_execute_save_path(
              "/tmp/firestaff-dm2-startup-missing/Other.dat",
              save_root,
              (int)sizeof(save_root),
              &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_STATUS_REDRAW &&
              strcmp(execution.status, "DM2 RESUME PATH INVALID") == 0 &&
              save_root[0] == '\0',
          "direct resume path execution reports invalid save path");
    memset(save_root, 0, sizeof(save_root));
    check(dm2_v1_startup_execute_save_path(
              "/tmp/firestaff-dm2-startup-missing/SKSave03.dat",
              save_root,
              (int)sizeof(save_root),
              &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_STATUS_REDRAW &&
              strcmp(execution.status, "DM2 RESUME FAILED") == 0 &&
              strcmp(save_root, "/tmp/firestaff-dm2-startup-missing") == 0,
          "direct resume path execution reports missing parsed slot");

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
    check(dm2_v1_startup_menu_snapshot_from_facts(
              &snapshot,
              "",
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              99) &&
              strcmp(snapshot.save_root, "/tmp/firestaff-dm2-startup") == 0 &&
              snapshot.resume_available == 1 &&
              snapshot.slot_mask == (1u << 2) &&
              snapshot.row_count == 3 &&
              snapshot.selected_row == 2,
          "snapshot facts helper owns fallback root and selected-row clamp");
    check(dm2_v1_startup_menu_snapshot_scan_saves_from_facts(
              &snapshot,
              "",
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              1,
              "/tmp/firestaff-dm2-startup") &&
              snapshot.resume_available == 0 &&
              snapshot.slot_mask == 0u &&
              snapshot.row_count == 1 &&
              snapshot.selected_row == 0,
          "snapshot facts scan helper owns save scan normalization");
    check(dm2_v1_startup_menu_state_receipt_from_snapshot(
              &snapshot,
              &state_receipt) &&
              strcmp(state_receipt.save_root, "/tmp/firestaff-dm2-startup") == 0 &&
              state_receipt.resume_available == 0 &&
              state_receipt.slot_mask == 0u &&
              state_receipt.row_count == 1 &&
              state_receipt.selected_row == 0,
          "state receipt mirrors normalized startup snapshot for M11");
    check(dm2_v1_startup_menu_state_receipt_from_facts(
              &state_receipt,
              "",
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              99) &&
              strcmp(state_receipt.save_root, "/tmp/firestaff-dm2-startup") == 0 &&
              state_receipt.resume_available == 1 &&
              state_receipt.slot_mask == (1u << 2) &&
              state_receipt.row_count == 3 &&
              state_receipt.selected_row == 2,
          "state receipt facts helper owns M11 selected-row clamp");
    check(dm2_v1_startup_menu_state_receipt_scan_saves_from_facts(
              &state_receipt,
              "",
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              1,
              "/tmp/firestaff-dm2-startup") &&
              state_receipt.resume_available == 0 &&
              state_receipt.slot_mask == 0u &&
              state_receipt.row_count == 1 &&
              state_receipt.selected_row == 0,
          "state receipt scan helper owns M11 save scan copy contract");
    memset(&host_facts, 0, sizeof(host_facts));
    host_facts.save_root = "";
    host_facts.fallback_save_root = "/tmp/firestaff-dm2-startup";
    host_facts.resume_available = 1;
    host_facts.slot_mask = (1u << 2);
    host_facts.selected_row = 1;
    host_facts.scan_save_root = "/tmp/firestaff-dm2-startup";
    check(dm2_v1_startup_menu_state_receipt_scan_saves_from_host_facts(
              &state_receipt,
              &host_facts) &&
              state_receipt.resume_available == 0 &&
              state_receipt.slot_mask == 0u &&
              state_receipt.row_count == 1 &&
              state_receipt.selected_row == 0,
          "state receipt host facts scan helper owns M11 save scan copy contract");
    check(dm2_v1_startup_launch_from_host_facts_with_receipt(
              &host_facts,
              &launch_receipt) &&
              launch_receipt.session_valid &&
              launch_receipt.session.champion_count == 4 &&
              launch_receipt.menu_state_receipt_valid &&
              launch_receipt.menu_state_receipt.row_count == 1 &&
              launch_receipt.host_receipt.mode_update.set_startup_menu_active &&
              launch_receipt.host_receipt.mode_update.startup_menu_active == 1 &&
              launch_receipt.host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              strcmp(launch_receipt.host_receipt.status, "DM2 START MENU") == 0,
          "launch receipt owns DM2 startup session, save scan, and active menu");
    check(!dm2_v1_startup_launch_from_host_facts_with_receipt(
              NULL,
              &launch_receipt) &&
              launch_receipt.host_receipt.status_scope &&
              strcmp(launch_receipt.host_receipt.status_scope, "BOOT") == 0 &&
              launch_receipt.host_receipt.status &&
              strcmp(launch_receipt.host_receipt.status,
                     "DM2 START MENU FAILED") == 0,
          "launch receipt owns DM2 startup menu failure status");
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
    check(dm2_v1_startup_execution_mode_update(&execution, &mode_update) &&
              !mode_update.set_startup_menu_active,
          "failed Continue redraw owns no-op startup mode update");
    check(dm2_v1_startup_execution_input_outcome(&execution, 0, &outcome) &&
              outcome.result == DM2_V1_STARTUP_INPUT_RESULT_REDRAW &&
              outcome.rescan_saves == 1 &&
              strcmp(outcome.status_scope, "STARTUP") == 0 &&
              strcmp(outcome.status, "DM2 CONTINUE FAILED") == 0,
          "failed Continue execution owns redraw input outcome");
    check(dm2_v1_startup_apply_receipt_from_execution(
              &execution, 0, &receipt) &&
              !receipt.session_should_apply &&
              !receipt.mode_update.set_startup_menu_active &&
              receipt.outcome.result == DM2_V1_STARTUP_INPUT_RESULT_REDRAW &&
              receipt.outcome.rescan_saves == 1,
          "failed Continue receipt owns redraw and rescan policy");
    check(dm2_v1_startup_host_receipt_from_apply_receipt(
              &receipt, &host_receipt) &&
              host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              host_receipt.rescan_saves == 1 &&
              !host_receipt.mode_update.set_startup_menu_active,
          "failed Continue host receipt owns M11 result and rescan policy");
    memset(&host_facts, 0, sizeof(host_facts));
    host_facts.save_root = "/tmp/firestaff-dm2-startup-missing";
    host_facts.fallback_save_root = "/tmp/firestaff-dm2-startup-missing";
    host_facts.resume_available = 1;
    host_facts.slot_mask = (1u << 2);
    host_facts.selected_row = 0;
    host_facts.scan_save_root = "/tmp/firestaff-dm2-startup-missing";
    check(dm2_v1_startup_execute_action_from_host_facts_with_receipt(
              &action,
              &host_facts,
              test_apply_session,
              NULL,
              &execution,
              &host_action_receipt) &&
              host_action_receipt.host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              host_action_receipt.host_receipt.rescan_saves == 1 &&
              host_action_receipt.menu_state_receipt_valid &&
              host_action_receipt.menu_state_receipt.row_count == 1 &&
              host_action_receipt.menu_state_receipt.selected_row == 0,
          "host action receipt owns failed Continue rescan state for M11");
    check(dm2_v1_startup_menu_handle_input(
              &menu, DM2_V1_STARTUP_INPUT_DOWN, &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE &&
              menu.selected_row == 1,
          "keyboard Down returns navigation action directly");
    check(dm2_v1_startup_menu_snapshot_from_menu(&snapshot, &menu) &&
              dm2_v1_startup_menu_snapshot_handle_firestaff_input(
                  &snapshot, 1, &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE &&
              snapshot.selected_row == 0,
          "snapshot Firestaff input helper owns input-code mapping and action");
    check(dm2_v1_startup_menu_handle_firestaff_input_from_facts(
              &snapshot,
              "",
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              0,
              2,
              &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE &&
              snapshot.save_root[0] == '/' &&
              snapshot.row_count == 3 &&
              snapshot.selected_row == 1,
          "snapshot facts input helper owns M11 input snapshot construction");
    check(dm2_v1_startup_menu_handle_firestaff_input_from_facts_with_receipt(
              &state_receipt,
              "",
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              0,
              2,
              &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE &&
              strcmp(state_receipt.save_root, "/tmp/firestaff-dm2-startup") == 0 &&
              state_receipt.row_count == 3 &&
              state_receipt.selected_row == 1,
          "state receipt input helper owns M11 input state copy contract");
    host_facts.resume_available = 1;
    host_facts.slot_mask = (1u << 2);
    host_facts.selected_row = 0;
    check(dm2_v1_startup_menu_handle_firestaff_input_from_host_facts_with_receipt(
              &state_receipt,
              &host_facts,
              2,
              &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE &&
              state_receipt.row_count == 3 &&
              state_receipt.selected_row == 1,
          "state receipt host facts input helper owns M11 input state copy contract");
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
    check(dm2_v1_startup_execution_mode_update(&execution, &mode_update) &&
              mode_update.set_startup_menu_active &&
              mode_update.startup_menu_active == 0,
          "session-ready execution owns startup-menu close update");
    check(dm2_v1_startup_execution_input_outcome(&execution, 1, &outcome) &&
              outcome.result == DM2_V1_STARTUP_INPUT_RESULT_REDRAW &&
              outcome.rescan_saves == 0 &&
              strcmp(outcome.status_scope, "STARTUP") == 0 &&
              strcmp(outcome.status, "DM2 NEW GAME") == 0,
          "session-ready execution owns successful redraw input outcome");
    check(dm2_v1_startup_apply_receipt_from_execution(
              &execution, 1, &receipt) &&
              receipt.session_should_apply &&
              receipt.session_applied &&
              receipt.mode_update.set_startup_menu_active &&
              receipt.mode_update.startup_menu_active == 0 &&
              receipt.outcome.result == DM2_V1_STARTUP_INPUT_RESULT_REDRAW &&
              strcmp(receipt.outcome.status, "DM2 NEW GAME") == 0,
          "session-ready receipt closes startup after applied session");
    g_apply_calls = 0;
    g_apply_result = 1;
    memset(&g_applied_session, 0, sizeof(g_applied_session));
    check(dm2_v1_startup_execute_action_with_receipt(
              &action,
              "/tmp/firestaff-dm2-startup-missing",
              test_apply_session,
              NULL,
              &execution,
              &receipt) &&
              g_apply_calls == 1 &&
              g_applied_session.champion_count == 4 &&
              receipt.session_should_apply &&
              receipt.session_applied &&
              receipt.mode_update.set_startup_menu_active &&
              receipt.mode_update.startup_menu_active == 0 &&
              strcmp(receipt.outcome.status, "DM2 NEW GAME") == 0,
          "combined action execution applies session and returns close receipt");
    g_apply_calls = 0;
    g_apply_result = 1;
    memset(&g_applied_session, 0, sizeof(g_applied_session));
    check(dm2_v1_startup_execute_action_with_host_receipt(
              &action,
              "/tmp/firestaff-dm2-startup-missing",
              test_apply_session,
              NULL,
              &execution,
              &host_receipt) &&
              g_apply_calls == 1 &&
              g_applied_session.champion_count == 4 &&
              host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              host_receipt.mode_update.set_startup_menu_active &&
              host_receipt.mode_update.startup_menu_active == 0 &&
              strcmp(host_receipt.status, "DM2 NEW GAME") == 0,
          "combined action execution can return M11-ready host receipt directly");
    host_facts.resume_available = 1;
    host_facts.slot_mask = (1u << 2);
    host_facts.selected_row = 2;
    g_apply_calls = 0;
    g_apply_result = 1;
    memset(&g_applied_session, 0, sizeof(g_applied_session));
    check(dm2_v1_startup_execute_firestaff_input_from_host_facts_with_receipt(
              &host_facts,
              9,
              test_apply_session,
              NULL,
              &execution,
              &host_action_receipt) &&
              g_apply_calls == 1 &&
              g_applied_session.champion_count == 4 &&
              host_action_receipt.host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              host_action_receipt.host_receipt.mode_update.set_startup_menu_active &&
              host_action_receipt.host_receipt.mode_update.startup_menu_active == 0 &&
              host_action_receipt.menu_state_receipt_valid &&
              host_action_receipt.menu_state_receipt.selected_row == 2,
          "host facts keyboard wrapper executes startup action and returns M11 receipt");
    check(dm2_v1_startup_execution_input_outcome(&execution, 0, &outcome) &&
              outcome.result == DM2_V1_STARTUP_INPUT_RESULT_REDRAW &&
              strcmp(outcome.status_scope, "STARTUP") == 0 &&
              strcmp(outcome.status, "DM2 LOAD FAILED") == 0,
          "session-ready execution owns failed apply input outcome");
    check(dm2_v1_startup_apply_receipt_from_execution(
              &execution, 0, &receipt) &&
              receipt.session_should_apply &&
              !receipt.session_applied &&
              !receipt.mode_update.set_startup_menu_active &&
              strcmp(receipt.outcome.status, "DM2 LOAD FAILED") == 0,
          "session-ready receipt keeps startup open after failed apply");
    g_apply_calls = 0;
    g_apply_result = 0;
    memset(&g_applied_session, 0, sizeof(g_applied_session));
    check(dm2_v1_startup_execute_action_with_receipt(
              &action,
              "/tmp/firestaff-dm2-startup-missing",
              test_apply_session,
              NULL,
              NULL,
              &receipt) &&
              g_apply_calls == 1 &&
              receipt.session_should_apply &&
              !receipt.session_applied &&
              !receipt.mode_update.set_startup_menu_active &&
              strcmp(receipt.outcome.status, "DM2 LOAD FAILED") == 0,
          "combined action execution keeps startup open when session apply fails");
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
    check(dm2_v1_startup_execution_mode_update(&execution, &mode_update) &&
              !mode_update.set_startup_menu_active,
          "launcher-return execution owns no-op startup mode update");
    check(dm2_v1_startup_execution_input_outcome(&execution, 0, &outcome) &&
              outcome.result ==
                  DM2_V1_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER &&
              outcome.rescan_saves == 0 &&
              strcmp(outcome.status_scope, "RETURN") == 0 &&
              strcmp(outcome.status, "BACK TO LAUNCHER") == 0,
          "launcher-return execution owns return input outcome");
    check(dm2_v1_startup_apply_receipt_from_execution(
              &execution, 0, &receipt) &&
              !receipt.session_should_apply &&
              receipt.outcome.result ==
                  DM2_V1_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER &&
              strcmp(receipt.outcome.status, "BACK TO LAUNCHER") == 0,
          "launcher-return receipt owns return policy");

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
    check(dm2_v1_startup_panel_rect(&panel_rect) &&
              dm2_v1_startup_menu_snapshot_from_menu(&snapshot, &menu) &&
              dm2_v1_startup_menu_snapshot_handle_pointer(
                  &snapshot,
                  snapshot.row_count,
                  panel_rect.x + 8,
                  panel_rect.y + 8,
                  &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE,
          "snapshot pointer helper owns pointer hit-test and action");
    check(dm2_v1_startup_menu_handle_pointer_from_facts(
              &snapshot,
              "",
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              0,
              panel_rect.x + 8,
              panel_rect.y + 8,
              &action) &&
              snapshot.save_root[0] == '/' &&
              snapshot.row_count == 3 &&
              snapshot.selected_row == 0 &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE,
          "snapshot facts pointer helper owns M11 pointer snapshot construction");
    check(dm2_v1_startup_menu_handle_pointer_from_facts_with_receipt(
              &state_receipt,
              "",
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              0,
              panel_rect.x + 8,
              panel_rect.y + 8,
              &action) &&
              strcmp(state_receipt.save_root, "/tmp/firestaff-dm2-startup") == 0 &&
              state_receipt.row_count == 3 &&
              state_receipt.selected_row == 0 &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE,
          "state receipt pointer helper owns M11 pointer state copy contract");
    host_facts.resume_available = 1;
    host_facts.slot_mask = (1u << 2);
    host_facts.selected_row = 0;
    check(dm2_v1_startup_menu_handle_pointer_from_host_facts_with_receipt(
              &state_receipt,
              &host_facts,
              panel_rect.x + 8,
              panel_rect.y + 8,
              &action) &&
              state_receipt.row_count == 3 &&
              state_receipt.selected_row == 0 &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE,
          "state receipt host facts pointer helper owns M11 pointer state copy contract");
    host_facts.resume_available = 1;
    host_facts.slot_mask = (1u << 2);
    host_facts.selected_row = 0;
    g_apply_calls = 0;
    g_apply_result = 1;
    memset(&g_applied_session, 0, sizeof(g_applied_session));
    check(dm2_v1_startup_row_rect(2, &row_rect) &&
              dm2_v1_startup_execute_pointer_from_host_facts_with_receipt(
                  &host_facts,
                  row_rect.x + 4,
                  row_rect.y + 4,
                  test_apply_session,
                  NULL,
                  &execution,
                  &host_action_receipt) &&
              g_apply_calls == 1 &&
              host_action_receipt.host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              host_action_receipt.host_receipt.mode_update.set_startup_menu_active &&
              host_action_receipt.menu_state_receipt_valid &&
              host_action_receipt.menu_state_receipt.selected_row == 2,
          "host facts pointer wrapper executes startup action and returns M11 receipt");
    check(!dm2_v1_startup_menu_handle_input(
              &menu, DM2_V1_STARTUP_INPUT_NONE, &action),
          "idle input is ignored");
    host_facts.startup_menu_active = 1;
    check(dm2_v1_startup_advance_idle_from_host_facts_with_receipt(
              &host_facts, 1, &idle_receipt) &&
              idle_receipt.host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              strcmp(idle_receipt.host_receipt.status, "DM2 STARTUP MENU") == 0,
          "startup idle receipt owns DM2 menu redraw policy");
    check(dm2_v1_startup_advance_idle_from_host_facts_with_receipt(
              &host_facts, 0, &idle_receipt) &&
              idle_receipt.host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_IGNORED,
          "startup idle receipt owns DM2 menu no-redraw policy");
    host_facts.startup_menu_active = 0;
    check(!dm2_v1_startup_advance_idle_from_host_facts_with_receipt(
              &host_facts, 1, &idle_receipt),
          "startup idle receipt rejects inactive DM2 menu");
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
