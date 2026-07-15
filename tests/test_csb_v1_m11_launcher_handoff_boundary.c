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

    if (!view || !frame) {
        expect_true(0, message);
        return;
    }
    expected_hash = indexed_frame_hash(frame, 320u * 200u);
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

static void drive_csb_entrance_opening(M11_GameViewState *view,
                                       const char *message) {
    unsigned int guard =
        csb_v1_startup_entrance_pre_open_delay_ticks_pc34() +
        ENTRANCE_Compat_GetDoorAnimationStepCount() + 8u;
    int tick_before;
    if (!view) {
        expect_true(0, message);
        return;
    }
    tick_before = view->csbState.tick_count;
    expect_true(view->csbState.startup_entrance_active == 1 &&
                    view->csbState.startup_entrance_opening_active == 1,
                "M11 CSB launcher entrance starts door-opening phase");
    while (guard-- > 0 && view->csbState.startup_entrance_active) {
        expect_true(M11_GameView_AdvanceIdleTick(view) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB launcher entrance door-opening tick redraws");
    }
    expect_true(view->csbState.tick_count == tick_before,
                "M11 CSB launcher entrance door-opening blocks runtime ticks");
    expect_true(view->csbState.startup_entrance_active == 0 &&
                    view->csbState.startup_entrance_dismissed == 1,
                message);
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
                     view.csbStartupRuntimeAssetSession)->real_asset_matched &&
                    real_package.assets_verified &&
                    real_package.graphics_verified &&
                    real_package.dungeon_verified,
                "M11 CSB launcher session is bound to the hash-verified PC34 package");
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

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                     view.csbStartupRuntimeAssetSession)
                    ->surfaces.surfaces[
                        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].width ==
                    320 &&
                    ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                         view.csbStartupRuntimeAssetSession)
                        ->surfaces.surfaces[
                            CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].height ==
                    16,
                "M11 CSB launcher emits C001 PRESENTS through its source geometry");
    expect_true(M11_GameView_GetPresentationSpecialPalette(&view) ==
                    VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS,
                "M11 CSB launcher PRESENTS frame keeps C001 special palette");
    memcpy(title_presents_frame, framebuffer, sizeof(title_presents_frame));
    record_presented_real_package_frame(
        &view, title_presents_frame,
        "M11 CSB launcher records the presented C001 PRESENTS package frame");

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
         i < csb_v1_startup_title_presents_ticks_pc34() &&
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
         i < csb_v1_startup_title_chaos_zoom_ticks_pc34() +
                 csb_v1_startup_title_chaos_hold_ticks_pc34() &&
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
                    view.csbState.startup_title_source_step == 21,
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
    drive_csb_entrance_opening(
        &view,
        "M11 CSB launcher entrance clears after explicit command");
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
