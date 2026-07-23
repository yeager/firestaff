/*
 * test_csb_v1_m11_launcher_handoff_boundary.c
 *
 * M12 -> M11 CSB V1 normal launcher handoff boundary.
 *
 * CSB has broad direct-start, resume, and utility-import coverage. This
 * focused gate proves the production selected-menu path for a plain CSB
 * start: M12_StartupMenu_GetLaunchIntent() -> M11_GameView_OpenSelectedMenuEntry().
 *
 * Source-lock: ReDMCSB ENTRANCE.C F0806 lines 409-441 and 857-883.
 * A fresh CSB boot must stop at the entrance and block gameplay ticks until
 * the player chooses an entrance command.
 *
 * Skip-clean without user-supplied CSB data. With ~/.firestaff/data staged,
 * this becomes a real launcher-to-entrance proof.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "m11_game_view.h"
#include "csb_v1_boot.h"
#include "csb_v1_startup_real_asset_receipt.h"
#include "entrance_frontend_pc34_compat.h"
#include "firestaff/dm1/v1/box_movement_arrows_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "main_loop_m11.h"
#include "menu_startup_m12.h"
#include "vga_palette_pc34_compat.h"

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

static int count_changed_pixels(const unsigned char* a,
                                const unsigned char* b,
                                size_t count) {
    size_t i;
    int changed = 0;
    if (!a || !b) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (a[i] != b[i]) {
            ++changed;
        }
    }
    return changed;
}

static unsigned int indexed_frame_hash(const unsigned char* pixels,
                                       size_t count) {
    unsigned int hash = 2166136261u;
    size_t i;

    if (!pixels || count == 0u) {
        return 0u;
    }
    for (i = 0u; i < count; ++i) {
        hash ^= pixels[i];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

/* The production loop records the indexed M11 framebuffer only after it has
 * presented. Exercise that same receipt with each real package phase rather
 * than treating a decoded source surface as presentation evidence. */
static void record_presented_real_package_frame(M11_GameViewState* view,
                                                const unsigned char* frame,
                                                const char* message) {
    M11_BootProbeReceipt probe;
    unsigned int expected_hash;
    int special_palette;

    if (!view || !frame) {
        expect_true(0, message);
        return;
    }
    expected_hash = indexed_frame_hash(frame, 320u * 200u);
    special_palette = M11_GameView_GetPresentationSpecialPalette(view);
    if (special_palette >= 0) {
        expect_true(M11_GameView_CSBPresentedFrameMatchesCurrentSource(
                        view, frame, 320, 200, special_palette) == 1,
                    "M11 CSB title/entrance frame matches its current PC34 source plan");
    }
    M11_GameView_RecordCSBPresentedIndexedFrame(view, frame, 320, 200, 0, 0);
    expect_true(M11_GameView_GetBootProbeReceipt(view, &probe) == 1 &&
                    probe.csbPresentedFrameCaptureReady == 1 &&
                    probe.csbPresentedFrameWidth == 320 &&
                    probe.csbPresentedFrameHeight == 200 &&
                    probe.csbPresentedFrameHash == expected_hash,
                message);
}

static int rows_are_color(const unsigned char* pixels,
                          int first_row,
                          int last_row,
                          unsigned char color) {
    int x;
    int y;

    if (!pixels || first_row < 0 || last_row > 200 || first_row >= last_row) {
        return 0;
    }
    for (y = first_row; y < last_row; ++y) {
        for (x = 0; x < 320; ++x) {
            if (pixels[y * 320 + x] != color) {
                return 0;
            }
        }
    }
    return 1;
}

static int frame_rect_matches(const unsigned char* first,
                              const unsigned char* second,
                              int x,
                              int y,
                              int w,
                              int h) {
    int row;

    if (!first || !second || x < 0 || y < 0 || w <= 0 || h <= 0 ||
        x + w > 320 || y + h > 200) {
        return 0;
    }
    for (row = 0; row < h; ++row) {
        if (memcmp(first + (size_t)(y + row) * 320u + (size_t)x,
                   second + (size_t)(y + row) * 320u + (size_t)x,
                   (size_t)w) != 0) {
            return 0;
        }
    }
    return 1;
}

static int frame_rect_is_color(const unsigned char* pixels,
                               int x,
                               int y,
                               int w,
                               int h,
                               unsigned char color) {
    int row;

    if (!pixels || x < 0 || y < 0 || w <= 0 || h <= 0 ||
        x + w > 320 || y + h > 200) {
        return 0;
    }
    for (row = 0; row < h; ++row) {
        int column;
        for (column = 0; column < w; ++column) {
            if (pixels[(size_t)(y + row) * 320u +
                       (size_t)(x + column)] != color) {
                return 0;
            }
        }
    }
    return 1;
}

static int frame_matches_source_rect(const unsigned char* frame,
                                     const CSB_V1_StartupRuntimeSurface_PC34* source,
                                     int source_x,
                                     int source_y,
                                     int dest_x,
                                     int dest_y,
                                     int width,
                                     int height) {
    int row;

    if (!frame || !source || !source->valid || !source->pixels ||
        source_x < 0 || source_y < 0 || dest_x < 0 || dest_y < 0 ||
        width <= 0 || height <= 0 || source_x + width > source->width ||
        source_y + height > source->height || dest_x + width > 320 ||
        dest_y + height > 200) {
        return 0;
    }
    for (row = 0; row < height; ++row) {
        if (memcmp(frame + (size_t)(dest_y + row) * 320u + (size_t)dest_x,
                   source->pixels +
                       (size_t)(source_y + row) * (size_t)source->width +
                       (size_t)source_x,
                   (size_t)width) != 0) {
            return 0;
        }
    }
    return 1;
}

static int title_capture_hash_matches_real_source(
    const CSB_V1_StartupRuntimeAssetSession_PC34* session,
    int title_frame,
    unsigned int source_tick,
    uint32_t expected_hash)
{
    CSB_V1_StartupRuntimeAssetSession_PC34 candidate;
    CSB_V1_StartupRenderState_PC34 state;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 host;
    int matches;

    if (!session || !expected_hash) return 0;
    candidate = *session;
    memset(&state, 0, sizeof(state));
    memset(&plan, 0, sizeof(plan));
    memset(&host, 0, sizeof(host));
    state.entrance_active = 1;
    state.title_active = 1;
    state.title_frame = title_frame;
    if (!csb_v1_startup_source_render_plan_from_state_pc34(&state, &plan) ||
        !csb_v1_boot_startup_title_capture_plan_admit_pc34(
            &plan, title_frame) ||
        !csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
            &candidate, &plan, source_tick, &host)) {
        csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&host);
        return 0;
    }
    matches = host.valid && host.raster.title_composited &&
        host.raster.pixel_hash == expected_hash;
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&host);
    return matches;
}

static void capture_csb_opening_sequence(M11_GameViewState *view) {
    CSB_V1_StartupRuntimeAssetSession_PC34 *session;
    const CSB_V1_StartupRuntimeSurface_PC34 *left;
    const CSB_V1_StartupRuntimeSurface_PC34 *right;
    unsigned char frame[320 * 200];
    unsigned int step_count;
    unsigned int step;
    int tick_before;

    if (!view || !view->csbStartupRuntimeAssetSession) {
        expect_true(0, "M11 CSB opening capture has a source session");
        return;
    }
    session = (CSB_V1_StartupRuntimeAssetSession_PC34 *)
        view->csbStartupRuntimeAssetSession;
    left = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34];
    right = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34];
    step_count = ENTRANCE_Compat_GetDoorAnimationStepCount();
    tick_before = view->csbState.tick_count;

    for (step = 1u; step <= step_count; ++step) {
        const int left_right = 100 - 4 * ((int)step - 1);
        const int right_left = 109 + 4 * ((int)step - 1);
        const int left_w = left_right >= 0 ? left_right + 1 : 0;
        const int right_w = right_left <= 231 ? 232 - right_left : 0;
        const int left_source_x = ((int)step & 0x00fc) << 2;
        const int right_source_x = ((int)step & 0x0003) << 2;

        memset(frame, 0, sizeof(frame));
        M11_GameView_Draw(view, frame, 320, 200);
        expect_true(view->csbState.startup_entrance_active == 1 &&
                        view->csbState.startup_entrance_opening_active == 1 &&
                        view->csbState.startup_entrance_opening_step == (int)step,
                    "M11 CSB opening capture retains the source animation step");
        expect_true(left_w <= 0 ||
                        frame_matches_source_rect(frame, left,
                                                  left_source_x, 0,
                                                  0, 30, left_w, 161),
                    "M11 CSB opening capture consumes the exact C002 strip");
        expect_true(right_w <= 0 ||
                        frame_matches_source_rect(frame, right,
                                                  right_source_x, 0,
                                                  right_left, 30, right_w, 161),
                    "M11 CSB opening capture consumes the exact C003 strip");
        record_presented_real_package_frame(
            view, frame,
            "M11 CSB opening capture records a real C002/C003 package frame");

        if (step < step_count) {
            expect_true(M11_GameView_AdvanceIdleTick(view) ==
                            M11_GAME_INPUT_REDRAW,
                        "M11 CSB opening capture advances to the next source door step");
        }
    }
    {
        const uint32_t saved_expected = view->csbStartupExpectedPackageIdentity;

        view->csbStartupExpectedPackageIdentity = 0u;
        expect_true(M11_GameView_AdvanceIdleTick(view) == M11_GAME_INPUT_IGNORED &&
                        view->csbState.startup_entrance_active == 1 &&
                        view->csbState.startup_entrance_dismissed == 0,
                    "M11 CSB door handoff rejects a missing launch-owned package identity");
        view->csbStartupExpectedPackageIdentity = saved_expected;
    }
    expect_true(M11_GameView_AdvanceIdleTick(view) == M11_GAME_INPUT_REDRAW &&
                    view->csbState.startup_entrance_active == 0 &&
                    view->csbState.startup_entrance_dismissed == 1,
                "M11 CSB opening capture hands C002/C003 directly to the live HUD session");
    expect_true(view->csbState.tick_count == tick_before,
                "M11 CSB opening capture blocks gameplay ticks until F0806 completes");
}

static void drive_csb_entrance_to_wait(M11_GameViewState *view,
                                       const char *message) {
    int guard = 96;
    if (!view) {
        expect_true(0, message);
        return;
    }
    while (guard-- > 0 && view->csbState.startup_entrance_source_step < 4) {
        int tick_before = view->csbState.tick_count;
        expect_true(M11_GameView_AdvanceIdleTick(view) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB launcher title/entrance source prelude redraws");
        expect_true(view->csbState.tick_count == tick_before,
                    "M11 CSB launcher title/entrance source prelude blocks runtime ticks");
    }
    expect_true(view->csbState.startup_entrance_active == 1 &&
                    view->csbState.startup_entrance_source_step == 4,
                message);
}

static void make_empty_data_dir(char out[512]) {
    int rc = snprintf(out, 512,
                      "%s%sfirestaff_csb_launcher_empty_%ld",
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
    struct stat st;
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(fallback, 512, "%s/.firestaff/data/csb", home);
    if (stat(fallback, &st) == 0) {
        return fallback;
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
                "CSB launcher handoff empty data dir path was created");

    init_menu_without_gallery(&menu, empty_dir, "csb");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 1);
    expect_true(entry != NULL,
                "M12 exposes a CSB menu entry at game slot 1");
    expect_true(entry && entry->gameId && strcmp(entry->gameId, "csb") == 0,
                "M12 game slot 1 gameId is \"csb\"");
    expect_true(entry && entry->available == 0,
                "CSB entry is unavailable when required assets are absent");

    menu.selectedIndex = 1;
    menu.activatedIndex = 1;
    menu.launchRequested = 1;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.gameId && strcmp(intent.gameId, "csb") == 0,
                "CSB launch intent carries gameId=\"csb\"");
    expect_true(intent.valid == 0,
                "CSB launch intent is invalid when assets are absent");
}

static void run_real_launcher_handoff_if_available(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    M11_GameViewState view;
    M11_BootProbeReceipt probe;
    CSB_V1_StartupRealReceipt real_package;
    const M12_MenuEntry* entry;
    char real_dir[512];
    const char* data_dir = default_data_root(real_dir);
    unsigned char framebuffer[320 * 200];
    unsigned char title_presents_frame[320 * 200];
    unsigned char title_chaos_frame[320 * 200];
    unsigned char title_strikes_frame[320 * 200];
    unsigned char entrance_closed_frame[320 * 200];
    unsigned char entrance_opening_frame[320 * 200];
    unsigned char first_live_dungeon_frame[320 * 200];
    unsigned char movement_base_frame[320 * 200];
    int tick_before;
    int entrance_frame_before;

    if (!data_dir || !data_dir[0]) {
        expect_skip("HOME is unset; no default Firestaff data root");
        return;
    }

    /* This is an M12 -> M11 integration route, so admit only the same
     * hash-verified PC34 pair consumed by ReDMCSB/CSBWin.  In particular,
     * an otherwise launchable directory must not promote a fallback surface
     * into the title, door, or terminal HUD assertions below. */
    csb_v1_startup_real_receipt_init(&real_package);
    if (csb_v1_startup_real_scan_and_receipt(data_dir, 4, &real_package) !=
            CSB_V1_STARTUP_REAL_OK ||
        !real_package.matched ||
        real_package.variant_id != CSB_V1_VARIANT_PC34_EN ||
        real_package.graphics_kind != CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS ||
        real_package.receipt_hash == 0u) {
        expect_skip("no hash-verified PC34 CSB package under default data root");
        return;
    }

    init_menu_without_gallery(&menu, data_dir, "csb");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 1);
    if (!entry || !entry->available ||
        !M12_AssetStatus_GameAvailable(&menu.assetStatus, "csb")) {
        expect_skip("no launchable CSB data under default data root");
        return;
    }

    menu.selectedIndex = 1;
    menu.activatedIndex = 1;
    menu.launchRequested = 1;
    {
        CSB_V1_CSBWinSaveCorpusDiscoveryReceipt_PC34 discovery;
        memset(&discovery, 0, sizeof(discovery));
        discovery.valid = 1;
        discovery.admitted_count = 1u;
        discovery.candidate.valid = 1;
        discovery.candidate.candidate_hash = 0x9ac34e71u;
        snprintf(discovery.candidate.source_path,
                 sizeof(discovery.candidate.source_path), "%s", "csb-save");
        snprintf(menu.quickResumeGameId, sizeof(menu.quickResumeGameId), "%s", "csb");
        snprintf(menu.quickResumeSavePath, sizeof(menu.quickResumeSavePath), "%s", "csb-save");
        expect_true(M12_StartupMenu_ConsumeCSBSaveCandidateDiscovery(
                        &menu, &discovery) == 1,
                    "M12 accepts the current CSB discovery identity");
        snprintf(discovery.candidate.source_path,
                 sizeof(discovery.candidate.source_path), "%s", "stale-save");
        expect_true(M12_StartupMenu_ConsumeCSBSaveCandidateDiscovery(
                        &menu, &discovery) == 0 &&
                        menu.csbSaveCandidateIdentity == 0u,
                    "M12 rejects stale CSB discovery identity");
        expect_true(M12_StartupMenu_ConsumeCSBSaveCandidateDiscovery(
                        &menu, NULL) == 0 && menu.csbSaveCandidateIdentity == 0u,
                    "M12 rejects missing CSB discovery identity");
        snprintf(menu.quickResumeGameId, sizeof(menu.quickResumeGameId), "%s", "dm1");
        expect_true(M12_StartupMenu_ConsumeCSBSaveCandidateDiscovery(
                        &menu, &discovery) == 0 &&
                        menu.csbSaveCandidateIdentity == 0u,
                    "M12 rejects non-CSB discovery identity route");
        menu.quickResumeGameId[0] = '\0';
        menu.quickResumeSavePath[0] = '\0';
    }
    M12_StartupMenu_BindCSBSaveCandidateIdentity(&menu, 0x9ac34e71u);
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.valid == 1,
                "M12 CSB launch intent is valid with real staged data");
    expect_true(intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL,
                "M12 CSB launch intent uses V1 original presentation");
    if (!intent.valid) {
        return;
    }

    M11_GameView_Init(&view);
    expect_true(M11_GameView_OpenSelectedMenuEntry(&view, &menu) == 1,
                "M11 opens CSB through M12 selected-menu entry");
    expect_true(view.startedFromLauncher == 1,
                "M11 marks CSB startup as launcher-started");
    expect_true(view.active == 1,
                "M11 CSB launcher handoff leaves view active");
    expect_true(view.sourceKind == M11_GAME_SOURCE_CSB_BOOT,
                "M11 CSB launcher handoff claims CSB boot source");
    expect_true(view.csbBootProfile != NULL,
                "M11 CSB launcher handoff owns a CSB boot profile");
    expect_true(view.csbStartupRuntimeAssetSession != NULL &&
                    ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                         view.csbStartupRuntimeAssetSession)->valid &&
                    ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                         view.csbStartupRuntimeAssetSession)
                        ->rejects_legacy_wrappers,
                "M11 CSB launcher handoff owns source-session startup surfaces");
    expect_true(((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                     view.csbStartupRuntimeAssetSession)
                    ->csbSaveCandidateIdentity == 0x9ac34e71u,
                "M11 CSB launcher preserves the M12 CSB candidate identity in its session");
    expect_true(((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                     view.csbStartupRuntimeAssetSession)->real_asset_matched &&
                    real_package.assets_verified &&
                    real_package.graphics_verified &&
                    real_package.dungeon_verified,
                "M11 CSB launcher session is bound to the hash-verified PC34 package");
    expect_true(((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                     view.csbStartupRuntimeAssetSession)->csbStartupPackageIdentity != 0u,
                "M11 CSB ordinary hash-verified boot publishes a startup package identity");
    expect_true(view.csbStartupExpectedPackageIdentity ==
                    ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                         view.csbStartupRuntimeAssetSession)
                        ->csbStartupPackageIdentity,
                "M11 CSB title session retains the launch-owned package identity");
    expect_true(view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_entrance_dismissed == 0,
                "M11 CSB launcher handoff stops at startup title/entrance");
    expect_true(view.csbState.startup_title_active == 1 &&
                    view.csbState.startup_title_source_step == 1 &&
                    view.csbState.startup_entrance_source_step == 0,
                "M11 CSB launcher handoff starts at the source title prelude");
    expect_true(view.csbState.startup_entrance_last_command ==
                    ENTRANCE_COMPAT_RUNTIME_COMMAND_NONE,
                "M11 CSB launcher handoff has no recycled entrance command");
    expect_true(view.csbState.startup_import_available == 0,
                "M11 CSB normal launcher handoff has no import panel armed");
    expect_true(M11_GameView_GetBootProbeReceipt(&view, &probe) == 1,
                "M11 CSB startup exposes boot probe receipt at title prelude");
    expect_true(probe.startupActive == 1 &&
                    probe.startupInputReady == 0 &&
                    probe.startupHudMenuReady == 0,
                "M11 CSB title prelude blocks startup input/HUD through receipt");
    expect_true(probe.startupTitleReady == 0 &&
                    probe.startupHudMenuKind ==
                        CSB_V1_BOOT_STARTUP_HUD_MENU_NONE_PC34,
                "M11 CSB title prelude receipt has no HUD menu route");

    {
        CSB_V1_StartupRuntimeAssetSession_PC34 *session =
            (CSB_V1_StartupRuntimeAssetSession_PC34 *)
                view.csbStartupRuntimeAssetSession;
        const uint32_t saved_expected = view.csbStartupExpectedPackageIdentity;
        const uint32_t saved_current = session->csbStartupPackageIdentity;

        view.csbStartupExpectedPackageIdentity = 0u;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0 &&
                        M11_GameView_GetPresentationSpecialPalette(&view) < 0,
                    "M11 CSB title rejects a missing launch-owned package identity");
        view.csbStartupExpectedPackageIdentity = saved_expected ^ 1u;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0,
                    "M11 CSB title rejects a stale launch-owned package identity");
        view.csbStartupExpectedPackageIdentity = saved_expected;
        session->csbStartupPackageIdentity = saved_current ^ 1u;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0,
                    "M11 CSB title rejects a drifted current package identity");
        session->csbStartupPackageIdentity = saved_current;
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 0,
                "M11 CSB launcher title prelude draws a visible first frame");
    expect_true(M11_GameView_GetPresentationSpecialPalette(&view) ==
                    VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS,
                "M11 CSB launcher PRESENTS frame keeps C001 special palette");
    memcpy(title_presents_frame, framebuffer, sizeof(title_presents_frame));
    record_presented_real_package_frame(
        &view, title_presents_frame,
        "M11 CSB launcher records the presented C001 PRESENTS package frame");
    expect_true(view.csbStartupReleaseAppCaptureReceipt.valid &&
                    view.csbStartupReleaseLifecycleReceipt.valid &&
                    view.csbStartupReleaseLifecycleReceipt.session_generation ==
                        ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                             view.csbStartupRuntimeAssetSession)->generation,
                "M11 CSB title delivery owns the boot release capture receipt");
    {
        const CSB_V1_StartupRuntimeAssetSession_PC34 *session =
            (const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                view.csbStartupRuntimeAssetSession;
        uint32_t *hashes =
            view.csbStartupReleaseAppCaptureReceipt.title_runtime_phase_hashes;
        uint32_t saved_presents_hash = hashes[0];

        expect_true(title_capture_hash_matches_real_source(
                        session, 0, 1u, hashes[0]) &&
                        title_capture_hash_matches_real_source(
                        session, 60, 2u, hashes[1]) &&
                        title_capture_hash_matches_real_source(
                        session, 79, 3u, hashes[2]) &&
                        title_capture_hash_matches_real_source(
                        session, 100, 4u, hashes[3]),
                    "M11 title lifecycle captures exact real C001 PRESENTS/CHAOS/STRIKES rasters");
        hashes[0] ^= 1u;
        expect_true(!title_capture_hash_matches_real_source(
                        session, 0, 1u, hashes[0]),
                    "M11 title lifecycle rejects a legacy wrapper-derived PRESENTS hash");
        hashes[0] = saved_presents_hash;
    }
    {
        const uint32_t saved_release_hash =
            view.csbStartupReleaseLifecycleReceipt.release_capture_hash;

        view.csbStartupReleaseLifecycleReceipt.release_capture_hash ^= 1u;
        expect_true(M11_GameView_CSBPresentedFrameMatchesCurrentSource(
                        &view, title_presents_frame, 320, 200,
                        M11_GameView_GetPresentationSpecialPalette(&view)) == 0,
                    "M11 CSB rejects a title capture with stale release receipt identity");
        view.csbStartupReleaseLifecycleReceipt.release_capture_hash =
            saved_release_hash;
    }

    tick_before = view.csbState.tick_count;
    entrance_frame_before = view.csbState.startup_entrance_frame;
    expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                "M11 CSB launcher entrance idle redraws the startup screen");
    expect_true(view.csbState.tick_count == tick_before,
                "M11 CSB launcher entrance blocks gameplay tick aging");
    expect_true(view.csbState.startup_entrance_frame ==
                    entrance_frame_before + 1,
                "M11 CSB launcher entrance advances only entrance frame time");
    expect_true(view.csbState.startup_title_active == 1 &&
                    view.csbState.startup_title_source_step == 1 &&
                    view.csbState.startup_entrance_source_step == 0,
                "M11 CSB launcher title prelude starts on PRESENTS before entrance");
    for (int i = 0;
         i < csb_v1_startup_title_presents_ticks_pc34() - 1 &&
         view.csbState.startup_title_active;
         ++i) {
        int tick_before_loop = view.csbState.tick_count;
        expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB launcher title prelude holds PRESENTS before zoom");
        expect_true(view.csbState.tick_count == tick_before_loop,
                    "M11 CSB launcher title prelude blocks runtime ticks");
    }
    expect_true(view.csbState.startup_title_active == 1 &&
                    view.csbState.startup_title_source_step == 2 &&
                    view.csbState.startup_entrance_source_step == 0,
                "M11 CSB launcher title prelude reaches CHAOS zoom before entrance");
    memset(title_chaos_frame, 0, sizeof(title_chaos_frame));
    M11_GameView_Draw(&view, title_chaos_frame, 320, 200);
    expect_true(count_nonzero_pixels(title_chaos_frame,
                                     sizeof(title_chaos_frame)) > 0,
                "M11 CSB launcher CHAOS title phase draws visible pixels");
    expect_true(M11_GameView_GetPresentationSpecialPalette(&view) ==
                    VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS,
                "M11 CSB launcher CHAOS frame switches to C001 title palette");
    expect_true(count_changed_pixels(title_presents_frame,
                                     title_chaos_frame,
                                     sizeof(title_chaos_frame)) > 64,
                "M11 CSB title phases are visually distinct");
    record_presented_real_package_frame(
        &view, title_chaos_frame,
        "M11 CSB launcher records the presented C001 CHAOS package frame");
    for (int i = 0;
         i < csb_v1_startup_title_total_ticks_pc34() &&
         view.csbState.startup_title_active &&
         csb_v1_startup_title_stage_for_frame_pc34(
             view.csbState.startup_title_frame) !=
             CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34;
         ++i) {
        expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB launcher title prelude advances CHAOS before STRIKES BACK");
    }
    expect_true(view.csbState.startup_title_active == 1 &&
                    csb_v1_startup_title_stage_for_frame_pc34(
                        view.csbState.startup_title_frame) ==
                        CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 &&
                    view.csbState.startup_title_source_step == 22,
                "M11 CSB launcher title prelude reaches the source STRIKES BACK phase");
    memset(title_strikes_frame, 0, sizeof(title_strikes_frame));
    M11_GameView_Draw(&view, title_strikes_frame, 320, 200);
    {
        const CSB_V1_StartupRuntimeSurface_PC34 *strikes =
            &((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                  view.csbStartupRuntimeAssetSession)
                 ->surfaces.surfaces[
                     CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34];
        int row_matches = strikes->valid && strikes->width == 320 &&
            strikes->height == 57 && strikes->transparent_color == 0;
        int row;

        for (row = 0; row_matches && row < strikes->height; ++row) {
            if (memcmp(title_strikes_frame + (size_t)(118 + row) * 320u,
                       strikes->pixels + (size_t)row * strikes->width,
                       (size_t)strikes->width) != 0) {
                row_matches = 0;
            }
        }
        expect_true(row_matches,
                    "M11 CSB launcher STRIKES BACK frame consumes C001 source bytes at C426 geometry");
    }
    expect_true(M11_GameView_GetPresentationSpecialPalette(&view) ==
                    VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES,
                "M11 CSB launcher STRIKES BACK frame keeps the C426 special palette");
    expect_true(count_changed_pixels(title_chaos_frame,
                                     title_strikes_frame,
                                     sizeof(title_strikes_frame)) > 64,
                "M11 CSB CHAOS and STRIKES BACK title captures are visually distinct");
    record_presented_real_package_frame(
        &view, title_strikes_frame,
        "M11 CSB launcher records the presented C001 STRIKES BACK package frame");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_IGNORED,
                "M11 CSB launcher title/entrance ignores Enter before source wait loop");
    drive_csb_entrance_to_wait(
        &view,
        "M11 CSB launcher entrance reaches source wait loop before commands");
    expect_true(view.csbState.startup_title_active == 0 &&
                    view.csbState.startup_title_source_step == 0,
                "M11 CSB launcher title prelude completed before entrance input");
    expect_true(M11_GameView_GetBootProbeReceipt(&view, &probe) == 1,
                "M11 CSB startup exposes boot probe receipt at entrance wait loop");
    expect_true(probe.startupActive == 1 &&
                    probe.startupInputReady == 1 &&
                    probe.startupHudMenuReady == 1,
                "M11 CSB entrance wait loop is input/HUD-ready through receipt");
    expect_true(probe.startupHudMenuKind ==
                    CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34,
                "M11 CSB entrance wait loop consumes entrance HUD draw receipt");
    expect_true(probe.startupHudMenuOptionCount >= 3 &&
                    probe.startupSelectedCommandId ==
                        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34,
                "M11 CSB entrance wait loop receipt owns menu options and selection");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(view.csbStartupF0128EntranceBound &&
                    view.csbStartupF0128EntranceSourceTick ==
                        (uint32_t)view.csbState.startup_entrance_frame &&
                    view.csbStartupF0128EntranceSessionGeneration ==
                        ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                             view.csbStartupRuntimeAssetSession)->generation &&
                    view.csbStartupF0128EntranceMaterialReceipt.valid &&
                    view.csbStartupF0128EntranceRasterReceipt.valid,
                "M11 CSB retains its self-owned F0128 receipt before C002/C003 composition");
    expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 1000,
                "M11 CSB launcher entrance draws a nonblank wait-loop frame");
    memcpy(entrance_closed_frame, framebuffer, sizeof(entrance_closed_frame));
    record_presented_real_package_frame(
        &view, entrance_closed_frame,
        "M11 CSB launcher records the presented closed-door package frame");

    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 CSB launcher entrance accepts explicit Enter command");
    for (int i = 0;
         i < csb_v1_startup_entrance_pre_open_delay_ticks_pc34() + 1 &&
         view.csbState.startup_entrance_active &&
         view.csbState.startup_entrance_opening_delay_ticks > 0;
         ++i) {
        expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB launcher entrance advances to visible door opening");
    }
    expect_true(view.csbState.startup_entrance_opening_delay_ticks == 0,
                "M11 CSB launcher entrance leaves pre-open delay");
    expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_opening_step == 1,
                "M11 CSB launcher publishes the first C002/C003 door step after the source delay");
    memset(entrance_opening_frame, 0, sizeof(entrance_opening_frame));
    M11_GameView_Draw(&view, entrance_opening_frame, 320, 200);
    expect_true(count_changed_pixels(entrance_closed_frame,
                                     entrance_opening_frame,
                                     sizeof(entrance_opening_frame)) > 64,
                "M11 CSB entrance opening visibly changes the door frame");
    record_presented_real_package_frame(
        &view, entrance_opening_frame,
        "M11 CSB launcher records the presented opening-door package frame");
    {
        const CSB_V1_StartupRuntimeAssetSession_PC34 *session =
            (const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                view.csbStartupRuntimeAssetSession;
        const CSB_V1_StartupRuntimeSurface_PC34 *left =
            &session->surfaces.surfaces[
                CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34];
        const CSB_V1_StartupRuntimeSurface_PC34 *right =
            &session->surfaces.surfaces[
                CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34];

        /* ReDMCSB ENTRANCE.C F0438/F0807 lines 142-304: the first
         * visible PC34 step draws C002[0..100] and C003[4..126] at y=30.
         * CSBWin Graphics.cpp ReadGraphic is the matching archive-read
         * boundary for those two resident source surfaces. */
        expect_true(view.csbState.startup_entrance_opening_step == 1,
                    "M11 CSB launcher captures the first visible source door step");
        expect_true(left->valid && left->source_asset_id == 2 &&
                        left->width >= 101 && left->height >= 161 &&
                        frame_matches_source_rect(entrance_opening_frame,
                                                  left, 0, 0, 0, 30, 101,
                                                  161),
                    "M11 CSB opening capture consumes C002 first-step bytes at PC34 geometry");
        expect_true(right->valid && right->source_asset_id == 3 &&
                        right->width >= 127 && right->height >= 161 &&
                        frame_matches_source_rect(entrance_opening_frame,
                                                  right, 4, 0, 109, 30, 123,
                                                  161),
                    "M11 CSB opening capture consumes C003 first-step bytes at PC34 geometry");
    }
    {
        CSB_V1_StartupRuntimeAssetSession_PC34 *session =
            (CSB_V1_StartupRuntimeAssetSession_PC34 *)
                view.csbStartupRuntimeAssetSession;
        CSB_V1_StartupGraphicDecodeReceipt_PC34 saved_receipt;

        saved_receipt = session->surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].decode_receipt;
        session->surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34]
            .decode_receipt.ended_at_record_boundary = 0;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0,
                    "M11 CSB opening handoff rejects a stale C002 package plan");
        session->surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34]
            .decode_receipt = saved_receipt;

        saved_receipt = session->surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].decode_receipt;
        session->surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34]
            .decode_receipt.ended_at_record_boundary = 0;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0,
                    "M11 CSB opening handoff rejects a stale C003 package plan");
        session->surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34]
            .decode_receipt = saved_receipt;
    }
    capture_csb_opening_sequence(&view);
    expect_true(view.csbState.startup_entrance_last_command ==
                    ENTRANCE_COMPAT_RUNTIME_COMMAND_ENTER_DUNGEON,
                "M11 CSB launcher handoff records source enter-dungeon command");
    expect_true(((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                     view.csbStartupRuntimeAssetSession)->playback.stage ==
                    CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34,
                "M11 CSB launcher door completion releases the same source session to HUD");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(view.csbState.level_loaded == 1 &&
                    view.csbState.current_level >= 0,
                "M11 CSB post-entrance handoff retains the loaded source dungeon");
    expect_true(view.csbState.runtime_viewport_source_session_ready == 1 &&
                    view.csbState.runtime_viewport_source_session_generation ==
                        ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                             view.csbStartupRuntimeAssetSession)->generation &&
                    view.csbState.runtime_viewport_pixel_hash != 0u &&
                    view.csbState.runtime_viewport_draw_counts_hash != 0u,
                "M11 CSB first F0128 dungeon frame consumes the terminal PC3.4 session receipt");
    {
        const uint32_t saved_expected = view.csbStartupExpectedPackageIdentity;

        view.csbStartupExpectedPackageIdentity = 0u;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0,
                    "M11 CSB live HUD rejects a missing launch-owned package identity");
        view.csbStartupExpectedPackageIdentity = saved_expected ^ 1u;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0,
                    "M11 CSB live HUD rejects a stale launch-owned package identity");
        view.csbStartupExpectedPackageIdentity = saved_expected;
    }
    {
        const CSB_V1_StartupRuntimeAssetSession_PC34 *session =
            (const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                view.csbStartupRuntimeAssetSession;
        const CSB_V1_BootProfile *profile =
            (const CSB_V1_BootProfile *)view.csbBootProfile;
        CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 dsa_receipt;
        CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 timer_outcome;

        /* Receipt-only lifecycle coverage: M11 names the existing source
         * session and Dungeon.dat, but never provides DSA program bytes or
         * invokes a DSA action. The admission test owns byte structure. */
        memset(&dsa_receipt, 0, sizeof(dsa_receipt));
        dsa_receipt.valid = 1;
        dsa_receipt.source_admission_consumed = 1;
        dsa_receipt.save_handoff_consumed = 1;
        dsa_receipt.dungeon_identity_current = 1;
        dsa_receipt.startup_session_consumed = 1;
        dsa_receipt.runtime_chain_consumed = 1;
        dsa_receipt.save_fnv1a = 0x41u;
        dsa_receipt.gameblock_fnv1a = 0x42u;
        dsa_receipt.source_admission_hash = 0x43u;
        dsa_receipt.startup_session_generation = session->generation;
        dsa_receipt.startup_source_tick = session->source_tick;
        dsa_receipt.runtime_game_time = profile->runtime.game_time;
        dsa_receipt.imported_action_count = 1;
        dsa_receipt.handoff_hash = 0x44u;
        snprintf(dsa_receipt.dungeon_md5, sizeof(dsa_receipt.dungeon_md5),
                 "%s", profile->dungeon_md5);
        expect_true(M11_GameView_BindCSBDSASaveRuntimeHandoff(
                        &view, &dsa_receipt) == 1,
                    "M11 CSB admits a complete receipt-bound DSA runtime route");
        memset(&timer_outcome, 0, sizeof(timer_outcome));
        timer_outcome.valid = 1;
        timer_outcome.handoff_consumed = 1;
        timer_outcome.session_current = 1;
        timer_outcome.save_identity_current = 1;
        timer_outcome.tick_order_current = 1;
        timer_outcome.timer_record_consumed = 1;
        timer_outcome.local_state_consumed = 1;
        timer_outcome.opcode_body_admitted = 1;
        timer_outcome.action_executed = 1;
        timer_outcome.save_fnv1a = dsa_receipt.save_fnv1a;
        timer_outcome.startup_session_generation = session->generation;
        timer_outcome.startup_source_tick = session->source_tick;
        timer_outcome.runtime_game_time = profile->runtime.game_time;
        timer_outcome.bridge_hash = 1u;
        expect_true(!M11_GameView_CommitCSBDSARestoredTimerOutcome(
                        &view, &timer_outcome) &&
                        !view.csbDsaRestoredTimerTransactionRequired,
                    "M11 CSB rejects an unbacked timer outcome without mutating its route");
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(view.csbState.runtime_viewport_source_session_ready == 1,
                    "M11 CSB preserves the live viewport for a current DSA receipt");
        view.csbDsaSaveRuntimeReceipt.startup_source_tick ^= 1u;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(view.csbState.runtime_viewport_source_session_ready == 0 &&
                        count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0,
                    "M11 CSB rejects a stale DSA receipt before viewport delivery");
        view.csbDsaSaveRuntimeReceipt.startup_source_tick = session->source_tick;
        snprintf(dsa_receipt.dungeon_md5, sizeof(dsa_receipt.dungeon_md5),
                 "%s", "00000000000000000000000000000000");
        expect_true(M11_GameView_BindCSBDSASaveRuntimeHandoff(
                        &view, &dsa_receipt) == 0,
                    "M11 CSB rejects a mixed DSA/Dungeon receipt atomically");
        dsa_receipt.startup_source_tick = session->source_tick;
        snprintf(dsa_receipt.dungeon_md5, sizeof(dsa_receipt.dungeon_md5),
                 "%s", profile->dungeon_md5);
        expect_true(M11_GameView_BindCSBDSASaveRuntimeHandoff(
                        &view, &dsa_receipt) == 1,
                    "M11 CSB accepts the restored current DSA receipt");
        expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                    "M11 CSB advances the source runtime through the boot tick boundary");
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(view.csbState.runtime_viewport_source_session_ready == 0 &&
                        count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0,
                    "M11 CSB rejects a pre-tick DSA receipt at the live runtime epoch");
        dsa_receipt.runtime_game_time = profile->runtime.game_time;
        expect_true(M11_GameView_BindCSBDSASaveRuntimeHandoff(
                        &view, &dsa_receipt) == 1,
                    "M11 CSB accepts a receipt rebound at the live runtime epoch");
        /* This launcher fixture continues through non-DSA HUD interactions.
         * End the opt-in DSA route here so those existing interactions retain
         * their normal source-session contract. */
        memset(&view.csbDsaSaveRuntimeReceipt, 0,
               sizeof(view.csbDsaSaveRuntimeReceipt));
        view.csbDsaSaveRuntimeRouteRequired = 0;
    }
    /* F0806 releases C004/C002/C003 before F0128 begins the first live
     * dungeon pass. A caller-provided stale page must not survive above the
     * viewport after this source-owned transition. */
    memcpy(first_live_dungeon_frame, framebuffer, sizeof(first_live_dungeon_frame));
    memset(framebuffer, 0xff, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(rows_are_color(framebuffer, 0, 30, 0u),
                "M11 CSB first live dungeon frame clears released C004 before F0128");
    expect_true(memcmp(first_live_dungeon_frame, framebuffer,
                       sizeof(first_live_dungeon_frame)) == 0,
                "M11 CSB first live dungeon frame is independent of released C004 host pixels");
    {
        CSB_V1_StartupRuntimeAssetSession_PC34 *session =
            (CSB_V1_StartupRuntimeAssetSession_PC34 *)
                view.csbStartupRuntimeAssetSession;
        int saved_surface_set_valid = session->surfaces.valid;

        /* The first F0128 page must consume the completed CSB session host
         * receipt, not a stale M11 C017/C040 or C004 surface. */
        session->surfaces.valid = 0;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0,
                    "M11 CSB rejects first live dungeon draw without the completed source session receipt");
        session->surfaces.valid = saved_surface_set_valid;
    }
    M11_MessageLog_Push(&view.messageLog, "M11 HOST TELEMETRY", 15);
    memset(framebuffer, 0xff, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(rows_are_color(framebuffer, 173, 200, 0u),
                "M11 CSB clears C015 but never renders generic host telemetry as CSB text");
    {
        DM1_V1_MovementArrowRectPc34 arrows;
        view.v1MovementArrowVisualTicks = 0;
        view.v1MovementArrowVisualMask = 0;
        memset(movement_base_frame, 0xff, sizeof(movement_base_frame));
        M11_GameView_Draw(&view, movement_base_frame, 320, 200);
        expect_true(dm1_v1_movement_arrows_outer_rect_pc34(&arrows) == 1,
                    "M11 CSB resolves the source C013 movement-arrow box");
        view.v1MovementArrowVisualTicks =
            DM1_V1_MOVEMENT_ARROW_VIS_TICKS_PC34;
        view.v1MovementArrowVisualMask =
            DM1_V1_MOVEMENT_ARROW_VIS_TURN_LEFT_PC34;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(frame_rect_matches(movement_base_frame, framebuffer,
                                       arrows.x, arrows.y,
                                       arrows.w, arrows.h),
                    "M11 CSB preserves source C013 without generic keyboard hatch overlay");
        view.v1MovementArrowVisualTicks = 0;
        view.v1MovementArrowVisualMask = 0;
    }
    {
        M11_AssetSlot *c028 = (M11_AssetSlot *)M11_AssetLoader_Load(
            &view.assetLoader, 28u);
        DM1_V1_LayoutZoneRectPc34 iconRect;
        struct ChampionState_Compat savedChampion = view.world.party.champions[0];
        int savedChampionCount = view.world.party.championCount;
        int savedPartyDirection = view.world.party.direction;
        unsigned short savedWidth = c028 ? c028->width : 0u;

        expect_true(c028 && c028->pixels && c028->loaded &&
                        c028->width == 76u && c028->height == 14u,
                    "M11 CSB exposes the authentic C028 icon strip");
        expect_true(dm1_v1_champion_icon_rect_pc34(0, &iconRect) == 1,
                    "M11 CSB resolves C028 champion icon geometry");
        view.world.party.championCount = 1;
        view.world.party.direction = 0;
        memset(&view.world.party.champions[0], 0,
               sizeof(view.world.party.champions[0]));
        view.world.party.champions[0].present = 1;
        view.world.party.champions[0].direction = 0;
        if (c028) {
            c028->width = 0u;
        }
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(frame_rect_is_color(framebuffer,
                                        iconRect.x, iconRect.y,
                                        iconRect.w, iconRect.h, 0u),
                    "M11 CSB clears C113 rather than rendering a C028-free host icon");
        if (c028) {
            c028->width = savedWidth;
        }
        view.world.party.champions[0] = savedChampion;
        view.world.party.championCount = savedChampionCount;
        view.world.party.direction = savedPartyDirection;
    }
    {
        CSB_V1_BootProfile *boot =
            (CSB_V1_BootProfile *)view.csbBootProfile;
        DM1_V1_LayoutZoneRectPc34 iconRect;
        DM1_V1_ChampionStatusRectPc34 statusRect;
        int saved_party_state_valid = boot ? boot->runtime.party_state_valid : 0;
        struct ChampionState_Compat savedChampion = view.world.party.champions[0];
        int savedChampionCount = view.world.party.championCount;

        expect_true(boot != NULL &&
                        dm1_v1_champion_icon_rect_pc34(0, &iconRect) == 1 &&
                        dm1_v1_champion_status_box_rect_pc34(0, &statusRect) == 1,
                    "M11 CSB resolves party receipt-owned C113/C150 geometry");
        if (boot) {
            /* Deliberately seed the presentation mirror: a missing source
             * GAMEBLOCK party must clear it rather than redraw it. */
            boot->runtime.party_state_valid = 0;
            view.world.party.championCount = 1;
            memset(&view.world.party.champions[0], 0,
                   sizeof(view.world.party.champions[0]));
            view.world.party.champions[0].present = 1;
            view.world.party.champions[0].hp.current = 100;
            view.world.party.champions[0].hp.maximum = 100;
            memset(framebuffer, 0xff, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            expect_true(frame_rect_is_color(framebuffer,
                                            iconRect.x, iconRect.y,
                                            iconRect.w, iconRect.h, 0u) &&
                            frame_rect_is_color(framebuffer,
                                                statusRect.x, statusRect.y,
                                                statusRect.w, statusRect.h, 0u),
                        "M11 CSB clears stale party HUD without a source runtime receipt");
            boot->runtime.party_state_valid = saved_party_state_valid;
            view.world.party.champions[0] = savedChampion;
            view.world.party.championCount = savedChampionCount;
        }
    }
    {
        M11_AssetSlot *c033 = (M11_AssetSlot *)M11_AssetLoader_Load(
            &view.assetLoader, 33u);
        M11_AssetSlot *c020 = (M11_AssetSlot *)M11_AssetLoader_Load(
            &view.assetLoader, 20u);
        DM1_V1_ChampionStatusRectPc34 handRect;
        struct ChampionState_Compat savedChampion = view.world.party.champions[0];
        int savedChampionCount = view.world.party.championCount;
        unsigned short savedC033Width = c033 ? c033->width : 0u;
        unsigned short savedC020Width = c020 ? c020->width : 0u;

        M11_AssetSlot *c015 = (M11_AssetSlot *)M11_AssetLoader_Load(
            &view.assetLoader, 15u);
        DM1_V1_ChampionStatusRectPc34 numberOrigin;
        int savedFontAvailable = view.originalFontAvailable;
        int savedDamageTimer = view.championDamageTimer[0];
        int savedDamageAmount = view.championDamageAmount[0];
        unsigned short savedC015Width = c015 ? c015->width : 0u;

        expect_true(c033 && c033->pixels && c033->loaded &&
                        c033->width == 18u && c033->height == 18u,
                    "M11 CSB exposes the authentic C033 hand-slot surface");
        expect_true(dm1_v1_champion_status_hand_slot_box_rect_pc34(
                        0, 0, &handRect) == 1,
                    "M11 CSB resolves C211 ready-hand geometry");
        expect_true(c015 && c015->pixels && c015->loaded &&
                        c015->width == 45u && c015->height == 7u,
                    "M11 CSB exposes the authentic C015 damage surface");
        expect_true(dm1_v1_champion_damage_number_origin_variant_pc34(
                        0, 42, 0, &numberOrigin) == 1,
                    "M11 CSB resolves F0320 two-digit damage origin");
        view.world.party.championCount = 1;
        memset(&view.world.party.champions[0], 0,
               sizeof(view.world.party.champions[0]));
        view.world.party.champions[0].present = 1;
        view.world.party.champions[0].hp.current = 1;
        view.world.party.champions[0].hp.maximum = 1;
        view.world.party.champions[0].stamina.current = 1;
        view.world.party.champions[0].stamina.maximum = 1;
        view.world.party.champions[0].mana.current = 1;
        view.world.party.champions[0].mana.maximum = 1;
        if (c033) c033->width = 0u;
        if (c020) c020->width = 0u;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(frame_rect_is_color(
                        framebuffer, handRect.x, handRect.y,
                        handRect.w, handRect.h,
                        0u),
                    "M11 CSB leaves C211 black when no source party or C033/C020 is available");
        if (c033) c033->width = savedC033Width;
        if (c020) c020->width = savedC020Width;
        view.originalFontAvailable = 0;
        if (c015) c015->width = 0u;
        view.championDamageTimer[0] = 0;
        view.championDamageAmount[0] = 0;
        memset(movement_base_frame, 0xff, sizeof(movement_base_frame));
        M11_GameView_Draw(&view, movement_base_frame, 320, 200);
        view.championDamageTimer[0] = 1;
        view.championDamageAmount[0] = 42;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(frame_rect_matches(
                        movement_base_frame, framebuffer,
                        numberOrigin.x, numberOrigin.y, 12, 6),
                    "M11 CSB leaves F0053 damage text blank without C015/font bytes");
        if (c015) c015->width = savedC015Width;
        view.championDamageTimer[0] = savedDamageTimer;
        view.championDamageAmount[0] = savedDamageAmount;
        view.originalFontAvailable = savedFontAvailable;
        view.world.party.champions[0] = savedChampion;
        view.world.party.championCount = savedChampionCount;
    }
     {
        CSB_V1_StartupRuntimeAssetSession_PC34 *session =
            (CSB_V1_StartupRuntimeAssetSession_PC34 *)
                view.csbStartupRuntimeAssetSession;
        const CSB_V1_StartupRuntimeSurface_PC34 *c017 =
            &session->surfaces.surfaces[
                CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
        int row_matches = 1;
        int row;

        expect_true(M11_GameView_ToggleInventoryPanel(&view) == 1,
                    "M11 CSB post-entrance input opens the live inventory surface");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        for (row = 0; row < c017->height; ++row) {
            if (memcmp(framebuffer + (size_t)(33 + row) * 320u,
                       c017->pixels + (size_t)row * c017->width,
                       c017->width) != 0) {
                row_matches = 0;
                break;
            }
        }
        expect_true(row_matches,
                    "M11 CSB inventory consumes the terminal C017 bytes at the source viewport geometry");
        record_presented_real_package_frame(
            &view, framebuffer,
            "M11 CSB launcher records the presented terminal C017 HUD package frame");
        expect_true(M11_GameView_ToggleInventoryPanel(&view) == 0,
                    "M11 CSB post-entrance input closes the live inventory surface");
    }
    {
        CSB_V1_StartupRuntimeAssetSession_PC34 *session =
            (CSB_V1_StartupRuntimeAssetSession_PC34 *)
                view.csbStartupRuntimeAssetSession;
        const CSB_V1_StartupRuntimeSurface_PC34 *c017 =
            &session->surfaces.surfaces[
                CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
        const CSB_V1_StartupRuntimeSurface_PC34 *c040 =
            &session->surfaces.surfaces[
                CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34];
        int c040_composed = 1;
        int row;

        expect_true(M11_GameView_ToggleInventoryPanel(&view) == 1,
                    "M11 CSB candidate route opens the terminal inventory base");
        view.candidateMirrorPanelActive = 1;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        for (row = 0; row < c040->height; ++row) {
            int column;
            for (column = 0; column < c040->width; ++column) {
                unsigned char expected = c040->pixels[
                    (size_t)row * c040->width + (size_t)column];
                unsigned char actual = framebuffer[
                    (size_t)(33 + 52 + row) * 320u +
                    (size_t)(80 + column)];
                if (expected == 6) {
                    expected = c017->pixels[
                        (size_t)(52 + row) * c017->width +
                        (size_t)(80 + column)];
                }
                if (actual != expected) {
                    c040_composed = 0;
                    break;
                }
            }
            if (!c040_composed) break;
        }
        expect_true(c040_composed,
                    "M11 CSB candidate panel composes terminal C040 over C017 with source C06 transparency");
        view.candidateMirrorPanelActive = 0;
        expect_true(M11_GameView_ToggleInventoryPanel(&view) == 0,
                    "M11 CSB candidate route closes the terminal inventory base");
        {
            unsigned int saved_generation = session->generation;
            session->generation = saved_generation + 1u;
            memset(framebuffer, 0xff, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) ==
                            0,
                        "M11 CSB rejects a stale terminal session before clearing C040");
            session->generation = saved_generation;
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            expect_true(view.csbState.c040_panel_session_active == 0 &&
                            rows_are_color(framebuffer, 173, 200, 0u),
                        "M11 CSB clears C040 into source HUD while C015 stays source-cleared");
            {
                unsigned int clear_tick = session->source_tick;
                unsigned int saved_generation = session->generation;
                session->generation = saved_generation + 1u;
                expect_true(M11_GameView_HandleInput(
                                &view, M12_MENU_INPUT_TURN_RIGHT) ==
                                M11_GAME_INPUT_IGNORED &&
                                session->source_tick == clear_tick &&
                                view.csbState.c040_clear_live_hud_ready,
                            "M11 CSB rejects a stale terminal session before first post-C040 turn");
                session->generation = saved_generation;
                expect_true(M11_GameView_HandleInput(
                                &view, M12_MENU_INPUT_TURN_RIGHT) ==
                                M11_GAME_INPUT_REDRAW &&
                                session->source_tick == clear_tick + 1u &&
                                !view.csbState.c040_clear_live_hud_ready,
                            "M11 CSB first post-C040 turn consumes the same terminal session tick");
            }
        }
    }
    {
        void *saved_session = view.csbStartupRuntimeAssetSession;
        memset(framebuffer, 0xff, sizeof(framebuffer));
        view.csbStartupRuntimeAssetSession = NULL;
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0,
                    "M11 CSB live HUD rejects a missing terminal source session");
        view.csbStartupRuntimeAssetSession = saved_session;
    }
    {
        const M11_AssetSlot *loaded_c017 =
            M11_AssetLoader_Load(&view.assetLoader, 17u);
        CSB_V1_StartupRuntimeAssetSession_PC34 *session =
            (CSB_V1_StartupRuntimeAssetSession_PC34 *)
                view.csbStartupRuntimeAssetSession;
        const CSB_V1_StartupRuntimeSurface_PC34 *session_c017 =
            &session->surfaces.surfaces[
                CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
        unsigned char saved_c017_byte = 0;
        int row_matches = 1;
        int row;

        expect_true(loaded_c017 && loaded_c017->pixels,
                    "M11 CSB keeps the generic C017 cache separate from the terminal session");
        if (loaded_c017 && loaded_c017->pixels) {
            saved_c017_byte = loaded_c017->pixels[0];
            loaded_c017->pixels[0] ^= 0x0fu;
            expect_true(M11_GameView_ToggleInventoryPanel(&view) == 1,
                        "M11 CSB opens the terminal C017 panel without consulting the generic cache");
            memset(framebuffer, 0xff, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            for (row = 0; row < session_c017->height; ++row) {
                if (memcmp(framebuffer + (size_t)(33 + row) * 320u,
                           session_c017->pixels +
                               (size_t)row * session_c017->width,
                           session_c017->width) != 0) {
                    row_matches = 0;
                    break;
                }
            }
            expect_true(row_matches,
                        "M11 CSB terminal C017 ignores a mismatched generic cache");
            expect_true(M11_GameView_ToggleInventoryPanel(&view) == 0,
                        "M11 CSB closes the terminal C017 panel after cache-isolation proof");
            loaded_c017->pixels[0] = saved_c017_byte;
        }
    }
    expect_true(view.csbState.runtime_object_marker_drawn_count == 0 &&
                    view.csbState.runtime_group_marker_drawn_count == 0 &&
                    view.csbState.runtime_projectile_marker_drawn_count == 0 &&
                    view.csbState.runtime_explosion_marker_drawn_count == 0,
                "M11 CSB post-entrance runtime frame contains no diagnostic material markers");

    M11_GameView_Shutdown(&view);
    M12_StartupMenu_Destroy(&menu);
}

int main(void) {
    printf("=== CSB V1 M12/M11 launcher handoff boundary ===\n");
    expect_true(csb_v1_startup_sequence_source_order_valid_pc34(),
                "CSB launcher startup sequence contract is source-ordered");
    expect_true(csb_v1_startup_title_stage_for_frame_pc34(0) ==
                    CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
                    csb_v1_startup_title_stage_for_frame_pc34(
                        csb_v1_startup_title_presents_ticks_pc34()) ==
                        CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34,
                "CSB launcher title helper matches M11 handoff stages");

    run_empty_launcher_boundary();
    run_real_launcher_handoff_if_available();

    printf("\nCSB V1 M12/M11 launcher handoff boundary: %d passed, %d failed, %d skipped\n",
           g_passed, g_failures, g_skipped);
    if (g_failures) {
        fprintf(stderr,
                "CSB V1 M12/M11 launcher handoff boundary FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: CSB V1 M12/M11 launcher handoff boundary is wired");
    return 0;
}
