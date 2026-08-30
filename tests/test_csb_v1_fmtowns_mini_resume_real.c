/* End-to-end CSB FM Towns MINI.DAT bootstrap proof.
 *
 * MINI.DAT is the authenticated F31 bootstrap state shipped with the real
 * CD, not a player-created CSBGAME slot.  It must enter through the normal
 * CHTWE/CHTWJ startup route, where M11 keeps its archive member in RAM.  Do
 * not pass it through savePath: that boundary is deliberately reserved for
 * a native C5 user save and must reject a bootstrap image. */

#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    const char *language = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE");
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    M11_BootProbeReceipt probe;
    unsigned tick;
    const int japanese = language && strcmp(language, "ja") == 0;

    if (!archive || !archive[0] ||
        (language && language[0] && strcmp(language, "en") != 0 &&
         !japanese)) {
        puts("SKIP: CSB FM Towns archive and en/ja language are required");
        return 77;
    }

    memset(&spec, 0, sizeof(spec));
    spec.title = "CHAOS STRIKES BACK";
    spec.gameId = "csb";
    spec.sourceId = "csb";
    spec.dataDir = archive;
    spec.verifiedAssetMd5 = japanese
        ? "761d6fc588b31aeaaa9caf3725e111b9"
        : "405b757038eea3c263e60f240854d6de";
    spec.csbFmtownsJapanese = japanese;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;

    M11_GameView_Init(&view);
    check(M11_GameView_Start(&view, &spec) == 1,
          "authentic F31 media enters the M11 title startup boundary");
    /* MINI.DAT is loaded by CHTWE/CHTWJ after the independently-owned
     * ANIMTW TITLE.ANM and SWITCHTW programs.  A direct M11 start is not a
     * direct dungeon start: drive the exact F31 program sequence and the
     * visible C004 Prison command before asking the boot probe for the live
     * F0435 state. */
    for (tick = 0u; tick < 630u; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    for (tick = 0u; tick < 80u &&
                    view.csbFmtownsSwitchVblanksRemaining != 0u; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    check(view.csbFmtownsSwitchBound &&
              view.csbFmtownsSwitchVblanksRemaining == 0u,
          "TITLE.ANM reaches the ready source-owned SWITCHTW page");
    check(M11_GameView_HandlePointerButton(&view, 52, 110,
                                           DM1_V1_MOUSE_MASK_LEFT_PC34) ==
              M11_GAME_INPUT_REDRAW &&
              view.csbFmtownsGameHandoffReceipt.valid &&
              view.csbState.startup_entrance_active,
          "SWITCHTW Game enters the selected language-owned CHTWE/CHTWJ route");
    check(M11_GameView_HandlePointerButton(&view, 250, 50,
                                           DM1_V1_MOUSE_MASK_LEFT_PC34) ==
              M11_GAME_INPUT_REDRAW &&
              view.csbState.startup_entrance_opening_active,
          "C004 Prison command starts the native F31 entrance transition");
    for (tick = 0u; tick < 240u && view.csbState.startup_entrance_active;
         ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    memset(&probe, 0, sizeof(probe));
    check(M11_GameView_GetBootProbeReceipt(&view, &probe) &&
              !probe.startupActive && probe.levelLoaded &&
              probe.mapIndex == 4 && probe.partyX == 22 && probe.partyY == 18 &&
              probe.partyDir == 2 && probe.championCount == 1,
          "MINI.DAT publishes its source-owned bootstrap party pose");
    check(view.csbBootProfile &&
              ((const CSB_V1_BootProfile *)view.csbBootProfile)->variant_id ==
                  (japanese ? CSB_V1_VARIANT_FMTOWNS_JA
                            : CSB_V1_VARIANT_FMTOWNS_EN) &&
              view.csbFmtownsGameHandoffReceipt.valid,
          "MINI.DAT startup remains bound to the selected CHTWE/CHTWJ profile");
    check(M11_GameView_HandleInput(&view, M12_MENU_INPUT_TURN_RIGHT) ==
              M11_GAME_INPUT_REDRAW,
          "bootstrapped MINI.DAT session accepts a source-owned turn");
    M11_GameView_Shutdown(&view);
    if (failures) return 1;
    printf("PASS: authentic CSB FM Towns %s MINI.DAT boots through M11\n",
           japanese ? "Japanese" : "English");
    return 0;
}
