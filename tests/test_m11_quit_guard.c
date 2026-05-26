/* M11 quit-guard test — source-locked to ReDMCSB LOADSAVE.C:1371-1379.
 *
 * Verifies that ESC (M12_MENU_INPUT_BACK) surfaces the "GAME NOT SAVED"
 * prompt instead of the plain "RETURN TO START MENU?" confirm when the
 * dungeon tick has advanced more than 100 ticks past both lastSaveTick
 * and loadGameTick.
 *
 * Mirrors the ReDMCSB compound condition:
 *     (G0313_ul_GameTime > (G0319_ul_LoadGameTime + 100)) &&
 *     (G0313_ul_GameTime > (G2018_ul_LastSaveTime + 100))
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int fails = 0;
static void expect(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        ++fails;
    }
}

int main(void) {
    const char* dataDir = getenv("FIRESTAFF_DM1_CANONICAL_DIR");
    M11_GameViewState view;
    M11_GameLaunchSpec spec;

    if (!dataDir || dataDir[0] == '\0') {
        dataDir = "/Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1";
    }
    if (access(dataDir, R_OK) != 0) {
        printf("skip: DM1 canonical dir not available: %s\n", dataDir);
        return 0;
    }

    memset(&spec, 0, sizeof(spec));
    spec.title = "DUNGEON MASTER";
    spec.gameId = "dm1";
    spec.dataDir = dataDir;
    spec.sourceId = "dm1";
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&view);
    expect(M11_GameView_Start(&view, &spec), "DM1 start should succeed");

    /* Case A: fresh game, no progress beyond initial tick.  ESC should
     * show the plain RETURN TO START MENU? confirm (quitGuardActive == 0). */
    view.world.gameTick = 10;
    view.lastSaveTick = 0;
    view.loadGameTick = 0;
    view.dialogOverlayActive = 0;
    view.returnToMenuConfirmActive = 0;
    view.quitGuardActive = 0;
    M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(view.dialogOverlayActive == 1, "case A: dialog active");
    expect(view.returnToMenuConfirmActive == 1, "case A: confirm flag set");
    expect(view.quitGuardActive == 0, "case A: quit-guard NOT active (tick<=100)");

    /* Case B: tick advanced 200 ticks past both save anchors -> quit guard. */
    M11_GameView_DismissDialogOverlay(&view);
    view.world.gameTick = 200;
    view.lastSaveTick = 0;
    view.loadGameTick = 0;
    view.dialogOverlayActive = 0;
    view.returnToMenuConfirmActive = 0;
    view.quitGuardActive = 0;
    M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(view.dialogOverlayActive == 1, "case B: dialog active");
    expect(view.returnToMenuConfirmActive == 1, "case B: confirm flag set");
    expect(view.quitGuardActive == 1, "case B: quit-guard ACTIVE");

    /* Case C: tick just past lastSaveTick but within 100 of loadGameTick
     * -> the compound AND must keep quitGuardActive == 0. */
    M11_GameView_DismissDialogOverlay(&view);
    view.world.gameTick = 250;
    view.lastSaveTick = 0;     /* 250 > 100 -> true */
    view.loadGameTick = 200;   /* 250 > 300 -> false */
    view.dialogOverlayActive = 0;
    view.returnToMenuConfirmActive = 0;
    view.quitGuardActive = 0;
    M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(view.quitGuardActive == 0, "case C: quit-guard NOT active (compound AND fails)");

    /* Case D: cancel from quit-guard dialog clears the flag. */
    M11_GameView_DismissDialogOverlay(&view);
    view.world.gameTick = 500;
    view.lastSaveTick = 0;
    view.loadGameTick = 0;
    view.dialogOverlayActive = 0;
    view.returnToMenuConfirmActive = 0;
    view.quitGuardActive = 0;
    M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(view.quitGuardActive == 1, "case D: enter quit-guard");
    M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK); /* cancel */
    expect(view.quitGuardActive == 0, "case D: cancel clears guard");
    expect(view.dialogOverlayActive == 0, "case D: dialog dismissed");

    M11_GameView_Shutdown(&view);

    if (fails == 0) {
        puts("ok: G2018 quit-guard surfaces UNSAVED prompt on advanced tick");
        return 0;
    }
    return 1;
}
