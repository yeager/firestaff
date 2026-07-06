/*
 * test_nexus_v1_m11_launcher_handoff_boundary.c
 *
 * M12 -> M11 Nexus V1 launcher handoff boundary.
 *
 * The direct M11 Nexus startup gate proves M11_GameView_Start(). This
 * companion gate proves the production launcher entry path:
 * M12_StartupMenu_GetLaunchIntent() -> M11_GameView_OpenSelectedMenuEntry().
 * It is skip-clean without user-supplied Saturn data and becomes a real
 * title/startup handoff proof when ~/.firestaff/data/nexus is staged.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "nexus_v1_launcher.h"
#include "nexus_v1_title.h"

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

static int count_diff_pixels(const unsigned char* a,
                             const unsigned char* b,
                             size_t count) {
    size_t i;
    int diff = 0;
    if (!a || !b) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (a[i] != b[i]) {
            ++diff;
        }
    }
    return diff;
}

static void make_empty_data_dir(char out[512]) {
    int rc = snprintf(out, 512,
                      "%s%sfirestaff_nexus_launcher_empty_%ld",
                      (getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp"),
                      TEST_PATH_SEP, (long)TEST_GETPID());
    if (rc > 0 && rc < 512) {
        (void)TEST_MKDIR(out);
    } else {
        out[0] = '\0';
    }
}

static const char* default_nexus_data_dir(char fallback[512]) {
    const char* home = getenv("HOME");
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(fallback, 512, "%s/.firestaff/data/nexus", home);
    return fallback;
}

static void run_empty_launcher_boundary(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    const M12_MenuEntry* entry;
    char empty_dir[512];

    make_empty_data_dir(empty_dir);
    expect_true(empty_dir[0] != '\0',
                "Nexus launcher handoff empty data dir path was created");

    M12_StartupMenu_InitWithDataDir(&menu, empty_dir, "nexus");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 3);
    expect_true(entry != NULL,
                "M12 exposes a Nexus menu entry at game slot 3");
    expect_true(entry && entry->gameId && strcmp(entry->gameId, "nexus") == 0,
                "M12 game slot 3 gameId is \"nexus\"");
    expect_true(entry && entry->available == 0,
                "Nexus entry is unavailable when no Nexus assets are present");

    menu.selectedIndex = 3;
    menu.activatedIndex = 3;
    menu.launchRequested = 1;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.gameId && strcmp(intent.gameId, "nexus") == 0,
                "Nexus launch intent carries gameId=\"nexus\"");
    expect_true(intent.valid == 0,
                "Nexus launch intent is invalid when assets are absent");
}

static void run_nexus_runtime_path_resolution(void) {
    M11_GameLaunchSpec spec;
    char out[512];

    memset(&spec, 0, sizeof(spec));
    spec.gameId = "nexus";
    spec.dataDir = "/firestaff/data";
    spec.verifiedAssetPath = "/firestaff/data/nexus/DM.BIN";
    expect_true(M11_GameView_ResolveNexusRuntimeDataDir(&spec, out, sizeof(out)) == 1,
                "Nexus runtime resolver accepts extracted DM.BIN marker");
    expect_true(strcmp(out, "/firestaff/data/nexus") == 0,
                "Nexus extracted marker resolves to its parent directory");

    spec.verifiedAssetPath = "/firestaff/data/nexus/disc.bin::DM.BIN";
    expect_true(M11_GameView_ResolveNexusRuntimeDataDir(&spec, out, sizeof(out)) == 1,
                "Nexus runtime resolver accepts virtual BIN marker");
    expect_true(strcmp(out, "/firestaff/data/nexus/disc.bin") == 0,
                "Nexus virtual BIN marker resolves to the physical BIN image");

    spec.verifiedAssetPath = "/firestaff/data/nexus/disc.iso::LEV00.DGN";
    expect_true(M11_GameView_ResolveNexusRuntimeDataDir(&spec, out, sizeof(out)) == 1,
                "Nexus runtime resolver accepts virtual ISO marker");
    expect_true(strcmp(out, "/firestaff/data/nexus/disc.iso") == 0,
                "Nexus virtual ISO marker resolves to the physical ISO image");

    spec.verifiedAssetPath = "";
    spec.dungeonPath = "";
    expect_true(M11_GameView_ResolveNexusRuntimeDataDir(&spec, out, sizeof(out)) == 1,
                "Nexus runtime resolver falls back to dataDir");
    expect_true(strcmp(out, "/firestaff/data") == 0,
                "Nexus runtime resolver keeps broad dataDir fallback");
}

static void run_real_launcher_handoff_if_available(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    M11_GameViewState view;
    const M12_MenuEntry* entry;
    char real_dir[512];
    const char* data_dir = default_nexus_data_dir(real_dir);
    unsigned char framebuffer[320 * 200];
    unsigned char framebuffer_later[320 * 200];

    if (!data_dir || !data_dir[0]) {
        expect_skip("HOME is unset; no default Nexus data dir");
        return;
    }

    M12_StartupMenu_InitWithDataDir(&menu, data_dir, "nexus");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 3);
    if (!entry || !entry->available ||
        !M12_AssetStatus_GameAvailable(&menu.assetStatus, "nexus")) {
        expect_skip("no launchable Nexus data under default data dir");
        return;
    }

    menu.selectedIndex = 3;
    menu.activatedIndex = 3;
    menu.launchRequested = 1;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.valid == 1,
                "M12 Nexus launch intent is valid with real staged data");
    expect_true(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL,
                "M12 Nexus launch intent uses V1 original presentation");
    if (!intent.valid) {
        return;
    }

    M11_GameView_Init(&view);
    expect_true(M11_GameView_OpenSelectedMenuEntry(&view, &menu) == 1,
                "M11 opens Nexus through M12 selected-menu entry");
    expect_true(view.startedFromLauncher == 1,
                "M11 marks Nexus startup as launcher-started");
    expect_true(view.active == 1,
                "M11 Nexus launcher handoff leaves view active");
    expect_true(view.sourceKind == M11_GAME_SOURCE_NEXUS_DGN,
                "M11 Nexus launcher handoff claims Nexus DGN source");
    expect_true(view.nexusEngine != NULL,
                "M11 Nexus launcher handoff exposes Nexus engine");
    expect_true(view.nexusState.level_loaded == 1,
                "M11 Nexus launcher handoff loads level zero");
    expect_true(view.nexusState.title_active == 1,
                "M11 Nexus launcher handoff enters title phase");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 500,
                "M11 Nexus launcher title phase draws a nonblank frame");
    for (int t = 0;
         t < 128 &&
             view.nexusState.title_frame <
                 nexus_title_boot_warning_frames() + 16;
         ++t) {
        expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                    "M11 Nexus launcher title idle advances title animation");
    }
    expect_true(view.nexusState.title_frame >=
                    nexus_title_boot_warning_frames() + 16,
                "M11 Nexus launcher title advances frame counter");
    memset(framebuffer_later, 0, sizeof(framebuffer_later));
    M11_GameView_Draw(&view, framebuffer_later, 320, 200);
    expect_true(count_diff_pixels(framebuffer,
                                  framebuffer_later,
                                  sizeof(framebuffer)) > 100,
                "M11 Nexus launcher TITLE.CG reveal changes after warning");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                    M11_GAME_INPUT_IGNORED,
                "M11 Nexus launcher title ignores movement input");
    expect_true(view.nexusState.title_active == 1,
                "M11 Nexus launcher title remains active after movement");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 Nexus launcher title blocks early accept");
    expect_true(view.nexusState.title_active == 1,
                "M11 Nexus launcher title remains active after early accept");
    for (int t = 0; t < 128 &&
                    view.nexusState.title_frame <
                        nexus_title_boot_warning_frames() +
                            nexus_title_min_boot_frames();
         ++t) {
        expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                    "M11 Nexus launcher title completes boot reveal");
    }
    expect_true(view.nexusState.title_frame >=
                    nexus_title_boot_warning_frames() +
                        nexus_title_min_boot_frames(),
                "M11 Nexus launcher title reaches boot reveal frame");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 Nexus launcher title blocks explicit accept during hold");
    expect_true(view.nexusState.title_active == 1,
                "M11 Nexus launcher title remains active during hold");
    for (int t = 0; t < 128 &&
                    view.nexusState.title_frame <
                        nexus_title_boot_start_ready_frames();
         ++t) {
        expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                    "M11 Nexus launcher title completes startup hold");
    }
    expect_true(view.nexusState.title_frame >=
                    nexus_title_boot_start_ready_frames(),
                "M11 Nexus launcher title reaches start-ready frame");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 Nexus launcher title advances on explicit accept after hold");
    expect_true(view.nexusState.title_active == 0,
                "M11 Nexus launcher title clears after accept");
    M11_GameView_Shutdown(&view);
    nexus_v1_launcher_shutdown();
}

int main(void) {
    printf("=== Nexus V1 M12/M11 launcher handoff boundary ===\n");

    run_empty_launcher_boundary();
    run_nexus_runtime_path_resolution();
    run_real_launcher_handoff_if_available();

    printf("\nNexus V1 M12/M11 launcher handoff boundary: %d passed, %d failed, %d skipped\n",
           g_passed, g_failures, g_skipped);
    if (g_failures) {
        fprintf(stderr,
                "Nexus V1 M12/M11 launcher handoff boundary FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: Nexus V1 M12/M11 launcher handoff boundary is wired");
    return 0;
}
