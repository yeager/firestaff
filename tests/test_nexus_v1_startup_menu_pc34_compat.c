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
    Nexus_V1_StartupHit hit;
    Nexus_V1_StartupRowKind kind;
    Nexus_V1_TitleFrame title_frame;
    Nexus_V1_BootFrame boot_frame;
    int slot;
    int cursor;
    Nexus_V1_ChampionPool empty_champions;

    if (!make_temp_root(root, sizeof(root))) {
        fprintf(stderr, "FAIL: could not create temporary root\n");
        return 1;
    }
    snprintf(save_dir, sizeof(save_dir), "%s/saves", root);

    memset(&empty_champions, 0, sizeof(empty_champions));
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
    expect(nexus_v1_startup_champion_handle_input(
               &champions,
               &cursor,
               0u,
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE,
           "Nexus champion startup Back returns to title with no save slots");
    expect(nexus_v1_startup_champion_handle_input(
               &champions,
               &cursor,
               (1u << 3),
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT,
           "Nexus champion startup Back returns to save select when slots exist");
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
    expect(nexus_v1_startup_title_handle_input(
               12,
               menu.slot_mask,
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER,
           "startup title Back returns to launcher");
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
    expect(nexus_v1_startup_menu_handle_input(
               &menu,
               NEXUS_V1_STARTUP_INPUT_DOWN,
               &action) &&
               menu.selected_row == 1,
           "startup menu Down advances to NEW GAME");
    expect(action.kind == NEXUS_V1_STARTUP_ACTION_NONE,
           "startup menu Down is a navigation-only action");
    expect(nexus_v1_startup_menu_handle_input(
               &menu,
               NEXUS_V1_STARTUP_INPUT_DOWN,
               &action) &&
               menu.selected_row == 1,
           "startup menu Down clamps at last row");
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
    expect(nexus_v1_startup_menu_handle_input(
               &menu,
               NEXUS_V1_STARTUP_INPUT_BACK,
               &action) &&
               action.kind == NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE,
           "startup menu Back reports return-to-title action");
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
