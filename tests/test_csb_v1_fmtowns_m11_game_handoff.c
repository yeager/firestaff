/* Real FM Towns CSB Game-program handoff.
 *
 * This opt-in test uses a materialized original F31E media root. It proves
 * that the presentation path reaches SWITCHTW, consumes its Game rectangle,
 * and opens the CHTWE-owned C004 entrance session. No generated title,
 * switch, entrance or HUD pixels are accepted.
 */
#include "m11_game_view.h"
#include "csb_v1_boot.h"
#include "csb_v1_fmtowns_switch.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", (message)); \
        ++failures; \
    } \
} while (0)

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR");
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    unsigned char framebuffer[320 * 200];
    unsigned int tick;
    int result;
    int door_frame_seen = 0;
    int live_frame_nonblack = 0;
    CSB_V1_FmtownsSwitchInputReceipt switch_input;
    CSB_V1_StartupRuntimeAssetSession_PC34 direct_session;
    CSB_V1_StartupFullRuntimeReceipt_PC34 direct_runtime;
    CSB_V1_FmtownsGameHandoffReceipt direct_handoff;
    CSB_V1_StartupSessionTerminalReceipt_PC34 terminal;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR not set");
        return 0;
    }
    memset(&spec, 0, sizeof(spec));
    spec.gameId = "csb";
    spec.sourceId = "csb";
    spec.title = "CHAOS STRIKES BACK";
    spec.dataDir = data_dir;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    M11_GameView_Init(&view);
    CHECK(M11_GameView_Start(&view, &spec),
          "verified F31E media opens its real TITLE.ANM owner");
    CHECK(view.csbFmtownsTitleBound && !view.csbStartupRuntimeAssetSession,
          "FM Towns title remains separate from the Game entrance session");

    /* TITLE.ANM has 606 Timer-A ticks. The M11 title owner advances once per
     * 16 ms wake; SWITCH.C then waits its source sixty VBlanks. */
    for (tick = 0u; tick < 700u && !view.csbFmtownsSwitchBound; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    for (tick = 0u; tick < 80u &&
                       view.csbFmtownsSwitchVblanksRemaining != 0u; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    CHECK(view.csbFmtownsSwitchBound &&
              view.csbFmtownsSwitchVblanksRemaining == 0u,
          "TITLE.ANM returns into the original ready SWITCHTW page");
    memset(&switch_input, 0, sizeof(switch_input));
    CHECK(csb_v1_fmtowns_switch_route_click(
              &view.csbFmtownsSwitchReceipt, view.csbFmtownsSwitchLanguage,
              52, 110, 1, &switch_input) &&
              switch_input.action == CSB_FMTOWNS_SWITCH_ACTION_GAME,
          "source SWITCHTW decoder classifies the Game rectangle as C03_GAME");
    memset(&direct_session, 0, sizeof(direct_session));
    memset(&direct_runtime, 0, sizeof(direct_runtime));
    CHECK(csb_v1_boot_startup_runtime_asset_session_open_pc34(
              (const CSB_V1_BootProfile *)view.csbBootProfile,
              &direct_session),
          "real F31E GRAPHICS.DAT opens its C001--C005/C017/C040 session");
    CHECK(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &direct_session, &direct_runtime),
          "real F31E session satisfies the authenticated runtime surface set");
    csb_v1_boot_startup_runtime_asset_session_release_pc34(&direct_session);
    memset(&direct_handoff, 0, sizeof(direct_handoff));
    CHECK(csb_v1_fmtowns_game_handoff_open(
              (const CSB_V1_BootProfile *)view.csbBootProfile,
              CSB_FMTOWNS_SWITCH_ENGLISH, &direct_handoff),
          "verified F31E profile resolves CHTWE beside its original media");

    /* ReDMCSB SWITCH.C F2279 registers G4171 at (47,105), 62x39. */
    result = M11_GameView_HandlePointerButton(&view, 52, 110,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    CHECK(result == M11_GAME_INPUT_REDRAW,
          "SWITCHTW Game rectangle is handled as a modal source action");
    CHECK(view.csbFmtownsGameHandoffReceipt.valid &&
              view.csbFmtownsGameHandoffReceipt.executable_verified &&
              strcmp(view.csbFmtownsGameHandoffReceipt.executable_name,
                     "CHTWE.EXP") == 0 &&
              view.csbStartupRuntimeAssetSession &&
              view.csbState.startup_entrance_active &&
              !view.csbState.startup_title_active,
          "Game click opens only the verified CHTWE entrance owner");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    CHECK(memcmp(framebuffer,
                 ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                  view.csbStartupRuntimeAssetSession)->surfaces.surfaces[
                     CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34]
                     .pixels,
                 sizeof(framebuffer)) == 0,
          "F31 Game handoff draws the authenticated C004 entrance raster");
    CHECK(M11_GameView_GetPresentationSpecialPalette(&view) ==
              VGA_PALETTE_PC34_SPECIAL_CSB_ENTRANCE,
          "F31 C004 uses the source-owned entrance palette");
    CHECK(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
              M11_GAME_INPUT_REDRAW,
          "F31 CHTWE accepts the source-owned Prison command");
    for (tick = 0u; tick < 240u && view.csbState.startup_entrance_active;
         ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        if (view.csbState.startup_entrance_opening_active &&
            view.csbState.startup_entrance_opening_step > 0 &&
            memcmp(framebuffer,
                   ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                    view.csbStartupRuntimeAssetSession)->surfaces.surfaces[
                       CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34]
                       .pixels,
                   sizeof(framebuffer)) != 0) {
            door_frame_seen = 1;
        }
    }
    CHECK(door_frame_seen,
          "F31 Prison transition draws a source-owned C002/C003 door frame");
    CHECK(!view.csbState.startup_entrance_active && view.csbState.level_loaded,
          "F31 Prison door handoff reaches the live CSB runtime");
    memset(&terminal, 0, sizeof(terminal));
    CHECK(csb_v1_startup_session_terminal_receipt_pc34(
              (CSB_V1_StartupRuntimeAssetSession_PC34 *)
                  view.csbStartupRuntimeAssetSession, &terminal) &&
              terminal.valid && terminal.c017_ready && terminal.c040_ready,
          "F31 title, Switch and Game handoff reaches the real C017/C040 terminal session");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    for (tick = 0u; tick < sizeof(framebuffer); ++tick) {
        if (framebuffer[tick] != 0u) {
            live_frame_nonblack = 1;
            break;
        }
    }
    CHECK(live_frame_nonblack,
          "F31 C017 HUD and F0128 viewport draw a real live frame after Prison");
    M11_GameView_Shutdown(&view);
    if (failures) return 1;
    puts("PASS: real FM Towns SWITCHTW -> CHTWE entrance handoff");
    return 0;
}
