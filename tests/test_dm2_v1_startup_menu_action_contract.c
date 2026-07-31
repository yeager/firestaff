#include "dm2_v1_startup_menu.h"
#include "dm2_v1_startup_presentation.h"
#include "dm2_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
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
    DM2_V1_StartupDirectResumeReceipt direct_resume_receipt;
    DM2_V1_StartupHostFacts host_facts;
    DM2_V1_StartupMenuStateReceipt state_receipt;
    DM2_V1_StartupHit hit;
    DM2_V1_StartupRect panel_rect;
    DM2_V1_StartupRect row_rect;
    DM2_V1_StartupMenuSnapshot snapshot;
    DM2_V1_StartupRowKind row_kind;
    DM2_V1_StartupRenderRow rows[4];
    DM2_V1_StartupDrawCommand commands[16];
    DM2_V1_StartupRenderReceipt render_receipt;
    DM2_V1_StartupViewReceipt view_receipt;
    DM2_V1_StartupRuntimeHandoffReceipt runtime_handoff;
    DM2_V1_BootRuntimeStartupSnapshot boot_snapshot;
    DM2_V1_BootStartupHostViewReceipt boot_host_view_receipt;
    DM2_V1_BootStartupPackagedFullStartReceipt boot_full_start_package;
    DM2_V1_BootStartupPackagedConsumerReceipt boot_consumer_receipt;
    DM2_V1_BootStartupHostFrameReceipt boot_host_frame_receipt;
    DM2_V1_BootStartupRenderOwnershipReceipt boot_render_ownership_receipt;
    DM2_V1_BootStartupRealVisualCaptureReceipt boot_real_visual_capture;
    DM2_V1_BootStartupLaunch boot_launch;
    DM2_V1_BootProfile boot_launch_profile;
    DM2_V1_SessionState direct_session;
    DM2_V1_StartupSavePathResult save_path_result;
    const char *real_data_dir;
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
    check(dm2_v1_startup_runtime_handoff_receipt_from_state(
              &runtime_handoff, 1, 1) &&
              runtime_handoff.valid &&
              runtime_handoff.startup_menu_active == 1 &&
              strcmp(runtime_handoff.animation, "dm2-startup-menu") == 0 &&
              runtime_handoff.title_animation_tick == 0 &&
              runtime_handoff.title_frame == 0 &&
              runtime_handoff.animation_active == 1 &&
              runtime_handoff.title_frame_max == 0 &&
              runtime_handoff.title_frame_duration_ticks == 0 &&
              runtime_handoff.title_ready == 1 &&
              runtime_handoff.runtime_menu_ready == 1 &&
              runtime_handoff.runtime_action_ready == 0 &&
              runtime_handoff.first_hud_frame_ready == 0,
          "runtime handoff helper owns startup menu readiness");
    check(dm2_v1_startup_runtime_handoff_receipt_from_tick(
              &runtime_handoff, 1, 1, 13) &&
              runtime_handoff.valid &&
              runtime_handoff.animation_active == 1 &&
              runtime_handoff.title_animation_tick == 0 &&
              runtime_handoff.title_frame == 0 &&
              runtime_handoff.title_frame_max == 0 &&
              runtime_handoff.title_frame_duration_ticks == 0 &&
              runtime_handoff.title_ready == 1,
          "runtime handoff keeps the static SKProject menu timing");
    check(dm2_v1_startup_runtime_handoff_receipt_from_state(
              &runtime_handoff, 0, 1) &&
              runtime_handoff.valid &&
              runtime_handoff.startup_menu_active == 0 &&
              strcmp(runtime_handoff.animation, "dm2-runtime") == 0 &&
              runtime_handoff.title_animation_tick == 0 &&
              runtime_handoff.title_frame == 0 &&
              runtime_handoff.title_frame_max == 0 &&
              runtime_handoff.title_frame_duration_ticks == 0 &&
              runtime_handoff.runtime_menu_ready == 0 &&
              runtime_handoff.runtime_action_ready == 1 &&
              runtime_handoff.first_hud_frame_ready == 1,
          "runtime handoff helper owns first HUD-frame readiness");

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
    check(dm2_v1_startup_resume_status_host_receipt(
              DM2_V1_STARTUP_RESUME_STATUS_PATH_INVALID,
              &host_receipt) &&
              strcmp(host_receipt.status_scope, "BOOT") == 0 &&
              strcmp(host_receipt.status, "DM2 RESUME PATH INVALID") == 0,
          "direct resume host receipt owns invalid-path status");
    check(dm2_v1_startup_resume_status_host_receipt(
              DM2_V1_STARTUP_RESUME_STATUS_FAILED,
              &host_receipt) &&
              strcmp(host_receipt.status_scope, "BOOT") == 0 &&
              strcmp(host_receipt.status, "DM2 RESUME FAILED") == 0,
          "direct resume host receipt owns failed status");
    check(dm2_v1_startup_resume_status_host_receipt(
              DM2_V1_STARTUP_RESUME_STATUS_RESUMED,
              &host_receipt) &&
              host_receipt.input_result == DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              strcmp(host_receipt.status, "DM2 RESUMED") == 0 &&
              host_receipt.inspect_scope &&
              strcmp(host_receipt.inspect_scope, "READY") == 0 &&
              host_receipt.log_line &&
              strcmp(host_receipt.log_line, "T0: DM2 RESUMED") == 0,
          "direct resume host receipt owns resumed redraw status, inspect, and log");
    memset(save_root, 0, sizeof(save_root));
    check(dm2_v1_startup_execute_save_path_with_host_receipt(
              "/tmp/firestaff-dm2-startup-missing/Other.dat",
              test_apply_session,
              NULL,
              &execution,
              &direct_resume_receipt) &&
              !direct_resume_receipt.session_ready &&
              !direct_resume_receipt.session_applied &&
              !direct_resume_receipt.save_root_valid &&
              strcmp(direct_resume_receipt.host_receipt.status_scope, "BOOT") == 0 &&
              strcmp(direct_resume_receipt.host_receipt.status,
                     "DM2 RESUME PATH INVALID") == 0,
          "direct resume wrapper owns invalid-path host receipt");
    check(dm2_v1_startup_execute_save_path_with_host_receipt(
              "/tmp/firestaff-dm2-startup-missing/SKSave03.dat",
              test_apply_session,
              NULL,
              &execution,
              &direct_resume_receipt) &&
              !direct_resume_receipt.session_ready &&
              !direct_resume_receipt.session_applied &&
              direct_resume_receipt.save_root_valid &&
              strcmp(direct_resume_receipt.save_root,
                     "/tmp/firestaff-dm2-startup-missing") == 0 &&
              strcmp(direct_resume_receipt.host_receipt.status_scope, "BOOT") == 0 &&
              strcmp(direct_resume_receipt.host_receipt.status,
                     "DM2 RESUME FAILED") == 0,
          "direct resume wrapper owns missing-slot host receipt and parsed save root");

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
              !launch_receipt.session_valid &&
              launch_receipt.menu_state_receipt_valid &&
              launch_receipt.menu_state_receipt.row_count == 1 &&
              launch_receipt.host_receipt.mode_update.set_startup_menu_active &&
              launch_receipt.host_receipt.mode_update.startup_menu_active == 1 &&
              launch_receipt.host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              strcmp(launch_receipt.host_receipt.status, "DM2 START MENU") == 0 &&
              launch_receipt.host_receipt.inspect_scope &&
              strcmp(launch_receipt.host_receipt.inspect_scope, "READY") == 0 &&
              launch_receipt.host_receipt.inspect_detail &&
              strstr(launch_receipt.host_receipt.inspect_detail,
                     "DM2 V1 ASSETS VERIFIED") != NULL &&
              launch_receipt.host_receipt.log_line &&
              strcmp(launch_receipt.host_receipt.log_line,
                     "T0: DM2 START MENU") == 0 &&
              launch_receipt.runtime_handoff.valid &&
              launch_receipt.runtime_handoff.startup_menu_active == 1 &&
              launch_receipt.runtime_handoff.animation_active == 1 &&
              strcmp(launch_receipt.runtime_handoff.animation,
                     "dm2-startup-menu") == 0 &&
              launch_receipt.runtime_handoff.title_frame == 0 &&
              launch_receipt.runtime_handoff.title_frame_max == 0 &&
              launch_receipt.runtime_handoff.title_frame_duration_ticks == 0 &&
              launch_receipt.runtime_handoff.title_ready == 1 &&
              launch_receipt.runtime_handoff.initialize_v2_runtime == 1 &&
              launch_receipt.runtime_handoff.initialize_hud_runtime == 1 &&
              launch_receipt.runtime_handoff.initialize_touch_runtime == 1 &&
              launch_receipt.runtime_handoff.hud_runtime_ready == 1 &&
              launch_receipt.runtime_handoff.runtime_menu_ready == 1 &&
              launch_receipt.runtime_handoff.runtime_action_ready == 0 &&
              launch_receipt.runtime_handoff.first_hud_frame_ready == 0,
          "launch receipt owns the source title/menu boundary, save scan, inspect, log, and runtime handoff without a session");
    memset(&boot_launch, 0, sizeof(boot_launch));
    dm2_v1_boot_profile_init(&boot_launch_profile);
    boot_launch.profile = &boot_launch_profile;
    check(dm2_v1_boot_startup_launch_from_launch(&boot_launch,
                                                 &launch_receipt) &&
              launch_receipt.host_receipt.mode_update.set_startup_menu_active &&
              launch_receipt.host_receipt.mode_update.startup_menu_active == 1 &&
              launch_receipt.runtime_handoff.valid &&
              launch_receipt.runtime_handoff.startup_menu_active == 1 &&
              launch_receipt.runtime_handoff.runtime_action_ready == 0 &&
              launch_receipt.runtime_handoff.first_hud_frame_ready == 0,
          "boot launch wrapper keeps skproject SHOW_MENU_SCREEN active before GAME_LOAD");
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
    row_count = dm2_v1_startup_presentation_build(
        &menu,
        commands,
        (int)(sizeof(commands) / sizeof(commands[0])));
    check(row_count == 2 &&
              commands[0].kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE &&
              commands[0].gdat_category == DM2_GDAT_CATEGORY_TITLE &&
              commands[0].gdat_index == 0 &&
              commands[0].gdat_field == 4 &&
              commands[0].frame_owner == DM2_V1_FRAME_OWNER_STARTUP_TITLE &&
              commands[1].kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE &&
              commands[1].gdat_category == DM2_GDAT_CATEGORY_TITLE &&
              commands[1].gdat_index == 0 &&
              commands[1].gdat_field == 4 &&
              commands[1].frame_owner == DM2_V1_FRAME_OWNER_STARTUP_MENU &&
              dm2_v1_startup_presentation_render_receipt(
                  &menu, commands, row_count, 1, &render_receipt) &&
              render_receipt.valid &&
              render_receipt.command_count == row_count &&
              render_receipt.row_count == 3 &&
              render_receipt.selected_row == 0 &&
              render_receipt.title_gdat_found &&
              render_receipt.title_gdat_category == 0x05 &&
              render_receipt.title_gdat_index == 0 &&
              render_receipt.title_gdat_field == 4 &&
              render_receipt.menu_gdat_found &&
              render_receipt.menu_gdat_category == 0x05 &&
              render_receipt.menu_gdat_index == 0 &&
              render_receipt.menu_gdat_field == 4 &&
              render_receipt.skproject_title_query_ready == 1 &&
              render_receipt.skproject_menu_query_ready == 1 &&
              render_receipt.skproject_title_category == 0x05 &&
              render_receipt.skproject_title_index == 0 &&
              render_receipt.skproject_credit_screen_field == 1 &&
              render_receipt.skproject_menu_screen_field == 4 &&
              render_receipt.title_rect.w == 320 &&
              render_receipt.title_rect.h == 200 &&
              render_receipt.menu_rect.w == 320 &&
              render_receipt.menu_rect.h == 200 &&
              render_receipt.panel_rect.w == 0 &&
              render_receipt.panel_rect.h == 0 &&
              render_receipt.menu_text_count == 0 &&
              render_receipt.selectable_text_count == 0 &&
              render_receipt.selected_highlight_count == 0 &&
              render_receipt.hud_runtime_ready == 1 &&
              render_receipt.hud_overlay_suppressed == 1 &&
              render_receipt.title_backdrop_ready == 1 &&
              render_receipt.resume_menu_ready == 1 &&
              render_receipt.save_slot_menu_ready == 1 &&
              render_receipt.new_game_menu_ready == 1 &&
              render_receipt.full_start_graphics_ready == 1,
          "startup presentation emits title and SHOW_MENU_SCREEN GDAT commands");
    host_facts.startup_menu_active = 1;
    check(dm2_v1_startup_presentation_view_receipt_from_host_facts(
              &host_facts,
              1,
              commands,
              (int)(sizeof(commands) / sizeof(commands[0])),
              &view_receipt) &&
              view_receipt.valid &&
              view_receipt.command_count == view_receipt.render.command_count &&
              view_receipt.menu_state.row_count == 3 &&
              view_receipt.menu_state.selected_row == 1 &&
              view_receipt.render.title_gdat_found &&
              view_receipt.render.skproject_title_query_ready == 1 &&
              view_receipt.render.skproject_menu_query_ready == 1 &&
              view_receipt.render.title_rect.w == 320 &&
              view_receipt.render.title_rect.h == 200 &&
              view_receipt.render.selectable_text_count == 0 &&
              view_receipt.render.selected_highlight_count == 0 &&
              view_receipt.render.hud_overlay_suppressed == 1 &&
              view_receipt.render.title_backdrop_ready == 1 &&
              view_receipt.render.resume_menu_ready == 1 &&
              view_receipt.render.save_slot_menu_ready == 1 &&
              view_receipt.render.new_game_menu_ready == 1 &&
              view_receipt.render.full_start_graphics_ready == 1 &&
              view_receipt.runtime_handoff.valid &&
              view_receipt.runtime_handoff.startup_menu_active == 1 &&
              view_receipt.runtime_handoff.animation_active == 1 &&
              strcmp(view_receipt.runtime_handoff.animation,
                     "dm2-startup-menu") == 0 &&
              view_receipt.runtime_handoff.title_ready == 1 &&
              view_receipt.runtime_handoff.initialize_hud_runtime == 1 &&
              view_receipt.runtime_handoff.hud_runtime_ready == 1 &&
              view_receipt.runtime_handoff.runtime_menu_ready == 1 &&
              view_receipt.runtime_handoff.runtime_action_ready == 0 &&
              view_receipt.runtime_handoff.first_hud_frame_ready == 0,
          "startup view receipt joins title commands, menu state, and HUD handoff");
    memset(&boot_snapshot, 0, sizeof(boot_snapshot));
    boot_snapshot.startup_menu_active = 1;
    boot_snapshot.startup_save_root = "/tmp/firestaff-dm2-startup";
    boot_snapshot.resume_available = 1;
    boot_snapshot.slot_mask = (1u << 2);
    boot_snapshot.selected_row = 1;
    check(dm2_v1_boot_startup_host_view_receipt_from_snapshot(
              &boot_snapshot,
              &boot_host_view_receipt) &&
              boot_host_view_receipt.valid &&
              boot_host_view_receipt.title_timing_ready == 1 &&
              boot_host_view_receipt.title_animation_tick == 0 &&
              boot_host_view_receipt.title_frame == 0 &&
              boot_host_view_receipt.title_frame_max == 0 &&
              boot_host_view_receipt.title_frame_duration_ticks == 0 &&
              boot_host_view_receipt.title_cycle_ticks == 1 &&
              boot_host_view_receipt.title_cycle_position_tick == 0 &&
              boot_host_view_receipt.title_frame_start_tick == 0 &&
              boot_host_view_receipt.title_next_frame_tick == 1 &&
              boot_host_view_receipt.title_frame_elapsed_ticks == 0 &&
              boot_host_view_receipt.title_frame_remaining_ticks == 1 &&
              boot_host_view_receipt.title_cycle_remaining_ticks == 1 &&
              boot_host_view_receipt.exact_title_timing_ready == 1 &&
              boot_host_view_receipt.menu_row_count == 3 &&
              boot_host_view_receipt.menu_text_count == 0 &&
              boot_host_view_receipt.selectable_text_count == 0 &&
              boot_host_view_receipt.selected_highlight_count == 0 &&
              boot_host_view_receipt.menu_panel_ready == 1 &&
              boot_host_view_receipt.startup_menu_assets_ready == 1 &&
              boot_host_view_receipt.startup_hud_handoff_ready == 1 &&
              boot_host_view_receipt.m11_host_view_ready == 1 &&
              boot_host_view_receipt.capture_proof_valid == 1 &&
              boot_host_view_receipt.capture_proof.valid == 1 &&
              boot_host_view_receipt.capture_proof.title_capture_ready == 0 &&
              boot_host_view_receipt.capture_proof.menu_capture_ready == 1 &&
              boot_host_view_receipt.capture_proof.hud_handoff_capture_ready == 1 &&
              boot_host_view_receipt.capture_proof.packaged_capture_hash != 0u,
          "boot host-view receipt owns exact title timing and menu asset proof");
    check(dm2_v1_boot_startup_packaged_full_start_receipt_from_snapshot(
              &boot_snapshot,
              &boot_full_start_package) &&
              boot_full_start_package.valid &&
              boot_full_start_package.full_start_valid == 1 &&
              boot_full_start_package.capture_proof_valid == 1 &&
              boot_full_start_package.packaged_full_start_hash != 0u &&
              boot_full_start_package.title_animation_tick == 0 &&
              boot_full_start_package.title_frame_start_tick == 0 &&
              boot_full_start_package.title_next_frame_tick == 1 &&
              boot_full_start_package.title_frame_remaining_ticks == 1 &&
              boot_full_start_package.capture_proof.title_ready == 1 &&
              boot_full_start_package.title_ready == 1 &&
              boot_full_start_package.menu_capture_ready == 1 &&
              boot_full_start_package.hud_handoff_capture_ready == 1 &&
              boot_full_start_package.runtime_menu_ready == 1 &&
              boot_full_start_package.runtime_action_ready == 0 &&
              boot_full_start_package.first_hud_frame_ready == 0 &&
              boot_full_start_package.m11_consumer_ready == 1 &&
              boot_full_start_package.full_start.valid == 1 &&
              boot_full_start_package.capture_proof.valid == 1,
          "boot packaged full-start receipt joins full-start and capture proof");
    check(dm2_v1_boot_startup_packaged_consumer_receipt_from_snapshot(
              &boot_snapshot,
              &boot_consumer_receipt) &&
              boot_consumer_receipt.valid &&
              boot_consumer_receipt.packaged_full_start_valid == 1 &&
              boot_consumer_receipt.packaged_full_start_hash ==
                  boot_full_start_package.packaged_full_start_hash &&
              boot_consumer_receipt.startup_active == 1 &&
              boot_consumer_receipt.startup_animation_active == 1 &&
              boot_consumer_receipt.startup_title_frame == 0 &&
              boot_consumer_receipt.startup_title_frame_max == 0 &&
              boot_consumer_receipt.startup_title_ready == 1 &&
              boot_consumer_receipt.startup_hud_runtime_ready == 1 &&
              boot_consumer_receipt.packaged_title_timing_consumed == 1 &&
              boot_consumer_receipt.packaged_first_hud_receipt_consumed == 1 &&
              boot_consumer_receipt.m11_startup_receipt_ready == 1 &&
              boot_consumer_receipt.runtime_menu_ready == 1 &&
              boot_consumer_receipt.runtime_action_ready == 0 &&
              boot_consumer_receipt.first_hud_frame_ready == 0 &&
              boot_consumer_receipt.startup_draw_ready == 1 &&
              boot_consumer_receipt.startup_draw_command_count ==
                  boot_full_start_package.command_count &&
              boot_consumer_receipt.startup_draw_menu_capture_ready == 1 &&
              boot_consumer_receipt.startup_draw_hud_handoff_ready == 1 &&
              strcmp(boot_consumer_receipt.phase, "dm2-startup-menu") == 0,
          "boot packaged consumer receipt gives M11 startup draw gate");
    check(dm2_v1_boot_startup_host_frame_receipt_from_snapshot(
              &boot_snapshot,
              &boot_host_frame_receipt) &&
              boot_host_frame_receipt.valid &&
              boot_host_frame_receipt.consume_startup_package == 1 &&
              boot_host_frame_receipt.render_startup_title == 1 &&
              boot_host_frame_receipt.render_startup_menu == 1 &&
              boot_host_frame_receipt.suppress_game_hud == 1 &&
              boot_host_frame_receipt.enable_runtime_input == 0 &&
              boot_host_frame_receipt.present_first_hud_frame == 0 &&
              boot_host_frame_receipt.schedule_next_title_tick == 0 &&
              boot_host_frame_receipt.next_title_tick_delta == 0 &&
              boot_host_frame_receipt.title_animation_tick == 0 &&
              boot_host_frame_receipt.title_frame == 0 &&
              boot_host_frame_receipt.title_frame_remaining_ticks == 1 &&
              boot_host_frame_receipt.startup_draw_command_count ==
                  boot_consumer_receipt.startup_draw_command_count &&
              boot_host_frame_receipt.packaged_full_start_hash ==
                  boot_consumer_receipt.packaged_full_start_hash &&
              strcmp(boot_host_frame_receipt.phase,
                     "dm2-startup-menu") == 0,
          "boot host-frame receipt owns startup title/menu/HUD decision");
    check(dm2_v1_boot_startup_render_ownership_receipt_from_snapshot(
              &boot_snapshot,
              &boot_render_ownership_receipt) &&
              boot_render_ownership_receipt.valid &&
              boot_render_ownership_receipt.packaged_full_start_valid == 1 &&
              boot_render_ownership_receipt.host_frame_valid == 1 &&
              boot_render_ownership_receipt.consume_host_frame_receipt == 1 &&
              boot_render_ownership_receipt.execute_startup_draw_commands == 1 &&
              boot_render_ownership_receipt.draw_command_count ==
                  boot_consumer_receipt.startup_draw_command_count &&
              boot_render_ownership_receipt.executed_command_count ==
                  boot_render_ownership_receipt.draw_command_count &&
              boot_render_ownership_receipt.executed_gdat_image_count == 2 &&
              boot_render_ownership_receipt.title_gdat_command_count == 1 &&
              boot_render_ownership_receipt.menu_gdat_command_count == 1 &&
              boot_render_ownership_receipt.executed_rect_count == 0 &&
              boot_render_ownership_receipt.executed_text_count == 0 &&
              boot_render_ownership_receipt.title_gdat_asset_required == 0 &&
              boot_render_ownership_receipt.title_gdat_asset_consumed == 0 &&
              boot_render_ownership_receipt.fallback_title_blit_used == 0 &&
              boot_render_ownership_receipt.final_m11_draw_caller_ready == 1 &&
              boot_render_ownership_receipt.final_m11_draw_caller_consumes_ownership == 1 &&
              boot_render_ownership_receipt.packaged_draw_commands_consumed == 1 &&
              boot_render_ownership_receipt.title_timing_receipt_consumed == 1 &&
              boot_render_ownership_receipt.real_gdat_title_asset_receipt_breadth == 1 &&
              boot_render_ownership_receipt.menu_hud_startup_receipt_breadth == 1 &&
              boot_render_ownership_receipt.suppress_game_hud == 1 &&
              boot_render_ownership_receipt.present_first_hud_frame == 0 &&
              boot_render_ownership_receipt.schedule_next_title_tick == 0 &&
              boot_render_ownership_receipt.next_title_tick_delta == 0 &&
              boot_render_ownership_receipt.packaged_full_start_hash ==
                  boot_consumer_receipt.packaged_full_start_hash,
          "boot render-ownership receipt consumes host frame and GDAT title draw contract");
    check(dm2_v1_boot_startup_host_view_receipt_from_runtime_state(
              NULL,
              1,
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              1,
              13,
              &boot_host_view_receipt) &&
              boot_host_view_receipt.valid &&
              boot_host_view_receipt.title_animation_tick == 0 &&
              boot_host_view_receipt.title_frame == 0 &&
              boot_host_view_receipt.title_frame_elapsed_ticks == 0 &&
              boot_host_view_receipt.title_frame_remaining_ticks == 1 &&
              boot_host_view_receipt.m11_host_view_ready == 1 &&
              boot_host_view_receipt.capture_proof_valid == 1 &&
              boot_host_view_receipt.capture_proof.title_animation_tick == 0 &&
              boot_host_view_receipt.capture_proof.title_frame_start_tick == 0 &&
              boot_host_view_receipt.capture_proof.title_next_frame_tick == 1,
          "boot host-view receipt keeps the menu static");
    check(dm2_v1_boot_startup_packaged_full_start_receipt_from_runtime_state(
              NULL,
              1,
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              1,
              13,
              &boot_full_start_package) &&
              boot_full_start_package.valid &&
              boot_full_start_package.title_animation_tick == 0 &&
              boot_full_start_package.title_frame == 0 &&
              boot_full_start_package.title_frame_elapsed_ticks == 0 &&
              boot_full_start_package.title_frame_remaining_ticks == 1 &&
              boot_full_start_package.title_cycle_position_tick == 0 &&
              boot_full_start_package.title_frame_start_tick == 0 &&
              boot_full_start_package.title_next_frame_tick == 1 &&
              boot_full_start_package.capture_proof.title_ready == 1 &&
              boot_full_start_package.menu_capture_ready == 1 &&
              boot_full_start_package.m11_consumer_ready == 1,
          "boot packaged full-start receipt owns real title ticks");
    check(dm2_v1_boot_startup_packaged_consumer_receipt_from_runtime_state(
              NULL,
              1,
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              1,
              13,
              &boot_consumer_receipt) &&
              boot_consumer_receipt.valid &&
              boot_consumer_receipt.startup_title_frame == 0 &&
              boot_consumer_receipt.title_frame_start_tick == 0 &&
              boot_consumer_receipt.title_next_frame_tick == 1 &&
              boot_consumer_receipt.packaged_title_timing_consumed == 1 &&
              boot_consumer_receipt.packaged_first_hud_receipt_consumed == 1 &&
              boot_consumer_receipt.m11_startup_receipt_ready == 1 &&
              boot_consumer_receipt.startup_draw_command_count ==
                  boot_full_start_package.command_count &&
              boot_consumer_receipt.startup_draw_ready == 1,
          "boot packaged consumer receipt keeps real startup draw timing");
    check(dm2_v1_boot_startup_host_frame_receipt_from_runtime_state(
              NULL,
              1,
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              1,
              13,
              &boot_host_frame_receipt) &&
              boot_host_frame_receipt.valid &&
              boot_host_frame_receipt.render_startup_title == 1 &&
              boot_host_frame_receipt.render_startup_menu == 1 &&
              boot_host_frame_receipt.suppress_game_hud == 1 &&
              boot_host_frame_receipt.schedule_next_title_tick == 0 &&
              boot_host_frame_receipt.next_title_tick_delta == 0 &&
              boot_host_frame_receipt.title_animation_tick == 0 &&
              boot_host_frame_receipt.title_frame == 0 &&
              boot_host_frame_receipt.title_frame_elapsed_ticks == 0 &&
              boot_host_frame_receipt.title_frame_remaining_ticks == 1 &&
              boot_host_frame_receipt.title_next_frame_tick == 1,
          "boot host-frame receipt keeps the menu static");
    check(dm2_v1_boot_startup_render_ownership_receipt_from_runtime_state(
              NULL,
              1,
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              1,
              13,
              &boot_render_ownership_receipt) &&
              boot_render_ownership_receipt.valid &&
              boot_render_ownership_receipt.title_animation_tick == 0 &&
              boot_render_ownership_receipt.title_frame == 0 &&
              boot_render_ownership_receipt.title_frame_elapsed_ticks == 0 &&
              boot_render_ownership_receipt.title_frame_remaining_ticks == 1 &&
              boot_render_ownership_receipt.next_title_tick_delta == 0 &&
              boot_render_ownership_receipt.title_next_frame_tick == 1 &&
              boot_render_ownership_receipt.executed_gdat_image_count == 2 &&
              boot_render_ownership_receipt.title_gdat_command_count == 1 &&
              boot_render_ownership_receipt.menu_gdat_command_count == 1 &&
              boot_render_ownership_receipt.fallback_title_blit_used == 0 &&
              boot_render_ownership_receipt.final_m11_draw_caller_ready == 1 &&
              boot_render_ownership_receipt.final_m11_draw_caller_consumes_ownership == 1 &&
              boot_render_ownership_receipt.packaged_draw_commands_consumed == 1 &&
              boot_render_ownership_receipt.title_timing_receipt_consumed == 1 &&
              boot_render_ownership_receipt.real_gdat_title_asset_receipt_breadth == 1 &&
              boot_render_ownership_receipt.menu_hud_startup_receipt_breadth == 1 &&
              boot_render_ownership_receipt.suppress_game_hud == 1,
          "boot render-ownership receipt owns real GDAT title/menu draw");
    check(!dm2_v1_boot_startup_real_visual_capture_receipt_from_runtime_state(
              NULL,
              1,
              "/tmp/firestaff-dm2-startup",
              1,
              (1u << 2),
              1,
              13,
              &boot_real_visual_capture) &&
              boot_real_visual_capture.valid == 0,
          "boot real visual capture rejects missing GRAPHICS.DAT profile");
    real_data_dir = getenv("FIRESTAFF_DM2_REAL_DATA_DIR");
    if (real_data_dir && real_data_dir[0]) {
        memset(&boot_launch, 0, sizeof(boot_launch));
        if (dm2_v1_boot_startup_launch_alloc(real_data_dir,
                                             &boot_launch) &&
            boot_launch.prepare_result == DM2_V1_BOOT_STARTUP_PREPARE_OK &&
            boot_launch.profile) {
            check(dm2_v1_boot_startup_packaged_consumer_receipt_from_runtime_state(
                      boot_launch.profile,
                      1,
                      "/tmp/firestaff-dm2-startup",
                      1,
                      (1u << 2),
                      1,
                      13,
                      &boot_consumer_receipt) &&
                      boot_consumer_receipt.valid &&
                      boot_consumer_receipt.full_start_real_asset_ready == 1 &&
                      boot_consumer_receipt.startup_title_menu_decoded_gdat_capture_ready == 1 &&
                      boot_consumer_receipt.startup_title_decoded_gdat_hash != 0u &&
                      boot_consumer_receipt.startup_title_decoded_gdat_pixel_count == 320u * 200u &&
                      boot_consumer_receipt.startup_menu_decoded_gdat_hash != 0u &&
                      boot_consumer_receipt.startup_menu_decoded_gdat_pixel_count == 320u * 200u &&
                      boot_consumer_receipt.m11_startup_receipt_ready == 1,
                  "real DM2 consumer receipt consumes decoded title/menu GDAT");
            check(dm2_v1_boot_startup_host_frame_receipt_from_runtime_state(
                      boot_launch.profile,
                      1,
                      "/tmp/firestaff-dm2-startup",
                      1,
                      (1u << 2),
                      1,
                      13,
                      &boot_host_frame_receipt) &&
                      boot_host_frame_receipt.valid &&
                      boot_host_frame_receipt.startup_title_menu_decoded_gdat_capture_ready == 1 &&
                      boot_host_frame_receipt.startup_title_decoded_gdat_hash != 0u &&
                      boot_host_frame_receipt.startup_menu_decoded_gdat_hash != 0u &&
                      boot_host_frame_receipt.render_startup_title == 1,
                  "real DM2 host frame carries decoded title/menu GDAT");
            check(dm2_v1_boot_startup_render_ownership_receipt_from_runtime_state(
                      boot_launch.profile,
                      1,
                      "/tmp/firestaff-dm2-startup",
                      1,
                      (1u << 2),
                      1,
                      13,
                      &boot_render_ownership_receipt) &&
                      boot_render_ownership_receipt.valid &&
                      boot_render_ownership_receipt.startup_title_menu_decoded_gdat_receipt_consumed == 1 &&
                      boot_render_ownership_receipt.startup_title_decoded_gdat_hash != 0u &&
                      boot_render_ownership_receipt.startup_title_decoded_gdat_pixel_count == 320u * 200u &&
                      boot_render_ownership_receipt.startup_menu_decoded_gdat_hash != 0u &&
                      boot_render_ownership_receipt.startup_menu_decoded_gdat_pixel_count == 320u * 200u &&
                      boot_render_ownership_receipt.final_m11_draw_caller_ready == 1,
                  "real DM2 render ownership consumes decoded title/menu GDAT");
            check(dm2_v1_boot_startup_real_visual_capture_receipt_from_runtime_state(
                      boot_launch.profile,
                      1,
                      "/tmp/firestaff-dm2-startup",
                      1,
                      (1u << 2),
                      1,
                      13,
                      &boot_real_visual_capture) &&
                      boot_real_visual_capture.valid &&
                      boot_real_visual_capture.profile_ready == 1 &&
                      boot_real_visual_capture.graphics_dat_ready == 1 &&
                      boot_real_visual_capture.packaged_full_start_valid == 1 &&
                      boot_real_visual_capture.packaged_consumer_valid == 1 &&
                      boot_real_visual_capture.host_frame_valid == 1 &&
                      boot_real_visual_capture.render_ownership_valid == 1 &&
                      boot_real_visual_capture.real_visual_capture_consumes_package == 1 &&
                      boot_real_visual_capture.real_visual_capture_consumes_host_frame == 1 &&
                      boot_real_visual_capture.real_visual_status_consumer_ready == 1 &&
                      boot_real_visual_capture.packaged_status_consumed == 1 &&
                      boot_real_visual_capture.packaged_startup_phase_consumed == 1 &&
                      boot_real_visual_capture.packaged_hud_suppression_consumed == 1 &&
                      boot_real_visual_capture.sampled_title_timing_capture_count >= 3 &&
                      boot_real_visual_capture.sampled_title_pixel_capture_count >= 3 &&
                      boot_real_visual_capture.sampled_title_unique_pixel_hash_count >= 1 &&
                      boot_real_visual_capture.sampled_title_pixel_hash != 0u &&
                      boot_real_visual_capture.sampled_menu_selection_capture_count >= 3 &&
                      boot_real_visual_capture.sampled_menu_composite_capture_count >= 3 &&
                      boot_real_visual_capture.sampled_menu_unique_composite_hash_count >= 1 &&
                      boot_real_visual_capture.sampled_menu_composite_hash != 0u &&
                      (boot_real_visual_capture.sampled_menu_selection_mask & 0x7) == 0x7 &&
                      boot_real_visual_capture.sampled_runtime_hud_handoff_capture_ready == 1 &&
                      /* SHOW_MENU_SCREEN precedes GAME_LOAD: only the real
                       * handoff is present, never a fabricated party HUD. */
                      boot_real_visual_capture.runtime_hud_capture_consumed == 0 &&
                      boot_real_visual_capture.runtime_hud_real_gdat_ready == 0 &&
                      boot_real_visual_capture.runtime_hud_total_fallback_portrait_count == 0 &&
                      boot_real_visual_capture.runtime_hud_total_fallback_floor_ceiling_count == 0 &&
                      boot_real_visual_capture.runtime_hud_total_fallback_wall_count == 0 &&
                      boot_real_visual_capture.real_gdat_capture_breadth_ready == 1 &&
                      boot_real_visual_capture.packaged_full_start_hash != 0u &&
                      boot_real_visual_capture.packaged_consumer_hash ==
                          boot_real_visual_capture.packaged_full_start_hash &&
                      strcmp(boot_real_visual_capture.phase,
                             "dm2-startup-menu") == 0 &&
                      strcmp(boot_real_visual_capture.animation,
                             "dm2-startup-menu") == 0 &&
                      strcmp(boot_real_visual_capture.status_scope,
                             "STARTUP") == 0 &&
                      strcmp(boot_real_visual_capture.status,
                             "DM2 STARTUP MENU") == 0 &&
                      boot_real_visual_capture.real_gdat_title_asset_required == 1 &&
                      boot_real_visual_capture.real_gdat_title_asset_consumed == 1 &&
                      boot_real_visual_capture.real_gdat_menu_asset_consumed == 1 &&
                      boot_real_visual_capture.raw_gdat_capture_ready == 1 &&
                      boot_real_visual_capture.title_raw_byte_hash != 0u &&
                      boot_real_visual_capture.title_raw_byte_count > 0u &&
                      boot_real_visual_capture.menu_raw_byte_hash != 0u &&
                      boot_real_visual_capture.menu_raw_byte_count > 0u &&
                      boot_real_visual_capture.title_capture_ready == 1 &&
                      boot_real_visual_capture.menu_gdat_capture_ready == 1 &&
                      boot_real_visual_capture.skproject_title_query_ready == 1 &&
                      boot_real_visual_capture.skproject_menu_query_ready == 1 &&
                      boot_real_visual_capture.skproject_title_category == 0x05 &&
                      boot_real_visual_capture.skproject_credit_screen_field == 1 &&
                      boot_real_visual_capture.skproject_menu_screen_field == 4 &&
                      boot_real_visual_capture.title_gdat_asset_w == 320 &&
                      boot_real_visual_capture.title_gdat_asset_h == 200 &&
                      boot_real_visual_capture.title_pixel_count == 64000u &&
                      boot_real_visual_capture.title_pixel_hash != 0u &&
                      boot_real_visual_capture.menu_pixel_count == 64000u &&
                      boot_real_visual_capture.menu_pixel_hash != 0u &&
                      boot_real_visual_capture.full_visual_composite_capture_ready == 1 &&
                      boot_real_visual_capture.composite_gdat_blit_count == 2 &&
                      boot_real_visual_capture.composite_rect_count == 0 &&
                      boot_real_visual_capture.composite_text_zone_count == 0 &&
                      boot_real_visual_capture.composite_pixel_count == 64000u &&
                      boot_real_visual_capture.composite_pixel_hash != 0u &&
                      boot_real_visual_capture.menu_capture_ready == 1 &&
                      boot_real_visual_capture.menu_gdat_command_count == 2 &&
                      boot_real_visual_capture.menu_rect_command_count == 0 &&
                      boot_real_visual_capture.menu_text_command_count == 0 &&
                      boot_real_visual_capture.resume_menu_ready == 1 &&
                      boot_real_visual_capture.save_slot_menu_ready == 1 &&
                      boot_real_visual_capture.new_game_menu_ready == 1 &&
                      boot_real_visual_capture.startup_title_menu_hud_breadth_ready == 1 &&
                      boot_real_visual_capture.hud_handoff_capture_ready == 1 &&
                      boot_real_visual_capture.hud_suppressed_capture_ready == 1 &&
                      boot_real_visual_capture.suppress_game_hud == 1 &&
                      boot_real_visual_capture.no_fallback_title_blit == 1 &&
                      boot_real_visual_capture.packaged_visual_capture_hash != 0u,
                  "boot real visual capture consumes broad local GRAPHICS.DAT title/menu/HUD package");
        } else {
            printf("SKIP boot real visual capture: no verified DM2 data in %s\n",
                   real_data_dir);
        }
        dm2_v1_boot_startup_launch_cleanup(&boot_launch);
    } else {
        printf("SKIP boot real visual capture: FIRESTAFF_DM2_REAL_DATA_DIR unset\n");
    }
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
    host_facts.startup_menu_active = 1;
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
              host_action_receipt.menu_state_receipt.selected_row == 0 &&
              host_action_receipt.save_menu_handoff.valid &&
              host_action_receipt.save_menu_handoff.action_kind ==
                  DM2_V1_STARTUP_ACTION_CONTINUE &&
              host_action_receipt.save_menu_handoff.plan_kind ==
                  DM2_V1_STARTUP_PLAN_CONTINUE &&
              host_action_receipt.save_menu_handoff.execution_kind ==
                  DM2_V1_STARTUP_EXEC_STATUS_REDRAW &&
              host_action_receipt.save_menu_handoff.source_row == 0 &&
              host_action_receipt.save_menu_handoff.source_slot == -1 &&
              strcmp(host_action_receipt.save_menu_handoff.save_root,
                     "/tmp/firestaff-dm2-startup-missing") == 0 &&
              host_action_receipt.save_menu_handoff.rescan_saves == 1 &&
              host_action_receipt.save_menu_handoff.session_ready == 0 &&
              host_action_receipt.save_menu_handoff.menu_state_receipt_valid &&
              host_action_receipt.save_menu_handoff.selected_row_after == 0 &&
              host_action_receipt.host_menu_route.valid &&
              host_action_receipt.host_menu_route.consume_input == 1 &&
              host_action_receipt.host_menu_route.redraw_startup_menu == 1 &&
              host_action_receipt.host_menu_route.close_startup_menu == 0 &&
              host_action_receipt.host_menu_route.return_to_launcher == 0 &&
              host_action_receipt.host_menu_route.apply_session == 0 &&
              host_action_receipt.host_menu_route.rescan_saves == 1 &&
              host_action_receipt.host_menu_route.selected_row_after == 0 &&
              host_action_receipt.host_menu_route.runtime_menu_ready == 1 &&
              host_action_receipt.host_menu_route.runtime_action_ready == 1 &&
              host_action_receipt.host_menu_route.first_hud_frame_ready == 0,
          "host action receipt owns failed Continue M11 menu route");
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
          "New Game action preserves the source-owned startup selection");
    check(dm2_v1_startup_execute_plan(
              &plan, "/tmp/firestaff-dm2-startup-missing", &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_GAME_LOAD_REQUIRED &&
              execution.rescan_saves == 0 &&
              strcmp(execution.status, "DM2 GAME_LOAD DATA REQUIRED") == 0,
          "New Game plan requires the original GAME_LOAD data path");
    check(dm2_v1_startup_execute_action(
              &action, "/tmp/firestaff-dm2-startup-missing", &execution) &&
              execution.kind == DM2_V1_STARTUP_EXEC_GAME_LOAD_REQUIRED &&
              execution.rescan_saves == 0 &&
              strcmp(execution.status, "DM2 GAME_LOAD DATA REQUIRED") == 0,
          "New Game action keeps the original GAME_LOAD boundary explicit");
    check(dm2_v1_startup_execution_mode_update(&execution, &mode_update) &&
              !mode_update.set_startup_menu_active,
          "GAME_LOAD requirement leaves the startup menu active");
    check(dm2_v1_startup_execution_input_outcome(&execution, 1, &outcome) &&
              outcome.result == DM2_V1_STARTUP_INPUT_RESULT_REDRAW &&
              outcome.rescan_saves == 0 &&
              strcmp(outcome.status_scope, "GAME_LOAD") == 0 &&
              strcmp(outcome.status, "DM2 GAME_LOAD DATA REQUIRED") == 0,
          "New Game outcome reports the source-owned GAME_LOAD boundary");
    check(dm2_v1_startup_apply_receipt_from_execution(
              &execution, 1, &receipt) &&
              !receipt.session_should_apply &&
              receipt.session_applied &&
              !receipt.mode_update.set_startup_menu_active &&
              receipt.outcome.result == DM2_V1_STARTUP_INPUT_RESULT_REDRAW &&
              strcmp(receipt.outcome.status, "DM2 GAME_LOAD DATA REQUIRED") == 0,
          "New Game receipt blocks synthetic session handoff");
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
              g_apply_calls == 0 &&
              !receipt.session_should_apply &&
              !receipt.session_applied &&
              !receipt.mode_update.set_startup_menu_active &&
              strcmp(receipt.outcome.status, "DM2 GAME_LOAD DATA REQUIRED") == 0,
          "combined action execution blocks synthetic Game Load handoff");
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
              g_apply_calls == 0 &&
              host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              !host_receipt.mode_update.set_startup_menu_active &&
              strcmp(host_receipt.status, "DM2 GAME_LOAD DATA REQUIRED") == 0,
          "combined action execution preserves M11 title/menu gate");
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
              g_apply_calls == 0 &&
              host_action_receipt.host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              !host_action_receipt.host_receipt.mode_update.set_startup_menu_active &&
              host_action_receipt.menu_state_receipt_valid &&
              host_action_receipt.menu_state_receipt.selected_row == 2 &&
              host_action_receipt.input_route.valid &&
              host_action_receipt.input_route.source_kind == 1 &&
              host_action_receipt.input_route.firestaff_menu_input == 9 &&
              host_action_receipt.input_route.startup_input ==
                  DM2_V1_STARTUP_INPUT_ACCEPT &&
              host_action_receipt.input_route.selected_row_before == 2 &&
              host_action_receipt.input_route.selected_row_after == 2 &&
              host_action_receipt.input_route.action_kind ==
                  DM2_V1_STARTUP_ACTION_NEW_GAME &&
              host_action_receipt.input_route.action_row == 2 &&
              host_action_receipt.input_route.action_slot == -1 &&
              host_action_receipt.save_menu_handoff.valid &&
              host_action_receipt.save_menu_handoff.action_kind ==
                  DM2_V1_STARTUP_ACTION_NEW_GAME &&
              host_action_receipt.save_menu_handoff.plan_kind ==
                  DM2_V1_STARTUP_PLAN_NEW_GAME &&
              host_action_receipt.save_menu_handoff.execution_kind ==
                  DM2_V1_STARTUP_EXEC_GAME_LOAD_REQUIRED &&
              host_action_receipt.save_menu_handoff.source_row == 2 &&
              host_action_receipt.save_menu_handoff.session_ready == 0 &&
              host_action_receipt.save_menu_handoff.return_to_launcher == 0 &&
              host_action_receipt.save_menu_handoff.selected_row_after == 2 &&
              host_action_receipt.host_menu_route.valid &&
              host_action_receipt.host_menu_route.redraw_startup_menu == 1 &&
              host_action_receipt.host_menu_route.close_startup_menu == 0 &&
              host_action_receipt.host_menu_route.apply_session == 0 &&
              host_action_receipt.host_menu_route.selected_row_after == 2 &&
              host_action_receipt.host_menu_route.runtime_menu_ready == 1 &&
              host_action_receipt.host_menu_route.runtime_action_ready == 0 &&
              host_action_receipt.host_menu_route.first_hud_frame_ready == 0,
          "host facts keyboard wrapper preserves the GAME_LOAD title gate");
    check(dm2_v1_startup_execution_input_outcome(&execution, 0, &outcome) &&
              outcome.result == DM2_V1_STARTUP_INPUT_RESULT_REDRAW &&
              strcmp(outcome.status_scope, "GAME_LOAD") == 0 &&
              strcmp(outcome.status, "DM2 GAME_LOAD DATA REQUIRED") == 0,
          "GAME_LOAD requirement ignores synthetic session apply state");
    check(dm2_v1_startup_apply_receipt_from_execution(
              &execution, 0, &receipt) &&
              !receipt.session_should_apply &&
              !receipt.session_applied &&
              !receipt.mode_update.set_startup_menu_active &&
              strcmp(receipt.outcome.status, "DM2 GAME_LOAD DATA REQUIRED") == 0,
          "GAME_LOAD receipt keeps startup open without a session");
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
              g_apply_calls == 0 &&
              !receipt.session_should_apply &&
              !receipt.session_applied &&
              !receipt.mode_update.set_startup_menu_active &&
              strcmp(receipt.outcome.status, "DM2 GAME_LOAD DATA REQUIRED") == 0,
          "combined action execution keeps GAME_LOAD behind the title menu");
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
              g_apply_calls == 0 &&
              host_action_receipt.host_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_REDRAW &&
              !host_action_receipt.host_receipt.mode_update.set_startup_menu_active &&
              host_action_receipt.menu_state_receipt_valid &&
              host_action_receipt.menu_state_receipt.selected_row == 2 &&
              host_action_receipt.input_route.valid &&
              host_action_receipt.input_route.source_kind == 2 &&
              host_action_receipt.input_route.pointer_x == row_rect.x + 4 &&
              host_action_receipt.input_route.pointer_y == row_rect.y + 4 &&
              host_action_receipt.input_route.selected_row_before == 0 &&
              host_action_receipt.input_route.selected_row_after == 2 &&
              host_action_receipt.input_route.action_kind ==
                  DM2_V1_STARTUP_ACTION_NEW_GAME &&
              host_action_receipt.input_route.action_row == 2 &&
              host_action_receipt.input_route.action_slot == -1 &&
              host_action_receipt.host_menu_route.valid &&
              host_action_receipt.host_menu_route.close_startup_menu == 0 &&
              host_action_receipt.host_menu_route.apply_session == 0 &&
              host_action_receipt.host_menu_route.selected_row_after == 2 &&
              host_action_receipt.host_menu_route.runtime_menu_ready == 1 &&
              host_action_receipt.host_menu_route.runtime_action_ready == 0 &&
              host_action_receipt.host_menu_route.first_hud_frame_ready == 0,
          "host facts pointer wrapper preserves the GAME_LOAD title gate");
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
    check(!dm2_v1_startup_menu_handle_firestaff_input_from_host_facts_with_receipt(
              &state_receipt,
              &host_facts,
              2,
              &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE,
          "inactive DM2 menu rejects host input consumption");
    check(!dm2_v1_startup_menu_handle_pointer_from_host_facts_with_receipt(
              &state_receipt,
              &host_facts,
              panel_rect.x + 8,
              panel_rect.y + 8,
              &action) &&
              action.kind == DM2_V1_STARTUP_ACTION_NONE,
          "inactive DM2 menu rejects host pointer consumption");
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
