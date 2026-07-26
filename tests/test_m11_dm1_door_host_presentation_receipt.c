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
    M11_Dm1DoorHostPresentationReceipt receipt;
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
    if (!M11_GameView_ProbeDrawDm1CenterDoorHostReceipt(
            &state, framebuffer, 320, 200)) {
        fprintf(stderr, "real PC34 center-door material draw failed\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1DoorHostPresentationReceipt(&receipt);
    if (!receipt.valid || receipt.depthIndex != 0 || receipt.doorState != 4 ||
        !receipt.panelVisible || receipt.frameCount != 3 || receipt.blitCount != 4 ||
        receipt.graphicsId[0] != 91 || receipt.graphicsId[1] != 87 ||
        receipt.graphicsId[2] != 87 || receipt.graphicsId[3] != 248 ||
        receipt.width[3] != 96 || receipt.height[3] != 86) {
        fprintf(stderr, "real PC34 center-door receipt incomplete\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 F0111 center-door host material receipt");
    return 0;
}
