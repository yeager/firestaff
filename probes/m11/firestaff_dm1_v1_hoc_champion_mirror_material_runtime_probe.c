/*
 * DM1 V1 Hall of Champions champion-mirror material receipt probe.
 *
 * This is deliberately a real-data route: it searches the loaded Hall map
 * for a source-visible C127 sensor, then binds that sensor's live ordinal to
 * the C026 atlas cell and the D1C destination without manufacturing a
 * sensor, atlas, or framebuffer surface.
 *
 * ReDMCSB: DUNGEON.C F0172 lines 2573,2608-2612; DUNVIEW.C F0107 lines
 * 3913-3928; ENTRANCE.C routes the opening Hall view into the dungeon view.
 */
#include "dm1_v1_probe_assets.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_Y = 33,
    ATLAS_W = 256,
    ATLAS_H = 87,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    PORTRAIT_DST_X = 96,
    PORTRAIT_DST_Y = VIEWPORT_Y + 35,
    TRANSPARENT_INDEX = 1,
    MIN_MATCH_PERCENT = 90
};

static int expect(const char* label, int condition) {
    printf("%s %s\n", condition ? "PASS" : "FAIL", label);
    return condition;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuInitOptions options;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* atlas;
    unsigned char framebuffer[FB_W * FB_H];
    int matched = 0;
    int compared = 0;
    int paletteIndex = -1;
    int ordinal = -1;
    int poseX = -1;
    int poseY = -1;
    int poseDirection = -1;
    int sourceX;
    int sourceY;
    int x;
    int y;
    int ok = 1;

    if (argc < 2 || !argv[1] || !argv[1][0]) {
        printf("SKIP dm1 HoC champion mirror material: no DATA_DIR supplied\n");
        return 0;
    }
    dataDir = argv[1];
    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&menu, dataDir, "dm1", &options);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu) || !game.assetsAvailable) {
        printf("SKIP dm1 HoC champion mirror material: data unavailable at %s\n",
               dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    atlas = dm1_v1_probe_load_c026_champion_portrait_atlas(&game.assetLoader);
    if (!atlas || !atlas->loaded || !atlas->pixels ||
        atlas->width != ATLAS_W || atlas->height != ATLAS_H) {
        printf("SKIP dm1 HoC champion mirror material: C026 unavailable\n");
        M11_GameView_Shutdown(&game);
        return 0;
    }

    {
        int y;
        for (y = 0; y < (int)game.world.dungeon->maps[0].height && ordinal < 0; ++y) {
            int x;
            for (x = 0; x < (int)game.world.dungeon->maps[0].width && ordinal < 0; ++x) {
                int direction;
                for (direction = 0; direction < 4; ++direction) {
                    game.world.party.mapIndex = 0;
                    game.world.party.mapX = x;
                    game.world.party.mapY = y;
                    game.world.party.direction = direction;
                    game.showDebugHUD = 0;
                    game.candidateMirrorPanelActive = 0;
                    game.candidateMirrorOrdinal = -1;
                    game.candidateMirrorPartyIndex = -1;
                    ordinal = M11_GameView_GetFrontMirrorOrdinal(&game);
                    if (ordinal >= 0) {
                        poseX = x;
                        poseY = y;
                        poseDirection = direction;
                        break;
                    }
                }
            }
        }
    }
    if (ordinal < 0) {
        printf("SKIP dm1 HoC champion mirror material: no source-visible C127 route in %s\n",
               dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    sourceX = (ordinal & 7) * PORTRAIT_W;
    sourceY = (ordinal >> 3) * PORTRAIT_H;
    printf("INFO live C127 pose=(%d,%d,%d) ordinal=%d\n",
           poseX, poseY, poseDirection, ordinal);
    ok &= expect("live C127 route supplies a C026 ordinal", ordinal < 24);
    ok &= expect("live C026 source cell is inside the 256x87 atlas",
                 sourceX + PORTRAIT_W <= (int)atlas->width &&
                 sourceY + PORTRAIT_H <= (int)atlas->height);
    ok &= expect("D1C destination is framebuffer (96,68), 32x29",
                 PORTRAIT_DST_X == 96 && PORTRAIT_DST_Y == 68);

    for (y = 0; y < PORTRAIT_H && paletteIndex < 0; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int index = atlas->pixels[(sourceY + y) * (int)atlas->width +
                                      sourceX + x] & 0x0F;
            if (index != TRANSPARENT_INDEX) {
                paletteIndex = index;
                break;
            }
        }
    }
    ok &= expect("C026 cell has an opaque palette-indexed pixel", paletteIndex >= 0);
    ok &= expect("V1 palette resolves the C026 opaque index",
                 paletteIndex >= 0 && paletteIndex < 16 &&
                 G9010_auc_VgaPaletteAll_Compat[0][paletteIndex][0] <= 63 &&
                 G9010_auc_VgaPaletteAll_Compat[0][paletteIndex][1] <= 63 &&
                 G9010_auc_VgaPaletteAll_Compat[0][paletteIndex][2] <= 63);

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&game, framebuffer, FB_W, FB_H);
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char source = (unsigned char)(atlas->pixels[
                (sourceY + y) * (int)atlas->width + sourceX + x] & 0x0F);
            unsigned char destination = (unsigned char)M11_FB_DECODE_INDEX(
                framebuffer[(PORTRAIT_DST_Y + y) * FB_W + PORTRAIT_DST_X + x]);
            if (source == TRANSPARENT_INDEX) {
                continue;
            }
            ++compared;
            if (source == destination) {
                ++matched;
            }
        }
    }
    printf("INFO C026 ordinal=%d source=(%d,%d,32,29) dst=(96,68,32,29) "
           "palette-index=%d matched=%d/%d\n",
           ordinal, sourceX, sourceY, paletteIndex, matched, compared);
    ok &= expect("C026 source material reaches the D1C destination",
                 compared > 0 && matched * 100 >= compared * MIN_MATCH_PERCENT);

    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
