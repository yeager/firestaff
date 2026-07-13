#include "m11_game_view.h"
#include "dm1_v1_creature_render_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* pc34_graphics_path(void)
{
    static char path[1024];
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
    M11_Dm1CreatureHostPresentationReceipt receipt;
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
    if (!M11_GameView_ProbeDrawDm1CreatureHostReceipt(
            &state, framebuffer, 320, 200)) {
        fprintf(stderr, "real PC34 F0115 creature blit failed\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1CreatureHostPresentationReceipt(&receipt);
    if (!receipt.valid || !receipt.creatureLane ||
        receipt.creatureType != DM1_CREATURE_MUMMY || receipt.depthIndex != 1 ||
        receipt.graphicsId <= 0 || receipt.transparentColor < 0 ||
        receipt.destinationW <= 0 || receipt.destinationH <= 0 ||
        receipt.assetWidth <= 0 || receipt.assetHeight <= 0 ||
        receipt.paletteChecksum == 0) {
        fprintf(stderr, "real PC34 F0115 creature receipt incomplete\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 F0115 creature host material receipt");
    return 0;
}
