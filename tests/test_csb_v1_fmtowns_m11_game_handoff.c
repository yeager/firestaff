/* Real FM Towns CSB Game-program handoff.
 *
 * This opt-in test uses a materialized original F31E/F31J media root. It
 * proves that the presentation path reaches SWITCHTW, consumes its Game
 * rectangle, and opens the language-owned C004 entrance session. No generated
 * title, switch, entrance or HUD pixels are accepted.
 */
#include "m11_game_view.h"
#include "asset_status_m12.h"
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
    const char *archive_data_dir =
        getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE_DATA_DIR");
    const char *language_name = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE");
    const char *version_id;
    const char *expected_program;
    CSB_V1_FmtownsSwitchLanguage language;
    char materialized_data_dir[M12_ASSET_DATA_DIR_CAPACITY];
    M12_AssetStatus asset_status;
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
    uint8_t music_track;

    if (language_name && strcmp(language_name, "ja") == 0) {
        language = CSB_FMTOWNS_SWITCH_JAPANESE;
        version_id = "fmtowns-ja";
        expected_program = "CHTWJ.EXP";
    } else if (!language_name || language_name[0] == '\0' ||
               strcmp(language_name, "en") == 0) {
        language = CSB_FMTOWNS_SWITCH_ENGLISH;
        version_id = "fmtowns-en";
        expected_program = "CHTWE.EXP";
    } else {
        fprintf(stderr, "SKIP: unsupported FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE\n");
        return 0;
    }
    memset(materialized_data_dir, 0, sizeof(materialized_data_dir));
    memset(&asset_status, 0, sizeof(asset_status));
    if (archive_data_dir && archive_data_dir[0]) {
        M12_AssetStatus_ScanGame(&asset_status, archive_data_dir, "csb");
        if (!M12_AssetStatus_MaterializeCSBRuntimeVersion(
                &asset_status, version_id, materialized_data_dir,
                sizeof(materialized_data_dir))) {
            fprintf(stderr, "SKIP: verified FM Towns %s archive unavailable\n",
                    version_id);
            return 0;
        }
        data_dir = materialized_data_dir;
    }
    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR or "
             "FIRESTAFF_CSB_FMTOWNS_ARCHIVE_DATA_DIR not set");
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
          "verified F31 media opens its real TITLE.ANM owner");
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
          "real F31 GRAPHICS.DAT opens its C001--C005/C017/C040 session");
    CHECK(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &direct_session, &direct_runtime),
          "real F31 session satisfies the authenticated runtime surface set");
    csb_v1_boot_startup_runtime_asset_session_release_pc34(&direct_session);
    memset(&direct_handoff, 0, sizeof(direct_handoff));
    CHECK(csb_v1_fmtowns_game_handoff_open(
              (const CSB_V1_BootProfile *)view.csbBootProfile,
              language, &direct_handoff) &&
              strcmp(direct_handoff.executable_name, expected_program) == 0 &&
              direct_handoff.music_table_verified &&
              csb_v1_fmtowns_game_music_track_at(&direct_handoff, 0u, 2u, 0u,
                                                  &music_track),
          "verified F31 profile resolves its language-owned Game program");

    /* ReDMCSB SWITCH.C F2279 registers G4171 at (47,105), 62x39. */
    result = M11_GameView_HandlePointerButton(&view, 52, 110,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    CHECK(result == M11_GAME_INPUT_REDRAW,
          "SWITCHTW Game rectangle is handled as a modal source action");
    CHECK(view.csbFmtownsGameHandoffReceipt.valid &&
              view.csbFmtownsGameHandoffReceipt.executable_verified &&
              strcmp(view.csbFmtownsGameHandoffReceipt.executable_name,
                     expected_program) == 0 &&
              view.csbStartupRuntimeAssetSession &&
              view.csbState.startup_entrance_active &&
              !view.csbState.startup_title_active,
          "Game click opens only the verified language-owned entrance owner");
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
          "F31 Game accepts the source-owned Prison command");
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
    printf("PASS: real FM Towns SWITCHTW -> %s entrance handoff\n",
           expected_program);
    return 0;
}
