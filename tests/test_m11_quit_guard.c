/* M11 quit-guard test — source-locked to ReDMCSB LOADSAVE.C:1371-1379.
 *
 * Verifies that ESC (M12_MENU_INPUT_BACK) surfaces the "GAME NOT SAVED"
 * prompt instead of the plain "RETURN TO START MENU?" confirm when the
 * dungeon tick has advanced more than 100 ticks past both lastSaveTick
 * and loadGameTick.  This is a data-free M11 dialog-path gate: it seeds the
 * minimal active game-view state directly instead of starting the DM1 asset
 * loader, because the quit guard only depends on the tick anchors.
 *
 * Mirrors the ReDMCSB compound condition:
 *     (G0313_ul_GameTime > (G0319_ul_LoadGameTime + 100)) &&
 *     (G0313_ul_GameTime > (G2018_ul_LastSaveTime + 100))
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

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

static void seed_active_view(M11_GameViewState* view,
                             uint32_t gameTick,
                             uint32_t lastSaveTick,
                             uint32_t loadGameTick) {
    M11_GameView_Init(view);
    view->active = 1;
    snprintf(view->sourceId, sizeof(view->sourceId), "dm1");
    view->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    view->world.gameTick = gameTick;
    view->lastSaveTick = lastSaveTick;
    view->loadGameTick = loadGameTick;
    view->dialogOverlayActive = 0;
    view->returnToMenuConfirmActive = 0;
    view->quitGuardActive = 0;
}

static void expect_plain_confirm(const M11_GameViewState* view,
                                 const char* label) {
    char msg[160];
    snprintf(msg, sizeof(msg), "%s: dialog active", label);
    expect(view->dialogOverlayActive == 1, msg);
    snprintf(msg, sizeof(msg), "%s: confirm flag set", label);
    expect(view->returnToMenuConfirmActive == 1, msg);
    snprintf(msg, sizeof(msg), "%s: quit-guard not active", label);
    expect(view->quitGuardActive == 0, msg);
    snprintf(msg, sizeof(msg), "%s: plain confirm text", label);
    expect(strstr(view->dialogOverlayText, "RETURN TO START MENU") != NULL, msg);
    snprintf(msg, sizeof(msg), "%s: YES choice", label);
    expect(strcmp(view->dialogChoices[0], "YES") == 0, msg);
    snprintf(msg, sizeof(msg), "%s: NO choice", label);
    expect(strcmp(view->dialogChoices[1], "NO") == 0, msg);
}

static void expect_unsaved_confirm(const M11_GameViewState* view,
                                   const char* label) {
    char msg[160];
    snprintf(msg, sizeof(msg), "%s: dialog active", label);
    expect(view->dialogOverlayActive == 1, msg);
    snprintf(msg, sizeof(msg), "%s: confirm flag set", label);
    expect(view->returnToMenuConfirmActive == 1, msg);
    snprintf(msg, sizeof(msg), "%s: quit-guard active", label);
    expect(view->quitGuardActive == 1, msg);
    snprintf(msg, sizeof(msg), "%s: unsaved prompt text", label);
    expect(strstr(view->dialogOverlayText, "GAME NOT SAVED") != NULL, msg);
    snprintf(msg, sizeof(msg), "%s: SAVE AND QUIT choice", label);
    expect(strcmp(view->dialogChoices[0], "SAVE AND QUIT") == 0, msg);
    snprintf(msg, sizeof(msg), "%s: CANCEL choice", label);
    expect(strcmp(view->dialogChoices[1], "CANCEL") == 0, msg);
}

int main(void) {
    M11_GameViewState view;
    M11_GameInputResult result;

    /* Case A: fresh game, no progress beyond initial tick. BACK should
     * show the plain RETURN TO START MENU? confirm (quitGuardActive == 0). */
    seed_active_view(&view, 10, 0, 0);
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(result == M11_GAME_INPUT_REDRAW, "case A: BACK redraws");
    expect_plain_confirm(&view, "case A");
    M11_GameView_Shutdown(&view);

    /* Case B: tick advanced 200 ticks past both save anchors -> quit guard. */
    seed_active_view(&view, 200, 0, 0);
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(result == M11_GAME_INPUT_REDRAW, "case B: BACK redraws");
    expect_unsaved_confirm(&view, "case B");
    M11_GameView_Shutdown(&view);

    /* Case C: tick just past lastSaveTick but within 100 of loadGameTick
     * -> the compound AND must keep quitGuardActive == 0. */
    seed_active_view(&view, 250, 0, 200); /* 250 > 100, but 250 <= 300. */
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(result == M11_GAME_INPUT_REDRAW, "case C: BACK redraws");
    expect_plain_confirm(&view, "case C");
    M11_GameView_Shutdown(&view);

    /* Case D: exactly +100 from both anchors is still source-safe; the
     * ReDMCSB condition is strict >, not >=. */
    seed_active_view(&view, 200, 100, 100);
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(result == M11_GAME_INPUT_REDRAW, "case D: BACK redraws");
    expect_plain_confirm(&view, "case D");
    M11_GameView_Shutdown(&view);

    /* Case E: +101 from both anchors enters the unsaved prompt. */
    seed_active_view(&view, 201, 100, 100);
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(result == M11_GAME_INPUT_REDRAW, "case E: BACK redraws");
    expect_unsaved_confirm(&view, "case E");
    M11_GameView_Shutdown(&view);

    /* Case F: map/inventory overlays get first refusal on BACK.  A later BACK
     * opens the quit modal and clears the remaining spell panel first. */
    seed_active_view(&view, 201, 100, 100);
    view.inventoryPanelActive = 1;
    view.mapOverlayActive = 1;
    view.spellPanelOpen = 1;
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(result == M11_GAME_INPUT_REDRAW, "case F: overlay BACK redraws");
    expect(view.dialogOverlayActive == 0, "case F: first BACK does not open modal");
    expect(view.inventoryPanelActive == 0, "case F: inventory overlay cleared first");
    expect(view.mapOverlayActive == 0, "case F: map overlay cleared first");
    expect(view.spellPanelOpen == 1, "case F: spell panel waits for modal BACK");
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(result == M11_GAME_INPUT_REDRAW, "case F: modal BACK redraws");
    expect_unsaved_confirm(&view, "case F modal");
    expect(view.spellPanelOpen == 0, "case F: spell panel cleared before modal");
    M11_GameView_Shutdown(&view);

    /* Case G: cancel from quit-guard dialog clears the flag and dismisses. */
    seed_active_view(&view, 500, 0, 0);
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK);
    expect(result == M11_GAME_INPUT_REDRAW, "case G: enter redraws");
    expect_unsaved_confirm(&view, "case G enter");
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK); /* cancel */
    expect(result == M11_GAME_INPUT_REDRAW, "case G: cancel redraws");
    expect(view.quitGuardActive == 0, "case G: cancel clears guard");
    expect(view.dialogOverlayActive == 0, "case G: dialog dismissed");
    M11_GameView_Shutdown(&view);

    if (fails == 0) {
        puts("ok: data-free G2018 quit-guard prompt and boundary checks");
        return 0;
    }
    return 1;
}
