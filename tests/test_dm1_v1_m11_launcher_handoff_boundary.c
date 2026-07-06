/*
 * test_dm1_v1_m11_launcher_handoff_boundary.c
 *
 * M12 -> M11 DM1 V1 normal launcher handoff boundary.
 *
 * The source-order gates prove the ReDMCSB SWSH/TITLE/entrance sequence.
 * This focused C gate proves the production selected-menu path for a plain
 * DM1 start: M12_StartupMenu_GetLaunchIntent() ->
 * M11_GameView_OpenSelectedMenuEntry().
 *
 * Source-lock: ReDMCSB APPA.C FTL_SWSH -> FTL_TITL, TITLE.C F0437,
 * and ENTRANCE.C F0441. A launcher/CLI DM1 start must remain classified as
 * source-visible startup, while direct M11 test/dev starts are the explicit
 * game-view intro bypass.
 *
 * Skip-clean without user-supplied DM1 data. With ~/.firestaff/data staged,
 * this becomes a real launcher-to-DM1 handoff proof.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_PATH_SEP "\\"
#define TEST_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_PATH_SEP "/"
#define TEST_GETPID() getpid()
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures = 0;
static int g_passed = 0;
static int g_skipped = 0;

static void expect_true(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    } else {
        ++g_passed;
    }
}

static void expect_skip(const char* message) {
    fprintf(stderr, "SKIP: %s\n", message);
    ++g_skipped;
}

static void dismiss_initial_message(M12_StartupMenuState* state) {
    if (state && state->view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
    }
}

static void make_empty_data_dir(char out[512]) {
    int rc = snprintf(out, 512,
                      "%s%sfirestaff_dm1_launcher_empty_%ld",
                      (getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp"),
                      TEST_PATH_SEP, (long)TEST_GETPID());
    if (rc > 0 && rc < 512) {
        (void)TEST_MKDIR(out);
    } else {
        out[0] = '\0';
    }
}

static const char* default_data_root(char fallback[512]) {
    const char* home = getenv("HOME");
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(fallback, 512, "%s/.firestaff/data", home);
    return fallback;
}

static void run_empty_launcher_boundary(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    const M12_MenuEntry* entry;
    char empty_dir[512];

    make_empty_data_dir(empty_dir);
    expect_true(empty_dir[0] != '\0',
                "DM1 launcher handoff empty data dir path was created");

    M12_StartupMenu_InitWithDataDir(&menu, empty_dir, "dm1");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 0);
    expect_true(entry != NULL,
                "M12 exposes a DM1 menu entry at game slot 0");
    expect_true(entry && entry->gameId && strcmp(entry->gameId, "dm1") == 0,
                "M12 game slot 0 gameId is \"dm1\"");
    expect_true(entry && entry->available == 0,
                "DM1 entry is unavailable when required assets are absent");

    menu.selectedIndex = 0;
    menu.activatedIndex = 0;
    menu.launchRequested = 1;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.gameId && strcmp(intent.gameId, "dm1") == 0,
                "DM1 launch intent carries gameId=\"dm1\"");
    expect_true(intent.valid == 0,
                "DM1 launch intent is invalid when assets are absent");

    M12_StartupMenu_Destroy(&menu);
}

static void run_real_launcher_handoff_if_available(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    M11_GameViewState launcher_view;
    M11_GameViewState direct_view;
    M11_GameLaunchSpec direct_spec;
    const M12_MenuEntry* entry;
    char real_dir[512];
    const char* data_dir = default_data_root(real_dir);

    if (!data_dir || !data_dir[0]) {
        expect_skip("HOME is unset; no default Firestaff data root");
        return;
    }

    M12_StartupMenu_InitWithDataDir(&menu, data_dir, "dm1");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 0);
    if (!entry || !entry->available ||
        !M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        M12_StartupMenu_Destroy(&menu);
        expect_skip("no launchable DM1 data under default data root");
        return;
    }

    menu.selectedIndex = 0;
    menu.activatedIndex = 0;
    menu.launchRequested = 1;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.valid == 1,
                "M12 DM1 launch intent is valid with real staged data");
    expect_true(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL,
                "M12 DM1 launch intent uses V1 original presentation");
    if (!intent.valid) {
        M12_StartupMenu_Destroy(&menu);
        return;
    }

    M11_GameView_Init(&launcher_view);
    expect_true(M11_GameView_OpenSelectedMenuEntry(&launcher_view, &menu) == 1,
                "M11 opens DM1 through M12 selected-menu entry");
    expect_true(launcher_view.startedFromLauncher == 1,
                "M11 marks DM1 startup as launcher-started");
    expect_true(launcher_view.active == 1,
                "M11 DM1 launcher handoff leaves view active");
    expect_true(launcher_view.sourceKind == M11_GAME_SOURCE_BUILTIN_CATALOG,
                "M11 DM1 launcher handoff claims builtin catalog source");
    expect_true(strcmp(launcher_view.sourceId, "dm1") == 0,
                "M11 DM1 launcher handoff preserves sourceId dm1");
    expect_true(M11_GameView_Dm1StartupIntroBypassed(&launcher_view) == 0,
                "M11 DM1 launcher handoff does not mark intro bypass");
    expect_true(launcher_view.assetsAvailable == 1,
                "M11 DM1 launcher handoff opens GRAPHICS.DAT assets");
    expect_true(launcher_view.dungeonPath[0] != '\0',
                "M11 DM1 launcher handoff records a DUNGEON.DAT path");
    expect_true(launcher_view.mirrorCatalogAvailable == 1,
                "M11 DM1 launcher handoff builds the HoC mirror catalog");

    memset(&direct_spec, 0, sizeof(direct_spec));
    direct_spec.title = "DUNGEON MASTER";
    direct_spec.gameId = "dm1";
    direct_spec.sourceId = "dm1";
    direct_spec.dataDir = data_dir;
    direct_spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    direct_spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    direct_spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&direct_view);
    expect_true(M11_GameView_Start(&direct_view, &direct_spec) == 1,
                "M11 direct DM1 game-view start succeeds with real data");
    expect_true(M11_GameView_Dm1StartupIntroBypassed(&direct_view) == 1,
                "M11 direct DM1 game-view start is the explicit intro bypass");

    M11_GameView_Shutdown(&direct_view);
    M11_GameView_Shutdown(&launcher_view);
    M12_StartupMenu_Destroy(&menu);
}

int main(void) {
    printf("=== DM1 V1 M12/M11 launcher handoff boundary ===\n");

    run_empty_launcher_boundary();
    run_real_launcher_handoff_if_available();

    printf("\nDM1 V1 M12/M11 launcher handoff boundary: %d passed, %d failed, %d skipped\n",
           g_passed, g_failures, g_skipped);
    if (g_failures) {
        fprintf(stderr,
                "DM1 V1 M12/M11 launcher handoff boundary FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: DM1 V1 M12/M11 launcher handoff boundary is wired");
    return 0;
}
