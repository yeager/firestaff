#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

static int open_game(const char* dataDir,
                     M12_StartupMenuState* menu,
                     M11_GameViewState* game)
{
    M12_StartupMenu_InitWithDataDir(menu, dataDir, NULL);
    M11_GameView_Init(game);
    return M11_GameView_OpenSelectedMenuEntry(game, menu);
}

static int expected_backing_graphic_for_f0107_view(int viewWallIndex)
{
    /* DUNVIEW.C F0107 advances the 345/346 pair for every projection
     * except the native D3 and D2 side slots 0, 1, 5 and 6. */
    return 345 + ((viewWallIndex >= 2 && viewWallIndex != 5 &&
                   viewWallIndex != 6) ? 1 : 0);
}

int main(int argc, char** argv)
{
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const struct DungeonMapDesc_Compat* map;
    unsigned char framebuffer[320 * 200];
    int sideCount = 0;
    int depthBackingCount = 0;
    int clearedAfterMaterial = 0;
    int previousHadMaterial = 0;
    int ok = 1;
    int x;
    int y;
    int direction;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    if (!open_game(argv[1], &menu, &game) || !game.world.dungeon ||
        game.world.dungeon->header.mapCount < 1 || !game.assetsAvailable) {
        fprintf(stderr, "FAIL could not open real DM1 PC34 data\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    map = &game.world.dungeon->maps[0];
    for (x = 0; x < (int)map->width; ++x) {
        for (y = 0; y < (int)map->height; ++y) {
            for (direction = 0; direction < 4; ++direction) {
                M11_Dm1HoCMirrorViewportMaterialFrameReceipt frame;
                int frameHadMaterial = 0;
                int i;

                game.world.party.mapIndex = 0;
                game.world.party.mapX = x;
                game.world.party.mapY = y;
                game.world.party.direction = direction;
                memset(framebuffer, 0, sizeof(framebuffer));
                M11_GameView_Draw(&game, framebuffer, 320, 200);
                memset(&frame, 0, sizeof(frame));
                M11_GameView_GetDm1HoCMirrorViewportMaterialFrameReceipt(&frame);

                for (i = 0; i < frame.count; ++i) {
                    const M11_Dm1HoCMirrorViewportMaterialReceipt* entry =
                        &frame.entries[i];
                    if (!entry->valid || entry->renderIndex < 0 ||
                        entry->renderIndex >= game.mirrorCatalog.count ||
                        !entry->suppressHostFallbackVisuals) {
                        fprintf(stderr, "FAIL invalid C127 viewport material receipt\n");
                        ok = 0;
                        continue;
                    }
                    if (entry->relativeForward == 1 &&
                        entry->relativeSide == 0) {
                        /* D1C's C346/C026 decision depends on the separate
                         * live F0115 route. Its dedicated all-ordinal test
                         * owns that contract; this test covers F0107's
                         * side/depth C345/C346 materialization. */
                        continue;
                    } else if (entry->relativeForward >= 1) {
                        if (!entry->materialized ||
                            entry->backingGraphicIndex !=
                                expected_backing_graphic_for_f0107_view(
                                    entry->viewWallIndex) ||
                            entry->portraitGraphicIndex != -1 ||
                            !entry->suppressChampionPortrait) {
                            fprintf(stderr,
                                    "FAIL D%d C127 %d view=%d material=%d backing=%d portrait=%d suppress=%d\n",
                                    entry->relativeForward, entry->renderIndex,
                                    entry->viewWallIndex,
                                    entry->materialized,
                                    entry->backingGraphicIndex,
                                    entry->portraitGraphicIndex,
                                    entry->suppressChampionPortrait);
                            ok = 0;
                        }
                        if (entry->relativeForward == 1) ++sideCount;
                        else ++depthBackingCount;
                        frameHadMaterial = 1;
                    }
                }
                if (previousHadMaterial && frame.count == 0) {
                    clearedAfterMaterial = 1;
                }
                previousHadMaterial = frameHadMaterial;
            }
        }
    }

    if (sideCount <= 0 || depthBackingCount <= 0 || !clearedAfterMaterial) {
        fprintf(stderr,
                "FAIL C127 viewport coverage side=%d depth=%d clear=%d\n",
                sideCount, depthBackingCount, clearedAfterMaterial);
        ok = 0;
    }

    M11_GameView_Shutdown(&game);
    if (ok) {
        puts("ok: real HoC C127 F0107 C345/C346 side/depth material, no C026 fallback");
    }
    return ok ? 0 : 1;
}
