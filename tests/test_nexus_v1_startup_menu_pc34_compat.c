#include "nexus_v1_startup_menu.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_launcher.h"
#include "nexus_v1_title_sequence.h"
#include "nexus_v1_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#include <process.h>
#define TST_MKDIR(path) _mkdir(path)
#define TST_RMDIR(path) _rmdir(path)
#define TST_UNLINK(path) _unlink(path)
#define TST_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define TST_MKDIR(path) mkdir((path), 0700)
#define TST_RMDIR(path) rmdir(path)
#define TST_UNLINK(path) unlink(path)
#define TST_GETPID() getpid()
#endif

static int g_failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int startup_load_success(void *userdata, const char *save_path)
{
    int *calls = (int *)userdata;
    if (calls) {
        *calls += 1;
    }
    return save_path && strstr(save_path, "nexus_save_03.dat") ? 1 : 0;
}

static int make_temp_root(char *out, size_t out_size)
{
    const char *base = getenv("TMPDIR");
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)TST_GETPID();
    int i;

#if defined(_WIN32)
    if (!base || !base[0]) base = getenv("TEMP");
#endif
    if (!base || !base[0]) base = "/tmp";
    for (i = 0; i < 64; ++i) {
        int n = snprintf(out, out_size,
                         "%s/firestaff-nexus-startup-menu-%u-%d",
                         base, seed, i);
        if (n < 0 || (size_t)n >= out_size) return 0;
        if (TST_MKDIR(out) == 0) return 1;
    }
    return 0;
}

static int local_file_exists(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    fclose(fp);
    return 1;
}

static void build_world(Nexus_V1_World *world)
{
    nexus_v1_world_init(world);
    nexus_v1_party_place(world, 2, 17, 21, 3);
    nexus_v1_world_tick(world);
}

int main(void)
{
    char root[512];
    char save_dir[512];
    char path[512];
    Nexus_V1_SaveManager mgr;
    Nexus_V1_ChampionPool champions;
    Nexus_V1_World world;
    Nexus_V1_StartupMenu menu;
    Nexus_V1_StartupAction action;
    Nexus_V1_StartupSaveExecution execution;
    Nexus_V1_StartupTitleExecution title_execution;
    Nexus_V1_StartupChampionExecution champion_execution;
    Nexus_V1_StartupModeUpdate mode_update;
    Nexus_V1_StartupApplyReceipt receipt;
    Nexus_V1_StartupHostReceipt host_receipt;
    Nexus_V1_StartupHostActionReceipt host_action_receipt;
    Nexus_V1_StartupIdleReceipt idle_receipt;
    Nexus_V1_StartupSaveRouteReceipt save_route_receipt;
    Nexus_V1_StartupTitleRouteReceipt title_route_receipt;
    Nexus_V1_StartupHit hit;
    Nexus_V1_StartupRect footer_rect;
    Nexus_V1_StartupSaveRenderRow save_rows[4];
    Nexus_V1_StartupChampionRenderRow champion_rows[12];
    Nexus_V1_StartupChampionFooterRender champion_footer;
    Nexus_V1_StartupChromeRender chrome;
    Nexus_V1_StartupDrawCommand draw_commands[80];
    Nexus_V1_StartupMenuSnapshot menu_snapshot;
    Nexus_V1_StartupChampionSnapshot champion_snapshot;
    Nexus_V1_StartupHostFacts host_facts;
    Nexus_V1_StartupRuntimeState runtime_state;
    Nexus_V1_LauncherRuntimeStartupSnapshot runtime_snapshot;
    Nexus_V1_StartupMenuPresentationReceipt presentation_receipt;
    Nexus_V1_StartupTitleHandoffReceipt title_handoff_receipt;
    Nexus_V1_StartupRuntimeHandoffReceipt runtime_handoff_receipt;
    Nexus_V1_StartupRuntimeRouteReceipt runtime_route_receipt;
    Nexus_V1_StartupRouteProofReceipt route_proof_receipt;
    Nexus_V1_StartupFullStartReceipt full_start_receipt;
    Nexus_V1_StartupFullStartConsumerReceipt full_start_consumer_receipt;
    Nexus_V1_StartupFullStartPackageReceipt full_start_package_receipt;
    Nexus_V1_StartupFullStartPackageReceipt mutated_package_receipt;
    Nexus_V1_M12StartupPackageReceipt m12_package_receipt;
    Nexus_V1_StartupReceiptBundle startup_bundle_receipt;
    Nexus_V1_StartupRealAssetOwnershipReceipt real_asset_ownership_receipt;
    Nexus_V1_StartupHostCallerReceipt host_caller_receipt;
    Nexus_V1_LauncherStartupAssetsReceipt startup_assets_receipt;
    Nexus_V1_StartupLaunchGateReceipt launch_gate_receipt;
    Nexus_V1_StartupAssetHandoffReceipt asset_handoff_receipt;
    Nexus_V1_LauncherRuntimeReceipt synthetic_runtime_receipt;
    Nexus_V1_DgnRenderCommand dgn_commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_Engine synthetic_engine;
    uint8_t synthetic_surface_pixel = 1;
    Nexus_V1_StartupRowKind kind;
    Nexus_V1_TitleFrame title_frame;
    Nexus_V1_BootFrame boot_frame;
    int slot;
    int cursor;
    int draw_count;
    int portrait_draws;
    int load_calls;
    char package_phase[32];
    char package_animation[32];
    int package_startup_active;
    int package_startup_frame;
    int package_animation_active;
    int package_title_frame;
    int package_title_frame_max;
    int package_title_ready;
    Nexus_V1_ChampionPool empty_champions;

    if (!make_temp_root(root, sizeof(root))) {
        fprintf(stderr, "FAIL: could not create temporary root\n");
        return 1;
    }
    snprintf(save_dir, sizeof(save_dir), "%s/saves", root);

    nexus_v1_launcher_m12_startup_package_receipt_clear(&m12_package_receipt);
    expect(m12_package_receipt.capture_route ==
               NEXUS_V1_STARTUP_CAPTURE_INVALID &&
               m12_package_receipt.first_capture_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_NONE &&
               strcmp(m12_package_receipt.game_id, "nexus") == 0 &&
               strcmp(m12_package_receipt.card_title_label,
                      "DM Nexus") == 0 &&
               strcmp(m12_package_receipt.card_subtitle_label,
                      "Saturn boot, title, save, champions") == 0 &&
               strcmp(m12_package_receipt.timing_summary_label,
                      "warning 48f / title ready 102f") == 0,
           "Nexus M12 startup package clear resets capture and card fields");
    expect(nexus_v1_launcher_m12_startup_package_from_flags(
               1,
               1,
               1,
               &m12_package_receipt) &&
               m12_package_receipt.packaged_capture_ready == 1 &&
               m12_package_receipt.startup_step_count == 7 &&
               m12_package_receipt.startup_step_ready_count == 7 &&
               m12_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_TITLE &&
               m12_package_receipt.capture_command_count == 1 &&
               m12_package_receipt.first_capture_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND &&
               m12_package_receipt.boot_warning_frames == 48 &&
               m12_package_receipt.boot_start_ready_frames == 102 &&
               m12_package_receipt.title_frame_max == 102 &&
               m12_package_receipt.title_prompt_visible == 1 &&
               m12_package_receipt.warning_surface_loaded == 1 &&
               m12_package_receipt.title_surface_loaded == 1 &&
               m12_package_receipt.gameover_surface_loaded == 1 &&
               m12_package_receipt.warning_capture_surface_ready == 1 &&
               m12_package_receipt.title_capture_surface_ready == 1 &&
               m12_package_receipt.gameover_capture_surface_ready == 1 &&
               m12_package_receipt.warning_capture_frame == 0 &&
               m12_package_receipt.title_capture_frame == 48 &&
               m12_package_receipt.save_capture_frame == -1 &&
               m12_package_receipt.champion_capture_frame == -1 &&
               m12_package_receipt.dungeon_capture_frame == -1 &&
               m12_package_receipt.gameover_capture_frame == 0 &&
               strcmp(m12_package_receipt.card_title_label,
                      "DM Nexus") == 0 &&
               strcmp(m12_package_receipt.card_subtitle_label,
                      "Saturn boot, title, save, champions") == 0 &&
               strcmp(m12_package_receipt.timing_summary_label,
                      "warning 48f / title ready 102f") == 0 &&
               strcmp(m12_package_receipt.capture_route_label,
                      "title-warning") == 0 &&
               strcmp(m12_package_receipt.first_capture_draw_label,
                      "warning-background") == 0 &&
               m12_package_receipt.saturn_warning_frame == 0 &&
               m12_package_receipt.saturn_title_capture_frame == 48 &&
               m12_package_receipt.saturn_save_capture_frame == -1 &&
               m12_package_receipt.saturn_champion_capture_frame == -1 &&
               m12_package_receipt.saturn_dungeon_capture_frame == -1 &&
               m12_package_receipt.saturn_title_ready_frame == 102 &&
               m12_package_receipt.saturn_gameover_capture_frame == 0 &&
               m12_package_receipt.saturn_timing_exact == 1 &&
               m12_package_receipt.saturn_capture_frames_exact == 1 &&
               m12_package_receipt.full_start_package_receipt_ready == 1 &&
               m12_package_receipt.host_display_caller_expected == 1 &&
               strcmp(m12_package_receipt.contract_label,
                      "NEXUS HOST-CALLER/FULL-START PACKAGE RECEIPTS") == 0 &&
               strcmp(m12_package_receipt.active_proof_label,
                      "NEXUS TIMING CAPTURE PROOF") == 0 &&
               strcmp(m12_package_receipt.launch_status_label,
                      "READY TO LAUNCH") == 0 &&
               strcmp(m12_package_receipt.launch_detail_label,
                      "NEXUS TITLE MENU") == 0,
           "Nexus M12 startup package owns ready card display/timing/capture facts");
    expect(nexus_v1_launcher_m12_startup_package_from_flags(
               1,
               1,
               0,
               &m12_package_receipt) &&
               m12_package_receipt.packaged_capture_ready == 0 &&
               m12_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_BLOCKED &&
               strcmp(m12_package_receipt.capture_route_label,
                      "blocked-startup") == 0 &&
               strcmp(m12_package_receipt.next_step_label,
                      "SELECTED VERSION") == 0 &&
               strcmp(m12_package_receipt.active_proof_label,
                      "SELECTED VERSION") == 0 &&
               strcmp(m12_package_receipt.blocked_status_label,
                      "VERSION MISSING") == 0 &&
               strcmp(m12_package_receipt.blocked_detail_label,
                      "SELECTED VERSION") == 0 &&
               strcmp(m12_package_receipt.card_subtitle_label,
                      "Saturn boot, title, save, champions") == 0,
           "Nexus M12 startup package blocks display/capture before version proof");

    memset(&empty_champions, 0, sizeof(empty_champions));
    expect(nexus_v1_startup_input_from_firestaff_menu_code(0) ==
               NEXUS_V1_STARTUP_INPUT_NONE &&
               nexus_v1_startup_input_from_firestaff_menu_code(1) ==
                   NEXUS_V1_STARTUP_INPUT_UP &&
               nexus_v1_startup_input_from_firestaff_menu_code(2) ==
                   NEXUS_V1_STARTUP_INPUT_DOWN &&
               nexus_v1_startup_input_from_firestaff_menu_code(9) ==
                   NEXUS_V1_STARTUP_INPUT_ACCEPT &&
               nexus_v1_startup_input_from_firestaff_menu_code(10) ==
                   NEXUS_V1_STARTUP_INPUT_BACK &&
               nexus_v1_startup_input_from_firestaff_menu_code(11) ==
                   NEXUS_V1_STARTUP_INPUT_ACTION,
           "Firestaff menu input codes map through Nexus startup input adapter");
    expect(nexus_v1_startup_input_from_firestaff_menu_code(999) ==
               NEXUS_V1_STARTUP_INPUT_NONE,
           "unknown Firestaff menu input maps to Nexus startup idle input");

    cursor = 7;
    memset(&action, 0, sizeof(action));
    expect(nexus_v1_startup_champion_handle_input(
               &empty_champions,
               &cursor,
               0u,
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE,
           "Nexus champion startup Back returns to title even with an empty roster");
    expect(nexus_v1_startup_champion_handle_input(
               &empty_champions,
               &cursor,
               (1u << 3),
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT,
           "Nexus champion startup Back returns to save select with slots and an empty roster");
    expect(!nexus_v1_startup_champion_handle_input(
               &empty_champions,
               &cursor,
               0u,
               NEXUS_V1_STARTUP_INPUT_ACCEPT,
               &action),
           "Nexus champion startup ignores non-Back input with an empty roster");

    nexus_v1_champions_init(&champions);
    expect(champions.champion_count == NEXUS_MAX_CHAMPIONS,
           "Nexus startup champion roster exposes all 24 mirror rows");
    cursor = 0;
    memset(champion_rows, 0, sizeof(champion_rows));
    memset(&champion_footer, 0, sizeof(champion_footer));
    expect(nexus_v1_startup_menu_build_champion_render_rows(
               &champions,
               cursor,
               champion_rows,
               (int)(sizeof(champion_rows) / sizeof(champion_rows[0])),
               &champion_footer) == 12,
           "Nexus champion startup render plan exposes bounded visible rows");
    expect(champion_rows[0].row == 0 &&
               champion_rows[0].selected == 1 &&
               champion_rows[0].in_party == 0 &&
               champion_rows[0].portrait_index ==
                   champions.champions[0].portrait_index &&
               champion_rows[0].portrait_x ==
                   NEXUS_V1_STARTUP_CHAMPION_PORTRAIT_X &&
               champion_rows[0].highlight_visible == 1 &&
               champion_rows[0].text_color == 11 &&
               champion_rows[0].portrait_border_color == 11 &&
               champion_rows[0].text_x ==
                   NEXUS_V1_STARTUP_CHAMPION_ROW_TEXT_X &&
               strstr(champion_rows[0].label,
                      champions.champions[0].name_ascii) != NULL,
           "Nexus champion startup render row carries selection, portrait, text and label");
    memset(champion_rows, 0, sizeof(champion_rows));
    expect(nexus_v1_startup_menu_build_champion_render_rows_for_frame(
               &champions,
               cursor,
               12,
               champion_rows,
               (int)(sizeof(champion_rows) / sizeof(champion_rows[0])),
               &champion_footer) == 12 &&
               champion_rows[0].selected == 1 &&
               champion_rows[0].highlight_visible == 0 &&
               champion_rows[0].text_color == 15 &&
               champion_rows[0].portrait_border_color == 12,
           "Nexus champion startup render row owns cursor blink timing");
    expect(strstr(champion_footer.label, "PARTY 0/4") != NULL &&
               champion_footer.text_x == NEXUS_V1_STARTUP_FOOTER_X &&
               champion_footer.text_y == NEXUS_V1_STARTUP_FOOTER_Y,
           "Nexus champion startup footer render metadata is Nexus-owned");
    cursor = 13;
    memset(champion_rows, 0, sizeof(champion_rows));
    memset(&champion_footer, 0, sizeof(champion_footer));
    expect(nexus_v1_startup_champion_visible_first_row(
               champions.champion_count, cursor, 12) == 12,
           "Nexus champion startup second page starts at champion 12");
    expect(nexus_v1_startup_menu_build_champion_render_rows(
               &champions,
               cursor,
               champion_rows,
               (int)(sizeof(champion_rows) / sizeof(champion_rows[0])),
               &champion_footer) == 12 &&
               champion_rows[0].row == 12 &&
               champion_rows[1].row == 13 &&
               champion_rows[1].selected == 1 &&
               champion_rows[1].highlight_visible == 1 &&
               champion_rows[1].text_color == 11 &&
               champion_rows[0].rect.y == 37 &&
               strstr(champion_rows[0].label,
                      champions.champions[12].name_ascii) != NULL,
           "Nexus champion startup render plan pages to the cursor-visible roster half");
    memset(&hit, 0, sizeof(hit));
    expect(nexus_v1_startup_champion_hit_at_cursor(
               champions.champion_count,
               cursor,
               20,
               49,
               &hit) &&
               hit.kind == NEXUS_V1_STARTUP_HIT_CHAMPION_ROW &&
               hit.row == 13,
           "Nexus champion startup pointer hit maps visible page rows to absolute champion rows");
    cursor = 0;
    memset(&action, 0, sizeof(action));
    expect(nexus_v1_startup_champion_handle_input(
               &champions,
               &cursor,
               0u,
               NEXUS_V1_STARTUP_INPUT_ACTION,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_NEED_CHAMPION &&
               champions.party_count == 0,
           "Nexus champion startup refuses dungeon start without a party");
    expect(nexus_v1_startup_execute_champion_action(&action,
                                                    &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW &&
               strcmp(champion_execution.status,
                      "NEXUS NEEDS CHAMPION") == 0,
           "Nexus champion execution resolves empty-party start");
    expect(nexus_v1_startup_champion_handle_input(
               &champions,
               &cursor,
               0u,
               NEXUS_V1_STARTUP_INPUT_ACCEPT,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_CHAMPION_ADDED &&
               action.row == 0 &&
               cursor == 1 &&
               champions.party_count == 1,
           "Nexus champion startup Accept recruits and advances cursor");
    memset(champion_rows, 0, sizeof(champion_rows));
    memset(&champion_footer, 0, sizeof(champion_footer));
    expect(nexus_v1_startup_menu_build_champion_render_rows(
               &champions,
               cursor,
               champion_rows,
               (int)(sizeof(champion_rows) / sizeof(champion_rows[0])),
               &champion_footer) == 12 &&
               champion_rows[0].in_party == 1 &&
               champion_rows[1].selected == 1 &&
               strstr(champion_rows[0].label, "*") != NULL &&
               strstr(champion_footer.label, "PARTY 1/4") != NULL,
           "Nexus champion startup render plan tracks recruited party and advanced cursor");
    expect(nexus_v1_startup_execute_champion_action(&action,
                                                    &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW &&
               strcmp(champion_execution.status,
                      "NEXUS CHAMPION ADDED") == 0,
           "Nexus champion execution resolves recruited champion redraw");
    expect(nexus_v1_startup_champion_handle_input(
               &champions,
               &cursor,
               0u,
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_CHAMPION_REMOVED &&
               action.row == 0 &&
               cursor == 0 &&
               champions.party_count == 0,
           "Nexus champion startup Back removes last recruit");
    expect(nexus_v1_startup_execute_champion_action(&action,
                                                    &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR &&
               champion_execution.cursor == 0 &&
               strcmp(champion_execution.status,
                      "NEXUS CHAMPION REMOVED") == 0,
           "Nexus champion execution resolves removed champion cursor");
    expect(nexus_v1_startup_champion_handle_input(
               &champions,
               &cursor,
               0u,
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE,
           "Nexus champion startup Back returns to title with no save slots");
    expect(nexus_v1_startup_execute_champion_action(&action,
                                                    &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_SHOW_TITLE &&
               strcmp(champion_execution.status, "NEXUS TITLE") == 0,
           "Nexus champion execution resolves no-slot Back to title");
    expect(nexus_v1_startup_champion_handle_input(
               &champions,
               &cursor,
               (1u << 3),
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT,
           "Nexus champion startup Back returns to save select when slots exist");
    expect(nexus_v1_startup_execute_champion_action(&action,
                                                    &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_SHOW_SAVE_SELECT &&
               champion_execution.select_last_save_row &&
               strcmp(champion_execution.status,
                      "NEXUS LOAD GAME") == 0,
           "Nexus champion execution resolves save-select return");
    cursor = 0;
    memset(&hit, 0, sizeof(hit));
    hit.kind = NEXUS_V1_STARTUP_HIT_CHAMPION_PANEL;
    hit.row = -1;
    memset(&action, 0, sizeof(action));
    expect(nexus_v1_startup_champion_handle_hit(
               &champions,
               &cursor,
               0u,
               &hit,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_NONE &&
               champions.party_count == 0,
           "Nexus champion startup panel hit is consumed without recruiting");
    expect(nexus_v1_startup_execute_champion_action(&action,
                                                    &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW &&
               strcmp(champion_execution.status, "NEXUS CHAMPIONS") == 0,
           "Nexus champion execution resolves consumed panel hit");
    hit.kind = NEXUS_V1_STARTUP_HIT_CHAMPION_ROW;
    hit.row = 1;
    expect(nexus_v1_startup_champion_handle_hit(
               &champions,
               &cursor,
               0u,
               &hit,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_CHAMPION_ADDED &&
               action.row == 1 &&
               cursor == 2 &&
               champions.party_count == 1,
           "Nexus champion startup row hit recruits through Nexus-owned action");
    expect(nexus_v1_startup_execute_champion_action(
               &action,
               &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW &&
               strcmp(champion_execution.status,
                      "NEXUS CHAMPION ADDED") == 0,
           "Nexus champion execution resolves recruited champion redraw");
    {
        Nexus_V1_StartupChampionSnapshot champion_snapshot;
        Nexus_V1_StartupChampionStateReceipt champion_receipt;
        Nexus_V1_StartupChampionRenderRow snapshot_rows[4];
        Nexus_V1_StartupChampionFooterRender snapshot_footer;
        int snapshot_row_count;

        memset(&champion_snapshot, 0, sizeof(champion_snapshot));
        champion_snapshot.cursor = 0;
        champion_snapshot.frame = 12;
        expect(nexus_v1_startup_champion_snapshot_from_facts(
                   &champions,
                   &champion_snapshot,
                   0x0fffu,
                   99,
                   -5) &&
                   champion_snapshot.slot_mask == 0x00ffu &&
                   champion_snapshot.cursor == 0 &&
                   champion_snapshot.frame == 0,
               "Nexus champion snapshot facts helper clamps cursor and frame");
        champion_snapshot.cursor = 0;
        champion_snapshot.frame = 12;
        expect(nexus_v1_startup_champion_snapshot_handle_input(
                   &champions,
                   &champion_snapshot,
                   NEXUS_V1_STARTUP_INPUT_DOWN,
                   &action) &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR &&
                   champion_snapshot.cursor == 2,
               "Nexus champion snapshot owns cursor input");
        {
            Nexus_V1_StartupChampionSnapshot facts_snapshot;
            memset(&facts_snapshot, 0, sizeof(facts_snapshot));
            expect(nexus_v1_startup_champion_handle_firestaff_input_from_facts(
                       &champions,
                       &facts_snapshot,
                       0x0fffu,
                       99,
                       -5,
                       2,
                       &action) &&
                       facts_snapshot.slot_mask == 0x00ffu &&
                       facts_snapshot.cursor == 2 &&
                       facts_snapshot.frame == 0 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR,
               "Nexus champion facts input helper owns M11 keyboard construction");
        }
        expect(nexus_v1_startup_champion_state_receipt_from_snapshot(
                   &champion_snapshot,
                   &champion_receipt) &&
                   champion_receipt.slot_mask == champion_snapshot.slot_mask &&
                   champion_receipt.cursor == champion_snapshot.cursor &&
                   champion_receipt.frame == champion_snapshot.frame,
               "Nexus champion state receipt mirrors sanitized snapshot state");
        expect(nexus_v1_startup_champion_state_receipt_from_facts(
                   &champions,
                   &champion_receipt,
                   0x0fffu,
                   99,
                   -5) &&
                   champion_receipt.slot_mask == 0x00ffu &&
                   champion_receipt.cursor == 0 &&
                   champion_receipt.frame == 0,
               "Nexus champion state receipt facts helper owns M11 state clamp");
        expect(nexus_v1_startup_champion_handle_firestaff_input_from_facts_with_receipt(
                   &champions,
                   &champion_receipt,
                   0x0fffu,
                   99,
                   -5,
                   2,
                   &action) &&
                   champion_receipt.slot_mask == 0x00ffu &&
                   champion_receipt.cursor == 2 &&
                   champion_receipt.frame == 0 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR,
               "Nexus champion receipt input helper owns M11 keyboard state");
        memset(&host_facts, 0, sizeof(host_facts));
        host_facts.slot_mask = 0x0fffu;
        host_facts.champion_pool = &champions;
        host_facts.champion_cursor = 99;
        host_facts.champion_frame = -5;
        expect(nexus_v1_startup_champion_handle_firestaff_input_from_host_facts_with_receipt(
                   &champion_receipt,
                   &host_facts,
                   2,
                   &action) &&
                   champion_receipt.slot_mask == 0x00ffu &&
                   champion_receipt.cursor == 2 &&
                   champion_receipt.frame == 0 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR,
               "Nexus champion host facts input helper owns M11 keyboard state");
        expect(nexus_v1_startup_execute_champion_firestaff_input_from_host_facts_with_receipt(
                   &host_facts,
                   2,
                   &champion_execution,
                   &host_action_receipt) &&
                   host_action_receipt.champion_state_receipt_valid &&
                   host_action_receipt.champion_state_receipt.cursor == 2 &&
                   host_action_receipt.host_receipt.input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                   host_action_receipt.host_receipt.mode_update
                       .set_champion_cursor &&
                   host_action_receipt.host_receipt.mode_update
                       .champion_cursor == 2 &&
                   champion_execution.kind ==
                       NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR,
               "Nexus champion input wrapper returns M11-ready state and action receipt");
        snapshot_row_count =
            nexus_v1_startup_champion_snapshot_build_render_rows(
                &champions,
                &champion_snapshot,
                snapshot_rows,
                (int)(sizeof(snapshot_rows) / sizeof(snapshot_rows[0])),
                &snapshot_footer);
        expect(snapshot_row_count > 0 &&
                   snapshot_rows[0].row == 0 &&
                   snapshot_rows[2].row == 2 &&
                   snapshot_rows[2].selected == 1 &&
                   snapshot_rows[2].highlight_visible == 0 &&
                   strstr(snapshot_footer.label, "PARTY 1/") != NULL,
               "Nexus champion snapshot owns frame-aware render rows");
        hit.kind = NEXUS_V1_STARTUP_HIT_CHAMPION_FOOTER;
        hit.row = -1;
        expect(nexus_v1_startup_champion_snapshot_handle_hit(
                   &champions,
                   &champion_snapshot,
                   &hit,
                   &action) &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_START_DUNGEON,
               "Nexus champion snapshot owns footer hit action");
        expect(nexus_v1_startup_champion_handle_pointer_from_facts(
                   &champions,
                   &champion_snapshot,
                   0x0fffu,
                   13,
                   12,
                   20,
                   38,
                   &action) &&
                   champion_snapshot.slot_mask == 0x00ffu &&
                   champion_snapshot.cursor == 13 &&
                   champion_snapshot.frame == 12 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_CHAMPION_ADDED &&
                   action.row == 12,
               "Nexus champion facts pointer helper owns M11 pointer hit construction");
        {
            Nexus_V1_ChampionPool receipt_pool;
            Nexus_V1_ChampionPool host_pool;
            nexus_v1_champions_init(&receipt_pool);
            expect(nexus_v1_startup_champion_handle_pointer_from_facts_with_receipt(
                       &receipt_pool,
                       &champion_receipt,
                       0x0fffu,
                       13,
                       12,
                       20,
                       38,
                       &action) &&
                       champion_receipt.slot_mask == 0x00ffu &&
                       champion_receipt.cursor == 13 &&
                       champion_receipt.frame == 12 &&
                       action.kind == NEXUS_V1_STARTUP_ACTION_CHAMPION_ADDED &&
                       action.row == 12,
                   "Nexus champion receipt pointer helper owns M11 pointer state");
            nexus_v1_champions_init(&host_pool);
            memset(&host_facts, 0, sizeof(host_facts));
            host_facts.slot_mask = 0x0fffu;
            host_facts.champion_pool = &host_pool;
            host_facts.champion_cursor = 13;
            host_facts.champion_frame = 12;
            expect(nexus_v1_startup_champion_handle_pointer_from_host_facts_with_receipt(
                       &champion_receipt,
                       &host_facts,
                       20,
                       38,
                       &action) &&
                       champion_receipt.slot_mask == 0x00ffu &&
                       champion_receipt.cursor == 13 &&
                       champion_receipt.frame == 12 &&
                       action.kind == NEXUS_V1_STARTUP_ACTION_CHAMPION_ADDED &&
                       action.row == 12,
                   "Nexus champion host facts pointer helper owns M11 pointer state");
            nexus_v1_champions_init(&host_pool);
            host_facts.champion_pool = &host_pool;
        expect(nexus_v1_startup_execute_champion_pointer_from_host_facts_with_receipt(
                       &host_facts,
                       20,
                       38,
                       &champion_execution,
                       &host_action_receipt) &&
                       host_action_receipt.champion_state_receipt_valid &&
                       host_action_receipt.champion_state_receipt.cursor ==
                           13 &&
                       host_action_receipt.host_receipt.input_result ==
                           NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                       champion_execution.kind ==
                           NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW &&
                       host_pool.party_count == 1,
                   "Nexus champion pointer wrapper returns M11-ready state and action receipt");
        }
        memset(&host_facts, 0, sizeof(host_facts));
        host_facts.title_active = 1;
        host_facts.title_frame = 41;
        expect(nexus_v1_startup_advance_idle_from_host_facts_with_receipt(
                   &host_facts,
                   &idle_receipt) &&
                   idle_receipt.host_receipt.input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                   idle_receipt.host_receipt.mode_update.set_title_frame &&
                   idle_receipt.host_receipt.mode_update.title_frame == 42,
               "Nexus idle receipt owns title frame advance");
        memset(&host_facts, 0, sizeof(host_facts));
        host_facts.save_select_active = 1;
        expect(nexus_v1_startup_advance_idle_from_host_facts_with_receipt(
                   &host_facts,
                   &idle_receipt) &&
                   idle_receipt.host_receipt.input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_IGNORED,
               "Nexus idle receipt owns save-select idle no-op");
        memset(&host_facts, 0, sizeof(host_facts));
        host_facts.champion_select_active = 1;
        host_facts.champion_frame = 12;
        expect(nexus_v1_startup_advance_idle_from_host_facts_with_receipt(
                   &host_facts,
                   &idle_receipt) &&
                   idle_receipt.host_receipt.input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                   idle_receipt.host_receipt.mode_update
                       .set_champion_frame &&
                   idle_receipt.host_receipt.mode_update.champion_frame == 13,
               "Nexus idle receipt owns champion cursor blink frame advance");
        {
            Nexus_V1_StartupDrawCommand commands[80];
            int command_count =
                nexus_v1_startup_presentation_build_champion_from_facts(
                    &champions,
                    0x0fffu,
                    13,
                    12,
                    commands,
                    (int)(sizeof(commands) / sizeof(commands[0])));
            expect(command_count > 3 &&
                       commands[0].kind ==
                           NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
                   "Nexus champion presentation facts helper owns M11 render snapshot construction");
            memset(&host_facts, 0, sizeof(host_facts));
            host_facts.slot_mask = 0x0fffu;
            host_facts.champion_pool = &champions;
            host_facts.champion_cursor = 13;
            host_facts.champion_frame = 12;
            command_count =
                nexus_v1_startup_presentation_build_champion_from_host_facts(
                    &host_facts,
                    commands,
                    (int)(sizeof(commands) / sizeof(commands[0])));
            expect(command_count > 3 &&
                       commands[0].kind ==
                           NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
                   "Nexus champion presentation host facts helper owns M11 render snapshot construction");
        }
    }
    action.kind = NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR;
    action.row = 2;
    expect(nexus_v1_startup_execute_champion_action(&action,
                                                    &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR &&
               champion_execution.cursor == 2 &&
               strcmp(champion_execution.status,
                      "NEXUS CHAMPION CURSOR") == 0,
           "Nexus champion execution resolves cursor movement");
    expect(nexus_v1_startup_champion_execution_mode_update(
               &champion_execution,
               2,
               &mode_update) &&
               mode_update.set_champion_cursor &&
               mode_update.champion_cursor == 2 &&
               !mode_update.set_champion_select_active,
           "Nexus champion execution owns cursor state update");
    expect(nexus_v1_startup_apply_receipt_from_champion_execution(
               &champion_execution,
               2,
               &receipt) &&
               receipt.result == NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW &&
               receipt.mode_update.set_champion_cursor &&
               receipt.mode_update.champion_cursor == 2 &&
               strcmp(receipt.status, "NEXUS CHAMPION CURSOR") == 0,
           "Nexus champion apply receipt owns cursor redraw policy");
    expect(nexus_v1_startup_host_receipt_from_apply_receipt(
               &receipt,
               &host_receipt) &&
               host_receipt.input_result ==
                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
               host_receipt.mode_update.set_champion_cursor &&
               host_receipt.mode_update.champion_cursor == 2 &&
               strcmp(host_receipt.status, "NEXUS CHAMPION CURSOR") == 0,
           "Nexus champion host receipt owns M11 cursor contract");
    expect(nexus_v1_startup_execute_champion_action_with_receipt(
               &action,
               2,
               &champion_execution,
               &receipt) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR &&
               receipt.result == NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW &&
               receipt.mode_update.set_champion_cursor &&
               receipt.mode_update.champion_cursor == 2 &&
               strcmp(receipt.status, "NEXUS CHAMPION CURSOR") == 0,
           "Nexus champion action helper owns execution and receipt");
    expect(nexus_v1_startup_execute_champion_action_with_host_receipt(
               &action,
               2,
               &champion_execution,
               &host_receipt) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR &&
               host_receipt.input_result ==
                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
               host_receipt.mode_update.set_champion_cursor &&
               host_receipt.mode_update.champion_cursor == 2 &&
               strcmp(host_receipt.status, "NEXUS CHAMPION CURSOR") == 0,
           "Nexus champion action helper can return M11-ready host receipt directly");
    hit.kind = NEXUS_V1_STARTUP_HIT_CHAMPION_FOOTER;
    hit.row = -1;
    expect(nexus_v1_startup_champion_handle_hit(
               &champions,
               &cursor,
               0u,
               &hit,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_START_DUNGEON,
           "Nexus champion startup footer hit starts when party exists");
    expect(nexus_v1_startup_execute_champion_action(
               &action,
               &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON &&
               strcmp(champion_execution.status_scope, "BOOT") == 0 &&
               strcmp(champion_execution.status, "NEXUS READY") == 0,
           "Nexus champion execution resolves start-dungeon handoff");
    memset(&synthetic_engine, 0, sizeof(synthetic_engine));
    synthetic_engine.level_loaded = 1;
    synthetic_engine.game.party_x = 3;
    synthetic_engine.game.party_y = 4;
    synthetic_engine.game.party_dir = 0;
    synthetic_engine.ui.surfaces[NEXUS_SURFACE_TITLE].data =
        &synthetic_surface_pixel;
    synthetic_engine.ui.surfaces[NEXUS_SURFACE_TITLE].w = 320;
    synthetic_engine.ui.surfaces[NEXUS_SURFACE_TITLE].h = 200;
    synthetic_engine.ui.surfaces[NEXUS_SURFACE_WARNING].data =
        &synthetic_surface_pixel;
    synthetic_engine.ui.surfaces[NEXUS_SURFACE_WARNING].w = 320;
    synthetic_engine.ui.surfaces[NEXUS_SURFACE_WARNING].h = 200;
    synthetic_engine.ui.surfaces[NEXUS_SURFACE_GAMEOVER].data =
        &synthetic_surface_pixel;
    synthetic_engine.ui.surfaces[NEXUS_SURFACE_GAMEOVER].w = 320;
    synthetic_engine.ui.surfaces[NEXUS_SURFACE_GAMEOVER].h = 200;
    synthetic_engine.ui_startup_surfaces_expected = 1;
    synthetic_engine.ui_startup_surfaces_loaded = 1;
    synthetic_engine.ui_faces_expected = NEXUS_MAX_CHAMPIONS;
    synthetic_engine.ui_faces_loaded = NEXUS_MAX_CHAMPIONS;
    synthetic_engine.menu_bpk_upload_receipt_valid = 1;
    synthetic_engine.menu_bpk_upload_receipt.route =
        NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED;
    synthetic_engine.menu_bpk_upload_receipt.ready_uploads = 3;
    synthetic_engine.menu_bpk_upload_receipt.planned_rows = 3;
    synthetic_engine.menu_bpk_decode_receipt_valid = 1;
    synthetic_engine.menu_bpk_decode_receipt_attempted = 1;
    synthetic_engine.menu_bpk_decode_receipt.route =
        NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED;
    synthetic_engine.menu_bpk_decode_receipt.surface_entries = 3;
    synthetic_engine.menu_bpk_decode_receipt.ready_stored_surfaces = 3;
    synthetic_engine.sfx_runtime_receipt.status =
        NEXUS_SFX_RUNTIME_READY_DECODED;
    synthetic_engine.sfx_runtime_receipt.level_index = 0;
    synthetic_engine.sfx_runtime_receipt.cd_track = 2;
    synthetic_engine.script_runtime_receipt.status =
        NEXUS_SCRIPT_RUNTIME_READY_PARSED;
    synthetic_engine.script_runtime_receipt.level_index = 0;
    synthetic_engine.script_runtime_receipt.candidate_source_loaded = 1;
    synthetic_engine.script_runtime_receipt.candidate_source_bytes = 2388;
    synthetic_engine.script_runtime_receipt.parser_supported = 1;
    synthetic_engine.script_runtime_receipt.dispatch_enabled = 1;
    synthetic_engine.script_runtime_receipt.rules_loaded = 2;
    synthetic_engine.current_level.width = NEXUS_MAX_MAP_SIZE;
    synthetic_engine.current_level.height = NEXUS_MAX_MAP_SIZE;
    synthetic_engine.current_level.geometry_info.dmweb_container = 1;
    synthetic_engine.current_level.geometry_info.mesh_ready = 1;
    synthetic_engine.current_level.geometry_info.geometry_offset = 0x9000;
    synthetic_engine.current_level.geometry_info.geometry_size = 2048;
    synthetic_engine.current_level.geometry_info.collision_ref_count = 4;
    synthetic_engine.current_level.geometry_info.collision_ref_unique_count = 1;
    synthetic_engine.current_level.geometry_info.max_collision_ref = 5;
    synthetic_engine.current_level.squares[4][3] = 1;
    synthetic_engine.current_level.squares[3][3] = 1;
    synthetic_engine.current_level.squares[4][4] = 1;
    synthetic_engine.current_level.collision_refs[4][3] = 0x0100;
    synthetic_engine.current_level.collision_refs[3][3] = 0x0fff;
    nexus_v1_champions_init(&synthetic_engine.champions);
    expect(nexus_v1_champion_recruit(&synthetic_engine.champions, 0) == 0,
           "Nexus synthetic runtime has a party for menu-to-runtime route");
    nexus_v1_launcher_runtime_receipt_clear(&synthetic_runtime_receipt);
    synthetic_runtime_receipt.engine = &synthetic_engine;
    synthetic_runtime_receipt.level_loaded = 1;
    synthetic_runtime_receipt.title_loaded = 1;
    synthetic_runtime_receipt.party_x = synthetic_engine.game.party_x;
    synthetic_runtime_receipt.party_y = synthetic_engine.game.party_y;
    synthetic_runtime_receipt.party_dir = synthetic_engine.game.party_dir;
    snprintf(synthetic_runtime_receipt.title,
             sizeof(synthetic_runtime_receipt.title),
             "%s",
             NEXUS_V1_GAME_LABEL);
    snprintf(synthetic_runtime_receipt.source_id,
             sizeof(synthetic_runtime_receipt.source_id),
             "%s",
             NEXUS_V1_GAME_ID);
    nexus_v1_launcher_startup_runtime_state_clear(&runtime_state);
    runtime_state.engine = &synthetic_engine;
    runtime_state.champion_select_active = 1;
    runtime_state.champion_cursor = 0;
    runtime_state.champion_frame = 0;
    expect(nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
               &runtime_state,
               &synthetic_runtime_receipt.startup_assets),
           "Nexus synthetic runtime assets build for route proof");
    expect(nexus_v1_launcher_startup_asset_handoff_from_runtime_receipt(
               &synthetic_runtime_receipt,
               &asset_handoff_receipt) &&
               asset_handoff_receipt.route ==
                   NEXUS_V1_STARTUP_ASSET_HANDOFF_MAIN_MENU_READY &&
               strcmp(nexus_v1_launcher_startup_asset_handoff_route_name(
                          asset_handoff_receipt.route),
                      "main-menu-ready") == 0 &&
               asset_handoff_receipt.title_asset_handoff_ready == 1 &&
               asset_handoff_receipt.real_menu_asset_handoff_ready == 1 &&
               asset_handoff_receipt.audio_asset_handoff_ready == 1 &&
               asset_handoff_receipt.main_menu_route_ready == 1 &&
               asset_handoff_receipt.saturn_asset_handoff_ready == 1 &&
               asset_handoff_receipt.real_asset_route_ready == 1 &&
               asset_handoff_receipt.menu_bpk_renderer_handoff_valid == 1 &&
               asset_handoff_receipt.menu_bpk_renderer_handoff.status ==
                   NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED &&
               asset_handoff_receipt.menu_bpk_prs3_blocks_real_menu_route == 0 &&
               asset_handoff_receipt.blocks_main_menu_route == 0 &&
               asset_handoff_receipt.fallback_visuals_permitted == 0 &&
               strcmp(asset_handoff_receipt.title_asset_route,
                      "ready-title-assets") == 0 &&
               strcmp(asset_handoff_receipt.menu_asset_route,
                      "ready-real-menu-surfaces") == 0 &&
               strcmp(asset_handoff_receipt.audio_asset_route,
                      "ready-track02-sfx") == 0,
           "Nexus startup asset handoff proves title menu audio readiness");
    memset(dgn_commands, 0, sizeof(dgn_commands));
    expect(nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
               &runtime_state,
               &champion_execution,
               NULL,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &runtime_handoff_receipt) &&
               runtime_handoff_receipt.route ==
                   NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE &&
               strcmp(nexus_v1_launcher_startup_runtime_handoff_route_name(
                          runtime_handoff_receipt.route),
                      "ready-render-state") == 0 &&
               runtime_handoff_receipt.runtime_ready == 1 &&
               runtime_handoff_receipt.dgn_render_ready == 1 &&
               runtime_handoff_receipt.hud_ready == 1 &&
               runtime_handoff_receipt.dgn_render_blocked == 0 &&
               runtime_handoff_receipt.script_receipt.status ==
                   NEXUS_SCRIPT_RUNTIME_READY_PARSED &&
               runtime_handoff_receipt.script_runtime_ready == 1 &&
               runtime_handoff_receipt.script_runtime_blocked == 0 &&
               runtime_handoff_receipt.asset_handoff.route ==
                   NEXUS_V1_STARTUP_ASSET_HANDOFF_MAIN_MENU_READY &&
               runtime_handoff_receipt.asset_handoff
                       .menu_bpk_renderer_handoff.status ==
                   NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED &&
               runtime_handoff_receipt.asset_handoff.real_asset_route_ready == 1 &&
               runtime_handoff_receipt.render_plan.plan_ready == 1 &&
               runtime_handoff_receipt.command_count > 0 &&
               runtime_handoff_receipt.fallback_visuals_permitted == 0 &&
               dgn_commands[0].kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
           "Nexus startup handoff builds first DGN render state after champion start");
    memset(dgn_commands, 0, sizeof(dgn_commands));
    expect(nexus_v1_launcher_startup_route_proof_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               &champion_execution,
               NULL,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &route_proof_receipt) &&
               route_proof_receipt.route ==
                   NEXUS_V1_STARTUP_ROUTE_PROOF_RUNTIME_READY &&
               strcmp(nexus_v1_launcher_startup_route_proof_route_name(
                          route_proof_receipt.route),
                      "runtime-ready") == 0 &&
               route_proof_receipt.saturn_asset_boot_ready == 1 &&
               route_proof_receipt.title_route_ready == 1 &&
               route_proof_receipt.menu_route_ready == 1 &&
               route_proof_receipt.startup_surfaces_real_ready == 1 &&
               route_proof_receipt.faces_real_ready == 1 &&
               route_proof_receipt.full_start_graphics_ready == 1 &&
               route_proof_receipt.save_load_menu_route_ready == 1 &&
               route_proof_receipt.startup_ui_route_ready == 1 &&
               strcmp(route_proof_receipt.startup_ui_blocker, "none") == 0 &&
               route_proof_receipt.runtime_route_ready == 1 &&
               route_proof_receipt.graphics_ready == 1 &&
               route_proof_receipt.audio_ready == 1 &&
               route_proof_receipt.startup_sfx_status ==
                   NEXUS_SFX_RUNTIME_READY_DECODED &&
               route_proof_receipt.startup_sfx_level_index == 0 &&
               route_proof_receipt.startup_cd_track == 2 &&
               route_proof_receipt.startup_sfx_blocks_real_playback == 0 &&
               route_proof_receipt.asset_handoff.route ==
                   NEXUS_V1_STARTUP_ASSET_HANDOFF_MAIN_MENU_READY &&
               route_proof_receipt.asset_handoff
                       .menu_bpk_renderer_handoff_valid == 1 &&
               route_proof_receipt.asset_handoff
                       .menu_bpk_renderer_handoff.status ==
                   NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED &&
               route_proof_receipt.asset_handoff.real_asset_route_ready == 1 &&
               route_proof_receipt.runtime_route_receipt.route ==
                   NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE &&
               route_proof_receipt.runtime_route_receipt.consumed_by_nexus == 1 &&
               route_proof_receipt.runtime_route_receipt.runtime_route_ready == 1 &&
               route_proof_receipt.runtime_route_receipt
                       .runtime_handoff.asset_handoff.real_asset_route_ready == 1 &&
               route_proof_receipt.title_menu_route_ready == 1 &&
               route_proof_receipt.menu_runtime_route_ready == 1 &&
               route_proof_receipt.first_runtime_route_ready == 1 &&
               route_proof_receipt.audio_runtime_route_ready == 1 &&
               route_proof_receipt.audio_runtime_route_blocked == 0 &&
               route_proof_receipt.script_runtime_status ==
                   NEXUS_SCRIPT_RUNTIME_READY_PARSED &&
               route_proof_receipt.script_candidate_source_bytes == 2388 &&
               route_proof_receipt.script_runtime_route_ready == 1 &&
               route_proof_receipt.script_runtime_route_blocked == 0 &&
               route_proof_receipt.full_startup_route_ready == 1 &&
               strcmp(route_proof_receipt.first_runtime_route,
                      "first-dgn-render-state") == 0 &&
               route_proof_receipt.fallback_visuals_permitted == 0 &&
               route_proof_receipt.runtime_handoff.command_count > 0,
           "Nexus startup route proof spans Saturn assets title menu and runtime");
    expect(nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               &full_start_receipt) &&
               full_start_receipt.route ==
                   NEXUS_V1_STARTUP_FULL_START_MENU_READY &&
               strcmp(nexus_v1_launcher_startup_full_start_route_name(
                          full_start_receipt.route),
                      "menu-ready") == 0 &&
               full_start_receipt.warning_art_loaded == 1 &&
               full_start_receipt.title_art_loaded == 1 &&
               full_start_receipt.warning_status_ready == 1 &&
               full_start_receipt.title_status_ready == 1 &&
               full_start_receipt.boot_warning_title_ready == 1 &&
               full_start_receipt.startup_surfaces_real_ready == 1 &&
               full_start_receipt.faces_real_ready == 1 &&
               full_start_receipt.menu_bpk_route_ready == 1 &&
               full_start_receipt.save_menu_route_ready == 1 &&
               full_start_receipt.champion_menu_route_ready == 1 &&
               full_start_receipt.save_status_ready == 1 &&
               full_start_receipt.champion_status_ready == 1 &&
               full_start_receipt.audio_track02_ready == 1 &&
               full_start_receipt.cd_track == 2 &&
               full_start_receipt.sfx_status ==
                   NEXUS_SFX_RUNTIME_READY_DECODED &&
               full_start_receipt.full_start_graphics_ready == 1 &&
               full_start_receipt.full_start_menu_ready == 1 &&
               full_start_receipt.m11_host_route_ready == 1 &&
               strcmp(full_start_receipt.m11_host_route,
                      "champion-menu") == 0 &&
               full_start_receipt.host_receipt.input_result ==
                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
               strcmp(full_start_receipt.host_receipt.status,
                      "NEXUS CHAMPIONS") == 0 &&
               full_start_receipt.fallback_visuals_permitted == 0 &&
               strcmp(full_start_receipt.startup_ui_blocker, "none") == 0,
           "Nexus full-start receipt gates warning title menus audio and graphics");
    {
        int old_faces_loaded = synthetic_engine.ui_faces_loaded;
        int old_faces_fallback = synthetic_engine.ui_faces_fallback;
        synthetic_engine.ui_faces_loaded = 19;
        synthetic_engine.ui_faces_fallback =
            synthetic_engine.ui_faces_expected - synthetic_engine.ui_faces_loaded;
        expect(nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
                   &synthetic_runtime_receipt,
                   &runtime_state,
                   &full_start_receipt) &&
                   full_start_receipt.faces_real_ready == 1 &&
                   full_start_receipt.full_start_graphics_ready == 1 &&
                   full_start_receipt.full_start_menu_ready == 1 &&
                   full_start_receipt.m11_host_route_ready == 1 &&
                   full_start_receipt.fallback_visuals_permitted == 0 &&
                   strcmp(full_start_receipt.m11_host_route,
                          "champion-menu") == 0,
               "Nexus package route accepts real FACE coverage with handled missing rows");
        synthetic_engine.ui_faces_loaded = old_faces_loaded;
        synthetic_engine.ui_faces_fallback = old_faces_fallback;
    }
    expect(nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               11,
               NULL,
               NULL,
               &full_start_consumer_receipt) &&
               full_start_consumer_receipt.m11_ready == 1 &&
               full_start_consumer_receipt.m12_ready == 1 &&
               full_start_consumer_receipt.redraw == 1 &&
               strcmp(full_start_consumer_receipt.consumer_route,
                      "champion-menu") == 0 &&
               full_start_consumer_receipt.presentation_valid == 1 &&
               full_start_consumer_receipt.title_handoff_valid == 0 &&
               full_start_consumer_receipt.save_route_valid == 0 &&
               full_start_consumer_receipt.draw_command_count > 3 &&
               strcmp(full_start_consumer_receipt.status,
                      "NEXUS CHAMPIONS") == 0,
           "Nexus full-start consumer receipt owns champion M11/M12 facts");
    runtime_snapshot.runtime = runtime_state;
    expect(nexus_v1_launcher_startup_full_start_consumer_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               11,
               NULL,
               NULL,
               &full_start_consumer_receipt) &&
               full_start_consumer_receipt.m11_ready == 1 &&
               strcmp(full_start_consumer_receipt.consumer_route,
                      "champion-menu") == 0 &&
               full_start_consumer_receipt.presentation_valid == 1,
           "Nexus full-start snapshot consumer owns champion M11/M12 facts");
    expect(nexus_v1_launcher_startup_full_start_package_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               11,
               NULL,
               NULL,
               &full_start_package_receipt) &&
               full_start_package_receipt.m11_ready == 1 &&
               full_start_package_receipt.m12_ready == 1 &&
               full_start_package_receipt.graphics_ready == 1 &&
               full_start_package_receipt.audio_ready == 1 &&
               full_start_package_receipt.champion_menu_ready == 1 &&
               strcmp(full_start_package_receipt.consumer_route,
                      "champion-menu") == 0 &&
               strcmp(full_start_package_receipt.animation,
                      "nexus-champion-select") == 0 &&
               full_start_package_receipt.capture_valid == 1 &&
               full_start_package_receipt.capture_route_ready == 1 &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
               full_start_package_receipt.champion_route_active == 1 &&
               full_start_package_receipt.save_route_active == 0 &&
               full_start_package_receipt.title_route_active == 0 &&
               full_start_package_receipt.champion_capture_ready == 1 &&
               full_start_package_receipt.capture_command_count > 3 &&
               full_start_package_receipt.saturn_warning_frame == 0 &&
               full_start_package_receipt.saturn_title_capture_frame == 48 &&
               full_start_package_receipt.saturn_champion_capture_frame == 102 &&
               full_start_package_receipt.saturn_save_capture_frame == -1 &&
               full_start_package_receipt.saturn_dungeon_capture_frame == -1 &&
               full_start_package_receipt.saturn_title_ready_frame == 102 &&
               full_start_package_receipt.saturn_gameover_capture_frame == 0 &&
               full_start_package_receipt.saturn_timing_exact == 1 &&
               full_start_package_receipt.saturn_capture_frames_exact == 1 &&
               full_start_package_receipt.full_start_package_receipt_ready == 1 &&
               full_start_package_receipt.host_display_caller_expected == 1 &&
               full_start_package_receipt.first_capture_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND &&
               strcmp(full_start_package_receipt.startup_ui_blocker,
                      "none") == 0,
           "Nexus full-start package owns champion startup capture proof");
    expect(nexus_v1_launcher_m12_startup_package_from_full_start_package(
               &full_start_package_receipt,
               &m12_package_receipt) &&
               m12_package_receipt.packaged_capture_ready == 1 &&
               m12_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
               m12_package_receipt.save_capture_frame == -1 &&
               m12_package_receipt.champion_capture_frame == 102 &&
               m12_package_receipt.dungeon_capture_frame == -1 &&
               m12_package_receipt.saturn_save_capture_frame == -1 &&
               m12_package_receipt.saturn_champion_capture_frame == 102 &&
               m12_package_receipt.saturn_dungeon_capture_frame == -1,
           "Nexus M12 startup package consumes CHAMPION capture receipt");
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_full_start_package_build_commands_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               11,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
               full_start_package_receipt.capture_command_count > 3 &&
               full_start_package_receipt.fallback_visuals_permitted == 0 &&
               draw_commands[0].kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
           "Nexus full-start package command helper owns champion draw route");
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_receipt_bundle_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               11,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &startup_bundle_receipt) &&
               startup_bundle_receipt.m11_ready == 1 &&
               startup_bundle_receipt.m12_ready == 1 &&
               startup_bundle_receipt.display_ready == 1 &&
               startup_bundle_receipt.blocked == 0 &&
               startup_bundle_receipt.capture_ready == 1 &&
               startup_bundle_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
               startup_bundle_receipt.first_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND &&
               startup_bundle_receipt.command_count > 3 &&
               startup_bundle_receipt.copied_command_count ==
                   startup_bundle_receipt.command_count &&
               startup_bundle_receipt.timing_frame == -1 &&
               startup_bundle_receipt.timing_frame_max ==
                   nexus_v1_boot_start_ready_frames() &&
               startup_bundle_receipt.timing_ready == 1 &&
               startup_bundle_receipt.saturn_timing_exact == 1 &&
               startup_bundle_receipt.saturn_capture_frames_exact == 1 &&
               strcmp(startup_bundle_receipt.route_label,
                      "champion-menu") == 0 &&
               strcmp(startup_bundle_receipt.first_draw_label,
                      "title-background") == 0 &&
               strcmp(startup_bundle_receipt.m12_package.launch_status_label,
                      "READY TO LAUNCH") == 0 &&
               draw_commands[0].kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
           "Nexus startup receipt bundle exports champion timing capture and M12 card facts");
    expect(nexus_v1_launcher_startup_real_asset_ownership_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               11,
               NULL,
               NULL,
               &real_asset_ownership_receipt) &&
               real_asset_ownership_receipt.route ==
                   NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF &&
               strcmp(nexus_v1_launcher_startup_real_asset_ownership_route_name(
                          real_asset_ownership_receipt.route),
                      "runtime-handoff") == 0 &&
               real_asset_ownership_receipt.receipt_owner_is_nexus == 1 &&
               real_asset_ownership_receipt.title_menu_receipt_owned == 1 &&
               real_asset_ownership_receipt.capture_receipt_owned == 1 &&
               real_asset_ownership_receipt.real_asset_receipt_owned == 1 &&
               real_asset_ownership_receipt.consumes_bpk_menu_handoff == 1 &&
               real_asset_ownership_receipt.consumes_prs3_blocker == 0 &&
               real_asset_ownership_receipt.consumes_dgn_handoff == 1 &&
               real_asset_ownership_receipt.menu_bpk_handoff.status ==
                   NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED &&
               real_asset_ownership_receipt.dgn_handoff.status ==
                   NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH &&
               real_asset_ownership_receipt.dgn_render_plan.plan_ready == 1 &&
               real_asset_ownership_receipt.runtime_dgn_handoff_ready == 1 &&
               real_asset_ownership_receipt.menu_capture_uses_real_assets == 1 &&
               real_asset_ownership_receipt.full_start_package_consumed == 1 &&
               real_asset_ownership_receipt.package_capture_consumed_by_host == 1 &&
               real_asset_ownership_receipt.title_menu_capture_route_joined == 1 &&
               real_asset_ownership_receipt.bpk_menu_route_joined == 1 &&
               real_asset_ownership_receipt.runtime_dgn_route_joined == 1 &&
               real_asset_ownership_receipt.first_host_draw_uses_package == 1 &&
               real_asset_ownership_receipt.saturn_timing_exact == 1 &&
               real_asset_ownership_receipt.saturn_capture_frames_exact == 1 &&
               real_asset_ownership_receipt.saturn_champion_capture_frame == 102 &&
               real_asset_ownership_receipt.saturn_save_capture_frame == -1 &&
               real_asset_ownership_receipt.saturn_dungeon_capture_frame == 102 &&
               real_asset_ownership_receipt.no_fallback_visuals_enforced == 1 &&
               real_asset_ownership_receipt.fallback_visuals_permitted == 0 &&
               real_asset_ownership_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
               real_asset_ownership_receipt.first_startup_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND &&
               real_asset_ownership_receipt.first_dgn_draw_kind ==
                   NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
               real_asset_ownership_receipt.startup_draw_command_count > 3 &&
               real_asset_ownership_receipt.dgn_draw_command_count > 0 &&
               strcmp(real_asset_ownership_receipt.receipt_owner,
                      "nexus-v1-launcher") == 0 &&
               strcmp(real_asset_ownership_receipt.status,
                      "runtime-handoff-owned") == 0,
           "Nexus real-asset ownership receipt joins title menu capture and DGN handoff without fallback");
    memset(draw_commands, 0, sizeof(draw_commands));
    memset(dgn_commands, 0, sizeof(dgn_commands));
    expect(nexus_v1_launcher_startup_host_caller_receipt_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               11,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &host_caller_receipt) &&
               host_caller_receipt.host_caller_ready == 1 &&
               host_caller_receipt.receipt_owner_is_nexus == 1 &&
               host_caller_receipt.host_startup_capture_ready == 1 &&
               host_caller_receipt.host_runtime_dgn_ready == 1 &&
               host_caller_receipt.host_execute_startup_draws == 1 &&
               host_caller_receipt.host_execute_dgn_draws == 1 &&
               host_caller_receipt.bpk_handoff_consumed == 1 &&
               host_caller_receipt.prs3_blocker_consumed == 0 &&
               host_caller_receipt.dgn_handoff_consumed == 1 &&
               host_caller_receipt.full_start_package_consumed == 1 &&
               host_caller_receipt.package_capture_consumed_by_host == 1 &&
               host_caller_receipt.startup_bundle_consumed == 1 &&
               host_caller_receipt.display_callers_use_package_receipt == 1 &&
               host_caller_receipt.single_saturn_startup_owner_ready == 1 &&
               host_caller_receipt.title_menu_capture_route_joined == 1 &&
               host_caller_receipt.runtime_dgn_route_joined == 1 &&
               host_caller_receipt.suppress_fallback_visuals == 1 &&
               host_caller_receipt.suppress_legacy_placeholder_visuals == 1 &&
               host_caller_receipt.no_fallback_visuals_enforced == 1 &&
               host_caller_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
               host_caller_receipt.ownership_route ==
                   NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF &&
               host_caller_receipt.startup_command_count > 3 &&
               host_caller_receipt.copied_startup_command_count ==
                   host_caller_receipt.startup_command_count &&
               host_caller_receipt.dgn_command_count > 0 &&
               host_caller_receipt.copied_dgn_command_count ==
                   host_caller_receipt.dgn_command_count &&
               host_caller_receipt.title_timing_ready == 1 &&
               host_caller_receipt.saturn_warning_frame == 0 &&
               host_caller_receipt.saturn_title_capture_frame == 48 &&
               host_caller_receipt.saturn_champion_capture_frame == 102 &&
               host_caller_receipt.saturn_save_capture_frame == -1 &&
               host_caller_receipt.saturn_dungeon_capture_frame == 102 &&
               host_caller_receipt.saturn_title_ready_frame == 102 &&
               host_caller_receipt.saturn_gameover_capture_frame == 0 &&
               host_caller_receipt.saturn_timing_exact == 1 &&
               host_caller_receipt.saturn_capture_frames_exact == 1 &&
               draw_commands[0].kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND &&
               dgn_commands[0].kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
               strcmp(host_caller_receipt.host_route,
                      "runtime-dgn-handoff") == 0 &&
               strcmp(host_caller_receipt.status,
                      "runtime-handoff-owned") == 0,
           "Nexus host-caller receipt owns startup capture and DGN draw commands without fallback");
    runtime_snapshot.runtime = runtime_state;
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
               NULL,
               &runtime_snapshot,
               0,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_CHAMPION &&
               full_start_package_receipt.capture_command_count > 3 &&
               full_start_package_receipt.fallback_visuals_permitted == 0 &&
               draw_commands[0].kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
           "Nexus package snapshot helper owns M11 champion capture without runtime receipt");
    runtime_state.champion_select_active = 0;
    runtime_state.save_select_active = 1;
    expect(nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               &full_start_receipt) &&
               full_start_receipt.m11_host_route_ready == 1 &&
               strcmp(full_start_receipt.m11_host_route,
                      "save-menu") == 0 &&
               strcmp(full_start_receipt.host_receipt.status,
                      "NEXUS SAVE SELECT") == 0,
           "Nexus full-start receipt owns save-menu host route");
    expect(nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               2,
               NULL,
               NULL,
               &full_start_consumer_receipt) &&
               full_start_consumer_receipt.m11_ready == 1 &&
               full_start_consumer_receipt.m12_ready == 1 &&
               strcmp(full_start_consumer_receipt.consumer_route,
                      "save-menu") == 0 &&
               full_start_consumer_receipt.presentation_valid == 1 &&
               full_start_consumer_receipt.presentation.kind ==
                   NEXUS_V1_STARTUP_MENU_PRESENTATION_SAVE &&
               full_start_consumer_receipt.save_route_valid == 1 &&
               full_start_consumer_receipt.draw_command_count > 3 &&
               strcmp(full_start_consumer_receipt.status,
                      "NEXUS SAVE SELECT") == 0,
           "Nexus full-start consumer receipt owns save-menu M11/M12 facts");
    runtime_snapshot.runtime = runtime_state;
    expect(nexus_v1_launcher_startup_full_start_consumer_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               2,
               NULL,
               NULL,
               &full_start_consumer_receipt) &&
               full_start_consumer_receipt.m11_ready == 1 &&
               strcmp(full_start_consumer_receipt.consumer_route,
                      "save-menu") == 0 &&
               full_start_consumer_receipt.presentation_valid == 1 &&
               full_start_consumer_receipt.save_route_valid == 1,
           "Nexus full-start snapshot consumer owns save M11/M12 facts");
    expect(nexus_v1_launcher_startup_full_start_package_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               2,
               NULL,
               NULL,
               &full_start_package_receipt) &&
               full_start_package_receipt.m11_ready == 1 &&
               full_start_package_receipt.save_menu_ready == 1 &&
               strcmp(full_start_package_receipt.consumer_route,
                      "save-menu") == 0 &&
               full_start_package_receipt.consumer.presentation_valid == 1 &&
               full_start_package_receipt.consumer.save_route_valid == 1 &&
               full_start_package_receipt.capture_valid == 1 &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_SAVE &&
               full_start_package_receipt.save_route_active == 1 &&
               full_start_package_receipt.champion_route_active == 0 &&
               full_start_package_receipt.title_route_active == 0 &&
               full_start_package_receipt.save_capture_ready == 1 &&
               full_start_package_receipt.saturn_save_capture_frame == 102 &&
               full_start_package_receipt.saturn_champion_capture_frame == -1 &&
               full_start_package_receipt.saturn_dungeon_capture_frame == -1 &&
               full_start_package_receipt.capture_command_count > 3 &&
               full_start_package_receipt.first_capture_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
           "Nexus full-start package owns save startup capture proof");
    expect(nexus_v1_launcher_m12_startup_package_from_full_start_package(
               &full_start_package_receipt,
               &m12_package_receipt) &&
               m12_package_receipt.packaged_capture_ready == 1 &&
               m12_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_SAVE &&
               m12_package_receipt.save_capture_frame == 102 &&
               m12_package_receipt.champion_capture_frame == -1 &&
               m12_package_receipt.dungeon_capture_frame == -1 &&
               m12_package_receipt.saturn_save_capture_frame == 102 &&
               m12_package_receipt.saturn_champion_capture_frame == -1 &&
               m12_package_receipt.saturn_dungeon_capture_frame == -1,
           "Nexus M12 startup package consumes SAVE capture receipt");
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               2,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_SAVE &&
               full_start_package_receipt.capture_command_count > 3 &&
               full_start_package_receipt.fallback_visuals_permitted == 0 &&
               draw_commands[0].kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
           "Nexus full-start package command helper owns save draw route");
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
               NULL,
               &runtime_snapshot,
               0,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_SAVE &&
               full_start_package_receipt.capture_command_count > 3 &&
               full_start_package_receipt.fallback_visuals_permitted == 0 &&
               draw_commands[0].kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
           "Nexus package snapshot helper owns M11 save capture without runtime receipt");
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_receipt_bundle_from_snapshot(
               NULL,
               &runtime_snapshot,
               0,
               NULL,
               NULL,
               draw_commands,
               2,
               &startup_bundle_receipt) &&
               startup_bundle_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_SAVE &&
               startup_bundle_receipt.command_count > 3 &&
               startup_bundle_receipt.copied_command_count == 2 &&
               startup_bundle_receipt.display_ready == 1 &&
               strcmp(startup_bundle_receipt.route_label,
                      "save-menu") == 0 &&
               draw_commands[0].kind ==
                   NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
           "Nexus startup receipt bundle caps copied save commands for M11 capture buffers");
    runtime_state.save_select_active = 0;
    runtime_state.title_active = 1;
    runtime_state.title_frame = nexus_v1_boot_start_ready_frames();
    expect(nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               &full_start_receipt) &&
               full_start_receipt.m11_host_route_ready == 1 &&
               strcmp(full_start_receipt.m11_host_route,
                      "title-warning") == 0 &&
               strcmp(full_start_receipt.host_receipt.status,
                      "NEXUS TITLE") == 0,
           "Nexus full-start receipt owns title/warning host route");
    expect(nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               9,
               NULL,
               NULL,
               &full_start_consumer_receipt) &&
               full_start_consumer_receipt.m11_ready == 1 &&
               full_start_consumer_receipt.m12_ready == 1 &&
               strcmp(full_start_consumer_receipt.consumer_route,
                      "title-warning") == 0 &&
               full_start_consumer_receipt.title_handoff_valid == 1 &&
               full_start_consumer_receipt.title_handoff.title_draw_ready == 1 &&
               full_start_consumer_receipt.presentation_valid == 0 &&
               full_start_consumer_receipt.save_route_valid == 0 &&
               strcmp(full_start_consumer_receipt.status,
                      "NEXUS TITLE") == 0,
           "Nexus full-start consumer receipt owns title M11/M12 facts");
    runtime_snapshot.runtime = runtime_state;
    expect(nexus_v1_launcher_startup_full_start_consumer_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               9,
               NULL,
               NULL,
               &full_start_consumer_receipt) &&
               full_start_consumer_receipt.m11_ready == 1 &&
               strcmp(full_start_consumer_receipt.consumer_route,
                      "title-warning") == 0 &&
               full_start_consumer_receipt.title_handoff_valid == 1,
           "Nexus full-start snapshot consumer owns title M11/M12 facts");
    expect(nexus_v1_launcher_startup_full_start_package_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               9,
               NULL,
               NULL,
               &full_start_package_receipt) &&
               full_start_package_receipt.m11_ready == 1 &&
               full_start_package_receipt.title_frame >= 0 &&
               full_start_package_receipt.title_frame_max > 0 &&
               full_start_package_receipt.boot_warning_frames == 48 &&
               full_start_package_receipt.boot_start_ready_frames == 102 &&
               full_start_package_receipt.boot_frame_in_phase == 54 &&
               full_start_package_receipt.title_frames_until_ready == 0 &&
               full_start_package_receipt.title_hold_frame == 24 &&
               full_start_package_receipt.title_prompt_visible == 1 &&
               full_start_package_receipt.title_reveal_y0 == 0 &&
               full_start_package_receipt.title_reveal_y1 == NEXUS_FB_H &&
               full_start_package_receipt.title_reveal_h == NEXUS_FB_H &&
               full_start_package_receipt.warning_surface_loaded == 1 &&
               full_start_package_receipt.title_surface_loaded == 1 &&
               full_start_package_receipt.gameover_surface_loaded == 1 &&
               full_start_package_receipt.warning_capture_surface_ready == 1 &&
               full_start_package_receipt.title_capture_surface_ready == 1 &&
               full_start_package_receipt.gameover_capture_surface_ready == 1 &&
               full_start_package_receipt.warning_capture_frame == 0 &&
               full_start_package_receipt.title_capture_frame == 48 &&
               full_start_package_receipt.gameover_capture_frame == 0 &&
               full_start_package_receipt.saturn_warning_frame == 0 &&
               full_start_package_receipt.saturn_title_capture_frame == 48 &&
               full_start_package_receipt.saturn_title_ready_frame == 102 &&
               full_start_package_receipt.saturn_gameover_capture_frame == 0 &&
               full_start_package_receipt.saturn_timing_exact == 1 &&
               full_start_package_receipt.saturn_capture_frames_exact == 1 &&
               full_start_package_receipt.full_start_package_receipt_ready == 1 &&
               full_start_package_receipt.host_display_caller_expected == 1 &&
               strcmp(full_start_package_receipt.consumer_route,
                      "title-warning") == 0 &&
               strcmp(full_start_package_receipt.animation,
                      "nexus-title") == 0 &&
               full_start_package_receipt.consumer.title_handoff_valid == 1 &&
               full_start_package_receipt.capture_valid == 1 &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_TITLE &&
               full_start_package_receipt.title_route_active == 1 &&
               full_start_package_receipt.save_route_active == 0 &&
               full_start_package_receipt.champion_route_active == 0 &&
               full_start_package_receipt.title_capture_ready == 1 &&
               full_start_package_receipt.capture_command_count > 0 &&
               full_start_package_receipt.first_capture_draw_kind !=
                   NEXUS_V1_STARTUP_DRAW_NONE,
           "Nexus full-start package owns title startup capture proof");
    expect(nexus_v1_launcher_m12_startup_package_from_full_start_package(
               &full_start_package_receipt,
               &m12_package_receipt) &&
               m12_package_receipt.packaged_capture_ready == 1 &&
               m12_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_TITLE &&
               m12_package_receipt.first_capture_draw_kind ==
                   full_start_package_receipt.first_capture_draw_kind &&
               m12_package_receipt.warning_surface_loaded == 1 &&
               m12_package_receipt.title_surface_loaded == 1 &&
               m12_package_receipt.gameover_surface_loaded == 1 &&
               m12_package_receipt.warning_capture_surface_ready == 1 &&
               m12_package_receipt.title_capture_surface_ready == 1 &&
               m12_package_receipt.gameover_capture_surface_ready == 1 &&
               m12_package_receipt.warning_capture_frame == 0 &&
               m12_package_receipt.title_capture_frame == 48 &&
               m12_package_receipt.save_capture_frame == -1 &&
               m12_package_receipt.champion_capture_frame == -1 &&
               m12_package_receipt.dungeon_capture_frame == -1 &&
               m12_package_receipt.gameover_capture_frame == 0 &&
               m12_package_receipt.saturn_warning_frame == 0 &&
               m12_package_receipt.saturn_title_capture_frame == 48 &&
               m12_package_receipt.saturn_save_capture_frame == -1 &&
               m12_package_receipt.saturn_champion_capture_frame == -1 &&
               m12_package_receipt.saturn_dungeon_capture_frame == -1 &&
               m12_package_receipt.saturn_title_ready_frame == 102 &&
               m12_package_receipt.saturn_gameover_capture_frame == 0 &&
               m12_package_receipt.saturn_timing_exact == 1 &&
               m12_package_receipt.saturn_capture_frames_exact == 1 &&
               m12_package_receipt.full_start_package_receipt_ready == 1 &&
               m12_package_receipt.host_display_caller_expected == 1 &&
               m12_package_receipt.capture_command_count ==
                   full_start_package_receipt.capture_command_count,
           "Nexus M12 startup package consumes full-start TITLE/WARNING/GAMEOVER capture receipt");
    mutated_package_receipt = full_start_package_receipt;
    mutated_package_receipt.title_frame_max =
        nexus_v1_boot_start_ready_frames() - 1;
    mutated_package_receipt.saturn_timing_exact = 0;
    mutated_package_receipt.full_start_package_receipt_ready = 0;
    mutated_package_receipt.host_display_caller_expected = 0;
    expect(nexus_v1_launcher_m12_startup_package_from_full_start_package(
               &mutated_package_receipt,
               &m12_package_receipt) &&
               m12_package_receipt.packaged_capture_ready == 0 &&
               m12_package_receipt.full_start_package_receipt_ready == 0 &&
               m12_package_receipt.host_display_caller_expected == 0 &&
               strcmp(m12_package_receipt.launch_status_label,
                      "READY TO LAUNCH") != 0,
           "Nexus M12 startup package rejects non-exact Saturn title timing");
    mutated_package_receipt = full_start_package_receipt;
    mutated_package_receipt.title_capture_frame =
        nexus_v1_boot_warning_frames() + 1;
    mutated_package_receipt.saturn_title_capture_frame =
        mutated_package_receipt.title_capture_frame;
    mutated_package_receipt.saturn_capture_frames_exact = 0;
    mutated_package_receipt.full_start_package_receipt_ready = 0;
    mutated_package_receipt.host_display_caller_expected = 0;
    expect(nexus_v1_launcher_m12_startup_package_from_full_start_package(
               &mutated_package_receipt,
               &m12_package_receipt) &&
               m12_package_receipt.packaged_capture_ready == 0 &&
               m12_package_receipt.saturn_title_capture_frame ==
                   nexus_v1_boot_warning_frames() + 1 &&
               m12_package_receipt.saturn_capture_frames_exact == 0 &&
               m12_package_receipt.host_display_caller_expected == 0,
           "Nexus M12 startup package rejects off-by-one Saturn capture frame");
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_receipt_bundle_from_snapshot(
               NULL,
               &runtime_snapshot,
               0,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &startup_bundle_receipt) &&
               startup_bundle_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_TITLE &&
               startup_bundle_receipt.command_count > 0 &&
               startup_bundle_receipt.warning_visible == 0 &&
               startup_bundle_receipt.prompt_visible == 1 &&
               startup_bundle_receipt.timing_ready == 1 &&
               strcmp(startup_bundle_receipt.route_label,
                      "title-warning") == 0 &&
               strcmp(startup_bundle_receipt.m12_package.capture_label,
                      "NEXUS TIMING CAPTURE PROOF") == 0 &&
               draw_commands[0].kind ==
                   startup_bundle_receipt.first_draw_kind &&
               startup_bundle_receipt.first_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_BOOT_TITLE_FRAME,
           "Nexus startup receipt bundle exports title warning capture timing");
    memset(package_phase, 0, sizeof(package_phase));
    memset(package_animation, 0, sizeof(package_animation));
    expect(nexus_v1_launcher_startup_full_start_package_export_presentation(
               &full_start_package_receipt,
               package_phase,
               (int)sizeof(package_phase),
               &package_startup_active,
               &package_startup_frame,
               package_animation,
               (int)sizeof(package_animation),
               &package_animation_active,
               &package_title_frame,
               &package_title_frame_max,
               &package_title_ready) &&
               strcmp(package_phase, full_start_package_receipt.phase) == 0 &&
               strcmp(package_animation, "nexus-title") == 0 &&
               package_startup_active ==
                   full_start_package_receipt.startup_active &&
               package_startup_frame ==
                   full_start_package_receipt.startup_frame &&
               package_animation_active ==
                   full_start_package_receipt.animation_active &&
               package_title_frame == full_start_package_receipt.title_frame &&
               package_title_frame_max ==
                   full_start_package_receipt.title_frame_max &&
               package_title_ready == full_start_package_receipt.title_ready,
           "Nexus full-start package exports M11 presentation fields");
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               9,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_TITLE &&
               full_start_package_receipt.capture_command_count > 0 &&
               draw_commands[0].kind != NEXUS_V1_STARTUP_DRAW_NONE,
           "Nexus full-start package command helper owns title draw route");
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
               NULL,
               &runtime_snapshot,
               0,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_TITLE &&
               full_start_package_receipt.capture_command_count > 0 &&
               full_start_package_receipt.fallback_visuals_permitted == 0 &&
               draw_commands[0].kind != NEXUS_V1_STARTUP_DRAW_NONE,
           "Nexus package snapshot helper owns M11 title capture without runtime receipt");
    runtime_state.title_frame = 0;
    runtime_snapshot.runtime = runtime_state;
    expect(nexus_v1_launcher_startup_full_start_package_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               0,
               NULL,
               NULL,
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_TITLE &&
               full_start_package_receipt.boot_warning_frames == 48 &&
               full_start_package_receipt.boot_start_ready_frames == 102 &&
               full_start_package_receipt.boot_frame_in_phase == 0 &&
               full_start_package_receipt.title_frames_until_ready == 102 &&
               full_start_package_receipt.warning_visible == 1 &&
               full_start_package_receipt.warning_capture_surface_ready == 1 &&
               full_start_package_receipt.title_capture_surface_ready == 1 &&
               full_start_package_receipt.gameover_capture_surface_ready == 1 &&
               full_start_package_receipt.first_capture_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND &&
               strcmp(full_start_package_receipt.animation,
                      "nexus-title") == 0,
           "Nexus full-start package owns warning startup capture proof");
    expect(nexus_v1_launcher_m12_startup_package_from_full_start_package(
               &full_start_package_receipt,
               &m12_package_receipt) &&
               m12_package_receipt.packaged_capture_ready == 1 &&
               m12_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_TITLE &&
               m12_package_receipt.first_capture_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND &&
               m12_package_receipt.warning_capture_surface_ready == 1 &&
               m12_package_receipt.title_capture_surface_ready == 1 &&
               m12_package_receipt.gameover_capture_surface_ready == 1 &&
               m12_package_receipt.title_frames_until_ready == 102,
           "Nexus M12 startup package consumes WARNING frame capture receipt");
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               0,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_TITLE &&
               full_start_package_receipt.warning_visible == 1 &&
               full_start_package_receipt.capture_command_count == 1 &&
               draw_commands[0].kind ==
                   NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND,
           "Nexus full-start package command helper owns warning draw route");
    memset(draw_commands, 0, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
               NULL,
               &runtime_snapshot,
               0,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_TITLE &&
               full_start_package_receipt.warning_visible == 1 &&
               full_start_package_receipt.capture_command_count == 1 &&
               full_start_package_receipt.fallback_visuals_permitted == 0 &&
               draw_commands[0].kind ==
                   NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND,
           "Nexus package snapshot helper owns M11 warning capture without runtime receipt");
    runtime_state.title_frame = nexus_v1_boot_start_ready_frames();
    runtime_state.title_active = 0;
    runtime_state.champion_select_active = 1;
    memset(dgn_commands, 0, sizeof(dgn_commands));
    expect(nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input(
               &runtime_state,
               11,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &runtime_handoff_receipt) &&
               runtime_handoff_receipt.route ==
                   NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE &&
               runtime_handoff_receipt.champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON &&
               runtime_handoff_receipt.host_action_receipt
                       .champion_state_receipt_valid &&
               runtime_handoff_receipt.command_count > 0 &&
               dgn_commands[0].kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
           "Nexus startup Action input routes menu directly to first DGN render state");
    memset(dgn_commands, 0, sizeof(dgn_commands));
    expect(nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input(
               &runtime_state,
               11,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &runtime_route_receipt) &&
               runtime_route_receipt.route ==
                   NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE &&
               runtime_route_receipt.host_action_valid == 1 &&
               runtime_route_receipt.consumed_by_nexus == 1 &&
               runtime_route_receipt.runtime_route_ready == 1 &&
               runtime_route_receipt.runtime_route_blocked == 0 &&
               runtime_route_receipt.startup_sfx_status ==
                   NEXUS_SFX_RUNTIME_READY_DECODED &&
               runtime_route_receipt.startup_sfx_level_index == 0 &&
               runtime_route_receipt.startup_cd_track == 2 &&
               runtime_route_receipt.startup_audio_handoff_ready == 1 &&
               runtime_route_receipt.startup_sfx_blocks_real_playback == 0 &&
               runtime_route_receipt.dgn_handoff_status ==
                   NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH &&
               runtime_route_receipt.dgn_render_plan_status ==
                   NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH &&
               runtime_route_receipt.dgn_render_plan_ready == 1 &&
               runtime_route_receipt.dgn_render_command_count > 0 &&
               runtime_route_receipt.dgn_render_floor_count > 0 &&
               runtime_route_receipt.first_dgn_render_command_kind ==
                   NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
               runtime_route_receipt.dgn_blocks_real_mesh_render == 0 &&
               runtime_route_receipt.script_runtime_status ==
                   NEXUS_SCRIPT_RUNTIME_READY_PARSED &&
               runtime_route_receipt.script_candidate_source_bytes == 2388 &&
               runtime_route_receipt.script_rules_loaded == 2 &&
               runtime_route_receipt.script_runtime_ready == 1 &&
               runtime_route_receipt.script_runtime_blocked == 0 &&
               runtime_route_receipt.fallback_visuals_permitted == 0 &&
               runtime_route_receipt.runtime_handoff.asset_handoff
                       .real_asset_route_ready == 1 &&
               runtime_route_receipt.runtime_handoff.command_count > 0 &&
               dgn_commands[0].kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
           "Nexus runtime route receipt consumes host action and DGN route");
    synthetic_engine.script_runtime_receipt.status =
        NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT;
    synthetic_engine.script_runtime_receipt.parser_supported = 0;
    synthetic_engine.script_runtime_receipt.dispatch_enabled = 0;
    synthetic_engine.script_runtime_receipt.rules_loaded = 0;
    synthetic_engine.script_runtime_receipt.blocks_real_script_dispatch = 1;
    memset(dgn_commands, 0, sizeof(dgn_commands));
    expect(nexus_v1_launcher_startup_route_proof_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               &champion_execution,
               NULL,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &route_proof_receipt) &&
               route_proof_receipt.runtime_route_ready == 1 &&
               route_proof_receipt.first_runtime_route_ready == 1 &&
               route_proof_receipt.script_runtime_status ==
                   NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT &&
               route_proof_receipt.script_candidate_source_bytes == 2388 &&
               route_proof_receipt.script_runtime_route_ready == 0 &&
               route_proof_receipt.script_runtime_route_blocked == 1 &&
               route_proof_receipt.full_startup_route_ready == 0 &&
               route_proof_receipt.fallback_visuals_permitted == 0,
           "Nexus startup route proof exposes script parser blocker");
    synthetic_engine.sfx_runtime_receipt.status =
        NEXUS_SFX_RUNTIME_BLOCKED_MISSING_ASSET;
    synthetic_engine.sfx_runtime_receipt.level_index = -1;
    synthetic_engine.sfx_runtime_receipt.cd_track = -1;
    synthetic_engine.sfx_runtime_receipt.blocks_real_sfx_playback = 1;
    expect(nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
               &runtime_state,
               &synthetic_runtime_receipt.startup_assets),
           "Nexus synthetic audio-blocked assets rebuild for route proof");
    memset(dgn_commands, 0, sizeof(dgn_commands));
    expect(nexus_v1_launcher_startup_route_proof_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               &champion_execution,
               NULL,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &route_proof_receipt) &&
               route_proof_receipt.route ==
                   NEXUS_V1_STARTUP_ROUTE_PROOF_ASSET_BLOCKED &&
               route_proof_receipt.startup_sfx_status ==
                   NEXUS_SFX_RUNTIME_BLOCKED_MISSING_ASSET &&
               route_proof_receipt.startup_sfx_level_index == -1 &&
               route_proof_receipt.startup_cd_track == -1 &&
               route_proof_receipt.startup_sfx_blocks_real_playback == 1 &&
               route_proof_receipt.audio_ready == 0 &&
               route_proof_receipt.audio_runtime_route_ready == 0 &&
               route_proof_receipt.audio_runtime_route_blocked == 1 &&
               route_proof_receipt.full_start_graphics_ready == 1 &&
               route_proof_receipt.save_load_menu_route_ready == 0 &&
               route_proof_receipt.startup_ui_route_ready == 0 &&
               strcmp(route_proof_receipt.startup_ui_blocker,
                      "track02-sfx") == 0 &&
               route_proof_receipt.asset_handoff.audio_asset_handoff_ready == 0 &&
               route_proof_receipt.runtime_route_receipt.startup_sfx_status ==
                   NEXUS_SFX_RUNTIME_BLOCKED_MISSING_ASSET &&
               strcmp(route_proof_receipt.status,
                      "blocked-track02-sfx") == 0,
           "Nexus startup route proof exposes Track 02 SFX blocker");
    expect(nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               &full_start_receipt) &&
               full_start_receipt.route ==
                   NEXUS_V1_STARTUP_FULL_START_BLOCKED_ASSETS &&
               full_start_receipt.warning_art_loaded == 1 &&
               full_start_receipt.title_art_loaded == 1 &&
               full_start_receipt.boot_warning_title_ready == 1 &&
               full_start_receipt.full_start_graphics_ready == 1 &&
               full_start_receipt.save_menu_route_ready == 0 &&
               full_start_receipt.champion_menu_route_ready == 0 &&
               full_start_receipt.audio_track02_ready == 0 &&
               full_start_receipt.cd_track == -1 &&
               full_start_receipt.sfx_status ==
                   NEXUS_SFX_RUNTIME_BLOCKED_MISSING_ASSET &&
               full_start_receipt.sfx_blocks_real_playback == 1 &&
               full_start_receipt.full_start_menu_ready == 0 &&
               full_start_receipt.m11_host_route_ready == 0 &&
               strcmp(full_start_receipt.m11_host_route,
                      "blocked-startup") == 0 &&
               full_start_receipt.host_receipt.input_result ==
                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
               strcmp(full_start_receipt.startup_ui_blocker,
                      "track02-sfx") == 0 &&
               strcmp(full_start_receipt.status,
                      "blocked-track02-sfx") == 0,
           "Nexus full-start receipt blocks on Track 02 SFX handoff");
    expect(nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               11,
               NULL,
               NULL,
               &full_start_consumer_receipt) &&
               full_start_consumer_receipt.m11_ready == 0 &&
               full_start_consumer_receipt.m12_ready == 0 &&
               strcmp(full_start_consumer_receipt.consumer_route,
                      "blocked-startup") == 0 &&
               full_start_consumer_receipt.presentation_valid == 0 &&
               strcmp(full_start_consumer_receipt.status,
                      "blocked-track02-sfx") == 0,
           "Nexus full-start consumer receipt blocks M11/M12 on SFX");
    runtime_snapshot.runtime = runtime_state;
    expect(nexus_v1_launcher_startup_full_start_consumer_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               11,
               NULL,
               NULL,
               &full_start_consumer_receipt) &&
               full_start_consumer_receipt.m11_ready == 0 &&
               strcmp(full_start_consumer_receipt.consumer_route,
                      "blocked-startup") == 0 &&
               full_start_consumer_receipt.presentation_valid == 0,
           "Nexus full-start snapshot consumer blocks M11/M12 on SFX");
    expect(nexus_v1_launcher_startup_full_start_package_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               11,
               NULL,
               NULL,
               &full_start_package_receipt) &&
               full_start_package_receipt.m11_ready == 0 &&
               full_start_package_receipt.audio_ready == 0 &&
               full_start_package_receipt.graphics_ready == 1 &&
               strcmp(full_start_package_receipt.consumer_route,
                      "blocked-startup") == 0 &&
               full_start_package_receipt.capture_valid == 1 &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_BLOCKED &&
               full_start_package_receipt.capture_route_ready == 0 &&
               full_start_package_receipt.capture_command_count == 0 &&
               full_start_package_receipt.blocked_draw_suppressed == 1 &&
               full_start_package_receipt.first_capture_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_NONE &&
               strcmp(full_start_package_receipt.startup_ui_blocker,
                      "track02-sfx") == 0,
           "Nexus full-start package blocks startup capture proof on SFX");
    memset(draw_commands, 0x7f, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               11,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_BLOCKED &&
               full_start_package_receipt.capture_command_count == 0 &&
               full_start_package_receipt.blocked_draw_suppressed == 1 &&
               draw_commands[0].kind == NEXUS_V1_STARTUP_DRAW_NONE,
           "Nexus full-start package command helper blocks SFX fallback draw");
    synthetic_engine.sfx_runtime_receipt.status =
        NEXUS_SFX_RUNTIME_READY_DECODED;
    synthetic_engine.sfx_runtime_receipt.level_index = 0;
    synthetic_engine.sfx_runtime_receipt.cd_track = 2;
    synthetic_engine.sfx_runtime_receipt.blocks_real_sfx_playback = 0;
    expect(nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
               &runtime_state,
               &synthetic_runtime_receipt.startup_assets),
           "Nexus synthetic audio-ready assets restore for later route proof");
    synthetic_engine.script_runtime_receipt.status =
        NEXUS_SCRIPT_RUNTIME_READY_PARSED;
    synthetic_engine.script_runtime_receipt.parser_supported = 1;
    synthetic_engine.script_runtime_receipt.dispatch_enabled = 1;
    synthetic_engine.script_runtime_receipt.rules_loaded = 2;
    synthetic_engine.script_runtime_receipt.blocks_real_script_dispatch = 0;
    expect(nexus_v1_startup_champion_footer_rect(&footer_rect),
           "Nexus champion footer rect builds for pointer route");
    runtime_snapshot.runtime = runtime_state;
    memset(dgn_commands, 0, sizeof(dgn_commands));
    expect(nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer_snapshot(
               &runtime_snapshot,
               footer_rect.x + 1,
               footer_rect.y + 1,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &runtime_handoff_receipt) &&
               runtime_handoff_receipt.route ==
                   NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE &&
               runtime_handoff_receipt.command_count > 0 &&
               strcmp(runtime_handoff_receipt.status,
                      "ready-render-state") == 0,
           "Nexus startup footer pointer routes menu directly to first DGN render state");
    champion_execution.kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW;
    expect(nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
               &runtime_state,
               &champion_execution,
               NULL,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &runtime_handoff_receipt) &&
               runtime_handoff_receipt.route ==
                   NEXUS_V1_STARTUP_RUNTIME_HANDOFF_NOT_START,
           "Nexus startup handoff ignores non-start champion routes");
    champion_execution.kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON;
    synthetic_engine.menu_bpk_upload_receipt.route =
        NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3;
    synthetic_engine.menu_bpk_upload_receipt.blocked_prs3_uploads = 3;
    synthetic_engine.menu_bpk_upload_receipt.blocks_real_menu_surface_render = 1;
    synthetic_engine.menu_bpk_decode_receipt.route =
        NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3;
    synthetic_engine.menu_bpk_decode_receipt.blocked_prs3_surfaces = 3;
    synthetic_engine.menu_bpk_decode_receipt.prs3_stream_plans = 3;
    synthetic_engine.menu_bpk_decode_receipt.requires_prs3_decoder = 1;
    synthetic_engine.menu_bpk_decode_receipt.decode_blocked = 1;
    expect(nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
               &runtime_state,
               &synthetic_runtime_receipt.startup_assets),
           "Nexus synthetic blocked runtime assets rebuild for route proof");
    expect(nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
               &runtime_state,
               &champion_execution,
               NULL,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &runtime_handoff_receipt) &&
               runtime_handoff_receipt.route ==
                   NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED &&
               runtime_handoff_receipt.dgn_render_ready == 0 &&
               runtime_handoff_receipt.hud_ready == 0 &&
               runtime_handoff_receipt.dgn_render_blocked == 1 &&
               runtime_handoff_receipt.asset_handoff.route ==
                   NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED &&
               runtime_handoff_receipt.asset_handoff
                       .menu_bpk_prs3_blocks_real_menu_route == 1 &&
               runtime_handoff_receipt.asset_handoff
                       .menu_bpk_renderer_handoff.status ==
                   NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3 &&
               runtime_handoff_receipt.asset_handoff
                       .menu_bpk_renderer_handoff.fallback_visuals_permitted == 0 &&
               strcmp(runtime_handoff_receipt.status,
                      "blocked-menu-bpk-prs3") == 0,
           "Nexus startup handoff blocks DGN route when Saturn menu assets are blocked");
    memset(dgn_commands, 0, sizeof(dgn_commands));
    expect(nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input(
               &runtime_state,
               11,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &runtime_route_receipt) &&
               runtime_route_receipt.route ==
                   NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED &&
               runtime_route_receipt.host_action_valid == 1 &&
               runtime_route_receipt.consumed_by_nexus == 1 &&
               runtime_route_receipt.runtime_route_ready == 0 &&
               runtime_route_receipt.runtime_route_blocked == 1 &&
               runtime_route_receipt.startup_sfx_status ==
                   NEXUS_SFX_RUNTIME_READY_DECODED &&
               runtime_route_receipt.startup_sfx_level_index == 0 &&
               runtime_route_receipt.startup_cd_track == 2 &&
               runtime_route_receipt.startup_audio_handoff_ready == 1 &&
               runtime_route_receipt.startup_sfx_blocks_real_playback == 0 &&
               runtime_route_receipt.dgn_handoff_status ==
                   NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING &&
               runtime_route_receipt.dgn_render_plan_status ==
                   NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING &&
               runtime_route_receipt.dgn_render_plan_ready == 0 &&
               runtime_route_receipt.dgn_render_command_count == 0 &&
               runtime_route_receipt.first_dgn_render_command_kind == 0 &&
               runtime_route_receipt.dgn_blocks_real_mesh_render == 0 &&
               runtime_route_receipt.fallback_visuals_permitted == 0 &&
               runtime_route_receipt.runtime_handoff.asset_handoff
                       .menu_bpk_prs3_blocks_real_menu_route == 1 &&
               runtime_route_receipt.runtime_handoff.asset_handoff
                       .menu_bpk_renderer_handoff.status ==
                   NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3 &&
               strcmp(runtime_route_receipt.status,
                      "blocked-menu-bpk-prs3") == 0,
           "Nexus runtime route receipt blocks PRS3 without fallback");
    expect(nexus_v1_launcher_startup_route_proof_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               &champion_execution,
               NULL,
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &route_proof_receipt) &&
               route_proof_receipt.route ==
                   NEXUS_V1_STARTUP_ROUTE_PROOF_ASSET_BLOCKED &&
               route_proof_receipt.title_route_ready == 1 &&
               route_proof_receipt.menu_route_ready == 0 &&
               route_proof_receipt.startup_surfaces_real_ready == 1 &&
               route_proof_receipt.faces_real_ready == 1 &&
               route_proof_receipt.full_start_graphics_ready == 0 &&
               route_proof_receipt.save_load_menu_route_ready == 0 &&
               route_proof_receipt.startup_ui_route_ready == 0 &&
               strcmp(route_proof_receipt.startup_ui_blocker,
                      "menu-bpk-prs3") == 0 &&
               route_proof_receipt.runtime_route_ready == 0 &&
               route_proof_receipt.graphics_ready == 0 &&
               route_proof_receipt.audio_ready == 1 &&
               route_proof_receipt.asset_handoff.route ==
                   NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED &&
               route_proof_receipt.asset_handoff
                       .menu_bpk_prs3_blocks_real_menu_route == 1 &&
               route_proof_receipt.asset_handoff
                       .menu_bpk_renderer_handoff.status ==
                   NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3 &&
               route_proof_receipt.asset_handoff
                       .menu_bpk_renderer_handoff.blocked_prs3_surfaces == 3 &&
               route_proof_receipt.asset_handoff
                       .menu_bpk_renderer_handoff.fallback_visuals_permitted == 0 &&
               route_proof_receipt.runtime_route_receipt.route ==
                   NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED &&
               route_proof_receipt.runtime_route_receipt.consumed_by_nexus == 1 &&
               route_proof_receipt.runtime_route_receipt.runtime_route_blocked == 1 &&
               route_proof_receipt.runtime_route_receipt
                       .runtime_handoff.asset_handoff
                       .menu_bpk_prs3_blocks_real_menu_route == 1 &&
               route_proof_receipt.title_menu_route_ready == 0 &&
               route_proof_receipt.menu_runtime_route_ready == 0 &&
               route_proof_receipt.first_runtime_route_ready == 0 &&
               route_proof_receipt.audio_runtime_route_ready == 0 &&
               route_proof_receipt.full_startup_route_ready == 0 &&
               strcmp(route_proof_receipt.status,
                      "blocked-menu-bpk-prs3") == 0,
           "Nexus startup route proof exposes Saturn asset blocker before runtime");
    expect(nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               &full_start_receipt) &&
               full_start_receipt.route ==
                   NEXUS_V1_STARTUP_FULL_START_BLOCKED_ASSETS &&
               full_start_receipt.warning_art_loaded == 1 &&
               full_start_receipt.title_art_loaded == 1 &&
               full_start_receipt.boot_warning_title_ready == 1 &&
               full_start_receipt.startup_surfaces_real_ready == 1 &&
               full_start_receipt.faces_real_ready == 1 &&
               full_start_receipt.menu_bpk_route_ready == 0 &&
               full_start_receipt.save_menu_route_ready == 0 &&
               full_start_receipt.champion_menu_route_ready == 0 &&
               full_start_receipt.audio_track02_ready == 1 &&
               full_start_receipt.full_start_graphics_ready == 0 &&
               full_start_receipt.full_start_menu_ready == 0 &&
               full_start_receipt.m11_host_route_ready == 0 &&
               strcmp(full_start_receipt.m11_host_route,
                      "blocked-startup") == 0 &&
               full_start_receipt.host_receipt.input_result ==
                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
               strcmp(full_start_receipt.startup_ui_blocker,
                      "menu-bpk-prs3") == 0 &&
               strcmp(full_start_receipt.status,
                      "blocked-menu-bpk-prs3") == 0,
           "Nexus full-start receipt blocks save and champion menus on PRS3");
    expect(nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
               &synthetic_runtime_receipt,
               &runtime_state,
               11,
               NULL,
               NULL,
               &full_start_consumer_receipt) &&
               full_start_consumer_receipt.m11_ready == 0 &&
               full_start_consumer_receipt.m12_ready == 0 &&
               strcmp(full_start_consumer_receipt.consumer_route,
                      "blocked-startup") == 0 &&
               full_start_consumer_receipt.presentation_valid == 0 &&
               strcmp(full_start_consumer_receipt.status,
                      "blocked-menu-bpk-prs3") == 0,
           "Nexus full-start consumer receipt blocks M11/M12 on PRS3");
    runtime_snapshot.runtime = runtime_state;
    expect(nexus_v1_launcher_startup_full_start_consumer_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               11,
               NULL,
               NULL,
               &full_start_consumer_receipt) &&
               full_start_consumer_receipt.m11_ready == 0 &&
               strcmp(full_start_consumer_receipt.consumer_route,
                      "blocked-startup") == 0 &&
               full_start_consumer_receipt.presentation_valid == 0,
           "Nexus full-start snapshot consumer blocks M11/M12 on PRS3");
    expect(nexus_v1_launcher_startup_full_start_package_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               11,
               NULL,
               NULL,
               &full_start_package_receipt) &&
               full_start_package_receipt.m11_ready == 0 &&
               full_start_package_receipt.graphics_ready == 0 &&
               full_start_package_receipt.audio_ready == 1 &&
               strcmp(full_start_package_receipt.consumer_route,
                      "blocked-startup") == 0 &&
               full_start_package_receipt.capture_valid == 1 &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_BLOCKED &&
               full_start_package_receipt.capture_route_ready == 0 &&
               full_start_package_receipt.capture_command_count == 0 &&
               full_start_package_receipt.blocked_draw_suppressed == 1 &&
               full_start_package_receipt.first_capture_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_NONE &&
               strcmp(full_start_package_receipt.startup_ui_blocker,
                      "menu-bpk-prs3") == 0,
           "Nexus full-start package blocks startup capture proof on PRS3");
    memset(draw_commands, 0x7f, sizeof(draw_commands));
    expect(nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               11,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               &full_start_package_receipt) &&
               full_start_package_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_BLOCKED &&
               full_start_package_receipt.capture_command_count == 0 &&
               full_start_package_receipt.blocked_draw_suppressed == 1 &&
               full_start_package_receipt.fallback_visuals_permitted == 0 &&
               draw_commands[0].kind == NEXUS_V1_STARTUP_DRAW_NONE,
           "Nexus full-start package command helper blocks PRS3 fallback draw");
    expect(nexus_v1_launcher_startup_real_asset_ownership_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               11,
               NULL,
               NULL,
               &real_asset_ownership_receipt) &&
               real_asset_ownership_receipt.route ==
                   NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_BLOCKED_ASSETS &&
               real_asset_ownership_receipt.receipt_owner_is_nexus == 1 &&
               real_asset_ownership_receipt.capture_receipt_owned == 1 &&
               real_asset_ownership_receipt.real_asset_receipt_owned == 1 &&
               real_asset_ownership_receipt.consumes_bpk_menu_handoff == 1 &&
               real_asset_ownership_receipt.consumes_prs3_blocker == 1 &&
               real_asset_ownership_receipt.consumes_dgn_handoff == 0 &&
               real_asset_ownership_receipt.menu_bpk_handoff.status ==
                   NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3 &&
               real_asset_ownership_receipt.menu_bpk_handoff
                       .blocked_prs3_surfaces == 3 &&
               real_asset_ownership_receipt.runtime_dgn_handoff_ready == 0 &&
               real_asset_ownership_receipt.menu_capture_uses_real_assets == 0 &&
               real_asset_ownership_receipt.package_capture_consumed_by_host == 0 &&
               real_asset_ownership_receipt.title_menu_capture_route_joined == 0 &&
               real_asset_ownership_receipt.bpk_menu_route_joined == 0 &&
               real_asset_ownership_receipt.runtime_dgn_route_joined == 0 &&
               real_asset_ownership_receipt.no_fallback_visuals_enforced == 1 &&
               real_asset_ownership_receipt.fallback_visuals_permitted == 0 &&
               real_asset_ownership_receipt.blocked_draw_suppressed == 1 &&
               real_asset_ownership_receipt.blocked_route_suppresses_startup_draws == 1 &&
               real_asset_ownership_receipt.blocked_route_suppresses_dgn_draws == 1 &&
               real_asset_ownership_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_BLOCKED &&
               real_asset_ownership_receipt.first_startup_draw_kind ==
                   NEXUS_V1_STARTUP_DRAW_NONE &&
               real_asset_ownership_receipt.startup_draw_command_count == 0 &&
               strcmp(real_asset_ownership_receipt.asset_blocker,
                      "menu-bpk-prs3") == 0 &&
               strcmp(real_asset_ownership_receipt.status,
                      "blocked-menu-bpk-prs3") == 0,
           "Nexus real-asset ownership receipt blocks PRS3 startup without fallback visuals");
    memset(draw_commands, 0x7f, sizeof(draw_commands));
    memset(dgn_commands, 0x7f, sizeof(dgn_commands));
    expect(nexus_v1_launcher_startup_host_caller_receipt_from_snapshot(
               &synthetic_runtime_receipt,
               &runtime_snapshot,
               11,
               NULL,
               NULL,
               draw_commands,
               (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
               dgn_commands,
               NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
               &host_caller_receipt) &&
               host_caller_receipt.host_caller_ready == 1 &&
               host_caller_receipt.receipt_owner_is_nexus == 1 &&
               host_caller_receipt.host_startup_capture_ready == 0 &&
               host_caller_receipt.host_runtime_dgn_ready == 0 &&
               host_caller_receipt.host_execute_startup_draws == 0 &&
               host_caller_receipt.host_execute_dgn_draws == 0 &&
               host_caller_receipt.bpk_handoff_consumed == 1 &&
               host_caller_receipt.prs3_blocker_consumed == 1 &&
               host_caller_receipt.dgn_handoff_consumed == 0 &&
               host_caller_receipt.single_saturn_startup_owner_ready == 0 &&
               host_caller_receipt.title_menu_capture_route_joined == 0 &&
               host_caller_receipt.runtime_dgn_route_joined == 0 &&
               host_caller_receipt.blocked_route_suppresses_all_draws == 1 &&
               host_caller_receipt.suppress_fallback_visuals == 1 &&
               host_caller_receipt.suppress_legacy_placeholder_visuals == 1 &&
               host_caller_receipt.no_fallback_visuals_enforced == 1 &&
               host_caller_receipt.capture_route ==
                   NEXUS_V1_STARTUP_CAPTURE_BLOCKED &&
               host_caller_receipt.ownership_route ==
                   NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_BLOCKED_ASSETS &&
               host_caller_receipt.startup_command_count == 0 &&
               host_caller_receipt.copied_startup_command_count == 0 &&
               host_caller_receipt.dgn_command_count == 0 &&
               host_caller_receipt.copied_dgn_command_count == 0 &&
               draw_commands[0].kind == NEXUS_V1_STARTUP_DRAW_NONE &&
               dgn_commands[0].kind == 0 &&
               strcmp(host_caller_receipt.host_route,
                      "blocked-startup") == 0 &&
               strcmp(host_caller_receipt.status,
                      "blocked-menu-bpk-prs3") == 0,
           "Nexus host-caller receipt suppresses PRS3 fallback startup and DGN draws");
    expect(nexus_v1_startup_champion_execution_mode_update(
               &champion_execution,
               2,
               &mode_update) &&
               mode_update.set_champion_select_active &&
               mode_update.champion_select_active == 0 &&
               mode_update.set_champion_frame &&
               mode_update.champion_frame == 0,
           "Nexus champion execution owns start-dungeon state update");
    action.kind = NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR;
    action.row = 5;
    expect(nexus_v1_startup_execute_champion_action(
               &action,
               &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR &&
               champion_execution.cursor == 5 &&
               strcmp(champion_execution.status,
                      "NEXUS CHAMPION CURSOR") == 0,
           "Nexus champion execution resolves cursor movement");
    action.kind = NEXUS_V1_STARTUP_ACTION_CHAMPION_REMOVED;
    action.row = 2;
    expect(nexus_v1_startup_execute_champion_action(
               &action,
               &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR &&
               champion_execution.cursor == 2 &&
               strcmp(champion_execution.status,
                      "NEXUS CHAMPION REMOVED") == 0,
           "Nexus champion execution resolves removal cursor reset");
    action.kind = NEXUS_V1_STARTUP_ACTION_NEED_CHAMPION;
    action.row = 0;
    expect(nexus_v1_startup_execute_champion_action(
               &action,
               &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW &&
               strcmp(champion_execution.status,
                      "NEXUS NEEDS CHAMPION") == 0,
           "Nexus champion execution resolves empty-party warning");
    action.kind = NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT;
    expect(nexus_v1_startup_execute_champion_action(
               &action,
               &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_SHOW_SAVE_SELECT &&
               champion_execution.select_last_save_row == 1 &&
               strcmp(champion_execution.status, "NEXUS LOAD GAME") == 0,
           "Nexus champion execution resolves back to save select");
    expect(nexus_v1_startup_champion_execution_mode_update(
               &champion_execution,
               2,
               &mode_update) &&
               mode_update.set_champion_select_active &&
               mode_update.champion_select_active == 0 &&
               mode_update.set_save_select_active &&
               mode_update.save_select_active == 1 &&
               mode_update.set_save_selected_row &&
               mode_update.save_selected_row == 1,
           "Nexus champion execution owns save-select state update");
    action.kind = NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE;
    expect(nexus_v1_startup_execute_champion_action(
               &action,
               &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_SHOW_TITLE &&
               strcmp(champion_execution.status, "NEXUS TITLE") == 0,
           "Nexus champion execution resolves back to title");
    expect(nexus_v1_startup_champion_execution_mode_update(
               &champion_execution,
               0,
               &mode_update) &&
               mode_update.set_champion_select_active &&
               mode_update.champion_select_active == 0 &&
               mode_update.set_title_active &&
               mode_update.title_active == 1 &&
               mode_update.set_title_frame &&
               mode_update.title_frame == 0,
           "Nexus champion execution owns title state update");
    action.kind = NEXUS_V1_STARTUP_ACTION_NONE;
    expect(nexus_v1_startup_execute_champion_action(
               &action,
               &champion_execution) &&
               champion_execution.kind ==
                   NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW &&
               strcmp(champion_execution.status, "NEXUS CHAMPIONS") == 0,
           "Nexus champion execution resolves consumed panel redraw");
    expect(!nexus_v1_startup_execute_champion_action(
               NULL,
               &champion_execution),
           "Nexus champion execution rejects NULL action");
    expect(!nexus_v1_startup_execute_champion_action(&action, NULL),
           "Nexus champion execution rejects NULL output");

    build_world(&world);
    nexus_v1_save_init(&mgr, save_dir);
    expect(nexus_v1_save_full(&mgr, 3,
                              world.party_level,
                              world.party_x,
                              world.party_y,
                              world.party_dir,
                              (uint32_t)world.world_tick,
                              world.state_hash,
                              &champions,
                              &world) == NEXUS_SAVE_OK,
           "wrote Nexus FNXS slot 03 fixture");

    nexus_v1_startup_menu_init(&menu, save_dir);
    expect(nexus_v1_startup_menu_scan(&menu) == 0,
           "startup menu scans save directory");
    expect(menu.slot_mask == (1u << 3),
           "startup menu slot mask exposes occupied slot 03");
    expect(menu.row_count == 2,
           "startup menu has one slot row plus NEW GAME");
    {
        Nexus_V1_StartupMenu render_menu;
        nexus_v1_startup_menu_init(&render_menu, save_dir);
        render_menu.selected_row = 1;
        expect(nexus_v1_startup_menu_refresh(&render_menu, (1u << 3)),
               "startup menu render fixture refreshes occupied slot rows");
        memset(save_rows, 0, sizeof(save_rows));
        memset(&chrome, 0, sizeof(chrome));
        expect(nexus_v1_startup_menu_build_save_chrome_render(&chrome) &&
                   strcmp(chrome.title, "DUNGEON MASTER NEXUS") == 0 &&
                   strcmp(chrome.subtitle, "LOAD GAME") == 0 &&
                   strcmp(chrome.footer, "ACCEPT LOADS  ACTION STARTS") == 0 &&
                   chrome.title_x == NEXUS_V1_STARTUP_TITLE_X &&
                   chrome.subtitle_y == NEXUS_V1_STARTUP_SUBTITLE_Y &&
                   chrome.footer_x == NEXUS_V1_STARTUP_FOOTER_X,
               "Nexus save-select chrome render metadata is Nexus-owned");
        expect(nexus_v1_startup_menu_build_save_render_rows(
                   &render_menu,
                   save_rows,
                   (int)(sizeof(save_rows) / sizeof(save_rows[0]))) == 2 &&
                   save_rows[0].kind == NEXUS_V1_STARTUP_ROW_SLOT &&
                   save_rows[0].slot == 3 &&
                   save_rows[0].selected == 0 &&
                   strstr(save_rows[0].label, "LOAD SLOT 03") != NULL &&
                   save_rows[1].kind == NEXUS_V1_STARTUP_ROW_NEW_GAME &&
                   save_rows[1].selected == 1 &&
                   strstr(save_rows[1].label, "NEW GAME") != NULL,
               "Nexus save-select render rows carry slot and New Game labels");
        memset(&chrome, 0, sizeof(chrome));
        expect(nexus_v1_startup_menu_build_champion_chrome_render(&chrome) &&
                   strcmp(chrome.title, "DUNGEON MASTER NEXUS") == 0 &&
                   strcmp(chrome.subtitle, "SELECT CHAMPIONS") == 0 &&
                   chrome.footer[0] == '\0' &&
                   chrome.title_x == NEXUS_V1_STARTUP_TITLE_X &&
                   chrome.subtitle_y == NEXUS_V1_STARTUP_SUBTITLE_Y,
               "Nexus champion-select chrome render metadata is Nexus-owned");
        memset(&menu_snapshot, 0, sizeof(menu_snapshot));
        snprintf(menu_snapshot.save_dir,
                 sizeof(menu_snapshot.save_dir),
                 "%s",
                 save_dir);
        draw_count = nexus_v1_startup_presentation_build_title(
            17,
            draw_commands,
            (int)(sizeof(draw_commands) / sizeof(draw_commands[0])));
        expect(draw_count == 1 &&
                   draw_commands[0].kind ==
                       NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND,
               "Nexus startup presentation owns WARNING.BIN boot draw frame");
        draw_count = nexus_v1_startup_presentation_build_title(
            60,
            draw_commands,
            (int)(sizeof(draw_commands) / sizeof(draw_commands[0])));
        expect(draw_count == 1 &&
                   draw_commands[0].kind ==
                       NEXUS_V1_STARTUP_DRAW_BOOT_TITLE_FRAME &&
                   draw_commands[0].title_frame ==
                       60 - nexus_v1_boot_warning_frames(),
               "Nexus startup presentation maps boot frame to TITLE.CG frame");
        menu_snapshot.slot_mask = (1u << 3);
        menu_snapshot.row_count = 2;
        menu_snapshot.selected_row = 1;
        draw_count = nexus_v1_startup_presentation_build_save(
            &menu_snapshot,
            draw_commands,
            (int)(sizeof(draw_commands) / sizeof(draw_commands[0])));
        expect(draw_count >= 6 &&
                   draw_commands[0].kind ==
                       NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND &&
                   draw_commands[1].kind == NEXUS_V1_STARTUP_DRAW_TEXT &&
                   draw_commands[1].text_style ==
                       NEXUS_V1_STARTUP_TEXT_TITLE &&
                   strcmp(draw_commands[1].label,
                          "DUNGEON MASTER NEXUS") == 0,
               "Nexus save startup presentation starts from title art and owns chrome commands");
        memset(&champion_snapshot, 0, sizeof(champion_snapshot));
        champion_snapshot.cursor = 0;
        champion_snapshot.frame = 0;
        champion_snapshot.slot_mask = 0u;
        draw_count = nexus_v1_startup_presentation_build_champion(
            &champions,
            &champion_snapshot,
            draw_commands,
            (int)(sizeof(draw_commands) / sizeof(draw_commands[0])));
        portrait_draws = 0;
        for (slot = 0; slot < draw_count; ++slot) {
            if (draw_commands[slot].kind ==
                NEXUS_V1_STARTUP_DRAW_PORTRAIT) {
                ++portrait_draws;
            }
        }
        expect(draw_count > 24 &&
                   draw_commands[0].kind ==
                       NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND &&
                   portrait_draws == 12,
               "Nexus champion startup presentation combines title art and FACE portrait commands");
    }
    menu.selected_row = 99;
    expect(nexus_v1_startup_menu_refresh(&menu, menu.slot_mask) &&
               menu.row_count == 2 &&
               menu.selected_row == 1,
           "startup menu refresh clamps stale selected row to last visible row");
    expect(nexus_v1_startup_menu_refresh(&menu, 0u) &&
               menu.slot_mask == 0u &&
               menu.row_count == 1 &&
               menu.selected_row == 0,
           "startup menu refresh handles empty save list as NEW GAME only");
    expect(nexus_v1_startup_menu_refresh(&menu, (1u << 3)) &&
               menu.slot_mask == (1u << 3) &&
               menu.row_count == 2,
           "startup menu refresh restores occupied slot rows");
    nexus_v1_startup_menu_init(&menu, save_dir);
    menu.selected_row = 99;
    expect(nexus_v1_startup_menu_scan_or_new_game(&menu) &&
               menu.slot_mask == (1u << 3) &&
               menu.row_count == 2 &&
               menu.selected_row == 1,
           "startup menu scan-or-new-game publishes scanned slot rows");
    expect(nexus_v1_startup_menu_scan_or_new_game(NULL) == 0,
           "startup menu scan-or-new-game rejects NULL menu");

    memset(&action, 0, sizeof(action));
    expect(!nexus_v1_startup_title_handle_input(
               54,
               menu.slot_mask,
               NEXUS_V1_STARTUP_INPUT_NONE,
               &action),
           "startup title ignores idle input");
    expect(action.kind == NEXUS_V1_STARTUP_ACTION_NONE,
           "startup title idle action remains NONE");
    expect(nexus_v1_startup_title_handle_input(
               53,
               menu.slot_mask,
               NEXUS_V1_STARTUP_INPUT_ACCEPT,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_HOLD_TITLE,
           "startup title holds Accept before start-ready frame");
    expect(nexus_v1_startup_execute_title_action(&action,
                                                 &title_execution) &&
               title_execution.kind ==
                   NEXUS_V1_STARTUP_TITLE_EXEC_HOLD_TITLE &&
               strcmp(title_execution.status_scope, "STARTUP") == 0 &&
               strcmp(title_execution.status, "NEXUS TITLE") == 0,
           "startup title execution resolves hold-title redraw");
    expect(nexus_v1_startup_title_execution_mode_update(
               &title_execution,
               &mode_update) &&
               !mode_update.set_title_active &&
               !mode_update.set_save_select_active &&
               !mode_update.set_champion_select_active,
           "startup title hold owns no-op mode update");
    expect(nexus_v1_title_frame(30, 200, &title_frame) &&
               title_frame.phase == NEXUS_V1_TITLE_PHASE_HOLD &&
               title_frame.boot_reveal_complete &&
               !title_frame.start_ready,
           "startup title frame 30 is full reveal but still in hold phase");
    expect(nexus_v1_startup_title_handle_input(
               30,
               menu.slot_mask,
               NEXUS_V1_STARTUP_INPUT_ACTION,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_HOLD_TITLE,
           "startup title holds Action during post-reveal hold phase");
    expect(nexus_v1_startup_title_handle_input(
               54,
               menu.slot_mask,
               NEXUS_V1_STARTUP_INPUT_ACCEPT,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT,
           "startup title routes ready Accept to save select when slots exist");
    expect(nexus_v1_startup_execute_title_action(&action,
                                                 &title_execution) &&
               title_execution.kind ==
                   NEXUS_V1_STARTUP_TITLE_EXEC_SHOW_SAVE_SELECT &&
               strcmp(title_execution.status_scope, "STARTUP") == 0 &&
               strcmp(title_execution.status, "NEXUS LOAD GAME") == 0,
           "startup title execution resolves save-select handoff");
    expect(nexus_v1_startup_title_execution_mode_update(
               &title_execution,
               &mode_update) &&
               mode_update.set_title_active &&
               mode_update.title_active == 0 &&
               mode_update.set_title_frame &&
               mode_update.title_frame == 0 &&
               mode_update.set_save_select_active &&
               mode_update.save_select_active == 1 &&
               mode_update.set_save_selected_row &&
               mode_update.save_selected_row == 0,
           "startup title execution owns save-select mode update");
    expect(nexus_v1_startup_apply_receipt_from_title_execution(
               &title_execution,
               &receipt) &&
               receipt.result == NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW &&
               receipt.mode_update.set_save_select_active &&
               receipt.mode_update.save_select_active == 1 &&
               strcmp(receipt.status, "NEXUS LOAD GAME") == 0,
           "startup title apply receipt owns save-select redraw policy");
    expect(nexus_v1_startup_host_receipt_from_apply_receipt(
               &receipt,
               &host_receipt) &&
               host_receipt.input_result ==
                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
               host_receipt.mode_update.set_save_select_active &&
               host_receipt.mode_update.save_select_active == 1 &&
               strcmp(host_receipt.status, "NEXUS LOAD GAME") == 0,
           "startup title host receipt owns M11 redraw contract");
    expect(nexus_v1_startup_execute_title_action_with_receipt(
               &action,
               &title_execution,
               &receipt) &&
               title_execution.kind ==
                   NEXUS_V1_STARTUP_TITLE_EXEC_SHOW_SAVE_SELECT &&
               receipt.result == NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW &&
               receipt.mode_update.set_save_select_active &&
               receipt.mode_update.save_select_active == 1 &&
               strcmp(receipt.status, "NEXUS LOAD GAME") == 0,
           "startup title action helper owns execution and receipt");
    expect(nexus_v1_startup_execute_title_action_with_host_receipt(
               &action,
               &title_execution,
               &host_receipt) &&
               title_execution.kind ==
                   NEXUS_V1_STARTUP_TITLE_EXEC_SHOW_SAVE_SELECT &&
               host_receipt.input_result ==
                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
               host_receipt.mode_update.set_save_select_active &&
               host_receipt.mode_update.save_select_active == 1 &&
               strcmp(host_receipt.status, "NEXUS LOAD GAME") == 0,
           "startup title action helper can return M11-ready host receipt directly");
    {
        Nexus_V1_StartupHostFacts title_facts;
        Nexus_V1_StartupHostActionReceipt title_action_receipt;

        memset(&title_facts, 0, sizeof(title_facts));
        title_facts.title_active = 1;
        title_facts.title_frame = nexus_v1_boot_start_ready_frames();
        title_facts.slot_mask = menu.slot_mask;
        expect(nexus_v1_startup_execute_title_firestaff_input_from_host_facts_with_receipt(
                   &title_facts,
                   9,
                   &title_execution,
                   &title_action_receipt) &&
                   !title_action_receipt.save_state_receipt_valid &&
                   !title_action_receipt.champion_state_receipt_valid &&
                   title_action_receipt.host_receipt.input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                   title_action_receipt.host_receipt.mode_update
                       .set_save_select_active &&
                   strcmp(title_action_receipt.host_receipt.status,
                          "NEXUS LOAD GAME") == 0,
               "startup title keyboard wrapper returns one M11-ready action receipt");
        title_facts.title_frame = 30;
        expect(nexus_v1_startup_execute_title_pointer_from_host_facts_with_receipt(
                   &title_facts,
                   &title_execution,
                   &title_action_receipt) &&
                   title_execution.kind ==
                       NEXUS_V1_STARTUP_TITLE_EXEC_HOLD_TITLE &&
                   title_action_receipt.host_receipt.input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                   !title_action_receipt.host_receipt.mode_update
                        .set_save_select_active &&
                   strcmp(title_action_receipt.host_receipt.status,
                          "NEXUS TITLE") == 0,
               "startup title pointer wrapper preserves boot hold gate");
        title_facts.title_active = 0;
        expect(!nexus_v1_startup_execute_title_firestaff_input_from_host_facts_with_receipt(
                   &title_facts,
                   9,
                   &title_execution,
                   &title_action_receipt),
               "startup title keyboard wrapper rejects inactive host facts");

        title_facts.title_active = 1;
        title_facts.title_frame = nexus_v1_boot_start_ready_frames();
        expect(nexus_v1_startup_title_route_receipt_from_host_facts_input(
                   &title_facts,
                   9,
                   &title_route_receipt) &&
                   title_route_receipt.route ==
                       NEXUS_V1_STARTUP_TITLE_ROUTE_SAVE_SELECT &&
                   strcmp(nexus_v1_startup_title_route_name(
                              title_route_receipt.route),
                          "save-select") == 0 &&
                   title_route_receipt.handled &&
                   title_route_receipt.draw_command_count > 0 &&
                   title_route_receipt.host_input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                   title_route_receipt.set_save_select_active &&
                   title_route_receipt.save_select_active == 1 &&
                   title_route_receipt.set_save_selected_row &&
                   title_route_receipt.save_selected_row == 0 &&
                   strcmp(title_route_receipt.status, "NEXUS LOAD GAME") == 0,
               "startup title route receipt hands ready Accept to save select");

        title_facts.title_frame = 30;
        expect(nexus_v1_startup_title_route_receipt_from_host_facts_pointer(
                   &title_facts,
                   &title_route_receipt) &&
                   title_route_receipt.route ==
                       NEXUS_V1_STARTUP_TITLE_ROUTE_HOLD &&
                   title_route_receipt.handled &&
                   title_route_receipt.draw_command_count > 0 &&
                   title_route_receipt.host_input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                   !title_route_receipt.set_save_select_active &&
                   strcmp(title_route_receipt.status, "NEXUS TITLE") == 0,
               "startup title route receipt preserves pointer hold gate");

        title_facts.title_frame = nexus_v1_boot_start_ready_frames();
        title_facts.slot_mask = 0u;
        expect(nexus_v1_startup_title_route_receipt_from_host_facts_input(
                   &title_facts,
                   11,
                   &title_route_receipt) &&
                   title_route_receipt.route ==
                       NEXUS_V1_STARTUP_TITLE_ROUTE_CHAMPION_SELECT &&
                   title_route_receipt.set_champion_select_active &&
                   title_route_receipt.champion_select_active == 1 &&
                   title_route_receipt.set_champion_cursor &&
                   title_route_receipt.champion_cursor == 0 &&
                   strcmp(title_route_receipt.status, "NEXUS CHAMPIONS") == 0,
               "startup title route receipt hands no-save Action to champions");

        title_facts.slot_mask = menu.slot_mask;
        expect(nexus_v1_startup_title_route_receipt_from_host_facts_input(
                   &title_facts,
                   10,
                   &title_route_receipt) &&
                   title_route_receipt.route ==
                       NEXUS_V1_STARTUP_TITLE_ROUTE_RETURN_TO_LAUNCHER &&
                   title_route_receipt.host_input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_RETURN_TO_LAUNCHER &&
                   strcmp(title_route_receipt.status_scope, "RETURN") == 0 &&
                   strcmp(title_route_receipt.status, "BACK TO LAUNCHER") == 0,
               "startup title route receipt exposes launcher return");

        title_facts.title_active = 0;
        expect(!nexus_v1_startup_title_route_receipt_from_host_facts_input(
                   &title_facts,
                   9,
                   &title_route_receipt) &&
                   title_route_receipt.route ==
                       NEXUS_V1_STARTUP_TITLE_ROUTE_INVALID &&
                   !title_route_receipt.handled,
               "startup title route receipt rejects inactive host facts");
    }
    expect(nexus_v1_startup_title_handle_hit(
               54,
               menu.slot_mask,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT,
           "startup title pointer hit routes through Nexus-owned title action");
    expect(nexus_v1_startup_title_handle_hit(
               30,
               menu.slot_mask,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_HOLD_TITLE,
           "startup title pointer hit respects hold gate");
    expect(!nexus_v1_startup_title_handle_hit(
               54,
               menu.slot_mask,
               NULL),
           "startup title pointer rejects NULL output");
    expect(nexus_v1_startup_title_handle_input(
               54,
               0u,
               NEXUS_V1_STARTUP_INPUT_ACTION,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_SHOW_CHAMPION_SELECT,
           "startup title routes ready Action to champion select without slots");
    expect(nexus_v1_startup_execute_title_action(&action,
                                                 &title_execution) &&
               title_execution.kind ==
                   NEXUS_V1_STARTUP_TITLE_EXEC_SHOW_CHAMPIONS &&
               strcmp(title_execution.status_scope, "STARTUP") == 0 &&
               strcmp(title_execution.status, "NEXUS CHAMPIONS") == 0,
           "startup title execution resolves champion-select handoff");
    expect(nexus_v1_startup_title_execution_mode_update(
               &title_execution,
               &mode_update) &&
               mode_update.set_title_active &&
               mode_update.title_active == 0 &&
               mode_update.set_champion_select_active &&
               mode_update.champion_select_active == 1 &&
               mode_update.set_champion_cursor &&
               mode_update.champion_cursor == 0 &&
               mode_update.set_champion_frame &&
               mode_update.champion_frame == 0,
           "startup title execution owns champion-select mode update");
    expect(nexus_v1_startup_title_handle_input(
               12,
               menu.slot_mask,
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER,
           "startup title Back returns to launcher");
    expect(nexus_v1_startup_execute_title_action(&action,
                                                 &title_execution) &&
               title_execution.kind ==
                   NEXUS_V1_STARTUP_TITLE_EXEC_RETURN_TO_LAUNCHER &&
               strcmp(title_execution.status_scope, "RETURN") == 0 &&
               strcmp(title_execution.status, "BACK TO LAUNCHER") == 0,
           "startup title execution resolves launcher return");
    expect(nexus_v1_startup_title_execution_mode_update(
               &title_execution,
               &mode_update) &&
               !mode_update.set_title_active &&
               !mode_update.set_save_select_active,
           "startup title launcher return owns no-op mode update");
    expect(nexus_v1_startup_apply_receipt_from_title_execution(
               &title_execution,
               &receipt) &&
               receipt.result ==
                   NEXUS_V1_STARTUP_APPLY_RESULT_RETURN_TO_LAUNCHER &&
               strcmp(receipt.status_scope, "RETURN") == 0,
           "startup title apply receipt owns launcher return policy");
    expect(nexus_v1_boot_frame(0, 200, &boot_frame) &&
               boot_frame.phase == NEXUS_V1_BOOT_PHASE_WARNING &&
               boot_frame.warning_visible &&
               !boot_frame.start_ready,
           "startup full boot frame 0 is the warning phase");
    expect(nexus_v1_startup_boot_handle_input(
               nexus_v1_boot_warning_frames() - 1,
               menu.slot_mask,
               NEXUS_V1_STARTUP_INPUT_ACCEPT,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_HOLD_TITLE,
           "startup full boot holds Accept during warning phase");
    expect(nexus_v1_startup_boot_handle_input(
               nexus_v1_boot_warning_frames() + 53,
               menu.slot_mask,
               NEXUS_V1_STARTUP_INPUT_ACCEPT,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_HOLD_TITLE,
           "startup full boot holds Accept during title hold");
    expect(nexus_v1_startup_boot_handle_input(
               nexus_v1_boot_start_ready_frames(),
               menu.slot_mask,
               NEXUS_V1_STARTUP_INPUT_ACCEPT,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT,
           "startup full boot routes ready Accept after warning and title");
    expect(nexus_v1_startup_boot_handle_input(
               4,
               menu.slot_mask,
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER,
           "startup full boot Back returns to launcher during warning");

    kind = NEXUS_V1_STARTUP_ROW_NONE;
    slot = -1;
    expect(nexus_v1_startup_menu_row_at(&menu, 0, &kind, &slot),
           "startup menu row 0 exists");
    expect(kind == NEXUS_V1_STARTUP_ROW_SLOT && slot == 3,
           "startup menu row 0 is LOAD SLOT 03");
    menu.selected_row = 0;
    expect(nexus_v1_startup_menu_selected_path(&menu,
                                               path,
                                               sizeof(path)),
           "startup menu selected slot builds load path");
    expect(strstr(path, "nexus_save_03.dat") != NULL,
           "startup menu selected path points at nexus_save_03.dat");
    memset(&action, 0, sizeof(action));
    expect(nexus_v1_startup_menu_handle_input(
               &menu,
               NEXUS_V1_STARTUP_INPUT_ACCEPT,
               &action),
           "startup menu selected slot input activates");
    expect(action.kind == NEXUS_V1_STARTUP_ACTION_LOAD_SLOT &&
               action.row == 0 &&
               action.slot == 3 &&
               strstr(action.path, "nexus_save_03.dat") != NULL,
           "startup menu activation reports load-slot action");
    expect(nexus_v1_startup_execute_save_action(&action, &execution) &&
               execution.kind == NEXUS_V1_STARTUP_SAVE_EXEC_LOAD_SLOT &&
               strcmp(execution.status_scope, "BOOT") == 0 &&
               strcmp(execution.status, "NEXUS RESUMED") == 0 &&
               strcmp(execution.failure_status, "NEXUS LOAD FAILED") == 0 &&
               strstr(execution.path, "nexus_save_03.dat") != NULL,
           "startup save execution resolves load-slot handoff");
    expect(nexus_v1_startup_save_execution_mode_update(
               &execution,
               &mode_update) &&
               mode_update.set_save_select_active &&
               mode_update.save_select_active == 0 &&
               !mode_update.set_champion_select_active,
           "startup save execution owns load-slot mode update");
    expect(nexus_v1_startup_apply_receipt_from_save_execution(
               &execution,
               1,
               &receipt) &&
               receipt.result == NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW &&
               receipt.mode_update.set_save_select_active &&
               receipt.mode_update.save_select_active == 0 &&
               strcmp(receipt.status_scope, "BOOT") == 0 &&
               strcmp(receipt.status, "NEXUS RESUMED") == 0,
           "startup save apply receipt owns successful load policy");
    expect(nexus_v1_startup_host_receipt_from_apply_receipt(
               &receipt,
               &host_receipt) &&
               host_receipt.input_result ==
                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
               host_receipt.mode_update.set_save_select_active &&
               host_receipt.mode_update.save_select_active == 0 &&
               strcmp(host_receipt.status_scope, "BOOT") == 0 &&
               strcmp(host_receipt.status, "NEXUS RESUMED") == 0,
           "startup save host receipt owns M11 resumed contract");
    load_calls = 0;
    expect(nexus_v1_startup_execute_save_action_with_receipt(
               &action,
               startup_load_success,
               &load_calls,
               &execution,
               &receipt) &&
               load_calls == 1 &&
               receipt.result == NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW &&
               receipt.mode_update.set_save_select_active &&
               receipt.mode_update.save_select_active == 0 &&
               strcmp(receipt.status_scope, "BOOT") == 0 &&
               strcmp(receipt.status, "NEXUS RESUMED") == 0,
           "startup save action helper owns execution, load callback, and receipt");
    load_calls = 0;
    expect(nexus_v1_startup_execute_save_action_with_host_receipt(
               &action,
               startup_load_success,
               &load_calls,
               &execution,
               &host_receipt) &&
               load_calls == 1 &&
               host_receipt.input_result ==
                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
               host_receipt.mode_update.set_save_select_active &&
               host_receipt.mode_update.save_select_active == 0 &&
               strcmp(host_receipt.status_scope, "BOOT") == 0 &&
               strcmp(host_receipt.status, "NEXUS RESUMED") == 0,
           "startup save action helper can return M11-ready host receipt directly");
    expect(nexus_v1_startup_apply_receipt_from_save_execution(
               &execution,
               0,
               &receipt) &&
               receipt.result == NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW &&
               !receipt.mode_update.set_save_select_active &&
               strcmp(receipt.status_scope, "STARTUP") == 0 &&
               strcmp(receipt.status, "NEXUS LOAD FAILED") == 0,
           "startup save apply receipt owns failed load policy");
    expect(nexus_v1_startup_menu_handle_input(
               &menu,
               NEXUS_V1_STARTUP_INPUT_DOWN,
               &action) &&
               menu.selected_row == 1,
           "startup menu Down advances to NEW GAME");
    expect(action.kind == NEXUS_V1_STARTUP_ACTION_NONE,
           "startup menu Down is a navigation-only action");
    expect(nexus_v1_startup_execute_save_action(&action, &execution) &&
               execution.kind == NEXUS_V1_STARTUP_SAVE_EXEC_STATUS_REDRAW &&
               strcmp(execution.status, "NEXUS SAVE SELECT") == 0,
           "startup save execution resolves navigation redraw");
    expect(nexus_v1_startup_save_execution_mode_update(
               &execution,
               &mode_update) &&
               !mode_update.set_save_select_active &&
               !mode_update.set_champion_select_active,
           "startup save navigation owns no-op mode update");
    expect(nexus_v1_startup_menu_handle_input(
               &menu,
               NEXUS_V1_STARTUP_INPUT_DOWN,
               &action) &&
               menu.selected_row == 1,
           "startup menu Down clamps at last row");
    {
        Nexus_V1_StartupMenuSnapshot snapshot;
        Nexus_V1_StartupMenuStateReceipt state_receipt;
        Nexus_V1_StartupSaveRenderRow snapshot_rows[4];
        Nexus_V1_StartupRowKind snapshot_kind;
        int snapshot_slot;
        int snapshot_row_count;

        memset(&snapshot, 0, sizeof(snapshot));
        snprintf(snapshot.save_dir, sizeof(snapshot.save_dir), "%s", save_dir);
        snapshot.slot_mask = menu.slot_mask;
        snapshot.row_count = menu.row_count;
        snapshot.selected_row = 0;
        expect(nexus_v1_startup_menu_snapshot_row_at(
                   &snapshot,
                   0,
                   &snapshot_kind,
                   &snapshot_slot) &&
                   snapshot_kind == NEXUS_V1_STARTUP_ROW_SLOT &&
                   snapshot_slot == 3,
               "startup snapshot row lookup resolves slot without M11 row adapter");
        expect(nexus_v1_startup_menu_snapshot_from_facts(
                   &snapshot,
                   save_dir,
                   menu.slot_mask,
                   99) &&
                   strcmp(snapshot.save_dir, save_dir) == 0 &&
                   snapshot.slot_mask == menu.slot_mask &&
                   snapshot.row_count == 2 &&
                   snapshot.selected_row == 1,
               "startup snapshot facts helper owns save row clamp");
        expect(nexus_v1_startup_menu_snapshot_scan_or_new_game_from_facts(
                   &snapshot,
                   save_dir,
                   99) &&
                   strcmp(snapshot.save_dir, save_dir) == 0 &&
                   snapshot.slot_mask == menu.slot_mask &&
                   snapshot.row_count == 2 &&
                   snapshot.selected_row == 1,
               "startup snapshot scan helper owns save scan from facts");
        snapshot.selected_row = 0;
        expect(nexus_v1_startup_menu_snapshot_handle_input(
                   &snapshot,
                   NEXUS_V1_STARTUP_INPUT_DOWN,
                   &action) &&
                   snapshot.selected_row == 1 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_NONE,
               "startup snapshot input owns save navigation");
        expect(nexus_v1_startup_menu_handle_firestaff_input_from_facts(
                   &snapshot,
                   save_dir,
                   menu.slot_mask,
                   0,
                   2,
                   &action) &&
                   strcmp(snapshot.save_dir, save_dir) == 0 &&
                   snapshot.row_count == 2 &&
                   snapshot.selected_row == 1 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_NONE,
               "startup snapshot facts input owns M11 save input construction");
        expect(nexus_v1_startup_menu_state_receipt_from_snapshot(
                   &snapshot,
                   &state_receipt) &&
                   strcmp(state_receipt.save_dir, save_dir) == 0 &&
                   state_receipt.slot_mask == snapshot.slot_mask &&
                   state_receipt.row_count == snapshot.row_count &&
                   state_receipt.selected_row == snapshot.selected_row,
               "Nexus save state receipt mirrors sanitized snapshot state");
        expect(nexus_v1_startup_menu_state_receipt_from_facts(
                   &state_receipt,
                   save_dir,
                   menu.slot_mask,
                   99) &&
                   strcmp(state_receipt.save_dir, save_dir) == 0 &&
                   state_receipt.slot_mask == menu.slot_mask &&
                   state_receipt.row_count == 2 &&
                   state_receipt.selected_row == 1,
               "Nexus save state receipt facts helper owns M11 row clamp");
        expect(nexus_v1_startup_menu_state_receipt_scan_or_new_game_from_facts(
                   &state_receipt,
                   save_dir,
                   99) &&
                   strcmp(state_receipt.save_dir, save_dir) == 0 &&
                   state_receipt.slot_mask == menu.slot_mask &&
                   state_receipt.row_count == 2 &&
                   state_receipt.selected_row == 1,
               "Nexus save state receipt scan helper owns M11 save scan");
        memset(&host_facts, 0, sizeof(host_facts));
        host_facts.save_dir = save_dir;
        host_facts.save_selected_row = 99;
        expect(nexus_v1_startup_menu_state_receipt_scan_or_new_game_from_host_facts(
                   &state_receipt,
                   &host_facts) &&
                   strcmp(state_receipt.save_dir, save_dir) == 0 &&
                   state_receipt.slot_mask == menu.slot_mask &&
                   state_receipt.row_count == 2 &&
                   state_receipt.selected_row == 1,
               "Nexus save state receipt host facts scan helper owns M11 save scan");
        {
            Nexus_V1_StartupLaunchReceipt launch_receipt;
            Nexus_V1_StartupHostReceipt boot_receipt;
            expect(nexus_v1_startup_launch_from_host_facts_with_receipt(
                       &host_facts,
                       &launch_receipt) &&
                       launch_receipt.save_state_receipt_valid &&
                       strcmp(launch_receipt.save_state_receipt.save_dir,
                              save_dir) == 0 &&
                       launch_receipt.save_state_receipt.row_count == 2 &&
                       launch_receipt.host_receipt.mode_update.
                           set_title_active &&
                       launch_receipt.host_receipt.mode_update.title_active &&
                       launch_receipt.host_receipt.mode_update.
                           set_title_frame &&
                       launch_receipt.host_receipt.mode_update.title_frame == 0 &&
                       launch_receipt.host_receipt.input_result ==
                           NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                       strcmp(launch_receipt.host_receipt.status,
                              "NEXUS TITLE") == 0,
                   "Nexus launch receipt owns initial title and save scan state");
            expect(!nexus_v1_startup_launch_from_host_facts_with_receipt(
                       NULL,
                       &launch_receipt) &&
                       launch_receipt.host_receipt.status_scope &&
                       strcmp(launch_receipt.host_receipt.status_scope,
                              "BOOT") == 0 &&
                       launch_receipt.host_receipt.status &&
                       strcmp(launch_receipt.host_receipt.status,
                              "NEXUS STARTUP FAILED") == 0,
                   "Nexus launch receipt owns startup failure status");
            expect(nexus_v1_startup_boot_status_host_receipt(
                       NEXUS_V1_STARTUP_BOOT_STATUS_DATA_ERROR,
                       &boot_receipt) &&
                       boot_receipt.status_scope &&
                       strcmp(boot_receipt.status_scope, "BOOT") == 0 &&
                       boot_receipt.status &&
                       strcmp(boot_receipt.status, "NEXUS DATA ERROR") == 0,
                   "Nexus boot status receipt owns data-error status");
            expect(nexus_v1_startup_boot_status_host_receipt(
                       NEXUS_V1_STARTUP_BOOT_STATUS_LEVEL_ERROR,
                       &boot_receipt) &&
                       boot_receipt.status &&
                       strcmp(boot_receipt.status, "NEXUS LEVEL ERROR") == 0,
                   "Nexus boot status receipt owns level-error status");
            expect(nexus_v1_startup_boot_status_host_receipt(
                       NEXUS_V1_STARTUP_BOOT_STATUS_TITLE,
                       &boot_receipt) &&
                       boot_receipt.input_result ==
                           NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                       boot_receipt.status &&
                       strcmp(boot_receipt.status, "NEXUS TITLE") == 0,
                   "Nexus boot status receipt owns title status");
            expect(nexus_v1_startup_resume_status_host_receipt(
                       NEXUS_V1_STARTUP_RESUME_STATUS_FAILED,
                       &boot_receipt) &&
                       boot_receipt.status_scope &&
                       strcmp(boot_receipt.status_scope, "BOOT") == 0 &&
                       boot_receipt.status &&
                       strcmp(boot_receipt.status,
                              "NEXUS RESUME FAILED") == 0,
                   "Nexus resume receipt owns load failure status");
            expect(nexus_v1_startup_resume_status_host_receipt(
                       NEXUS_V1_STARTUP_RESUME_STATUS_LEVEL_INVALID,
                       &boot_receipt) &&
                       boot_receipt.status &&
                       strcmp(boot_receipt.status,
                              "NEXUS RESUME LEVEL INVALID") == 0,
                   "Nexus resume receipt owns bad-level status");
            expect(nexus_v1_startup_resume_status_host_receipt(
                       NEXUS_V1_STARTUP_RESUME_STATUS_RESUMED,
                       &boot_receipt) &&
                       boot_receipt.input_result ==
                           NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                       boot_receipt.status &&
                       strcmp(boot_receipt.status, "NEXUS RESUMED") == 0,
                   "Nexus resume receipt owns resumed redraw status");
        }
        expect(nexus_v1_startup_menu_handle_firestaff_input_from_facts_with_receipt(
                   &state_receipt,
                   save_dir,
                   menu.slot_mask,
                   0,
                   2,
                   &action) &&
                   strcmp(state_receipt.save_dir, save_dir) == 0 &&
                   state_receipt.row_count == 2 &&
                   state_receipt.selected_row == 1 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_NONE,
               "Nexus save receipt input helper owns M11 keyboard state");
        memset(&host_facts, 0, sizeof(host_facts));
        host_facts.save_dir = save_dir;
        host_facts.slot_mask = menu.slot_mask;
        host_facts.save_selected_row = 0;
        host_facts.save_row_count = menu.row_count;
        expect(nexus_v1_startup_menu_handle_firestaff_input_from_host_facts_with_receipt(
                   &state_receipt,
                   &host_facts,
                   2,
                   &action) &&
                   strcmp(state_receipt.save_dir, save_dir) == 0 &&
                   state_receipt.row_count == 2 &&
                   state_receipt.selected_row == 1 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_NONE,
               "Nexus save host facts input helper owns M11 keyboard state");
        load_calls = 0;
        expect(nexus_v1_startup_execute_save_firestaff_input_from_host_facts_with_receipt(
                   &host_facts,
                   2,
                   startup_load_success,
                   &load_calls,
                   &execution,
                   &host_action_receipt) &&
                   host_action_receipt.save_state_receipt_valid &&
                   host_action_receipt.save_state_receipt.selected_row == 1 &&
                   host_action_receipt.host_receipt.input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                   strcmp(host_action_receipt.host_receipt.status,
                          "NEXUS SAVE SELECT") == 0 &&
                   execution.kind ==
                       NEXUS_V1_STARTUP_SAVE_EXEC_STATUS_REDRAW &&
                   load_calls == 0,
               "Nexus save input wrapper returns M11-ready state and action receipt");
        load_calls = 0;
        memset(&save_route_receipt, 0, sizeof(save_route_receipt));
        expect(nexus_v1_startup_save_route_receipt_from_host_facts_input(
                   &host_facts,
                   2,
                   startup_load_success,
                   &load_calls,
                   &save_route_receipt) &&
                   save_route_receipt.route ==
                       NEXUS_V1_STARTUP_SAVE_ROUTE_NAVIGATE &&
                   strcmp(nexus_v1_startup_save_route_name(
                              save_route_receipt.route),
                          "navigate") == 0 &&
                   save_route_receipt.handled == 1 &&
                   save_route_receipt.save_state_receipt_valid &&
                   save_route_receipt.selected_row == 1 &&
                   save_route_receipt.row_count == 2 &&
                   save_route_receipt.draw_command_count > 3 &&
                   save_route_receipt.host_input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                   strcmp(save_route_receipt.status,
                          "NEXUS SAVE SELECT") == 0 &&
                   load_calls == 0,
               "Nexus save route receipt owns keyboard navigation handoff");
        snapshot_row_count =
            nexus_v1_startup_menu_snapshot_build_save_render_rows(
                &snapshot,
                snapshot_rows,
                (int)(sizeof(snapshot_rows) / sizeof(snapshot_rows[0])));
        expect(snapshot_row_count == 2 &&
                   snapshot_rows[0].kind == NEXUS_V1_STARTUP_ROW_SLOT &&
                   snapshot_rows[0].slot == 3 &&
                   snapshot_rows[0].selected == 0 &&
                   snapshot_rows[1].kind == NEXUS_V1_STARTUP_ROW_NEW_GAME &&
                   snapshot_rows[1].selected == 1,
               "startup snapshot render rows preserve slot and selection");
        {
            Nexus_V1_StartupDrawCommand commands[48];
            int command_count =
                nexus_v1_startup_presentation_build_save_from_facts(
                    save_dir,
                    menu.slot_mask,
                    1,
                    commands,
                    (int)(sizeof(commands) / sizeof(commands[0])));
            expect(command_count > 3 &&
                       commands[0].kind ==
                           NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
                   "startup save presentation facts helper owns M11 render snapshot construction");
            memset(&host_facts, 0, sizeof(host_facts));
            host_facts.save_dir = save_dir;
            host_facts.slot_mask = menu.slot_mask;
            host_facts.save_selected_row = 1;
            command_count =
                nexus_v1_startup_presentation_build_save_from_host_facts(
                    &host_facts,
                    commands,
                    (int)(sizeof(commands) / sizeof(commands[0])));
            expect(command_count > 3 &&
                       commands[0].kind ==
                           NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND,
                   "startup save presentation host facts helper owns M11 render snapshot construction");
        }
        memset(&hit, 0, sizeof(hit));
        hit.kind = NEXUS_V1_STARTUP_HIT_SAVE_ROW;
        hit.row = 0;
        expect(nexus_v1_startup_menu_snapshot_handle_hit(
                   &snapshot,
                   &hit,
                   &action) &&
                   snapshot.selected_row == 0 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_LOAD_SLOT &&
                   action.slot == 3 &&
                   strstr(action.path, "nexus_save_03.dat") != NULL,
               "startup snapshot pointer hit owns save activation");
        expect(nexus_v1_startup_menu_handle_pointer_from_facts(
                   &snapshot,
                   save_dir,
                   menu.slot_mask,
                   1,
                   menu.row_count,
                   20,
                   44,
                   &action) &&
                   strcmp(snapshot.save_dir, save_dir) == 0 &&
                   snapshot.row_count == 2 &&
                   snapshot.selected_row == 0 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_LOAD_SLOT &&
                   action.slot == 3 &&
                   strstr(action.path, "nexus_save_03.dat") != NULL,
               "startup snapshot facts pointer helper owns M11 save hit construction");
        expect(nexus_v1_startup_menu_handle_pointer_from_facts_with_receipt(
                   &state_receipt,
                   save_dir,
                   menu.slot_mask,
                   1,
                   menu.row_count,
                   20,
                   44,
                   &action) &&
                   strcmp(state_receipt.save_dir, save_dir) == 0 &&
                   state_receipt.row_count == 2 &&
                   state_receipt.selected_row == 0 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_LOAD_SLOT &&
                   action.slot == 3 &&
                   strstr(action.path, "nexus_save_03.dat") != NULL,
               "Nexus save receipt pointer helper owns M11 pointer state");
        memset(&host_facts, 0, sizeof(host_facts));
        host_facts.save_dir = save_dir;
        host_facts.slot_mask = menu.slot_mask;
        host_facts.save_selected_row = 1;
        host_facts.save_row_count = menu.row_count;
        expect(nexus_v1_startup_menu_handle_pointer_from_host_facts_with_receipt(
                   &state_receipt,
                   &host_facts,
                   20,
                   44,
                   &action) &&
                   strcmp(state_receipt.save_dir, save_dir) == 0 &&
                   state_receipt.row_count == 2 &&
                   state_receipt.selected_row == 0 &&
                   action.kind == NEXUS_V1_STARTUP_ACTION_LOAD_SLOT &&
                   action.slot == 3 &&
                   strstr(action.path, "nexus_save_03.dat") != NULL,
               "Nexus save host facts pointer helper owns M11 pointer state");
        load_calls = 0;
        expect(nexus_v1_startup_execute_save_pointer_from_host_facts_with_receipt(
                   &host_facts,
                   20,
                   44,
                   startup_load_success,
                   &load_calls,
                   &execution,
                   &host_action_receipt) &&
                   host_action_receipt.save_state_receipt_valid &&
                   host_action_receipt.save_state_receipt.selected_row == 0 &&
                   host_action_receipt.host_receipt.input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                   host_action_receipt.host_receipt.mode_update
                       .set_save_select_active &&
                   host_action_receipt.host_receipt.mode_update
                       .save_select_active == 0 &&
                   execution.kind ==
                       NEXUS_V1_STARTUP_SAVE_EXEC_LOAD_SLOT &&
                   load_calls == 1,
               "Nexus save pointer wrapper returns M11-ready state and action receipt");
        load_calls = 0;
        memset(&save_route_receipt, 0, sizeof(save_route_receipt));
        expect(nexus_v1_startup_save_route_receipt_from_host_facts_pointer(
                   &host_facts,
                   20,
                   44,
                   startup_load_success,
                   &load_calls,
                   &save_route_receipt) &&
                   save_route_receipt.route ==
                       NEXUS_V1_STARTUP_SAVE_ROUTE_LOAD_SLOT &&
                   save_route_receipt.handled == 1 &&
                   save_route_receipt.save_state_receipt_valid &&
                   save_route_receipt.selected_row == 0 &&
                   save_route_receipt.selected_slot == 3 &&
                   save_route_receipt.draw_command_count > 3 &&
                   save_route_receipt.set_save_select_active &&
                   save_route_receipt.save_select_active == 0 &&
                   load_calls == 1,
               "Nexus save route receipt owns pointer load-slot handoff");
        memset(&save_route_receipt, 0, sizeof(save_route_receipt));
        expect(nexus_v1_startup_save_route_receipt_from_host_facts_pointer(
                   &host_facts,
                   -50,
                   -50,
                   startup_load_success,
                   &load_calls,
                   &save_route_receipt) &&
                   save_route_receipt.route ==
                       NEXUS_V1_STARTUP_SAVE_ROUTE_POINTER_MISS &&
                   save_route_receipt.handled == 0 &&
                   save_route_receipt.save_state_receipt_valid &&
                   save_route_receipt.selected_row == 1 &&
                   save_route_receipt.draw_command_count > 3,
               "Nexus save route receipt preserves render state on pointer miss");

        nexus_v1_launcher_startup_runtime_state_clear(&runtime_state);
        runtime_state.save_select_active = 1;
        runtime_state.save_dir = save_dir;
        runtime_state.slot_mask = menu.slot_mask;
        runtime_state.save_selected_row = 0;
        runtime_state.save_row_count = menu.row_count;
        load_calls = 0;
        expect(nexus_v1_launcher_startup_save_route_receipt_from_runtime_state(
                   &runtime_state,
                   2,
                   startup_load_success,
                   &load_calls,
                   &save_route_receipt) &&
                   save_route_receipt.route ==
                       NEXUS_V1_STARTUP_SAVE_ROUTE_NAVIGATE &&
                   save_route_receipt.selected_row == 1 &&
                   save_route_receipt.draw_command_count > 3 &&
                   load_calls == 0,
               "Nexus launcher runtime save route owns keyboard handoff");

        runtime_state.save_selected_row = 1;
        load_calls = 0;
        expect(nexus_v1_launcher_startup_save_pointer_route_receipt_from_runtime_state(
                   &runtime_state,
                   20,
                   44,
                   startup_load_success,
                   &load_calls,
                   &save_route_receipt) &&
                   save_route_receipt.route ==
                       NEXUS_V1_STARTUP_SAVE_ROUTE_LOAD_SLOT &&
                   save_route_receipt.selected_slot == 3 &&
                   save_route_receipt.set_save_select_active &&
                   save_route_receipt.save_select_active == 0 &&
                   load_calls == 1,
               "Nexus launcher runtime save route owns slot handoff");

        nexus_v1_launcher_runtime_startup_snapshot_clear(&runtime_snapshot);
        runtime_snapshot.runtime = runtime_state;
        load_calls = 0;
        expect(nexus_v1_launcher_startup_save_pointer_route_receipt_from_snapshot(
                   &runtime_snapshot,
                   -50,
                   -50,
                   startup_load_success,
                   &load_calls,
                   &save_route_receipt) &&
                   save_route_receipt.route ==
                       NEXUS_V1_STARTUP_SAVE_ROUTE_POINTER_MISS &&
                   save_route_receipt.selected_row == 1 &&
                   save_route_receipt.draw_command_count > 3 &&
                   load_calls == 0,
               "Nexus launcher snapshot save route preserves pointer miss state");

        nexus_v1_launcher_startup_runtime_state_clear(&runtime_state);
        runtime_state.title_active = 1;
        runtime_state.title_frame = nexus_v1_boot_start_ready_frames();
        runtime_state.slot_mask = menu.slot_mask;
        expect(nexus_v1_launcher_startup_title_route_receipt_from_runtime_state(
                   &runtime_state,
                   10,
                   &title_route_receipt) &&
                   title_route_receipt.route ==
                       NEXUS_V1_STARTUP_TITLE_ROUTE_RETURN_TO_LAUNCHER &&
                   title_route_receipt.host_input_result ==
                       NEXUS_V1_STARTUP_HOST_INPUT_RETURN_TO_LAUNCHER &&
                   strcmp(title_route_receipt.status, "BACK TO LAUNCHER") == 0,
               "Nexus launcher runtime title route owns launcher return");

        nexus_v1_launcher_runtime_startup_snapshot_clear(&runtime_snapshot);
        runtime_snapshot.runtime = runtime_state;
        runtime_snapshot.runtime.title_frame = 30;
        expect(nexus_v1_launcher_startup_title_pointer_route_receipt_from_snapshot(
                   &runtime_snapshot,
                   &title_route_receipt) &&
                   title_route_receipt.route ==
                       NEXUS_V1_STARTUP_TITLE_ROUTE_HOLD &&
                   title_route_receipt.draw_command_count > 0 &&
                   strcmp(title_route_receipt.status, "NEXUS TITLE") == 0,
               "Nexus launcher snapshot title route preserves hold gate");
    }
    memset(&hit, 0, sizeof(hit));
    hit.kind = NEXUS_V1_STARTUP_HIT_SAVE_PANEL;
    hit.row = -1;
    memset(&action, 0, sizeof(action));
    expect(nexus_v1_startup_menu_handle_hit(&menu, &hit, &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_NONE &&
               menu.selected_row == 1,
           "startup menu panel hit is consumed without row activation");
    hit.kind = NEXUS_V1_STARTUP_HIT_SAVE_ROW;
    hit.row = 0;
    expect(nexus_v1_startup_menu_handle_hit(&menu, &hit, &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_LOAD_SLOT &&
               action.row == 0 &&
               action.slot == 3 &&
               menu.selected_row == 0,
           "startup menu save row hit activates selected slot through Nexus API");
    menu.selected_row = 1;

    kind = NEXUS_V1_STARTUP_ROW_NONE;
    slot = -1;
    expect(nexus_v1_startup_menu_row_at(&menu, 1, &kind, &slot),
           "startup menu row 1 exists");
    expect(kind == NEXUS_V1_STARTUP_ROW_NEW_GAME && slot == -1,
           "startup menu row 1 is NEW GAME");
    memset(&action, 0, sizeof(action));
    expect(nexus_v1_startup_menu_handle_input(
               &menu,
               NEXUS_V1_STARTUP_INPUT_ACTION,
               &action),
           "startup menu NEW GAME action activates");
    expect(action.kind == NEXUS_V1_STARTUP_ACTION_NEW_GAME &&
               action.row == 1 &&
               action.slot == -1 &&
               action.path[0] == '\0',
           "startup menu activation reports new-game action");
    expect(nexus_v1_startup_execute_save_action(&action, &execution) &&
               execution.kind == NEXUS_V1_STARTUP_SAVE_EXEC_SHOW_CHAMPIONS &&
               strcmp(execution.status, "NEXUS CHAMPIONS") == 0,
           "startup save execution resolves new-game champion select");
    expect(nexus_v1_startup_save_execution_mode_update(
               &execution,
               &mode_update) &&
               mode_update.set_save_select_active &&
               mode_update.save_select_active == 0 &&
               mode_update.set_champion_select_active &&
               mode_update.champion_select_active == 1 &&
               mode_update.set_champion_cursor &&
               mode_update.champion_cursor == 0 &&
               mode_update.set_champion_frame &&
               mode_update.champion_frame == 0,
           "startup save execution owns champion-select mode update");
    expect(nexus_v1_startup_menu_handle_input(
               &menu,
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE,
           "startup menu Back reports return-to-title action");
    expect(nexus_v1_startup_execute_save_action(&action, &execution) &&
               execution.kind == NEXUS_V1_STARTUP_SAVE_EXEC_SHOW_TITLE &&
               strcmp(execution.status, "NEXUS TITLE") == 0,
           "startup save execution resolves return-to-title");
    expect(nexus_v1_startup_save_execution_mode_update(
               &execution,
               &mode_update) &&
               mode_update.set_save_select_active &&
               mode_update.save_select_active == 0 &&
               mode_update.set_title_active &&
               mode_update.title_active == 1 &&
               mode_update.set_title_frame &&
               mode_update.title_frame == 0,
           "startup save execution owns title-return mode update");
    expect(nexus_v1_startup_menu_move_selected(&menu, -5) &&
               menu.selected_row == 0,
           "startup menu move selected clamps at first row");

    {
        Nexus_V1_LauncherBootReceipt boot_receipt;
        Nexus_V1_LauncherRuntimeReceipt runtime_receipt;
        char missing_dir[512];
        snprintf(missing_dir,
                 sizeof(missing_dir),
                 "%s/missing-nexus-data",
                 root);
        nexus_v1_launcher_boot_receipt_clear(&boot_receipt);
        expect(!nexus_v1_launcher_boot_level0_startup(
                   missing_dir,
                   NULL,
                   &boot_receipt),
               "Nexus launcher boot receipt rejects missing data");
        expect(boot_receipt.engine == NULL &&
                   !boot_receipt.level_loaded &&
                   boot_receipt.startup_receipt.host_receipt.status_scope &&
                   strcmp(boot_receipt.startup_receipt.host_receipt.status_scope,
                          "BOOT") == 0 &&
                   boot_receipt.startup_receipt.host_receipt.status &&
                   strcmp(boot_receipt.startup_receipt.host_receipt.status,
                          "NEXUS DATA ERROR") == 0,
               "Nexus launcher boot receipt owns missing-data status");
        expect(nexus_v1_launcher_get_engine() == NULL,
               "Nexus launcher missing-data boot leaves no active engine");
        nexus_v1_launcher_runtime_receipt_clear(&runtime_receipt);
        expect(!nexus_v1_launcher_boot_level0_runtime_startup(
                   missing_dir,
                   NULL,
                   &runtime_receipt),
               "Nexus launcher runtime receipt rejects missing data");
        expect(runtime_receipt.engine == NULL &&
                   strcmp(runtime_receipt.title,
                          NEXUS_V1_GAME_LABEL) == 0 &&
                   strcmp(runtime_receipt.source_id,
                          NEXUS_V1_GAME_ID) == 0 &&
                   runtime_receipt.startup_receipt.host_receipt.status &&
                   strcmp(runtime_receipt.startup_receipt.host_receipt.status,
                          "NEXUS DATA ERROR") == 0,
               "Nexus launcher runtime receipt owns M11 identity and failure status");
        expect(runtime_receipt.startup_assets.main_menu_route_ready == 0 &&
                   runtime_receipt.startup_assets.startup_audio_handoff_ready == 0 &&
                   runtime_receipt.startup_assets.menu_bpk_upload_route ==
                       NEXUS_V1_BPK_UPLOAD_ROUTE_INVALID &&
                   runtime_receipt.startup_assets.startup_sfx_status ==
                       NEXUS_SFX_RUNTIME_MISSING,
               "Nexus launcher missing-data startup asset receipt stays blocked");
        expect(nexus_v1_launcher_startup_launch_gate_from_runtime_receipt(
                   &runtime_receipt,
                   &launch_gate_receipt) &&
                   launch_gate_receipt.route ==
                       NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR &&
                   strcmp(nexus_v1_launcher_startup_launch_gate_route_name(
                              launch_gate_receipt.route),
                          "data-error") == 0 &&
                   !launch_gate_receipt.engine_ready &&
                   strcmp(launch_gate_receipt.status, "NEXUS DATA ERROR") == 0,
               "Nexus launcher missing-data runtime emits launch gate receipt");
        expect(nexus_v1_launcher_startup_asset_handoff_from_runtime_receipt(
                   &runtime_receipt,
                   &asset_handoff_receipt) &&
                   asset_handoff_receipt.route ==
                       NEXUS_V1_STARTUP_ASSET_HANDOFF_DATA_ERROR &&
                   strcmp(nexus_v1_launcher_startup_asset_handoff_route_name(
                              asset_handoff_receipt.route),
                          "data-error") == 0 &&
                   asset_handoff_receipt.title_asset_handoff_ready == 0 &&
                   asset_handoff_receipt.audio_asset_handoff_ready == 0 &&
                   asset_handoff_receipt.main_menu_route_ready == 0 &&
                   asset_handoff_receipt.saturn_asset_handoff_ready == 0 &&
                   asset_handoff_receipt.real_asset_route_ready == 0 &&
                   asset_handoff_receipt.blocks_main_menu_route == 1 &&
                   strcmp(asset_handoff_receipt.status,
                          "NEXUS DATA ERROR") == 0,
               "Nexus startup asset handoff blocks missing runtime data");
    }

    {
        const char *home = getenv("HOME");
        char nexus_dir[512];
        char dm_bin[512];
        char lev00[512];
        Nexus_TitleScreen title_screen;
        Nexus_V1_LauncherRuntimeReceipt runtime_receipt;
        if (!home || !home[0]) {
            puts("SKIP: HOME unset; no local Nexus launcher asset receipt check");
        } else {
            snprintf(nexus_dir, sizeof(nexus_dir),
                     "%s/.firestaff/data/nexus", home);
            snprintf(dm_bin, sizeof(dm_bin), "%s/DM.BIN", nexus_dir);
            snprintf(lev00, sizeof(lev00), "%s/LEV00.DGN", nexus_dir);
            if (!local_file_exists(dm_bin) || !local_file_exists(lev00)) {
                puts("SKIP: local Nexus DM.BIN/LEV00.DGN not present for launcher asset receipt");
            } else {
                memset(&title_screen, 0, sizeof(title_screen));
                nexus_v1_launcher_runtime_receipt_clear(&runtime_receipt);
                expect(nexus_v1_launcher_boot_level0_runtime_startup(
                           nexus_dir,
                           &title_screen,
                           &runtime_receipt),
                       "Nexus launcher runtime startup boots local real data");
                expect(runtime_receipt.startup_assets.title_screen_loaded == 1 &&
                           runtime_receipt.startup_assets.startup_surfaces_expected >= 1 &&
                           runtime_receipt.startup_assets.faces_expected == NEXUS_MAX_CHAMPIONS,
                       "Nexus launcher asset receipt exposes title/startup/face route");
                expect(runtime_receipt.startup_assets.startup_sfx_level_index == 0 &&
                           runtime_receipt.startup_assets.startup_cd_track == 2 &&
                           runtime_receipt.startup_assets.startup_audio_handoff_ready == 1,
                       "Nexus launcher asset receipt exposes level-0 audio handoff");
                expect(runtime_receipt.startup_assets.main_menu_route_ready == 1,
                       "Nexus launcher asset receipt marks main menu route ready");
                expect(runtime_receipt.startup_assets.title_route_ready == 1,
                       "Nexus launcher asset gate allows title route with real startup surfaces");
                if (runtime_receipt.startup_assets.menu_bpk_upload_receipt_valid) {
                    expect(runtime_receipt.startup_assets.menu_bpk_upload_route ==
                               NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3 &&
                               runtime_receipt.startup_assets.menu_bpk_blocked_prs3_uploads > 0 &&
                               runtime_receipt.startup_assets.menu_bpk_blocks_real_menu_surface_render == 1,
                           "Nexus launcher asset receipt exposes MENU.BPK PRS3 blocker");
                    expect(runtime_receipt.startup_assets.real_menu_surface_route_ready == 0 &&
                               runtime_receipt.startup_assets.real_menu_surface_route_blocked == 1 &&
                               runtime_receipt.startup_assets.save_menu_route_ready == 0 &&
                               runtime_receipt.startup_assets.champion_menu_route_ready == 0 &&
                               strcmp(runtime_receipt.startup_assets.real_menu_surface_blocker,
                                      "menu-bpk-prs3") == 0 &&
                               strcmp(runtime_receipt.startup_assets.startup_menu_asset_route,
                                      "blocked-menu-bpk-prs3") == 0,
                           "Nexus launcher asset gate blocks save/champion menus on PRS3");
                    expect(nexus_v1_launcher_startup_launch_gate_from_runtime_receipt(
                               &runtime_receipt,
                               &launch_gate_receipt) &&
                               launch_gate_receipt.route ==
                                   NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_ASSET_BLOCKED &&
                               strcmp(nexus_v1_launcher_startup_launch_gate_route_name(
                                          launch_gate_receipt.route),
                                      "menu-asset-blocked") == 0 &&
                               launch_gate_receipt.engine_ready == 1 &&
                               launch_gate_receipt.title_draw_ready == 1 &&
                               launch_gate_receipt.real_menu_ready == 0 &&
                               launch_gate_receipt.fallback_visuals_permitted == 0 &&
                               strcmp(launch_gate_receipt.asset_blocker,
                                      "menu-bpk-prs3") == 0 &&
                               strcmp(launch_gate_receipt.status,
                                      "blocked-menu-bpk-prs3") == 0,
                           "Nexus launcher runtime emits MENU.BPK launch gate blocker");
                    expect(nexus_v1_launcher_startup_asset_handoff_from_runtime_receipt(
                               &runtime_receipt,
                               &asset_handoff_receipt) &&
                               asset_handoff_receipt.route ==
                                   NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED &&
                               strcmp(nexus_v1_launcher_startup_asset_handoff_route_name(
                                          asset_handoff_receipt.route),
                                      "menu-blocked") == 0 &&
                               asset_handoff_receipt.title_asset_handoff_ready == 1 &&
                               asset_handoff_receipt.audio_asset_handoff_ready == 1 &&
                               asset_handoff_receipt.real_menu_asset_handoff_ready == 0 &&
                               asset_handoff_receipt.main_menu_route_ready == 0 &&
                               asset_handoff_receipt.saturn_asset_handoff_ready == 1 &&
                               asset_handoff_receipt.real_asset_route_ready == 0 &&
                               asset_handoff_receipt.menu_bpk_renderer_handoff_valid == 1 &&
                               asset_handoff_receipt.menu_bpk_renderer_handoff.status ==
                                   NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3 &&
                               asset_handoff_receipt.menu_bpk_renderer_handoff
                                       .blocked_prs3_surfaces > 0 &&
                               asset_handoff_receipt.menu_bpk_renderer_handoff
                                       .fallback_visuals_permitted == 0 &&
                               asset_handoff_receipt
                                       .menu_bpk_prs3_blocks_real_menu_route == 1 &&
                               asset_handoff_receipt.blocks_main_menu_route == 1 &&
                               strcmp(asset_handoff_receipt.title_asset_route,
                                      "ready-title-assets") == 0 &&
                               strcmp(asset_handoff_receipt.audio_asset_route,
                                      "ready-track02-sfx") == 0 &&
                               strcmp(asset_handoff_receipt.menu_asset_route,
                                      "blocked-menu-bpk-prs3") == 0 &&
                               strcmp(asset_handoff_receipt.status,
                                      "blocked-menu-bpk-prs3") == 0,
                           "Nexus startup asset handoff blocks main menu on PRS3 surfaces");
                    nexus_v1_launcher_startup_runtime_state_clear(
                        &runtime_state);
                    runtime_state.engine = runtime_receipt.engine;
                    runtime_state.title_active = 1;
                    runtime_state.title_frame =
                        nexus_v1_boot_start_ready_frames();
                    runtime_state.slot_mask = menu.slot_mask;
                    expect(nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
                               &runtime_state,
                               &startup_assets_receipt) &&
                               startup_assets_receipt.title_route_ready ==
                                   runtime_receipt.startup_assets.title_route_ready &&
                               startup_assets_receipt.save_menu_route_ready == 0 &&
                               startup_assets_receipt.champion_menu_route_ready == 0 &&
                               strcmp(startup_assets_receipt.startup_menu_asset_route,
                                      "blocked-menu-bpk-prs3") == 0,
                           "Nexus launcher runtime state exposes startup asset gate");
                    nexus_v1_launcher_runtime_startup_snapshot_clear(
                        &runtime_snapshot);
                    runtime_snapshot.runtime = runtime_state;
                    expect(nexus_v1_launcher_startup_assets_receipt_from_snapshot(
                               &runtime_snapshot,
                               &startup_assets_receipt) &&
                               startup_assets_receipt
                                   .menu_bpk_blocks_real_menu_surface_render == 1 &&
                               strcmp(startup_assets_receipt.real_menu_surface_blocker,
                                      "menu-bpk-prs3") == 0,
                           "Nexus launcher snapshot exposes startup asset gate");
                    expect(nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
                               &runtime_state,
                               9,
                               &title_handoff_receipt) &&
                               title_handoff_receipt.title_route.route ==
                                   NEXUS_V1_STARTUP_TITLE_ROUTE_SAVE_SELECT &&
                               title_handoff_receipt.route_ready == 0 &&
                               title_handoff_receipt.route_blocked == 1 &&
                               title_handoff_receipt.title_draw_ready == 1 &&
                               title_handoff_receipt.save_menu_ready == 0 &&
                               title_handoff_receipt.host_receipt.input_result ==
                                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                               !title_handoff_receipt.host_receipt.mode_update
                                    .set_save_select_active &&
                               strcmp(title_handoff_receipt.status_scope,
                                      "ASSETS") == 0 &&
                               strcmp(title_handoff_receipt.status,
                                      "blocked-menu-bpk-prs3") == 0,
                           "Nexus launcher title handoff blocks save route on MENU.BPK PRS3");
                    expect(nexus_v1_launcher_startup_execute_title_firestaff_input_from_runtime_state(
                               &runtime_state,
                               9,
                               &title_execution,
                               &host_action_receipt) &&
                               title_execution.kind ==
                                   NEXUS_V1_STARTUP_TITLE_EXEC_IGNORE &&
                               host_action_receipt.host_receipt.input_result ==
                                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                               !host_action_receipt.host_receipt.mode_update
                                    .set_save_select_active &&
                               strcmp(host_action_receipt.host_receipt.status_scope,
                                      "ASSETS") == 0 &&
                               strcmp(host_action_receipt.host_receipt.status,
                                      "blocked-menu-bpk-prs3") == 0,
                           "Nexus launcher title execute blocks save route on MENU.BPK PRS3");
                    runtime_state.slot_mask = 0u;
                    expect(nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
                               &runtime_state,
                               11,
                               &title_handoff_receipt) &&
                               title_handoff_receipt.title_route.route ==
                                   NEXUS_V1_STARTUP_TITLE_ROUTE_CHAMPION_SELECT &&
                               title_handoff_receipt.route_blocked == 1 &&
                               title_handoff_receipt.champion_menu_ready == 0 &&
                               !title_handoff_receipt.host_receipt.mode_update
                                    .set_champion_select_active &&
                               strcmp(title_handoff_receipt.status,
                                      "blocked-menu-bpk-prs3") == 0,
                           "Nexus launcher title handoff blocks champion route on MENU.BPK PRS3");
                    runtime_state.slot_mask = menu.slot_mask;
                    runtime_state.title_frame = 30;
                    expect(nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_runtime_state(
                               &runtime_state,
                               &title_handoff_receipt) &&
                               title_handoff_receipt.title_route.route ==
                                   NEXUS_V1_STARTUP_TITLE_ROUTE_HOLD &&
                               title_handoff_receipt.route_ready == 1 &&
                               title_handoff_receipt.route_blocked == 0 &&
                               strcmp(title_handoff_receipt.status,
                                      "NEXUS TITLE") == 0,
                           "Nexus launcher title handoff keeps title hold route drawable");
                    expect(nexus_v1_launcher_startup_execute_title_pointer_from_runtime_state(
                               &runtime_state,
                               &title_execution,
                               &host_action_receipt) &&
                               title_execution.kind ==
                                   NEXUS_V1_STARTUP_TITLE_EXEC_HOLD_TITLE &&
                               host_action_receipt.host_receipt.input_result ==
                                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                               strcmp(host_action_receipt.host_receipt.status,
                                      "NEXUS TITLE") == 0,
                           "Nexus launcher title execute preserves hold route");
                    runtime_state.title_frame =
                        nexus_v1_boot_start_ready_frames();
                    runtime_state.save_select_active = 1;
                    runtime_state.save_dir = save_dir;
                    runtime_state.save_selected_row = 0;
                    runtime_state.save_row_count = menu.row_count;
                    load_calls = 0;
                    expect(nexus_v1_launcher_startup_save_pointer_route_receipt_from_runtime_state(
                               &runtime_state,
                               20,
                               44,
                               startup_load_success,
                               &load_calls,
                               &save_route_receipt) &&
                               save_route_receipt.route ==
                                   NEXUS_V1_STARTUP_SAVE_ROUTE_ASSET_BLOCKED &&
                               strcmp(nexus_v1_startup_save_route_name(
                                          save_route_receipt.route),
                                      "asset-blocked") == 0 &&
                               save_route_receipt.host_input_result ==
                                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                               strcmp(save_route_receipt.status_scope,
                                      "ASSETS") == 0 &&
                               strcmp(save_route_receipt.status,
                                      "blocked-menu-bpk-prs3") == 0 &&
                               load_calls == 0,
                           "Nexus launcher blocks save-slot route on MENU.BPK PRS3");
                    load_calls = 0;
                    expect(nexus_v1_launcher_startup_execute_save_pointer_from_runtime_state(
                               &runtime_state,
                               20,
                               44,
                               startup_load_success,
                               &load_calls,
                               &execution,
                               &host_action_receipt) &&
                               execution.kind ==
                                   NEXUS_V1_STARTUP_SAVE_EXEC_IGNORE &&
                               host_action_receipt.save_state_receipt_valid &&
                               host_action_receipt.host_receipt.input_result ==
                                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                               strcmp(host_action_receipt.host_receipt.status,
                                      "blocked-menu-bpk-prs3") == 0 &&
                               load_calls == 0,
                           "Nexus launcher save execute blocks load-slot on MENU.BPK PRS3");
                    runtime_state.champion_select_active = 1;
                    runtime_state.champion_cursor = 0;
                    runtime_state.champion_frame = 0;
                    expect(nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
                               &runtime_state,
                               9,
                               &champion_execution,
                               &host_action_receipt) &&
                               champion_execution.kind ==
                                   NEXUS_V1_STARTUP_CHAMPION_EXEC_IGNORE &&
                               host_action_receipt.champion_state_receipt_valid &&
                               host_action_receipt.host_receipt.input_result ==
                                   NEXUS_V1_STARTUP_HOST_INPUT_REDRAW &&
                               strcmp(host_action_receipt.host_receipt.status_scope,
                                      "ASSETS") == 0 &&
                               strcmp(host_action_receipt.host_receipt.status,
                                      "blocked-menu-bpk-prs3") == 0,
                           "Nexus launcher blocks champion route on MENU.BPK PRS3");
                    memset(draw_commands, 0, sizeof(draw_commands));
                    expect(nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
                               &runtime_state,
                               draw_commands,
                               (int)(sizeof(draw_commands) /
                                     sizeof(draw_commands[0])),
                               &presentation_receipt) &&
                               presentation_receipt.kind ==
                                   NEXUS_V1_STARTUP_MENU_PRESENTATION_SAVE &&
                               presentation_receipt.route_blocked == 1 &&
                               presentation_receipt.route_ready == 0 &&
                               presentation_receipt.draw_command_count == 0 &&
                               presentation_receipt.host_caller_valid == 1 &&
                               presentation_receipt.package_capture_consumed_by_host == 0 &&
                               presentation_receipt.display_callers_use_package_receipt == 0 &&
                               presentation_receipt.suppress_fallback_visuals == 1 &&
                               presentation_receipt.blocked_route_suppresses_all_draws == 1 &&
                               presentation_receipt.assets
                                   .menu_bpk_blocks_real_menu_surface_render == 1 &&
                               strcmp(presentation_receipt.status_scope,
                                      "ASSETS") == 0 &&
                               strcmp(presentation_receipt.status,
                                      "blocked-menu-bpk-prs3") == 0,
                           "Nexus launcher save presentation receipt blocks fallback draw commands");
                    memset(draw_commands, 0, sizeof(draw_commands));
                    expect(nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
                               &runtime_state,
                               draw_commands,
                               (int)(sizeof(draw_commands) /
                                     sizeof(draw_commands[0])),
                               &presentation_receipt) &&
                               presentation_receipt.kind ==
                                   NEXUS_V1_STARTUP_MENU_PRESENTATION_CHAMPION &&
                               presentation_receipt.route_blocked == 1 &&
                               presentation_receipt.draw_command_count == 0 &&
                               presentation_receipt.host_caller_valid == 1 &&
                               presentation_receipt.package_capture_consumed_by_host == 0 &&
                               presentation_receipt.display_callers_use_package_receipt == 0 &&
                               presentation_receipt.suppress_fallback_visuals == 1 &&
                               presentation_receipt.blocked_route_suppresses_all_draws == 1 &&
                               strcmp(presentation_receipt.asset_blocker,
                                      "menu-bpk-prs3") == 0 &&
                               strcmp(presentation_receipt.host_receipt.status,
                                      "blocked-menu-bpk-prs3") == 0,
                           "Nexus launcher champion presentation receipt blocks fallback draw commands");
                }
                nexus_title_free(&title_screen);
                nexus_v1_launcher_shutdown();
            }
        }
    }

    snprintf(path, sizeof(path), "%s/nexus_save_03.dat", save_dir);
    TST_UNLINK(path);
    TST_RMDIR(save_dir);
    TST_RMDIR(root);

    if (g_failures) {
        fprintf(stderr,
                "test_nexus_v1_startup_menu_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    puts("ok: Nexus startup menu scans FNXS slots and exposes LOAD/NEW rows");
    return 0;
}
