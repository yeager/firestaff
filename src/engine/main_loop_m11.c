/*
 * main_loop_m11.c — M11 Phase A stub.
 *
 * Opens a window via render_sdl_m11, presents a black framebuffer, pumps
 * events until either the user quits or the configured duration elapses,
 * then shuts down.
 */

#include "main_loop_m11.h"
#include "m11_loose_sibling.h"
#include "config_m12.h"

#include "menu_startup_m12.h"
#include "menu_startup_render_modern_m12.h"
#include "startup_intro_m12.h"
#include "firestaff_version.h"
#include "menu_hit_m12.h"
#include "m11_game_view.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_hud_hit_rects.h"
#include "dm1_entrance_m11.h"
#include "firestaff_po_loader.h"
#include "firestaff_accessibility.h"
#include "firestaff_retroachievements.h"
#include "audio_sdl_m11.h"
#include "render_sdl_m11.h"
#include "dm1_v2_startup_title_filter_handoff_pc34.h"
#include "csb_v2_filter_config_pc34.h"
#include "csb_v2_texture_upscale_pc34.h"
#include "m11_qol_runtime.h"
#include "dm1_v1_minimap_pc34_compat.h"
#include "dm1_v1_automap_pc34_compat.h"
#include "dm1_v1_combat_log_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "title_frontend_v1.h"
#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "asset_status_m12.h"
#include "firestaff_l10n.h"
#include "asset_find_by_hash.h"
#include "m11_game_text_ttf_renderer_pc34_compat.h"
#include "fs_portable_compat.h"
#include "dm1_v1_vblank_timing.h"
#include "dm1_v1_fmtowns_startup.h"
#include "dm1_v1_fmtowns_title.h"
#include "dm1_v1_i34e_title_anim_pc34.h"
#include "dm1_v1_selector_pc34_compat.h"
#include "dm1_v1_dos_pc34_real_data_manifest.h"
#include "firestaff_x68k_media_receipt.h"
#include "dm1_v1_fmtowns_cd_audio.h"
#include "entrance_frontend_pc34_compat.h"
#include "entrance_mouse_routes_pc34_compat.h"
#include "csb_v1_keyboard_commands_pc34_compat.h"
#include "input_remap_m11.h"
#include "colorblind_m11.h"
#include "ui_scale_m11.h"
#include "touch_layout_m12.h"
#include "gamepad_config_m12.h"
#include "vga_palette_pc34_compat.h"
#include "swsh_frontend_pc34_compat.h"
#include "dm1_v1_amiga_swsh.h"
#include "dm1_v1_amiga_title_f0437.h"
#include "dm1_v1_amiga_entrance_f0441.h"
#include "dm1_v1_amiga_graphics_dat.h"
#include "redmcsb_amiga_graphic_expand.h"
#include "csb_v1_amiga_swsh_ftl.h"
#include "dm1_v1_iigs_graphics_dat.h"
#include "dm1_v1_iigs_installation.h"
#include "dm1_v1_iigs_title_f0437.h"
#include "dm1_v1_amiga_palette_fade.h"
#include "screenshot_m11.h"
#include "v1_swsh_intro_pathfinder_pc34_compat.h"
#include "v1_title_intro_pathfinder_pc34_compat.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v20_startup_presentation_timing_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
#include "csb_v22_finished_art_material_gate_pc34.h"
#include "csb_v22_modern_assets_pc34.h"
#include "csb_v1_boot.h"
#include "csb_v1_amiga_english_programs.h"
#include "csb_v1_f0908_f0909_f0910_swsh_sound_pc34_compat.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_mac_input.h"
#include "fs_gesture_navigation_gate.h"
#include "theron_v1_world.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_hud_hit_rects.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <sys/resource.h>
#if defined(__APPLE__)
#include <mach/mach.h>
#elif defined(__linux__)
#include <unistd.h>
#endif
#endif

#include <SDL3/SDL.h>

#if !SDL_VERSION_ATLEAST(3, 0, 0)
#include <SDL.h>
#define SDLK_A SDLK_a
#define SDLK_C SDLK_c
#define SDLK_D SDLK_d
#define SDLK_E SDLK_e
#define SDLK_G SDLK_g
#define SDLK_I SDLK_i
#define SDLK_M SDLK_m
#define SDLK_P SDLK_p
#define SDLK_Q SDLK_q
#define SDLK_R SDLK_r
#define SDLK_S SDLK_s
#define SDLK_U SDLK_u
#define SDLK_V SDLK_v
#define SDLK_W SDLK_w
#define SDLK_X SDLK_x
#endif

enum {
    M11_LAUNCHER_FB_WIDTH = 480,
    M11_LAUNCHER_FB_HEIGHT = 270,
    M11_LAUNCHER_MODERN_WIDTH = M12_MODERN_MENU_NATIVE_WIDTH,
    M11_LAUNCHER_MODERN_HEIGHT = M12_MODERN_MENU_NATIVE_HEIGHT
};

static void m11_debug_log_scan_progress(const M12_AssetScanProgress* progress,
                                        int languageIndex);
static void m11_debug_reset_scan_progress_trace(void);

/* The launcher owns persistence and editing of the touch layout; the active
 * game loop owns only the translated input.  Keeping this bridge here sends
 * virtual controls through precisely the same M12 input path as keyboard and
 * gamepad input, rather than synthesizing pointer clicks into a dungeon.
 */
static M12_MenuInput m11_touch_layout_action_to_input(M12_InputAction action) {
    switch (action) {
    case M12_ACTION_MOVE_FORWARD:     return M12_MENU_INPUT_UP;
    case M12_ACTION_MOVE_BACKWARD:    return M12_MENU_INPUT_DOWN;
    case M12_ACTION_TURN_LEFT:        return M12_MENU_INPUT_TURN_LEFT;
    case M12_ACTION_TURN_RIGHT:       return M12_MENU_INPUT_TURN_RIGHT;
    case M12_ACTION_STRAFE_LEFT:      return M12_MENU_INPUT_STRAFE_LEFT;
    case M12_ACTION_STRAFE_RIGHT:     return M12_MENU_INPUT_STRAFE_RIGHT;
    case M12_ACTION_ACCEPT:           return M12_MENU_INPUT_ACCEPT;
    case M12_ACTION_BACK:             return M12_MENU_INPUT_BACK;
    case M12_ACTION_ACTION:           return M12_MENU_INPUT_ACTION;
    case M12_ACTION_CYCLE_CHAMPION:   return M12_MENU_INPUT_CYCLE_CHAMPION;
    case M12_ACTION_REST_TOGGLE:      return M12_MENU_INPUT_REST_TOGGLE;
    case M12_ACTION_USE_STAIRS:       return M12_MENU_INPUT_USE_STAIRS;
    case M12_ACTION_PICKUP_ITEM:      return M12_MENU_INPUT_PICKUP_ITEM;
    case M12_ACTION_DROP_ITEM:        return M12_MENU_INPUT_DROP_ITEM;
    case M12_ACTION_SPELL_RUNE_1:     return M12_MENU_INPUT_SPELL_RUNE_1;
    case M12_ACTION_SPELL_RUNE_2:     return M12_MENU_INPUT_SPELL_RUNE_2;
    case M12_ACTION_SPELL_RUNE_3:     return M12_MENU_INPUT_SPELL_RUNE_3;
    case M12_ACTION_SPELL_RUNE_4:     return M12_MENU_INPUT_SPELL_RUNE_4;
    case M12_ACTION_SPELL_RUNE_5:     return M12_MENU_INPUT_SPELL_RUNE_5;
    case M12_ACTION_SPELL_RUNE_6:     return M12_MENU_INPUT_SPELL_RUNE_6;
    case M12_ACTION_SPELL_CAST:       return M12_MENU_INPUT_SPELL_CAST;
    case M12_ACTION_SPELL_CLEAR:      return M12_MENU_INPUT_SPELL_CLEAR;
    case M12_ACTION_USE_ITEM:         return M12_MENU_INPUT_USE_ITEM;
    case M12_ACTION_MAP_TOGGLE:       return M12_MENU_INPUT_MAP_TOGGLE;
    case M12_ACTION_INVENTORY_TOGGLE: return M12_MENU_INPUT_INVENTORY_TOGGLE;
    case M12_ACTION_QUICK_SAVE:       return M12_MENU_INPUT_SAVE_GAME;
    case M12_ACTION_QUICK_LOAD:
    case M12_ACTION_COUNT:
    default:                          return M12_MENU_INPUT_NONE;
    }
}

static M12_MenuInput m11_touch_layout_input_at(float normalizedX,
                                               float normalizedY) {
    static M12_TouchLayout layout;
    static int initialized = 0;
    int zone;
    int x;
    int y;

    if (!initialized) {
        M12_TouchLayout_SetDefaults(&layout);
        (void)M12_TouchLayout_Load(&layout);
        initialized = 1;
    }
    if (normalizedX < 0.0f || normalizedX > 1.0f ||
        normalizedY < 0.0f || normalizedY > 1.0f) return M12_MENU_INPUT_NONE;
    x = (int)(normalizedX * (float)M12_TOUCH_CANVAS_W);
    y = (int)(normalizedY * (float)M12_TOUCH_CANVAS_H);
    if (x >= M12_TOUCH_CANVAS_W) x = M12_TOUCH_CANVAS_W - 1;
    if (y >= M12_TOUCH_CANVAS_H) y = M12_TOUCH_CANVAS_H - 1;
    zone = M12_TouchLayout_HitTest(&layout, x, y);
    return zone >= 0 ? m11_touch_layout_action_to_input(layout.zones[zone].action)
                     : M12_MENU_INPUT_NONE;
}

uint32_t M11_GameView_IdleTickIntervalMs(const M11_GameViewState* gameView,
                                         int speedMultiplier) {
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
        gameView->dm2State.startup_menu_active) {
        const DM2_V1_BootProfile *profile =
            (const DM2_V1_BootProfile *)gameView->dm2BootProfile;
        if (profile && profile->platform == DM2_PLATFORM_AMIGA_EN &&
            gameView->dm2FmtownsTitleBound) {
            /* Amiga SWSH/TITL/ENDA are VBlank streams.  Unlike the Towns
             * Timer-A path, their authentic 50 Hz period is integral in the
             * host millisecond scheduler, so do not wake at the PC/Towns
             * 16-ms quantum and overdrive the selected Amiga media. */
            return 20u;
        }
        /* DM2's IBMIOP MVE, FM Towns TWANIM and Amiga VBlank streams are
         * startup media, not the 200-ms gameplay loop.  M11 wakes at the
         * same 16-ms scheduler quantum used for the Towns Timer-A path; the
         * DM2 owner itself accumulates the exact 16,632-us Towns and
         * 20,000-us Amiga source periods, while the MVE presenter keeps its
         * own 83,328-us frame clock.  Game-speed settings must not alter any
         * of those original media clocks. */
        return 16u;
    }
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT &&
        (gameView->csbState.startup_title_active ||
         gameView->csbState.startup_entrance_active)) {
        const CSB_V1_BootProfile* profile =
            (const CSB_V1_BootProfile*)gameView->csbBootProfile;
        if (profile &&
            (profile->variant_id == CSB_V1_VARIANT_FMTOWNS_EN ||
             profile->variant_id == CSB_V1_VARIANT_FMTOWNS_JA) &&
            gameView->csbFmtownsTitleBound) {
            /* TOWNSIO.C F2263 selects Timer A count 100. YM2612 Timer A is
             * 18*(1024-count) us (16,632 us here); M11's 16-ms wakeup is
             * only a scheduler quantum. The state-side accumulator keeps
             * the original Timer-A period instead of rounding title frames. */
            return 16u;
        }
        if (profile &&
            (profile->variant_id == CSB_V1_VARIANT_AMIGA31_EN ||
             profile->variant_id == CSB_V1_VARIANT_AMIGA31_MULTI ||
             profile->variant_id == CSB_V1_VARIANT_AMIGA35_EN ||
             profile->variant_id == CSB_V1_VARIANT_AMIGA35_MULTI)) {
            /* ReDMCSB's Amiga startup waits on PAL VBlank (50 Hz). Keep the
             * source interval independent of the host display's refresh. */
            return 20u;
        }
        if (profile &&
            (profile->variant_id == CSB_V1_VARIANT_ST20_EN ||
             profile->variant_id == CSB_V1_VARIANT_ST21_EN)) {
            /* BASE.C's VBL handler runs at the Atari ST's 50 Hz source
             * cadence. Use 20 ms host scheduler steps during title and
             * Entrance instead of treating each VGA tick as a PAL blank. */
            return 20u;
        }
        /* TITLE.C F0437's waits are source VBlank waits.  Each tick in the
         * CSB title sequence corresponds to one M526_WaitVerticalBlank call.
         * On VGA mode 13h that polls 0x3DA at ~70 Hz = 14ms per VBlank.
         * All phases (PRESENTS, CHAOS zoom, CHAOS hold, STRIKES BACK) use
         * the same VBlank period — the 4× multiplier on zoom was wrong and
         * caused STRIKES BACK to appear at the wrong time. */
        return DM1_V1_VGA_VBLANK_MS;
    }

    if (speedMultiplier < 50) speedMultiplier = 50;
    if (speedMultiplier > 400) speedMultiplier = 400;
    if (gameView && gameView->sourceId[0] != '\0' &&
        strcmp(gameView->sourceId, "dm1") == 0 &&
        gameView->assetLoader.dm1Amiga36) {
        /* ReDMCSB GAMELOOP.C:46-50 uses 12 waits for A36M; VBLANK.C:F0577
         * advances once per PAL blank. Keep the gameplay clock at 12*20ms. */
        const uint32_t sourceInterval =
            gameView->vblankTiming.vblankPeriodMs *
            (uint32_t)gameView->vblankTiming.maxVBlankCount;
        if (sourceInterval > 0u) {
            return (sourceInterval * 100u + (uint32_t)speedMultiplier / 2u) /
                (uint32_t)speedMultiplier;
        }
    }
    if (gameView && gameView->sourceId[0] != '\0' &&
        strcmp(gameView->sourceId, "dm1") == 0 &&
        gameView->vblankTiming.pc34PitInputWait) {
        /* ReDMCSB IO.C:F0782 adds 3 per IBMIO callback; GAMELOOP.C selects
         * 12 for PC34 media, so four nominal 55 ms BIOS/PIT callbacks gate
         * each source tick. */
        const uint32_t sourceInterval = 220u;
        return (sourceInterval * 100u + (uint32_t)speedMultiplier / 2u) /
            (uint32_t)speedMultiplier;
    }
    return (uint32_t)((200 * 100 + speedMultiplier / 2) / speedMultiplier);
}

int M11_GameView_DropsIdleCatchupForStartup(const M11_GameViewState* gameView)
{
    if (!gameView) return 0;
    if (gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
        gameView->dm2State.startup_menu_active) {
        /* DM2 movie and animation steps advance in Draw against absolute
         * source deadlines. Keep elapsed scheduler debt so a slow or low-Hz
         * host can decode overdue source steps in order before the next
         * present, instead of stretching the intro to the display cadence. */
        return 0;
    }
    /* CSB startup advances one source VBlank per elapsed source interval.
     * Keep accumulator debt across a blocking present so 30/60/120 Hz host
     * displays cannot slow the original title and entrance timeline. */
    return 0;
}

static int m11_should_use_modern_launcher(const M12_StartupMenuState* menuState) {
    /* The startup menu is Firestaff's shared product front door for every
     * presentation mode, including V1 original.  V1 parity begins after the
     * user launches a game: TITLE/entrance/Hall-of-Champions sequencing must
     * not force the launcher itself back to the old sparse indexed renderer.
     * The old 480x270 path has no corresponding pointer hit map, so allowing
     * it here made a visible launcher wholly inaccessible by mouse. */
    return menuState != NULL;
}

/* DM2 action/save feedback belongs to its original GUI/dialogue producer.
 * Until that owner is bound, M11 must not expose host-authored SAVE/LOAD
 * strings for the DM2 source route. Other source kinds retain their existing
 * diagnostic logging. */
static int m11_dm2_host_save_log_allowed(const M11_GameViewState* gameView)
{
    return gameView && gameView->sourceKind != M11_GAME_SOURCE_DM2_BOOT;
}

int M11_ResolveScaleModeForWindowMode(int requestedScaleMode, int windowMode);
static int m11_running_from_macos_app_bundle(void);
static int m11_map_window_pointer_to_game_source(const M11_GameViewState* gameView,
                                                 int windowX,
                                                 int windowY,
                                                 int* outX,
                                                 int* outY);
static int m11_draw_entrance_credits_asset(M11_GameViewState* gameView,
                                           unsigned char* framebuffer) {
    const M11_AssetSlot* credits;
    if (!gameView || !framebuffer || !gameView->assetsAvailable) {
        return 0;
    }
    /* ReDMCSB ENTRANCE.C:F0442 displays C005 directly.  It must never
     * substitute a host-made frame when the authenticated PC34 page is
     * available, because its colour indices only make sense with G0019. */
    credits = M11_AssetLoader_Load(&gameView->assetLoader, 5U);
    if (!credits || !credits->pixels || credits->width == 0U ||
        credits->height == 0U) {
        return 0;
    }
    memset(framebuffer, 0, (size_t)M11_FB_BYTES);
    M11_AssetLoader_Blit(credits, framebuffer,
                         M11_FB_WIDTH, M11_FB_HEIGHT, 0, 0, -1);
    return 1;
}
static int DM1_V1_Entrance_FullStartRenderReceiptHostReadyPc34Compat(
    const DM1_V1_EntranceFullStartRenderReceiptPc34* receipt) {
    return receipt && receipt->realAssetCaptureProof &&
           receipt->requiresGraphicsDat && receipt->noHostRenderInference;
}
static int DM1_V1_InputMenuTokenUsesHeldRepeatPc34Compat(int token) {
    return token == M12_MENU_INPUT_LEFT || token == M12_MENU_INPUT_RIGHT ||
           token == M12_MENU_INPUT_UP || token == M12_MENU_INPUT_DOWN ||
           token == M12_MENU_INPUT_STRAFE_LEFT ||
           token == M12_MENU_INPUT_STRAFE_RIGHT;
}
static int M12_StartupMenu_PrepareSelectedGameLaunch(
        M12_StartupMenuState* menuState) {
    const M12_MenuEntry* entry;
    const M12_AssetVersionStatus* version;
    M12_LaunchIntent intent;
    char runtimeDir[M12_ASSET_DATA_DIR_CAPACITY];
    int versionIndex;
    int gameOptionsSlot;
    if (!menuState || menuState->activatedIndex < 0 ||
        menuState->activatedIndex >= M12_StartupMenu_GetEntryCount()) {
        return 0;
    }
    entry = M12_StartupMenu_GetEntry(menuState, menuState->activatedIndex);
    if (!entry || entry->kind != M12_MENU_ENTRY_GAME || !entry->gameId) {
        return 0;
    }
    gameOptionsSlot = M12_StartupMenu_GetGameOptionsSlot(entry->gameId);
    if (gameOptionsSlot < 0 || gameOptionsSlot >= M12_CONFIG_GAME_COUNT) {
        return 0;
    }
    if (strcmp(entry->gameId, "dm1") == 0) {
        intent = M12_StartupMenu_GetLaunchIntent(menuState);
        versionIndex = intent.valid && intent.options.versionIndex >= 0
            ? intent.options.versionIndex
            : menuState->gameOptions[gameOptionsSlot].versionIndex;
        version = versionIndex >= 0
            ? M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                         (size_t)versionIndex) : NULL;
        if (!version || !version->matched || !version->versionId ||
            !M12_AssetStatus_PrepareDM1RuntimeVersion(
                &menuState->assetStatus, version->versionId, runtimeDir,
                sizeof(runtimeDir))) return 0;
        snprintf(menuState->assetStatus.runtimeDataDirs[0],
                 sizeof(menuState->assetStatus.runtimeDataDirs[0]), "%s",
                 runtimeDir);
        return 1;
    }
    if (strcmp(entry->gameId, "csb") != 0) return 1;
    /* The selected CSB edition owns this launch. Archives and ADFs retain
     * their original source locator; M11 opens verified members in bounded
     * RAM. A loose package likewise keeps its original directory and startup
     * siblings. */
    /* M12 may repair a stale persisted version/architecture selection while
     * building the launch intent.  Direct CLI starts previously validated
     * that effective ST edition, then this preparatory CSB step reopened the
     * stale (often unavailable PC3.4) slot and failed before M11 received
     * the verified runtime directory.  The intent is the selected-launch
     * authority for both the menu and CLI. */
    intent = M12_StartupMenu_GetLaunchIntent(menuState);
    versionIndex = intent.valid && intent.options.versionIndex >= 0
        ? intent.options.versionIndex
        : menuState->gameOptions[gameOptionsSlot].versionIndex;
    version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&menuState->assetStatus, "csb",
                                     (size_t)versionIndex) : NULL;
    if (!version || !version->matched || !version->versionId) {
        return 0;
    }
    if (!M12_AssetStatus_PrepareCSBRuntimeVersion(
            &menuState->assetStatus, version->versionId, runtimeDir,
            sizeof(runtimeDir))) {
        const char *separator;
        /* F31's in-process boot accepts the authenticated original archive
         * directly. No derived disk image is a launch prerequisite: keep a
         * verified Japanese/English CD launch available from its source
         * archive. */
        separator = (strcmp(version->versionId, "fmtowns-en") == 0 ||
                     strcmp(version->versionId, "fmtowns-ja") == 0)
            ? strstr(version->matchedPath, "::") : NULL;
        if (!separator || (size_t)(separator - version->matchedPath) >=
                              sizeof(runtimeDir)) {
            return 0;
        }
        memcpy(runtimeDir, version->matchedPath,
               (size_t)(separator - version->matchedPath));
        runtimeDir[separator - version->matchedPath] = '\0';
        if (!FSP_FileExists(runtimeDir)) {
            return 0;
        }
    }
    snprintf(menuState->assetStatus.runtimeDataDirs[1],
             sizeof(menuState->assetStatus.runtimeDataDirs[1]), "%s",
             runtimeDir);
    return 1;
}

static int m11_apply_architecture_override(M12_StartupMenuState* menuState,
                                           const char* gameId,
                                           int architecture);

static void m11_set_launch_failed_message(M12_StartupMenuState* menuState) {
    const M12_MenuEntry* entry = NULL;
    const char* gameId = NULL;
    if (!menuState) {
        return;
    }
    if (menuState->activatedIndex >= 0) {
        entry = M12_StartupMenu_GetEntry(menuState, menuState->activatedIndex);
    }
    if (!entry && menuState->selectedIndex >= 0) {
        entry = M12_StartupMenu_GetEntry(menuState, menuState->selectedIndex);
    }
    gameId = entry ? entry->gameId : NULL;
    menuState->launchRequested = 0;
    menuState->quickResumeLaunchRequested = 0;
    /* A failed CSB import is a completed launch attempt. Do not leave its
     * source path attached to a later, unrelated CSB launch intent. */
    menuState->csbImportDm1LaunchRequested = 0;
    menuState->csbImportDm1SavePath[0] = '\0';
    menuState->view = M12_MENU_VIEW_MESSAGE;
    if (gameId && strcmp(gameId, "nexus") == 0) {
        menuState->messageLine1 = M12_StartupMenu_Translate(
            menuState, "NEXUS LOAD FAILED");
        menuState->messageLine2 = M12_StartupMenu_Translate(
            menuState, "CHECK ISO/BIN OR EXTRACTED FILES");
    } else if (gameId && strcmp(gameId, "theron") == 0) {
        /* This generic launcher path covers both invalid media and a valid
         * Track 02 whose later original graphics route is still unavailable.
         * The message must not imply that a hash-verified CUE/BIN is corrupt.
         */
        menuState->messageLine1 = M12_StartupMenu_Translate(
            menuState, "THERON STARTUP FAILED");
        menuState->messageLine2 = M12_StartupMenu_Translate(
            menuState, "VERIFY CUE/BIN AND STARTUP DETAILS");
    } else if (gameId && strcmp(gameId, "dm2") == 0) {
        /* DM2's original startup/load path owns its dialogue surface.  No
         * source-owned c_gui_draw producer is connected to this generic M11
         * failure callback, so do not publish host-authored English text as
         * if it were the game.  Keep the structured launch failure and view
         * transition intact; the visible channel remains empty fail-closed. */
        menuState->messageLine1 = "";
        menuState->messageLine2 = "";
        menuState->messageLine3 = "";
        /* There is no source-owned failure dialogue on this generic M11
         * callback. Do not leave the launcher displaying its empty MESSAGE
         * panel; the next frame belongs to the M12 main view until a real
         * DM2 dialogue producer is bound. */
        menuState->view = M12_MENU_VIEW_MAIN;
        return;
    } else if (gameId && strcmp(gameId, "csb") == 0) {
        menuState->messageLine1 = M12_StartupMenu_Translate(
            menuState, "CSB LOAD FAILED");
        menuState->messageLine2 = M12_StartupMenu_Translate(
            menuState, "CHECK GRAPHICS/DUNGEON DATA");
    } else {
        menuState->messageLine1 = M12_StartupMenu_Translate(
            menuState, "DUNGEON LOAD FAILED");
        menuState->messageLine2 = M12_StartupMenu_Translate(
            menuState, "CHECK DUNGEON.DAT");
    }
    menuState->messageLine3 = M12_StartupMenu_Translate(
        menuState, "ESC RETURNS TO MENU");
}

static void m11_draw_intro_progress_bar(unsigned char* rgba,
                                        int w, int h,
                                        const M12_AssetScanProgress* progress,
                                        Uint64 elapsedMs,
                                        int languageIndex);

static void m11_draw_launcher_legacy(const M12_StartupMenuState* menuState,
                                     unsigned char* launcherFramebuffer) {
    if (!menuState || !launcherFramebuffer) {
        return;
    }
    M12_StartupMenu_Draw(menuState,
                         launcherFramebuffer,
                         M11_LAUNCHER_FB_WIDTH,
                         M11_LAUNCHER_FB_HEIGHT);
}

static void m11_draw_launcher_modern(const M12_StartupMenuState* menuState,
                                     unsigned char* modernRgba) {
    if (!menuState || !modernRgba) {
        return;
    }
    M12_ModernMenu_Render(menuState,
                          modernRgba,
                          M11_LAUNCHER_MODERN_WIDTH,
                          M11_LAUNCHER_MODERN_HEIGHT);
}

static void m11_draw_launcher(const M12_StartupMenuState* menuState,
                              unsigned char* launcherFramebuffer,
                              unsigned char* modernRgba,
                              int useModern) {
    M12_AssetScanProgress scanProgress;
    if (useModern && modernRgba) {
        m11_draw_launcher_modern(menuState, modernRgba);
    } else if (launcherFramebuffer) {
        m11_draw_launcher_legacy(menuState, launcherFramebuffer);
    }
    if (menuState && menuState->deferredScanActive &&
        M12_StartupMenu_GetDeferredScanProgress(menuState, &scanProgress)) {
        m11_debug_log_scan_progress(&scanProgress,
                                    menuState->settings.languageIndex);
        if (useModern && modernRgba) {
            m11_draw_intro_progress_bar(modernRgba,
                                        M11_LAUNCHER_MODERN_WIDTH,
                                        M11_LAUNCHER_MODERN_HEIGHT,
                                        &scanProgress,
                                        SDL_GetTicks(),
                                        menuState->settings.languageIndex);
        } else if (launcherFramebuffer) {
            M12_StartupMenu_DrawScanProgressOverlayLocalized(
                &scanProgress, menuState->settings.languageIndex,
                launcherFramebuffer, M11_LAUNCHER_FB_WIDTH,
                M11_LAUNCHER_FB_HEIGHT);
        }
    } else {
        m11_debug_reset_scan_progress_trace();
    }
}

static int m11_present_launcher(unsigned char* launcherFramebuffer,
                                unsigned char* modernRgba,
                                int useModern) {
    /* The launcher is application UI, not a 320x200 game surface.  It must
     * occupy a maximized/fullscreen window regardless of the selected game
     * aspect.  The renderer uses the same flag when it maps menu clicks. */
    (void)M11_Render_SetPresentationFillWindow(1);
    if (useModern && modernRgba) {
        return M11_Render_PresentRGBA(modernRgba,
                                      M11_LAUNCHER_MODERN_WIDTH,
                                      M11_LAUNCHER_MODERN_HEIGHT);
    }
    return M11_Render_PresentIndexed(launcherFramebuffer,
                                     M11_LAUNCHER_FB_WIDTH,
                                     M11_LAUNCHER_FB_HEIGHT);
}

typedef struct {
    unsigned char* framebuffer;
    unsigned char* modernRgba;
    int width;
    int height;
    int languageIndex;
    int colorblindMode;
    int uiScale;
    int debug;
    int useModern;
    int* quitRequested;
    int autotestQuitInjected;
    int returnMenuRescan;
    Uint64 startedMs;
} M11_ScanProgressContext;

static int m11_scan_progress_callback(const M12_AssetScanProgress* progress,
                                      void* userData) {
    M11_ScanProgressContext* ctx = (M11_ScanProgressContext*)userData;
    if (!ctx || (!ctx->framebuffer && !ctx->modernRgba)) return 1;
    if (ctx->debug) {
        m11_debug_log_scan_progress(progress, ctx->languageIndex);
    }
    /* A game frame leaves presentation constrained to its viewport. The scan
     * panel is launcher UI and must use the full window, including after a
     * game returns to the menu. */
    (void)M11_Render_SetPresentationFillWindow(1);
    if (ctx->useModern && ctx->modernRgba) {
        M11_UIScale_SetPercent(ctx->uiScale);
        M12_ModernMenu_RenderScanProgressLocalized(
            progress, ctx->languageIndex, ctx->modernRgba,
            M11_LAUNCHER_MODERN_WIDTH, M11_LAUNCHER_MODERN_HEIGHT);
        M11_Colorblind_ApplyRGBA(ctx->colorblindMode, ctx->modernRgba,
                                 M11_LAUNCHER_MODERN_WIDTH,
                                 M11_LAUNCHER_MODERN_HEIGHT);
        M11_Render_PresentRGBA(ctx->modernRgba,
                               M11_LAUNCHER_MODERN_WIDTH,
                               M11_LAUNCHER_MODERN_HEIGHT);
    } else if (ctx->framebuffer) {
        M12_StartupMenu_DrawScanProgressLocalized(progress, ctx->languageIndex,
                                                  ctx->framebuffer,
                                                  ctx->width, ctx->height);
        M11_Render_PresentIndexed(ctx->framebuffer, ctx->width, ctx->height);
    }
    if (!ctx->autotestQuitInjected && getenv("FIRESTAFF_AUTOTEST") &&
        ((ctx->returnMenuRescan &&
          getenv("FIRESTAFF_AUTOTEST_QUIT_DURING_RESCAN") &&
          strcmp(getenv("FIRESTAFF_AUTOTEST_QUIT_DURING_RESCAN"), "1") == 0) ||
         (!ctx->returnMenuRescan &&
          getenv("FIRESTAFF_AUTOTEST_QUIT_DURING_SCAN") &&
          strcmp(getenv("FIRESTAFF_AUTOTEST_QUIT_DURING_SCAN"), "1") == 0))) {
        SDL_Event event;
        SDL_Window* window = M11_Render_GetWindow();
        memset(&event, 0, sizeof(event));
        if (window) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
            event.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
            event.window.windowID = SDL_GetWindowID(window);
#else
            event.type = SDL_WINDOWEVENT;
            event.window.windowID = SDL_GetWindowID(window);
            event.window.event = SDL_WINDOWEVENT_CLOSE;
#endif
            if (SDL_PushEvent(&event)) {
                ctx->autotestQuitInjected = 1;
                (void)FSP_SetEnv("FIRESTAFF_AUTOTEST_SCAN_QUIT_INJECTED", "1", 1);
                if (ctx->debug && ctx->returnMenuRescan) {
                    fprintf(stderr,
                            "firestaff: autotest injected window-close during return-menu rescan\n");
                }
            }
        }
    }
    if (M11_Render_PumpEvents()) {
        if (ctx->quitRequested) *ctx->quitRequested = 1;
        return 0;
    }
    return 1;
}

static void m11_rescan_launcher_asset_status(
    M12_StartupMenuState* menuState,
    unsigned char* launcherFramebuffer,
    unsigned char* modernRgba,
    int useModern,
    int debug,
    int* quitRequested) {
    M11_ScanProgressContext scanCtx;
    if (!menuState || !launcherFramebuffer) return;
    scanCtx.framebuffer = launcherFramebuffer;
    scanCtx.modernRgba = modernRgba;
    scanCtx.width = M11_LAUNCHER_FB_WIDTH;
    scanCtx.height = M11_LAUNCHER_FB_HEIGHT;
    scanCtx.languageIndex = menuState->settings.languageIndex;
    scanCtx.colorblindMode = menuState->settings.colorblindMode;
    scanCtx.uiScale = menuState->settings.uiScale;
    scanCtx.debug = debug;
    scanCtx.useModern = useModern;
    scanCtx.quitRequested = quitRequested;
    scanCtx.autotestQuitInjected = 0;
    scanCtx.returnMenuRescan = 1;
    scanCtx.startedMs = SDL_GetTicks();
    M12_StartupMenu_RescanAllGames(menuState,
                                   m11_scan_progress_callback,
                                   &scanCtx);
    if (!quitRequested || !*quitRequested)
        M12_StartupMenu_FinalizeDeferredScanMessages(menuState);
    if (debug && quitRequested && *quitRequested) {
        fprintf(stderr, "firestaff: return-menu rescan-cancelled by window close\n");
    } else if (debug) {
        static const char* const gameIds[] = {
            "dm1", "csb", "dm2", "nexus", "theron"
        };
        const char* dataDir = M12_AssetStatus_GetDataDir(
            &menuState->assetStatus);
        size_t i;
        fprintf(stderr,
                "firestaff: return-menu rescan-complete data=%s\n",
                dataDir && dataDir[0] ? dataDir : "(unknown)");
        for (i = 0U; i < sizeof(gameIds) / sizeof(gameIds[0]); ++i) {
            fprintf(stderr,
                    "firestaff: return-menu rescan game=%s available=%d\n",
                    gameIds[i],
                    M12_AssetStatus_GameAvailable(&menuState->assetStatus,
                                                  gameIds[i]));
        }
    }
}

static void m11_verbose_original_media(const M12_AssetStatus* status,
                                       const char* gameId) {
    size_t i;
    if (!status || !gameId) return;
    fprintf(stderr, "firestaff: %s available=%s scan-owner=%s\n",
            gameId, M12_AssetStatus_GameAvailable(status, gameId) ? "yes" : "no",
            M12_AssetStatus_GetRuntimeDataDir(status, gameId));
    for (i = 0; i < M12_AssetStatus_GetVersionCount(gameId); ++i) {
        const M12_AssetVersionStatus* version =
            M12_AssetStatus_GetVersion(status, gameId, i);
        if (!version) continue;
        {
            const char* const* candidates =
                M12_AssetStatus_GetVersionCandidateNames(gameId, i);
            size_t candidateIndex;
            for (candidateIndex = 0U; candidates && candidates[candidateIndex];
                 ++candidateIndex) {
                fprintf(stderr,
                        "firestaff: %s edition=%s candidate-file=%s edition-result=%s\n",
                        gameId,
                        version->versionId ? version->versionId : "unknown",
                        candidates[candidateIndex],
                        version->matched ? "matched" : "not matched");
            }
        }
        fprintf(stderr, "firestaff: %s platform=%s edition=%s %s%s%s\n",
                gameId, M12_Architecture_Label(
                    M12_AssetStatus_GetVersionArchitecture(gameId, i)),
                version->versionId ? version->versionId : "unknown",
                version->matched ? "matched" : "not matched",
                version->matched && version->matchedPath[0] ? " source=" : "",
                version->matched ? version->matchedPath : "");
    }
    for (i = 0; i < M12_AssetStatus_GetRequiredFileCount(status, gameId); ++i) {
        const M12_AssetRequiredFileStatus* file =
            M12_AssetStatus_GetRequiredFile(status, gameId, i);
        if (!file) continue;
        fprintf(stderr, "firestaff: %s inventory-file=%s %s%s%s\n", gameId,
                file->roleId ? file->roleId : "unknown",
                file->matched ? "matched" : "missing",
                file->matched && (file->sourcePath[0] || file->matchedPath[0])
                    ? " source=" : "",
                file->sourcePath[0] ? file->sourcePath : file->matchedPath);
    }
}

typedef struct {
    int valid;
    Uint64 startedMs;
    Uint64 previousPerfCounter;
    double previousCpuSeconds;
    int previousCpuAvailable;
    M11_BootProbeReceipt previous;
    int dm2TitleBound;
    int dm2TitleFinished;
    int dm2SwooshActive;
    uint32_t dm2TitleFrameIndex;
    uint32_t dm2FrameTimerARemaining;
    int dm2MacMovieActive;
    int dm2MacMovieComplete;
    int dm2MacMovieRejected;
    uint32_t dm2MacMovieFrameIndex;
} M11_DebugStartupTrace;

static int m11_game_option_slot(const char *gameId);
static int m11_is_supported_game_id(const char *gameId);
static int m11_process_cpu_seconds(double* seconds);
static int m11_process_memory_bytes(uint64_t* residentBytes,
                                   uint64_t* virtualBytes);
static double m11_counter_milliseconds(Uint64 ticks);

static void m11_debug_trace_startup_frame(M11_DebugStartupTrace* trace,
                                          const M11_GameViewState* gameView,
                                          const char* gameId) {
    M11_BootProbeReceipt current;
    const char* tracedGameId = gameId && gameId[0]
        ? gameId : (gameView ? gameView->sourceId : NULL);
    int changed;
    double cpuNow = 0.0;
    double cpuPercent = -1.0;
    uint64_t residentBytes = 0;
    uint64_t virtualBytes = 0;
    int cpuAvailable;
    int memoryAvailable;
    Uint64 perfNow;
    if (!trace || !gameView || !tracedGameId || !gameView->active ||
        !m11_is_supported_game_id(tracedGameId) ||
        !M11_GameView_GetStartupTraceReceipt(gameView, &current)) {
        return;
    }
    changed = !trace->valid ||
        strcmp(current.sourceId, trace->previous.sourceId) != 0 ||
        strcmp(current.startupPhase, trace->previous.startupPhase) != 0 ||
        current.startupActive != trace->previous.startupActive ||
        current.startupFrame != trace->previous.startupFrame ||
        strcmp(current.startupAnimation,
               trace->previous.startupAnimation) != 0 ||
        current.startupAnimationActive != trace->previous.startupAnimationActive ||
        current.startupTitleFrame != trace->previous.startupTitleFrame ||
        current.startupTitleFrameMax != trace->previous.startupTitleFrameMax ||
        current.startupTitleReady != trace->previous.startupTitleReady ||
        current.dm1StartupSwshConsumed !=
            trace->previous.dm1StartupSwshConsumed ||
        current.dm1StartupTitleConsumed !=
            trace->previous.dm1StartupTitleConsumed ||
        current.dm1StartupEntranceConsumed !=
            trace->previous.dm1StartupEntranceConsumed ||
        current.dm1StartupFullGraphicsConsumed !=
            trace->previous.dm1StartupFullGraphicsConsumed ||
        current.levelLoaded != trace->previous.levelLoaded ||
        current.mapIndex != trace->previous.mapIndex ||
        current.partyX != trace->previous.partyX ||
        current.partyY != trace->previous.partyY ||
        current.partyDir != trace->previous.partyDir ||
        current.championCount != trace->previous.championCount ||
        (gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
         (gameView->dm2FmtownsTitleBound !=
              trace->dm2TitleBound ||
          gameView->dm2FmtownsTitleFinished !=
              trace->dm2TitleFinished ||
          gameView->dm2FmtownsSwooshActive !=
              trace->dm2SwooshActive ||
          gameView->dm2FmtownsTitleFrameIndex !=
              trace->dm2TitleFrameIndex ||
          gameView->dm2FmtownsFrameTimerARemaining !=
              trace->dm2FrameTimerARemaining ||
          gameView->dm2MacMovieActive != trace->dm2MacMovieActive ||
          gameView->dm2MacMovieComplete != trace->dm2MacMovieComplete ||
          gameView->dm2MacMovieRejected != trace->dm2MacMovieRejected ||
          gameView->dm2MacMovieDecoder.frame_index !=
              trace->dm2MacMovieFrameIndex));
    if (!changed) return;
    cpuAvailable = m11_process_cpu_seconds(&cpuNow);
    memoryAvailable = m11_process_memory_bytes(
        &residentBytes, &virtualBytes);
    perfNow = SDL_GetPerformanceCounter();
    if (!trace->valid) trace->startedMs = SDL_GetTicks();
    if (trace->valid && trace->previousCpuAvailable && cpuAvailable &&
        cpuNow >= trace->previousCpuSeconds &&
        perfNow > trace->previousPerfCounter) {
        const double elapsedMs = m11_counter_milliseconds(
            perfNow - trace->previousPerfCounter);
        if (elapsedMs > 0.0) {
            cpuPercent = (cpuNow - trace->previousCpuSeconds) *
                         100000.0 / elapsedMs;
        }
    }
    fprintf(stderr,
            "firestaff: startup-frame game=%s elapsed-ms=%llu process-cpu-of-one-core-percent=",
            tracedGameId,
            (unsigned long long)(SDL_GetTicks() - trace->startedMs));
    if (cpuPercent >= 0.0) fprintf(stderr, "%.1f", cpuPercent);
    else fprintf(stderr, "unavailable");
    fprintf(stderr,
            " process-rss-bytes=");
    if (memoryAvailable) fprintf(stderr, "%llu",
        (unsigned long long)residentBytes);
    else fprintf(stderr, "unavailable");
    fprintf(stderr, " process-virtual-bytes=");
    if (memoryAvailable) fprintf(stderr, "%llu",
        (unsigned long long)virtualBytes);
    else fprintf(stderr, "unavailable");
    fprintf(stderr,
            " source=%s phase=%s active=%d frame=%d animation=%s animation-active=%d title-frame=%d/%d title-ready=%d dm1-phases=%d%d%d%d dm2-fmtowns-title={bound:%d finished:%d rejected:%d swoosh:%d frame:%u/%u ticks-remaining:%u} dm2-mac-movie={active:%d complete:%d rejected:%d frame:%u} level-loaded=%d map=%d party=%d,%d dir=%d champions=%d\n",
            current.sourceId[0] ? current.sourceId : "unknown",
            current.startupPhase[0] ? current.startupPhase : "unknown",
            current.startupActive,
            current.startupFrame,
            current.startupAnimation[0] ? current.startupAnimation : "none",
            current.startupAnimationActive,
            current.startupTitleFrame,
            current.startupTitleFrameMax,
            current.startupTitleReady,
            current.dm1StartupSwshConsumed,
            current.dm1StartupTitleConsumed,
            current.dm1StartupEntranceConsumed,
            current.dm1StartupFullGraphicsConsumed,
            gameView->dm2FmtownsTitleBound,
            gameView->dm2FmtownsTitleFinished,
            gameView->dm2FmtownsTitleRejected,
            gameView->dm2FmtownsSwooshActive,
            gameView->dm2FmtownsTitleFrameIndex,
            gameView->dm2FmtownsFrameCount,
            gameView->dm2FmtownsFrameTimerARemaining,
            gameView->dm2MacMovieActive,
            gameView->dm2MacMovieComplete,
            gameView->dm2MacMovieRejected,
            gameView->dm2MacMovieDecoder.frame_index,
            current.levelLoaded,
            current.mapIndex,
            current.partyX,
            current.partyY,
            current.partyDir,
            current.championCount);
    trace->previousPerfCounter = perfNow;
    trace->previousCpuSeconds = cpuNow;
    trace->previousCpuAvailable = cpuAvailable;
    trace->previous = current;
    if (gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT) {
        trace->dm2TitleBound = gameView->dm2FmtownsTitleBound;
        trace->dm2TitleFinished = gameView->dm2FmtownsTitleFinished;
        trace->dm2SwooshActive = gameView->dm2FmtownsSwooshActive;
        trace->dm2TitleFrameIndex = gameView->dm2FmtownsTitleFrameIndex;
        trace->dm2FrameTimerARemaining =
            gameView->dm2FmtownsFrameTimerARemaining;
        trace->dm2MacMovieActive = gameView->dm2MacMovieActive;
        trace->dm2MacMovieComplete = gameView->dm2MacMovieComplete;
        trace->dm2MacMovieRejected = gameView->dm2MacMovieRejected;
        trace->dm2MacMovieFrameIndex =
            gameView->dm2MacMovieDecoder.frame_index;
    }
    trace->valid = 1;
}

static void m11_debug_log_launch_selection(
    const M12_StartupMenuState* menuState,
    const M11_GameViewState* gameView,
    const char* launchMode) {
    int slot;
    int versionIndex;
    const M12_AssetVersionStatus* version;
    const char* gameId;
    if (!menuState || !gameView || !launchMode) return;
    gameId = gameView->sourceId;
    if (!m11_is_supported_game_id(gameId)) return;
    slot = m11_game_option_slot(gameId);
    versionIndex = menuState->gameOptions[slot].versionIndex;
    version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&menuState->assetStatus, gameId,
                                     (size_t)versionIndex)
        : NULL;
    fprintf(stderr,
            "firestaff: launch phase=game-handoff mode=%s game=%s platform=%s edition=%s source=%s\n",
            launchMode,
            gameId,
            version ? M12_Architecture_Label(
                M12_AssetStatus_GetVersionArchitecture(
                    gameId, (size_t)versionIndex)) : "unknown",
            version && version->versionId ? version->versionId : "unknown",
            version && version->matchedPath[0]
                ? version->matchedPath : "unknown");
}

static int g_m11_debug_startup_detail = 0;

typedef struct {
    int hasLastProgress;
    Uint64 startedMs;
    Uint64 lastProgressMs;
    M12_AssetScanProgress lastProgress;
} M11_DebugScanProgressTrace;

static M11_DebugScanProgressTrace g_m11_debug_scan_progress_trace;

static void m11_debug_reset_scan_progress_trace(void) {
    memset(&g_m11_debug_scan_progress_trace, 0,
           sizeof(g_m11_debug_scan_progress_trace));
}

static void m11_debug_log_scan_progress(const M12_AssetScanProgress* progress,
                                        int languageIndex) {
    M11_DebugScanProgressTrace* trace = &g_m11_debug_scan_progress_trace;
    Uint64 nowMs;
    const char* localizedGame;
    const char* localizedTask;
    if (!g_m11_debug_startup_detail || !progress) return;
    nowMs = SDL_GetTicks();
    if (!trace->hasLastProgress ||
        progress->completedSteps != trace->lastProgress.completedSteps ||
        progress->currentStepPercent !=
            trace->lastProgress.currentStepPercent ||
        strcmp(progress->currentTask, trace->lastProgress.currentTask) != 0 ||
        strcmp(progress->currentGameId,
               trace->lastProgress.currentGameId) != 0 ||
        strcmp(progress->currentPath, trace->lastProgress.currentPath) != 0) {
        if (!trace->hasLastProgress) {
            trace->startedMs = nowMs;
            trace->lastProgressMs = nowMs;
        }
        localizedGame = progress->currentGameId[0]
            ? M12_StartupMenu_GameDisplayTitleForLocale(
                  languageIndex, progress->currentGameId)
            : "";
        localizedTask = M12_StartupMenu_ScanTaskDisplayForLocale(
            languageIndex, progress->currentTask);
        fprintf(stderr,
                "firestaff: scan elapsed-ms=%llu delta-ms=%llu progress=%zu%% game=%s game-label=\"%s\" task=%s task-label=\"%s\" path=%s\n",
                (unsigned long long)(nowMs - trace->startedMs),
                (unsigned long long)(nowMs - trace->lastProgressMs),
                progress->totalSteps > 0U
                    ? (progress->completedSteps * 100U +
                       progress->currentStepPercent) / progress->totalSteps
                    : 0U,
                progress->currentGameId[0] ? progress->currentGameId : "all",
                localizedGame ? localizedGame : "",
                progress->currentTask,
                localizedTask ? localizedTask : "",
                progress->currentPath[0] ? progress->currentPath : "(none)");
        trace->lastProgressMs = nowMs;
        trace->lastProgress = *progress;
        trace->hasLastProgress = 1;
    }
    if (progress->complete || progress->cancelled) {
        trace->hasLastProgress = 0;
    }
}

static void m11_draw_intro_progress_bar(unsigned char* rgba,
                                        int w, int h,
                                        const M12_AssetScanProgress* progress,
                                        Uint64 elapsedMs,
                                        int languageIndex) {
    int panelW, panelH, panelX, panelY;
    int barW, barH, barX, barY, fillW;
    size_t pct = 0;
    char label[128];
    int labelWidth = 0;
    int fontSize;
    (void)elapsedMs;
    if (!rgba || w <= 0 || h <= 0) return;
    if (progress && progress->totalSteps > 0) {
        pct = (progress->completedSteps * 100U +
               progress->currentStepPercent) / progress->totalSteps;
        if (pct > 100) pct = 100;
    }
    panelW = w < 440 ? w - 32 : 408;
    panelH = 48;
    panelX = (w - panelW) / 2;
    /* Keep the overlay above the menu footer; the startup animation and
     * launcher share this renderer but use different screen heights. */
    panelY = h * 74 / 100;
    if (panelY < 0) panelY = 0;
    {
        int px, py;
        for (py = panelY; py < panelY + panelH && py < h; ++py) {
            for (px = panelX; px < panelX + panelW && px < w; ++px) {
                int idx = (py * w + px) * 4;
                rgba[idx] = (unsigned char)((rgba[idx] * 2U + 16U) / 3U);
                rgba[idx+1] = (unsigned char)((rgba[idx+1] * 2U + 17U) / 3U);
                rgba[idx+2] = (unsigned char)((rgba[idx+2] * 2U + 20U) / 3U);
                rgba[idx+3] = 255;
            }
        }
    }
    barW = panelW - 32;
    barH = 4;
    barX = (w - barW) / 2;
    barY = panelY + panelH - 12;
    {
        int bx, by;
        for (by = barY; by < barY + barH && by < h; ++by) {
            for (bx = barX; bx < barX + barW && bx < w; ++bx) {
                int idx = (by * w + bx) * 4;
                rgba[idx] = 62; rgba[idx+1] = 63; rgba[idx+2] = 66; rgba[idx+3] = 255;
            }
        }
    }
    fillW = (int)((size_t)barW * pct / 100);
    if (fillW > 0) {
        int bx, by;
        for (by = barY; by < barY + barH && by < h; ++by) {
            for (bx = barX; bx < barX + fillW && bx < w; ++bx) {
                int idx = (by * w + bx) * 4;
                rgba[idx] = 200; rgba[idx+1] = 202; rgba[idx+2] = 207; rgba[idx+3] = 255;
            }
        }
    }
    if (progress && progress->currentGameId[0] != '\0') {
        snprintf(label, sizeof(label), "%s  %s  %zu%%",
                 M12_StartupMenu_TranslateForLocale(languageIndex,
                                                    "SCANNING GAME DATA"),
                 M12_StartupMenu_GameDisplayTitleForLocale(
                     languageIndex, progress->currentGameId), pct);
    } else {
        snprintf(label, sizeof(label), "%s  %zu%%",
                 M12_StartupMenu_TranslateForLocale(languageIndex,
                                                    "SCANNING GAME DATA"), pct);
    }
    fontSize = M11_UIScale_Apply(13);
    while (fontSize > 8 &&
           (!m11_ttf_measure_string(label, fontSize, &labelWidth) ||
            labelWidth > panelW - 20)) {
        fontSize -= 1;
        labelWidth = 0;
    }
    if (labelWidth == 0)
        (void)m11_ttf_measure_string(label, fontSize, &labelWidth);
    if (labelWidth > 0) {
        int labelX = (w - labelWidth) / 2;
        if (labelX < panelX + 10) labelX = panelX + 10;
        (void)m11_ttf_render_rgba_string(rgba, w, h, labelX,
                                         panelY + 8, label, fontSize,
                                         236, 238, 240);
    }
}

static int m11_play_firestaff_startup_intro(M12_StartupMenuState* menuState) {
    unsigned char* rgba;
    char resourcePath[1024];
    const char* basePath;
    Uint64 started;
    int scanActive = 0;
    int autotestQuitInjected = 0;

    rgba = (unsigned char*)malloc((size_t)M12_STARTUP_INTRO_WIDTH *
                                  (size_t)M12_STARTUP_INTRO_HEIGHT * 4U);
    if (!rgba) return 0;
    if (menuState) {
        M11_UIScale_SetPercent(menuState->settings.uiScale);
    }
    (void)M11_Render_SetPresentationFillWindow(1);
    /* The generated dungeon composition is the preferred intro art. Keep
     * the established PPM as a compatible fallback for partial installs. */
    (void)M12_StartupIntro_LoadBackground(
        "assets/branding/firestaff-startup-dungeon-v2.ppm");
    (void)M12_StartupIntro_LoadBackground("assets/branding/firestaff-startup-intro.ppm");
    basePath = SDL_GetBasePath();
    if (basePath) {
        snprintf(resourcePath, sizeof(resourcePath),
                 "%sfirestaff-startup-dungeon-v2.ppm", basePath);
        (void)M12_StartupIntro_LoadBackground(resourcePath);
        snprintf(resourcePath, sizeof(resourcePath), "%sfirestaff-startup-intro.ppm", basePath);
        (void)M12_StartupIntro_LoadBackground(resourcePath);
        snprintf(resourcePath, sizeof(resourcePath),
                 "%s../Resources/firestaff-startup-dungeon-v2.ppm", basePath);
        (void)M12_StartupIntro_LoadBackground(resourcePath);
        snprintf(resourcePath, sizeof(resourcePath), "%s../Resources/firestaff-startup-intro.ppm", basePath);
        (void)M12_StartupIntro_LoadBackground(resourcePath);
        snprintf(resourcePath, sizeof(resourcePath),
                 "%s../share/firestaff/firestaff-startup-dungeon-v2.ppm", basePath);
        (void)M12_StartupIntro_LoadBackground(resourcePath);
        snprintf(resourcePath, sizeof(resourcePath), "%s../share/firestaff/firestaff-startup-intro.ppm", basePath);
        (void)M12_StartupIntro_LoadBackground(resourcePath);
#if !SDL_VERSION_ATLEAST(3, 0, 0)
        /* SDL2 allocates the base path; SDL3 returns a library-owned path. */
        SDL_free((void*)basePath);
#endif
    }
    (void)M12_StartupIntro_LoadBackground(
        "/usr/share/firestaff/firestaff-startup-dungeon-v2.ppm");
    (void)M12_StartupIntro_LoadBackground("/usr/share/firestaff/firestaff-startup-intro.ppm");
#ifdef FIRESTAFF_SOURCE_DIR
    snprintf(resourcePath, sizeof(resourcePath),
             FIRESTAFF_SOURCE_DIR "/assets/branding/firestaff-startup-dungeon-v2.ppm");
    (void)M12_StartupIntro_LoadBackground(resourcePath);
    snprintf(resourcePath, sizeof(resourcePath),
             FIRESTAFF_SOURCE_DIR "/assets/branding/firestaff-startup-intro.ppm");
    (void)M12_StartupIntro_LoadBackground(resourcePath);
#endif

    if (menuState && menuState->deferredScanPending) {
        scanActive = M12_StartupMenu_BeginDeferredScan(menuState);
    }

    started = SDL_GetTicks();
    for (;;) {
        Uint64 elapsed = SDL_GetTicks() - started;
        uint32_t effectiveDuration = M12_STARTUP_INTRO_DURATION_MS;
        M12_StartupIntro_Render(rgba,
                                M12_STARTUP_INTRO_WIDTH,
                                M12_STARTUP_INTRO_HEIGHT,
                                (uint32_t)(elapsed < effectiveDuration
                                               ? elapsed
                                               : effectiveDuration - 1),
                                effectiveDuration,
                                FIRESTAFF_VERSION_NUMBER);
        if (scanActive) {
            M12_AssetScanProgress snap;
            (void)M12_StartupMenu_GetDeferredScanProgress(menuState, &snap);
            m11_draw_intro_progress_bar(rgba,
                                        M12_STARTUP_INTRO_WIDTH,
                                        M12_STARTUP_INTRO_HEIGHT,
                                        &snap,
                                        elapsed,
                                        menuState ? menuState->settings.languageIndex : 0);
            m11_debug_log_scan_progress(
                &snap, menuState ? menuState->settings.languageIndex : 0);
            {
                const char* quitAny =
                    getenv("FIRESTAFF_AUTOTEST_QUIT_DURING_SCAN");
                const char* quitDm2 =
                    getenv("FIRESTAFF_AUTOTEST_QUIT_DURING_DM2_IMAGE_INFLATE");
                int requestAny = quitAny && strcmp(quitAny, "1") == 0;
                int requestDm2 = quitDm2 && strcmp(quitDm2, "1") == 0 &&
                    strcmp(snap.currentGameId, "dm2") == 0 &&
                    strcmp(snap.currentTask,
                           "reading original DM2 disc image") == 0 &&
                    snap.currentStepPercent > 0U;
                if (!autotestQuitInjected && getenv("FIRESTAFF_AUTOTEST") &&
                    (requestAny || requestDm2)) {
                    SDL_Event event;
                    SDL_Window* window = M11_Render_GetWindow();
                    memset(&event, 0, sizeof(event));
                    if (window) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
                        event.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
                        event.window.windowID = SDL_GetWindowID(window);
#else
                        event.type = SDL_WINDOWEVENT;
                        event.window.windowID = SDL_GetWindowID(window);
                        event.window.event = SDL_WINDOWEVENT_CLOSE;
#endif
                        if (SDL_PushEvent(&event)) {
                            autotestQuitInjected = 1;
                            (void)FSP_SetEnv(
                                "FIRESTAFF_AUTOTEST_SCAN_QUIT_INJECTED",
                                "1", 1);
                            if (g_m11_debug_startup_detail) {
                                fprintf(stderr, requestDm2
                                    ? "firestaff: autotest injected window-close during DM2 disc image inflate\n"
                                    : "firestaff: autotest injected window-close during startup intro asset scan\n");
                            }
                        }
                    }
                }
            }
        }
        (void)M11_Render_PresentRGBA(rgba,
                                     M12_STARTUP_INTRO_WIDTH,
                                     M12_STARTUP_INTRO_HEIGHT);
        if (M11_Render_PumpEvents()) {
            /* Cancellation is observed at the next scan progress boundary;
             * state destruction joins the isolated worker during cleanup. */
            M12_StartupMenu_CancelDeferredScan(menuState);
            free(rgba);
            return 1;
        }
        if (elapsed >= M12_STARTUP_INTRO_DURATION_MS) break;
        SDL_Delay(33U);
    }
    free(rgba);
    return 0;
}

int M11_GameView_PresentationIndexedScale(int presentationMode) {
    return presentationMode == M12_PRESENTATION_V20_FILTERED ? 2 : 1;
}

int M11_GameView_PresentationTarget(int presentationMode,
                                    int presentationWidth,
                                    int presentationHeight,
                                    int* outW,
                                    int* outH) {
    int targetW = M11_FB_WIDTH;
    int targetH = M11_FB_HEIGHT;
    if (presentationMode == M12_PRESENTATION_V20_FILTERED) {
        targetW = M11_FB_WIDTH * 2;
        targetH = M11_FB_HEIGHT * 2;
    } else if ((presentationMode == M12_PRESENTATION_V1_ORIGINAL ||
                M12_PresentationMode_AllowsResolutionChoice(presentationMode)) &&
               presentationWidth > 0 &&
               presentationHeight > 0) {
        /* Original keeps source pixels intact but may still use a selected
         * host presentation target.  Input performs the matching inverse
         * map before it reaches ReDMCSB's 320x200 command rectangles. */
        targetW = presentationWidth;
        targetH = presentationHeight;
    }
    if (outW) {
        *outW = targetW;
    }
    if (outH) {
        *outH = targetH;
    }
    return targetW != M11_FB_WIDTH || targetH != M11_FB_HEIGHT;
}

int M11_ResolveGameScaleFilterForPresentation(int presentationMode,
                                              int requestedScaleFilter) {
    if (presentationMode == M12_PRESENTATION_V1_ORIGINAL ||
        presentationMode == M12_PRESENTATION_V20_FILTERED) {
        return M11_SCALE_FILTER_NEAREST;
    }
    if (requestedScaleFilter != M11_SCALE_FILTER_NEAREST &&
        requestedScaleFilter != M11_SCALE_FILTER_LINEAR) {
        return M11_SCALE_FILTER_NEAREST;
    }
    return requestedScaleFilter;
}

static int m11_map_window_pointer_to_game_source(
    const M11_GameViewState* gameView,
    int windowX,
    int windowY,
    int* outX,
    int* outY)
{
    int framebufferX;
    int framebufferY;

    if (!gameView || !outX || !outY ||
        !M11_Render_MapWindowToFramebuffer(windowX, windowY,
                                           &framebufferX, &framebufferY)) {
        return 0;
    }
    /* F31J C06 owns an observed native 640x400 selector.  Its coordinates
     * are already the renderer's source coordinates; M11's 320x200 mapping
     * must not halve its hit rectangles. */
    if (M11_GameView_GetCsbFmtownsUtilityJapaneseFrame(
            gameView, NULL, NULL, NULL)) {
        *outX = framebufferX;
        *outY = framebufferY;
        return 1;
    }
    /* V1_ORIGINAL returns 0 (pass-through): framebuffer coords ARE
     * source coords (320x200).  Only fail on NULL pointers. */
    (void)M11_MapPresentedGamePointToSourceForPresentation(
            gameView->presentationMode,
            gameView->presentationWidth,
            gameView->presentationHeight,
            &framebufferX,
            &framebufferY);
    *outX = framebufferX;
    *outY = framebufferY;
    return 1;
}

static void m11_sync_touch_gesture_gate(const M11_GameViewState* gameView,
                                        const M12_StartupMenuState* menuState)
{
    FsGgGameId game;
    int enabled;

    if (!fs_gesture_gate_is_initialized()) {
        (void)fs_gesture_gate_init();
    }
    if (!gameView || !gameView->active || !menuState) {
        fs_gesture_recognizer_reset();
        return;
    }
    switch (gameView->sourceKind) {
    case M11_GAME_SOURCE_CSB_BOOT: game = FS_GG_GAME_CSB; break;
    case M11_GAME_SOURCE_DM2_BOOT: game = FS_GG_GAME_DM2; break;
    case M11_GAME_SOURCE_NEXUS_DGN: game = FS_GG_GAME_NEXUS; break;
    case M11_GAME_SOURCE_THERON_TRACK02: game = FS_GG_GAME_THERON; break;
    default: game = FS_GG_GAME_DM1; break;
    }
    enabled = menuState->settings.touchControlsIndex > 0 &&
        (menuState->settings.inputModeIndex == 0 ||
         menuState->settings.inputModeIndex == 2);
    if (fs_gesture_gate_active_game() != game) {
        fs_gesture_recognizer_reset();
        (void)fs_gesture_gate_set_active_game(game);
    }
    (void)fs_gesture_gate_set_enabled(game, enabled);
    if (!enabled) {
        fs_gesture_recognizer_reset();
    }
}

static int m11_map_normalized_touch_to_game_source(
    const M11_GameViewState* gameView,
    float normalizedX,
    float normalizedY,
    int* outX,
    int* outY)
{
    int windowX;
    int windowY;
    int windowW = M11_Render_GetWindowWidth();
    int windowH = M11_Render_GetWindowHeight();
    if (normalizedX < 0.0f || normalizedX > 1.0f ||
        normalizedY < 0.0f || normalizedY > 1.0f ||
        windowW <= 0 || windowH <= 0) {
        return 0;
    }
    windowX = (int)(normalizedX * (float)windowW);
    windowY = (int)(normalizedY * (float)windowH);
    if (windowX >= windowW) windowX = windowW - 1;
    if (windowY >= windowH) windowY = windowH - 1;
    return m11_map_window_pointer_to_game_source(gameView, windowX, windowY,
                                                  outX, outY);
}

static void m11_sync_source_object_cursor(const M11_GameViewState* gameView) {
    static int hostCursorHidden = 0;
    int shouldHide;

    shouldHide = gameView && gameView->active &&
        ((gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN &&
          gameView->nexusState.hand_pointer_active) ||
         gameView->sourceKind == M11_GAME_SOURCE_BUILTIN_CATALOG ||
         gameView->sourceKind == M11_GAME_SOURCE_CUSTOM_DUNGEON ||
         gameView->sourceKind == M11_GAME_SOURCE_DIRECT_DUNGEON ||
         gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT) &&
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(gameView) != THING_NONE;
    if (shouldHide == hostCursorHidden) {
        return;
    }
    if (shouldHide) {
        SDL_HideCursor();
    } else {
        SDL_ShowCursor();
    }
    hostCursorHidden = shouldHide;
}

static int m11_dm1_v20_presentation_active(const M11_GameViewState* gameView) {
    return gameView &&
        gameView->presentationMode == M12_PRESENTATION_V20_FILTERED &&
        (gameView->sourceKind == M11_GAME_SOURCE_BUILTIN_CATALOG ||
         gameView->sourceKind == M11_GAME_SOURCE_CUSTOM_DUNGEON ||
         gameView->sourceKind == M11_GAME_SOURCE_DIRECT_DUNGEON);
}

static int m11_present_game_frame(const M11_GameViewState* gameView,
                                  const unsigned char** outPresentedFrame) {
    static unsigned char dm2_mac_movie_rgba[M11_FB_WIDTH * M11_FB_HEIGHT * 4];
    const uint8_t *csb_fmtowns_japanese_frame = NULL;
    int csb_fmtowns_japanese_width = 0;
    int csb_fmtowns_japanese_height = 0;
    int scale = M11_GameView_PresentationIndexedScale(
        gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL);
    int specialPalette =
        M11_GameView_GetPresentationSpecialPalette(gameView);
    int targetW = M11_FB_WIDTH;
    int targetH = M11_FB_HEIGHT;

    /* Restore source/game presentation before the first runtime frame after
     * a launcher click. */
    (void)M11_Render_SetPresentationFillWindow(0);
    int requestedFilter = M11_Render_GetScaleFilter();
    int effectiveFilter = M11_ResolveGameScaleFilterForPresentation(
        gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL,
        requestedFilter);
    int restoreFilter = 0;
    int result;
    const unsigned char* presented_frame = M11_Render_GetFramebuffer();
    int csb_v20_active = gameView &&
        gameView->presentationMode == M12_PRESENTATION_V20_FILTERED &&
        gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT;

    /* The original object cursor is painted into the source framebuffer.
     * Keep the host arrow from covering it while an item is in G4055. */
    m11_sync_source_object_cursor(gameView);
    M11_GameView_DrawNexusPointerOverlay(
        gameView, M11_Render_GetFramebuffer(), M11_FB_WIDTH, M11_FB_HEIGHT);

    if (outPresentedFrame) {
        *outPresentedFrame = NULL;
    }
    /* QuickTime Cinepak is a 24-bit true-colour source. Keep its decoded
     * channels intact instead of reducing every movie frame to RGB332 before
     * sending it through the indexed game palette. */
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
        gameView->dm2MacMovieActive) {
        size_t pixel;
        for (pixel = 0u; pixel < (size_t)M11_FB_WIDTH * M11_FB_HEIGHT; ++pixel) {
            dm2_mac_movie_rgba[pixel * 4u + 0u] =
                gameView->dm2MacMovieDecoder.rgb24[pixel * 3u + 0u];
            dm2_mac_movie_rgba[pixel * 4u + 1u] =
                gameView->dm2MacMovieDecoder.rgb24[pixel * 3u + 1u];
            dm2_mac_movie_rgba[pixel * 4u + 2u] =
                gameView->dm2MacMovieDecoder.rgb24[pixel * 3u + 2u];
            dm2_mac_movie_rgba[pixel * 4u + 3u] = 0xffu;
        }
        result = M11_Render_PresentRGBA(dm2_mac_movie_rgba,
                                        M11_FB_WIDTH, M11_FB_HEIGHT);
        if (result == M11_RENDER_OK && outPresentedFrame)
            *outPresentedFrame = presented_frame;
        return result == M11_RENDER_OK;
    }
    /* C06 F31J is a Towns-native 640x400 page, not an M11 320x200 source
     * frame. Present it directly and exclude it from PC3.4 startup receipts. */
    if (M11_GameView_GetCsbFmtownsUtilityJapaneseFrame(
            gameView, &csb_fmtowns_japanese_frame,
            &csb_fmtowns_japanese_width, &csb_fmtowns_japanese_height)) {
        M11_Render_SetV2PresentationActive(0);
        M11_Render_SetModernPresentationActive(0);
        if (effectiveFilter != requestedFilter) {
            M11_Render_SetScaleFilter(effectiveFilter);
            restoreFilter = 1;
        }
        result = M11_Render_PresentIndexed(csb_fmtowns_japanese_frame,
                                           csb_fmtowns_japanese_width,
                                           csb_fmtowns_japanese_height);
        if (restoreFilter) M11_Render_SetScaleFilter(requestedFilter);
        return result == M11_RENDER_OK;
    }
    /* CSB's title, PRESENTS, CHAOS/STRIKES and Entrance pages own special
     * palette indices.  V2.0 cleanup is valid for the normal game surface,
     * but rewriting those indices corrupts their source palette mapping.
     *
     * At runtime, CSBWin's source viewport is exactly (48,33) 224x136;
     * the remainder is source-owned panel/HUD material.  Filter only a
     * detached viewport copy so palette interpolation and dither cleanup
     * cannot fragment C013's cyan controls or mutate panel text. */
    if (csb_v20_active && presented_frame && specialPalette < 0) {
        enum {
            CSB_RUNTIME_VIEWPORT_X = 48,
            CSB_RUNTIME_VIEWPORT_Y = 33,
            CSB_RUNTIME_VIEWPORT_WIDTH = 224,
            CSB_RUNTIME_VIEWPORT_HEIGHT = 136
        };
        static unsigned char csb_v20_scratch[M11_FB_BYTES];
        static unsigned char csb_v20_viewport[
            CSB_RUNTIME_VIEWPORT_WIDTH * CSB_RUNTIME_VIEWPORT_HEIGHT];
        int y;

        memcpy(csb_v20_scratch, presented_frame, sizeof(csb_v20_scratch));
        for (y = 0; y < CSB_RUNTIME_VIEWPORT_HEIGHT; ++y) {
            memcpy(csb_v20_viewport +
                       (size_t)y * CSB_RUNTIME_VIEWPORT_WIDTH,
                   csb_v20_scratch +
                       (size_t)(CSB_RUNTIME_VIEWPORT_Y + y) * M11_FB_WIDTH +
                       CSB_RUNTIME_VIEWPORT_X,
                   CSB_RUNTIME_VIEWPORT_WIDTH);
        }
        (void)csb_v2_filter_chain_apply_indexed(
            csb_v20_viewport, CSB_RUNTIME_VIEWPORT_WIDTH,
            CSB_RUNTIME_VIEWPORT_HEIGHT);
        for (y = 0; y < CSB_RUNTIME_VIEWPORT_HEIGHT; ++y) {
            memcpy(csb_v20_scratch +
                       (size_t)(CSB_RUNTIME_VIEWPORT_Y + y) * M11_FB_WIDTH +
                       CSB_RUNTIME_VIEWPORT_X,
                   csb_v20_viewport +
                       (size_t)y * CSB_RUNTIME_VIEWPORT_WIDTH,
                   CSB_RUNTIME_VIEWPORT_WIDTH);
        }
        presented_frame = csb_v20_scratch;
    }

    /* M12 persists V2.0 preferences globally, but those post-filters are
     * only valid for the DM1 V2.0 framebuffer route. */
    /* The renderer must retain the selected Modern render target instead of
     * CPU-expanding it to the host drawable.  That keeps F10's resolution
     * choice observable and bounds the filter chain on HiDPI displays.  The
     * per-game filter owner still decides which effects are enabled. */
    M11_Render_SetV2PresentationActive(m11_dm1_v20_presentation_active(gameView));
    M11_Render_SetModernPresentationActive(
        gameView && gameView->presentationMode != M12_PRESENTATION_V1_ORIGINAL);
    /* ReDMCSB DUNVIEW.C:3619-3638 draws DM1 inscriptions as hard-edged
     * M648 8x8 glyphs into the 320x200 viewport.  If the launcher's global
     * scaling filter is LINEAR, SDL smooths those glyphs during window
     * presentation and the wall text becomes unreadable.  V1 original mode
     * therefore presents with nearest-neighbor regardless of the enhanced-mode
     * filter setting.  V2.0 still shows the same source glyphs through the
     * 320x200 -> 640x400 indexed path, so it also keeps nearest presentation;
     * V2.1/V2.2 keep honoring the user setting. */
    if (effectiveFilter != requestedFilter) {
        M11_Render_SetScaleFilter(effectiveFilter);
        restoreFilter = 1;
    }
    if (specialPalette >= 0) {
        if (gameView &&
            gameView->presentationMode == M12_PRESENTATION_V21_UPSCALED) {
            (void)M11_GameView_PresentationTarget(
                gameView->presentationMode,
                gameView->presentationWidth,
                gameView->presentationHeight,
                &targetW,
                &targetH);
            result = M11_Render_PresentEpxIndexedToResolutionWithSpecialPalette(
                presented_frame, M11_FB_WIDTH, M11_FB_HEIGHT,
                targetW, targetH, specialPalette);
            if (restoreFilter) {
                M11_Render_SetScaleFilter(requestedFilter);
            }
            if (result == M11_RENDER_OK && outPresentedFrame) {
                *outPresentedFrame = presented_frame;
            }
            return result == M11_RENDER_OK;
        }
        if (M11_GameView_PresentationTarget(
                gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL,
                gameView ? gameView->presentationWidth : 0,
                gameView ? gameView->presentationHeight : 0,
                &targetW,
                &targetH)) {
            result = M11_Render_PresentIndexedToResolutionWithSpecialPalette(
                presented_frame,
                M11_FB_WIDTH,
                M11_FB_HEIGHT,
                targetW,
                targetH,
                specialPalette);
        } else {
            result = M11_Render_PresentIndexedWithSpecialPalette(
                presented_frame,
                M11_FB_WIDTH,
                M11_FB_HEIGHT,
                specialPalette);
        }
        if (restoreFilter) {
            M11_Render_SetScaleFilter(requestedFilter);
        }
        if (result == M11_RENDER_OK && outPresentedFrame) {
            *outPresentedFrame = presented_frame;
        }
        return result == M11_RENDER_OK;
    }
    if (gameView &&
        gameView->presentationMode == M12_PRESENTATION_V21_UPSCALED) {
        int epxScale = 2;
        if (gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT) {
            epxScale = csb_v2_upscale_get_scale();
            if (csb_v2_upscale_get_bilinear()) {
                (void)M11_Render_SetScaleFilter(M11_SCALE_FILTER_LINEAR);
            }
        }
        (void)M11_GameView_PresentationTarget(gameView->presentationMode,
                                               gameView->presentationWidth,
                                               gameView->presentationHeight,
                                               &targetW,
                                               &targetH);
        result = M11_Render_PresentEpxIndexedToResolutionAtScale(
            presented_frame, M11_FB_WIDTH, M11_FB_HEIGHT, targetW, targetH,
            epxScale);
        if (restoreFilter) {
            M11_Render_SetScaleFilter(requestedFilter);
        }
        if (result == M11_RENDER_OK && outPresentedFrame) {
            *outPresentedFrame = presented_frame;
        }
        return result == M11_RENDER_OK;
    }
    if (scale > 1) {
        result = M11_Render_PresentScaledIndexed(presented_frame,
                                                 M11_FB_WIDTH,
                                                 M11_FB_HEIGHT,
                                                 scale);
        if (restoreFilter) {
            M11_Render_SetScaleFilter(requestedFilter);
        }
        if (result == M11_RENDER_OK && outPresentedFrame) {
            *outPresentedFrame = presented_frame;
        }
        return result == M11_RENDER_OK;
    }
    if (M11_GameView_PresentationTarget(gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL,
                                         gameView ? gameView->presentationWidth : 0,
                                         gameView ? gameView->presentationHeight : 0,
                                         &targetW,
                                         &targetH)) {
        result = M11_Render_PresentIndexedToResolution(presented_frame,
                                                       M11_FB_WIDTH,
                                                       M11_FB_HEIGHT,
                                                       targetW,
                                                       targetH);
        if (restoreFilter) {
            M11_Render_SetScaleFilter(requestedFilter);
        }
        if (result == M11_RENDER_OK && outPresentedFrame) {
            *outPresentedFrame = presented_frame;
        }
        return result == M11_RENDER_OK;
    }
    result = M11_Render_Present();
    if (restoreFilter) {
        M11_Render_SetScaleFilter(requestedFilter);
    }
    if (result == M11_RENDER_OK && outPresentedFrame) {
        *outPresentedFrame = presented_frame;
    }
    return result == M11_RENDER_OK;
}

static unsigned int g_m11_sync_launch_serial;

static void m11_publish_dm1_hoc_presented_capture_to_m12(
    M11_GameViewState* gameView,
    M12_StartupMenuState* menuState) {
    if (!gameView || !menuState || strcmp(gameView->sourceId, "dm1") != 0) {
        return;
    }

    /* This is M12's startup observation, not a current-frame render gate.
     * Explicit boot/capture getters still inspect fresh presented pixels.
     * A failed or cleared publication must remain retryable, and only this
     * menu instance's successfully imported launch receipt may be reused. */
    if ((!menuState->dm1HoCPresentedCaptureLaunchSerial ||
         menuState->dm1HoCPresentedCaptureLaunchSerial != g_m11_sync_launch_serial ||
         !menuState->dm1HoCPresentedCaptureReceipt.presentedCaptureReady) &&
        M11_Render_HasHostPresentationWindow()) {
        M11_BootProbeReceipt boot;
        DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34 export_receipt;
        M12_DM1HoCPresentedCaptureReceipt capture;
        memset(&export_receipt, 0, sizeof(export_receipt));
        if (M11_GameView_GetBootProbeReceipt(gameView, &boot) &&
            strcmp(boot.sourceId, "dm1") == 0 &&
            dm1_v1_startup_hoc_presented_capture_host_export_from_boot_summary_pc34(
                &boot.dm1HoCBootSummary, &export_receipt) &&
            export_receipt.ready) {
            memset(&capture, 0, sizeof(capture));
            capture.handled = export_receipt.handled;
            capture.presentedCaptureReady = export_receipt.presented_capture_ready;
            capture.hostWindowPresent = export_receipt.host_window_present;
            capture.capturedFromMacWindow = export_receipt.captured_from_mac_window;
            capture.capturedFromReleaseApp = export_receipt.captured_from_release_app;
            capture.width = export_receipt.width;
            capture.height = export_receipt.height;
            capture.byteCount = export_receipt.byte_count;
            capture.framebufferHash = export_receipt.framebuffer_hash;
            capture.consumerMask = export_receipt.consumer_mask;
            capture.chainHash = export_receipt.chain_hash;
            if (M12_StartupMenu_SetDM1HoCPresentedCaptureReceipt(menuState, &capture) &&
                menuState->dm1HoCPresentedCaptureReceipt.presentedCaptureReady) {
                menuState->dm1HoCPresentedCaptureLaunchSerial = g_m11_sync_launch_serial;
            }
        }
    }

    /* The source HoC CD event remains live on every present; freezing the
     * startup capture must not suppress a later required track transition. */
    if (gameView->dm1FmtownsStartupReceiptValid &&
        menuState->dm1HoCPresentedCaptureLaunchSerial != 0U &&
        menuState->dm1HoCPresentedCaptureLaunchSerial == g_m11_sync_launch_serial &&
        menuState->dm1HoCPresentedCaptureReceipt.presentedCaptureReady) {
        int hocTrack = dm1_v1_fmtowns_cd_track_for_event(1);
        if (hocTrack > 0 &&
            gameView->dm1FmtownsCddaCurrentTrack != hocTrack) {
            M11_GameView_PlayFmtownsCdda(gameView, hocTrack);
        }
    }
}

static int m11_running_from_macos_app_bundle(void)
{
#ifdef __APPLE__
    const char *base_path = SDL_GetBasePath();
    int running_from_bundle =
        base_path && strstr(base_path, ".app/Contents/MacOS/") != NULL;
#if !SDL_VERSION_ATLEAST(3, 0, 0)
    /* SDL2 allocates this string; SDL3 returns a library-owned const path. */
    SDL_free((void *)base_path);
#endif
    return running_from_bundle;
#else
    return 0;
#endif
}

/* Resolve tracked runtime PO sources independently of the game-data tree.
 * Installed packages set FIRESTAFF_LOCALE_DIR (or use the FHS location),
 * while developer builds retain relative/source-tree fallbacks. */
static int m11_load_po_domain(const char* domain,
                              const char* language,
                              const char* dataDir) {
    char path[512];
    const char* localeDir = getenv("FIRESTAFF_LOCALE_DIR");
    if (!domain || !domain[0] || !language || !language[0]) return 0;
    if (localeDir && localeDir[0] &&
        snprintf(path, sizeof(path), "%s/%s.%s.po", localeDir,
                 domain, language) > 0 && fs_po_load(path) > 0) return 1;
    if (snprintf(path, sizeof(path), "/usr/share/firestaff/po/%s.%s.po",
                 domain, language) > 0 && fs_po_load(path) > 0) return 1;
    if (snprintf(path, sizeof(path), "po/%s.%s.po", domain, language) > 0 &&
        fs_po_load(path) > 0) return 1;
#ifdef FIRESTAFF_SOURCE_DIR
    if (snprintf(path, sizeof(path), FIRESTAFF_SOURCE_DIR "/po/%s.%s.po",
                 domain, language) > 0 && fs_po_load(path) > 0) return 1;
#endif
    /* Compatibility for older portable trees that placed po/ beside their
     * selected data directory. New packages never install catalogs there. */
    if (dataDir && dataDir[0] &&
        snprintf(path, sizeof(path), "%s/po/%s.%s.po", dataDir,
                 domain, language) > 0 && fs_po_load(path) > 0) return 1;
    return 0;
}

/* Keep one ordering shared by CLI parsing, M12 and the runtime catalog
 * handoff. The game selector owns a language per title; loading every
 * catalog from the launcher-wide setting alone used to discard that choice
 * when the selected game entered M11. */
static const char* m11_language_code_from_index(int languageIndex) {
    static const char* const codes[] = {
        "en", "sv", "fr", "de", "ja", "zh", "cs", "da", "es", "fi",
        "hu", "it", "ko", "nl", "no", "pl", "pt", "ru", "tr", "id"
    };
    if (languageIndex < 0 ||
        languageIndex >= (int)(sizeof(codes) / sizeof(codes[0]))) {
        return "en";
    }
    return codes[languageIndex];
}

static void m11_apply_selected_game_language(
    const M12_StartupMenuState* menuState,
    const M12_MenuEntry* entry,
    const char* dataDir) {
    M12_LaunchIntent intent;
    const char* domain = NULL;
    const char* language;

    if (!menuState || !entry || !entry->gameId) {
        return;
    }
    if (strcmp(entry->gameId, "dm1") == 0) domain = "dm1";
    else if (strcmp(entry->gameId, "csb") == 0) domain = "csb";
    else if (strcmp(entry->gameId, "dm2") == 0) domain = "dm2";
    else if (strcmp(entry->gameId, "nexus") == 0) domain = "nexus";
    else if (strcmp(entry->gameId, "theron") == 0) domain = "theron";
    if (!domain) {
        return;
    }

    intent = M12_StartupMenu_GetLaunchIntent(menuState);
    language = m11_language_code_from_index(intent.options.languageIndex);
    fs_l10n_set_language(fs_l10n_language_from_locale(language));
    /* A missing catalog is deliberately not substituted with another game
     * domain. fs_po's exact-msgid fallback leaves the selected game's
     * authenticated English source intact. */
    (void)m11_load_po_domain(domain, language, dataDir);
}

/* Opt-in evidence capture for the actual post-present SDL surface. The
 * capture is downstream of the source-raster and RGBA palette gates; it
 * never promotes a screenshot into source evidence. One file per source
 * palette phase avoids writing one capture for every startup VBlank. */
static void m11_capture_csb_presented_source_phase(int special_palette)
{
    const char *capture_dir = getenv("FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR");
    static unsigned int captured_palette_mask;
    char output_path[1024];
    unsigned int palette_bit;

    if (!capture_dir || !capture_dir[0] || special_palette < 0 ||
        special_palette >= (int)(sizeof(captured_palette_mask) * 8u)) {
        return;
    }
    palette_bit = 1u << (unsigned int)special_palette;
    if (captured_palette_mask & palette_bit) {
        return;
    }
    if (!M11_Screenshot_CapturePresentedRGBA(capture_dir, output_path,
                                              (int)sizeof(output_path))) {
        fprintf(stderr, "firestaff: CSB presented capture failed: %s\n",
                capture_dir);
        return;
    }
    captured_palette_mask |= palette_bit;
    fprintf(stderr, "CSB PRESENTED SOURCE CAPTURE: palette=%d %s\n",
            special_palette, output_path);
}

/* The normal game page has no TITLE/ENTRANCE palette identifier. It is still
 * captured only after M11 has authenticated the completed source-owned F0128
 * aperture for the current package session. */
static void m11_capture_csb_presented_runtime_source(void)
{
    const char *capture_dir = getenv("FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR");
    static int captured_runtime;
    char output_path[1024];

    if (captured_runtime || !capture_dir || !capture_dir[0]) {
        return;
    }
    if (!M11_Screenshot_CapturePresentedRGBA(capture_dir, output_path,
                                              (int)sizeof(output_path))) {
        fprintf(stderr, "firestaff: CSB runtime presented capture failed: %s\n",
                capture_dir);
        return;
    }
    captured_runtime = 1;
    fprintf(stderr, "CSB PRESENTED RUNTIME CAPTURE: %s\n", output_path);
}

/* Optional DM1 startup evidence uses the same post-present RGBA screenshot
 * path as the normal screenshot hook. Capture only after a real PC34 page has
 * been presented; this records source frames without making capture a launch
 * dependency or a pixel-parity assertion. */
static void m11_capture_dm1_startup_presented_frame(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media,
    const char* phase)
{
    const char* capture_root = getenv("FIRESTAFF_DM1_STARTUP_CAPTURE_DIR");
    char capture_dir[1024];
    char output_path[1280];
    int written;

    if (!capture_root || !capture_root[0] || !media || !media->handled ||
        media->platform != DM1_V1_STARTUP_MEDIA_PLATFORM_PC34 ||
        !phase || !phase[0]) {
        return;
    }
    written = snprintf(capture_dir, sizeof(capture_dir), "%s/%s",
                       capture_root, phase);
    if (written < 0 || written >= (int)sizeof(capture_dir)) {
        fprintf(stderr, "firestaff: DM1 startup capture path is too long\n");
        return;
    }
    if (!M11_Screenshot_CapturePresentedRGBA(capture_dir, output_path,
                                              (int)sizeof(output_path))) {
        fprintf(stderr, "firestaff: DM1 startup presented capture failed: %s\n",
                phase);
        return;
    }
    fprintf(stderr, "DM1 STARTUP PRESENTED CAPTURE: %s %s\n",
            phase, output_path);
}

static void m11_capture_dm1_authenticated_startup_frame(const char* phase)
{
    const char* capture_root = getenv("FIRESTAFF_DM1_STARTUP_CAPTURE_DIR");
    char capture_dir[1024];
    char output_path[1280];
    int written;

    if (!capture_root || !capture_root[0] || !phase || !phase[0]) return;
    written = snprintf(capture_dir, sizeof(capture_dir), "%s/%s",
                       capture_root, phase);
    if (written < 0 || written >= (int)sizeof(capture_dir)) {
        fprintf(stderr, "firestaff: DM1 startup capture path is too long\n");
        return;
    }
    if (!M11_Screenshot_CapturePresentedRGBA(capture_dir, output_path,
                                              (int)sizeof(output_path))) {
        fprintf(stderr, "firestaff: DM1 startup presented capture failed: %s\n",
                phase);
        return;
    }
    fprintf(stderr, "DM1 STARTUP PRESENTED CAPTURE: %s %s\n",
            phase, output_path);
}

/* CSB V2.0 has two source-preserving presentation passes: indexed cleanup
 * happens before palette conversion in m11_present_game_frame(), while CRT
 * scanlines operate on the final RGBA surface. Keep the second pass here,
 * after the normal M11 present, so it uses the actual target dimensions and
 * never mutates the source-owned indexed framebuffer or its palette receipt. */
static int m11_present_csb_v20_rgba_filter_if_enabled(
    const M11_GameViewState *gameView,
    const unsigned char **out_rgba,
    int *out_width,
    int *out_height)
{
    const unsigned char *presented_rgba;
    unsigned char *filtered_rgba;
    const CSB_V2_FilterConfig *filter_config;
    size_t bytes;
    int width = 0;
    int height = 0;

    if (!gameView ||
        gameView->sourceKind != M11_GAME_SOURCE_CSB_BOOT ||
        gameView->presentationMode != M12_PRESENTATION_V20_FILTERED ||
        M11_GameView_GetPresentationSpecialPalette(gameView) >= 0) {
        return 1;
    }
    filter_config = csb_v2_filter_config_get();
    if (!filter_config || !filter_config->crtScanlinesEnabled) {
        return 1;
    }
    presented_rgba = M11_Render_GetPresentedRGBA(&width, &height);
    if (!presented_rgba || width <= 0 || height <= 0 ||
        (size_t)width > (size_t)-1 / (size_t)height ||
        (size_t)width * (size_t)height > (size_t)-1 / 4u) {
        return 0;
    }
    bytes = (size_t)width * (size_t)height * 4u;
    filtered_rgba = (unsigned char *)malloc(bytes);
    if (!filtered_rgba) {
        return 0;
    }
    memcpy(filtered_rgba, presented_rgba, bytes);
    if (csb_v2_filter_chain_apply_rgba(filtered_rgba, width, height) > 0 &&
        M11_Render_PresentRGBA(filtered_rgba, width, height) != M11_RENDER_OK) {
        free(filtered_rgba);
        return 0;
    }
    free(filtered_rgba);
    if (out_rgba) {
        *out_rgba = M11_Render_GetPresentedRGBA(&width, &height);
    }
    if (out_width) {
        *out_width = width;
    }
    if (out_height) {
        *out_height = height;
    }
    return 1;
}

/* ReDMCSB/CSBWin startup hands source-owned pages to the host before input.
 * Capture may observe only a completed SDL presentation, never a pre-upload
 * framebuffer or a merely allocated window. */
static int m11_present_game_frame_and_publish_startup_capture(
    M11_GameViewState* gameView,
    M12_StartupMenuState* menuState) {
    const unsigned char* presented_frame = NULL;
    const unsigned char* source_frame = NULL;
    const unsigned char* presented_rgba = NULL;
    int presented_width = 0;
    int presented_height = 0;
    int mac_window_capture_ready = 0;
    int csb_source_output_matches = 0;
    int csb_special_palette = -1;

    if (!m11_present_game_frame(gameView, &presented_frame)) {
        return 0;
    }
    /* V2.0 may put a filtered copy on the host presentation path.  Startup
     * receipts must still authenticate the unmodified PC3.4 source page. */
    source_frame = M11_Render_GetFramebuffer();
    /* M11_Render_GetPresentedRGBA is populated in the renderer immediately
     * before SDL_RenderPresent.  Do not promote a CSB startup receipt when
     * that actual host buffer is absent, even if SDL still has a window. */
    presented_rgba = M11_Render_GetPresentedRGBA(&presented_width,
                                                  &presented_height);
    if (!m11_present_csb_v20_rgba_filter_if_enabled(gameView,
                                                    &presented_rgba,
                                                    &presented_width,
                                                    &presented_height)) {
        return 0;
    }
#ifdef __APPLE__
    mac_window_capture_ready =
        M11_Render_GetWindow() != NULL && presented_rgba != NULL &&
        presented_width > 0 && presented_height > 0;
#endif
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT &&
        presented_frame && source_frame) {
        csb_special_palette =
             M11_GameView_GetPresentationSpecialPalette(gameView);
        csb_source_output_matches = csb_special_palette >= 0 &&
            M11_GameView_CSBPresentedFrameMatchesCurrentSource(
                gameView, source_frame, M11_FB_WIDTH, M11_FB_HEIGHT,
                csb_special_palette) &&
            (gameView->presentationMode == M12_PRESENTATION_V21_UPSCALED
                ? M11_Render_PresentedEpxIndexedSpecialMatches(
                      presented_frame, M11_FB_WIDTH, M11_FB_HEIGHT,
                      csb_special_palette)
                : M11_Render_PresentedIndexedSpecialMatches(
                      presented_frame, M11_FB_WIDTH, M11_FB_HEIGHT,
                      csb_special_palette));
    }
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT &&
        source_frame && presented_frame && presented_rgba && presented_width > 0 &&
        presented_height > 0) {
        if (csb_source_output_matches) {
            M11_GameView_RecordCSBPresentedIndexedFrame(
                (M11_GameViewState *)gameView,
                source_frame,
                M11_FB_WIDTH,
                M11_FB_HEIGHT,
                m11_running_from_macos_app_bundle(),
                mac_window_capture_ready
            );
            m11_capture_csb_presented_source_phase(csb_special_palette);
        } else if (M11_GameView_CSBNormalRuntimeFrameHasCurrentSourceReceipt(
                       gameView, source_frame, M11_FB_WIDTH, M11_FB_HEIGHT)) {
            m11_capture_csb_presented_runtime_source();
        }
    }
    m11_publish_dm1_hoc_presented_capture_to_m12(gameView, menuState);
    return 1;
}

void M11_ApplyStartupMenuRuntime(M12_StartupMenuState* menuState) {
    int requestedWindowMode;
    int requestedScaleMode;
    int actualWindowMode;
    if (!menuState) {
        return;
    }
    requestedWindowMode = menuState->settings.windowModeIndex;
    actualWindowMode = M11_Render_SyncWindowModeFromWindow();
    /* If the user maximized the OS window directly while the saved setting
     * still says WINDOWED, do not shrink the window just because the launcher
     * is switching between menu/game/settings surfaces. Keep the observed
     * maximized/fullscreen mode as the runtime setting outside the settings
     * editor; inside settings, an explicit WINDOWED selection must still be
     * able to restore the window. */
    if (menuState->view != M12_MENU_VIEW_SETTINGS &&
        requestedWindowMode == M11_WINDOW_MODE_WINDOWED &&
        (actualWindowMode == M11_WINDOW_MODE_MAXIMIZED ||
         actualWindowMode == M11_WINDOW_MODE_FULLSCREEN)) {
        requestedWindowMode = actualWindowMode;
        menuState->settings.windowModeIndex = actualWindowMode;
    }
    requestedScaleMode = menuState->settings.scaleModeIndex;
    requestedScaleMode = M11_ResolveScaleModeForWindowMode(requestedScaleMode,
                                                            requestedWindowMode);
    if (requestedWindowMode != M11_WINDOW_MODE_WINDOWED &&
        menuState->settings.scaleModeIndex != requestedScaleMode) {
        menuState->settings.scaleModeIndex = requestedScaleMode;
    }
    if (M11_Render_GetPaletteLevel() != M12_StartupMenu_GetRenderPaletteLevel(menuState)) {
        M11_Render_SetPaletteLevel(M12_StartupMenu_GetRenderPaletteLevel(menuState));
    }
    if (M11_Render_GetWindowMode() != requestedWindowMode) {
        M11_Render_SetWindowMode(requestedWindowMode);
    }
    if (M11_Render_GetScaleMode() != requestedScaleMode) {
        M11_Render_SetScaleMode(requestedScaleMode);
    }
    if (M11_Render_GetDisplayAspectMode() != menuState->settings.displayAspectMode) {
        M11_Render_SetDisplayAspectMode(menuState->settings.displayAspectMode);
    }
    if (M11_Render_GetIntegerScaling() != menuState->settings.integerScaling) {
        M11_Render_SetIntegerScaling(menuState->settings.integerScaling);
    }
    if (M11_Render_GetScaleFilter() != menuState->settings.scalingFilterIndex) {
        M11_Render_SetScaleFilter(menuState->settings.scalingFilterIndex);
    }
    if (M11_Render_GetVSync() != menuState->settings.vsyncIndex) {
        M11_Render_SetVSync(menuState->settings.vsyncIndex);
    }
    if (M11_Render_GetGlobalBrightness() != menuState->settings.displayBrightness) {
        M11_Render_SetGlobalBrightness(menuState->settings.displayBrightness);
    }
}

static void m11_sync_runtime_graphics_popup_to_menu(
    const M11_GameViewState* gameView,
    M12_StartupMenuState* menuState)
{
    M12_Config config;
    int slot = 0;
    if (!gameView || !menuState) return;
    if (gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT) slot = 1;
    else if (gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT) slot = 2;
    else if (gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN) slot = 3;
    else if (gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02) slot = 4;
    M12_Config_Load(&config, NULL);
    menuState->settings.graphicsIndex = config.graphicsIndex;
    menuState->settings.windowModeIndex = config.windowModeIndex;
    menuState->settings.scaleModeIndex = config.scaleModeIndex;
    menuState->settings.displayAspectMode = config.displayAspectMode;
    menuState->settings.integerScaling = config.integerScaling;
    menuState->settings.scalingFilterIndex = config.scalingFilterIndex;
    menuState->settings.vsyncIndex = config.vsyncIndex;
    menuState->settings.showFpsOverlay = config.showFpsOverlay ? 1 : 0;
    /* M12_SaveConfig serializes these fields from menuState.  Keep that
     * shadow copy current after a live M11 edit so returning to the launcher
     * cannot write pre-popup V2 values back over the saved config. */
    menuState->settings.dm1V2SmoothingEnabled = config.dm1V2SmoothingEnabled;
    menuState->settings.dm1V2DynamicLightingEnabled = config.dm1V2DynamicLightingEnabled;
    menuState->settings.dm1V2SmoothTurnPanEnabled = config.dm1V2SmoothTurnPanEnabled;
    menuState->settings.dm1V2CrtScanlinesEnabled = config.dm1V2CrtScanlinesEnabled;
    menuState->settings.dm1V2CrtScanlineStrength = config.dm1V2CrtScanlineStrength;
    menuState->settings.dm1V2PaletteCorrectionEnabled = config.dm1V2PaletteCorrectionEnabled;
    menuState->settings.dm1V2PaletteGamma = config.dm1V2PaletteGamma;
    menuState->settings.dm1V2PaletteBrightness = config.dm1V2PaletteBrightness;
    menuState->settings.dm1V2PaletteContrast = config.dm1V2PaletteContrast;
    menuState->settings.dm1V2DitherCleanupEnabled = config.dm1V2DitherCleanupEnabled;
    menuState->settings.dm1V2SharpeningEnabled = config.dm1V2SharpeningEnabled;
    menuState->settings.dm1V2SharpeningStrength = config.dm1V2SharpeningStrength;
    menuState->settings.dm1V2PhosphorPersistenceEnabled = config.dm1V2PhosphorPersistenceEnabled;
    menuState->settings.dm1V2PhosphorDecay = config.dm1V2PhosphorDecay;
    menuState->settings.dm1V2ColorPreset = config.dm1V2ColorPreset;
    menuState->settings.dm1V2PixelGridEnabled = config.dm1V2PixelGridEnabled;
    menuState->settings.dm1V2PixelGridIntensity = config.dm1V2PixelGridIntensity;
    menuState->settings.dm1V2MotionBlurEnabled = config.dm1V2MotionBlurEnabled;
    menuState->settings.dm1V2MotionBlurStrength = config.dm1V2MotionBlurStrength;
    menuState->gameOptions[slot].presentationModeIndex = config.graphicsIndex;
    menuState->gameOptions[slot].aspectRatio = config.gameAspectRatio[slot];
    menuState->gameOptions[slot].resolution = config.gameResolution[slot];
}

int M11_ResolveScaleModeForWindowMode(int requestedScaleMode, int windowMode) {
    /* A fixed 1x--4x framebuffer inside a maximized Cocoa window leaves
     * DM1's 320x200 V1 frame looking like a thumbnail. M11 is the shared
     * host for all game views, so maximized/fullscreen presentation uses FIT
     * unless the user explicitly selected STRETCH. Windowed mode retains
     * fixed-scale choices for pixel inspection. */
    if (windowMode != M11_WINDOW_MODE_WINDOWED &&
        requestedScaleMode >= M11_SCALE_1X &&
        requestedScaleMode <= M11_SCALE_4X) {
        return M11_SCALE_FIT;
    }
    return requestedScaleMode;
}

static int m11_is_default_window_size(int width, int height) {
    return width == 960 && height == 540;
}

static void m11_apply_persisted_window_size(M11_PhaseA_Options* opts) {
    M12_Config config;
    if (!opts || !m11_is_default_window_size(opts->windowWidth, opts->windowHeight)) {
        return;
    }
    M12_Config_Load(&config, opts->dataDir);
    if (config.windowWidth > 0 && config.windowHeight > 0) {
        opts->windowWidth = config.windowWidth;
        opts->windowHeight = config.windowHeight;
    }
}

static void m11_sync_startup_text_input(const M12_StartupMenuState* menuState,
                                        int launcherOwnsInput,
                                        int* hostTextInputActive) {
    M12_StartupTextInputHostReceipt receipt;
    int active;
    if (!hostTextInputActive) {
        return;
    }
    active = *hostTextInputActive;
    if (!launcherOwnsInput) {
        if (active) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
            (void)SDL_StopTextInput(M11_Render_GetWindow());
#else
            SDL_StopTextInput();
#endif
            *hostTextInputActive = 0;
        }
        return;
    }
    if (!M12_StartupMenu_TextInputHostReceipt(menuState, active, &receipt)) {
        return;
    }
    if (receipt.startTextInput) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        if (SDL_StartTextInput(M11_Render_GetWindow())) {
            *hostTextInputActive = 1;
        }
#else
        SDL_StartTextInput();
        *hostTextInputActive = 1;
#endif
    } else if (receipt.stopTextInput) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        (void)SDL_StopTextInput(M11_Render_GetWindow());
#else
        SDL_StopTextInput();
#endif
        *hostTextInputActive = 0;
    }
}

static void m11_sync_and_save_window_size(M12_StartupMenuState* menuState) {
    if (!menuState || !M11_Render_IsInitialized()) {
        return;
    }
    menuState->settings.windowWidth = M11_Render_GetWindowWidth();
    menuState->settings.windowHeight = M11_Render_GetWindowHeight();
    M12_StartupMenu_SaveConfig(menuState);
}


static int m11_draw_entrance_screen_asset(M11_GameViewState* gameView,
                                          unsigned char* framebuffer) {
    const M11_AssetSlot* entranceScreen;
    if (!gameView || !framebuffer || !gameView->assetsAvailable) {
        return 0;
    }
    entranceScreen = M11_AssetLoader_Load(&gameView->assetLoader, 4U);
    if (!entranceScreen || entranceScreen->width != 320U || entranceScreen->height != 200U) {
        return 0;
    }
    M11_AssetLoader_Blit(entranceScreen,
                         framebuffer,
                         M11_FB_WIDTH,
                         M11_FB_HEIGHT,
                         0,
                         0,
                         -1);
    return 1;
}


static int m11_draw_entrance_closed_doors_asset(M11_GameViewState* gameView,
                                                unsigned char* framebuffer) {
    unsigned int ordinal;
    if (!gameView || !framebuffer || !gameView->assetsAvailable) {
        return 0;
    }
    for (ordinal = 1U; ordinal <= 2U; ++ordinal) {
        EntranceCompatClosedDoorBlit blit;
        const M11_AssetSlot* door;
        if (!ENTRANCE_Compat_GetClosedDoorBlit(ordinal, &blit)) return 0;
        door = M11_AssetLoader_Load(&gameView->assetLoader, blit.assetId);
        if (!door ||
            door->width < blit.srcX + blit.width ||
            door->height < blit.srcY + blit.height) {
            return 0;
        }
        M11_AssetLoader_BlitRegion(door,
                                   (int)blit.srcX,
                                   (int)blit.srcY,
                                   (int)blit.width,
                                   (int)blit.height,
                                   framebuffer,
                                   M11_FB_WIDTH,
                                   M11_FB_HEIGHT,
                                   (int)blit.dstX,
                                   (int)blit.dstY,
                                   blit.transparentColor);
    }
    return 1;
}

typedef enum {
    M11_ENTRANCE_COMMAND_QUIT = ENTRANCE_COMPAT_COMMAND_PATH_QUIT,
    M11_ENTRANCE_COMMAND_NONE = ENTRANCE_COMPAT_COMMAND_PATH_NONE,
    M11_ENTRANCE_COMMAND_ENTER = ENTRANCE_COMPAT_COMMAND_PATH_ENTER,
    M11_ENTRANCE_COMMAND_RESUME = ENTRANCE_COMPAT_COMMAND_PATH_RESUME,
    M11_ENTRANCE_COMMAND_CREDITS = ENTRANCE_COMPAT_COMMAND_PATH_CREDITS
} M11_EntranceCommand;

int M11_FocusPauseRequired(int enabled, int hasFocus, const char* videoDriver) {
    return enabled && !hasFocus &&
           (!videoDriver || strcmp(videoDriver, "dummy") != 0);
}

static int g_m11_intro_delay_fast_forward = 0;
static int g_m11_startup_intro_quit_requested = 0;
static int g_m11_dm1_i34e_intro_preplayed = 0;
static int g_m11_dm1_direct_cli_launch = 0;
static Uint32 g_m11_phase_duration_started_ms;
static int g_m11_phase_duration_ms = -1;
static int g_m11_phase_duration_expired;
static uint16_t g_m11_dm1_amiga_swsh_palette[16];
static int g_m11_dm1_amiga_swsh_palette_valid = 0;

static int m11_event_requests_window_close(const SDL_Event* event);

int M11_RunDurationHasElapsed(int durationMs,
                              uint32_t startedAtMs,
                              uint32_t nowMs) {
    if (durationMs < 0) return 0;
    return (uint32_t)(nowMs - startedAtMs) >= (uint32_t)durationMs;
}

static int m11_phase_duration_expired(void) {
    if (g_m11_phase_duration_expired) return 1;
    if (M11_RunDurationHasElapsed(g_m11_phase_duration_ms,
                                  g_m11_phase_duration_started_ms,
                                  (Uint32)SDL_GetTicks())) {
        g_m11_phase_duration_expired = 1;
        return 1;
    }
    return 0;
}

static int m11_process_cpu_seconds(double* seconds) {
    if (!seconds) return 0;
#if defined(_WIN32)
    FILETIME creation, exitTime, kernel, user;
    ULARGE_INTEGER kernelValue, userValue;
    if (!GetProcessTimes(GetCurrentProcess(), &creation, &exitTime,
                         &kernel, &user)) return 0;
    kernelValue.LowPart = kernel.dwLowDateTime;
    kernelValue.HighPart = kernel.dwHighDateTime;
    userValue.LowPart = user.dwLowDateTime;
    userValue.HighPart = user.dwHighDateTime;
    *seconds = (double)(kernelValue.QuadPart + userValue.QuadPart) / 10000000.0;
    return 1;
#elif defined(__APPLE__) || defined(__linux__)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) != 0) return 0;
    *seconds = (double)usage.ru_utime.tv_sec +
               (double)usage.ru_utime.tv_usec / 1000000.0 +
               (double)usage.ru_stime.tv_sec +
               (double)usage.ru_stime.tv_usec / 1000000.0;
    return 1;
#else
    return 0;
#endif
}

static int m11_process_memory_bytes(uint64_t* residentBytes,
                                   uint64_t* virtualBytes) {
    if (!residentBytes || !virtualBytes) return 0;
#if defined(_WIN32)
    {
        PROCESS_MEMORY_COUNTERS_EX counters;
        memset(&counters, 0, sizeof(counters));
        counters.cb = sizeof(counters);
        if (!GetProcessMemoryInfo(GetCurrentProcess(),
                (PROCESS_MEMORY_COUNTERS*)&counters, sizeof(counters)))
            return 0;
        *residentBytes = (uint64_t)counters.WorkingSetSize;
        *virtualBytes = (uint64_t)counters.PrivateUsage;
        return 1;
    }
#elif defined(__APPLE__)
    {
        mach_task_basic_info_data_t info;
        mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                      (task_info_t)&info, &count) != KERN_SUCCESS)
            return 0;
        *residentBytes = (uint64_t)info.resident_size;
        *virtualBytes = (uint64_t)info.virtual_size;
        return 1;
    }
#elif defined(__linux__)
    {
        FILE* statm = fopen("/proc/self/statm", "r");
        unsigned long long virtualPages = 0;
        unsigned long long residentPages = 0;
        long pageSize;
        int readOk;
        if (!statm) return 0;
        readOk = fscanf(statm, "%llu %llu", &virtualPages, &residentPages);
        fclose(statm);
        pageSize = sysconf(_SC_PAGESIZE);
        if (readOk != 2 || pageSize <= 0) return 0;
        *residentBytes = (uint64_t)residentPages * (uint64_t)pageSize;
        *virtualBytes = (uint64_t)virtualPages * (uint64_t)pageSize;
        return 1;
    }
#else
    return 0;
#endif
}

static double m11_counter_milliseconds(Uint64 ticks) {
    const Uint64 frequency = SDL_GetPerformanceFrequency();
    return frequency ? (double)ticks * 1000.0 / (double)frequency : 0.0;
}

static Uint64 m11_sdl_event_timestamp_to_counter(Uint64 eventTimestamp) {
    const Uint64 nowCounter = SDL_GetPerformanceCounter();
    const Uint64 frequency = SDL_GetPerformanceFrequency();
    double ageCounter = 0.0;
    if (!eventTimestamp || !frequency) return nowCounter;
#if SDL_VERSION_ATLEAST(3, 0, 0)
    {
        const Uint64 nowNs = SDL_GetTicksNS();
        if (eventTimestamp > nowNs) return nowCounter;
        ageCounter = (double)(nowNs - eventTimestamp) *
                     (double)frequency / 1000000000.0;
    }
#else
    {
        const Uint32 ageMs = (Uint32)SDL_GetTicks() - (Uint32)eventTimestamp;
        ageCounter = (double)ageMs * (double)frequency / 1000.0;
    }
#endif
    if (ageCounter >= (double)nowCounter) return 0;
    return nowCounter - (Uint64)ageCounter;
}

/* Host stage timings begin at the completed source command outcome. They do
 * not include SDL delivery or command FIFO wait, and Present return is not
 * proof that a physical display has scanned out the image. */
static struct {
    unsigned int launchSerial;
    uint64_t observedSerial;
    uint64_t drawnSerial;
    Uint64 sourceCounter;
    Uint64 drawCounter;
    Uint64 pendingInputEventCounter;
    Uint64 pendingInputPollCounter;
    Uint64 inputEventCounter;
    Uint64 inputPollCounter;
    int moved;
    int turned;
    int blocked;
    unsigned int superseded;
} g_m11_debug_motion;
static Uint64 g_m11_debug_polled_event_counter;
static Uint64 g_m11_debug_polled_at_counter;

static void m11_debug_motion_drawn(const M11_GameViewState* state,
                                    Uint64 completed) {
    if (!state || !state->active || !state->debugMovementTraceEnabled ||
        strcmp(state->sourceId, "dm1") != 0) return;
    if (g_m11_debug_motion.launchSerial != g_m11_sync_launch_serial) {
        memset(&g_m11_debug_motion, 0, sizeof(g_m11_debug_motion));
        g_m11_debug_motion.launchSerial = g_m11_sync_launch_serial;
    }
    if (state->dm1V1MovementOutcomeSerial == g_m11_debug_motion.observedSerial) {
        /* A retained outcome may be drawn again before the first successful
         * Present. Time the latest completed Draw of that outcome's frame. */
        if (g_m11_debug_motion.drawnSerial == state->dm1V1MovementOutcomeSerial)
            g_m11_debug_motion.drawCounter = completed;
        return;
    }
    g_m11_debug_motion.observedSerial = state->dm1V1MovementOutcomeSerial;
    if (!state->dm1V1MovementOutcomeCounter ||
        !state->lastDm1V1MovementPipelineResult.core.queue.dequeued) {
        /* A newer source result invalidates the old frame association, even
         * if it did not dequeue a command. Never time that new image as the
         * previous movement's presentation. */
        if (g_m11_debug_motion.drawnSerial)
            fprintf(stderr, "firestaff: dm1-motion-discard outcome-serial=%llu "
                    "reason=new-source-outcome-without-command\n",
                    (unsigned long long)g_m11_debug_motion.drawnSerial);
        g_m11_debug_motion.drawnSerial = 0;
        g_m11_debug_motion.superseded = 0;
        return;
    }
    if (g_m11_debug_motion.drawnSerial) ++g_m11_debug_motion.superseded;
    g_m11_debug_motion.drawnSerial = state->dm1V1MovementOutcomeSerial;
    g_m11_debug_motion.sourceCounter = state->dm1V1MovementOutcomeCounter;
    g_m11_debug_motion.drawCounter = completed;
    g_m11_debug_motion.inputEventCounter =
        g_m11_debug_motion.pendingInputEventCounter;
    g_m11_debug_motion.inputPollCounter =
        g_m11_debug_motion.pendingInputPollCounter;
    g_m11_debug_motion.pendingInputEventCounter = 0;
    g_m11_debug_motion.pendingInputPollCounter = 0;
    g_m11_debug_motion.moved = state->lastDm1V1MovementPipelineResult.anyMovementOccurred;
    g_m11_debug_motion.turned = state->lastDm1V1MovementPipelineResult.anyTurnOccurred;
    g_m11_debug_motion.blocked = state->lastDm1V1MovementPipelineResult.core.movementBlocked;
}

static void m11_debug_motion_presented(const M11_GameViewState* state,
                                        Uint64 completed) {
    M11_RenderPresentTiming presentTiming;
    const int hasPresentTiming =
        M11_Render_GetLastPresentTiming(&presentTiming);
    if (!state || !state->debugMovementTraceEnabled ||
        !g_m11_debug_motion.drawnSerial ||
        g_m11_debug_motion.launchSerial != g_m11_sync_launch_serial) return;
    if (!state->active || strcmp(state->sourceId, "dm1") != 0 ||
        state->dm1V1MovementOutcomeSerial != g_m11_debug_motion.drawnSerial) {
        fprintf(stderr, "firestaff: dm1-motion-discard outcome-serial=%llu "
                "reason=new-source-outcome-after-draw\n",
                (unsigned long long)g_m11_debug_motion.drawnSerial);
        g_m11_debug_motion.drawnSerial = 0;
        g_m11_debug_motion.superseded = 0;
        return;
    }
    fprintf(stderr,
            "firestaff: dm1-motion-present launch=%u outcome-serial=%llu "
            "moved=%d turned=%d blocked=%d superseded-before-present=%u "
            "source-outcome-to-draw-ms=%.3f draw-to-present-return-ms=%.3f "
            "source-outcome-to-present-return-ms=%.3f "
            "input-event-timing=%d event-queue-age-ms=%.3f "
            "input-event-to-source-outcome-ms=%.3f "
            "input-event-to-present-return-ms=%.3f "
            "present-timing=%s framebuffer-convert-ms=%.3f texture-upload-ms=%.3f "
            "renderer-submit-ms=%.3f sdl-present-call-ms=%.3f "
            "cpu-nearest-upscale=%d texture-size=%dx%d "
            "physical-scanout=unmeasured\n",
            g_m11_debug_motion.launchSerial,
            (unsigned long long)g_m11_debug_motion.drawnSerial,
            g_m11_debug_motion.moved, g_m11_debug_motion.turned,
            g_m11_debug_motion.blocked, g_m11_debug_motion.superseded,
            m11_counter_milliseconds(g_m11_debug_motion.drawCounter -
                                      g_m11_debug_motion.sourceCounter),
            m11_counter_milliseconds(completed - g_m11_debug_motion.drawCounter),
            m11_counter_milliseconds(completed - g_m11_debug_motion.sourceCounter),
            g_m11_debug_motion.inputEventCounter ? 1 : 0,
            g_m11_debug_motion.inputEventCounter &&
                    g_m11_debug_motion.inputPollCounter >=
                        g_m11_debug_motion.inputEventCounter
                ? m11_counter_milliseconds(
                      g_m11_debug_motion.inputPollCounter -
                      g_m11_debug_motion.inputEventCounter)
                : 0.0,
            g_m11_debug_motion.inputEventCounter &&
                    g_m11_debug_motion.sourceCounter >=
                        g_m11_debug_motion.inputEventCounter
                ? m11_counter_milliseconds(
                      g_m11_debug_motion.sourceCounter -
                      g_m11_debug_motion.inputEventCounter)
                : 0.0,
            g_m11_debug_motion.inputEventCounter &&
                    completed >= g_m11_debug_motion.inputEventCounter
                ? m11_counter_milliseconds(
                      completed - g_m11_debug_motion.inputEventCounter)
                : 0.0,
            hasPresentTiming ? "available" : "unavailable",
            hasPresentTiming ? presentTiming.framebufferConvertMs : 0.0,
            hasPresentTiming ? presentTiming.textureUploadMs : 0.0,
            hasPresentTiming ? presentTiming.rendererSubmitMs : 0.0,
            hasPresentTiming ? presentTiming.presentCallMs : 0.0,
            hasPresentTiming ? presentTiming.cpuNearestUpscale : 0,
            hasPresentTiming ? presentTiming.width : 0,
            hasPresentTiming ? presentTiming.height : 0);
    g_m11_debug_motion.drawnSerial = 0;
    g_m11_debug_motion.superseded = 0;
}

static struct {
    unsigned int launchSerial;
    uint64_t sequence;
} g_m11_debug_dm1_runtime_receipts;

static void m11_debug_dm1_combat_receipt(const M11_GameViewState* state,
                                         const char* eventName,
                                         int onlyCombatEvents) {
    DM1RuntimeEventReceipt_Compat receipts[DM1_RUNTIME_EVENT_RECEIPT_CAPACITY];
    uint64_t dropped = 0;
    size_t count;
    size_t i;
    (void)eventName;
    (void)onlyCombatEvents;
    if (!state || !state->debugMovementTraceEnabled ||
        strcmp(state->sourceId, "dm1") != 0) return;
    if (g_m11_debug_dm1_runtime_receipts.launchSerial !=
        g_m11_sync_launch_serial) {
        g_m11_debug_dm1_runtime_receipts.launchSerial =
            g_m11_sync_launch_serial;
        g_m11_debug_dm1_runtime_receipts.sequence = 0;
    }
    count = DM1_RuntimeEventReceiptsSince_Compat(
        &state->world, &g_m11_debug_dm1_runtime_receipts.sequence,
        receipts, sizeof(receipts) / sizeof(receipts[0]), &dropped);
    if (dropped)
        fprintf(stderr,
                "firestaff: game-event game=dm1 "
                "type=runtime-receipt-gap source=reDMCSB-derived-simulation "
                "dropped=%llu\n", (unsigned long long)dropped);
    for (i = 0; i < count; ++i) {
        const DM1RuntimeEventReceipt_Compat* receipt = &receipts[i];
        const char* type = "unknown";
        switch (receipt->kind) {
            case DM1_RUNTIME_EVENT_PLAYER_ATTACK: type = "player-attack"; break;
            case DM1_RUNTIME_EVENT_CREATURE_MOVED: type = "monster-moved"; break;
            case DM1_RUNTIME_EVENT_CREATURE_ATTACK: type = "monster-attack"; break;
            case DM1_RUNTIME_EVENT_CHAMPION_DAMAGED: type = "champion-damaged"; break;
            case DM1_RUNTIME_EVENT_CREATURE_PROJECTILE_LAUNCHED:
                type = "monster-projectile-launch";
                break;
            default: break;
        }
        fprintf(stderr,
                "firestaff: game-event game=dm1 tick=%u type=%s "
                "source=reDMCSB-derived-simulation sequence=%llu "
                "map=%d group=%d creature=%d creature-type=%d champion=%d "
                "outcome=%d attack-power=%d damage=%d hp=%d->%d "
                "from=%d,%d to=%d,%d\n",
                receipt->gameTick, type,
                (unsigned long long)receipt->sequence, receipt->mapIndex,
                receipt->groupIndex, receipt->creatureIndex,
                receipt->creatureType, receipt->championIndex,
                receipt->outcome, receipt->attackPower, receipt->damage,
                receipt->hpBefore,
                receipt->hpAfter, receipt->fromX, receipt->fromY,
                receipt->toX, receipt->toY);
    }
}

static void m11_debug_pointer_dispatch(const M11_GameViewState* state,
                                       const char* eventName,
                                       int windowX,
                                       int windowY,
                                       int mapped,
                                       int sourceX,
                                       int sourceY,
                                       int buttonMask,
                                       int activeChampionBefore,
                                       int inputResult) {
    if (!state || !state->debugMovementTraceEnabled ||
        strcmp(state->sourceId, "dm1") != 0) return;
    if (!mapped) {
        fprintf(stderr,
                "firestaff: dm1-pointer event=%s window=%d,%d mapped=no "
                "button-mask=0x%x active-champion=%d result=not-dispatched\n",
                eventName, windowX, windowY, buttonMask,
                state->world.party.activeChampionIndex);
        return;
    }
    fprintf(stderr,
            "firestaff: dm1-pointer event=%s window=%d,%d source=%d,%d "
            "button-mask=0x%x active-champion=%d->%d champions=%d result=%d\n",
            eventName, windowX, windowY, sourceX, sourceY, buttonMask,
            activeChampionBefore, state->world.party.activeChampionIndex,
            state->world.party.championCount, inputResult);
    m11_debug_dm1_combat_receipt(state,
                                 eventName ? eventName : "pointer-unknown", 0);
}

static void m11_debug_dm1_combat_tick(const M11_GameViewState* state) {
    m11_debug_dm1_combat_receipt(state, "tick", 1);
}

static const char* m11_debug_input_name(M12_MenuInput input) {
    switch (input) {
        case M12_MENU_INPUT_UP: return "forward";
        case M12_MENU_INPUT_DOWN: return "backward";
        case M12_MENU_INPUT_STRAFE_LEFT: return "strafe-left";
        case M12_MENU_INPUT_STRAFE_RIGHT: return "strafe-right";
        case M12_MENU_INPUT_TURN_LEFT: return "turn-left";
        case M12_MENU_INPUT_TURN_RIGHT: return "turn-right";
        case M12_MENU_INPUT_ACTION: return "action";
        case M12_MENU_INPUT_CYCLE_CHAMPION: return "cycle-champion";
        case M12_MENU_INPUT_REST_TOGGLE: return "rest-toggle";
        case M12_MENU_INPUT_USE_STAIRS: return "use-stairs";
        case M12_MENU_INPUT_PICKUP_ITEM: return "pickup";
        case M12_MENU_INPUT_DROP_ITEM: return "drop";
        case M12_MENU_INPUT_USE_ITEM: return "use-item";
        case M12_MENU_INPUT_SPELL_CAST: return "spell-cast";
        case M12_MENU_INPUT_SPELL_CLEAR: return "spell-clear";
        case M12_MENU_INPUT_SPELL_RUNE_1: return "spell-rune-1";
        case M12_MENU_INPUT_SPELL_RUNE_2: return "spell-rune-2";
        case M12_MENU_INPUT_SPELL_RUNE_3: return "spell-rune-3";
        case M12_MENU_INPUT_SPELL_RUNE_4: return "spell-rune-4";
        case M12_MENU_INPUT_SPELL_RUNE_5: return "spell-rune-5";
        case M12_MENU_INPUT_SPELL_RUNE_6: return "spell-rune-6";
        case M12_MENU_INPUT_MAP_TOGGLE: return "map-toggle";
        case M12_MENU_INPUT_INVENTORY_TOGGLE: return "inventory-toggle";
        case M12_MENU_INPUT_SAVE_GAME: return "save";
        case M12_MENU_INPUT_FREEZE_TOGGLE: return "freeze-toggle";
        case M12_MENU_INPUT_NEXUS_POINTER_TOGGLE: return "nexus-pointer-toggle";
        case M12_MENU_INPUT_NEXUS_POINTER_UP_LEFT: return "nexus-pointer-up-left";
        case M12_MENU_INPUT_NEXUS_POINTER_UP_RIGHT: return "nexus-pointer-up-right";
        case M12_MENU_INPUT_NEXUS_POINTER_DOWN_LEFT: return "nexus-pointer-down-left";
        case M12_MENU_INPUT_NEXUS_POINTER_DOWN_RIGHT: return "nexus-pointer-down-right";
        case M12_MENU_INPUT_NEXUS_CYCLE_CHAMPION_PREVIOUS: return "nexus-cycle-champion-previous";
        case M12_MENU_INPUT_DISK_MENU: return "disk-menu";
        case M12_MENU_INPUT_CHAMPION_1_INVENTORY: return "champion-1-inventory";
        case M12_MENU_INPUT_CHAMPION_2_INVENTORY: return "champion-2-inventory";
        case M12_MENU_INPUT_CHAMPION_3_INVENTORY: return "champion-3-inventory";
        case M12_MENU_INPUT_CHAMPION_4_INVENTORY: return "champion-4-inventory";
        case M12_MENU_INPUT_LEADER_INVENTORY: return "leader-inventory";
        case M12_MENU_INPUT_MAC_WALL_LEFT: return "wall-left";
        case M12_MENU_INPUT_MAC_WALL_CENTER: return "wall-center";
        case M12_MENU_INPUT_MAC_WALL_RIGHT: return "wall-right";
        case M12_MENU_INPUT_MAC_OPEN_GAME: return "open-game";
        case M12_MENU_INPUT_BACK: return "back";
        case M12_MENU_INPUT_ACCEPT: return "accept";
        case M12_MENU_INPUT_VALUE_LEFT: return "value-left";
        case M12_MENU_INPUT_VALUE_RIGHT: return "value-right";
        case M12_MENU_INPUT_GRAPHICS_POPUP: return "graphics-popup";
        default: return "other";
    }
}

static const char* m11_debug_emission_name(uint8_t kind) {
    switch (kind) {
        case EMIT_DAMAGE_DEALT: return "damage-dealt";
        case EMIT_SOUND_REQUEST: return "sound";
        case EMIT_XP_AWARD: return "experience";
        case EMIT_KILL_NOTIFY: return "kill";
        case EMIT_DOOR_STATE: return "door-state";
        case EMIT_PARTY_MOVED: return "party-moved";
        case EMIT_CHAMPION_DOWN: return "champion-down";
        case EMIT_GAME_WON: return "game-won";
        case EMIT_PARTY_DEAD: return "party-dead";
        case EMIT_SPELL_EFFECT: return "spell-effect";
        case EMIT_PARTY_FELL: return "party-fell";
        case EMIT_PARTY_TELEPORTED: return "party-teleported";
        case EMIT_SENSOR_EFFECT: return "sensor";
        case EMIT_ACTION_DISABLED: return "action-disabled";
        case EMIT_CREATURE_ATTACK: return "monster-attack";
        case EMIT_CHAMPION_DAMAGED: return "champion-damaged";
        case EMIT_CHAMPION_DAMAGE_HIDDEN: return "champion-damage-hidden";
        case EMIT_TEXT_MESSAGE: return "message";
        case EMIT_ACTION_ENABLED: return "action-enabled";
        case EMIT_CREATURE_MOVED: return "monster-moved";
        case EMIT_LEVEL_UP: return "level-up";
        default: return "unknown";
    }
}

static struct {
    unsigned int launchSerial;
    int initialized;
    size_t groupCapacity;
    int groupCount;
    int mapIndex;
    int partyMapX;
    int partyMapY;
    struct {
        unsigned short next;
        unsigned short slot;
        unsigned char creatureType;
        unsigned int health[4];
    }* groups;
    unsigned int championHealth[CHAMPION_MAX_PARTY];
} g_m11_debug_health_snapshot;

static struct {
    unsigned int launchSerial;
    int valid;
    uint64_t fingerprint;
} g_m11_debug_emission_snapshot;

static uint64_t m11_debug_tick_emission_fingerprint(
    const M11_GameViewState* state) {
    uint64_t hash = UINT64_C(1469598103934665603);
    int i;
    int j;
#define M11_DEBUG_HASH_WORD(value) do { \
        uint32_t word_ = (uint32_t)(value); \
        int byte_; \
        for (byte_ = 0; byte_ < 4; ++byte_) { \
            hash ^= (uint8_t)(word_ >> (byte_ * 8)); \
            hash *= UINT64_C(1099511628211); \
        } \
    } while (0)
    M11_DEBUG_HASH_WORD(state->sourceKind);
    M11_DEBUG_HASH_WORD(state->lastTickResult.preTick);
    M11_DEBUG_HASH_WORD(state->lastTickResult.postTick);
    M11_DEBUG_HASH_WORD(state->lastTickResult.worldHashPost);
    M11_DEBUG_HASH_WORD(state->lastTickResult.emissionCount);
    for (i = 0; i < state->lastTickResult.emissionCount; ++i) {
        const struct TickEmission_Compat* event =
            &state->lastTickResult.emissions[i];
        M11_DEBUG_HASH_WORD(event->kind);
        M11_DEBUG_HASH_WORD(event->payloadSize);
        for (j = 0; j < 4; ++j) M11_DEBUG_HASH_WORD(event->payload[j]);
    }
#undef M11_DEBUG_HASH_WORD
    return hash;
}

/* The compatibility world and TickResult below are authoritative for DM1
 * only. CSB and DM2 own separate boot runtimes; emit their own state rather
 * than presenting the stale DM1 mirror as if it described those games. */
static void m11_debug_native_game_state(const M11_GameViewState* state,
                                       int reportSummary) {
    if (!state || !state->debugMovementTraceEnabled || !state->active) return;
    if (state->sourceKind == M11_GAME_SOURCE_CSB_BOOT &&
        state->csbBootProfile) {
        typedef struct {
            uint16_t thing;
            int map;
            int x;
            int y;
            int level;
        } CsbGroupSnapshot;
        static unsigned int snapshotLaunchSerial;
        static int snapshotInitialized;
        static uint16_t previousGroupCount;
        static int previousLevel;
        static int previousPartyX;
        static int previousPartyY;
        static int previousPartyZ;
        static CsbGroupSnapshot previousGroups[CSB_V1_RUNTIME_ACTIVE_GROUP_CAP];
        static uint16_t previousChampionDamage[CSB_V1_MAX_CHAMPIONS];
        static int16_t previousChampionHealth[CSB_V1_MAX_CHAMPIONS];
        const CSB_V1_BootProfile* boot =
            (const CSB_V1_BootProfile*)state->csbBootProfile;
        const CSB_V1_RuntimeProfile* runtime = &boot->runtime;
        if (reportSummary) {
            fprintf(stderr,
                    "firestaff: game-state game=csb source=csb-runtime "
                    "tick=%u level=%d party=%d,%d,%d dir=%d champions=%d "
                    "leader=%d pending-champion-damage=",
                    runtime->game_time, runtime->current_level,
                    runtime->party_x, runtime->party_y, runtime->party_z,
                    runtime->party_dir, runtime->champion_count,
                    runtime->leader_index);
            {
                unsigned int pending = 0;
                int i;
                for (i = 0; i < CSB_V1_MAX_CHAMPIONS; ++i)
                    pending += runtime->champion_pending_damage[i];
                fprintf(stderr, "%u\n", pending);
            }
            fprintf(stderr,
                    "firestaff: game-debug-capabilities game=csb "
                    "source=csb-runtime actions=melee-and-projectile-launch "
                    "monster-movement=active-group-position "
                    "player-attack=melee-and-projectile-launch "
                    "monster-health=melee-only-receipt "
                    "champion-health=source-party-state\n");
        }
        if (snapshotLaunchSerial != g_m11_sync_launch_serial) {
            snapshotLaunchSerial = g_m11_sync_launch_serial;
            snapshotInitialized = 0;
            previousGroupCount = 0;
            previousLevel = runtime->current_level;
            previousPartyX = runtime->party_x;
            previousPartyY = runtime->party_y;
            previousPartyZ = runtime->party_z;
            memset(previousGroups, 0, sizeof(previousGroups));
            memset(previousChampionDamage, 0, sizeof(previousChampionDamage));
            memset(previousChampionHealth, 0, sizeof(previousChampionHealth));
        }
        if (snapshotInitialized &&
            (previousLevel != runtime->current_level ||
             previousPartyX != runtime->party_x ||
             previousPartyY != runtime->party_y ||
             previousPartyZ != runtime->party_z)) {
            /* A changed party location can put a different set of active
             * groups into the runtime snapshot. Do not compare across it. */
            snapshotInitialized = 0;
            previousGroupCount = 0;
        }
        {
            CsbGroupSnapshot current[CSB_V1_RUNTIME_ACTIVE_GROUP_CAP];
            uint16_t currentCount = runtime->active_group_state_count;
            int i;
            if (currentCount > CSB_V1_RUNTIME_ACTIVE_GROUP_CAP)
                currentCount = CSB_V1_RUNTIME_ACTIVE_GROUP_CAP;
            if (snapshotInitialized && currentCount != previousGroupCount)
                snapshotInitialized = 0;
            memset(current, 0, sizeof(current));
            for (i = 0; i < currentCount; ++i) {
                const CSB_V1_RuntimeActiveGroupState* group =
                    &runtime->active_group_state[i];
                int prior;
                current[i].thing = group->group_thing;
                current[i].map = group->map_index;
                current[i].x = group->map_x;
                current[i].y = group->map_y;
                current[i].level = runtime->current_level;
                for (prior = 0; snapshotInitialized &&
                     prior < previousGroupCount; ++prior) {
                    if (previousGroups[prior].thing == current[i].thing) break;
                }
                if (snapshotInitialized && prior < previousGroupCount &&
                    (previousGroups[prior].level != current[i].level ||
                     previousGroups[prior].map != current[i].map ||
                     previousGroups[prior].x != current[i].x ||
                     previousGroups[prior].y != current[i].y)) {
                    fprintf(stderr,
                            "firestaff: game-event game=csb tick=%u "
                            "type=monster-moved group-thing=%u level=%d map=%d "
                            "from=%d,%d to=%d,%d\n",
                            runtime->game_time, current[i].thing, current[i].level,
                            current[i].map, previousGroups[prior].x,
                            previousGroups[prior].y, current[i].x, current[i].y);
                }
            }
            for (i = 0; i < CSB_V1_MAX_CHAMPIONS; ++i) {
                const uint16_t damage = runtime->champion_pending_damage[i];
                const int16_t health = runtime->party_state_valid &&
                    i < runtime->champion_count
                    ? runtime->party_state.Champions[i].CurrentHealth : 0;
                if (snapshotInitialized && damage != previousChampionDamage[i])
                    fprintf(stderr,
                            "firestaff: game-event game=csb tick=%u "
                            "type=champion-pending-damage champion=%d "
                            "amount=%u previous=%u\n",
                            runtime->game_time, i, damage,
                            previousChampionDamage[i]);
                previousChampionDamage[i] = damage;
                if (snapshotInitialized && i < runtime->champion_count &&
                    health != previousChampionHealth[i])
                    fprintf(stderr,
                            "firestaff: game-health game=csb tick=%u "
                            "target=champion index=%d hp=%d->%d delta=%+d\n",
                            runtime->game_time, i, previousChampionHealth[i],
                            health, (int)health - previousChampionHealth[i]);
                previousChampionHealth[i] = health;
            }
            memcpy(previousGroups, current,
                   (size_t)currentCount * sizeof(current[0]));
            previousGroupCount = currentCount;
            previousLevel = runtime->current_level;
            previousPartyX = runtime->party_x;
            previousPartyY = runtime->party_y;
            previousPartyZ = runtime->party_z;
            snapshotInitialized = 1;
        }
    } else if (state->sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
               state->dm2World) {
        static unsigned int receiptLaunchSerial;
        static int haveWieldReceipt;
        static int haveCreatureDamageReceipt;
        static int heroSnapshotCount;
        static int haveHeroSnapshot;
        static DM2_V1_RuntimeSourceHeroStateReceipt previousHeroes[DM2_MAX_HEROES];
        static uint64_t lastCreatureMovementSequence;
        static DM2_V1_RuntimeWieldAttackReceipt previousWieldReceipt;
        static DM2_V1_RuntimeCreatureDamageReceipt previousDamageReceipt;
        static uint64_t lastMissileImpactSequence;
        const DM2_V1_GameState* world =
            (const DM2_V1_GameState*)state->dm2World;
        DM2_V1_RuntimeWieldAttackReceipt wieldReceipt;
        DM2_V1_RuntimeCreatureDamageReceipt damageReceipt;
        DM2_V1_RuntimeCreatureMovementReceipt movementReceipts[64];
        uint64_t movementDropped = 0;
        const int tick = dm2_v1_runtime_get_tick_count();
        if (reportSummary)
            fprintf(stderr,
                    "firestaff: game-state game=dm2 source=dm2-world "
                    "tick=%d level=%d party=%d,%d dir=%d "
                    "outdoor=%d champions=unavailable\n",
                    tick, world->current_level, world->party_x, world->party_y,
                    world->party_dir, world->outdoor);
        if (reportSummary)
            fprintf(stderr,
                    "firestaff: game-debug-capabilities game=dm2 "
                    "source=dm2-runtime actions=partial-observed "
                    "player-attack=wield-source-receipt "
                    "monster-damage=think-and-missile-source-receipts "
                    "monster-movement=committed-ring-64 "
                    "champion-health=source-party-state\n");
        if (receiptLaunchSerial != g_m11_sync_launch_serial) {
            receiptLaunchSerial = g_m11_sync_launch_serial;
            haveWieldReceipt = 0;
            haveCreatureDamageReceipt = 0;
            memset(&previousWieldReceipt, 0, sizeof(previousWieldReceipt));
            memset(&previousDamageReceipt, 0, sizeof(previousDamageReceipt));
            lastMissileImpactSequence = 0;
            memset(previousHeroes, 0, sizeof(previousHeroes));
            heroSnapshotCount = 0;
            haveHeroSnapshot = 0;
            lastCreatureMovementSequence = 0;
        }
        {
            const int championCount = dm2_v1_runtime_get_champion_count();
            int champion;
            if (championCount < 0 || championCount > DM2_MAX_HEROES) {
                haveHeroSnapshot = 0;
            } else {
                if (haveHeroSnapshot && championCount != heroSnapshotCount)
                    haveHeroSnapshot = 0;
                {
                    int completeSnapshot = 1;
                for (champion = 0; champion < championCount; ++champion) {
                    DM2_V1_RuntimeSourceHeroStateReceipt currentHero;
                    const DM2_V1_RuntimeSourceHeroStateReceipt *previousHero =
                        &previousHeroes[champion];
                    memset(&currentHero, 0, sizeof(currentHero));
                    if (!dm2_v1_runtime_get_source_hero_state(
                            (uint8_t)champion, &currentHero)) {
                        completeSnapshot = 0;
                        continue;
                    }
                    if (haveHeroSnapshot &&
                        memcmp(previousHero->name1, currentHero.name1,
                               sizeof(currentHero.name1)) == 0 &&
                        memcmp(previousHero->name2, currentHero.name2,
                               sizeof(currentHero.name2)) == 0 &&
                        previousHero->party_position ==
                            currentHero.party_position &&
                        previousHero->max_hp == currentHero.max_hp &&
                        previousHero->cur_hp != currentHero.cur_hp) {
                        fprintf(stderr,
                                "firestaff: game-health game=dm2 tick=%d "
                                "target=champion index=%d hp=%d->%d delta=%+d\n",
                                tick, champion, previousHero->cur_hp,
                                currentHero.cur_hp,
                                currentHero.cur_hp - previousHero->cur_hp);
                    }
                    previousHeroes[champion] = currentHero;
                }
                    if (completeSnapshot) {
                        heroSnapshotCount = championCount;
                        haveHeroSnapshot = 1;
                    } else {
                        haveHeroSnapshot = 0;
                    }
                }
            }
        }
        {
            size_t movementCount =
                dm2_v1_runtime_creature_movement_receipts_since(
                    &lastCreatureMovementSequence, movementReceipts,
                    sizeof(movementReceipts) / sizeof(movementReceipts[0]),
                    &movementDropped);
            size_t movementIndex;
            if (movementDropped)
                fprintf(stderr,
                        "firestaff: game-event game=dm2 tick=%d "
                        "type=monster-movement-gap dropped=%llu\n",
                        tick, (unsigned long long)movementDropped);
            for (movementIndex = 0; movementIndex < movementCount;
                 ++movementIndex) {
                const DM2_V1_RuntimeCreatureMovementReceipt *movementReceipt =
                    &movementReceipts[movementIndex];
                fprintf(stderr,
                        "firestaff: game-event game=dm2 tick=%u "
                        "type=monster-moved creature=%d type-id=%u "
                        "from=%d,%d,%d to=%d,%d,%d sequence=%llu\n",
                        movementReceipt->game_tick,
                        (int)movementReceipt->record_handle,
                        (unsigned int)movementReceipt->creature_type,
                        (int)movementReceipt->from_map,
                        (int)movementReceipt->from_x,
                        (int)movementReceipt->from_y,
                        (int)movementReceipt->to_map,
                        (int)movementReceipt->to_x,
                        (int)movementReceipt->to_y,
                        (unsigned long long)movementReceipt->sequence);
            }
        }
        memset(&wieldReceipt, 0, sizeof(wieldReceipt));
        if (dm2_v1_runtime_last_wield_attack_receipt(&wieldReceipt) &&
            wieldReceipt.valid &&
            (!haveWieldReceipt || memcmp(&wieldReceipt,
                                         &previousWieldReceipt,
                                         sizeof(wieldReceipt)) != 0)) {
            fprintf(stderr,
                    "firestaff: game-combat game=dm2 tick=%d type=player-attack "
                    "admitted=%d creature-resolved=%d creature=%d "
                    "hit=%d miss=%d raw-damage=%d final-damage=%d "
                    "hp-applied=%d fail-closed=%d\n",
                    tick, wieldReceipt.command_admitted,
                    wieldReceipt.creature_resolved,
                    (int)wieldReceipt.creature_record,
                    wieldReceipt.calculation_hit, wieldReceipt.calculation_miss,
                    (int)wieldReceipt.raw_damage,
                    (int)wieldReceipt.final_damage, wieldReceipt.hp_applied,
                    wieldReceipt.calculation_fail_closed);
            previousWieldReceipt = wieldReceipt;
            haveWieldReceipt = 1;
        }
        memset(&damageReceipt, 0, sizeof(damageReceipt));
        if (dm2_v1_runtime_last_creature_damage_receipt(&damageReceipt) &&
            damageReceipt.valid &&
            (!haveCreatureDamageReceipt || memcmp(&damageReceipt,
                                                  &previousDamageReceipt,
                                                  sizeof(damageReceipt)) != 0)) {
            fprintf(stderr,
                    "firestaff: game-combat game=dm2 tick=%d type=monster-damaged "
                    "creature=%d creature-type=%d damage=%d hp=%d->%d "
                    "wound=%d lethal=%d deallocated=%d drops=%d\n",
                    tick, (int)damageReceipt.creature_record,
                    damageReceipt.creature_type, damageReceipt.pending_damage,
                    damageReceipt.hp_before, damageReceipt.hp_after,
                    damageReceipt.wound_applied, damageReceipt.lethal,
                    damageReceipt.deallocated, damageReceipt.drops_placed);
            previousDamageReceipt = damageReceipt;
            haveCreatureDamageReceipt = 1;
        }
        {
            DM2_V1_RuntimeMissileImpactReceipt missileReceipt;
            memset(&missileReceipt, 0, sizeof(missileReceipt));
            if (dm2_v1_runtime_last_missile_impact_receipt(&missileReceipt) &&
                missileReceipt.valid &&
                missileReceipt.sequence > lastMissileImpactSequence) {
                fprintf(stderr,
                        "firestaff: game-combat game=dm2 tick=%d "
                        "type=missile-impact missile=%d creature=%d "
                        "creature-type=%d destination-hit=%d attack-completed=%d "
                        "hp-applied=%d damage=%d hp-after=%d consumed=%d "
                        "rescheduled=%d\n",
                        tick, (int)missileReceipt.missile_record,
                        (int)missileReceipt.creature_record,
                        missileReceipt.creature_type,
                        missileReceipt.destination_hit,
                        missileReceipt.attack_completed,
                        missileReceipt.hp_applied,
                        missileReceipt.damage_amount,
                        missileReceipt.damage_hp_word_after,
                        missileReceipt.missile_consumed,
                        missileReceipt.damage_rescheduled);
                lastMissileImpactSequence = missileReceipt.sequence;
            }
        }
    } else if (reportSummary && strcmp(state->sourceId, "dm1") == 0) {
        fprintf(stderr,
                "firestaff: game-debug-capabilities game=dm1 "
                "source=reDMCSB-derived-simulation "
                "events=sequenced-runtime-receipts "
                "player-attack=F0231-mutation "
                "monster-attack=F0207-resolution "
                "monster-movement=F0209-commit "
                "champion-health=F0889-application "
                "monster-health=F0190-mutation\n");
    }
}

/* Print source-world events only when the game tick actually emits them. This
 * keeps --debug useful during combat and ordinary play without per-frame spam.
 * The payload is intentionally retained verbatim so logs can be compared with
 * the source-engine event stream during parity work. */
static void m11_debug_game_events(const M11_GameViewState* state) {
    int i;
    if (!state || !state->debugMovementTraceEnabled || !state->active) return;
    m11_debug_native_game_state(state, 0);
    if (strcmp(state->sourceId, "dm1") == 0) {
        m11_debug_dm1_combat_receipt(state, "tick", 0);
        return;
    }
    if (state->sourceKind != M11_GAME_SOURCE_BUILTIN_CATALOG &&
        state->sourceKind != M11_GAME_SOURCE_CUSTOM_DUNGEON &&
        state->sourceKind != M11_GAME_SOURCE_DIRECT_DUNGEON) {
        const char* game = state->sourceId[0] ? state->sourceId : "unknown";
        if (state->sourceKind == M11_GAME_SOURCE_CSB_BOOT ||
            state->sourceKind == M11_GAME_SOURCE_DM2_BOOT) return;
        fprintf(stderr,
                "firestaff: game-event-stream game=%s source=native-runtime "
                "status=unavailable reason=compatibility-tick-result-is-not-authoritative\n",
                game);
        return;
    }
    if (g_m11_debug_health_snapshot.launchSerial != g_m11_sync_launch_serial) {
        g_m11_debug_health_snapshot.initialized = 0;
        memset(g_m11_debug_health_snapshot.championHealth, 0,
               sizeof(g_m11_debug_health_snapshot.championHealth));
        g_m11_debug_health_snapshot.launchSerial = g_m11_sync_launch_serial;
    }
    if (state->world.things && state->world.things->groups) {
        int groupCount = state->world.things->groupCount;
        if (groupCount < 0 || groupCount > 65536) groupCount = 0;
        if (g_m11_debug_health_snapshot.initialized &&
            (state->world.party.mapIndex != g_m11_debug_health_snapshot.mapIndex ||
             state->world.party.mapX != g_m11_debug_health_snapshot.partyMapX ||
             state->world.party.mapY != g_m11_debug_health_snapshot.partyMapY ||
             groupCount != g_m11_debug_health_snapshot.groupCount))
            g_m11_debug_health_snapshot.initialized = 0;
        if ((size_t)groupCount > g_m11_debug_health_snapshot.groupCapacity) {
            void* grown = realloc(g_m11_debug_health_snapshot.groups,
                (size_t)groupCount * sizeof(*g_m11_debug_health_snapshot.groups));
            if (grown) {
                g_m11_debug_health_snapshot.groups = grown;
                g_m11_debug_health_snapshot.groupCapacity =
                    (size_t)groupCount;
            }
        }
        if ((size_t)groupCount > g_m11_debug_health_snapshot.groupCapacity)
            groupCount = 0;
        for (i = 0; i < groupCount; ++i) {
            const struct DungeonGroup_Compat* group =
                &state->world.things->groups[i];
            const int sameGroup = g_m11_debug_health_snapshot.initialized &&
                g_m11_debug_health_snapshot.groups[i].next == group->next &&
                g_m11_debug_health_snapshot.groups[i].slot == group->slot &&
                g_m11_debug_health_snapshot.groups[i].creatureType ==
                    group->creatureType;
            int creature;
            for (creature = 0; creature < 4; ++creature) {
                unsigned int health = creature <= (int)group->count
                    ? group->health[creature] : 0U;
                unsigned int previous =
                    g_m11_debug_health_snapshot.groups[i].health[creature];
                if (g_m11_debug_health_snapshot.initialized &&
                    sameGroup &&
                    health != previous) {
                    fprintf(stderr,
                            "firestaff: game-health game=%s tick=%u "
                            "target=monster group=%d creature=%d type=%u "
                            "hp=%u->%u delta=%+d\n",
                            state->sourceId[0] ? state->sourceId : "unknown",
                            (unsigned int)state->world.gameTick, i, creature,
                            (unsigned int)group->creatureType, previous, health,
                            (int)health - (int)previous);
                }
                g_m11_debug_health_snapshot.groups[i].health[creature] = health;
            }
            g_m11_debug_health_snapshot.groups[i].next = group->next;
            g_m11_debug_health_snapshot.groups[i].slot = group->slot;
            g_m11_debug_health_snapshot.groups[i].creatureType =
                group->creatureType;
        }
        g_m11_debug_health_snapshot.groupCount = groupCount;
        g_m11_debug_health_snapshot.mapIndex = state->world.party.mapIndex;
        g_m11_debug_health_snapshot.partyMapX = state->world.party.mapX;
        g_m11_debug_health_snapshot.partyMapY = state->world.party.mapY;
    } else {
        g_m11_debug_health_snapshot.initialized = 0;
    }
    for (i = 0; i < state->world.party.championCount &&
                i < CHAMPION_MAX_PARTY; ++i) {
        unsigned int health = state->world.party.champions[i].hp.current;
        unsigned int previous = g_m11_debug_health_snapshot.championHealth[i];
        if (g_m11_debug_health_snapshot.initialized && health != previous) {
            fprintf(stderr,
                    "firestaff: game-health game=%s tick=%u "
                    "target=champion index=%d hp=%u->%u delta=%+d\n",
                    state->sourceId[0] ? state->sourceId : "unknown",
                    (unsigned int)state->world.gameTick, i, previous, health,
                    (int)health - (int)previous);
        }
        g_m11_debug_health_snapshot.championHealth[i] = health;
    }
    g_m11_debug_health_snapshot.initialized = 1;
    if (g_m11_debug_emission_snapshot.launchSerial !=
        g_m11_sync_launch_serial) {
        g_m11_debug_emission_snapshot.launchSerial =
            g_m11_sync_launch_serial;
        g_m11_debug_emission_snapshot.valid = 0;
    }
    if (state->lastTickResult.emissionCount > 0) {
        const uint64_t fingerprint =
            m11_debug_tick_emission_fingerprint(state);
        if (g_m11_debug_emission_snapshot.valid &&
            g_m11_debug_emission_snapshot.fingerprint == fingerprint)
            return;
        g_m11_debug_emission_snapshot.fingerprint = fingerprint;
        g_m11_debug_emission_snapshot.valid = 1;
    }
    for (i = 0; i < state->lastTickResult.emissionCount; ++i) {
        const struct TickEmission_Compat* event =
            &state->lastTickResult.emissions[i];
        char detail[160];
        detail[0] = '\0';
        switch (event->kind) {
            case EMIT_DAMAGE_DEALT:
                snprintf(detail, sizeof(detail),
                         "champion=%d group=%d amount-field=%d outcome-code=%d",
                         event->payload[0], event->payload[1],
                         event->payload[2], event->payload[3]);
                break;
            case EMIT_CREATURE_ATTACK:
                snprintf(detail, sizeof(detail),
                         "group=%d monster=%d damage-or-projectile=%d ranged=%d",
                         event->payload[0], event->payload[1],
                         event->payload[2], event->payload[3]);
                break;
            case EMIT_CREATURE_MOVED:
                snprintf(detail, sizeof(detail),
                         "group=%d monster-type=%d map-x=%d map-y=%d",
                         event->payload[0], event->payload[1],
                         event->payload[2], event->payload[3]);
                break;
            case EMIT_CHAMPION_DAMAGED:
                snprintf(detail, sizeof(detail),
                         "champion=%d cell=%d damage=%d wound-mask=%d",
                         event->payload[0], event->payload[1],
                         event->payload[2], event->payload[3]);
                break;
            case EMIT_DOOR_STATE:
                snprintf(detail, sizeof(detail), "map-x=%d map-y=%d state=%d map=%d",
                         event->payload[0], event->payload[1],
                         event->payload[2], event->payload[3]);
                break;
            case EMIT_ACTION_DISABLED:
                snprintf(detail, sizeof(detail),
                         "champion=%d ticks=%d action=%d slot=%d",
                         event->payload[0], event->payload[1],
                         event->payload[2], event->payload[3]);
                break;
            case EMIT_ACTION_ENABLED:
                snprintf(detail, sizeof(detail),
                         "champion=%d action=%d arg2=%d arg3=%d",
                         event->payload[0], event->payload[1],
                         event->payload[2], event->payload[3]);
                break;
            case EMIT_SPELL_EFFECT:
                snprintf(detail, sizeof(detail),
                         "champion=%d target=%d outcome=%d packed-power-skill-xp=%d",
                         event->payload[0], event->payload[1],
                         event->payload[2], event->payload[3]);
                break;
            default:
                snprintf(detail, sizeof(detail), "fields=see-payload");
                break;
        }
        fprintf(stderr,
                "firestaff: game-event game=%s tick=%u type=%s "
                "detail={%s} payload=%d,%d,%d,%d action=\"%s\" outcome=\"%s\" "
                "party=%d,%d dir=%d champions=%d active-champion=%d\n",
                state->sourceId[0] ? state->sourceId : "unknown",
                (unsigned int)state->world.gameTick,
                m11_debug_emission_name(event->kind),
                detail,
                event->payload[0], event->payload[1],
                event->payload[2], event->payload[3],
                state->lastAction[0] ? state->lastAction : "(none)",
                state->lastOutcome[0] ? state->lastOutcome : "(none)",
                state->world.party.mapX, state->world.party.mapY,
                state->world.party.direction,
                state->world.party.championCount,
                state->world.party.activeChampionIndex);
    }
}

static void m11_debug_game_action(const M11_GameViewState* state,
                                  M12_MenuInput input,
                                  M11_GameInputResult result) {
    if (!state || !state->debugMovementTraceEnabled || !state->active) return;
    if (state->sourceKind == M11_GAME_SOURCE_CSB_BOOT ||
        state->sourceKind == M11_GAME_SOURCE_DM2_BOOT) {
        fprintf(stderr,
                "firestaff: game-action game=%s tick=native input=%s "
                "result=%d action=\"%s\" outcome=\"%s\" "
                "compatibility-world=not-authoritative\n",
                state->sourceId[0] ? state->sourceId : "unknown",
                m11_debug_input_name(input), (int)result,
                state->lastAction[0] ? state->lastAction : "(none)",
                state->lastOutcome[0] ? state->lastOutcome : "(none)");
        m11_debug_native_game_state(state, 1);
        m11_debug_game_events(state);
        return;
    }
    fprintf(stderr,
            "firestaff: game-action game=%s tick=%u input=%s result=%d "
            "action=\"%s\" outcome=\"%s\" party=%d,%d dir=%d "
            "active-champion=%d\n",
            state->sourceId[0] ? state->sourceId : "unknown",
            (unsigned int)state->world.gameTick, m11_debug_input_name(input),
            (int)result,
            state->lastAction[0] ? state->lastAction : "(none)",
            state->lastOutcome[0] ? state->lastOutcome : "(none)",
            state->world.party.mapX, state->world.party.mapY,
            state->world.party.direction,
            state->world.party.activeChampionIndex);
    m11_debug_native_game_state(state, 1);
    m11_debug_game_events(state);
}

static void m11_debug_game_pointer(const M11_GameViewState* state,
                                   const char* eventName,
                                   int sourceX,
                                   int sourceY,
                                   M11_GameInputResult result) {
    if (!state || !state->debugMovementTraceEnabled || !state->active) return;
    if (state->sourceKind == M11_GAME_SOURCE_CSB_BOOT ||
        state->sourceKind == M11_GAME_SOURCE_DM2_BOOT) {
        fprintf(stderr,
                "firestaff: game-action game=%s tick=native input=%s "
                "point=%d,%d result=%d action=\"%s\" outcome=\"%s\" "
                "compatibility-world=not-authoritative\n",
                state->sourceId[0] ? state->sourceId : "unknown",
                eventName ? eventName : "pointer", sourceX, sourceY,
                (int)result,
                state->lastAction[0] ? state->lastAction : "(none)",
                state->lastOutcome[0] ? state->lastOutcome : "(none)");
        m11_debug_native_game_state(state, 1);
        m11_debug_game_events(state);
        return;
    }
    fprintf(stderr,
            "firestaff: game-action game=%s tick=%u input=%s point=%d,%d "
            "result=%d action=\"%s\" outcome=\"%s\" party=%d,%d dir=%d "
            "active-champion=%d\n",
            state->sourceId[0] ? state->sourceId : "unknown",
            (unsigned int)state->world.gameTick,
            eventName ? eventName : "pointer", sourceX, sourceY, (int)result,
            state->lastAction[0] ? state->lastAction : "(none)",
            state->lastOutcome[0] ? state->lastOutcome : "(none)",
            state->world.party.mapX, state->world.party.mapY,
            state->world.party.direction,
            state->world.party.activeChampionIndex);
    m11_debug_native_game_state(state, 1);
    m11_debug_game_events(state);
}

static void m11_draw_game_frame_for_loop(M11_GameViewState* state,
                                           int debug,
                                           double* drawMs,
                                           Uint64* drawCount) {
    const Uint64 start = debug ? SDL_GetPerformanceCounter() : 0;
    M11_GameView_Draw(state, M11_Render_GetFramebuffer(), M11_FB_WIDTH, M11_FB_HEIGHT);
    M11_GameView_DrawNexusPointerOverlay(
        state, M11_Render_GetFramebuffer(), M11_FB_WIDTH, M11_FB_HEIGHT);
    ++*drawCount;
    if (debug) {
        const Uint64 completed = SDL_GetPerformanceCounter();
        *drawMs += m11_counter_milliseconds(completed - start);
        m11_debug_motion_drawn(state, completed);
    }
}

static void m11_reset_debug_perf_window(Uint64* windowStart,
                                        Uint64* lastLoopEnd,
                                        Uint64* loopCount,
                                        Uint64* overBudgetLoops,
                                        Uint64* gameTickCount,
                                        Uint64* presentedCount,
                                        Uint64* drawCount,
                                        double* renderTotalMs,
                                        double* renderMaxMs,
                                        double* loopMaxMs,
                                        double* cpuWindowStart,
                                        int* cpuAvailable) {
    const Uint64 now = SDL_GetPerformanceCounter();
    *windowStart = now;
    *lastLoopEnd = now;
    *loopCount = 0;
    *overBudgetLoops = 0;
    *gameTickCount = 0;
    *presentedCount = 0;
    *drawCount = 0;
    *renderTotalMs = 0.0;
    *renderMaxMs = 0.0;
    *loopMaxMs = 0.0;
    *cpuAvailable = m11_process_cpu_seconds(cpuWindowStart);
}

static uint32_t m11_debug_game_tick_count(const M11_GameViewState* gameView) {
    if (!gameView) return 0U;
    switch (gameView->sourceKind) {
        case M11_GAME_SOURCE_CSB_BOOT:
        case M11_GAME_SOURCE_CSB_HINT_ORACLE:
            return (uint32_t)gameView->csbState.tick_count;
        case M11_GAME_SOURCE_DM2_BOOT:
            return (uint32_t)gameView->dm2State.tick_count;
        case M11_GAME_SOURCE_NEXUS_DGN:
            return (uint32_t)gameView->nexusState.tick_count;
        case M11_GAME_SOURCE_THERON_TRACK02:
            return (uint32_t)gameView->theronState.tick_count;
        default:
            return gameView->world.gameTick;
    }
}

/* Scope is installed from the current launch menu and removed on every launch
 * return. These host focus waits do not change ReDMCSB's VBlank counts. */
static const M12_StartupMenuState* g_m11_intro_menu;
static M11_GameViewState* g_m11_intro_game_view;
static M11_AudioState* g_m11_intro_local_audio;
static Uint64 g_m11_intro_paused_ms;

typedef struct {
    M11_AudioState* audio;
    M11_EntranceObserver observer;
    void* observerUser;
    Uint64 waitStartedMs;
    int waitStarted;
    int phase;
    int songBound;
    int playRequested;
} M11_SelectorMusicOwner;

static M11_SelectorMusicOwner* g_m11_selector_music;

static Uint64 m11_intro_active_ticks(void) {
    return SDL_GetTicks() - g_m11_intro_paused_ms;
}

static int m11_intro_wait_for_focus_impl(void) {
    SDL_Window* window;
    const char* driver;
    Uint64 started;
    int quit = 0;
    if (!g_m11_intro_menu || !g_m11_intro_menu->settings.autoPause) return 0;
    window = M11_Render_GetWindow();
    driver = SDL_GetCurrentVideoDriver();
    /* A dummy device has no desktop focus to regain. */
    if (!window) return 0;
    SDL_PumpEvents();
    if (!M11_FocusPauseRequired(g_m11_intro_menu->settings.autoPause,
            (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0,
            driver)) return 0;
    started = SDL_GetTicks();
    if (g_m11_intro_local_audio)
        (void)M11_Audio_SetHostPaused(g_m11_intro_local_audio, 1);
    if (g_m11_intro_game_view)
        M11_GameView_SetPauseReason(g_m11_intro_game_view,
                                    M11_GAME_PAUSE_REASON_FOCUS, 1);
    while (!(SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)) {
        /* Pump quit/resize while suspended; no source animation advances. */
        if (m11_phase_duration_expired()) { quit = 1; break; }
        if (M11_Render_PumpEvents()) {
            g_m11_startup_intro_quit_requested = 1;
            quit = 1;
            break;
        }
        SDL_Delay(10U);
        SDL_PumpEvents();
    }
    g_m11_intro_paused_ms += SDL_GetTicks() - started;
    if (g_m11_intro_local_audio)
        (void)M11_Audio_SetHostPaused(g_m11_intro_local_audio, 0);
    if (g_m11_intro_game_view)
        M11_GameView_SetPauseReason(g_m11_intro_game_view,
                                    M11_GAME_PAUSE_REASON_FOCUS, 0);
    return quit;
}

static int m11_intro_wait_for_focus(void) {
    M11_SelectorMusicOwner* owner = g_m11_selector_music;
    Uint64 elapsedMs = 0U;
    int quit = m11_intro_wait_for_focus_impl();
    if (quit || !owner || !owner->phase) return quit;
    if (!owner->waitStarted) {
        owner->waitStarted = 1;
        owner->waitStartedMs = m11_intro_active_ticks();
    }
    elapsedMs = m11_intro_active_ticks() - owner->waitStartedMs;
    if (owner->songBound && owner->phase != M11_ENTRANCE_PHASE_DOORS) {
        /* SELECTOR.C:515, F8367:814-825 waits 60 VIDRV_07 retraces.
         * VIDEODRV.C F8153:3163-3185 polls VGA 0x3DA, mode 13h (~70 Hz).
         * Keep the rational deadline instead of rounding each tick to 14 ms.
         * This is the PC VGA selector, not the PAL/gameplay VBlank clock. */
        const Uint64 initialDelayMs =
            (60U * 1000U + DM1_V1_VGA_VBLANK_HZ - 1U) /
            DM1_V1_VGA_VBLANK_HZ;
        if (!owner->playRequested && elapsedMs >= initialDelayMs) {
            owner->playRequested = 1;
            (void)M11_Audio_PlayTitleMusic(owner->audio);
        }
        if (owner->playRequested) {
            (void)M11_Audio_PumpTitleMusic(owner->audio);
        }
    }
    if (owner->observer) {
        owner->observer(owner->observerUser, owner->phase, elapsedMs,
                          owner->audio);
    }
    return 0;
}


static unsigned int m11_startup_remaining_delay_ms(
    unsigned int source_delay_ms,
    Uint64 presentation_started_ms)
{
    /* Source waits start when a frame is submitted. A VSync-backed present
     * consumes part of that interval; sleeping the full source duration
     * afterward would make startup cadence depend on host refresh rate. */
    if (presentation_started_ms == 0U) {
        return source_delay_ms;
    }
    return dm1_v20_startup_presentation_remaining_delay_ms_pc34(
        source_delay_ms, m11_intro_active_ticks() - presentation_started_ms);
}

static int m11_wait_for_entrance_credits_done(unsigned int wait_ticks,
                                              int input_only,
                                              unsigned int vblank_delay_ms,
                                              Uint64 presentation_started_ms) {
    unsigned int ticks;
    SDL_Event ev;
    const int v20TimingActive = dm1_v2_presentation_mode_is_v20();
    const Uint64 sourceDeadlineMs = presentation_started_ms +
        (Uint64)wait_ticks * (Uint64)vblank_delay_ms;
    /* ReDMCSB ENTRANCE.C:1012-1091 F0442 sets L1406=1800, discards stale
     * keyboard input, then waits one delay/vblank per tick until any
     * keyboard or mouse input is present before returning to the entrance
     * command loop. */
    while (SDL_PollEvent(&ev)) {
        (void)ev;
    }
    /* SELECTOR.C:1002-1004 (MEDIA707_I34E_I34M) instead pumps F8367
     * until F8327 reports input, without the ENTRANCE.C timeout. */
    for (ticks = 0U; input_only || ticks < wait_ticks; ++ticks) {
        if (m11_intro_wait_for_focus()) return M11_ENTRANCE_COMMAND_QUIT;
        while (SDL_PollEvent(&ev)) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
            if (ev.type == SDL_EVENT_QUIT) return M11_ENTRANCE_COMMAND_QUIT;
            if (ev.type == SDL_EVENT_KEY_DOWN ||
                ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                ev.type == SDL_EVENT_FINGER_DOWN) {
                return M11_ENTRANCE_COMMAND_NONE;
            }
            if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            }
#else
            if (ev.type == SDL_QUIT) return M11_ENTRANCE_COMMAND_QUIT;
            if (ev.type == SDL_KEYDOWN ||
                ev.type == SDL_MOUSEBUTTONDOWN ||
                ev.type == SDL_FINGERDOWN) {
                return M11_ENTRANCE_COMMAND_NONE;
            }
            if (ev.type == SDL_WINDOWEVENT &&
                ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            }
#endif
        }
        if (v20TimingActive && !input_only) {
            const Uint64 nowMs = m11_intro_active_ticks();
            const Uint64 remainingMs = nowMs >= sourceDeadlineMs
                ? 0U : sourceDeadlineMs - nowMs;
            if (remainingMs == 0U) {
                return M11_ENTRANCE_COMMAND_NONE;
            }
            SDL_Delay((unsigned int)(remainingMs < vblank_delay_ms
                ? remainingMs : vblank_delay_ms));
        } else {
            SDL_Delay(vblank_delay_ms);
        }
    }
    return M11_ENTRANCE_COMMAND_NONE;
}

/* TITLE.C/ENTRANCE.C compose their authentic indexed page before scaling.
 * Route that completed page through the selected presentation target while
 * retaining the source palette.  Calling the 320x200 presenter directly
 * made Original correct but silently ignored Modern's resolution choice. */
static int m11_present_dm1_startup_special_palette(
    const M11_GameViewState* gameView,
    const unsigned char* framebuffer,
    int specialPalette) {
    int targetW = M11_FB_WIDTH;
    int targetH = M11_FB_HEIGHT;
    int result;

    if (!gameView || !framebuffer || specialPalette < 0) return 0;
    M11_Render_SetV2PresentationActive(
        m11_dm1_v20_presentation_active(gameView));
    M11_Render_SetModernPresentationActive(
        gameView->presentationMode != M12_PRESENTATION_V1_ORIGINAL);
    if (gameView->presentationMode == M12_PRESENTATION_V21_UPSCALED) {
        (void)M11_GameView_PresentationTarget(
            gameView->presentationMode,
            gameView->presentationWidth,
            gameView->presentationHeight,
            &targetW,
            &targetH);
        result = M11_Render_PresentEpxIndexedToResolutionWithSpecialPalette(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
            targetW, targetH, specialPalette);
    } else if (M11_GameView_PresentationTarget(
                   gameView->presentationMode,
                   gameView->presentationWidth,
                   gameView->presentationHeight,
                   &targetW,
                   &targetH)) {
        result = M11_Render_PresentIndexedToResolutionWithSpecialPalette(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
            targetW, targetH, specialPalette);
    } else {
        result = M11_Render_PresentIndexedWithSpecialPalette(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, specialPalette);
    }
    return result == M11_RENDER_OK;
}

static int m11_present_dm1_startup_base_palette(
    const M11_GameViewState* gameView,
    const unsigned char* framebuffer) {
    int targetW = M11_FB_WIDTH;
    int targetH = M11_FB_HEIGHT;
    int result;
    if (!gameView || !framebuffer) return 0;
    M11_Render_SetV2PresentationActive(
        m11_dm1_v20_presentation_active(gameView));
    M11_Render_SetModernPresentationActive(
        gameView->presentationMode != M12_PRESENTATION_V1_ORIGINAL);
    if (gameView->presentationMode == M12_PRESENTATION_V21_UPSCALED) {
        (void)M11_GameView_PresentationTarget(
            gameView->presentationMode,
            gameView->presentationWidth,
            gameView->presentationHeight,
            &targetW,
            &targetH);
        result = M11_Render_PresentEpxIndexedToResolution(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, targetW, targetH);
    } else if (M11_GameView_PresentationTarget(
            gameView->presentationMode,
            gameView->presentationWidth,
            gameView->presentationHeight,
            &targetW,
            &targetH)) {
        result = M11_Render_PresentIndexedToResolution(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, targetW, targetH);
    } else {
        result = M11_Render_PresentIndexed(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    }
    return result == M11_RENDER_OK;
}

static int m11_show_redmcsb_entrance_credits(M11_GameViewState* gameView,
                                             unsigned char* framebuffer,
                                             const DM1_V1_StartupFullGraphicsMediaReceipt_PC34*
                                                 media_receipt,
                                             DM1_V1_StartupEntranceCreditsPresentationCommand_PC34*
                                                 out_command) {
    const M11_AssetSlot* credits;
    DM1_V1_StartupEntranceCreditsPresentationCommand_PC34 command;
    Uint64 presentationStartedMs;
    int waitResult;
    if (!gameView || !framebuffer) return M11_ENTRANCE_COMMAND_NONE;
    if (!m11_draw_entrance_credits_asset(gameView, framebuffer)) {
        return M11_ENTRANCE_COMMAND_NONE;
    }
    credits = M11_AssetLoader_Load(&gameView->assetLoader, 5U);
    if (!credits || !dm1_v1_startup_entrance_credits_presentation_command_pc34(
                        media_receipt, credits->pixels, credits->width,
                        credits->height, &command)) {
        return M11_ENTRANCE_COMMAND_NONE;
    }
    M11_AssetLoader_Blit(credits, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
                         0, 0, -1);
    presentationStartedMs = m11_intro_active_ticks();
    if (command.special_palette >= 0
            ? !m11_present_dm1_startup_special_palette(
                  gameView, framebuffer, command.special_palette)
            : !m11_present_dm1_startup_base_palette(gameView, framebuffer)) {
        return M11_ENTRANCE_COMMAND_NONE;
    }
    if (g_m11_selector_music)
        g_m11_selector_music->phase = M11_ENTRANCE_PHASE_CREDITS;
    /* ENTRANCE.C F0442:1067-1091 polls after each VBlank. The command
     * stores the whole credits timeout; use the receipt per-VBlank cadence
     * here so one SDL_Delay does not block input/audio for 36 seconds. */
    waitResult = m11_wait_for_entrance_credits_done(
        command.credits_wait_ticks,
        media_receipt->platform == DM1_V1_STARTUP_MEDIA_PLATFORM_PC34 &&
        !gameView->dm1FmtownsStartupReceiptValid &&
        strcmp(gameView->sourceId, "dm1") == 0,
        media_receipt->entrance_vblank_ms, presentationStartedMs);
    if (out_command) *out_command = command;
    return waitResult;
}

static int m11_draw_entrance_opening_doors_asset(M11_GameViewState* gameView,
                                                 unsigned char* framebuffer,
                                                 const unsigned char* dungeonFrame,
                                                 const EntranceCompatDoorStep* door) {
    const M11_AssetSlot* entranceScreen;
    const M11_AssetSlot* leftDoor;
    const M11_AssetSlot* rightDoor;
    EntranceCompatCompositePixels pixels;
    if (!gameView || !framebuffer || !dungeonFrame || !door || !gameView->assetsAvailable) {
        return 0;
    }
    entranceScreen = M11_AssetLoader_Load(&gameView->assetLoader, 4U);
    leftDoor = M11_AssetLoader_Load(&gameView->assetLoader, 2U);
    rightDoor = M11_AssetLoader_Load(&gameView->assetLoader, 3U);
    if (!entranceScreen || !leftDoor || !rightDoor) {
        return 0;
    }
    memset(&pixels, 0, sizeof(pixels));
    pixels.entranceScreen = entranceScreen->pixels;
    pixels.entranceWidth = entranceScreen->width;
    pixels.entranceHeight = entranceScreen->height;
    pixels.dungeonFrame = dungeonFrame;
    pixels.dungeonFrameWidth = M11_FB_WIDTH;
    pixels.dungeonFrameHeight = M11_FB_HEIGHT;
    pixels.leftDoor = leftDoor->pixels;
    pixels.leftDoorWidth = leftDoor->width;
    pixels.leftDoorHeight = leftDoor->height;
    pixels.rightDoor = rightDoor->pixels;
    pixels.rightDoorWidth = rightDoor->width;
    pixels.rightDoorHeight = rightDoor->height;
    return ENTRANCE_Compat_CompositeDoorOpeningFrame(framebuffer,
                                                     M11_FB_WIDTH,
                                                     M11_FB_HEIGHT,
                                                     &pixels,
                                                     door);
}

static int m11_entrance_input_event_is_fresh(const SDL_Event* event,
                                              Uint64 waitStartedCounter,
                                              const char* inputKind) {
    if (!event || ENTRANCE_Compat_IsFreshInputCounter(
            m11_sdl_event_timestamp_to_counter(event->common.timestamp),
            waitStartedCounter)) {
        return 1;
    }
    if (g_m11_debug_startup_detail) {
        fprintf(stderr,
                "firestaff: startup-input-ignore game=dm1 phase=entrance reason=stale-event input=%s\n",
                inputKind ? inputKind : "unknown");
    }
    return 0;
}

static M11_EntranceCommand m11_wait_for_redmcsb_entrance_command(
    int autoEnterAfterMs,
    int mouseOnly);
static int m11_dm1_amiga_present_entrance_palette(
    const M11_GameViewState* gameView,
    const uint16_t colors[DM1_V1_AMIGA_ENTRANCE_F0441_PALETTE_ENTRIES],
    const uint8_t* framebuffer);
static int m11_delay_ms_with_intro_event_pump(unsigned int delayMs);
static M12_MenuInput m11_next_script_input(const char** cursor,
                                          int* outWaitFrames,
                                          int* outWaitForRuntime,
                                          int* outWaitForScan);
static M12_MenuInput m11_map_script_token(const char* token, size_t len);
static int m11_push_script_event_token(const char* token, size_t len);
static int m11_script_event_token_is_valid(const char* token, size_t len);
static int m11_game_view_is_dm1(const M11_GameViewState* gameView);
static int m11_apply_boot_probe_event_token(M11_GameViewState* gameView,
                                            const char* token,
                                            size_t len,
                                            M11_GameInputResult* outResult);
static int m11_boot_probe_expected_source_kind(const char* gameId,
                                               M11_GameSourceKind* outKind);
static int m11_selected_dm1_launch_version_index(
    const M12_StartupMenuState* menuState,
    const M12_MenuEntry* entry);

static EntranceCompatKey m11_entrance_compat_key_from_sdl_key(int keyCode) {
    switch (keyCode) {
    case SDLK_RETURN:
        return ENTRANCE_COMPAT_KEY_RETURN;
    case SDLK_KP_ENTER:
        return ENTRANCE_COMPAT_KEY_KEYPAD_RETURN;
    case SDLK_ESCAPE:
        return ENTRANCE_COMPAT_KEY_ESCAPE;
    case SDLK_Q:
        return ENTRANCE_COMPAT_KEY_Q;
    case SDLK_SPACE:
        return ENTRANCE_COMPAT_KEY_SPACE;
    default:
        return ENTRANCE_COMPAT_KEY_OTHER;
    }
}

static int m11_entrance_dispatch_source_locked_key_command(int keyCode) {
    return ENTRANCE_Compat_DispatchKeyCommand(m11_entrance_compat_key_from_sdl_key(keyCode));
}

/* Public focused-test and host-input entry point. Keep this wrapper on the
 * SDL-keycode translation route used by the entrance wait loop; linking a
 * test directly to ENTRANCE_Compat_DispatchKeyCommand would incorrectly pass
 * SDLK_RETURN/SDLK_ESCAPE values as EntranceCompatKey enum values. */
int M11_Entrance_DispatchSourceLockedKeyCommand(int keyCode) {
    return m11_entrance_dispatch_source_locked_key_command(keyCode);
}

static M11_EntranceCommand m11_entrance_command_path_from_source_command(int commandId) {
    return (M11_EntranceCommand)ENTRANCE_Compat_CommandPathFromSourceCommand(commandId);
}

static int m11_dm1_amiga36_fade(
    M11_GameViewState *gameView,
    const DM1_V1_AmigaEntranceF0441Receipt *receipt,
    const uint16_t source[16], const uint16_t target[16], uint8_t *page) {
    for (unsigned int frame = 1; frame <= 15; ++frame) {
        uint16_t palette[16];
        Uint64 started = m11_intro_active_ticks();
        Uint64 elapsed;
        unsigned int remaining;
        if (!dm1_v1_amiga36_entrance_fade_step(receipt, source, target,
                frame, palette) ||
            !m11_dm1_amiga_present_entrance_palette(gameView, palette, page))
            return 0;
        elapsed = m11_intro_active_ticks() - started;
        remaining = elapsed >= 20u ? 0u : 20u - (unsigned int)elapsed;
        if (remaining && m11_delay_ms_with_intro_event_pump(remaining)) return 0;
    }
    if (g_m11_debug_startup_detail)
        fprintf(stderr, "firestaff: startup-source-fade game=dm1 platform=amiga-v36 frames=15 driver=%s\n",
                receipt->a36_driver_md5);
    return 1;
}

/* A36M ENTRANCE.C F0439/F0438 uses C696, not the older A20 door boxes. */
static int m11_play_dm1_amiga36_entrance(
    M11_GameViewState *gameView, int autoEnterAfterMs,
    const DM1_V1_AmigaEntranceF0441Receipt *receipt) {
    static const uint16_t credits_palette[16] = {
        0x006,0x0aa,0xff6,0x840,0xff8,0x000,0x080,0xa00,
        0xc84,0xffa,0xf84,0xfc0,0xfa0,0x000,0x620,0xffc
    }; /* DATA.C G0019, MEDIA424 including A36M. */
    static const uint16_t blank_palette[16] = {0};
    uint8_t *page = M11_Render_GetFramebuffer();
    uint8_t *micro = malloc(224u * 136u);
    M11_EntranceCommand command;
    if (!page || !micro || !receipt || !receipt->a36_layout_valid || !receipt->a36_fade_valid ||
        !dm1_v1_amiga36_entrance_render_c255(&gameView->assetLoader,
                                            micro, 224u * 136u)) {
        free(micro); return 0;
    }
    for (;;) {
        if (!dm1_v1_amiga36_entrance_compose(&gameView->assetLoader,
                receipt, micro, 0, -1, page, M11_FB_BYTES) ||
            !m11_dm1_amiga_present_entrance_palette(gameView, blank_palette, page) ||
            !m11_dm1_amiga36_fade(gameView, receipt, blank_palette,
                receipt->entrance_palette_rgb4, page)) {
            free(micro); return 0;
        }
        m11_capture_dm1_authenticated_startup_frame("a36-closed");
        command = m11_wait_for_redmcsb_entrance_command(autoEnterAfterMs, 0);
        if (command != M11_ENTRANCE_COMMAND_CREDITS) break;
        {
            const M11_AssetSlot *credits =
                M11_AssetLoader_Load(&gameView->assetLoader, 5);
            Uint64 started;
            if (!credits || !credits->pixels || credits->width != 320 ||
                credits->height != 200) { free(micro); return 0; }
            if (!m11_dm1_amiga36_fade(gameView, receipt,
                    receipt->entrance_palette_rgb4, blank_palette, page)) {
                free(micro); return M11_ENTRANCE_COMMAND_QUIT;
            }
            memcpy(page, credits->pixels, M11_FB_BYTES);
            if (!m11_dm1_amiga36_fade(gameView, receipt, blank_palette,
                    credits_palette, page)) { free(micro); return 0; }
            /* ENTRANCE.C F0442:993 starts its 1800-VBlank wait after
             * presenting the credits palette. */
            started = m11_intro_active_ticks();
            if (m11_wait_for_entrance_credits_done(1800, 0, 20, started) ==
                    M11_ENTRANCE_COMMAND_QUIT) {
                free(micro); return M11_ENTRANCE_COMMAND_QUIT;
            }
            if (!m11_dm1_amiga36_fade(gameView, receipt, credits_palette,
                    blank_palette, page)) {
                free(micro); return M11_ENTRANCE_COMMAND_QUIT;
            }
        }
    }
    if (command == M11_ENTRANCE_COMMAND_QUIT ||
        command == M11_ENTRANCE_COMMAND_NONE) { free(micro); return command; }
    if (!dm1_v1_amiga36_entrance_compose(&gameView->assetLoader,
            receipt, micro, 0, command == M11_ENTRANCE_COMMAND_RESUME,
            page, M11_FB_BYTES) ||
        !m11_dm1_amiga_present_entrance_palette(gameView,
            receipt->entrance_palette_rgb4, page)) { free(micro); return 0; }
    (void)M11_Audio_EmitSourceSoundIndex(&gameView->audioState, 1);
    if (m11_delay_ms_with_intro_event_pump(receipt->switch_delay_ticks * 20u)) {
        free(micro); return M11_ENTRANCE_COMMAND_QUIT;
    }
    if (command == M11_ENTRANCE_COMMAND_RESUME) { free(micro); return command; }
    for (unsigned int frame = 1; frame <= receipt->opening_steps; ++frame) {
        Uint64 started = m11_intro_active_ticks();
        Uint64 elapsed;
        unsigned int remaining;
        if (!dm1_v1_amiga36_entrance_compose(&gameView->assetLoader,
                receipt, micro, frame, 0, page, M11_FB_BYTES) ||
            !m11_dm1_amiga_present_entrance_palette(gameView,
                receipt->entrance_palette_rgb4, page)) { free(micro); return 0; }
        if (frame == 1 || frame == (receipt->opening_steps / 2) ||
            frame == receipt->opening_steps) {
            char phase[48];
            snprintf(phase, sizeof(phase), "a36-door-%u", frame);
            m11_capture_dm1_authenticated_startup_frame(phase);
        }
        /* F0438's rattle deadline is forty VBlanks; each A36 frame is two. */
        if ((frame - 1) % 20 == 0)
            (void)M11_Audio_EmitSourceSoundIndex(&gameView->audioState, 3);
        elapsed = m11_intro_active_ticks() - started;
        remaining = elapsed >= 40u ? 0u : 40u - (unsigned int)elapsed;
        if (g_m11_debug_startup_detail)
            fprintf(stderr, "firestaff: startup-source-frame game=dm1 platform=amiga-v36 phase=entrance-door frame=%u/%u step-pixels=%u vblanks=2 wait-ms=%u\n",
                frame, receipt->opening_steps, receipt->a36_step_pixels, remaining);
        if (frame < receipt->opening_steps && remaining &&
            m11_delay_ms_with_intro_event_pump(remaining)) {
            free(micro); return M11_ENTRANCE_COMMAND_QUIT;
        }
    }
    free(micro);
    return M11_ENTRANCE_COMMAND_ENTER;
}

static int m11_play_redmcsb_entrance_transition_impl(
    M11_GameViewState* gameView,
    int autoEnterAfterMs,
    const DM1_V1_EntranceFullStartRenderReceiptPc34* entranceReceipt,
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* mediaReceipt,
    const DM1_V1_AmigaEntranceF0441Receipt* amigaReceipt) {
    if (gameView && gameView->active && amigaReceipt &&
        amigaReceipt->valid && amigaReceipt->a36_layout_valid)
        return m11_play_dm1_amiga36_entrance(gameView, autoEnterAfterMs,
                                            amigaReceipt);
    unsigned char* framebuffer;
    unsigned char* dungeonFrame;
    unsigned int sourceStep;
    unsigned int amigaDoorFrameOrdinal = 0U;
    unsigned int fmtownsDoorFrameOrdinal = 0U;
    Uint64 previousAmigaDoorPresentationMs = 0U;
    Uint64 previousFmtownsDoorPresentationMs = 0U;
    Uint64 previousPcDoorPresentationMs = 0U;
    int entrancePalette;
    const int amigaReceiptValid = amigaReceipt && amigaReceipt->valid;
    if (!gameView || !gameView->active || !mediaReceipt ||
        (!amigaReceiptValid &&
         !dm1_v1_startup_entrance_timing_receipt_valid_pc34(mediaReceipt)) ||
        (amigaReceiptValid &&
         (amigaReceipt->opening_steps !=
              DM1_V1_AMIGA_ENTRANCE_F0441_OPENING_STEPS ||
          (amigaReceipt->mouse_input_only &&
           amigaReceipt->has_entrance_buttons_graphic)))) {
        return 0;
    }
    /* Startup presentation runs before the ordinary frame loop selects the
     * V2.0 lane. Keep its original special-palette frames in that same lane. */
    M11_Render_SetV2PresentationActive(
        m11_dm1_v20_presentation_active(gameView));
    entrancePalette = mediaReceipt->entrance_palette;
    if (!amigaReceiptValid && entrancePalette != VGA_PALETTE_PC34_SPECIAL_ENTRANCE &&
        !(mediaReceipt->platform ==
              DM1_V1_STARTUP_MEDIA_PLATFORM_ATARI_ST &&
          entrancePalette == -1)) {
        return 0;
    }
    if (!amigaReceiptValid &&
        !DM1_V1_Entrance_FullStartRenderReceiptHostReadyPc34Compat(
            entranceReceipt)) {
        return 0;
    }
    framebuffer = M11_Render_GetFramebuffer();
    if (!framebuffer) return 0;
    dungeonFrame = (unsigned char*)malloc((size_t)M11_FB_BYTES);
    if (!dungeonFrame) return 0;

    if (gameView->dm1FmtownsStartupReceiptValid) {
        int entranceTrack = DM1_FMTOWNS_TRACK_ENTRANCE_MAP6;
        if (gameView->dm1FmtownsCddaCurrentTrack != entranceTrack) {
            M11_GameView_PlayFmtownsCdda(gameView, entranceTrack);
        }
    }

    M11_GameView_Draw(gameView, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    if (gameView->dm1FmtownsStartupReceiptValid) {
        DM1_V1_FmtownsEntranceViewportReceipt c255Receipt;
        int row;
        int c255Rendered = M11_GameView_RenderDm1FmtownsEntranceC255(
                gameView, dungeonFrame,
                (size_t)M11_FB_BYTES,
                DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH,
                &c255Receipt);
        int apertureValid = c255Receipt.aperture_x >= 0 &&
            c255Receipt.aperture_y >= 0 &&
            c255Receipt.aperture_x +
                DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH <= M11_FB_WIDTH &&
            c255Receipt.aperture_y +
                DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT <= M11_FB_HEIGHT;
        if (g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "[dm1-fmtowns-c255] edition=%s language=%s result=%s valid=%d aperture=(%d,%d %dx%d) requested=0x%08x loaded=0x%08x missing=0x%08x source-records=0x%08x raster_fnv1a=0x%08x reason=%s\n",
                    gameView->dm1FmtownsStartupReceipt.language ==
                            DM1_FMTOWNS_LANG_JP ? "JDM" : "EDM",
                    gameView->dm1FmtownsStartupReceipt.language ==
                            DM1_FMTOWNS_LANG_JP ? "ja" : "en",
                    c255Rendered && c255Receipt.valid && apertureValid
                        ? "ok" : "failed",
                    c255Receipt.valid,
                    c255Receipt.aperture_x, c255Receipt.aperture_y,
                    DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH,
                    DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT,
                    (unsigned)c255Receipt.requested_source_graphics,
                    (unsigned)c255Receipt.loaded_source_graphics,
                    (unsigned)c255Receipt.missing_source_graphics,
                    (unsigned)c255Receipt.source_graphic_record_mask,
                    (unsigned)c255Receipt.pixel_hash_fnv1a,
                    !c255Rendered ? "renderer-or-authenticated-assets-rejected"
                        : !c255Receipt.valid ? "receipt-invalid"
                        : !apertureValid ? "aperture-out-of-framebuffer"
                        : "none");
        }
        if (!c255Rendered || !c255Receipt.valid || !apertureValid ||
            c255Receipt.missing_source_graphics != 0u) {
            free(dungeonFrame);
            return 0;
        }
        /* ENTRANCE.C F0797 renders into the live viewport before F0439
         * snapshots the dungeon page. COORD.C G2067/G2068 owns its origin:
         * (0,33) for English and (0,31) for Japanese. Keep the HUD and the
         * rest of the page from M11_GameView_Draw byte-for-byte intact. */
        for (row = 0; row < DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT; ++row) {
            memcpy(framebuffer +
                       (size_t)(c255Receipt.aperture_y + row) * M11_FB_WIDTH +
                       (size_t)c255Receipt.aperture_x,
                   dungeonFrame +
                       (size_t)row *
                           DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH,
                   DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH);
        }
    }
    memcpy(dungeonFrame, framebuffer, (size_t)M11_FB_BYTES);

    /* ENTRANCE.C F0439 first composites the micro-dungeon, curtains it to
     * black, then draws C004/C002/C003.  Do not pre-present closed doors:
     * doing so inserted an extra frame before that source sequence and made
     * the title-to-Entrance transition flash abruptly. */

    /* ReDMCSB ENTRANCE.C source-lock:
     * - F0441_STARTEND_ProcessEntrance() waits in entrance mode until C200.
     * - ENTRANCE.C:935 delays 20 ticks before opening.
     * - F0438_STARTEND_OpenEntranceDoors() runs 31 one-VBlank steps.
     * - ENTRANCE.C:149-231 moves the left/right door boxes by 4px/step
     *   from DATA.C source boxes left {0,100,0,160}, right {109,231,0,160}.
     * This runtime transition uses the source schedule/boxes here; C004 and
     * C002/C003 are blitted from GRAPHICS.DAT. Missing source material aborts
     * the presentation rather than substituting generated pixels. */
    for (sourceStep = 1U; sourceStep <= ENTRANCE_Compat_GetSourceAnimationStepCount(); ++sourceStep) {
        EntranceCompatSourceAnimationStep step;
        DM1_V1_StartupEntranceRenderAudioCommand_PC34 command;
        Uint64 presentationStartedMs = 0U;
        if (!ENTRANCE_Compat_GetSourceAnimationStep(sourceStep, &step)) break;
        memset(&command, 0, sizeof(command));
        if (amigaReceiptValid) {
            command.handled = 1;
            command.render_kind =
                step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_FADE_TO_BLACK
                    ? DM1_V1_STARTUP_ENTRANCE_RENDER_FADE_BLACK_PC34
                    : (step.kind ==
                           ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP
                           ? DM1_V1_STARTUP_ENTRANCE_RENDER_OPENING_DOOR_PC34
                           : (step.kind ==
                                  ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_MICRO_DUNGEON ||
                              step.kind ==
                                  ENTRANCE_COMPAT_SOURCE_EVENT_FINAL_DUNGEON_VIEW
                                  ? DM1_V1_STARTUP_ENTRANCE_RENDER_DUNGEON_FRAME_PC34
                                  : DM1_V1_STARTUP_ENTRANCE_RENDER_CLOSED_DOORS_PC34));
            command.present_entrance_palette = 1;
            command.source_step = sourceStep;
            command.delay_ms = step.delayTicks * 20u;
            if (step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP) {
                EntranceCompatDoorStep sourceDoor;
                if (!ENTRANCE_Compat_GetDoorAnimationStep(
                        sourceStep - 6u, &sourceDoor)) {
                    free(dungeonFrame);
                    return 0;
                }
                command.door_geometry_ready = 1;
                command.door_animation_step = sourceDoor.animationStep;
                command.door_left_box_x = sourceDoor.leftBoxX;
                command.door_left_box_y = sourceDoor.leftBoxY;
                command.door_left_box_w = sourceDoor.leftBoxW;
                command.door_left_box_h = sourceDoor.leftBoxH;
                command.door_right_box_x = sourceDoor.rightBoxX;
                command.door_right_box_y = sourceDoor.rightBoxY;
                command.door_right_box_w = sourceDoor.rightBoxW;
                command.door_right_box_h = sourceDoor.rightBoxH;
                command.door_left_source_x = sourceDoor.leftSourceX;
                command.door_right_source_x = sourceDoor.rightSourceX;
                command.play_door_rattle_sound = sourceDoor.soundRattle;
                command.audio_request_ready = sourceDoor.soundRattle;
                command.audio_sound_index = 2;
                command.delay_ms = 40u;
            } else if (step.kind ==
                       ENTRANCE_COMPAT_SOURCE_EVENT_SWITCH_SOUND) {
                command.audio_request_ready = 1;
                command.audio_sound_index = 1;
            }
        } else if (!dm1_v1_startup_entrance_render_audio_command_pc34(
                       mediaReceipt, sourceStep, (int)step.kind,
                       step.delayTicks, step.vblankLoopCount, &command)) {
            free(dungeonFrame);
            return 0;
        }

        if (command.render_kind ==
            DM1_V1_STARTUP_ENTRANCE_RENDER_FADE_BLACK_PC34) {
            memset(framebuffer, 0, (size_t)M11_FB_BYTES);
        } else if (command.render_kind ==
                   DM1_V1_STARTUP_ENTRANCE_RENDER_CLOSED_DOORS_PC34) {
            if (!m11_draw_entrance_screen_asset(gameView, framebuffer) ||
                !m11_draw_entrance_closed_doors_asset(gameView, framebuffer)) {
                free(dungeonFrame);
                return 0;
            }
        } else if (command.render_kind ==
                   DM1_V1_STARTUP_ENTRANCE_RENDER_OPENING_DOOR_PC34) {
            EntranceCompatDoorStep door;
            if (command.door_geometry_ready) {
                memset(&door, 0, sizeof(door));
                door.animationStep = command.door_animation_step;
                if (amigaReceiptValid) {
                    amigaDoorFrameOrdinal = door.animationStep;
                } else if (gameView->dm1FmtownsStartupReceiptValid) {
                    fmtownsDoorFrameOrdinal = door.animationStep;
                }
                door.soundRattle = (unsigned int)(command.play_door_rattle_sound ? 1 : 0);
                door.vblankBeforeCopy = amigaReceiptValid ? 2U : 1U;
                door.leftBoxX = command.door_left_box_x;
                door.leftBoxY = command.door_left_box_y;
                door.leftBoxW = command.door_left_box_w;
                door.leftBoxH = command.door_left_box_h;
                door.rightBoxX = command.door_right_box_x;
                door.rightBoxY = command.door_right_box_y;
                door.rightBoxW = command.door_right_box_w;
                door.rightBoxH = command.door_right_box_h;
                door.leftSourceX = command.door_left_source_x;
                door.rightSourceX = command.door_right_source_x;
                if (!amigaReceiptValid && command.audio_request_ready) {
                    (void)M11_Audio_EmitSourceSoundIndex(
                        &gameView->audioState, command.audio_sound_index);
                }
                if (!m11_draw_entrance_opening_doors_asset(
                        gameView,
                        framebuffer,
                        dungeonFrame,
                        &door)) {
                    free(dungeonFrame);
                    return 0;
                }
            }
        } else {
            memcpy(framebuffer, dungeonFrame, (size_t)M11_FB_BYTES);
        }

        if (command.present_entrance_palette) {
            if (amigaReceiptValid) {
                presentationStartedMs = m11_intro_active_ticks();
                if (!m11_dm1_amiga_present_entrance_palette(
                        gameView, amigaReceipt->entrance_palette_rgb4,
                        framebuffer)) {
                    free(dungeonFrame);
                    return 0;
                }
            } else if (command.entrance_palette != entrancePalette ||
                command.entrance_palette_fingerprint !=
                    mediaReceipt->entrance_palette_fingerprint) {
                free(dungeonFrame);
                return 0;
            } else {
                presentationStartedMs = m11_intro_active_ticks();
                if (!m11_present_dm1_startup_special_palette(
                        gameView, framebuffer, command.entrance_palette)) {
                    free(dungeonFrame);
                    return 0;
                }
            }
        } else if (amigaReceiptValid
                       ? !m11_dm1_amiga_present_entrance_palette(
                             gameView, amigaReceipt->entrance_palette_rgb4,
                             framebuffer)
                       : !m11_present_dm1_startup_base_palette(
                             gameView, framebuffer)) {
            free(dungeonFrame);
            return 0;
        }
        if (mediaReceipt->platform == DM1_V1_STARTUP_MEDIA_PLATFORM_PC34 &&
            (sourceStep == 3U || sourceStep == 7U)) {
            m11_capture_dm1_startup_presented_frame(
                mediaReceipt,
                sourceStep == 3U ? "c004-closed" : "c004-door-step-1");
        }
        if (step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT) {
            if (g_m11_selector_music)
                g_m11_selector_music->phase = M11_ENTRANCE_PHASE_WAIT;
            M11_EntranceCommand cmd = m11_wait_for_redmcsb_entrance_command(
                autoEnterAfterMs,
                (amigaReceiptValid ? amigaReceipt->mouse_input_only :
                    mediaReceipt->platform ==
                        DM1_V1_STARTUP_MEDIA_PLATFORM_ATARI_ST));
            if (cmd == M11_ENTRANCE_COMMAND_QUIT) {
                free(dungeonFrame);
                return M11_ENTRANCE_COMMAND_QUIT;
            }
            if (cmd == M11_ENTRANCE_COMMAND_RESUME) {
                free(dungeonFrame);
                return M11_ENTRANCE_COMMAND_RESUME;
            }
            if (cmd == M11_ENTRANCE_COMMAND_CREDITS) {
                if (amigaReceiptValid && amigaReceipt->mouse_input_only) {
                    sourceStep = 2U;
                    continue;
                }
                int creditsResult =
                    m11_show_redmcsb_entrance_credits(
                        gameView, framebuffer, mediaReceipt, NULL);
                if (creditsResult == M11_ENTRANCE_COMMAND_QUIT) {
                    free(dungeonFrame);
                    return M11_ENTRANCE_COMMAND_QUIT;
                }
                sourceStep = 0U;
                continue;
            }
            if (g_m11_selector_music) {
                /* SELECTOR.C:1026 starts the switch effect before opening
                 * doors; F8367 no longer submits music once a command exits
                 * the selector. Do not mix a stale music queue into gameplay. */
                g_m11_selector_music->phase = M11_ENTRANCE_PHASE_DOORS;
                if (g_m11_selector_music->songBound)
                    (void)M11_Audio_RequestSourceMusicTrack(
                        g_m11_selector_music->audio, 0);
            }
        }
        if (amigaReceiptValid && command.audio_request_ready &&
            step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_SWITCH_SOUND) {
            (void)M11_Audio_EmitSourceSoundIndex(
                &gameView->audioState, command.audio_sound_index);
        }
        {
            unsigned int delayMs;
            if (amigaDoorFrameOrdinal != 0U && presentationStartedMs != 0U) {
                /* ReDMCSB ENTRANCE.C F0441:233-236 waits two Amiga VBlanks
                 * after each door blit (20 ms each). Account for the native
                 * display present in that interval instead of sleeping another
                 * 40 ms after the display server has already presented it. */
                const unsigned int frameIntervalMs = 40u;
                const Uint64 elapsedMs = m11_intro_active_ticks() -
                                          presentationStartedMs;
                delayMs = elapsedMs >= (Uint64)frameIntervalMs
                    ? 0U : frameIntervalMs - (unsigned int)elapsedMs;
                if (g_m11_debug_startup_detail) {
                    const Uint64 frameInterval =
                        previousAmigaDoorPresentationMs == 0U
                            ? 0U
                            : presentationStartedMs -
                                  previousAmigaDoorPresentationMs;
                    fprintf(stderr,
                            "firestaff: startup-source-frame game=dm1 platform=amiga phase=entrance-door frame=%u/31 interval-ms=%llu wait-ms=%u\n",
                            amigaDoorFrameOrdinal,
                            (unsigned long long)frameInterval, delayMs);
                }
                previousAmigaDoorPresentationMs = presentationStartedMs;
                amigaDoorFrameOrdinal = 0U;
            } else if (fmtownsDoorFrameOrdinal != 0U &&
                presentationStartedMs != 0U) {
                /* ReDMCSB ENTRANCE.C F0438 waits for the VBlank counter to
                 * advance by three between FM Towns door steps. Measure that
                 * source interval from the palette/present transaction so
                 * SDL's first retrace is included rather than added twice. */
                const unsigned int frameIntervalMs =
                    dm1_v1_fmtowns_door_step_delay_ms(
                        fmtownsDoorFrameOrdinal);
                const Uint64 elapsedMs = m11_intro_active_ticks() -
                                          presentationStartedMs;
                delayMs = elapsedMs >= (Uint64)frameIntervalMs
                    ? 0U : frameIntervalMs - (unsigned int)elapsedMs;
                if (g_m11_debug_startup_detail) {
                    const Uint64 frameInterval =
                        previousFmtownsDoorPresentationMs == 0U
                            ? 0U
                            : presentationStartedMs -
                                  previousFmtownsDoorPresentationMs;
                    fprintf(stderr,
                            "firestaff: startup-source-frame game=dm1 platform=fm-towns phase=entrance-door frame=%u/31 interval-ms=%llu wait-ms=%u\n",
                            fmtownsDoorFrameOrdinal,
                            (unsigned long long)frameInterval, delayMs);
                }
                previousFmtownsDoorPresentationMs = presentationStartedMs;
                fmtownsDoorFrameOrdinal = 0U;
            } else {
                delayMs = m11_startup_remaining_delay_ms(
                    command.delay_ms, presentationStartedMs);
            }
            if (!amigaReceiptValid &&
                mediaReceipt->platform ==
                    DM1_V1_STARTUP_MEDIA_PLATFORM_PC34 &&
                command.render_kind ==
                    DM1_V1_STARTUP_ENTRANCE_RENDER_OPENING_DOOR_PC34 &&
                presentationStartedMs != 0U) {
                if (g_m11_debug_startup_detail) {
                    const Uint64 frameInterval =
                        previousPcDoorPresentationMs == 0U
                            ? 0U
                            : presentationStartedMs -
                                  previousPcDoorPresentationMs;
                    fprintf(stderr,
                            "firestaff: startup-source-frame game=dm1 platform=pc phase=entrance-door frame=%u/31 interval-ms=%llu requested-wait-ms=%u wait-ms=%u\n",
                            command.door_animation_step,
                            (unsigned long long)frameInterval,
                            command.delay_ms, delayMs);
                }
                previousPcDoorPresentationMs = presentationStartedMs;
            }
            if (delayMs > 0U && m11_delay_ms_with_intro_event_pump(delayMs)) {
                free(dungeonFrame);
                return M11_ENTRANCE_COMMAND_QUIT;
            }
        }
        if (M11_Render_PumpEvents()) {
            free(dungeonFrame);
            return M11_ENTRANCE_COMMAND_QUIT;
        }
    }
    memcpy(framebuffer, dungeonFrame, (size_t)M11_FB_BYTES);
    (void)m11_present_dm1_startup_base_palette(gameView, framebuffer);
    free(dungeonFrame);
    return 1;
}

static int m11_entrance_run_source_transition(
    M11_GameViewState* gameView,
    int autoEnterAfterMs,
    const DM1_V1_EntranceFullStartRenderReceiptPc34* entranceReceipt,
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* mediaReceipt,
    const DM1_V1_AmigaEntranceF0441Receipt* amigaReceipt,
    M11_EntranceObserver observer,
    void* user) {
    M11_SelectorMusicOwner owner;
    M11_SelectorMusicOwner* previousOwner = g_m11_selector_music;
    M11_GameViewState* previousView = g_m11_intro_game_view;
    int result;
    if (!gameView) return M11_ENTRANCE_COMMAND_NONE;
    memset(&owner, 0, sizeof(owner));
    owner.audio = &gameView->audioState;
    owner.observer = observer;
    owner.observerUser = user;
    /* SELECTOR.C F8368:909-913 owns SONG only for PC34 E/M. The common
     * entrance renderer also serves Atari and Towns; those must keep their
     * original audio owners. Authenticate only this selected companion. */
    if (!amigaReceipt && gameView->active && mediaReceipt && mediaReceipt->handled &&
        mediaReceipt->platform == DM1_V1_STARTUP_MEDIA_PLATFORM_PC34 &&
        !gameView->dm1FmtownsStartupReceiptValid &&
        strcmp(gameView->sourceId, "dm1") == 0) {
        owner.songBound = M11_BindIntroSongForSelectedGraphics(
            owner.audio, gameView->assetLoader.graphicsDatPath);
    }
    g_m11_selector_music = &owner;
    g_m11_intro_game_view = gameView;
    result = m11_play_redmcsb_entrance_transition_impl(
        gameView, autoEnterAfterMs, entranceReceipt, mediaReceipt,
        amigaReceipt);
    /* F8369:1051 releases the selector score on every exit. Also cover
     * missing assets, failed presentation, Resume and host Quit. */
    if (owner.songBound)
        (void)M11_Audio_RequestSourceMusicTrack(owner.audio, 0);
    g_m11_selector_music = previousOwner;
    g_m11_intro_game_view = previousView;
    return result;
}

int M11_Entrance_RunSourceTransition(
    M11_GameViewState* gameView,
    int autoEnterAfterMs,
    const DM1_V1_EntranceFullStartRenderReceiptPc34* entranceReceipt,
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* mediaReceipt,
    M11_EntranceObserver observer,
    void* user) {
    return m11_entrance_run_source_transition(
        gameView, autoEnterAfterMs, entranceReceipt, mediaReceipt, NULL,
        observer, user);
}

static int m11_play_redmcsb_entrance_transition(
    M11_GameViewState* gameView,
    int autoEnterAfterMs,
    const DM1_V1_EntranceFullStartRenderReceiptPc34* entranceReceipt,
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* mediaReceipt) {
    return M11_Entrance_RunSourceTransition(gameView, autoEnterAfterMs,
        entranceReceipt, mediaReceipt, NULL, NULL);
}

static int m11_play_dm1_amiga_entrance_f0441(
    M11_GameViewState* gameView,
    int autoEnterAfterMs,
    const DM1_V1_EntranceFullStartRenderReceiptPc34* entranceReceipt,
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* mediaReceipt,
    const DM1_V1_AmigaEntranceF0441Receipt* amigaReceipt) {
    if (!amigaReceipt || !amigaReceipt->valid) return 0;
    return m11_entrance_run_source_transition(
        gameView, autoEnterAfterMs, entranceReceipt, mediaReceipt,
        amigaReceipt, NULL, NULL);
}

static M11_EntranceCommand m11_entrance_route_framebuffer_pointer(int fbX,
                                                                  int fbY,
                                                                  unsigned int buttonMask) {
    return (M11_EntranceCommand)ENTRANCE_Compat_CommandPathFromPointerCommand(fbX,
                                                                              fbY,
                                                                              buttonMask);
}

/* ReDMCSB ENTRANCE.C -> COMMAND.C pointer route exported for the focused
 * HiDPI gate. The event loop uses the same framebuffer-space helper.
 * Restored after the worktree merge drift (newest author state ca0ab2a51). */
int M11_Entrance_DispatchSourceLockedPointerCommand(int framebufferX,
                                                    int framebufferY,
                                                    unsigned int buttonMask) {
    return ENTRANCE_Compat_DispatchMouseRouteCommand(framebufferX,
                                                      framebufferY,
                                                      buttonMask);
}

static M11_EntranceCommand m11_entrance_route_window_pointer(int windowX,
                                                            int windowY,
                                                            unsigned int buttonMask) {
    int presentedX = 0;
    int presentedY = 0;
    int presentedWidth = 0;
    int presentedHeight = 0;
    int sourceX = 0;
    int sourceY = 0;
    /* M11_Render_MapWindowToFramebuffer first returns a coordinate in the
     * active target surface.  Startup pages may be 640x400 or a Custom
     * target, but ReDMCSB COMMAND.C:340-353 / 1379-1449 requires C407 etc.
     * to be hit-tested in the original 320x200 source space. */
    if (!M11_Render_MapWindowToFramebuffer(windowX, windowY,
                                           &presentedX, &presentedY) ||
        !M11_Render_GetContentSize(&presentedWidth, &presentedHeight) ||
        !ENTRANCE_Compat_MapPresentedPointToSource(
            presentedX, presentedY, presentedWidth, presentedHeight,
            M11_FB_WIDTH, M11_FB_HEIGHT, &sourceX, &sourceY)) {
        return M11_ENTRANCE_COMMAND_NONE;
    }
    return m11_entrance_route_framebuffer_pointer(sourceX, sourceY,
                                                   buttonMask);
}

static M11_EntranceCommand m11_entrance_route_normalized_touch(float normalizedX,
                                                              float normalizedY) {
    int windowW = M11_Render_GetWindowWidth();
    int windowH = M11_Render_GetWindowHeight();
    int windowX;
    int windowY;
    if (!ENTRANCE_Compat_NormalizedTouchToWindowPoint(windowW,
                                                      windowH,
                                                      normalizedX,
                                                      normalizedY,
                                                      &windowX,
                                                      &windowY)) {
        return M11_ENTRANCE_COMMAND_NONE;
    }
    return m11_entrance_route_window_pointer(windowX,
                                             windowY,
                                             ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT);
}

static M11_EntranceCommand m11_wait_for_redmcsb_entrance_command(
    int autoEnterAfterMs,
    int mouseOnly) {
    /* ReDMCSB ENTRANCE.C:850-883 redraws the entrance, discards previous
     * input, then waits in the entrance command loop until a fresh command
     * changes G0298_B_NewGame away from C099_MODE_WAITING_ON_ENTRANCE.
     * Do the same at the SDL boundary: drain the launch key/button that got
     * us here, then require a new Enter/Space/click before the doors open. */
    Uint64 started;
    Uint64 waitStartedCounter;
    int allowHeadlessTimeout = 0;
    int drained = 0;
    SDL_Event ev;
    const char* videoDriver = getenv("SDL_VIDEODRIVER");
    if ((videoDriver && strcmp(videoDriver, "dummy") == 0) || getenv("FIRESTAFF_AUTOTEST")) {
        allowHeadlessTimeout = 1;
    }
    /* A probe has no second physical C200 command.  Keep the interactive
     * entrance source-faithful, while ensuring dummy-video boot verification
     * completes even when its caller has no explicit timeout policy. */
    if (allowHeadlessTimeout && autoEnterAfterMs <= 0) {
        autoEnterAfterMs = 1;
    }

    while (SDL_PollEvent(&ev)) {
        drained += 1;
    }
    (void)drained;
    waitStartedCounter = SDL_GetPerformanceCounter();
    /* Tests can request one fresh entrance input after the launch event has
     * been drained. This exercises the same SDL event path as a physical
     * Amiga Return key without making normal startup self-advancing. */
    if (allowHeadlessTimeout && getenv("FIRESTAFF_AUTOTEST_ENTRANCE_INPUT") &&
        strcmp(getenv("FIRESTAFF_AUTOTEST_ENTRANCE_INPUT"), "key:return") == 0) {
        static const char freshReturn[] = "key:return";
        (void)m11_push_script_event_token(freshReturn, sizeof(freshReturn) - 1u);
    } else if (allowHeadlessTimeout && mouseOnly &&
               getenv("FIRESTAFF_AUTOTEST_ENTRANCE_INPUT") &&
               strcmp(getenv("FIRESTAFF_AUTOTEST_ENTRANCE_INPUT"),
                      "mouse:enter") == 0) {
        int windowWidth = M11_Render_GetWindowWidth();
        int windowHeight = M11_Render_GetWindowHeight();
        float scale = (float)windowWidth / (float)M11_FB_WIDTH;
        float heightScale = (float)windowHeight / (float)M11_FB_HEIGHT;
        float x, y;
        if (heightScale < scale) scale = heightScale;
        x = ((float)windowWidth - (float)M11_FB_WIDTH * scale) * 0.5f +
            270.0f * scale;
        y = ((float)windowHeight - (float)M11_FB_HEIGHT * scale) * 0.5f +
            52.0f * scale;
        memset(&ev, 0, sizeof(ev));
#if SDL_VERSION_ATLEAST(3, 0, 0)
        ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        ev.button.timestamp = SDL_GetTicksNS();
        ev.button.button = SDL_BUTTON_LEFT;
        ev.button.x = x;
        ev.button.y = y;
#else
        ev.type = SDL_MOUSEBUTTONDOWN;
        ev.button.timestamp = SDL_GetTicks();
        ev.button.button = SDL_BUTTON_LEFT;
        ev.button.x = (int)x;
        ev.button.y = (int)y;
#endif
        (void)SDL_PushEvent(&ev);
    } else if (allowHeadlessTimeout &&
               getenv("FIRESTAFF_AUTOTEST_ENTRANCE_INPUT") &&
               strcmp(getenv("FIRESTAFF_AUTOTEST_ENTRANCE_INPUT"),
                      "key:return-repeat") == 0) {
        SDL_Event repeatedReturn;
        memset(&repeatedReturn, 0, sizeof(repeatedReturn));
#if SDL_VERSION_ATLEAST(3, 0, 0)
        repeatedReturn.type = SDL_EVENT_KEY_DOWN;
        repeatedReturn.key.key = SDLK_RETURN;
#else
        repeatedReturn.type = SDL_KEYDOWN;
        repeatedReturn.key.keysym.sym = SDLK_RETURN;
#endif
        repeatedReturn.key.repeat = 1;
        (void)SDL_PushEvent(&repeatedReturn);
    }
    if (g_m11_intro_delay_fast_forward) {
        if (g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "firestaff: startup-input-autoenter game=dm1 phase=entrance reason=explicit-fast-forward\n");
        }
        return M11_ENTRANCE_COMMAND_ENTER;
    }
    started = m11_intro_active_ticks();
    if (g_m11_debug_startup_detail) {
        fprintf(stderr,
                "firestaff: startup-input-wait game=dm1 phase=entrance drained-events=%d timeout-enabled=%d timeout-ms=%d mouse-only=%d\n",
                drained, allowHeadlessTimeout, autoEnterAfterMs, mouseOnly);
    }

    for (;;) {
        if (m11_phase_duration_expired()) return M11_ENTRANCE_COMMAND_QUIT;
        if (m11_intro_wait_for_focus()) return M11_ENTRANCE_COMMAND_QUIT;
        while (SDL_PollEvent(&ev)) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
            if (ev.type == SDL_EVENT_QUIT) return M11_ENTRANCE_COMMAND_QUIT;
            /* ReDMCSB ENTRANCE.C:850-883 accepts a fresh Return while waiting
             * at the entrance; Space remains inert on the PC/F20 path. */
            if (!mouseOnly && ev.type == SDL_EVENT_KEY_DOWN &&
                ev.key.repeat) {
                if (g_m11_debug_startup_detail) {
                    fprintf(stderr,
                            "firestaff: startup-input-ignore game=dm1 phase=entrance reason=key-repeat key=%d\n",
                            (int)ev.key.key);
                }
                continue;
            }
            if (!mouseOnly && ev.type == SDL_EVENT_KEY_DOWN) {
                if (!m11_entrance_input_event_is_fresh(
                        &ev, waitStartedCounter, "keyboard")) continue;
                M11_EntranceCommand keyCommand =
                    m11_entrance_command_path_from_source_command(
                        m11_entrance_dispatch_source_locked_key_command((int)ev.key.key));
                if (keyCommand != M11_ENTRANCE_COMMAND_NONE) {
                    if (g_m11_debug_startup_detail) {
                        fprintf(stderr, "firestaff: startup-input-source keyboard command=%d\n",
                                (int)keyCommand);
                    }
                    return keyCommand;
                }
            }
            if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (!m11_entrance_input_event_is_fresh(
                        &ev, waitStartedCounter, "mouse")) continue;
                if (ev.button.button != SDL_BUTTON_LEFT) continue;
                {
                    M11_EntranceCommand pointerCommand =
                        m11_entrance_route_window_pointer((int)ev.button.x,
                                                          (int)ev.button.y,
                                                          ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT);
                    if (g_m11_debug_startup_detail) {
                        fprintf(stderr,
                            "firestaff: startup-input-source mouse x=%d y=%d command=%d\n",
                            (int)ev.button.x, (int)ev.button.y,
                            (int)pointerCommand);
                    }
                    if (pointerCommand != M11_ENTRANCE_COMMAND_NONE) return pointerCommand;
                }
                continue;
            }
            if (ev.type == SDL_EVENT_FINGER_DOWN) {
                if (!m11_entrance_input_event_is_fresh(
                        &ev, waitStartedCounter, "touch")) continue;
                M11_EntranceCommand touchCommand =
                    m11_entrance_route_normalized_touch(ev.tfinger.x, ev.tfinger.y);
                if (touchCommand != M11_ENTRANCE_COMMAND_NONE) return touchCommand;
                continue;
            }
            if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            }
#else
            if (ev.type == SDL_QUIT) return M11_ENTRANCE_COMMAND_QUIT;
            /* ReDMCSB ENTRANCE.C:850-883 accepts a fresh Return while waiting
             * at the entrance; Space remains inert on the PC/F20 path. */
            if (!mouseOnly && ev.type == SDL_KEYDOWN && ev.key.repeat) {
                if (g_m11_debug_startup_detail) {
                    fprintf(stderr,
                            "firestaff: startup-input-ignore game=dm1 phase=entrance reason=key-repeat key=%d\n",
                            (int)ev.key.keysym.sym);
                }
                continue;
            }
            if (!mouseOnly && ev.type == SDL_KEYDOWN) {
                if (!m11_entrance_input_event_is_fresh(
                        &ev, waitStartedCounter, "keyboard")) continue;
                M11_EntranceCommand keyCommand =
                    m11_entrance_command_path_from_source_command(
                        m11_entrance_dispatch_source_locked_key_command((int)ev.key.keysym.sym));
                if (keyCommand != M11_ENTRANCE_COMMAND_NONE) {
                    if (g_m11_debug_startup_detail) {
                        fprintf(stderr, "firestaff: startup-input-source keyboard command=%d\n",
                                (int)keyCommand);
                    }
                    return keyCommand;
                }
            }
            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                if (!m11_entrance_input_event_is_fresh(
                        &ev, waitStartedCounter, "mouse")) continue;
                if (ev.button.button != SDL_BUTTON_LEFT) continue;
                {
                    M11_EntranceCommand pointerCommand =
                        m11_entrance_route_window_pointer(ev.button.x,
                                                          ev.button.y,
                                                          ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT);
                    if (g_m11_debug_startup_detail) {
                        fprintf(stderr,
                            "firestaff: startup-input-source mouse x=%d y=%d command=%d\n",
                            ev.button.x, ev.button.y, (int)pointerCommand);
                    }
                    if (pointerCommand != M11_ENTRANCE_COMMAND_NONE) return pointerCommand;
                }
                continue;
            }
            if (ev.type == SDL_FINGERDOWN) {
                if (!m11_entrance_input_event_is_fresh(
                        &ev, waitStartedCounter, "touch")) continue;
                M11_EntranceCommand touchCommand =
                    m11_entrance_route_normalized_touch(ev.tfinger.x, ev.tfinger.y);
                if (touchCommand != M11_ENTRANCE_COMMAND_NONE) return touchCommand;
                continue;
            }
            if (ev.type == SDL_WINDOWEVENT &&
                ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            }
#endif
        }

        /* Scripted/headless probes cannot send a second physical command.
         * ReDMCSB ENTRANCE.C waits at C099_MODE_WAITING_ON_ENTRANCE until a
         * fresh command arrives; the launcher click/key that got us here was
         * drained above and must not count as that command.  Therefore timeout
         * auto-enter is strictly a dummy-video/autotest escape hatch, never an
         * interactive runtime behavior. */
        if (ENTRANCE_Compat_ShouldAutoEnterForTimeout(
                allowHeadlessTimeout,
                autoEnterAfterMs,
                (unsigned long long)(m11_intro_active_ticks() - started))) {
            if (g_m11_debug_startup_detail) {
                fprintf(stderr,
                        "firestaff: startup-input-autoenter game=dm1 phase=entrance reason=headless-timeout elapsed-ms=%llu\n",
                        (unsigned long long)(m11_intro_active_ticks() - started));
            }
            return M11_ENTRANCE_COMMAND_ENTER;
        }
        SDL_Delay(16);
    }
}

/* Play the FTL swoosh palette animation. ReDMCSB SWSH.C: static logo on black palette,
 * then V0901006_PaletteCommands lights colors sequentially via Setcolor()/Vsync.
 * ESC/Enter/click skips. Skipped when --game was used (direct launch skips full intro). */
static void m11_swsh_indexed_to_rgba(const unsigned char* indexed,
                                     unsigned char* rgba,
                                     const unsigned char palette[16][3]) {
    unsigned int i;
    if (!indexed || !rgba || !palette) return;
    for (i = 0U; i < (unsigned int)M11_FB_BYTES; ++i) {
        unsigned char idx = indexed[i] & 0x0Fu;
        rgba[i * 4U + 0U] = palette[idx][0];
        rgba[i * 4U + 1U] = palette[idx][1];
        rgba[i * 4U + 2U] = palette[idx][2];
        rgba[i * 4U + 3U] = 0xFFu;
    }
}

/* Unpack a 4-bit-per-pixel Atari ST low-res buffer (160 bytes per row,
 * 32000 bytes total for 320x200) into a 1-byte-per-pixel indexed
 * framebuffer (320 bytes per row, 64000 bytes total).
 *
 * ReDMCSB SWSHGDAT.C FTL logo is 4bpp-packed in the IMG1 source. The
 * SWSH_Compat_ExpandLogoToBitmap decode path emits 4bpp-packed pixels
 * (matching the original Atari ST Physbase layout). The FTL swoosh
 * renderer treats `screenFb` as 1-byte-per-pixel, so the packed buffer
 * must be unpacked before indexed->RGBA conversion. Without this,
 * pixels 160..319 of every row stay zero (logo only renders in the
 * left half) and even pixels 0..159 show alternating nibbles (visible
 * vertical stripes). ReDMCSB TITLE.C PC/F20 port uses the same Atari
 * ST logo layout; TITLE.DAT fallback frames use the corresponding
 * V1_TitleFrontend_Unpack4bppScreenToIndexed path.
 *
 * Source-lock: Atari ST VDI / LINEA: F0080_vq_extend / F0086_vs_clip
 * low-res Physbase layout is 4 planes of (width+1)/2 bytes per row.
 */
static void m11_swsh_unpack_4bpp_to_indexed(const unsigned char* packed,
                                           unsigned char* indexed) {
    unsigned int y;
    unsigned int x;
    if (!packed || !indexed) return;
    for (y = 0U; y < (unsigned int)M11_FB_HEIGHT; ++y) {
        const unsigned char* src = packed + y * 160U;
        unsigned char* dst = indexed + y * (unsigned int)M11_FB_WIDTH;
        for (x = 0U; x < (unsigned int)M11_FB_WIDTH; x += 2U) {
            unsigned char b = src[x >> 1];
            dst[x]         = (unsigned char)((b >> 4) & 0x0Fu);
            dst[x + 1U]    = (unsigned char)(b & 0x0Fu);
        }
    }
}

static int m11_delay_ms_with_intro_event_pump(unsigned int delayMs) {
    Uint64 start;
    if (m11_phase_duration_expired()) return 1;
    if (m11_intro_wait_for_focus()) return 1;
    if (g_m11_intro_delay_fast_forward) {
        const int quit = M11_Render_PumpEvents();
        if (quit) g_m11_startup_intro_quit_requested = 1;
        return quit;
    }
    if (delayMs == 0U) {
        const int quit = M11_Render_PumpEvents();
        if (quit) g_m11_startup_intro_quit_requested = 1;
        return quit;
    }
    start = m11_intro_active_ticks();
    while ((m11_intro_active_ticks() - start) < (Uint64)delayMs) {
        Uint64 elapsed;
        unsigned int remaining;
        if (m11_phase_duration_expired() || m11_intro_wait_for_focus()) return 1;
        if (M11_Render_PumpEvents()) {
            g_m11_startup_intro_quit_requested = 1;
            return 1;
        }
        elapsed = m11_intro_active_ticks() - start;
        if (elapsed >= (Uint64)delayMs) {
            break;
        }
        remaining = delayMs - (unsigned int)elapsed;
        SDL_Delay(remaining > 10U ? 10U : remaining);
    }
    return 0;
}

static void m11_autotest_dm1_swsh_quit_after_first_frame(void) {
    if (getenv("FIRESTAFF_AUTOTEST") &&
        getenv("FIRESTAFF_AUTOTEST_DM1_SWSH_QUIT_AFTER_FRAME") &&
        strcmp(getenv("FIRESTAFF_AUTOTEST_DM1_SWSH_QUIT_AFTER_FRAME"),
               "1") == 0) {
        SDL_Event event;
        memset(&event, 0, sizeof(event));
#if SDL_VERSION_ATLEAST(3, 0, 0)
        event.type = SDL_EVENT_QUIT;
#else
        event.type = SDL_QUIT;
#endif
        if (SDL_PushEvent(&event)) {
            fprintf(stderr,
                    "AUTOTEST: DM1 SWSH quit injected after first rendered frame\n");
        }
    }
}

/* Keep source 50 Hz animation events on one clock. A delay after Present adds
 * the renderer's VSync wait to every period and slows Amiga title frames on
 * 60 Hz desktop displays. Absolute deadlines let a VSync present count toward
 * the source interval while preserving the original 20 ms cadence. */
static int m11_wait_for_intro_active_tick(Uint64 deadlineMs,
                                          int *outQuitRequested) {
    if (g_m11_intro_delay_fast_forward) {
        const int quit = M11_Render_PumpEvents();
        if (quit && outQuitRequested) *outQuitRequested = 1;
        return quit;
    }
    for (;;) {
        Uint64 nowMs;
        unsigned int remainingMs;
        if (m11_phase_duration_expired()) return 1;
        if (m11_intro_wait_for_focus()) {
            if (outQuitRequested && !m11_phase_duration_expired())
                *outQuitRequested = 1;
            return 1;
        }
        if (M11_Render_PumpEvents()) {
            if (outQuitRequested) *outQuitRequested = 1;
            return 1;
        }
        nowMs = m11_intro_active_ticks();
        if (nowMs >= deadlineMs) return 0;
        remainingMs = (unsigned int)(deadlineMs - nowMs);
        SDL_Delay(remainingMs > 10U ? 10U : remainingMs);
    }
}

/* ReDMCSB's A20 title waits for the Amiga viewport bottom (BOVP) before
 * clearing and before the final master/title palette transition. The host
 * renderer is not beam-synchronised, so represent that source event on the
 * original PAL 50 Hz clock. Keep a single absolute deadline to avoid adding
 * the host Present interval to every source frame. */
static int m11_dm1_amiga_wait_bovp(Uint64* nextVblankMs,
                                   int *outQuitRequested) {
    if (!nextVblankMs) return 0;
    *nextVblankMs += 20u;
    return m11_wait_for_intro_active_tick(*nextVblankMs,
                                          outQuitRequested) == 0;
}

/* ReDMCSB NECIO.C F0022 lines 3592-3609 and TITLE.C F0437 lines 319-409:
 * DM1 startup presents SWSH palette waits and TITLE C001 palette/timing in
 * source order.  Keep production rendering on the DM1 receipt so M11 does
 * not infer host cadence independently from the source-locked startup route. */
static unsigned int m11_startup_media_swsh_wait_ms(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media,
    unsigned int vblankCount) {
    if (!media || !media->handled || media->swsh_vblank_ms == 0U) {
        return SWSH_Compat_GetRuntimeDelayMsForVblankCount(vblankCount);
    }
    if (vblankCount > (0xffffffffU / media->swsh_vblank_ms)) {
        return SWSH_Compat_GetRuntimeDelayMsForVblankCount(vblankCount);
    }
    return vblankCount * media->swsh_vblank_ms;
}

/* The launcher has just been rendering a different surface when SWSH takes
 * over.  On composited desktops (most visibly macOS), the first
 * SDL_RenderPresent() can be consumed by the window/compositor handoff.  A
 * delay alone is not sufficient: the display server may never have received
 * a stable FTL frame before the source palette starts changing.  Submit the
 * immutable, black-palette source frame twice, with one host VBlank and event
 * turn in between, then begin the *full* source-owned initial hold.  The
 * two-frame prelude is explicitly host presentation synchronisation; it does
 * not consume or shorten any of SWSH.C's twenty original VBlanks. */
static int m11_prepare_swsh_source_clock_after_launcher_handoff(
    const unsigned char* initialRgba,
    int width,
    int height,
    unsigned int vblankMs) {
    if (!initialRgba || width <= 0 || height <= 0) {
        return 0;
    }
    if (M11_Render_PumpEvents()) {
        g_m11_startup_intro_quit_requested = 1;
        return 0;
    }
    if (vblankMs != 0U && m11_delay_ms_with_intro_event_pump(vblankMs)) {
        return 0;
    }
    if (M11_Render_PresentRGBA(initialRgba, width, height) != M11_RENDER_OK) {
        return 0;
    }
    if (M11_Render_PumpEvents()) {
        return 0;
    }
    return vblankMs == 0U || !m11_delay_ms_with_intro_event_pump(vblankMs);
}

int M11_BindIntroSongForSelectedGraphics(M11_AudioState* audio,
                                          const char* graphicsPath) {
    static const char graphicsName[] = "GRAPHICS.DAT";
    static const char songName[] = "SONG.DAT";
    char songPath[M11_GAME_VIEW_PATH_CAPACITY];
    DM1_V1_F0740F0743MusicSourcePc34 source;
    size_t length, prefix;
    if (!audio || !audio->initialized) return 0;
    /* Match m11_dm1_rebind_source_song: the companion belongs to this
     * selected asset owner. MUSIC.C F0740-F0743:559-672 routing is admitted
     * by the existing PC34 SONG.DAT hash/sequence receipt, never by a search
     * for another installed edition. Clearing first also stops stale PCM. */
    (void)M11_Audio_BindOriginalSongPath(audio, NULL);
    if (!graphicsPath) return 0;
    length = strlen(graphicsPath);
    if (length <= sizeof(graphicsName) - 1U ||
        strcmp(graphicsPath + length - (sizeof(graphicsName) - 1U), graphicsName)) return 0;
    prefix = length - (sizeof(graphicsName) - 1U);
    if (prefix + sizeof(songName) > sizeof(songPath)) return 0;
    memcpy(songPath, graphicsPath, prefix);
    memcpy(songPath + prefix, songName, sizeof(songName));
    if (!dm1_v1_f0740_f0743_bind_song_dat_pc34(songPath, &source)) return 0;
    return M11_Audio_BindOriginalSongPath(audio, songPath);
}

int M11_ApplyIntroAudioPreferences(M11_AudioState* audio,
                                   const M12_StartupMenuState* menu) {
    int master, music, sfx;
    if (!audio || !audio->initialized) return 0;
    if (!menu) return 1;
    /* M12 persists the same 0..128 scale consumed by the runtime owner.
     * These are host preferences, independent of source SWSH/TITLE timing. */
    master = menu->settings.audioMasterVolume;
    music = menu->settings.audioMusicVolume;
    sfx = menu->settings.audioSfxVolume;
    if (menu->settings.audioMuted) master = music = sfx = 0;
    return M11_Audio_SetVolumes(audio, master, sfx, music, sfx);
}

static unsigned int m11_dm1_amiga_swsh_fnv1a(const uint8_t* bytes,
                                             size_t byteCount) {
    unsigned int hash = 2166136261u;
    size_t index;
    if (!bytes) return 0u;
    for (index = 0u; index < byteCount; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

/* Startup companions belong beside the selected graphics file or member.
 * Preserve every nested transport and directory; never search other media. */
static int m11_dm1_amiga_sibling_path(const M12_AssetVersionStatus* version,
                                      const char* sibling,
                                      char* outPath, size_t outPathBytes) {
    const char* path;
    const char* separator = NULL;
    const char* scan;
    const char* leaf;
    size_t prefixBytes;
    size_t memberBytes;
    size_t siblingBytes;
    size_t i;
    static const char graphicsMember[] = "GRAPHICS.DAT";
    if (!version || !sibling || !sibling[0] || !outPath || !outPathBytes ||
        strchr(sibling, '/') || strchr(sibling, ':') ||
        strchr(sibling, '\\')) return 0;
    outPath[0] = '\0';
    path = version->matchedPath;
    if (!path[0]) return 0;
    for (scan = path; (scan = strstr(scan, "::")) != NULL; scan += 2)
        separator = scan;
    leaf = separator ? separator + 2 : path;
    {
        const char* slash = strrchr(leaf, '/');
        const char* backslash = strrchr(leaf, '\\');
        if (!slash || (backslash && backslash > slash)) slash = backslash;
        if (slash) leaf = slash + 1;
    }
    if (separator) {
        memberBytes = strlen(leaf);
        if (memberBytes != sizeof(graphicsMember) - 1U) return 0;
        for (i = 0u; i < memberBytes; ++i)
            if ((unsigned char)toupper((unsigned char)leaf[i]) !=
                (unsigned char)graphicsMember[i]) return 0;
    } else if (!version->matched || !version->matchedMd5[0] ||
               !asset_file_matches_md5(path, version->matchedMd5)) {
        /* Loose discovery admits original graphics by content, including
         * renamed files. Recheck this selected file before F0437/F0441's
         * same-parent companion lookup; archive member policy stays above. */
        return 0;
    }
    prefixBytes = (size_t)(leaf - path);
    siblingBytes = strlen(sibling) + 1u;
    if (prefixBytes >= outPathBytes ||
        siblingBytes > outPathBytes - prefixBytes) return 0;
    memcpy(outPath, path, prefixBytes);
    memcpy(outPath + prefixBytes, sibling, siblingBytes);
    if (!separator) {
        char parent[FSP_PATH_MAX] = {0};
        int resolved;
        /* ReDMCSB TITLE.C:12 F0437 / ENTRANCE.C:620 F0441 run over the
         * selected AmigaDOS installation without imposing POSIX case.
         * Resolve only this admitted parent;
         * archive members retain their existing native reader semantics. */
        resolved = FSP_ParentDir(parent, sizeof(parent), path)
            ? m11_find_unique_loose_sibling(parent, sibling, outPath, outPathBytes)
            : -1;
        if (resolved != 1) {
            if (g_m11_debug_startup_detail) {
                fprintf(stderr, "firestaff: startup-sibling-rejected game=dm1 platform=amiga wanted=%s parent=%s reason=%s\n",
                        sibling, parent, resolved == -2 ? "ambiguous" :
                        resolved == -1 ? "enumeration" : "missing");
            }
            outPath[0] = '\0';
            return 0;
        }
        if (g_m11_debug_startup_detail) {
            fprintf(stderr, "firestaff: startup-sibling-resolved game=dm1 platform=amiga wanted=%s parent=%s source=%s\n",
                    sibling, parent, outPath);
        }
    }
    return 1;
}

static int m11_dm1_amiga_swsh_path(const M12_AssetVersionStatus* version,
                                   char* outPath, size_t outPathBytes) {
    return m11_dm1_amiga_sibling_path(version, "swoosh", outPath, outPathBytes);
}

static int m11_dm1_amiga_read_startup_companions(
    const M12_AssetVersionStatus* version,
    uint8_t** dungeonData, size_t* dungeonBytes,
    uint8_t** dm, size_t* dmBytes) {
    char path[FSP_PATH_MAX];
    *dungeonData = NULL;
    *dungeonBytes = 0u;
    *dm = NULL;
    *dmBytes = 0u;
    if (!m11_dm1_amiga_sibling_path(version, "dungeon.dat", path, sizeof(path)) ||
        !asset_read_path_alloc(path, dungeonData, dungeonBytes) ||
        !m11_dm1_amiga_sibling_path(version, "dm", path, sizeof(path)) ||
        !asset_read_path_alloc(path, dm, dmBytes)) {
        free(*dungeonData);
        free(*dm);
        *dungeonData = *dm = NULL;
        *dungeonBytes = *dmBytes = 0u;
        return 0;
    }
    return 1;
}

static int m11_dm1_amiga_entrance_f0441_receipt_if_selected(
    const M12_StartupMenuState* menuState,
    const M12_MenuEntry* entry,
    const M11_GameViewState* gameView,
    DM1_V1_AmigaEntranceF0441Receipt* outReceipt) {
    int versionIndex;
    const M12_AssetVersionStatus* version;
    char swooshPath[FSP_PATH_MAX];
    uint8_t* graphics = NULL;
    size_t graphicsBytes = 0U;
    uint8_t* executable = NULL;
    size_t executableBytes = 0U;
    uint8_t* dungeonData = NULL;
    size_t dungeonBytes = 0U;
    uint8_t* dm = NULL;
    size_t dmBytes = 0U;
    int authenticated = 0;
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!menuState || !entry || !entry->gameId ||
        strcmp(entry->gameId, "dm1") != 0 || !gameView || !outReceipt) {
        return 0;
    }
    versionIndex = m11_selected_dm1_launch_version_index(menuState, entry);
    if (versionIndex < 0 ||
        M12_AssetStatus_GetVersionArchitecture(
            "dm1", (size_t)versionIndex) != M12_ARCH_AMIGA) {
        return 0;
    }
    version = M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                          (size_t)versionIndex);
    if (!version || !version->versionId ||
        (strcmp(version->versionId, "amiga20-en") != 0 &&
         strcmp(version->versionId, "amiga20-fr") != 0 &&
         strcmp(version->versionId, "amiga20-de") != 0 &&
         strcmp(version->versionId, "amiga21-en") != 0 &&
         strcmp(version->versionId, "amiga-demo-en") != 0) ||
        strstr(version->matchedPath, "[HD]") != NULL ||
        !gameView->assetLoader.graphicsDatPath[0] ||
        strcmp(gameView->assetLoader.graphicsDatPath,
               version->matchedPath) != 0 ||
        !m11_dm1_amiga_swsh_path(version, swooshPath, sizeof(swooshPath)) ||
        !asset_read_path_alloc(
            gameView->assetLoader.graphicsDatPath, &graphics,
            &graphicsBytes) ||
        !asset_read_path_alloc(swooshPath, &executable,
                                       &executableBytes)) {
        goto cleanup;
    }
    authenticated = dm1_v1_amiga_entrance_f0441_receipt(
        graphics, graphicsBytes, executable, executableBytes, outReceipt);
    if (!authenticated && m11_dm1_amiga_read_startup_companions(
            version, &dungeonData, &dungeonBytes, &dm, &dmBytes)) {
        const DM1_V1_AmigaStartupMedia media = {
            graphics, graphicsBytes, dungeonData, dungeonBytes,
            dm, dmBytes, executable, executableBytes};
        authenticated = dm1_v1_amiga_entrance_f0441_receipt_for_startup(
            &media, outReceipt);
        if (authenticated && g_m11_debug_startup_detail) {
            fprintf(stderr, "firestaff: startup-media-authenticated game=dm1 edition=amiga22-de tuple=a22g-four-files phase=entrance source=%s\n",
                    version->matchedPath);
        }
    }

cleanup:
    free(graphics);
    free(executable);
    free(dungeonData);
    free(dm);
    return authenticated;
}

static int m11_ascii_path_equal_ignore_case(const char* left,
                                            const char* right) {
    if (!left || !right) return 0;
    while (*left && *right) {
        if (tolower((unsigned char)*left) !=
            tolower((unsigned char)*right)) return 0;
        ++left;
        ++right;
    }
    return *left == '\0' && *right == '\0';
}

static int m11_dm1_amiga36_entrance_f0441_receipt_if_selected(
    const M12_StartupMenuState* menuState,
    const M12_MenuEntry* entry,
    const M11_GameViewState* gameView,
    DM1_V1_AmigaEntranceF0441Receipt* outReceipt) {
    int versionIndex;
    const M12_AssetVersionStatus* version;
    uint8_t* graphics = NULL;
    size_t graphicsBytes = 0U;
    uint8_t *driver = NULL;
    size_t driverBytes = 0;
    char driverPath[FSP_PATH_MAX];
    int authenticated = 0;
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!menuState || !entry || !entry->gameId ||
        strcmp(entry->gameId, "dm1") != 0 || !gameView || !outReceipt) {
        return 0;
    }
    versionIndex = m11_selected_dm1_launch_version_index(menuState, entry);
    version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                     (size_t)versionIndex)
        : NULL;
    if (!version || !version->versionId ||
        strcmp(version->versionId, "amiga36-multi") != 0 ||
        M12_AssetStatus_GetVersionArchitecture(
            "dm1", (size_t)versionIndex) != M12_ARCH_AMIGA ||
        !gameView->assetLoader.graphicsDatPath[0] ||
        !m11_ascii_path_equal_ignore_case(
            gameView->assetLoader.graphicsDatPath, version->matchedPath) ||
        !asset_read_path_alloc(
            gameView->assetLoader.graphicsDatPath, &graphics,
            &graphicsBytes)) {
        goto cleanup;
    }
    authenticated = dm1_v1_amiga36_entrance_f0441_receipt(
        graphics, graphicsBytes, outReceipt) &&
        m11_dm1_amiga_sibling_path(version, "VDEO.FTL", driverPath,
                                   sizeof(driverPath)) &&
        asset_read_path_alloc(driverPath, &driver, &driverBytes) &&
        dm1_v1_amiga36_entrance_bind_driver(outReceipt, driver, driverBytes);

cleanup:
    free(driver);
    free(graphics);
    return authenticated;
}

static void m11_dm1_amiga_rgb4_to_rgb6(
    const uint16_t colors[DM1_V1_AMIGA_TITLE_RGB4_ENTRIES],
    uint8_t outPalette[256][3]) {
    unsigned int color;
    memset(outPalette, 0, 256u * 3u);
    for (color = 0u; color < DM1_V1_AMIGA_TITLE_RGB4_ENTRIES; ++color) {
        const unsigned int rgb4 = colors[color];
        const unsigned int red = (rgb4 >> 8u) & 0x0fu;
        const unsigned int green = (rgb4 >> 4u) & 0x0fu;
        const unsigned int blue = rgb4 & 0x0fu;
        outPalette[color][0] = (uint8_t)((red << 2u) | (red >> 2u));
        outPalette[color][1] = (uint8_t)((green << 2u) | (green >> 2u));
        outPalette[color][2] = (uint8_t)((blue << 2u) | (blue >> 2u));
    }
}

static int m11_dm1_amiga_present_title_palette(
    const uint16_t colors[DM1_V1_AMIGA_TITLE_RGB4_ENTRIES],
    const uint8_t* framebuffer) {
    uint8_t palette[256][3];
    m11_dm1_amiga_rgb4_to_rgb6(colors, palette);
    return M11_Render_SetIndexedPaletteRgb6(palette) == M11_RENDER_OK &&
           M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                     M11_FB_HEIGHT) == M11_RENDER_OK;
}

static int m11_dm1_amiga_present_entrance_palette(
    const M11_GameViewState* gameView,
    const uint16_t colors[DM1_V1_AMIGA_ENTRANCE_F0441_PALETTE_ENTRIES],
    const uint8_t* framebuffer) {
    uint8_t palette[256][3];
    int targetW = M11_FB_WIDTH;
    int targetH = M11_FB_HEIGHT;
    int result;
    if (!gameView || !colors || !framebuffer) return 0;
    if (gameView->assetLoader.dm1Amiga36) {
        uint8_t rgb4[256][3] = {{0}};
        /* A36's native DRIA words are RGB4; passing through DAC6 loses
         * precision (for example source6 becomes101 instead of102). */
        for (unsigned int color = 0; color < 256; ++color) {
            unsigned int word = colors[color & 15];
            rgb4[color][0] = (uint8_t)((word >> 8) & 15);
            rgb4[color][1] = (uint8_t)((word >> 4) & 15);
            rgb4[color][2] = (uint8_t)(word & 15);
        }
        if (M11_Render_SetIndexedPaletteRgb4(rgb4) != M11_RENDER_OK) return 0;
    } else {
        m11_dm1_amiga_rgb4_to_rgb6(colors, palette);
        if (M11_Render_SetIndexedPaletteRgb6(palette) != M11_RENDER_OK) return 0;
    }
    M11_Render_SetV2PresentationActive(0);
    M11_Render_SetModernPresentationActive(0);
    if (gameView->presentationMode == M12_PRESENTATION_V21_UPSCALED) {
        (void)M11_GameView_PresentationTarget(
            gameView->presentationMode, gameView->presentationWidth,
            gameView->presentationHeight, &targetW, &targetH);
        result = M11_Render_PresentEpxIndexedToResolution(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, targetW, targetH);
    } else if (M11_GameView_PresentationTarget(
                   gameView->presentationMode,
                   gameView->presentationWidth,
                   gameView->presentationHeight,
                   &targetW, &targetH)) {
        result = M11_Render_PresentIndexedToResolution(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, targetW, targetH);
    } else {
        result = M11_Render_PresentIndexed(
            framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    }
    return result == M11_RENDER_OK;
}

static int m11_dm1_amiga_fade_title_palette(
    const uint16_t source[DM1_V1_AMIGA_TITLE_RGB4_ENTRIES],
    const uint16_t target[DM1_V1_AMIGA_TITLE_RGB4_ENTRIES],
    uint8_t* framebuffer,
    int *outQuitRequested) {
    DM1_V1_AmigaRgb4Palette sourcePalette = {
        source, DM1_V1_AMIGA_TITLE_RGB4_ENTRIES, 1, 1, 1
    };
    DM1_V1_AmigaRgb4Palette targetPalette = {
        target, DM1_V1_AMIGA_TITLE_RGB4_ENTRIES, 1, 1, 1
    };
    DM1_V1_AmigaPaletteFade fade;
    DM1_V1_AmigaPaletteFadeReceipt step;
    Uint64 nextVblankMs = m11_intro_active_ticks();
    unsigned int frame;
    if (!dm1_v1_amiga_palette_fade_begin(&fade, &sourcePalette,
                                         &targetPalette)) {
        return 0;
    }
    for (frame = 0u; frame < DM1_V1_AMIGA_RGB4_FADE_FRAMES; ++frame) {
        nextVblankMs += 20u;
        if (m11_wait_for_intro_active_tick(nextVblankMs,
                                           outQuitRequested) ||
            !dm1_v1_amiga_palette_fade_step(&fade, &step) ||
            !m11_dm1_amiga_present_title_palette(step.rgb4, framebuffer)) {
            return 0;
        }
    }
    return 1;
}

static int m11_play_dm1_amiga_title_f0437_if_available(
    const M12_StartupMenuState* menuState,
    const M12_MenuEntry* entry,
    M11_GameViewState* gameView,
    int *outQuitRequested) {
    int versionIndex;
    const M12_AssetVersionStatus* version;
    char swooshPath[FSP_PATH_MAX];
    uint8_t* graphics = NULL;
    size_t graphicsBytes = 0u;
    uint8_t* executable = NULL;
    size_t executableBytes = 0u;
    uint8_t* dungeonData = NULL;
    size_t dungeonBytes = 0u;
    uint8_t* dm = NULL;
    size_t dmBytes = 0u;
    uint8_t* c001 = NULL;
    uint8_t* dungeon = NULL;
    uint8_t* master = NULL;
    uint8_t* framebuffer;
    uint16_t width = 0u;
    uint16_t height = 0u;
    DM1_V1_AmigaTitleF0437Receipt receipt;
    uint16_t activePalette[DM1_V1_AMIGA_TITLE_RGB4_ENTRIES];
    unsigned int stepIndex;
    Uint64 titleStartedMs = 0u;
    Uint64 zoomStartedMs = 0u;
    Uint64 zoomElapsedMs = 0u;
    Uint64 holdStartedMs = 0u;
    Uint64 holdElapsedMs = 0u;
    Uint64 finalFadeStartedMs = 0u;
    Uint64 finalFadeElapsedMs = 0u;
    Uint64 nextZoomFrameMs = 0u;
    Uint64 nextSourceVblankMs = 0u;
    unsigned int paletteFadeCount = 0u;
    int completed = 0;
    int autotestQuitInjected = 0;

    if (outQuitRequested) *outQuitRequested = 0;

    if (!menuState || !entry || !entry->gameId ||
        strcmp(entry->gameId, "dm1") != 0 || !gameView ||
        !gameView->assetsAvailable) {
        return 0;
    }
    versionIndex = m11_selected_dm1_launch_version_index(menuState, entry);
    if (versionIndex < 0 ||
        M12_AssetStatus_GetVersionArchitecture(
            "dm1", (size_t)versionIndex) != M12_ARCH_AMIGA) {
        return 0;
    }
    version = M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                          (size_t)versionIndex);
    if (!version || strstr(version->matchedPath, "[HD]") != NULL ||
        !gameView->assetLoader.graphicsDatPath[0] ||
        strcmp(gameView->assetLoader.graphicsDatPath,
               version->matchedPath) != 0 ||
        !m11_dm1_amiga_swsh_path(version, swooshPath, sizeof(swooshPath)) ||
        !asset_read_path_alloc(
            gameView->assetLoader.graphicsDatPath, &graphics,
            &graphicsBytes) ||
        !asset_read_path_alloc(swooshPath, &executable,
                                       &executableBytes)) {
        goto cleanup;
    }
    if (!dm1_v1_amiga_title_f0437_receipt(
            graphics, graphicsBytes, executable, executableBytes, &receipt)) {
        if (!m11_dm1_amiga_read_startup_companions(
                version, &dungeonData, &dungeonBytes, &dm, &dmBytes)) goto cleanup;
        const DM1_V1_AmigaStartupMedia media = {
            graphics, graphicsBytes, dungeonData, dungeonBytes,
            dm, dmBytes, executable, executableBytes};
        if (!dm1_v1_amiga_title_f0437_receipt_for_startup(&media, &receipt))
            goto cleanup;
        if (g_m11_debug_startup_detail) {
            fprintf(stderr, "firestaff: startup-media-authenticated game=dm1 edition=amiga22-de tuple=a22g-four-files phase=title source=%s\n",
                    version->matchedPath);
        }
    }
    c001 = (uint8_t*)malloc((size_t)DM1_V1_AMIGA_TITLE_WIDTH *
                            DM1_V1_AMIGA_TITLE_HEIGHT);
    dungeon = (uint8_t*)malloc((size_t)DM1_V1_AMIGA_TITLE_WIDTH * 80u);
    master = (uint8_t*)malloc((size_t)DM1_V1_AMIGA_TITLE_WIDTH * 57u);
    framebuffer = M11_Render_GetFramebuffer();
    if (!c001 || !dungeon || !master || !framebuffer ||
        !dm1_v1_amiga_graphics_decode(
            graphics, graphicsBytes, DM1_V1_AMIGA_TITLE_GRAPHIC_C001,
            c001, (size_t)DM1_V1_AMIGA_TITLE_WIDTH *
                      DM1_V1_AMIGA_TITLE_HEIGHT,
            &width, &height) ||
        width != DM1_V1_AMIGA_TITLE_WIDTH ||
        height != DM1_V1_AMIGA_TITLE_HEIGHT) {
        goto cleanup;
    }

    /* TITLE.C F0437 A20 uses destination boxes G0003-G0005/G1075 with
     * source offsets passed separately to F0132_VIDEO_Blit.  Keep the two
     * meanings distinct: PRESENTS is C001 y=137 -> screen y=90, MASTER is
     * C001 y=80 -> its 320x57 temporary, and the zoom source is C001 y=0
     * through y=79. */
    memcpy(dungeon, c001,
           (size_t)DM1_V1_AMIGA_TITLE_WIDTH * 80u);
    memcpy(master,
           c001 + (size_t)receipt.master_source_y *
                      DM1_V1_AMIGA_TITLE_WIDTH,
           (size_t)DM1_V1_AMIGA_TITLE_WIDTH * 57u);
    M11_Render_SetV2PresentationActive(0);
    M11_Render_SetModernPresentationActive(0);
    titleStartedMs = m11_intro_active_ticks();
    if (g_m11_dm1_amiga_swsh_palette_valid) {
        memcpy(activePalette, g_m11_dm1_amiga_swsh_palette,
               sizeof(activePalette));
    } else {
        memcpy(activePalette, receipt.initial_palette, sizeof(activePalette));
        memset(framebuffer, 0, (size_t)M11_FB_BYTES);
    }
    if (!m11_dm1_amiga_present_title_palette(activePalette, framebuffer)) {
        goto cleanup;
    }
    if (g_m11_dm1_amiga_swsh_palette_valid) {
        /* ReDMCSB TITLE.C F0437 A20 lines 113-124 first fades the preceding
         * SWSH image to the dark-blue palette, then fills the screen. */
        if (!m11_dm1_amiga_fade_title_palette(
                activePalette, receipt.initial_palette, framebuffer,
                outQuitRequested)) {
            goto cleanup;
        }
        ++paletteFadeCount;
        memcpy(activePalette, receipt.initial_palette,
               sizeof(activePalette));
    }
    nextSourceVblankMs = m11_intro_active_ticks();
    if (!m11_dm1_amiga_wait_bovp(&nextSourceVblankMs,
                                 outQuitRequested)) goto cleanup;
    memset(framebuffer, 0, (size_t)M11_FB_BYTES);
    if (M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                  M11_FB_HEIGHT) != M11_RENDER_OK) {
        goto cleanup;
    }
    memcpy(framebuffer + (size_t)receipt.presents_destination_y *
                              M11_FB_WIDTH,
           c001 + (size_t)receipt.presents_source_y * M11_FB_WIDTH,
           (size_t)M11_FB_WIDTH * receipt.presents_source_height);
    if (!m11_dm1_amiga_fade_title_palette(
            activePalette, receipt.presents_palette, framebuffer,
            outQuitRequested)) {
        goto cleanup;
    }
    ++paletteFadeCount;
    memcpy(activePalette, receipt.presents_palette, sizeof(activePalette));
    /* ReDMCSB TITLE.C F0437 A20 lines 171-184 fades PRESENTS back to
     * dark blue and clears it before the zoom palette is installed. */
    if (!m11_dm1_amiga_fade_title_palette(
            activePalette, receipt.initial_palette, framebuffer,
            outQuitRequested)) {
        goto cleanup;
    }
    ++paletteFadeCount;
    memcpy(activePalette, receipt.initial_palette, sizeof(activePalette));
    nextSourceVblankMs = m11_intro_active_ticks();
    if (!m11_dm1_amiga_wait_bovp(&nextSourceVblankMs,
                                 outQuitRequested)) goto cleanup;
    memset(framebuffer, 0, (size_t)M11_FB_BYTES);
    if (M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                  M11_FB_HEIGHT) != M11_RENDER_OK) {
        goto cleanup;
    }
    if (!m11_dm1_amiga_fade_title_palette(
            activePalette, receipt.zoom_palette, framebuffer,
            outQuitRequested)) {
        goto cleanup;
    }
    ++paletteFadeCount;
    memcpy(activePalette, receipt.zoom_palette, sizeof(activePalette));

    zoomStartedMs = m11_intro_active_ticks();
    nextSourceVblankMs = zoomStartedMs;
    /* TITLE.C F0437 first waits for VBlank, then VBeamPos() line 152. PAL
     * has 312 lines per 20 ms frame, so schedule that first beam event after
     * both source waits; later blits retain the source 50 Hz cadence across
     * host Present. This models the source phase without polling a host beam. */
    nextZoomFrameMs = zoomStartedMs +
        (Uint64)receipt.pre_zoom_vblanks * 20u +
        ((Uint64)receipt.source_beam_wait_line * 20u + 311u) / 312u;
    for (stepIndex = 0u;
         stepIndex < DM1_V1_AMIGA_TITLE_ZOOM_STEP_COUNT; ++stepIndex) {
        DM1_V1_AmigaTitleF0437ZoomStep step;
        unsigned int y;
        Uint64 presentStartedMs;
        Uint64 presentReturnedMs;
        if (!dm1_v1_amiga_title_f0437_zoom_step(
                &receipt, stepIndex, &step)) {
            goto cleanup;
        }
        if (m11_wait_for_intro_active_tick(nextZoomFrameMs,
                                           outQuitRequested)) {
            goto cleanup;
        }
        presentStartedMs = m11_intro_active_ticks();
        for (y = 0u; y < step.destination_height; ++y) {
            unsigned int x;
            const unsigned int sourceY =
                y * step.source_height / step.destination_height;
            for (x = 0u; x < step.destination_width; ++x) {
                const unsigned int sourceX =
                    x * step.source_width / step.destination_width;
                framebuffer[(size_t)(step.destination_y + y) * M11_FB_WIDTH +
                            step.destination_x + x] =
                    dungeon[(size_t)sourceY * DM1_V1_AMIGA_TITLE_WIDTH +
                            sourceX];
            }
        }
        if (M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                      M11_FB_HEIGHT) != M11_RENDER_OK) {
            goto cleanup;
        }
        /* TITLE.C F0437's first Amiga C001 zoom blit is the first distinct
         * source frame. Keep the close injection scoped to this event so the
         * regression follows the real title wait and cleanup path. */
        if (stepIndex == 0u && !autotestQuitInjected &&
            getenv("FIRESTAFF_AUTOTEST") &&
            getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_QUIT_AFTER_FRAME") &&
            strcmp(getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_QUIT_AFTER_FRAME"),
                   "1") == 0) {
            SDL_Event event;
            memset(&event, 0, sizeof(event));
#if SDL_VERSION_ATLEAST(3, 0, 0)
            event.type = SDL_EVENT_QUIT;
#else
            event.type = SDL_QUIT;
#endif
            if (SDL_PushEvent(&event)) {
                autotestQuitInjected = 1;
                fprintf(stderr,
                        "AUTOTEST: DM1 Amiga title quit injected after first rendered frame\n");
            }
        }
        presentReturnedMs = m11_intro_active_ticks();
        if (g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "firestaff: startup-source-vsync game=dm1 platform=amiga phase=title-zoom-vsync step=%u/%u source-deadline-ms=%llu present-start-ms=%llu present-return-ms=%llu present-ms=%llu\n",
                    stepIndex + 1u, DM1_V1_AMIGA_TITLE_ZOOM_STEP_COUNT,
                    (unsigned long long)nextZoomFrameMs,
                    (unsigned long long)presentStartedMs,
                    (unsigned long long)presentReturnedMs,
                    (unsigned long long)(presentReturnedMs - presentStartedMs));
        }
        nextZoomFrameMs += 20u;
    }
    zoomElapsedMs = m11_intro_active_ticks() - zoomStartedMs;
    holdStartedMs = m11_intro_active_ticks();
    nextSourceVblankMs = holdStartedMs;
    if (m11_delay_ms_with_intro_event_pump(
            receipt.source_delay_ticks * 20u)) {
        if (outQuitRequested && !m11_phase_duration_expired())
            *outQuitRequested = 1;
        goto cleanup;
    }
    holdElapsedMs = m11_intro_active_ticks() - holdStartedMs;
    if (!m11_dm1_amiga_wait_bovp(&nextSourceVblankMs,
                                 outQuitRequested)) goto cleanup;
    {
        unsigned int y;
        for (y = 0u; y < receipt.master_source_height; ++y) {
            unsigned int x;
            for (x = 0u; x < M11_FB_WIDTH; ++x) {
                const uint8_t pixel = master[(size_t)y * M11_FB_WIDTH + x];
                if (pixel != 0u) {
                    framebuffer[(size_t)(receipt.master_destination_y + y) *
                                    M11_FB_WIDTH + x] = pixel;
                }
            }
        }
    }
    nextSourceVblankMs = m11_intro_active_ticks();
    if (!m11_dm1_amiga_wait_bovp(&nextSourceVblankMs,
                                 outQuitRequested)) goto cleanup;
    finalFadeStartedMs = m11_intro_active_ticks();
    if (!m11_dm1_amiga_fade_title_palette(
            activePalette, receipt.final_palette, framebuffer,
            outQuitRequested)) {
        goto cleanup;
    }
    ++paletteFadeCount;
    finalFadeElapsedMs = m11_intro_active_ticks() - finalFadeStartedMs;
    memcpy(activePalette, receipt.final_palette, sizeof(activePalette));
    if (M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                  M11_FB_HEIGHT) != M11_RENDER_OK) {
        goto cleanup;
    }
    if (g_m11_debug_startup_detail) {
        fprintf(stderr,
                "firestaff: startup-source-complete game=dm1 platform=%s phase=title-f0437 frames=%u palette-fades=%u source=%s graphics=%s zoom-elapsed-ms=%llu hold-elapsed-ms=%llu final-fade-elapsed-ms=%llu total-elapsed-ms=%llu\n",
                strcmp(version->versionId, "amiga-demo-en") == 0
                    ? "amiga-demo"
                    : (strcmp(version->versionId, "amiga21-en") == 0
                           ? "amiga-v21" : "amiga-v20"),
                DM1_V1_AMIGA_TITLE_ZOOM_STEP_COUNT,
                paletteFadeCount,
                receipt.executable_md5, receipt.graphics_md5,
                (unsigned long long)zoomElapsedMs,
                (unsigned long long)holdElapsedMs,
                (unsigned long long)finalFadeElapsedMs,
                (unsigned long long)(m11_intro_active_ticks() -
                                     titleStartedMs));
    }
    completed = 1;

cleanup:
    free(graphics);
    free(executable);
    free(c001);
    free(dungeon);
    free(master);
    free(dungeonData);
    free(dm);
    return completed;
}

/* ReDMCSB SWSH.C F0902/F0904 and SWSHSND.C F0908/F0909 use the embedded
 * Amiga Hunk executable's four bitplanes, 27 source palette rows, and Paula
 * sample. Accept only the selected authenticated Amiga executable profile,
 * read directly from its selected file/container, and leave the transaction on
 * its existing title/entrance path after this prelude. */
static int m11_play_dm1_amiga_swsh_if_available(
    const M12_StartupMenuState* menuState,
    const M12_MenuEntry* entry,
    int skipSwoosh) {
    int versionIndex;
    const M12_AssetVersionStatus* version;
    char swooshPath[FSP_PATH_MAX];
    uint8_t* executable = NULL;
    size_t executableBytes = 0U;
    DM1_V1_AmigaSwshAssets assets;
    uint8_t* indexed = NULL;
    uint8_t* rgba = NULL;
    unsigned char palette[16][3];
    M11_AudioState audio;
    int audioInitialized = 0;
    int completed = 0;
    unsigned int eventIndex;
    unsigned int sourceFrames = 0u;
    unsigned int sourceVblanks = 0u;
    Uint64 audioStartedAt = 0u;
    Uint64 sourcePresentationStartedMs = 0u;
    const unsigned int vblankMs = 20u;
    const Uint64 sourceSoundDurationMs =
        ((Uint64)DM1_V1_AMIGA_SWSH_SOUND_BYTES *
         DM1_V1_AMIGA_SWSH_PAULA_PERIOD * 1000u + 3579544u) / 3579545u;

    g_m11_dm1_amiga_swsh_palette_valid = 0;
    g_m11_startup_intro_quit_requested = 0;
    if (!menuState || !entry || !entry->gameId ||
        strcmp(entry->gameId, "dm1") != 0) return -1;
    versionIndex = m11_selected_dm1_launch_version_index(menuState, entry);
    if (versionIndex < 0 ||
        M12_AssetStatus_GetVersionArchitecture("dm1",
                                                (size_t)versionIndex) !=
            M12_ARCH_AMIGA) {
        return -1;
    }
    version = M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                          (size_t)versionIndex);
    if (!version || strstr(version->matchedPath, "[HD]") != NULL) return -1;
    if (skipSwoosh) return 1;
    if (!m11_dm1_amiga_swsh_path(version, swooshPath, sizeof(swooshPath)) ||
        !asset_read_path_alloc(swooshPath, &executable,
                                       &executableBytes) ||
        !executable ||
        !dm1_v1_amiga_swsh_parse(executable, executableBytes, &assets)) {
        goto cleanup;
    }
    indexed = (uint8_t*)malloc((size_t)DM1_V1_AMIGA_SWSH_WIDTH *
                               DM1_V1_AMIGA_SWSH_HEIGHT);
    rgba = (uint8_t*)malloc((size_t)DM1_V1_AMIGA_SWSH_WIDTH *
                            DM1_V1_AMIGA_SWSH_HEIGHT * 4u);
    if (!indexed || !rgba ||
        !dm1_v1_amiga_swsh_decode_logo(
            &assets, indexed,
            (size_t)DM1_V1_AMIGA_SWSH_WIDTH * DM1_V1_AMIGA_SWSH_HEIGHT)) {
        goto cleanup;
    }
    memset(palette, 0, sizeof(palette));
    m11_swsh_indexed_to_rgba(indexed, rgba, palette);
    sourcePresentationStartedMs = m11_intro_active_ticks();
    if (M11_Render_PresentRGBA(rgba, DM1_V1_AMIGA_SWSH_WIDTH,
                               DM1_V1_AMIGA_SWSH_HEIGHT) != M11_RENDER_OK) {
        goto cleanup;
    }
    m11_autotest_dm1_swsh_quit_after_first_frame();
    if (!m11_prepare_swsh_source_clock_after_launcher_handoff(
            rgba, DM1_V1_AMIGA_SWSH_WIDTH, DM1_V1_AMIGA_SWSH_HEIGHT,
            vblankMs)) {
        goto cleanup;
    }
    /* The two-frame launcher handoff is host synchronization, not a
     * source-owned SWSH palette interval. */
    sourcePresentationStartedMs = 0u;
    memset(&audio, 0, sizeof(audio));
    g_m11_intro_local_audio = &audio;
    if (M11_Audio_Init(&audio)) {
        audioInitialized = 1;
        (void)M11_ApplyIntroAudioPreferences(&audio, menuState);
        audioStartedAt = m11_intro_active_ticks();
        if (!M11_Audio_PlayAmigaDmaPcmAtPaulaVolume(
                &audio, assets.sound, (int)assets.soundBytes,
                DM1_V1_AMIGA_SWSH_PAULA_PERIOD,
                m11_dm1_amiga_swsh_fnv1a(assets.sound, assets.soundBytes),
                64)) {
            M11_Audio_Shutdown(&audio);
            audioInitialized = 0;
        }
    }
    for (eventIndex = 0u;
         eventIndex < DM1_V1_AMIGA_SWSH_PALETTE_PAIRS; ++eventIndex) {
        int paletteIndex;
        unsigned int value;
        if (M11_Render_PumpEvents()) {
            g_m11_startup_intro_quit_requested = 1;
            goto cleanup;
        }
        if (!dm1_v1_amiga_swsh_palette_event(&assets, eventIndex,
                                            &paletteIndex, &value)) {
            goto cleanup;
        }
        if (paletteIndex >= 0) {
            palette[paletteIndex][0] =
                (unsigned char)(((value >> 8) & 0x0fu) * 17u);
            palette[paletteIndex][1] =
                (unsigned char)(((value >> 4) & 0x0fu) * 17u);
            palette[paletteIndex][2] =
                (unsigned char)((value & 0x0fu) * 17u);
        } else {
            m11_swsh_indexed_to_rgba(indexed, rgba, palette);
            sourcePresentationStartedMs = m11_intro_active_ticks();
            if (M11_Render_PresentRGBA(rgba, DM1_V1_AMIGA_SWSH_WIDTH,
                                       DM1_V1_AMIGA_SWSH_HEIGHT) !=
                    M11_RENDER_OK ||
                m11_delay_ms_with_intro_event_pump(
                    m11_startup_remaining_delay_ms(
                        value * vblankMs,
                        sourcePresentationStartedMs))) {
                goto cleanup;
            }
            sourcePresentationStartedMs = 0u;
            sourceVblanks += value;
            ++sourceFrames;
        }
    }
    if (audioInitialized) {
        Uint64 elapsed = m11_intro_active_ticks() - audioStartedAt;
        if (elapsed < sourceSoundDurationMs &&
            m11_delay_ms_with_intro_event_pump(
                (unsigned int)(sourceSoundDurationMs - elapsed))) {
            goto cleanup;
        }
    }
    {
        uint8_t indexedPalette[256][3] = {{0}};
        unsigned char* framebuffer = M11_Render_GetFramebuffer();
        unsigned int color;
        if (!framebuffer ||
            DM1_V1_AMIGA_SWSH_WIDTH != M11_FB_WIDTH ||
            DM1_V1_AMIGA_SWSH_HEIGHT != M11_FB_HEIGHT) {
            goto cleanup;
        }
        for (color = 0u; color < 16u; ++color) {
            const unsigned int red = palette[color][0] / 17u;
            const unsigned int green = palette[color][1] / 17u;
            const unsigned int blue = palette[color][2] / 17u;
            if (palette[color][0] != red * 17u ||
                palette[color][1] != green * 17u ||
                palette[color][2] != blue * 17u) {
                goto cleanup;
            }
            indexedPalette[color][0] = (uint8_t)red;
            indexedPalette[color][1] = (uint8_t)green;
            indexedPalette[color][2] = (uint8_t)blue;
            g_m11_dm1_amiga_swsh_palette[color] =
                (uint16_t)((red << 8) | (green << 4) | blue);
        }
        if (M11_Render_SetIndexedPaletteRgb4(indexedPalette) !=
                M11_RENDER_OK) {
            goto cleanup;
        }
        memcpy(framebuffer, indexed,
               (size_t)DM1_V1_AMIGA_SWSH_WIDTH *
                   DM1_V1_AMIGA_SWSH_HEIGHT);
        if (M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                      M11_FB_HEIGHT) != M11_RENDER_OK) {
            goto cleanup;
        }
        g_m11_dm1_amiga_swsh_palette_valid = 1;
    }
    completed = 1;
    if (g_m11_debug_startup_detail) {
        fprintf(stderr,
                "firestaff: startup-source-complete game=dm1 phase=amiga-swsh profile=%s frames=%u source-vblanks=%u\n",
                assets.executableMd5, sourceFrames, sourceVblanks);
    }
cleanup:
    g_m11_intro_local_audio = NULL;
    if (audioInitialized) M11_Audio_Shutdown(&audio);
    free(executable);
    free(indexed);
    free(rgba);
    return completed;
}

static int m11_play_ftl_swoosh_for_game_if_available(
                                              const M12_StartupMenuState* menuState,
                                              const char* dataDir,
                                              const char* gameId,
                                              int skipSwoosh,
                                              const DM1_V1_StartupFullGraphicsMediaReceipt_PC34*
                                                  dm1MediaReceipt) {
    char logoPath[FSP_PATH_MAX];
    unsigned char* logoImg = NULL;
    unsigned char* screenFbPacked = NULL;
    unsigned char* screenFbIndexed = NULL;
    unsigned char* screenRgba = NULL;
    size_t logoImgSize = 0U;
    SWSH_CompatLogoPayload logoPayload;
    unsigned char swshPalette[16][3];
    M11_AudioState swshAudio;
    const unsigned char* dosoundProgram = NULL;
    unsigned int dosoundProgramBytes = 0U;
    int swshAudioInitialized = 0;
    int csbSwshAudioInitialized = 0;
    int completed = 0;
    CSB_V1_BootProfile csbBoot;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 dm1Media;
    int hasDm1Media = 0;
    g_m11_startup_intro_quit_requested = 0;
    if (skipSwoosh) return 1;
    /* SWSH is a source-owned prelude, not a V2 post-process input.  A prior
     * Modern session can otherwise leave its target/palette state latched
     * until the first ordinary game frame, causing this 320x200 RGBA logo to
     * be scaled or filtered through stale presentation state and occasionally
     * disappear during the immediate title handoff.  The selected V2 mode is
     * restored by m11_present_game_frame once the game view is active. */
    M11_Render_SetV2PresentationActive(0);
    M11_Render_SetModernPresentationActive(0);
    memset(&logoPayload, 0, sizeof(logoPayload));
    memset(&swshAudio, 0, sizeof(swshAudio));
    memset(&csbBoot, 0, sizeof(csbBoot));
    memset(&dm1Media, 0, sizeof(dm1Media));
    if (gameId && strcmp(gameId, "csb") == 0 && dataDir && dataDir[0]) {
        CSB_V1_BootProfile amigaBoot;
        CSB_V1_AmigaSwshFtlAssets amigaAssets;
        char amigaPath[FSP_PATH_MAX];
        char amigaMd5[33];
        uint8_t *amigaBytes = NULL;
        size_t amigaByteCount = 0u;
        uint8_t indexed[CSB_V1_AMIGA_SWSH_WIDTH * CSB_V1_AMIGA_SWSH_HEIGHT];
        uint8_t rgba[CSB_V1_AMIGA_SWSH_WIDTH * CSB_V1_AMIGA_SWSH_HEIGHT * 4u];
        unsigned char palette[16][3] = {{0}};
        M11_AudioState amigaAudio;
        int amigaAudioInitialized = 0;
        int amigaProfile = 0;
        int amigaComplete = 0;
        unsigned int sourceFrames = 0u;
        unsigned int sourceVblanks = 0u;
        Uint64 audioStartedAt = 0u;
        Uint64 soundDurationMs = 0u;

        memset(&amigaBoot, 0, sizeof(amigaBoot));
        memset(&amigaAssets, 0, sizeof(amigaAssets));
        memset(&amigaAudio, 0, sizeof(amigaAudio));
        csb_v1_boot_profile_init(&amigaBoot);
        if (csb_v1_boot_scan_assets(&amigaBoot, dataDir) == 0 &&
            amigaBoot.assets_verified &&
            amigaBoot.variant_id == CSB_V1_VARIANT_AMIGA31_MULTI) {
            amigaProfile = 1;
        }
        if (amigaProfile) {
            /* ReDMCSB APPA.C calls FTL_SWSH before FTL_ANIM and ignores a
             * missing prelude. Bind only to the selected Amiga disk member;
             * the PC SWOOSH finder must not substitute another edition. */
            if (csb_v1_amiga_selected_companion(amigaBoot.graphics_path,
                    "SWSH.FTL", amigaPath, sizeof(amigaPath)) &&
                asset_file_md5_hex(amigaPath, amigaMd5) &&
                strcmp(amigaMd5, "ff3872baaed8ee4e83ee3c0684b2eeec") == 0 &&
                asset_read_path_alloc(amigaPath, &amigaBytes,
                                      &amigaByteCount) &&
                csb_v1_amiga_swsh_ftl_decode(amigaBytes, amigaByteCount,
                                              &amigaAssets) &&
                redmcsb_amiga_graphic_expand(
                    amigaAssets.logo_graphic,
                    sizeof(amigaAssets.logo_graphic), indexed,
                    sizeof(indexed), NULL, NULL)) {
                soundDurationMs =
                    ((Uint64)sizeof(amigaAssets.sample) *
                     amigaAssets.sample_period * 1000u + 3579544u) /
                    3579545u;
                g_m11_intro_local_audio = &amigaAudio;
                m11_swsh_indexed_to_rgba(indexed, rgba, palette);
                if (M11_Render_PresentRGBA(rgba,
                        CSB_V1_AMIGA_SWSH_WIDTH,
                        CSB_V1_AMIGA_SWSH_HEIGHT) == M11_RENDER_OK &&
                    m11_prepare_swsh_source_clock_after_launcher_handoff(
                        rgba, CSB_V1_AMIGA_SWSH_WIDTH,
                        CSB_V1_AMIGA_SWSH_HEIGHT, 20u)) {
                    if (M11_Audio_Init(&amigaAudio)) {
                        amigaAudioInitialized = 1;
                        (void)M11_ApplyIntroAudioPreferences(&amigaAudio,
                                                              menuState);
                        audioStartedAt = m11_intro_active_ticks();
                        if (!M11_Audio_PlayAmigaDmaPcmAtPaulaVolume(
                                &amigaAudio, amigaAssets.sample,
                                (int)sizeof(amigaAssets.sample),
                                (int)amigaAssets.sample_period,
                                m11_dm1_amiga_swsh_fnv1a(
                                    amigaAssets.sample,
                                    sizeof(amigaAssets.sample)), 64)) {
                            M11_Audio_Shutdown(&amigaAudio);
                            amigaAudioInitialized = 0;
                        }
                    }
                    amigaComplete = 1;
                    for (unsigned int eventIndex = 0u;
                         eventIndex < CSB_V1_AMIGA_SWSH_PALETTE_EVENT_COUNT;
                         ++eventIndex) {
                        const CSB_V1_AmigaSwshPaletteEvent *event =
                            &amigaAssets.palette[eventIndex];
                        if (M11_Render_PumpEvents()) {
                            g_m11_startup_intro_quit_requested = 1;
                            amigaComplete = 0;
                            break;
                        }
                        if (event->command >= 0) {
                            const unsigned int color = event->parameter;
                            palette[event->command][0] =
                                (unsigned char)(((color >> 8) & 0x0fu) * 17u);
                            palette[event->command][1] =
                                (unsigned char)(((color >> 4) & 0x0fu) * 17u);
                            palette[event->command][2] =
                                (unsigned char)((color & 0x0fu) * 17u);
                            m11_swsh_indexed_to_rgba(indexed, rgba, palette);
                            if (M11_Render_PresentRGBA(rgba,
                                    CSB_V1_AMIGA_SWSH_WIDTH,
                                    CSB_V1_AMIGA_SWSH_HEIGHT) !=
                                    M11_RENDER_OK) {
                                amigaComplete = 0;
                                break;
                            }
                            ++sourceFrames;
                        } else {
                            m11_swsh_indexed_to_rgba(indexed, rgba, palette);
                            if (M11_Render_PresentRGBA(rgba,
                                    CSB_V1_AMIGA_SWSH_WIDTH,
                                    CSB_V1_AMIGA_SWSH_HEIGHT) !=
                                    M11_RENDER_OK ||
                                m11_delay_ms_with_intro_event_pump(
                                    event->parameter * 20u)) {
                                amigaComplete = 0;
                                break;
                            }
                            sourceVblanks += event->parameter;
                        }
                    }
                    if (amigaComplete && amigaAudioInitialized) {
                        const Uint64 elapsed = m11_intro_active_ticks() -
                                               audioStartedAt;
                        if (elapsed < soundDurationMs &&
                            m11_delay_ms_with_intro_event_pump(
                                (unsigned int)(soundDurationMs - elapsed))) {
                            amigaComplete = 0;
                        }
                    }
                }
                if (amigaComplete && g_m11_debug_startup_detail) {
                    fprintf(stderr,
                        "firestaff: startup-source-complete game=csb phase=amiga-swsh-ftl profile=%s frames=%u source-vblanks=%u audio=%s\n",
                        amigaMd5, sourceFrames, sourceVblanks,
                        !amigaAudioInitialized ||
                                !M11_Audio_IsAvailable(&amigaAudio)
                            ? "device-unavailable"
                            : (amigaAudio.queuedSampleCount > 0
                                   ? "queued"
                                   : "no-samples-queued"));
                }
            }
            if (!amigaComplete && g_m11_debug_startup_detail &&
                !g_m11_startup_intro_quit_requested) {
                fprintf(stderr,
                    "firestaff: startup-source-skip game=csb phase=amiga-swsh-ftl reason=missing-or-invalid-source\n");
            }
            g_m11_intro_local_audio = NULL;
            if (amigaAudioInitialized) M11_Audio_Shutdown(&amigaAudio);
            free(amigaBytes);
            return g_m11_startup_intro_quit_requested ? 0 : 1;
        }
    }
    if (dm1MediaReceipt && dm1MediaReceipt->handled) {
        dm1Media = *dm1MediaReceipt;
        hasDm1Media = 1;
    } else if (gameId && strcmp(gameId, "dm1") == 0) {
        hasDm1Media =
            dm1_v1_startup_full_graphics_media_receipt_for_source_pc34(
                gameId,
                &dm1Media);
    }
    /* ReDMCSB APPA.C calls FTL_SWSH through PRIM_26, ignores its result, then
     * continues into FTL_ANIM. The logo is an optional prelude; installations
     * containing the authenticated GRAPHICS.DAT/DUNGEON.DAT pair must still
     * start when SWSH is absent or unreadable. */
    if (!V1_SWSH_Intro_FindLogoPathForGame(menuState,
                                           dataDir,
                                           gameId,
                                           logoPath,
                                           sizeof(logoPath))) {
        if (g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "firestaff: startup-source-skip game=%s phase=swsh-intro reason=not-found\n",
                    gameId ? gameId : "unknown");
        }
        return 1;
    }
    if (!asset_read_path_alloc(logoPath, &logoImg, &logoImgSize) ||
        !logoImg || logoImgSize == 0U || logoImgSize > (size_t)UINT_MAX) {
        free(logoImg);
        if (g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "firestaff: startup-source-skip game=%s phase=swsh-intro reason=unreadable\n",
                    gameId ? gameId : "unknown");
        }
        return 1;
    }
    g_m11_intro_local_audio = &swshAudio;
    /* Atari ST low-res FTL logo: 4bpp packed, 160 bytes/row, 32000 bytes total.
     * Use a dedicated packed buffer for the SWSH_Compat_ExpandLogoToBitmap
     * output (was previously aliased onto screenFb which is 1bpp). */
    screenFbPacked  = (unsigned char*)malloc((size_t)(M11_FB_WIDTH * M11_FB_HEIGHT) / 2U);
    /* 1-byte-per-pixel indexed framebuffer that m11_swsh_indexed_to_rgba reads. */
    screenFbIndexed = (unsigned char*)calloc(1, (size_t)M11_FB_BYTES);
    screenRgba      = (unsigned char*)malloc((size_t)M11_FB_BYTES * 4U);
    if (!screenFbPacked || !screenFbIndexed || !screenRgba) goto cleanup;
    if (!SWSH_Compat_FindLogoImagePayloadEx(logoImg, (unsigned int)logoImgSize,
                                             &logoPayload)) goto cleanup;
    SWSH_Compat_ExpandLogoToBitmap(logoPayload.payload, screenFbPacked);
    /* BUG-PASS841-FIX: pass841 — the FTL swoosh logo was rendered as a
     * half-blank vertically-striped image because the runtime treated
     * the 4bpp-packed Atari ST low-res output of SWSH_Compat_ExpandLogoToBitmap
     * as a 1-byte-per-pixel indexed framebuffer. Unpack 4bpp->1bpp before
     * any palette/RGBA conversion. Without this, pixels 160..319 of every
     * row stay zero and even pixels 0..159 show alternating nibbles. */
    m11_swsh_unpack_4bpp_to_indexed(screenFbPacked, screenFbIndexed);

    /* ReDMCSB SWSH.C F2255:2975-3039 / DRAWVIEW.C G8162-G8171:
     * the PC/F20E path applies each Swoosh palette row before waiting
     * for its source delay. Present each palette mutation immediately
     * so the logo does not collapse several source colors into one
     * modern frame. */
    memset(swshPalette, 0, sizeof(swshPalette));
    {
      unsigned int sourceStep;
      unsigned int sourceFrames = 0U;
      unsigned int sourceVblanks = 0U;
      Uint64 sourceStartedMs = m11_intro_active_ticks();
      Uint64 sourcePresentationStartedMs = 0U;
      /* SWSH.C:10-14 starts the immutable V0901005 Dosound program before
       * it mutates a single palette register.  It is deliberately scoped to
       * DM1: CSB owns a different F0908 DMA source path.  A media receipt
       * whose cadence is not the PC34 PAL 20 ms VBlank cannot authorize this
       * PSG program, so leave audio silent rather than inventing a host cue. */
      if (gameId && strcmp(gameId, "dm1") == 0 &&
          (!hasDm1Media ||
           dm1Media.swsh_vblank_ms == SWSH_COMPAT_RUNTIME_VBLANK_MS) &&
          M11_Audio_Init(&swshAudio)) {
          (void)M11_ApplyIntroAudioPreferences(&swshAudio, menuState);
          dosoundProgram = SWSH_Compat_GetPc34DosoundProgram(
              &dosoundProgramBytes);
          if (dosoundProgram &&
              M11_Audio_PlayDm1SwshDosoundProgram(
                  &swshAudio,
                  dosoundProgram,
                  (int)dosoundProgramBytes,
                  hasDm1Media ? dm1Media.swsh_vblank_ms :
                                SWSH_COMPAT_RUNTIME_VBLANK_MS)) {
              swshAudioInitialized = 1;
          } else {
              M11_Audio_Shutdown(&swshAudio);
          }
      }
      /* CSB PC3.4 has its own SWSHSND.C F0908 DMA sample.  The audio
       * transport accepts that exact source format, but the old M11 prelude
       * initialized only the DM1 PSG branch.  Scan the selected CSB root
       * through the normal hash-first boot profile and queue only the
       * authenticated 9,078-byte source buffer.  Missing media remains
       * silent: no DM1 cue or generated substitute is permitted. */
      if (gameId && strcmp(gameId, "csb") == 0 && dataDir && dataDir[0]) {
          csb_v1_boot_profile_init(&csbBoot);
          if (csb_v1_boot_scan_assets(&csbBoot, dataDir) == 0 &&
              csbBoot.swoosh_source_bound && M11_Audio_Init(&swshAudio)) {
              (void)M11_ApplyIntroAudioPreferences(&swshAudio, menuState);
              if (M11_Audio_PlayCsbSwshPcm(
                      &swshAudio,
                      csbBoot.swoosh_source_bytes,
                      (int)sizeof(csbBoot.swoosh_source_bytes),
                      CSB_V1_SWSH_F0908_SOUND_PERIOD_PC34,
                      csbBoot.swoosh_source_fnv1a)) {
                  csbSwshAudioInitialized = 1;
              } else {
                  M11_Audio_Shutdown(&swshAudio);
              }
          }
      }
      {
          const unsigned int initialHoldMs =
              hasDm1Media ? dm1Media.swsh_initial_logo_hold_ms :
                            SWSH_Compat_GetRuntimeInitialLogoHoldMs();
          const unsigned int vblankMs =
              hasDm1Media && dm1Media.swsh_vblank_ms != 0U
                  ? dm1Media.swsh_vblank_ms
                  : SWSH_COMPAT_RUNTIME_VBLANK_MS;
          m11_swsh_indexed_to_rgba(screenFbIndexed, screenRgba, swshPalette);
          sourcePresentationStartedMs = m11_intro_active_ticks();
          if (M11_Render_PresentRGBA(screenRgba, M11_FB_WIDTH,
                                     M11_FB_HEIGHT) != M11_RENDER_OK) {
              goto cleanup;
          }
          m11_autotest_dm1_swsh_quit_after_first_frame();
          if (!m11_prepare_swsh_source_clock_after_launcher_handoff(
                  screenRgba, M11_FB_WIDTH, M11_FB_HEIGHT, vblankMs)) {
              goto cleanup;
          }
          /* The host prelude above is synchronization and does not consume
           * SWSH.C's source initial hold. */
          sourcePresentationStartedMs = 0U;
          if (m11_delay_ms_with_intro_event_pump(initialHoldMs)) {
              goto cleanup;
          }
      }
      for (sourceStep = 1U; sourceStep <= SWSH_Compat_GetSourceAnimationStepCount(); ++sourceStep) {
          SWSH_CompatSourceAnimationStep step;
          if (M11_Render_PumpEvents()) {
              g_m11_startup_intro_quit_requested = 1;
              goto cleanup;
          }
          if (!SWSH_Compat_GetSourceAnimationStep(sourceStep, &step)) goto cleanup;
          if (step.kind == SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR) {
              SWSH_Compat_ConvertPcSwooshRgbWordToRgb8(step.colorValue,
                                                       swshPalette[step.colorIndex & 0x0FU]);
              m11_swsh_indexed_to_rgba(screenFbIndexed, screenRgba, swshPalette);
              sourcePresentationStartedMs = m11_intro_active_ticks();
              if (M11_Render_PresentRGBA(screenRgba, M11_FB_WIDTH,
                                         M11_FB_HEIGHT) != M11_RENDER_OK) {
                  goto cleanup;
              }
              ++sourceFrames;
          } else if (step.kind == SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS) {
              /* ReDMCSB SWSH.C:33-37: each Vsync wait is one 50 Hz vertical
               * blank (~20 ms).  Use wall-clock timing so high-refresh displays
               * (e.g. MacBook Pro 120 Hz ProMotion) do not race through the
               * palette animation faster than the original Atari ST rate.
               * The previous dead-code `paletteDirty` flag was removed:
               * the WAIT_VBLANKS branch never had a palette to "re-render"
               * because every SET_PALETTE_COLOR step renders its own frame. */
              sourceVblanks += step.vblankCount;
              {
                  const unsigned int sourceWaitMs = hasDm1Media
                      ? m11_startup_media_swsh_wait_ms(&dm1Media,
                                                        step.vblankCount)
                      : SWSH_Compat_GetRuntimeDelayMsForVblankCount(
                            step.vblankCount);
                  const unsigned int remainingWaitMs =
                      m11_startup_remaining_delay_ms(
                          sourceWaitMs, sourcePresentationStartedMs);
                  sourcePresentationStartedMs = 0U;
                  if (m11_delay_ms_with_intro_event_pump(remainingWaitMs)) {
                      goto cleanup;
                  }
              }
          } else if (step.kind == SWSH_COMPAT_SOURCE_EVENT_RUN_START_PROGRAM) {
              /* No palette was queued between the previous SET_PALETTE_COLOR
               * and now (the dead paletteDirty=1 path was never reachable). */
          }
      }
      if (m11_delay_ms_with_intro_event_pump(
              hasDm1Media ? dm1Media.swsh_final_hold_ms :
                            SWSH_Compat_GetRuntimeFinalHoldMs())) {
          goto cleanup;
      }
      completed = 1;
      if (g_m11_debug_startup_detail) {
          fprintf(stderr,
                  "firestaff: startup-source-complete game=%s phase=swsh-intro frames=%u source-vblanks=%u elapsed-ms=%llu\n",
                  gameId ? gameId : "unknown",
                  sourceFrames,
                  sourceVblanks,
                  (unsigned long long)(m11_intro_active_ticks() -
                                       sourceStartedMs));
      }
    }
cleanup:
    g_m11_intro_local_audio = NULL;
    if (swshAudioInitialized || csbSwshAudioInitialized) {
        M11_Audio_Shutdown(&swshAudio);
    }
    SWSH_Compat_ReleaseLogoImagePayload(&logoPayload);
    if (logoImg) free(logoImg);
    if (screenFbPacked) free(screenFbPacked);
    if (screenFbIndexed) free(screenFbIndexed);
    if (screenRgba) free(screenRgba);
    return completed;
}

static int m11_play_ftl_swoosh_if_available(const M12_StartupMenuState* menuState,
                                              const char* dataDir,
                                              int skipSwoosh,
                                              const DM1_V1_StartupFullGraphicsMediaReceipt_PC34*
                                                  dm1MediaReceipt) {
    return m11_play_ftl_swoosh_for_game_if_available(menuState,
                                                      dataDir,
                                                      "dm1",
                                                      skipSwoosh,
                                                      dm1MediaReceipt);
}

static int m11_play_redmcsb_title_graphic_intro_if_available(
    const M12_StartupMenuState* menuState,
    M11_GameViewState* gameView,
    const char* sourceId,
    int* outCompleted,
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* dm1MediaReceipt) {
    const M11_AssetSlot* titleGraphic;
    unsigned char* framebuffer;
    int titlePalette = -1;
    Uint64 presentationStartedMs = 0U;
    Uint64 titleStartedMs = 0U;
    unsigned int sourceStep;
    unsigned int completedSteps = 0U;
    int autotestQuitInjected = 0;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 dm1Media;
    DM1_V1_StartupTitleRuntimeAssetReceipt_PC34 titleAssetReceipt;
    DM1_V1_StartupTitleSourceHandoffReceipt_PC34 titleSourceHandoff;
    DM1_V1_StartupTitlePresentationCommand_PC34 command;
    char titleDatPath[FSP_PATH_MAX];
    const char* titleDatProvenancePath = NULL;

    (void)menuState;
    if (outCompleted) {
        *outCompleted = 0;
    }
    /* ReDMCSB TITLE.C has separate PC/F20 and CSB/A31 implementations.
     * C001 plus C12/C13/C14 is the PC/F20 DM1 contract only; never let a
     * caller selecting CSB consume this DM1 asset/palette route. */
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(sourceId) ||
        !gameView || !gameView->assetsAvailable) {
        return 0;
    }
    gameView->dm1StartupTitleRuntimeSource =
        V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP;
    gameView->dm1StartupTitleCompletedSteps = 0U;
    /* TITLE.C renders before m11_present_game_frame() can activate V2.0. */
    M11_Render_SetV2PresentationActive(
        m11_dm1_v20_presentation_active(gameView));
    titleGraphic = M11_AssetLoader_Load(&gameView->assetLoader, 1U);
    {
        DM1_V1_StartupTitleRuntimeSourceReceipt_PC34 sourceReceipt;
        if (!dm1_v1_startup_title_runtime_source_receipt_pc34(
                "dm1",
                titleGraphic != NULL,
                titleGraphic ? titleGraphic->width : 0U,
                titleGraphic ? titleGraphic->height : 0U,
                0,
                &sourceReceipt) ||
            !sourceReceipt.handled ||
            sourceReceipt.selected_runtime_source !=
                (int)V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001) {
            return 0;
        }
        gameView->dm1StartupTitleRuntimeSource =
            sourceReceipt.selected_runtime_source;
    }
    framebuffer = M11_Render_GetFramebuffer();
    if (!framebuffer) {
        return 0;
    }
    memset(&dm1Media, 0, sizeof(dm1Media));
    if (dm1MediaReceipt && dm1MediaReceipt->handled) {
        dm1Media = *dm1MediaReceipt;
    } else {
        (void)dm1_v1_startup_full_graphics_media_receipt_for_source_pc34(
            "dm1", &dm1Media);
    }

    memset(framebuffer, 0, (size_t)M11_FB_BYTES);
    memset(&titleAssetReceipt, 0, sizeof(titleAssetReceipt));
    if (!dm1_v1_startup_title_runtime_asset_receipt_pc34(
            "dm1",
            titleGraphic ? titleGraphic->pixels : NULL,
            titleGraphic ? titleGraphic->width : 0U,
            titleGraphic ? titleGraphic->height : 0U,
            &titleAssetReceipt)) {
        memset(&titleAssetReceipt, 0, sizeof(titleAssetReceipt));
    }
    titleDatPath[0] = '\0';
    /* ReDMCSB STARTUP1.C:160-170 selects Atari's C001 title graphic, and
     * TITLE.C:309-409 renders that graphic directly. TITLE.DAT is optional
     * PC/F20 provenance; searching it recursively on Atari startup can scan
     * every mounted archive even though this route never consumes it. */
    if (dm1Media.platform != DM1_V1_STARTUP_MEDIA_PLATFORM_ATARI_ST &&
        V1_TitleIntro_FindTitleDatPath(menuState, NULL, titleDatPath,
                                       sizeof(titleDatPath))) {
        titleDatProvenancePath = titleDatPath;
    }
    memset(&titleSourceHandoff, 0, sizeof(titleSourceHandoff));
    if (!dm1_v1_startup_title_source_handoff_receipt_pc34(
            "dm1", titleDatProvenancePath,
            titleGraphic ? titleGraphic->pixels : NULL,
            titleGraphic ? titleGraphic->width : 0U,
            titleGraphic ? titleGraphic->height : 0U,
            &titleSourceHandoff) ||
        !dm1_v1_startup_title_source_handoff_valid_pc34(
            &titleSourceHandoff)) {
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 PC/F20 source-lock:
     * - TITLE.C:309 loads/decompresses C001_GRAPHIC_TITLE.
     * - TITLE.C:319-324 blits PRESENTS from source y=137 to 0,90..105.
     * - TITLE.C:340-360 builds 18 shrinked 320x80 title bitmaps.
     * - TITLE.C:385-387 waits VBlank and blits those bitmaps in reverse
     *   order (small 48x12 at 136,74 through full 320x80 at 0,40).
     * - TITLE.C:395-402 waits twice, then blits MASTER from source y=80
     *   to 0,118..174 with black transparency, before the final guard.
     *
     * Firestaff previously drove this handoff from the decoded TITLE.DAT
     * animation bank.  That bank is a separate PC title-file animation; the
     * ReDMCSB startup TITLE routine uses GRAPHICS.DAT graphic C001 instead.
     */
    titleStartedMs = m11_intro_active_ticks();
    for (sourceStep = 1U; sourceStep <= V1_TitleFrontend_GetSourceAnimationStepCount(); ++sourceStep) {
        V1_TitleFrontendSourceAnimationStep step;
        V1_TitleFrontendC001BlitPlan blitPlan;
        int stepPalette;
        Uint64 stepStartedMs = m11_intro_active_ticks();
        Uint64 presentElapsedMs = 0U;
        Uint64 waitElapsedMs = 0U;
        unsigned int requestedWaitMs = 0U;
        if (!V1_TitleFrontend_GetSourceAnimationStep(sourceStep, &step)) {
            break;
        }
        if (!V1_TitleFrontend_GetC001BlitPlanForStep(&step, &blitPlan)) {
            break;
        }
        memset(&command, 0, sizeof(command));
        if (!dm1_v1_startup_title_presentation_command_pc34(
                &dm1Media, &titleAssetReceipt, sourceStep, &command)) {
            break;
        }
        if (!V1_TitleFrontend_GetStepPalette(step.kind, &stepPalette) ||
            stepPalette != command.special_palette) {
            break;
        }
        {
            DM1_V2_StartupTitleFilterHandoffReceiptPc34 filterReceipt;
            const int paletteValid = command.special_palette >= 0;

            (void)dm1_v2_startup_title_filter_handoff_pc34(
                sourceId,
                gameView->presentationMode,
                command.source_timing_receipt_consumed,
                paletteValid,
                &filterReceipt);
            M11_Render_SetV2PresentationActive(filterReceipt.filters_active);
        }
        if (command.clear_before_present) {
            memset(framebuffer, 0, (size_t)M11_FB_BYTES);
        }
        /* ReDMCSB TITLE.C F0437:362-387: the C13/C14 palette is live
         * before the first zoom VBlank, not after it.  Present the
         * cleared indexed surface to make the hardware-equivalent palette
         * latch visible before waiting. */
        if (command.palette_before_pre_present_delay &&
            titlePalette != stepPalette) {
            presentationStartedMs = m11_intro_active_ticks();
            {
                Uint64 presentStartedMs = m11_intro_active_ticks();
                if (!m11_present_dm1_startup_special_palette(
                        gameView, framebuffer, stepPalette)) {
                    break;
                }
                presentElapsedMs += m11_intro_active_ticks() - presentStartedMs;
            }
        }
        /* ReDMCSB TITLE.C F0437:385-387 waits before each prepared
         * zoom bitmap is blitted. Steps 20, 21, and 23 model the two
         * post-zoom waits and the final guard individually, so do not add
         * an aggregate delay after this loop. */
        if (command.pre_present_delay_ms > 0U) {
            const unsigned int delayMs = m11_startup_remaining_delay_ms(
                command.pre_present_delay_ms, presentationStartedMs);
            const Uint64 waitStartedMs = m11_intro_active_ticks();
            requestedWaitMs += delayMs;
            if (m11_delay_ms_with_intro_event_pump(delayMs)) {
                break;
            }
            waitElapsedMs += m11_intro_active_ticks() - waitStartedMs;
        }
        presentationStartedMs = 0U;
        if (!command.present_frame) {
            ++completedSteps;
            gameView->dm1StartupTitleCompletedSteps = completedSteps;
            if (g_m11_debug_startup_detail) {
                fprintf(stderr,
                        "firestaff: startup-source-frame game=dm1 platform=pc phase=title-f0437 step=%u/%u event=%u presented=0 requested-wait-ms=%u actual-wait-ms=%llu present-ms=%llu step-elapsed-ms=%llu total-elapsed-ms=%llu fast-forward=%d\n",
                        sourceStep,
                        V1_TitleFrontend_GetSourceAnimationStepCount(),
                        (unsigned int)step.kind,
                        requestedWaitMs,
                        (unsigned long long)waitElapsedMs,
                        (unsigned long long)presentElapsedMs,
                        (unsigned long long)(m11_intro_active_ticks() - stepStartedMs),
                        (unsigned long long)(m11_intro_active_ticks() - titleStartedMs),
                        g_m11_intro_delay_fast_forward);
            }
            continue;
        }
        if (blitPlan.kind == V1_TITLE_FRONTEND_C001_BLIT_REGION) {
            M11_AssetLoader_BlitRegion(titleGraphic,
                                       (int)blitPlan.srcX,
                                       (int)blitPlan.srcY,
                                       (int)blitPlan.srcW,
                                       (int)blitPlan.srcH,
                                       framebuffer,
                                       M11_FB_WIDTH,
                                       M11_FB_HEIGHT,
                                       (int)blitPlan.dstX,
                                       (int)blitPlan.dstY,
                                       blitPlan.transparentColor);
        } else if (blitPlan.kind == V1_TITLE_FRONTEND_C001_BLIT_SCALED_REGION) {
            M11_AssetLoader_BlitSubRectScaled(titleGraphic,
                                              framebuffer,
                                              M11_FB_WIDTH,
                                              M11_FB_HEIGHT,
                                              (int)blitPlan.dstX,
                                              (int)blitPlan.dstY,
                                              (int)blitPlan.dstW,
                                              (int)blitPlan.dstH,
                                              (int)blitPlan.srcX,
                                              (int)blitPlan.srcY,
                                              (int)blitPlan.srcW,
                                              (int)blitPlan.srcH,
                                              blitPlan.transparentColor);
        } else {
            break;
        }

        /* ReDMCSB TITLE.C F0437 PC/F20: PRESENTS uses C12_PRESENTS
         * (VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS), ZOOM and STRIKES
         * BACK use the merged C13_DUNGEON + C14_MASTER palette
         * (VGA_PALETTE_PC34_SPECIAL_TITLE).  The helper
         * V1_TitleFrontend_GetStepPalette is the single source of
         * truth for that mapping; v2.7.4 always used
         * VGA_PALETTE_PC34_SPECIAL_TITLE for every step and painted
         * the "PRESENTS" word red instead of plain white. */
        presentationStartedMs = m11_intro_active_ticks();
        {
            Uint64 presentStartedMs = m11_intro_active_ticks();
            if (!m11_present_dm1_startup_special_palette(
                    gameView, framebuffer, stepPalette)) {
                break;
            }
            presentElapsedMs += m11_intro_active_ticks() - presentStartedMs;
        }
        if (sourceStep == 1U || sourceStep == 10U || sourceStep == 19U) {
            const char* phase = sourceStep == 1U
                ? "c001-first-frame"
                : (sourceStep == 10U ? "c001-zoom-mid" : "c001-zoom-full");
            m11_capture_dm1_startup_presented_frame(&dm1Media, phase);
        }
        if (command.post_present_delay_ms > 0U) {
            const unsigned int delayMs = m11_startup_remaining_delay_ms(
                command.post_present_delay_ms, presentationStartedMs);
            const Uint64 waitStartedMs = m11_intro_active_ticks();
            requestedWaitMs += delayMs;
            if (m11_delay_ms_with_intro_event_pump(delayMs)) {
                break;
            }
            waitElapsedMs += m11_intro_active_ticks() - waitStartedMs;
        }
        if (command.post_present_delay_ms > 0U) {
            presentationStartedMs = 0U;
        }
        ++completedSteps;
        gameView->dm1StartupTitleCompletedSteps = completedSteps;
        if (g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "firestaff: startup-source-frame game=dm1 platform=pc phase=title-f0437 step=%u/%u event=%u presented=1 requested-wait-ms=%u actual-wait-ms=%llu present-ms=%llu step-elapsed-ms=%llu total-elapsed-ms=%llu fast-forward=%d\n",
                    sourceStep,
                    V1_TitleFrontend_GetSourceAnimationStepCount(),
                    (unsigned int)step.kind,
                    requestedWaitMs,
                    (unsigned long long)waitElapsedMs,
                    (unsigned long long)presentElapsedMs,
                    (unsigned long long)(m11_intro_active_ticks() - stepStartedMs),
                    (unsigned long long)(m11_intro_active_ticks() - titleStartedMs),
                    g_m11_intro_delay_fast_forward);
        }
        /* Deterministically exercise host close handling against the first
         * rendered source frame. This hook is inert unless both explicit
         * autotest switches are set; ReDMCSB TITLE.C F0437 itself has no
         * host-window close event to consume. */
        if (!autotestQuitInjected && getenv("FIRESTAFF_AUTOTEST") &&
            getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_QUIT_AFTER_FRAME") &&
            strcmp(getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_QUIT_AFTER_FRAME"),
                   "1") == 0) {
            SDL_Event event;
            memset(&event, 0, sizeof(event));
#if SDL_VERSION_ATLEAST(3, 0, 0)
            event.type = SDL_EVENT_QUIT;
#else
            event.type = SDL_QUIT;
#endif
            if (SDL_PushEvent(&event)) {
                autotestQuitInjected = 1;
                fprintf(stderr,
                        "AUTOTEST: DM1 C001 quit injected after first rendered frame\n");
            }
        }
    }
    if (completedSteps == V1_TitleFrontend_GetSourceAnimationStepCount()) {
        if (g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "firestaff: startup-source-complete game=dm1 platform=pc phase=title-f0437 steps=%u elapsed-ms=%llu\n",
                    completedSteps,
                    (unsigned long long)(m11_intro_active_ticks() - titleStartedMs));
        }
        if (outCompleted) {
            *outCompleted = 1;
        }
        return 1;
    }
    /* A user quit during the source-timed C001 animation is a normal app
     * exit, not missing or corrupt title media. Let the selected startup
     * route propagate that command without displaying a launch failure. */
    if (M11_Render_PumpEvents()) {
        return M11_ENTRANCE_COMMAND_QUIT;
    }
    return 0;
}

static int m11_dm1_title_candidate_matches_pc34(const uint8_t *bytes,
                                                 size_t size) {
    char hex[65];
    uint8_t digest[32];
    size_t i;
    if (!bytes || firestaff_x68k_media_receipt_sha256_hex(
                      bytes, size, hex, sizeof(hex)) != 0) return 0;
    for (i = 0u; i < sizeof(digest); ++i) {
        unsigned int hi, lo;
        char a = hex[i * 2u], b = hex[i * 2u + 1u];
        hi = a >= '0' && a <= '9' ? (unsigned)(a - '0')
           : a >= 'a' && a <= 'f' ? (unsigned)(a - 'a' + 10) : 255u;
        lo = b >= '0' && b <= '9' ? (unsigned)(b - '0')
           : b >= 'a' && b <= 'f' ? (unsigned)(b - 'a' + 10) : 255u;
        if (hi > 15u || lo > 15u) return 0;
        digest[i] = (uint8_t)((hi << 4u) | lo);
    }
    return dm1_v1_dos_pc34_file_fp_matches_pc34("TITLE", size, digest);
}

/* IBMIO.C:2411-2418 starts ANIM TITLE and SELECTOR from one installation.
 * Loose DATA layouts may retain the original DOS filename case; resolve only
 * that directory and its DATA parent, never an unrelated neighboring install. */
static int m11_dm1_i34e_loose_companion_path(
    const M12_AssetVersionStatus* version, const char* wanted,
    char* outPath, size_t outBytes) {
    char root[FSP_PATH_MAX];
    char parent[FSP_PATH_MAX];
    const char* leaf;
    const char* backslash;
    int resolved;
    if (!version || !version->matched || !version->versionId ||
        strcmp(version->versionId, "pc34-en") != 0 ||
        !version->matchedPath[0] || strstr(version->matchedPath, "::") ||
        !version->matchedMd5[0] ||
        !asset_file_matches_md5(version->matchedPath, version->matchedMd5) ||
        !FSP_ParentDir(root, sizeof(root), version->matchedPath)) return 0;
    resolved = m11_find_unique_loose_sibling(root, wanted, outPath, outBytes);
    leaf = strrchr(root, '/');
    backslash = strrchr(root, '\\');
    if (!leaf || (backslash && backslash > leaf)) leaf = backslash;
    leaf = leaf ? leaf + 1 : root;
    /* A missing companion may belong beside DATA. A present ambiguous or
     * invalid companion must not fall through to another source directory. */
    if (resolved == 0 && m11_ascii_path_equal_ignore_case(leaf, "DATA") &&
        FSP_ParentDir(parent, sizeof(parent), root) && strcmp(parent, root) != 0) {
        snprintf(root, sizeof(root), "%s", parent);
        resolved = m11_find_unique_loose_sibling(root, wanted, outPath, outBytes);
    }
    if (g_m11_debug_startup_detail) {
        if (resolved == 1)
            fprintf(stderr, "firestaff: startup-sibling-resolved game=dm1 platform=pc34 wanted=%s parent=%s source=%s\n",
                    wanted, root, outPath);
        else
            fprintf(stderr, "firestaff: startup-sibling-rejected game=dm1 platform=pc34 wanted=%s parent=%s reason=%s\n",
                    wanted, root, resolved == -2 ? "ambiguous" :
                    resolved == -1 ? "enumeration" : "missing");
    }
    return resolved == 1;
}

static int m11_dm1_i34e_title_read_authentic(
    const M12_StartupMenuState *menuState,
    const M12_AssetVersionStatus *version,
    uint8_t **outBytes, size_t *outSize) {
    char root[FSP_PATH_MAX];
    char candidate[FSP_PATH_MAX];
    const char *ext;
    char candidates[3][FSP_PATH_MAX];
    size_t candidateCount = 0u;
    size_t rootBytes;
    uint8_t *bytes = NULL;
    size_t size = 0u;
    size_t candidateIndex;
    if (outBytes) *outBytes = NULL;
    if (outSize) *outSize = 0u;
    if (!menuState || !version || !version->matched || !version->versionId ||
        strcmp(version->versionId, "pc34-en") != 0 || !outBytes || !outSize) {
        return 0;
    }
    if (!M12_AssetStatus_ResolveRuntimeDataDirForVersion(
            &menuState->assetStatus, "dm1", version->versionId,
            root, sizeof(root))) {
        if (g_m11_debug_startup_detail)
            fprintf(stderr, "firestaff: startup-source-skip game=dm1 platform=pc34 phase=anim-title reason=runtime-data-dir-unresolved\n");
        return 0;
    }
    if (g_m11_debug_startup_detail)
        fprintf(stderr, "firestaff: startup-source-probe game=dm1 platform=pc34 phase=anim-title root=%s\n", root);
    rootBytes = strlen(root);
    ext = strrchr(root, '.');
    if (ext && m11_ascii_path_equal_ignore_case(ext, ".zip")) {
        const char *member = strstr(version->matchedPath, "::");
        if (member) {
            size_t memberBytes = strlen(member + 2u);
            if (memberBytes + 1u < sizeof(candidate)) {
                memcpy(candidate, member + 2u, memberBytes + 1u);
                char *graphicsSlash = strrchr(candidate, '/');
                if (graphicsSlash &&
                    m11_ascii_path_equal_ignore_case(
                        graphicsSlash + 1, "GRAPHICS.DAT")) {
                    *graphicsSlash = '\0';
                    char *dataSlash = strrchr(candidate, '/');
                    if (dataSlash &&
                        m11_ascii_path_equal_ignore_case(
                            dataSlash + 1, "DATA")) {
                        *dataSlash = '\0';
                        int written = snprintf(candidates[candidateCount],
                                     sizeof(candidates[candidateCount]),
                                     "%.*s::%s/TITLE",
                                     (int)(member - version->matchedPath),
                                     version->matchedPath,
                                     candidate);
                        if (written > 0 &&
                            (size_t)written <
                                sizeof(candidates[candidateCount])) {
                            ++candidateCount;
                        }
                    }
                }
            }
        }
        if (rootBytes + sizeof("::TITLE") <= sizeof(candidates[0])) {
            memcpy(candidates[candidateCount], root, rootBytes);
            strcpy(candidates[candidateCount] + rootBytes, "::TITLE");
            ++candidateCount;
        }
    } else if (m11_dm1_i34e_loose_companion_path(
                   version, "TITLE", candidates[0], sizeof(candidates[0]))) {
        candidateCount = 1u;
    }
    for (candidateIndex = 0u; candidateIndex < candidateCount;
         ++candidateIndex) {
        snprintf(candidate, sizeof(candidate), "%s",
                 candidates[candidateIndex]);
        if (g_m11_debug_startup_detail)
            fprintf(stderr, "firestaff: startup-source-probe game=dm1 platform=pc34 phase=anim-title file=%s\n", candidate);
        if (((ext && m11_ascii_path_equal_ignore_case(ext, ".zip"))
                 ? asset_read_virtual_path_alloc(candidate, &bytes, &size)
                 : asset_read_path_alloc(candidate, &bytes, &size))) {
            if (m11_dm1_title_candidate_matches_pc34(bytes, size)) {
                *outBytes = bytes;
                *outSize = size;
                if (g_m11_debug_startup_detail)
                    fprintf(stderr, "firestaff: startup-source-ready game=dm1 platform=pc34 phase=anim-title bytes=%zu\n", size);
                return 1;
            }
            if (g_m11_debug_startup_detail)
                fprintf(stderr, "firestaff: startup-source-reject game=dm1 platform=pc34 phase=anim-title bytes=%zu reason=fingerprint-mismatch\n", size);
            free(bytes); bytes = NULL; size = 0u;
        }
    }
    if (g_m11_debug_startup_detail)
        fprintf(stderr, "firestaff: startup-source-skip game=dm1 platform=pc34 phase=anim-title reason=title-not-found\n");
    return 0;
}

/* I34E ANIM TITLE runs before SELECTOR; M12's already-selected launch state
 * stands in for that menu. ANIM.C:2043 assigns raw counters; default IBMIO
 * F8114:1280-1290 returns decrement 3 at the BIOS/PIT cadence. ReDMCSB
 * ANIM.C F8292_ dispatches PL channels to
 * VGA VIDEODRV.C F8230, which writes (value << 2) + 3 to DAC6; the indexed
 * RGB6 renderer preserves those values. */
static int m11_play_dm1_i34e_title_anim_if_available(
    const M12_StartupMenuState *menuState,
    const M12_AssetVersionStatus *version) {
    DM1_V1_I34ETitleAnimPc34 player;
    DM1_V1_I34ETitleAnimFramePc34 frame;
    uint8_t *data = NULL;
    size_t size = 0u;
    unsigned char *framebuffer;
    uint8_t palette[256][3];
    uint64_t startMs;
    uint64_t animationEndMs;
    uint64_t counterUnits = 0u;
    uint64_t pitEpochMs;
    unsigned frames = 0u;
    int step;
    int userAborted = 0;
    Uint32 userAbortEventType = 0u;
    Sint32 userAbortKey = 0;
    Sint32 userAbortScancode = 0;
    Uint32 userAbortWindowId = 0u;
    Uint64 userAbortTimestamp = 0u;
    Uint8 userAbortRepeat = 0u;
    int autotestQuitInjected = 0;
    int autotestKeyInjected = 0;
    int autotestMouseInjected = 0;
    if (!version || !version->versionId ||
        strcmp(version->versionId, "pc34-en") != 0 ||
        !m11_dm1_i34e_title_read_authentic(menuState, version, &data, &size))
        return 0;
    if (!dm1_v1_i34e_title_anim_pc34_init(&player, data, size)) {
        free(data);
        return 0;
    }
    framebuffer = M11_Render_GetFramebuffer();
    if (!framebuffer) { free(data); return 0; }
    pitEpochMs = m11_intro_active_ticks();
    /* IBMIO.C:2413 selects -as (ANIM.C main:1894-1898,1974-1976):
     * erase the previous phase before clearing/playing TITLE. F8169's
     * pixel order is exact; the existing chunk presentation cadence is a
     * host approximation, not a measured original CPU-duration receipt. */
    {
        DM1_V1_I34ETitleErasePc34 erase;
        uint64_t eraseStartedMs = m11_intro_active_ticks();
        unsigned eraseFrames = 0u;
        int eraseResult;
        dm1_v1_i34e_title_erase_pc34_init(&erase);
        do {
            SDL_Event event;
            eraseResult = dm1_v1_i34e_title_erase_pc34_step(
                &erase, framebuffer, (size_t)M11_FB_BYTES, 1024u);
            if (eraseResult < 0 ||
                M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                          M11_FB_HEIGHT) != M11_RENDER_OK) {
                free(data); return 0;
            }
            ++eraseFrames;
            if (eraseFrames == 1u)
                m11_capture_dm1_authenticated_startup_frame(
                    "i34e-title-erase-first");
            else if (eraseFrames == 32u)
                m11_capture_dm1_authenticated_startup_frame(
                    "i34e-title-erase-mid");
            else if (eraseResult == 0)
                m11_capture_dm1_authenticated_startup_frame(
                    "i34e-title-erase-final");
            if (eraseResult == 0) break;
            for (;;) {
                if (m11_phase_duration_expired() ||
                    m11_intro_wait_for_focus()) {
                    free(data);
                    return M11_ENTRANCE_COMMAND_QUIT;
                }
                while (SDL_PollEvent(&event)) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
                    if (event.type == SDL_EVENT_QUIT) {
#else
                    if (event.type == SDL_QUIT) {
#endif
                        g_m11_startup_intro_quit_requested = 1;
                        free(data);
                        return M11_ENTRANCE_COMMAND_QUIT;
                    }
#if SDL_VERSION_ATLEAST(3, 0, 0)
                    if (event.type == SDL_EVENT_WINDOW_RESIZED)
                        M11_Render_HandleResize(event.window.data1,
                                                event.window.data2);
#else
                    if (event.type == SDL_WINDOWEVENT &&
                        event.window.event == SDL_WINDOWEVENT_RESIZED)
                        M11_Render_HandleResize(event.window.data1,
                                                event.window.data2);
#endif
                }
                if (m11_intro_active_ticks() - eraseStartedMs >=
                    ((uint64_t)eraseFrames * 1000u +
                     DM1_V1_VGA_VBLANK_HZ - 1u) /
                        DM1_V1_VGA_VBLANK_HZ)
                    break;
                SDL_Delay(1u);
            }
        } while (1);
        if (g_m11_debug_startup_detail)
            fprintf(stderr,
                    "firestaff: startup-source-complete game=dm1 platform=pc34 phase=anim-title-erase pixels=%u frames=%u elapsed-ms=%llu modeled-vblank-hz=%u\n",
                    (unsigned)erase.pixels_cleared, eraseFrames,
                    (unsigned long long)(m11_intro_active_ticks() -
                                         eraseStartedMs),
                    DM1_V1_VGA_VBLANK_HZ);
    }
    memset(palette, 0, sizeof(palette));
    memset(framebuffer, 0, (size_t)M11_FB_BYTES);
    if (M11_Render_SetIndexedPaletteRgb6(palette) != M11_RENDER_OK ||
        M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                  M11_FB_HEIGHT) != M11_RENDER_OK) {
        free(data); return 0;
    }
    startMs = m11_intro_active_ticks();
    for (;;) {
        const uint64_t frameStartedMs = m11_intro_active_ticks();
        uint64_t frameDeadlineMs;
        SDL_Event event;
        int abortAnimation = 0;
        unsigned i;
        step = dm1_v1_i34e_title_anim_pc34_step(&player, &frame);
        if (step != 1) break;
        frameDeadlineMs = dm1_v1_i34e_title_pc34_deadline_ms(
            pitEpochMs, frameStartedMs, frame.source_counter_units);
        if (frameDeadlineMs == UINT64_MAX) { step = -1; break; }
        for (i = 0u; i < 16u; ++i) {
            palette[i][0] = frame.palette[i].r;
            palette[i][1] = frame.palette[i].g;
            palette[i][2] = frame.palette[i].b;
        }
        memcpy(framebuffer, frame.pixels, (size_t)M11_FB_BYTES);
        if (M11_Render_SetIndexedPaletteRgb6(palette) != M11_RENDER_OK ||
            M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                      M11_FB_HEIGHT) != M11_RENDER_OK) {
            step = -1; break;
        }
        ++frames;
        counterUnits += frame.source_counter_units;
        if (frames == 1u)
            m11_capture_dm1_authenticated_startup_frame("i34e-title-first");
        else if (frames == 27u)
            m11_capture_dm1_authenticated_startup_frame("i34e-title-mid");
        else if (frames == 53u)
            m11_capture_dm1_authenticated_startup_frame("i34e-title-final");
        if (!autotestKeyInjected && getenv("FIRESTAFF_AUTOTEST") &&
            getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_KEY_AFTER_FRAME") &&
            strcmp(getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_KEY_AFTER_FRAME"),
                   "1") == 0 && frames == 1u) {
            memset(&event, 0, sizeof(event));
#if SDL_VERSION_ATLEAST(3, 0, 0)
            event.type = SDL_EVENT_KEY_DOWN;
            event.key.key = SDLK_RIGHT;
            event.key.scancode = SDL_SCANCODE_RIGHT;
            event.key.windowID = SDL_GetWindowID(M11_Render_GetWindow());
            event.key.timestamp = SDL_GetTicks();
            event.key.repeat = 0;
#else
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = SDLK_RIGHT;
            event.key.keysym.scancode = SDL_SCANCODE_RIGHT;
            event.key.windowID = SDL_GetWindowID(M11_Render_GetWindow());
            event.key.timestamp = SDL_GetTicks();
            event.key.repeat = 0;
#endif
            if (SDL_PushEvent(&event)) {
                autotestKeyInjected = 1;
                fprintf(stderr,
                        "AUTOTEST: DM1 I34E title key injected after first rendered frame\n");
            }
        }
        if (!autotestQuitInjected && getenv("FIRESTAFF_AUTOTEST") &&
            getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_QUIT_AFTER_FRAME") &&
            strcmp(getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_QUIT_AFTER_FRAME"),
                   "1") == 0) {
            memset(&event, 0, sizeof(event));
#if SDL_VERSION_ATLEAST(3, 0, 0)
            event.type = SDL_EVENT_QUIT;
#else
            event.type = SDL_QUIT;
#endif
            if (SDL_PushEvent(&event)) {
                autotestQuitInjected = 1;
                fprintf(stderr,
                        "AUTOTEST: DM1 I34E title quit injected after first rendered frame\n");
            }
        }
        if (!autotestMouseInjected && getenv("FIRESTAFF_AUTOTEST") &&
            getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_MOUSE_AFTER_FRAME") &&
            strcmp(getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_MOUSE_AFTER_FRAME"),
                   "1") == 0 && frames == 1u) {
            memset(&event, 0, sizeof(event));
#if SDL_VERSION_ATLEAST(3, 0, 0)
            event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
            event.button.button = SDL_BUTTON_LEFT;
            event.button.x = 100.0f;
            event.button.y = 100.0f;
            event.button.windowID = SDL_GetWindowID(M11_Render_GetWindow());
            event.button.timestamp = SDL_GetTicks();
#else
            event.type = SDL_MOUSEBUTTONDOWN;
            event.button.button = SDL_BUTTON_LEFT;
            event.button.x = 100;
            event.button.y = 100;
            event.button.windowID = SDL_GetWindowID(M11_Render_GetWindow());
            event.button.timestamp = SDL_GetTicks();
#endif
            if (SDL_PushEvent(&event)) {
                autotestMouseInjected = 1;
                fprintf(stderr,
                        "AUTOTEST: DM1 I34E title mouse click injected after first rendered frame\n");
            }
        }
        for (;;) {
            if (m11_phase_duration_expired() || m11_intro_wait_for_focus()) {
                step = M11_ENTRANCE_COMMAND_QUIT; break;
            }
            while (SDL_PollEvent(&event)) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
                if (event.type == SDL_EVENT_QUIT) {
#else
                if (event.type == SDL_QUIT) {
#endif
                    g_m11_startup_intro_quit_requested = 1;
                    step = M11_ENTRANCE_COMMAND_QUIT;
                    break;
                }
#if SDL_VERSION_ATLEAST(3, 0, 0)
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (frame.break_allowed) {
                        abortAnimation = 1;
                        userAbortEventType = event.type;
                        userAbortKey = event.key.key;
                        userAbortScancode = event.key.scancode;
                        userAbortWindowId = event.key.windowID;
                        userAbortTimestamp = event.key.timestamp;
                        userAbortRepeat = event.key.repeat;
                        break;
                    }
                    if (g_m11_debug_startup_detail)
                        fprintf(stderr,
                                "firestaff: startup-input-ignore game=dm1 phase=anim-title reason=source-break-disabled event=%u window=%u timestamp=%llu key=%d scancode=%d repeat=%u frame=%u\n",
                                (unsigned)event.type,
                                (unsigned)event.key.windowID,
                                (unsigned long long)event.key.timestamp,
                                (int)event.key.key,
                                (int)event.key.scancode,
                                (unsigned)event.key.repeat, frames);
                    continue;
                }
                /* ReDMCSB ANIM.C:2113-2114 checks F0539 only; IO2.C:179-185
                 * binds it to keyboard input. A mouse click cannot end TITLE. */
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    if (g_m11_debug_startup_detail)
                        fprintf(stderr,
                                "firestaff: startup-input-ignore game=dm1 phase=anim-title reason=source-keyboard-only button=%u break-allowed=%u frame=%u\n",
                                (unsigned)event.button.button,
                                (unsigned)frame.break_allowed, frames);
                    continue;
                }
#else
                if (event.type == SDL_KEYDOWN) {
                    if (frame.break_allowed) {
                        abortAnimation = 1;
                        userAbortEventType = event.type;
                        userAbortKey = event.key.keysym.sym;
                        userAbortScancode = event.key.keysym.scancode;
                        userAbortWindowId = event.key.windowID;
                        userAbortTimestamp = event.key.timestamp;
                        userAbortRepeat = event.key.repeat;
                        break;
                    }
                    if (g_m11_debug_startup_detail)
                        fprintf(stderr,
                                "firestaff: startup-input-ignore game=dm1 phase=anim-title reason=source-break-disabled event=%u window=%u timestamp=%llu key=%d scancode=%d repeat=%u frame=%u\n",
                                (unsigned)event.type,
                                (unsigned)event.key.windowID,
                                (unsigned long long)event.key.timestamp,
                                (int)event.key.keysym.sym,
                                (int)event.key.keysym.scancode,
                                (unsigned)event.key.repeat, frames);
                    continue;
                }
                /* ReDMCSB ANIM.C:2113-2114 checks F0539 only; IO2.C:179-185
                 * binds it to keyboard input. A mouse click cannot end TITLE. */
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (g_m11_debug_startup_detail)
                        fprintf(stderr,
                                "firestaff: startup-input-ignore game=dm1 phase=anim-title reason=source-keyboard-only button=%u break-allowed=%u frame=%u\n",
                                (unsigned)event.button.button,
                                (unsigned)frame.break_allowed, frames);
                    continue;
                }
#endif
#if SDL_VERSION_ATLEAST(3, 0, 0)
                if (event.type == SDL_EVENT_WINDOW_RESIZED)
                    M11_Render_HandleResize(event.window.data1, event.window.data2);
#else
                if (event.type == SDL_WINDOWEVENT &&
                    event.window.event == SDL_WINDOWEVENT_RESIZED)
                    M11_Render_HandleResize(event.window.data1, event.window.data2);
#endif
            }
            if (step == M11_ENTRANCE_COMMAND_QUIT || abortAnimation) break;
            if (m11_intro_active_ticks() >= frameDeadlineMs)
                break;
            SDL_Delay(1u);
        }
        if (step == M11_ENTRANCE_COMMAND_QUIT) break;
        if (abortAnimation) { userAborted = 1; break; }
    }
    if (step == M11_ENTRANCE_COMMAND_QUIT) {
        free(data);
        return M11_ENTRANCE_COMMAND_QUIT;
    }
    animationEndMs = m11_intro_active_ticks();
    if (step < 0 || (!player.finished && !userAborted) || frames == 0u) {
        free(data);
        return 0;
    }
    if (userAborted && g_m11_debug_startup_detail)
        fprintf(stderr,
                "firestaff: startup-source-abort game=dm1 platform=pc34 phase=anim-title event=%u window=%u timestamp=%llu key=%d scancode=%d repeat=%u frame=%u source-counter-units=%llu\n",
                (unsigned)userAbortEventType, (unsigned)userAbortWindowId,
                (unsigned long long)userAbortTimestamp, (int)userAbortKey,
                (int)userAbortScancode, (unsigned)userAbortRepeat, frames,
                (unsigned long long)counterUnits);
    if (g_m11_debug_startup_detail)
        fprintf(stderr, "firestaff: startup-source-complete game=dm1 platform=pc34 phase=anim-title frames=%u source-counter-units=%llu elapsed-ms=%llu timing-model=ibmio-default-pit callback-decrement=3 pit-input-hz=1193182 pit-divisor=65536\n",
                frames, (unsigned long long)counterUnits,
                (unsigned long long)(animationEndMs - startMs));
    /* IBMIO.C:2413 invokes `anim title -ah -as`, not -ae. ANIM.C
     * main:1880-1898,1974-1976,2127-2135 erases before playback and retains
     * the final image/palette afterwards. The next source phase owns the
     * next screen change; do not synthesize a terminal fade or wipe. */
    m11_capture_dm1_authenticated_startup_frame("i34e-title-held");
    if (g_m11_debug_startup_detail)
        fprintf(stderr, "firestaff: startup-source-held game=dm1 platform=pc34 phase=anim-title flags=-ah,-as terminal-erase=0\n");
    free(data);
    return 1;
}

/* I34E's SELECTOR is the source-owned screen immediately following ANIM
 * TITLE. A direct `--game dm1` launch preserves its Enter, Resume and Quit
 * choices. Reconstruct the selector from authenticated SELECTOR + GRAPHICS.DAT
 * media; the M12 interactive launcher keeps its own input/menu lifecycle.
 * SELECTOR.C F8360 and VIDEODRV.C G8148 own the pixels, frame cadence and
 * palette. */
static int m11_show_dm1_i34e_selector_credits(
    M11_AssetLoader *loader,
    const unsigned char *closedFrame,
    const unsigned char entrancePalette[256][3]) {
    const M11_AssetSlot *credits;
    unsigned char creditsPalette[256][3];
    if (!loader || !closedFrame || !entrancePalette ||
        !(credits = M11_AssetLoader_Load(loader, 5u)) ||
        !credits->pixels || credits->width != 320u ||
        credits->height != 200u) {
        return 0;
    }
    memset(creditsPalette, 0, sizeof(creditsPalette));
    memcpy(creditsPalette,
           G9013_auc_VgaPaletteSpecial_Compat
               [VGA_PALETTE_PC34_SPECIAL_CREDITS],
           16u * 3u);
    if (M11_Render_SetIndexedPaletteRgb8(creditsPalette) != M11_RENDER_OK ||
        M11_Render_PresentIndexed(credits->pixels, credits->width,
                                  credits->height) != M11_RENDER_OK) {
        return 0;
    }
    m11_capture_dm1_authenticated_startup_frame("i34e-selector-credits");
    for (;;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (m11_event_requests_window_close(&event)) {
                g_m11_startup_intro_quit_requested = 1;
                return M11_ENTRANCE_COMMAND_QUIT;
            }
#if SDL_VERSION_ATLEAST(3, 0, 0)
            if ((event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) ||
                event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
#else
            if ((event.type == SDL_KEYDOWN && !event.key.repeat) ||
                event.type == SDL_MOUSEBUTTONDOWN) {
#endif
                if (M11_Render_SetIndexedPaletteRgb8(entrancePalette) !=
                        M11_RENDER_OK ||
                    M11_Render_PresentIndexed(closedFrame, M11_FB_WIDTH,
                                               M11_FB_HEIGHT) !=
                        M11_RENDER_OK) {
                    return 0;
                }
                m11_capture_dm1_authenticated_startup_frame(
                    "i34e-selector-credits-return");
                return 1;
            }
        }
        if (m11_phase_duration_expired() || m11_intro_wait_for_focus()) {
            return M11_ENTRANCE_COMMAND_QUIT;
        }
        SDL_Delay(1u);
    }
}

static int m11_play_dm1_i34e_selector_enter_if_available(
    M11_GameViewState *gameView,
    M12_StartupMenuState *menuState,
    const M12_AssetVersionStatus *version) {
    static const char selectorMd5[] = "165ae5438ce2686ae62f2fbb61f3e831";
    M11_AssetLoader loader;
    uint8_t *graphics = NULL, *selector = NULL;
    size_t graphicsBytes = 0u, selectorBytes = 0u;
    char graphicsPath[FSP_PATH_MAX];
    char selectorPath[FSP_PATH_MAX], md5[33];
    const char *memberSeparator = NULL;
    const char *scan;
    int virtualPath;
    const M11_AssetSlot *entrance, *left, *right;
    unsigned char *view = NULL;
    unsigned int viewBytes = 0u, step, steps;
    unsigned char frame[M11_FB_BYTES];
    unsigned char palette[256][3];
    unsigned int width;
    unsigned int doorSoundTimer = 21u;
    unsigned int doorSoundCount = 0u;
    int switchSoundQueued = 0;
    int configuredVsync = M11_VSYNC_OFF;
    int selectorVsyncOverride = 0;
    uint64_t vblankStartMs;
    uint64_t selectorStartMs;
    uint64_t doorAnimationEndMs = 0u;
    uint64_t selectorCompleteMs = 0u;
    uint64_t previousDoorPresentMs = 0u;
    int result = 0;

    memset(&loader, 0, sizeof(loader));
    if (!gameView || !menuState || !version || !version->matched ||
        !version->versionId || strcmp(version->versionId, "pc34-en") != 0 ||
        !version->matchedPath[0] ||
        strlen(version->matchedPath) >= sizeof(graphicsPath)) return 0;
    /* The admitted graphics member identifies its installation, including
     * archive case, nested transports and any wrapper directory. Keep every
     * outer locator; only the last member supplies the companion directory. */
    strcpy(graphicsPath, version->matchedPath);
    for (scan = graphicsPath; (scan = strstr(scan, "::")) != NULL; scan += 2)
        memberSeparator = scan;
    virtualPath = memberSeparator != NULL;
    if (virtualPath) {
        size_t memberOffset = (size_t)(memberSeparator - graphicsPath) + 2u;
        char *member;
        char *slash;
        char *directory;
        size_t prefixBytes;
        strcpy(selectorPath, graphicsPath);
        member = selectorPath + memberOffset;
        slash = strrchr(member, '/');
        if (!m11_ascii_path_equal_ignore_case(slash ? slash + 1 : member,
                                                "GRAPHICS.DAT")) goto done;
        if (slash) {
            *slash = '\0';
            directory = strrchr(member, '/');
            directory = directory ? directory + 1 : member;
            /* SELECTOR.C lives beside DATA, not inside it. */
            if (m11_ascii_path_equal_ignore_case(directory, "DATA")) {
                prefixBytes = (size_t)(directory - selectorPath);
            } else {
                *slash = '/';
                prefixBytes = (size_t)(slash - selectorPath) + 1u;
            }
        } else {
            prefixBytes = memberOffset;
        }
        if (prefixBytes + sizeof("SELECTOR") > sizeof(selectorPath)) goto done;
        memcpy(selectorPath + prefixBytes, "SELECTOR", sizeof("SELECTOR"));
    } else if (!m11_dm1_i34e_loose_companion_path(
                   version, "SELECTOR", selectorPath, sizeof(selectorPath))) {
        goto done;
    }

    if (!asset_file_md5_hex(selectorPath, md5) || strcmp(md5, selectorMd5) != 0 ||
        !(virtualPath ? asset_read_virtual_path_alloc(graphicsPath, &graphics,
                                                  &graphicsBytes)
                 : asset_read_path_alloc(graphicsPath, &graphics,
                                          &graphicsBytes)) ||
        !(virtualPath ? asset_read_virtual_path_alloc(selectorPath, &selector,
                                                  &selectorBytes)
                 : asset_read_path_alloc(selectorPath, &selector,
                                          &selectorBytes)) ||
        !M11_AssetLoader_InitFromBuffer(&loader, graphics,
                                        (long)graphicsBytes) ||
        !DM1_V1_Selector_DecodeDungeonViewPc34(
            selector, (unsigned int)selectorBytes, &view, &viewBytes) ||
        viewBytes != 224u * 136u ||
        !(entrance = M11_AssetLoader_Load(&loader, 4u)) ||
        !(left = M11_AssetLoader_Load(&loader, 2u)) ||
        !(right = M11_AssetLoader_Load(&loader, 3u)) ||
        entrance->width != 320u || entrance->height != 200u ||
        left->height < 161u || right->height < 161u ||
        right->width == 0u || right->width > 256u) goto done;

    /* SELECTOR.C loads SND3 items 672/673 from this same GRAPHICS.DAT, then
     * plays event 1 (switch) and event 2 (door rattle). Bind the selected
     * installation here so an unrelated default user's bank cannot leak in. */
    if (gameView->audioState.initialized) {
        (void)M11_ApplyIntroAudioPreferences(&gameView->audioState,
                                              menuState);
        (void)M11_Audio_BindOriginalSnd3Path(&gameView->audioState,
                                             graphicsPath);
    }

    /* SELECTOR.C waits for VGA vertical retrace before every door step. Keep
     * that source behavior even when the user's general runtime VSync option
     * is disabled, then restore the user's setting on every exit path. */
    configuredVsync = M11_Render_GetVSync();
    if (configuredVsync != M11_VSYNC_ON) {
        (void)M11_Render_SetVSync(M11_VSYNC_ON);
        selectorVsyncOverride = 1;
    }

    memset(palette, 0, sizeof(palette));
    for (step = 0u; step < 16u; ++step) {
        memcpy(palette[step],
               G9013_auc_VgaPaletteSpecial_Compat
                   [VGA_PALETTE_PC34_SPECIAL_ENTRANCE][step], 3u);
    }
    if (!DM1_V1_Selector_ComposeFramePc34(
            entrance->pixels, entrance->width, entrance->height,
            left->pixels, left->width, left->height,
            right->pixels, right->width, right->height,
            view, 224u, 136u, 0u, frame, sizeof(frame)) ||
        M11_Render_SetIndexedPaletteRgb8(palette) != M11_RENDER_OK ||
        M11_Render_PresentIndexed(frame, M11_FB_WIDTH, M11_FB_HEIGHT) !=
            M11_RENDER_OK) goto done;
    m11_capture_dm1_authenticated_startup_frame("i34e-selector-closed");
    /* The original I34E SELECTOR waits for a user command after drawing the
     * closed entrance. `--game dm1` selects the edition, not SELECTOR's Enter
     * button. Keep automated boot receipts non-interactive; normal direct
     * launches wait for Enter or a click on the source Enter button. */
    if (g_m11_dm1_direct_cli_launch &&
        !getenv("FIRESTAFF_EXIT_AFTER_LAUNCH") &&
        !getenv("FIRESTAFF_AUTOTEST")) {
        int resumeSelected = 0;
        if (g_m11_debug_startup_detail)
            fprintf(stderr,
                    "firestaff: startup-source-wait game=dm1 platform=pc34 phase=selector action=await-enter-or-click\n");
        if (getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT") &&
            strcmp(getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT"),
                   "key:return-repeat") == 0) {
            SDL_Event repeatedReturn;
            memset(&repeatedReturn, 0, sizeof(repeatedReturn));
#if SDL_VERSION_ATLEAST(3, 0, 0)
            repeatedReturn.type = SDL_EVENT_KEY_DOWN;
            repeatedReturn.key.key = SDLK_RETURN;
#else
            repeatedReturn.type = SDL_KEYDOWN;
            repeatedReturn.key.keysym.sym = SDLK_RETURN;
#endif
            repeatedReturn.key.repeat = 1;
            (void)SDL_PushEvent(&repeatedReturn);
        } else if (getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT") &&
                   strcmp(getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT"),
                          "key:return") == 0) {
            static const char freshReturn[] = "key:return";
            (void)m11_push_script_event_token(
                freshReturn, sizeof(freshReturn) - 1u);
        } else if (getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT") &&
                   strcmp(getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT"),
                          "key:r") == 0) {
            static const char freshResume[] = "key:r";
            (void)m11_push_script_event_token(
                freshResume, sizeof(freshResume) - 1u);
        } else if (getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT") &&
                   (strcmp(getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT"),
                           "key:q") == 0 ||
                    strcmp(getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT"),
                           "key:escape") == 0)) {
            const char *quitKey = getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT");
            (void)m11_push_script_event_token(quitKey, strlen(quitKey));
        } else if (getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT") &&
                   strcmp(getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT"),
                          "click:quit") == 0) {
            /* ReDMCSB SELECTOR.C places Quit at source (243,111); the dummy
             * 960x540 window maps the center of that 14x13 box to 723,316. */
            static const char quitClick[] = "click:723:316";
            (void)m11_push_script_event_token(
                quitClick, sizeof(quitClick) - 1u);
        } else if (getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT") &&
                   strcmp(getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT"),
                          "click:resume") == 0) {
            /* ReDMCSB SELECTOR.C:960 places the 14x13 Resume hitbox at
             * source (243,78); the dummy 960x540 window maps its center to
             * window (723,227). */
            static const char resumeClick[] = "click:723:227";
            (void)m11_push_script_event_token(
                resumeClick, sizeof(resumeClick) - 1u);
        } else if (getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT") &&
                   strcmp(getenv("FIRESTAFF_AUTOTEST_SELECTOR_INPUT"),
                          "credits-then-enter") == 0) {
            /* The dummy test window is 960x540 with a 2.7x 320x200 surface
             * centered at x=48, so this lands inside the source CREDITS box. */
            static const char creditsClick[] = "click:720:510";
            static const char creditsDismiss[] = "key:return";
            static const char enterAfterCredits[] = "key:return";
            (void)m11_push_script_event_token(
                creditsClick, sizeof(creditsClick) - 1u);
            (void)m11_push_script_event_token(
                creditsDismiss, sizeof(creditsDismiss) - 1u);
            (void)m11_push_script_event_token(
                enterAfterCredits, sizeof(enterAfterCredits) - 1u);
        }
        for (;;) {
            SDL_Event event;
            int accepted = 0;
            while (SDL_PollEvent(&event)) {
                int keyDown = 0;
                int enterClick = 0;
                int creditsClick = 0;
                if (m11_event_requests_window_close(&event)) {
                    g_m11_startup_intro_quit_requested = 1;
                    result = M11_ENTRANCE_COMMAND_QUIT;
                    goto done;
                }
#if SDL_VERSION_ATLEAST(3, 0, 0)
                if (event.type == SDL_EVENT_KEY_DOWN) {
#else
                if (event.type == SDL_KEYDOWN) {
#endif
                    keyDown = 1;
                }
                if (keyDown && (event.key.key == SDLK_RETURN ||
                                event.key.key == SDLK_KP_ENTER ||
                                event.key.key == SDLK_E)) {
                    if (event.key.repeat) {
                        if (g_m11_debug_startup_detail) {
                            fprintf(stderr,
                                    "firestaff: startup-input-ignore game=dm1 phase=selector reason=key-repeat key=%d\n",
                                    (int)event.key.key);
                        }
                        continue;
                    }
                    accepted = 1;
                    break;
                }
                if (keyDown && (event.key.key == SDLK_ESCAPE ||
                                event.key.key == SDLK_Q)) {
                    if (g_m11_debug_startup_detail) {
                        fprintf(stderr,
                                "firestaff: startup-source-input game=dm1 platform=pc34 phase=selector command=quit source=keyboard key=%d\n",
                                (int)event.key.key);
                    }
                    g_m11_startup_intro_quit_requested = 1;
                    result = M11_ENTRANCE_COMMAND_QUIT;
                    goto done;
                }
                if (keyDown && event.key.key == SDLK_R) {
                    if (menuState->quickResumeAvailable &&
                        strcmp(menuState->quickResumeGameId, "dm1") == 0 &&
                        menuState->quickResumeSavePath[0] != '\0') {
                        resumeSelected = 1;
                        accepted = 1;
                        break;
                    }
                    if (g_m11_debug_startup_detail) {
                        fprintf(stderr,
                                "firestaff: startup-input-ignore game=dm1 phase=selector reason=resume-unavailable\n");
                    }
                    continue;
                }
#if SDL_VERSION_ATLEAST(3, 0, 0)
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
#else
                if (event.type == SDL_MOUSEBUTTONDOWN &&
#endif
                    event.button.button == SDL_BUTTON_LEFT) {
                    int sourceX = -1, sourceY = -1;
                    if (M11_Render_MapWindowToFramebuffer(
                            (int)event.button.x, (int)event.button.y,
                            &sourceX, &sourceY) &&
                        sourceX >= 243 && sourceX < 257 &&
                        sourceY >= 46 && sourceY < 59) {
                        enterClick = 1;
                    } else if (sourceX >= 243 && sourceX < 257 &&
                               sourceY >= 78 && sourceY < 91 &&
                               menuState->quickResumeAvailable &&
                               strcmp(menuState->quickResumeGameId, "dm1") == 0 &&
                               menuState->quickResumeSavePath[0] != '\0') {
                        resumeSelected = 1;
                        accepted = 1;
                        break;
                    } else if (sourceX >= 243 && sourceX < 257 &&
                               sourceY >= 111 && sourceY < 124) {
                        if (g_m11_debug_startup_detail) {
                            fprintf(stderr,
                                    "firestaff: startup-source-input game=dm1 platform=pc34 phase=selector command=quit source=hitbox\n");
                        }
                        g_m11_startup_intro_quit_requested = 1;
                        result = M11_ENTRANCE_COMMAND_QUIT;
                        goto done;
                    } else if (sourceX >= 248 && sourceX < 294 &&
                               sourceY >= 186 && sourceY < 200) {
                        creditsClick = 1;
                    }
                }
                if (creditsClick) {
                    int creditsResult = m11_show_dm1_i34e_selector_credits(
                        &loader, frame, palette);
                    if (creditsResult == M11_ENTRANCE_COMMAND_QUIT) {
                        result = M11_ENTRANCE_COMMAND_QUIT;
                        goto done;
                    }
                    if (g_m11_debug_startup_detail) {
                        fprintf(stderr,
                                "firestaff: startup-source-input game=dm1 platform=pc34 phase=selector command=credits-return\n");
                    }
                    continue;
                }
                if (enterClick) {
                    accepted = 1;
                    break;
                }
            }
            if (accepted) break;
            if (m11_phase_duration_expired() || m11_intro_wait_for_focus()) {
                result = M11_ENTRANCE_COMMAND_QUIT;
                goto done;
            }
            SDL_Delay(1u);
        }
        if (resumeSelected) {
            menuState->quickResumeLaunchRequested = 1;
            (void)M11_Audio_EmitSourceSoundIndex(
                &gameView->audioState, 1);
            while (!M11_Audio_SfxDrained(&gameView->audioState)) {
                if (M11_Render_PumpEvents()) {
                    g_m11_startup_intro_quit_requested = 1;
                    result = M11_ENTRANCE_COMMAND_QUIT;
                    goto done;
                }
                SDL_Delay(1u);
            }
            if (g_m11_debug_startup_detail) {
                fprintf(stderr,
                        "firestaff: startup-source-input game=dm1 platform=pc34 phase=selector command=resume save=%s\n",
                        menuState->quickResumeSavePath);
            }
            result = 1;
            goto done;
        }
        if (g_m11_debug_startup_detail)
            fprintf(stderr,
                    "firestaff: startup-source-input game=dm1 platform=pc34 phase=selector command=enter\n");
    }
    vblankStartMs = m11_intro_active_ticks();
    selectorStartMs = vblankStartMs;
    switchSoundQueued = M11_Audio_EmitSourceSoundIndex(
        &gameView->audioState, 1);
    if (g_m11_debug_startup_detail) {
#if SDL_VERSION_ATLEAST(3, 2, 0)
        SDL_DisplayID displayId = SDL_GetDisplayForWindow(
            M11_Render_GetWindow());
        const SDL_DisplayMode *displayMode = displayId
            ? SDL_GetCurrentDisplayMode(displayId) : NULL;
        int effectiveVsync = -1;
        const int hasEffectiveVsync = SDL_GetRenderVSync(
            M11_Render_GetRenderer(), &effectiveVsync);
        fprintf(stderr,
                "firestaff: startup-pacing game=dm1 platform=pc34 phase=selector-pacing configured-vsync=%d requested-vsync=%d effective-vsync-known=%d effective-vsync=%d display-refresh-hz=%.3f display-brightness-percent=%d\n",
                configuredVsync, M11_Render_GetVSync(),
                hasEffectiveVsync ? 1 : 0, effectiveVsync,
                displayMode ? (double)displayMode->refresh_rate : 0.0,
                M11_Render_GetGlobalBrightness());
#else
        fprintf(stderr,
                "firestaff: startup-pacing game=dm1 platform=pc34 phase=selector-pacing configured-vsync=%d requested-vsync=%d effective-vsync-known=0 effective-vsync=-1 display-refresh-hz=0.000 display-brightness-percent=%d\n",
                configuredVsync, M11_Render_GetVSync(),
                M11_Render_GetGlobalBrightness());
#endif
    }

    /* Direct CLI selection and M12 New Game both select the I34E FIRES row.
     * F8360 is called once per source VBlank, with G8248=4; preserve its
     * initial step zero and every subsequent image. */
    width = right->width;
    steps = (width + 3u) / 4u;
    for (step = 0u; step < steps; ++step) {
        SDL_Event event;
        uint64_t presentStartedMs;
        uint64_t presentedAtMs;
        if (!DM1_V1_Selector_ComposeFramePc34(
                entrance->pixels, entrance->width, entrance->height,
                left->pixels, left->width, left->height,
                right->pixels, right->width, right->height,
                view, 224u, 136u, step, frame, sizeof(frame))) goto done;
        presentStartedMs = m11_intro_active_ticks();
        if (M11_Render_PresentIndexed(frame, M11_FB_WIDTH, M11_FB_HEIGHT) !=
            M11_RENDER_OK) goto done;
        presentedAtMs = m11_intro_active_ticks();
        if (g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "firestaff: startup-source-frame game=dm1 platform=pc34 phase=selector-door frame=%u/%u elapsed-ms=%llu interval-ms=%llu present-ms=%llu vsync=%d source-step-px=4\n",
                    step + 1u, steps,
                    (unsigned long long)(presentedAtMs - selectorStartMs),
                    (unsigned long long)(previousDoorPresentMs
                        ? presentedAtMs - previousDoorPresentMs
                        : presentedAtMs - selectorStartMs),
                    (unsigned long long)(presentedAtMs - presentStartedMs),
                    M11_Render_GetVSync());
        }
        previousDoorPresentMs = presentedAtMs;
        if (step == 0u)
            m11_capture_dm1_authenticated_startup_frame(
                "i34e-selector-door-step-1");
        else if (step + 1u == steps / 2u)
            m11_capture_dm1_authenticated_startup_frame(
                "i34e-selector-door-mid");
        else if (step + 1u == steps)
            m11_capture_dm1_authenticated_startup_frame(
                "i34e-selector-door-open");
        while (SDL_PollEvent(&event)) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
            if (event.type == SDL_EVENT_QUIT) {
#else
            if (event.type == SDL_QUIT) {
#endif
                g_m11_startup_intro_quit_requested = 1;
                result = M11_ENTRANCE_COMMAND_QUIT;
                goto done;
            }
        }
        if (m11_phase_duration_expired() || m11_intro_wait_for_focus()) {
            result = M11_ENTRANCE_COMMAND_QUIT;
            goto done;
        }
        /* SELECTOR.C F8353 -> VIDEODRV.C F8153 polls VGA status port 0x3DA
         * until the next vertical retrace. Model that retrace with Firestaff's
         * shared 70 Hz VGA clock (DM1_V1_VGA_VBLANK_HZ), using a rational
         * deadline so 32 frames do not drift from rounding to 14 ms each.
         * This path must not use ANIM/TITLE's 18.2 Hz DOS-tick wait. */
        {
            const uint64_t targetMs =
                ((uint64_t)(step + 1u) * 1000u +
                 DM1_V1_VGA_VBLANK_HZ - 1u) /
                DM1_V1_VGA_VBLANK_HZ;
            while (m11_intro_active_ticks() - vblankStartMs < targetMs) {
                SDL_Delay(1u);
            }
        }
        /* SELECTOR.C starts G8232 at 21 and its video interrupt advances the
         * counter once per retrace. The I34E door sample is retriggered each
         * time it passes 20; keep that source cadence on the 70 Hz clock. */
        ++doorSoundTimer;
        if (doorSoundTimer > 20u) {
            doorSoundTimer = 0u;
            if (M11_Audio_EmitSourceSoundIndex(&gameView->audioState, 2))
                ++doorSoundCount;
        }
    }
    doorAnimationEndMs = m11_intro_active_ticks();
    while (!M11_Audio_SfxDrained(&gameView->audioState)) {
        SDL_Event event;
        if (M11_Render_PumpEvents()) {
            g_m11_startup_intro_quit_requested = 1;
            result = M11_ENTRANCE_COMMAND_QUIT;
            goto done;
        }
        while (SDL_PollEvent(&event)) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
            if (event.type == SDL_EVENT_QUIT) {
#else
            if (event.type == SDL_QUIT) {
#endif
                g_m11_startup_intro_quit_requested = 1;
                result = M11_ENTRANCE_COMMAND_QUIT;
                goto done;
            }
        }
        if (m11_phase_duration_expired() || m11_intro_wait_for_focus()) {
            result = M11_ENTRANCE_COMMAND_QUIT;
            goto done;
        }
        SDL_Delay(1u);
    }
    selectorCompleteMs = m11_intro_active_ticks();
    result = M11_ENTRANCE_COMMAND_ENTER;
    if (g_m11_debug_startup_detail)
        fprintf(stderr, "firestaff: startup-source-complete game=dm1 platform=pc34 phase=selector graphics=%s entrance=%ux%u left-door=%ux%u right-door=%ux%u selector-view=%ux%u frames=%u source-vblanks=%u elapsed-ms=%llu door-animation-ms=%llu post-animation-audio-wait-ms=%llu modeled-vblank-hz=%u switch-event-queued=%d door-events-queued=%u snd3-events-loaded=%d handoff=fires command=enter\n",
                graphicsPath, (unsigned)entrance->width,
                (unsigned)entrance->height, (unsigned)left->width,
                (unsigned)left->height, (unsigned)right->width,
                (unsigned)right->height, 224u, 136u, steps, steps,
                (unsigned long long)(selectorCompleteMs - selectorStartMs),
                (unsigned long long)(doorAnimationEndMs - selectorStartMs),
                (unsigned long long)(selectorCompleteMs - doorAnimationEndMs),
                DM1_V1_VGA_VBLANK_HZ, switchSoundQueued,
                doorSoundCount, gameView->audioState.originalSnd3LoadedCount);
done:
    if (selectorVsyncOverride) {
        (void)M11_Render_SetVSync(configuredVsync);
        if (g_m11_debug_startup_detail) {
            int effectiveVsync = -1;
            int effectiveVsyncKnown = 0;
#if SDL_VERSION_ATLEAST(3, 2, 0)
            effectiveVsyncKnown = SDL_GetRenderVSync(
                M11_Render_GetRenderer(), &effectiveVsync) ? 1 : 0;
#endif
            fprintf(stderr,
                    "firestaff: startup-pacing game=dm1 platform=pc34 phase=selector-vsync-restore configured-vsync=%d requested-vsync=%d effective-vsync-known=%d effective-vsync=%d\n",
                    configuredVsync, M11_Render_GetVSync(),
                    effectiveVsyncKnown, effectiveVsync);
        }
    }
    free(view);
    free(selector);
    free(graphics);
    M11_AssetLoader_Shutdown(&loader);
    if (result == 0 && g_m11_debug_startup_detail)
        fprintf(stderr, "firestaff: startup-source-skip game=dm1 platform=pc34 phase=selector reason=missing-or-unverified-source-media\n");
    return result;
}

static int m11_play_redmcsb_title_intro_if_available(const M12_StartupMenuState* menuState,
                                                     M11_GameViewState* gameView,
                                                     const char* sourceId,
                                                     int* outCompleted,
                                                     const DM1_V1_StartupFullGraphicsMediaReceipt_PC34*
                                                         dm1MediaReceipt) {
    if (outCompleted) {
        *outCompleted = 0;
    }
    /* ReDMCSB TITLE.C F0437 PC/F20 loads GRAPHICS.DAT C001 at lines 309-310
     * and builds PRESENTS/MASTER from that bitmap through line 409. The loose
     * TITLE animation is separate provenance and cannot replace those source
     * blits, even when its canonical file is installed. */
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(sourceId)) {
        return 0;
    }
    return m11_play_redmcsb_title_graphic_intro_if_available(
        menuState, gameView, sourceId, outCompleted, dm1MediaReceipt);
}

typedef struct M11_DM1StartupHandoffContext {
    M12_StartupMenuState* menuState;
    const M12_MenuEntry* launchEntry;
    M11_GameViewState* gameView;
    uint32_t* idleAccumulatorMs;
    const char* dataDir;
    int bootProbe;
    DM1_V1_StartupHandoffPreludePlan_PC34 activePreludePlan;
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 activePostLaunchPlan;
    int activePreludePlanValid;
    int activePostLaunchPlanValid;
    int titleQuitRequested;
    int startupQuitRequested;
} M11_DM1StartupHandoffContext;

static int m11_dm1_handoff_report_source_order_failure(void* user,
                                                       const char* evidence) {
    (void)user;
    fprintf(stderr,
            "DM1 startup source-order guard failed: %s\n",
            evidence ? evidence : "");
    return 1;
}

static int m11_dm1_handoff_raise_window(void* user) {
    (void)user;
    M11_Render_RaiseWindow();
    return 1;
}

static int m11_dm1_handoff_begin_prelude_plan(
    void* user,
    const DM1_V1_StartupHandoffPreludePlan_PC34* plan) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !plan) {
        return 0;
    }
    ctx->activePreludePlan = *plan;
    ctx->activePreludePlanValid = 1;
    return 1;
}

static int m11_dm1_handoff_end_prelude_plan(void* user) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx) {
        return 0;
    }
    memset(&ctx->activePreludePlan, 0, sizeof(ctx->activePreludePlan));
    ctx->activePreludePlanValid = 0;
    return 1;
}

static int m11_dm1_handoff_begin_post_launch_plan(
    void* user,
    const DM1_V1_StartupHandoffPostLaunchPlan_PC34* plan) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !plan) {
        return 0;
    }
    ctx->activePostLaunchPlan = *plan;
    ctx->activePostLaunchPlanValid = 1;
    return 1;
}

static int m11_dm1_handoff_end_post_launch_plan(void* user) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx) {
        return 0;
    }
    memset(&ctx->activePostLaunchPlan, 0, sizeof(ctx->activePostLaunchPlan));
    ctx->activePostLaunchPlanValid = 0;
    return 1;
}

static int m11_dm1_handoff_play_swsh(void* user,
                                     const char* game_id,
                                     int preserve_audio) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media = NULL;
    int amigaResult;
    (void)game_id;
    if (ctx && ctx->activePreludePlanValid &&
        ctx->activePreludePlan.media_receipt.handled) {
        media = &ctx->activePreludePlan.media_receipt;
    }
    amigaResult = m11_play_dm1_amiga_swsh_if_available(
        ctx ? ctx->menuState : NULL,
        ctx ? ctx->launchEntry : NULL,
        preserve_audio);
    if (amigaResult >= 0) {
        if (!amigaResult && ctx && g_m11_startup_intro_quit_requested)
            ctx->startupQuitRequested = 1;
        return amigaResult;
    }
    {
        int result = m11_play_ftl_swoosh_if_available(
            ctx ? ctx->menuState : NULL, ctx ? ctx->dataDir : NULL,
            preserve_audio, media);
        if (!result && ctx && g_m11_startup_intro_quit_requested)
            ctx->startupQuitRequested = 1;
        return result;
    }
}

static int m11_dm1_handoff_discard_presentation_texture(void* user) {
    (void)user;
    M11_Render_DiscardPresentationTexture();
    return 1;
}

static int m11_dm1_handoff_play_title(void* user,
                                      const char* source_id,
                                      int* out_played_any_frame) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media = NULL;
    int versionIndex;
    const M12_AssetVersionStatus* version;
    if (!ctx ||
        !dm1_v1_startup_source_visible_handoff_required_pc34(source_id)) {
        return 0;
    }
    versionIndex = m11_selected_dm1_launch_version_index(
        ctx->menuState, ctx->launchEntry);
    version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&ctx->menuState->assetStatus, "dm1",
                                     (size_t)versionIndex)
        : NULL;
    if (versionIndex >= 0 &&
        M12_AssetStatus_GetVersionArchitecture(
            "dm1", (size_t)versionIndex) == M12_ARCH_AMIGA &&
        version && version->versionId &&
        (strcmp(version->versionId, "amiga20-en") == 0 ||
         strcmp(version->versionId, "amiga20-fr") == 0 ||
         strcmp(version->versionId, "amiga20-de") == 0 ||
         strcmp(version->versionId, "amiga21-en") == 0 ||
         strcmp(version->versionId, "amiga-demo-en") == 0) &&
        strstr(version->matchedPath, "[HD]") == NULL) {
        int quitRequested = 0;
        const int played = m11_play_dm1_amiga_title_f0437_if_available(
            ctx->menuState, ctx->launchEntry, ctx->gameView,
            &quitRequested);
        if (quitRequested) ctx->titleQuitRequested = 1;
        if (!quitRequested && played <= 0 && g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "firestaff: startup-source-rejected game=dm1 phase=title-f0437 edition=%s assets=%d graphics=%s\n",
                    version->versionId,
                    ctx->gameView ? ctx->gameView->assetsAvailable : 0,
                    ctx->gameView
                        ? ctx->gameView->assetLoader.graphicsDatPath : "");
        }
        if (out_played_any_frame) *out_played_any_frame = played;
        return played;
    }
    if (ctx->activePostLaunchPlanValid &&
        ctx->activePostLaunchPlan.media_receipt.handled) {
        media = &ctx->activePostLaunchPlan.media_receipt;
    }
    return m11_play_redmcsb_title_intro_if_available(ctx->menuState,
                                                     ctx->gameView,
                                                     source_id,
                                                     out_played_any_frame,
                                                     media);
}

static int m11_dm1_handoff_play_entrance(void* user,
                                         const char* source_id,
                                         int auto_enter_after_ms,
                                         int* out_entrance_command) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    const DM1_V1_EntranceFullStartRenderReceiptPc34* entrance = NULL;
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media = NULL;
    DM1_V1_AmigaEntranceF0441Receipt amigaReceipt;
    int versionIndex;
    const M12_AssetVersionStatus* version;
    int selectedAmigaV2;
    int command;
    (void)source_id;
    if (!ctx || !ctx->gameView) {
        return 0;
    }
    versionIndex = m11_selected_dm1_launch_version_index(
        ctx->menuState, ctx->launchEntry);
    version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&ctx->menuState->assetStatus, "dm1",
                                     (size_t)versionIndex)
        : NULL;
    selectedAmigaV2 = version && version->versionId &&
        (strcmp(version->versionId, "amiga20-en") == 0 ||
         strcmp(version->versionId, "amiga20-fr") == 0 ||
         strcmp(version->versionId, "amiga20-de") == 0 ||
         strcmp(version->versionId, "amiga21-en") == 0 ||
         strcmp(version->versionId, "amiga-demo-en") == 0) &&
        M12_AssetStatus_GetVersionArchitecture(
            "dm1", (size_t)versionIndex) == M12_ARCH_AMIGA;
    if (!ctx->activePostLaunchPlanValid ||
        !ctx->activePostLaunchPlan.entrance_full_start_receipt.valid ||
        ctx->activePostLaunchPlan.entrance_auto_enter_ms != auto_enter_after_ms ||
        (selectedAmigaV2
             ? !ctx->activePostLaunchPlan.media_receipt.handled
             : !dm1_v1_startup_entrance_timing_receipt_valid_pc34(
                   &ctx->activePostLaunchPlan.media_receipt))) {
        return 0;
    }
    entrance = &ctx->activePostLaunchPlan.entrance_full_start_receipt;
    media = &ctx->activePostLaunchPlan.media_receipt;
    if (selectedAmigaV2) {
        if (!m11_dm1_amiga_entrance_f0441_receipt_if_selected(
                ctx->menuState, ctx->launchEntry, ctx->gameView,
                &amigaReceipt)) {
            return 0;
        }
        command = m11_play_dm1_amiga_entrance_f0441(
            ctx->gameView, auto_enter_after_ms, entrance, media,
            &amigaReceipt);
        if (g_m11_debug_startup_detail && command > 0) {
            fprintf(stderr,
                    "firestaff: startup-source-complete game=dm1 platform=%s phase=entrance-f0441 steps=%u graphics=%s input=mouse-only palette=rgb4\n",
                    strcmp(version->versionId, "amiga-demo-en") == 0
                        ? "amiga-demo"
                        : (strcmp(version->versionId, "amiga21-en") == 0
                               ? "amiga-v21" : "amiga-v20"),
                    amigaReceipt.opening_steps, amigaReceipt.graphics_md5);
        }
    } else {
        command = m11_play_redmcsb_entrance_transition(
            ctx->gameView, auto_enter_after_ms, entrance, media);
    }
    /* A CLI/start-menu resume is an explicit F0435 request.  The generic
     * auto-enter timing above only drives the visual ENTRANCE.C sequence;
     * it must not reinterpret an already selected original save as a new
     * Hall session.  ReDMCSB COMMAND.C M566 takes the RESUME branch from the
     * entrance and LOADSAVE.C F0435 owns the following world replacement.
     *
     * Keep the authored entrance presentation, but direct the post-entrance
     * source handoff to that requested RESUME action.  In particular, do not
     * load the save before the title/entrance callback and then apply an
     * empty-party HoC receipt over it. */
    if (command == ENTRANCE_COMPAT_COMMAND_PATH_ENTER &&
        ctx->menuState->quickResumeAvailable &&
        ctx->menuState->quickResumeLaunchRequested &&
        ctx->menuState->quickResumeSavePath[0] != '\0' &&
        strcmp(ctx->menuState->quickResumeGameId, "dm1") == 0) {
        command = ENTRANCE_COMPAT_COMMAND_PATH_RESUME;
    }
    if (out_entrance_command) {
        *out_entrance_command = command;
    }
    return 1;
}

static int m11_dm1_host_set_game_active(void* user, int active) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->gameView) {
        return 0;
    }
    ctx->gameView->active = active;
    return 1;
}

static int m11_dm1_host_resolve_resume_save_path(void* user,
                                                 const char* source_id,
                                                 char* out_path,
                                                 int out_path_size) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->menuState) {
        return 0;
    }
    return ENTRANCE_Compat_ResolveDm1ResumeSavePath(
        source_id,
        ctx->menuState->quickResumeAvailable,
        ctx->menuState->quickResumeGameId,
        ctx->menuState->quickResumeSavePath,
        out_path,
        (size_t)out_path_size);
}

static int m11_dm1_host_load_resume_save_path(void* user,
                                              const char* save_path,
                                              int* out_used_backup) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    int loaded;
    if (!ctx || !ctx->gameView) {
        return 0;
    }
    loaded = M11_GameView_LoadDm1SavePath(ctx->gameView,
                                           save_path,
                                           out_used_backup);
    if (!loaded && g_m11_debug_startup_detail) {
        fprintf(stderr,
                "firestaff: dm1-resume-rejected source=%s action=%s outcome=%s candidate-panel=%d path=%s\n",
                ctx->gameView->sourceId[0] ? ctx->gameView->sourceId : "(none)",
                ctx->gameView->lastAction[0] ? ctx->gameView->lastAction : "(none)",
                ctx->gameView->lastOutcome[0] ? ctx->gameView->lastOutcome : "(none)",
                ctx->gameView->candidateMirrorPanelActive,
                save_path ? save_path : "(none)");
    }
    return loaded;
}

static int m11_dm1_host_log_resume_loaded(void* user,
                                          const char* save_path,
                                          int used_backup) {
    (void)user;
    fprintf(stderr, "RESUME: loaded save from %s%s\n", save_path,
            used_backup ? " backup" : "");
    return 1;
}

static int m11_dm1_host_log_resume_missing(void* user, const char* save_path) {
    (void)user;
    fprintf(stderr, "RESUME: no save found at %s, starting new game\n",
            save_path ? save_path : "(unresolved)");
    return 1;
}

static int m11_dm1_host_log_entrance_skipped(void* user) {
    (void)user;
    fprintf(stderr, "entrance transition skipped (non-fatal)\n");
    return 1;
}

static int m11_dm1_selected_launch_open(void* user,
                                        char* out_source_id,
                                        int out_source_id_size) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->gameView || !ctx->menuState) {
        return 0;
    }
    if (!M11_GameView_OpenSelectedMenuEntry(ctx->gameView, ctx->menuState)) {
        return 0;
    }
    if (out_source_id && out_source_id_size > 0) {
        snprintf(out_source_id,
                 (size_t)out_source_id_size,
                 "%s",
                 ctx->gameView->sourceId);
    }
    return 1;
}

static int m11_dm1_selected_launch_after_open(void* user) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->menuState) {
        return 0;
    }
    ctx->menuState->launchRequested = 0;
    (void)M11_Render_SetPaletteLevel(0);
    if (ctx->idleAccumulatorMs) {
        *ctx->idleAccumulatorMs = 0;
    }
    return 1;
}

static int m11_dm1_selected_launch_apply_runtime_handoff(
    void* user,
    const DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* receipt) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->gameView) {
        return 0;
    }
    /* The selected-launch transaction has just consumed ENTRANCE.C F0441.
     * Materialize its existing Hall VIEWING receipt before draw_opened so
     * the first runtime frame cannot fall back to the pre-Entrance world. */
    return M11_GameView_ApplyDm1StartupRuntimeHandoff(ctx->gameView,
                                                       receipt);
}

static int m11_dm1_selected_launch_draw_opened(void* user) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->gameView) {
        return 0;
    }
    M11_GameView_Draw(ctx->gameView,
                      M11_Render_GetFramebuffer(),
                      M11_FB_WIDTH,
                      M11_FB_HEIGHT);
    return 1;
}

static int m11_dm1_selected_launch_mark_failed(void* user) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->menuState) {
        return 0;
    }
    m11_set_launch_failed_message(ctx->menuState);
    return 1;
}

/* Resolve edition-specific startup routing from the launch intent rather
 * than the persisted menu row. A data-directory change or a repaired AUTO
 * selection can make that row stale while the intent already points to a
 * different authenticated edition. */
static int m11_selected_dm1_launch_version_index(
    const M12_StartupMenuState* menuState,
    const M12_MenuEntry* entry) {
    const M12_AssetVersionStatus* version;
    M12_LaunchIntent intent;
    int versionIndex;
    if (!menuState || !entry || !entry->gameId ||
        strcmp(entry->gameId, "dm1") != 0) {
        return -1;
    }
    /* Startup routing must follow the same authenticated edition as the
     * launch transaction. GetLaunchIntent can repair stale persisted version
     * selections against the current scan; reading gameOptions directly here
     * can send a valid DOS launch through an unrelated Atari/Towns path. */
    intent = M12_StartupMenu_GetLaunchIntent(menuState);
    versionIndex = intent.valid && intent.options.versionIndex >= 0
        ? intent.options.versionIndex : menuState->gameOptions[0].versionIndex;
    if (versionIndex < 0) {
        return -1;
    }
    version = M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                         (size_t)versionIndex);
    return version && version->matched ? versionIndex : -1;
}

static int m11_selected_dm1_is_fmtowns(const M12_StartupMenuState* menuState,
                                       const M12_MenuEntry* entry) {
    /* The HMA-240 executable owns DO_TITLE_ANIMATION and DYNAMENU. Never send
     * this edition through PC34 SWSH -> TITLE -> ENTRANCE: its title frames
     * use the matched EDM/JDM program, and DATA/JDATA stays on the same disc.
     * See dm1_v1_fmtowns_startup.c and the FM Towns parity evidence. */
    int versionIndex =
        m11_selected_dm1_launch_version_index(menuState, entry);
    const M12_AssetVersionStatus* version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                     (size_t)versionIndex)
        : NULL;
    return version && version->versionId &&
           (strcmp(version->versionId, "fmtowns-en") == 0 ||
            strcmp(version->versionId, "fmtowns-ja") == 0);
}

static int m11_selected_dm1_is_iigs(const M12_StartupMenuState* menuState,
                                    const M12_MenuEntry* entry) {
    int versionIndex = m11_selected_dm1_launch_version_index(menuState, entry);
    return versionIndex >= 0 &&
           M12_AssetStatus_GetVersionArchitecture("dm1", (size_t)versionIndex) ==
               M12_ARCH_APPLE_IIGS;
}

/* ReDMCSB TITLE.C F0437, MEDIA351 (G20E/G21E), lines 185-214, and
 * STARTUP2.C MEDIA274, lines 1391-1392: play native C001 before
 * LOADSAVE/F0434 opens the first world. Key and pointer input is
 * consumed by the intro event pump while no game view is active. The title
 * zoom's host scaler is currently nearest-neighbour, not GS F0129 parity. */
static int m11_play_dm1_iigs_title_f0437(
    const M12_StartupMenuState* menuState, const M12_MenuEntry* entry,
    int bootProbe) {
    DM1_V1_IigsInstallation installation;
    const M12_AssetVersionStatus* version;
    int versionIndex;
    int expectedVersion;
    uint8_t *graphics=NULL, *title=NULL, *framebuffer;
    size_t graphicsSize=0u;
    uint16_t width=0u,height=0u;
    uint16_t palette[16];
    uint64_t started;
    uint8_t rgb4[256][3]={{0}};
    int quitRequested=0;
    int result=0;

    if (!m11_selected_dm1_is_iigs(menuState,entry)) return 1;
    if (bootProbe) return 1;
    versionIndex=m11_selected_dm1_launch_version_index(menuState,entry);
    version=versionIndex>=0 ? M12_AssetStatus_GetVersion(
        &menuState->assetStatus,"dm1",(size_t)versionIndex) : NULL;
    if (!version || !version->versionId) goto cleanup;
    expectedVersion=strcmp(version->versionId,"iigs20-en")==0 ? 20 :
                    strcmp(version->versionId,"iigs21-en")==0 ? 21 : 0;
    if (!expectedVersion || dm1_v1_iigs_installation_find(
            version->matchedPath,expectedVersion,&installation)!=1 ||
        !asset_read_path_alloc(installation.graphics_path,&graphics,
                               &graphicsSize)) goto cleanup;
    title=(uint8_t*)malloc(320u*200u);
    framebuffer=M11_Render_GetFramebuffer();
    if (!title || !framebuffer || !dm1_v1_iigs_graphics_decode(
            graphics,graphicsSize,1u,title,320u*200u,&width,&height) ||
        width!=320u || height!=200u) goto cleanup;

    M11_Render_SetV2PresentationActive(0);
    M11_Render_SetModernPresentationActive(0);
    started=SDL_GetTicks();
    if (!dm1_v1_iigs_title_f0437_compose(title,0u,framebuffer,palette))
        goto cleanup;
    for (unsigned int i=0u;i<16u;++i) {
        rgb4[i][0]=(uint8_t)((palette[i]>>8)&15u);
        rgb4[i][1]=(uint8_t)((palette[i]>>4)&15u);
        rgb4[i][2]=(uint8_t)(palette[i]&15u);
    }
    for (unsigned int i=16u;i<256u;++i) {
        rgb4[i][0]=rgb4[i&15u][0];
        rgb4[i][1]=rgb4[i&15u][1];
        rgb4[i][2]=rgb4[i&15u][2];
    }
    if (M11_Render_SetIndexedPaletteRgb4(rgb4)!=M11_RENDER_OK) goto cleanup;
    for (unsigned int frame=0u;frame<DM1_V1_IIGS_TITLE_ZOOM_FRAMES;++frame) {
        const Uint64 deadline=started+((Uint64)(frame+1u)*1000u+30u)/60u;
        if (m11_wait_for_intro_active_tick((Uint64)deadline,&quitRequested)) {
            if (quitRequested) result=2;
            goto cleanup;
        }
        if (!dm1_v1_iigs_title_f0437_compose(title,frame,framebuffer,palette))
            goto cleanup;
        if (M11_Render_PresentIndexed(framebuffer,320,200)!=M11_RENDER_OK)
            goto cleanup;
        if (g_m11_debug_startup_detail)
            fprintf(stderr,"firestaff: startup-source-frame game=dm1 platform=apple-iigs phase=title-f0437 frame=%u/18 input=ignored\n",frame+1u);
    }
    for (unsigned int hold=1u;hold<=2u;++hold) {
        if (m11_wait_for_intro_active_tick(
                started+((Uint64)(DM1_V1_IIGS_TITLE_ZOOM_FRAMES+hold)*1000u+30u)/60u,
                &quitRequested)) {
            if (quitRequested) result=2;
            goto cleanup;
        }
    }
    if (g_m11_debug_startup_detail)
        fprintf(stderr,"firestaff: startup-source-complete game=dm1 platform=apple-iigs phase=title-f0437 frames=18 cadence-hz=60 sampling=nearest-neighbour-approx source=%s graphics=%s\n",
            version->versionId,installation.graphics_path);
    result=1;
cleanup:
    if (!result && g_m11_debug_startup_detail)
        fprintf(stderr,"firestaff: startup-source-rejected game=dm1 platform=apple-iigs phase=title-f0437\n");
    free(title); free(graphics);
    return result;
}

/* ReDMCSB STARTUP2.C enters ENTRANCE.C F0441 after opening the selected
 * dungeon. The original GS page uses native C004/C002/C003 records and
 * F0439's transient C255 viewport while waiting for a fresh mouse click. */
static int m11_play_dm1_iigs_entrance_f0441(
    const M12_StartupMenuState* menuState, const M12_MenuEntry* entry,
    M11_GameViewState* gameView, int bootProbe) {
    DM1_V1_IigsInstallation installation;
    const M12_AssetVersionStatus* version;
    const M11_AssetSlot* entrance;
    const M11_AssetSlot* leftDoor;
    const M11_AssetSlot* rightDoor;
    EntranceCompatCompositePixels pixels;
    unsigned char* page;
    unsigned char* dungeonFrame;
    unsigned char paletteRgb4[256][3] = {{0}};
    int versionIndex;
    int expectedVersion;
    M11_EntranceCommand command;
    Uint64 started;
    int quitRequested = 0;

    if (!m11_selected_dm1_is_iigs(menuState, entry)) return 1;
    if (bootProbe || getenv("FIRESTAFF_EXIT_AFTER_LAUNCH")) return -1;
    if (!gameView || !gameView->active) return 0;
    versionIndex = m11_selected_dm1_launch_version_index(menuState, entry);
    version = versionIndex >= 0 ? M12_AssetStatus_GetVersion(
        &menuState->assetStatus, "dm1", (size_t)versionIndex) : NULL;
    expectedVersion = version && version->versionId
        ? (strcmp(version->versionId, "iigs20-en") == 0 ? 20 :
           strcmp(version->versionId, "iigs21-en") == 0 ? 21 : 0)
        : 0;
    if (!version || !expectedVersion ||
        dm1_v1_iigs_installation_find(version->matchedPath,
            expectedVersion, &installation) != 1) return 0;

    entrance = M11_AssetLoader_Load(&gameView->assetLoader, 4u);
    leftDoor = M11_AssetLoader_Load(&gameView->assetLoader, 2u);
    rightDoor = M11_AssetLoader_Load(&gameView->assetLoader, 3u);
    page = M11_Render_GetFramebuffer();
    dungeonFrame = (unsigned char*)malloc((size_t)M11_FB_BYTES);
    if (!entrance || entrance->width != 320u || entrance->height != 200u ||
        !leftDoor || leftDoor->width != 128u || leftDoor->height != 161u ||
        !rightDoor || rightDoor->width != 128u || rightDoor->height != 161u ||
        !page || !dungeonFrame) {
        free(dungeonFrame);
        return 0;
    }

    /* IIGS F0439 constructs a transient C255 micro-map; render it from the
     * authenticated original graphics bank without exposing the campaign map. */
    if (!M11_GameView_RenderDm1IigsEntranceC255(
            gameView, dungeonFrame, M11_FB_BYTES, M11_FB_WIDTH)) {
        free(dungeonFrame);
        return 0;
    }
    memset(page, 0, (size_t)M11_FB_BYTES);
    memcpy(page, entrance->pixels, 320u * 200u);
    for (unsigned int row = 0u; row < 136u; ++row) {
        memcpy(page + (size_t)(33u + row) * 320u,
               dungeonFrame + (size_t)row * 320u, 224u);
    }
    if (!m11_draw_entrance_closed_doors_asset(gameView, page)) {
        free(dungeonFrame);
        return 0;
    }
    for (unsigned int i = 0u; i < 16u; ++i) {
        uint16_t color = installation.metadata.entrance_palette[i];
        paletteRgb4[i][0] = (unsigned char)((color >> 8) & 15u);
        paletteRgb4[i][1] = (unsigned char)((color >> 4) & 15u);
        paletteRgb4[i][2] = (unsigned char)(color & 15u);
    }
    for (unsigned int i = 16u; i < 256u; ++i) {
        paletteRgb4[i][0] = paletteRgb4[i & 15u][0];
        paletteRgb4[i][1] = paletteRgb4[i & 15u][1];
        paletteRgb4[i][2] = paletteRgb4[i & 15u][2];
    }
    M11_Render_SetV2PresentationActive(0);
    M11_Render_SetModernPresentationActive(0);
    if (M11_Render_SetIndexedPaletteRgb4(paletteRgb4) != M11_RENDER_OK ||
        M11_Render_PresentIndexed(page, 320, 200) != M11_RENDER_OK) {
        free(dungeonFrame);
        return 0;
    }
    if (g_m11_debug_startup_detail) {
        fprintf(stderr,
            "firestaff: startup-source-frame game=dm1 platform=apple-iigs phase=entrance-f0441 state=closed input=mouse-only viewport=f0439-c255 source=F0441+C004+C002+C003\n");
    }

    /* IIGS ENTRANCE.C sets no primary keyboard table. Requiring a fresh
     * source-space mouse click also prevents the launcher click from being
     * reused as C200. Headless verification uses the existing bounded
     * dummy-video auto-enter path. */
    do {
        command = m11_wait_for_redmcsb_entrance_command(-1, 1);
    } while (command != M11_ENTRANCE_COMMAND_ENTER &&
             command != M11_ENTRANCE_COMMAND_QUIT);
    if (command == M11_ENTRANCE_COMMAND_QUIT) {
        free(dungeonFrame);
        return 2;
    }
    (void)M11_Audio_EmitSourceSoundIndex(&gameView->audioState, 1);
    if (m11_delay_ms_with_intro_event_pump(400u)) {
        free(dungeonFrame);
        return 2;
    }
    pixels.entranceScreen = entrance->pixels;
    pixels.entranceWidth = entrance->width;
    pixels.entranceHeight = entrance->height;
    pixels.dungeonFrame = dungeonFrame;
    pixels.dungeonFrameWidth = M11_FB_WIDTH;
    pixels.dungeonFrameHeight = M11_FB_HEIGHT;
    pixels.leftDoor = leftDoor->pixels;
    pixels.leftDoorWidth = leftDoor->width;
    pixels.leftDoorHeight = leftDoor->height;
    pixels.rightDoor = rightDoor->pixels;
    pixels.rightDoorWidth = rightDoor->width;
    pixels.rightDoorHeight = rightDoor->height;
    started = m11_intro_active_ticks();
    for (unsigned int frame = 1u;
         frame <= ENTRANCE_Compat_GetDoorAnimationStepCount(); ++frame) {
        EntranceCompatDoorStep door;
        if (!ENTRANCE_Compat_GetDoorAnimationStep(frame, &door) ||
            !ENTRANCE_Compat_CompositeDoorOpeningFrame(
                page, 320u, 200u, &pixels, &door)) {
            free(dungeonFrame);
            return 0;
        }
        if (door.soundRattle)
            (void)M11_Audio_EmitSourceSoundIndex(&gameView->audioState, 2);
        if (M11_Render_PresentIndexed(page, 320, 200) != M11_RENDER_OK ||
            m11_wait_for_intro_active_tick(
                started + ((Uint64)frame * 1000u + 30u) / 60u,
                &quitRequested)) {
            free(dungeonFrame);
            return quitRequested ? 2 : 0;
        }
        if (g_m11_debug_startup_detail &&
            (frame == 1u || frame == 16u || frame == 31u)) {
            fprintf(stderr,
                "firestaff: startup-source-frame game=dm1 platform=apple-iigs phase=entrance-door frame=%u/31 source-vblanks=1\n",
                frame);
        }
    }
    free(dungeonFrame);
    if (g_m11_debug_startup_detail) {
        fprintf(stderr,
            "firestaff: startup-f0441-authenticated-handoff game=dm1 platform=apple-iigs version=%d graphics=%s input=mouse-only doors=31\n",
            expectedVersion, installation.graphics_path);
    }
    return 1;
}

static int m11_selected_dm1_is_atari(const M12_StartupMenuState* menuState,
                                     const M12_MenuEntry* entry) {
    int versionIndex =
        m11_selected_dm1_launch_version_index(menuState, entry);
    return versionIndex >= 0 &&
           M12_AssetStatus_GetVersionArchitecture("dm1",
                                                   (size_t)versionIndex) ==
           M12_ARCH_ATARI_ST;
}

static int m11_selected_dm1_is_amiga(const M12_StartupMenuState* menuState,
                                     const M12_MenuEntry* entry) {
    int versionIndex =
        m11_selected_dm1_launch_version_index(menuState, entry);
    return versionIndex >= 0 &&
           M12_AssetStatus_GetVersionArchitecture(
               "dm1", (size_t)versionIndex) == M12_ARCH_AMIGA;
}

static int m11_selected_dm1_is_amiga_hd(const M12_StartupMenuState* menuState,
                                        const M12_MenuEntry* entry) {
    int versionIndex =
        m11_selected_dm1_launch_version_index(menuState, entry);
    const M12_AssetVersionStatus* version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                     (size_t)versionIndex)
        : NULL;
    /* The preserved [HD] image is installed-disk media and enters directly
     * through its authenticated IMG2 runtime. The v2.0 floppy set has the
     * source-visible SWSH/TITLE/ENTRANCE handoff and retains that transaction. */
    return versionIndex >= 0 &&
           M12_AssetStatus_GetVersionArchitecture(
               "dm1", (size_t)versionIndex) == M12_ARCH_AMIGA &&
           version && version->matchedPath[0] != '\0' &&
           strstr(version->matchedPath, "[HD]") != NULL;
}

static int m11_selected_dm1_is_amiga36(
    const M12_StartupMenuState* menuState,
    const M12_MenuEntry* entry) {
    int versionIndex =
        m11_selected_dm1_launch_version_index(menuState, entry);
    const M12_AssetVersionStatus* version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                     (size_t)versionIndex)
        : NULL;
    /* ReDMCSB STARTUP1.C:143 calls F0437 before the Amiga startup path.
     * TITLE.C's F0437 artwork branches do not include A36M; the present
     * source-authenticated title/entrance transaction is PC34/A20E-specific.
     * Admit A36M's authenticated virtual-disk runtime directly until its
     * ENTRANCE.C F0441 path is bound. */
    return version && version->versionId &&
           strcmp(version->versionId, "amiga36-multi") == 0 &&
           M12_AssetStatus_GetVersionArchitecture(
               "dm1", (size_t)versionIndex) == M12_ARCH_AMIGA;
}

static int m11_selected_dm1_uses_direct_amiga_launch(
    const M12_StartupMenuState* menuState,
    const M12_MenuEntry* entry) {
    return m11_selected_dm1_is_amiga_hd(menuState, entry) ||
           m11_selected_dm1_is_amiga36(menuState, entry);
}

static int m11_play_dm1_fmtowns_title_if_available(
    M11_GameViewState *gameView, int *outPlayedAnyFrame) {
    const DM1_V1_FmtownsStartupReceipt *plan;
    const M11_AssetSlot *title;
    unsigned char *framebuffer;
    uint8_t titlePaletteRgb6[256][3];
    unsigned int frame;
    unsigned int waitedVblanks = 0u;
    int autotestQuitInjected = 0;
    Uint64 titleStartedMs;
    if (outPlayedAnyFrame) *outPlayedAnyFrame = 0;
    if (!gameView || !gameView->dm1FmtownsStartupReceiptValid ||
        !gameView->assetLoader.legacyDm1 ||
        !gameView->dm1FmtownsStartupReceipt.game_title_animation_plan_verified ||
        !gameView->dm1FmtownsStartupReceipt.game_title_palettes_verified) {
        return 0;
    }
    plan = &gameView->dm1FmtownsStartupReceipt;
    title = M11_AssetLoader_Load(&gameView->assetLoader,
                                 plan->game_title_graphic_index);
    framebuffer = M11_Render_GetFramebuffer();
    if (!title || !framebuffer || title->width < 320u || title->height < 200u) {
        if (g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "firestaff: startup-rejected game=dm1 platform=fm-towns phase=title-asset index=%u loaded=%d size=%ux%u framebuffer=%d\n",
                    plan->game_title_graphic_index, title ? 1 : 0,
                    title ? title->width : 0u, title ? title->height : 0u,
                    framebuffer ? 1 : 0);
        }
        return 0;
    }
    /* DMWeb's authenticated FM Towns track table assigns CDDA track 02 to
     * the title screen.  Start it before the first EDM.EXP presentation;
     * the game-map dispatcher owns the later track change.  A missing or
     * malformed original CUE/BIN stays silent rather than borrowing SONG.DAT.
     */
    /* This routine is the sole title-track owner.  Starting it in the
     * launcher as well would make the second dispatch stop and recreate the
     * original CDDA stream immediately before frame zero. */
    (void)M11_GameView_PlayDm1FmtownsCddaTrack(gameView, plan->title_track);
    titleStartedMs = m11_intro_active_ticks();
    /* EDM.EXP DO_TITLE_ANIMATION presents the prepared zoom bitmaps in
     * reverse order: 48x12 first, then 16x4 larger per frame.  Use the
     * receipt-bound compositor for M11 as well, so the PRESENTS strip and
     * final TITLE_MASTER cannot diverge from the exact P3 plan verified in
     * dm1_v1_fmtowns_title.c.  EDM.EXP +0xc3f0 first flips PRESENTS by
     * itself; +0xc563 waits once before each prepared zoom bitmap. The host
     * records each blank after the preceding presentation, including the
     * PRESENTS-to-zoom boundary. Cumulative 60 Hz deadlines avoid the drift
     * from rounding every blank to 17 ms.
     */
    /* STARTEND/TITLE.C's F20 branch displays PRESENTS, then waits before
     * each of the eighteen reverse-order zoom bitmaps. It does not put
     * TITLE_MASTER on screen immediately after the final zoom: it waits two
     * VBlanks first, blits
     * MASTER, then holds that page for one final VBlank.  Keeping MASTER in
     * the loop used to reverse that boundary (and omit the final hold),
     * making the transition visibly premature on a real-time host. */
    for (frame = 0u; frame < DM1_FMTOWNS_TITLE_FINAL_FRAME; ++frame) {
        const char *titlePaletteName =
            frame == DM1_FMTOWNS_TITLE_PRESENTS_FRAME
                ? "PRESENTS" : "DUNGEON+MASTER";
        if (!dm1_v1_fmtowns_title_compose_frame(
                plan, title->pixels, title->width, title->height, frame,
                framebuffer, (size_t)M11_FB_BYTES)) {
            return 0;
        }
        memset(titlePaletteRgb6, 0, sizeof(titlePaletteRgb6));
        if (!dm1_v1_fmtowns_title_palette_for_frame(
                plan, frame, titlePaletteRgb6)) return 0;
        if (M11_Render_SetIndexedPaletteRgb6(titlePaletteRgb6) != M11_RENDER_OK)
            return 0;
        if (M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                      M11_FB_HEIGHT) != M11_RENDER_OK) return 0;
        if (outPlayedAnyFrame) *outPlayedAnyFrame = 1;
        /* Exercise the native title's close path after a real source frame,
         * matching the PC/Atari M12 interruption regressions. */
        if (!autotestQuitInjected && getenv("FIRESTAFF_AUTOTEST") &&
            getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_QUIT_AFTER_FRAME") &&
            strcmp(getenv("FIRESTAFF_AUTOTEST_DM1_TITLE_QUIT_AFTER_FRAME"),
                   "1") == 0) {
            SDL_Event event;
            memset(&event, 0, sizeof(event));
#if SDL_VERSION_ATLEAST(3, 0, 0)
            event.type = SDL_EVENT_QUIT;
#else
            event.type = SDL_QUIT;
#endif
            if (SDL_PushEvent(&event)) {
                autotestQuitInjected = 1;
                fprintf(stderr,
                        "AUTOTEST: DM1 FM Towns title quit injected after first rendered frame\n");
            }
        }
        {
            const unsigned int frameWaitVblanks =
                dm1_v1_fmtowns_title_frame_wait_vblanks(frame);
            if (g_m11_debug_startup_detail) {
                fprintf(stderr,
                        "firestaff: startup-source-frame game=dm1 platform=fm-towns phase=title frame=%u/%u palette=%s elapsed-ms=%llu source-wait-vblanks=%u\n",
                        frame,
                        DM1_FMTOWNS_TITLE_FINAL_FRAME + 1u,
                        titlePaletteName,
                        (unsigned long long)(m11_intro_active_ticks() -
                                             titleStartedMs),
                        frameWaitVblanks);
            }
            if (frameWaitVblanks) {
                ++waitedVblanks;
                if (m11_delay_ms_with_intro_event_pump(
                        dm1_v1_fmtowns_title_vblank_delay_ms(waitedVblanks))) {
                    return M11_ENTRANCE_COMMAND_QUIT;
                }
            }
        }
    }
    /* TITLE.C: F20E/F20J executes two VBlanks after the final zoom and
     * before F0132_VIDEO_Blit writes TITLE_MASTER.  The ordinal-based
     * helper preserves a 60 Hz aggregate deadline rather than rounding
     * each VBlank independently. */
    for (frame = 0u; frame < 2u; ++frame) {
        ++waitedVblanks;
        if (m11_delay_ms_with_intro_event_pump(
                dm1_v1_fmtowns_title_vblank_delay_ms(waitedVblanks))) {
            return M11_ENTRANCE_COMMAND_QUIT;
        }
    }
    if (!dm1_v1_fmtowns_title_compose_frame(
            plan, title->pixels, title->width, title->height,
            DM1_FMTOWNS_TITLE_FINAL_FRAME, framebuffer,
            (size_t)M11_FB_BYTES)) {
        return 0;
    }
    memset(titlePaletteRgb6, 0, sizeof(titlePaletteRgb6));
    if (!dm1_v1_fmtowns_title_palette_for_frame(
            plan, DM1_FMTOWNS_TITLE_FINAL_FRAME, titlePaletteRgb6)) return 0;
    if (M11_Render_SetIndexedPaletteRgb6(titlePaletteRgb6) != M11_RENDER_OK ||
        M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH,
                                  M11_FB_HEIGHT) != M11_RENDER_OK) {
        return 0;
    }
    if (outPlayedAnyFrame) *outPlayedAnyFrame = 1;
    if (g_m11_debug_startup_detail) {
        fprintf(stderr,
                "firestaff: startup-source-frame game=dm1 platform=fm-towns phase=title frame=%u/%u palette=MASTER elapsed-ms=%llu source-wait-vblanks=0\n",
                DM1_FMTOWNS_TITLE_FINAL_FRAME,
                DM1_FMTOWNS_TITLE_FINAL_FRAME + 1u,
                (unsigned long long)(m11_intro_active_ticks() -
                                     titleStartedMs));
    }
    /* The source's final M526_WaitVerticalBlank follows the master blit. */
    ++waitedVblanks;
    if (m11_delay_ms_with_intro_event_pump(
            dm1_v1_fmtowns_title_vblank_delay_ms(waitedVblanks))) {
        return M11_ENTRANCE_COMMAND_QUIT;
    }
    /* TITLE_MASTER is the only full-size, legible Dungeon Master screen.
     * The original blit leaves that page resident until another screen is
     * drawn; the host handoff immediately starts Entrance, so retain it for
     * a readable interval instead of replacing it on the next host call. */
    if (m11_delay_ms_with_intro_event_pump(
            DM1_FMTOWNS_TITLE_MASTER_HOLD_MS)) {
        return M11_ENTRANCE_COMMAND_QUIT;
    }
    if (g_m11_debug_startup_detail) {
        fprintf(stderr,
                "firestaff: startup-source-complete game=dm1 platform=fm-towns phase=title frames=%u source-vblanks=%u master-hold-ms=%u elapsed-ms=%llu\n",
                DM1_FMTOWNS_TITLE_FINAL_FRAME + 1u,
                waitedVblanks,
                DM1_FMTOWNS_TITLE_MASTER_HOLD_MS,
                (unsigned long long)(m11_intro_active_ticks() -
                                     titleStartedMs));
    }
    return M11_ENTRANCE_COMMAND_ENTER;
}

static int m11_game_display_aspect(int aspectRatio) {
    switch (aspectRatio) {
        case M12_ASPECT_4_3: return M11_DISPLAY_ASPECT_4_3;
        case M12_ASPECT_16_9: return M11_DISPLAY_ASPECT_16_9;
        case M12_ASPECT_16_10: return M11_DISPLAY_ASPECT_16_10;
        case M12_ASPECT_32_9: return M11_DISPLAY_ASPECT_32_9;
        default: return M11_DISPLAY_ASPECT_CONTENT;
    }
}

static int m11_game_option_slot(const char *gameId) {
    int slot = M12_StartupMenu_GetGameOptionsSlot(gameId);
    return slot >= 0 ? slot : 0;
}

static int m11_is_supported_game_id(const char *gameId) {
    return gameId &&
        (strcmp(gameId, "dm1") == 0 || strcmp(gameId, "csb") == 0 ||
         strcmp(gameId, "dm2") == 0 || strcmp(gameId, "nexus") == 0 ||
         strcmp(gameId, "theron") == 0);
}

static int m11_open_requested_launch_impl(M11_GameViewState* gameView,
                                     M12_StartupMenuState* menuState,
                                     uint32_t* idleAccumulatorMs,
                                     const char* dataDir,
                                     int bootProbe) {
    M11_DM1StartupHandoffContext dm1HandoffContext;
    DM1_V1_StartupHandoffCallbacks_PC34 dm1HandoffCallbacks;
    DM1_V1_StartupHostCallbacks_PC34 dm1HostCallbacks;
    DM1_V1_StartupSelectedLaunchCallbacks_PC34 dm1SelectedLaunchCallbacks;
    DM1_V1_StartupSelectedLaunchResult_PC34 dm1SelectedLaunchResult;
    DM1_V1_StartupSelectedLaunchRouteFacts_PC34 dm1RouteFacts;
    DM1_V1_StartupSelectedLaunchRouteReceipt_PC34 dm1RouteReceipt;
    const M12_MenuEntry* launchEntry;
    if (!gameView || !menuState || !menuState->launchRequested) {
        return 0;
    }
    if (!M12_StartupMenu_PrepareSelectedGameLaunch(menuState)) {
        /* A launch gate can be ready while its edition-specific runtime
         * preparation still fails (for example, a required native package
         * handoff cannot be bound). Do not leave the launcher on its stale
         * READY message with launchRequested latched; use the same
         * game-specific failure path as a rejected M11 open. */
        m11_set_launch_failed_message(menuState);
        return 0;
    }
    launchEntry = M12_StartupMenu_GetEntry(menuState, menuState->activatedIndex);
    /* The language on a game's Custom card is a launch option. Apply it at
     * the handoff boundary, after the selected-version checks and before a
     * title or entrance frame can expose player text. */
    m11_apply_selected_game_language(menuState, launchEntry, dataDir);
    /* Game options are per title.  Apply the selected ratio before any title
     * frame is shown and disable the launcher-only fill mode.  The renderer
     * computes a fitted destination rectangle, so this is letterbox or
     * pillarbox presentation, never a distorted stretch. */
    (void)M11_Render_SetPresentationFillWindow(0);
    (void)M11_Render_SetDisplayAspectMode(m11_game_display_aspect(
        menuState->gameOptions[m11_game_option_slot(
            launchEntry ? launchEntry->gameId : NULL)].aspectRatio));
    /* Nexus follows the same in-process native launch path as every other
     * game. Its runtime retains the real-asset admission gates; M11 must not
     * substitute an external emulator process. */
    memset(&dm1HandoffContext, 0, sizeof(dm1HandoffContext));
    memset(&dm1HandoffCallbacks, 0, sizeof(dm1HandoffCallbacks));
    memset(&dm1HostCallbacks, 0, sizeof(dm1HostCallbacks));
    memset(&dm1SelectedLaunchCallbacks, 0, sizeof(dm1SelectedLaunchCallbacks));
    memset(&dm1SelectedLaunchResult, 0, sizeof(dm1SelectedLaunchResult));
    memset(&dm1RouteFacts, 0, sizeof(dm1RouteFacts));
    memset(&dm1RouteReceipt, 0, sizeof(dm1RouteReceipt));
    dm1HandoffContext.menuState = menuState;
    dm1HandoffContext.launchEntry = launchEntry;
    dm1HandoffContext.gameView = gameView;
    dm1HandoffContext.dataDir = dataDir;
    dm1HandoffContext.idleAccumulatorMs = idleAccumulatorMs;
    dm1HandoffContext.bootProbe = bootProbe ? 1 : 0;
    dm1HandoffCallbacks.user = &dm1HandoffContext;
    dm1HandoffCallbacks.begin_prelude_plan =
        m11_dm1_handoff_begin_prelude_plan;
    dm1HandoffCallbacks.end_prelude_plan =
        m11_dm1_handoff_end_prelude_plan;
    dm1HandoffCallbacks.begin_post_launch_plan =
        m11_dm1_handoff_begin_post_launch_plan;
    dm1HandoffCallbacks.end_post_launch_plan =
        m11_dm1_handoff_end_post_launch_plan;
    dm1HandoffCallbacks.report_source_order_failure =
        m11_dm1_handoff_report_source_order_failure;
    dm1HandoffCallbacks.raise_window = m11_dm1_handoff_raise_window;
    dm1HandoffCallbacks.play_swsh = m11_dm1_handoff_play_swsh;
    dm1HandoffCallbacks.discard_presentation_texture =
        m11_dm1_handoff_discard_presentation_texture;
    dm1HandoffCallbacks.play_title = m11_dm1_handoff_play_title;
    dm1HandoffCallbacks.play_entrance = m11_dm1_handoff_play_entrance;
    dm1HostCallbacks.user = &dm1HandoffContext;
    dm1HostCallbacks.set_game_active = m11_dm1_host_set_game_active;
    dm1HostCallbacks.resolve_resume_save_path =
        m11_dm1_host_resolve_resume_save_path;
    dm1HostCallbacks.load_resume_save_path =
        m11_dm1_host_load_resume_save_path;
    dm1HostCallbacks.log_resume_loaded = m11_dm1_host_log_resume_loaded;
    dm1HostCallbacks.log_resume_missing = m11_dm1_host_log_resume_missing;
    dm1HostCallbacks.log_entrance_skipped = m11_dm1_host_log_entrance_skipped;
    dm1SelectedLaunchCallbacks.user = &dm1HandoffContext;
    dm1SelectedLaunchCallbacks.handoff_callbacks = &dm1HandoffCallbacks;
    dm1SelectedLaunchCallbacks.host_callbacks = &dm1HostCallbacks;
    dm1SelectedLaunchCallbacks.open_selected_entry = m11_dm1_selected_launch_open;
    dm1SelectedLaunchCallbacks.after_open = m11_dm1_selected_launch_after_open;
    dm1SelectedLaunchCallbacks.apply_runtime_handoff =
        m11_dm1_selected_launch_apply_runtime_handoff;
    dm1SelectedLaunchCallbacks.draw_opened = m11_dm1_selected_launch_draw_opened;
    dm1SelectedLaunchCallbacks.mark_launch_failed =
        m11_dm1_selected_launch_mark_failed;
    {
        int selectedVersionIndex = m11_selected_dm1_launch_version_index(
            menuState, launchEntry);
        const M12_AssetVersionStatus* selectedVersion = selectedVersionIndex >= 0
            ? M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                         (size_t)selectedVersionIndex)
            : NULL;
        const int selectedI34E = selectedVersion &&
            selectedVersion->versionId &&
            strcmp(selectedVersion->versionId, "pc34-en") == 0 &&
            M12_AssetStatus_GetVersionArchitecture(
                "dm1", (size_t)selectedVersionIndex) == M12_ARCH_PC;
        const int introWasPreplayed = selectedI34E
            ? g_m11_dm1_i34e_intro_preplayed : 0;
        g_m11_dm1_i34e_intro_preplayed = 0;
        if (selectedI34E && !bootProbe) {
            int titleResult = 0;
            if (!introWasPreplayed) {
                M11_Render_RaiseWindow();
                (void)m11_play_ftl_swoosh_for_game_if_available(
                    menuState, dataDir, "dm1", 0, NULL);
                if (g_m11_startup_intro_quit_requested &&
                    g_m11_debug_startup_detail) {
                    fprintf(stderr,
                            "firestaff: startup-source-quit game=dm1 phase=swsh-intro action=exit\n");
                }
                titleResult = g_m11_startup_intro_quit_requested
                    ? M11_ENTRANCE_COMMAND_QUIT
                    : m11_play_dm1_i34e_title_anim_if_available(
                          menuState, selectedVersion);
            }
            if (titleResult == M11_ENTRANCE_COMMAND_QUIT) {
                menuState->launchRequested = 0;
                menuState->shouldExit = 1;
                return 1;
            }
            if (introWasPreplayed || titleResult > 0) {
                const int selectorResult = m11_play_dm1_i34e_selector_enter_if_available(
                    gameView, menuState, selectedVersion);
                if (selectorResult == M11_ENTRANCE_COMMAND_QUIT) {
                    menuState->launchRequested = 0;
                    menuState->shouldExit = 1;
                    return 1;
                }
            }
        }
        /* I34E IBMIO owns ANIM/TITLE and SELECTOR, then starts FIRES. It
         * never enters the shared F0437/F0441 handoff. The pre-menu title
         * animation or this selected-launch boundary owns the title. M12
         * replaces I34E's following selector on the menu route. */
        dm1RouteFacts.selected_game_id = selectedI34E
            ? "dm1-i34e"
            : (m11_selected_dm1_is_fmtowns(menuState, launchEntry)
                ? "dm1-fmtowns"
                : ((launchEntry && launchEntry->gameId)
                    ? launchEntry->gameId : NULL));
    }
    if (!dm1_v1_startup_selected_launch_route_receipt_pc34(
            &dm1RouteFacts,
            &dm1RouteReceipt)) {
        m11_set_launch_failed_message(menuState);
        return 0;
    }
    /* FM Towns owns a native EDM/JDM title/menu path.  The PC34
     * source-visible transaction is only for the DOS launcher and would
     * incorrectly force the native edition through the entrance handoff. */
    if (m11_selected_dm1_is_fmtowns(menuState, launchEntry)) {
        dm1RouteReceipt.use_dm1_transaction = 0;
        dm1RouteReceipt.use_generic_launch = 1;
    }
    /* Native GS startup consumes its own ProDOS installation and LE world.
     * The PC/A20 title/entrance transaction cannot interpret those sources.
     * Native intro/entrance playback remains separate unfinished work. */
    if (m11_selected_dm1_is_iigs(menuState, launchEntry)) {
        dm1RouteReceipt.use_dm1_transaction = 0;
        dm1RouteReceipt.use_generic_launch = 1;
        {
            const int titleResult=m11_play_dm1_iigs_title_f0437(
                menuState,launchEntry,bootProbe);
            if (titleResult==2) {
                menuState->launchRequested=0;
                menuState->shouldExit=1;
                return 1;
            }
            if (!titleResult) {
                m11_set_launch_failed_message(menuState);
                return 0;
            }
        }
    }
    if (m11_selected_dm1_is_atari(menuState, launchEntry)) {
        dm1RouteReceipt.use_dm1_transaction = 0;
        dm1RouteReceipt.use_generic_launch = 1;
    }
    if (m11_selected_dm1_uses_direct_amiga_launch(menuState, launchEntry)) {
        dm1RouteReceipt.use_dm1_transaction = 0;
        dm1RouteReceipt.use_generic_launch = 1;
    }
    if (launchEntry && launchEntry->gameId &&
        strcmp(launchEntry->gameId, "dm1") == 0 &&
        m11_selected_dm1_is_amiga(menuState, launchEntry) &&
        menuState->quickResumeAvailable &&
        menuState->quickResumeLaunchRequested &&
        menuState->quickResumeSavePath[0] != '\0' &&
        strstr(menuState->quickResumeSavePath, "::") != NULL) {
        /* An original Amiga save selected from ZIP -> ADF belongs to the
         * Amiga F0435 route. Keep it virtual for the Amiga in-memory loader;
         * never let an unrelated FM Towns/Atari edition bypass its own
         * source-visible startup because a stale cross-platform save exists. */
        dm1RouteReceipt.use_dm1_transaction = 0;
        dm1RouteReceipt.use_generic_launch = 1;
    }
    if (dm1RouteReceipt.use_dm1_transaction) {
        int oldFastForward = g_m11_intro_delay_fast_forward;
        g_m11_intro_delay_fast_forward = bootProbe ? 1 : oldFastForward;
        if (!dm1_v1_startup_execute_selected_launch_transaction_pc34(
                launchEntry->gameId,
                &dm1SelectedLaunchCallbacks,
                &dm1SelectedLaunchResult)) {
            g_m11_intro_delay_fast_forward = oldFastForward;
            if (dm1HandoffContext.titleQuitRequested ||
                dm1HandoffContext.startupQuitRequested) {
                if (dm1HandoffContext.startupQuitRequested &&
                    g_m11_debug_startup_detail) {
                    fprintf(stderr,
                            "firestaff: startup-source-quit game=dm1 phase=swsh-intro action=exit\n");
                }
                M11_GameView_Shutdown(gameView);
                M11_GameView_Init(gameView);
                menuState->launchRequested = 0;
                menuState->shouldExit = 1;
                return 1;
            }
            m11_set_launch_failed_message(menuState);
            return 0;
        }
        g_m11_intro_delay_fast_forward = oldFastForward;
        if (dm1SelectedLaunchResult.runtime_handoff_receipt.handled) {
            /* Only the real selected-launch transaction may publish this
             * state.  M11_GameView_OpenSelectedMenuEntry merely opens the
             * dungeon so it cannot claim that source-visible startup frames
             * have already played. */
            gameView->dm1StartupHandoffExecuted =
                dm1SelectedLaunchResult.runtime_handoff_receipt
                        .runtime_first_frame_ready ? 1 : 0;
            return (dm1SelectedLaunchResult.runtime_handoff_receipt
                        .runtime_first_frame_ready ||
                    dm1SelectedLaunchResult.runtime_handoff_receipt
                        .return_to_launcher)
                       ? 1
                       : 0;
        }
        return dm1SelectedLaunchResult.opened ? 1 : 0;
    }
    {
        /* CSB has its own title/entrance sequence after the common FTL/SWSH
         * prelude.  ReDMCSB SWSH.C runs the FTL logo before the started
         * program hands off to TITLE/ENTRANCE; Firestaff keeps CSB title and
         * entrance in M11 but still needs the SWSH prelude. */
        if (launchEntry && launchEntry->gameId &&
            strcmp(launchEntry->gameId, "csb") == 0) {
            M11_Render_RaiseWindow();
            m11_play_ftl_swoosh_for_game_if_available(menuState,
                                                       dataDir,
                                                       "csb",
                                                       0,
                                                       NULL);
            M11_Render_DiscardPresentationTexture();
        }
        /* Theron enters its native runtime directly; the DM1/CSB title
         * prelude does not apply to this launch path. */
    }
    if (M11_GameView_OpenSelectedMenuEntry(gameView, menuState)) {
        if (m11_selected_dm1_is_iigs(menuState, launchEntry) &&
            launchEntry && launchEntry->gameId &&
            strcmp(launchEntry->gameId, "dm1") == 0) {
            const int entranceResult =
                m11_play_dm1_iigs_entrance_f0441(menuState, launchEntry,
                                                  gameView, bootProbe);
            if (entranceResult == 2) {
                M11_GameView_Shutdown(gameView);
                M11_GameView_Init(gameView);
                menuState->launchRequested = 0;
                menuState->shouldExit = 1;
                return 1;
            }
            if (entranceResult == 0) {
                M11_GameView_Shutdown(gameView);
                M11_GameView_Init(gameView);
                m11_set_launch_failed_message(menuState);
                return 0;
            }
            if (entranceResult == 1) {
                gameView->dm1StartupIntroBypassed = 0;
                gameView->dm1StartupHandoffExecuted = 1;
                gameView->startedFromLauncher = 1;
            }
        }
        if (m11_selected_dm1_is_amiga36(menuState, launchEntry) &&
            launchEntry && launchEntry->gameId &&
            strcmp(launchEntry->gameId, "dm1") == 0) {
            /* STARTUP2.C calls F0441 for A36M after STARTUP1 skips F0437.
             * Authenticate its five source graphics from the selected
             * virtual ADF, then run the shared C004/C002/C003 source
             * choreography. F0441 itself supplies C200; do not reuse the
             * A20-only SWSH/title receipt or synthesize pixels. */
            if (!getenv("FIRESTAFF_EXIT_AFTER_LAUNCH")) {
                DM1_V1_AmigaEntranceF0441Receipt amigaReceipt;
                DM1_V1_StartupFullGraphicsMediaReceipt_PC34 mediaReceipt;
                M11_EntranceCommand entranceCommand;
                memset(&amigaReceipt, 0, sizeof(amigaReceipt));
                memset(&mediaReceipt, 0, sizeof(mediaReceipt));
                mediaReceipt.handled = 1;
                /* A36M uses the Amiga renderer below. The non-PC34 marker
                 * suppresses PC-only startup capture hooks in that renderer. */
                mediaReceipt.platform = DM1_V1_STARTUP_MEDIA_PLATFORM_ATARI_ST;
                if (!m11_dm1_amiga36_entrance_f0441_receipt_if_selected(
                        menuState, launchEntry, gameView, &amigaReceipt)) {
                    M11_GameView_Shutdown(gameView);
                    M11_GameView_Init(gameView);
                    m11_set_launch_failed_message(menuState);
                    return 0;
                }
                entranceCommand = m11_play_dm1_amiga_entrance_f0441(
                    gameView, -1, NULL, &mediaReceipt, &amigaReceipt);
                if (entranceCommand != M11_ENTRANCE_COMMAND_ENTER) {
                    M11_GameView_Shutdown(gameView);
                    M11_GameView_Init(gameView);
                    if (entranceCommand == M11_ENTRANCE_COMMAND_QUIT) {
                        menuState->launchRequested = 0;
                        menuState->shouldExit = 1;
                        return 1;
                    }
                    m11_set_launch_failed_message(menuState);
                    return 0;
                }
                if (g_m11_debug_startup_detail) {
                    fprintf(stderr,
                            "firestaff: startup-f0441-authenticated-handoff game=dm1 platform=amiga-v36 graphics=%s input=keyboard+mouse palette=rgb4 visual-parity=unverified\n",
                            amigaReceipt.graphics_md5);
                }
                gameView->dm1StartupIntroBypassed = 0;
                gameView->dm1StartupHandoffExecuted = 1;
            }
            gameView->startedFromLauncher = 1;
        }
        if (m11_selected_dm1_is_atari(menuState, launchEntry)) {
            if (!getenv("FIRESTAFF_EXIT_AFTER_LAUNCH")) {
                DM1_V1_StartupHandoffPostLaunchPlan_PC34 entrancePlan;
                DM1_V1_StartupHandoffOutcome_PC34 entranceOutcome;
                DM1_V1_StartupHostApplyResult_PC34 entranceHostResult;
                DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34
                    entranceRuntimeHandoff;
                M11_EntranceCommand entranceCommand =
                    M11_ENTRANCE_COMMAND_NONE;
                int titlePlayed = 0;
                int oldFastForward = g_m11_intro_delay_fast_forward;

                memset(&entrancePlan, 0, sizeof(entrancePlan));
                memset(&entranceOutcome, 0, sizeof(entranceOutcome));
                memset(&entranceHostResult, 0, sizeof(entranceHostResult));
                memset(&entranceRuntimeHandoff, 0,
                       sizeof(entranceRuntimeHandoff));
                if (!dm1_v1_startup_handoff_post_launch_plan_pc34(
                        "dm1", &entrancePlan) ||
                    !entrancePlan.play_entrance ||
                    !entrancePlan.entrance_full_start_receipt.valid ||
                    !dm1_v1_startup_full_graphics_media_receipt_atari_st_pc34(
                        "dm1", &entrancePlan.media_receipt)) {
                    M11_GameView_Shutdown(gameView);
                    M11_GameView_Init(gameView);
                    m11_set_launch_failed_message(menuState);
                    return 0;
                }

                /* Atari STARTUP1.C enters the common ENTRANCE.C path without
                 * the PC SWSH/title phases. A headless boot probe may
                 * synthesize its bounded C200 handoff; interactive play only
                 * accepts the source mouse rectangle in COMMAND.C. */
                if (bootProbe) {
                    g_m11_intro_delay_fast_forward = 1;
                }
                /* ReDMCSB STARTUP1.C:143 runs F0437 before F0441 on Atari
                 * ST. Use the decoded C001 title from this authenticated
                 * Atari GRAPHICS.DAT; do not substitute a PC TITLE.DAT bank
                 * or claim the title phase was consumed without presenting
                 * its original source surface. */
                M11_Render_RaiseWindow();
                {
                    int titleResult =
                        m11_play_redmcsb_title_graphic_intro_if_available(
                        menuState, gameView, "dm1", &titlePlayed,
                        &entrancePlan.media_receipt);
                    if (titleResult == M11_ENTRANCE_COMMAND_QUIT) {
                        g_m11_intro_delay_fast_forward = oldFastForward;
                        M11_GameView_Shutdown(gameView);
                        M11_GameView_Init(gameView);
                        menuState->launchRequested = 0;
                        menuState->shouldExit = 1;
                        return 1;
                    }
                    if (!titleResult || !titlePlayed) {
                        g_m11_intro_delay_fast_forward = oldFastForward;
                        M11_GameView_Shutdown(gameView);
                        M11_GameView_Init(gameView);
                        m11_set_launch_failed_message(menuState);
                        return 0;
                    }
                }
                entranceCommand = m11_play_redmcsb_entrance_transition(
                    gameView, -1,
                    &entrancePlan.entrance_full_start_receipt,
                    &entrancePlan.media_receipt);
                g_m11_intro_delay_fast_forward = oldFastForward;
                if (entranceCommand == M11_ENTRANCE_COMMAND_QUIT) {
                    M11_GameView_Shutdown(gameView);
                    M11_GameView_Init(gameView);
                    menuState->launchRequested = 0;
                    menuState->shouldExit = 1;
                    return 1;
                }
                if (entranceCommand != M11_ENTRANCE_COMMAND_ENTER) {
                    M11_GameView_Shutdown(gameView);
                    M11_GameView_Init(gameView);
                    m11_set_launch_failed_message(menuState);
                    return 0;
                }

                entranceOutcome.title_played = titlePlayed;
                entranceOutcome.entrance_command = (int)entranceCommand;
                entranceOutcome.action =
                    DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34;
                entranceOutcome.status = "ATARI ST ENTRANCE C200";
                entranceHostResult.handled = 1;
                if (!dm1_v1_startup_full_graphics_runtime_handoff_receipt_for_media_pc34(
                        "dm1", "dm1", &entrancePlan.media_receipt,
                        &entranceOutcome, &entranceHostResult,
                        &entranceRuntimeHandoff) ||
                    !M11_GameView_ApplyDm1StartupRuntimeHandoff(
                        gameView, &entranceRuntimeHandoff)) {
                    M11_GameView_Shutdown(gameView);
                    M11_GameView_Init(gameView);
                    m11_set_launch_failed_message(menuState);
                    return 0;
                }
            }
        }
        if (launchEntry && launchEntry->gameId &&
            strcmp(launchEntry->gameId, "dm1") == 0 &&
            m11_selected_dm1_is_amiga(menuState, launchEntry) &&
            menuState->quickResumeAvailable &&
            menuState->quickResumeLaunchRequested &&
            menuState->quickResumeSavePath[0] != '\0' &&
            strstr(menuState->quickResumeSavePath, "::") != NULL) {
            int used_backup = 0;
            /* The virtual Amiga route bypasses the PC34 TITLE/ENTRANCE
             * transaction above, but LOADSAVE.C F0435 still owns the
             * selected ADF save immediately after the verified dungeon is
             * live.  Load the exact in-memory member now; never fall back
             * to a loose host save or construct a replacement world. */
            if (!M11_GameView_LoadDm1SavePath(
                    gameView, menuState->quickResumeSavePath,
                    &used_backup)) {
                m11_set_launch_failed_message(menuState);
                return 0;
            }
            (void)m11_dm1_host_log_resume_loaded(
                &dm1HandoffContext, menuState->quickResumeSavePath,
                used_backup);
        }
        /* ReDMCSB I34E runs its own ANIM/TITLE/SELECTOR path and therefore
         * bypasses the shared F0435 startup transaction above. Its selector
         * Resume choice still means LOADSAVE.C F0435: restore the explicitly
         * selected PC34 save after the verified dungeon has been opened. */
        if (launchEntry && launchEntry->gameId &&
            strcmp(launchEntry->gameId, "dm1") == 0 &&
            menuState->quickResumeAvailable &&
            menuState->quickResumeLaunchRequested &&
            menuState->quickResumeSavePath[0] != '\0' &&
            strstr(menuState->quickResumeSavePath, "::") == NULL) {
            int versionIndex = m11_selected_dm1_launch_version_index(
                menuState, launchEntry);
            const M12_AssetVersionStatus* selectedVersion = versionIndex >= 0
                ? M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                             (size_t)versionIndex)
                : NULL;
            if (selectedVersion && selectedVersion->versionId &&
                strcmp(selectedVersion->versionId, "pc34-en") == 0 &&
                M12_AssetStatus_GetVersionArchitecture(
                    "dm1", (size_t)versionIndex) == M12_ARCH_PC) {
                int used_backup = 0;
                if (!M11_GameView_LoadDm1SavePath(
                        gameView, menuState->quickResumeSavePath,
                        &used_backup)) {
                    m11_set_launch_failed_message(menuState);
                    return 0;
                }
                (void)m11_dm1_host_log_resume_loaded(
                    &dm1HandoffContext, menuState->quickResumeSavePath,
                    used_backup);
            }
        }
        /* A direct CLI launch can select a concrete FM Towns archive before
         * M12 has populated its card-version index.  At this point
         * OpenSelectedMenuEntry has parsed the original disc in memory, so
         * its EDM/JDM receipt is the authoritative platform decision.  Do
         * not let a stale/unset menu-card selection skip the native title
         * and expose the generic PC34 title route for valid Towns media. */
        if (launchEntry && launchEntry->gameId &&
            strcmp(launchEntry->gameId, "dm1") == 0 &&
            gameView->dm1FmtownsStartupReceiptValid) {
            DM1_V1_StartupHandoffPostLaunchPlan_PC34 entrancePlan;
            M11_EntranceCommand entranceCommand = M11_ENTRANCE_COMMAND_NONE;
            int played = 0;
            int oldFastForward = g_m11_intro_delay_fast_forward;
            /* FM Towns must either consume its authenticated native title
             * plan or fail closed; do not expose the generic PC34 title. */
            (void)M11_Render_SetPaletteLevel(0);
            /* Load the FM Towns DM1 menu font eagerly at title time so
             * the later menu-draw path never blocks on I/O. Safe to
             * call repeatedly; returns 1 if the font is already loaded
             * or the load succeeds. */
            (void)M11_GameView_LoadDm1FmtownsMenuFontIfAvailable(gameView);
            /* A boot probe is a bounded non-interactive evidence route.  Its
             * requested frame count must not spend the full retail title
             * duration before reaching the probe receipt.  Keep this switch
             * scoped to that explicit CLI mode: the interactive EDM/JDM
             * title continues to use its source VBlank deadlines. */
            if (bootProbe) {
                g_m11_intro_delay_fast_forward = 1;
            }
            {
                const int titleResult =
                    m11_play_dm1_fmtowns_title_if_available(gameView, &played);
                if (titleResult == M11_ENTRANCE_COMMAND_QUIT) {
                    g_m11_intro_delay_fast_forward = oldFastForward;
                    M11_GameView_Shutdown(gameView);
                    M11_GameView_Init(gameView);
                    menuState->launchRequested = 0;
                    menuState->shouldExit = 1;
                    return 1;
                }
                if (titleResult != M11_ENTRANCE_COMMAND_ENTER || !played) {
                    if (g_m11_debug_startup_detail) {
                        fprintf(stderr,
                                "firestaff: startup-rejected game=dm1 platform=fm-towns phase=native-title result=%d played=%d native-graphics=%d\n",
                                (int)titleResult, played,
                                gameView->assetLoader.dm1Fmtowns);
                    }
                    g_m11_intro_delay_fast_forward = oldFastForward;
                    M11_GameView_Shutdown(gameView);
                    M11_GameView_Init(gameView);
                    m11_set_launch_failed_message(menuState);
                    return 0;
                }
            }
            g_m11_intro_delay_fast_forward = oldFastForward;
            /* EDM/JDM owns the Towns title, but completing that title is not
             * permission to expose a live, party-less dungeon.  The common
             * DM1 entrance owns the first interactive Hall-of-Champions
             * handoff; it consumes the already-bound original FM Towns
             * GRAPHICS.DAT surfaces and waits for a fresh entrance command.
             * Build the existing source-locked plan explicitly rather than
             * manufacturing a Towns-only shortcut or treating the title
             * launch input as C200. */
            memset(&entrancePlan, 0, sizeof(entrancePlan));
            if (!dm1_v1_startup_handoff_post_launch_plan_pc34(
                    "dm1", &entrancePlan) ||
                !entrancePlan.play_entrance ||
                !entrancePlan.entrance_full_start_receipt.valid) {
                M11_GameView_Shutdown(gameView);
                M11_GameView_Init(gameView);
                m11_set_launch_failed_message(menuState);
                return 0;
            }
            /* Never continue from an FM Towns title into the already-opened
             * game view unless the source-owned entrance completed with a
             * concrete handoff command.  The prior QUIT-only check treated
             * an asset/compositor failure (NONE) as success; the generic
             * post-open draw then exposed the party-less, black dungeon.
             * That is neither an original title/Entrance sequence nor a
             * playable game.  FIRESTAFF_EXIT_AFTER_LAUNCH remains a narrow
             * boot-probe escape hatch and deliberately does not claim an
             * interactive entrance handoff. */
            if (!getenv("FIRESTAFF_EXIT_AFTER_LAUNCH")) {
                DM1_V1_StartupHandoffOutcome_PC34 entranceOutcome;
                DM1_V1_StartupHostApplyResult_PC34 entranceHostResult;
                DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34
                    entranceRuntimeHandoff;
                /* The FM Towns title is a native EDM/JDM program, but its
                 * first Hall page is still an interactive entrance state.
                 * Passing the PC plan's automatic timeout here consumed C200
                 * before the host event loop regained control: on fast
                 * machines that skipped the usable Entrance page and exposed
                 * a party-less black dungeon.  Require a fresh source-space
                 * command. The headless probe keeps its explicit, narrow
                 * timeout policy inside m11_wait_for_redmcsb_entrance_command.
                 */
                entranceCommand = m11_play_redmcsb_entrance_transition(
                    gameView, -1,
                    &entrancePlan.entrance_full_start_receipt,
                    &entrancePlan.media_receipt);
                if (!dm1_v1_fmtowns_startup_handoff_allows_gameplay(
                        0, (int)entranceCommand)) {
                    M11_GameView_Shutdown(gameView);
                    M11_GameView_Init(gameView);
                    m11_set_launch_failed_message(menuState);
                    return 0;
                }
                /* EDM/JDM owns the title pixels, but the shared Entrance
                 * command is what changes the loaded DM1 world into the
                 * empty-party Hall route.  Previously this native branch
                 * presented C004 and accepted C200, then left the generic
                 * pre-Entrance world live.  On the first Draw that exposed
                 * a party-less black dungeon instead of the Hall mirrors.
                 *
                 * Build the already source-locked common Entrance receipt
                 * from the command actually consumed above.  This does not
                 * replay a PC title or fabricate a Towns shortcut: it only
                 * applies the common post-Entrance runtime boundary that
                 * EDM/JDM reaches after its own title program. */
                memset(&entranceOutcome, 0, sizeof(entranceOutcome));
                memset(&entranceHostResult, 0, sizeof(entranceHostResult));
                memset(&entranceRuntimeHandoff, 0,
                       sizeof(entranceRuntimeHandoff));
                entranceOutcome.title_played = 1;
                entranceOutcome.entrance_command = (int)entranceCommand;
                entranceOutcome.action =
                    DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34;
                entranceOutcome.status = "FMTOWNS EDM/JDM ENTRANCE ENTER";
                entranceHostResult.handled = 1;
                if (!dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
                        "dm1", "dm1", &entranceOutcome,
                        &entranceHostResult, &entranceRuntimeHandoff) ||
                    !M11_GameView_ApplyDm1StartupRuntimeHandoff(
                        gameView, &entranceRuntimeHandoff)) {
                    M11_GameView_Shutdown(gameView);
                    M11_GameView_Init(gameView);
                    m11_set_launch_failed_message(menuState);
                    return 0;
                }
            }
        }
        menuState->launchRequested = 0;
        (void)M11_Render_SetPaletteLevel(0);
        if (idleAccumulatorMs) {
            *idleAccumulatorMs = 0;
        }
        M11_GameView_Draw(gameView,
                          M11_Render_GetFramebuffer(),
                          M11_FB_WIDTH,
                          M11_FB_HEIGHT);
        return 1;
    }
    m11_set_launch_failed_message(menuState);
    return 0;
}

/* Keep synchronous intro policy local to this launch, including failure exits. */
static int m11_open_requested_launch(M11_GameViewState* gameView,
                                     M12_StartupMenuState* menuState,
                                     uint32_t* idleAccumulatorMs,
                                     const char* dataDir,
                                     int bootProbe) {
    const M12_StartupMenuState* previousMenu = g_m11_intro_menu;
    M11_GameViewState* previousView = g_m11_intro_game_view;
    M11_AudioState* previousAudio = g_m11_intro_local_audio;
    Uint64 previousPausedMs = g_m11_intro_paused_ms;
    const int debug = g_m11_debug_startup_detail;
    const Uint64 startupStartedCounter = debug
        ? SDL_GetPerformanceCounter() : 0;
    double startupCpuStarted = 0.0;
    const int startupCpuStartedAvailable = debug &&
        m11_process_cpu_seconds(&startupCpuStarted);
    int result;
    if (menuState && menuState->launchRequested) {
        /* A previous session's pixels cannot authorize this launch, including
         * a failed attempt, restart, or a different selected media owner. */
        (void)M12_StartupMenu_SetDM1HoCPresentedCaptureReceipt(menuState, NULL);
    }
    g_m11_intro_menu = menuState;
    g_m11_intro_game_view = gameView;
    g_m11_intro_local_audio = NULL;
    g_m11_intro_paused_ms = 0U;
    /* Synchronous startup frames belong to this attempt as well. */
    ++g_m11_sync_launch_serial;
    result = m11_open_requested_launch_impl(gameView, menuState,
                                            idleAccumulatorMs, dataDir, bootProbe);
    /* All selected-launch routes converge here, including the DM1 source-owned
     * transaction that returns before the generic post-open path. Stop later
     * archive admission only after a live game view has been handed off; the
     * full return-to-menu rescan joins the worker before refreshing assets. */
    if (result > 0 && gameView && gameView->active && menuState &&
        menuState->deferredScanActive) {
        M12_StartupMenu_CancelDeferredScan(menuState);
    }
    if (debug) {
        const Uint64 startupEndedCounter = SDL_GetPerformanceCounter();
        const double startupElapsedMs = m11_counter_milliseconds(
            startupEndedCounter - startupStartedCounter);
        double startupCpuEnded = 0.0;
        const int startupCpuEndedAvailable =
            m11_process_cpu_seconds(&startupCpuEnded);
        uint64_t residentBytes = 0;
        uint64_t virtualBytes = 0;
        const int memoryAvailable = m11_process_memory_bytes(
            &residentBytes, &virtualBytes);
        double cpuPercent = -1.0;
        if (startupCpuStartedAvailable && startupCpuEndedAvailable &&
            startupElapsedMs > 0.0 && startupCpuEnded >= startupCpuStarted) {
            cpuPercent = (startupCpuEnded - startupCpuStarted) *
                         100000.0 / startupElapsedMs;
        }
        fprintf(stderr,
                "firestaff: startup-cost game=%s result=%d elapsed-ms=%.1f process-cpu-of-one-core-percent=",
                gameView && gameView->sourceId[0]
                    ? gameView->sourceId : "unknown",
                result, startupElapsedMs);
        if (cpuPercent >= 0.0) fprintf(stderr, "%.1f", cpuPercent);
        else fprintf(stderr, "unavailable");
        fprintf(stderr, " process-rss-bytes=");
        if (memoryAvailable) fprintf(stderr, "%llu",
            (unsigned long long)residentBytes);
        else fprintf(stderr, "unavailable");
        fprintf(stderr, " process-virtual-bytes=");
        if (memoryAvailable) fprintf(stderr, "%llu",
            (unsigned long long)virtualBytes);
        else fprintf(stderr, "unavailable");
        fprintf(stderr, "\n");
    }
    /* Source-faithful title and entrance helpers may run synchronously before
     * the main frame loop. A finite CLI duration still owns that run budget;
     * turn its expiry into a clean process exit rather than a launch error. */
    if (g_m11_phase_duration_expired) {
        if (menuState) {
            menuState->launchRequested = 0;
            menuState->shouldExit = 1;
        }
        result = 1;
    }
    g_m11_intro_menu = previousMenu;
    g_m11_intro_game_view = previousView;
    g_m11_intro_local_audio = previousAudio;
    g_m11_intro_paused_ms = previousPausedMs;
    return result;
}

/* The F31 Utility route is not a generic game launch and must not inherit
 * whatever CSB platform was last selected.  It first selects a matched FM
 * Towns package from M12's catalogue, then uses the regular verified C03
 * boot boundary before entering C06_CEDT. */
static int m11_open_csb_fmtowns_utility_from_menu(
    M11_GameViewState *gameView, M12_StartupMenuState *menuState,
    uint32_t *idleAccumulatorMs, const char *dataDir)
{
    int index;
    int count;

    if (!gameView || !menuState ||
        !menuState->csbFmtownsUtilityLaunchRequested) return 0;
    menuState->csbFmtownsUtilityLaunchRequested = 0;
    if (!m11_apply_architecture_override(menuState, "csb",
                                         M12_ARCH_FM_TOWNS)) {
        goto unavailable;
    }
    count = M12_StartupMenu_GetEntryCount();
    for (index = 0; index < count; ++index) {
        const M12_MenuEntry *entry = M12_StartupMenu_GetEntry(menuState, index);
        if (entry && entry->kind == M12_MENU_ENTRY_GAME && entry->gameId &&
            strcmp(entry->gameId, "csb") == 0) {
            menuState->activatedIndex = index;
            menuState->launchRequested = 1;
            if (m11_open_requested_launch(gameView, menuState,
                                          idleAccumulatorMs, dataDir, 0) &&
                M11_GameView_EnterCsbFmtownsUtility(gameView)) {
                return 1;
            }
            goto unavailable;
        }
    }
unavailable:
    menuState->launchRequested = 0;
    menuState->view = M12_MENU_VIEW_MESSAGE;
    menuState->messageLine1 = M12_StartupMenu_Translate(
        menuState, "CSB UTILITY DISK NOT READY");
    menuState->messageLine2 = M12_StartupMenu_Translate(
        menuState, "VERIFIED FM TOWNS CSB MEDIA REQUIRED");
    menuState->messageLine3 = M12_StartupMenu_Translate(
        menuState, "ESC RETURNS TO MENU");
    return 0;
}

static int m11_open_csb_hint_oracle_from_menu(
    M11_GameViewState *gameView, M12_StartupMenuState *menuState)
{
    const char *data_dir;
    if (!gameView || !menuState || !menuState->csbHintOracleLaunchRequested)
        return 0;
    data_dir = M12_StartupMenu_AssetDataDir(menuState);
    if (!data_dir || !data_dir[0] ||
        /* The selected archive can contain the complete original R1 triplet.
         * Let the source-owned Hint Oracle ingress discover and materialize
         * its hash-admitted MINI.DAT instead of requiring a loose sibling. */
        !M11_GameView_StartCsbHintOracle(gameView, data_dir, NULL)) {
        menuState->csbHintOracleLaunchRequested = 0;
        menuState->view = M12_MENU_VIEW_MESSAGE;
        menuState->messageLine1 = M12_StartupMenu_Translate(
            menuState, "CSB UTILITY DISK NOT READY");
        menuState->messageLine2 = M12_StartupMenu_Translate(
            menuState, "ATARI R1 HCSB.HTC, HCSB.DAT AND MINI.DAT REQUIRED");
        menuState->messageLine3 = M12_StartupMenu_Translate(
            menuState, "ESC RETURNS TO MENU");
        return 0;
    }
    menuState->csbHintOracleLaunchRequested = 0;
    return 1;
}

static int m11_restart_current_launch(M11_GameViewState* gameView,
                                      M12_StartupMenuState* menuState,
                                      uint32_t* idleAccumulatorMs,
                                      const char* dataDir) {
    if (!gameView || !menuState) {
        return 0;
    }
    /* ReDMCSB ENDGAME.C F0444 lines 568-590 observes
     * G0523_B_RestartGameRequested after queue processing and returns to
     * the top-level load/start path.  Firestaff mirrors that as a full
     * teardown followed by the same selected-entry launch handoff used by
     * the modern launcher. */
    M11_GameView_Shutdown(gameView);
    M11_GameView_Init(gameView);
    menuState->launchRequested = 1;
    return m11_open_requested_launch(gameView,
                                     menuState,
                                     idleAccumulatorMs,
                                     dataDir,
                                     0);
}

int M11_PrepareDirectLaunchForGame(M12_StartupMenuState* menuState,
                                   const char* gameId) {
    int i;
    int entryCount;
    int gameOptionsSlot = M12_StartupMenu_GetGameOptionsSlot(gameId);
    if (!menuState || !gameId || gameId[0] == '\0' ||
        gameOptionsSlot < 0 ||
        gameOptionsSlot >= M12_CONFIG_GAME_COUNT) {
        return 0;
    }
    entryCount = M12_StartupMenu_GetEntryCount();
    for (i = 0; i < entryCount; ++i) {
        const M12_MenuEntry* entry = M12_StartupMenu_GetEntry(menuState, i);
        if (entry &&
            entry->kind == M12_MENU_ENTRY_GAME &&
            entry->gameId &&
            strcmp(entry->gameId, gameId) == 0) {
            M12_LaunchIntent intent;
            if (!M12_AssetStatus_GameAvailable(&menuState->assetStatus, gameId)) {
                return 0;
            }
            menuState->selectedIndex = i;
            menuState->activatedIndex = i;
            menuState->launchRequested = 1;
            /* `--game` requests this game directly. A save discovered from
             * launcher config is only a Resume candidate and must not turn
             * that request into an implicit load. The explicit --save path
             * is bound to the launch intent later in Phase A. */
            menuState->quickResumeLaunchRequested = 0;
            /* A direct CLI launch still goes through the M12 launch intent,
             * whose generic V2.2 gate quite rightly refuses CSB unless every
             * reviewed CSB material is installed.  The CSB presentation
             * runtime already has the supported V2.2 -> V2.1 resolution for
             * this exact case.  Consume it before asking M12 for the intent,
             * so --game csb --presentation-mode v22 starts in V2.1 rather
             * than reporting the verified game data as unavailable. */
            if (menuState->gameOptions[gameOptionsSlot].presentationModeIndex ==
                M12_PRESENTATION_V22_MODERN) {
                int resolveToV21 = 0;
                if (strcmp(gameId, "csb") == 0) {
                    const char* dataDir = M12_AssetStatus_GetRuntimeDataDir(
                        &menuState->assetStatus, gameId);
                    csb_v22_set_manifest_path(dataDir);
                    csb_v22_famg_set_manifest_path(dataDir);
                    csb_v2_presentation_mode_set_m12(
                        M12_PRESENTATION_V22_MODERN);
                    resolveToV21 = csb_v2_presentation_mode_get() ==
                        CSB_V2_PM_V21_UPSCALED;
                } else if (strcmp(gameId, "dm1") != 0 &&
                           strcmp(gameId, "nexus") != 0) {
                    /* M12 only admits a native V2.2 pack for DM1/Nexus.
                     * A persisted global V2.2 choice must not make an
                     * otherwise verified direct launch of another game fail. */
                    resolveToV21 = 1;
                }
                if (resolveToV21) {
                    menuState->settings.graphicsIndex =
                        M12_PRESENTATION_V21_UPSCALED;
                    menuState->gameOptions[gameOptionsSlot].presentationModeIndex =
                        M12_PRESENTATION_V21_UPSCALED;
                }
            }
            if (gameId && strcmp(gameId, "theron") == 0) {
                /* Resolve the same version that the launch intent will use.
                 * The catalogue's first match can be a different edition
                 * from an explicit or AUTO-selected version, which would
                 * bind campaign media from one release to another. */
                M12_LaunchIntent selectedIntent =
                    M12_StartupMenu_GetLaunchIntent(menuState);
                const M12_AssetVersionStatus* version =
                    selectedIntent.options.versionIndex >= 0
                    ? M12_AssetStatus_GetVersion(
                          &menuState->assetStatus, gameId,
                          (size_t)selectedIntent.options.versionIndex)
                    : NULL;
                const char* mediaPath =
                    selectedIntent.options.versionIndex >= 0
                    ? M12_AssetStatus_GetTheronLaunchMediaPathForVersion(
                          &menuState->assetStatus,
                          (size_t)selectedIntent.options.versionIndex)
                    : NULL;
                if (version && version->matchedPath[0] != '\0' &&
                    version->matchedMd5[0] != '\0' &&
                    mediaPath && strstr(version->matchedPath, "::") == NULL) {
                    M12_StartupMenu_ScanTheronCampaignMedia(
                        menuState, mediaPath, version->matchedMd5, NULL);
                }
            }
            intent = M12_StartupMenu_GetLaunchIntent(menuState);
            if (!intent.valid) {
                M12_StartupLaunchGate gate;
                memset(&gate, 0, sizeof(gate));
                if (M12_StartupMenu_GetLaunchGate(menuState, i, &gate) &&
                    gate.blockedLabel && gate.blockedDetail) {
                    fprintf(stderr,
                            "firestaff: direct launch blocked: %s: %s\n",
                            gate.blockedLabel, gate.blockedDetail);
                }
                menuState->launchRequested = 0;
                return 0;
            }
            return 1;
        }
    }
    return 0;
}

static int m11_boot_probe_expected_source_kind(const char* gameId,
                                               M11_GameSourceKind* outKind) {
    if (!gameId || !outKind) {
        return 0;
    }
    if (strcmp(gameId, "dm1") == 0) {
        *outKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
        return 1;
    }
    if (strcmp(gameId, "csb") == 0) {
        *outKind = M11_GAME_SOURCE_CSB_BOOT;
        return 1;
    }
    if (strcmp(gameId, "dm2") == 0) {
        *outKind = M11_GAME_SOURCE_DM2_BOOT;
        return 1;
    }
    if (strcmp(gameId, "nexus") == 0) {
        *outKind = M11_GAME_SOURCE_NEXUS_DGN;
        return 1;
    }
    if (strcmp(gameId, "theron") == 0) {
        *outKind = M11_GAME_SOURCE_THERON_TRACK02;
        return 1;
    }
    return 0;
}

void M11_PhaseA_SetDefaultOptions(M11_PhaseA_Options* opts) {
    if (!opts) {
        return;
    }
    opts->windowWidth    = 960;
    opts->windowHeight   = 540;
    opts->scaleMode      = M11_SCALE_FIT;
    opts->scaleModeOverride = 0;
    opts->presentationModeOverride = -1;
    opts->windowModeOverride = -1;
    opts->vsyncOverride = -1;
    opts->musicEnabledOverride = -1;
    opts->languageOverride = -1;
    opts->durationMs     = -1;
    opts->presentEveryMs = 16;
    opts->script         = NULL;
    opts->dataDir        = NULL;
    opts->savePath       = NULL;
    opts->csbHintOracle  = 0;
    opts->csbFmtownsUtilityDisk = 0;
    opts->dm2EnglishCompanionPath = NULL;
    opts->gameId         = NULL;
    opts->architectureOverride = -1;
    opts->csbFmtownsJapanese = 0;
    opts->dm1FmtownsJapanese = 0;
    opts->directLaunch   = 0;
    opts->verbose = 0;
    opts->debug = 0;
    opts->debugFullRedraw = 0;
    /* --menu is an explicit CLI opt-out from direct launch.  Leaving this
     * field uninitialised made an ordinary `--game dm1` depend on stack
     * contents and intermittently return to M12 instead of starting the
     * selected game. */
    opts->menuRequested  = 0;
    opts->bootProbe      = 0;
    opts->bootProbeFrames = 0;
    opts->bootProbeExpectPhase = NULL;
    opts->bootProbeExpectRuntime = 0;
    opts->bootProbeExpectParty = 0;
    opts->bootProbeExpectPartyX = -1;
    opts->bootProbeExpectPartyY = -1;
    opts->bootProbeExpectPartyDir = -1;
    opts->bootProbeExpectChampions = 0;
    opts->bootProbeExpectChampionCount = -1;
    opts->bootProbeExpectLevelLoaded = -1;
    opts->bootProbeExpectAssetMd5 = NULL;
    opts->bootProbeExpectMap = 0;
    opts->bootProbeExpectMapIndex = -1;
    opts->bootProbeExpectRuntimeTickMin = -1;
    opts->bootProbeExpectRuntimeTickMax = -1;
    opts->bootProbeExpectStartupActive = -1;
    opts->bootProbeExpectStartupFrameMin = -1;
    opts->bootProbeExpectStartupFrameMax = -1;
    opts->bootProbeExpectStartupAnimation = NULL;
    opts->bootProbeExpectStartupAnimationActive = -1;
    opts->bootProbeExpectTitleFrameMin = -1;
    opts->bootProbeExpectTitleFrameMax = -1;
    opts->bootProbeExpectTitleFrameBoundary = -1;
    opts->bootProbeExpectTitleReady = -1;
    opts->bootProbeExpectDm1HoCFullGraphics = 0;
    opts->bootProbeExpectDm1HoCReleaseAppCapture = 0;
    opts->retroAchievementsEnabled = 0;
    opts->showFpsOverlay = 0;
    opts->retroAchievementsHardcore = 1;
    opts->retroAchievementsUser = NULL;
    opts->retroAchievementsToken = NULL;
    opts->retroAchievementsEndpoint = NULL;
}

static void m11_phase_a_advance_boot_probe_frames(M11_GameViewState* gameView,
                                                  int frameCount) {
    int i;
    if (!gameView || !gameView->active || frameCount <= 0) {
        return;
    }
    for (i = 0; i < frameCount; ++i) {
        M11_GameInputResult result = M11_GameView_AdvanceIdleTick(gameView);
        m11_debug_game_events(gameView);
        if ((result == M11_GAME_INPUT_REDRAW &&
             /* FM Towns DM2 advances TWANIM from its Timer-A receipt in
              * AdvanceIdleTick.  Rendering every intermediate title tick
              * needlessly reparses the same retained GDAT plan and makes a
              * deterministic source-time boot probe exceed its timeout.
              * The final draw below still publishes the exact post-tick
              * source frame before the next scripted input.  Do not extend
              * this to DOS MVE or Macintosh Title.MooV: those streams own
              * their frame advance from Draw. */
             !(gameView->bootProbeFastForward &&
               gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
               (gameView->dm2FmtownsTitleBound ||
                gameView->dm2FmtownsTitleFinished))) ||
            i == frameCount - 1 ||
            /* A real Macintosh QuickTime frame advances during Draw, not
             * during the generic DM2 idle tick.  A boot probe is explicitly
             * deterministic fast-forward work, so draw each source frame
             * while Title.MooV owns startup.  Normal interactive playback
             * never sets this flag and remains wall-clock paced. */
            (gameView->bootProbeFastForward && gameView->dm2MacMovieActive)) {
            M11_GameView_Draw(gameView,
                              M11_Render_GetFramebuffer(),
                              M11_FB_WIDTH,
                              M11_FB_HEIGHT);
        }
    }
}

/* Select a source-owned architecture in the same M12 version catalogue used
 * by the visible game-options screen.  The version index is chosen from the
 * catalogue, preferring a matched hash but retaining the first source entry
 * when media is absent so the menu can show the requested platform as
 * unavailable instead of silently falling back to PC. */
static int m11_apply_architecture_override(M12_StartupMenuState* menuState,
                                           const char* gameId,
                                           int architecture) {
    static const char* const gameIds[] = {
        "dm1", "csb", "dm2", "nexus", "theron"
    };
    size_t gameStart = 0U;
    size_t gameEnd = sizeof(gameIds) / sizeof(gameIds[0]);
    size_t gameIndex;
    int applied = 0;

    if (!menuState || architecture <= M12_ARCH_AUTO ||
        architecture >= M12_ARCH_COUNT ||
        (architecture == M12_ARCH_PC98 &&
         (!gameId || strcmp(gameId, "dm2") != 0)) ||
        architecture == M12_ARCH_X68000 ||
        (gameId && strcmp(gameId, "csb") == 0 &&
         architecture == M12_ARCH_PC)) {
        return 0;
    }
    if (gameId && gameId[0] != '\0') {
        gameStart = gameEnd;
        for (gameIndex = 0U; gameIndex < gameEnd; ++gameIndex) {
            if (strcmp(gameIds[gameIndex], gameId) == 0) {
                gameStart = gameIndex;
                gameEnd = gameIndex + 1U;
                break;
            }
        }
        if (gameStart == sizeof(gameIds) / sizeof(gameIds[0])) {
            return 0;
        }
    }
    for (gameIndex = gameStart; gameIndex < gameEnd; ++gameIndex) {
        const char* id = gameIds[gameIndex];
        size_t count = M12_AssetStatus_GetVersionCount(id);
        size_t versionIndex;
        int first = -1;
        int matched;
        for (versionIndex = 0U; versionIndex < count; ++versionIndex) {
            if (M12_AssetStatus_GetVersionArchitecture(id, versionIndex) !=
                    architecture ||
                (strcmp(id, "csb") == 0 &&
                 architecture == M12_ARCH_PC)) {
                continue;
            }
            if (first < 0) first = (int)versionIndex;
        }
        /* Version discovery order is not a launch policy. In particular,
         * CSB A31E is scanner-visible but intentionally lacks its native
         * APPB.C03 handoff; the status policy selects a verified A31M/A35
         * sibling when one exists. */
        matched = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
            &menuState->assetStatus, id, architecture);
        if (first < 0) continue;
        menuState->gameOptions[gameIndex].architectureIndex = architecture;
        menuState->gameOptions[gameIndex].versionIndex =
            matched >= 0 ? matched : first;
        applied++;
    }
    return applied;
}

/* F31E and F31J share the FM Towns architecture, so selecting the platform
 * alone deliberately follows the ordinary matched-version policy (English
 * first). The CLI needs a separate, exact edition request for a user who has
 * the Japanese retail package. Never infer this from host locale or generic
 * language settings: it selects only the authenticated CSB catalogue row. */
static int m11_apply_csb_fmtowns_japanese_override(
    M12_StartupMenuState* menuState)
{
    static const char* const game_ids[M12_CONFIG_GAME_COUNT] = {
        "dm1", "csb", "dm2", "nexus", "theron"
    };
    int game_index;
    size_t version_count;
    size_t version_index;
    if (!menuState) return 0;
    for (game_index = 0; game_index < M12_CONFIG_GAME_COUNT; ++game_index) {
        if (strcmp(game_ids[game_index], "csb") == 0) break;
    }
    if (game_index >= M12_CONFIG_GAME_COUNT) return 0;
    version_count = M12_AssetStatus_GetVersionCount("csb");
    for (version_index = 0u; version_index < version_count; ++version_index) {
        const M12_AssetVersionStatus* version =
            M12_AssetStatus_GetVersion(&menuState->assetStatus, "csb",
                                       version_index);
        if (!version || !version->matched || !version->versionId ||
            strcmp(version->versionId, "fmtowns-ja") != 0) continue;
        menuState->gameOptions[game_index].architectureIndex = M12_ARCH_FM_TOWNS;
        menuState->gameOptions[game_index].versionIndex = (int)version_index;
        return 1;
    }
    return 0;
}

static int m11_apply_dm1_fmtowns_japanese_override(
    M12_StartupMenuState* menuState)
{
    size_t version_count;
    size_t version_index;
    if (!menuState) return 0;
    version_count = M12_AssetStatus_GetVersionCount("dm1");
    for (version_index = 0u; version_index < version_count; ++version_index) {
        const M12_AssetVersionStatus* version =
            M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1",
                                       version_index);
        if (!version || !version->matched || !version->versionId ||
            strcmp(version->versionId, "fmtowns-ja") != 0) continue;
        menuState->gameOptions[0].architectureIndex = M12_ARCH_FM_TOWNS;
        menuState->gameOptions[0].versionIndex = (int)version_index;
        return 1;
    }
    return 0;
}

static int m11_apply_auto_architecture_override(M12_StartupMenuState* menuState,
                                                const char* gameId) {
    static const char* const gameIds[M12_CONFIG_GAME_COUNT] = {
        "dm1", "csb", "dm2", "nexus", "theron"
    };
    int gameIndex;
    int matchedVersion;

    if (!menuState || !gameId || gameId[0] == '\0') {
        return 0;
    }
    for (gameIndex = 0; gameIndex < M12_CONFIG_GAME_COUNT; ++gameIndex) {
        if (strcmp(gameIds[gameIndex], gameId) == 0) {
            break;
        }
    }
    if (gameIndex >= M12_CONFIG_GAME_COUNT) {
        return 0;
    }
    matchedVersion = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
        &menuState->assetStatus, gameId, M12_ARCH_AUTO);
    if (matchedVersion < 0) {
        return 0;
    }
    /* An explicit AUTO command is a policy reset.  Do not retain a stale
     * persisted FM Towns/PC choice after the user selects a different data
     * directory.  The asset scan has already chosen the authenticated
     * version according to the game's AUTO priority. */
    menuState->gameOptions[gameIndex].architectureIndex = M12_ARCH_AUTO;
    menuState->gameOptions[gameIndex].versionIndex = matchedVersion;
    return 1;
}

static int m11_script_next_token(const char** cursor,
                                 const char** outStart,
                                 size_t* outLen) {
    const char* start;
    const char* end;
    if (outStart) {
        *outStart = NULL;
    }
    if (outLen) {
        *outLen = 0U;
    }
    if (!cursor || !*cursor) {
        return 0;
    }
    start = *cursor;
    while (*start == ' ' || *start == ',') {
        ++start;
    }
    if (*start == '\0') {
        *cursor = start;
        return 0;
    }
    end = start;
    while (*end != '\0' && *end != ',') {
        ++end;
    }
    *cursor = end;
    if (outStart) {
        *outStart = start;
    }
    if (outLen) {
        *outLen = (size_t)(end - start);
    }
    return 1;
}

static int m11_script_wait_frames(const char* token, size_t len) {
    unsigned long value = 0UL;
    size_t pos = 0U;
    if (!token || len == 0U) {
        return -1;
    }
    if (len >= 4U && strncmp(token, "wait", 4U) == 0) {
        pos = 4U;
        if (pos < len && token[pos] == ':') {
            ++pos;
        }
    } else if (len >= 6U && strncmp(token, "frames", 6U) == 0) {
        pos = 6U;
        if (pos < len && token[pos] == ':') {
            ++pos;
        }
    } else {
        return -1;
    }
    if (pos >= len) {
        return -1;
    }
    while (pos < len) {
        if (token[pos] < '0' || token[pos] > '9') {
            return -1;
        }
        value = value * 10UL + (unsigned long)(token[pos] - '0');
        if (value > 100000UL) {
            value = 100000UL;
        }
        ++pos;
    }
    return (int)value;
}

static int m11_phase_a_apply_boot_probe_script(M11_GameViewState* gameView,
                                               const char* script,
                                               int framesAfterInput,
                                               int* outWaitFrames) {
    const char* cursor = script;
    int applied = 0;
    int waited = 0;
    if (outWaitFrames) {
        *outWaitFrames = 0;
    }
    if (!gameView || !gameView->active || !script || script[0] == '\0') {
        return 0;
    }
    while (cursor && *cursor != '\0') {
        const char* token = NULL;
        size_t tokenLen = 0U;
        int waitFrames;
        M12_MenuInput input;
        M11_GameInputResult result;
        if (!m11_script_next_token(&cursor, &token, &tokenLen)) {
            break;
        }
        waitFrames = m11_script_wait_frames(token, tokenLen);
        if (waitFrames >= 0) {
            m11_phase_a_advance_boot_probe_frames(gameView, waitFrames);
            waited += waitFrames;
            continue;
        }
        if (m11_apply_boot_probe_event_token(gameView, token, tokenLen, &result)) {
            applied++;
            if (result == M11_GAME_INPUT_REDRAW) {
                M11_GameView_Draw(gameView,
                                  M11_Render_GetFramebuffer(),
                                  M11_FB_WIDTH,
                                  M11_FB_HEIGHT);
            }
            if (result == M11_GAME_INPUT_RETURN_TO_MENU ||
                result == M11_GAME_INPUT_RESTART_GAME) {
                break;
            }
            m11_phase_a_advance_boot_probe_frames(gameView, framesAfterInput);
            waited += framesAfterInput > 0 ? framesAfterInput : 0;
            continue;
        }
        input = m11_map_script_token(token, tokenLen);
        if (input == M12_MENU_INPUT_NONE) {
            continue;
        }
        result = M11_GameView_HandleInput(gameView, input);
        m11_debug_game_action(gameView, input, result);
        applied++;
        if (result == M11_GAME_INPUT_REDRAW) {
            M11_GameView_Draw(gameView,
                              M11_Render_GetFramebuffer(),
                              M11_FB_WIDTH,
                              M11_FB_HEIGHT);
        }
        if (result == M11_GAME_INPUT_RETURN_TO_MENU ||
            result == M11_GAME_INPUT_RESTART_GAME) {
            break;
        }
        m11_phase_a_advance_boot_probe_frames(gameView, framesAfterInput);
        waited += framesAfterInput > 0 ? framesAfterInput : 0;
    }
    if (outWaitFrames) {
        *outWaitFrames = waited;
    }
    return applied;
}

static void m11_phase_a_print_boot_probe_receipt(
    const M11_GameViewState* gameView,
    const M12_StartupMenuState* menuState,
    const char* gameId,
    int advancedFrames,
    int scriptInputs,
    int scriptFrames) {
    M11_BootProbeReceipt receipt;
    const char* runtimeDir = "";
    const char* platformHandoff = "none";
    const char* fmtownsProgram = "";
    const char* fmtownsProgramMd5 = "";
    int fmtownsMenuSelectsProgram = 0;
    DM2_V1_RuntimeGraphicsSetSceneReceipt dm2Scene;
    int dm2SceneReady = 0;

    memset(&dm2Scene, 0, sizeof(dm2Scene));
    int theronTrack02ItemNameBanks = 0;
    int theronTrack02ItemNameVariant = 0;
    int theronDungeonLevelsLoaded = 0;
    int theronDungeonSourceHeaders = 0;
    size_t theronDungeonSourceNonzeroTiles = 0u;
    size_t theronSelectedItemNameBytes = 0u;
    uint32_t theronSelectedItemNameFnv1a = 0u;
    if (menuState && gameId && gameId[0] != '\0') {
        runtimeDir = M12_AssetStatus_GetRuntimeDataDir(&menuState->assetStatus,
                                                       gameId);
    }
    if (!runtimeDir) {
        runtimeDir = "";
    }
    /* Asset status can point at another installed edition's extracted
     * directory even after DM2's hash admission has selected a Macintosh
     * archive. Report the boot profile's owner so diagnostics and CLI tests
     * identify the data actually backing the running game. */
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
        gameView->dm2BootProfile) {
        const DM2_V1_BootProfile *profile =
            (const DM2_V1_BootProfile *)gameView->dm2BootProfile;
        if (profile->asset_root[0] != '\0')
            runtimeDir = profile->asset_root;
    }
    if (!M11_GameView_GetBootProbeReceipt(gameView, &receipt)) {
        fprintf(stderr,
                "FIRESTAFF BOOT PROBE READY: gameId=%s sourceKind=%d sourceId=%s dataDir=%s frames=%d inputs=%d scriptFrames=%d\n",
                gameId ? gameId : "",
                gameView ? (int)gameView->sourceKind : 0,
                gameView ? gameView->sourceId : "",
                runtimeDir,
                advancedFrames,
                scriptInputs,
                scriptFrames);
        return;
    }
    /* An FM Towns GRAPHICS.DAT hash and shared DM1 runtime phase do not prove
     * the original Towns launcher route.  Expose the source-bound TMENU
     * selection and selected EDM/JDM executable receipt so CLI verification
     * cannot promote the common runtime path as platform-native by itself. */
    if (gameView && gameView->dm1FmtownsStartupReceiptValid &&
        dm1_v1_fmtowns_startup_receipt_is_native(
            &gameView->dm1FmtownsStartupReceipt)) {
        platformHandoff =
            gameView->dm1FmtownsStartupReceipt.language == DM1_FMTOWNS_LANG_JP
                ? "fmtowns-tmenu-jdm" : "fmtowns-tmenu-edm";
        fmtownsProgram = gameView->dm1FmtownsStartupReceipt.game_program_name;
        fmtownsProgramMd5 = gameView->dm1FmtownsStartupReceipt.game_program_md5;
        fmtownsMenuSelectsProgram =
            gameView->dm1FmtownsStartupReceipt.menu_info_selects_game;
    }
    if (gameView && gameView->dm1IigsActive) {
        platformHandoff = gameView->dm1IigsInstallation.metadata.program_version ==
            DM1_V1_IIGS_PROGRAM_G20 ? "iigs-g20-native" : "iigs-g21-native";
    }
    if (gameView && receipt.sourceKind == M11_GAME_SOURCE_DM2_BOOT) {
        dm2SceneReady = dm2_v1_runtime_graphicsset_scene_receipt(&dm2Scene);
    }
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02 &&
        gameView->theronWorld) {
        const Theron_V1_World *world =
            (const Theron_V1_World *)gameView->theronWorld;
        unsigned int i;
        int level_index;
        for (i = 0u; i < THERON_DUNGEON_COUNT; ++i) {
            if (world->track02_item_names[i].valid) {
                ++theronTrack02ItemNameBanks;
                if (!theronTrack02ItemNameVariant)
                    theronTrack02ItemNameVariant =
                        world->track02_item_names[i].variant;
            }
        }
        if (world->current_dungeon >= THERON_DUNGEON_1_AKUTUBA &&
            world->current_dungeon <= THERON_DUNGEON_COUNT) {
            int dungeon_index = world->current_dungeon - 1;
            for (level_index = 0;
                 level_index < THERON_MAX_LEVELS_PER_DUNGEON;
                 ++level_index) {
                const Theron_V1_Level *level;
                int y, x;
                if (!world->level_loaded[dungeon_index][level_index]) continue;
                ++theronDungeonLevelsLoaded;
                level = &world->levels[dungeon_index][level_index];
                if (level->source_header_verified)
                    ++theronDungeonSourceHeaders;
                for (y = 0; y < level->height; ++y)
                    for (x = 0; x < level->width; ++x)
                        if (level->source_tiles[y][x] != 0u)
                            ++theronDungeonSourceNonzeroTiles;
            }
        }
        if (gameView->inventorySelectedSlot >= 0 &&
            gameView->inventorySelectedSlot < THERON_INVENTORY_SLOTS &&
            world->party.active_slot >= 0 &&
            world->party.active_slot < THERON_MAX_CHAMPIONS) {
            const Theron_V1_InventorySourceRecord *item =
                &world->inventory_source[world->party.active_slot]
                                        [gameView->inventorySelectedSlot];
            const uint8_t *name = NULL;
            size_t name_size = 0u;
            if (item->valid && item->source_origin_valid &&
                (theron_v1_world_inventory_source_track19_item_name_raw(
                     world, world->party.active_slot,
                     gameView->inventorySelectedSlot, &name, &name_size) ||
                 theron_v1_world_inventory_source_track02_item_name_raw(
                     world, world->party.active_slot,
                     gameView->inventorySelectedSlot, &name, &name_size))) {
                uint32_t hash = 2166136261u;
                size_t at;
                for (at = 0u; at < name_size; ++at) {
                    hash ^= name[at];
                    hash *= 16777619u;
                }
                theronSelectedItemNameBytes = name_size;
                theronSelectedItemNameFnv1a = hash;
            }
        }
    }
    {
        DM1_V1_StartupHoCBootProbeLogReceipt_PC34 dm1Log;
        memset(&dm1Log, 0, sizeof(dm1Log));
        dm1_v1_startup_hoc_boot_probe_log_receipt_pc34(
            &receipt.dm1HoCBootSummary,
            &dm1Log);
    fprintf(stderr,
            "FIRESTAFF BOOT PROBE READY: gameId=%s sourceKind=%d sourceId=%s assetMd5=%s dataDir=%s frames=%d inputs=%d scriptFrames=%d window=%dx%d windowMode=%d presentationMode=%d presentation=%dx%d phase=%s startupActive=%d startupFrame=%d startupAnimation=%s startupAnimationActive=%d titleFrame=%d titleFrameMax=%d titleReady=%d levelLoaded=%d map=%d party=%d,%d,%d champions=%d runtimeTick=%d dm2FrameAccepted=%d dm2RealAssets=%d dm2NoCoreFallbacks=%d dm2FallbackDraws=%d dm2SceneReady=%d dm2GraphicsSet=%d dm2SceneHash=%u dm2SceneColorKey=%u dm2SceneFlags=%u dm2ScenePaletteHash=%u theronSourceObjects=%u theronDungeonLevelsLoaded=%d theronDungeonSourceHeaders=%d theronDungeonSourceNonzeroTiles=%zu theronActiveChampion=%d theronTrack01CddaReady=%d theronSpawnSourceAuthenticated=%d theronSpawnSourceVariant=%d theronTrack02ItemNameBanks=%d theronTrack02ItemNameVariant=%d theronSelectedItemNameBytes=%zu theronSelectedItemNameFnv1a=%u theronTrack19NameBankReady=%d theronTrack19NameVariant=%d theronTrack19ItemMappingProven=%d csbViewportHash=%u csbV22CellsPainted=%d dm1WorldTick=%u dm1HocCandidatePanel=%d dm1HocCandidateOrdinal=%d dm1HocCandidatePartyIndex=%d dm1InventoryPanel=%d dm1FoodWaterPanel=%d dm1FmtownsCddaPlaying=%d dm1FmtownsCddaTrack=%d startedFromLauncher=%d introBypassed=%d dm1StartupHandoffExecuted=%d platformHandoff=%s fmtownsProgram=%s fmtownsProgramMd5=%s fmtownsMenuSelectsProgram=%d dm1FmtownsMenuFontLoaded=%d %s\n",
            gameId ? gameId : "",
            (int)receipt.sourceKind,
            receipt.sourceId,
            receipt.bootAssetMd5,
            runtimeDir,
            advancedFrames,
            scriptInputs,
            scriptFrames,
            M11_Render_GetWindowWidth(),
            M11_Render_GetWindowHeight(),
            M11_Render_GetWindowMode(),
            receipt.presentationMode,
            receipt.presentationWidth,
            receipt.presentationHeight,
            receipt.startupPhase,
            receipt.startupActive,
            receipt.startupFrame,
            receipt.startupAnimation,
            receipt.startupAnimationActive,
            receipt.startupTitleFrame,
            receipt.startupTitleFrameMax,
            receipt.startupTitleReady,
            receipt.levelLoaded,
            receipt.mapIndex,
            receipt.partyX,
            receipt.partyY,
            receipt.partyDir,
            receipt.championCount,
            receipt.runtimeTick,
            receipt.dm2RuntimeFrameAccepted,
            receipt.dm2RuntimeRealAssetsReady,
            receipt.dm2RuntimeNoCoreFallbacks,
            receipt.dm2RuntimeFallbackDrawCount,
            dm2SceneReady,
            dm2Scene.map_graphics_style,
            (unsigned int)dm2Scene.hash,
            (unsigned int)dm2Scene.scene_colorkey,
            (unsigned int)dm2Scene.scene_flags,
            (unsigned int)dm2Scene.interface_palette_hash,
            gameView && gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02 &&
                    gameView->theronWorld
                ? ((const Theron_V1_World *)gameView->theronWorld)
                      ->source_object_count
                : 0u,
            theronDungeonLevelsLoaded,
            theronDungeonSourceHeaders,
            theronDungeonSourceNonzeroTiles,
            gameView && gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02 &&
                    gameView->theronWorld
                ? ((const Theron_V1_World *)gameView->theronWorld)
                      ->party.active_slot
                : -1,
            receipt.theronTrack01CddaReady,
            gameView && gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02 &&
                    gameView->theronWorld
                ? ((const Theron_V1_World *)gameView->theronWorld)
                      ->track02_spawn_source.authenticated
                : 0,
            gameView && gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02 &&
                    gameView->theronWorld
                ? ((const Theron_V1_World *)gameView->theronWorld)
                      ->track02_spawn_source_variant
                : 0,
            theronTrack02ItemNameBanks,
            theronTrack02ItemNameVariant,
            theronSelectedItemNameBytes,
            (unsigned int)theronSelectedItemNameFnv1a,
            gameView && gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02 &&
                    gameView->theronWorld
                ? ((const Theron_V1_World *)gameView->theronWorld)
                      ->track19_item_names.valid
                : 0,
            gameView && gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02 &&
                    gameView->theronWorld
                ? ((const Theron_V1_World *)gameView->theronWorld)
                      ->track19_item_names.variant
                : 0,
            gameView && gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02 &&
                    gameView->theronWorld
                ? ((const Theron_V1_World *)gameView->theronWorld)
                      ->track19_item_names.item_mapping_proven
                : 0,
            gameView ? (unsigned int)gameView->csbState.runtime_viewport_pixel_hash : 0u,
            gameView ? gameView->csbState.runtime_v22_cells_painted : 0,
            (unsigned int)receipt.dm1WorldTick,
            gameView ? gameView->candidateMirrorPanelActive : 0,
            gameView ? gameView->candidateMirrorOrdinal : -1,
            gameView ? gameView->candidateMirrorPartyIndex : -1,
            gameView ? gameView->inventoryPanelActive : 0,
            gameView ? gameView->v1FoodWaterPanelActive : 0,
            gameView ? gameView->dm1FmtownsCddaPlaying : 0,
            gameView ? gameView->dm1FmtownsCddaCurrentTrack : 0,
            receipt.startedFromLauncher,
            receipt.dm1StartupIntroBypassed,
            receipt.dm1StartupHandoffExecuted,
            platformHandoff,
            fmtownsProgram,
            fmtownsProgramMd5,
            fmtownsMenuSelectsProgram,
            gameView ? gameView->dm1FmtownsMenuFontLoaded : 0,
            dm1Log.fields[0] ? dm1Log.fields : "dm1HoCBootSummary=missing");
    }
}

static int m11_boot_probe_expected_phase_is_runtime(const char *phase) {
    size_t len;
    const char *suffix = "-runtime";
    size_t suffix_len = 8U;
    if (!phase) {
        return 0;
    }
    len = strlen(phase);
    if (len == 7U && strcmp(phase, "runtime") == 0) {
        return 1;
    }
    return len > suffix_len &&
           strcmp(phase + len - suffix_len, suffix) == 0;
}

static int m11_boot_probe_iigs_source_ready(const M11_GameViewState* view,
    const M11_BootProbeReceipt* receipt, const char* expectedGame) {
    return view && receipt && expectedGame && strcmp(expectedGame, "dm1") == 0 &&
        view->dm1IigsActive && view->dm1IigsDrawCount > 0 &&
        view->assetLoader.iigsDm1 && view->world.iigsCreatureProfileBound &&
        view->originalFontAvailable && view->originalFont.iigsNative &&
        (view->dm1IigsInstallation.metadata.program_version == DM1_V1_IIGS_PROGRAM_G20 ||
         view->dm1IigsInstallation.metadata.program_version == DM1_V1_IIGS_PROGRAM_G21) &&
        receipt->active && receipt->startedFromLauncher && receipt->levelLoaded &&
        receipt->sourceKind == M11_GAME_SOURCE_BUILTIN_CATALOG &&
        strcmp(receipt->sourceId, "dm1") == 0 &&
        strcmp(receipt->bootAssetMd5, "1045a36952e64eb2bc7b4c7b2965b112") == 0 &&
        strcmp(view->assetLoader.graphicsDatPath,
               view->dm1IigsInstallation.graphics_path) == 0;
}

static int m11_boot_probe_runtime_receipt_ready(
    const M11_BootProbeReceipt *receipt)
{
    return receipt &&
           receipt->active &&
           !receipt->startupActive &&
           receipt->levelLoaded;
}

static void m11_write_autotest_runtime_probe(const char* path,
                                             int launchedEver,
                                             const M11_GameViewState* gameView,
                                             const char* scriptCursor,
                                             int scriptWaitFramesRemaining,
                                             int inputRedrawDrawCount,
                                             int inputRedrawAfterViewportDirtyCount,
                                             int lastInputRedrawAfterViewportDirty) {
    FILE* f;
    M11_BootProbeReceipt startupReceipt;
    int startupReceiptReady;
    int dm2Platform = -1;
    const char *dm1FmtownsProgram = "";
    const char *dm1FmtownsProgramMd5 = "";
    int dm1FmtownsMenuSelectsProgram = 0;
    int outputDimensionsReady = 0;
    int windowWidth = 0;
    int windowHeight = 0;
    int drawableWidth = 0;
    DM2_V1_RuntimeFrameOwnershipReceipt dm2FrameOwnership = { 0 };
    DM2_V1_ViewportM11FrameReceipt dm2M11FrameReceipt = { 0 };
    int drawableHeight = 0;
    if (!path || path[0] == '\0') {
        return;
    }
    f = fopen(path, "w");
    if (!f) {
        return;
    }
    memset(&startupReceipt, 0, sizeof(startupReceipt));
    startupReceiptReady = M11_GameView_GetBootProbeReceipt(gameView,
                                                            &startupReceipt);
    outputDimensionsReady = M11_Render_GetWindowAndDrawableSize(
        &windowWidth, &windowHeight, &drawableWidth, &drawableHeight);
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
        gameView->dm2BootProfile) {
        dm2Platform = ((const DM2_V1_BootProfile *)gameView->dm2BootProfile)->platform;
        (void)dm2_v1_runtime_last_frame_ownership(&dm2FrameOwnership);
        (void)dm2_v1_runtime_last_m11_frame_receipt(&dm2M11FrameReceipt);
    }
    if (gameView && gameView->dm1FmtownsStartupReceiptValid &&
        dm1_v1_fmtowns_startup_receipt_is_native(
            &gameView->dm1FmtownsStartupReceipt)) {
        dm1FmtownsProgram =
            gameView->dm1FmtownsStartupReceipt.game_program_name;
        dm1FmtownsProgramMd5 =
            gameView->dm1FmtownsStartupReceipt.game_program_md5;
        dm1FmtownsMenuSelectsProgram =
            gameView->dm1FmtownsStartupReceipt.menu_info_selects_game;
    }
    /* Theron keeps its source-owned party/runtime state in theronState;
     * M11_GameViewState.world is the shared DM1-compatible shell and remains
     * zeroed for this game.  Reporting the shell here made an authenticated
     * Track 02 dungeon handoff look like a parked startup in readiness
     * receipts. */
    {
        int mapIndex = gameView ? gameView->world.party.mapIndex : -1;
        int mapX = gameView ? gameView->world.party.mapX : -1;
        int mapY = gameView ? gameView->world.party.mapY : -1;
        int direction = gameView ? gameView->world.party.direction : -1;
        int championCount = gameView ? gameView->world.party.championCount : -1;
        unsigned int gameTick = gameView ? (unsigned int)gameView->world.gameTick : 0U;
        const char *lastOutcome = gameView ? gameView->lastOutcome : "";
        if (gameView && gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02) {
            mapIndex = gameView->theronState.selected_dungeon - 1;
            mapX = gameView->theronState.party_x;
            mapY = gameView->theronState.party_y;
            direction = gameView->theronState.party_dir;
            championCount = gameView->theronState.companion_count +
                (gameView->theronState.level_loaded ? 1 : 0);
            gameTick = (unsigned int)gameView->theronState.tick_count;
            if (gameView->theronState.startup_phase ==
                    THERON_STARTUP_PHASE_IN_DUNGEON &&
                gameView->theronState.level_loaded) {
                /* This label is derived from the source-owned state receipt;
                 * it is not a claim that the missing creature/AI semantics
                 * have been reconstructed. */
                lastOutcome = "THERON RUNTIME (TRACK 02 DUNGEON)";
            }
        }
        /* Nexus title/asset boot is deliberately not a playable dungeon
         * state.  Keep the JSON receipt source-faithful: zero is a valid
         * coordinate, so it must not stand in for an unknown Saturn start
         * pose while LEV01 admission is blocked. */
        if (gameView && gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN &&
            !gameView->nexusState.level_loaded) {
            mapIndex = -1;
            mapX = -1;
            mapY = -1;
            direction = -1;
            championCount = 0;
        }
        fprintf(f,
            "{\n"
            "  \"schema\": \"firestaff_m11_autotest_runtime_probe.v1\",\n"
            "  \"launchedEver\": %d,\n"
            "  \"active\": %d,\n"
            "  \"title\": \"%s\",\n"
            "  \"sourceId\": \"%s\",\n"
            "  \"bootAssetMd5\": \"%s\",\n"
            "  \"dm1FmtownsStartup\": {\"program\": \"%s\", \"programMd5\": \"%s\", \"menuSelectsProgram\": %d},\n"
            "  \"dm2Startup\": {\"platform\": %d, \"movieActive\": %d, \"movieComplete\": %d, \"movieRejected\": %d, \"movieFrame\": %u},\n"
            "  \"dm2RuntimeFrame\": {\"accepted\": %d, \"realAssets\": %d, \"noCoreFallbacks\": %d, \"fallbackDraws\": %d},\n"
            "  \"dm2FrameOwnership\": {\"runtimeOwned\": %d, \"fullGdatValid\": %d, \"valid\": %d, \"floorCeilingBlits\": %d, \"floorCeilingRequiredMask\": %u, \"floorCeilingConsumedMask\": %u, \"floorCeilingPaletteBlits\": %d, \"wallBlits\": %d, \"hudBlits\": %d, \"sceneControlConsumed\": %d, \"blockedMaterials\": %d, \"fallbackDraws\": %d},\n"
            "  \"dm2M11Frame\": {\"valid\": %d, \"floorCeilingComplete\": %d, \"mapLoadToken\": %u, \"sceneControlHash\": %u, \"sceneLightHash\": %u, \"floorMaterialHash\": %u, \"ceilingMaterialHash\": %u, \"wallPlanHash\": %u, \"presentationStateHash\": %u},\n"
            "  \"dm2LiveMissileDrawSchedule\": {\"valid\": %d, \"partial\": %d, \"calls\": %d},\n"
            "  \"dm2FmtownsStartup\": {\"titleBound\": %d, \"titleFinished\": %d, \"titleRejected\": %d, \"swooshActive\": %d, \"frameIndex\": %u, \"frameCount\": %u, \"frameTicksRemaining\": %u, \"timerAccumulatorUs\": %u},\n"
            "  \"csbViewportHash\": %u,\n"
            "  \"script\": {\"waitFramesRemaining\": %d, \"pending\": %d},\n"
            "  \"presentation\": {\"mode\": %d, \"width\": %d, \"height\": %d},\n"
            "  \"outputSize\": {\"valid\": %d, \"windowWidth\": %d, \"windowHeight\": %d, \"drawableWidth\": %d, \"drawableHeight\": %d},\n"
            "  \"startup\": {\"receiptReady\": %d, \"phase\": \"%s\", \"active\": %d, \"startupActive\": %d, \"levelLoaded\": %d, \"dm1StartupHandoffExecuted\": %d, \"dm1StartupTitleRuntimeSource\": %d, \"dm1StartupTitleCompletedSteps\": %u, \"dm1StartupSwshConsumed\": %d, \"dm1StartupTitleConsumed\": %d, \"dm1StartupEntranceConsumed\": %d, \"dm1StartupFullGraphicsConsumed\": %d, \"dm1StartupHoCFirstFrameReady\": %d, \"dm1CompleteEntranceToHoC\": %d, \"dm1StartupPartyPlacement\": {\"executed\": %d, \"destinationGroupDeleted\": %d, \"sensorEffectCount\": %d, \"mapIndex\": %d, \"mapX\": %d, \"mapY\": %d}},\n"
            "  \"lastAction\": \"%s\",\n"
            "  \"lastOutcome\": \"%s\",\n"
            "  \"gameTick\": %u,\n"
            "  \"party\": {\"mapIndex\": %d, \"mapX\": %d, \"mapY\": %d, \"direction\": %d, \"championCount\": %d},\n"
            "  \"dm1HoC\": {\"candidatePanel\": %d, \"candidateOrdinal\": %d, \"candidatePartyIndex\": %d},\n"
            "  \"pipeline\": {\"dequeued\": %d, \"command\": %d, \"turnApplied\": %d, \"stepApplied\": %d, \"movementBlocked\": %d, \"anyMovementOccurred\": %d, \"anyTurnOccurred\": %d, \"viewportDirty\": %d},\n"
            "  \"redraw\": {\"inputRedrawDrawCount\": %d, \"inputRedrawAfterViewportDirtyCount\": %d, \"lastInputRedrawAfterViewportDirty\": %d}\n"
            "}\n",
            launchedEver,
            gameView ? gameView->active : 0,
            gameView ? gameView->title : "",
            gameView ? gameView->sourceId : "",
            startupReceipt.bootAssetMd5,
            dm1FmtownsProgram,
            dm1FmtownsProgramMd5,
            dm1FmtownsMenuSelectsProgram,
            dm2Platform,
            gameView ? gameView->dm2MacMovieActive : 0,
            gameView ? gameView->dm2MacMovieComplete : 0,
            gameView ? gameView->dm2MacMovieRejected : 0,
            gameView ? gameView->dm2MacMovieDecoder.frame_index : 0u,
            startupReceipt.dm2RuntimeFrameAccepted,
            startupReceipt.dm2RuntimeRealAssetsReady,
            startupReceipt.dm2RuntimeNoCoreFallbacks,
            startupReceipt.dm2RuntimeFallbackDrawCount,
            dm2FrameOwnership.runtime_frame_owned,
            dm2FrameOwnership.full_gdat_frame_valid,
            dm2FrameOwnership.valid,
            dm2FrameOwnership.floor_ceiling_gdat_blits,
            (unsigned int)dm2FrameOwnership.floor_ceiling_material_required_mask,
            (unsigned int)dm2FrameOwnership.floor_ceiling_material_consumed_mask,
            dm2FrameOwnership.gdat_material_palette_floor_ceiling_consumed,
            dm2FrameOwnership.wall_gdat_blits,
            dm2FrameOwnership.hud_gdat_blits,
            dm2FrameOwnership.gdat_scene_control_consumed,
            dm2FrameOwnership.blocked_material_draws,
            dm2FrameOwnership.total_runtime_fallback_draws,
            dm2M11FrameReceipt.valid,
            dm2M11FrameReceipt.floor_ceiling_materials_complete,
            (unsigned int)dm2M11FrameReceipt.map_load_token,
            (unsigned int)dm2M11FrameReceipt.scene_control_hash,
            (unsigned int)dm2M11FrameReceipt.scene_light_hash,
            (unsigned int)dm2M11FrameReceipt.floor_material_hash,
            (unsigned int)dm2M11FrameReceipt.ceiling_material_hash,
            (unsigned int)dm2M11FrameReceipt.wall_material_plan_hash,
            (unsigned int)dm2M11FrameReceipt.presentation_state_hash,
            dm2M11FrameReceipt.composition
                .indoor_live_missile_draw_plan_valid,
            dm2M11FrameReceipt.composition
                .indoor_live_missile_draw_plan_partial,
            dm2M11FrameReceipt.composition
                .indoor_live_missile_draw_call_count,
            gameView ? gameView->dm2FmtownsTitleBound : 0,
            gameView ? gameView->dm2FmtownsTitleFinished : 0,
            gameView ? gameView->dm2FmtownsTitleRejected : 0,
            gameView ? gameView->dm2FmtownsSwooshActive : 0,
            gameView ? gameView->dm2FmtownsTitleFrameIndex : 0u,
            gameView ? gameView->dm2FmtownsFrameCount : 0u,
            gameView ? gameView->dm2FmtownsFrameTimerARemaining : 0u,
            gameView ? gameView->dm2FmtownsTimerAAccumulatorUs : 0u,
            gameView ? (unsigned int)gameView->csbState.runtime_viewport_pixel_hash : 0u,
            scriptWaitFramesRemaining,
            scriptCursor && *scriptCursor != '\0' ? 1 : 0,
            gameView ? gameView->presentationMode : -1,
            gameView ? gameView->presentationWidth : 0,
            gameView ? gameView->presentationHeight : 0,
            outputDimensionsReady,
            windowWidth,
            windowHeight,
            drawableWidth,
            drawableHeight,
            startupReceiptReady,
            startupReceipt.startupPhase,
            startupReceipt.active,
            startupReceipt.startupActive,
            startupReceipt.levelLoaded,
            gameView ? gameView->dm1StartupHandoffExecuted : 0,
            gameView ? gameView->dm1StartupTitleRuntimeSource :
                       V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP,
            gameView ? gameView->dm1StartupTitleCompletedSteps : 0U,
            startupReceipt.dm1StartupSwshConsumed,
            startupReceipt.dm1StartupTitleConsumed,
            startupReceipt.dm1StartupEntranceConsumed,
            startupReceipt.dm1StartupFullGraphicsConsumed,
            gameView && gameView->dm1StartupRuntimeHandoffValid &&
                    gameView->dm1StartupRuntimeHandoffReceipt
                        .hoc_first_frame_ready
                ? 1
                : 0,
            startupReceipt.dm1CompleteEntranceToHoC,
            gameView ? gameView->dm1StartupPartyPlacementExecuted : 0,
            gameView ? gameView->dm1StartupPartyPlacementDestinationGroupDeleted : 0,
            gameView ? gameView->dm1StartupPartyPlacementSensorEffectCount : 0,
            gameView ? gameView->dm1StartupPartyPlacementMapIndex : 0,
            gameView ? gameView->dm1StartupPartyPlacementMapX : 0,
            gameView ? gameView->dm1StartupPartyPlacementMapY : 0,
            gameView ? gameView->lastAction : "",
            lastOutcome,
            gameTick,
            mapIndex,
            mapX,
            mapY,
            direction,
            championCount,
            gameView ? gameView->candidateMirrorPanelActive : 0,
            gameView ? gameView->candidateMirrorOrdinal : -1,
            gameView ? gameView->candidateMirrorPartyIndex : -1,
            gameView ? gameView->lastDm1V1MovementPipelineResult.core.queue.dequeued : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.core.queue.command : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.core.turnApplied : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.core.stepApplied : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.core.movementBlocked : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.anyMovementOccurred : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.anyTurnOccurred : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.viewportDirty : 0,
            inputRedrawDrawCount,
            inputRedrawAfterViewportDirtyCount,
            lastInputRedrawAfterViewportDirty);
        fclose(f);
    }
}

static void m11_write_autotest_screenshot(const char* outputDir) {
    char outPath[1024];
    if (!outputDir || outputDir[0] == '\0') {
        return;
    }
    if (!M11_Screenshot_CaptureCurrent(outputDir, outPath, (int)sizeof(outPath))) {
        fprintf(stderr, "firestaff: autotest screenshot capture failed: %s\n", outputDir);
        return;
    }
    fprintf(stderr, "AUTOTEST SCREENSHOT: %s\n", outPath);
}

static void m11_write_autotest_presented_screenshot(const char* outputDir) {
    char outPath[1024];
    if (!outputDir || outputDir[0] == '\0') {
        return;
    }
    if (!M11_Screenshot_CapturePresentedRGBA(outputDir, outPath, (int)sizeof(outPath))) {
        fprintf(stderr, "firestaff: autotest presented screenshot capture failed: %s\n", outputDir);
        return;
    }
    fprintf(stderr, "AUTOTEST PRESENTED SCREENSHOT: %s\n", outPath);
}

/* V2.1's EPX surface is the user-visible image. Capturing the indexed
 * source here would produce a valid-looking but unscaled BMP. */
static void m11_capture_user_screenshot(const M11_GameViewState* gameView,
                                        const M12_StartupMenuState* menuState) {
    const char* outputDir = NULL;
    char outPath[1024];
    int captured;

    if (!gameView || !gameView->active) {
        return;
    }
    if (menuState && menuState->settings.screenshotPath[0] != '\0') {
        outputDir = menuState->settings.screenshotPath;
    }
    /* TITLE.C/ENTRANCE.C have already expanded their source-owned indexed
     * page through a phase-specific VGA palette. Capturing the raw M11
     * framebuffer here would reinterpret C001--C005 with the live dungeon
     * palette, so the saved CSB screenshot would not match the window. */
    if (M11_GameView_GetPresentationSpecialPalette(gameView) >= 0) {
        captured = M11_Screenshot_CapturePresentedRGBA(outputDir,
                                                        outPath,
                                                        (int)sizeof(outPath));
    } else if (m11_game_view_is_dm1(gameView) &&
        gameView->presentationMode == M12_PRESENTATION_V21_UPSCALED) {
        /* V2.1 only enlarges DM1's original indexed graphics.  Do not turn
         * an asset-free fallback frame into a user-facing capture receipt. */
        if (!gameView->assetsAvailable || !gameView->assetLoader.fileState) {
            fprintf(stderr,
                    "SCREENSHOT FAILED: DM1 V2.1 requires loaded original assets\n");
            return;
        }
        captured = M11_Screenshot_CapturePresentedRGBA(outputDir,
                                                        outPath,
                                                        (int)sizeof(outPath));
    } else {
        captured = M11_Screenshot_CaptureCurrent(outputDir,
                                                  outPath,
                                                  (int)sizeof(outPath));
    }
    if (captured) {
        fprintf(stderr, "SCREENSHOT: %s\n", outPath);
    } else {
        fprintf(stderr, "SCREENSHOT FAILED\n");
    }
}

static M12_MenuInput m11_map_script_token(const char* token, size_t len) {
    if (!token || len == 0U) {
        return M12_MENU_INPUT_NONE;
    }
    if ((len == 2U && strncmp(token, "up", len) == 0) ||
        (len == 1U && strncmp(token, "u", len) == 0)) {
        return M12_MENU_INPUT_UP;
    }
    if ((len == 4U && strncmp(token, "down", len) == 0) ||
        (len == 1U && strncmp(token, "d", len) == 0)) {
        return M12_MENU_INPUT_DOWN;
    }
    if ((len == 11U && strncmp(token, "strafe-left", len) == 0) ||
        (len == 2U && strncmp(token, "sl", len) == 0)) {
        return M12_MENU_INPUT_STRAFE_LEFT;
    }
    if ((len == 12U && strncmp(token, "strafe-right", len) == 0) ||
        (len == 2U && strncmp(token, "sr", len) == 0)) {
        return M12_MENU_INPUT_STRAFE_RIGHT;
    }
    /* v2.8.x: script-token `left` keeps its historical turn-left
     * semantics so existing probe and replay-script code (pass373's
     * script, firestaff_m11_wall_collision_capture_probe,
     * firestaff_m11_turn_viewport_orientation_probe, etc.) keeps
     * working unchanged.  The user's keyboard-mapping request is
     * honoured at the SDL scancode layer: SDLK_LEFT maps to
     * STRAFE_LEFT (which feeds the gameplay pipeline through
     * M12_MENU_INPUT_STRAFE_LEFT, not M12_MENU_INPUT_LEFT).  Use
     * `strafe-left` or `sl` for the strafe-left replay token. */
    if ((len == 4U && strncmp(token, "left", len) == 0) ||
        (len == 1U && strncmp(token, "l", len) == 0)) {
        return M12_MENU_INPUT_TURN_LEFT;
    }
    /* v2.8.x: script-token `right` keeps its historical turn-right
     * semantics so pass373's `enter,down,down,down,down,down,down,enter,right`
     * script (and similar replay scripts in probe code) still drives
     * a turn.  Use `strafe-right` or `sr` for the strafe-right token
     * that the SDL scancode handler produces when SDLK_RIGHT is
     * pressed.  Same for `left` / `l` aliasing turn-left.  The
     * gameplay pipeline switch (m11_dm1_v1_pipeline_command_for_input
     * + m11_apply_tick) treats M12_MENU_INPUT_LEFT/RIGHT as turn so
     * script-token and probe-code paths both reach the same DM1 V1
     * command id (TURN_LEFT/RIGHT). */
    if ((len == 5U && strncmp(token, "right", len) == 0) ||
        (len == 1U && strncmp(token, "r", len) == 0)) {
        return M12_MENU_INPUT_TURN_RIGHT;
    }
    if ((len == 9U && strncmp(token, "turn-left", len) == 0) ||
        (len == 2U && strncmp(token, "tl", len) == 0) ||
        (len == 4U && strncmp(token, "home", len) == 0)) {
        return M12_MENU_INPUT_TURN_LEFT;
    }
    if ((len == 10U && strncmp(token, "turn-right", len) == 0) ||
        (len == 2U && strncmp(token, "tr", len) == 0) ||
        (len == 3U && strncmp(token, "end", len) == 0)) {
        return M12_MENU_INPUT_TURN_RIGHT;
    }
    if ((len == 5U && strncmp(token, "enter", len) == 0) ||
        (len == 6U && strncmp(token, "return", len) == 0)) {
        return M12_MENU_INPUT_ACCEPT;
    }
    if ((len == 5U && strncmp(token, "space", len) == 0) ||
        (len == 6U && strncmp(token, "action", len) == 0) ||
        (len == 3U && strncmp(token, "act", len) == 0)) {
        return M12_MENU_INPUT_ACTION;
    }
    if ((len == 3U && strncmp(token, "tab", len) == 0) ||
        (len == 5U && strncmp(token, "champ", len) == 0)) {
        return M12_MENU_INPUT_CYCLE_CHAMPION;
    }
    if ((len == 3U && strncmp(token, "esc", len) == 0) ||
        (len == 6U && strncmp(token, "escape", len) == 0) ||
        (len == 4U && strncmp(token, "back", len) == 0)) {
        return M12_MENU_INPUT_BACK;
    }
    if (len == 4U && strncmp(token, "rest", len) == 0) {
        return M12_MENU_INPUT_REST_TOGGLE;
    }
    if ((len == 6U && strncmp(token, "stairs", len) == 0) ||
        (len == 7U && strncmp(token, "descend", len) == 0)) {
        return M12_MENU_INPUT_USE_STAIRS;
    }
    if ((len == 4U && strncmp(token, "grab", len) == 0) ||
        (len == 6U && strncmp(token, "pickup", len) == 0) ||
        (len == 1U && strncmp(token, "g", len) == 0)) {
        return M12_MENU_INPUT_PICKUP_ITEM;
    }
    if ((len == 4U && strncmp(token, "drop", len) == 0) ||
        (len == 3U && strncmp(token, "put", len) == 0) ||
        (len == 1U && strncmp(token, "p", len) == 0)) {
        return M12_MENU_INPUT_DROP_ITEM;
    }
    if (len == 5U && strncmp(token, "rune1", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_1;
    if (len == 5U && strncmp(token, "rune2", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_2;
    if (len == 5U && strncmp(token, "rune3", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_3;
    if (len == 5U && strncmp(token, "rune4", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_4;
    if (len == 5U && strncmp(token, "rune5", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_5;
    if (len == 5U && strncmp(token, "rune6", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_6;
    if ((len == 4U && strncmp(token, "cast", len) == 0) ||
        (len == 5U && strncmp(token, "spell", len) == 0)) {
        return M12_MENU_INPUT_SPELL_CAST;
    }
    if (len == 5U && strncmp(token, "clear", len) == 0) return M12_MENU_INPUT_SPELL_CLEAR;
    if ((len == 3U && strncmp(token, "use", len) == 0) ||
        (len == 5U && strncmp(token, "drink", len) == 0) ||
        (len == 3U && strncmp(token, "eat", len) == 0)) {
        return M12_MENU_INPUT_USE_ITEM;
    }
    return M12_MENU_INPUT_NONE;
}

static int m11_script_keycode_from_name(const char* name) {
    if (!name || name[0] == '\0') {
        return 0;
    }
    if (strncmp(name, "command+", 8) == 0) {
        name += 8;
    }
    if (strcmp(name, "up") == 0) return SDLK_UP;
    if (strcmp(name, "down") == 0) return SDLK_DOWN;
    if (strcmp(name, "left") == 0) return SDLK_LEFT;
    if (strcmp(name, "right") == 0) return SDLK_RIGHT;
    if (strcmp(name, "home") == 0) return SDLK_HOME;
    if (strcmp(name, "end") == 0) return SDLK_END;
    if (strcmp(name, "enter") == 0 || strcmp(name, "return") == 0) return SDLK_RETURN;
    if (strcmp(name, "kp-enter") == 0) return SDLK_KP_ENTER;
    if (strcmp(name, "kp-1") == 0 || strcmp(name, "kp1") == 0) return SDLK_KP_1;
    if (strcmp(name, "kp-2") == 0 || strcmp(name, "kp2") == 0) return SDLK_KP_2;
    if (strcmp(name, "kp-3") == 0 || strcmp(name, "kp3") == 0) return SDLK_KP_3;
    if (strcmp(name, "kp-4") == 0 || strcmp(name, "kp4") == 0) return SDLK_KP_4;
    if (strcmp(name, "kp-5") == 0 || strcmp(name, "kp5") == 0) return SDLK_KP_5;
    if (strcmp(name, "kp-6") == 0 || strcmp(name, "kp6") == 0) return SDLK_KP_6;
    if (strcmp(name, "kp-7") == 0 || strcmp(name, "kp7") == 0) return SDLK_KP_7;
    if (strcmp(name, "kp-8") == 0 || strcmp(name, "kp8") == 0) return SDLK_KP_8;
    if (strcmp(name, "kp-9") == 0 || strcmp(name, "kp9") == 0) return SDLK_KP_9;
    if (strcmp(name, "space") == 0) return SDLK_SPACE;
    if (strcmp(name, "tab") == 0) return SDLK_TAB;
    if (strcmp(name, "esc") == 0 || strcmp(name, "escape") == 0) return SDLK_ESCAPE;
    /* ReDMCSB COMMAND.C:245-260 gives CSB its F1-F4 champion keys.  Let
     * real-media CLI probes replay the same SDL key route as live input. */
    if (strcmp(name, "f1") == 0) return SDLK_F1;
    if (strcmp(name, "f2") == 0) return SDLK_F2;
    if (strcmp(name, "f3") == 0) return SDLK_F3;
    if (strcmp(name, "f4") == 0) return SDLK_F4;
    if (strcmp(name, "f5") == 0) return SDLK_F5;
    if (strcmp(name, "f9") == 0) return SDLK_F9;
    if (strcmp(name, "f10") == 0) return SDLK_F10;
    if (strcmp(name, "f11") == 0) return SDLK_F11;
    /* The script grammar uses commas between tokens, so the Macintosh
     * lateral keys need spelled names to reach the same SDL keydown route. */
    if (strcmp(name, "comma") == 0) return SDLK_COMMA;
    if (strcmp(name, "period") == 0) return SDLK_PERIOD;
    if (name[1] == '\0') {
        switch (name[0]) {
            case 'a': return SDLK_A;
            case 'c': return SDLK_C;
            case 'd': return SDLK_D;
            case 'e': return SDLK_E;
            case 'g': return SDLK_G;
            case 'i': return SDLK_I;
            case 'j': return SDLK_J;
            case 'k': return SDLK_K;
            case 'l': return SDLK_L;
            case 'm': return SDLK_M;
            case 'p': return SDLK_P;
            case 'q': return SDLK_Q;
            case 'r': return SDLK_R;
            case 's': return SDLK_S;
            case 'u': return SDLK_U;
            case 'v': return SDLK_V;
            case 'w': return SDLK_W;
            case 'x': return SDLK_X;
            case 'z': return SDLK_Z;
            case '1': return SDLK_1;
            case '2': return SDLK_2;
            case '3': return SDLK_3;
            case '4': return SDLK_4;
            case '5': return SDLK_5;
            case '6': return SDLK_6;
            default: break;
        }
    }
    return 0x7fffffff;
}

static int m11_game_view_is_csb(const M11_GameViewState* gameView) {
    return gameView && gameView->active && strcmp(gameView->sourceId, "csb") == 0;
}

static int m11_game_view_is_dm1(const M11_GameViewState* gameView) {
    return gameView &&
           DM1_V1_InputSourceIsActivePc34Compat(gameView->active,
                                                gameView->sourceId);
}

static int m11_game_view_is_dm2_mac(const M11_GameViewState* gameView) {
    return gameView && gameView->active &&
           gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
           gameView->dm2BootProfile &&
           ((const DM2_V1_BootProfile *)gameView->dm2BootProfile)->platform ==
               DM2_PLATFORM_MAC_EN;
}

static int m11_game_view_is_theron(const M11_GameViewState* gameView) {
    return gameView && gameView->active &&
           gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02;
}

static int m11_game_view_supports_held_motion_input(
    const M11_GameViewState* gameView) {
    if (!gameView || !gameView->active) {
        return 0;
    }
    /* TITLE/ENTRANCE have their own source dispatch.  GAMELOOP.C's held
     * command wait begins only after F0441/F0435 has entered the dungeon, so
     * a held controller must not repeatedly advance startup commands. */
    if (m11_game_view_is_theron(gameView)) {
        return gameView->theronState.startup_phase ==
                   THERON_STARTUP_PHASE_IN_DUNGEON &&
               gameView->theronState.level_loaded;
    }
    /* Theron is identified by its authenticated Track 02 source kind above,
     * not by a launcher label.  Other source kinds retain the shared source
     * id gate below.  This keeps a verified regional/format-specific Theron
     * label from disabling normal held WASD/arrow input. */
    if (!M11_InputSourceSupportsHeldMotion(gameView->sourceId,
                                           gameView->active)) {
        if (gameView->sourceKind != M11_GAME_SOURCE_NEXUS_DGN ||
            !gameView->nexusState.level_loaded ||
            gameView->nexusState.title_active ||
            gameView->nexusState.startup_save_select_active ||
            gameView->nexusState.champion_select_active) {
            return 0;
        }
        return 1;
    }
    return !m11_game_view_is_csb(gameView) ||
           (!gameView->csbState.startup_title_active &&
            !gameView->csbState.startup_entrance_active);
}

static int m11_csb_sdl_key_to_menu_input(int key, int ctrlDown, M12_MenuInput* outInput) {
    CsbV1KeyboardKeyPc34Compat csbKey = CSB_V1_KEYBOARD_KEY_NONE;
    switch (key) {
        case SDLK_F1: csbKey = CSB_V1_KEYBOARD_KEY_F1; break;
        case SDLK_F2: csbKey = CSB_V1_KEYBOARD_KEY_F2; break;
        case SDLK_F3: csbKey = CSB_V1_KEYBOARD_KEY_F3; break;
        case SDLK_F4: csbKey = CSB_V1_KEYBOARD_KEY_F4; break;
        case SDLK_ESCAPE: csbKey = CSB_V1_KEYBOARD_KEY_ESCAPE; break;
        case SDLK_RETURN: csbKey = CSB_V1_KEYBOARD_KEY_RETURN; break;
        case SDLK_KP_ENTER: csbKey = CSB_V1_KEYBOARD_KEY_ENTER; break;
#if SDL_VERSION_ATLEAST(3, 0, 0)
        case SDLK_S:
#else
        case SDLK_s:
#endif
            csbKey = CSB_V1_KEYBOARD_KEY_S;
            break;
        case SDLK_INSERT: csbKey = CSB_V1_KEYBOARD_KEY_INSERT; break;
        case SDLK_UP: csbKey = CSB_V1_KEYBOARD_KEY_UP; break;
        case SDLK_HOME: csbKey = CSB_V1_KEYBOARD_KEY_CLR_HOME; break;
        case SDLK_LEFT: csbKey = CSB_V1_KEYBOARD_KEY_LEFT; break;
        case SDLK_DOWN: csbKey = CSB_V1_KEYBOARD_KEY_DOWN; break;
        case SDLK_RIGHT: csbKey = CSB_V1_KEYBOARD_KEY_RIGHT; break;
        default:
            return 0;
    }
    return CSB_V1_KeyboardCommandToMenuInputPc34Compat(csbKey, ctrlDown, outInput);
}

/* ReDMCSB COMMAND.C:579-610, MEDIA707_I34E/I34M.  Keep the original
 * interface keys ahead of Firestaff-only shortcuts in the live host route. */
static int m11_dm1_sdl_key_to_menu_input(int key, int ctrlDown, int shiftDown,
                                         M12_MenuInput* outInput) {
    if (!outInput) {
        return 0;
    }
    if (key == SDLK_ESCAPE) {
        /* DM1 uses Escape for the LOADSAVE.C return/quit route.  C147/C148
         * freeze is CSB-only, so routing DM1 through FREEZE_TOGGLE left the
         * game on a misleading frozen status instead of its source dialog. */
        *outInput = M12_MENU_INPUT_BACK;
        return 1;
    }
#if SDL_VERSION_ATLEAST(3, 0, 0)
    if (ctrlDown && key == SDLK_S) {
#else
    if (ctrlDown && key == SDLK_s) {
#endif
        *outInput = M12_MENU_INPUT_SAVE_GAME;
        return 1;
    }
    if (!shiftDown) {
        switch (key) {
            case SDLK_1: *outInput = M12_MENU_INPUT_CHAMPION_1_INVENTORY; return 1;
            case SDLK_2: *outInput = M12_MENU_INPUT_CHAMPION_2_INVENTORY; return 1;
            case SDLK_3: *outInput = M12_MENU_INPUT_CHAMPION_3_INVENTORY; return 1;
            case SDLK_4: *outInput = M12_MENU_INPUT_CHAMPION_4_INVENTORY; return 1;
            default: break;
        }
    }
    return 0;
}

/* The two admitted US Macintosh DM2 editions share this English input table.
 * Resolve it before the generic PC aliases so left/right and the keypad keep
 * their original Macintosh meaning. */
static int m11_dm2_mac_sdl_key_to_menu_input(
    const M11_GameViewState *gameView, const SDL_KeyboardEvent *keyEvent,
    M12_MenuInput *outInput)
{
    DM2_V1_MacInputReceipt receipt;
    DM2_V1_MacInputPhase phase = DM2_V1_MAC_INPUT_GAMEPLAY;
    uint32_t key;
    uint32_t modifiers = 0u;

    if (!gameView || !keyEvent || !outInput ||
        gameView->sourceKind != M11_GAME_SOURCE_DM2_BOOT ||
        !gameView->dm2BootProfile ||
        ((const DM2_V1_BootProfile *)gameView->dm2BootProfile)->platform !=
            DM2_PLATFORM_MAC_EN) {
        return 0;
    }
    /* The Macintosh table has separate entrance and credits owners.  The
     * old host adapter always asked for GAMEPLAY, which happened to turn
     * Return into the same host Accept token but lost the source action
     * identity and could admit gameplay keys while the modal movie/event
     * loop still owned the screen. */
    if (gameView->dm2State.startup_credits_active) {
        phase = DM2_V1_MAC_INPUT_CREDITS;
    } else if (gameView->dm2State.startup_menu_active) {
        phase = DM2_V1_MAC_INPUT_ENTRANCE;
    }
    key = (uint32_t)keyEvent->key;
    if (keyEvent->mod & SDL_KMOD_GUI) modifiers |= DM2_V1_MAC_MOD_COMMAND;

    switch (keyEvent->key) {
        case SDLK_RETURN: key = DM2_V1_MAC_KEY_RETURN; break;
        case SDLK_KP_ENTER: key = DM2_V1_MAC_KEY_ENTER; break;
        case SDLK_ESCAPE: key = DM2_V1_MAC_KEY_ESCAPE; break;
        case SDLK_UP: key = DM2_V1_MAC_KEY_UP; break;
        case SDLK_DOWN: key = DM2_V1_MAC_KEY_DOWN; break;
        case SDLK_LEFT: key = DM2_V1_MAC_KEY_LEFT; break;
        case SDLK_RIGHT: key = DM2_V1_MAC_KEY_RIGHT; break;
        case SDLK_HOME: key = DM2_V1_MAC_KEY_HOME; break;
        case SDLK_END: key = DM2_V1_MAC_KEY_END; break;
        case SDLK_PAGEUP: key = DM2_V1_MAC_KEY_PAGE_UP; break;
        case SDLK_PAGEDOWN: key = DM2_V1_MAC_KEY_PAGE_DOWN; break;
        case SDLK_HELP: key = DM2_V1_MAC_KEY_HELP; break;
        case SDLK_DELETE: key = DM2_V1_MAC_KEY_DELETE; break;
        case SDLK_KP_1: key = DM2_V1_MAC_KEY_NUMPAD_1; break;
        case SDLK_KP_2: key = DM2_V1_MAC_KEY_NUMPAD_2; break;
        case SDLK_KP_3: key = DM2_V1_MAC_KEY_NUMPAD_3; break;
        case SDLK_KP_4: key = DM2_V1_MAC_KEY_NUMPAD_4; break;
        case SDLK_KP_5: key = DM2_V1_MAC_KEY_NUMPAD_5; break;
        case SDLK_KP_6: key = DM2_V1_MAC_KEY_NUMPAD_6; break;
        case SDLK_KP_7: key = DM2_V1_MAC_KEY_NUMPAD_7; break;
        case SDLK_KP_8: key = DM2_V1_MAC_KEY_NUMPAD_8; break;
        case SDLK_KP_9: key = DM2_V1_MAC_KEY_NUMPAD_9; break;
        case SDLK_F1: key = DM2_V1_MAC_KEY_F1; break;
        case SDLK_F2: key = DM2_V1_MAC_KEY_F2; break;
        case SDLK_F3: key = DM2_V1_MAC_KEY_F3; break;
        case SDLK_F4: key = DM2_V1_MAC_KEY_F4; break;
        case SDLK_F13: key = DM2_V1_MAC_KEY_F13; break;
        case SDLK_F14: key = DM2_V1_MAC_KEY_F14; break;
        case SDLK_F15: key = DM2_V1_MAC_KEY_F15; break;
        default: break;
    }
    if (!dm2_v1_mac_input_resolve(key, modifiers, phase, &receipt)) return 0;
    switch (receipt.action) {
        case DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_0: *outInput = M12_MENU_INPUT_CHAMPION_1_INVENTORY; return 1;
        case DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_1: *outInput = M12_MENU_INPUT_CHAMPION_2_INVENTORY; return 1;
        case DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_2: *outInput = M12_MENU_INPUT_CHAMPION_3_INVENTORY; return 1;
        case DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_3: *outInput = M12_MENU_INPUT_CHAMPION_4_INVENTORY; return 1;
        case DM2_V1_MAC_ACTION_TOGGLE_LEADER: *outInput = M12_MENU_INPUT_LEADER_INVENTORY; return 1;
        case DM2_V1_MAC_ACTION_FREEZE: *outInput = M12_MENU_INPUT_FREEZE_TOGGLE; return 1;
        case DM2_V1_MAC_ACTION_SAVE_GAME: *outInput = M12_MENU_INPUT_SAVE_GAME; return 1;
        case DM2_V1_MAC_ACTION_OPEN_GAME: *outInput = M12_MENU_INPUT_MAC_OPEN_GAME; return 1;
        case DM2_V1_MAC_ACTION_QUIT: *outInput = M12_MENU_INPUT_BACK; return 1;
        case DM2_V1_MAC_ACTION_TURN_LEFT: *outInput = M12_MENU_INPUT_TURN_LEFT; return 1;
        case DM2_V1_MAC_ACTION_MOVE_FORWARD: *outInput = M12_MENU_INPUT_UP; return 1;
        case DM2_V1_MAC_ACTION_TURN_RIGHT: *outInput = M12_MENU_INPUT_TURN_RIGHT; return 1;
        case DM2_V1_MAC_ACTION_MOVE_LEFT: *outInput = M12_MENU_INPUT_STRAFE_LEFT; return 1;
        case DM2_V1_MAC_ACTION_MOVE_BACKWARD: *outInput = M12_MENU_INPUT_DOWN; return 1;
        case DM2_V1_MAC_ACTION_MOVE_RIGHT: *outInput = M12_MENU_INPUT_STRAFE_RIGHT; return 1;
        case DM2_V1_MAC_ACTION_WALL_LEFT: *outInput = M12_MENU_INPUT_MAC_WALL_LEFT; return 1;
        case DM2_V1_MAC_ACTION_WALL_CENTER: *outInput = M12_MENU_INPUT_MAC_WALL_CENTER; return 1;
        case DM2_V1_MAC_ACTION_WALL_RIGHT: *outInput = M12_MENU_INPUT_MAC_WALL_RIGHT; return 1;
        /* The retail Macintosh table assigns Return to the modal entrance
         * and credits owners before it means Wake in gameplay. Preserve that
         * source action identity through the M11 menu bridge: both owners
         * already consume Accept in their respective state machines. */
        case DM2_V1_MAC_ACTION_NEW_GAME:
        case DM2_V1_MAC_ACTION_CLOSE_CREDITS:
        case DM2_V1_MAC_ACTION_WAKE: *outInput = M12_MENU_INPUT_ACCEPT; return 1;
        default: return 0;
    }
}

static int m11_dm2_mac_key_event_is_held_motion(
    const M11_GameViewState *gameView, const SDL_KeyboardEvent *keyEvent) {
    M12_MenuInput input = M12_MENU_INPUT_NONE;
    return m11_game_view_is_dm2_mac(gameView) && keyEvent && keyEvent->repeat &&
           m11_dm2_mac_sdl_key_to_menu_input(gameView, keyEvent, &input) &&
           DM1_V1_InputMenuTokenUsesHeldRepeatPc34Compat((int)input);
}

static int m11_push_script_event_token(const char* token, size_t len) {
    char buffer[128];
    SDL_Event ev;
    int x = 0;
    int y = 0;
    if (!token || len == 0U || len >= sizeof(buffer)) {
        return 0;
    }
    memcpy(buffer, token, len);
    buffer[len] = '\0';
    memset(&ev, 0, sizeof(ev));

    if (strcmp(buffer, "touch-swipe-up") == 0 ||
        strcmp(buffer, "touch-swipe-left") == 0) {
        const int swipeUp = strcmp(buffer, "touch-swipe-up") == 0;
        const float startX = swipeUp ? 0.50f : 0.60f;
        const float startY = swipeUp ? 0.55f : 0.40f;
        const float endX = swipeUp ? 0.50f : 0.32f;
        const float endY = swipeUp ? 0.25f : 0.40f;
        int phase;
        for (phase = 0; phase < 3; ++phase) {
            const float x = phase == 0 ? startX : endX;
            const float y = phase == 0 ? startY : endY;
            memset(&ev, 0, sizeof(ev));
#if SDL_VERSION_ATLEAST(3, 0, 0)
            ev.type = phase == 0 ? SDL_EVENT_FINGER_DOWN
                     : phase == 1 ? SDL_EVENT_FINGER_MOTION
                                  : SDL_EVENT_FINGER_UP;
            ev.tfinger.touchID = 1;
            ev.tfinger.fingerID = 0;
#else
            ev.type = phase == 0 ? SDL_FINGERDOWN
                     : phase == 1 ? SDL_FINGERMOTION
                                  : SDL_FINGERUP;
            ev.tfinger.touchId = 1;
            ev.tfinger.fingerId = 0;
#endif
            ev.tfinger.x = x;
            ev.tfinger.y = y;
            ev.tfinger.pressure = phase == 2 ? 0.0f : 1.0f;
            SDL_PushEvent(&ev);
        }
        return 1;
    }

    if (strncmp(buffer, "key:", 4) == 0) {
        const char *keyName = buffer + 4;
        int commandModifier = strncmp(keyName, "command+", 8) == 0;
        if (commandModifier) keyName += 8;
#if SDL_VERSION_ATLEAST(3, 0, 0)
        ev.type = SDL_EVENT_KEY_DOWN;
        ev.key.key = (SDL_Keycode)m11_script_keycode_from_name(keyName);
        ev.key.scancode = SDL_GetScancodeFromKey(ev.key.key, NULL);
        if (commandModifier) ev.key.mod = SDL_KMOD_GUI;
#else
        ev.type = SDL_KEYDOWN;
        ev.key.keysym.sym = (SDL_Keycode)m11_script_keycode_from_name(keyName);
        ev.key.keysym.scancode = SDL_GetScancodeFromKey(ev.key.keysym.sym);
        if (commandModifier) ev.key.keysym.mod = KMOD_GUI;
#endif
        SDL_PushEvent(&ev);
        return 1;
    }
    if (sscanf(buffer, "rightdown:%d:%d", &x, &y) == 2 ||
        sscanf(buffer, "rightup:%d:%d", &x, &y) == 2) {
        const int isDown = strncmp(buffer, "rightdown:", 10) == 0;
#if SDL_VERSION_ATLEAST(3, 0, 0)
        ev.type = isDown ? SDL_EVENT_MOUSE_BUTTON_DOWN
                         : SDL_EVENT_MOUSE_BUTTON_UP;
        ev.button.button = SDL_BUTTON_RIGHT;
        ev.button.x = (float)x;
        ev.button.y = (float)y;
#else
        ev.type = isDown ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
        ev.button.button = SDL_BUTTON_RIGHT;
        ev.button.x = x;
        ev.button.y = y;
#endif
        SDL_PushEvent(&ev);
        return 1;
    }
    if (sscanf(buffer, "click:%d:%d", &x, &y) == 2) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        ev.button.button = SDL_BUTTON_LEFT;
        ev.button.x = (float)x;
        ev.button.y = (float)y;
#else
        ev.type = SDL_MOUSEBUTTONDOWN;
        ev.button.button = SDL_BUTTON_LEFT;
        ev.button.x = x;
        ev.button.y = y;
#endif
        SDL_PushEvent(&ev);
#if SDL_VERSION_ATLEAST(3, 0, 0)
        ev.type = SDL_EVENT_MOUSE_BUTTON_UP;
#else
        ev.type = SDL_MOUSEBUTTONUP;
#endif
        SDL_PushEvent(&ev);
        return 1;
    }
    if (sscanf(buffer, "move:%d:%d", &x, &y) == 2) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        ev.type = SDL_EVENT_MOUSE_MOTION;
        ev.motion.x = (float)x;
        ev.motion.y = (float)y;
#else
        ev.type = SDL_MOUSEMOTION;
        ev.motion.x = x;
        ev.motion.y = y;
#endif
        SDL_PushEvent(&ev);
        return 1;
    }
    return 0;
}

static int m11_script_event_token_is_valid(const char* token, size_t len) {
    char buffer[128];
    int x = 0;
    int y = 0;
    if (!token || len == 0U || len >= sizeof(buffer)) {
        return 0;
    }
    memcpy(buffer, token, len);
    buffer[len] = '\0';
    if (strcmp(buffer, "touch-swipe-up") == 0 ||
        strcmp(buffer, "touch-swipe-left") == 0) {
        return 1;
    }
    if (strncmp(buffer, "key:", 4) == 0) {
        const char *keyName = buffer + 4;
        if (strncmp(keyName, "command+", 8) == 0) keyName += 8;
        return keyName[0] != '\0' &&
               m11_script_keycode_from_name(keyName) != 0x7fffffff;
    }
    if (sscanf(buffer, "click:%d:%d", &x, &y) == 2) {
        return 1;
    }
    if (sscanf(buffer, "move:%d:%d", &x, &y) == 2) {
        return 1;
    }
    return 0;
}

static M12_MenuInput m11_boot_probe_key_input(const M11_GameViewState* gameView,
                                              int keycode) {
    M12_MenuInput csbInput = M12_MENU_INPUT_NONE;
    if (m11_game_view_is_csb(gameView) &&
        m11_csb_sdl_key_to_menu_input(keycode, 0, &csbInput)) {
        return csbInput;
    }
    if (m11_game_view_is_theron(gameView)) {
        const int gameplayInput =
            m11_game_view_supports_held_motion_input(gameView);
        M12_MenuInput theronInput = gameplayInput
            ? M11_TheronGameplayInputFromKeycode(keycode)
            : M11_TheronNavigationInputFromKeycode(keycode);
        if (theronInput != M12_MENU_INPUT_NONE) {
            return theronInput;
        }
    }
    switch (keycode) {
        case SDLK_UP: return M12_MENU_INPUT_UP;
        case SDLK_DOWN: return M12_MENU_INPUT_DOWN;
        case SDLK_LEFT: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_RIGHT: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_KP_5: return M12_MENU_INPUT_UP;
        case SDLK_KP_2: return M12_MENU_INPUT_DOWN;
        case SDLK_KP_1: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_KP_3: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_KP_4: return M12_MENU_INPUT_TURN_LEFT;
        case SDLK_KP_6: return M12_MENU_INPUT_TURN_RIGHT;
        case SDLK_Q: return M12_MENU_INPUT_TURN_LEFT;
        case SDLK_E: return M12_MENU_INPUT_TURN_RIGHT;
        case SDLK_HOME: return M12_MENU_INPUT_TURN_LEFT;
        case SDLK_END: return M12_MENU_INPUT_TURN_RIGHT;
        case SDLK_A: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_D: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_W: return M12_MENU_INPUT_UP;
        case SDLK_S: return M12_MENU_INPUT_DOWN;
        case SDLK_RETURN:
        case SDLK_KP_ENTER: return M12_MENU_INPUT_ACCEPT;
        case SDLK_ESCAPE: return M12_MENU_INPUT_BACK;
        case SDLK_SPACE: return M12_MENU_INPUT_ACTION;
        case SDLK_TAB: return M12_MENU_INPUT_CYCLE_CHAMPION;
        case SDLK_R: return gameView && gameView->active ? M12_MENU_INPUT_REST_TOGGLE : M12_MENU_INPUT_NONE;
        case SDLK_X: return gameView && gameView->active ? M12_MENU_INPUT_USE_STAIRS : M12_MENU_INPUT_NONE;
        case SDLK_G: return gameView && gameView->active ? M12_MENU_INPUT_PICKUP_ITEM : M12_MENU_INPUT_NONE;
        case SDLK_P: return gameView && gameView->active ? M12_MENU_INPUT_DROP_ITEM : M12_MENU_INPUT_NONE;
        case SDLK_1: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_1 : M12_MENU_INPUT_NONE;
        case SDLK_2: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_2 : M12_MENU_INPUT_NONE;
        case SDLK_3: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_3 : M12_MENU_INPUT_NONE;
        case SDLK_4: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_4 : M12_MENU_INPUT_NONE;
        case SDLK_5: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_5 : M12_MENU_INPUT_NONE;
        case SDLK_6: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_6 : M12_MENU_INPUT_NONE;
        case SDLK_C: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_CAST : M12_MENU_INPUT_NONE;
        case SDLK_V: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_CLEAR : M12_MENU_INPUT_NONE;
        case SDLK_U: return gameView && gameView->active ? M12_MENU_INPUT_USE_ITEM : M12_MENU_INPUT_NONE;
        case SDLK_M: return gameView && gameView->active ? M12_MENU_INPUT_MAP_TOGGLE : M12_MENU_INPUT_NONE;
        case SDLK_I: return gameView && gameView->active ? M12_MENU_INPUT_INVENTORY_TOGGLE : M12_MENU_INPUT_NONE;
        default: return M12_MENU_INPUT_NONE;
    }
}

static int m11_apply_boot_probe_event_token(M11_GameViewState* gameView,
                                            const char* token,
                                            size_t len,
                                            M11_GameInputResult* outResult) {
    char buffer[128];
    int x = 0;
    int y = 0;
    if (outResult) {
        *outResult = M11_GAME_INPUT_IGNORED;
    }
    if (!gameView || !gameView->active ||
        !token || len == 0U || len >= sizeof(buffer)) {
        return 0;
    }
    memcpy(buffer, token, len);
    buffer[len] = '\0';
    if (strncmp(buffer, "key:", 4) == 0) {
        int keycode = m11_script_keycode_from_name(buffer + 4);
        M12_MenuInput input = m11_boot_probe_key_input(gameView, keycode);
        if (keycode == SDLK_F5) {
            if (outResult) {
                *outResult = M11_GameView_QuickSave(gameView)
                    ? M11_GAME_INPUT_REDRAW : M11_GAME_INPUT_IGNORED;
            } else {
                (void)M11_GameView_QuickSave(gameView);
            }
            return 1;
        }
        if (input == M12_MENU_INPUT_NONE) {
            return 0;
        }
        if (outResult) {
            *outResult = M11_GameView_HandleInput(gameView, input);
            m11_debug_game_action(gameView, input, *outResult);
        } else {
            M11_GameInputResult inputResult =
                M11_GameView_HandleInput(gameView, input);
            m11_debug_game_action(gameView, input, inputResult);
        }
        return 1;
    }
    if (sscanf(buffer, "click:%d:%d", &x, &y) == 2) {
        M11_GameInputResult press_result;
        M11_GameInputResult release_result;
        /* The normal SDL event loop has already presented the current frame
         * before it maps a window point through its content rectangle.  A
         * boot-probe advances game state without drawing, which left the
         * renderer reporting its stale 320x200 V1 content size for V2.0/V2.1
         * clicks.  Present once here so the probe exercises the identical
         * 640x400/upscaled input geometry as a live CSB window. */
        if (!m11_present_game_frame(gameView, NULL)) {
            return 1;
        }
        if (!m11_map_window_pointer_to_game_source(gameView, x, y, &x, &y)) {
            return 1;
        }
        /* A script click represents the same press/release pair delivered
         * by SDL.  Some source-owned controls, including CSB Amiga's APPB
         * language panel, deliberately consume the release rather than the
         * initial press.  Sending only the press made that real path
         * unreachable through --boot-probe even though it worked in the app. */
        press_result = M11_GameView_HandlePointerButton(
            gameView, x, y, DM1_V1_MOUSE_MASK_LEFT_PC34);
        release_result = M11_GameView_HandlePointerButtonRelease(
            gameView, x, y, DM1_V1_MOUSE_MASK_LEFT_PC34);
        if (outResult) {
            *outResult = release_result != M11_GAME_INPUT_IGNORED
                ? release_result : press_result;
        }
        m11_debug_game_pointer(
            gameView, "pointer-click", x, y,
            release_result != M11_GAME_INPUT_IGNORED
                ? release_result : press_result);
        return 1;
    }
    if (sscanf(buffer, "move:%d:%d", &x, &y) == 2) {
        if (!m11_present_game_frame(gameView, NULL)) {
            return 1;
        }
        if (!m11_map_window_pointer_to_game_source(gameView, x, y, &x, &y)) {
            return 1;
        }
        if (outResult) {
            *outResult = M11_GameView_HandlePointerMove(gameView, x, y);
        } else {
            (void)M11_GameView_HandlePointerMove(gameView, x, y);
        }
        return 1;
    }
    return 0;
}

int M11_BootProbeScript_Validate(const char* script,
                                 char* firstInvalidOut,
                                 size_t firstInvalidOutSize) {
    const char* cursor = script;
    int invalid = 0;
    if (firstInvalidOut && firstInvalidOutSize > 0U) {
        firstInvalidOut[0] = '\0';
    }
    if (!script || script[0] == '\0') {
        return 0;
    }
    while (cursor && *cursor != '\0') {
        const char* token = NULL;
        size_t tokenLen = 0U;
        int recognized = 0;
        if (!m11_script_next_token(&cursor, &token, &tokenLen)) {
            break;
        }
        if (m11_script_wait_frames(token, tokenLen) >= 0 ||
            m11_script_event_token_is_valid(token, tokenLen) ||
            m11_map_script_token(token, tokenLen) != M12_MENU_INPUT_NONE) {
            recognized = 1;
        }
        if (!recognized) {
            if (invalid == 0 && firstInvalidOut && firstInvalidOutSize > 0U) {
                size_t copyLen = tokenLen;
                if (copyLen >= firstInvalidOutSize) {
                    copyLen = firstInvalidOutSize - 1U;
                }
                memcpy(firstInvalidOut, token, copyLen);
                firstInvalidOut[copyLen] = '\0';
            }
            ++invalid;
        }
    }
    return invalid;
}

static int m11_input_action_is_motion(int action) {
    switch (action) {
        case M11_ACTION_MOVE_FORWARD:
        case M11_ACTION_MOVE_BACKWARD:
        case M11_ACTION_TURN_LEFT:
        case M11_ACTION_TURN_RIGHT:
        case M11_ACTION_STRAFE_LEFT:
        case M11_ACTION_STRAFE_RIGHT:
            return 1;
        default:
            return 0;
    }
}

static M12_MenuInput m11_menu_input_for_m11_action(int action) {
    switch (action) {
        case M11_ACTION_MOVE_FORWARD:
            return M12_MENU_INPUT_UP;
        case M11_ACTION_MOVE_BACKWARD:
            return M12_MENU_INPUT_DOWN;
        case M11_ACTION_TURN_LEFT:
            return M12_MENU_INPUT_TURN_LEFT;
        case M11_ACTION_TURN_RIGHT:
            return M12_MENU_INPUT_TURN_RIGHT;
        case M11_ACTION_STRAFE_LEFT:
            return M12_MENU_INPUT_STRAFE_LEFT;
        case M11_ACTION_STRAFE_RIGHT:
            return M12_MENU_INPUT_STRAFE_RIGHT;
        default:
            return M12_MENU_INPUT_NONE;
    }
}

M12_MenuInput M11_DM1V1_NavigationInputFromScancode(int scancode) {
    /* ReDMCSB COMMAND.C:677-684 maps the PC34 navigation keys into the
     * same C068..C073 movement-arrow commands used by mouse clicks. Keep
     * the host keyboard route on those tokens so presentation can highlight
     * the exact clicked arrow zone. */
    switch (scancode) {
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_KP_5:
            return M12_MENU_INPUT_UP;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_KP_2:
            return M12_MENU_INPUT_DOWN;
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_KP_1:
            return M12_MENU_INPUT_STRAFE_LEFT;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_KP_3:
            return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDL_SCANCODE_HOME:
        case SDL_SCANCODE_Q:
        case SDL_SCANCODE_KP_4:
            return M12_MENU_INPUT_TURN_LEFT;
        case SDL_SCANCODE_END:
        case SDL_SCANCODE_E:
        case SDL_SCANCODE_KP_6:
            return M12_MENU_INPUT_TURN_RIGHT;
        default:
            return M12_MENU_INPUT_NONE;
    }
}

static int m11_dm1_v1_input_is_immediate_turn(M12_MenuInput input) {
    return DM1_V1_InputMenuTokenIsImmediateTurnPc34Compat((int)input);
}

static M12_MenuInput m11_motion_input_from_scancode(SDL_Scancode scancode) {
    int action = M11_Input_ActionForScancode(scancode);
    if (!m11_input_action_is_motion(action)) {
        switch (scancode) {
            case SDL_SCANCODE_KP_5:
                return M12_MENU_INPUT_UP;
            case SDL_SCANCODE_KP_2:
                return M12_MENU_INPUT_DOWN;
            case SDL_SCANCODE_KP_1:
                return M12_MENU_INPUT_STRAFE_LEFT;
            case SDL_SCANCODE_KP_3:
                return M12_MENU_INPUT_STRAFE_RIGHT;
            case SDL_SCANCODE_KP_4:
                return M12_MENU_INPUT_TURN_LEFT;
            case SDL_SCANCODE_KP_6:
                return M12_MENU_INPUT_TURN_RIGHT;
            default:
                return M12_MENU_INPUT_NONE;
        }
    }
    return m11_menu_input_for_m11_action(action);
}

static M12_MenuInput m11_gamepad_button_input(const M12_GamepadMap* map,
                                              SDL_GamepadButton button,
                                              int gameplayActive) {
    M12_InputAction action;
    if (!map || !map->enabled) return M12_MENU_INPUT_NONE;
    action = M12_GamepadMap_ActionForButton(map, button);
    if (action == M12_ACTION_COUNT) return M12_MENU_INPUT_NONE;
    return M11_GamepadActionToMenuInput(action, gameplayActive);
}

static M12_MenuInput m11_gamepad_axis_input(const M12_GamepadMap* map,
                                            SDL_GamepadAxis axis,
                                            int rawValue,
                                            int gameplayActive) {
    const M12_GamepadAxisConfig* cfg;
    int value;
    if (!map || !map->enabled) return M12_MENU_INPUT_NONE;
    cfg = M12_GamepadMap_GetAxisConfig(map, axis);
    if (!cfg) return M12_MENU_INPUT_NONE;
    value = M12_GamepadAxis_Process(cfg, rawValue);
    return M11_GamepadAxisToMenuInput(axis, cfg->role, value, gameplayActive);
}

static M12_MenuInput m11_held_motion_input_from_gamepad(
    const M11_GameViewState* gameView,
    const M12_GamepadStatus* gamepadStatus,
    const M12_GamepadMap* gamepadMap) {
    static const SDL_GamepadButton preferredButtons[] = {
        SDL_GAMEPAD_BUTTON_DPAD_UP,
        SDL_GAMEPAD_BUTTON_DPAD_DOWN,
        SDL_GAMEPAD_BUTTON_DPAD_LEFT,
        SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
        SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
        SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER
    };
    static const SDL_GamepadAxis preferredAxes[] = {
        SDL_GAMEPAD_AXIS_LEFTY,
        SDL_GAMEPAD_AXIS_LEFTX,
        SDL_GAMEPAD_AXIS_RIGHTX
    };
    size_t i;
    int gameplayActive = gameView && gameView->active;
    if (!gamepadStatus || !gamepadStatus->handle ||
        !gamepadMap || !gamepadMap->enabled) {
        return M12_MENU_INPUT_NONE;
    }
    for (i = 0; i < sizeof(preferredButtons) / sizeof(preferredButtons[0]); ++i) {
        SDL_GamepadButton button = preferredButtons[i];
        if (SDL_GetGamepadButton(gamepadStatus->handle, button)) {
            M12_MenuInput input =
                m11_gamepad_button_input(gamepadMap, button, gameplayActive);
            if (input != M12_MENU_INPUT_NONE) return input;
        }
    }
    for (i = 0; i < sizeof(preferredAxes) / sizeof(preferredAxes[0]); ++i) {
        SDL_GamepadAxis axis = preferredAxes[i];
        M12_MenuInput input =
            m11_gamepad_axis_input(gamepadMap,
                                   axis,
                                   SDL_GetGamepadAxis(gamepadStatus->handle, axis),
                                   gameplayActive);
        if (input != M12_MENU_INPUT_NONE) return input;
    }
    return M12_MENU_INPUT_NONE;
}

static M12_MenuInput m11_held_motion_input_from_keyboard(M11_GameViewState* gameView) {
    int count = 0;
    int i;
#if SDL_VERSION_ATLEAST(3, 0, 0)
    const bool* keys = SDL_GetKeyboardState(&count);
#else
    const Uint8* keys = SDL_GetKeyboardState(&count);
#endif
    static const SDL_Scancode preferred[] = {
        SDL_SCANCODE_UP,
        SDL_SCANCODE_W,
        SDL_SCANCODE_DOWN,
        SDL_SCANCODE_S,
        SDL_SCANCODE_LEFT,
        SDL_SCANCODE_A,
        SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_D,
        SDL_SCANCODE_HOME,
        SDL_SCANCODE_Q,
        SDL_SCANCODE_END,
        SDL_SCANCODE_E,
        SDL_SCANCODE_KP_5,
        SDL_SCANCODE_KP_8,
        SDL_SCANCODE_KP_2,
        SDL_SCANCODE_KP_1,
        SDL_SCANCODE_KP_3,
        SDL_SCANCODE_KP_4,
        SDL_SCANCODE_KP_6,
        SDL_SCANCODE_X,
        SDL_SCANCODE_Z,
        SDL_SCANCODE_C,
        /* dm2_v1_mac_input.c:76-86 maps the retail J/K/L turn-forward and
         * M/,/. movement keys.  SKProject SKULLWIN/c_input.cpp:818-838 polls
         * queued keys each game tick.  SDL repeats are discarded above, so
         * sample these held keys for movement after the first keydown. */
        SDL_SCANCODE_J,
        SDL_SCANCODE_K,
        SDL_SCANCODE_L,
        SDL_SCANCODE_M,
        SDL_SCANCODE_COMMA,
        SDL_SCANCODE_PERIOD
    };

    /* In the verified retail hand-pointer mode, D-pad direction moves the
     * cursor and must not be translated through the DM1 movement layout. */
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN &&
        gameView->nexusState.hand_pointer_active) {
        const int up = keys &&
            ((count > SDL_SCANCODE_UP && keys[SDL_SCANCODE_UP]) ||
             (count > SDL_SCANCODE_W && keys[SDL_SCANCODE_W]));
        const int down = keys &&
            ((count > SDL_SCANCODE_DOWN && keys[SDL_SCANCODE_DOWN]) ||
             (count > SDL_SCANCODE_S && keys[SDL_SCANCODE_S]));
        const int left = keys &&
            ((count > SDL_SCANCODE_LEFT && keys[SDL_SCANCODE_LEFT]) ||
             (count > SDL_SCANCODE_A && keys[SDL_SCANCODE_A]));
        const int right = keys &&
            ((count > SDL_SCANCODE_RIGHT && keys[SDL_SCANCODE_RIGHT]) ||
             (count > SDL_SCANCODE_D && keys[SDL_SCANCODE_D]));

        const M12_MenuInput pointerInput =
            M11_NexusPointerInputFromDirections(up, down, left, right);
        if (pointerInput != M12_MENU_INPUT_NONE) {
            if (gameView->nexusState.hand_pointer_repeat_ticks++ % 3 == 0)
                return pointerInput;
            return M12_MENU_INPUT_NONE;
        }
        gameView->nexusState.hand_pointer_repeat_ticks = 0;
    }

    if (m11_game_view_is_theron(gameView)) {
        return M11_TheronHeldMotionInputFromKeyboardState(
            (const uint8_t*)keys, count, gameView->theronButtonIIHeld);
    }

    if (!keys || count <= 0) return M12_MENU_INPUT_NONE;
    for (i = 0; i < (int)(sizeof(preferred) / sizeof(preferred[0])); i++) {
        int sc = (int)preferred[i];
        /* KP8 is a Theron convenience alias; the Theron branch above owns
         * its held-state route, so keep this shared route unchanged. */
        if (preferred[i] == SDL_SCANCODE_KP_8) {
            continue;
        }
        if (sc >= 0 && sc < count && keys[sc]) {
            if (m11_game_view_is_dm2_mac(gameView)) {
                M12_MenuInput macInput = M12_MENU_INPUT_NONE;
                SDL_KeyboardEvent keyEvent;
                memset(&keyEvent, 0, sizeof(keyEvent));
                keyEvent.scancode = preferred[i];
                keyEvent.mod = SDL_GetModState();
                keyEvent.key = SDL_GetKeyFromScancode(
                    preferred[i], keyEvent.mod, true);
                if (m11_dm2_mac_sdl_key_to_menu_input(
                        gameView, &keyEvent, &macInput) &&
                    (macInput == M12_MENU_INPUT_UP ||
                     macInput == M12_MENU_INPUT_DOWN ||
                     macInput == M12_MENU_INPUT_TURN_LEFT ||
                     macInput == M12_MENU_INPUT_TURN_RIGHT ||
                     macInput == M12_MENU_INPUT_STRAFE_LEFT ||
                     macInput == M12_MENU_INPUT_STRAFE_RIGHT)) {
                    return macInput;
                }
                continue;
            }
            M12_MenuInput input =
                m11_game_view_is_dm1(gameView)
                    ? M11_DM1V1_NavigationInputFromScancode((int)preferred[i])
                    : gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN
                    ? M11_NexusMovementInputFromScancode((int)preferred[i])
                    : M12_MENU_INPUT_NONE;
            if (input == M12_MENU_INPUT_NONE) {
                input = m11_motion_input_from_scancode(preferred[i]);
            }
            if (input != M12_MENU_INPUT_NONE) {
                return input;
            }
        }
    }
    return M12_MENU_INPUT_NONE;
}

static M12_MenuInput m11_next_script_input(const char** cursor,
                                          int* outWaitFrames,
                                          int* outWaitForRuntime,
                                          int* outWaitForScan) {
    const char* start;
    const char* end;
    if (outWaitFrames) {
        *outWaitFrames = 0;
    }
    if (outWaitForRuntime) {
        *outWaitForRuntime = 0;
    }
    if (outWaitForScan) {
        *outWaitForScan = 0;
    }
    if (!cursor || !*cursor) {
        return M12_MENU_INPUT_NONE;
    }
    start = *cursor;
    while (*start == ' ' || *start == ',') {
        ++start;
    }
    if (*start == '\0') {
        *cursor = start;
        return M12_MENU_INPUT_NONE;
    }
    end = start;
    while (*end != '\0' && *end != ',') {
        ++end;
    }
    *cursor = end;
    if ((size_t)(end - start) == sizeof("wait-game-runtime") - 1U &&
        strncmp(start, "wait-game-runtime", sizeof("wait-game-runtime") - 1U) == 0) {
        if (outWaitForRuntime) {
            *outWaitForRuntime = 1;
        }
        return M12_MENU_INPUT_NONE;
    }
    /* Startup selection tests must not send input against the deliberately
     * unavailable cards while authenticated collection discovery is active. */
    if ((size_t)(end - start) == sizeof("wait-scan-complete") - 1U &&
        strncmp(start, "wait-scan-complete",
                sizeof("wait-scan-complete") - 1U) == 0) {
        if (outWaitForScan) {
            *outWaitForScan = 1;
        }
        return M12_MENU_INPUT_NONE;
    }
    {
        int waitFrames = m11_script_wait_frames(start,
                                                (size_t)(end - start));
        if (waitFrames >= 0) {
            if (outWaitFrames) {
                *outWaitFrames = waitFrames;
            }
            return M12_MENU_INPUT_NONE;
        }
    }
    if (m11_push_script_event_token(start, (size_t)(end - start))) {
        return M12_MENU_INPUT_NONE;
    }
    return m11_map_script_token(start, (size_t)(end - start));
}

/* Result of polling a single pump. `menuPointerChanged` is set to 1
 * when a launcher mouse event mutated the menu state (the caller
 * should redraw the launcher). */
typedef struct {
    M12_MenuInput menuInput;
    int menuPointerChanged;
    int useModernLauncher;
} M11_PumpResult;

static int m11_map_window_to_launcher(int wx, int wy,
                                      int useModern,
                                      int* outX, int* outY) {
    int fbX = 0;
    int fbY = 0;
    if (!M11_Render_MapWindowToFramebuffer(wx, wy, &fbX, &fbY)) {
        return 0;
    }
    /* MapWindowToFramebuffer already maps into the current presented
     * content dimensions (1920x1080 for modern, 480x270 for legacy). */
    (void)useModern;
    if (outX) *outX = fbX;
    if (outY) *outY = fbY;
    return 1;
}

static int m11_legacy_card_pointer_active(
    const M12_StartupMenuState* menuState, int useModernLauncher) {
    return !useModernLauncher && menuState &&
           menuState->view == M12_MENU_VIEW_GAME_OPTIONS &&
           (menuState->gameCardFlowStage == 0 ||
            menuState->gameCardFlowStage == 1);
}

static int m11_handle_launcher_pointer(M12_StartupMenuState* menuState,
                                       int useModernLauncher,
                                       int x, int y, int clicked) {
    if (useModernLauncher) {
        int beforeView = menuState ? (int)menuState->view : -1;
        int beforeSelected = menuState ? menuState->selectedIndex : -1;
        M12_MouseHit hit = M12_ModernMenu_HitTest(menuState, x, y);
        int changed = M12_ModernMenu_HandlePointer(menuState, x, y, clicked, NULL);
        if (g_m11_debug_startup_detail) {
            fprintf(stderr,
                    "firestaff: menu-pointer x=%d y=%d clicked=%d hit=%d index=%d view=%d->%d selected=%d->%d changed=%d\n",
                    x, y, clicked, (int)hit.kind, hit.index,
                    beforeView, menuState ? (int)menuState->view : -1,
                    beforeSelected, menuState ? menuState->selectedIndex : -1,
                    changed);
        }
        return changed;
    }
    return M12_LegacyMenu_HandleCompactCardPointer(menuState, x, y, clicked);
}

static int m11_route_launcher_touch(M12_StartupMenuState* menuState,
                                     int useModernLauncher,
                                     float normalizedX,
                                     float normalizedY,
                                     int clicked,
                                     int* menuPointerChanged) {
    int windowWidth;
    int windowHeight;
    int lx;
    int ly;
    int changed;
    if (!menuState || !useModernLauncher) {
        return 0;
    }
    windowWidth = M11_Render_GetWindowWidth();
    windowHeight = M11_Render_GetWindowHeight();
    if (windowWidth <= 0 || windowHeight <= 0 ||
        normalizedX < 0.0f || normalizedX > 1.0f ||
        normalizedY < 0.0f || normalizedY > 1.0f) {
        return 0;
    }
    if (!m11_map_window_to_launcher((int)(normalizedX * windowWidth),
                                    (int)(normalizedY * windowHeight),
                                    1, &lx, &ly)) {
        return 0;
    }
    changed = M12_ModernMenu_HandlePointer(menuState, lx, ly, clicked, NULL);
    if (changed && menuPointerChanged) {
        *menuPointerChanged = 1;
    }
    return 1;
}

static int m11_dm1_rename_text_input_active(const M11_GameViewState* gameView) {
    return gameView &&
           dm1_v1_resurrection_rename_ui_gate_host_active_pc34(
               gameView->active,
               gameView->candidateMirrorPanelActive,
               gameView->candidateMirrorRenameActive);
}

static M11_GameInputResult
m11_dm1_rename_apply_ascii(M11_GameViewState* gameView, int ch) {
    if (!m11_dm1_rename_text_input_active(gameView)) {
        return M11_GAME_INPUT_IGNORED;
    }
    /* ReDMCSB REVIVE.C F0281:515-529 appends the selected A-Z/space/
     * punctuation character; the Firestaff gate keeps the same accepted
     * ASCII set and rejects other text-input bytes. */
    return M11_GameView_ApplyMirrorCandidateRenameAscii(gameView, ch)
               ? M11_GAME_INPUT_REDRAW
               : M11_GAME_INPUT_IGNORED;
}

static int m11_dm1_rename_consume_text_input(M11_GameViewState* gameView,
                                             const char* text,
                                             M11_GameInputResult* outResult) {
    int changed = 0;
    const unsigned char* p = (const unsigned char*)text;
    while (p && *p) {
        unsigned char ch = *p++;
        DM1_V1_ResurrectionRenameUiHostTextByteDecisionPc34Compat decision;
        if (!dm1_v1_resurrection_rename_ui_gate_host_text_byte_decision_pc34(
                gameView ? gameView->active : 0,
                gameView ? gameView->candidateMirrorPanelActive : 0,
                gameView ? gameView->candidateMirrorRenameActive : 0,
                (int)ch,
                &decision)) {
            return 0;
        }
        if (decision.useAscii &&
            m11_dm1_rename_apply_ascii(gameView, decision.ascii) ==
                M11_GAME_INPUT_REDRAW) {
            changed = 1;
        }
    }
    if (changed && outResult) {
        *outResult = M11_GAME_INPUT_REDRAW;
    }
    return 1;
}

static int m11_drain_retroachievements_events(Firestaff_RA_Runtime* runtime,
                                              M11_GameViewState* gameView) {
    Firestaff_RA_Event event;
    int pushed = 0;

    if (!runtime || !gameView) {
        return 0;
    }
    while (firestaff_ra_poll_event(runtime, &event)) {
        if (firestaff_ra_overlay_push_event(&gameView->retroAchievementsOverlay,
                                            runtime,
                                            &event)) {
            pushed = 1;
        }
    }
    return pushed;
}

static M11_GameInputResult
m11_dm1_rename_handle_keydown(M11_GameViewState* gameView,
                              int key,
                              int keypadEnterKey) {
    DM1_V1_ResurrectionRenameUiHostKeyDecisionPc34Compat decision;
    int hostKey = DM1_V1_RESURRECTION_RENAME_UI_HOST_KEY_OTHER_PC34_COMPAT;
    /* ReDMCSB REVIVE.C F0281:535-545 uses Return to move from name to
     * title, F0281:549-567 uses backspace within the active field, and
     * F0282:806-808 enters F0281 from C161.  Consume all other keydown
     * events here so SDL_TEXTINPUT, not the movement/shortcut mapper,
     * owns printable rename characters while the panel is active. */
    if (key == SDLK_BACKSPACE) {
        hostKey = DM1_V1_RESURRECTION_RENAME_UI_HOST_KEY_BACKSPACE_PC34_COMPAT;
    } else if (key == SDLK_ESCAPE) {
        hostKey = DM1_V1_RESURRECTION_RENAME_UI_HOST_KEY_ESCAPE_PC34_COMPAT;
    } else if (key == SDLK_RETURN) {
        hostKey = DM1_V1_RESURRECTION_RENAME_UI_HOST_KEY_RETURN_PC34_COMPAT;
    } else if (key == keypadEnterKey) {
        hostKey = DM1_V1_RESURRECTION_RENAME_UI_HOST_KEY_KEYPAD_RETURN_PC34_COMPAT;
    }
    if (!dm1_v1_resurrection_rename_ui_gate_host_keydown_route_pc34(
            gameView ? gameView->active : 0,
            gameView ? gameView->candidateMirrorPanelActive : 0,
            gameView ? gameView->candidateMirrorRenameActive : 0,
            gameView ? &gameView->candidateMirrorRename : NULL,
            hostKey,
            &decision)) {
        return M11_GAME_INPUT_IGNORED;
    }
    if (decision.useAscii) {
        return m11_dm1_rename_apply_ascii(gameView, decision.ascii);
    }
    if (decision.useCommand) {
        return M11_GameView_ApplyMirrorCandidateRenameCommand(
                   gameView, decision.command)
                   ? M11_GAME_INPUT_REDRAW
                   : M11_GAME_INPUT_IGNORED;
    }
    return M11_GAME_INPUT_IGNORED;
}

static int m11_csb_fmtowns_utility_consume_text_input(
    M11_GameViewState *gameView, const char *text,
    M11_GameInputResult *outResult)
{
    M11_GameInputResult result;
    if (!M11_GameView_CsbFmtownsUtilityTextInputActive(gameView)) return 0;
    result = M11_GameView_ConsumeCsbFmtownsUtilityTextInput(gameView, text);
    if (outResult) *outResult = result;
    return 1;
}

static int m11_csb_fmtowns_utility_handle_keydown(
    M11_GameViewState *gameView, int key, M11_GameInputResult *outResult)
{
    M11_CsbFmtownsUtilityTextKey mapped;
    M11_GameInputResult result;
    if (!M11_GameView_CsbFmtownsUtilityTextInputActive(gameView)) return 0;
    switch (key) {
        case SDLK_BACKSPACE: mapped = M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_BACKSPACE; break;
        case SDLK_ESCAPE: mapped = M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_CLEAR; break;
        case SDLK_LEFT: mapped = M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_LEFT; break;
        case SDLK_RIGHT: mapped = M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_RIGHT; break;
        case SDLK_HOME: mapped = M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_HOME; break;
        case SDLK_END: mapped = M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_END; break;
        case SDLK_UP: mapped = M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_UP; break;
        case SDLK_DOWN: mapped = M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_DOWN; break;
        case SDLK_PAGEUP: mapped = M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_PAGE_UP; break;
        case SDLK_PAGEDOWN: mapped = M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_PAGE_DOWN; break;
        default: return 1;
    }
    result = M11_GameView_HandleCsbFmtownsUtilityTextKey(gameView, mapped);
    if (outResult) *outResult = result;
    return 1;
}

/* Host accessibility policy, independent of each original game's clocks. */
static void m11_sync_focus_pause(M11_GameViewState* gameView,
                                  const M12_StartupMenuState* menuState,
                                  int hasFocus) {
    int supported;
    if (!gameView || !gameView->active || !menuState) return;
    supported = m11_game_view_is_dm1(gameView) ||
                m11_game_view_is_csb(gameView) ||
                gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT;
    if (!supported) return;
    M11_GameView_SetPauseReason(gameView, M11_GAME_PAUSE_REASON_FOCUS,
                                M11_FocusPauseRequired(menuState->settings.autoPause,
                                    hasFocus, SDL_GetCurrentVideoDriver()));
}

static int m11_event_requests_window_close(const SDL_Event* event) {
    SDL_Window* window;
    if (!event) return 0;
    window = M11_Render_GetWindow();
#if SDL_VERSION_ATLEAST(3, 0, 0)
    return event->type == SDL_EVENT_QUIT ||
           (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && window &&
            event->window.windowID == SDL_GetWindowID(window));
#else
    return event->type == SDL_QUIT ||
           (event->type == SDL_WINDOWEVENT &&
            event->window.event == SDL_WINDOWEVENT_CLOSE && window &&
            event->window.windowID == SDL_GetWindowID(window));
#endif
}

M12_MenuInput M11_StartupSettingsHorizontalKeyInput(
    int keycode,
    const M12_StartupMenuState* menuState,
    int gameplayActive,
    int textEditingActive) {
    if (!menuState || gameplayActive || textEditingActive ||
        menuState->view != M12_MENU_VIEW_SETTINGS) {
        return M12_MENU_INPUT_NONE;
    }
    if (keycode == SDLK_LEFT) return M12_MENU_INPUT_LEFT;
    if (keycode == SDLK_RIGHT) return M12_MENU_INPUT_RIGHT;
    return M12_MENU_INPUT_NONE;
}

static M12_MenuInput m11_poll_menu_input(M11_GameViewState* gameView,
                                         M12_StartupMenuState* menuState,
                                         const M12_GamepadMap* gamepadMap,
                                         M12_GamepadStatus* gamepadStatus,
                                         int useModernLauncher,
                                         M11_GameInputResult* gameViewResult,
                                         int* quitRequested,
                                         int* menuPointerChanged) {
    SDL_Event ev;
    int mappedX;
    int mappedY;
    static SDL_FingerID overlayFingerId;
    static int overlayFingerActive;
    if (gameViewResult) {
        *gameViewResult = M11_GAME_INPUT_IGNORED;
    }
    if (menuPointerChanged) {
        *menuPointerChanged = 0;
    }
    g_m11_debug_polled_event_counter = 0;
    g_m11_debug_polled_at_counter = 0;
    while (SDL_PollEvent(&ev)) {
        if (gameView && gameView->debugMovementTraceEnabled &&
            gameView->active && strcmp(gameView->sourceId, "dm1") == 0) {
            g_m11_debug_polled_event_counter =
                m11_sdl_event_timestamp_to_counter(ev.common.timestamp);
            g_m11_debug_polled_at_counter = SDL_GetPerformanceCounter();
        }
#if SDL_VERSION_ATLEAST(3, 0, 0)
        if (m11_event_requests_window_close(&ev)) {
            if (quitRequested) {
                *quitRequested = 1;
            }
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
            ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
            M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            if (menuState) {
                menuState->settings.windowWidth = M11_Render_GetWindowWidth();
                menuState->settings.windowHeight = M11_Render_GetWindowHeight();
                /* An OS maximize can arrive while a DM1 game is active.
                 * Reapply the host presentation policy immediately so the
                 * next pointer event maps through the same FIT rect M11
                 * presents, rather than a stale 1x--4x rectangle. */
                M11_ApplyStartupMenuRuntime(menuState);
            }
            if (gameView && gameView->active && gameViewResult) {
                *gameViewResult = M11_GAME_INPUT_REDRAW;
            }
            continue;
        }
        if (ev.type == SDL_EVENT_WINDOW_FOCUS_LOST ||
            ev.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
            if (ev.type == SDL_EVENT_WINDOW_FOCUS_LOST && gameView) {
                gameView->theronButtonIIHeld = 0;
            }
            m11_sync_focus_pause(gameView, menuState,
                                ev.type == SDL_EVENT_WINDOW_FOCUS_GAINED);
            overlayFingerActive = 0;
            continue;
        }
        if (ev.type == SDL_EVENT_GAMEPAD_ADDED ||
            ev.type == SDL_EVENT_GAMEPAD_REMOVED) {
            if (gamepadStatus) {
                M12_GamepadStatus_Update(gamepadStatus);
            }
            continue;
        }
        /* Device topology still changes while gameplay is suspended. */
        if (gameView &&
            (gameView->pauseReasons & M11_GAME_PAUSE_REASON_FOCUS)) continue;
        if (ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            M12_MenuInput gpadInput =
                m11_gamepad_button_input(gamepadMap,
                                         (SDL_GamepadButton)ev.gbutton.button,
                                         gameView && gameView->active);
            if (gpadInput != M12_MENU_INPUT_NONE) {
                return gpadInput;
            }
            continue;
        }
        if (ev.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
            M12_MenuInput gpadInput =
                m11_gamepad_axis_input(gamepadMap,
                                       (SDL_GamepadAxis)ev.gaxis.axis,
                                       ev.gaxis.value,
                                       gameView && gameView->active);
            if (gpadInput != M12_MENU_INPUT_NONE) {
                return gpadInput;
            }
            continue;
        }
        if ((ev.type == SDL_EVENT_FINGER_DOWN ||
             ev.type == SDL_EVENT_FINGER_MOTION ||
             ev.type == SDL_EVENT_FINGER_UP ||
             ev.type == SDL_EVENT_FINGER_CANCELED) &&
            menuState && useModernLauncher &&
            (!gameView || !gameView->active)) {
            int clicked = ev.type == SDL_EVENT_FINGER_DOWN ? 1 : 0;
            if (ev.type != SDL_EVENT_FINGER_CANCELED) {
                (void)m11_route_launcher_touch(menuState,
                                                useModernLauncher,
                                                ev.tfinger.x,
                                                ev.tfinger.y,
                                                clicked,
                                                menuPointerChanged);
            }
            continue;
        }
        if ((ev.type == SDL_EVENT_FINGER_DOWN ||
             ev.type == SDL_EVENT_FINGER_MOTION ||
             ev.type == SDL_EVENT_FINGER_UP ||
             ev.type == SDL_EVENT_FINGER_CANCELED) &&
            gameView && gameView->active) {
            /* Virtual touch controls are evaluated in full-window normalized
             * coordinates before the letterboxed game-source mapper.  Once a
             * finger begins on a control, consume its entire lifetime: the
             * corresponding UP must never become an Eye/Mouth/world tap. */
            if (overlayFingerActive && ev.tfinger.fingerID == overlayFingerId) {
                if (ev.type == SDL_EVENT_FINGER_UP ||
                    ev.type == SDL_EVENT_FINGER_CANCELED) {
                    overlayFingerActive = 0;
                }
                continue;
            }
            if (ev.type == SDL_EVENT_FINGER_DOWN) {
                M12_MenuInput overlayInput =
                    m11_touch_layout_input_at(ev.tfinger.x, ev.tfinger.y);
                if (overlayInput != M12_MENU_INPUT_NONE) {
                    overlayFingerId = ev.tfinger.fingerID;
                    overlayFingerActive = 1;
                    return overlayInput;
                }
            }
            M11_TouchEventKind kind = ev.type == SDL_EVENT_FINGER_DOWN
                ? M11_TOUCH_EVENT_DOWN
                : (ev.type == SDL_EVENT_FINGER_MOTION
                   ? M11_TOUCH_EVENT_MOVE
                   : (ev.type == SDL_EVENT_FINGER_UP
                      ? M11_TOUCH_EVENT_UP : M11_TOUCH_EVENT_CANCEL));
            /* A cancel has no source-space meaning. It must still clear the
             * recognizer if the host reports it in a letterbox margin, where
             * window-to-game mapping correctly rejects the coordinate. */
            if (kind == M11_TOUCH_EVENT_CANCEL && gameViewResult) {
                *gameViewResult = M11_GameView_HandleTouchEvent(
                    gameView, kind, 0, 0, (uint32_t)SDL_GetTicks());
                continue;
            }
            if (gameViewResult &&
                m11_map_normalized_touch_to_game_source(gameView,
                                                        ev.tfinger.x,
                                                        ev.tfinger.y,
                                                        &mappedX,
                                                        &mappedY)) {
                *gameViewResult = M11_GameView_HandleTouchEvent(
                    gameView, kind, mappedX, mappedY, (uint32_t)SDL_GetTicks());
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_MOTION &&
            gameView && gameView->active) {
            if (gameViewResult &&
                m11_map_window_pointer_to_game_source(gameView,
                                                       (int)ev.motion.x,
                                                       (int)ev.motion.y,
                                                       &mappedX,
                                                       &mappedY)) {
                /* Nexus's retail hand cursor addresses the full game screen,
                 * including the HUD. The hit table's bottom is below M11's
                 * 200-line framebuffer, so clamp the accepted desktop range
                 * to the pixels Firestaff can actually display. */
                if (gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN &&
                    gameView->nexusState.hand_pointer_active &&
                    gameView->nexusEngine &&
                    gameView->nexusEngine->hud_hit_rect_count ==
                        NEXUS_HIT_RECT_COUNT) {
                    const Nexus_HitRect *game_screen =
                        &gameView->nexusEngine->hud_hit_rects[
                            NEXUS_HIT_FULL_GAME_SCREEN];
                    const int screen_bottom =
                        game_screen->y2 < M11_FB_HEIGHT
                            ? game_screen->y2 : M11_FB_HEIGHT;
                    if (mappedX >= game_screen->x1 &&
                        mappedX < game_screen->x2 &&
                        mappedY >= game_screen->y1 &&
                        mappedY < screen_bottom) {
                        gameView->nexusState.hand_pointer_x = mappedX;
                        gameView->nexusState.hand_pointer_y = mappedY;
                        gameView->nexusState.hand_pointer_position_known = 1;
                        *gameViewResult = M11_GAME_INPUT_REDRAW;
                        return M12_MENU_INPUT_NONE;
                    }
                }
                *gameViewResult = M11_GameView_HandlePointerMove(
                    gameView,
                    mappedX,
                    mappedY);
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_MOTION &&
            menuState && (useModernLauncher ||
                          m11_legacy_card_pointer_active(menuState,
                                                         useModernLauncher)) &&
            (!gameView || !gameView->active)) {
            int lx, ly;
            if (m11_map_window_to_launcher((int)ev.motion.x, (int)ev.motion.y,
                                           useModernLauncher, &lx, &ly)) {
                m11_handle_launcher_pointer(menuState, useModernLauncher,
                                            lx, ly, 0);
            }
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            gameView && gameView->active &&
            (ev.button.button == SDL_BUTTON_LEFT ||
             ev.button.button == SDL_BUTTON_MIDDLE ||
             ev.button.button == SDL_BUTTON_RIGHT)) {
            if (gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN) {
                /* Nexus's retail callback is not yet semantically bound to
                 * individual regions. Never send a hand-pointer click into
                 * the generic DM1 click dispatcher. */
                if (gameView->nexusState.hand_pointer_active ||
                    ev.button.button != SDL_BUTTON_LEFT) {
                    continue;
                }
                if (gameViewResult &&
                    m11_map_window_pointer_to_game_source(
                        gameView, (int)ev.button.x, (int)ev.button.y,
                        &mappedX, &mappedY) &&
                    gameView->nexusEngine &&
                    gameView->nexusEngine->hud_hit_rect_count ==
                        NEXUS_HIT_RECT_COUNT) {
                    const Nexus_HitRect *pad =
                        &gameView->nexusEngine->hud_hit_rects[
                            NEXUS_HIT_MOVEMENT_PAD];
                    const M12_MenuInput movement =
                        M11_NexusMovementPadInputAt(
                            mappedX, mappedY, pad->x1, pad->y1,
                            pad->x2, pad->y2);
                    if (movement != M12_MENU_INPUT_NONE) {
                        *gameViewResult = M11_GameView_HandleInput(
                            gameView, movement);
                        if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                            return M12_MENU_INPUT_NONE;
                        }
                    }
                }
                continue;
            }
            if (m11_game_view_is_theron(gameView)) {
                if (ev.button.button == SDL_BUTTON_RIGHT &&
                    !gameView->graphicsPopupActive) {
                    gameView->theronButtonIIHeld =
                        gameView->theronState.startup_phase ==
                                THERON_STARTUP_PHASE_IN_DUNGEON &&
                        gameView->theronState.level_loaded;
                    if (gameView->theronButtonIIHeld) {
                        /* The USA retail manual p. 5 defines Button II as the
                         * simplified-movement modifier. Do not also route its
                         * press through desktop Back, which opens a quit modal
                         * and prevents the direction key from reaching play.
                         * See docs/source-lock/theron-button-ii-manual-source-lock.md. */
                        continue;
                    }
                }
                M12_MenuInput buttonInput =
                    M11_TheronMouseButtonToInput(ev.button.button);
                /* Ordinary desktop mouse contract: Button I/II are left/right
                 * only.  Do not let a physical middle button fall through to
                 * the pointer dispatcher as a synthetic right click. */
                if (buttonInput == M12_MENU_INPUT_NONE) {
                    continue;
                }
                if (gameViewResult) {
                    if (m11_map_window_pointer_to_game_source(
                            gameView, (int)ev.button.x, (int)ev.button.y,
                            &mappedX, &mappedY)) {
                        (void)M11_GameView_HandlePointerMove(
                            gameView, mappedX, mappedY);
                        /* Startup uses Theron's source-space selection route.
                         * Dungeon clicks are accepted only by the explicit
                         * Firestaff control panel; arbitrary viewport points
                         * are not treated as original Theron hit targets. */
                        *gameViewResult = M11_GameView_HandlePointerButton(
                            gameView,
                            mappedX,
                            mappedY,
                            ev.button.button == SDL_BUTTON_LEFT
                                ? DM1_V1_MOUSE_MASK_LEFT_PC34
                                : DM1_V1_MOUSE_MASK_RIGHT_PC34);
                        /* --debug's pointer trace is the live evidence for
                         * Theron's host-panel routing. Keep it at the same
                         * mapped source coordinates the dispatcher consumed,
                         * so letterboxing and presentation scaling can be
                         * diagnosed without guessing from window pixels. */
                        m11_debug_game_pointer(
                            gameView,
                            ev.button.button == SDL_BUTTON_LEFT
                                ? "pointer-left" : "pointer-right",
                            mappedX, mappedY, *gameViewResult);
                    } else {
                        /* Letterbox clicks have no source-space coordinate. */
                        continue;
                    }
                    if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                        return M12_MENU_INPUT_NONE;
                    }
                }
                continue;
            }
            if (gameViewResult) {
                const int windowX = (int)ev.button.x;
                const int windowY = (int)ev.button.y;
                const int buttonMask = ev.button.button == SDL_BUTTON_RIGHT
                    ? DM1_V1_MOUSE_MASK_RIGHT_PC34
                    : DM1_V1_MOUSE_MASK_LEFT_PC34;
                const int activeChampionBefore =
                    gameView->world.party.activeChampionIndex;
                const int pointerMapped =
                    m11_map_window_pointer_to_game_source(
                        gameView, windowX, windowY, &mappedX, &mappedY);
                if (pointerMapped) {
                    *gameViewResult = M11_GameView_HandlePointerButton(
                        gameView, mappedX, mappedY, buttonMask);
                }
                m11_debug_pointer_dispatch(
                    gameView,
                    ev.button.button == SDL_BUTTON_RIGHT ? "down-right" : "down-left",
                    windowX, windowY, pointerMapped, mappedX, mappedY,
                    buttonMask, activeChampionBefore,
                    pointerMapped ? (int)*gameViewResult : -1);
                if (pointerMapped) {
                    m11_debug_game_pointer(
                        gameView,
                        ev.button.button == SDL_BUTTON_RIGHT
                            ? "pointer-right" : "pointer-left",
                        mappedX, mappedY, *gameViewResult);
                }
                if (pointerMapped &&
                    *gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP && gameView &&
            m11_game_view_is_theron(gameView) &&
            ev.button.button == SDL_BUTTON_RIGHT) {
            gameView->theronButtonIIHeld = 0;
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP &&
            gameView && gameView->active &&
            ev.button.button == SDL_BUTTON_LEFT) {
            if (gameViewResult &&
                m11_map_window_pointer_to_game_source(gameView,
                                                       (int)ev.button.x,
                                                       (int)ev.button.y,
                                                       &mappedX,
                                                       &mappedY)) {
                *gameViewResult = M11_GameView_HandlePointerButtonRelease(
                    gameView, mappedX, mappedY, DM1_V1_MOUSE_MASK_LEFT_PC34);
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            menuState && (useModernLauncher ||
                          m11_legacy_card_pointer_active(menuState,
                                                         useModernLauncher)) &&
            (!gameView || !gameView->active) &&
            ev.button.button == SDL_BUTTON_LEFT) {
            int lx, ly;
            if (m11_map_window_to_launcher((int)ev.button.x, (int)ev.button.y,
                                           useModernLauncher, &lx, &ly)) {
                int changed = m11_handle_launcher_pointer(
                    menuState, useModernLauncher, lx, ly, 1);
                if (changed && menuPointerChanged) {
                    *menuPointerChanged = 1;
                }
            }
            continue;
        }
        /* SDL3 is the macOS/Steam Deck path.  Keep launcher text editing
         * ahead of the game rename route so RetroAchievements credentials
         * receive both hardware-keyboard and virtual-keyboard text events. */
        if (ev.type == SDL_EVENT_TEXT_INPUT &&
            menuState && useModernLauncher &&
            (!gameView || !gameView->active) &&
            M12_StartupMenu_ConsumeTextInput(menuState, ev.text.text)) {
            if (menuPointerChanged) {
                *menuPointerChanged = 1;
            }
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_EVENT_TEXT_INPUT &&
            m11_dm1_rename_consume_text_input(gameView,
                                              ev.text.text,
                                              gameViewResult)) {
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_EVENT_TEXT_INPUT &&
            m11_csb_fmtowns_utility_consume_text_input(gameView,
                                                        ev.text.text,
                                                        gameViewResult)) {
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_EVENT_KEY_DOWN) {
            if (menuState && useModernLauncher &&
                (!gameView || !gameView->active) &&
                M12_StartupMenu_TextEditActive(menuState)) {
                if ((ev.key.mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) &&
                    ev.key.key == SDLK_V) {
                    char* clipboard = SDL_GetClipboardText();
                    if (clipboard) {
                        (void)M12_StartupMenu_ConsumeTextInput(menuState,
                                                                clipboard);
                        SDL_free(clipboard);
                    }
                    if (menuPointerChanged) {
                        *menuPointerChanged = 1;
                    }
                    return M12_MENU_INPUT_NONE;
                }
                switch (ev.key.key) {
                    case SDLK_BACKSPACE:
                        (void)M12_StartupMenu_TextEditBackspace(menuState);
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        (void)M12_StartupMenu_TextEditCommit(menuState);
                        break;
                    case SDLK_ESCAPE:
                        (void)M12_StartupMenu_TextEditCancel(menuState);
                        break;
                    default:
                        return M12_MENU_INPUT_NONE;
                }
                if (menuPointerChanged) {
                    *menuPointerChanged = 1;
                }
                return M12_MENU_INPUT_NONE;
            }
            if (m11_dm1_rename_text_input_active(gameView)) {
                M11_GameInputResult renameResult =
                    m11_dm1_rename_handle_keydown(gameView,
                                                  (int)ev.key.key,
                                                  SDLK_KP_ENTER);
                if (gameViewResult) {
                    *gameViewResult = renameResult;
                }
                return M12_MENU_INPUT_NONE;
            }
            if (m11_csb_fmtowns_utility_handle_keydown(gameView,
                                                       (int)ev.key.key,
                                                       gameViewResult)) {
                return M12_MENU_INPUT_NONE;
            }
            if (gameView && gameView->active && ev.key.key == SDLK_H &&
                (ev.key.mod & SDL_KMOD_CTRL)) {
                return M12_MENU_INPUT_GRAPHICS_POPUP;
            }
            if (m11_game_view_is_csb(gameView)) {
                M12_MenuInput csbInput = M12_MENU_INPUT_NONE;
                if (m11_csb_sdl_key_to_menu_input((int)ev.key.key,
                                                  (ev.key.mod & SDL_KMOD_CTRL) != 0,
                                                  &csbInput)) {
                    return csbInput;
                }
            }
            if (m11_dm2_mac_key_event_is_held_motion(gameView, &ev.key)) {
                return M12_MENU_INPUT_NONE;
            }
            if (gameView && gameView->active) {
                M12_MenuInput macInput = M12_MENU_INPUT_NONE;
                if (m11_dm2_mac_sdl_key_to_menu_input(
                        gameView, &ev.key, &macInput)) {
                    return macInput;
                }
            }
            if (gameView && gameView->active &&
                m11_game_view_supports_held_motion_input(gameView) &&
                ev.key.repeat) {
                M12_MenuInput repeatInput =
                    gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN
                        ? M11_NexusMovementInputFromScancode(
                              (int)ev.key.scancode)
                        : m11_game_view_is_theron(gameView)
                        ? M11_TheronGameplayInputFromScancodeWithButtonII(
                              (int)ev.key.scancode,
                              gameView->theronButtonIIHeld)
                        : M11_DM1V1_NavigationInputFromScancode(
                              (int)ev.key.scancode);
                if (repeatInput == M12_MENU_INPUT_NONE) {
                    repeatInput =
                        m11_motion_input_from_scancode(ev.key.scancode);
                }
                if (DM1_V1_InputMenuTokenUsesHeldRepeatPc34Compat(
                        (int)repeatInput)) {
                    /* ReDMCSB COMMAND.C F0361 -> F0380 consumes one
                     * keyboard command at a source boundary.  Ignore the
                     * OS autorepeat copy; m11_held_motion_input_from_keyboard
                     * will issue the next held command when that boundary
                     * opens, including Q/E and arrow-key feedback routes. */
                    return M12_MENU_INPUT_NONE;
                }
            }
            if (gameView && gameView->active) {
                M12_MenuInput mappedInput = M12_MENU_INPUT_NONE;
                if (gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN &&
                    gameView->nexusState.title_active) {
                    mappedInput = M11_NexusStartupMenuInputFromScancode(
                        (int)ev.key.scancode,
                        gameView->nexusState.title_active);
                    if (mappedInput != M12_MENU_INPUT_NONE) {
                        return mappedInput;
                    }
                }
                if (m11_game_view_is_theron(gameView)) {
                    const int gameplayInput =
                        m11_game_view_supports_held_motion_input(gameView);
                    mappedInput = gameplayInput
                        ? M11_TheronGameplayInputFromScancodeWithButtonII(
                              (int)ev.key.scancode,
                              gameView->theronButtonIIHeld)
                        : M11_TheronNavigationInputFromScancode(
                              (int)ev.key.scancode);
                    if (mappedInput == M12_MENU_INPUT_NONE) {
                        mappedInput = gameplayInput
                            ? M11_TheronGameplayInputFromKeycodeWithButtonII(
                                  (int)ev.key.key,
                                  gameView->theronButtonIIHeld)
                            : M11_TheronNavigationInputFromKeycode(
                                  (int)ev.key.key);
                    }
                    if (mappedInput != M12_MENU_INPUT_NONE) {
                        return mappedInput;
                    }
                } else if (gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN) {
                    mappedInput = M11_NexusLeaderInputFromScancode(
                        (int)ev.key.scancode,
                        (ev.key.mod & SDL_KMOD_SHIFT) != 0);
                    if (mappedInput != M12_MENU_INPUT_NONE) return mappedInput;
                    mappedInput = M11_NexusPointerToggleInputFromScancode(
                        (int)ev.key.scancode);
                    if (mappedInput != M12_MENU_INPUT_NONE) return mappedInput;
                    if (gameView->nexusState.hand_pointer_active) {
                        mappedInput = M11_NexusPointerInputFromScancode(
                            (int)ev.key.scancode);
                        if (mappedInput != M12_MENU_INPUT_NONE) return mappedInput;
                    }
                } else if (m11_game_view_is_dm1(gameView)) {
                    if (m11_dm1_sdl_key_to_menu_input(
                            (int)ev.key.key,
                            (ev.key.mod & SDL_KMOD_CTRL) != 0,
                            (ev.key.mod & SDL_KMOD_SHIFT) != 0,
                            &mappedInput)) {
                        return mappedInput;
                    }
                    mappedInput =
                        M11_DM1V1_NavigationInputFromScancode((int)ev.key.scancode);
                    if (mappedInput != M12_MENU_INPUT_NONE) {
                        return mappedInput;
                    }
                }
                mappedInput = gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN
                    ? M11_NexusMovementInputFromScancode(ev.key.scancode)
                    : M12_MENU_INPUT_NONE;
                if (mappedInput != M12_MENU_INPUT_NONE) return mappedInput;
                mappedInput = m11_motion_input_from_scancode(ev.key.scancode);
                if (mappedInput != M12_MENU_INPUT_NONE) {
                    return mappedInput;
                }
            }
            {
                M12_MenuInput settingsInput =
                    M11_StartupSettingsHorizontalKeyInput(
                        (int)ev.key.key,
                        menuState,
                        gameView && gameView->active,
                        menuState &&
                            M12_StartupMenu_TextEditActive(menuState));
                if (settingsInput != M12_MENU_INPUT_NONE) {
                    return settingsInput;
                }
            }
            switch (ev.key.key) {
                case SDLK_UP:
                    return M12_MENU_INPUT_UP;
                case SDLK_DOWN:
                    return M12_MENU_INPUT_DOWN;
                /* ReDMCSB PC-34/I34E source lock: COMMAND.C:677-684 maps
                 * keypad scancodes 0x4B/0x4C/0x4D/0x4F/0x50/0x51 to
                 * turn-left/forward/turn-right/strafe-left/back/strafe-right;
                 * IO2.C:47-59 normalizes shifted arrow scancodes into the
                 * same command-table codes. SDL reports NumLock-on keypad
                 * keys as SDLK_KP_N, so route them explicitly before the
                 * generic q/e/wasd convenience aliases. */
                case SDLK_KP_5:
                    return M12_MENU_INPUT_UP;
                case SDLK_KP_2:
                    return M12_MENU_INPUT_DOWN;
                case SDLK_KP_1:
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_KP_3:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                /* v2.8.x: arrow Left/Right now mean strafe (matches
                 * the original DM1 PC 3.4 convention; see also the
                 * user's keyboard-mapping request).  Q/E + Home/End +
                 * KP_4/KP_6 produce the turn-left/turn-right input
                 * tokens.  KP_4/KP_6 stay turn-left/turn-right
                 * because that's what COMMAND.C:677-684 maps them to
                 * on the original PC 3.4 keyboard (source-locked by
                 * test_dm1_v1_input_command_queue_pc34_compat). */
                case SDLK_LEFT:
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_RIGHT:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                case SDLK_KP_4:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_KP_6:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_Q:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_E:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_HOME:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_END:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_A:
                    /* v2.9.x: WASD mirrors the arrow keys unconditionally.
                     * Per the user's request, WASD should always work
                     * exactly like arrow keys for navigation. The
                     * wasdMovementEnabled settings toggle is preserved
                     * as legacy state for backward-compatible config
                     * files but is no longer consulted here -- the
                     * settings menu entry has been removed and a
                     * migration warning is logged once on first launch
                     * if the user had previously disabled it. */
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_D:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                case SDLK_W:
                    return M12_MENU_INPUT_UP;
                case SDLK_S:
                    if (ev.key.mod & SDL_KMOD_CTRL) {
                        if (gameView && gameView->active)
                            return M12_MENU_INPUT_SAVE_GAME;
                        return M12_MENU_INPUT_NONE;
                    }
                    return M12_MENU_INPUT_DOWN;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    return M12_MENU_INPUT_ACCEPT;
                case SDLK_ESCAPE:
                    return M12_MENU_INPUT_BACK;
                case SDLK_SPACE:
                    return M12_MENU_INPUT_ACTION;
                case SDLK_TAB:
                    return M12_MENU_INPUT_CYCLE_CHAMPION;
                case SDLK_F5: {
                    /* F5 = quick save */
                    if (gameView && gameView->active) {
                        if (M11_GameView_QuickSave(gameView)) {
                            if (m11_dm2_host_save_log_allowed(gameView)) {
                                fprintf(stderr, "SAVE: quicksave written\n");
                            }
                        } else if (m11_dm2_host_save_log_allowed(gameView)) {
                            fprintf(stderr, "SAVE FAILED: quicksave rejected\n");
                        }
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                }
                case SDLK_F9: {
                    /* F9 = quick load */
                    if (gameView && gameView->active) {
                        if (M11_GameView_QuickLoad(gameView)) {
                            if (m11_dm2_host_save_log_allowed(gameView)) {
                                fprintf(stderr, "LOAD: quicksave restored\n");
                            }
                            if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                        } else if (m11_dm2_host_save_log_allowed(gameView)) {
                            fprintf(stderr, "LOAD FAILED: quicksave rejected\n");
                        }
                    }
                    return M12_MENU_INPUT_NONE;
                }
                case SDLK_F6: {
                    int m = M11_QolRuntime_CycleSpeedMultiplier();
                    fprintf(stderr, "QoL: game speed %d%% (%.1fx)\n", m, m / 100.0);
                    if (gameViewResult && gameView && gameView->active) {
                        *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                }
                case SDLK_F7:
                    /* F7 = toggle dungeon minimap overlay */
                    if (gameView && gameView->active) {
                        int on = M11_QolRuntime_ToggleMinimap();
                        fprintf(stderr, "QoL: minimap %s\n", on ? "on" : "off");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_F8:
                    /* F8 = export auto-map for current level
                     * (Ctrl+M alias: ReDMCSB M already triggers a
                     * fullscreen map overlay so we keep that key for
                     * the V1 chrome and add a dedicated export key.) */
                    if (gameView && gameView->active) {
                        int ex = DM1_V1_AutoMap_ExportCurrentLevelPc34Compat(gameView);
                        fprintf(stderr, "QoL: auto-map export %s\n",
                                ex ? "ok" : "FAILED");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_L:
                    /* L = toggle combat log overlay */
                    if (gameView && gameView->active) {
                        int on = M11_QolRuntime_ToggleCombatLog();
                        fprintf(stderr, "QoL: combat log %s\n", on ? "on" : "off");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_R:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_REST_TOGGLE;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_X:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_USE_STAIRS;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_G:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_PICKUP_ITEM;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_P:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_DROP_ITEM;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_1:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_1;
                    return M12_MENU_INPUT_NONE;
                case SDLK_2:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_2;
                    return M12_MENU_INPUT_NONE;
                case SDLK_3:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_3;
                    return M12_MENU_INPUT_NONE;
                case SDLK_4:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_4;
                    return M12_MENU_INPUT_NONE;
                case SDLK_5:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_5;
                    return M12_MENU_INPUT_NONE;
                case SDLK_6:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_6;
                    return M12_MENU_INPUT_NONE;
                case SDLK_C:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_CAST;
                    return M12_MENU_INPUT_NONE;
                case SDLK_V:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_CLEAR;
                    return M12_MENU_INPUT_NONE;
                case SDLK_U:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_USE_ITEM;
                    return M12_MENU_INPUT_NONE;
                case SDLK_M:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_MAP_TOGGLE;
                    return M12_MENU_INPUT_NONE;
                case SDLK_I:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_INVENTORY_TOGGLE;
                    return M12_MENU_INPUT_NONE;
                case SDLK_F10:
                    return (gameView && gameView->active)
                        ? M12_MENU_INPUT_GRAPHICS_POPUP
                        : M12_MENU_INPUT_NONE;
                case SDLK_F11:
                    M11_Render_ToggleFullscreen();
                    return M12_MENU_INPUT_NONE;
                case SDLK_F12:
                    m11_capture_user_screenshot(gameView, menuState);
                    return M12_MENU_INPUT_NONE;
                default:
                    break;
            }
        }
#else
        if (m11_event_requests_window_close(&ev)) {
            if (quitRequested) {
                *quitRequested = 1;
            }
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_WINDOWEVENT &&
            ev.window.event == SDL_WINDOWEVENT_RESIZED) {
            M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            if (menuState) {
                menuState->settings.windowWidth = M11_Render_GetWindowWidth();
                menuState->settings.windowHeight = M11_Render_GetWindowHeight();
                M11_ApplyStartupMenuRuntime(menuState);
            }
            continue;
        }
        if (ev.type == SDL_WINDOWEVENT &&
            ev.window.event == SDL_WINDOWEVENT_FOCUS_LOST && gameView) {
            gameView->theronButtonIIHeld = 0;
            continue;
        }
        if (ev.type == SDL_CONTROLLERDEVICEADDED ||
            ev.type == SDL_CONTROLLERDEVICEREMOVED) {
            if (gamepadStatus) {
                M12_GamepadStatus_Update(gamepadStatus);
            }
            continue;
        }
        if (ev.type == SDL_CONTROLLERBUTTONDOWN) {
            M12_MenuInput gpadInput =
                m11_gamepad_button_input(gamepadMap,
                                         (SDL_GamepadButton)ev.cbutton.button,
                                         gameView && gameView->active);
            if (gpadInput != M12_MENU_INPUT_NONE) {
                return gpadInput;
            }
            continue;
        }
        if (ev.type == SDL_CONTROLLERAXISMOTION) {
            M12_MenuInput gpadInput =
                m11_gamepad_axis_input(gamepadMap,
                                       (SDL_GamepadAxis)ev.caxis.axis,
                                       ev.caxis.value,
                                       gameView && gameView->active);
            if (gpadInput != M12_MENU_INPUT_NONE) {
                return gpadInput;
            }
            continue;
        }
        if ((ev.type == SDL_FINGERDOWN ||
             ev.type == SDL_FINGERMOTION ||
             ev.type == SDL_FINGERUP ||
             ev.type == SDL_FINGERCANCEL) &&
            menuState && useModernLauncher &&
            (!gameView || !gameView->active)) {
            int clicked = ev.type == SDL_FINGERDOWN ? 1 : 0;
            if (ev.type != SDL_FINGERCANCEL) {
                (void)m11_route_launcher_touch(menuState,
                                                useModernLauncher,
                                                ev.tfinger.x,
                                                ev.tfinger.y,
                                                clicked,
                                                menuPointerChanged);
            }
            continue;
        }
        if (ev.type == SDL_MOUSEMOTION &&
            gameView && gameView->active) {
            if (gameViewResult &&
                m11_map_window_pointer_to_game_source(gameView,
                                                       ev.motion.x,
                                                       ev.motion.y,
                                                       &mappedX,
                                                       &mappedY)) {
                /* Keep the SDL2 path aligned with the SDL3 full-screen
                 * pointer bounds above. */
                if (gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN &&
                    gameView->nexusState.hand_pointer_active &&
                    gameView->nexusEngine &&
                    gameView->nexusEngine->hud_hit_rect_count ==
                        NEXUS_HIT_RECT_COUNT) {
                    const Nexus_HitRect *game_screen =
                        &gameView->nexusEngine->hud_hit_rects[
                            NEXUS_HIT_FULL_GAME_SCREEN];
                    const int screen_bottom =
                        game_screen->y2 < M11_FB_HEIGHT
                            ? game_screen->y2 : M11_FB_HEIGHT;
                    if (mappedX >= game_screen->x1 &&
                        mappedX < game_screen->x2 &&
                        mappedY >= game_screen->y1 &&
                        mappedY < screen_bottom) {
                        gameView->nexusState.hand_pointer_x = mappedX;
                        gameView->nexusState.hand_pointer_y = mappedY;
                        gameView->nexusState.hand_pointer_position_known = 1;
                        *gameViewResult = M11_GAME_INPUT_REDRAW;
                        return M12_MENU_INPUT_NONE;
                    }
                }
                *gameViewResult = M11_GameView_HandlePointerMove(
                    gameView,
                    mappedX,
                    mappedY);
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_MOUSEMOTION &&
            menuState && (useModernLauncher ||
                          m11_legacy_card_pointer_active(menuState,
                                                         useModernLauncher)) &&
            (!gameView || !gameView->active)) {
            int lx, ly;
            if (m11_map_window_to_launcher(ev.motion.x, ev.motion.y,
                                           useModernLauncher, &lx, &ly)) {
                m11_handle_launcher_pointer(menuState, useModernLauncher,
                                            lx, ly, 0);
            }
            continue;
        }
        if (ev.type == SDL_MOUSEBUTTONDOWN &&
            gameView && gameView->active &&
            (ev.button.button == SDL_BUTTON_LEFT ||
             ev.button.button == SDL_BUTTON_MIDDLE ||
             ev.button.button == SDL_BUTTON_RIGHT)) {
            if (gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN) {
                /* The hand-pointer action callback is not yet bound to
                 * retail consumers. Keep its clicks inert, and route only
                 * left-clicks on the authentic movement pad to locomotion. */
                if (gameView->nexusState.hand_pointer_active ||
                    ev.button.button != SDL_BUTTON_LEFT) {
                    continue;
                }
                if (gameViewResult &&
                    m11_map_window_pointer_to_game_source(
                        gameView, ev.button.x, ev.button.y,
                        &mappedX, &mappedY) &&
                    gameView->nexusEngine &&
                    gameView->nexusEngine->hud_hit_rect_count ==
                        NEXUS_HIT_RECT_COUNT) {
                    const Nexus_HitRect *pad =
                        &gameView->nexusEngine->hud_hit_rects[
                            NEXUS_HIT_MOVEMENT_PAD];
                    const M12_MenuInput movement =
                        M11_NexusMovementPadInputAt(
                            mappedX, mappedY, pad->x1, pad->y1,
                            pad->x2, pad->y2);
                    if (movement != M12_MENU_INPUT_NONE) {
                        *gameViewResult = M11_GameView_HandleInput(
                            gameView, movement);
                        if (*gameViewResult != M11_GAME_INPUT_IGNORED)
                            return M12_MENU_INPUT_NONE;
                    }
                }
                continue;
            }
            if (m11_game_view_is_theron(gameView)) {
                if (ev.button.button == SDL_BUTTON_RIGHT &&
                    !gameView->graphicsPopupActive) {
                    gameView->theronButtonIIHeld =
                        gameView->theronState.startup_phase ==
                                THERON_STARTUP_PHASE_IN_DUNGEON &&
                        gameView->theronState.level_loaded;
                    if (gameView->theronButtonIIHeld) {
                        /* Keep the SDL2 path aligned with SDL3: Button II is
                         * a dungeon modifier, not a simultaneous Back action. */
                        continue;
                    }
                }
                M12_MenuInput buttonInput =
                    M11_TheronMouseButtonToInput(ev.button.button);
                /* SDL_BUTTON_MIDDLE is intentionally not a Theron control;
                 * keep it inert during startup and dungeon pointer routing. */
                if (buttonInput == M12_MENU_INPUT_NONE) {
                    continue;
                }
                if (gameViewResult) {
                    if (m11_map_window_pointer_to_game_source(
                            gameView, ev.button.x, ev.button.y,
                            &mappedX, &mappedY)) {
                        (void)M11_GameView_HandlePointerMove(
                            gameView, mappedX, mappedY);
                        /* Startup uses Theron's source-space selection route.
                         * Dungeon clicks are accepted only by the explicit
                         * Firestaff control panel; arbitrary viewport points
                         * are not treated as original Theron hit targets. */
                        *gameViewResult = M11_GameView_HandlePointerButton(
                            gameView,
                            mappedX,
                            mappedY,
                            ev.button.button == SDL_BUTTON_LEFT
                                ? DM1_V1_MOUSE_MASK_LEFT_PC34
                                : DM1_V1_MOUSE_MASK_RIGHT_PC34);
                    } else {
                        /* Do not dispatch a source button in the window
                         * margin where no source point exists. */
                        continue;
                    }
                    if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                        return M12_MENU_INPUT_NONE;
                    }
                }
                continue;
            }
            if (gameViewResult) {
                const int windowX = ev.button.x;
                const int windowY = ev.button.y;
                const int buttonMask = ev.button.button == SDL_BUTTON_RIGHT
                    ? DM1_V1_MOUSE_MASK_RIGHT_PC34
                    : DM1_V1_MOUSE_MASK_LEFT_PC34;
                const int activeChampionBefore =
                    gameView->world.party.activeChampionIndex;
                const int pointerMapped =
                    m11_map_window_pointer_to_game_source(
                        gameView, windowX, windowY, &mappedX, &mappedY);
                if (pointerMapped) {
                    *gameViewResult = M11_GameView_HandlePointerButton(
                        gameView, mappedX, mappedY, buttonMask);
                }
                m11_debug_pointer_dispatch(
                    gameView,
                    ev.button.button == SDL_BUTTON_RIGHT ? "down-right" : "down-left",
                    windowX, windowY, pointerMapped, mappedX, mappedY,
                    buttonMask, activeChampionBefore,
                    pointerMapped ? *gameViewResult : -1);
                if (pointerMapped &&
                    *gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_MOUSEBUTTONUP && gameView &&
            m11_game_view_is_theron(gameView) &&
            ev.button.button == SDL_BUTTON_RIGHT) {
            gameView->theronButtonIIHeld = 0;
            continue;
        }
        if (ev.type == SDL_MOUSEBUTTONUP &&
            gameView && gameView->active &&
            ev.button.button == SDL_BUTTON_LEFT) {
            if (gameViewResult &&
                m11_map_window_pointer_to_game_source(gameView,
                                                       ev.button.x,
                                                       ev.button.y,
                                                       &mappedX,
                                                       &mappedY)) {
                *gameViewResult = M11_GameView_HandlePointerButtonRelease(
                    gameView, mappedX, mappedY, DM1_V1_MOUSE_MASK_LEFT_PC34);
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_MOUSEBUTTONDOWN &&
            menuState && (useModernLauncher ||
                          m11_legacy_card_pointer_active(menuState,
                                                         useModernLauncher)) &&
            (!gameView || !gameView->active) &&
            ev.button.button == SDL_BUTTON_LEFT) {
            int lx, ly;
            if (m11_map_window_to_launcher(ev.button.x, ev.button.y,
                                           useModernLauncher, &lx, &ly)) {
                int changed = m11_handle_launcher_pointer(
                    menuState, useModernLauncher, lx, ly, 1);
                if (changed && menuPointerChanged) {
                    *menuPointerChanged = 1;
                }
            }
            continue;
        }
        if (ev.type == SDL_TEXTINPUT &&
            menuState && useModernLauncher &&
            (!gameView || !gameView->active) &&
            M12_StartupMenu_ConsumeTextInput(menuState, ev.text.text)) {
            if (menuPointerChanged) {
                *menuPointerChanged = 1;
            }
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_TEXTINPUT &&
            m11_dm1_rename_consume_text_input(gameView,
                                              ev.text.text,
                                              gameViewResult)) {
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_TEXTINPUT &&
            m11_csb_fmtowns_utility_consume_text_input(gameView,
                                                        ev.text.text,
                                                        gameViewResult)) {
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_KEYDOWN) {
            if (menuState && useModernLauncher &&
                (!gameView || !gameView->active) &&
                M12_StartupMenu_TextEditActive(menuState)) {
                if ((ev.key.keysym.mod & (KMOD_CTRL | KMOD_GUI)) &&
                    ev.key.keysym.sym == SDLK_V) {
                    char* clipboard = SDL_GetClipboardText();
                    if (clipboard) {
                        (void)M12_StartupMenu_ConsumeTextInput(menuState,
                                                                clipboard);
                        SDL_free(clipboard);
                    }
                    if (menuPointerChanged) {
                        *menuPointerChanged = 1;
                    }
                    return M12_MENU_INPUT_NONE;
                }
                switch (ev.key.keysym.sym) {
                    case SDLK_BACKSPACE:
                        (void)M12_StartupMenu_TextEditBackspace(menuState);
                        if (menuPointerChanged) {
                            *menuPointerChanged = 1;
                        }
                        return M12_MENU_INPUT_NONE;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        (void)M12_StartupMenu_TextEditCommit(menuState);
                        if (menuPointerChanged) {
                            *menuPointerChanged = 1;
                        }
                        return M12_MENU_INPUT_NONE;
                    case SDLK_ESCAPE:
                        (void)M12_StartupMenu_TextEditCancel(menuState);
                        if (menuPointerChanged) {
                            *menuPointerChanged = 1;
                        }
                        return M12_MENU_INPUT_NONE;
                    default:
                        return M12_MENU_INPUT_NONE;
                }
            }
            if (m11_dm1_rename_text_input_active(gameView)) {
                M11_GameInputResult renameResult =
                    m11_dm1_rename_handle_keydown(gameView,
                                                  (int)ev.key.keysym.sym,
                                                  SDLK_KP_ENTER);
                if (gameViewResult) {
                    *gameViewResult = renameResult;
                }
                return M12_MENU_INPUT_NONE;
            }
            if (m11_csb_fmtowns_utility_handle_keydown(gameView,
                                                       (int)ev.key.keysym.sym,
                                                       gameViewResult)) {
                return M12_MENU_INPUT_NONE;
            }
            if (gameView && gameView->active &&
                ev.key.keysym.sym == SDLK_h &&
                (ev.key.keysym.mod & KMOD_CTRL)) {
                return M12_MENU_INPUT_GRAPHICS_POPUP;
            }
            if (m11_game_view_is_csb(gameView)) {
                M12_MenuInput csbInput = M12_MENU_INPUT_NONE;
                if (m11_csb_sdl_key_to_menu_input((int)ev.key.keysym.sym,
                                                  (ev.key.keysym.mod & KMOD_CTRL) != 0,
                                                  &csbInput)) {
                    return csbInput;
                }
            }
            if (gameView && gameView->active &&
                m11_game_view_supports_held_motion_input(gameView) &&
                ev.key.repeat) {
                M12_MenuInput repeatInput =
                    gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN
                        ? M11_NexusMovementInputFromScancode(
                              (int)ev.key.keysym.scancode)
                        : m11_game_view_is_theron(gameView)
                        ? M11_TheronGameplayInputFromScancodeWithButtonII(
                              (int)ev.key.keysym.scancode,
                              gameView->theronButtonIIHeld)
                        : M11_DM1V1_NavigationInputFromScancode(
                              (int)ev.key.keysym.scancode);
                if (repeatInput == M12_MENU_INPUT_NONE) {
                    repeatInput =
                        m11_motion_input_from_scancode(ev.key.keysym.scancode);
                }
                if (DM1_V1_InputMenuTokenUsesHeldRepeatPc34Compat(
                        (int)repeatInput)) {
                    /* See the SDL3 branch above: held-key polling owns
                     * DM1's paced repeat after F0361/F0380. */
                    return M12_MENU_INPUT_NONE;
                }
            }
            if (gameView && gameView->active) {
                M12_MenuInput mappedInput = M12_MENU_INPUT_NONE;
                if (gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN &&
                    gameView->nexusState.title_active) {
                    mappedInput = M11_NexusStartupMenuInputFromScancode(
                        (int)ev.key.scancode,
                        gameView->nexusState.title_active);
                    if (mappedInput != M12_MENU_INPUT_NONE) {
                        return mappedInput;
                    }
                }
                if (m11_game_view_is_theron(gameView)) {
                    const int gameplayInput =
                        m11_game_view_supports_held_motion_input(gameView);
                    mappedInput = gameplayInput
                        ? M11_TheronGameplayInputFromScancodeWithButtonII(
                              (int)ev.key.keysym.scancode,
                              gameView->theronButtonIIHeld)
                        : M11_TheronNavigationInputFromScancode(
                              (int)ev.key.keysym.scancode);
                    if (mappedInput == M12_MENU_INPUT_NONE) {
                        mappedInput = gameplayInput
                            ? M11_TheronGameplayInputFromKeycodeWithButtonII(
                                  (int)ev.key.keysym.sym,
                                  gameView->theronButtonIIHeld)
                            : M11_TheronNavigationInputFromKeycode(
                                  (int)ev.key.keysym.sym);
                    }
                    if (mappedInput != M12_MENU_INPUT_NONE) {
                        return mappedInput;
                    }
                } else if (gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN) {
                    mappedInput = M11_NexusLeaderInputFromScancode(
                        (int)ev.key.keysym.scancode,
                        (ev.key.keysym.mod & KMOD_SHIFT) != 0);
                    if (mappedInput != M12_MENU_INPUT_NONE) return mappedInput;
                    mappedInput = M11_NexusPointerToggleInputFromScancode(
                        (int)ev.key.keysym.scancode);
                    if (mappedInput != M12_MENU_INPUT_NONE) return mappedInput;
                    if (gameView->nexusState.hand_pointer_active) {
                        mappedInput = M11_NexusPointerInputFromScancode(
                            (int)ev.key.keysym.scancode);
                        if (mappedInput != M12_MENU_INPUT_NONE) return mappedInput;
                    }
                } else if (m11_game_view_is_dm1(gameView)) {
                    if (m11_dm1_sdl_key_to_menu_input(
                            (int)ev.key.keysym.sym,
                            (ev.key.keysym.mod & KMOD_CTRL) != 0,
                            (ev.key.keysym.mod & KMOD_SHIFT) != 0,
                            &mappedInput)) {
                        return mappedInput;
                    }
                    mappedInput =
                        M11_DM1V1_NavigationInputFromScancode((int)ev.key.keysym.scancode);
                    if (mappedInput != M12_MENU_INPUT_NONE) {
                        return mappedInput;
                    }
                }
                mappedInput = gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN
                    ? M11_NexusMovementInputFromScancode(
                          ev.key.keysym.scancode)
                    : M12_MENU_INPUT_NONE;
                if (mappedInput != M12_MENU_INPUT_NONE) return mappedInput;
                mappedInput = m11_motion_input_from_scancode(ev.key.keysym.scancode);
                if (mappedInput != M12_MENU_INPUT_NONE) {
                    return mappedInput;
                }
            }
            switch (ev.key.keysym.sym) {
                case SDLK_UP:
                    return M12_MENU_INPUT_UP;
                case SDLK_DOWN:
                    return M12_MENU_INPUT_DOWN;
                /* ReDMCSB PC-34/I34E source lock: COMMAND.C:677-684 maps
                 * keypad scancodes 0x4B/0x4C/0x4D/0x4F/0x50/0x51 to
                 * turn-left/forward/turn-right/strafe-left/back/strafe-right;
                 * IO2.C:47-59 normalizes shifted arrow scancodes into the
                 * same command-table codes. SDL reports NumLock-on keypad
                 * keys as SDLK_KP_N, so route them explicitly before the
                 * generic q/e/wasd convenience aliases. */
                case SDLK_KP_5:
                    return M12_MENU_INPUT_UP;
                case SDLK_KP_2:
                    return M12_MENU_INPUT_DOWN;
                case SDLK_KP_1:
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_KP_3:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                /* v2.8.x: arrow Left/Right now mean strafe (matches
                 * the original DM1 PC 3.4 convention; see also the
                 * user's keyboard-mapping request).  Q/E + Home/End +
                 * KP_4/KP_6 produce the turn-left/turn-right input
                 * tokens.  KP_4/KP_6 stay turn-left/turn-right
                 * because that's what COMMAND.C:677-684 maps them to
                 * on the original PC 3.4 keyboard (source-locked by
                 * test_dm1_v1_input_command_queue_pc34_compat). */
                case SDLK_LEFT:
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_RIGHT:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                case SDLK_KP_4:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_KP_6:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_Q:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_E:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_HOME:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_END:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_A:
                    /* v2.9.x: see the SDL3 branch above; WASD is now
                     * unconditional and mirrors the arrow keys. */
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_D:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                case SDLK_W:
                    return M12_MENU_INPUT_UP;
                case SDLK_S:
                    if (ev.key.keysym.mod & KMOD_CTRL) {
                        if (gameView && gameView->active)
                            return M12_MENU_INPUT_SAVE_GAME;
                        return M12_MENU_INPUT_NONE;
                    }
                    return M12_MENU_INPUT_DOWN;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    return M12_MENU_INPUT_ACCEPT;
                case SDLK_ESCAPE:
                    return M12_MENU_INPUT_BACK;
                case SDLK_SPACE:
                    return M12_MENU_INPUT_ACTION;
                case SDLK_TAB:
                    return M12_MENU_INPUT_CYCLE_CHAMPION;
                case SDLK_F5: {
                    /* F5 = quick save */
                    if (gameView && gameView->active) {
                        if (M11_GameView_QuickSave(gameView)) {
                            if (m11_dm2_host_save_log_allowed(gameView)) {
                                fprintf(stderr, "SAVE: quicksave written\n");
                            }
                        } else if (m11_dm2_host_save_log_allowed(gameView)) {
                            fprintf(stderr, "SAVE FAILED: quicksave rejected\n");
                        }
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                }
                case SDLK_F9: {
                    /* F9 = quick load */
                    if (gameView && gameView->active) {
                        if (M11_GameView_QuickLoad(gameView)) {
                            if (m11_dm2_host_save_log_allowed(gameView)) {
                                fprintf(stderr, "LOAD: quicksave restored\n");
                            }
                            if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                        } else if (m11_dm2_host_save_log_allowed(gameView)) {
                            fprintf(stderr, "LOAD FAILED: quicksave rejected\n");
                        }
                    }
                    return M12_MENU_INPUT_NONE;
                }
                case SDLK_F7:
                    if (gameView && gameView->active) {
                        int on = M11_QolRuntime_ToggleMinimap();
                        fprintf(stderr, "QoL: minimap %s\n", on ? "on" : "off");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_F8:
                    if (gameView && gameView->active) {
                        int ex = DM1_V1_AutoMap_ExportCurrentLevelPc34Compat(gameView);
                        fprintf(stderr, "QoL: auto-map export %s\n",
                                ex ? "ok" : "FAILED");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_l:
                    if (gameView && gameView->active) {
                        int on = M11_QolRuntime_ToggleCombatLog();
                        fprintf(stderr, "QoL: combat log %s\n", on ? "on" : "off");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_r:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_REST_TOGGLE;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_x:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_USE_STAIRS;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_g:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_PICKUP_ITEM;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_p:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_DROP_ITEM;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_1:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_1;
                    return M12_MENU_INPUT_NONE;
                case SDLK_2:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_2;
                    return M12_MENU_INPUT_NONE;
                case SDLK_3:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_3;
                    return M12_MENU_INPUT_NONE;
                case SDLK_4:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_4;
                    return M12_MENU_INPUT_NONE;
                case SDLK_5:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_5;
                    return M12_MENU_INPUT_NONE;
                case SDLK_6:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_6;
                    return M12_MENU_INPUT_NONE;
                case SDLK_c:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_CAST;
                    return M12_MENU_INPUT_NONE;
                case SDLK_v:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_CLEAR;
                    return M12_MENU_INPUT_NONE;
                case SDLK_u:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_USE_ITEM;
                    return M12_MENU_INPUT_NONE;
                case SDLK_F10:
                    return (gameView && gameView->active)
                        ? M12_MENU_INPUT_GRAPHICS_POPUP
                        : M12_MENU_INPUT_NONE;
                case SDLK_F11:
                    M11_Render_ToggleFullscreen();
                    return M12_MENU_INPUT_NONE;
                case SDLK_F12:
                    m11_capture_user_screenshot(gameView, menuState);
                    return M12_MENU_INPUT_NONE;
                default:
                    break;
            }
        }
#endif
    }
    return M12_MENU_INPUT_NONE;
}

int M11_PhaseA_Run(const M11_PhaseA_Options* opts) {
    M11_PhaseA_Options defaults;
    M11_PhaseA_SetDefaultOptions(&defaults);
    M11_PhaseA_Options runtimeOptions;
    const M11_PhaseA_Options* o;
    M11_DebugStartupTrace debugStartupTrace = {0};
    g_m11_phase_duration_ms = -1;
    g_m11_phase_duration_started_ms = 0U;
    g_m11_phase_duration_expired = 0;
    runtimeOptions = opts ? *opts : defaults;
    g_m11_debug_startup_detail = 0;
    /* A direct launch always names a concrete game. Treat a stale caller flag
     * without a game id as a launcher request so an ordinary app start can
     * never inherit the default DM1 card as an implicit launch selection. */
    if (!runtimeOptions.gameId || !runtimeOptions.gameId[0]) {
        runtimeOptions.directLaunch = 0;
    }
    /* Keep the public options boundary consistent with the CLI parser:
     * requesting the interactive launcher always disables direct launch. */
    if (runtimeOptions.menuRequested) {
        runtimeOptions.directLaunch = 0;
    }
    if (runtimeOptions.csbHintOracle && (!runtimeOptions.dataDir ||
                                         !runtimeOptions.dataDir[0])) {
        fprintf(stderr, "firestaff: --csb-hint-oracle requires --data-dir\n");
        return 2;
    }
    if (runtimeOptions.csbFmtownsUtilityDisk &&
        (!runtimeOptions.gameId ||
         strcmp(runtimeOptions.gameId, "csb") != 0)) {
        fprintf(stderr, "firestaff: --csb-utility-disk requires CSB FM Towns media\n");
        return 2;
    }
    if (runtimeOptions.bootProbe && (!runtimeOptions.gameId || runtimeOptions.gameId[0] == '\0')) {
        fprintf(stderr, "firestaff: --boot-probe requires --game <id>\n");
        return 2;
    }
    if (runtimeOptions.bootProbe && runtimeOptions.menuRequested) {
        fprintf(stderr,
                "firestaff: --boot-probe exercises direct launch and cannot be combined with --menu\n");
        return 2;
    }
    /* CSB has no original DOS/PC release. Reject this explicit CLI pairing
     * before SDL, renderer setup, or the synchronous media scan. Keep the
     * later catalogue guard as protection for selections made in the menu. */
    if (runtimeOptions.gameId &&
        strcmp(runtimeOptions.gameId, "csb") == 0 &&
        runtimeOptions.architectureOverride == M12_ARCH_PC) {
        fprintf(stderr,
                "firestaff: CSB has no original DOS/PC release; use --platform amiga, atari-st, or fm-towns\n");
        return 2;
    }
    if (runtimeOptions.bootProbe) {
        char invalidToken[64];
        int invalidCount = M11_BootProbeScript_Validate(runtimeOptions.script,
                                                        invalidToken,
                                                        sizeof(invalidToken));
        if (invalidCount > 0) {
            fprintf(stderr,
                    "firestaff: --boot-probe script contains %d invalid token(s); first invalid token '%s'\n",
                    invalidCount,
                    invalidToken);
            return 5;
        }
        runtimeOptions.directLaunch = 1;
        /* Boot probes are a receipt-producing CLI contract, not an
         * interactive presentation run.  Make them deterministic on CI and
         * on hosts without a window server when the caller did not already
         * select an SDL video driver.  An explicit caller choice still wins;
        * normal gameplay never enters this branch. */
        if (!getenv("SDL_VIDEODRIVER")) {
            (void)FSP_SetEnv("SDL_VIDEODRIVER", "dummy", 0);
        }
    }
    m11_apply_persisted_window_size(&runtimeOptions);
    /* Enable the disk-backed automation manifest only on request. It writes
     * ~/.firestaff/accessibility.json atomically from M11_GameView_Draw();
     * leaving it always-on made normal gameplay do per-frame filesystem I/O. */
    if (getenv("FS_ACCESSIBILITY")) {
        fs_ax_set_enabled(1);
    }
    /* An unqualified game selection follows the edition priority, even when
     * a previous launcher session persisted a different platform choice. */
    if (m11_is_supported_game_id(runtimeOptions.gameId) &&
        runtimeOptions.architectureOverride < M12_ARCH_AUTO)
        runtimeOptions.architectureOverride = M12_ARCH_AUTO;
    o = &runtimeOptions;
    g_m11_debug_startup_detail = o->debug ? 1 : 0;
    if (o->verbose) {
        fprintf(stderr, "firestaff: startup game=%s mode=%s platform=%s data=%s\n",
                o->gameId ? o->gameId : "none",
                o->directLaunch ? "direct" : "menu",
                o->architectureOverride == M12_ARCH_AUTO ? "auto" :
                    M12_Architecture_Label(o->architectureOverride),
                o->dataDir && o->dataDir[0] ? o->dataDir : "default search roots");
    }
    M12_StartupMenuState menuState;
    M11_GameViewState gameView;
    const char* scriptCursor = o->script;
    int scriptWaitFramesRemaining = 0;
    int scriptWaitForRuntime = 0;
    int scriptWaitForScan = 0;
    unsigned char* launcherFramebuffer = NULL;
    unsigned char* modernRgba = NULL;
    int useModern = 0;
    int quitRequested = 0;
    uint32_t idleAccumulatorMs = 0;
    M12_GamepadMap gamepadMap;
    M12_GamepadStatus gamepadStatus;
    int gamepadConfiguredEnabled;
    struct Dm1V1PendingMotionQueuePc34Compat pendingDm1V1MotionQueue;
    Firestaff_RA_Runtime raRuntime;

    int startupWindowMode = M11_WINDOW_MODE_MAXIMIZED;
    {
        M12_Config startupConfig;
        M12_Config_Load(&startupConfig, o->dataDir);
        M11_Render_SetBackendPreference(startupConfig.rendererBackendIndex);
        if (startupConfig.windowModeIndex >= M11_WINDOW_MODE_WINDOWED &&
            startupConfig.windowModeIndex <= M11_WINDOW_MODE_FULLSCREEN) {
            startupWindowMode = startupConfig.windowModeIndex;
        }
    }
    if (o->windowModeOverride >= M11_WINDOW_MODE_WINDOWED &&
        o->windowModeOverride <= M11_WINDOW_MODE_FULLSCREEN) {
        startupWindowMode = o->windowModeOverride;
    }
    int rc = M11_Render_InitWithWindowMode(o->windowWidth, o->windowHeight,
                                           o->scaleMode, startupWindowMode);
    if (rc != M11_RENDER_OK) {
        return rc;
    }
    M11_Render_SetPresentTimingEnabled(o->debug);
    if (o->debug) debugStartupTrace.startedMs = SDL_GetTicks();
    /* SDL's dummy driver can retain its default 1024x768 logical window even
     * when a test requested another size.  Scripted mouse events use the
     * requested host-window coordinates, so keep the renderer mapper on that
     * same surface in every headless mode, including the ordinary main loop. */
    if (SDL_GetCurrentVideoDriver() != NULL &&
        strcmp(SDL_GetCurrentVideoDriver(), "dummy") == 0) {
        (void)M11_Render_HandleResize(o->windowWidth, o->windowHeight);
    }
    if (o->debug) M11_Debug_LogHardware();
    if (o->verbose) {
        int windowWidth = 0;
        int windowHeight = 0;
        int drawableWidth = 0;
        int drawableHeight = 0;
        int presentX = 0;
        int presentY = 0;
        int presentWidth = 0;
        int presentHeight = 0;
        const int windowMode = M11_Render_GetWindowMode();
        const char* windowModeName =
            windowMode == M11_WINDOW_MODE_WINDOWED ? "windowed" :
            windowMode == M11_WINDOW_MODE_MAXIMIZED ? "maximized" :
            windowMode == M11_WINDOW_MODE_FULLSCREEN ? "fullscreen" : "unknown";
        (void)M11_Render_GetWindowAndDrawableSize(
            &windowWidth, &windowHeight, &drawableWidth, &drawableHeight);
        (void)M11_Render_GetPresentRect(&presentX, &presentY,
                                        &presentWidth, &presentHeight);
        fprintf(stderr,
                "firestaff: renderer ready video-driver=%s window=%dx%d drawable=%dx%d present=%d,%d %dx%d window-mode=%s scale=%d aspect=%d\n",
                SDL_GetCurrentVideoDriver() ? SDL_GetCurrentVideoDriver() : "unknown",
                windowWidth, windowHeight, drawableWidth, drawableHeight,
                presentX, presentY, presentWidth, presentHeight,
                windowModeName, o->scaleMode,
                M11_Render_GetDisplayAspectMode());
    }
    launcherFramebuffer = (unsigned char*)calloc(
        (size_t)M11_LAUNCHER_FB_WIDTH,
        (size_t)M11_LAUNCHER_FB_HEIGHT);
    if (!launcherFramebuffer) {
        M11_Render_Shutdown();
        return M11_RENDER_ERR_TEXTURE;
    }
    /* The startup renderer is always modern. Allocate its surface before the
     * synchronous first scan so startup and return-to-menu progress use the
     * same true-color launcher presentation. */
    modernRgba = (unsigned char*)calloc(
        (size_t)M11_LAUNCHER_MODERN_WIDTH *
            (size_t)M11_LAUNCHER_MODERN_HEIGHT,
        4U);
    if (!modernRgba) {
        free(launcherFramebuffer);
        M11_Render_Shutdown();
        return M11_RENDER_ERR_TEXTURE;
    }
    useModern = 1;
    /* Present a black frame and pump events before the potentially slow
     * asset scan so macOS does not flag the window as unresponsive. */
    {
        memset(launcherFramebuffer, 0,
               (size_t)M11_LAUNCHER_FB_WIDTH * (size_t)M11_LAUNCHER_FB_HEIGHT);
        M11_Render_PresentIndexed(launcherFramebuffer,
                                  M11_LAUNCHER_FB_WIDTH, M11_LAUNCHER_FB_HEIGHT);
        if (M11_Render_PumpEvents()) quitRequested = 1;
    }
    {
        M12_StartupMenuInitOptions menuInitOptions;
        M11_ScanProgressContext scanCtx;
        M12_Config scanConfig;
        m11_ttf_renderer_init();
        M12_Config_Load(&scanConfig, o->dataDir);
        if (o->languageOverride >= 0 && o->languageOverride < 20) {
            /* The CLI is an explicit user choice, including while the scan
             * progress UI is visible.  It must outrank a saved AUTO policy. */
            scanConfig.languageIndex = o->languageOverride;
        }
        memset(&menuInitOptions, 0, sizeof(menuInitOptions));
        /* Admission must use the requested original-world language on the
         * first scan, before selecting an edition or reporting availability. */
        menuInitOptions.hasLanguageOverride =
            o->languageOverride >= 0 && o->languageOverride < 20;
        menuInitOptions.languageOverride = o->languageOverride;
        menuInitOptions.skipScreenshotGalleryScan = o->bootProbe ? 1 : 0;
        /* A diagnostic launch uses the same bounded, authenticated media
         * admission as ordinary startup, including persisted/default ZIPs. */
        menuInitOptions.looseFilesOnlyAssetScan = 0;
        /* A no-game launch opens the collection menu by default, so discover
         * all installed titles even when --menu was not explicit. An explicit
         * file/container path remains a precise media selection: keep its
         * selected-game scan so a sibling loose copy cannot silently replace
         * the source the user chose. */
        menuInitOptions.scanAllGames =
            ((!o->gameId || o->gameId[0] == '\0') &&
             (!o->dataDir || o->dataDir[0] == '\0' ||
              FSP_DirExists(o->dataDir))) ||
            (o->menuRequested &&
             (!o->dataDir || o->dataDir[0] == '\0' ||
              FSP_DirExists(o->dataDir)));
        /* With no game selected, overlap full collection admission with the
         * branded startup intro. The intro displays scan progress and waits
         * for authenticated availability before the menu accepts a choice.
         * Explicit game launches remain synchronous because platform
         * overrides need a matched edition before they can be applied. */
        menuInitOptions.skipAssetScan =
            menuInitOptions.scanAllGames && (!o->gameId || !o->gameId[0]) &&
            !o->bootProbe;
        scanCtx.framebuffer = launcherFramebuffer;
        scanCtx.modernRgba = modernRgba;
        scanCtx.width = M11_LAUNCHER_FB_WIDTH;
        scanCtx.height = M11_LAUNCHER_FB_HEIGHT;
        scanCtx.languageIndex = scanConfig.languageIndex;
        scanCtx.colorblindMode = scanConfig.colorblindMode;
        scanCtx.uiScale = scanConfig.uiScale;
        scanCtx.debug = o->debug;
        scanCtx.useModern = 1;
        scanCtx.quitRequested = &quitRequested;
        scanCtx.autotestQuitInjected = 0;
        scanCtx.returnMenuRescan = 0;
        scanCtx.startedMs = SDL_GetTicks();
        menuInitOptions.scanProgressFn = m11_scan_progress_callback;
        menuInitOptions.scanProgressUserData = &scanCtx;
        M12_StartupMenu_InitWithOptions(&menuState,
                                        o->dataDir,
                                        o->gameId,
                                        &menuInitOptions);
        menuState.debugStartup = o->debug ? 1 : 0;
        if (o->languageOverride >= 0 && o->languageOverride < 20) {
            static const char* const languageCodes[] = {
                "en", "sv", "fr", "de", "ja", "zh", "cs", "da", "es", "fi",
                "hu", "it", "ko", "nl", "no", "pl", "pt", "ru", "tr", "id"
            };
            menuState.settings.languageIndex = o->languageOverride;
            menuState.languageExplicit = 1;
            /* --lang is an explicit user request and therefore must also
             * replace a stale per-game Custom-card language. */
            for (int gameIndex = 0; gameIndex < M12_ASSET_GAME_COUNT;
                 ++gameIndex) {
                menuState.gameOptions[gameIndex].languageIndex =
                    o->languageOverride;
            }
            fs_l10n_set_language(
                fs_l10n_language_from_locale(languageCodes[o->languageOverride]));
        }
    }
    if (!quitRequested && o->architectureOverride == M12_ARCH_AUTO) {
        if (!m11_apply_auto_architecture_override(&menuState, o->gameId)) {
            if (o->directLaunch) {
                fprintf(stderr,
                        "firestaff: game unavailable for --game: %s\n",
                        o->gameId ? o->gameId : "(null)");
                free(launcherFramebuffer);
                free(modernRgba);
                M11_Render_Shutdown();
                return 2;
            }
            /* An interactive menu must still open when its initially
             * selected game's only files are unsupported.  Keep AUTO
             * visible with no bound release so its platform card remains
             * unavailable and cannot fall through to a different edition. */
            if (o->gameId) {
                int slot = m11_game_option_slot(o->gameId);
                menuState.gameOptions[slot].architectureIndex = M12_ARCH_AUTO;
                menuState.gameOptions[slot].versionIndex = -1;
            }
        }
    } else if (!quitRequested &&
               o->architectureOverride > M12_ARCH_AUTO) {
        if (!m11_apply_architecture_override(&menuState,
                                             o->gameId,
                                             o->architectureOverride)) {
            /* CSB never had an original DOS release.  Say that explicitly
             * instead of leaving an apparent generic platform error which
             * could invite a caller to substitute the same-hash Atari ST
             * GRAPHICS.DAT or a stale materialized cache.  CSBWin is a
             * source/disassembly reference, not a Firestaff PC catalogue
             * row or a substitute game package. */
            if (o->gameId && strcmp(o->gameId, "csb") == 0 &&
                o->architectureOverride == M12_ARCH_PC) {
                fprintf(stderr,
                        "firestaff: CSB has no original DOS/PC release; use --platform amiga, atari-st, or fm-towns\n");
            } else {
                fprintf(stderr,
                        "firestaff: requested platform %s is not catalogued for the selected game\n",
                        M12_Architecture_Label(o->architectureOverride));
            }
            free(launcherFramebuffer);
            free(modernRgba);
            M11_Render_Shutdown();
            return 2;
        }
    }
    if (!quitRequested && o->csbFmtownsJapanese) {
        if (!o->gameId || strcmp(o->gameId, "csb") != 0 ||
            !m11_apply_csb_fmtowns_japanese_override(&menuState)) {
            fprintf(stderr,
                    "firestaff: --csb-fmtowns-ja requires matched CSB FM Towns Japanese media\n");
            free(launcherFramebuffer);
            free(modernRgba);
            M11_Render_Shutdown();
            return 2;
        }
    }
    if (!quitRequested && o->dm1FmtownsJapanese) {
        if (!o->gameId || strcmp(o->gameId, "dm1") != 0 ||
            !m11_apply_dm1_fmtowns_japanese_override(&menuState)) {
            fprintf(stderr,
                    "firestaff: --dm1-fmtowns-ja requires matched DM1 FM Towns Japanese media\n");
            free(launcherFramebuffer);
            free(modernRgba);
            M11_Render_Shutdown();
            return 2;
        }
    }
    if (o->verbose && m11_is_supported_game_id(o->gameId)) {
        int slot = m11_game_option_slot(o->gameId);
        int selected = menuState.gameOptions[slot].versionIndex;
        const M12_AssetVersionStatus* version = selected >= 0
            ? M12_AssetStatus_GetVersion(&menuState.assetStatus, o->gameId,
                                         (size_t)selected) : NULL;
        fprintf(stderr, "firestaff: selected game=%s platform=%s edition=%s source=%s\n",
                o->gameId,
                selected >= 0 ? M12_Architecture_Label(
                    M12_AssetStatus_GetVersionArchitecture(o->gameId,
                                                            (size_t)selected)) : "unknown",
                version && version->versionId ? version->versionId : "none",
                version && version->matchedPath[0] ? version->matchedPath : "none");
    }
    if (o->dm2EnglishCompanionPath && o->dm2EnglishCompanionPath[0] != '\0') {
        snprintf(menuState.dm2EnglishCompanionPath,
                 sizeof(menuState.dm2EnglishCompanionPath), "%s",
                 o->dm2EnglishCompanionPath);
    }
    /* The playback device is a launcher-wide host preference, not a
     * per-game effect.  Bind it before title/entrance audio can create a
     * stream, then M11_GameView repeats the same handoff at game start after
     * each game-owned audio reinitialisation. */
    M11_Audio_SetPreferredPlaybackDeviceName(menuState.settings.audioDeviceName);
    menuState.settings.windowWidth = M11_Render_GetWindowWidth();
    menuState.settings.windowHeight = M11_Render_GetWindowHeight();
    if (o->scaleModeOverride) {
        menuState.settings.scaleModeIndex = o->scaleMode;
    }
    if (o->windowModeOverride >= M11_WINDOW_MODE_WINDOWED &&
        o->windowModeOverride <= M11_WINDOW_MODE_FULLSCREEN) {
        menuState.settings.windowModeIndex = o->windowModeOverride;
    }
    if (o->vsyncOverride == 0 || o->vsyncOverride == 1) {
        menuState.settings.vsyncIndex = o->vsyncOverride;
    }
    if (o->musicEnabledOverride == 0 || o->musicEnabledOverride == 1) {
        /* Music is intentionally separate from the all-audio mute switch:
         * users may keep UI, effects and accessibility cues while disabling
         * title and in-game music for any native game route.  The value is
         * exported with the launch intent and reapplied after game-specific
         * audio initialisation. */
        menuState.settings.audioMusicVolume =
            o->musicEnabledOverride ? 128 : 0;
    }
    if (o->presentationModeOverride >= M12_PRESENTATION_V1_ORIGINAL &&
        o->presentationModeOverride < M12_PRESENTATION_MODE_COUNT) {
        int game_slot = 0;
        if (o->gameId && strcmp(o->gameId, "csb") == 0) game_slot = 1;
        else if (o->gameId && strcmp(o->gameId, "dm2") == 0) game_slot = 2;
        else if (o->gameId && strcmp(o->gameId, "nexus") == 0) game_slot = 3;
        else if (o->gameId && strcmp(o->gameId, "theron") == 0) game_slot = 4;
        menuState.settings.graphicsIndex = o->presentationModeOverride;
        menuState.gameOptions[game_slot].presentationModeIndex =
            o->presentationModeOverride;
    }
    useModern = m11_should_use_modern_launcher(&menuState);
    if (useModern) {
        if (!modernRgba) {
            modernRgba = (unsigned char*)calloc(
                (size_t)M11_LAUNCHER_MODERN_WIDTH *
                    (size_t)M11_LAUNCHER_MODERN_HEIGHT,
                4U);
        }
        if (!modernRgba) {
            /* Do not fall back to the legacy indexed launcher: it has no
             * mouse hit routing, which leaves the application visibly alive
             * but impossible to operate with a pointer. */
            free(launcherFramebuffer);
            M11_Render_Shutdown();
            return M11_RENDER_ERR_TEXTURE;
        }
    }
    /* Boot probes exercise the launch/runtime contract and emit a
     * machine-readable receipt; they do not draw translated launcher text.
     * Keep that path independent of filesystem-backed PO catalogs so a
     * headless DM1 entrance/HoC probe cannot be delayed by unrelated UI I/O. */
    if (!o->bootProbe) {
        const char* langCodes[] = {
            "en", "sv", "fr", "de", "ja", "zh",
            "cs", "da", "es", "fi", "hu", "it",
            "ko", "nl", "no", "pl", "pt", "ru", "tr", "id"
        };
        const char* langCode = "en";
        /* The menu state already resolves AUTO to the host locale and stores
         * an explicit user selection verbatim. Using GetAutoLanguageIndex()
         * here ignored a language chosen in the startup menu for every
         * in-game domain. */
        int langIdx = menuState.settings.languageIndex;
        if (langIdx >= 0 && langIdx < 20) langCode = langCodes[langIdx];
        (void)m11_load_po_domain("startup-menu", langCode, o->dataDir);
        /* The startup catalog cannot serve in-game strings: the loader is
         * intentionally domain-isolated. Load every native game domain once
         * so Original and Modern can translate source-owned presentation text
         * without tying localization availability to the selected launch
         * route. Missing or empty entries fall through to the exact retail
         * source msgid. */
        {
            static const char* gameDomains[] = {
                "dm1", "csb", "dm2", "nexus", "theron"
            };
            size_t domainIndex;
            for (domainIndex = 0;
                 domainIndex < sizeof(gameDomains) / sizeof(gameDomains[0]);
                 ++domainIndex) {
                (void)m11_load_po_domain(gameDomains[domainIndex], langCode,
                                         o->dataDir);
            }
        }
    }
    M11_GameView_Init(&gameView);
    gameView.fpsOverlayEnabled = menuState.settings.showFpsOverlay ? 1 : 0;
    if (o->showFpsOverlay) {
        gameView.fpsOverlayEnabled = 1;
    }
    if (o->debug) {
        gameView.fpsOverlayEnabled = 1;
    }
    DM1_V1_PendingMotionQueue_InitPc34Compat(&pendingDm1V1MotionQueue);
    M12_GamepadMap_SetDefaults(&gamepadMap);
    (void)M12_GamepadMap_Load(&gamepadMap);
    gamepadConfiguredEnabled = gamepadMap.enabled ? 1 : 0;
    memset(&gamepadStatus, 0, sizeof(gamepadStatus));
    M12_GamepadStatus_Update(&gamepadStatus);
    int launchedEver = 0;
    int inputRedrawDrawCount = 0;
    int inputRedrawAfterViewportDirtyCount = 0;
    int lastInputRedrawAfterViewportDirty = 0;
    int exitAfterLaunch = getenv("FIRESTAFF_EXIT_AFTER_LAUNCH") != NULL;
    int failIfNoLaunch = getenv("FIRESTAFF_FAIL_IF_NO_LAUNCH") != NULL;
    int runRc = 0;
    int startupTextInputActive = 0;
    if (!quitRequested && !o->bootProbe && !o->directLaunch &&
        o->durationMs < 0 &&
        getenv("FIRESTAFF_SKIP_INTRO") == NULL &&
        m11_play_firestaff_startup_intro(&menuState)) {
        goto cleanup;
    }
    if (menuState.deferredScanPending && !quitRequested) {
        if (!M12_StartupMenu_BeginDeferredScan(&menuState)) {
            M11_ScanProgressContext fallbackCtx;
            fallbackCtx.framebuffer = launcherFramebuffer;
            fallbackCtx.modernRgba = modernRgba;
            fallbackCtx.width = M11_LAUNCHER_FB_WIDTH;
            fallbackCtx.height = M11_LAUNCHER_FB_HEIGHT;
            fallbackCtx.languageIndex = menuState.settings.languageIndex;
            fallbackCtx.colorblindMode = menuState.settings.colorblindMode;
            fallbackCtx.uiScale = menuState.settings.uiScale;
            fallbackCtx.debug = o->debug;
            fallbackCtx.useModern = useModern;
            fallbackCtx.quitRequested = &quitRequested;
            fallbackCtx.autotestQuitInjected = 0;
            fallbackCtx.returnMenuRescan = 0;
            fallbackCtx.startedMs = SDL_GetTicks();
            M12_StartupMenu_RunDeferredScan(&menuState,
                                            m11_scan_progress_callback,
                                            &fallbackCtx);
            if (!quitRequested && !o->directLaunch)
                M12_StartupMenu_FinalizeDeferredScanMessages(&menuState);
        }
    }
    /* Terminal boot probes exit before the interactive loop. Their selected
     * edition is already known from the completed direct-game scan, so emit
     * the detailed file list here; ordinary no-game scans report after the
     * asynchronous result has been published. */
    if (o->verbose && o->bootProbe && !menuState.deferredScanActive) {
        static const char* const gameIds[] = {
            "dm1", "csb", "dm2", "nexus", "theron"
        };
        size_t gameIndex;
        for (gameIndex = 0U;
             gameIndex < sizeof(gameIds) / sizeof(gameIds[0]);
             ++gameIndex) {
            if (o->menuRequested || !o->gameId ||
                strcmp(o->gameId, gameIds[gameIndex]) == 0) {
                m11_verbose_original_media(&menuState.assetStatus,
                                           gameIds[gameIndex]);
            }
        }
    }
    M11_ApplyStartupMenuRuntime(&menuState);
    /* Query the applied renderer state, after saved settings and CLI
     * overrides. Creation-time VSync alone is not the gameplay policy. */
    if (o->verbose) {
        int windowWidth = 0, windowHeight = 0;
        int drawableWidth = 0, drawableHeight = 0;
        const char* rendererName = NULL;
        int vsyncInterval = 0;
        int vsyncAvailable = 0;
#if SDL_VERSION_ATLEAST(3, 2, 0)
        SDL_Renderer* renderer = M11_Render_GetRenderer();
        rendererName = SDL_GetRendererName(renderer);
        vsyncAvailable = SDL_GetRenderVSync(renderer, &vsyncInterval);
#endif
        (void)M11_Render_GetWindowAndDrawableSize(
            &windowWidth, &windowHeight, &drawableWidth, &drawableHeight);
        fprintf(stderr, "firestaff: renderer backend=%s vsync=",
                rendererName ? rendererName : "unavailable");
        if (vsyncAvailable) fprintf(stderr, "%d", vsyncInterval);
        else fprintf(stderr, "unavailable");
        fprintf(stderr, " drawable-scale=%.3fx%.3f\n",
                windowWidth > 0 ? (double)drawableWidth/windowWidth : 0.0,
                windowHeight > 0 ? (double)drawableHeight/windowHeight : 0.0);
    }
    /* Direct CLI selection normally resolves after startup presentation. Move
     * that preparation ahead of the source-owned I34E TITLE animation so its
     * authenticated edition and runtime data root are available to the player.
     * Keep the existing failure and unsupported-CSB handling unchanged. */
    if (o->directLaunch && !quitRequested && !o->csbHintOracle) {
        if (o->gameId && strcmp(o->gameId, "csb") == 0 &&
            o->architectureOverride == M12_ARCH_PC) {
            fprintf(stderr,
                    "firestaff: CSB has no original DOS/PC release; use --platform amiga, atari-st, or fm-towns\n");
            runRc = 2;
            goto cleanup;
        }
        if (!M11_PrepareDirectLaunchForGame(&menuState, o->gameId)) {
            fprintf(stderr, "firestaff: game unavailable for --game: %s\n",
                    o->gameId ? o->gameId : "(null)");
            runRc = 2;
            goto cleanup;
        }
    }
    /* IBMIO.C:2411-2418 runs ANIM TITLE before SELECTOR. In Firestaff the
     * launcher has already resolved the initial game/edition, so show only
     * its authenticated I34E title here and let the M12 menu take the
     * selector's place. Keep this outside the later F0437/F0441 transaction:
     * I34E never calls either routine. */
    if (!quitRequested && !o->bootProbe && o->durationMs < 0 &&
        getenv("FIRESTAFF_SKIP_INTRO") == NULL &&
        (o->directLaunch && o->gameId && strcmp(o->gameId, "dm1") == 0)) {
        if (g_m11_debug_startup_detail)
            fprintf(stderr, "firestaff: startup-source-route game=dm1 platform=pc34 phase=anim-title direct=%d selected-game=%d cli-game=%s\n",
                    o->directLaunch, (int)menuState.selectedGameId,
                    o->gameId ? o->gameId : "(none)");
        int versionIndex = menuState.gameOptions[0].versionIndex;
        const M12_AssetVersionStatus *version = versionIndex >= 0
            ? M12_AssetStatus_GetVersion(&menuState.assetStatus, "dm1",
                                         (size_t)versionIndex)
            : NULL;
        if (version && version->matched && version->versionId &&
            strcmp(version->versionId, "pc34-en") == 0 &&
            M12_AssetStatus_GetVersionArchitecture(
                "dm1", (size_t)versionIndex) == M12_ARCH_PC) {
            M11_Render_RaiseWindow();
            /* IBMIO.C:2411-2418 owns the preceding SWOOSH process and ignores
             * its media/playback result, then continues to ANIM TITLE. Preserve
             * that order; only a user/window quit aborts the title/menu route. */
            (void)m11_play_ftl_swoosh_for_game_if_available(
                &menuState, o->dataDir, "dm1", 0, NULL);
            g_m11_dm1_i34e_intro_preplayed = 1;
            if (g_m11_startup_intro_quit_requested) {
                quitRequested = 1;
                menuState.shouldExit = 1;
            } else {
                int titleResult = m11_play_dm1_i34e_title_anim_if_available(
                    &menuState, version);
                if (titleResult == M11_ENTRANCE_COMMAND_QUIT) {
                    quitRequested = 1;
                    menuState.shouldExit = 1;
                }
            }
        }
    }
    firestaff_ra_runtime_init(&raRuntime);
    if (o->retroAchievementsEnabled ||
        menuState.settings.retroAchievementsEnabled) {
        Firestaff_RA_Config raConfig;
        firestaff_ra_config_init(&raConfig);
        raConfig.enabled = 1;
        raConfig.hardcore = o->retroAchievementsEnabled
                                ? (o->retroAchievementsHardcore ? 1 : 0)
                                : (menuState.settings.retroAchievementsHardcore ? 1 : 0);
        firestaff_ra_set_credentials(&raConfig,
                                     o->retroAchievementsUser
                                         ? o->retroAchievementsUser
                                         : (menuState.settings.retroAchievementsUsername[0]
                                                ? menuState.settings.retroAchievementsUsername
                                                : getenv("FIRESTAFF_RA_USER")),
                                     o->retroAchievementsToken
                                         ? o->retroAchievementsToken
                                         : (menuState.settings.retroAchievementsToken[0]
                                                ? menuState.settings.retroAchievementsToken
                                                : getenv("FIRESTAFF_RA_TOKEN")));
        snprintf(raConfig.endpoint,
                 sizeof(raConfig.endpoint),
                 "%s",
                 o->retroAchievementsEndpoint
                     ? o->retroAchievementsEndpoint
                     : (menuState.settings.retroAchievementsEndpoint[0]
                            ? menuState.settings.retroAchievementsEndpoint
                            : "https://retroachievements.org"));
        raRuntime.backend_available = 1;
        firestaff_ra_runtime_apply_config(&raRuntime, &raConfig);
        /* Credentials are intentionally never included in diagnostics.  Even
         * a suffix from a redacted API token is unnecessary information in a
         * log that may be shared with a bug report. */
        fprintf(stderr,
                "RetroAchievements: %s user=%s credentials=%s hardcore=%d\n",
                firestaff_ra_status_label(firestaff_ra_status(&raRuntime)),
                raConfig.username[0] ? raConfig.username : "(none)",
                (raConfig.username[0] && raConfig.api_token[0])
                    ? "configured" : "missing",
                raConfig.hardcore);
        (void)m11_drain_retroachievements_events(&raRuntime, &gameView);
    }
    {
        M12_Config qolCfg;
        M12_Config_Load(&qolCfg, o->dataDir);
        M11_QolRuntime_InitFromConfig(&qolCfg);
    }

    /* Compute deadlines using millisecond ticks. SDL_GetTicks returns
       Uint64 in SDL3 and Uint32 in SDL2. Both are fine for our math. */
#if SDL_VERSION_ATLEAST(3, 0, 0)
    Uint64 start = SDL_GetTicks();
    Uint64 now = start;
    Uint64 lastLoopTick = start;
    const Uint64 interval = (Uint64)(o->presentEveryMs < 1
                                         ? 1
                                         : o->presentEveryMs);
    Uint64 gameTickInterval = 200; /* DM1 V1 authentic PAL: 10 VBlanks * 20ms = 200ms (VBLANK.C:F0577, GAMELOOP.C:F0002) */
#else
    Uint32 start = SDL_GetTicks();
    Uint32 now = start;
    Uint32 lastLoopTick = start;
    const Uint32 interval = (Uint32)(o->presentEveryMs < 1
                                         ? 1
                                         : o->presentEveryMs);
    Uint32 gameTickInterval = 200; /* DM1 V1 authentic PAL: 10 VBlanks * 20ms = 200ms (VBLANK.C:F0577, GAMELOOP.C:F0002) */
#endif
    /* --boot-probe has its own frame-bounded terminal contract and commonly
     * uses --duration 0 to avoid the interactive loop. Keep that probe budget
     * independent from the normal presentation deadline. */
    g_m11_phase_duration_ms = o->bootProbe ? -1 : o->durationMs;
    g_m11_phase_duration_started_ms = (Uint32)start;
    g_m11_phase_duration_expired = 0;

    /* Always present at least once so the window actually has content. */
    /* Keep an explicit CLI resume on the same M12 intent -> M11 launch
     * contract as the save browser.  This belongs outside directLaunch: with
     * `--menu --game ... --save ...`, the selected menu row must carry the
     * same supplied path when Enter requests its later handoff.  The
     * game-specific importer remains the only authority on the bytes. */
    if (o->savePath && o->savePath[0] != '\0' && !o->csbHintOracle) {
        if (o->gameId && strcmp(o->gameId, "dm2") == 0 &&
            !M12_StartupMenu_DM2ResumeSupportedOnSelectedPlatform(
                &menuState)) {
            fprintf(stderr,
                    "firestaff: DM2 Macintosh Resume is unavailable until its complete original GAME_LOAD path is supported\n");
            runRc = 2;
            goto cleanup;
        }
        snprintf(menuState.quickResumeSavePath,
                 sizeof(menuState.quickResumeSavePath),
                 "%s", o->savePath);
        snprintf(menuState.quickResumeGameId,
                 sizeof(menuState.quickResumeGameId),
                 "%s", o->gameId ? o->gameId : "");
        menuState.quickResumeAvailable = 1;
        menuState.quickResumeLaunchRequested = 1;
        if (o->menuRequested) {
            /* An explicit --save with --menu --game is a Quick Resume
             * request.  Leaving --game's highlighted row selected makes the
             * first Enter call m12_activate_selected(), which clears the
             * resume request and launches a fresh game instead.  Start on
             * the launcher's Quick Resume row so the supplied original path
             * remains the selected M12 intent. */
            menuState.selectedIndex = -1;
        }
        if (o->gameId && strcmp(o->gameId, "csb") == 0) {
            /* The menu discovery identity is mandatory for an implicit
             * CSBWin corpus choice.  `--save` is an explicit user path; let
             * F0435/original-container admission validate its bytes instead
             * of dropping it before the launch intent is built.  This token
             * is only the M12 handoff marker, never a save identity. */
            M12_StartupMenu_BindCSBSaveCandidateIdentity(&menuState, 1u);
        }
    }
    if (!quitRequested && o->csbHintOracle) {
        if (!M11_GameView_StartCsbHintOracle(&gameView, o->dataDir, o->savePath)) {
            fprintf(stderr, "firestaff: CSB Hint Oracle requires a verified Utility Disk HCSB pair and a valid native CSB save; pass --save for an explicit save path\n");
            runRc = 3;
            goto cleanup;
        }
        launchedEver = 1;
        fprintf(stdout,
                "CSB HINT ORACLE READY: save=%s source=%s native=in-memory\n",
                o->savePath && o->savePath[0] ? o->savePath : "auto",
                strcmp(gameView.sourceId, "csb-hint-oracle-amiga-au2-en") == 0
                    ? "amiga-au2-en" : "atari-r1");
        if (exitAfterLaunch && !o->bootProbe) {
            goto cleanup;
        }
    } else if (!quitRequested && o->csbFmtownsUtilityDisk) {
        if (!m11_open_requested_launch(&gameView, &menuState,
                                       &idleAccumulatorMs, o->dataDir,
                                       o->bootProbe) ||
            !M11_GameView_EnterCsbFmtownsUtility(&gameView)) {
            fprintf(stderr, "firestaff: CSB Utility Disk requires verified FM Towns CSB media\n");
            runRc = 3;
            goto cleanup;
        }
        launchedEver = 1;
    } else if (o->directLaunch && !quitRequested) {
        /* CLI direct launch bypasses only the M12 menu. The launch still
         * enters through M11_GameView_OpenSelectedMenuEntry(), so DM1 keeps
         * the ReDMCSB TITLE/ENTRANCE order (TITLE.C F0437 before
         * ENTRANCE.C F0441). */
        if (o->verbose && m11_is_supported_game_id(o->gameId))
            fprintf(stderr, "firestaff: launch phase=game-handoff game=%s\n",
                    o->gameId);
        g_m11_dm1_direct_cli_launch = 1;
        if (!m11_open_requested_launch(&gameView,
                                       &menuState,
                                       &idleAccumulatorMs,
                                       o->dataDir,
                                       o->bootProbe)) {
            g_m11_dm1_direct_cli_launch = 0;
            if (g_m11_debug_startup_detail &&
                (gameView.lastAction[0] || gameView.lastOutcome[0])) {
                fprintf(stderr,
                        "firestaff: launch-rejected game=%s action=%s outcome=%s\n",
                        o->gameId ? o->gameId : "(none)",
                        gameView.lastAction[0] ? gameView.lastAction : "(none)",
                        gameView.lastOutcome[0] ? gameView.lastOutcome : "(none)");
            }
            fprintf(stderr, "firestaff: direct launch failed for --game %s\n", o->gameId);
            runRc = 3;
            goto cleanup;
        }
        g_m11_dm1_direct_cli_launch = 0;
        if (menuState.shouldExit || g_m11_phase_duration_expired)
            goto cleanup;
        launchedEver = 1;
        /* Direct launches and bounded boot probes run before the interactive
         * loop sets this flag. Enable the same in-game event trace immediately
         * after the native handoff so scripted and live play share diagnostics. */
        gameView.debugMovementTraceEnabled = o->debug ? 1 : 0;
        if (o->debug) m11_debug_game_events(&gameView);
        if (o->verbose && m11_is_supported_game_id(o->gameId)) {
            M11_BootProbeReceipt startupReceipt;
            int hasStartupReceipt = M11_GameView_GetBootProbeReceipt(
                &gameView, &startupReceipt);
            const char *launchPhase = "runtime-entered";
            if (hasStartupReceipt) {
                if (strcmp(o->gameId, "nexus") == 0 &&
                    strcmp(startupReceipt.startupPhase, "nexus-title") == 0 &&
                    startupReceipt.startupActive &&
                    !startupReceipt.levelLoaded) {
                    launchPhase = "title-entered";
                }
            }
            fprintf(stderr, "firestaff: launch phase=%s game=%s\n",
                    launchPhase, o->gameId);
            if (hasStartupReceipt)
                fprintf(stderr, "firestaff: runtime game=%s startup-phase=%s animation=%s active=%d\n",
                        o->gameId, startupReceipt.startupPhase,
                        startupReceipt.startupAnimation,
                        startupReceipt.startupActive);
        }
        if (o->debug) {
            m11_debug_log_launch_selection(
                &menuState, &gameView,
                o->directLaunch ? "direct" : "menu");
            m11_debug_trace_startup_frame(&debugStartupTrace, &gameView,
                                          o->gameId);
        }
        if (o->bootProbe) {
            int frames = o->bootProbeFrames < 0 ? 0 : o->bootProbeFrames;
            int scriptInputs;
            int scriptFrames = 0;
            M11_BootProbeReceipt receipt;
            DM1_V1_StartupSelectedBootProbeFacts_PC34 selectedFacts;
            DM1_V1_StartupSelectedBootProbeReceipt_PC34 selectedReceipt;
            DM1_V1_StartupSelectedBootProbeSourceKindFacts_PC34 selectedKindFacts;
            DM1_V1_StartupSelectedBootProbeSourceKindReceipt_PC34 selectedKindReceipt;
            M11_GameSourceKind expectedSourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
            M11_GameView_SetBootProbeMode(&gameView, 1);
            m11_phase_a_advance_boot_probe_frames(&gameView, frames);
            scriptInputs = m11_phase_a_apply_boot_probe_script(&gameView,
                                                               o->script,
                                                               frames,
                                                               &scriptFrames);
            /* A boot-probe script may complete CSB's C004/C002/C003
             * entrance handoff without entering the ordinary frame loop.
             * Present the resulting F0128 runtime frame before emitting
             * receipts or optional captures: otherwise the receipt described
             * map 0 while the SDL surface still contained Entrance. */
            if (gameView.active) {
                M11_GameView_Draw(&gameView,
                                  M11_Render_GetFramebuffer(),
                                  M11_FB_WIDTH,
                                  M11_FB_HEIGHT);
                M11_GameView_DrawLeaderHandCursor(
                    &gameView, M11_Render_GetFramebuffer(),
                    M11_FB_WIDTH, M11_FB_HEIGHT);
                M11_GameView_RecordPresentedFrame(&gameView, SDL_GetTicks());
                M11_GameView_DrawFpsOverlay(&gameView,
                                             M11_Render_GetFramebuffer(),
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT);
                (void)m11_present_game_frame_and_publish_startup_capture(
                    &gameView, &menuState);
            }
            m11_phase_a_print_boot_probe_receipt(&gameView,
                                                 &menuState,
                                                 o->gameId,
                                                 frames,
                                                 scriptInputs,
                                                 scriptFrames);
            memset(&receipt, 0, sizeof(receipt));
            memset(&selectedFacts, 0, sizeof(selectedFacts));
            memset(&selectedReceipt, 0, sizeof(selectedReceipt));
            memset(&selectedKindFacts, 0, sizeof(selectedKindFacts));
            memset(&selectedKindReceipt, 0, sizeof(selectedKindReceipt));
            if (!M11_GameView_GetBootProbeReceipt(&gameView, &receipt)) {
                fprintf(stderr,
                        "firestaff: boot-probe could not read runtime receipt for --game %s\n",
                        o->gameId ? o->gameId : "");
                runRc = 4;
                goto boot_probe_terminal_exit;
            }
            selectedFacts.expected_game_id = o->gameId;
            selectedFacts.actual_source_id = receipt.sourceId;
            selectedFacts.active = receipt.active;
            selectedFacts.started_from_launcher = receipt.startedFromLauncher;
            selectedFacts.intro_bypassed = receipt.dm1StartupIntroBypassed;
            selectedFacts.verified_asset_md5 = receipt.bootAssetMd5;
            /* GS first-world startup has its own authenticated native route.
             * Preserve the PC startup validator and its intro requirements. */
            int selectedSourceReady = gameView.dm1IigsActive
                ? m11_boot_probe_iigs_source_ready(&gameView, &receipt, o->gameId)
                : (dm1_v1_startup_selected_boot_probe_receipt_pc34(
                       &selectedFacts, &selectedReceipt) && selectedReceipt.valid);
            if (!selectedSourceReady ||
                (selectedKindFacts.expected_game_id = o->gameId,
                 selectedKindFacts.actual_source_kind = (int)receipt.sourceKind,
                 selectedKindFacts.dm1_builtin_source_kind =
                     (int)M11_GAME_SOURCE_BUILTIN_CATALOG,
                 !dm1_v1_startup_selected_boot_probe_source_kind_receipt_pc34(
                     &selectedKindFacts,
                     &selectedKindReceipt)) ||
                (selectedKindReceipt.handled &&
                 !selectedKindReceipt.valid) ||
                (!selectedKindReceipt.handled &&
                 (!m11_boot_probe_expected_source_kind(o->gameId,
                                                       &expectedSourceKind) ||
                  receipt.sourceKind != expectedSourceKind))) {
                if (selectedKindReceipt.handled) {
                    expectedSourceKind =
                        (M11_GameSourceKind)selectedKindReceipt.expected_source_kind;
                }
                fprintf(stderr,
                        "firestaff: boot-probe expected selected-entry source '%s' kind=%d but got active=%d sourceId='%s' kind=%d startedFromLauncher=%d introBypassed=%d\n",
                        o->gameId ? o->gameId : "",
                        (int)expectedSourceKind,
                        receipt.active,
                        receipt.sourceId,
                        (int)receipt.sourceKind,
                        receipt.startedFromLauncher,
                        receipt.dm1StartupIntroBypassed);
                runRc = 4;
            }
            if (o->bootProbeExpectPhase && o->bootProbeExpectPhase[0] != '\0') {
                if (strcmp(receipt.startupPhase, o->bootProbeExpectPhase) != 0) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected phase '%s' but got '%s'\n",
                            o->bootProbeExpectPhase,
                            receipt.startupPhase);
                    runRc = 4;
                } else if (m11_boot_probe_expected_phase_is_runtime(
                               o->bootProbeExpectPhase) &&
                           !m11_boot_probe_runtime_receipt_ready(&receipt)) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected runtime-ready phase '%s' but receipt has active=%d startupActive=%d levelLoaded=%d\n",
                            o->bootProbeExpectPhase,
                            receipt.active,
                            receipt.startupActive,
                            receipt.levelLoaded);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectRuntime) {
                if (receipt.startupActive ||
                    !receipt.levelLoaded) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected runtime handoff but got phase='%s' startupActive=%d levelLoaded=%d\n",
                            receipt.startupPhase,
                            receipt.startupActive,
                            receipt.levelLoaded);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectParty) {
                if (receipt.partyX != o->bootProbeExpectPartyX ||
                    receipt.partyY != o->bootProbeExpectPartyY ||
                    receipt.partyDir != o->bootProbeExpectPartyDir) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected party %d,%d,%d but got %d,%d,%d\n",
                            o->bootProbeExpectPartyX,
                            o->bootProbeExpectPartyY,
                            o->bootProbeExpectPartyDir,
                            receipt.partyX,
                            receipt.partyY,
                            receipt.partyDir);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectChampions) {
                if (receipt.championCount != o->bootProbeExpectChampionCount) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected champions %d but got %d\n",
                            o->bootProbeExpectChampionCount,
                            receipt.championCount);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectLevelLoaded >= 0) {
                if (receipt.levelLoaded != o->bootProbeExpectLevelLoaded) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected levelLoaded=%d but got %d\n",
                            o->bootProbeExpectLevelLoaded,
                            receipt.levelLoaded);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectAssetMd5 &&
                o->bootProbeExpectAssetMd5[0] != '\0') {
                if (strcmp(receipt.bootAssetMd5,
                           o->bootProbeExpectAssetMd5) != 0) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected asset md5 '%s' but got '%s'\n",
                            o->bootProbeExpectAssetMd5,
                            receipt.bootAssetMd5);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectMap) {
                if (receipt.mapIndex != o->bootProbeExpectMapIndex) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected map %d but got %d\n",
                            o->bootProbeExpectMapIndex,
                            receipt.mapIndex);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectRuntimeTickMin >= 0) {
                if (receipt.runtimeTick < o->bootProbeExpectRuntimeTickMin) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected runtime tick >= %d but got %d\n",
                            o->bootProbeExpectRuntimeTickMin,
                            receipt.runtimeTick);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectRuntimeTickMax >= 0) {
                if (receipt.runtimeTick > o->bootProbeExpectRuntimeTickMax) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected runtime tick <= %d but got %d\n",
                            o->bootProbeExpectRuntimeTickMax,
                            receipt.runtimeTick);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectStartupActive >= 0) {
                if (receipt.startupActive != o->bootProbeExpectStartupActive) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected startupActive=%d but got %d\n",
                            o->bootProbeExpectStartupActive,
                            receipt.startupActive);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectStartupFrameMin >= 0) {
                if (receipt.startupFrame < o->bootProbeExpectStartupFrameMin) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected startup frame >= %d but got %d\n",
                            o->bootProbeExpectStartupFrameMin,
                            receipt.startupFrame);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectStartupFrameMax >= 0) {
                if (receipt.startupFrame > o->bootProbeExpectStartupFrameMax) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected startup frame <= %d but got %d\n",
                            o->bootProbeExpectStartupFrameMax,
                            receipt.startupFrame);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectStartupAnimation &&
                o->bootProbeExpectStartupAnimation[0] != '\0') {
                if (strcmp(receipt.startupAnimation,
                           o->bootProbeExpectStartupAnimation) != 0) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected startup animation '%s' but got '%s'\n",
                            o->bootProbeExpectStartupAnimation,
                            receipt.startupAnimation);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectStartupAnimationActive >= 0) {
                if (receipt.startupAnimationActive !=
                        o->bootProbeExpectStartupAnimationActive) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected startupAnimationActive=%d but got %d\n",
                            o->bootProbeExpectStartupAnimationActive,
                            receipt.startupAnimationActive);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectTitleFrameMin >= 0) {
                if (receipt.startupTitleFrame < o->bootProbeExpectTitleFrameMin) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected title frame >= %d but got %d\n",
                            o->bootProbeExpectTitleFrameMin,
                            receipt.startupTitleFrame);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectTitleFrameMax >= 0) {
                if (receipt.startupTitleFrame > o->bootProbeExpectTitleFrameMax) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected title frame <= %d but got %d\n",
                            o->bootProbeExpectTitleFrameMax,
                            receipt.startupTitleFrame);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectTitleFrameBoundary >= 0) {
                if (receipt.startupTitleFrameMax !=
                        o->bootProbeExpectTitleFrameBoundary) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected title frame boundary %d but got %d\n",
                            o->bootProbeExpectTitleFrameBoundary,
                            receipt.startupTitleFrameMax);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectTitleReady >= 0) {
                if (receipt.startupTitleReady != o->bootProbeExpectTitleReady) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected titleReady=%d but got %d\n",
                            o->bootProbeExpectTitleReady,
                            receipt.startupTitleReady);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectDm1HoCFullGraphics) {
                DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34 expectation;
                memset(&expectation, 0, sizeof(expectation));
                if (!dm1_v1_startup_hoc_boot_probe_expectation_receipt_pc34(
                        &receipt.dm1HoCBootSummary,
                        DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_COMPLETE_SUPPORT_PC34,
                        &expectation) ||
                    !expectation.ready) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected DM1 HoC complete support but got %s\n",
                            expectation.diagnostic[0] ?
                                expectation.diagnostic :
                                "missing DM1 expectation diagnostic");
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectDm1HoCReleaseAppCapture) {
                DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34 expectation;
                memset(&expectation, 0, sizeof(expectation));
                if (!dm1_v1_startup_hoc_boot_probe_expectation_receipt_pc34(
                        &receipt.dm1HoCBootSummary,
                        DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_RELEASE_APP_CAPTURE_PC34,
                        &expectation) ||
                    !expectation.ready) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected DM1 HoC release-app host capture but got %s\n",
                            expectation.diagnostic[0] ?
                                expectation.diagnostic :
                                "missing DM1 expectation diagnostic");
                    runRc = 4;
                }
            }
            goto boot_probe_terminal_exit;
        }
        if (exitAfterLaunch) {
            goto cleanup;
        }
    } else {
        m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
    }
    if (gameView.active) {
        /* ReDMCSB ENTRANCE.C F0797/F0441 hands its opened Hall frame to
         * DRAWVIEW before input.  The initial HoC frame must use the same
         * M11 presentation path as every later materialized DM1 frame so
         * V1 palette/nearest scaling and V2 targets are applied before the
         * M12 presented-capture consumer records the PC34 receipt.  Calling
         * bare Render_Present() here bypassed that contract on first launch. */
        (void)m11_present_game_frame_and_publish_startup_capture(
            &gameView, &menuState);
    } else {
        m11_present_launcher(launcherFramebuffer, modernRgba, useModern);
    }
    int gameFrameNeedsPresent = 0;
    uint32_t foodClockLastMs = (uint32_t)SDL_GetTicks();
    unsigned int observedLaunchSerial = 0;
    Uint64 perfWindowStart = SDL_GetPerformanceCounter();
    Uint64 perfLastLoopEnd = perfWindowStart;
    Uint64 perfLoopCount = 0;
    Uint64 perfOverBudgetLoops = 0;
    Uint64 perfGameTickCount = 0;
    Uint64 perfPresentedCount = 0;
    Uint64 perfDrawCount = 0;
    double perfRenderTotalMs = 0.0;
    double perfRenderMaxMs = 0.0;
    double perfLoopMaxMs = 0.0;
    double perfCpuWindowStart = 0.0;
    int perfCpuAvailable = o->debug &&
        m11_process_cpu_seconds(&perfCpuWindowStart);
    int autotestMotionEventInjected = 0;

    while (!quitRequested && !menuState.shouldExit &&
           (!M11_RunDurationHasElapsed(o->durationMs, (Uint32)start,
                                      (Uint32)now) ||
            ((o->verbose || o->debug) && menuState.deferredScanActive))) {
#if SDL_VERSION_ATLEAST(3, 2, 0)
        const Uint64 loopStartNs = SDL_GetTicksNS();
#else
        const Uint32 loopStartMs = (Uint32)SDL_GetTicks();
#endif
        gameView.debugMovementTraceEnabled = o->debug;
        double perfLoopDrawMs = 0.0;
        Uint64 perfLoopDrawCount = 0;
        if (o->debug) ++perfLoopCount;
        M12_MenuInput input = M12_MENU_INPUT_NONE;
        M11_GameInputResult pointerResult = M11_GAME_INPUT_IGNORED;
        uint32_t tickBeforeEvents = gameView.world.gameTick;
        uint32_t tickBeforeInput = gameView.world.gameTick;
        int theronTickBeforeInput = gameView.theronState.tick_count;
        const uint32_t perfTickBefore = o->debug
            ? m11_debug_game_tick_count(&gameView) : 0U;
        const int perfActiveBefore = gameView.active;
        const int perfSourceBefore = (int)gameView.sourceKind;
        const unsigned int perfLaunchSerialBefore = g_m11_sync_launch_serial;

        /* Settings may change while the launcher is visible. Apply the
         * persisted input-mode choice before either event or held-state
         * controller input is translated into shared CSB command tokens. */
        gamepadMap.enabled = M11_GamepadEnabledForInputMode(
            menuState.settings.inputModeIndex, gamepadConfiguredEnabled);
        m11_sync_touch_gesture_gate(&gameView, &menuState);

        {
            int speedMul = M11_QolRuntime_GetSpeedMultiplier();
            /* Normal gameplay uses the original 200 ms source tick divided
             * by the configured speed. CSB startup instead advances one
             * original VBlank per source tick. */
#if SDL_VERSION_ATLEAST(3, 0, 0)
            gameTickInterval = (Uint64)M11_GameView_IdleTickIntervalMs(
                &gameView, speedMul);
#else
            gameTickInterval = (Uint32)M11_GameView_IdleTickIntervalMs(
                &gameView, speedMul);
#endif
            if (gameTickInterval < 1) gameTickInterval = 1;
        }
        /* SDL updates window focus while pumping. Observe it before advancing
         * any source clocks, including when scripted input bypasses polling. */
        {
            int wasPaused = M11_GameView_IsPaused(&gameView);
            SDL_Window* window = M11_Render_GetWindow();
            SDL_PumpEvents();
            if (window)
                m11_sync_focus_pause(&gameView, &menuState,
                    (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0);
            if (wasPaused || M11_GameView_IsPaused(&gameView) ||
                observedLaunchSerial != g_m11_sync_launch_serial) {
                observedLaunchSerial = g_m11_sync_launch_serial;
                lastLoopTick = SDL_GetTicks();
                foodClockLastMs = (uint32_t)lastLoopTick;
                idleAccumulatorMs = 0;
                DM1_V1_PendingMotionQueue_ClearPc34Compat(&pendingDm1V1MotionQueue);
            }
        }
        now = SDL_GetTicks();
        if (gameView.active) {
            uint32_t loopDeltaMs = (uint32_t)(now - lastLoopTick);
            int timerReminderBefore = gameView.sessionTimerReminderOverlayActive;
            int timerPauseBefore = gameView.sessionTimerForcedPauseDialogActive;
            if (M11_GameView_IsPaused(&gameView)) loopDeltaMs = 0;
            int foodCommandWasPending = gameView.v1FoodCommandPending;
            uint32_t foodElapsedMs = (uint32_t)now - foodClockLastMs;
            foodClockLastMs = (uint32_t)now;
            if (M11_GameView_AdvanceFoodClockMs(&gameView, foodElapsedMs) ==
                M11_GAME_INPUT_REDRAW) {
                m11_draw_game_frame_for_loop(&gameView, o->debug,
                    &perfLoopDrawMs, &perfLoopDrawCount);
                gameFrameNeedsPresent = 1;
            }
            /* Source F0349 cannot run ordinary game-loop work during its
             * synchronous waits. Keep pumping host events, but discard
             * simulation debt including the command's completion slice. */
            if (M11_GameView_IsPaused(&gameView) || foodCommandWasPending || gameView.v1FoodCommandPending)
                idleAccumulatorMs = 0;
            else
                idleAccumulatorMs += loopDeltaMs;
            /* DM1 V1: feed elapsed time to VBlank simulation */
            DM1_V1_VBlankTiming_Update(&gameView.vblankTiming, loopDeltaMs);
            /* Preserve subsecond frame time; ordinary frames are much shorter
             * than one second. The view resets the remainder per session. */
            (void)M11_GameView_TickSessionTimerMs(&gameView, loopDeltaMs);
            if (timerReminderBefore != gameView.sessionTimerReminderOverlayActive ||
                timerPauseBefore != gameView.sessionTimerForcedPauseDialogActive) {
                if (gameView.sessionTimerForcedPauseDialogActive)
                    idleAccumulatorMs = 0;
                m11_draw_game_frame_for_loop(&gameView, o->debug,
                    &perfLoopDrawMs, &perfLoopDrawCount);
                gameFrameNeedsPresent = 1;
            }
            firestaff_ra_overlay_tick(&gameView.retroAchievementsOverlay,
                                      (int)loopDeltaMs);
            if (gameView.retroAchievementsOverlay.active_valid ||
                gameView.retroAchievementsOverlay.queue_count > 0) {
                gameFrameNeedsPresent = 1;
            }
        } else {
            idleAccumulatorMs = 0;
        }
        lastLoopTick = now;

        if (scriptWaitForScan && !menuState.deferredScanActive) {
            scriptWaitForScan = 0;
        }
        if (scriptWaitForRuntime) {
            M11_BootProbeReceipt runtimeReceipt;
            memset(&runtimeReceipt, 0, sizeof(runtimeReceipt));
            if (M11_GameView_GetStartupTraceReceipt(&gameView, &runtimeReceipt) &&
                m11_boot_probe_runtime_receipt_ready(&runtimeReceipt)) {
                scriptWaitForRuntime = 0;
            }
        }
        if (scriptWaitForRuntime) {
            input = M12_MENU_INPUT_NONE;
        } else if (scriptWaitForScan) {
            input = M12_MENU_INPUT_NONE;
        } else if (scriptWaitFramesRemaining > 0) {
            --scriptWaitFramesRemaining;
        } else if (scriptCursor && *scriptCursor != '\0') {
            int waitFrames = 0;
            int waitForRuntime = 0;
            int waitForScan = 0;
            input = m11_next_script_input(&scriptCursor,
                                          &waitFrames,
                                          &waitForRuntime,
                                          &waitForScan);
            scriptWaitFramesRemaining = waitFrames;
            scriptWaitForRuntime = waitForRuntime;
            scriptWaitForScan = waitForScan;
        }
        int menuPointerChanged = 0;
        if (!autotestMotionEventInjected && input == M12_MENU_INPUT_NONE &&
            getenv("FIRESTAFF_AUTOTEST") && gameView.active &&
            g_m11_debug_motion.observedSerial >= 5) {
            const char* eventToken =
                getenv("FIRESTAFF_AUTOTEST_MOTION_EVENT");
            if (eventToken && eventToken[0]) {
                (void)m11_push_script_event_token(eventToken,
                                                  strlen(eventToken));
                autotestMotionEventInjected = 1;
                if (o->debug) {
                    fprintf(stderr,
                            "firestaff: autotest input-event-injected game=%s token=%s\n",
                            gameView.sourceId[0] ? gameView.sourceId : "unknown",
                            eventToken);
                }
            }
        }
        if (input == M12_MENU_INPUT_NONE) {
            input = m11_poll_menu_input(&gameView,
                                        &menuState,
                                        &gamepadMap,
                                        &gamepadStatus,
                                        useModern,
                                        &pointerResult,
                                        &quitRequested,
                                        &menuPointerChanged);
        }
        if (input != M12_MENU_INPUT_NONE && o->debug && gameView.active &&
            strcmp(gameView.sourceId, "dm1") == 0 &&
            M11_GameView_InputConsumesDm1V1SourceTick(&gameView, input) &&
            g_m11_debug_polled_event_counter &&
            g_m11_debug_polled_at_counter >=
                g_m11_debug_polled_event_counter) {
            g_m11_debug_motion.pendingInputEventCounter =
                g_m11_debug_polled_event_counter;
            g_m11_debug_motion.pendingInputPollCounter =
                g_m11_debug_polled_at_counter;
        }
        m11_sync_startup_text_input(&menuState,
                                    useModern && !gameView.active,
                                    &startupTextInputActive);
        if (quitRequested) {
            break;
        }
        if (menuPointerChanged && !gameView.active) {
            if (menuState.shouldExit) {
                break;
            }
            if (m11_open_csb_fmtowns_utility_from_menu(
                    &gameView, &menuState, &idleAccumulatorMs, o->dataDir) ||
                m11_open_csb_hint_oracle_from_menu(&gameView, &menuState) ||
                m11_open_requested_launch(&gameView,
                                          &menuState,
                                          &idleAccumulatorMs,
                                          o->dataDir,
                                          0)) {
                launchedEver = 1;
                if (o->debug) {
                    m11_debug_log_launch_selection(
                        &menuState, &gameView, "menu");
                    m11_debug_trace_startup_frame(
                        &debugStartupTrace, &gameView, o->gameId);
                    m11_debug_game_events(&gameView);
                    /* Startup and Entrance can run synchronously inside the
                     * menu loop. Exclude that time from active-loop metrics. */
                    m11_reset_debug_perf_window(
                        &perfWindowStart, &perfLastLoopEnd,
                        &perfLoopCount, &perfOverBudgetLoops,
                        &perfGameTickCount, &perfPresentedCount, &perfDrawCount,
                        &perfRenderTotalMs, &perfRenderMaxMs,
                        &perfLoopMaxMs, &perfCpuWindowStart,
                        &perfCpuAvailable);
                }
                if (gameView.active) {
                    gameFrameNeedsPresent = 1;
                }
                if (exitAfterLaunch) {
                    break;
                }
                continue;
            }
            M11_ApplyStartupMenuRuntime(&menuState);
            m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
        }
        if (pointerResult != M11_GAME_INPUT_IGNORED) {
            if (pointerResult == M11_GAME_INPUT_RETURN_TO_MENU) {
                /* `--game` is a single-game command-line session. When that
                 * game asks to return to the launcher, finish the process
                 * instead of opening the multi-game start menu. */
                if (o->directLaunch) {
                    if (o->debug) {
                        fprintf(stderr,
                                "firestaff: direct game ended; exiting instead of opening the start menu\n");
                    }
                    break;
                }
                (void)M12_StartupMenu_SetDM1HoCPresentedCaptureReceipt(&menuState, NULL);
                M11_GameView_Shutdown(&gameView);
                M11_GameView_Init(&gameView);
                DM1_V1_PendingMotionQueue_ClearPc34Compat(
                    &pendingDm1V1MotionQueue);
                idleAccumulatorMs = 0;
                m11_rescan_launcher_asset_status(&menuState,
                                                 launcherFramebuffer,
                                                 modernRgba,
                                                 useModern,
                                                 o->debug,
                                                 &quitRequested);
                if (quitRequested) break;
                M11_ApplyStartupMenuRuntime(&menuState);
                m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
            } else if (pointerResult == M11_GAME_INPUT_RESTART_GAME) {
                DM1_V1_PendingMotionQueue_ClearPc34Compat(
                    &pendingDm1V1MotionQueue);
                if (m11_restart_current_launch(&gameView,
                                                &menuState,
                                                &idleAccumulatorMs,
                                                o->dataDir) &&
                    gameView.active) {
                    launchedEver = 1;
                    if (o->debug) {
                        gameView.debugMovementTraceEnabled = 1;
                        m11_debug_game_events(&gameView);
                    }
                    gameFrameNeedsPresent = 1;
                    if (exitAfterLaunch) {
                        break;
                    }
                } else {
                    M11_ApplyStartupMenuRuntime(&menuState);
                    m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                }
            } else if (pointerResult == M11_GAME_INPUT_REDRAW) {
                if (gameView.graphicsPopupActive) {
                    m11_sync_runtime_graphics_popup_to_menu(&gameView, &menuState);
                }
                m11_draw_game_frame_for_loop(&gameView, o->debug,
                    &perfLoopDrawMs, &perfLoopDrawCount);
                gameFrameNeedsPresent = 1;
                if (gameView.world.gameTick != tickBeforeEvents) {
                    idleAccumulatorMs = 0;
                }
            }
        }
        if (input == M12_MENU_INPUT_NONE &&
            pointerResult == M11_GAME_INPUT_IGNORED &&
            m11_game_view_supports_held_motion_input(&gameView) &&
            /* THQUEST.ASM T520/T600/T700 owns the dungeon input cadence;
             * it does not publish DM1's VBlank stopWaitingForInput flag. */
            (m11_game_view_is_theron(&gameView)
                ? idleAccumulatorMs >= (uint32_t)gameTickInterval
                : M11_GameView_Dm1V1SourceTickReadyForInput(&gameView))) {
            {
                int pendingInput = M12_MENU_INPUT_NONE;
                if (DM1_V1_PendingMotionQueue_PopPc34Compat(
                        &pendingDm1V1MotionQueue, &pendingInput)) {
                    input = (M12_MenuInput)pendingInput;
                }
            }
            if (input == M12_MENU_INPUT_NONE) {
                input = m11_held_motion_input_from_keyboard(&gameView);
            }
            if (input == M12_MENU_INPUT_NONE) {
                input = m11_held_motion_input_from_gamepad(&gameView,
                                                           &gamepadStatus,
                                                           &gamepadMap);
            }
        }
        if (input != M12_MENU_INPUT_NONE) {
            tickBeforeInput = gameView.world.gameTick;
            if (gameView.active) {
                M11_GameInputResult result = M11_GAME_INPUT_IGNORED;
                if (!gameView.graphicsPopupActive &&
                    input != M12_MENU_INPUT_GRAPHICS_POPUP &&
                    M11_GameView_InputConsumesDm1V1SourceTick(&gameView, input) &&
                    !m11_dm1_v1_input_is_immediate_turn(input) &&
                    !M11_GameView_Dm1V1SourceTickReadyForInput(&gameView) &&
                    (DM1_V1_PendingMotionQueue_CountPc34Compat(&pendingDm1V1MotionQueue) > 0U ||
                     !M11_GameView_Dm1V1FreshMotionClosesInputWait(&gameView, input))) {
                    /* GAMELOOP.C:215 polls F0380 while G0321 is false;
                     * a fresh accepted step closes the wait immediately.
                     * Preserve pending FIFO order and defer blocked/cooldown
                     * commands without aging a tick for each host key. */
                    (void)DM1_V1_PendingMotionQueue_PushPc34Compat(
                        &pendingDm1V1MotionQueue, (int)input);
                    input = M12_MENU_INPUT_NONE;
                } else {
                    result = M11_GameView_HandleInput(&gameView, input);
                    m11_debug_game_action(&gameView, input, result);
                }
                if (o->debug && input == M12_MENU_INPUT_BACK &&
                    gameView.sourceId[0] &&
                    strcmp(gameView.sourceId, "dm2") == 0) {
                    fprintf(stderr,
                            "firestaff: input game=dm2 action=back result=%d startup-menu=%d frozen=%d\n",
                            (int)result,
                            gameView.dm2State.startup_menu_active,
                            gameView.dm2GameFrozen);
                }
                if (result == M11_GAME_INPUT_RETURN_TO_MENU) {
                    /* Match the pointer-driven return path above: a direct
                     * `--game` launch exits after the game, while a game
                     * started from the launcher returns to that launcher. */
                    if (o->directLaunch) {
                        if (o->debug) {
                            fprintf(stderr,
                                    "firestaff: direct game ended; exiting instead of opening the start menu\n");
                        }
                        break;
                    }
                    (void)M12_StartupMenu_SetDM1HoCPresentedCaptureReceipt(&menuState, NULL);
                    M11_GameView_Shutdown(&gameView);
                    M11_GameView_Init(&gameView);
                    DM1_V1_PendingMotionQueue_ClearPc34Compat(
                        &pendingDm1V1MotionQueue);
                    idleAccumulatorMs = 0;
                    m11_rescan_launcher_asset_status(&menuState,
                                                     launcherFramebuffer,
                                                     modernRgba,
                                                     useModern,
                                                     o->debug,
                                                     &quitRequested);
                    if (quitRequested) break;
                    M11_ApplyStartupMenuRuntime(&menuState);
                    m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                } else if (result == M11_GAME_INPUT_RESTART_GAME) {
                    DM1_V1_PendingMotionQueue_ClearPc34Compat(
                        &pendingDm1V1MotionQueue);
                    if (m11_restart_current_launch(&gameView,
                                                    &menuState,
                                                    &idleAccumulatorMs,
                                                    o->dataDir) &&
                        gameView.active) {
                        launchedEver = 1;
                        if (o->debug) {
                            gameView.debugMovementTraceEnabled = 1;
                            m11_debug_game_events(&gameView);
                        }
                        gameFrameNeedsPresent = 1;
                        if (exitAfterLaunch) {
                            break;
                        }
                    } else {
                        M11_ApplyStartupMenuRuntime(&menuState);
                        m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                    }
                } else if (result == M11_GAME_INPUT_REDRAW) {
                    if (gameView.graphicsPopupActive ||
                        input == M12_MENU_INPUT_GRAPHICS_POPUP) {
                        m11_sync_runtime_graphics_popup_to_menu(&gameView, &menuState);
                    }
                    int redrawWasAfterViewportDirty =
                        gameView.lastDm1V1MovementPipelineResult.viewportDirty;
                    m11_draw_game_frame_for_loop(&gameView, o->debug,
                        &perfLoopDrawMs, &perfLoopDrawCount);
                    gameFrameNeedsPresent = 1;
                    inputRedrawDrawCount++;
                    if (redrawWasAfterViewportDirty) {
                        inputRedrawAfterViewportDirtyCount++;
                    }
                    lastInputRedrawAfterViewportDirty = redrawWasAfterViewportDirty;
                    if (gameView.world.gameTick != tickBeforeInput ||
                        (m11_game_view_is_theron(&gameView) &&
                         gameView.theronState.tick_count !=
                             theronTickBeforeInput)) {
                        idleAccumulatorMs = 0;
                    }
                }
            } else {
                int launchHandled = 0;
                /* Do not short-circuit Enter/Right on the top-level launcher into
                 * M11_GameView_OpenSelectedMenuEntry(). ReDMCSB waits at the entrance
                 * command surface (ENTRANCE.C:739-747 installs entrance input;
                 * COMMAND.C:2438-2451 handles Enter/Resume) and only the explicit
                 * launch row in M12 should request runtime handoff. */
                if (!launchHandled) {
                    if (input == M12_MENU_INPUT_CYCLE_CHAMPION ||
                        input == M12_MENU_INPUT_STRAFE_LEFT ||
                        input == M12_MENU_INPUT_STRAFE_RIGHT ||
                        input == M12_MENU_INPUT_PICKUP_ITEM ||
                        input == M12_MENU_INPUT_DROP_ITEM) {
                        input = M12_MENU_INPUT_NONE;
                    }
                    M12_StartupMenu_HandleInput(&menuState, input);
                    if (M12_StartupMenu_TakeRescanAllGamesRequest(
                            &menuState)) {
                        m11_rescan_launcher_asset_status(
                            &menuState, launcherFramebuffer, modernRgba,
                            useModern, o->debug, &quitRequested);
                        if (quitRequested) break;
                    }
                    m11_sync_startup_text_input(&menuState,
                                                useModern && !gameView.active,
                                                &startupTextInputActive);
                    if (menuState.shouldExit) {
                        break;
                    }
                    if (m11_open_csb_fmtowns_utility_from_menu(
                            &gameView, &menuState, &idleAccumulatorMs,
                            o->dataDir) ||
                        m11_open_csb_hint_oracle_from_menu(&gameView, &menuState) ||
                        m11_open_requested_launch(&gameView,
                                                  &menuState,
                                                  &idleAccumulatorMs,
                                                  o->dataDir,
                                                  0)) {
                        launchedEver = 1;
                        if (o->debug) {
                            m11_debug_log_launch_selection(
                                &menuState, &gameView, "menu");
                            m11_debug_trace_startup_frame(
                                &debugStartupTrace, &gameView, o->gameId);
                            m11_debug_game_events(&gameView);
                            /* Startup and Entrance can run synchronously
                             * inside the menu loop. Exclude that time from
                             * active-loop metrics. */
                            m11_reset_debug_perf_window(
                                &perfWindowStart, &perfLastLoopEnd,
                                &perfLoopCount, &perfOverBudgetLoops,
                                &perfGameTickCount, &perfPresentedCount, &perfDrawCount,
                                &perfRenderTotalMs, &perfRenderMaxMs,
                                &perfLoopMaxMs, &perfCpuWindowStart,
                                &perfCpuAvailable);
                        }
                        if (gameView.active) {
                            gameFrameNeedsPresent = 1;
                        }
                        if (exitAfterLaunch) {
                            break;
                        }
                        continue;
                    }
                    M11_ApplyStartupMenuRuntime(&menuState);
                    m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                }
            }
        }
        {
            int dm2TimedMediaRedraw = 0;
            int csbStartupSourceRedraw = 0;
            while (gameView.active && idleAccumulatorMs >= (uint32_t)gameTickInterval) {
                M11_GameInputResult tickResult =
                    M11_GameView_AdvanceIdleTick(&gameView);
                m11_debug_dm1_combat_tick(&gameView);
                m11_debug_game_events(&gameView);
                if (tickResult == M11_GAME_INPUT_REDRAW) {
                    if (gameView.sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
                        gameView.dm2State.startup_menu_active &&
                        (gameView.dm2FmtownsTitleBound ||
                         gameView.dm2FmtownsTitleFinished)) {
                        dm2TimedMediaRedraw = 1;
                    } else if (gameView.sourceKind ==
                                   M11_GAME_SOURCE_CSB_BOOT &&
                               (gameView.csbState.startup_title_active ||
                                gameView.csbState.startup_entrance_active)) {
                        /* Retain source VBlank debt and compose only the
                         * newest state after catch-up. Intermediate startup
                         * frames are intentionally dropped when the host
                         * display cannot present at the source rate. */
                        csbStartupSourceRedraw = 1;
                        gameFrameNeedsPresent = 1;
                    } else {
                        m11_draw_game_frame_for_loop(&gameView, o->debug,
                            &perfLoopDrawMs, &perfLoopDrawCount);
                        gameFrameNeedsPresent = 1;
                    }
                }
                idleAccumulatorMs -= (uint32_t)gameTickInterval;
                /* Source-ticked DM2 media and CSB startup retain elapsed
                 * scheduler debt. They consume overdue source steps in order
                 * and present the latest composed frame after catch-up. */
                if (M11_GameView_DropsIdleCatchupForStartup(&gameView)) {
                    idleAccumulatorMs = 0;
                    break;
                }
            }
            if (dm2TimedMediaRedraw) {
                m11_draw_game_frame_for_loop(&gameView, o->debug,
                    &perfLoopDrawMs, &perfLoopDrawCount);
                gameFrameNeedsPresent = 1;
            }
            if (csbStartupSourceRedraw) {
                m11_draw_game_frame_for_loop(&gameView, o->debug,
                    &perfLoopDrawMs, &perfLoopDrawCount);
                gameFrameNeedsPresent = 1;
            }
        }
        if (gameView.active) {
            /* Static DM1 V1 frames do not need a 60 Hz full redraw/present.
             * Resurrecting a champion enables the full HUD/champion render
             * path, so keep CPU bounded by presenting only after input,
             * resize, launch, restart, or a source tick dirties the frame. */
            if (o->debugFullRedraw) {
                /* Count real source composition, not another presentation
                 * of the retained framebuffer. Existing dirty draws already
                 * satisfy this host frame's full-redraw request. */
                if (perfLoopDrawCount == 0U)
                    m11_draw_game_frame_for_loop(&gameView, o->debug,
                        &perfLoopDrawMs, &perfLoopDrawCount);
                gameFrameNeedsPresent = 1;
            }
            if (gameFrameNeedsPresent) {
                const Uint64 perfRenderStart = o->debug
                    ? SDL_GetPerformanceCounter() : 0;
                DM1_V1_AutoMap_RecordVisitPc34Compat(&gameView);
                DM1_V1_Minimap_RenderPc34Compat(&gameView,
                                   M11_Render_GetFramebuffer(),
                                   M11_FB_WIDTH, M11_FB_HEIGHT);
                DM1_CombatLog_Render(&gameView,
                                     M11_Render_GetFramebuffer(),
                                     M11_FB_WIDTH, M11_FB_HEIGHT);
                M11_GameView_DrawGraphicsPopup(&gameView,
                                                M11_Render_GetFramebuffer(),
                                                M11_FB_WIDTH, M11_FB_HEIGHT);
                M11_GameView_DrawLeaderHandCursor(
                    &gameView, M11_Render_GetFramebuffer(),
                    M11_FB_WIDTH, M11_FB_HEIGHT);
                M11_GameView_RecordPresentedFrame(&gameView, SDL_GetTicks());
                M11_GameView_DrawFpsOverlay(&gameView,
                                             M11_Render_GetFramebuffer(),
                                             M11_FB_WIDTH, M11_FB_HEIGHT);
                if (o->debug) {
                    m11_debug_trace_startup_frame(&debugStartupTrace,
                                                  &gameView, o->gameId);
                }
                {
                    const uint64_t foodFrameSerial = gameView.v1FoodPresentationSerial;
                    const int presented = m11_present_game_frame_and_publish_startup_capture(
                        &gameView, &menuState);
                    if (o->debug && presented) {
                        ++perfPresentedCount;
                        m11_debug_motion_presented(&gameView, SDL_GetPerformanceCounter());
                    }
                    if (gameView.v1FoodAwaitingPresentation) {
                        const uint32_t presentedAtMs = (uint32_t)SDL_GetTicks();
                        /* Edges during render/present belong to the wait for
                         * visibility, not to the subsequent Delay(8). Account
                         * for them before acknowledging successful present. */
                        (void)M11_GameView_AdvanceFoodClockMs(
                            &gameView, presentedAtMs - foodClockLastMs);
                        foodClockLastMs = presentedAtMs;
                        if (presented)
                            (void)M11_GameView_AcknowledgeFoodPresentation(
                                &gameView, foodFrameSerial);
                    }
                    gameFrameNeedsPresent = gameView.v1FoodAwaitingPresentation;
                }
                if (o->debug) {
                    const double renderMs = m11_counter_milliseconds(
                        SDL_GetPerformanceCounter() - perfRenderStart) + perfLoopDrawMs;
                    perfRenderTotalMs += renderMs;
                    if (renderMs > perfRenderMaxMs)
                        perfRenderMaxMs = renderMs;
                }
            }
        } else {
            /* Redraw the launcher every tick so animations (pulse,
             * hover) remain alive even without input. */
            foodClockLastMs = (uint32_t)SDL_GetTicks();
            menuState.frameTick += 1U;
            if (M12_StartupMenu_Update(&menuState)) {
                M11_ApplyStartupMenuRuntime(&menuState);
            }
            m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
            m11_present_launcher(launcherFramebuffer, modernRgba, useModern);
        }
        /* `Present` may already wait for the next VBlank when VSync is on.
         * Sleeping a full interval after that wait doubles the frame period
         * (for example, 30 Hz on a 60 Hz display). Pace the whole loop instead:
         * wait only for the portion of the target interval spent outside
         * rendering and event handling. */
        {
#if SDL_VERSION_ATLEAST(3, 2, 0)
            /* SDL_Delay may oversleep by several milliseconds on macOS.
             * Keep the existing host interval, but measure its remainder
             * without millisecond truncation and request a precise wait.
             * Original game clocks and startup VBlank waits are separate. */
            const Uint64 intervalNs = interval * SDL_NS_PER_MS;
            const Uint64 elapsedNs = SDL_GetTicksNS() - loopStartNs;
            if (elapsedNs < intervalNs)
                SDL_DelayPrecise(intervalNs - elapsedNs);
#else
            const Uint32 elapsedMs =
                (Uint32)SDL_GetTicks() - loopStartMs;
            if ((Uint64)elapsedMs < interval)
                SDL_Delay((Uint32)(interval - (Uint64)elapsedMs));
#endif
        }
        now = SDL_GetTicks();
        if (o->debug &&
            (gameView.active != perfActiveBefore ||
             (int)gameView.sourceKind != perfSourceBefore ||
             g_m11_sync_launch_serial != perfLaunchSerialBefore)) {
            /* A return/restart may reset the source tick counter and spend
             * synchronous time scanning media or presenting startup. Start a
             * fresh measurement window instead of mixing sessions or counting
             * that reset as an unsigned tick-counter wrap. */
            m11_reset_debug_perf_window(
                &perfWindowStart, &perfLastLoopEnd,
                &perfLoopCount, &perfOverBudgetLoops,
                &perfGameTickCount, &perfPresentedCount, &perfDrawCount,
                &perfRenderTotalMs, &perfRenderMaxMs,
                &perfLoopMaxMs, &perfCpuWindowStart,
                &perfCpuAvailable);
        } else if (o->debug) {
            if (gameView.active) {
                /* Unsigned subtraction retains genuine same-session wrap. */
                const uint32_t tickDelta = m11_debug_game_tick_count(&gameView) -
                                           perfTickBefore;
                perfGameTickCount += tickDelta;
                perfDrawCount += perfLoopDrawCount;
            }
            const Uint64 perfNow = SDL_GetPerformanceCounter();
            const double loopMs = m11_counter_milliseconds(
                perfNow - perfLastLoopEnd);
            perfLastLoopEnd = perfNow;
            if (loopMs > perfLoopMaxMs) perfLoopMaxMs = loopMs;
            if (loopMs > (double)interval + 5.0) ++perfOverBudgetLoops;
            const double elapsedMs = m11_counter_milliseconds(
                perfNow - perfWindowStart);
            if (elapsedMs >= 5000.0) {
                double cpuNow = 0.0;
                const int cpuNowAvailable = m11_process_cpu_seconds(&cpuNow);
                uint64_t residentBytes = 0;
                uint64_t virtualBytes = 0;
                const int memoryAvailable = m11_process_memory_bytes(
                    &residentBytes, &virtualBytes);
                double cpuPercent = -1.0;
                if (perfCpuAvailable && cpuNowAvailable &&
                    elapsedMs > 0.0 && cpuNow >= perfCpuWindowStart) {
                    cpuPercent = (cpuNow - perfCpuWindowStart) *
                                 100000.0 / elapsedMs;
                }
                fprintf(stderr,
                        "firestaff: perf game=%s active=%d debug-full-redraw=%d window-ms=%.0f "
                        "process-cpu-of-one-core-percent=",
                        o->gameId ? o->gameId : "menu", gameView.active,
                        o->debugFullRedraw, elapsedMs);
                if (cpuPercent >= 0.0) fprintf(stderr, "%.1f", cpuPercent);
                else fprintf(stderr, "unavailable");
                fprintf(stderr,
                        " target-loop-fps=%.2f loops-per-sec=%.1f "
                        "process-rss-bytes=",
                        interval ? 1000.0 / (double)interval : 0.0,
                        (double)perfLoopCount * 1000.0 / elapsedMs);
                if (memoryAvailable) fprintf(stderr, "%llu",
                    (unsigned long long)residentBytes);
                else fprintf(stderr, "unavailable");
                fprintf(stderr, " process-virtual-bytes=");
                if (memoryAvailable) fprintf(stderr, "%llu",
                    (unsigned long long)virtualBytes);
                else fprintf(stderr, "unavailable");
                fprintf(stderr,
                        " loop-ms-max=%.2f "
                        "loop-over-budget-count=%llu game-ticks-per-sec=%.2f "
                        "game-frames-rendered-per-sec=%.2f game-frames-presented-per-sec=%.2f "
                        "source-draw-count=%llu presented-frame-count=%llu "
                        "frame-compose-present-call-ms-avg=%.2f "
                        "frame-compose-present-call-ms-max=%.2f "
                        "gpu-utilization=unavailable\n",
                        perfLoopMaxMs,
                        (unsigned long long)perfOverBudgetLoops,
                        (double)perfGameTickCount * 1000.0 / elapsedMs,
                        (double)perfDrawCount * 1000.0 / elapsedMs,
                        (double)perfPresentedCount * 1000.0 / elapsedMs,
                        (unsigned long long)perfDrawCount,
                        (unsigned long long)perfPresentedCount,
                        perfPresentedCount
                            ? perfRenderTotalMs / (double)perfPresentedCount
                            : 0.0,
                        perfRenderMaxMs);
                perfWindowStart = perfNow;
                perfLoopCount = 0;
                perfOverBudgetLoops = 0;
                perfGameTickCount = 0;
                perfPresentedCount = 0;
                perfDrawCount = 0;
                perfRenderTotalMs = 0.0;
                perfRenderMaxMs = 0.0;
                perfLoopMaxMs = 0.0;
                perfCpuWindowStart = cpuNow;
                perfCpuAvailable = cpuNowAvailable;
            }
        }
    }

    if (o->verbose) {
        static const char* const gameIds[] = {
            "dm1", "csb", "dm2", "nexus", "theron"
        };
        size_t gameIndex;
        for (gameIndex = 0U;
             gameIndex < sizeof(gameIds) / sizeof(gameIds[0]);
             ++gameIndex) {
            if (o->menuRequested || !o->gameId ||
                strcmp(o->gameId, gameIds[gameIndex]) == 0) {
                m11_verbose_original_media(&menuState.assetStatus,
                                           gameIds[gameIndex]);
            }
        }
    }
    if (failIfNoLaunch && !launchedEver) {
        fprintf(stderr, "firestaff: launch smoke failed: no launch reached before exit\n");
        runRc = 3;
    }
cleanup:
    m11_sync_startup_text_input(&menuState, 0, &startupTextInputActive);
    M12_StartupMenu_Destroy(&menuState);
    m11_write_autotest_screenshot(getenv("FIRESTAFF_AUTOTEST_SCREENSHOT_DIR"));
    m11_write_autotest_presented_screenshot(getenv("FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR"));
    m11_write_autotest_runtime_probe(getenv("FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON"),
                                     launchedEver,
                                     &gameView,
                                     scriptCursor,
                                     scriptWaitFramesRemaining,
                                     inputRedrawDrawCount,
                                     inputRedrawAfterViewportDirtyCount,
                                     lastInputRedrawAfterViewportDirty);
    m11_sync_and_save_window_size(&menuState);
    M12_GamepadStatus_Close(&gamepadStatus);
    M11_GameView_Shutdown(&gameView);
    free(launcherFramebuffer);
    if (modernRgba) {
        free(modernRgba);
    }
    M11_Render_Shutdown();
    g_m11_phase_duration_ms = -1;
    g_m11_phase_duration_expired = 0;
    return runRc;

boot_probe_terminal_exit:
    /* Process-terminal probe path: the printed boot receipt is the contract.
     * It still owns game-side resources: in particular, an authenticated FM
     * Towns launch can have a live CD-audio stream.  Leaving that stream
     * alive while only the video renderer is destroyed makes a successful
     * --duration 0 probe print its receipt and then hang at process exit.
     * Shut the game down before M11_Render so the probe is genuinely
     * terminal, while preserving the already-recorded receipt/capture. */
    /* Keep optional visual evidence available for the exact terminal runtime
     * page. This is deliberately opt-in and runs before the renderer is
     * released, so real-data V1/V2.x capture tests do not need a synthetic
     * second presentation loop merely to obtain a screenshot. */
    m11_write_autotest_screenshot(getenv("FIRESTAFF_AUTOTEST_SCREENSHOT_DIR"));
    m11_write_autotest_presented_screenshot(
        getenv("FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR"));
    m11_write_autotest_runtime_probe(
        getenv("FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON"),
        launchedEver,
        &gameView,
        scriptCursor,
        scriptWaitFramesRemaining,
        inputRedrawDrawCount,
        inputRedrawAfterViewportDirtyCount,
        lastInputRedrawAfterViewportDirty);
    M11_GameView_Shutdown(&gameView);
    M11_Render_Shutdown();
    free(launcherFramebuffer);
    if (modernRgba) {
        free(modernRgba);
    }
    g_m11_phase_duration_ms = -1;
    g_m11_phase_duration_expired = 0;
    return runRc;
}
