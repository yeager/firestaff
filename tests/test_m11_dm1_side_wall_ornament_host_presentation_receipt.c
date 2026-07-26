#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* pc34_graphics_path(void)
{
    static char path[2048];
    const char* configured = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char* home;
    if (configured && configured[0]) return configured;
    home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    return path;
}

int main(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState state;
    M11_Dm1WallOrnamentHostPresentationReceipt receipt;
    unsigned char framebuffer[320 * 200];

    if (!graphicsPath) return 0;
    M11_GameView_Init(&state);
    if (!M11_AssetLoader_Init(&state.assetLoader, graphicsPath)) {
        if (getenv("FIRESTAFF_DM1_GRAPHICS_DAT")) {
            fprintf(stderr, "configured PC34 GRAPHICS.DAT failed to load\n");
            return 1;
        }
        M11_GameView_Shutdown(&state);
        return 0;
    }
    state.assetsAvailable = 1;
    state.sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;
    memset(framebuffer, 0, sizeof(framebuffer));
    if (!M11_GameView_ProbeDrawDm1SideWallOrnamentHostReceipt(
            &state, framebuffer, 320, 200)) {
        fprintf(stderr, "real PC34 side-wall ornament material draw failed\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1WallOrnamentHostPresentationReceipt(&receipt);
    if (!receipt.valid || receipt.globalOrnamentIndex != 1 ||
        receipt.viewWallIndex != 1 || receipt.graphicIndex != 261 ||
        receipt.transparentColor != 10 || !receipt.flipHorizontal ||
        !receipt.paletteMapValid || receipt.paletteMap[2] != 12 ||
        receipt.width <= 0 || receipt.height <= 0) {
        fprintf(stderr, "real PC34 side-wall ornament receipt incomplete\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 F0107 side-wall ornament host material receipt");
    return 0;
}
