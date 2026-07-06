/*
 * test_theron_v1_m11_launcher_handoff_boundary.c
 *
 * M12 -> M11 Theron's Quest V1 launcher handoff boundary.
 *
 * The direct M11 Theron gate proves M11_GameView_Start() when a caller
 * already supplies a verified Track 02 path. This companion gate proves
 * the production launcher entry path:
 * M12_StartupMenu_GetLaunchIntent() -> M11_GameView_OpenSelectedMenuEntry().
 *
 * Source-lock boundary: THQUEST.ASM T400 data-track loading. ReDMCSB has
 * no Theron code; the Firestaff contract here is that M12's hash-verified
 * Track 02 selection reaches M11's visible stage-select/Soul Room startup
 * model without a separate runtime data-root walk.
 *
 * Skip-clean without user-supplied Theron data. With ~/.firestaff/data
 * staged, this becomes a real launcher handoff proof.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "theron_v1_startup_flow.h"

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

static int count_nonzero_pixels(const unsigned char* pixels, size_t count) {
    size_t i;
    int nonzero = 0;
    if (!pixels) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (pixels[i] != 0u) {
            ++nonzero;
        }
    }
    return nonzero;
}

static int startup_rows_contain(
    char rows[][M11_THERON_STARTUP_RENDER_ROW_CAPACITY],
    int row_count,
    const char* needle) {
    int i;
    if (!rows || !needle) {
        return 0;
    }
    for (i = 0; i < row_count; ++i) {
        if (strstr(rows[i], needle) != NULL) {
            return 1;
        }
    }
    return 0;
}

static void make_empty_data_dir(char out[512]) {
    int rc = snprintf(out, 512,
                      "%s%sfirestaff_theron_launcher_empty_%ld",
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
                "Theron launcher handoff empty data dir path was created");

    M12_StartupMenu_InitWithDataDir(&menu, empty_dir, "theron");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 4);
    expect_true(entry != NULL,
                "M12 exposes a Theron menu entry at game slot 4");
    expect_true(entry && entry->gameId &&
                    strcmp(entry->gameId, "theron") == 0,
                "M12 game slot 4 gameId is \"theron\"");
    expect_true(entry && entry->available == 0,
                "Theron entry is unavailable when no Track 02 assets are present");

    menu.selectedIndex = 4;
    menu.activatedIndex = 4;
    menu.launchRequested = 1;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.gameId && strcmp(intent.gameId, "theron") == 0,
                "Theron launch intent carries gameId=\"theron\"");
    expect_true(intent.valid == 0,
                "Theron launch intent is invalid when assets are absent");
}

static void run_real_launcher_handoff_if_available(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    M11_GameViewState view;
    const M12_MenuEntry* entry;
    char real_dir[512];
    const char* data_dir = default_data_root(real_dir);
    unsigned char framebuffer[320 * 200];
    char startup_rows[16][M11_THERON_STARTUP_RENDER_ROW_CAPACITY];
    int row_count;

    if (!data_dir || !data_dir[0]) {
        expect_skip("HOME is unset; no default Firestaff data root");
        return;
    }

    M12_StartupMenu_InitWithDataDir(&menu, data_dir, "theron");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 4);
    if (!entry || !entry->available ||
        !M12_AssetStatus_GameAvailable(&menu.assetStatus, "theron")) {
        expect_skip("no launchable Theron Track 02 data under default data root");
        return;
    }

    menu.selectedIndex = 4;
    menu.activatedIndex = 4;
    menu.launchRequested = 1;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.valid == 1,
                "M12 Theron launch intent is valid with real staged data");
    expect_true(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL,
                "M12 Theron launch intent uses V1 original presentation");
    if (!intent.valid) {
        return;
    }

    M11_GameView_Init(&view);
    expect_true(M11_GameView_OpenSelectedMenuEntry(&view, &menu) == 1,
                "M11 opens Theron through M12 selected-menu entry");
    expect_true(view.startedFromLauncher == 1,
                "M11 marks Theron startup as launcher-started");
    expect_true(view.active == 1,
                "M11 Theron launcher handoff leaves view active");
    expect_true(view.sourceKind == M11_GAME_SOURCE_THERON_TRACK02,
                "M11 Theron launcher handoff claims Track 02 source");
    expect_true(view.theronBootProfile != NULL,
                "M11 Theron launcher handoff exposes boot profile");
    expect_true(view.theronWorld != NULL && view.theronViewport != NULL,
                "M11 Theron launcher handoff builds world and viewport");
    expect_true(view.theronState.level_loaded == 0,
                "M11 Theron launcher handoff waits before dungeon load");
    expect_true(view.theronState.startup_phase ==
                    THERON_STARTUP_PHASE_TITLE,
                "M11 Theron launcher handoff enters bounded title gate");
    expect_true(view.theronState.selected_dungeon == 1,
                "M11 Theron launcher handoff selects chapter 1 first");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 1000,
                "M11 Theron launcher stage select draws a nonblank frame");
    row_count = M11_GameView_GetTheronStartupRenderRows(
        &view, startup_rows, 16);
    expect_true(row_count >= 3 &&
                    startup_rows_contain(startup_rows, row_count,
                                         "Chapter 1: Hall of Records") &&
                    startup_rows_contain(startup_rows, row_count,
                                         "PRESS ENTER TO START"),
                "M11 Theron launcher rows expose title-gate state");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 Theron launcher title accept opens stage select");
    expect_true(view.theronState.startup_phase ==
                    THERON_STARTUP_PHASE_STAGE_SELECT,
                "M11 Theron launcher handoff enters visible stage select");
    row_count = M11_GameView_GetTheronStartupRenderRows(
        &view, startup_rows, 16);
    expect_true(row_count >= 5 &&
                    startup_rows_contain(startup_rows, row_count,
                                         "Chapter 1: Hall of Records") &&
                    startup_rows_contain(startup_rows, row_count,
                                         "CHOOSE A STAGE") &&
                    startup_rows_contain(startup_rows, row_count,
                                         "> 1  Hall of Records"),
                "M11 Theron launcher rows expose stage-selection state");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 Theron launcher stage accept opens Soul Room");
    expect_true(view.theronState.startup_phase ==
                    THERON_STARTUP_PHASE_SOUL_ROOM,
                "M11 Theron launcher handoff enters Soul Room before dungeon");
    row_count = M11_GameView_GetTheronStartupRenderRows(
        &view, startup_rows, 16);
    expect_true(startup_rows_contain(startup_rows, row_count, "SOUL ROOM"),
                "M11 Theron launcher rows expose Soul Room state");
    expect_true(view.theronState.level_loaded == 0,
                "M11 Theron launcher Soul Room still gates dungeon load");

    M11_GameView_Shutdown(&view);
}

int main(void) {
    printf("=== Theron V1 M12/M11 launcher handoff boundary ===\n");

    run_empty_launcher_boundary();
    run_real_launcher_handoff_if_available();

    printf("\nTheron V1 M12/M11 launcher handoff boundary: %d passed, %d failed, %d skipped\n",
           g_passed, g_failures, g_skipped);
    if (g_failures) {
        fprintf(stderr,
                "Theron V1 M12/M11 launcher handoff boundary FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: Theron V1 M12/M11 launcher handoff boundary is wired");
    return 0;
}
