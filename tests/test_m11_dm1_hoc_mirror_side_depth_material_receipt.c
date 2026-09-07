#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* pc34_graphics_path(void)
{
    static char path[2048];
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    size_t rootLength;

    if (!root || !root[0]) return NULL;
    rootLength = strlen(root);
    /* The real-data root is deliberately kept as original archives.  This
     * gate must therefore open GRAPHICS.DAT through Firestaff's native ZIP
     * reader rather than silently requiring an extracted test fixture.  A
     * direct archive remains useful for isolated runners. */
    if (rootLength > 4 && strcmp(root + rootLength - 4, ".zip") == 0) {
        snprintf(path, sizeof(path), "%s::DATA/GRAPHICS.DAT", root);
    } else {
        snprintf(path, sizeof(path),
                 "%s/Dungeon-Master_DOS_EN_Version-34.zip::DATA/GRAPHICS.DAT",
                 root);
    }
    return path;
}

int main(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState state;
    M11_Dm1WallOrnamentHostPresentationReceipt wall;
    M11_Dm1HoCMirrorHostPresentationReceipt mirror;
    unsigned char framebuffer[320 * 200];
    static const struct {
        int relForward;
        int relSide;
        int viewWallIndex;
    } cases[] = {
        {1, -1, 10}, {1, 1, 11}, {2, -1, 5}, {2, 0, 8},
        {2, 1, 9}, {3, -2, 13}, {3, 0, 3}, {3, 2, 14},
        {3, -1, 2}, {3, 1, 4}
    };
    size_t index;

    if (!graphicsPath) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_AssetLoader_Init(&state.assetLoader, graphicsPath)) {
        M11_GameView_Shutdown(&state);
        fputs("configured PC34 GRAPHICS.DAT failed to load\n", stderr);
        return 1;
    }
    state.assetsAvailable = 1;
    state.sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;

    for (index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        memset(framebuffer, 0, sizeof(framebuffer));
        if (!M11_GameView_ProbeDrawDm1ChampionMirrorBackingHostReceipt(
                &state, cases[index].relForward, cases[index].relSide,
                framebuffer, 320, 200)) {
            fprintf(stderr, "D%d side %d C127 backing draw failed\n",
                    cases[index].relForward, cases[index].relSide);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        memset(&wall, 0, sizeof(wall));
        M11_GameView_GetDm1WallOrnamentHostPresentationReceipt(&wall);
        if (!wall.valid || wall.globalOrnamentIndex != 43 ||
            wall.viewWallIndex != cases[index].viewWallIndex ||
            wall.transparentColor != 10 || wall.width <= 0 || wall.height <= 0 ||
            wall.flipHorizontal !=
                (cases[index].viewWallIndex == 1 ||
                 cases[index].viewWallIndex == 6 ||
                 cases[index].viewWallIndex == 11 ||
                 cases[index].viewWallIndex == 14)) {
            fprintf(stderr, "D%d side %d C127 backing receipt drifted\n",
                    cases[index].relForward, cases[index].relSide);
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
    puts("ok: real PC34 C127 F0107 C346 side/depth material, no C026 fallback");
    return 0;
}
