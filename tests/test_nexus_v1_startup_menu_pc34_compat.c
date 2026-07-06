#include "nexus_v1_startup_menu.h"
#include "nexus_v1_champions.h"
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
    Nexus_V1_StartupRowKind kind;
    int slot;

    if (!make_temp_root(root, sizeof(root))) {
        fprintf(stderr, "FAIL: could not create temporary root\n");
        return 1;
    }
    snprintf(save_dir, sizeof(save_dir), "%s/saves", root);

    nexus_v1_champions_init(&champions);
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
    expect(nexus_v1_startup_menu_activate_selected(&menu, &action),
           "startup menu selected slot activates");
    expect(action.kind == NEXUS_V1_STARTUP_ACTION_LOAD_SLOT &&
               action.row == 0 &&
               action.slot == 3 &&
               strstr(action.path, "nexus_save_03.dat") != NULL,
           "startup menu activation reports load-slot action");
    expect(nexus_v1_startup_menu_move_selected(&menu, 1) &&
               menu.selected_row == 1,
           "startup menu move selected advances to NEW GAME");
    expect(nexus_v1_startup_menu_move_selected(&menu, 1) &&
               menu.selected_row == 1,
           "startup menu move selected clamps at last row");

    kind = NEXUS_V1_STARTUP_ROW_NONE;
    slot = -1;
    expect(nexus_v1_startup_menu_row_at(&menu, 1, &kind, &slot),
           "startup menu row 1 exists");
    expect(kind == NEXUS_V1_STARTUP_ROW_NEW_GAME && slot == -1,
           "startup menu row 1 is NEW GAME");
    memset(&action, 0, sizeof(action));
    expect(nexus_v1_startup_menu_activate_selected(&menu, &action),
           "startup menu NEW GAME activates");
    expect(action.kind == NEXUS_V1_STARTUP_ACTION_NEW_GAME &&
               action.row == 1 &&
               action.slot == -1 &&
               action.path[0] == '\0',
           "startup menu activation reports new-game action");
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
