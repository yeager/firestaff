/*
 * test_m11_direct_launch_prepare_all_games.c
 *
 * CLI --game direct launch must bypass only the visible M12 menu. It must
 * still prepare a normal selected game entry and enter M11 through
 * M12_StartupMenu_GetLaunchIntent() / M11_GameView_OpenSelectedMenuEntry().
 *
 * This is the testable contract behind main_loop_m11.c directLaunch: no
 * per-game shortcut may skip the startup gates owned by DM1/CSB/DM2/Nexus
 * or Theron's Quest.
 *
 * Skip-clean without user-supplied data. With ~/.firestaff/data staged, each
 * available game gets a real M12 direct-launch preparation and M11 handoff.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "main_loop_m11.h"
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
static void test_setenv(const char* name, const char* value) {
    (void)_putenv_s(name, value ? value : "");
}
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_PATH_SEP "/"
#define TEST_GETPID() getpid()
static void test_setenv(const char* name, const char* value) {
    if (value) {
        (void)setenv(name, value, 1);
    } else {
        (void)unsetenv(name);
    }
}
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

typedef struct {
    const char* gameId;
    int slot;
    M11_GameSourceKind sourceKind;
} DirectLaunchCase;

static const DirectLaunchCase kCases[] = {
    {"dm1", 0, M11_GAME_SOURCE_BUILTIN_CATALOG},
    {"csb", 1, M11_GAME_SOURCE_CSB_BOOT},
    {"dm2", 2, M11_GAME_SOURCE_DM2_BOOT},
    {"nexus", 3, M11_GAME_SOURCE_NEXUS_DGN},
    {"theron", 4, M11_GAME_SOURCE_THERON_TRACK02},
};

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
                      "%s%sfirestaff_direct_launch_empty_%ld",
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

static void run_empty_data_rejection(void) {
    size_t i;
    char empty_dir[512];

    make_empty_data_dir(empty_dir);
    expect_true(empty_dir[0] != '\0',
                "direct launch empty data dir path was created");

    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        M12_StartupMenuState menu;
        const M12_MenuEntry* entry;

        M12_StartupMenu_InitWithDataDir(&menu, empty_dir, kCases[i].gameId);
        dismiss_initial_message(&menu);
        entry = M12_StartupMenu_GetEntry(&menu, kCases[i].slot);

        expect_true(entry && entry->gameId &&
                        strcmp(entry->gameId, kCases[i].gameId) == 0,
                    "direct launch empty-data case selects expected game slot");
        expect_true(entry && entry->available == 0,
                    "direct launch empty-data case has unavailable entry");
        expect_true(M11_PrepareDirectLaunchForGame(&menu, kCases[i].gameId) == 0,
                    "direct launch refuses unavailable game data");
        expect_true(menu.launchRequested == 0,
                    "direct launch unavailable data does not request launch");
        M12_StartupMenu_Destroy(&menu);
    }

    {
        M12_StartupMenuState menu;
        M12_StartupMenu_InitWithDataDir(&menu, empty_dir, NULL);
        dismiss_initial_message(&menu);
        expect_true(M11_PrepareDirectLaunchForGame(&menu, "not-a-game") == 0,
                    "direct launch refuses unknown game id");
        expect_true(M11_PrepareDirectLaunchForGame(&menu, NULL) == 0,
                    "direct launch refuses null game id");
        M12_StartupMenu_Destroy(&menu);
    }
}

static void run_boot_probe_empty_data_rejection(void) {
    M11_PhaseA_Options opts;
    char empty_dir[512];

    make_empty_data_dir(empty_dir);
    expect_true(empty_dir[0] != '\0',
                "boot-probe empty data dir path was created");

    test_setenv("SDL_VIDEODRIVER", "dummy");
    M11_PhaseA_SetDefaultOptions(&opts);
    expect_true(opts.bootProbeFrames == 0,
                "boot-probe default advances zero startup frames");
    opts.bootProbe = 1;
    opts.gameId = "dm1";
    opts.dataDir = empty_dir;
    opts.durationMs = 0;
    opts.bootProbeFrames = 2;
    expect_true(M11_PhaseA_Run(&opts) == 2,
                "boot-probe refuses missing game data without entering the loop");

    M11_PhaseA_SetDefaultOptions(&opts);
    opts.bootProbe = 1;
    opts.dataDir = empty_dir;
    opts.durationMs = 0;
    expect_true(M11_PhaseA_Run(&opts) == 2,
                "boot-probe refuses missing game id before renderer startup");
}

static void run_real_data_handoff_if_available(void) {
    size_t i;
    int available_count = 0;
    char real_dir[512];
    const char* data_dir = default_data_root(real_dir);

    if (!data_dir || !data_dir[0]) {
        expect_skip("HOME is unset; no default Firestaff data root");
        return;
    }

    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        M12_StartupMenuState menu;
        M12_LaunchIntent intent;
        M11_GameViewState view;
        M11_BootProbeReceipt receipt;
        const M12_MenuEntry* entry;

        M12_StartupMenu_InitWithDataDir(&menu, data_dir, kCases[i].gameId);
        dismiss_initial_message(&menu);
        entry = M12_StartupMenu_GetEntry(&menu, kCases[i].slot);
        if (!entry || !entry->available ||
            !M12_AssetStatus_GameAvailable(&menu.assetStatus, kCases[i].gameId)) {
            char msg[96];
            snprintf(msg, sizeof(msg),
                     "no launchable %s data under default data root",
                     kCases[i].gameId);
            expect_skip(msg);
            M12_StartupMenu_Destroy(&menu);
            continue;
        }
        ++available_count;

        expect_true(M11_PrepareDirectLaunchForGame(&menu, kCases[i].gameId) == 1,
                    "direct launch prepares available game data");
        expect_true(menu.selectedIndex == kCases[i].slot &&
                        menu.activatedIndex == kCases[i].slot &&
                        menu.launchRequested == 1,
                    "direct launch prepares selected and activated slot");
        expect_true(menu.quickResumeLaunchRequested == 0,
                    "direct launch clears quick-resume launch request");

        intent = M12_StartupMenu_GetLaunchIntent(&menu);
        expect_true(intent.valid == 1,
                    "direct launch prepared intent is valid");
        expect_true(intent.gameId && strcmp(intent.gameId, kCases[i].gameId) == 0,
                    "direct launch prepared intent keeps game id");

        M11_GameView_Init(&view);
        expect_true(M11_GameView_OpenSelectedMenuEntry(&view, &menu) == 1,
                    "direct launch prepared menu enters M11 selected-entry path");
        expect_true(view.active == 1,
                    "direct launch prepared M11 handoff leaves view active");
        expect_true(view.startedFromLauncher == 1,
                    "direct launch prepared M11 handoff uses launcher contract");
        expect_true(strcmp(view.sourceId, kCases[i].gameId) == 0,
                    "direct launch prepared M11 handoff preserves source id");
        expect_true(view.sourceKind == kCases[i].sourceKind,
                    "direct launch prepared M11 handoff reaches expected source kind");
        if (strcmp(kCases[i].gameId, "dm1") == 0) {
            expect_true(M11_GameView_Dm1StartupIntroBypassed(&view) == 0,
                        "direct --game dm1 does not use the game-view intro bypass");
        }
        expect_true(M11_GameView_GetBootProbeReceipt(&view, &receipt) == 1,
                    "direct launch handoff exports a boot receipt");
        expect_true(receipt.active == 1 &&
                        receipt.sourceKind == kCases[i].sourceKind &&
                        strcmp(receipt.sourceId, kCases[i].gameId) == 0,
                    "direct launch boot receipt keeps source identity");
        expect_true(receipt.startupPhase[0] != '\0',
                    "direct launch boot receipt names startup/runtime phase");

        M11_GameView_Shutdown(&view);
        M12_StartupMenu_Destroy(&menu);

        {
            M11_PhaseA_Options opts;
            char appdata_dir[512];
            M11_PhaseA_SetDefaultOptions(&opts);
            opts.bootProbe = 1;
            opts.bootProbeFrames = 2;
            opts.gameId = kCases[i].gameId;
            opts.dataDir = data_dir;
            opts.durationMs = 0;
            if (strcmp(kCases[i].gameId, "dm2") == 0) {
                opts.script = "enter";
                opts.bootProbeExpectPhase = "dm2-runtime";
            } else if (strcmp(kCases[i].gameId, "csb") == 0) {
                opts.bootProbeFrames = 240;
                opts.script = "enter";
                opts.bootProbeExpectPhase = "csb-runtime";
            } else if (strcmp(kCases[i].gameId, "nexus") == 0) {
                int rc = snprintf(appdata_dir, sizeof(appdata_dir),
                                  "%s%sfirestaff_nexus_boot_probe_appdata_%ld",
                                  (getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp"),
                                  TEST_PATH_SEP, (long)TEST_GETPID());
                if (rc > 0 && rc < (int)sizeof(appdata_dir)) {
                    (void)TEST_MKDIR(appdata_dir);
                    test_setenv("APPDATA", appdata_dir);
                }
                opts.script = "wait120,enter,enter,act";
                opts.bootProbeExpectPhase = "nexus-runtime";
            } else if (strcmp(kCases[i].gameId, "theron") == 0) {
                opts.script = "enter,enter,act";
                opts.bootProbeExpectPhase = "theron-runtime";
            }
            test_setenv("SDL_VIDEODRIVER", "dummy");
            expect_true(M11_PhaseA_Run(&opts) == 0,
                        "boot-probe advances selected-entry startup frames");
            if (strcmp(kCases[i].gameId, "nexus") == 0) {
                test_setenv("APPDATA", NULL);
            }
        }
    }

    if (available_count == 0) {
        expect_skip("no launchable game data under default data root");
    }
}

int main(void) {
    printf("=== M11 direct-launch preparation all-games gate ===\n");

    run_empty_data_rejection();
    run_boot_probe_empty_data_rejection();
    run_real_data_handoff_if_available();

    printf("\nM11 direct-launch preparation all-games gate: %d passed, %d failed, %d skipped\n",
           g_passed, g_failures, g_skipped);
    if (g_failures) {
        fprintf(stderr,
                "M11 direct-launch preparation all-games gate FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: M11 direct-launch preparation keeps all games on selected-entry startup paths");
    return 0;
}
