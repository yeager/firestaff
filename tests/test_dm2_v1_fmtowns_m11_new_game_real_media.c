/* Opt-in M11 regression for the authentic DM2 FM Towns startup and New Game. */

#include "m11_game_view.h"
#include "render_sdl_m11.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_startup_menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM2_FMTOWNS_ARCHIVE");
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    DM2_V1_StartupMenuPointerLayout layout;
    DM2_V1_BootRuntimeReceipt runtime;
    uint8_t framebuffer[M11_FB_WIDTH * M11_FB_HEIGHT];
    unsigned int tick;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM2_FMTOWNS_ARCHIVE is not set");
        return 77;
    }
    memset(&spec, 0, sizeof(spec));
    spec.gameId = "dm2";
    spec.sourceId = "dm2";
    spec.title = "DUNGEON MASTER II";
    spec.dataDir = archive;
    spec.launcherOptionsBound = 1;
    spec.presentationWidth = M11_FB_WIDTH;
    spec.presentationHeight = M11_FB_HEIGHT;
    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec) || !view.dm2BootProfile) {
        fputs("FAIL: DM2 FM Towns retail archive did not enter M11\n", stderr);
        return 1;
    }

    /* SKWINSPX startend.cpp::DM2_INIT:600-608 calls SHOW_MENU_SCREEN and then
     * DM2_GAME_LOAD. Advance the authenticated Timer-A stream to that SKULL
     * handoff without substituting a host title or PC startup surface. */
    M11_GameView_SetBootProbeMode(&view, 1);
    for (tick = 0; tick < 10000u && !view.dm2FmtownsTitleFinished &&
                    !view.dm2FmtownsTitleRejected; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    if (!view.dm2FmtownsTitleFinished || view.dm2FmtownsTitleRejected ||
        !view.dm2State.startup_menu_active || view.dm2State.level_loaded) {
        fprintf(stderr,
                "FAIL: Towns SWOOSH/TITLE did not hand off to SKULL menu "
                "(ticks=%u finished=%d rejected=%d menu=%d level=%d)\n",
                tick, view.dm2FmtownsTitleFinished,
                view.dm2FmtownsTitleRejected,
                view.dm2State.startup_menu_active,
                view.dm2State.level_loaded);
        M11_GameView_Shutdown(&view);
        return 1;
    }

    memset(&layout, 0, sizeof(layout));
    if (!dm2_v1_boot_startup_menu_pointer_layout(
            (DM2_V1_BootProfile *)view.dm2BootProfile, &layout) ||
        !layout.valid || layout.new_game.w <= 0 || layout.new_game.h <= 0) {
        fputs("FAIL: Towns SKULL GDAT has no New Game rectangle\n", stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    printf("DM2 FM Towns New Game rect: %d,%d %dx%d\n",
           layout.new_game.x, layout.new_game.y,
           layout.new_game.w, layout.new_game.h);
    if (
        M11_GameView_HandlePointerButton(
            &view, layout.new_game.x + layout.new_game.w / 2,
            layout.new_game.y + layout.new_game.h / 2,
            DM1_V1_MOUSE_MASK_LEFT_PC34) != M11_GAME_INPUT_REDRAW ||
        !view.dm2State.startup_menu_active || view.dm2State.level_loaded ||
        !dm2_v1_boot_prepared_new_game_world_readonly(
            (DM2_V1_BootProfile *)view.dm2BootProfile)) {
        fputs("FAIL: Towns SKULL New Game did not retain source GAME_LOAD preselection\n",
              stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }

    /* SKWINSPX startend.cpp::DM2_INIT:605-608 enters DM2_GAME_LOAD after the
     * menu. Its preselected mirror remains on the original entrance viewport;
     * clicking resolves that object through the authenticated Towns owner. */
    if (M11_GameView_HandlePointerButton(
            &view, 100, 60, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        view.dm2State.startup_menu_active || !view.dm2State.level_loaded) {
        fputs("FAIL: Towns GAME_LOAD mirror selection did not start a party\n",
              stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    memset(&runtime, 0, sizeof(runtime));
    if (!dm2_v1_boot_runtime_capture(
            (DM2_V1_BootProfile *)view.dm2BootProfile, &runtime) ||
        !runtime.runtime_ready || runtime.current_level != 0) {
        fputs("FAIL: Towns first mirror did not publish the source map-0 runtime\n",
              stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    if (!view.dm2LastRuntimeFrameAccepted ||
        !view.dm2LastRuntimeRealAssetsReady ||
        !view.dm2LastRuntimeNoCoreFallbacks ||
        view.dm2LastRuntimeFallbackDrawCount != 0) {
        fprintf(stderr,
                "FAIL: Towns map-0 runtime frame was not admitted "
                "(accepted=%d real=%d noFallbacks=%d fallbackDraws=%d)\n",
                view.dm2LastRuntimeFrameAccepted,
                view.dm2LastRuntimeRealAssetsReady,
                view.dm2LastRuntimeNoCoreFallbacks,
                view.dm2LastRuntimeFallbackDrawCount);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    M11_GameView_Shutdown(&view);
    puts("PASS: DM2 FM Towns TWANIM -> SKULL -> New Game -> mirror -> map-0 runtime");
    return 0;
}
