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
    M11_Dm1WallOrnamentHostPresentationReceipt wall;
    M11_Dm1HoCMirrorHostPresentationReceipt mirror;
    unsigned char framebuffer[320 * 200];
    int side;

    if (!graphicsPath) return 0;
    M11_GameView_Init(&state);
    if (!M11_AssetLoader_Init(&state.assetLoader, graphicsPath)) {
        M11_GameView_Shutdown(&state);
        return getenv("FIRESTAFF_DM1_GRAPHICS_DAT") ? 1 : 0;
    }
    state.assetsAvailable = 1;
    state.sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;

    for (side = -1; side <= 1; side += 2) {
        memset(framebuffer, 0, sizeof(framebuffer));
        if (!M11_GameView_ProbeDrawDm1ChampionMirrorSideBackingHostReceipt(
                &state, side, framebuffer, 320, 200)) {
            fprintf(stderr, "D1%s C127 backing draw failed\n",
                    side < 0 ? "L" : "R");
            M11_GameView_Shutdown(&state);
            return 1;
        }
        memset(&wall, 0, sizeof(wall));
        M11_GameView_GetDm1WallOrnamentHostPresentationReceipt(&wall);
        if (!wall.valid || wall.globalOrnamentIndex != 43 ||
            wall.viewWallIndex != (side < 0 ? 10 : 11) ||
            wall.transparentColor != 10 || wall.width <= 0 || wall.height <= 0 ||
            wall.flipHorizontal != (side > 0)) {
            fprintf(stderr, "D1%s C127 backing receipt drifted\n",
                    side < 0 ? "L" : "R");
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    memset(&mirror, 0, sizeof(mirror));
    M11_GameView_GetDm1HoCMirrorHostPresentationReceipt(&mirror);
    if (mirror.valid) {
        fprintf(stderr, "D1 side C127 path published forbidden C026 receipt\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 C127 D1L/D1R C346 material, no C026 fallback");
    return 0;
}
