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
#include "entrance_frontend_pc34_compat.h"
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

static int count_nonzero_pixels_in_rows(const unsigned char* pixels,
                                        int first_row,
                                        int last_row) {
    int x;
    int y;
    int count = 0;
    if (!pixels || first_row < 0 || last_row > 200 || first_row >= last_row) {
        return 0;
    }
    for (y = first_row; y < last_row; ++y) {
        for (x = 0; x < 320; ++x) {
            if (pixels[y * 320 + x] != 0u) {
                ++count;
            }
        }
    }
    return count;
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
    const M12_MenuEntry* entry;
    char real_dir[512];
    const char* data_dir = default_data_root(real_dir);
    unsigned char framebuffer[320 * 200];
    unsigned char title_presents_frame[320 * 200];
    unsigned char title_chaos_frame[320 * 200];
    unsigned char entrance_closed_frame[320 * 200];
    unsigned char entrance_opening_frame[320 * 200];
    int tick_before;
    int entrance_frame_before;

    if (!data_dir || !data_dir[0]) {
        expect_skip("HOME is unset; no default Firestaff data root");
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
    drive_csb_entrance_opening(
        &view,
        "M11 CSB launcher entrance clears after explicit command");
    expect_true(view.csbState.startup_entrance_last_command ==
                    ENTRANCE_COMPAT_RUNTIME_COMMAND_ENTER_DUNGEON,
                "M11 CSB launcher handoff records source enter-dungeon command");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(view.csbState.level_loaded == 1 &&
                    view.csbState.current_level >= 0,
                "M11 CSB post-entrance handoff retains the loaded source dungeon");
    expect_true(count_nonzero_pixels_in_rows(framebuffer, 169, 200) > 0,
                "M11 CSB post-entrance handoff draws the V1 champion/control HUD band");
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
