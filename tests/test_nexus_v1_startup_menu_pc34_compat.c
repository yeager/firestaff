#include "nexus_v1_startup_menu.h"
#include "nexus_v1_champions.h"
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
    Nexus_V1_StartupHit hit;
    Nexus_V1_StartupSaveRenderRow save_rows[4];
    Nexus_V1_StartupChampionRenderRow champion_rows[12];
    Nexus_V1_StartupChampionFooterRender champion_footer;
    Nexus_V1_StartupChromeRender chrome;
    Nexus_V1_StartupDrawCommand draw_commands[80];
    Nexus_V1_StartupMenuSnapshot menu_snapshot;
    Nexus_V1_StartupChampionSnapshot champion_snapshot;
    Nexus_V1_StartupRowKind kind;
    Nexus_V1_TitleFrame title_frame;
    Nexus_V1_BootFrame boot_frame;
    int slot;
    int cursor;
    int draw_count;
    int portrait_draws;
    int load_calls;
    Nexus_V1_ChampionPool empty_champions;

    if (!make_temp_root(root, sizeof(root))) {
        fprintf(stderr, "FAIL: could not create temporary root\n");
        return 1;
    }
    snprintf(save_dir, sizeof(save_dir), "%s/saves", root);

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
        }
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
