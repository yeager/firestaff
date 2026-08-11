/* End-to-end CSB FM Towns MINI.DAT resume proof.
 * MINI.DAT is the authenticated F31 bootstrap save shipped with the real
 * CD.  This test exercises the same M11 savePath boundary used by Resume;
 * it does not manufacture a save body or substitute the invalid external
 * CSBGAME candidates. */

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
    const char *root = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR");
    const char *language = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE");
    char save_path[1024];
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    M11_BootProbeReceipt probe;
    const int japanese = language && strcmp(language, "ja") == 0;
    const char *tree = japanese ? "CJDATA" : "CDATA";

    if (!root || !root[0] ||
        (language && language[0] && strcmp(language, "en") != 0 &&
         !japanese)) {
        puts("SKIP: CSB FM Towns loose root and en/ja language are required");
        return 0;
    }
    if (snprintf(save_path, sizeof(save_path), "%s/%s/MINI.DAT", root,
                 tree) <= 0 || strlen(save_path) >= sizeof(save_path)) {
        return 1;
    }

    memset(&spec, 0, sizeof(spec));
    spec.title = "CHAOS STRIKES BACK";
    spec.gameId = "csb";
    spec.sourceId = "csb";
    spec.dataDir = root;
    spec.savePath = save_path;
    spec.csbFmtownsJapanese = japanese;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;

    M11_GameView_Init(&view);
    check(M11_GameView_Start(&view, &spec) == 1,
          "authentic MINI.DAT enters the M11 CSB resume boundary");
    memset(&probe, 0, sizeof(probe));
    check(M11_GameView_GetBootProbeReceipt(&view, &probe) &&
              !probe.startupActive && probe.levelLoaded &&
              probe.mapIndex == 4 && probe.partyX == 22 && probe.partyY == 18 &&
              probe.partyDir == 2 && probe.championCount == 1,
          "MINI.DAT publishes its source-owned party pose without title replay");
    check(view.csbBootProfile &&
              ((const CSB_V1_BootProfile *)view.csbBootProfile)->variant_id ==
                  (japanese ? CSB_V1_VARIANT_FMTOWNS_JA
                            : CSB_V1_VARIANT_FMTOWNS_EN) &&
              view.csbFmtownsGameHandoffReceipt.valid,
          "MINI.DAT resume remains bound to the selected CHTWE/CHTWJ profile");
    check(M11_GameView_HandleInput(&view, M12_MENU_INPUT_TURN_RIGHT) ==
              M11_GAME_INPUT_REDRAW,
          "resumed MINI.DAT session accepts a source-owned turn");
    M11_GameView_Shutdown(&view);
    if (failures) return 1;
    printf("PASS: authentic CSB FM Towns %s MINI.DAT resumes through M11\n",
           japanese ? "Japanese" : "English");
    return 0;
}
