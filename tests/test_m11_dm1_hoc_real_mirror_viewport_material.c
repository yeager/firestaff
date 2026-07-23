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

int main(int argc, char** argv)
{
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const struct DungeonMapDesc_Compat* map;
    unsigned char framebuffer[320 * 200];
    int frontSeen[32];
    int frontCount = 0;
    int sideCount = 0;
    int depthSuppressedCount = 0;
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
    memset(frontSeen, 0, sizeof(frontSeen));
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
                        if (!entry->materialized ||
                            entry->backingGraphicIndex != 346 ||
                            entry->portraitGraphicIndex != 26 ||
                            entry->suppressChampionPortrait) {
                            fprintf(stderr,
                                    "FAIL D1C C127 %d lacks real C346/C026 material\n",
                                    entry->renderIndex);
                            ok = 0;
                        }
                        if (!frontSeen[entry->renderIndex]) {
                            frontSeen[entry->renderIndex] = 1;
                            ++frontCount;
                        }
                        frameHadMaterial = 1;
                    } else if (entry->relativeForward == 1 &&
                               (entry->relativeSide == -1 ||
                                entry->relativeSide == 1)) {
                        if (!entry->materialized ||
                            entry->backingGraphicIndex != 346 ||
                            entry->portraitGraphicIndex != -1 ||
                            !entry->suppressChampionPortrait) {
                            fprintf(stderr,
                                    "FAIL D1 side C127 %d used global=%d material=%d backing=%d portrait=%d suppress=%d\n",
                                    entry->renderIndex, entry->globalOrnamentIndex,
                                    entry->materialized,
                                    entry->backingGraphicIndex,
                                    entry->portraitGraphicIndex,
                                    entry->suppressChampionPortrait);
                            ok = 0;
                        }
                        ++sideCount;
                        frameHadMaterial = 1;
                    } else if (entry->relativeForward >= 2) {
                        if (entry->materialized || entry->backingGraphicIndex != -1 ||
                            entry->portraitGraphicIndex != -1 ||
                            !entry->suppressChampionPortrait) {
                            fprintf(stderr,
                                    "FAIL D%d C127 %d manufactured distant mirror art\n",
                                    entry->relativeForward, entry->renderIndex);
                            ok = 0;
                        }
                        ++depthSuppressedCount;
                    }
                }
                if (previousHadMaterial && frame.count == 0) {
                    clearedAfterMaterial = 1;
                }
                previousHadMaterial = frameHadMaterial;
            }
        }
    }

    if (frontCount != 24) {
        fprintf(stderr, "FAIL front real C127 material count got=%d want=24\n",
                frontCount);
        ok = 0;
    }
    for (x = 0; x < 24; ++x) {
        if (!frontSeen[x]) {
            fprintf(stderr, "FAIL front C127 portrait %d was not materialized\n", x);
            ok = 0;
        }
    }
    if (sideCount <= 0 || depthSuppressedCount <= 0 || !clearedAfterMaterial) {
        fprintf(stderr,
                "FAIL C127 viewport coverage side=%d depth=%d clear=%d\n",
                sideCount, depthSuppressedCount, clearedAfterMaterial);
        ok = 0;
    }

    M11_GameView_Shutdown(&game);
    if (ok) {
        puts("ok: real HoC C127 C346/C026 front, C346 side, and D2 clear-only material");
    }
    return ok ? 0 : 1;
}
