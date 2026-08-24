/* Opt-in M11 regression for DM2 DOS's source-owned NEW GAME handoff. */

#include "m11_game_view.h"
#include "render_sdl_m11.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_startup_menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DOS_ROOT");
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    DM2_V1_StartupMenuPointerLayout layout;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ROOT is not set");
        return 0;
    }
    memset(&spec, 0, sizeof(spec));
    spec.gameId = "dm2";
    spec.sourceId = "dm2";
    spec.title = "DUNGEON MASTER II";
    spec.dataDir = root;
    spec.presentationWidth = M11_FB_WIDTH;
    spec.presentationHeight = M11_FB_HEIGHT;
    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec)) {
        fputs("FAIL: DM2 DOS real archive did not enter M11\n", stderr);
        return 1;
    }
    memset(&layout, 0, sizeof(layout));
    if (!dm2_v1_boot_startup_menu_pointer_layout(
            (DM2_V1_BootProfile *)view.dm2BootProfile, &layout) ||
        !layout.valid || layout.new_game.w <= 0 || layout.new_game.h <= 0) {
        fputs("FAIL: DM2 DOS real GDAT lacks a New Game rectangle\n", stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    /* Enter is the DOS title's source 0xd7 action; M11 resolves it through
     * this verified GDAT rectangle rather than an invented menu row. */
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        view.dm2State.startup_menu_active || !view.dm2State.level_loaded ||
        view.world.party.championCount < 1) {
        fputs("FAIL: DM2 DOS New Game did not commit the source runtime\n", stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    M11_GameView_Shutdown(&view);
    puts("PASS: DM2 DOS M11 New Game commits real-media runtime");
    return 0;
}
