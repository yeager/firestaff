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
#include "theron_v1_boot.h"
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

static void init_menu_without_gallery(M12_StartupMenuState* state,
                                      const char* data_dir,
                                      const char* game_id) {
    M12_StartupMenuInitOptions options;
    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(state, data_dir, game_id, &options);
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

    init_menu_without_gallery(&menu, empty_dir, "theron");
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
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    menu.view = M12_MENU_VIEW_GAME_OPTIONS;
    menu.gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_ACCEPT);
    expect_true(menu.launchRequested == 0,
                "Theron Launch action rejects absent campaign media");
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.gameId && strcmp(intent.gameId, "theron") == 0,
                "Theron launch intent carries gameId=\"theron\"");
    expect_true(intent.valid == 0,
                "Theron launch intent is invalid when assets are absent");
}

static void run_track02_startup_overlay_regression(void) {
    Theron_V1_BootStartupHostRenderReceipt receipt;
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    Theron_Track02StartupLoaderReceipt bad_loader_receipt;
    Theron_Track02StartupLoaderReceipt iso_loader_receipt;

    theron_v1_boot_startup_host_render_receipt_init(&receipt);
    receipt.track02_startup_graphics_executed = 1;
    expect_true(!theron_v1_boot_startup_host_render_plan_fallback_allowed(
                    &receipt),
                "completed Track 02 graphics suppress synthetic startup plan in every phase");
    receipt.track02_startup_graphics_executed = 0;
    expect_true(theron_v1_boot_startup_host_render_plan_fallback_allowed(
                    &receipt),
                "startup plan fallback remains available before Track 02 graphics execute");

    M11_GameView_Init(&view);
    view.active = 1;
    view.sourceKind = M11_GAME_SOURCE_THERON_TRACK02;
    view.theronState.startup_phase = THERON_STARTUP_PHASE_TITLE;
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_RETURN_TO_MENU &&
                    strcmp(view.lastAction, "STARTUP") == 0 &&
                    strcmp(view.lastOutcome, "TRACK02 ATLAS ROUTES INVALID") == 0,
                "M11 returns to launcher when Track02 startup atlas routes are absent");
    M11_GameView_Shutdown(&view);

    memset(&spec, 0, sizeof(spec));
    memset(&bad_loader_receipt, 0, sizeof(bad_loader_receipt));
    bad_loader_receipt.valid = 1;
    spec.gameId = "theron";
    spec.title = "THERON'S QUEST";
    spec.dataDir = ".";
    spec.verifiedAssetPath = "not-a-track02";
    spec.verifiedAssetMd5 = "f23601102138f87c33025877767ebf76";
    spec.theronTrack02LoaderReceipt = &bad_loader_receipt;
    M11_GameView_Init(&view);
    expect_true(!M11_GameView_Start(&view, &spec) &&
                    strcmp(view.lastOutcome,
                           "TRACK02 CUE LOADER RECEIPT INVALID") == 0,
                "M11 rejects an invalid scanner Track02 CUE loader receipt before boot");
    M11_GameView_Shutdown(&view);

    /* MODE1/2048 CUE media has an authenticated payload path but no raw
     * MODE1/2352 IPL receipt. It must get past this receipt gate and fail
     * only at its later verified-media startup stage when the fixture is not
     * a real Track 02 image. */
    memset(&iso_loader_receipt, 0, sizeof(iso_loader_receipt));
    spec.theronTrack02LoaderReceipt = &iso_loader_receipt;
    M11_GameView_Init(&view);
    expect_true(!M11_GameView_Start(&view, &spec) &&
                    strcmp(view.lastOutcome,
                           "TRACK02 CUE LOADER RECEIPT INVALID") != 0,
                "M11 does not misdiagnose valid MODE1/2048 CUE media as invalid raw Track02");
    M11_GameView_Shutdown(&view);
}

static void run_real_launcher_handoff_if_available(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    M11_GameViewState view;
    M11_BootProbeReceipt boot_receipt;
    const M12_MenuEntry* entry;
    const M12_AssetVersionStatus* version;
    const Theron_Track02StartupLoaderReceipt* loader_receipt;
    unsigned int expected_index01_sector;
    int raw_track02;
    char real_dir[512];
    const char* data_dir = default_data_root(real_dir);
    unsigned char framebuffer[320 * 200];
    char startup_rows[16][M11_THERON_STARTUP_RENDER_ROW_CAPACITY];
    int row_count;
    int opened;

    if (!data_dir || !data_dir[0]) {
        expect_skip("HOME is unset; no default Firestaff data root");
        return;
    }

    init_menu_without_gallery(&menu, data_dir, "theron");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 4);
    if (!entry || !entry->available ||
        !M12_AssetStatus_GameAvailable(&menu.assetStatus, "theron")) {
        expect_skip("no launchable Theron Track 02 data under default data root");
        return;
    }

    menu.selectedIndex = 4;
    menu.activatedIndex = 4;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    menu.view = M12_MENU_VIEW_GAME_OPTIONS;
    menu.gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_ACCEPT);
    expect_true(menu.launchRequested == 1,
                "Theron Launch action admits the selected real campaign media");
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    if (!M12_AssetStatus_TheronCampaignMediaLaunchReady(&menu.assetStatus)) {
        expect_true(intent.valid == 0,
                    "path-only Theron availability does not create a launch intent");
        expect_skip("no explicit verified Theron campaign-media scan for staged Track 02");
        return;
    }
    expect_true(intent.valid == 1,
                "M12 Theron launch intent is valid with real staged data");
    expect_true(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL,
                "M12 Theron launch intent uses V1 original presentation");
    if (!intent.valid) {
        return;
    }

    version = M12_AssetStatus_GetFirstMatchedVersion(&menu.assetStatus, "theron");
    loader_receipt = M12_AssetStatus_GetTheronTrack02LoaderReceipt(
        &menu.assetStatus);
    raw_track02 = version &&
        (strcmp(version->matchedMd5, THERON_TRACK02_MD5_JP_BIN) == 0 ||
         strcmp(version->matchedMd5, THERON_TRACK02_MD5_US_BIN) == 0);
    if (raw_track02) {
        expected_index01_sector = version &&
                strcmp(version->matchedMd5, THERON_TRACK02_MD5_JP_BIN) == 0
            ? THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR
            : THERON_TRACK02_IPL_US_INDEX01_RAW_SECTOR;
        expect_true(version && loader_receipt && loader_receipt->valid == 1 &&
                        strcmp(loader_receipt->track02_path, version->matchedPath) == 0 &&
                        loader_receipt->ipl_loader.data_track_index01_raw_sector ==
                            expected_index01_sector,
                    "M12 scanner retains the raw Track 02 pregap through the IPL receipt");
    } else {
        expect_true(version && loader_receipt && loader_receipt->valid == 0 &&
                        (strcmp(version->matchedMd5,
                                THERON_TRACK02_MD5_JP_REV1_ISO) == 0 ||
                         strcmp(version->matchedMd5,
                                THERON_TRACK02_MD5_US_ISO) == 0),
                    "M12 scanner keeps an ISO Track 02 on its non-raw CUE route");
    }

    M11_GameView_Init(&view);
    opened = M11_GameView_OpenSelectedMenuEntry(&view, &menu);
    if (!opened) {
        expect_true(!view.active,
                    "M11 releases rejected Track02 startup before launcher return");
        expect_skip("staged Track02 runtime handoff remains unavailable");
        M11_GameView_Shutdown(&view);
        return;
    }
    expect_true(opened == 1,
                "M11 opens Theron through M12 selected-menu entry");
    expect_true(view.startedFromLauncher == 1,
                "M11 marks Theron startup as launcher-started");
    expect_true(view.active == 1,
                "M11 Theron launcher handoff leaves view active");
    expect_true(view.sourceKind == M11_GAME_SOURCE_THERON_TRACK02,
                "M11 Theron launcher handoff claims Track 02 source");
    expect_true(view.theronBootProfile != NULL,
                "M11 Theron launcher handoff exposes boot profile");
    expect_true(version && view.theronBootProfile &&
                    strcmp(((const Theron_V1_BootProfile *)
                            view.theronBootProfile)->graphics_md5,
                           version->matchedMd5) == 0 &&
                    strcmp(view.theronState.startup_media_track02_md5,
                           version->matchedMd5) == 0,
                "M11 carries the verified Track 02 identity into startup media");
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
    memset(&boot_receipt, 0, sizeof(boot_receipt));
    expect_true(M11_GameView_GetBootProbeReceipt(&view, &boot_receipt) &&
                    boot_receipt.startupTitleFrame == 0 &&
                    boot_receipt.startupTitleFrameMax == 7 &&
                    boot_receipt.startupTitleReady == 0,
                "M11 Theron launcher title starts on animated frame 0");
    row_count = M11_GameView_GetTheronStartupRenderRows(
        &view, startup_rows, 16);
    expect_true(row_count >= 3 &&
                    startup_rows_contain(startup_rows, row_count,
                                         "Chapter 1: AKUTUBA") &&
                    startup_rows_contain(startup_rows, row_count,
                                         "PRESS ENTER TO START"),
                "M11 Theron launcher rows expose title-gate state");
    while (view.theronState.startup_title_animation_tick < 48) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    memset(&boot_receipt, 0, sizeof(boot_receipt));
    expect_true(M11_GameView_GetBootProbeReceipt(&view, &boot_receipt) &&
                    boot_receipt.startupTitleFrame == 7 &&
                    boot_receipt.startupTitleReady == 1,
                "M11 Theron launcher title reaches ready frame before accept");
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
                                         "Chapter 1: AKUTUBA") &&
                    startup_rows_contain(startup_rows, row_count,
                                         "CHOOSE A STAGE") &&
                    startup_rows_contain(startup_rows, row_count,
                                         "> 1  AKUTUBA"),
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

static void run_explicit_real_cue_campaign_if_available(void) {
    const char *cue_path = getenv("FIRESTAFF_THERON_CUE");
    M12_AssetStatus status;
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media;

    if (!cue_path || !cue_path[0]) {
        expect_skip("FIRESTAFF_THERON_CUE is unset");
        return;
    }
    expect_true(M12_AssetStatus_ScanTheronCampaignMedia(
                    &status, cue_path, THERON_TRACK02_MD5_US_BIN, NULL) == 1,
                "explicit authentic Theron CUE enters campaign media admission");
    media = M12_AssetStatus_GetTheronCampaignMedia(&status);
    expect_true(media && media->source == THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CUE &&
                    media->launchable_direct_media && media->direct_media.cue_consumed &&
                    strcmp(media->direct_media.media_path, cue_path) == 0,
                "explicit authentic Theron CUE retains CUE-backed raw Track 02 provenance");
}

int main(void) {
    printf("=== Theron V1 M12/M11 launcher handoff boundary ===\n");

    run_empty_launcher_boundary();
    run_track02_startup_overlay_regression();
    run_explicit_real_cue_campaign_if_available();
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
