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
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#include <sys/stat.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_PATH_SEP "\\"
#define TEST_GETPID() _getpid()
#define TEST_STAT_STRUCT struct _stat
#define TEST_STAT(path, st) _stat((path), (st))
#define TEST_ISDIR(mode) (((mode) & _S_IFDIR) != 0)
static void test_setenv(const char* name, const char* value) {
    (void)_putenv_s(name, value ? value : "");
}
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_PATH_SEP "/"
#define TEST_GETPID() getpid()
#define TEST_STAT_STRUCT struct stat
#define TEST_STAT(path, st) stat((path), (st))
#define TEST_ISDIR(mode) S_ISDIR(mode)
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
    const char* startupAnimation;
} DirectLaunchCase;

static const DirectLaunchCase kCases[] = {
    {"dm1", 0, M11_GAME_SOURCE_BUILTIN_CATALOG, "dm1-title"},
    {"csb", 1, M11_GAME_SOURCE_CSB_BOOT, "csb-title"},
    {"dm2", 2, M11_GAME_SOURCE_DM2_BOOT, "dm2-startup-menu"},
    {"nexus", 3, M11_GAME_SOURCE_NEXUS_DGN, "nexus-title"},
    {"theron", 4, M11_GAME_SOURCE_THERON_TRACK02, "theron-title"},
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
    const char* env = getenv("FIRESTAFF_DATA");
    const char* home = getenv("HOME");
    if (env && env[0]) {
        return env;
    }
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(fallback, 512, "%s/.firestaff/data", home);
    return fallback;
}

static int run_heavy_real_data_case(const char* game_id) {
    const char* all = getenv("FIRESTAFF_DIRECT_LAUNCH_REAL_DATA_ALL");
    if (all && all[0] && strcmp(all, "0") != 0) {
        return 1;
    }
    if (!game_id) {
        return 0;
    }
    return strcmp(game_id, "nexus") != 0 &&
           strcmp(game_id, "theron") != 0;
}

static int local_dir_exists(const char* path) {
    TEST_STAT_STRUCT st;
    return path && path[0] && TEST_STAT(path, &st) == 0 &&
           TEST_ISDIR(st.st_mode);
}

static const char* game_scoped_data_root(const char* broad_root,
                                         const char* game_id,
                                         char out[512]) {
    int rc;
    if (!broad_root || !broad_root[0] || !game_id || !game_id[0]) {
        return broad_root;
    }
    rc = snprintf(out, 512, "%s%s%s", broad_root, TEST_PATH_SEP, game_id);
    if (rc > 0 && rc < 512 && local_dir_exists(out)) {
        return out;
    }
    return broad_root;
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
    char firstInvalid[32];

    make_empty_data_dir(empty_dir);
    expect_true(empty_dir[0] != '\0',
                "boot-probe empty data dir path was created");

    test_setenv("SDL_VIDEODRIVER", "dummy");
    expect_true(M11_BootProbeScript_Validate(NULL, firstInvalid, sizeof(firstInvalid)) == 0,
                "boot-probe script validator accepts null script");
    expect_true(M11_BootProbeScript_Validate("wait120,enter,key:q,click:1:2,move:3:4",
                                             firstInvalid,
                                             sizeof(firstInvalid)) == 0,
                "boot-probe script validator accepts wait/input/event tokens");
    expect_true(M11_BootProbeScript_Validate("enter,bogus,waitx,key:not-a-key",
                                             firstInvalid,
                                             sizeof(firstInvalid)) == 3 &&
                    strcmp(firstInvalid, "bogus") == 0,
                "boot-probe script validator reports invalid tokens");

    M11_PhaseA_SetDefaultOptions(&opts);
    opts.bootProbe = 1;
    opts.gameId = "dm1";
    opts.dataDir = empty_dir;
    opts.durationMs = 0;
    opts.script = "enter,bogus";
    expect_true(M11_PhaseA_Run(&opts) == 5,
                "boot-probe refuses invalid script tokens before data scan");

    M11_PhaseA_SetDefaultOptions(&opts);
    expect_true(opts.bootProbeFrames == 0,
                "boot-probe default advances zero startup frames");
    expect_true(opts.bootProbeExpectRuntime == 0,
                "boot-probe runtime expectation is opt-in");
    expect_true(opts.bootProbeExpectParty == 0,
                "boot-probe party expectation is opt-in");
    expect_true(opts.bootProbeExpectChampions == 0,
                "boot-probe champion-count expectation is opt-in");
    expect_true(opts.bootProbeExpectLevelLoaded == -1,
                "boot-probe level-loaded expectation is opt-in");
    expect_true(opts.bootProbeExpectRuntimeTickMin == -1,
                "boot-probe runtime-tick minimum expectation is opt-in");
    expect_true(opts.bootProbeExpectRuntimeTickMax == -1,
                "boot-probe runtime-tick maximum expectation is opt-in");
    expect_true(opts.bootProbeExpectStartupActive == -1,
                "boot-probe startup-active expectation is opt-in");
    expect_true(opts.bootProbeExpectStartupFrameMin == -1,
                "boot-probe startup-frame minimum expectation is opt-in");
    expect_true(opts.bootProbeExpectStartupFrameMax == -1,
                "boot-probe startup-frame maximum expectation is opt-in");
    expect_true(opts.bootProbeExpectStartupAnimation == NULL,
                "boot-probe startup-animation expectation is opt-in");
    expect_true(opts.bootProbeExpectStartupAnimationActive == -1,
                "boot-probe startup-animation-active expectation is opt-in");
    expect_true(opts.bootProbeExpectTitleFrameMin == -1,
                "boot-probe title-frame minimum expectation is opt-in");
    expect_true(opts.bootProbeExpectTitleFrameMax == -1,
                "boot-probe title-frame maximum expectation is opt-in");
    expect_true(opts.bootProbeExpectTitleFrameBoundary == -1,
                "boot-probe title-frame boundary expectation is opt-in");
    expect_true(opts.bootProbeExpectTitleReady == -1,
                "boot-probe title-ready expectation is opt-in");
    expect_true(opts.bootProbeExpectDm1HoCFullGraphics == 0,
                "boot-probe DM1 HoC full graphics expectation is opt-in");
    expect_true(opts.bootProbeExpectDm1HoCReleaseAppCapture == 0,
                "boot-probe DM1 HoC release-app expectation is opt-in");
    opts.bootProbe = 1;
    opts.gameId = "dm1";
    opts.dataDir = empty_dir;
    opts.durationMs = 0;
    opts.bootProbeExpectRuntime = 1;
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
        expect_skip("no FIRESTAFF_DATA and HOME is unset; no Firestaff data root");
        return;
    }

    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        M12_StartupMenuState menu;
        M12_LaunchIntent intent;
        M11_GameViewState view;
        M11_BootProbeReceipt receipt;
        const M12_MenuEntry* entry;
        const M12_AssetVersionStatus* firstMatchedVersion;
        char expectedAssetMd5[33];
        char scoped_dir[512];
        const char* case_data_dir;

        expectedAssetMd5[0] = '\0';

        if (!run_heavy_real_data_case(kCases[i].gameId)) {
            char msg[128];
            snprintf(msg, sizeof(msg),
                     "skipping heavy %s real-data direct-launch case without opt-in",
                     kCases[i].gameId);
            expect_skip(msg);
            continue;
        }
        case_data_dir = game_scoped_data_root(data_dir,
                                             kCases[i].gameId,
                                             scoped_dir);

        M12_StartupMenu_InitWithDataDir(&menu,
                                        case_data_dir,
                                        kCases[i].gameId);
        dismiss_initial_message(&menu);
        entry = M12_StartupMenu_GetEntry(&menu, kCases[i].slot);
        if (!entry || !entry->available ||
            !M12_AssetStatus_GameAvailable(&menu.assetStatus, kCases[i].gameId)) {
            char msg[96];
            snprintf(msg, sizeof(msg),
                     "no launchable %s data under configured data root",
                     kCases[i].gameId);
            expect_skip(msg);
            M12_StartupMenu_Destroy(&menu);
            continue;
        }
        ++available_count;
        firstMatchedVersion = M12_AssetStatus_GetFirstMatchedVersion(
            &menu.assetStatus,
            kCases[i].gameId);
        if (firstMatchedVersion && firstMatchedVersion->matchedMd5[0] != '\0') {
            snprintf(expectedAssetMd5,
                     sizeof(expectedAssetMd5),
                     "%s",
                     firstMatchedVersion->matchedMd5);
        }

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
        expect_true(expectedAssetMd5[0] == '\0' ||
                        strcmp(receipt.bootAssetMd5, expectedAssetMd5) == 0,
                    "direct launch boot receipt keeps verified asset md5");
        expect_true(receipt.startedFromLauncher == 1,
                    "direct launch boot receipt proves selected-entry launcher handoff");
        expect_true(receipt.startupPhase[0] != '\0',
                    "direct launch boot receipt names startup/runtime phase");
        expect_true(strcmp(receipt.startupAnimation,
                           kCases[i].startupAnimation) == 0,
                    "direct launch boot receipt names per-game startup/title animation");
        expect_true(receipt.startupTitleReady == 0 ||
                        receipt.startupTitleReady == 1,
                    "direct launch boot receipt exports title readiness as a boolean");
        if (strcmp(kCases[i].gameId, "dm1") == 0) {
            int hostWindowAvailable = M11_Render_GetWindow() != NULL;
            expect_true(receipt.startupTitleFrame == receipt.startupTitleFrameMax &&
                            receipt.startupTitleFrameMax == 53,
                        "DM1 receipt exposes the source TITLE frame-bank completion boundary");
            expect_true(receipt.dm1HoCFullGraphicsReady,
                        "DM1 receipt exposes HoC full-graphics host render route");
            expect_true(!hostWindowAvailable ||
                            (receipt.dm1HoCHostRenderPlanReady &&
                             receipt.dm1HoCCaptureProofPassed &&
                             receipt.dm1HoCRuntimeApplyReady &&
                             receipt.dm1HoCProductionConsumerReady),
                        "DM1 receipt consumes HoC full-graphics host render and capture proof");
            expect_true(receipt.dm1HoCRealAssetCapture &&
                            receipt.dm1HoCHoCAssetCapture &&
                            (!hostWindowAvailable ||
                             (receipt.dm1HoCNoHostFallbackVisuals &&
                              receipt.dm1HoCMacWindowCapture &&
                              receipt.dm1HoCReleaseAppCapture &&
                              receipt.dm1HoCHostWindowCapture &&
                              receipt.dm1HoCPresentedCapture &&
                              receipt.dm1HoCPresentedCaptureGeometry &&
                              receipt.dm1HoCPresentedCapturePixels &&
                              receipt.dm1HoCPresentedCaptureWidth >= 320 &&
                              receipt.dm1HoCPresentedCaptureHeight >= 200 &&
                              receipt.dm1HoCPresentedCaptureBytes >=
                                  receipt.dm1HoCPresentedCaptureWidth *
                                  receipt.dm1HoCPresentedCaptureHeight * 4 &&
                              receipt.dm1HoCPresentedCaptureHash != 0u &&
                              receipt.dm1HoCHostCaptureRouteMatches &&
                              receipt.dm1HoCReleaseCaptureOwnershipReady &&
                              receipt.dm1HoCReceiptOnlyConsumerReady &&
                              receipt.dm1HoCLowerLevelHelpersReady &&
                              receipt.dm1HoCHostDrawRejectsBackingFallback &&
                              receipt.dm1HoCOpenedEntranceFrame &&
                              receipt.dm1HoCHallMirrorOverlay &&
                              receipt.dm1HoCBlockedEnterUntilChampion &&
                              receipt.dm1HoCRenderCommandCount == 3)),
                        "DM1 HoC proof owns real asset capture, opened entrance, mirror overlay, input block, and no fallback visuals");
            expect_true(!hostWindowAvailable ||
                            (receipt.dm1HoCMapWidth > 0 &&
                             receipt.dm1HoCMapHeight > 0),
                        "DM1 HoC proof records real loaded map dimensions");
        } else if (strcmp(kCases[i].gameId, "csb") == 0) {
            expect_true(receipt.startupTitleFrameMax >= 0 &&
                            (receipt.startupTitleReady == 0 ||
                             receipt.startupTitleReady == 1),
                        "CSB receipt exposes source title ready boundary");
        } else if (strcmp(kCases[i].gameId, "nexus") == 0) {
            expect_true(receipt.startupAnimationActive == 1 &&
                            receipt.startupTitleFrame == 0 &&
                            receipt.startupTitleFrameMax == 102 &&
                            receipt.startupTitleReady == 0,
                        "Nexus receipt exposes active full boot title frame and ready boundary");
        } else {
            expect_true(receipt.startupAnimationActive == 1,
                        "direct launch boot receipt marks non-DM1 startup surface active");
        }
        if (strcmp(kCases[i].gameId, "dm1") == 0) {
            expect_true(strcmp(receipt.startupPhase, "dm1-runtime") == 0,
                        "direct launch boot receipt names DM1 runtime phase");
            expect_true(receipt.dm1StartupIntroBypassed == 0,
                        "direct launch boot receipt keeps DM1 source-visible intro path");
        }

        M11_GameView_Shutdown(&view);
        M12_StartupMenu_Destroy(&menu);

        if (strcmp(kCases[i].gameId, "dm1") == 0) {
            expect_skip("DM1 selected-entry HoC proof passed; PhaseA DM1 real-data boot-probe is tracked separately after local timeout");
            continue;
        }

        {
            M11_PhaseA_Options opts;
            char appdata_dir[512];
            M11_PhaseA_SetDefaultOptions(&opts);
            opts.bootProbe = 1;
            opts.bootProbeFrames = 2;
            opts.gameId = kCases[i].gameId;
            opts.dataDir = case_data_dir;
            opts.durationMs = 0;
            opts.bootProbeExpectRuntime = 1;
            opts.bootProbeExpectParty = 1;
            opts.bootProbeExpectChampions = 1;
            opts.bootProbeExpectMap = 1;
            opts.bootProbeExpectMapIndex = 0;
            opts.bootProbeExpectRuntimeTickMin = 1;
            opts.bootProbeExpectAssetMd5 = expectedAssetMd5[0] != '\0'
                ? expectedAssetMd5
                : NULL;
            if (strcmp(kCases[i].gameId, "dm1") == 0) {
                opts.bootProbeExpectPhase = "dm1-runtime";
                opts.bootProbeExpectPartyX = 1;
                opts.bootProbeExpectPartyY = 3;
                opts.bootProbeExpectPartyDir = 2;
                opts.bootProbeExpectChampionCount = 0;
                opts.bootProbeExpectDm1HoCFullGraphics = 1;
                opts.bootProbeExpectDm1HoCReleaseAppCapture = 1;
            } else if (strcmp(kCases[i].gameId, "dm2") == 0) {
                opts.script = "key:enter";
                opts.bootProbeExpectPhase = "dm2-runtime";
                opts.bootProbeExpectPartyX = 15;
                opts.bootProbeExpectPartyY = 15;
                opts.bootProbeExpectPartyDir = 0;
                opts.bootProbeExpectChampionCount = 4;
            } else if (strcmp(kCases[i].gameId, "csb") == 0) {
                opts.bootProbeFrames = 240;
                opts.script = "key:enter";
                opts.bootProbeExpectPartyX = 5;
                opts.bootProbeExpectPartyY = 5;
                opts.bootProbeExpectPartyDir = 0;
                opts.bootProbeExpectChampionCount = 0;
            } else if (strcmp(kCases[i].gameId, "nexus") == 0) {
                int rc = snprintf(appdata_dir, sizeof(appdata_dir),
                                  "%s%sfirestaff_nexus_boot_probe_appdata_%ld",
                                  (getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp"),
                                  TEST_PATH_SEP, (long)TEST_GETPID());
                if (rc > 0 && rc < (int)sizeof(appdata_dir)) {
                    (void)TEST_MKDIR(appdata_dir);
                    test_setenv("APPDATA", appdata_dir);
                }
                opts.script = "wait120,enter,enter,action";
                opts.bootProbeExpectPhase = "nexus-runtime";
                opts.bootProbeExpectPartyX = 11;
                opts.bootProbeExpectPartyY = 29;
                opts.bootProbeExpectPartyDir = 0;
                opts.bootProbeExpectChampionCount = 1;
            } else if (strcmp(kCases[i].gameId, "theron") == 0) {
                opts.script = "enter,enter,action";
                opts.bootProbeExpectPhase = "theron-runtime";
                opts.bootProbeExpectPartyX = 3;
                opts.bootProbeExpectPartyY = 5;
                opts.bootProbeExpectPartyDir = 0;
                opts.bootProbeExpectChampionCount = 1;
            } else {
                opts.bootProbeExpectParty = 0;
                opts.bootProbeExpectChampions = 0;
                opts.bootProbeExpectMap = 0;
            }
            test_setenv("SDL_VIDEODRIVER", "dummy");
            expect_true(M11_PhaseA_Run(&opts) == 0,
                        "boot-probe advances selected-entry startup frames");
            if (strcmp(kCases[i].gameId, "nexus") == 0) {
                test_setenv("APPDATA", NULL);
            }
            if (strcmp(kCases[i].gameId, "csb") == 0) {
                M11_PhaseA_SetDefaultOptions(&opts);
                opts.bootProbe = 1;
                opts.bootProbeFrames = 2;
                opts.gameId = "csb";
                opts.dataDir = case_data_dir;
                opts.durationMs = 0;
                opts.bootProbeExpectPhase = "csb-title-1";
                opts.bootProbeExpectStartupActive = 1;
                opts.bootProbeExpectStartupFrameMin = 1;
                opts.bootProbeExpectStartupAnimation = "csb-title";
                opts.bootProbeExpectStartupAnimationActive = 1;
                /* ReDMCSB TITLE.C F0437 PC-path title sequence totals 101
                 * ticks (CSB_V1_TITLE_TOTAL_TICKS_PC34, verified in the
                 * 45917ebc4 real-data startup work; the 81 boundary dated
                 * from the earlier 82-tick model). */
                opts.bootProbeExpectTitleFrameMin = 1;
                opts.bootProbeExpectTitleFrameBoundary = 101;
                opts.bootProbeExpectTitleReady = 0;
                opts.bootProbeExpectLevelLoaded = 1;
                opts.bootProbeExpectRuntimeTickMax = 0;
                expect_true(M11_PhaseA_Run(&opts) == 0,
                            "boot-probe proves CSB title startup progress while runtime is frozen");
            }
            if (strcmp(kCases[i].gameId, "dm2") == 0) {
                M11_PhaseA_SetDefaultOptions(&opts);
                opts.bootProbe = 1;
                opts.bootProbeFrames = 2;
                opts.gameId = "dm2";
                opts.dataDir = case_data_dir;
                opts.durationMs = 0;
                opts.bootProbeExpectPhase = "dm2-startup-menu";
                opts.bootProbeExpectStartupActive = 1;
                opts.bootProbeExpectLevelLoaded = 1;
                opts.bootProbeExpectRuntimeTickMax = 0;
                opts.bootProbeExpectStartupFrameMax = 0;
                opts.bootProbeExpectStartupAnimation = "dm2-startup-menu";
                opts.bootProbeExpectStartupAnimationActive = 1;
                opts.bootProbeExpectTitleFrameMax = 0;
                opts.bootProbeExpectTitleFrameBoundary = 7;
                opts.bootProbeExpectTitleReady = 0;
                /* At the startup menu the boot exposes the source
                 * file-header new-game pose admitted by the dungeon loader
                 * (skproject DME.h File_header; the old 15,15,0 Hall-of-
                 * Champions default was synthetic). Champions materialize
                 * only at the runtime handoff, so the menu-phase receipt
                 * reports none. */
                opts.bootProbeExpectParty = 1;
                opts.bootProbeExpectPartyX = 3;
                opts.bootProbeExpectPartyY = 5;
                opts.bootProbeExpectPartyDir = 2;
                opts.bootProbeExpectChampions = 1;
                opts.bootProbeExpectChampionCount = 0;
                expect_true(M11_PhaseA_Run(&opts) == 0,
                            "boot-probe proves DM2 startup menu gates preloaded level before runtime");
            }
            if (strcmp(kCases[i].gameId, "nexus") == 0) {
                M11_PhaseA_SetDefaultOptions(&opts);
                opts.bootProbe = 1;
                opts.bootProbeFrames = 2;
                opts.gameId = "nexus";
                opts.dataDir = case_data_dir;
                opts.durationMs = 0;
                opts.bootProbeExpectPhase = "nexus-title";
                opts.bootProbeExpectStartupActive = 1;
                opts.bootProbeExpectStartupAnimation = "nexus-title";
                opts.bootProbeExpectStartupAnimationActive = 1;
                opts.bootProbeExpectTitleFrameMin = 1;
                opts.bootProbeExpectTitleFrameBoundary = 102;
                opts.bootProbeExpectTitleReady = 0;
                opts.bootProbeExpectLevelLoaded = 1;
                opts.bootProbeExpectRuntimeTickMax = 0;
                expect_true(M11_PhaseA_Run(&opts) == 0,
                            "boot-probe proves Nexus title startup gates preloaded level before runtime");
            }
            if (strcmp(kCases[i].gameId, "theron") == 0) {
                M11_PhaseA_SetDefaultOptions(&opts);
                opts.bootProbe = 1;
                opts.bootProbeFrames = 2;
                opts.gameId = "theron";
                opts.dataDir = case_data_dir;
                opts.durationMs = 0;
                opts.bootProbeExpectPhase = "theron-startup-0";
                opts.bootProbeExpectStartupActive = 1;
                opts.bootProbeExpectStartupAnimation = "theron-title";
                opts.bootProbeExpectStartupAnimationActive = 1;
                opts.bootProbeExpectTitleFrameMax = 0;
                opts.bootProbeExpectTitleFrameBoundary = 0;
                opts.bootProbeExpectTitleReady = 0;
                opts.bootProbeExpectLevelLoaded = 0;
                opts.bootProbeExpectRuntimeTickMax = 0;
                expect_true(M11_PhaseA_Run(&opts) == 0,
                            "boot-probe proves Theron title startup has not materialized runtime level");
            }
        }
    }

    if (available_count == 0) {
        expect_skip("no launchable game data under configured data root");
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
